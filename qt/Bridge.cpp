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

#include "bridge.hpp"

#include <COKit/COKit.hxx>

#include <qt/DBusService.hpp>
#include <qt/DocumentOperations.hpp>
#include <net/FakeSocket.hpp>
#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <common/MobileApp.hpp>
#include <common/Protocol.hpp>
#include <qt/QtClipboard.hpp>
#include <qt/qt.hpp>
#include <common/Util.hpp>
#include <qt/WebView.hpp>
#include <common/JsonUtil.hpp>
#include <common/SettingsStorage.hpp>
#include <common/StringVector.hpp>
#include <Poco/Path.h>
#include <Poco/URI.h>

#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMainWindow>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QWidget>

static const int SHOW_JS_MAXLEN = 300;

namespace
{
    void closeStarterScreen()
    {
        WebView* starterScreen = WebView::findStarterScreen();
        if (starterScreen)
        {
            LOG_TRC("Closing starter screen after document action");
            QTimer::singleShot(0, [starterScreen]() {
                if (starterScreen->getMainWindow())
                {
                    starterScreen->getMainWindow()->close();
                }
            });
        }
    }
} // namespace

Bridge::~Bridge() {
    if (_document._fakeClientFd != -1) {
        fakeSocketClose(_document._fakeClientFd);
    }
    if (_app2js.joinable()) {
        fakeSocketClose(_closeNotificationPipeForForwardingThread[0]);
        _app2js.join();
    }
}

void Bridge::createAndStartMessagePumpThread()
{
    // Create pipe for close notifications
    fakeSocketPipe2(_closeNotificationPipeForForwardingThread);

    // Thread pumping Online → JS
    assert(!_app2js.joinable());
    _app2js = std::thread(
        [this]
        {
            ProcUtil::setThreadName("app2js");
            bool unexpectedClose = false;
            while (true)
            {
                struct pollfd pfd[2];
                pfd[0].fd = _document._fakeClientFd;
                pfd[0].events = POLLIN;
                pfd[1].fd = _closeNotificationPipeForForwardingThread[1];
                pfd[1].events = POLLIN;
                if (fakeSocketPoll(pfd, 2, -1) > 0)
                {
                    if (pfd[1].revents & POLLIN)
                    {
                        break; // document closed
                    }
                    if (pfd[0].revents & POLLIN)
                    {
                        int n = fakeSocketAvailableDataLength(_document._fakeClientFd);
                        if (n == 0)
                        {
                            LOG_TRC("Socket closed #" << _document._fakeClientFd);
                            unexpectedClose = true;
                            break;
                        }
                        std::vector<char> buf(n);
                        fakeSocketRead(_document._fakeClientFd, buf.data(), n);
                        send2JS(buf);
                    }
                    if (pfd[0].revents & POLLERR)
                    {
                        LOG_TRC("Socket error #" << _document._fakeClientFd);
                        unexpectedClose = true;
                        break;
                    }
                }
            }
            LOG_TRC("Closing message pump thread");
            fakeSocketClose(_closeNotificationPipeForForwardingThread[1]);
            fakeSocketClose(_document._fakeClientFd);
            if (unexpectedClose) {
                LOG_WRN("Unexpected closing of message pump thread; closing window now");
                QMetaObject::invokeMethod(_window, "close", Qt::QueuedConnection);
            }
        });
}

void Bridge::evalJS(const std::string& script)
{
    // Ensure execution on GUI thread – queued if needed
    QMetaObject::invokeMethod(
        // TODO: fix needless `this` captures...
        _webView, [this, script]
        { _webView->page()->runJavaScript(QString::fromStdString(script)); },
        Qt::QueuedConnection);
}

