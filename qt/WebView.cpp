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

#include "WebView.hpp"

#include <qt/bridge.hpp>
#include <net/FakeSocket.hpp>
#include <common/LangUtil.hpp>
#include <common/Log.hpp>
#include <common/MobileApp.hpp>
#include <qt/qt.hpp>

#include <Poco/Path.h>
#include <Poco/URI.h>

#include <QApplication>
#include <QCloseEvent>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QKeySequence>
#include <QLabel>
#include <QMainWindow>
#include <QObject>
#include <QScreen>
#include <QShortcut>
#include <QStandardPaths>
#include <QUrl>
#include <QVariant>
#include <QWebChannel>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineSettings>

#include <algorithm>
#include <memory>
#include <utility>

std::vector<WebView*> WebView::s_instances;

namespace
{
std::string getUILanguage()
{
    const char* envVars[] = {"LC_ALL", "LC_MESSAGES", "LANG", "LANGUAGE"};
    std::string lang;

    // 1. Check environment variables in precedence order
    for (const char* var : envVars) {
        const char* val = std::getenv(var);
        if (val && *val) {
            lang = val;
            if (std::string(var) == "LANGUAGE") {
                // LANGUAGE can be a colon-separated list, take the first
                std::size_t pos = lang.find(':');
                if (pos != std::string::npos)
                    lang = lang.substr(0, pos);
            }
            break;
        }
    }

    // 2. Replace '_' with '-'
    for (char& c : lang)
        if (c == '_')
            c = '-';

    // 3. Strip encoding suffix (e.g. ".UTF-8", ".ISO8859-2")
    if (auto dot = lang.find('.'); dot != std::string::npos)
        lang.erase(dot);

    // 4. Now check for empty or C/POSIX-like locales
    if (lang.empty() || lang == "C" || lang == "POSIX")
        lang = "en-US";

    return lang;
}

QString getDocumentsDirectory()
{
    QString documentsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documentsDir.isEmpty())
    {
        // Fallback to home directory if Documents doesn't exist
        documentsDir = QDir::homePath();
    }
    return documentsDir;
}

Poco::Path getTemplatePath(const std::string& templateType, const std::string& templatePath)
{
    Poco::Path resolvedPath;

    if (!templatePath.empty())
    {
        if (templatePath.starts_with("/"))
        {
            // Absolute path - use directly
            resolvedPath = Poco::Path(templatePath);
        }
        else
        {
            // Relative path - resolve against browser/dist
            resolvedPath = Poco::Path(getDataDir());
            resolvedPath.append("browser/dist");
            resolvedPath.append(templatePath);
        }
    }

    // Check if resolved path exists, otherwise fall back to default
    if (templatePath.empty() || !QFileInfo(QString::fromStdString(resolvedPath.toString())).exists())
    {
        // Map template type to template filename
        std::string templateFileName = "TextDocument.odt"; // default fallback
        if (templateType == "impress")
            templateFileName = "Presentation.odp";
        else if (templateType == "writer")
            templateFileName = "TextDocument.odt";
        else if (templateType == "calc")
            templateFileName = "Spreadsheet.ods";
        else if (templateType == "draw")
            templateFileName = "Drawing.odg";

        Poco::Path defaultPath(getDataDir());
        defaultPath.append("browser/dist/templates");
        defaultPath.append(templateFileName);
        return defaultPath;
    }

    return resolvedPath;
}

std::pair<QString, QString> getDocumentNameInfo(const std::string& templateType,
                                                const std::string& baseName)
{
    QString docNamePrefix;
    QString extension;

    if (templateType == "impress")
    {
        docNamePrefix = QObject::tr("Presentation");
        extension = "odp";
    }
    else if (templateType == "writer")
    {
        docNamePrefix = QObject::tr("Text Document");
        extension = "odt";
    }
    else if (templateType == "calc")
    {
        docNamePrefix = QObject::tr("Spreadsheet");
        extension = "ods";
    }
    else if (templateType == "draw")
    {
        docNamePrefix = QObject::tr("Drawing");
        extension = "odg";
    }
    else
    {
        // Default fallback
        docNamePrefix = QObject::tr("Text Document");
        extension = "odt";
    }

    // if we received an explicit basename in the `newdoc` message use that.
    if (!baseName.empty())
        docNamePrefix = QString::fromStdString(baseName);

    return {docNamePrefix, extension};
}

