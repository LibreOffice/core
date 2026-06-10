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
#include "StandaloneWindow.hpp"
#include "TabbedWindow.hpp"
#include "TabManager.hpp"
#include "WindowUtils.hpp"

#include <QUrlQuery>

#include <qt/bridge.hpp>
#include <qt/CodaConfig.hpp>
#include <qt/DBusService.hpp>
#include <net/FakeSocket.hpp>
#include <common/LangUtil.hpp>
#include <common/Log.hpp>
#include <common/MobileApp.hpp>
#include <qt/qt.hpp>

#include <o3tl/unreachable.hxx>

#include <Poco/Path.h>
#include <Poco/URI.h>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDir>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileInfo>
#include <QMimeData>
#include <QGuiApplication>
#include <QMainWindow>
#include <QObject>
#include <QScreen>
#include <QWindow>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>
#include <QUrl>
#include <QVariant>
#include <common/SettingsStorage.hpp>
#include <QWebChannel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QWebEngineFullScreenRequest>
#include <QTimer>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWebSocket>

#include <algorithm>
#include <cctype>
#include <memory>
#include <set>
#include <utility>

std::vector<WebView*> WebView::s_instances;

namespace
{
class LoggingWebEnginePage: public QWebEnginePage {
public:
    using QWebEnginePage::QWebEnginePage;

private:
#if ENABLE_DEBUG
    void javaScriptConsoleMessage(
        JavaScriptConsoleMessageLevel level, QString const & message, int lineNumber,
        QString const & sourceID) override
    {
        char const * tag;
        switch (level) {
        case InfoMessageLevel:
            tag = "info";
            break;
        case WarningMessageLevel:
            tag = "warn";
            break;
        case ErrorMessageLevel:
            tag = "error";
            break;
        default:
            O3TL_UNREACHABLE;
        }
        LOG_TRC(
            "JS console [" << tag << "] " << sourceID.toStdString() << ":" << lineNumber << ": "
            << message.toStdString());
    }
#endif
};

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

// Move a window onto a given screen and show it full screen there.
//
// On a Wayland move to another output, tear the platform window down first so
// the reshow builds a fresh wl_surface. This works around QtWayland reusing the
// surface across hide and show, which the compositor rejects when the surface
// already had a buffer committed. Elsewhere just show full screen in place.
void showWindowFullScreenOnScreen(QWidget* pWindow, QScreen* pScreen)
{
    const bool bWayland
        = QGuiApplication::platformName().startsWith(QLatin1String("wayland"));
    if (bWayland && pWindow->screen() != pScreen)
    {
        if (QWindow* pHandle = pWindow->windowHandle())
            pHandle->destroy();
    }
    pWindow->setScreen(pScreen);
    pWindow->move(pScreen->geometry().topLeft());
    pWindow->showFullScreen();
}
} // namespace

void CODAWebEngineView::connectScreenChanges()
{
    if (!_screenAdded)
        _screenAdded = QObject::connect(qApp, &QGuiApplication::screenAdded,
                         [this]() {
                            arrangePresentationWindows();
                         });
    if (!_screenRemoved)
        _screenRemoved = QObject::connect(qApp, &QGuiApplication::screenRemoved,
                         [this]() {
                            arrangePresentationWindows();
                         });
}