void Bridge::send2JS(const std::vector<char>& buffer)
{
    if (buffer.empty())
        return;

    const std::string_view bufferView(buffer.data(), buffer.size());

    LOG_TRC_NOFILE(
        "Send to JS: " << COOLProtocol::getAbbreviatedMessage(bufferView.data(), bufferView.size()));

    // Determine if message is of a binary type
    bool binaryMessage = std::any_of(
        std::begin(COOLProtocol::binaryMessageTypes),
        std::end(COOLProtocol::binaryMessageTypes),
        [&](const char* type) {
            return bufferView.starts_with(type);
        }
    );

    QByteArray base64 = QByteArray(bufferView.data(), bufferView.size()).toBase64();

    std::string pretext = binaryMessage
                              ? "window.TheFakeWebSocket.onmessage({'data': window.atob('"
                              : "window.TheFakeWebSocket.onmessage({'data': window.b64d('";
    const std::string posttext = "')});";

    std::string js = pretext + base64.toStdString() + posttext;

    std::string subjs = js.substr(0, std::min<std::string::size_type>(SHOW_JS_MAXLEN, js.length()));
    if (js.length() > SHOW_JS_MAXLEN)
        subjs += "...";
    LOG_TRC_NOFILE("Evaluating JavaScript: " << subjs);

    evalJS(js);
}

void Bridge::debug(const QString& msg) { LOG_TRC_NOFILE("From JS: debug: " << msg.toStdString()); }

void Bridge::error(const QString& msg) { LOG_TRC_NOFILE("From JS: error: " << msg.toStdString()); }

void Bridge::promptSaveLocation(std::function<void(const std::string&, const std::string&)> callback)
{
    // Prompt user to pick a save location and format
    kit::Document* loKitDoc = DocumentData::get(_document._appDocId).loKitDocument;
    if (!loKitDoc)
    {
        LOG_ERR("promptSaveLocation: no loKitDocument");
        return;
    }

    const int docType = loKitDoc->getDocumentType();
    const auto formats = getSaveAsFormats(docType);

    if (formats.empty())
    {
        LOG_ERR("promptSaveLocation: no formats available for document type");
        return;
    }

    // Get document info for suggested filename
    const QUrl docUrl(QString::fromStdString(_document._fileURL.toString()));
    const QString docPath = docUrl.isLocalFile() ? docUrl.toLocalFile() : docUrl.toString();
    const QFileInfo docInfo(docPath);
    const QString baseName = docInfo.completeBaseName().isEmpty()
                               ? QStringLiteral("document")
                               : docInfo.completeBaseName();

    // Build file filter string with all available formats
    QString fileFilter;
    for (size_t i = 0; i < formats.size(); ++i)
    {
        if (i > 0)
            fileFilter += QStringLiteral(";;");
        fileFilter += formats[i].displayName + QStringLiteral(" (*.") + formats[i].extension + QStringLiteral(")");
    }

    QFileDialog* dialog = new QFileDialog(
        _webView,
        QObject::tr("Save Document"),
        QDir::home().filePath(baseName),
        fileFilter);

    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // Set default suffix to enforce extension in GUI
    if (!formats.empty())
        dialog->setDefaultSuffix(formats[0].extension);

    // Update default suffix when user changes the selected filter
    QObject::connect(dialog, QOverload<const QString&>::of(&QFileDialog::filterSelected),
                     [dialog, formats](const QString& selectedFilter)
                     {
                         for (const auto& fmt : formats)
                         {
                             if (selectedFilter.startsWith(fmt.displayName))
                             {
                                 dialog->setDefaultSuffix(fmt.extension);
                                 break;
                             }
                         }
                     });

    QObject::connect(dialog, &QFileDialog::fileSelected,
                     [callback, dialog, formats](const QString& destPath)
                     {
                         // Get the selected filter to determine the format
                         QString selectedFilter = dialog->selectedNameFilter();
                         QString format;

                         // Extract format from the selected filter (e.g., "ODF text document (*.odt)" -> "odt")
                         for (const auto& fmt : formats)
                         {
                             if (selectedFilter.startsWith(fmt.displayName))
                             {
                                 format = fmt.extension;
                                 break;
                             }
                         }

                         if (format.isEmpty() && !formats.empty())
                             format = formats[0].extension;

                         // Ensure file ends with the selected format's extension - save-as fails otherwise!
                         QString finalPath = destPath;
                         if (!finalPath.endsWith("." + format, Qt::CaseInsensitive))
                         {
                             finalPath += "." + format;
                         }

                         callback(finalPath.toStdString(), format.toStdString());
                     });

    dialog->open();
}