QString findNextAvailableDocumentName(const QString& documentsDir, const QString& docNamePrefix, const QString& extension)
{
    // First try without number: "Text Document.odt"
    // Then try with numbers: "Text Document (1).odt", "Text Document (2).odt", etc.
    QString baseFileName = QString("%1.%2").arg(docNamePrefix).arg(extension);
    QString baseFilePath = QDir(documentsDir).filePath(baseFileName);

    if (!QFileInfo::exists(baseFilePath))
    {
        // Use base name without number
        return baseFilePath;
    }

    // Base name exists, find next available number
    int docNumber = 1;
    QString newFilePath;
    while (true)
    {
        QString fileName = QString("%1 (%2).%3").arg(docNamePrefix).arg(docNumber).arg(extension);
        newFilePath = QDir(documentsDir).filePath(fileName);
        if (!QFileInfo::exists(newFilePath))
            break;
        docNumber++;
    }

    return newFilePath;
}

class Window: public QMainWindow {
public:
    Window(QWidget * parent, WebView * owner): QMainWindow(parent), owner_(owner) {
        auto* closeWindowShortcut = new QShortcut(QKeySequence::Quit, this);
        closeWindowShortcut->setContext(Qt::WindowShortcut);
        QObject::connect(closeWindowShortcut, &QShortcut::activated, this, &QMainWindow::close);
    }
    void setCloseCallback(const std::function<void()>& closeCallback)
    {
        closeCallback_ = closeCallback;
    }

private:
    void closeEvent(QCloseEvent * ev) override {
        if (closeCallback_)
            closeCallback_();

        auto const p = owner_;
        owner_ = nullptr;
        assert(p != nullptr);
        delete p;
        QMainWindow::closeEvent(ev);
    }

    WebView * owner_;
    std::function<void()> closeCallback_;
};
} // namespace

void CODAWebEngineView::arrangePresentationWindows()
{
    if (!_presenterFSWindow)
        return;

    QScreen* laptopScreen = QGuiApplication::primaryScreen();

    /* what we really want to happen by default is for the presenter
     * console to appear on the laptop screen and the presentation
     * on an external monitor. For now we'll assume the presentation
     * is already on the laptopScreen, which is nearly always the laptop,
     * and put the presenter console on the next available screen just
     * to test that we can put it somewhere else at all */
    QScreen* externalScreen = nullptr;
    QList<QScreen*> screens = QApplication::screens();
    for (QScreen* screen : screens)
    {
        if (screen != laptopScreen)
        {
            externalScreen = screen;
            break;
        }
    }

    _presenterFSWindow->hide();
    QScreen* presenterScreen = externalScreen ? externalScreen : laptopScreen;
    _presenterFSWindow->setScreen(presenterScreen);
    _presenterFSWindow->move(presenterScreen->geometry().topLeft());
    _presenterFSWindow->showFullScreen();

    Window* consoleWindow = _presenterConsole ? static_cast<Window*>(_presenterConsole->mainWindow()) : nullptr;
    if (consoleWindow)
    {
        consoleWindow->hide();
        consoleWindow->setScreen(laptopScreen);
        if (externalScreen)
        {
            consoleWindow->move(laptopScreen->geometry().topLeft());
            consoleWindow->showFullScreen();
        }
        else
        {
            consoleWindow->showNormal();
            consoleWindow->resize(consoleWindow->sizeHint());
            consoleWindow->show();
        }
    }
}

void CODAWebEngineView::createPresentationFS()
{
    // Move the contents into the presentation window so the original
    // window will remain in position, so we can work around the
    // stubbornness of wayland to allow restoring a window back to its
    // original size and position on the screen it started on.
    _presenterFSWindow = std::make_unique<QMainWindow>(nullptr);
    _presenterFSWindow->setCentralWidget(this);
    QLabel* label = new QLabel(QObject::tr("Presenting"));
    label->setAlignment(Qt::AlignCenter);
    _mainWindow->setCentralWidget(label);
    _mainWindow->setEnabled(false);

    arrangePresentationWindows();

    _screenRemoved = QObject::connect(qApp, &QGuiApplication::screenRemoved,
                     [this]() {
                        arrangePresentationWindows();
                     });

    _screenAdded = QObject::connect(qApp, &QGuiApplication::screenAdded,
                     [this]() {
                        arrangePresentationWindows();
                     });
}