void CODAWebEngineView::arrangePresentationWindows()
{
    QMainWindow* presenterFSWindow = _presentationView ? _presentationView->mainWindow() : nullptr;
    if (!presenterFSWindow)
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

    QScreen* presenterScreen = externalScreen ? externalScreen : laptopScreen;
    showWindowFullScreenOnScreen(presenterFSWindow, presenterScreen);

    QMainWindow* consoleWindow = _presenterConsole ? _presenterConsole->mainWindow() : nullptr;
    if (consoleWindow)
    {
        if (externalScreen)
        {
            showWindowFullScreenOnScreen(consoleWindow, laptopScreen);
        }
        else
        {
            // One screen: the console floats on top of the full screen
            // presentation. Give it most of the work area, but leave a margin
            // so the presentation stays visible around the edges and the user
            // can see it is running underneath. An arbitrary small default
            // size left it too small to use.
            // showNormal() first to leave any earlier full screen state (set
            // when an external monitor was present), so the resize and move
            // below act on the normal-state geometry rather than being
            // discarded when the window is restored out of full screen.
            consoleWindow->showNormal();
            const QRect area = laptopScreen->availableGeometry();
            const QSize size(area.width() * 17 / 20, area.height() * 17 / 20);
            consoleWindow->resize(size);
            consoleWindow->move(area.center() - QPoint(size.width() / 2, size.height() / 2));
        }
    }
}

namespace
{

// Collect the local filesystem paths of any files dragged in from the OS.
QStringList droppedLocalFiles(const QMimeData* mimeData)
{
    QStringList files;
    if (!mimeData || !mimeData->hasUrls())
        return files;

    for (const QUrl& url : mimeData->urls())
    {
        if (url.isLocalFile())
            files << url.toLocalFile();
    }
    return files;
}

constexpr const char* PORTAL_FILETRANSFER_MIME = "application/vnd.portal.filetransfer";

bool hasDroppableFiles(const QMimeData* mimeData)
{
    if (mimeData && mimeData->hasFormat(PORTAL_FILETRANSFER_MIME))
        return true;

    if (!mimeData || !mimeData->hasUrls())
        return false;

    for (const QUrl& url : mimeData->urls())
    {
        if (url.isLocalFile())
            return true;
    }
    return false;
}

// Resolve a FileTransfer drag payload into sandbox-accessible paths.
QStringList filesFromPortalTransfer(const QMimeData* mimeData)
{
    if (mimeData && mimeData->hasFormat(PORTAL_FILETRANSFER_MIME))
    {
        // The payload is the transfer key, it can contain a trailing NUL.
        const QString key =
            QString::fromUtf8(mimeData->data(PORTAL_FILETRANSFER_MIME)).remove(QChar('\0'));
        if (!key.isEmpty())
        {
            QDBusMessage message = QDBusMessage::createMethodCall(
                "org.freedesktop.portal.Documents", "/org/freedesktop/portal/documents",
                "org.freedesktop.portal.FileTransfer", "RetrieveFiles");
            message.setArguments(QVariantList{ key, QVariantMap{} });

            QDBusReply<QStringList> reply = QDBusConnection::sessionBus().call(message);
            if (reply.isValid())
                return reply.value();
            else
                LOG_WRN("FileTransfer portal RetrieveFiles failed: "
                        << reply.error().message().toStdString());
        }
    }

    return {};
}

QStringList hostDisplayUris(const QStringList& portalFiles, const QMimeData* mimeData)
{
    if (!mimeData || !mimeData->hasUrls())
        return {};

    QHash<QString, QString> hostUriByName;
    for (const QUrl& url : mimeData->urls())
        if (url.isLocalFile())
            hostUriByName.insert(QFileInfo(url.toLocalFile()).fileName(), url.toString());

    QStringList displayUris;
    displayUris.reserve(portalFiles.size());
    for (const QString& file : portalFiles)
        displayUris << hostUriByName.value(QFileInfo(file).fileName());

    return displayUris;
}

// Hands the native preference set to the page before any of its own scripts
// run. The WebChannel bridge only comes up once the page is loading, which is
// after it has composed its load message, so preferences that have to travel
// with that message cannot wait for the bridge to answer getAllPrefs().
void seedPrefs(QWebEnginePage* page)
{
    const QByteArray prefs =
        QByteArray::fromStdString(Application::getPrefs().serialize());

    // Parse before embedding: the page gets this as source code, so anything
    // malformed would be a syntax error in a script the page cannot report.
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(prefs, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject())
    {
        LOG_WRN("Not seeding the preferences into the page: "
                << error.errorString().toStdString());
        return;
    }

    LOG_TRC("Seeding " << doc.object().size() << " preferences into the page");

    QWebEngineScript script;
    script.setName(QStringLiteral("codaPrefs"));
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setRunsOnSubFrames(false);
    script.setSourceCode(QStringLiteral("window.codaPrefs = %1;")
                             .arg(QString::fromUtf8(doc.toJson(QJsonDocument::Compact))));
    page->scripts().insert(script);
}

} // namespace

