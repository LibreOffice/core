/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <wsd/COOLWSD.hpp>
#include <qt/DBusService.hpp>
#include <net/FakeSocket.hpp>
#include <common/Log.hpp>
#include <common/SettingsStorage.hpp>
#include <common/Util.hpp>
#include <qt/WebView.hpp>
#include <qt/qt.hpp>

#include <Poco/URI.h>

#include <QApplication>
#include <QByteArray>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDir>
#include <QLocale>
#include <QLoggingCategory>
#include <QString>
#include <QTranslator>
#include <QWebEngineProfile>
#include <QWebEngineUrlScheme>

#include <pwd.h>

#include <unistd.h>

#include <clocale>
#include <cstring>
#include <langinfo.h>

const char* user_name = nullptr;

int coolwsd_server_socket_fd = -1;
static COOLWSD* coolwsd = nullptr;
static std::thread coolwsdThread;

// Disable accessibility
void disableA11y() { qputenv("QT_LINUX_ACCESSIBILITY_ALWAYS_ON", "0"); }

namespace
{
    const char* getUserName()
    {
        static QByteArray storage;
        storage.clear();

        QDBusMessage message = QDBusMessage::createMethodCall(
            "org.freedesktop.portal.Desktop",
            "/org/freedesktop/portal/desktop",
            "org.freedesktop.portal.Accounts",
            "GetUserInformation"
        );

        QDBusReply<QVariantMap> reply = QDBusConnection::sessionBus().call(message);

        if (reply.isValid()) {
            QVariantMap map = reply.value();

            QString realName = map.value("realName").toString();
            QString userName = map.value("userName").toString();

            QString chosen;
            if (!realName.isEmpty())
                chosen = realName;
            else if (!userName.isEmpty())
                chosen = userName;

            if (!chosen.isEmpty()) {
                storage = chosen.toUtf8();
                return storage.constData();
            }
        }

        // fallback to /etc/passwd

        struct passwd *pw = getpwuid(getuid());
        if (pw) {
            if (pw->pw_gecos && pw->pw_gecos[0] != '\0') {
                QString gecos = QString::fromLocal8Bit(pw->pw_gecos);
                QString full = gecos.section(',', 0, 0);

                if (!full.isEmpty()) {
                    storage = full.toUtf8();
                    return storage.constData();
                }
            }

            // fallback to Linux username
            storage = QByteArray(pw->pw_name);
            return storage.constData();
        }

        return nullptr;
    }

    void stopServer()
    {
        LOG_TRC("Requesting shutdown");
        SigUtil::requestShutdown();

        // wait until coolwsdThread is torn down, so that we don't start cleaning up too early
        coolwsdThread.join();

        QWebEngineProfile* profile = Application::getProfile();
        if (profile) {
            profile->deleteLater();
        }
    }

    void updateBrowserEnvironment(void)
    {
        const char *varName = "QTWEBENGINE_CHROMIUM_FLAGS";
        std::string val = (getenv(varName) ? getenv(varName) : "");
        // avoiding a crasher bug around check-box state emission for now cool#14039
        val = "--disable-renderer-accessibility --force-renderer-accessibility=false " + val;
        setenv(varName, val.c_str(), 1);
    }
} // namespace