void CODAWebEngineView::destroyPresentationFS()
{
    if (_presenterFSWindow)
    {
        _mainWindow->setCentralWidget(this);
        _presenterFSWindow->setCentralWidget(nullptr);

        _presenterFSWindow->close();
        _presenterFSWindow.reset();

        _mainWindow->setEnabled(true);
    }
}

QWebEngineView* CODAWebEngineView::createWindow(QWebEnginePage::WebWindowType /*type*/)
{
    _presenterConsole = new WebView(Application::getProfile(), false);

    QWebEngineView* consoleView = _presenterConsole->webEngineView();
    QWebEnginePage* page = consoleView->page();
    QObject::connect(page, &QWebEnginePage::windowCloseRequested,
                     [this]() {
                         if (!_presenterConsole)
                             return;
                         QMainWindow* consoleWindow = _presenterConsole->mainWindow();
                         consoleWindow->close();
                     });

    Window* consoleWindow = static_cast<Window*>(_presenterConsole->mainWindow());
    consoleWindow->setCloseCallback(
                     [this]() {
                         _presenterConsole = nullptr;

                         destroyPresentationFS();
                     });

    createPresentationFS();

    return consoleView;
}

void CODAWebEngineView::exchangeMonitors()
{
    if (!_presenterFSWindow)
        return;

    QList<QScreen*> screens = QApplication::screens();
    if (screens.size() < 2)
        return;

    QMainWindow* consoleWindow = _presenterConsole ? _presenterConsole->mainWindow() : nullptr;

    size_t origConsoleScreen = 0;
    size_t origPresentationScreen = 0;
    for (size_t i = 0; i < static_cast<size_t>(screens.size()); ++i)
    {
        if (consoleWindow && screens[i] == consoleWindow->screen())
            origConsoleScreen = i;
        if (screens[i] == _presenterFSWindow->screen())
            origPresentationScreen = i;
    }

    _presenterFSWindow->hide();

    size_t newPresentationScreen = origPresentationScreen;

    if (consoleWindow)
    {
        consoleWindow->hide();

        // Rotate the console screen and rotate the presentation screen
        // every time the console catches up to it for the case there
        // are more than two screens. Typically there's just two screens
        // and they just swap.
        size_t newConsoleScreen = (origConsoleScreen + 1) % screens.size();
        if (newConsoleScreen == newPresentationScreen)
            newPresentationScreen = (newPresentationScreen + 1) % screens.size();

        consoleWindow->setScreen(screens[newConsoleScreen]);
        consoleWindow->move(screens[newConsoleScreen]->geometry().topLeft());
    }
    else
    {
        newPresentationScreen = (newPresentationScreen + 1) % screens.size();
    }

    _presenterFSWindow->setScreen(screens[newPresentationScreen]);
    _presenterFSWindow->move(screens[newPresentationScreen]->geometry().topLeft());

    _presenterFSWindow->showFullScreen();
    _presenterFSWindow->show();

    if (consoleWindow)
    {
        consoleWindow->showFullScreen();
        consoleWindow->show();
    }
}

CODAWebEngineView::~CODAWebEngineView()
{
    if (_screenAdded)
        QObject::disconnect(_screenAdded);
    if (_screenRemoved)
        QObject::disconnect(_screenRemoved);
}

WebView::WebView(QWebEngineProfile* profile, bool isWelcome, QMainWindow* parentWindow)
    : _mainWindow(new Window(parentWindow, this))
    , _webView(std::make_unique<CODAWebEngineView>(_mainWindow))
    , _isWelcome(isWelcome)
    , _bridge(nullptr)
{
    _mainWindow->setCentralWidget(_webView.get());

    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();

    if (_isWelcome)
    {
        _mainWindow->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        _mainWindow->setWindowModality(Qt::WindowModal);
    }
    else
    {
        // Use 1/3 of screen size, but enforce reasonable bounds
        int minWidth = qBound(800, screenGeometry.width() / 3, 1400);
        int minHeight = qBound(600, screenGeometry.height() / 3, 1000);
        _mainWindow->setMinimumSize(minWidth, minHeight);
    }

    QWebEnginePage* page = new QWebEnginePage(profile, _webView.get());
    _webView->setPage(page);

    page->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    QObject::connect(page, &QWebEnginePage::fullScreenRequested,
                     [this](QWebEngineFullScreenRequest request)
                     {
                         if (request.toggleOn())
                             _mainWindow->showFullScreen();
                         else
                             _mainWindow->showNormal();
                         request.accept();
                     });

    s_instances.push_back(this);
}