void CODAWebEngineView::setDropFeedbackVisible(bool bVisible)
{
    // The toggles are registered by the page once its scripts have loaded, so
    // guard against an early drag before they exist.
    const char* method = bVisible ? "showDropOverlay" : "hideDropOverlay";
    const QString script = QStringLiteral(
                               "window.app && window.app.%1 && window.app.%1();")
                               .arg(QLatin1String(method));
    page()->runJavaScript(script);
}

void CODAWebEngineView::dragEnterEvent(QDragEnterEvent* event)
{
    // Drags started inside the web content have a non-null source.
    // Let the base class handle those, only intercept genuine OS file drops.
    if (event->source())
    {
        QWebEngineView::dragEnterEvent(event);
        return;
    }

    if (hasDroppableFiles(event->mimeData()))
    {
        event->acceptProposedAction();
        setDropFeedbackVisible(true);
    }
}

void CODAWebEngineView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->source())
    {
        QWebEngineView::dragMoveEvent(event);
        return;
    }

    if (hasDroppableFiles(event->mimeData()))
        event->acceptProposedAction();
}

void CODAWebEngineView::dragLeaveEvent(QDragLeaveEvent* event)
{
    setDropFeedbackVisible(false);
    event->accept();
}

void CODAWebEngineView::dropEvent(QDropEvent* event)
{
    if (event->source())
    {
        QWebEngineView::dropEvent(event);
        return;
    }

    setDropFeedbackVisible(false);

    const QMimeData* mimeData = event->mimeData();

    const QStringList portalFiles = filesFromPortalTransfer(mimeData);
    if (!portalFiles.isEmpty())
    {
        event->acceptProposedAction();
        coda::openFiles(portalFiles, hostDisplayUris(portalFiles, mimeData));
        return;
    }

    const QStringList files = droppedLocalFiles(mimeData);
    if (!files.isEmpty())
    {
        event->acceptProposedAction();
        coda::openFiles(files);
    }
}

QWebEngineView* CODAWebEngineView::createWindow(QWebEnginePage::WebWindowType /*type*/)
{
    // A window.open from the page. The slideshow and the presenter console each
    // open their own top-level window, naming its role in the URL fragment they
    // pass to window.open. The original document view is never touched, so its
    // content stays live.
    WebView* child = new WebView(Application::getProfile(), false);
    // The standalone window owns the child WebView and deletes it on close.
    StandaloneWindow* childWindow = StandaloneWindow::wrap(child);

    QWebEngineView* childView = child->webEngineView();
    QWebEnginePage* page = childView->page();
    QObject::connect(page, &QWebEnginePage::windowCloseRequested, page,
                     [child]() {
                         if (QMainWindow* window = child->mainWindow())
                             window->close();
                     });

    childWindow->setCloseCallback(
                     [this, child]() {
                         if (_presenterConsole == child)
                             _presenterConsole = nullptr;
                         if (_presentationView == child)
                             _presentationView = nullptr;
                     });

    // The window opens at a URL whose fragment names its role. Read it once the
    // requested URL is known and place the window on the right screen.
    QObject::connect(page, &QWebEnginePage::urlChanged, page,
                     [this, child](const QUrl& url) {
                         claimChildWindow(child, url);
                     });

    return childView;
}