void Bridge::saveDocumentAs()
{
    promptSaveLocation([fakeClientFd = _document._fakeClientFd](const std::string& destPath, const std::string& format) {
            const QFileInfo destInfo(QString::fromStdString(destPath));

            Poco::URI destUri("file", destInfo.absoluteFilePath().toStdString());

            // Send saveas command to COOLWSD with the selected format
            std::string saveasCmd = "saveas url=" + destUri.toString() +
                                     " format=" + format +
                                     " options=";
            fakeSocketWriteQueue(fakeClientFd, saveasCmd.c_str(), saveasCmd.size());
    });
}

QVariant Bridge::cool(const QString& messageStr)
{
    const std::string message = messageStr.toStdString();
    LOG_TRC_NOFILE("From JS: cool: " << message);

    const StringVector tokens = StringVector::tokenize(message);
    if (tokens.empty())
        return {};

    if (tokens.equals(0, "HULLO"))
    {
        // Skip for starter screen (no document connection needed)
        if (_document._fakeClientFd == -1) {
            LOG_TRC_NOFILE("Starter screen - skipping COOLWSD connection");
            return {};
        }

        // JS side fully initialised – open our fake WebSocket to COOLWSD
        assert(coolwsd_server_socket_fd != -1);
        int rc = fakeSocketConnect(_document._fakeClientFd, coolwsd_server_socket_fd);
        assert(rc != -1);

        createAndStartMessagePumpThread();

        // 1st request: the initial GET /?file_path=...  (mimic WebSocket upgrade)
        std::string initMsg(_document._fileURL.toString() +
                            (" " + std::to_string(_document._appDocId)));
        fakeSocketWriteQueue(_document._fakeClientFd, initMsg.c_str(), initMsg.size());
    }
    else if (tokens.equals(0, "loaddocument"))
    {
        // loaddocument url=file:///path/to/file.odt
        std::string newFileUrl;
        if (!COOLProtocol::getTokenString(tokens, "url", newFileUrl))
        {
            LOG_ERR("loaddocument: no url= specified");
            return {};
        }

        LOG_TRC_NOFILE("loaddocument: switching to URL: " << newFileUrl);

        // Close the existing fakesocket
        if (_document._fakeClientFd != -1) {
            fakeSocketClose(_document._fakeClientFd);
            _document._fakeClientFd = -1;
        }
        // Close the existing forwarding thread
        if (_app2js.joinable()) {
            fakeSocketClose(_closeNotificationPipeForForwardingThread[0]);
            _app2js.join();
        }

        // Create a new fakesocket
        _document._fakeClientFd = fakeSocketSocket();
        // Generate a new appDocId
        _document._appDocId = coda::generateNewAppDocId();
        // Update the file URL
        _document._fileURL = Poco::URI(newFileUrl);

        LOG_TRC_NOFILE("loaddocument: created new appDocId=" << _document._appDocId);

        // Connect to COOLWSD
        int rc = fakeSocketConnect(_document._fakeClientFd, coolwsd_server_socket_fd);
        if (rc == -1) {
            LOG_ERR("loaddocument: failed to connect fakesocket");
            return {};
        }

        createAndStartMessagePumpThread();

        // Send the initial message with the new file URL and appDocId
        std::string initialMessage(_document._fileURL.toString() +
                                   (" " + std::to_string(_document._appDocId)));
        fakeSocketWriteQueue(_document._fakeClientFd, initialMessage.c_str(), initialMessage.size());

        // Update window title with new filename
        Poco::Path uriPath(_document._fileURL.getPath());
        QString fileName = QString::fromStdString(uriPath.getFileName());
        QString windowTitle = fileName + " - " APP_NAME;
        if (_window)
            _window->setWindowTitle(windowTitle);

        // Add the new document location to recent files
        Application::getRecentFiles().add(_document._fileURL.toString());

        LOG_TRC_NOFILE("loaddocument: sent initial message with new appDocId");
        return {};
    }
    else if (tokens.equals(0, "WELCOME"))
    {
        const std::string welcomePath = getDataDir() + "/browser/dist/welcome/welcome-slideshow.odp";
        struct stat st;
        if (FileUtil::getStatOfFile(welcomePath, st) == 0)
        {
            Poco::URI fileURL{Poco::Path(welcomePath)};
            QMainWindow* window = _window;
            QTimer::singleShot(0, [fileURL, window]() {
                WebView* webViewInstance = new WebView(Application::getProfile(), /*isWelcome*/ true, window);
                webViewInstance->load(fileURL);
            });
            LOG_TRC_NOFILE("Opening welcome slideshow: " << welcomePath);
        }
        else
        {
            LOG_TRC_NOFILE("Welcome slideshow not found at: " << welcomePath);
        }
    }
    else if (tokens.equals(0, "LICENSE"))
    {
        const std::string licensePath = LO_PATH "/LICENSE.html";
        struct stat st;
        if (FileUtil::getStatOfFile(licensePath, st) == 0)
        {
            const QUrl url = QUrl::fromLocalFile(QString::fromStdString(licensePath));
            QDesktopServices::openUrl(url);
            LOG_TRC_NOFILE("Opening LICENSE.html: " << licensePath);
        }
        else
        {
            LOG_TRC_NOFILE("LICENSE.html not found at: " << licensePath);
        }
    }
    else if (tokens.equals(0, "COMMANDSTATECHANGED"))
    {
        Poco::JSON::Object::Ptr object;
        if (!JsonUtil::parseJSON(tokens.substrFromToken(1), object))
            return {};

        const std::string commandName = object->get("commandName").toString();
        if (commandName != ".uno:ModifiedStatus")
            return {};

        const bool modified = (object->get("state").toString() == "true");
        LOG_TRC_NOFILE("Document modified status changed: " << (modified ? "modified" : "unmodified"));
    }
    else if (tokens.equals(0, "COMMANDRESULT"))
    {
        Poco::JSON::Object::Ptr object;
        if (!JsonUtil::parseJSON(tokens.substrFromToken(1), object))
            return {};

        const std::string commandName = object->get("commandName").toString();
        const bool success = object->get("success").convert<bool>();
        bool wasModified = false;
        if (object->has("wasModified"))
        {
            wasModified = object->get("wasModified").convert<bool>();
        }

        bool isAutosave = false;
        if (object->has("isAutosave"))
        {
            isAutosave = object->get("isAutosave").convert<bool>();
        }

        if (commandName == ".uno:Copy" || commandName == ".uno:Cut"
            || commandName == ".uno:CopySlide")
        {
            getClipboard(_document._appDocId, [this]() {
                evalJS("if (window.app && window.app.map && window.app.map.uiManager) "
                              "window.app.map.uiManager.closeSnackbar();");
                _copyInProgress = false;
            });
        }

        // only handle successful .uno:Save commands
        // let manually triggered saves through even if the document is not modified.
        if (commandName != ".uno:Save" || !success || (!wasModified && isAutosave))
            return {};
    }
    else if (tokens.equals(0, "UPLOADSETTINGS"))
    {
        Desktop::uploadSettings(tokens.substrFromToken(1));
        return {};
    }
    else if (tokens.equals(0, "FETCHSETTINGSFILE"))
    {
        auto result = Desktop::fetchSettingsFile(tokens.substrFromToken(1));
        if (result.content.empty())
            return {};

        QVariantMap resultMap;
        resultMap["fileName"] = QString::fromStdString(result.fileName);
        resultMap["mimeType"] = QString::fromStdString(result.mimeType);
        resultMap["content"] = QString::fromStdString(result.content);

        return resultMap;
    }
    else if (tokens.equals(0, "FETCHSETTINGSCONFIG"))
    {
        return QString::fromStdString(Desktop::fetchSettingsConfig());
    }
    else if (tokens.equals(0, "SYNCSETTINGS"))
    {
        Desktop::syncSettings([this](const std::vector<char>& data) {
            send2JS(data);
        });
        return {};
    }
    else if (tokens.equals(0, "PROCESSINTEGRATORADMINFILE"))
    {
        Desktop::processIntegratorAdminFile(tokens.substrFromToken(1));
        return {};
    }
    else if (tokens.equals(0, "BYE"))
    {
        LOG_TRC_NOFILE("Document window terminating on JavaScript side → closing fake socket");
        fakeSocketClose(_closeNotificationPipeForForwardingThread[0]);

        QTimer::singleShot(0, [this]() {
            if (_webView)
            {
                QWidget* topLevel = _webView->window();
                if (topLevel)
                {
                    LOG_INF("Closing document window");
                    topLevel->hide();
                    topLevel->close();
                    topLevel->deleteLater();
                }
            }
        });
    }
    else if (tokens.equals(0, "EXIT_TEST"))
    {
        LOG_INF("EXIT_TEST received -- closing document and quitting");
        fakeSocketClose(_closeNotificationPipeForForwardingThread[0]);
        QTimer::singleShot(0, [this]() {
            if (_webView)
            {
                QWidget* topLevel = _webView->window();
                if (topLevel)
                {
                    topLevel->hide();
                    topLevel->close();
                    topLevel->deleteLater();
                }
            }
            QApplication::quit();
        });
    }
    else if (tokens.equals(0, "TEXTCLIPBOARD"))
    {
        QString text = QString::fromStdString(tokens.substrFromToken(1));
        QApplication::clipboard()->setText(text);
    }
    else if (tokens.equals(0, "COPY") || tokens.equals(0, "COPYSLIDE") || tokens.equals(0, "CUT"))
    {
        _copyInProgress = true;

        // show a progress/snackbar while copy is in progress.
        evalJS("if (window.app && window.app.map && window.app.map.uiManager) "
               "window.app.map.uiManager.showProgressBar("
               "window._(''), null, null, -1, false, true);");

        std::string unoCmd;
        if (tokens.equals(0, "CUT"))
            unoCmd = "uno .uno:Cut";
        else if (tokens.equals(0, "COPYSLIDE"))
            unoCmd = "uno .uno:CopySlide";
        else
            unoCmd = "uno .uno:Copy";

        fakeSocketWriteQueue(_document._fakeClientFd, unoCmd.c_str(), unoCmd.size());
    }
    else if (tokens.equals(0, "PASTE"))
    {
        if (_copyInProgress)
        {
            LOG_DBG("Ignoring PASTE while copy is still in progress");
            return {};
        }
        // Sync system clipboard → LOKit internal clipboard only if an external app
        // wrote the clipboard since our last copy (same logic as Windows do_paste_or_read).
        if (!QApplication::clipboard()->ownsClipboard() ||
            sClipboardSourceDocId.load() != _document._appDocId)
            setClipboard(_document._appDocId);
        static const std::string pasteCmd = "uno .uno:Paste";
        fakeSocketWriteQueue(_document._fakeClientFd, pasteCmd.c_str(), pasteCmd.size());
    }
    else if (tokens.equals(0, "PASTESPECIAL"))
    {
        if (_copyInProgress)
        {
            LOG_DBG("Ignoring PASTESPECIAL while copy is still in progress");
            return {};
        }
        if (!QApplication::clipboard()->ownsClipboard() ||
            sClipboardSourceDocId.load() != _document._appDocId)
            setClipboard(_document._appDocId);
        static const std::string pasteCmd = "uno .uno:PasteSpecial";
        fakeSocketWriteQueue(_document._fakeClientFd, pasteCmd.c_str(), pasteCmd.size());
    }
    else if (tokens.equals(0, "GETRECENTDOCS"))
    {
        QString result = QString::fromStdString(Application::getRecentFiles().serialise());
        LOG_TRC_NOFILE("GETRECENTDOCS: returning recent documents");
        return result;
    }
    else if (tokens.equals(0, "FULLSCREENPRESENTATION"))
    {
        if (_webView)
        {
            if (tokens.equals(1, "true"))
                _webView->createPresentationFS();
            else
                _webView->destroyPresentationFS();
        }
    }
    else if (message == "uno .uno:Open")
    {
        QFileDialog* dialog =
            new QFileDialog(_webView, QObject::tr("Open File"), QString(),
                            QObject::tr("All Files (*);;"
                                        "Text Documents (*.odt *.ott *.doc *.docx *.rtf *.txt);;"
                                        "Spreadsheets (*.ods *.ots *.xls *.xlsx *.csv);;"
                                        "Presentations (*.odp *.otp *.ppt *.pptx)"));

        dialog->setFileMode(QFileDialog::ExistingFiles);
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        QObject::connect(dialog, &QFileDialog::filesSelected,
                         [](const QStringList& filePaths)
                         {
                            coda::openFiles(filePaths);
                             // Close starter screen if it exists
                             closeStarterScreen();
                         });

        dialog->open();
    }
    else if (message == "uno .uno:NewDoc" || message == "uno .uno:NewDocText")
    {
        WebView* webViewInstance = WebView::createNewDocument(Application::getProfile(), "writer", {}, {});
        if (!webViewInstance)
        {
            LOG_ERR("Failed to create new text document");
        }
    }
    else if (message == "uno .uno:NewDocSpreadsheet")
    {
        WebView* webViewInstance = WebView::createNewDocument(Application::getProfile(), "calc", {}, {});
        if (!webViewInstance)
        {
            LOG_ERR("Failed to create new spreadsheet");
        }
    }
    else if (message == "uno .uno:NewDocPresentation")
    {
        WebView* webViewInstance = WebView::createNewDocument(Application::getProfile(), "impress", {}, {});
        if (!webViewInstance)
        {
            LOG_ERR("Failed to create new presentation");
        }
    }
    else if (message == "uno .uno:NewDocDraw")
    {
        WebView* webViewInstance = WebView::createNewDocument(Application::getProfile(), "draw", {}, {});
        if (!webViewInstance)
        {
            LOG_ERR("Failed to create new drawing");
        }
    }
    else if (message == "uno .uno:SaveAs")
    {
        assert(_document._fakeClientFd != -1);

        saveDocumentAs();
    }
    else if (message == "uno .uno:CloseWin")
    {
        // Close the main window associated with this web view
        if (_webView && _webView->window())
        {
            _webView->window()->close();
        }
    }
    else if (tokens.equals(0, "PRINT"))
    {
        printDocument(_document._appDocId, _webView);
    }
    else if (tokens.equals(0, "EXCHANGEMONITORS"))
    {
        if (_webView)
            _webView->exchangeMonitors();
    }
    else if (tokens.equals(0, "downloadas"))
    {
        // Parse "format=" argument and handle "direct-" prefix
        std::string format;
        if (!COOLProtocol::getTokenString(tokens, "format", format))
        {
            LOG_ERR("downloadas: no format= specified");
            return {};
        }
        if (format.starts_with("direct-"))
            format.erase(0, strlen("direct-"));

        // Build a suggested filename from the current document
        const QUrl docUrl(QString::fromStdString(_document._fileURL.toString()));
        const QString docPath = docUrl.isLocalFile() ? docUrl.toLocalFile() : docUrl.toString();
        const QFileInfo docInfo(docPath);
        const QString baseName = docInfo.completeBaseName().isEmpty()
                                 ? QStringLiteral("document")
                                 : docInfo.completeBaseName();
        const QString suggestedName = baseName + "." + QString::fromStdString(format);

        // Ask the user for the destination
        QFileDialog* dialog = new QFileDialog(
            _webView,
            QObject::tr("Export As"),
            QDir::home().filePath(suggestedName),
            QObject::tr("All Files (*)"));

        dialog->setAcceptMode(QFileDialog::AcceptSave);
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        unsigned appDocId = _document._appDocId;
        QObject::connect(dialog, &QFileDialog::fileSelected,
                        [appDocId, format](const QString& destPath) {
            // Export directly to the chosen path
            kit::Document* loKitDoc = DocumentData::get(appDocId).loKitDocument;
            if (!loKitDoc)
            {
                LOG_ERR("downloadas: no loKitDocument");
                return;
            }

            const QUrl destUrl = QUrl::fromLocalFile(destPath);
            const QByteArray urlUtf8 =
                    destUrl.toString(QUrl::FullyEncoded | QUrl::PreferLocalFile).toUtf8();
            const QByteArray fmtUtf8 = QString::fromStdString(format).toUtf8();

            loKitDoc->saveAs(urlUtf8.constData(), fmtUtf8.constData(), nullptr);

            // Verify save
            const QFileInfo outInfo(destPath);
            if (!outInfo.exists() || outInfo.size() <= 0)
            {
                LOG_ERR("downloadas: failed to save to '" << destPath.toStdString() << "'");
                return;
            }

            LOG_INF("downloadas: exported to " << destPath.toStdString());
        });

        dialog->open();
    }
    else if (tokens.equals(0, "exportfile"))
    {
        // Used by both .uno:SaveGraphic (Save Image) and .uno:ExportToPDF
        // (PDF with options) — take the suggested filename from the source URL
        // so the dialog title and default name match what was actually written.
        std::string fileUrl;
        if (!COOLProtocol::getTokenString(tokens, "url", fileUrl))
        {
            LOG_ERR("exportfile: no url= specified");
            return {};
        }

        const QUrl srcUrl(QString::fromStdString(fileUrl));
        const QString srcPath = srcUrl.toLocalFile();
        if (srcPath.isEmpty() || !QFileInfo::exists(srcPath))
        {
            LOG_ERR("exportfile: source file not found: " << fileUrl);
            return {};
        }

        const QFileInfo srcInfo(srcPath);
        QString suggestedName = srcInfo.fileName();
        if (suggestedName.isEmpty())
        {
            const QString ext = srcInfo.suffix();
            suggestedName = QStringLiteral("export.") + (ext.isEmpty() ? QStringLiteral("bin") : ext);
        }

        QFileDialog* dialog = new QFileDialog(
            _webView,
            QObject::tr("Save File"),
            QDir::home().filePath(suggestedName),
            QObject::tr("All Files (*)"));

        dialog->setAcceptMode(QFileDialog::AcceptSave);
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        QObject::connect(dialog, &QFileDialog::fileSelected,
                        [srcPath](const QString& destPath) {
            if (QFile::exists(destPath))
                QFile::remove(destPath);
            if (!QFile::copy(srcPath, destPath))
            {
                LOG_ERR("exportfile: failed to copy to '" << destPath.toStdString() << "'");
                return;
            }
            LOG_INF("exportfile: saved to " << destPath.toStdString());
            QFile::remove(srcPath);
        });

        dialog->open();
    }
    else if (tokens.equals(0, "HYPERLINK"))
    {
        QString qurl = QString::fromStdString(tokens.substrFromToken(1));
        QDesktopServices::openUrl(QUrl::fromUserInput(qurl));
    }
    else if (tokens.equals(0, "opendoc"))
    {
        // e.g. "opendoc file=%2Fhome%2F...something.odt"
        std::string fileArg;
        if (!COOLProtocol::getTokenString(tokens, "file", fileArg))
        {
            LOG_ERR("opendoc: no file= specified");
            return {};
        }
        QString decodedUri = QUrl::fromPercentEncoding(QByteArray::fromStdString(fileArg));

        QUrl url(decodedUri);
        QString localPath = url.isLocalFile() ? url.toLocalFile() : decodedUri;

        QFileInfo fileInfo(localPath);
        if (!fileInfo.exists() || !fileInfo.isFile())
        {
            LOG_ERR("opendoc: file does not exist: " << localPath.toStdString());
            return {};
        }

        QString absolutePath = fileInfo.absoluteFilePath();
        coda::openFiles(QStringList() << absolutePath);

        LOG_INF("opendoc: opened file: " << absolutePath.toStdString());
        return {};
    }
    else if (tokens.equals(0, "newdoc"))
    {
        // e.g."newdoc type=writer template=%2Fhome%2F...something.ott basename=Cool%20Text%20CV"
        std::string typeToken, templateToken, basenameToken;
        if (!COOLProtocol::getTokenString(tokens, "type", typeToken))
        {
            LOG_ERR("No type parameter in message '" << message << "'");
            return {};
        }

        // template is optional
        COOLProtocol::getTokenString(tokens, "template", templateToken);

        // basename is optional, when provided overrides the default file
        // names like "Text Document (1).odt" to "basename (1).odt"
        COOLProtocol::getTokenString(tokens, "basename", basenameToken);

        std::string templatePath;
        if (!templateToken.empty())
        {
            templatePath =
                QUrl::fromPercentEncoding(QByteArray::fromStdString(templateToken)).toStdString();
        }

        std::string decodedBasename;
        Poco::URI::decode(basenameToken, decodedBasename);

        // Always create new window
        WebView* webViewInstance = WebView::createNewDocument(Application::getProfile(), typeToken,
                                                              templatePath, decodedBasename);
        if (!webViewInstance)
        {
            LOG_ERR("Failed to create new document of type: " << typeToken);
            return {};
        }

        // If this was triggered from a starter screen, close it
        closeStarterScreen();
    }
    else
    {
        // Forward arbitrary payload from JS → Online
        fakeSocketWriteQueue(_document._fakeClientFd, message.c_str(), message.size());
    }
    return {};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