WebView::~WebView() {
    std::erase(s_instances, this);

    // Only delete our bridge - Qt's parent-child ownership handles the rest
    // Note: QWebChannel was created with page as parent: new QWebChannel(_webView->page())
    // Qt will automatically delete it when page destructs - manual deletion causes double-free crash with multiple window open
    if (_bridge) {
        delete _bridge;
        _bridge = nullptr;
    }
}

std::pair<int, int> getWindowSize(bool isWelcome)
{
    QScreen* screen = QGuiApplication::primaryScreen();
    const int viewportWidth = screen->availableGeometry().width();
    const int viewportHeight = screen->availableGeometry().height();

    if (!isWelcome)
        return { viewportWidth, viewportHeight };

    const int maxWidth = 1280;
    const int maxHeight = 720;
    const int minWidth = 800;
    const int minHeight = 450;

    int width = static_cast<int>(std::floor(viewportWidth * 0.4));
    int height = static_cast<int>(std::floor((width * 9.0) / 16.0));

    width = std::min(std::max(width, minWidth), maxWidth);
    height = std::min(std::max(height, minHeight), maxHeight);

    return { width, height };
}

std::optional<bool> portalPrefersDark() {
    QDBusMessage message = QDBusMessage::createMethodCall(
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Settings",
        "Read"
    );
    message << "org.freedesktop.appearance" << "color-scheme";

    QDBusReply<QVariant> reply = QDBusConnection::sessionBus().call(message);
    if (!reply.isValid()) return std::nullopt;

    QVariant v = reply.value();
    if (v.userType() == qMetaTypeId<QDBusVariant>())
        v = qvariant_cast<QDBusVariant>(v).variant();

    bool ok = false;
    const uint code = v.toUInt(&ok);
    if (!ok || code == 0) return std::nullopt;     // 0 = no preference
    if (code == 1) return true;                    // 1 = prefer dark
    if (code == 2) return false;                   // 2 = prefer light
    return std::nullopt;
}

void WebView::load(const Poco::URI& fileURL, bool newFile, bool isStarterMode)
{
    if (isStarterMode)
    {
        // Starter screen mode: no COOLWSD connection needed
        _document = {
            ._fileURL = {},
            ._fakeClientFd = -1,
            ._appDocId = 0,
        };
    }
    else
    {
        // Normal document mode
        _document = {
            ._fileURL = fileURL,
            ._fakeClientFd = fakeSocketSocket(),
            ._appDocId = coda::generateNewAppDocId(),
        };
    }

    // setup js c++ communication
    QWebChannel* channel = new QWebChannel(_webView->page());
    // query gnome font scaling factor asynchronously and apply it to the web view
    queryGnomeFontScalingUpdateZoom();

    assert(_bridge == nullptr);
    _bridge = new Bridge(channel, _document, _mainWindow, _webView.get());
    channel->registerObject("bridge", _bridge);
    _webView->page()->setWebChannel(channel);

    Poco::Path coolHtmlPath(getDataDir());
    coolHtmlPath.append("/browser/dist/cool.html");
    Poco::URI urlAndQuery(coolHtmlPath);
    urlAndQuery.setScheme("file");
    std::string uiLanguage = getUILanguage();
    urlAndQuery.addQueryParameter("lang", uiLanguage);
    urlAndQuery.addQueryParameter("dir", LangUtil::isRtlLanguage(uiLanguage) ? "rtl" : "");

    if (isStarterMode)
    {
        urlAndQuery.addQueryParameter("starterMode", "true");
    }
    else
    {
        urlAndQuery.addQueryParameter("file_path", _document._fileURL.toString());
        urlAndQuery.addQueryParameter("permission", "edit");
        urlAndQuery.addQueryParameter("appdocid", std::to_string(_document._appDocId));
        urlAndQuery.addQueryParameter("userinterfacemode", "notebookbar");
    }

    if (portalPrefersDark())
        urlAndQuery.addQueryParameter("darkTheme", "true");

    if (!isStarterMode)
    {
        if (!newFile)
            urlAndQuery.addQueryParameter("startreadonly", "true");
        if (_isWelcome)
            urlAndQuery.addQueryParameter("welcome", "true");
    }

    const std::string urlAndQueryStr = urlAndQuery.toString();
    LOG_TRC("Open URL: " << urlAndQueryStr);

    // Set window title
    QString applicationTitle;
    if (isStarterMode)
    {
        applicationTitle = QString(APP_NAME) + " - " + QApplication::translate("WebView", "Start");
    }
    else
    {
        Poco::Path uriPath(_document._fileURL.getPath());
        QString fileName = QString::fromStdString(uriPath.getFileName());
        applicationTitle = fileName + " - " APP_NAME;
    }
    if (_webView->window())
        _webView->window()->setWindowTitle(applicationTitle);

    _webView->load(QUrl(QString::fromStdString(urlAndQueryStr)));

    auto size = getWindowSize(_isWelcome || isStarterMode);

    // TODO: Starter screen uses 1.5x welcome dimensions (width and height) as a temporary
    // solution. This should be refined with proper sizing logic based on user feedback.
    if (isStarterMode) {
        size.first = 1.5 * size.first;
        size.second = 1.5 * size.second;
    }
    _mainWindow->resize(size.first, size.second);
    _mainWindow->show();
}