void CODAWebEngineView::claimChildWindow(WebView* child, const QUrl& url)
{
    // The fragment values are set where window.open is called, in
    // browser/src/slideshow/SlideShowPresenter.ts and PresenterConsole.js.
    const QString role = url.fragment();
    if (role == "coda-presentation")
    {
        if (_presentationView == child)
            return;
        _presentationView = child;
        connectScreenChanges();
    }
    else if (role == "coda-console")
    {
        if (_presenterConsole == child)
            return;
        _presenterConsole = child;
    }
    else
    {
        // An empty fragment is the bare about:blank the window briefly reports
        // before the role URL arrives, so only a non-empty value is a genuine
        // mismatch between these names and the ones the page passes to
        // window.open.
        if (!role.isEmpty())
            LOG_WRN("Unclaimed child window, unexpected role fragment '" << role.toStdString() << "'");
        return;
    }

    arrangePresentationWindows();
}

void CODAWebEngineView::exchangeMonitors()
{
    QMainWindow* presenterFSWindow = _presentationView ? _presentationView->mainWindow() : nullptr;
    if (!presenterFSWindow)
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
        if (screens[i] == presenterFSWindow->screen())
            origPresentationScreen = i;
    }

    size_t newPresentationScreen = origPresentationScreen;
    size_t newConsoleScreen = origConsoleScreen;

    if (consoleWindow)
    {
        // Rotate the console screen and rotate the presentation screen
        // every time the console catches up to it for the case there
        // are more than two screens. Typically there's just two screens
        // and they just swap.
        newConsoleScreen = (origConsoleScreen + 1) % screens.size();
        if (newConsoleScreen == newPresentationScreen)
            newPresentationScreen = (newPresentationScreen + 1) % screens.size();
    }
    else
    {
        newPresentationScreen = (newPresentationScreen + 1) % screens.size();
    }

    showWindowFullScreenOnScreen(presenterFSWindow, screens[newPresentationScreen]);
    if (consoleWindow)
        showWindowFullScreenOnScreen(consoleWindow, screens[newConsoleScreen]);
}

void CODAWebEngineView::endPresentation()
{
    // These windows outlive the document view, and their close handlers refer
    // back to this view, so clear the members before closing them.
    WebView* presentationView = _presentationView;
    WebView* presenterConsole = _presenterConsole;
    _presentationView = nullptr;
    _presenterConsole = nullptr;
    if (presentationView && presentationView->mainWindow())
        presentationView->mainWindow()->close();
    if (presenterConsole && presenterConsole->mainWindow())
        presenterConsole->mainWindow()->close();
}

CODAWebEngineView::~CODAWebEngineView()
{
    if (_screenAdded)
        QObject::disconnect(_screenAdded);
    if (_screenRemoved)
        QObject::disconnect(_screenRemoved);

    endPresentation();
}