int main(int argc, char** argv)
{
    // The embedded engine needs a UTF-8 locale to open documents whose file
    // names contain non-ASCII characters (e.g. "Névtelen.odt"). The server
    // (COOLWSD::innerMain) forces this for itself, but deliberately exempts
    // QTAPP so we don't clobber the user's LC_ALL. An invalid or non-UTF-8
    // LANG (such as "hu" rather than "hu_HU.UTF-8") otherwise leaves LC_CTYPE
    // at ASCII, and the engine's osl text encoding falls back to ASCII_US and
    // fails to open the file. Force only LC_CTYPE, and only when the codeset
    // isn't already UTF-8, so the user's language and number/date formatting
    // are preserved. Setting it before QApplication (which runs
    // setlocale(LC_ALL, "")) makes the LC_CTYPE environment variable win for
    // that category process-wide.
    std::setlocale(LC_ALL, "");
    if (std::strcmp(nl_langinfo(CODESET), "UTF-8") != 0)
    {
        const char* loc = std::setlocale(LC_CTYPE, "C.UTF-8");
        if (!loc)
            loc = std::setlocale(LC_CTYPE, "en_US.UTF-8");
        if (loc)
            ::setenv("LC_CTYPE", loc, 1);
    }

    // Must run before QApplication.
    // LocalScheme + LocalAccessAllowed lets the file:// page reach cool:.
    {
        QWebEngineUrlScheme scheme("cool");
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::Path);
        scheme.setFlags(QWebEngineUrlScheme::SecureScheme
                      | QWebEngineUrlScheme::LocalScheme
                      | QWebEngineUrlScheme::LocalAccessAllowed
                      | QWebEngineUrlScheme::CorsEnabled);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    QApplication app(argc, argv);

    user_name = getUserName();

    updateBrowserEnvironment();

    QTranslator translator;
    QString locale = QLocale::system().name();
    QString appDir = QCoreApplication::applicationDirPath();
    QString dataDir = QDir(appDir + "/../share/coda-qt").absolutePath();

    if (translator.load("coda_" + locale, appDir + "/translations"))
        app.installTranslator(&translator);
    else if (translator.load("coda_" + locale, dataDir + "/translations"))
        app.installTranslator(&translator);

    // default application name
    QApplication::setApplicationName(APP_NAME);
    QApplication::setWindowIcon(QIcon::fromTheme("com.collaboraoffice.Office.startcenter"));

    QCommandLineParser argParser;
    argParser.setApplicationDescription("Collabora Office - Desktop Office Suite");
    argParser.addHelpOption();
    argParser.addVersionOption();

    QCommandLineOption debugOption(
        QStringList() << "d" << "debug",
        "Enable debug output (shortcut for --log-level=trace)."
    );
    QCommandLineOption logLevelOption(
        QStringList() << "log-level",
        "Set log level (none, fatal, critical, error, warning, notice, information, debug, trace).",
        "level",
        "warning"
    );
    QCommandLineOption logDisabledAreasOption(
        QStringList() << "log-disabled-areas",
        "Comma-separated list of log areas to disable (Generic, Pixel, Socket, WebSocket, Http, WebServer, Storage, WOPI, Admin, Javascript).",
        "areas",
        "Socket,WebSocket,Admin,Pixel"
    );
    QCommandLineOption textDocumentOption(
        QStringList() << "textdocument" << "writer",
        "Create a new text document."
    );
    QCommandLineOption spreadsheetOption(
        QStringList() << "spreadsheet" << "calc",
        "Create a new spreadsheet."
    );
    QCommandLineOption presentationOption(
        QStringList() << "presentation" << "impress",
        "Create a new presentation."
    );
    QCommandLineOption drawingOption(
        QStringList() << "drawing" << "draw",
        "Create a new vector drawing."
    );

    argParser.addOption(debugOption);
    argParser.addOption(logLevelOption);
    argParser.addOption(logDisabledAreasOption);
    argParser.addOption(textDocumentOption);
    argParser.addOption(spreadsheetOption);
    argParser.addOption(presentationOption);
    argParser.addOption(drawingOption);
    argParser.addPositionalArgument("DOCUMENT", "Document file(s) to open", "[DOCUMENT...]");
    argParser.process(app);
    QStringList files = argParser.positionalArguments();

    std::string logLevel = argParser.value(logLevelOption).toStdString();
    bool debugMode = argParser.isSet(debugOption);
    if (debugMode)
        logLevel = "trace";

    // Disable QtWebEngine's JavaScript console logging (js: ... messages) unless
    // in debug mode or user has set QT_LOGGING_RULES environment variable
    if (!debugMode && !qEnvironmentVariableIsSet("QT_LOGGING_RULES"))
        QLoggingCategory::setFilterRules(QStringLiteral("js=false"));

    Log::initialize(QApplication::applicationName().toStdString(), logLevel);
    Log::setDisabledAreas(argParser.value(logDisabledAreasOption).toStdString());

    ProcUtil::setThreadName("main");

    fakeSocketSetLoggingCallback([](const std::string& line) { LOG_TRC_NOFILE(line); });

    QStringList absoluteFiles;
    QString templateType;

    if (files.size() > 0)
    {
        // Convert relative paths to absolute paths
        for (const QString& file : files)
        {
            QFileInfo fileInfo(file);
            absoluteFiles << fileInfo.absoluteFilePath();
        }
    }
    else
    {
        if (argParser.isSet(presentationOption))
            templateType = "impress";
        else if (argParser.isSet(spreadsheetOption))
            templateType = "calc";
        else if (argParser.isSet(textDocumentOption))
            templateType = "writer";
        else if (argParser.isSet(drawingOption))
            templateType = "draw";
    }

    // single-instance using DBus: try to forward to existing instance
    if (DBusService::tryForwardToExistingInstance(absoluteFiles, templateType))
    {
        // Successfully forwarded to existing instance, exit
        return 0;
    }

    // The engine's xmlsecurity initializes NSS at startup and fails ("Error
    // initializing security context") when no Mozilla profile is available. We
    // don't have one on the desktop, so point NSS at a dedicated directory
    // under the app's config and let it create an empty database there. The
    // 'sql:' prefix forces the modern SQLite-based store (the legacy DBM format
    // NSS would otherwise default to is read-only/deprecated on most distros).
    {
        Poco::Path nssdb = Desktop::getConfigPath();
        nssdb.append("nssdb");
        Poco::File(nssdb).createDirectories();
        const std::string nssEnv = "sql:" + nssdb.toString();
        setenv("MOZILLA_CERTIFICATE_FOLDER", nssEnv.c_str(), 1);
    }

    // COOLWSD in a background thread
    coolwsdThread = std::thread(
        []
        {
            ProcUtil::setThreadName("app");
            char* argv_local[2] = { strdup("coda"), nullptr };
            coolwsd = new COOLWSD();
            coolwsd->run(1, argv_local);
            delete coolwsd;
            LOG_TRC("One run of COOLWSD completed");
        });

    Application::initialize();

    // register DBus service and object
    DBusService* dbusService = new DBusService(&app);
    DBusService::registerService(dbusService);

    if (!absoluteFiles.isEmpty())
    {
        coda::openFiles(absoluteFiles);
    }
    else if (!templateType.isEmpty())
    {
        coda::openNewDocument(templateType);
    }
    else
    {
        WebView* starterView = new WebView(Application::getProfile());
        starterView->load(Poco::URI(), false, true);
    }

    auto const ret = app.exec();
    stopServer();
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
