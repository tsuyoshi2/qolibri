/***************************************************************************
*   Copyright (C) 2007 by BOP                                             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
//#include <QtCore>
#ifdef USE_STATIC_PLUGIN
#include <QtPlugin>
#endif

#if defined (USE_STATIC_PLUGIN)
Q_IMPORT_PLUGIN(qjpcodecs)
Q_IMPORT_PLUGIN(qjpeg)
#endif

#if defined (USE_STATIC_PLUGIN) && defined (USE_GIF_FOR_FONT)
Q_IMPORT_PLUGIN(qgif)
#endif

#include "mainwindow.h"
#include "configure.h"
#include "method.h"
#include "server.h"
#include "client.h"

QoServer *server;

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(qolibri);

    QApplication app(argc, argv);
    QString searchText;
    int port = -1;
    bool qserv = false;

    for(int i=1; i<argc; i++){
        QString str = QString::fromLocal8Bit(argv[i]).toLower();
        if (str.toLower() == "-c" && (i+1) < argc) {
            CONF->settingOrg = QString::fromLocal8Bit(argv[i+1]);
            i++;
       
        } else if (str.toLower() == "-p" && (i+1) < argc) {
            port = QString::fromLocal8Bit(argv[i+1]).toInt();
            i++;
        } else if (str.toLower() == "-s") {
            qserv = true;
            if (port < 0) port = 0;
        } else {
            searchText += str;
            if ( (i+1) < argc) searchText += " ";
        }
    }

    if (qserv) {
        QoClient client("localhost", port);
        if (client.connectHost()) {
            client.sendText(searchText.toLocal8Bit());
            return 1;
        }
    }

    QString locale = QLocale::system().name();
#if defined (Q_WS_MAC) || defined (Q_WS_WIN)
    QString path = QCoreApplication::applicationDirPath() + "/i18n";
#else
    QString path = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
    QTranslator trans;
    trans.load(QString("qolibri_") + locale, path);
    app.installTranslator(&trans);

    //QTranslator transQt;
    //trans.load(QString("qt_") + locale, path);
    //app.installTranslator(&transQt);

    MainWindow mainWin;

    mainWin.show();

    if (qserv) {
        server = new QoServer(port);
        server->slotSearchText(&mainWin,
                               SLOT(searchClientText(const QString&)));
        server->slotShowStatus(&mainWin,
                               SLOT(showStatus(const QString&)));
        server->showStatus(QString("Start as server (port = %1)")
                                  .arg(server->serverPort()));
    }
    if (!searchText.isEmpty()) {
        mainWin.searchClientText(searchText);
    }

    return app.exec();
}