WebView::WebView(QWebEngineProfile* profile, bool isWelcome)
    : QObject(nullptr)
    , _mainWindow(nullptr)
    , _webView(std::make_unique<CODAWebEngineView>(nullptr))
    , _isWelcome(isWelcome)
    , _bridge(nullptr)
{
    QWebEnginePage* page = new LoggingWebEnginePage(profile, _webView.get());
    _webView->setPage(page);

    seedPrefs(page);

    page->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    // JS-driven clipboard access is off by default in QtWebEngine - enable it so
    // copy/paste buttons (e.g. the AI chat sidebar's "Copy to clipboard") work.
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanPaste, true);
    // cool.html is loaded over file://; without this, JS fetch() to https:// is
    // blocked (Zotero queries api.zotero.org directly from the page; the
    // COOL server's /co/collab/avatar endpoint is fetched for user avatars).
    page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    // Our own browser, so let slide-show video start with sound on slide
    // entry, without a click.
    page->settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);

    QObject::connect(page, &QWebEnginePage::fullScreenRequested,
                     [this](QWebEngineFullScreenRequest request)
                     {
                         if (!_mainWindow)
                         {
                             request.reject();
                             return;
                         }
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

void WebView::onWindowActiveChanged(bool active)
{
    if (active || isStarterScreen() || !_bridge)
        return;

    // Backgrounded: after a debounce, drop this document's own off-screen tiles
    // through its page's blur handler, unless its window is active again by then.
    constexpr int trimDelayMs = 5000;
    QTimer::singleShot(trimDelayMs, _webView.get(), [this]() {
        if (!_bridge || (_mainWindow && _mainWindow->isActiveWindow()))
            return;
        LOG_TRC("trimming off-screen tiles of backgrounded document appDocId="
                << _document._appDocId);
        _bridge->evalJS("if (window.app && window.app.map && window.app.map._docLayer) "
                        "window.app.map._docLayer._onDocumentBlur();");
    });
}

void WebView::setMainWindow(QMainWindow* window)
{
    _mainWindow = window;
    if (_webView)
        _webView->setMainWindow(window);
}

void WebView::requestClose()
{
    if (_onCloseRequest)
    {
        _onCloseRequest();
        return;
    }
    if (_mainWindow)
        _mainWindow->close();
}

void WebView::updateTitle(const QString& docTitle)
{
    _docTitle = docTitle;
    const QString windowTitle = composedWindowTitle();
    if (_onTitleChange)
        _onTitleChange(windowTitle);
    else if (_mainWindow)
        _mainWindow->setWindowTitle(windowTitle);
}

QString WebView::composedWindowTitle() const
{
    return _docTitle.isEmpty() ? QStringLiteral(APP_NAME)
                               : (_docTitle + QStringLiteral(" - ") + QStringLiteral(APP_NAME));
}

std::optional<bool> portalPrefersDark()
{
    static const std::optional<bool> cached = []() -> std::optional<bool> {
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
    }();
    return cached;
}

QString WebView::docTypeFromExtension(const QString& filePath)
{
    const QString ext = QFileInfo(filePath).suffix().toLower();
    static const QStringList writerExt = { "odt", "ott", "doc", "docx", "rtf", "txt", "fodt" };
    static const QStringList calcExt = { "ods", "ots", "xls", "xlsx", "csv", "fods" };
    static const QStringList impressExt = { "odp", "otp", "ppt", "pptx", "fodp" };
    static const QStringList drawExt = { "odg", "otg", "fodg" };
    if (writerExt.contains(ext))
        return QStringLiteral("writer");
    if (calcExt.contains(ext))
        return QStringLiteral("calc");
    if (impressExt.contains(ext))
        return QStringLiteral("impress");
    if (drawExt.contains(ext))
        return QStringLiteral("draw");
    return QStringLiteral("other");
}

void WebView::load(const Poco::URI& fileURL, bool newFile, bool isStarterMode, bool requiresSaveAs)
{
    if (isStarterMode)
    {
        // Starter screen mode: no COOLWSD connection needed
        _document = {
            ._fileURL = {},
            ._fakeClientFd = -1,
            ._appDocId = 0,
        };
        _docType = QStringLiteral("starter");
    }
    else
    {
        // Normal document mode
        _document = {
            ._fileURL = fileURL,
            ._fakeClientFd = fakeSocketSocket(),
            ._appDocId = coda::generateNewAppDocId(),
        };
        _docType = _isWelcome ? QStringLiteral("welcome")
                              : docTypeFromExtension(QString::fromStdString(fileURL.getPath()));
    }

    // setup js c++ communication
    QWebChannel* channel = new QWebChannel(_webView->page());
    // query gnome font scaling factor asynchronously and apply it to the web view
    queryGnomeFontScalingUpdateZoom();

    assert(_bridge == nullptr);
    _bridge = new Bridge(channel, this, _document, nullptr, _webView.get());
    if (requiresSaveAs)
        _bridge->setRequiresSaveAs(true);
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
        urlAndQuery.addQueryParameter(
            "permission", CodaConfig::instance().isForcedReadOnly() ? "readonly" : "edit");
        urlAndQuery.addQueryParameter("appdocid", std::to_string(_document._appDocId));
        urlAndQuery.addQueryParameter("userinterfacemode", "notebookbar");
    }

    // Dark mode: the user's saved choice wins, otherwise follow the system theme.
    const bool darkMode =
        Desktop::getDarkMode().value_or(portalPrefersDark().value_or(false));
    urlAndQuery.addQueryParameter("darkTheme", darkMode ? "true" : "false");

    if (!isStarterMode)
    {
        // A brand new file and a template-based document both open ready to edit;
        // an existing file opens read-only until the user chooses to edit it.
        if (!newFile && !requiresSaveAs)
            urlAndQuery.addQueryParameter("startreadonly", "true");
        if (_isWelcome)
            urlAndQuery.addQueryParameter("welcome", "true");
    }

    const std::string urlAndQueryStr = urlAndQuery.toString();
    LOG_TRC("Open URL: " << urlAndQueryStr);

    if (isStarterMode)
    {
        updateTitle(QApplication::translate("WebView", "Start"));
    }
    else
    {
        Poco::Path uriPath(_document._fileURL.getPath());
        updateTitle(QString::fromStdString(uriPath.getFileName()));
    }

    _webView->load(QUrl(QString::fromStdString(urlAndQueryStr)));
}

Bridge* coda::attachRemoteBridge(QWebEnginePage* page,
                                 coda::DocumentData& document,
                                 WebView* owner, QWidget* window,
                                 QWebEngineView* webView)
{
    QWebChannel* channel = new QWebChannel(page);
    Bridge* bridge = new Bridge(channel, owner, document, window, webView);
    channel->registerObject("bridge", bridge);
    page->setWebChannel(channel);
    return bridge;
}

void coda::addRemoteCoolParams(QUrl& url,
                               const coda::DocumentData& document)
{
    QUrlQuery q(url);
    std::string uiLanguage = getUILanguage();
    q.addQueryItem("lang", QString::fromStdString(uiLanguage));
    q.addQueryItem("dir",
        LangUtil::isRtlLanguage(uiLanguage) ? "rtl" : "");
    // file_path is intentionally NOT set here: for a remote document
    // the local temp file does not exist yet at this point - the
    // page-JS does the /co/collab fetch, asks Bridge::writeRemoteDocFile
    // to materialise the bytes, and uses the returned path to drive
    // the standard load flow.
    q.addQueryItem("permission", "edit");
    q.addQueryItem("startreadonly", "true");
    q.addQueryItem("appdocid",
        QString::number(document._appDocId));
    q.addQueryItem("userinterfacemode", "notebookbar");
    if (portalPrefersDark())
        q.addQueryItem("darkTheme", "true");
    url.setQuery(q);
}

void WebView::loadRemote(std::shared_ptr<coda::RemoteDocInfo> remoteInfo)
{
    _document = {
        ._fileURL = Poco::URI(),
        ._fakeClientFd = fakeSocketSocket(),
        ._appDocId = coda::generateNewAppDocId(),
        ._remoteInfo = std::move(remoteInfo),
    };
    // The document type is unknown until the page-JS resolves the file;
    // the tab falls back to the generic icon.
    _docType = QStringLiteral("other");

    queryGnomeFontScalingUpdateZoom();
    assert(_bridge == nullptr);
    _bridge = coda::attachRemoteBridge(
        _webView->page(), _document, this, nullptr, _webView.get());

    Poco::Path coolHtmlPath(getDataDir());
    coolHtmlPath.append("/browser/dist/cool.html");
    QUrl urlAndQuery = QUrl::fromLocalFile(
        QString::fromStdString(coolHtmlPath.toString()));
    coda::addRemoteCoolParams(urlAndQuery, _document);

    LOG_TRC("Open remote URL: " << urlAndQuery.toString().toStdString());

    // Tab/window title is a generic "<APP_NAME>" until the page-JS
    // resolves the actual filename via /co/collab/fetch - at which
    // point Permission.js or similar can update the title.
    updateTitle(QString());

    _webView->load(urlAndQuery);
}

QString WebView::createNewDocumentFile(const std::string& templateType, const std::string& templatePath, const std::string& basename)
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
        return {};
    }

    return newFilePath;
}