WebView* WebView::createNewDocument(QWebEngineProfile* profile, const std::string& templateType, const std::string& templatePath, const std::string& basename)
{
    // Get template file path
    Poco::Path templatePathObj = getTemplatePath(templateType, templatePath);

    // Get user's Documents directory
    QString documentsDir = getDocumentsDirectory();

    // Get document name prefix and extension based on template type
    auto [docNamePrefix, extension] = getDocumentNameInfo(templateType, basename);

    // Find the next available document name
    QString newFilePath = findNextAvailableDocumentName(documentsDir, docNamePrefix, extension);

    // Copy template to the new location
    QString templateFilePath = QString::fromStdString(templatePathObj.toString());
    if (!QFile::copy(templateFilePath, newFilePath))
    {
        LOG_ERR("Failed to copy template from " << templateFilePath.toStdString()
                << " to " << newFilePath.toStdString());
        return nullptr;
    }

    // Open the new document
    Poco::URI newDocumentURI(Poco::Path(newFilePath.toStdString()));
    WebView* webViewInstance = new WebView(profile, false);
    webViewInstance->load(newDocumentURI, true);

    // Add to recent files
    Application::getRecentFiles().add(newDocumentURI.toString());

    return webViewInstance;
}

WebView* WebView::findOpenDocument(const Poco::URI& documentURI)
{
    if (documentURI.empty())
        return nullptr;

    for (WebView* instance : s_instances)
    {
        if (instance->_document._fileURL.getPath() == documentURI.getPath())
        {
            return instance;
        }
    }
    return nullptr;
}

WebView* WebView::findStarterScreen()
{
    for (WebView* instance : s_instances)
    {
        if (instance->isStarterScreen())
        {
            return instance;
        }
    }
    return nullptr;
}

void WebView::activateWindow()
{
    if (_mainWindow)
    {
        _mainWindow->raise();
        _mainWindow->activateWindow();
    }
}

void WebView::queryGnomeFontScalingUpdateZoom()
{
    QDBusInterface portalInterface("org.freedesktop.portal.Desktop",
                                   "/org/freedesktop/portal/desktop",
                                   "org.freedesktop.portal.Settings",
                                   QDBusConnection::sessionBus());

    if (!portalInterface.isValid())
        return;

    QDBusPendingCall pendingCall = portalInterface.asyncCall("Read",
                                                              "org.gnome.desktop.interface",
                                                              "text-scaling-factor");

    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(pendingCall, _webView.get());
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished,
                     [this](QDBusPendingCallWatcher* watcher)
                     {
                         QDBusPendingReply<QVariant> reply = *watcher;
                         watcher->deleteLater();

                         if (reply.isError())
                             return;

                         QVariant result = reply.value();
                         // reply seems to be a (<<scalingFactor>>,)
                         // i.e. a tuple where there's a double nested variant as the first element.
                         if (!result.canConvert<QDBusVariant>())
                             return;

                         QDBusVariant dbusVariant = result.value<QDBusVariant>();
                         QVariant innerVariant = dbusVariant.variant();

                         // unwrap nested QDBusVariant if present
                         if (innerVariant.canConvert<QDBusVariant>())
                         {
                             QDBusVariant innerDbusVariant = innerVariant.value<QDBusVariant>();
                             innerVariant = innerDbusVariant.variant();
                         }

                         bool ok;
                         double factor = innerVariant.toDouble(&ok);
                         if (ok)
                             _webView->setZoomFactor(factor);
                     });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