bool WebView::isTemplate(const std::string& fileName)
{
    // Document-template extensions across the formats we load: ODF, the older
    // StarOffice formats, the OOXML templates and the older binary templates.
    // Opening one of these creates a new document based on it rather than
    // editing the template itself.
    static const std::set<std::string> templateExtensions = {
        // ODF templates
        "ott", "ots", "otp", "otg", "otm",
        // StarOffice templates
        "stw", "stc", "sti", "std",
        // OOXML templates
        "dotx", "dotm", "xltx", "xltm", "potx", "potm",
        // Older binary templates
        "dot", "xlt", "pot",
    };

    std::string extension = Poco::Path(fileName).getExtension();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return templateExtensions.find(extension) != templateExtensions.end();
}

WebView* WebView::openTemplateAsNewDocument(QWebEngineProfile* profile,
                                            const Poco::URI& templateURL)
{
    const QString templatePath = QString::fromStdString(templateURL.getPath());
    const QString templateFileName = QString::fromStdString(Poco::Path(templateURL.getPath()).getFileName());

    // Copy the template into a private temporary directory and load that copy,
    // so nothing we do can write back to the original template file. The copy
    // keeps the template's file name so the Save As dialog suggests it.
    auto workingDir = std::make_unique<QTemporaryDir>();
    if (!workingDir->isValid())
    {
        LOG_ERR("Failed to create a temporary directory for template " << templatePath.toStdString());
        return nullptr;
    }

    const QString workingCopyPath = workingDir->filePath(templateFileName);
    if (!QFile::copy(templatePath, workingCopyPath))
    {
        LOG_ERR("Failed to copy template " << templatePath.toStdString()
                << " to working copy " << workingCopyPath.toStdString());
        return nullptr;
    }

    Poco::URI workingCopyURI(Poco::Path(workingCopyPath.toStdString()));
    TabbedWindow* window = TabbedWindow::getOrCreate(profile);
    const int tabId = window->manager()->addDocumentTab(workingCopyURI, /*newFile*/ false,
                                                        /*requiresSaveAs*/ true);
    WebView* webViewInstance = window->manager()->webViewForTab(tabId);
    webViewInstance->_templateWorkingDir = std::move(workingDir);

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

void WebView::activateWindow()
{
    if (auto* tw = qobject_cast<TabbedWindow*>(_mainWindow))
        tw->activateTabFor(this); // raises and activates the window itself
    else if (_mainWindow)
        surfaceWindow(_mainWindow);
}

bool WebView::isSaveInFlight() const
{
    return _bridge && _bridge->isSaveInFlight();
}

void WebView::onSaveComplete(std::function<void()> callback)
{
    if (_bridge)
        _bridge->onSaveComplete(std::move(callback));
    else if (callback)
        callback();
}

bool WebView::isReadyToClose() const
{
    return !_bridge || _bridge->isReadyToClose();
}

void WebView::markReadyToClose()
{
    if (_bridge)
        _bridge->markReadyToClose();
}

void WebView::saveAndClose()
{
    if (_bridge)
        _bridge->saveAndClose();
}

void WebView::evalJS(const std::string& script)
{
    if (_bridge)
        _bridge->evalJS(script);
}

void WebView::sendCollabBye()
{
    if (_bridge)
        _bridge->sendCollabBye();
}

bool WebView::isDocumentModified() const
{
    return _bridge && _bridge->isModified();
}

void WebView::endPresentation()
{
    if (_webView)
        _webView->endPresentation();
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
