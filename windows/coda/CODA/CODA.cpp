// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <Windows.h>
#include <appmodel.h>
#include <dwmapi.h>
#include <ole2.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <shobjidl_core.h>

#include <wincodec.h>
#include <wincrypt.h>
#include <winhttp.h>

#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>

#include <wrl.h>
#include <wil/com.h>

#include <Poco/AutoPtr.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/JSON/Array.h>
#include <Poco/MemoryStream.h>
#include <Poco/Path.h>
#include <Poco/SAX/InputSource.h>

#include <common/AIHttpTransport.hpp>
#include <common/Clipboard.hpp>
#include <common/ConfigUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/LangUtil.hpp>
#include <common/Protocol.hpp>
#include <common/Log.hpp>
#include <common/MobileApp.hpp>
#include <common/RecentFiles.hpp>
#include <common/SettingsStorage.hpp>
#include <common/StringVector.hpp>
#include <common/Uri.hpp>
#include <net/FakeSocket.hpp>
#include <wsd/Extensions.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/RequestDetails.hpp>

#include "Resource.h"
#include "windows.hpp"

extern std::map<std::string, std::shared_ptr<DocumentBroker>> DocBrokers;
extern std::mutex DocBrokersMutex;

// Note that all pathnames in this code that are plain narrow strings (std::string) are in UTF-8 and
// can thus *not* be used for actual file system operations. They must always be converted to UTF-16
// first with Util::string_to_wide_string(). URIs one the other hand are valid as such as narrow
// strings.

enum class DocumentType
{
    TEXT,
    SPREADSHEET,
    PRESENTATION,
    DRAWING,
};

enum class DocumentMode { EDIT, NEW, WELCOME, STARTER };

struct FilenameAndUri
{
    // Just the basename (and extension), without folder
    std::string filename;

    // Complete file: URI
    std::string uri;
};

// One open document, shown as a tab of the tabbed window. The starter
// backstage, the welcome slideshow and the presenter console are each the only
// document of a window of their own.
struct DocumentTab
{
    int tabId = 0;
    // The window this document is shown in. A document can move to another window, and the
    // thread that forwards the engine's messages to the page reads this too.
    std::atomic<HWND> hWnd{ nullptr };
    // The presenter console this document opened, while it is presenting.
    HWND hConsoleWnd = 0;
    // For a presenter console, the document that opened it.
    int presentingTabId = 0;
    bool isConsole = false;
    int fakeClientFd = -1;
    int closeNotificationPipeForForwardingThread[2] = { -1, -1 };
    FilenameAndUri filenameAndUri;
    DocumentMode mode = DocumentMode::EDIT;
    int appDocId = 0;
    // The document kind the tab strip keys its icon and colour off: "writer",
    // "calc", "impress", "draw", "starter", "welcome" or "other".
    std::string docType = "other";
    // The document holds changes that have not reached the file yet.
    bool isModified = false;
    // A PNG image of what the document shows, sent by the page, or empty until the page has sent
    // one.
    std::string thumbnailPng;
    // The page has parsed its document and its stylesheets, so the WebView2 has something
    // styled to put on the screen. It stays hidden until then.
    bool hasContent = false;
    wil::com_ptr<ICoreWebView2Controller> webViewController;
    wil::com_ptr<ICoreWebView2> webView;
    std::thread app2js;
};

// One top-level window. It holds its documents in the order the tab strip shows
// them, and the window-wide state that belongs to none of them in particular.
struct WindowState
{
    HWND hWnd = 0;
    // The window shows documents as tabs, rather than being the welcome slideshow, the starter
    // backstage or a presenter console.
    bool isTabbed = false;
    std::vector<int> tabIds;
    int activeTabId = 0;
    // The tab strip. It is created with the window and shown while the window holds a document.
    wil::com_ptr<ICoreWebView2Controller> stripController;
    wil::com_ptr<ICoreWebView2> stripWebView;
    bool stripIsVisible = false;
    bool stripHasContent = false;
    // The transparent window raised over the web view for the duration of a drag from the desktop,
    // and the drop target registered for it. Both are created when the first such drag arrives.
    HWND hOverlayWnd = 0;
    IDropTarget* dropTarget = nullptr;
    int numMonitors = 0;
    RECT originalRect;
    LONG originalStyle;
    bool isFullScreen = false;
    bool isPresFullScreen = false;
};

// Both keep their entries at a fixed address: the WebView2 completion handlers
// capture an entry by reference and run long after the container has grown.
static std::map<int, DocumentTab> documentTabs;
static std::map<HWND, WindowState> windows;

// The window that shows documents as tabs and that a newly opened document goes to. There can
// be several such windows once a tab has been moved out into a window of its own, and this is the
// one that was active last.
static HWND tabbedWindow = 0;

static DocumentTab* findTab(int tabId)
{
    const auto it = documentTabs.find(tabId);
    return it == documentTabs.end() ? nullptr : &it->second;
}

static WindowState* findWindow(HWND hWnd)
{
    const auto it = windows.find(hWnd);
    return it == windows.end() ? nullptr : &it->second;
}

static DocumentTab* activeTabOf(const WindowState& window)
{
    return findTab(window.activeTabId);
}

static bool enableWebDriver = false;

static HINSTANCE appInstance;
static int appShowMode;

static bool weOwnTheClipboard = false;

// The COKit Office object, captured when the clipboard provider is installed, so a clipboard read
// can go straight to the process-shared clipboard without needing a particular document.
static COKit* office = nullptr;

const char* user_name = nullptr;
int coolwsd_server_socket_fd = -1;

static std::string app_exe_path;
std::string app_installation_path;

std::string app_installation_uri;

std::string localAppData;

static std::wstring appUserModelId;

static std::string uiLanguage = "en-US";
static std::wstring appName;

static COOLWSD* coolwsd = nullptr;

static std::thread coolwsdThread;

// The main window class name.
static const wchar_t windowClass[] = L"CODA";

// The class name of the transparent window that catches drops over the web view.
static const wchar_t overlayWindowClass[] = L"CODADropOverlayWindow";

// The class name of the card that shows a background document below its tab.
static const wchar_t hoverCardWindowClass[] = L"CODATabHoverCard";

// The hidden file open dialog and clipboard owner window class name.
static const wchar_t hiddenOwnerWindowClass[] = L"CODAHiddenOwnerWindow";
// The handle of that dummy window.
static HWND hiddenOwnerWindow;

// The tab whose web view the script or web message is meant for travels in the lParam of
// CODA_WM_EXECUTESCRIPT and CODA_WM_POSTWEBMESSAGE, as several documents share one window.
static const int CODA_WM_EXECUTESCRIPT = WM_APP + 1;
static const int CODA_WM_LOADNEXTDOCUMENT = WM_APP + 2;
static const int CODA_WM_POSTWEBMESSAGE = WM_APP + 3;
static const int CODA_WM_SHOWFILEPICKER = WM_APP + 4;
// Closes the tab named in the wParam. Posted rather than called, so a web view
// is never torn down from inside one of its own callbacks.
static const int CODA_WM_CLOSETAB = WM_APP + 5;
// Brings the next document of the window to the front, or the previous one when the wParam is -1
// rather than 1. Posted for the same reason the close above is.
static const int CODA_WM_ACTIVATEADJACENTTAB = WM_APP + 6;
// Pops up the menu for the tab named in the wParam, at the point of the window's client area in
// the lParam. Posted, so the menu's modal loop does not run inside a callback of the strip.
static const int CODA_WM_SHOWTABMENU = WM_APP + 7;
// Moves the tab named in the wParam into a new window of its own, placed at the point of the screen
// in the lParam. Posted, so the web views change windows outside a callback of the strip.
static const int CODA_WM_MOVETABTONEWWINDOW = WM_APP + 8;
// Moves the tab named in the wParam from another window into this one, to the position in the
// tab order given in the lParam, or to the end when that is -1. Posted for the same reason.
static const int CODA_WM_MOVETABTOWINDOW = WM_APP + 9;

// One file pick the engine asked for, posted as CODA_WM_SHOWFILEPICKER to the hidden owner
// window; the wide strings own the dialog's title and filter text.
struct FilePickerRequest
{
    std::wstring title;
    std::vector<std::pair<std::wstring, std::wstring>> filters;
    void (*pfnPicked)(void* pContext, const char* pUrl);
    void* pContext;
};

static HMONITOR primaryMonitor;

static RecentFiles recentFiles;

static FilenameAndUri fileSaveDialog(const std::string& name,
                                     const std::string& folder,
                                     const std::vector<COMDLG_FILTERSPEC>& extensions);

static void openCOOLWindow(const FilenameAndUri& filenameAndUri, DocumentMode mode);

static HANDLE copyEngineClipboardData(UINT format, const std::string& mimeType);

static std::string MIME_type_for_clipboard_format(UINT format);

static void applyTitleBarTheme(HWND hWnd);

static void applyTitleBarThemeToAllWindows();

static std::set<std::string> currentlyOpenDocumens()
{
    std::set<std::string> result;

    for (const auto& i : documentTabs)
        result.insert(i.second.filenameAndUri.uri);

    return result;
}

// Vector of documents to open passed on the command line, or multiple documents to open selected in
// a file open dialog. We open the next one only as soon as the previous one has finished loading.
// Only the UI thread uses it.
static std::deque<FilenameAndUri> filenamesAndUrisToOpen;

// Open the next document waiting in filenamesAndUrisToOpen, if there is one. This can be called on
// any thread, so it only posts a message, to the hidden window that is there for the whole run of
// the app. The UI thread then takes the document from the queue and opens its window.
void load_next_document()
{
    PostMessageW(hiddenOwnerWindow, CODA_WM_LOADNEXTDOCUMENT, 0, 0);
}

// The strings that CODA-W shows itself, in the user interface language, by context and source
// text. They come from the coda_*.ts catalogs that CODA-Q uses too, in the Qt Linguist XML format.
static std::map<std::pair<std::string, std::string>, std::wstring> translations;

// Read the catalog for the user interface language. The catalogs are named with an underscore, as
// coda_pt_BR.ts, and a region that has no catalog of its own falls back to the one for its
// language, so de-AT gets coda_de.ts. English is the source language and has no catalog.
static void loadTranslations()
{
    // A LANG environment variable can name a character set too, as in de_DE.UTF-8.
    std::string language = uiLanguage.substr(0, uiLanguage.find('.'));
    std::replace(language.begin(), language.end(), '-', '_');

    std::string path;
    for (const std::string& name : { language, language.substr(0, language.find('_')) })
    {
        const std::string candidate = app_installation_path + "translations\\coda_" + name + ".ts";
        if (Poco::File(candidate).exists())
        {
            path = candidate;
            break;
        }
    }
    if (path.empty())
        return;

    try
    {
        // The parser gets the file as a stream, as it would take a path name for a URI.
        Poco::FileInputStream stream(path);
        Poco::XML::InputSource input(stream);
        Poco::XML::DOMParser parser;
        Poco::AutoPtr<Poco::XML::Document> document = parser.parse(&input);
        Poco::AutoPtr<Poco::XML::NodeList> contexts = document->getElementsByTagName("context");
        for (unsigned long i = 0; i < contexts->length(); ++i)
        {
            auto context = static_cast<Poco::XML::Element*>(contexts->item(i));
            Poco::XML::Element* contextName = context->getChildElement("name");
            if (!contextName)
                continue;

            Poco::AutoPtr<Poco::XML::NodeList> messages = context->getElementsByTagName("message");
            for (unsigned long j = 0; j < messages->length(); ++j)
            {
                auto message = static_cast<Poco::XML::Element*>(messages->item(j));
                // A string with plural forms has a translation for each, and CODA-W has none.
                if (message->getAttribute("numerus") == "yes")
                    continue;

                Poco::XML::Element* source = message->getChildElement("source");
                Poco::XML::Element* translation = message->getChildElement("translation");
                if (!source || !translation || translation->innerText().empty())
                    continue;

                // As in a catalog that lrelease compiles for CODA-Q, a translation that is still
                // marked unfinished is used too, and one for a string that is gone is not.
                const std::string type = translation->getAttribute("type");
                if (type == "vanished" || type == "obsolete")
                    continue;

                translations[{ contextName->innerText(), source->innerText() }] =
                    Util::string_to_wide_string(translation->innerText());
            }
        }
    }
    catch (const Poco::Exception& exception)
    {
        LOG_ERR("Could not read the translations in '" << path << "': " << exception.displayText());
    }
}

// The text of a string that CODA-W shows, in the user interface language, or in English when the
// catalog has no translation for it. The call has the form that lupdate looks for, with both
// arguments as string literals, so update-translations finds the string. A string that CODA-Q
// shows too uses the context that it has there, so that it is translated once.
static std::wstring translate(const char* context, const char* source)
{
    const auto it = translations.find({ context, source });
    if (it != translations.end())
        return it->second;
    return Util::string_to_wide_string(source);
}

static void processMessage(DocumentTab& tab, wil::unique_cotaskmem_string& message);

static void closeTab(WindowState& window, int tabId);

static void activateAdjacentTab(WindowState& window, int direction);

static void showTabContextMenu(HWND hWnd, int tabId, POINT point);

static void moveTabToNewWindow(HWND hWnd, int tabId, const POINT* dropPoint = nullptr);

static void moveTabToWindow(int tabId, WindowState& target, int insertAt);

static void layoutTabs(WindowState& window);

static RECT documentArea(const WindowState& window);

static void updateStripVisibility(WindowState& window);

static void pushTabsToStrip(WindowState& window);

static void hideHoverCard();

static bool hasStripInTitleBar(const WindowState& window);

static int topResizeBorderHeight(HWND hWnd);

static int titleBarFrameHeight(const WindowState& window);

[[noreturn]] static void fatal(const std::string& message)
{
    MessageBoxW(hiddenOwnerWindow, Util::string_to_wide_string(message).c_str(), L"ERROR", MB_OK);
    std::abort();
}

static FilenameAndUri generate_new_copy(const std::wstring& templateSourcePath,
                                        const std::wstring& templateBasename,
                                        const std::wstring& templateExtension)
{
    PWSTR documents;
    SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &documents);

    int counter = 0;
    std::wstring templateCopyPath;

    do
    {
        std::wstring number = L"";
        if (counter > 0)
            number = L" (" + std::to_wstring(counter) + L")";
        templateCopyPath = std::wstring(documents) + L"\\" + templateBasename + number + L"." +
            templateExtension;
        counter++;
    } while (std::filesystem::exists(std::filesystem::path(templateCopyPath)));

    std::error_code ec;
    std::filesystem::copy_file(templateSourcePath, templateCopyPath, ec);

    if (ec)
        return {};

    auto path = Poco::Path(Util::wide_string_to_string(templateCopyPath));

    return { Util::wide_string_to_string(templateCopyPath), Poco::URI(path).toString() };
}

static std::wstring new_document(DocumentType type,
                                 const std::string& templateRelativePath,
                                 std::string basename)
{
    std::wstring templateBasename, templateExtension, templateSourcePath;

    if (templateRelativePath == "")
    {
        // Old-style simple blank documents
        switch (type)
        {
            case DocumentType::TEXT:
                templateBasename = L"TextDocument";
                templateExtension = L"odt";
                break;
            case DocumentType::SPREADSHEET:
                templateBasename = L"Spreadsheet";
                templateExtension = L"ods";
                break;
            case DocumentType::PRESENTATION:
                templateBasename = L"Presentation";
                templateExtension = L"odp";
                break;
            case DocumentType::DRAWING:
                templateBasename = L"Drawing";
                templateExtension = L"odg";
                break;
            default:
                fatal("Unexpected case in new_document()");
        }
        basename = Util::wide_string_to_string(templateBasename);
        templateSourcePath = Util::string_to_wide_string(app_installation_path) +
            L"..\\templates\\" + templateBasename + L"." + templateExtension;
    }
    else
    {
        // A template chosen from the "Backstage"
        std::string decodedTemplateRelativePath;
        Poco::URI::decode(templateRelativePath, decodedTemplateRelativePath);

        templateSourcePath =
            Util::string_to_wide_string(app_installation_path +
                                        "..\\cool\\" + decodedTemplateRelativePath);
        auto wrelpath = Util::string_to_wide_string(decodedTemplateRelativePath);
        auto const lastDot = wrelpath.find_last_of(L'.');
        if (lastDot == std::wstring::npos)
            return L"";
        templateExtension = wrelpath.substr(lastDot + 1);
    }

    // The basename is URI-encoded because in some localisation it might contain spaces.
    std::string decodedBasename;
    Poco::URI::decode(basename, decodedBasename);
    auto filenameAndUri = generate_new_copy(templateSourcePath,
                                            Util::string_to_wide_string(decodedBasename),
                                            templateExtension);

    // If creating a new copy of the template failed, return an empty string
    if (filenameAndUri.uri == "")
        return L"";

    auto path = Poco::URI(filenameAndUri.uri).getPath();
    if (path.length() > 4 && path[0] == '/' && path[2] == ':' && path[3] == '/')
        path = path.substr(1);
    auto templateCopyPath = Util::string_to_wide_string(Poco::Path(path).toString());

    return templateCopyPath;
}

static int generate_new_app_doc_id()
{
    // Start with a random document id to catch code that might assume it to be some fixed value,
    // like 0 or 1. Also make it obvious that this numeric "app doc id", used by the mobile apps and
    // CODA, is not related to the string document ids (usually with several leading zeroes) used in
    // the C++ bits of normal COOL.
    static int id = 42 + (std::time(nullptr) % 100);

    DocumentData::allocate(id);
    return id++;
}

static void send2JS(const DocumentTab& tab, const char* buffer, int length)
{
    const bool binaryMessage = COOLProtocol::isBinaryMessage(buffer, static_cast<size_t>(length));
    std::string pretext{ binaryMessage
                             ? "window.TheFakeWebSocket.onmessage({'data': window.atob('"
                             : "window.TheFakeWebSocket.onmessage({'data': window.b64d('" };
    const std::string posttext{ "')});" };

    DWORD base64len = length * 2 + 100;
    std::vector<char> base64(base64len);
    if (!CryptBinaryToStringA((BYTE*)buffer, length, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
                              base64.data(), &base64len))
    {
        LOG_ERR("CryptBinaryToStringA failed: " << GetLastError());
        return;
    }

    if (binaryMessage)
        LOG_TRC("To execute in JS: " << pretext << "stuff" << posttext);
    else
    {
        auto s = std::string(buffer, length);
        LOG_TRC("To execute in JS: " << pretext << s << posttext);
    }
    Log::flush();

    char* wparam = _strdup((pretext + std::string(base64.data()) + posttext).c_str());

    PostMessageW(tab.hWnd, CODA_WM_EXECUTESCRIPT, (WPARAM)wparam, (LPARAM)tab.tabId);
}

// Convert a file: URI to a native Windows path. A UNC location arrives as
// "file://host/share/..." with the host in the authority and becomes
// "\\host\share\..."; a local path arrives as "/C:/dir/file" and becomes
// "C:\dir\file". Non-file URIs, and anything that fails to parse, are returned
// unchanged.
static std::string fileUriToWindowsPath(const std::string& uri)
{
    if (!uri.starts_with("file:"))
        return uri;

    try
    {
        const Poco::URI parsedUri(uri);
        const std::string host = parsedUri.getHost();
        std::string path = parsedUri.getPath();
        if (!host.empty())
            path = "\\\\" + host + path;
        else if (path.size() > 3 && path[0] == '/' && path[2] == ':')
            path = path.substr(1);
        for (char& c : path)
        {
            if (c == '/')
                c = '\\';
        }
        return path;
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Bad file URI [" << uri << "]: " << exc.what());
        return uri;
    }
}

// COKit file save dialog callback.
void output_file_dialog_from_core(const char* suggestedURI, char* result, size_t resultLen)
{
    // Some sanity checks first.
    if (resultLen == 0)
        return;

    // The absolutely shortest file: URI on Windows would be something like "file:///C:/f".
    if (resultLen < 12)
    {
        result[0] = '\0';
        return;
    }

    // fileUriToWindowsPath returns a backslash-separated native path (UNC-aware),
    // so split the folder and filename on backslash.
    const std::string path = fileUriToWindowsPath(suggestedURI);
    const auto lastSlash = path.find_last_of('\\');
    const auto filename = path.substr(lastSlash + 1);
    const auto folder = path.substr(0, lastSlash);
    auto lastPeriod = filename.find_last_of('.');
    auto extension = filename.substr(lastPeriod + 1);
    auto filenameAndUri = fileSaveDialog(filename, folder,
                                         {
                                             {
                                                 Util::string_to_wide_string(extension).c_str(),
                                                 Util::string_to_wide_string("*." + extension).c_str()
                                             }
                                         });

    if (filenameAndUri.uri.size() < resultLen)
        strcpy(result, filenameAndUri.uri.c_str());
    else
        result[0] = '\0';
}

// COKit reveal-in-file-manager callback: open Explorer with the document selected.
void reveal_in_file_manager(const char* uri)
{
    if (uri == nullptr || *uri == '\0')
        return;

    const std::wstring widePath = Util::string_to_wide_string(fileUriToWindowsPath(uri));
    PIDLIST_ABSOLUTE pidl = nullptr;
    if (SUCCEEDED(SHParseDisplayName(widePath.c_str(), nullptr, &pidl, 0, nullptr)) && pidl)
    {
        SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
        CoTaskMemFree(pidl);
    }
}

static void stopServer()
{
    SigUtil::requestShutdown();

    // Wait until coolwsdThread is torn down, so that we don't start cleaning up too early.
    coolwsdThread.join();
}

static void createAndStartMessagePumpThread(DocumentTab& data)
{
    // Create a socket pair to notify the below thread when the document has been closed
    fakeSocketPipe2(data.closeNotificationPipeForForwardingThread);

    // Start another thread to read responses and forward them to the JavaScript
    data.app2js = std::thread(
        [&data]
        {
            ProcUtil::setThreadName("app2js " + std::to_string(data.appDocId));
            while (true)
            {
                struct pollfd pollfd[2];
                pollfd[0].fd = data.fakeClientFd;
                pollfd[0].events = POLLIN;
                pollfd[1].fd = data.closeNotificationPipeForForwardingThread[1];
                pollfd[1].events = POLLIN;
                if (fakeSocketPoll(pollfd, 2, -1) > 0)
                {
                    if (pollfd[1].revents == POLLIN)
                    {
                        // The code below handling the "BYE" fake Websocket message has closed the other
                        // end of the closeNotificationPipeForForwardingThread. Let's close the other
                        // end too just for cleanliness, even if a FakeSocket as such is not a system
                        // resource so nothing is saved by closing it.
                        fakeSocketClose(data.closeNotificationPipeForForwardingThread[1]);

                        // Close our end of the fake socket connection to the ClientSession thread, so
                        // that it terminates.
                        fakeSocketClose(data.fakeClientFd);

                        return;
                    }
                    if (pollfd[0].revents == POLLIN)
                    {
                        int n = fakeSocketAvailableDataLength(data.fakeClientFd);
                        // I don't want to check for n being -1 here, even if that will lead to a crash,
                        // as n being -1 is a sign of something being wrong elsewhere anyway, and I
                        // prefer to fix the root cause. Let's see how well this works out.
                        if (n == 0)
                            return;
                        std::vector<char> buf(n);
                        n = fakeSocketRead(data.fakeClientFd, buf.data(), n);
                        send2JS(data, buf.data(), n);
                    }
                }
                else
                {
                    break;
                }
            }
            assert(false);
        });
}

static void do_hullo_handling_things(DocumentTab& data)
{
    // Now we know that the JS has started completely

    // Contact the permanently (during app lifetime) listening COOLWSD server "public" socket
    assert(coolwsd_server_socket_fd != -1);
    int rc = fakeSocketConnect(data.fakeClientFd, coolwsd_server_socket_fd);
    (void)rc;
    assert(rc != -1);

    createAndStartMessagePumpThread(data);

    // First we must send the URL. This corresponds to the GET request with Upgrade to WebSocket.
    // This *must* be the first message written to the "client" thread. We don't need to do this
    // write in a separate thread, and we can't, because if we do that, we will occasionally run into
    // a bug when the "coolclient" message sent by the JS is received and gets forwarded to the
    // "client" thread before we have written the URL to it.

    std::string message(data.filenameAndUri.uri + " " + std::to_string(data.appDocId));
    fakeSocketWriteQueue(data.fakeClientFd, message.c_str(), message.size());
}

static void do_welcome_handling_things(DocumentTab& data)
{
#if ENABLE_DEBUG
    // A debug build bundles the placeholder slideshow, shown only when the configuration turns
    // the welcome screen on. A release build carries a slideshow only when the app branding
    // provides one.
    if (!ConfigUtil::getBool("welcome.enable", false))
        return;
#endif

    const auto welcomeSlideshow = Poco::Path(app_installation_path + "..\\cool\\welcome\\welcome-slideshow.odp");

    if (!Poco::File(welcomeSlideshow).exists())
        return;

    openCOOLWindow({ welcomeSlideshow.getFileName(), Poco::URI(welcomeSlideshow).toString() }, DocumentMode::WELCOME);
}

static void enter_full_screen(WindowState& data, HMONITOR monitor, bool saveRestoreInfo)
{
    if (data.isFullScreen)
        return;

    LONG style = GetWindowLong(data.hWnd, GWL_STYLE);

    if (saveRestoreInfo)
    {
        GetWindowRect(data.hWnd, &data.originalRect);
        data.originalStyle = style;
    }

    // Remove window borders and title bar
    style &= ~(WS_OVERLAPPEDWINDOW);
    SetWindowLong(data.hWnd, GWL_STYLE, style);

    MONITORINFO monitorInfo = { sizeof(monitorInfo) };
    GetMonitorInfo(monitor, &monitorInfo);

    // Resize window to fill the entire monitor
    SetWindowPos(data.hWnd, NULL,
                 monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                 monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                 monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                 SWP_NOZORDER | SWP_FRAMECHANGED);
    data.isFullScreen = true;

    // The presentation has the monitor to itself, tab strip included.
    updateStripVisibility(data);
}

static void leave_full_screen(WindowState& data)
{
    if (!data.isFullScreen)
        return;

    SetWindowLong(data.hWnd, GWL_STYLE, data.originalStyle);

    // Restore in two steps, position first, then size.
    // So if we restore from external monitor at a different resolution than
    // the laptop monitor, that a WM_DPICHANGED triggered from changing monitor
    // (which gets processed during SetWindowPos) doesn't mangle the size.
    SetWindowPos(data.hWnd, NULL,
                 data.originalRect.left, data.originalRect.top,
                 data.originalRect.right - data.originalRect.left,
                 data.originalRect.bottom - data.originalRect.top,
                 SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSIZE);
    SetWindowPos(data.hWnd, NULL,
                 data.originalRect.left, data.originalRect.top,
                 data.originalRect.right - data.originalRect.left,
                 data.originalRect.bottom - data.originalRect.top,
                 SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE);

    data.isFullScreen = false;

    updateStripVisibility(data);
}

static void do_bye_handling_things(const DocumentTab& data)
{
    LOG_TRC_NOFILE(
        "Document window terminating on JavaScript side. Closing our end of the socket.");

    // Close one end of the socket pair, that will wake up the forwarding thread above
    fakeSocketClose(data.closeNotificationPipeForForwardingThread[0]);

    // For the welcome slideshow we need to close the window ourselves
    if (data.mode == DocumentMode::WELCOME)
        PostMessageW(data.hWnd, WM_CLOSE, 0, 0);
}

static void do_print(int appDocId)
{
    const std::string tempFile = FileUtil::createRandomTmpDir() + "\\p.pdf";
    const std::string tempFileUri = Poco::URI(Poco::Path(tempFile)).toString();

    // Leave form fields and comments out of the printout, like the browser does when printing to
    // PDF.
    DocumentData::get(appDocId).loKitDocument->saveAs(
        tempFileUri.c_str(), "pdf",
        "{\"ExportFormFields\":{\"type\":\"boolean\",\"value\":\"false\"},"
        "\"ExportNotes\":{\"type\":\"boolean\",\"value\":\"false\"}}");

    STARTUPINFOW startupInfo{ sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION processInformation;

    if (!CreateProcessW(
            Util::string_to_wide_string(app_installation_path + "..\\PrintPDFAndDelete.exe").c_str(),
            Util::string_to_wide_string("PrintPDFAndDelete " + tempFileUri).data(), NULL, NULL,
            TRUE, 0, NULL, NULL, &startupInfo, &processInformation))
        LOG_ERR("CreateProcess failed: " << GetLastError());
}

static void do_other_message_handling_things(const DocumentTab& data, const char* message)
{
    LOG_TRC_NOFILE("Handling other message:'" << message << "'");

    fakeSocketWriteQueue(data.fakeClientFd, message, strlen(message));
}

namespace
{
// Deliver a reply to a browser-side postMobileCall() as a posted web message,
// i.e. structured data rather than generated JS source. We build the envelope
// {"id":<id>,"reply":<value>} and marshal it to the WebView2 UI thread, where
// PostWebMessageAsJson() hands the parsed value to the page's
// chrome.webview 'message' listener (see global.js). The browser receives
// <reply> as a real JS value (string or object), so Windows paths, file names
// etc. never get interpolated into a JS string literal. This mirrors how the
// macOS (WKScriptMessageHandlerWithReply) and Qt (QWebChannel) apps return
// values; JSON encoding takes care of all escaping.
void postReplyToCall(HWND hWnd, int tabId, int id, const Poco::Dynamic::Var& reply)
{
    Poco::JSON::Object::Ptr envelope = new Poco::JSON::Object();
    envelope->set("id", id);
    envelope->set("reply", reply);
    std::ostringstream oss;
    envelope->stringify(oss);
    PostMessageW(hWnd, CODA_WM_POSTWEBMESSAGE, (WPARAM)_strdup(oss.str().c_str()), (LPARAM)tabId);
}
} // namespace

static void do_getrecentdocs(const DocumentTab& data, int id)
{
    postReplyToCall(data.hWnd, data.tabId, id, recentFiles.serialiseFiltered(currentlyOpenDocumens()));
}

// It happens that some other process opens the clipboard for a short time, and if we happen to try
// during that time it will fail. Workaround for that: a function that tries a couple of times.

static BOOL try_open_clipboard(HWND hWnd)
{
    const int NTRIES = 10;
    for (int i = 0; i < NTRIES; i++)
    {
        if (OpenClipboard(hWnd))
            return TRUE;
        Sleep(5);
    }
    LOG_ERR("OpenClipboard() failed repeatedly, last error: " << GetLastError());
    return FALSE;
}

static std::wstring get_clipboard_format_name(UINT format)
{
    const int NNAME{ 1000 };
    wchar_t name[NNAME];
    int nwc = GetClipboardFormatNameW(format, name, NNAME);
    if (nwc)
        return std::wstring(name, nwc);
    return L"";
}

static std::string get_html_clipboard_fragment(const char* data)
{
    std::string htmlData(data);

    std::regex startRegex("(\r|\n)StartFragment:(\\d+)(\r|\n)");
    std::regex endRegex("(\r|\n)EndFragment:(\\d+)(\r|\n)");

    std::smatch match;
    size_t startPos = std::string::npos;
    size_t endPos = std::string::npos;

    if (std::regex_search(htmlData, match, startRegex))
        startPos = std::stoul(match[2]);

    if (std::regex_search(htmlData, match, endRegex))
        endPos = std::stoul(match[2]);

    if (startPos == std::string::npos || endPos == std::string::npos || startPos >= endPos || endPos > htmlData.size())
        return "";

    return htmlData.substr(startPos, endPos - startPos);
}

// Convert an HTML document into the Windows "HTML Format" clipboard payload.
//
// See
// https://learn.microsoft.com/en-us/windows/win32/dataxchg/html-clipboard-format
// for the specification of that format.
//
// This is deliberately a tiny tokenizer, not a full parser. It is not likely that
// engine would actually produce pathological HTML with things like the string "<body"
// inside an attribute, comment, or whatever, but be careful anyway.
//   * The <body> start tag may carry attributes; we find the true end '>'
//     of the start tag while respecting quoted attribute values.
//   * The substring "<body" may occur inside an attribute value in <head>
//     (e.g. <meta content="<body>">). Because we scan tag by tag and skip
//     over quoted attribute regions, such occurrences are not mistaken for
//     the body element.
//   * Comments, <!doctype>/<?...?> declarations, and the raw-text elements
//     <script>/<style>/<textarea>/<title> are skipped wholesale, so "<body"
//     or "</body>" appearing as text/inside them is ignored too.
//
//  Header lines use CRLF, matching what Windows browsers emit.

struct BodySpan
{
    bool found = false;
    bool endFound = false;
    std::size_t startBegin = 0;
    std::size_t endEnd = 0;
};

static bool case_insensitive_match(const std::string& haystack, std::size_t pos, const std::string_view needle)
{
    if (pos + needle.size() > haystack.size())
        return false;
    for (std::size_t k = 0; k < needle.size(); ++k)
    {
        auto a = static_cast<unsigned char>(haystack[pos + k]);
        auto b = static_cast<unsigned char>(needle[k]);
        if (std::tolower(a) != std::tolower(b))
            return false;
    }
    return true;
}

// i points at the '<' of a tag. Return the index just past the tag's
// closing '>', honoring single- and double-quoted attribute values (so a
// '>' inside a quoted value does not end the tag).
static std::size_t skip_tag(const std::string& s, std::size_t i)
{
    const std::size_t n = s.size();
    ++i;
    char quote = 0;
    while (i < n)
    {
        char c = s[i];
        if (quote)
        {
            if (c == quote)
                quote = 0;
        }
        else if (c == '"' || c == '\'')
            quote = c;
        else if (c == '>')
            return i + 1;
        ++i;
    }
    return n; // unterminated tag: treat rest of string as the tag
}

static bool name_equals(const std::string_view name, const std::string_view lit)
{
    if (name.size() != lit.size())
        return false;
    for (std::size_t k = 0; k < lit.size(); ++k)
    {
        if (std::tolower(static_cast<unsigned char>(name[k])) !=
            std::tolower(static_cast<unsigned char>(lit[k])))
            return false;
    }
    return true;
}

static BodySpan find_body(const std::string& s)
{
    BodySpan r;
    const std::size_t n = s.size();
    std::size_t i = 0;

    while (i < n)
    {
        if (s[i] != '<')
        {
            ++i;
            continue;
        }

        // Comment: <!-- ... -->
        if (case_insensitive_match(s, i, "<!--"))
        {
            std::size_t e = s.find("-->", i + 4);
            i = (e == std::string::npos) ? n : e + 3;
            continue;
        }
        // Declaration or processing instruction: <! ... >  /  <? ... >
        if (i + 1 < n && (s[i + 1] == '!' || s[i + 1] == '?'))
        {
            std::size_t e = s.find('>', i + 2);
            i = (e == std::string::npos) ? n : e + 1;
            continue;
        }

        const bool isEnd = (i + 1 < n && s[i + 1] == '/');
        const std::size_t nameStart = i + (isEnd ? 2 : 1);

        // Read an ASCII tag name (letters, digits, '-').
        std::size_t j = nameStart;
        while (j < n)
        {
            unsigned char u = static_cast<unsigned char>(s[j]);
            if (std::isalnum(u) || u == '-')
                ++j;
            else
                break;
        }
        std::string_view name(s.data() + nameStart, j - nameStart);

        if (name.empty()) // a bare '<' that isn't a real tag
        {
            ++i;
            continue;
        }

        if (!isEnd && name_equals(name, "body"))
        {
            r.found = true;
            r.startBegin = i;
            i = skip_tag(s, i); // step past the (possibly attributed) start tag
            continue;
        }
        if (isEnd && name_equals(name, "body"))
        {
            r.endFound = true;
            r.endEnd = skip_tag(s, i);
            break; // done: we have both ends
        }

        // Raw-text elements: skip their entire content to the matching end tag
        // so their text (which may contain '<', '>', quotes, "<body>", etc.)
        // never confuses the scan.
        if (!isEnd && (name_equals(name, "script") || name_equals(name, "style") ||
                       name_equals(name, "textarea") || name_equals(name, "title")))
        {
            std::size_t after = skip_tag(s, i);
            std::string endTag = "</";
            endTag.append(name);
            std::size_t e = after;
            std::size_t close = std::string::npos;
            while (e + endTag.size() <= n)
            {
                if (case_insensitive_match(s, e, endTag))
                {
                    close = e;
                    break;
                }
                ++e;
            }
            i = (close == std::string::npos) ? n : skip_tag(s, close);
            continue;
        }

        // Any other tag: skip it.
        i = skip_tag(s, i);
    }
    return r;
}

static std::string num10(std::size_t v)
{
    char b[24];
    std::snprintf(b, sizeof b, "%010zu", v);
    return std::string(b);
}

static std::string build_header(std::size_t startHtml, std::size_t endHtml,
                                std::size_t startFrag, std::size_t endFrag)
{
    std::string h;
    h += "Version:0.9\r\n";
    h += "StartHTML:"     + num10(startHtml) + "\r\n";
    h += "EndHTML:"       + num10(endHtml)   + "\r\n";
    h += "StartFragment:" + num10(startFrag) + "\r\n";
    h += "EndFragment:"   + num10(endFrag)   + "\r\n";
    return h;
}

std::string generate_html_format(const std::string& html)
{
    static const std::string SF = "<!--StartFragment-->";
    static const std::string EF = "<!--EndFragment-->";

    // Measure the headers once with dummy values.
    const std::size_t headerLen = build_header(0, 0, 0, 0).size();

    // Locate the body. Degrade gracefully if it's missing or unterminated.
    const BodySpan b = find_body(html);
    const std::size_t startBegin = b.found ? b.startBegin : 0;
    const std::size_t endEnd = (b.found && b.endFound) ? b.endEnd : html.size();

    // Assemble the content: prefix + <!--StartFragment--> + <body>..</body>
    //                       + <!--EndFragment--> + suffix.
    std::string content;
    content.reserve(html.size() + SF.size() + EF.size());
    content.append(html, 0, startBegin);
    content.append(SF);
    content.append(html, startBegin, endEnd - startBegin);
    content.append(EF);
    content.append(html, endEnd, std::string::npos);

    // Offsets are byte positions from the very start of the payload.
    const std::size_t startHtml = headerLen;
    const std::size_t endHtml = headerLen + content.size();
    const std::size_t startFrag = headerLen + startBegin + SF.size(); // at '<body'
    const std::size_t endFrag = headerLen + endEnd + SF.size(); // just past '</body>'

    return build_header(startHtml, endHtml, startFrag, endFrag) + content;
}

static void do_open_hyperlink(HWND hWnd, std::wstring url)
{
    // For file: URIs hand ShellExecute a native (UNC-aware) Windows path; other
    // schemes (http:, mailto:, ...) pass through unchanged.
    const std::wstring target =
        Util::string_to_wide_string(fileUriToWindowsPath(Util::wide_string_to_string(url)));
    ShellExecuteW(hWnd, NULL, target.c_str(), NULL, NULL, SW_SHOW);
}

struct MonitorInfo
{
    HMONITOR hMonitor;
    DWORD dwFlags;
};

typedef std::vector<MonitorInfo> Monitors;

BOOL monitorEnum(HMONITOR monitor, HDC, LPRECT, LPARAM data)
{
    MONITORINFO monitorInfo = { sizeof(monitorInfo) };
    GetMonitorInfo(monitor, &monitorInfo);

    Monitors& monitors = *reinterpret_cast<Monitors*>(data);
    monitors.push_back(MonitorInfo{monitor, monitorInfo.dwFlags});
    return true;
}

Monitors getMonitors()
{
    Monitors monitors;
    EnumDisplayMonitors(nullptr, nullptr, monitorEnum, reinterpret_cast<LPARAM>(&monitors));
    return monitors;
}

static void exchangeMonitors(DocumentTab& data)
{
    Monitors monitors(getMonitors());
    if (monitors.size() < 2)
        return;

    WindowState* presentationWindow = findWindow(data.hWnd);
    if (!presentationWindow)
        return;
    WindowState* consoleWindow = data.hConsoleWnd ? findWindow(data.hConsoleWnd) : nullptr;

    HMONITOR hConsoleMonitor = data.hConsoleWnd ? MonitorFromWindow(data.hConsoleWnd, MONITOR_DEFAULTTONEAREST) : 0;
    HMONITOR hPresentationMonitor = MonitorFromWindow(data.hWnd, MONITOR_DEFAULTTONEAREST);

    size_t origConsoleMonitor = 0;
    size_t origPresentationMonitor = 0;
    for (size_t i = 0; i < monitors.size(); ++i)
    {
        if (hConsoleMonitor && monitors[i].hMonitor == hConsoleMonitor)
            origConsoleMonitor = i;
        if (monitors[i].hMonitor == hPresentationMonitor)
            origPresentationMonitor = i;
    }

    leave_full_screen(*presentationWindow);

    size_t newPresentationMonitor = origPresentationMonitor;

    if (consoleWindow)
    {
        leave_full_screen(*consoleWindow);

        size_t newConsoleMonitor = (origConsoleMonitor + 1) % monitors.size();
        if (newConsoleMonitor == newPresentationMonitor)
            newPresentationMonitor = (newPresentationMonitor + 1) % monitors.size();

        enter_full_screen(*consoleWindow, monitors[newConsoleMonitor].hMonitor, false);
    }
    else
    {
        newPresentationMonitor = (newPresentationMonitor + 1) % monitors.size();
    }

    enter_full_screen(*presentationWindow, monitors[newPresentationMonitor].hMonitor, false);
}

static std::string pathToURI(const Poco::Path& path)
{
    auto uri = Poco::URI(path);

    if (path.getNode() == "")
        return uri.toString();

    uri.setHost(path.getNode());
    return uri.toString();
}

static std::vector<FilenameAndUri> fileOpenDialog(const std::wstring& title,
                                                  const std::vector<COMDLG_FILTERSPEC>& filters,
                                                  bool multiSelect)
{
    IFileOpenDialog* dialog;

    if (!SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IFileOpenDialog, reinterpret_cast<void**>(&dialog))))
        fatal("CoCreateInstance(CLSID_FileOpenDialog) failed");

    if (!title.empty())
        dialog->SetTitle(title.c_str());

    if (!filters.empty())
        if (!SUCCEEDED(dialog->SetFileTypes(filters.size(), filters.data())))
            fatal("dialog->SetFileTypes() failed");

    FILEOPENDIALOGOPTIONS options;
    if (SUCCEEDED(dialog->GetOptions(&options)))
    {
        if (multiSelect)
            options |= FOS_ALLOWMULTISELECT;
        options &= ~FOS_DONTADDTORECENT;
        dialog->SetOptions(options);
    }

    HRESULT dialogResult = dialog->Show(hiddenOwnerWindow);

    if (!SUCCEEDED(dialogResult))
        return {};

    std::vector<FilenameAndUri> result;

    IShellItemArray* items;
    if (!SUCCEEDED(dialog->GetResults(&items)))
        fatal("dialog->GetResults() failed");

    DWORD numItems;
    if (!SUCCEEDED(items->GetCount(&numItems)))
        fatal("items->GetCount() failed");

    for (int i = 0; i < numItems; i++)
    {
        IShellItem* item;
        PWSTR fileSysPath;
        if (SUCCEEDED(items->GetItemAt(i, &item)) &&
            SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &fileSysPath)))
        {
            auto path = Poco::Path(Util::wide_string_to_string(std::wstring(fileSysPath)));
            result.push_back({ path.getFileName(), pathToURI(path) });
            CoTaskMemFree(fileSysPath);
            item->Release();
        }
    }
    items->Release();
    dialog->Release();

    return result;
}

static std::vector<COMDLG_FILTERSPEC>getSaveAsFormats(COKitDocumentType docType)
{
    std::vector<COMDLG_FILTERSPEC> result;

    if (docType == COKitDocumentType::TEXT)
    {
        result.push_back({L"ODT", L"*.odt"});
        result.push_back({L"RTF", L"*.rtf"});
        result.push_back({L"DOCX", L"*.docx"});
        result.push_back({L"DOC", L"*.doc"});
    }
    else if (docType == COKitDocumentType::SPREADSHEET)
    {
        result.push_back({L"ODS", L"*.ods"});
        result.push_back({L"XLSX", L"*.xlsx"});
        result.push_back({L"XLS", L"*.xls"});
    }
    else if (docType == COKitDocumentType::PRESENTATION)
    {
        result.push_back({L"ODP", L"*.odp"});
        result.push_back({L"PPTX", L"*.pptx"});
        result.push_back({L"PPT", L"*.ppt"});
    }
    else if (docType == COKitDocumentType::DRAWING)
    {
        result.push_back({L"ODG", L"*.odg"});
    }

    return result;
}

static FilenameAndUri fileSaveDialog(const std::string& name,
                                     const std::string& folder,
                                     const std::vector<COMDLG_FILTERSPEC>& extensions)
{
    IFileSaveDialog* dialog;

    if (!SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IFileSaveDialog, reinterpret_cast<void**>(&dialog))))
        fatal("CoCreateInstance(CLSID_FileSaveDialog) failed");

    FILEOPENDIALOGOPTIONS options;

    if (!SUCCEEDED(dialog->GetOptions(&options)))
        fatal("dialog->GetOptions() failed");

    options |= FOS_STRICTFILETYPES;

    if (!SUCCEEDED(dialog->SetOptions(options)))
        fatal("dialog->SetOptions() failed");

    if (extensions.size() > 0)
    {
        if (!SUCCEEDED(dialog->SetDefaultExtension(extensions[0].pszSpec)))
            fatal("dialog->SetDefaultExtension() failed");

        if (!SUCCEEDED(dialog->SetFileTypes(extensions.size(), extensions.data())))
            fatal("dialog->SetFileTypes() failed");
    }

    if (name != "" && !SUCCEEDED(dialog->SetFileName(Util::string_to_wide_string(name).c_str())))
        fatal("dialog->SetFileName() failed");

    if (folder != "")
    {
        std::wstring wfolder = Util::string_to_wide_string(folder);
        std::replace(wfolder.begin(), wfolder.end(), L'/', L'\\');

        IShellItem* psiFolder;
        if (SUCCEEDED(SHCreateItemFromParsingName(wfolder.c_str(), nullptr, IID_PPV_ARGS(&psiFolder))))
        {
            if (!SUCCEEDED(dialog->SetFolder(psiFolder)))
                fatal("dialog->SetFolder() failed");
            psiFolder->Release();
        }
    }

    if (!SUCCEEDED(dialog->Show(hiddenOwnerWindow)))
        return {};

    IShellItem* item;
    if (!SUCCEEDED(dialog->GetResult(&item)))
        fatal("dialog->GetResult() failed");

    PWSTR fileSysPath;
    if (!SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &fileSysPath)))
        fatal("item->GetDisplayName() failed");

    auto path = Poco::Path(Util::wide_string_to_string(std::wstring(fileSysPath)));
    CoTaskMemFree(fileSysPath);
    item->Release();
    dialog->Release();

    return { path.getFileName(), pathToURI(path) };
}

static void arrangePresentationWindows(DocumentTab& data)
{
    WindowState* presentationWindow = findWindow(data.hWnd);
    if (!presentationWindow)
        return;
    WindowState* consoleWindow = data.hConsoleWnd ? findWindow(data.hConsoleWnd) : nullptr;

    Monitors monitors(getMonitors());
    presentationWindow->numMonitors = monitors.size();

    HMONITOR laptopMonitor = 0;
    HMONITOR externalMonitor = 0;

    for (const auto& monitor : monitors)
    {
        if (monitor.dwFlags & MONITORINFOF_PRIMARY)
        {
            if (!laptopMonitor)
                laptopMonitor = monitor.hMonitor;
        }
        else
        {
            if (!externalMonitor)
                externalMonitor = monitor.hMonitor;
        }
    }

    if (!laptopMonitor || !externalMonitor)
    {
        laptopMonitor = MonitorFromWindow(data.hWnd, MONITOR_DEFAULTTONEAREST);
        externalMonitor = 0;
        for (const auto& monitor : monitors)
        {
            if (monitor.hMonitor != laptopMonitor)
            {
                externalMonitor = monitor.hMonitor;
                break;
            }
        }
    }

    leave_full_screen(*presentationWindow);
    if (consoleWindow)
        leave_full_screen(*consoleWindow);

    HMONITOR presenterMonitor = externalMonitor ? externalMonitor : laptopMonitor;

    enter_full_screen(*presentationWindow, presenterMonitor, true);

    if (consoleWindow)
    {
        if (externalMonitor)
            enter_full_screen(*consoleWindow, laptopMonitor, true);
        else
            BringWindowToTop(data.hConsoleWnd);
    }
}

// The MIME type that Windows has registered for the extension of the given file name, or an empty
// string when the extension is unknown. This is the same registry entry that the shell and the
// WebView2 read, so a file dropped on the app gets the same MIME type that the web content would
// have seen for it.
static std::string mimeTypeForFileName(const std::string& filename)
{
    const auto lastPeriod = filename.find_last_of('.');
    if (lastPeriod == std::string::npos)
        return "";

    const std::wstring extension = Util::string_to_wide_string(filename.substr(lastPeriod));

    wchar_t mimeType[200];
    DWORD numChars = sizeof(mimeType) / sizeof(mimeType[0]);
    if (AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_CONTENTTYPE, extension.c_str(), nullptr, mimeType,
                          &numChars) != S_OK)
        return "";

    return Util::wide_string_to_string(std::wstring(mimeType));
}

// True for the files that go into a document instead of being opened as a document of their own.
// Images and media are what the document can hold, and what the browser side already knows how to
// insert.
static bool isInsertableIntoDocument(const std::string& mimeType)
{
    return mimeType.starts_with("image/") || mimeType.starts_with("audio/") ||
        mimeType.starts_with("video/");
}

// Hand a dropped image or media file to the JS, so that it ends up in the document the same way as
// a file dropped straight onto the document view used to. The bytes travel as base64 in a posted
// web message, because the bridge to the JavaScript carries strings only. The drop point travels
// with them, in physical pixels relative to the window client area, so that the insert can happen
// where the user aimed.
static void insertDroppedFile(HWND hWnd, int tabId, const std::string& path,
                              const std::string& mimeType, const POINT& point)
{
    const std::string filename = Poco::Path(path).getFileName();

    std::ifstream stream;
    FileUtil::openFileToIFStream(path, stream, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open())
    {
        LOG_ERR("Cannot open dropped file [" << path << "]");
        return;
    }

    const std::vector<char> contents((std::istreambuf_iterator<char>(stream)),
                                     std::istreambuf_iterator<char>());
    if (contents.empty())
        return;

    // The whole file becomes one base64 string, and its length has to fit in the DWORD that
    // CryptBinaryToStringA counts characters in.
    constexpr size_t maxSize = 256 * 1024 * 1024;
    if (contents.size() > maxSize)
    {
        LOG_ERR("Dropped file [" << filename << "] is too large to insert, " << contents.size()
                                 << " bytes");
        return;
    }

    // Four base64 characters for every three bytes, rounded up, and room for the terminating NUL
    // character.
    DWORD base64Length = static_cast<DWORD>((contents.size() + 2) / 3 * 4 + 8);
    std::vector<char> base64(base64Length);
    if (!CryptBinaryToStringA((BYTE*)contents.data(), static_cast<DWORD>(contents.size()),
                              CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64.data(),
                              &base64Length))
    {
        const auto error = GetLastError();
        LOG_ERR("CryptBinaryToStringA failed: " << error);
        return;
    }

    Poco::JSON::Object::Ptr message = new Poco::JSON::Object();
    message->set("command", "insertdroppedfile");
    message->set("name", filename);
    message->set("mimetype", mimeType);
    message->set("data", std::string(base64.data()));
    message->set("x", point.x);
    message->set("y", point.y);
    std::ostringstream oss;
    message->stringify(oss);

    PostMessageW(hWnd, CODA_WM_POSTWEBMESSAGE, (WPARAM)_strdup(oss.str().c_str()), (LPARAM)tabId);
}

static void hideDropOverlay(WindowState& data);

// The timer that watches for a drag that goes away without the overlay hearing about it.
static const int TIMER_ID_DROP_OVERLAY = 1;

// The pathnames in a CF_HDROP file list, in their order there.
static std::vector<std::wstring> pathsInFileList(HDROP drop)
{
    std::vector<std::wstring> paths;
    const UINT numFiles = DragQueryFileW(drop, 0xFFFFFFFF, NULL, 0);
    for (UINT i = 0; i < numFiles; i++)
    {
        // The length returned does not count the terminating NUL character, so the buffer needs
        // one more.
        const UINT length = DragQueryFileW(drop, i, NULL, 0);
        if (length == 0)
            continue;

        std::wstring path(length + 1, L'\0');
        if (DragQueryFileW(drop, i, path.data(), length + 1) == 0)
            continue;
        path.resize(length);
        paths.push_back(std::move(path));
    }
    return paths;
}

// Deal with the files in a drop, whether it arrived as a WM_DROPFILES message or through the drop
// target of the overlay window. An image or media file goes into the document, everything else
// becomes one more document to open: the first one opens right away, the rest follow one by one as
// each preceding document finishes loading. The drop point is in client coordinates, and negative
// when the drop happened outside the client area, on the window frame, in which case the file goes
// in at the cursor.
static void handleDroppedFiles(HWND hWnd, HDROP drop, const POINT& dropPoint)
{
    // The starter backstage window, the welcome slideshow and the presenter console show no
    // document to insert into, so everything dropped on them opens as a document. In the tabbed
    // window the drop goes to the document on show.
    WindowState* window = findWindow(hWnd);
    DocumentTab* tab = window ? activeTabOf(*window) : nullptr;
    const bool canInsert = tab && tab->mode == DocumentMode::EDIT && !tab->isConsole;

    // The document view starts below the tab strip, and the page places the insert relative to
    // its own top left corner. A drop on the window frame arrives negative and stays that way,
    // which puts the insert at the cursor instead.
    // The client coordinates of a mirrored window run from the right, and the page measures from
    // the left.
    POINT documentPoint = dropPoint;
    if (window && documentPoint.x >= 0 && documentPoint.y >= 0)
    {
        const RECT area = documentArea(*window);
        const bool mirrored = (GetWindowLongW(hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL) != 0;
        documentPoint.x = mirrored ? area.right - documentPoint.x : documentPoint.x - area.left;
        documentPoint.y -= area.top;
    }

    bool anyDocumentToOpen = false;
    for (const std::wstring& droppedPath : pathsInFileList(drop))
    {
        auto path = Poco::Path(Util::wide_string_to_string(droppedPath));
        const std::string mimeType = mimeTypeForFileName(path.getFileName());

        if (canInsert && isInsertableIntoDocument(mimeType))
        {
            insertDroppedFile(hWnd, tab->tabId, path.toString(), mimeType, documentPoint);
            continue;
        }

        filenamesAndUrisToOpen.push_back({ path.getFileName(), pathToURI(path) });
        anyDocumentToOpen = true;
    }

    if (anyDocumentToOpen)
        load_next_document();
}

// The drop target of the overlay window. Windows looks for a drop target by hit-testing the window
// under the pointer, so this one gets the drops that would otherwise go to the web view.
class FileDropTarget final : public IDropTarget
{
public:
    FileDropTarget(HWND hWnd)
        : m_hWnd(hWnd)
    {
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** object) override
    {
        if (riid == IID_IUnknown || riid == IID_IDropTarget)
        {
            *object = static_cast<IDropTarget*>(this);
            AddRef();
            return S_OK;
        }
        *object = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_referenceCount); }

    ULONG STDMETHODCALLTYPE Release() override
    {
        const LONG count = InterlockedDecrement(&m_referenceCount);
        if (count == 0)
            delete this;
        return count;
    }

    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* dataObject, DWORD, POINTL,
                                        DWORD* effect) override
    {
        m_hasFiles = holdsFileList(dataObject);
        *effect = m_hasFiles ? DROPEFFECT_COPY : DROPEFFECT_NONE;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragOver(DWORD, POINTL, DWORD* effect) override
    {
        *effect = m_hasFiles ? DROPEFFECT_COPY : DROPEFFECT_NONE;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DragLeave() override
    {
        if (WindowState* window = findWindow(m_hWnd))
            hideDropOverlay(*window);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Drop(IDataObject* dataObject, DWORD, POINTL point,
                                   DWORD* effect) override
    {
        *effect = DROPEFFECT_NONE;
        if (WindowState* window = findWindow(m_hWnd))
            hideDropOverlay(*window);

        FORMATETC format = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM medium;
        if (dataObject->GetData(&format, &medium) != S_OK)
            return S_OK;

        // The drop point comes in screen coordinates, and the overlay covers the client area of the
        // document window, so the client coordinates of the window are what the document sees.
        POINT clientPoint = { point.x, point.y };
        ScreenToClient(m_hWnd, &clientPoint);

        handleDroppedFiles(m_hWnd, static_cast<HDROP>(medium.hGlobal), clientPoint);
        ReleaseStgMedium(&medium);

        *effect = DROPEFFECT_COPY;
        return S_OK;
    }

private:
    static bool holdsFileList(IDataObject* dataObject)
    {
        FORMATETC format = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        return dataObject && dataObject->QueryGetData(&format) == S_OK;
    }

    HWND m_hWnd;
    LONG m_referenceCount = 1;
    bool m_hasFiles = false;
};

// The web view covers the whole client area and takes any drop there itself, so a file dropped on
// the document view never reaches the app. Raising this transparent window over the web view for as
// long as the drag lasts is what brings the drop to the app, with the paths of the dropped files.
// The web view reports the arriving drag, as it still sees the drag events until the overlay covers
// it.
static void showDropOverlay(WindowState& data)
{
    // Where the document view of the window is on the screen. The overlay is a window of its own
    // rather than a child window, because only a top-level window can be made transparent with a
    // layer, and it is owned by the document window, which keeps it above that window and takes it
    // away when the document window goes. It stops short of the tab strip, which takes its own
    // drags.
    // With exactly two points, MapWindowPoints() keeps left less than right also when it maps out
    // of a mirrored window.
    RECT bounds = documentArea(data);
    MapWindowPoints(data.hWnd, HWND_DESKTOP, reinterpret_cast<POINT*>(&bounds), 2);
    const int width = bounds.right - bounds.left;
    const int height = bounds.bottom - bounds.top;
    const POINT topLeft = { bounds.left, bounds.top };

    if (!data.hOverlayWnd)
    {
        data.hOverlayWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                                           overlayWindowClass, L"", WS_POPUP, topLeft.x, topLeft.y,
                                           width, height, data.hWnd, NULL,
                                           appInstance, NULL);
        if (!data.hOverlayWnd)
        {
            const auto error = GetLastError();
            LOG_ERR("CreateWindowExW() for the drop overlay failed: " << error
                                                                      << ", owner window "
                                                                      << data.hWnd << ", size "
                                                                      << width << "x"
                                                                      << height);
            return;
        }

        // Just short of fully transparent. The window has to be visible to be hit-tested as a drop
        // target, but the document has to stay readable under it.
        if (!SetLayeredWindowAttributes(data.hOverlayWnd, 0, 1, LWA_ALPHA))
        {
            const auto error = GetLastError();
            LOG_ERR("SetLayeredWindowAttributes() for the drop overlay failed: " << error);
        }

        data.dropTarget = new FileDropTarget(data.hWnd);
        const HRESULT hr = RegisterDragDrop(data.hOverlayWnd, data.dropTarget);
        if (hr != S_OK)
        {
            LOG_ERR("RegisterDragDrop() failed: " << hr);
            data.dropTarget->Release();
            data.dropTarget = nullptr;
            DestroyWindow(data.hOverlayWnd);
            data.hOverlayWnd = 0;
            return;
        }
    }

    SetWindowPos(data.hOverlayWnd, HWND_TOP, topLeft.x, topLeft.y, width, height,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);

    SetTimer(data.hWnd, TIMER_ID_DROP_OVERLAY, 500, NULL);
}

static void hideDropOverlay(WindowState& data)
{
    if (!data.hOverlayWnd)
        return;

    KillTimer(data.hWnd, TIMER_ID_DROP_OVERLAY);
    ShowWindow(data.hOverlayWnd, SW_HIDE);
}

// A drag that ends on the overlay takes the overlay away with it, and so does one that leaves it.
// A drag can also pass over the web view and be gone again before the overlay is up, and then
// nothing else would ever take it away, and it would sit in front of the document and swallow the
// input meant for it. A drag holds a mouse button down, so the button coming up says the drag is
// over, whether the overlay saw any of it or not.
static void checkDropOverlayStillNeeded(WindowState& data)
{
    const bool buttonIsDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ||
        (GetAsyncKeyState(VK_RBUTTON) & 0x8000);

    if (!buttonIsDown)
        hideDropOverlay(data);
}

static void destroyDropOverlay(WindowState& data)
{
    if (!data.hOverlayWnd)
        return;

    RevokeDragDrop(data.hOverlayWnd);
    data.dropTarget->Release();
    data.dropTarget = nullptr;
    DestroyWindow(data.hOverlayWnd);
    data.hOverlayWnd = 0;
}

// Identifier for the Ctrl+Shift+I hotkey that opens the WebView2 developer
// tools. The range 0x0000 to 0xBFFF is reserved for application hotkeys.
static const int HOTKEY_ID_DEVTOOLS = 0x00DA;

// Window procedure for a hidden window used as clipboard owner. It lives for the whole app run, so
// its delayed-render promise only has to be materialized (WM_RENDERALLFORMATS) once, when the app
// exits, not when an individual document window closes. Also used as parent window for the file
// open and save dialogs, and to open the documents waiting in filenamesAndUrisToOpen one at a time.
static LRESULT CALLBACK HiddenOwnerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_RENDERFORMAT:
        {
            UINT format = (UINT)wParam;
            std::string mimeType = MIME_type_for_clipboard_format(format);
            if (!mimeType.empty())
            {
                HANDLE hData = copyEngineClipboardData(format, mimeType);
                if (hData)
                    SetClipboardData(format, hData);
            }
            return 0;
        }

        case WM_RENDERALLFORMATS:
            // This window is about to be destroyed (the app is exiting) while it still owns the
            // clipboard with formats it only promised. Render them all now, so the content outlives
            // the app. The engine still holds the one shared clipboard, so the bytes are available.
            materialize_clipboard_formats();
            return 0;

        case WM_DESTROYCLIPBOARD:
            weOwnTheClipboard = false;
            return 0;

        case CODA_WM_SHOWFILEPICKER:
        {
            // The engine asked for a file (see filePickerProviderPick). Show the open
            // dialog here on the UI thread and answer the engine's completion; the
            // engine's main loop keeps running while the dialog is open.
            std::unique_ptr<FilePickerRequest> request(
                reinterpret_cast<FilePickerRequest*>(wParam));

            std::vector<COMDLG_FILTERSPEC> filterSpecs;
            for (auto const& f : request->filters)
                filterSpecs.push_back({ f.first.c_str(), f.second.c_str() });

            auto openResult = fileOpenDialog(request->title, filterSpecs, false);
            if (!openResult.empty())
                request->pfnPicked(request->pContext, openResult[0].uri.c_str());
            else
                request->pfnPicked(request->pContext, nullptr);
            return 0;
        }

        case CODA_WM_LOADNEXTDOCUMENT:
            if (filenamesAndUrisToOpen.size() > 0)
            {
                auto nextDocument = filenamesAndUrisToOpen.front();
                filenamesAndUrisToOpen.pop_front();
                openCOOLWindow(nextDocument, DocumentMode::EDIT);
            }
            return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // The system draws the caption buttons of a window with the strip in its title bar, and it
    // handles the mouse over them. WM_NCHITTEST asks it below, after the top resize border.
    WindowState* frameWindow = findWindow(hWnd);
    if (frameWindow && hasStripInTitleBar(*frameWindow) && message != WM_NCHITTEST)
    {
        LRESULT result;
        if (DwmDefWindowProc(hWnd, message, wParam, lParam, &result))
            return result;
    }

    switch (message)
    {
        case WM_NCCALCSIZE:
        {
            if (!wParam || !frameWindow || !hasStripInTitleBar(*frameWindow))
                return DefWindowProc(hWnd, message, wParam, lParam);

            // The client area takes the place of the title bar and of the resize border above it,
            // and the system keeps the borders at the sides and at the bottom. A maximized window
            // reaches above the screen by the height of that border, so its client area starts
            // lower by as much.
            NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
            const LONG top = params->rgrc[0].top;
            const LRESULT result = DefWindowProc(hWnd, message, wParam, lParam);
            params->rgrc[0].top = top;
            if (IsZoomed(hWnd))
                params->rgrc[0].top += topResizeBorderHeight(hWnd);
            return result;
        }

        case WM_NCHITTEST:
        {
            const LRESULT result = DefWindowProc(hWnd, message, wParam, lParam);
            if (result != HTCLIENT || !frameWindow || !hasStripInTitleBar(*frameWindow))
                return result;

            // The strip takes the clicks on itself, so what comes here is the top resize border,
            // the caption buttons, or the part of the title bar between the strip and the
            // buttons. The resize border reaches over the buttons, as on a normal title bar.
            POINT point = { static_cast<short>(LOWORD(lParam)),
                            static_cast<short>(HIWORD(lParam)) };
            ScreenToClient(hWnd, &point);
            RECT client;
            GetClientRect(hWnd, &client);
            const int border = topResizeBorderHeight(hWnd);
            if (!IsZoomed(hWnd) && point.y < border)
            {
                // The hit test codes name the sides of the screen, also in a mirrored window,
                // whose client coordinates run from the right.
                const bool mirrored = (GetWindowLongW(hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL) != 0;
                if (point.x < border)
                    return mirrored ? HTTOPRIGHT : HTTOPLEFT;
                if (point.x >= client.right - border)
                    return mirrored ? HTTOPLEFT : HTTOPRIGHT;
                return HTTOP;
            }
            LRESULT buttonResult;
            if (DwmDefWindowProc(hWnd, message, wParam, lParam, &buttonResult))
                return buttonResult;
            if (point.y < titleBarFrameHeight(*frameWindow))
                return HTCAPTION;
            return HTCLIENT;
        }

        case WM_PAINT:
        {
            if (!frameWindow || !hasStripInTitleBar(*frameWindow))
                return DefWindowProc(hWnd, message, wParam, lParam);

            // Black is clear in the frame that DWM extends into the client area, so the frame and
            // the caption buttons show around the strip.
            PAINTSTRUCT paint;
            HDC dc = BeginPaint(hWnd, &paint);
            RECT frame;
            GetClientRect(hWnd, &frame);
            frame.bottom =
                std::min(frame.bottom, static_cast<LONG>(titleBarFrameHeight(*frameWindow)));
            FillRect(dc, &frame, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
            EndPaint(hWnd, &paint);
            return 0;
        }

        case WM_CREATE:
            {
                // Accept files dragged from Explorer onto the window, which arrive as WM_DROPFILES.
                DragAcceptFiles(hWnd, TRUE);

                // Set the title bar before the window is shown, so it is never drawn in the wrong
                // theme first.
                applyTitleBarTheme(hWnd);

                // Contrary to documentation, when you use CW_USEDEFAULT for the x and y parameters
                // in the CreateWindowW() call, Windows will occasionally place the window so that
                // it is partially obscured by the taskbar. Workaround for that.

                MONITORINFO monitorInfo;
                monitorInfo.cbSize = sizeof(monitorInfo);
                GetMonitorInfoW(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &monitorInfo);

                CREATESTRUCT *cs = (CREATESTRUCT *)lParam;

                int x = cs->x, y = cs->y;

                if (cs->cx < (monitorInfo.rcWork.right - monitorInfo.rcWork.left))
                {
                    if (cs->x < monitorInfo.rcWork.left)
                    {
                        // Left edge obscured by taskbar at the left. Move window right by the width
                        // of the taskbar.
                        x = cs->x + (monitorInfo.rcWork.left - monitorInfo.rcMonitor.left);
                    } else if (cs->x + cs->cx > monitorInfo.rcWork.right)
                    {
                        // Left edge obscured by taskbar at the right. Move window left.
                        x = cs->x - (monitorInfo.rcMonitor.right - monitorInfo.rcWork.right);
                    }
                }
                if (cs->cy < (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top))
                {
                    if (cs->y < monitorInfo.rcWork.top)
                    {
                        // Top edge obscured by taskbar at the top. Move window down by the height
                        // of the taskbar.
                        y = cs->y + (monitorInfo.rcWork.top - monitorInfo.rcMonitor.top);
                    } else if (cs->y + cs->cy > monitorInfo.rcWork.bottom)
                    {
                        // Bottom edge obscured by taskbar at the bottom. Move window up.
                        y = cs->y - (monitorInfo.rcMonitor.bottom - monitorInfo.rcWork.bottom);
                    }
                }

                if (x != cs->x || y != cs->y)
                    SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                return 0;
            }

        case WM_SIZING:
            {
                int minimumWidth = 1000, minimumHeight = 800;

                HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                MONITORINFO monitorInfo;
                monitorInfo.cbSize = sizeof(monitorInfo);
                if (GetMonitorInfoW(monitor, &monitorInfo))
                {
                    // If the monitor has a "reasonable" aspect ratio (1:1 to 2:1) (taking into
                    // account it might be in landscape or portrait orientation), set minimum width
                    // to a quarter of monitor width and minimum height to a third of monitior
                    // height. (Because COOL requires more essential space in the vertical
                    // direction, I think.)
                    double aspectRatio =
                        (double)(monitorInfo.rcWork.right - monitorInfo.rcWork.left) / (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);
                    if (aspectRatio >= 0.49 && aspectRatio <= 2.01)
                    {
                        // Reasonable case
                        minimumWidth = (monitorInfo.rcWork.right - monitorInfo.rcWork.left) / 4;
                        minimumHeight = (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top) / 3;
                    }
                    else if (aspectRatio < 0.49)
                    {
                        // Very narrow, set just minimum height
                        minimumHeight = (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top) / 3;
                    }
                    else if (aspectRatio > 2.01)
                    {
                        // Very wide, set just minimum width
                        minimumWidth = (monitorInfo.rcWork.right - monitorInfo.rcWork.left) / 4;
                    }
                }

                RECT* rect = (RECT*)lParam;
                if (rect->right - rect->left < minimumWidth)
                {
                    switch (wParam)
                    {
                        case WMSZ_LEFT:
                        case WMSZ_TOPLEFT:
                        case WMSZ_BOTTOMLEFT:
                            rect->left = rect->right - minimumWidth;
                            break;
                        case WMSZ_RIGHT:
                        case WMSZ_TOPRIGHT:
                        case WMSZ_BOTTOMRIGHT:
                            rect->right = rect->left + minimumWidth;
                            break;
                        case WMSZ_TOP:
                        case WMSZ_BOTTOM:
                            {
                                // Weird case, resizing height but still the width goes below the
                                // minimum? Grow width on both sizes.
                                auto mid = (rect->left + rect->right) / 2;
                                rect->left = mid - minimumWidth/2;
                                rect->right = rect->left + minimumWidth;
                            }
                            break;
                    }
                }
                if (rect->bottom - rect->top < minimumHeight)
                {
                    switch (wParam)
                    {
                        case WMSZ_TOP:
                        case WMSZ_TOPLEFT:
                        case WMSZ_TOPRIGHT:
                            rect->top = rect->bottom - minimumHeight;
                            break;
                        case WMSZ_BOTTOM:
                        case WMSZ_BOTTOMLEFT:
                        case WMSZ_BOTTOMRIGHT:
                            rect->bottom = rect->top + minimumHeight;
                            break;
                        case WMSZ_LEFT:
                        case WMSZ_RIGHT:
                            {
                                // Weird case, resizing width but still the height goes below the
                                // minimum? Grow height on both sizes.
                                auto mid = (rect->top + rect->bottom) / 2;
                                rect->top = mid - minimumHeight/2;
                                rect->bottom = rect->top + minimumHeight;
                            }
                            break;
                    }
                }
            }
            return TRUE;

        case WM_SIZE:
            if (WindowState* window = findWindow(hWnd))
                layoutTabs(*window);
            break;

        case WM_SETFOCUS:
            if (WindowState* window = findWindow(hWnd))
            {
                DocumentTab* tab = activeTabOf(*window);
                if (tab && tab->webViewController)
                    tab->webViewController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
            }
            break;

        case WM_DPICHANGED:
        {
            const RECT* newRect = (RECT*)lParam;
            SetWindowPos(hWnd, NULL, newRect->left, newRect->top, newRect->right - newRect->left,
                         newRect->bottom - newRect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;

        case WM_DISPLAYCHANGE:
        {
            WindowState* window = findWindow(hWnd);
            DocumentTab* tab = window ? activeTabOf(*window) : nullptr;
            if (tab && (tab->hConsoleWnd || window->isPresFullScreen))
            {
                int numMonitors = getMonitors().size();
                if (window->numMonitors != numMonitors)
                    arrangePresentationWindows(*tab);
            }
        }
        break;

        case WM_SETTINGCHANGE:
            // Take into account the systme theme changes and use that for the titlebar in
            // case the in-app theme preference is not set. `applyTitleBarThemeToAllWindows`
            // need not be called since each window receives this message.
            if (lParam && wcscmp((const wchar_t*)lParam, L"ImmersiveColorSet") == 0)
                applyTitleBarTheme(hWnd);
        break;

        case WM_CLOSE:
        {
            WindowState* window = findWindow(hWnd);
            if (window)
            {
                // Each document gets the goodbye it would get if it were the only one here. The
                // list shrinks as they go, so always take the one at the front.
                while (!window->tabIds.empty())
                    closeTab(*window, window->tabIds.front());
            }
            DestroyWindow(hWnd);
            break;
        }

        case WM_DESTROY:
            if (WindowState* window = findWindow(hWnd))
                destroyDropOverlay(*window);
            if (DocumentData::count() == 0)
                stopServer();
            break;

        case WM_NCDESTROY:
        {
            WindowState* window = findWindow(hWnd);
            if (window)
            {
                if (window->stripController)
                {
                    window->stripController->Close();
                    window->stripController = nullptr;
                }
                window->stripWebView = nullptr;
                windows.erase(hWnd);
                // New documents go to another tabbed window, if there is one left.
                if (tabbedWindow == hWnd)
                {
                    tabbedWindow = 0;
                    for (const auto& i : windows)
                    {
                        if (i.second.isTabbed)
                        {
                            tabbedWindow = i.first;
                            break;
                        }
                    }
                }
            }
            break;
        }

        case CODA_WM_EXECUTESCRIPT:
        {
            DocumentTab* tab = findTab((int)lParam);
            if (tab && tab->webView)
                tab->webView->ExecuteScript(
                    Util::string_to_wide_string(std::string((char*)wParam)).c_str(),
                    Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                        [](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT
                        {
                            // LOG_TRC(Util::wide_string_to_string(resultObjectAsJson));
                            return S_OK;
                        })
                        .Get());
            std::free((char*)wParam);
            break;
        }

        case CODA_WM_POSTWEBMESSAGE:
        {
            DocumentTab* tab = findTab((int)lParam);
            if (tab && tab->webView)
                tab->webView->PostWebMessageAsJson(
                    Util::string_to_wide_string(std::string((char*)wParam)).c_str());
            std::free((char*)wParam);
            break;
        }

        case CODA_WM_CLOSETAB:
        {
            // The document can have moved to another window since the close was posted.
            DocumentTab* tab = findTab((int)wParam);
            WindowState* window = tab ? findWindow(tab->hWnd) : nullptr;
            if (!window)
                break;
            const HWND tabWindow = window->hWnd;
            closeTab(*window, (int)wParam);
            // The window is what is left of the last document, so it goes too.
            if (window->tabIds.empty())
                DestroyWindow(tabWindow);
            break;
        }

        case CODA_WM_ACTIVATEADJACENTTAB:
        {
            if (WindowState* window = findWindow(hWnd))
                activateAdjacentTab(*window, (int)wParam);
            break;
        }

        case CODA_WM_SHOWTABMENU:
            showTabContextMenu(hWnd, (int)wParam,
                               { (short)LOWORD(lParam), (short)HIWORD(lParam) });
            break;

        case CODA_WM_MOVETABTONEWWINDOW:
        {
            const POINT point = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
            moveTabToNewWindow(hWnd, (int)wParam, &point);
            break;
        }

        case CODA_WM_MOVETABTOWINDOW:
        {
            if (WindowState* window = findWindow(hWnd))
                moveTabToWindow((int)wParam, *window, (int)lParam);
            break;
        }

        case WM_TIMER:
            if (wParam == TIMER_ID_DROP_OVERLAY)
            {
                if (WindowState* window = findWindow(hWnd))
                    checkDropOverlayStillNeeded(*window);
            }
            else
                return DefWindowProc(hWnd, message, wParam, lParam);
            break;

        case WM_DROPFILES:
        {
            // This is a drop on the window frame. A drop on the document view goes to the drop
            // target of the overlay window instead.
            HDROP drop = (HDROP)wParam;

            POINT dropPoint;
            if (!DragQueryPoint(drop, &dropPoint))
                dropPoint = { -1, -1 };

            handleDroppedFiles(hWnd, drop, dropPoint);
            DragFinish(drop);
            break;
        }

        case WM_ACTIVATE:
            // Hold the developer-tools hotkey only while this window is the
            // active one, so the key is not taken from other applications and
            // each window opens the developer tools of the document it shows.
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                UnregisterHotKey(hWnd, HOTKEY_ID_DEVTOOLS);
                hideHoverCard();
            }
            else
            {
                RegisterHotKey(hWnd, HOTKEY_ID_DEVTOOLS,
                               MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'I');
                // The tabbed window the user worked in last takes the next document opened.
                const WindowState* window = findWindow(hWnd);
                if (window && window->isTabbed)
                    tabbedWindow = hWnd;
            }
            break;

        case WM_HOTKEY:
            if (wParam == HOTKEY_ID_DEVTOOLS)
            {
                WindowState* window = findWindow(hWnd);
                DocumentTab* tab = window ? activeTabOf(*window) : nullptr;
                if (tab && tab->webView)
                    tab->webView->OpenDevToolsWindow();
            }
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
            break;
    }

    return 0;
}

// From https://stackoverflow.com/questions/51334674/how-to-detect-windows-10-light-dark-mode-in-win32-application

static bool isLightTheme()
{
    int value;
    DWORD cbData = 4;
    auto res = RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme",
        RRF_RT_REG_DWORD,
        NULL,
        &value,
        &cbData);

    if (res != ERROR_SUCCESS)
        return true;

    return value == 1;
}

static bool darkModeEnabled() { return Desktop::getDarkMode().value_or(!isLightTheme()); }

// These might not be defined for older Windows versions
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif

// Make the native (DWM-drawn) title bar of a window match the effective dark mode. The document UI
// inside the web view follows the same state through the darkTheme parameter of the cool.html URL.
static void applyTitleBarTheme(HWND hWnd)
{
    const BOOL darkMode = darkModeEnabled() ? TRUE : FALSE;
    HRESULT hr =
        DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
    if (!SUCCEEDED(hr))
    {
        LOG_DBG("DwmSetWindowAttribute() for a dark title bar failed: " << hr);
        return;
    }

    // The frame around the caption buttons of a window with the strip in its title bar has the
    // background colour of the strip, which is --color-background-dark in the light palette and
    // --color-background-tabs-group in the dark one. Windows 10 has no caption colour and keeps
    // its own.
    const WindowState* window = findWindow(hWnd);
    if (window && hasStripInTitleBar(*window))
    {
        const COLORREF captionColor = darkMode ? RGB(0x30, 0x30, 0x30) : RGB(0xe8, 0xe8, 0xe8);
        DwmSetWindowAttribute(hWnd, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));
    }

    // A window that is already on screen does not necessarily repaint its frame by itself, so ask
    // for that. Harmless for a window that is not visible yet (during WM_CREATE, say).
    SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

// Used when the dark mode changes for the whole application (the web UI asked for it with
// SETDARKMODE), as opposed to a platform theme change, which each window is told about separately
// with its own WM_SETTINGCHANGE.
static void applyTitleBarThemeToAllWindows()
{
    for (const auto& i : windows)
        applyTitleBarTheme(i.second.hWnd);
}

static HRESULT GetStreamForIFStream(std::ifstream& file, IStream** outStream)
{
    std::vector<char> data((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

    wil::com_ptr<IStream> stream;
    stream.attach(SHCreateMemStream(
        reinterpret_cast<const BYTE*>(data.data()),
        static_cast<UINT>(data.size())));
    if (!stream)
        return E_OUTOFMEMORY;
    *outStream = stream.detach();
    return S_OK;
}

static HRESULT webResourceRequestedHandler(ICoreWebView2Environment* env,
                                           ICoreWebView2* sender,
                                           ICoreWebView2WebResourceRequestedEventArgs* args)
{
    wil::com_ptr<ICoreWebView2WebResourceRequest> request;
    HRESULT hr;

    hr = args->get_Request(&request);
    if (!SUCCEEDED(hr))
    {
        LOG_ERR_S("get_Request() failed");
        return hr;
    }

    wil::unique_cotaskmem_string uri;
    hr = request->get_Uri(&uri);
    if (!SUCCEEDED(hr))
    {
        LOG_ERR_S("get_Uri() failed");
        return hr;
    }

    std::string uri2 = Uri::decode(Util::wide_string_to_string(uri.get()));
    Poco::URI requestUri(uri2);
    Poco::URI::QueryParameters params = requestUri.getQueryParameters();
    std::string wopiSrc, tag;

    const bool isMedia = requestUri.getPath() == "/media";
    const bool isVtt = requestUri.getPath() == "/mediavtt";

    if (!isMedia && !isVtt)
    {
        LOG_WRN_S("Unhandled path [" << requestUri.getPath() << ']');
        return E_FAIL;
    }

    for (const auto& it : params)
    {
        if (it.first == "WOPISrc")
            wopiSrc = it.second;
        else if (it.first == "Tag")
            tag = it.second;
    }

    if (tag.empty() || wopiSrc.empty())
    {
        LOG_ERR_S("Missing WOPISrc or Tag in ["
                  << uri2 << ']');
        return E_FAIL;
    }

    // For some reason for local documents the WOPISrc comes
    // here with a drive letter in the host position of the URI.
    // I.e. "file://C:/Users/foo/bar.ext" =>
    // "file:///C:/Users/foo/bar.ext"
    if (wopiSrc.size() > 10)
    {
        if (isalpha((unsigned char)wopiSrc[7]) &&
            wopiSrc[8] == ':')
            wopiSrc = "file:///" +
                std::string(1, wopiSrc[7]) +
                ":" +
                wopiSrc.substr(9);
    }

    std::shared_ptr<DocumentBroker> docBroker;
    const std::string docKey = RequestDetails::getDocKey(wopiSrc);
    {
        std::lock_guard<std::mutex> lock(DocBrokersMutex);
        const auto it = DocBrokers.find(docKey);
        if (it != DocBrokers.end())
            docBroker = it->second;
    }
    if (!docBroker)
    {
        LOG_ERR_S("No DocBroker for WOPISrc [" << wopiSrc << ']');
        return E_FAIL;
    }

    std::string mediaPath = docBroker->getEmbeddedMediaPath(tag);
    if (mediaPath.empty())
    {
        LOG_ERR_S("No media path for tag [" << tag << ']');
        return E_FAIL;
    }

    // Yes, the same code snippet once again. FIXME: Should
    // obviously factor this out into a utility function.
    if (mediaPath.length() > 4 && mediaPath[0] == '/' &&
        mediaPath[2] == ':' && mediaPath[3] == '/')
        mediaPath = mediaPath.substr(1);

    std::ifstream file;
    FileUtil::openFileToIFStream(mediaPath, file);
    if (!file.is_open())
    {
        LOG_ERR_S("Cannot open [" << mediaPath << "]");
        return E_FAIL;
    }

    const std::wstring mimeType = (isVtt ? L"text/vtt" : L"application/octet-stream");
    wil::com_ptr<IStream> contentStream;
    hr = GetStreamForIFStream(file, &contentStream);
    if (!SUCCEEDED(hr))
        return hr;

    wil::com_ptr<ICoreWebView2WebResourceResponse> response;
    hr = env->CreateWebResourceResponse(
        contentStream.get(),
        200, L"OK",
        (L"Content-Type: " + mimeType + L"\r\n" +
         L"Access-Control-Allow-Origin: *\r\n").c_str(),
        &response);
    if (!SUCCEEDED(hr))
    {
        LOG_ERR_S("CreateWebResourceResponse() failed");
        return hr;
    }

    hr = args->put_Response(response.get());
    if (!SUCCEEDED(hr))
    {
        LOG_ERR_S("put_Response() failed");
        return hr;
    }
    return S_OK;
}

// Register the ODF IFilter (odffilter.dll) with Windows Search and the
// FullDetails / PreviewDetails property strings Explorer uses, both under
// HKCU\Software\Classes. The two bindings the MSIX manifest cannot
// express declaratively are:
//
//   - the per-extension IFilter chain Windows Search walks:
//       .odt -> PersistentHandler ->
//       PersistentAddinsRegistered\{IID_IFilter} -> IFilter CLSID
//   - the per-extension FullDetails / PreviewDetails strings under
//       SystemFileAssociations\.<ext>, which control the field list shown
//       in Properties->Details and in the preview pane (the property
//       handler itself is wired via desktop2:DesktopPropertyHandler in
//       the manifest, but it only fills the fields Explorer asks for).
//
// HKCU\Software\Classes is silo'd for packaged processes - writes from
// this process would land in a per-package virtual class hive that
// Windows Search and Explorer never read. ProcMon confirmed this even
// with unvirtualizedResources and with a self-respawned breakaway child:
// the silo follows packaged-EXE images regardless. So instead we build a
// .reg file and shell out to System32\reg.exe (unpackaged, escapes the
// silo with the DesktopAppBreakaway attribute). reg.exe writes the
// values to real HKCU where the shell looks for them.
//
// Idempotent on every launch.
//
// Limitation: HKCU writes survive package uninstall. Users who remove
// Collabora Office may continue to see (silent) failed lookups for ODF
// IFilter until the keys are pruned.
static void registerOdfShellExtensions()
{
    // CLSIDs match engine/shell/source/win32/shlxthandler/odffilter/odffilter.hxx
    static constexpr wchar_t kPersistentHandlerClsid[] =
        L"{3EE9BB34-748E-4FBA-B6A5-94C200A11455}";
    static constexpr wchar_t kIFilterClsid[] =
        L"{DEB88601-6245-4803-81A5-13082BB738FF}";
    // Microsoft's well-known IID_IFilter
    static constexpr wchar_t kIidIFilter[] =
        L"{89BCB740-6119-101A-BCB7-00DD010655AF}";

    // Extensions we can actually filter - matches OOFileExtensionTable in
    // engine/shell/source/win32/shlxthandler/util/fileextensions.cxx
    static constexpr const wchar_t* kExtensions[] = {
        L".odt", L".ott", L".odm", L".oth",
        L".ods", L".ots",
        L".odg", L".otg",
        L".odp", L".otp",
        L".odf", L".odb",
        L".sxw", L".stw", L".sxg",
        L".sxc", L".stc",
        L".sxi", L".sti",
        L".sxd", L".std",
        L".sxm",
        // Flat ODF (single XML file, no zip container).
        L".fodt", L".fods", L".fodg", L".fodp",
    };

    static constexpr wchar_t kFullDetails[] =
        L"prop:System.PropGroup.Description;"
        L"System.Title;System.Author;System.Subject;"
        L"System.Keywords;System.Comment;"
        L"System.PropGroup.FileSystem;"
        L"System.ItemNameDisplay;System.ItemTypeText;"
        L"System.ItemFolderPathDisplay;System.Size;"
        L"System.DateCreated;System.DateModified;System.FileAttributes";

    static constexpr wchar_t kPreviewDetails[] =
        L"prop:System.Title;*System.Author;*System.Subject;*System.Comment";

    // Build the .reg file content.
    std::wstring reg = L"Windows Registry Editor Version 5.00\r\n\r\n";
    for (const wchar_t* ext : kExtensions)
    {
        reg += L"[HKEY_CURRENT_USER\\Software\\Classes\\";
        reg += ext;
        reg += L"\\PersistentHandler]\r\n@=\"";
        reg += kPersistentHandlerClsid;
        reg += L"\"\r\n\r\n";
    }
    reg += L"[HKEY_CURRENT_USER\\Software\\Classes\\CLSID\\";
    reg += kPersistentHandlerClsid;
    reg += L"]\r\n@=\"Collabora Office ODF IFilter Persistent Handler\"\r\n\r\n";
    reg += L"[HKEY_CURRENT_USER\\Software\\Classes\\CLSID\\";
    reg += kPersistentHandlerClsid;
    reg += L"\\PersistentAddinsRegistered\\";
    reg += kIidIFilter;
    reg += L"]\r\n@=\"";
    reg += kIFilterClsid;
    reg += L"\"\r\n\r\n";
    for (const wchar_t* ext : kExtensions)
    {
        reg += L"[HKEY_CURRENT_USER\\Software\\Classes\\SystemFileAssociations\\";
        reg += ext;
        reg += L"]\r\n\"FullDetails\"=\"";
        reg += kFullDetails;
        reg += L"\"\r\n\"PreviewDetails\"=\"";
        reg += kPreviewDetails;
        reg += L"\"\r\n\r\n";
    }

    // Write the .reg file under %LocalAppData%\<appName> with the UTF-16 LE
    // BOM that reg.exe import expects.
    PWSTR appDataFolder = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appDataFolder)))
        return;
    std::wstring regFilePath = appDataFolder;
    CoTaskMemFree(appDataFolder);
    regFilePath += L"\\";
    regFilePath += appName;
    CreateDirectoryW(regFilePath.c_str(), nullptr);
    regFilePath += L"\\register-shellext.reg";

    HANDLE hFile = CreateFileW(regFilePath.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return;
    const WORD bom = 0xFEFF;
    DWORD written = 0;
    WriteFile(hFile, &bom, sizeof(bom), &written, nullptr);
    WriteFile(hFile, reg.data(),
              static_cast<DWORD>(reg.size() * sizeof(wchar_t)),
              &written, nullptr);
    CloseHandle(hFile);

    // Spawn System32\reg.exe import outside the silo via the
    // DesktopAppBreakaway process attribute. The breakaway attribute does
    // not let our own (packaged) EXE escape, but reg.exe is unpackaged so
    // it goes to a plain desktop process whose writes hit real HKCU.
    wchar_t systemDir[MAX_PATH];
    if (GetSystemDirectoryW(systemDir, ARRAYSIZE(systemDir)) == 0)
        return;
    std::wstring cmdLine = L"\"";
    cmdLine += systemDir;
    cmdLine += L"\\reg.exe\" import \"";
    cmdLine += regFilePath;
    cmdLine += L"\"";

    SIZE_T attrSize = 0;
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrSize);
    if (attrSize == 0)
        return;
    std::vector<BYTE> attrBuffer(attrSize);
    LPPROC_THREAD_ATTRIBUTE_LIST attrList =
        reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrBuffer.data());
    if (!InitializeProcThreadAttributeList(attrList, 1, 0, &attrSize))
        return;
    DWORD policy = PROCESS_CREATION_DESKTOP_APP_BREAKAWAY_ENABLE_PROCESS_TREE;
    if (!UpdateProcThreadAttribute(attrList, 0,
                                   PROC_THREAD_ATTRIBUTE_DESKTOP_APP_POLICY,
                                   &policy, sizeof(policy), NULL, NULL))
    {
        DeleteProcThreadAttributeList(attrList);
        return;
    }

    // Capture reg.exe's stderr/stdout via a pipe so a failure message can
    // be surfaced verbatim (e.g. "ERROR: Access is denied.") instead of
    // just the numeric exit code.
    SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
    HANDLE hReadPipe = nullptr;
    HANDLE hWritePipe = nullptr;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 65536))
    {
        DeleteProcThreadAttributeList(attrList);
        return;
    }
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOEXW si = {};
    si.StartupInfo.cb = sizeof(si);
    si.StartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.StartupInfo.wShowWindow = SW_HIDE;
    si.StartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.StartupInfo.hStdOutput = hWritePipe;
    si.StartupInfo.hStdError = hWritePipe;
    si.lpAttributeList = attrList;

    PROCESS_INFORMATION pi = {};
    BOOL ok = CreateProcessW(NULL, cmdLine.data(), NULL, NULL, TRUE,
                             EXTENDED_STARTUPINFO_PRESENT | CREATE_NO_WINDOW,
                             NULL, NULL,
                             reinterpret_cast<LPSTARTUPINFOW>(&si), &pi);
    DeleteProcThreadAttributeList(attrList);
    // Parent must close its copy of the write end so the read end sees EOF
    // when reg.exe exits.
    CloseHandle(hWritePipe);
    if (!ok)
    {
        CloseHandle(hReadPipe);
        return;
    }

    WaitForSingleObject(pi.hProcess, 5000);

    std::string output;
    char buf[1024];
    DWORD nRead = 0;
    while (ReadFile(hReadPipe, buf, sizeof(buf), &nRead, nullptr) && nRead > 0)
        output.append(buf, nRead);
    CloseHandle(hReadPipe);

    DWORD exitCode = STILL_ACTIVE;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    if (exitCode != 0)
    {
        while (!output.empty() && (output.back() == '\r' || output.back() == '\n'
                                   || output.back() == ' ' || output.back() == '\t'))
            output.pop_back();
        // reg.exe writes in the user's OEM (console) codepage; widen for
        // OutputDebugStringW. Log:: is not initialized yet at this point
        // in wWinMain. DebugView (Sysinternals) captures the output.
        int wlen = MultiByteToWideChar(CP_OEMCP, 0,
                                       output.c_str(), static_cast<int>(output.size()),
                                       nullptr, 0);
        std::wstring wide(wlen, L'\0');
        MultiByteToWideChar(CP_OEMCP, 0,
                            output.c_str(), static_cast<int>(output.size()),
                            wide.data(), wlen);
        std::wstring msg = L"registerOdfShellExtensions: reg.exe failed: ";
        msg += wide.empty() ? L"(no diagnostic on stderr)" : wide;
        msg += L"\n";
        OutputDebugStringW(msg.c_str());
    }
    DeleteFileW(regFilePath.c_str());
}

// The one WebView2 environment every web view in the app is created from, and
// the callbacks that asked for it while it was still being made.
static wil::com_ptr<ICoreWebView2Environment> webViewEnvironment;
static std::vector<std::function<void(ICoreWebView2Environment*)>> webViewEnvironmentWaiters;
static bool webViewEnvironmentUnderway = false;

// Hand the shared environment to the callback, creating it on the first call.
// Creation is asynchronous, so callers that arrive in the meantime wait and are
// served in order once it is there.
static void withWebView2Environment(std::function<void(ICoreWebView2Environment*)> callback)
{
    if (webViewEnvironment)
    {
        callback(webViewEnvironment.get());
        return;
    }

    webViewEnvironmentWaiters.push_back(std::move(callback));

    if (webViewEnvironmentUnderway)
        return;
    webViewEnvironmentUnderway = true;

    // Configure the "cool" custom scheme registration
    auto schemeRegistration =
        Microsoft::WRL::Make<CoreWebView2CustomSchemeRegistration>(L"cool");
    if (!SUCCEEDED(schemeRegistration->put_TreatAsSecure(TRUE)))
        fatal("schemeRegistration->put_TreatAsSecure() failed");
    if (!SUCCEEDED(schemeRegistration->put_HasAuthorityComponent(TRUE)))
        fatal("schemeRegistration->put_HasAuthorityComponent() failed");

    // We show a page from a file: URI, so we need to use "*"
    LPCWSTR allowedOrigins[1] = { L"*" };
    if (!SUCCEEDED(schemeRegistration->SetAllowedOrigins(1, allowedOrigins)))
        fatal("schemeRegistration->SetAllowedOrigins() failed");

    // Add the registration to the options (requires ICoreWebView2EnvironmentOptions4)
    ICoreWebView2CustomSchemeRegistration* registrations[1] =
        { schemeRegistration.Get() };

    // Required for instantiating new Web Workers, which otherwise fail with a
    // cross-origin SecurityError because file:// gets origin 'null'.
    std::wstring additionalArgs = L"--allow-file-access-from-files";
    if (enableWebDriver)
        additionalArgs += L" --remote-debugging-port=9222";

    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    options->put_AdditionalBrowserArguments(additionalArgs.c_str());

    Microsoft::WRL::ComPtr<ICoreWebView2EnvironmentOptions4> options4;
    if (!SUCCEEDED(options.As(&options4)))
        fatal("options.As() failed");
    if (!SUCCEEDED(options4->SetCustomSchemeRegistrations(1, registrations)))
        fatal("options4->SetCustomSchemeRegistrations() failed");

    CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        (Util::string_to_wide_string(localAppData) + L"\\UDF").c_str(),
        options.Get(),
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
            {
                if (!env)
                    fatal("CreateCoreWebView2EnvironmentWithOptions() gave us no environment");

                webViewEnvironment = env;

                const auto waiters = std::move(webViewEnvironmentWaiters);
                webViewEnvironmentWaiters.clear();
                for (const auto& waiter : waiters)
                    waiter(env);

                return S_OK;
            })
            .Get());
}

// The document kind the tab strip keys its icon and colour off. The engine
// decides what a file really is when it opens it; the strip needs a name for it
// before that, and the extension is what there is to go on.
static std::string docTypeFromFilename(const std::string& filename)
{
    const auto lastPeriod = filename.find_last_of('.');
    if (lastPeriod == std::string::npos)
        return "other";

    std::string extension = filename.substr(lastPeriod + 1);
    for (char& c : extension)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    static const std::set<std::string> writer = { "odt", "doc", "docx", "docm", "rtf", "txt", "fodt" };
    static const std::set<std::string> calc = { "ods", "xls", "xlsx", "xlsm", "csv", "fods" };
    static const std::set<std::string> impress = { "odp", "ppt", "pptx", "pptm", "ppsx", "fodp" };
    static const std::set<std::string> draw = { "odg", "fodg" };

    if (writer.count(extension))
        return "writer";
    if (calc.count(extension))
        return "calc";
    if (impress.count(extension))
        return "impress";
    if (draw.count(extension))
        return "draw";
    return "other";
}

// The window takes its title from the document it shows.
static std::wstring windowTitleForTab(const DocumentTab& tab)
{
    if (tab.filenameAndUri.filename.empty())
        return Util::string_to_wide_string(APP_NAME);
    return Util::string_to_wide_string(tab.filenameAndUri.filename + " - " APP_NAME);
}

// Give the window one more document, at the end of its tab order. The caller
// connects it to the server and gives it a document id, which the backstage and
// the presenter console, showing no document of their own, do without.
static DocumentTab& createTab(WindowState& window, const FilenameAndUri& filenameAndUri,
                              DocumentMode mode)
{
    static int nextTabId = 1;

    const int tabId = nextTabId++;

    DocumentTab& tab = documentTabs[tabId];
    tab.tabId = tabId;
    tab.hWnd = window.hWnd;
    tab.filenameAndUri = filenameAndUri;
    tab.mode = mode;
    if (mode == DocumentMode::STARTER)
        tab.docType = "starter";
    else if (mode == DocumentMode::WELCOME)
        tab.docType = "welcome";
    else
        tab.docType = docTypeFromFilename(filenameAndUri.filename);

    window.tabIds.push_back(tabId);

    return tab;
}

// How tall the tab strip is at the standard resolution, in the same units the
// strip page lays itself out in.
static const int TAB_STRIP_HEIGHT = 36;

static int stripHeight(const WindowState& window)
{
    if (!window.stripIsVisible)
        return 0;
    return MulDiv(TAB_STRIP_HEIGHT, GetDpiForWindow(window.hWnd), USER_DEFAULT_SCREEN_DPI);
}

// A tabbed window shows its tab strip where the title bar would be. The client area covers the
// title bar, and DWM extends its frame down over the strip, so the system still draws the minimize,
// maximize and close buttons there. A window in full screen has no frame at all.
static bool hasStripInTitleBar(const WindowState& window)
{
    return window.isTabbed &&
           (GetWindowLongW(window.hWnd, GWL_STYLE) & WS_CAPTION) == WS_CAPTION;
}

// The height of the resize border along the top of a window. A window with the strip in its title
// bar has that border inside its client area.
static int topResizeBorderHeight(HWND hWnd)
{
    const UINT dpi = GetDpiForWindow(hWnd);
    return GetSystemMetricsForDpi(SM_CYFRAME, dpi) +
           GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
}

// Where the strip starts from the top of the client area. A strip in the title bar leaves the top
// resize border free, except in a maximized window, which has that border above the screen.
static int stripTop(const WindowState& window)
{
    if (!hasStripInTitleBar(window) || IsZoomed(window.hWnd))
        return 0;
    return topResizeBorderHeight(window.hWnd);
}

// The height of the frame that DWM extends into the client area, and that the caption buttons are
// in, or 0 for a window that keeps its own title bar.
static int titleBarFrameHeight(const WindowState& window)
{
    if (!hasStripInTitleBar(window))
        return 0;
    return stripTop(window) + MulDiv(TAB_STRIP_HEIGHT, GetDpiForWindow(window.hWnd),
                                     USER_DEFAULT_SCREEN_DPI);
}

// Where the caption buttons start in client coordinates, or the end of the client area for a
// window that keeps its own title bar. The buttons are at the end of the title bar, which is its
// left end in a mirrored window, and the client coordinates of a mirrored window run from the
// right. So in both cases the buttons are at the high end of the client coordinates.
static LONG captionButtonsLeft(const WindowState& window)
{
    RECT client;
    GetClientRect(window.hWnd, &client);

    RECT buttons;
    if (!hasStripInTitleBar(window) ||
        FAILED(DwmGetWindowAttribute(window.hWnd, DWMWA_CAPTION_BUTTON_BOUNDS, &buttons,
                                     sizeof(buttons))) ||
        buttons.right <= buttons.left)
        return client.right;

    // The bounds are relative to the window, so the space between the buttons and the nearer edge
    // of the window, less the border at the side of the client area, is what the client area has
    // beyond the buttons.
    RECT windowRect;
    GetWindowRect(window.hWnd, &windowRect);
    const LONG windowWidth = windowRect.right - windowRect.left;
    const LONG border = (windowWidth - (client.right - client.left)) / 2;
    const LONG beyond = std::max(0L, std::min(buttons.left, windowWidth - buttons.right) - border);
    return std::clamp(client.right - beyond - (buttons.right - buttons.left), client.left,
                      client.right);
}

// A minimized window has no client area at all, so both of these keep the
// bottom edge at or below the top one: a web view rejects a rectangle that is
// the wrong way round.
static RECT stripBounds(const WindowState& window)
{
    RECT bounds;
    GetClientRect(window.hWnd, &bounds);
    bounds.top = std::min(bounds.bottom, bounds.top + stripTop(window));
    bounds.bottom = std::min(bounds.bottom, bounds.top + stripHeight(window));
    bounds.right = captionButtonsLeft(window);
    return bounds;
}

static RECT documentArea(const WindowState& window)
{
    RECT bounds;
    GetClientRect(window.hWnd, &bounds);
    bounds.top = std::min(bounds.bottom, bounds.top + stripTop(window) + stripHeight(window));
    return bounds;
}

// DWM draws its frame, with the caption buttons, in the top of the client area of a window with the
// strip in its title bar.
static void updateTitleBarFrame(const WindowState& window)
{
    MARGINS margins = { 0, 0, titleBarFrameHeight(window), 0 };
    DwmExtendFrameIntoClientArea(window.hWnd, &margins);
}

// A point that the strip page gives in its own CSS pixels from the top left corner of the strip,
// in screen coordinates. The page itself is never mirrored, so its left is the left of the screen
// also in a mirrored window.
static POINT stripPointToScreen(const WindowState& window, double x, double y)
{
    // With exactly two points, MapWindowPoints() keeps left less than right also when it maps out
    // of a mirrored window.
    RECT strip = stripBounds(window);
    MapWindowPoints(window.hWnd, HWND_DESKTOP, reinterpret_cast<POINT*>(&strip), 2);

    // The strip does not zoom, so the window's DPI is all that tells the two units apart.
    const UINT dpi = GetDpiForWindow(window.hWnd);
    return { strip.left + MulDiv(static_cast<int>(std::lround(x)), dpi, USER_DEFAULT_SCREEN_DPI),
             strip.top + MulDiv(static_cast<int>(std::lround(y)), dpi, USER_DEFAULT_SCREEN_DPI) };
}

static void layoutTabs(WindowState& window)
{
    updateTitleBarFrame(window);

    if (window.stripController)
        window.stripController->put_Bounds(stripBounds(window));

    // Every document keeps the right size, not just the one on show, so a
    // switch between tabs never lands on a stale layout.
    const RECT bounds = documentArea(window);
    for (int tabId : window.tabIds)
    {
        DocumentTab* tab = findTab(tabId);
        if (tab && tab->webViewController)
            tab->webViewController->put_Bounds(bounds);
    }
}

// Fresh strips start in the saved dark mode choice, or in the system theme.
static std::string currentStripTheme()
{
    return darkModeEnabled() ? "dark" : "light";
}

// The state the tab strip draws, one entry per document in tab order.
static std::string currentTabsJson(const WindowState& window)
{
    Poco::JSON::Array tabs;
    for (int tabId : window.tabIds)
    {
        const DocumentTab* tab = findTab(tabId);
        if (!tab)
            continue;

        Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
        entry->set("id", tab->tabId);
        entry->set("title", tab->filenameAndUri.filename);
        // The full path of the document file. The starter backstage and the welcome slideshow
        // have none.
        if (tab->mode != DocumentMode::STARTER && tab->mode != DocumentMode::WELCOME)
            entry->set("path", fileUriToWindowsPath(tab->filenameAndUri.uri));
        entry->set("docType", tab->docType);
        entry->set("modified", tab->isModified);
        entry->set("active", tab->tabId == window.activeTabId);
        entry->set("hasThumbnail", !tab->thumbnailPng.empty());
        tabs.add(entry);
    }

    std::ostringstream oss;
    tabs.stringify(oss);
    return oss.str();
}

// The strip page reads a notification as {signal, args}, the mirror image of the
// {call, args} it posts back.
static void notifyStrip(WindowState& window, const std::string& signal,
                        const std::string& argument)
{
    if (!window.stripWebView)
        return;

    Poco::JSON::Array::Ptr args = new Poco::JSON::Array();
    args->add(argument);

    Poco::JSON::Object::Ptr envelope = new Poco::JSON::Object();
    envelope->set("signal", signal);
    envelope->set("args", args);

    std::ostringstream oss;
    envelope->stringify(oss);

    window.stripWebView->PostWebMessageAsJson(
        Util::string_to_wide_string(oss.str()).c_str());
}

static void updateStripVisibility(WindowState& window)
{
    // The strip is to take the place of the title bar, so a tabbed window shows it also with a
    // single document. A presentation wants the whole monitor.
    const bool visible = window.isTabbed && !window.tabIds.empty() && !window.isFullScreen;
    if (visible == window.stripIsVisible)
        return;

    window.stripIsVisible = visible;
    if (!visible)
        hideHoverCard();
    if (window.stripController)
        window.stripController->put_IsVisible(visible && window.stripHasContent);
    layoutTabs(window);
}

static void pushTabsToStrip(WindowState& window)
{
    updateStripVisibility(window);
    notifyStrip(window, "tabsChanged", currentTabsJson(window));
}

// The strip page handles no keys, so the keyboard belongs to the document even
// when the click that got us here landed on a tab.
static void focusActiveDocument(const WindowState& window)
{
    DocumentTab* tab = activeTabOf(window);
    if (tab && tab->webViewController && tab->hasContent)
        tab->webViewController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
}

// How long the pointer has to rest on a tab before its hover card shows, in milliseconds.
static const UINT HOVER_CARD_DELAY = 500;
// The timer of the hover card window that holds that delay.
static const int TIMER_ID_HOVER_CARD = 1;
// The frame of the image in the hover card, at the standard resolution.
static const int HOVER_CARD_IMAGE_WIDTH = 218;
static const int HOVER_CARD_IMAGE_HEIGHT = 170;

// The card with the name and the image of a background document, below the tab that the pointer
// rests on. One card serves every window, because the pointer rests on one tab at a time.
struct HoverCard
{
    HWND hWnd = 0;
    // The tab the card is for, or 0 when the pointer rests on no tab.
    int tabId = 0;
    // The bottom corner of that tab where the tab starts, in screen coordinates.
    POINT position = {};
    // The tabs run from right to left, so the card lines up with the right edge of its tab.
    bool rightToLeft = false;
    // The colours of the card, from the theme of the strip.
    COLORREF background = RGB(0xf8, 0xf9, 0xfa);
    COLORREF text = RGB(0x33, 0x33, 0x33);
    COLORREF border = RGB(0x94, 0x94, 0x94);
    // What the card shows, laid out in the client area of the card.
    std::wstring title;
    HFONT font = nullptr;
    RECT titleRect = {};
    HBITMAP image = nullptr;
    RECT imageRect = {};
};

static HoverCard hoverCard;

// A colour from the strip page, which gives it as a CSS colour string such as "#f8f9fa" or
// "rgb(248, 249, 250)". Any other form gives the fallback.
static COLORREF cssColor(const std::string& css, COLORREF fallback)
{
    const std::size_t first = css.find_first_not_of(" \t");
    if (first == std::string::npos)
        return fallback;
    const std::string value = css.substr(first, css.find_last_not_of(" \t") - first + 1);

    if (value[0] == '#')
    {
        const std::string hex = value.substr(1);
        if (hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
            return fallback;
        // The short forms have one digit for each channel, and an alpha channel is ignored.
        if (hex.size() == 3 || hex.size() == 4)
            return RGB(std::stoi(hex.substr(0, 1), nullptr, 16) * 17,
                       std::stoi(hex.substr(1, 1), nullptr, 16) * 17,
                       std::stoi(hex.substr(2, 1), nullptr, 16) * 17);
        if (hex.size() == 6 || hex.size() == 8)
            return RGB(std::stoi(hex.substr(0, 2), nullptr, 16),
                       std::stoi(hex.substr(2, 2), nullptr, 16),
                       std::stoi(hex.substr(4, 2), nullptr, 16));
        return fallback;
    }

    if (value.starts_with("rgb(") || value.starts_with("rgba("))
    {
        const char* p = value.c_str() + value.find('(') + 1;
        int channels[3];
        for (int& channel : channels)
        {
            char* end;
            const long number = std::strtol(p, &end, 10);
            if (end == p || number < 0 || number > 255)
                return fallback;
            channel = static_cast<int>(number);
            p = end + std::strspn(end, " ,");
        }
        return RGB(channels[0], channels[1], channels[2]);
    }

    return fallback;
}

// A PNG image fitted inside a box of the given size in pixels, keeping its shape, and drawn over
// the background colour where it is transparent. The size of the bitmap goes to size. There is no
// bitmap when the PNG cannot be read.
static HBITMAP fittedImage(const std::string& png, int boxWidth, int boxHeight,
                           COLORREF background, SIZE& size)
{
    wil::com_ptr<IWICImagingFactory> factory;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(factory.put()))))
        return nullptr;

    wil::com_ptr<IWICStream> stream;
    if (FAILED(factory->CreateStream(&stream)) ||
        FAILED(stream->InitializeFromMemory(
            reinterpret_cast<BYTE*>(const_cast<char*>(png.data())),
            static_cast<DWORD>(png.size()))))
        return nullptr;

    wil::com_ptr<IWICBitmapDecoder> decoder;
    wil::com_ptr<IWICBitmapFrameDecode> frame;
    UINT width, height;
    if (FAILED(factory->CreateDecoderFromStream(stream.get(), nullptr,
                                                WICDecodeMetadataCacheOnDemand, &decoder)) ||
        FAILED(decoder->GetFrame(0, &frame)) || FAILED(frame->GetSize(&width, &height)) ||
        width == 0 || height == 0)
        return nullptr;

    const double scale = std::min(static_cast<double>(boxWidth) / width,
                                  static_cast<double>(boxHeight) / height);
    const UINT fittedWidth = std::max(1U, static_cast<UINT>(std::lround(width * scale)));
    const UINT fittedHeight = std::max(1U, static_cast<UINT>(std::lround(height * scale)));

    // The pixels come out with the colour already multiplied by the alpha, so the background
    // only has to be added in proportion to what the image leaves uncovered.
    wil::com_ptr<IWICBitmapScaler> scaler;
    wil::com_ptr<IWICFormatConverter> converter;
    if (FAILED(factory->CreateBitmapScaler(&scaler)) ||
        FAILED(scaler->Initialize(frame.get(), fittedWidth, fittedHeight,
                                  WICBitmapInterpolationModeFant)) ||
        FAILED(factory->CreateFormatConverter(&converter)) ||
        FAILED(converter->Initialize(scaler.get(), GUID_WICPixelFormat32bppPBGRA,
                                     WICBitmapDitherTypeNone, nullptr, 0.0,
                                     WICBitmapPaletteTypeCustom)))
        return nullptr;

    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = static_cast<LONG>(fittedWidth);
    // A negative height makes the rows run from the top down, as WIC gives them.
    info.bmiHeader.biHeight = -static_cast<LONG>(fittedHeight);
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap)
        return nullptr;

    const UINT stride = fittedWidth * 4;
    if (FAILED(converter->CopyPixels(nullptr, stride, stride * fittedHeight,
                                     static_cast<BYTE*>(bits))))
    {
        DeleteObject(bitmap);
        return nullptr;
    }

    BYTE* pixel = static_cast<BYTE*>(bits);
    for (UINT i = 0; i < fittedWidth * fittedHeight; i++, pixel += 4)
    {
        const int uncovered = 255 - pixel[3];
        pixel[0] = static_cast<BYTE>(pixel[0] + (GetBValue(background) * uncovered + 127) / 255);
        pixel[1] = static_cast<BYTE>(pixel[1] + (GetGValue(background) * uncovered + 127) / 255);
        pixel[2] = static_cast<BYTE>(pixel[2] + (GetRValue(background) * uncovered + 127) / 255);
        pixel[3] = 255;
    }

    size = { static_cast<LONG>(fittedWidth), static_cast<LONG>(fittedHeight) };
    return bitmap;
}

static void hideHoverCard()
{
    hoverCard.tabId = 0;
    if (hoverCard.hWnd)
    {
        KillTimer(hoverCard.hWnd, TIMER_ID_HOVER_CARD);
        ShowWindow(hoverCard.hWnd, SW_HIDE);
    }
}

// Show the card for the tab that the pointer rests on, with the image that its document sent
// last. The active tab shows the document itself, so it gets no card.
static void showHoverCard()
{
    DocumentTab* tab = findTab(hoverCard.tabId);
    WindowState* window = tab ? findWindow(tab->hWnd) : nullptr;
    if (!hoverCard.hWnd || !window || tab->thumbnailPng.empty() ||
        window->activeTabId == tab->tabId)
    {
        if (hoverCard.hWnd)
            ShowWindow(hoverCard.hWnd, SW_HIDE);
        return;
    }

    const UINT dpi = GetDpiForWindow(window->hWnd);
    const auto scaled = [dpi](int value) { return MulDiv(value, dpi, USER_DEFAULT_SCREEN_DPI); };

    if (hoverCard.font)
        DeleteObject(hoverCard.font);
    NONCLIENTMETRICSW metrics = { sizeof(NONCLIENTMETRICSW) };
    hoverCard.font = SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics),
                                                &metrics, 0, dpi)
                         ? CreateFontIndirectW(&metrics.lfStatusFont)
                         : nullptr;

    int titleHeight = scaled(16);
    if (HDC dc = GetDC(nullptr))
    {
        const HGDIOBJ oldFont = SelectObject(dc, hoverCard.font ? hoverCard.font
                                                                : GetStockObject(DEFAULT_GUI_FONT));
        TEXTMETRICW textMetrics;
        if (GetTextMetricsW(dc, &textMetrics))
            titleHeight = textMetrics.tmHeight;
        SelectObject(dc, oldFont);
        ReleaseDC(nullptr, dc);
    }

    // The title above the image, with margins of 8 at the sides and the bottom, 6 at the top and
    // 6 between the two. Every card has the same size, and an image of another shape is fitted
    // inside the frame and centred.
    const int imageWidth = scaled(HOVER_CARD_IMAGE_WIDTH);
    const int imageHeight = scaled(HOVER_CARD_IMAGE_HEIGHT);
    hoverCard.titleRect = { scaled(8), scaled(6), scaled(8) + imageWidth,
                            scaled(6) + titleHeight };
    const RECT imageFrame = { scaled(8), hoverCard.titleRect.bottom + scaled(6),
                              scaled(8) + imageWidth,
                              hoverCard.titleRect.bottom + scaled(6) + imageHeight };
    const int cardWidth = imageFrame.right + scaled(8);
    const int cardHeight = imageFrame.bottom + scaled(8);

    if (hoverCard.image)
        DeleteObject(hoverCard.image);
    SIZE size = {};
    hoverCard.image =
        fittedImage(tab->thumbnailPng, imageWidth, imageHeight, hoverCard.background, size);
    const int left = imageFrame.left + (imageWidth - size.cx) / 2;
    const int top = imageFrame.top + (imageHeight - size.cy) / 2;
    hoverCard.imageRect = { left, top, left + size.cx, top + size.cy };

    // The tooltip of a tab shows the full path of its document, so the card does too.
    const std::string path = fileUriToWindowsPath(tab->filenameAndUri.uri);
    hoverCard.title =
        Util::string_to_wide_string(path.empty() ? tab->filenameAndUri.filename : path);
    std::wstring accessibleName = translate("TabManager", "Preview of %1");
    const std::size_t placeholder = accessibleName.find(L"%1");
    if (placeholder != std::wstring::npos)
        accessibleName.replace(placeholder, 2, hoverCard.title);
    SetWindowTextW(hoverCard.hWnd, accessibleName.c_str());

    // The card stays on the monitor of the tab, even for a tab near its edge.
    POINT position = hoverCard.position;
    if (hoverCard.rightToLeft)
        position.x -= cardWidth;
    MONITORINFO monitor = { sizeof(MONITORINFO) };
    if (GetMonitorInfoW(MonitorFromPoint(position, MONITOR_DEFAULTTONEAREST), &monitor))
        position.x = std::max(monitor.rcWork.left,
                              std::min(position.x, monitor.rcWork.right - cardWidth));

    SetWindowPos(hoverCard.hWnd, HWND_TOPMOST, position.x, position.y, cardWidth, cardHeight,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(hoverCard.hWnd, nullptr, TRUE);
}

// The pointer has come to rest on a tab of the window. The point is the bottom corner of the tab
// where the tab starts, in the CSS pixels of the strip, which is its right corner when the tabs run
// from right to left. The colours are those of the theme of the strip.
static void startHoverCard(const WindowState& window, int tabId, POINT point,
                           const std::string& background, const std::string& text,
                           const std::string& border, bool rightToLeft)
{
    if (!hoverCard.hWnd)
        hoverCard.hWnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TOPMOST,
                                         hoverCardWindowClass, L"", WS_POPUP, 0, 0, 0, 0,
                                         nullptr, nullptr, appInstance, nullptr);
    if (!hoverCard.hWnd)
        return;

    hoverCard.tabId = tabId;
    hoverCard.position = stripPointToScreen(window, point.x, point.y);
    hoverCard.rightToLeft = rightToLeft;
    hoverCard.background = cssColor(background, RGB(0xf8, 0xf9, 0xfa));
    hoverCard.text = cssColor(text, RGB(0x33, 0x33, 0x33));
    hoverCard.border = cssColor(border, RGB(0x94, 0x94, 0x94));

    // Moving from one tab to the next while a card is up swaps the card at once.
    if (IsWindowVisible(hoverCard.hWnd))
        showHoverCard();
    else
        SetTimer(hoverCard.hWnd, TIMER_ID_HOVER_CARD, HOVER_CARD_DELAY, nullptr);
}

// The card only shows what the pointer rests on, so it takes neither the focus nor the mouse.
static LRESULT CALLBACK HoverCardWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_TIMER:
            if (wParam != TIMER_ID_HOVER_CARD)
                break;
            KillTimer(hWnd, TIMER_ID_HOVER_CARD);
            showHoverCard();
            return 0;

        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;

        case WM_NCHITTEST:
            return HTTRANSPARENT;

        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC dc = BeginPaint(hWnd, &paint);

            RECT client;
            GetClientRect(hWnd, &client);
            HBRUSH backgroundBrush = CreateSolidBrush(hoverCard.background);
            FillRect(dc, &client, backgroundBrush);
            DeleteObject(backgroundBrush);
            HBRUSH borderBrush = CreateSolidBrush(hoverCard.border);
            FrameRect(dc, &client, borderBrush);
            DeleteObject(borderBrush);

            const HGDIOBJ font =
                hoverCard.font ? hoverCard.font : GetStockObject(DEFAULT_GUI_FONT);
            const HGDIOBJ oldFont = SelectObject(dc, font);
            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, hoverCard.text);
            RECT titleRect = hoverCard.titleRect;
            DrawTextW(dc, hoverCard.title.c_str(), -1, &titleRect,
                      DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_PATH_ELLIPSIS);
            SelectObject(dc, oldFont);

            if (hoverCard.image)
            {
                if (HDC imageDC = CreateCompatibleDC(dc))
                {
                    const HGDIOBJ oldBitmap = SelectObject(imageDC, hoverCard.image);
                    BitBlt(dc, hoverCard.imageRect.left, hoverCard.imageRect.top,
                           hoverCard.imageRect.right - hoverCard.imageRect.left,
                           hoverCard.imageRect.bottom - hoverCard.imageRect.top, imageDC, 0, 0,
                           SRCCOPY);
                    SelectObject(imageDC, oldBitmap);
                    DeleteDC(imageDC);
                }
            }

            EndPaint(hWnd, &paint);
            return 0;
        }
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

// Bring one document of the window to the front and leave the rest behind it.
static void activateTab(WindowState& window, int tabId)
{
    DocumentTab* tab = findTab(tabId);
    if (!tab)
        return;

    hideHoverCard();

    // The tab the user leaves sends an image of what it shows, for the card of its tab.
    DocumentTab* previous = activeTabOf(window);
    if (previous && previous != tab && previous->webView && previous->hasContent)
        previous->webView->ExecuteScript(
            L"if (window.app && window.app.map && window.app.map.requestTabThumbnail)"
            L"  window.app.map.requestTabThumbnail();",
            Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                [](HRESULT, LPCWSTR) -> HRESULT { return S_OK; })
                .Get());

    window.activeTabId = tabId;

    for (int otherId : window.tabIds)
    {
        DocumentTab* other = findTab(otherId);
        if (other && other->webViewController)
            other->webViewController->put_IsVisible(otherId == tabId && other->hasContent);
    }

    focusActiveDocument(window);

    SetWindowTextW(window.hWnd, windowTitleForTab(*tab).c_str());

    pushTabsToStrip(window);
}

static void removeTabFromWindow(WindowState& window, int tabId);

static void closeTab(WindowState& window, int tabId)
{
    DocumentTab* tab = findTab(tabId);
    if (!tab)
        return;

    if (hoverCard.tabId == tabId)
        hideHoverCard();

    if (tab->isConsole)
    {
        // The presentation ends with its console: the document comes out of
        // full screen and forgets the window it was driving.
        DocumentTab* presenting = findTab(tab->presentingTabId);
        if (presenting)
        {
            WindowState* presentingWindow = findWindow(presenting->hWnd);
            if (presentingWindow)
                leave_full_screen(*presentingWindow);
            presenting->hConsoleWnd = 0;
        }
    }
    else if (tab->mode != DocumentMode::STARTER)
    {
        // A presentation ends with the document driving it: the console window
        // goes, and the window this document was playing in comes back out of
        // full screen with its tab strip.
        if (tab->hConsoleWnd)
        {
            PostMessageW(tab->hConsoleWnd, WM_CLOSE, 0, 0);
            tab->hConsoleWnd = 0;
        }
        if (window.isPresFullScreen)
        {
            window.isPresFullScreen = false;
            leave_full_screen(window);
        }

        // A document whose page never said hello has no forwarding thread to
        // wake and no socket of its own to close yet.
        if (tab->app2js.joinable())
        {
            do_bye_handling_things(*tab);

            // The goodbye closed the thread's end of the socket, so the thread
            // is on its way out and joins straight away.
            tab->app2js.join();
        }

        DocumentData::deallocate(tab->appDocId);
    }

    if (tab->webViewController)
    {
        tab->webViewController->Close();
        tab->webViewController = nullptr;
    }
    tab->webView = nullptr;

    documentTabs.erase(tabId);

    removeTabFromWindow(window, tabId);
}

// Take a document out of the tab order of its window. If it was the one on show, the one that
// moves up into its place, or the last one, comes to the front instead.
static void removeTabFromWindow(WindowState& window, int tabId)
{
    const auto position = std::find(window.tabIds.begin(), window.tabIds.end(), tabId);
    size_t index = 0;
    if (position != window.tabIds.end())
    {
        index = static_cast<size_t>(std::distance(window.tabIds.begin(), position));
        window.tabIds.erase(position);
    }

    if (window.tabIds.empty())
    {
        window.activeTabId = 0;
        return;
    }

    if (window.activeTabId == tabId)
    {
        activateTab(window, window.tabIds[std::min(index, window.tabIds.size() - 1)]);
    }
    else
    {
        pushTabsToStrip(window);
        focusActiveDocument(window);
    }
}

static void reorderTab(WindowState& window, int fromIndex, int toIndex)
{
    const int count = static_cast<int>(window.tabIds.size());
    if (fromIndex < 0 || fromIndex >= count || toIndex < 0 || toIndex >= count ||
        fromIndex == toIndex)
        return;

    const int tabId = window.tabIds[fromIndex];
    window.tabIds.erase(window.tabIds.begin() + fromIndex);
    window.tabIds.insert(window.tabIds.begin() + toIndex, tabId);

    pushTabsToStrip(window);
    focusActiveDocument(window);
}

// Walk the tab order by one, wrapping at either end, and bring that document to the front.
static void activateAdjacentTab(WindowState& window, int direction)
{
    const int count = static_cast<int>(window.tabIds.size());
    if (count < 2)
        return;

    const auto position =
        std::find(window.tabIds.begin(), window.tabIds.end(), window.activeTabId);
    if (position == window.tabIds.end())
        return;

    const int index = static_cast<int>(std::distance(window.tabIds.begin(), position));
    activateTab(window, window.tabIds[((index + direction) % count + count) % count]);
}

// The menu a right click on a tab brings up.
static void showTabContextMenu(HWND hWnd, int tabId, POINT point)
{
    if (!findWindow(hWnd) || !findTab(tabId))
        return;

    hideHoverCard();

    enum
    {
        CLOSE = 1,
        CLOSE_OTHERS,
        MOVE_TO_NEW_WINDOW
    };

    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, CLOSE, translate("TabManager", "Close").c_str());
    AppendMenuW(menu, MF_STRING, CLOSE_OTHERS, translate("TabManager", "Close Others").c_str());
    AppendMenuW(menu, MF_STRING, MOVE_TO_NEW_WINDOW,
                translate("TabManager", "Move Tab to New Window").c_str());

    ClientToScreen(hWnd, &point);
    const int command = static_cast<int>(TrackPopupMenu(
        menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, nullptr));
    DestroyMenu(menu);

    // The menu runs a message loop of its own, which can have taken the window or the tab away.
    WindowState* window = findWindow(hWnd);
    if (!window || !findTab(tabId))
        return;

    // Each close is posted, as from a tab's close button, so the window goes with its last
    // document.
    if (command == CLOSE)
    {
        PostMessageW(hWnd, CODA_WM_CLOSETAB, (WPARAM)tabId, 0);
    }
    else if (command == CLOSE_OTHERS)
    {
        for (int otherId : window->tabIds)
        {
            if (otherId != tabId)
                PostMessageW(hWnd, CODA_WM_CLOSETAB, (WPARAM)otherId, 0);
        }
    }
    else if (command == MOVE_TO_NEW_WINDOW)
    {
        moveTabToNewWindow(hWnd, tabId);
    }
}

// Ctrl+Tab and Ctrl+Shift+Tab move between the documents of a window. The app takes those keys
// rather than leaving them to the page, because a document knows nothing about the other tabs.
static void installTabSwitchAccelerator(ICoreWebView2Controller* controller)
{
    EventRegistrationToken token;
    controller->add_AcceleratorKeyPressed(
        Microsoft::WRL::Callback<ICoreWebView2AcceleratorKeyPressedEventHandler>(
            [](ICoreWebView2Controller* sender,
               ICoreWebView2AcceleratorKeyPressedEventArgs* args) -> HRESULT
            {
                COREWEBVIEW2_KEY_EVENT_KIND kind;
                UINT key;
                if (FAILED(args->get_KeyEventKind(&kind)) ||
                    FAILED(args->get_VirtualKey(&key)))
                    return S_OK;

                if (kind != COREWEBVIEW2_KEY_EVENT_KIND_KEY_DOWN &&
                    kind != COREWEBVIEW2_KEY_EVENT_KIND_SYSTEM_KEY_DOWN)
                    return S_OK;

                if (key != VK_TAB || !(GetKeyState(VK_CONTROL) & 0x8000))
                    return S_OK;

                // The window the WebView2 is in now, as a document can move to another one.
                HWND hWnd;
                if (FAILED(sender->get_ParentWindow(&hWnd)))
                    return S_OK;

                const WindowState* window = findWindow(hWnd);
                if (!window)
                    return S_OK;

                // A full-screen presentation hides the tab strip, and the key does nothing there.
                if (window->isFullScreen)
                {
                    args->put_Handled(TRUE);
                    return S_OK;
                }

                // A window showing a single document has no other tab to move to, so the
                // key belongs to the document.
                if (window->tabIds.size() < 2)
                    return S_OK;

                const int direction = (GetKeyState(VK_SHIFT) & 0x8000) ? -1 : 1;
                PostMessageW(hWnd, CODA_WM_ACTIVATEADJACENTTAB, (WPARAM)direction, 0);

                args->put_Handled(TRUE);
                return S_OK;
            })
            .Get(),
        &token);
}

static void openStarterWindow();

// What the tab strip page asks of the app, as {call, args}.
static void processStripMessage(WindowState& window, const std::string& message)
{
    Poco::JSON::Object::Ptr object;
    if (!JsonUtil::parseJSON(message, object))
    {
        LOG_ERR("Unparseable message from the tab strip: '" << message << "'");
        return;
    }

    const std::string call = object->optValue<std::string>("call", std::string());
    Poco::JSON::Array::Ptr args = object->getArray("args");
    const std::size_t argCount = args ? args->size() : 0;

    if (call == "requestSync")
    {
        // The page has its handlers in place and wants the state it missed.
        notifyStrip(window, "themeChanged", currentStripTheme());
        pushTabsToStrip(window);
    }
    else if (call == "tabActivated" && argCount >= 1)
    {
        activateTab(window, args->getElement<int>(0));
    }
    else if (call == "tabCloseRequested" && argCount >= 1)
    {
        PostMessageW(window.hWnd, CODA_WM_CLOSETAB, (WPARAM)args->getElement<int>(0), 0);
    }
    else if (call == "tabContextMenuRequested" && argCount >= 3)
    {
        POINT point = stripPointToScreen(window, args->getElement<double>(1),
                                         args->getElement<double>(2));
        ScreenToClient(window.hWnd, &point);
        PostMessageW(window.hWnd, CODA_WM_SHOWTABMENU, (WPARAM)args->getElement<int>(0),
                     MAKELPARAM(point.x, point.y));
    }
    else if (call == "tabReordered" && argCount >= 2)
    {
        reorderTab(window, args->getElement<int>(0), args->getElement<int>(1));
    }
    else if (call == "newTabRequested")
    {
        openStarterWindow();
    }
    else if (call == "debug" && argCount >= 1)
    {
        LOG_DBG("From the tab strip: " << args->getElement<std::string>(0));
    }
    else if (call == "tabDragEnded" && argCount >= 2)
    {
        // A drop on the strip has already reordered the tabs. The page sees no difference between
        // a drag cancelled with Escape and a drop that nothing took, so the key is checked here.
        if (args->getElement<bool>(1) || (GetAsyncKeyState(VK_ESCAPE) & 0x8000))
            return;

        // A tab released over the desktop, or over another application, moves out into a window
        // of its own, as with the context menu. The drop overlay belongs to the document window
        // that owns it.
        const int tabId = args->getElement<int>(0);
        POINT point;
        if (!GetCursorPos(&point))
            return;
        const HWND root = GetAncestor(WindowFromPoint(point), GA_ROOT);
        WindowState* target = root ? findWindow(root) : nullptr;
        if (!target && root)
            target = findWindow(GetWindow(root, GW_OWNER));
        if (!target)
        {
            PostMessageW(window.hWnd, CODA_WM_MOVETABTONEWWINDOW, (WPARAM)tabId,
                         MAKELPARAM(point.x, point.y));
            return;
        }

        // Released over the document of another tabbed window, the tab joins that window at the
        // end. A drop on the strip of that window arrives from its page as
        // tabAdoptFromOtherWindow instead, with the position in the tab order.
        if (target == &window || !target->isTabbed)
            return;
        RECT strip = stripBounds(*target);
        MapWindowPoints(target->hWnd, HWND_DESKTOP, reinterpret_cast<POINT*>(&strip), 2);
        if (PtInRect(&strip, point))
            return;
        PostMessageW(target->hWnd, CODA_WM_MOVETABTOWINDOW, (WPARAM)tabId, (LPARAM)-1);
    }
    else if (call == "tabAdoptFromOtherWindow" && argCount >= 2)
    {
        PostMessageW(window.hWnd, CODA_WM_MOVETABTOWINDOW, (WPARAM)args->getElement<int>(0),
                     (LPARAM)args->getElement<int>(1));
    }
    else if (call == "tabHoverStarted" && argCount >= 7)
    {
        const POINT point = { static_cast<LONG>(std::lround(args->getElement<double>(1))),
                              static_cast<LONG>(std::lround(args->getElement<double>(2))) };
        startHoverCard(window, args->getElement<int>(0), point, args->getElement<std::string>(3),
                       args->getElement<std::string>(4), args->getElement<std::string>(5),
                       args->getElement<bool>(6));
    }
    else if (call == "tabHoverEnded")
    {
        hideHoverCard();
    }
    else if (call == "tabDragStarted" || call == "targetDragOver")
    {
        // The app learns where a tab drag ended from tabDragEnded, and from the drop calls of the
        // strip it ended on, so it does not follow the drag as it goes. The start of a drag only
        // takes the hover card away.
        if (call == "tabDragStarted")
            hideHoverCard();
    }
    else
    {
        LOG_ERR("Unhandled message from the tab strip: '" << message << "'");
    }
}

// The tab strip is the same page CODA-Q draws, in a web view of its own along the top of the
// window.
static void createTabStrip(WindowState& window)
{
    const HWND hWnd = window.hWnd;

    withWebView2Environment(
        [hWnd](ICoreWebView2Environment* env)
        {
            env->CreateCoreWebView2Controller(
                hWnd,
                Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                    {
                        WindowState* window = findWindow(hWnd);
                        if (!controller || !window)
                            return E_FAIL;

                        ICoreWebView2* webView;
                        controller->get_CoreWebView2(&webView);
                        window->stripWebView = wil::com_ptr<ICoreWebView2>(webView);
                        window->stripController = controller;

                        wil::com_ptr<ICoreWebView2Settings> settings;
                        webView->get_Settings(&settings);
                        settings->put_IsScriptEnabled(TRUE);
                        settings->put_IsWebMessageEnabled(TRUE);
                        settings->put_AreDefaultContextMenusEnabled(FALSE);
                        // The strip is part of the window frame rather than content, so it stays at
                        // one scale. Ctrl with the wheel and the keyboard shortcuts go through
                        // IsZoomControlEnabled, and a touch pinch has its own setting on
                        // ICoreWebView2Settings5.
                        settings->put_IsZoomControlEnabled(FALSE);
                        wil::com_ptr<ICoreWebView2Settings5> settings5
                            = settings.try_query<ICoreWebView2Settings5>();
                        if (settings5)
                            settings5->put_IsPinchZoomEnabled(FALSE);
                        // The strip stays on the one page it loaded, so a horizontal swipe across
                        // it does not navigate.
                        wil::com_ptr<ICoreWebView2Settings6> settings6
                            = settings.try_query<ICoreWebView2Settings6>();
                        if (settings6)
                            settings6->put_IsSwipeNavigationEnabled(FALSE);
                        // The page marks the space around the tabs with app-region: drag, and a
                        // drag there moves the window as a drag on the title bar does.
                        wil::com_ptr<ICoreWebView2Settings9> settings9
                            = settings.try_query<ICoreWebView2Settings9>();
                        if (settings9)
                            settings9->put_IsNonClientRegionSupportEnabled(TRUE);

                        EventRegistrationToken token;
                        webView->add_WebMessageReceived(
                            Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                [hWnd](ICoreWebView2* sender,
                                       ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
                                {
                                    WindowState* window = findWindow(hWnd);
                                    if (!window)
                                        return S_OK;

                                    // The page posts an object, so the message
                                    // arrives as JSON rather than as a string.
                                    wil::unique_cotaskmem_string json;
                                    if (SUCCEEDED(args->get_WebMessageAsJson(&json)))
                                        processStripMessage(
                                            *window, Util::wide_string_to_string(json.get()));
                                    return S_OK;
                                })
                                .Get(),
                            &token);

                        installTabSwitchAccelerator(controller);

                        controller->put_IsVisible(FALSE);
                        controller->put_Bounds(stripBounds(*window));

                        // As for a document, the strip appears only once it has something styled
                        // to show, so the window frame covers the gap instead of a white page.
                        EventRegistrationToken readyToken;
                        if (auto stripWebView2 = window->stripWebView.try_query<ICoreWebView2_2>())
                            stripWebView2->add_DOMContentLoaded(
                                Microsoft::WRL::Callback<ICoreWebView2DOMContentLoadedEventHandler>(
                                    [hWnd](ICoreWebView2* sender,
                                           ICoreWebView2DOMContentLoadedEventArgs* args) -> HRESULT
                                    {
                                        WindowState* stripWindow = findWindow(hWnd);
                                        if (stripWindow && !stripWindow->stripHasContent)
                                        {
                                            stripWindow->stripHasContent = true;
                                            if (stripWindow->stripController)
                                                stripWindow->stripController->put_IsVisible(
                                                    stripWindow->stripIsVisible);
                                        }
                                        return S_OK;
                                    })
                                    .Get(),
                                &readyToken);

                        // The page reads the theme out of the query string, so
                        // it is in the right colours from its first paint.
                        const std::string stripURL =
                            app_installation_uri + "../cool/qtapp-tabstrip.html?darkTheme=" +
                            (currentStripTheme() == "dark" ? "true" : "false") + "&titleBar=true" +
                            // The tabs run from right to left in a language written that way, as
                            // the document UI does.
                            (LangUtil::isRtlLanguage(uiLanguage) ? "&dir=rtl" : "");
                        webView->Navigate(Util::string_to_wide_string(stripURL).c_str());

                        return S_OK;
                    })
                    .Get());
        });
}

// The web view of one document, in the window its tab belongs to. The document
// itself is loaded once the page reports back with HULLO.
static void createDocumentView(int tabId)
{
    withWebView2Environment(
        [tabId](ICoreWebView2Environment* env)
        {
            DocumentTab* tab = findTab(tabId);
            if (!tab)
                return;

            // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is
            // the window the document is shown in
            env->CreateCoreWebView2Controller(
                tab->hWnd,
                Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [tabId, env](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                    {
                        DocumentTab* data = findTab(tabId);
                        if (!controller || !data)
                            return E_FAIL;

                        ICoreWebView2* webView;
                        controller->get_CoreWebView2(&webView);
                        data->webView = wil::com_ptr<ICoreWebView2>(webView);
                        data->webViewController = controller;

                        wil::com_ptr<ICoreWebView2_22> webView22 = data->webView.try_query<ICoreWebView2_22>();
                        if (!webView22)
                            fatal("Could not get webView22");

                        // Add a few settings for the webview
                        // The demo step is redundant since the values are the default settings
                        wil::com_ptr<ICoreWebView2Settings> settings;
                        webView->get_Settings(&settings);
                        settings->put_IsScriptEnabled(TRUE);
                        settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                        settings->put_IsWebMessageEnabled(TRUE);
                        // Stop browser shortcut keys (such as F12 for the
                        // developer tools and F5 for reload) from being
                        // handled by the WebView, so the keys reach the
                        // document instead. F12 then toggles the numbered
                        // list rather than opening the developer tools.
                        wil::com_ptr<ICoreWebView2Settings4> settings4
                            = settings.try_query<ICoreWebView2Settings4>();
                        if (settings4)
                            settings4->put_AreBrowserAcceleratorKeysEnabled(FALSE);

                        // The document can have moved to another window while its WebView2 was
                        // being made.
                        controller->put_ParentWindow(data->hWnd);

                        installTabSwitchAccelerator(controller);

                        // Fit the WebView to the part of the window below the tab strip. It stays
                        // hidden until its page has something styled to show, so what fills the
                        // window until then is the frame, in the colour of the theme.
                        WindowState* window = findWindow(data->hWnd);
                        if (window)
                        {
                            controller->put_Bounds(documentArea(*window));
                            controller->put_IsVisible(FALSE);
                        }

                        // The page has parsed its document and the stylesheets that block its
                        // first paint. A navigation that fails reaches the second of these and
                        // not the first, and an empty document view is still better than one
                        // that never appears.
                        EventRegistrationToken readyToken;
                        auto showWhenReady = [tabId]()
                        {
                            DocumentTab* tab = findTab(tabId);
                            WindowState* tabWindow = tab ? findWindow(tab->hWnd) : nullptr;
                            if (!tab || !tabWindow || tab->hasContent)
                                return;

                            tab->hasContent = true;
                            if (tabWindow->activeTabId != tabId || !tab->webViewController)
                                return;

                            tab->webViewController->put_IsVisible(TRUE);
                            tab->webViewController->MoveFocus(
                                COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
                        };

                        if (auto webView2 = data->webView.try_query<ICoreWebView2_2>())
                            webView2->add_DOMContentLoaded(
                                Microsoft::WRL::Callback<ICoreWebView2DOMContentLoadedEventHandler>(
                                    [showWhenReady](ICoreWebView2* sender,
                                                    ICoreWebView2DOMContentLoadedEventArgs* args)
                                        -> HRESULT
                                    {
                                        showWhenReady();
                                        return S_OK;
                                    })
                                    .Get(),
                                &readyToken);

                        webView->add_NavigationCompleted(
                            Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                [showWhenReady](ICoreWebView2* sender,
                                                ICoreWebView2NavigationCompletedEventArgs* args)
                                    -> HRESULT
                                {
                                    showWhenReady();
                                    return S_OK;
                                })
                                .Get(),
                            &readyToken);

                        EventRegistrationToken token;
                        HRESULT hr;

                        hr = (webView22->AddWebResourceRequestedFilterWithRequestSourceKinds(
                                  L"cool://*",
                                  COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL,
                                  COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_ALL));
                        if (!SUCCEEDED(hr))
                        {
                            LOG_ERR_S("AddWebResourceRequestedFilterWithRequestSourceKinds() failed");
                            return hr;
                        }

                        hr = webView->add_WebResourceRequested(
                            Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>(
                                [env](ICoreWebView2* sender,
                                      ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT
                                {
                                    return webResourceRequestedHandler(env, sender, args);
                                }).Get(), &token);

                        if (!SUCCEEDED(hr))
                        {
                            LOG_ERR_S("add_WebResourceRequested() failed");
                            return hr;
                        }

                        // Communication between host and web content
                        // Set an event handler for the host to return received message back to the web content
                        webView->add_WebMessageReceived(
                            Microsoft::WRL::Callback<
                                ICoreWebView2WebMessageReceivedEventHandler>(
                                [tabId](
                                    ICoreWebView2* webView,
                                    ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
                                {
                                    DocumentTab* tab = findTab(tabId);
                                    if (!tab)
                                        return S_OK;

                                    wil::unique_cotaskmem_string message;
                                    args->TryGetWebMessageAsString(&message);
                                    processMessage(*tab, message);
                                    return S_OK;
                                })
                                .Get(),
                            &token);

                        webView->add_ContainsFullScreenElementChanged(
                            Microsoft::WRL::Callback<ICoreWebView2ContainsFullScreenElementChangedEventHandler>(
                                [tabId](ICoreWebView2* sender, IUnknown* args) -> HRESULT
                                {
                                    DocumentTab* tab = findTab(tabId);
                                    WindowState* window = tab ? findWindow(tab->hWnd) : nullptr;
                                    if (!window)
                                        return S_OK;

                                    BOOL containsFullscreenElement;
                                    sender->get_ContainsFullScreenElement(&containsFullscreenElement);
                                    if (containsFullscreenElement)
                                    {
                                        HMONITOR monitor = MonitorFromWindow(tab->hWnd, MONITOR_DEFAULTTONEAREST);
                                        enter_full_screen(*window, monitor, true);
                                    }
                                    else
                                        leave_full_screen(*window);
                                    return S_OK;
                                })
                                .Get(),
                            nullptr);

                        // New windows appear to need to reuse the original env of the parent, a good explanation
                        // of use at: https://github.com/MicrosoftEdge/WebView2Feedback/discussions/4501#discussioncomment-9215801
                        webView->add_NewWindowRequested(
                            Microsoft::WRL::Callback<ICoreWebView2NewWindowRequestedEventHandler>(
                                [env, tabId](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args)
                                {
                                    DocumentTab* data = findTab(tabId);
                                    if (!data)
                                        return S_OK;

                                    wil::com_ptr<ICoreWebView2Deferral> deferral;
                                    args->GetDeferral(&deferral);

                                    HMONITOR hMonitor = MonitorFromWindow(data->hWnd, MONITOR_DEFAULTTONEAREST);
                                    MONITORINFO monitorInfo = { sizeof(monitorInfo) };
                                    GetMonitorInfo(hMonitor, &monitorInfo);
                                    const RECT& area = monitorInfo.rcWork;
                                    int areaWidth = area.right - area.left;
                                    int areaHeight = area.bottom - area.top;
                                    int width = areaWidth * 17 / 20;
                                    int height = areaHeight * 17 / 20;
                                    int x = area.left + (areaWidth - width) / 2;
                                    int y = area.top + (areaHeight - height) / 2;

                                    // The presenter console is a window of its own rather than a
                                    // tab: it plays full screen on the monitor the presentation
                                    // does not use.
                                    data->hConsoleWnd = CreateWindowW(windowClass,
                                            Util::string_to_wide_string(APP_NAME).c_str(),
                                            WS_OVERLAPPEDWINDOW,
                                            x, y, width, height,
                                            NULL, NULL, appInstance, NULL);

                                    WindowState& consoleWindow = windows[data->hConsoleWnd];
                                    consoleWindow.hWnd = data->hConsoleWnd;

                                    DocumentTab& consoleTab = createTab(consoleWindow, {},
                                                                        DocumentMode::EDIT);
                                    consoleTab.isConsole = true;
                                    consoleTab.presentingTabId = tabId;
                                    consoleWindow.activeTabId = consoleTab.tabId;
                                    const int consoleTabId = consoleTab.tabId;

                                    ShowWindow(data->hConsoleWnd, appShowMode);

                                    env->CreateCoreWebView2Controller(
                                        data->hConsoleWnd,
                                        Microsoft::WRL::Callback<
                                            ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                                            [consoleTabId, tabId, args, deferral](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                                            {
                                                DocumentTab* consoleData = findTab(consoleTabId);
                                                DocumentTab* presenting = findTab(tabId);
                                                if (!controller || !consoleData || !presenting)
                                                    return E_FAIL;

                                                ICoreWebView2* webView;
                                                controller->get_CoreWebView2(&webView);
                                                consoleData->webView = wil::com_ptr<ICoreWebView2>(webView);

                                                webView->add_WindowCloseRequested(
                                                    Microsoft::WRL::Callback<ICoreWebView2WindowCloseRequestedEventHandler>(
                                                        [consoleTabId](ICoreWebView2* sender, IUnknown* args)
                                                        {
                                                            DocumentTab* consoleData = findTab(consoleTabId);
                                                            if (consoleData)
                                                                PostMessageW(consoleData->hWnd, WM_CLOSE, 0, 0);
                                                            return S_OK;
                                                        })
                                                        .Get(),
                                                    nullptr);

                                                controller->put_IsVisible(TRUE);

                                                consoleData->webViewController = controller;

                                                // Resize WebView to fit the bounds of the parent window
                                                RECT bounds;
                                                GetClientRect(consoleData->hWnd, &bounds);
                                                controller->put_Bounds(bounds);

                                                args->put_NewWindow(consoleData->webView.get());
                                                args->put_Handled(TRUE);
                                                deferral->Complete();

                                                arrangePresentationWindows(*presenting);

                                                return S_OK;
                                            })
                                            .Get());

                                    return S_OK;
                                })
                                .Get(),
                            nullptr);

                        std::string coolURL =
                            app_installation_uri + "../cool/cool.html?";
                        if (data->mode == DocumentMode::STARTER)
                            coolURL += "starterMode=true";
                        else
                        {
                            if (data->mode != DocumentMode::WELCOME)
                                recentFiles.add(data->filenameAndUri.uri);
                            coolURL +=
                                "file_path=" + data->filenameAndUri.uri +
                                std::string("&permission=edit") +
                                std::string("&appdocid=") + std::to_string(data->appDocId) +
                                std::string("&userinterfacemode=notebookbar");
                        }

                        coolURL += "&lang=" + uiLanguage;
                        coolURL += "&dir=" + std::string(LangUtil::isRtlLanguage(uiLanguage) ? "rtl" : "");

                        // Saved choice wins, otherwise follow the system theme.
                        coolURL += darkModeEnabled() ? "&darkTheme=true" : "&darkTheme=false";

                        if (data->mode != DocumentMode::STARTER)
                            coolURL +=
                                std::string((data->mode != DocumentMode::NEW ? "&startreadonly=true" : "")) +
                                std::string((data->mode == DocumentMode::WELCOME ? "&welcome=true" : ""));

                        webView->Navigate(Util::string_to_wide_string(coolURL).c_str());

                        return S_OK;
                    })
                    .Get());
        });
}

// Set up a top-level window of the app's own class, ready to take documents.
static WindowState& createAppWindow(const std::wstring& title, DWORD style, int x, int y,
                                    int width, int height)
{
    const HWND hWnd = CreateWindowW(windowClass, title.c_str(), style, x, y, width, height,
                                    NULL, NULL, appInstance, NULL);

    WindowState& window = windows[hWnd];
    window.hWnd = hWnd;

    ShowWindow(hWnd, appShowMode);
    UpdateWindow(hWnd);

    AddClipboardFormatListener(hWnd);

    return window;
}

// Set size of document window to be 90% of monitor width and height.
//
// FIXME: Should we actually, at least for text documents, ideally peek into the document and
// check what its page size is, and in the common case of a portrait orientation text document,
// make the document window also (if the monitor is large enough) higher than wider? On small
// monitors (1280x768 or less?) we should probably default to making the document window
// full-screen?
//
// FIXME: My initial assumption that the COOL window would open up on the monitor where the
// file section dialog was is incorrect.
static void defaultWindowSize(int& width, int& height)
{
    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfoW(primaryMonitor, &monitorInfo))
    {
        width = 0.9 * (monitorInfo.rcWork.right - monitorInfo.rcWork.left);
        height = 0.9 * (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);
    }
    else
    {
        width = 1200;
        height = 900;
    }
}

// A new window that shows documents as tabs. It goes once its last tab closes. Given a point of the
// screen, the window comes up with that point on its title bar, and inside the work area of the
// monitor the point is on.
static WindowState& createTabbedWindow(const POINT* titleBarPoint = nullptr)
{
    int width, height;
    defaultWindowSize(width, height);

    int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
    if (titleBarPoint)
    {
        x = titleBarPoint->x - width / 2;
        y = titleBarPoint->y - GetSystemMetrics(SM_CYSIZEFRAME) -
            GetSystemMetrics(SM_CXPADDEDBORDER) - GetSystemMetrics(SM_CYCAPTION) / 2;

        // The default size is taken from the primary monitor, which can be larger than this one.
        MONITORINFO monitorInfo;
        monitorInfo.cbSize = sizeof(monitorInfo);
        if (GetMonitorInfoW(MonitorFromPoint(*titleBarPoint, MONITOR_DEFAULTTONEAREST),
                            &monitorInfo))
        {
            const RECT& work = monitorInfo.rcWork;
            width = std::min(width, static_cast<int>(work.right - work.left));
            height = std::min(height, static_cast<int>(work.bottom - work.top));
            x = std::clamp(x, static_cast<int>(work.left), static_cast<int>(work.right) - width);
            y = std::clamp(y, static_cast<int>(work.top), static_cast<int>(work.bottom) - height);
        }
    }

    WindowState& window = createAppWindow(Util::string_to_wide_string(APP_NAME),
                                          WS_OVERLAPPEDWINDOW, x, y, width, height);
    window.isTabbed = true;
    tabbedWindow = window.hWnd;

    // In a language written from right to left the window is mirrored, as the tab strip is, so the
    // strip starts at the right and the caption buttons are at the left. The web views in it are
    // not mirrored, as their pages lay themselves out from the dir parameter.
    if (LangUtil::isRtlLanguage(uiLanguage))
        SetWindowLongW(window.hWnd, GWL_EXSTYLE,
                       GetWindowLongW(window.hWnd, GWL_EXSTYLE) | WS_EX_LAYOUTRTL |
                           WS_EX_NOINHERITLAYOUT);

    // The window was created before it was known to be tabbed, so its frame and the colour of its
    // title bar are computed again.
    applyTitleBarTheme(window.hWnd);

    createTabStrip(window);

    return window;
}

// The tabbed window that a newly opened document goes to, made with the first document.
static WindowState& getOrCreateTabbedWindow()
{
    WindowState* existing = findWindow(tabbedWindow);
    if (existing)
        return *existing;

    return createTabbedWindow();
}

// Take a document out of its window, and show it in another tabbed window, at the position
// insertAt in the tab order of that window, or at the end when insertAt is -1. The WebView2 moves
// with it, so the document stays loaded as it is. The window it leaves goes if it has no other
// document.
static void moveTabToWindow(int tabId, WindowState& target, int insertAt)
{
    DocumentTab* tab = findTab(tabId);
    WindowState* source = tab ? findWindow(tab->hWnd) : nullptr;
    if (!source || source == &target || !source->isTabbed || !target.isTabbed ||
        target.isFullScreen)
        return;

    removeTabFromWindow(*source, tabId);

    tab->hWnd = target.hWnd;
    const int count = static_cast<int>(target.tabIds.size());
    target.tabIds.insert(target.tabIds.begin() +
                             (insertAt < 0 || insertAt > count ? count : insertAt),
                         tabId);
    if (tab->webViewController)
    {
        tab->webViewController->put_ParentWindow(target.hWnd);
        tab->webViewController->put_Bounds(documentArea(target));
    }

    activateTab(target, tabId);
    SetForegroundWindow(target.hWnd);

    // The WebView2 has a new parent by now, so it lives on when the window it left goes.
    if (source->tabIds.empty())
        DestroyWindow(source->hWnd);
}

// Take a document out of its window, and show it in a new tabbed window of its own, with the title
// bar at the drop point when there is one.
static void moveTabToNewWindow(HWND hWnd, int tabId, const POINT* dropPoint)
{
    WindowState* source = findWindow(hWnd);
    DocumentTab* tab = findTab(tabId);
    if (!source || !tab || tab->hWnd != hWnd || source->tabIds.size() < 2)
        return;

    moveTabToWindow(tabId, createTabbedWindow(dropPoint), -1);
}

// The window the welcome slideshow plays in. It is displayed without
// decorations, and at 16:9 because we know it is that aspect ratio.
static WindowState& createWelcomeWindow()
{
    int width, height;
    int x = CW_USEDEFAULT, y = CW_USEDEFAULT;

    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfoW(primaryMonitor, &monitorInfo))
    {
        double aspectRatio =
            (double)(monitorInfo.rcWork.right - monitorInfo.rcWork.left) / (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);
        if (aspectRatio < 16.0/9.0)
        {
            width = 0.9 * (monitorInfo.rcWork.right - monitorInfo.rcWork.left);
            x = monitorInfo.rcWork.left + 0.05 * (monitorInfo.rcWork.right - monitorInfo.rcWork.left);
            height = width / (16.0/9.0);
            y = monitorInfo.rcWork.top + ((monitorInfo.rcWork.bottom - monitorInfo.rcWork.top) - height) / 2;
        }
        else
        {
            height = 0.9 * (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);
            y = monitorInfo.rcWork.top + 0.05 * (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);
            width = (16.0/9.0) * height;
            x = monitorInfo.rcWork.left + ((monitorInfo.rcWork.right - monitorInfo.rcWork.left) - width) / 2;
        }
    }
    else
    {
        width = 1280;
        height = 720;
    }

    return createAppWindow(Util::string_to_wide_string(APP_NAME), WS_POPUP, x, y, width, height);
}

// Find out whether the document is already open, and return its tab if it is.
static DocumentTab* findOpenDocumentTab(const std::string& uri)
{
    if (uri.empty())
        return nullptr;

    const std::string path = Poco::URI(uri).getPath();
    for (auto& i : documentTabs)
    {
        if (Poco::URI(i.second.filenameAndUri.uri).getPath() == path)
            return &i.second;
    }
    return nullptr;
}

static void openCOOLWindow(const FilenameAndUri& filenameAndUri, DocumentMode mode)
{
    // Don't open an already open document in a new tab, switch to its tab instead.
    if (mode == DocumentMode::EDIT)
    {
        if (DocumentTab* openTab = findOpenDocumentTab(filenameAndUri.uri))
        {
            if (WindowState* openWindow = findWindow(openTab->hWnd))
            {
                SetForegroundWindow(openWindow->hWnd);
                activateTab(*openWindow, openTab->tabId);
            }
            // Go on with the rest when several documents are opened at once.
            load_next_document();
            return;
        }
    }

    // The welcome slideshow and the starter backstage each get a window to
    // themselves: neither is a document the user works in alongside others.
    WindowState* window = nullptr;
    if (mode == DocumentMode::WELCOME)
    {
        window = &createWelcomeWindow();
    }
    else if (mode == DocumentMode::STARTER)
    {
        int width, height;
        defaultWindowSize(width, height);
        window = &createAppWindow(Util::string_to_wide_string(APP_NAME), WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, CW_USEDEFAULT, width, height);
    }
    else
    {
        window = &getOrCreateTabbedWindow();
        // The document the user just asked for comes to the front.
        SetForegroundWindow(window->hWnd);
    }

    DocumentTab& tab = createTab(*window, filenameAndUri, mode);
    if (mode != DocumentMode::STARTER)
    {
        tab.fakeClientFd = fakeSocketSocket();
        tab.appDocId = generate_new_app_doc_id();
    }

    activateTab(*window, tab.tabId);
    createDocumentView(tab.tabId);
}

static void openStarterWindow()
{
    // One backstage is enough: a second ask brings the open one forward.
    for (const auto& i : documentTabs)
    {
        if (i.second.mode == DocumentMode::STARTER)
        {
            SetForegroundWindow(i.second.hWnd);
            return;
        }
    }

    openCOOLWindow({}, DocumentMode::STARTER);
}

namespace
{
// Result of a WinHTTP request: an HTTP status code (>= 100), or one of the
// ai:: sentinels (HttpNoResponse / HttpConnectFailed) when no response arrived.
struct HttpResult
{
    int statusCode;
    std::string body;
};

// Perform a blocking HTTP request with WinHTTP. The desktop app has no COOL net
// stack for outbound requests, so the AI proxy and the Options dialog's model
// listing reach providers through this. \c authHeader, when non-empty, becomes
// the Authorization header value (e.g. "Bearer <key>").
HttpResult winHttpRequest(const wchar_t* verb, const std::string& url,
                          const std::string& authHeader, const std::string& body,
                          int timeoutSeconds)
{
    HttpResult result{ ai::HttpConnectFailed, std::string() };

    const std::wstring wideUrl = Util::string_to_wide_string(url);
    URL_COMPONENTS components;
    ZeroMemory(&components, sizeof(components));
    components.dwStructSize = sizeof(components);
    wchar_t hostName[256] = {};
    wchar_t urlPath[8192] = {};
    components.lpszHostName = hostName;
    components.dwHostNameLength = ARRAYSIZE(hostName);
    components.lpszUrlPath = urlPath;
    components.dwUrlPathLength = ARRAYSIZE(urlPath);
    if (!WinHttpCrackUrl(wideUrl.c_str(), 0, 0, &components))
        return result;

    HINTERNET session = WinHttpOpen(L"CODA", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (session == nullptr)
        return result;

    if (timeoutSeconds > 0)
    {
        const int ms = timeoutSeconds * 1000;
        WinHttpSetTimeouts(session, ms, ms, ms, ms);
    }

    HINTERNET connection = WinHttpConnect(session, hostName, components.nPort, 0);
    if (connection == nullptr)
    {
        WinHttpCloseHandle(session);
        return result;
    }

    const DWORD flags = (components.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET request = WinHttpOpenRequest(connection, verb, urlPath, nullptr, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (request == nullptr)
    {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return result;
    }

    std::wstring headers = L"Content-Type: application/json\r\n";
    if (!authHeader.empty())
        headers += L"Authorization: " + Util::string_to_wide_string(authHeader) + L"\r\n";
    WinHttpAddRequestHeaders(request, headers.c_str(), static_cast<DWORD>(-1),
                             WINHTTP_ADDREQ_FLAG_ADD);

    BOOL ok = WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                 body.empty() ? WINHTTP_NO_REQUEST_DATA
                                              : const_cast<char*>(body.data()),
                                 static_cast<DWORD>(body.size()),
                                 static_cast<DWORD>(body.size()), 0);
    if (ok)
        ok = WinHttpReceiveResponse(request, nullptr);

    if (!ok)
    {
        result.statusCode =
            (GetLastError() == ERROR_WINHTTP_TIMEOUT) ? ai::HttpNoResponse : ai::HttpConnectFailed;
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return result;
    }

    DWORD statusCode = 0;
    DWORD statusLen = sizeof(statusCode);
    WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusLen,
                        WINHTTP_NO_HEADER_INDEX);
    result.statusCode = static_cast<int>(statusCode);

    DWORD available = 0;
    do
    {
        available = 0;
        if (!WinHttpQueryDataAvailable(request, &available) || available == 0)
            break;
        std::string chunk(available, '\0');
        DWORD read = 0;
        if (!WinHttpReadData(request, &chunk[0], available, &read))
            break;
        chunk.resize(read);
        result.body += chunk;
    } while (available > 0);

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);
    return result;
}

// Register the AI chat HTTP transport. AIChatSession reaches the provider
// through this platform hook (the Windows counterpart of Qt's
// registerAIHttpTransport()). onDone may run on any thread; AIChatSession hops
// back onto its polling thread itself.
void registerAIHttpTransport()
{
    ai::setHttpPostFn(
        [](const std::string& url, const std::string& authHeader, std::string body,
           int timeoutSeconds, ai::HttpDoneCallback onDone)
        {
            // Called on the document's polling thread; run the blocking WinHTTP
            // request on its own thread so we don't stall that thread.
            std::thread(
                [url, authHeader, body = std::move(body), timeoutSeconds,
                 onDone = std::move(onDone)]() mutable
                {
                    ProcUtil::setThreadName("aihttp");
                    const HttpResult result =
                        winHttpRequest(L"POST", url, authHeader, body, timeoutSeconds);
                    onDone(result.statusCode, std::move(result.body));
                })
                .detach();
        });
}

// Fetch an AI provider's model list for the Options dialog. The desktop apps
// have no server-side proxy, so the app issues the request itself, mirroring
// Qt's Desktop::fetchAIModels(). The payload is {"provider","apiKey","baseUrl"};
// returns the provider's JSON body verbatim ({"data":[...]} or its own error
// JSON), or an {"error":...} JSON of our own.
std::string fetchAIModels(const std::string& payload)
{
    Poco::JSON::Object::Ptr obj;
    if (!JsonUtil::parseJSON(payload, obj))
        return R"({"error":"Invalid payload"})";

    std::string provider, apiKey, baseUrl;
    JsonUtil::findJSONValue(obj, "provider", provider);
    JsonUtil::findJSONValue(obj, "apiKey", apiKey);
    JsonUtil::findJSONValue(obj, "baseUrl", baseUrl);

    if (provider.empty() || apiKey.empty())
        return R"({"error":"Missing provider or apiKey"})";

    if (provider != "custom")
    {
        // Keep in sync with preCannedAIProviderBaseUrl() in wsd/FileServer.cpp
        // and the same map in qt/Application.cpp / macos COWrapper.mm.
        static const std::map<std::string, std::string> preCanned = {
            { "openai", "https://api.openai.com" },
            { "groq", "https://api.groq.com/openai" },
            { "together", "https://api.together.xyz" },
            { "mistral", "https://api.mistral.ai" },
        };
        const auto it = preCanned.find(provider);
        if (it == preCanned.end())
            return R"({"error":"Unknown provider"})";
        baseUrl = it->second;
    }
    else if (baseUrl.empty())
    {
        return R"({"error":"Missing baseUrl for custom provider"})";
    }

    if (!baseUrl.empty() && baseUrl.back() == '/')
        baseUrl.pop_back();
    baseUrl += "/v1/models";

    const HttpResult result = winHttpRequest(L"GET", baseUrl, "Bearer " + apiKey, std::string(), 30);
    if (result.statusCode < 100 && result.body.empty())
        return R"({"error":"Failed to reach the AI provider"})";
    return result.body;
}
} // namespace

static HANDLE copyEngineClipboardData(UINT format, const std::string& mimeType)
{
    // The clipboard is process-global (one shared clipboard for the desktop app), so read it
    // straight from the engine; no document is involved.
    std::vector<char> bytes;
    if (!office || !office->getGlobalClipboardData(mimeType.c_str(), bytes))
        return 0;

    HGLOBAL hMem;
    const void *src;
    size_t size;
    std::wstring wtemp;
    std::string temp;
    if (format == CF_UNICODETEXT && mimeType == "text/plain;charset=utf-8")
    {
        wtemp = Util::string_to_wide_string(std::string_view(bytes.data(), bytes.size()));
        src = wtemp.c_str();
        size = (wtemp.length() + 1) * 2;
    }
    else if (format == RegisterClipboardFormatW(L"HTML Format") && mimeType == "text/html")
    {
        temp = generate_html_format(std::string(bytes.data(), bytes.size()));
        src = temp.c_str();
        size = temp.length();
    }
    else
    {
        src = bytes.data();
        size = bytes.size();
    }
    hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (hMem == NULL)
    {
        return 0;
    }
    void* dest = GlobalLock(hMem);
    if (dest == nullptr)
    {
        GlobalFree(hMem);
        return 0;
    }
    std::memcpy(dest, src, size);
    GlobalUnlock(hMem);

    return hMem;
}

static std::string MIME_type_for_clipboard_format(UINT format)
{
    if (format == CF_UNICODETEXT)
        return "text/plain;charset=utf-8";
    if (format == CF_HDROP)
        return "text/uri-list";

    auto name = get_clipboard_format_name(format);

    if (name == L"text/markdown" ||
        name == L"text/rtf" ||
        name == L"image/png" ||
        name == L"image/svg+xml")
        // Clipboard format names that are directly MIME types
        return Util::wide_string_to_string(name);
    else if (name == L"Markdown")
        return "text/markdown";
    else if (name == L"Star Embed Source (XML)")
        return "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
    else if (name == L"Star Object Descriptor (XML)")
        return "application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"";
    else if (name == L"PNG")
        return "image/png";
    else if (name == L"Rich Text Format")
        return "text/rtf";
    else if (name == L"HTML Format")
        return "text/html";
    else if (name == L"FileNameW")
        return "text/uri-list";

    return "";
}

static std::vector<int> clipboard_formats_for_MIME_type(const char* mimeType)
{
    std::vector<int> result;

    if (std::strcmp(mimeType, "text/plain;charset=utf-8") == 0)
        result.push_back(CF_UNICODETEXT);
    else if (std::strcmp(mimeType, "text/rtf") == 0)
        result.push_back(RegisterClipboardFormatW(L"Rich Text Format"));
    else if (std::strcmp(mimeType, "text/html") == 0)
        result.push_back(RegisterClipboardFormatW(L"HTML Format"));
    else if (std::strcmp(mimeType, "text/markdown") == 0)
    {
        result.push_back(RegisterClipboardFormatW(L"text/markdown"));
        result.push_back(RegisterClipboardFormatW(L"Markdown"));
    }
    else if (std::strcmp(mimeType, "image/png") == 0)
    {
        result.push_back(RegisterClipboardFormatW(L"image/png"));
        result.push_back(RegisterClipboardFormatW(L"PNG"));
    }
    else if (std::strcmp(mimeType, "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"") == 0)
        result.push_back(RegisterClipboardFormatW(L"Star Embed Source (XML)"));
    else if (std::string(mimeType).starts_with("application/x-openoffice-objectdescriptor-xml;"))
        result.push_back(RegisterClipboardFormatW(L"Star Object Descriptor (XML)"));
    else if (std::strcmp(mimeType, "image/svg+xml;windows_formatname=\"image/svg+xml\"") == 0)
        result.push_back(RegisterClipboardFormatW(L"image/svg+xml"));

    return result;
}

/**
 * The clipboard provider the engine drives. On copy the engine advertises its formats through
 * advertise; on an external paste it reads the clipboard one format at a time. The callbacks
 * act on the process, not one window, so the one shared clipboard is reached from whichever
 * document is current.
 */

static void clipboardProviderAdvertise(const char** pMimeTypes)
{
    // Delayed rendering needs a live window to own the clipboard and receive WM_RENDERFORMAT. Use
    // the hidden owner window, which lives for the whole app run. The clipboard is only rendered in
    // full (WM_RENDERALLFORMATS) when the app exits, not when an individual document window closes.
    if (!hiddenOwnerWindow)
        return;

    if (!try_open_clipboard(hiddenOwnerWindow))
        return;

    if (!EmptyClipboard())
    {
        CloseClipboard();
        return;
    }

    bool didSetData = false;

    for (size_t i = 0; pMimeTypes && pMimeTypes[i]; ++i)
    {
        auto formats = clipboard_formats_for_MIME_type(pMimeTypes[i]);
        if (formats.size() != 0)
        {
            for (auto const &format : formats)
                SetClipboardData(format, NULL);
            didSetData = true;
        }
    }

    if (didSetData)
        weOwnTheClipboard = true;

    CloseClipboard();
}

static bool clipboardProviderOwns()
{
    return weOwnTheClipboard;
}

static std::vector<std::string> clipboardProviderGetMimeTypes()
{
    // Reading needs no owner window, so open the clipboard for the current "task".
    //
    // (Task is an 16-bit Windows term still used in documentation for the clipboard API that is
    // basically unchanged since then. In current Windows, it means thread, more or less.)
    if (!try_open_clipboard(NULL))
        return {};

    UINT format = 0;

    std::vector<std::string> mimeTypes;
    std::set<std::string> doneMimeTypes;

    while ((format = EnumClipboardFormats(format)) != 0)
    {
        if (format == CF_UNICODETEXT)
        {
            mimeTypes.push_back("text/plain;charset=utf-8");
            doneMimeTypes.insert("text/plain;charset=utf-8");
        }
        else
        {
            auto mimeType = MIME_type_for_clipboard_format(format);

            if (mimeType != "" && doneMimeTypes.count(mimeType) == 0)
            {
                doneMimeTypes.insert(mimeType);
                mimeTypes.push_back(mimeType);
            }
        }
    }

    CloseClipboard();

    return mimeTypes;
}

// When the user copies files in Explorer, the clipboard holds the files as a CF_HDROP file list. It
// also holds the FileNameW format, which is the pathname of the first file only, in UTF-16 and
// followed by a null. The text/uri-list made from them holds the file: URI of each file, each
// followed by CRLF.
static bool getClipboardFilesAsUriList(std::vector<char>* pOutData)
{
    if (!try_open_clipboard(NULL))
        return false;

    std::vector<std::wstring> paths;
    if (HANDLE handle = GetClipboardData(CF_HDROP))
        paths = pathsInFileList(static_cast<HDROP>(handle));

    if (paths.empty())
    {
        if (HANDLE handle = GetClipboardData(RegisterClipboardFormatW(L"FileNameW")))
        {
            if (const wchar_t* wpath = (const wchar_t*)GlobalLock(handle))
            {
                // The block can be larger than the data in it, so the pathname ends at the first
                // null.
                std::wstring path(wpath, wcsnlen(wpath, GlobalSize(handle) / sizeof(wchar_t)));
                GlobalUnlock(handle);
                if (!path.empty())
                    paths.push_back(std::move(path));
            }
        }
    }
    CloseClipboard();

    if (paths.empty())
        return false;

    std::string uriList;
    for (const std::wstring& path : paths)
        uriList += pathToURI(Poco::Path(Util::wide_string_to_string(path))) + "\r\n";
    pOutData->assign(uriList.begin(), uriList.end());

    return true;
}

static bool clipboardProviderGetData(const char* pMimeType, std::vector<char>* pOutData)
{
    if (std::strcmp(pMimeType, "text/uri-list") == 0)
        return getClipboardFilesAsUriList(pOutData);

    auto formats = clipboard_formats_for_MIME_type(pMimeType);

    if (formats.size() == 0)
        return false;

    if (!try_open_clipboard(NULL))
        return false;

    for (const auto& format : formats)
    {
        HANDLE handle;

        if (format == CF_UNICODETEXT)
        {
            handle = GetClipboardData(format);
            if (!handle)
                continue;
            wchar_t* wtext = (wchar_t*)GlobalLock(handle);
            if (!wtext)
                continue;

            std::string text = Util::wide_string_to_string(std::wstring(wtext));
            GlobalUnlock(handle);
            (*pOutData).resize(text.length());
            std::memcpy((*pOutData).data(), text.c_str(), text.length());

            CloseClipboard();

            return true;
        }

        handle = GetClipboardData(format);
        if (!handle)
            continue;
        size_t size = GlobalSize(handle);
        const char* source = (const char*)GlobalLock(handle);
        if (!source)
            continue;
        std::string fragment;
        if (format == RegisterClipboardFormatW(L"HTML Format"))
        {
            fragment = get_html_clipboard_fragment(source).c_str();
            source = fragment.c_str();
            size = std::strlen(source);
        }
        (*pOutData).resize(size);
        std::memcpy((*pOutData).data(), source, size);
        GlobalUnlock(handle);
        CloseClipboard();

        return true;
    }

    CloseClipboard();
    return false;
}


// Install the process-global clipboard provider (declared in windows.hpp). After this the engine
// advertises formats on copy and reads the clipboard on paste through the callbacks above, using
// one shared clipboard for every document.
void install_clipboard_provider(COKit& kitOffice)
{
    office = &kitOffice;

    static COKitClipboardProvider provider{};
    provider.advertiseToPlatform = clipboardProviderAdvertise;
    provider.ownsClipboard = clipboardProviderOwns;
    provider.getMimeTypes = clipboardProviderGetMimeTypes;
    provider.getDataForMimeType = clipboardProviderGetData;
    kitOffice.installClipboardProvider(&provider);
}

// The engine asks the user for a file. Copy the details, hand them to the hidden owner
// window's thread, and let CODA_WM_SHOWFILEPICKER show the dialog there; the completion
// answers from the dialog result.
static void filePickerProviderPick(const char* pTitle, const COKitFilePickerFilter* pFilters,
                                   size_t nFilters,
                                   void (*pfnPicked)(void* pContext, const char* pUrl),
                                   void* pContext)
{
    auto request = std::make_unique<FilePickerRequest>();
    if (pTitle)
        request->title = Util::string_to_wide_string(pTitle);
    for (size_t i = 0; i < nFilters; ++i)
        request->filters.push_back({ Util::string_to_wide_string(pFilters[i].pName),
                                     Util::string_to_wide_string(pFilters[i].pWildcards) });
    request->pfnPicked = pfnPicked;
    request->pContext = pContext;

    PostMessageW(hiddenOwnerWindow, CODA_WM_SHOWFILEPICKER,
                 reinterpret_cast<WPARAM>(request.release()), 0);
}

void install_filepicker_provider(COKit& kitOffice)
{
    static COKitFilePickerProvider provider{};
    provider.pick = filePickerProviderPick;
    kitOffice.installFilePickerProvider(&provider);
}

void materialize_clipboard_formats()
{
    static bool beenHere = false;

    if (beenHere)
        return;

    beenHere = true;

    if (GetClipboardOwner() == hiddenOwnerWindow)
    {
        if (try_open_clipboard(hiddenOwnerWindow))
        {
            // Collect the promised formats first, then render each. Do not probe with
            // GetClipboardData here(): on a still-promised format it would send
            // WM_RENDERFORMAT again. Re-setting an already-rendered format is harmless.
            std::vector<UINT> formats;
            UINT format = 0;
            while ((format = EnumClipboardFormats(format)) != 0)
                formats.push_back(format);
            for (UINT f : formats)
            {
                std::string mimeType = MIME_type_for_clipboard_format(f);
                if (mimeType.empty())
                    continue;
                HANDLE hData = copyEngineClipboardData(f, mimeType);
                if (hData)
                    SetClipboardData(f, hData);
            }
            CloseClipboard();
        }
    }
}

static void processMessage(DocumentTab& data, wil::unique_cotaskmem_string& message)
{
    std::wstring s(message.get());
    LOG_TRC(Util::wide_string_to_string(s));
    if (s.starts_with(L"MSG "))
    {
        s = s.substr(4);
        if (s == L"HULLO")
        {
            // If displaying the starter screen, do nothing
            if (data.mode == DocumentMode::STARTER)
                return;

            do_hullo_handling_things(data);
        }
        else if (s == L"WELCOME")
        {
            do_welcome_handling_things(data);
        }
        else if (s == L"FILEDRAGENTER")
        {
            // A drag carrying files has arrived over the web view. Take the drop away from the web
            // view, so that the app gets to see what was dropped. Repeats while the drag is still
            // in the part of the window the web view has, and stop once the overlay is up.
            if (WindowState* window = findWindow(data.hWnd))
                showDropOverlay(*window);
        }
        else if (s == L"BYE")
        {
            do_bye_handling_things(data);
        }
        else if (s == L"PRINT")
        {
            do_print(data.appDocId);
        }
        else if (s.starts_with(L"TEXTCLIPBOARD "))
        {
            // A plain-text copy from the web UI (for example the About dialog), not document
            // content, so it does not go through the engine's clipboard. Own it with the same
            // hidden window the provider uses, so no document window ever owns the clipboard.
            std::wstring text = s.substr(14);
            if (try_open_clipboard(hiddenOwnerWindow))
            {
                EmptyClipboard();
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t));
                if (hMem)
                {
                    memcpy(GlobalLock(hMem), text.c_str(), (text.size() + 1) * sizeof(wchar_t));
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_UNICODETEXT, hMem);
                }
                CloseClipboard();
            }
        }
        else if (s.starts_with(L"TABTHUMBNAIL "))
        {
            // An image of what the document shows, as a "data:image/png;base64," URL. Anything
            // else is ignored.
            const std::string dataUrl = Util::wide_string_to_string(s.substr(13));
            constexpr std::string_view prefix = "data:image/png;base64,";
            if (!dataUrl.starts_with(prefix))
                return;
            std::string png = Util::base64Decode(dataUrl.substr(prefix.size()));
            if (png.empty())
                return;

            // The strip only needs to know that a tab has an image, so later images skip the
            // push.
            const bool isFirstThumbnail = data.thumbnailPng.empty();
            data.thumbnailPng = std::move(png);
            WindowState* window = findWindow(data.hWnd);
            if (isFirstThumbnail && window)
                pushTabsToStrip(*window);
            if (hoverCard.tabId == data.tabId && IsWindowVisible(hoverCard.hWnd))
                showHoverCard();
        }
        else if (s.starts_with(L"HYPERLINK "))
        {
            do_open_hyperlink(data.hWnd, s.substr(10));
        }
        else if (s == L"LICENSE")
        {
            std::wstring licensePath = Util::string_to_wide_string(app_installation_path + "..\\LICENSE.html");
            ShellExecuteW(nullptr, L"open", licensePath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        }
        else if (s == L"EXCHANGEMONITORS")
        {
            exchangeMonitors(data);
        }
        else if (s.starts_with(L"FULLSCREENPRESENTATION "))
        {
            WindowState* window = findWindow(data.hWnd);
            if (!window)
                return;

            window->isPresFullScreen = s.substr(23) == L"true";
            if (window->isPresFullScreen)
                arrangePresentationWindows(data);
            else
                leave_full_screen(*window);
        }
        else if (s == L"SYNCSETTINGS")
        {
            Desktop::syncSettings([&data](const std::vector<char>& buf) {
                send2JS(data, buf.data(), buf.size());
            });
        }
        else if (s.starts_with(L"UPLOADSETTINGS "))
        {
            uploadAndApplySettings(Util::wide_string_to_string(s.substr(strlen("UPLOADSETTINGS "))));
        }
        else if (s.starts_with(L"SETDARKMODE "))
        {
            Desktop::setDarkMode(s.substr(strlen("SETDARKMODE ")) == L"true");
            applyTitleBarThemeToAllWindows();

            // The tab strip is a page of its own and follows along.
            for (auto& i : windows)
                notifyStrip(i.second, "themeChanged", currentStripTheme());
        }
        else if (s.starts_with(L"COMMANDSTATECHANGED "))
        {
            Poco::JSON::Object::Ptr object;
            if (!JsonUtil::parseJSON(Util::wide_string_to_string(s), object))
                return;

            if (object->optValue<std::string>("commandName", std::string()) !=
                ".uno:ModifiedStatus")
                return;

            const bool modified =
                object->optValue<std::string>("state", std::string()) == "true";
            if (modified == data.isModified)
                return;

            // The tab shows a document with unwritten changes differently.
            data.isModified = modified;
            if (WindowState* window = findWindow(data.hWnd))
                pushTabsToStrip(*window);
        }
        else if (s.starts_with(L"downloadas "))
        {
            // "downloadas name=document.rtf id=export format=rtf options="
            auto const ns = Util::wide_string_to_string(s);
            auto const tokens = StringVector::tokenize(ns);
            std::string name;
            if (!COOLProtocol::getTokenString(tokens, "name", name))
            {
                LOG_ERR("No name parameter in message '" << ns << "'");
                return;
            }
            auto dot = name.find_last_of('.');
            if (dot == std::string::npos || dot == name.length() - 1)
            {
                LOG_ERR("No file name extension in '" << ns << "'");
                return;
            }
            auto const extension = name.substr(dot + 1);
            auto const basename = data.filenameAndUri.filename.substr(
                0, data.filenameAndUri.filename.find_last_of('.'));
            auto filenameAndUri = fileSaveDialog(basename + "." + extension,
                                                 "",
                                                 {
                                                     {
                                                         Util::string_to_wide_string(extension).c_str(),
                                                         Util::string_to_wide_string("*." + extension).c_str()
                                                     }
                                                 });

            if (filenameAndUri.filename != "")
                DocumentData::get(data.appDocId).loKitDocument->saveAs(filenameAndUri.uri.c_str(), extension.c_str(), nullptr);
        }
        else if (s.starts_with(L"exportfile "))
        {
            // "exportfile url=file:///C:/Users/.../tmp/image.png"
            auto const ns = Util::wide_string_to_string(s);
            auto const tokens = StringVector::tokenize(ns);
            std::string fileUrl;
            if (!COOLProtocol::getTokenString(tokens, "url", fileUrl))
            {
                LOG_ERR("No url parameter in message '" << ns << "'");
                return;
            }

            auto srcPath = Poco::URI(fileUrl).getPath();
            // The usual hack to get rid of the leading slash in what Poco::URI::getPath() returns,
            // like "/C:/Users/bob/AppData/Local/Temp/image.jpg".
            if (srcPath.length() > 4 && srcPath[0] == '/' && srcPath[2] == ':' && srcPath[3] == '/')
                srcPath = srcPath.substr(1);

            if (!std::filesystem::exists(srcPath))
            {
                LOG_ERR("exportfile: source file not found: " << srcPath);
                return;
            }

            auto const extension = Poco::Path(srcPath).getExtension();
            auto filenameAndUri = fileSaveDialog("image." + extension,
                                                 "",
                                                 {
                                                     {
                                                         Util::string_to_wide_string(extension).c_str(),
                                                         Util::string_to_wide_string("*." + extension).c_str()
                                                     }
                                                 });

            if (filenameAndUri.filename != "")
            {
                auto destPath = Poco::URI(filenameAndUri.uri).getPath();
                // As above
                if (destPath.length() > 4 && destPath[0] == '/' && destPath[2] == ':' && destPath[3] == '/')
                    destPath = destPath.substr(1);
                std::error_code ec;
                std::filesystem::copy_file(srcPath, destPath,
                                           std::filesystem::copy_options::overwrite_existing, ec);
                if (ec)
                    LOG_ERR("exportfile: failed to copy to '" << destPath << "': " << ec.message());
                else
                    LOG_INF("exportfile: saved image to " << destPath);
            }
        }
        else if (s.starts_with(L"extensionsavefile "))
        {
            // A JS extension (see browser Control.Extension.ts) generated a file
            // - e.g. LibreLogo's PICTURE "x.svg" export - and asked to save it.
            // The bytes travel base64 in the message; decode them and write to
            // the user's chosen path, reusing the exportfile save dialog.
            auto const ns = Util::wide_string_to_string(s);
            auto const tokens = StringVector::tokenize(ns);
            std::string encodedName, base64;
            COOLProtocol::getTokenString(tokens, "name", encodedName);
            if (!COOLProtocol::getTokenString(tokens, "data", base64))
            {
                LOG_ERR("extensionsavefile: no data= in message '" << ns << "'");
                return;
            }
            std::string name;
            Poco::URI::decode(encodedName, name);
            if (name.empty())
                name = "file";

            // Decode the base64 payload.
            DWORD cbBinary = 0;
            if (!CryptStringToBinaryA(base64.c_str(), static_cast<DWORD>(base64.size()),
                                      CRYPT_STRING_BASE64, nullptr, &cbBinary, nullptr, nullptr))
            {
                LOG_ERR("extensionsavefile: base64 decode sizing failed: " << GetLastError());
                return;
            }
            std::vector<BYTE> bytes(cbBinary);
            if (!CryptStringToBinaryA(base64.c_str(), static_cast<DWORD>(base64.size()),
                                      CRYPT_STRING_BASE64, bytes.data(), &cbBinary, nullptr, nullptr))
            {
                LOG_ERR("extensionsavefile: base64 decode failed: " << GetLastError());
                return;
            }

            auto const extension = Poco::Path(name).getExtension();
            auto filenameAndUri = fileSaveDialog(
                Poco::Path(name).getFileName(), "",
                extension.empty()
                    ? std::vector<COMDLG_FILTERSPEC>{}
                    : std::vector<COMDLG_FILTERSPEC>{
                          { Util::string_to_wide_string(extension).c_str(),
                            Util::string_to_wide_string("*." + extension).c_str() } });

            if (filenameAndUri.filename != "")
            {
                auto destPath = Poco::URI(filenameAndUri.uri).getPath();
                // The usual hack to strip the leading slash before the drive letter.
                if (destPath.length() > 4 && destPath[0] == '/' && destPath[2] == ':' && destPath[3] == '/')
                    destPath = destPath.substr(1);
                std::ofstream out(destPath, std::ios::binary | std::ios::trunc);
                if (!out || !out.write(reinterpret_cast<const char*>(bytes.data()), bytes.size()))
                    LOG_ERR("extensionsavefile: failed to write '" << destPath << "'");
                else
                    LOG_INF("extensionsavefile: saved " << destPath);
            }
        }
        else if (s.starts_with(L"loaddocument "))
        {
            // "loaddocument url=file:///path/to/file.ext"
            auto const ns = Util::wide_string_to_string(s);
            auto const tokens = StringVector::tokenize(ns);
            std::string url;
            if (!COOLProtocol::getTokenString(tokens, "url", url))
            {
                LOG_ERR("No url parameter in message '" << ns << "'");
                return;
            }
            // Close the existing fakesocket
            if (data.fakeClientFd != -1)
            {
                fakeSocketClose(data.fakeClientFd);
                data.fakeClientFd = -1;
            }

            // Close the existing forwarding thread
            if (data.app2js.joinable()) {
                fakeSocketClose(data.closeNotificationPipeForForwardingThread[0]);
                data.app2js.join();
            }

            data.fakeClientFd = fakeSocketSocket();
            DocumentData::deallocate(data.appDocId);
            data.appDocId = generate_new_app_doc_id();
            auto path = Poco::URI(url).getPath();
            auto lastSlash = path.find_last_of('/');
            auto filename = path.substr(lastSlash + 1);
            const std::string previousUri = data.filenameAndUri.uri;
            data.filenameAndUri = { filename, Poco::URI(url).toString() } ;
            // The app picked the name of a new document, so only the name the user saved it as
            // belongs in the recent list.
            if (data.mode == DocumentMode::NEW)
            {
                recentFiles.remove(previousUri);
                data.mode = DocumentMode::EDIT;
            }
            recentFiles.add(data.filenameAndUri.uri);
            data.docType = docTypeFromFilename(filename);
            data.isModified = false;

            // Connect to COOLWSD
            int rc = fakeSocketConnect(data.fakeClientFd, coolwsd_server_socket_fd);
            if (rc == -1)
            {
                LOG_ERR("loaddocument: failed to connect fakesocket");
                return;
            }

            createAndStartMessagePumpThread(data);

            // Send the initial message with the new file URL and appDocId
            std::string message(data.filenameAndUri.uri + " " + std::to_string(data.appDocId));
            fakeSocketWriteQueue(data.fakeClientFd, message.c_str(), message.size());

            // Update window title and tab with new filename
            if (WindowState* window = findWindow(data.hWnd))
            {
                if (window->activeTabId == data.tabId)
                    SetWindowTextW(data.hWnd, windowTitleForTab(data).c_str());
                pushTabsToStrip(*window);
            }
        }
        else if (s == L"uno .uno:Open")
        {
            auto openResult = fileOpenDialog(
                L"",
                { { L"",
                    L"*.odt;*.fodt;*.docx;*.docm;*.doc;*.rtf;*.txt;*.md;*.ods;*.fods;*.xlsx;*.xlsm;*.xls;*.odp;*.fodp;*.pptx;*.pptm;*.ppsx;*.ppt" },
                  { L"", L"*.*" } },
                true);
            if (openResult.size() > 0)
            {
                for (const auto& i: openResult)
                    filenamesAndUrisToOpen.push_back(i);

                load_next_document();

                // The picked document opens in a tab of its own; return the
                // originating document to its document view.
                if (data.mode != DocumentMode::STARTER)
                    PostMessageW(data.hWnd, CODA_WM_EXECUTESCRIPT,
                                 (WPARAM)_strdup("window.app?.map?.backstageView?.returnToDocumentView()"),
                                 (LPARAM)data.tabId);
            }
            // Close the starter window
            if (data.mode == DocumentMode::STARTER)
                PostMessageW(data.hWnd, CODA_WM_CLOSETAB, (WPARAM)data.tabId, 0);
        }
        else if (s == L"uno .uno:SaveAs")
        {
            auto loKitDoc = DocumentData::get(data.appDocId).loKitDocument;
            const COKitDocumentType docType = loKitDoc->getDocumentType();
            const auto formats = getSaveAsFormats(docType);
            auto filenameAndUri = fileSaveDialog("", "", formats);

            if (filenameAndUri.filename != "")
            {
                auto dot = filenameAndUri.filename.find_last_of('.');
                if (dot == std::string::npos || dot == filenameAndUri.filename.length() - 1)
                {
                    LOG_ERR("No file name extension in '" << filenameAndUri.filename << "'");
                    return;
                }
                // Send saveas command to COOLWSD with the selected format
                std::string saveasCmd = "saveas url=" + filenameAndUri.uri +
                    " format=" + filenameAndUri.filename.substr(dot + 1) +
                    " options=";
                fakeSocketWriteQueue(data.fakeClientFd, saveasCmd.c_str(), saveasCmd.size());
            }
        }
        else if (s == L"uno .uno:CloseWin")
        {
            PostMessageW(data.hWnd, CODA_WM_CLOSETAB, (WPARAM)data.tabId, 0);
        }
        else if (s == L"uno .uno:Quit")
        {
            // Ask each window to close, the same way the window's own close button does. The
            // messages are queued, so the windows are all still up when this returns. The
            // application exits when the last one has gone.
            for (const auto& entry : windows)
                PostMessageW(entry.first, WM_CLOSE, 0, 0);
        }
        else if (s.starts_with(L"newdoc "))
        {
            auto const ns = Util::wide_string_to_string(s);
            auto const tokens = StringVector::tokenize(ns);
            std::string typeToken, templateToken, basenameToken;
            if (!COOLProtocol::getTokenString(tokens, "type", typeToken))
            {
                LOG_ERR("No type parameter in message '" << ns << "'");
                return;
            }
            if (!COOLProtocol::getTokenString(tokens, "basename", basenameToken))
            {
                LOG_ERR("No basename parameter in message '" << ns << "'");
                return;
            }
            DocumentType type;
            if (typeToken == "writer")
                type = DocumentType::TEXT;
            else if (typeToken == "calc")
                type = DocumentType::SPREADSHEET;
            else if (typeToken == "impress")
                type = DocumentType::PRESENTATION;
            else if (typeToken == "draw")
                type = DocumentType::DRAWING;
            else
                fatal("Unexpected type in newdoc message");

            // This might leave templateToken empty if it is an old-style newdoc message with just
            // the type parameter.
            COOLProtocol::getTokenString(tokens, "template", templateToken);
            auto newDocument = new_document(type, templateToken, basenameToken);
            if (newDocument != L"")
            {
                Poco::Path path = Poco::Path(Util::wide_string_to_string(newDocument));
                openCOOLWindow({ path.getFileName(), Poco::URI(path).toString() }, DocumentMode::NEW);
            }
            if (data.mode == DocumentMode::STARTER)
                PostMessageW(data.hWnd, CODA_WM_CLOSETAB, (WPARAM)data.tabId, 0);
        }
        else if (s.starts_with(L"opendoc "))
        {
            auto const ns = Util::wide_string_to_string(s);
            auto const tokens = StringVector::tokenize(ns);

            // Despite the name, the "file" parameter in the opendoc message is a file: URI, not a
            // pathname. Which is good as it reduces the character set confusion possibilities.
            std::string fileToken;
            if (!COOLProtocol::getTokenString(tokens, "file", fileToken))
            {
                LOG_ERR("No file parameter in message '" << ns << "'");
                return;
            }

            // For some reason, the URI has been pointlessly percent-re-encoded.
            fileToken = Uri::decode(fileToken);

            std::vector<std::string> segments;
            Poco::URI(fileToken).getPathSegments(segments);

            if (segments.empty())
            {
                LOG_ERR("Weird file parameter in message '" << ns << "'");
                return;
            }

            filenamesAndUrisToOpen.push_back({ segments.back(), fileToken });
            load_next_document();
            // Close the starter window
            if (data.mode == DocumentMode::STARTER)
                PostMessageW(data.hWnd, CODA_WM_CLOSETAB, (WPARAM)data.tabId, 0);
        }
        else
        {
            do_other_message_handling_things(data, Util::wide_string_to_string(s).c_str());
        }
    }
    else if (s.starts_with(L"CALL "))
    {
        s = s.substr(5);
        std::wstringstream ss(s);
        int id;
        ss >> id;
        s = s.substr(s.find_first_of(L' ') + 1);

        if (s == L"GETRECENTDOCS")
        {
            do_getrecentdocs(data, id);
        }
        else if (s.starts_with(L"FETCHSETTINGSFILE "))
        {
            auto result = Desktop::fetchSettingsFile(Util::wide_string_to_string(s.substr(strlen("FETCHSETTINGSFILE "))));
            if (!result.content.empty())
            {
                // The JS caller reads result.content, so the reply is an object.
                Poco::JSON::Object::Ptr reply = new Poco::JSON::Object();
                reply->set("fileName", result.fileName);
                reply->set("mimeType", result.mimeType);
                reply->set("content", result.content);
                postReplyToCall(data.hWnd, data.tabId, id, reply);
            }
        }
        else if (s == L"FETCHSETTINGSCONFIG")
        {
            postReplyToCall(data.hWnd, data.tabId, id, Desktop::fetchSettingsConfig());
        }
        else if (s.starts_with(L"FETCHAIMODELS "))
        {
            // The Options dialog asks for an AI provider's model list. This
            // performs a network request, so run it off the message thread and
            // reply asynchronously, mirroring the Qt and macOS apps.
            const std::string payload =
                Util::wide_string_to_string(s.substr(strlen("FETCHAIMODELS ")));
            const HWND hWnd = data.hWnd;
            const int tabId = data.tabId;
            std::thread(
                [payload, hWnd, tabId, id]()
                {
                    ProcUtil::setThreadName("aimodels");
                    const std::string result = fetchAIModels(payload);
                    postReplyToCall(hWnd, tabId, id, result);
                })
                .detach();
        }
        else
            LOG_ERR("Unhandled CALL message: " + Util::wide_string_to_string(s));
    }
    else if (s.starts_with(L"ERR "))
    {
        LOG_ERR("From JS: " + Util::wide_string_to_string(s));
    }
    else if (s.starts_with(L"DBG "))
    {
        LOG_DBG("From JS: " + Util::wide_string_to_string(s));
    }
}

extern "C" BOOLEAN WINAPI GetUserNameExW(
    ULONG NameFormat,   // EXTENDED_NAME_FORMAT underlying type
    LPWSTR lpNameBuffer,
    PULONG nSize
);

static const char* getUserName()
{
    static wchar_t buffer[256];
    static std::string userNameStorage;
    DWORD size = sizeof(buffer) / sizeof(wchar_t);

    // Try full display name
    if (GetUserNameExW(3 /* NameDisplay */, buffer, &size)) {
        if (buffer[0] != L'\0') {
            userNameStorage = Util::wide_string_to_string(std::wstring(buffer));
            return userNameStorage.c_str();
        }
    }

    // Reset size before next call
    size = sizeof(buffer) / sizeof(wchar_t);

    // Fallback: login name
    if (GetUserNameW(buffer, &size)) {
        if (buffer[0] != L'\0') {
            userNameStorage = Util::wide_string_to_string(std::wstring(buffer));
            return userNameStorage.c_str();
        }
    }

    return nullptr;
}

// These functions in the Desktop namespace are called from SettingsStorage.cpp. Unclear whether
// will be needed in the end. The name comes from CODA-Q. Should ideally be changed to FileUtil,
// perhaps.

// Expected to return the folder where installed data for the app is stored. Should not end with a
// slash (or backslash). Note the impedance mismatch with our app_installation_path.
std::string Desktop::getDataDir()
{
    std::string result = app_installation_path;
    if (!(result.ends_with("/") || result.ends_with("\\")))
        result += "/";
    result += "..";
    return result;
}

Poco::Path Desktop::getConfigPath()
{
    return Poco::Path(localAppData);
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int showWindowMode)
{
    appInstance = hInstance;
    appShowMode = showWindowMode;

    user_name = getUserName();

    wchar_t fileName[1000];
    GetModuleFileNameW(NULL, fileName, sizeof(fileName) / sizeof(fileName[0]));
    app_installation_path = app_exe_path = Util::wide_string_to_string(std::wstring(fileName));
    app_installation_path.resize(app_installation_path.find_last_of(L'\\') + 1);
    app_installation_uri = Poco::URI(Poco::Path(app_installation_path)).toString();

#if ENABLE_DEBUG
    // cool/ next to the install root is the vcxproj's robocopy of browser/dist/:
    Extensions::synthesizeBuiltinExtensionsIndex(app_installation_path + "..\\cool\\extensions");
#endif

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    appName = Util::string_to_wide_string(APP_NAME);

    if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
        fatal("CoInitializeEx() failed");

    // Drag and drop needs the OLE libraries on top of COM. The drop target that DragAcceptFiles()
    // installs to turn a drop into a WM_DROPFILES message is registered through RegisterDragDrop(),
    // which works only once OLE has been initialized. The apartment stays as CoInitializeEx() set it
    // up above.
    if (!SUCCEEDED(OleInitialize(NULL)))
        fatal("OleInitialize() failed");

    primaryMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

    UINT32 length = 0;
    LONG rc = GetCurrentApplicationUserModelId(&length, NULL);
    if (rc == ERROR_INSUFFICIENT_BUFFER)
    {
        appUserModelId.resize(length);
        GetCurrentApplicationUserModelId(&length, appUserModelId.data());
    }
    else
    {
        appUserModelId = Util::string_to_wide_string(APP_VENDOR) + L"." + appName;
        rc = SetCurrentProcessExplicitAppUserModelID(appUserModelId.c_str());
    }

    PWSTR appDataFolder;
    SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appDataFolder);
    localAppData = Util::wide_string_to_string(std::wstring(appDataFolder) + L"\\" + appName);
    CoTaskMemFree(appDataFolder);

    // Wire up the ODF IFilter into Windows Search and the FullDetails /
    // PreviewDetails strings the Properties dialog needs, both under
    // HKCU\Software\Classes. See registerOdfShellExtensions() for the
    // silo-escape mechanism. Idempotent on every launch.
    registerOdfShellExtensions();

    // A "LANG" environment variable is not a thing on Windows, but check
    // for a such anyway, for easier testing.
    auto langEnv = std::getenv("LANG");

    wchar_t bcp47[LOCALE_NAME_MAX_LENGTH];

    if (langEnv)
        uiLanguage = langEnv;
    else if (LCIDToLocaleName(MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT),
                              bcp47, LOCALE_NAME_MAX_LENGTH, 0))
        uiLanguage = Util::wide_string_to_string(bcp47);

    // Allow overriding log level in the debugger. Note that logging *always* goes just to
    // OutputDebugString() if running under a debugger. Never to stderr or stdout.
    const char* loglevel = std::getenv("CODA_LOGLEVEL");
    // COOLWSD_LOGLEVEL comes from the project file and differs for Debug and Release builds.
    if (!loglevel)
        loglevel = COOLWSD_LOGLEVEL;
    Log::initialize("CODA", loglevel);
    ProcUtil::setThreadName("main");

    loadTranslations();

    recentFiles.load(localAppData + "\\recentFiles.txt", 10);

    // Create a dummy hidden owner window so that the file open dialog can inherit its icon for the
    // task switcher (Alt-Tab) from it.

    {
        WNDCLASSEXW wcex;

        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = 0;
        wcex.lpfnWndProc = HiddenOwnerWndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CODA));
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = hiddenOwnerWindowClass;
        wcex.hIconSm = NULL;

        if (!RegisterClassExW(&wcex))
        {
            MessageBoxW(NULL, L"Call to RegisterClassExW failed", Util::string_to_wide_string(APP_NAME).c_str(), NULL);
            return 1;
        }

        hiddenOwnerWindow = CreateWindowW(hiddenOwnerWindowClass, L"CODAHiddenOwnerWindow",
                                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                          100, 100, NULL, NULL,
                                          hInstance, NULL);
        ShowWindow(hiddenOwnerWindow, SW_HIDE);
    }

    if (std::getenv("CODA_ENABLE_WEBDRIVER"))
        enableWebDriver = true;

    DocumentMode mode = DocumentMode::EDIT;
    if (__argc == 1 || wcscmp(__wargv[1], L"--disable-background-networking") == 0)
    {
        // No documents given on the command line, show the "Starter" "Backstage" dialog
        mode = DocumentMode::STARTER;
    }
    else
    {
        for (int i = 1; i < __argc; i++)
        {
            auto path = Poco::Path(Util::wide_string_to_string(__wargv[i]));
            filenamesAndUrisToOpen.push_back({ path.getFileName(), pathToURI(path) });
        }
    }

    fakeSocketSetLoggingCallback([](const std::string& line) { LOG_TRC_NOFILE(line); });

    // Give AIChatSession a native HTTP transport (no server-side AI proxy here).
    registerAIHttpTransport();

    coolwsdThread = std::thread(
        []
        {
            assert(coolwsd == nullptr);
            char* argv[2];
            // Yes, strdup() is apparently not standard, so MS wants you to call it as
            // _strdup(), and warns if you call strdup(). Sure, we could just silence such
            // warnings, but let's try to do as they want.
            argv[0] = _strdup("mobile");
            argv[1] = nullptr;
            ProcUtil::setThreadName("app");
            coolwsd = new COOLWSD();
            coolwsd->run(1, argv);
            // We will actually not get here, Util::forcedExit() will be called
            delete coolwsd;
            coolwsd = nullptr;
            LOG_TRC("COOLWSD completed");
        });

    {
        WNDCLASSEXW wcex;

        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CODA));
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        // The window frame shows through until a WebView2 covers it, so it follows the dark mode
        // the app will use, which is the saved choice where there is one and the system theme
        // otherwise. The class keeps the brush it is registered with, so a later change of theme
        // reaches the frame only on the next run.
        if (darkModeEnabled())
            wcex.hbrBackground = CreateSolidBrush(RGB(0x12, 0x12, 0x12));
        else
            wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = windowClass;
        wcex.hIconSm = NULL;

        if (!RegisterClassExW(&wcex))
        {
            MessageBoxW(NULL, L"Call to RegisterClassExW failed", Util::string_to_wide_string(APP_NAME).c_str(), NULL);
            return 1;
        }
   }

    {
        // The class of the transparent window that catches drops over the web view. It shows
        // nothing of its own, so it needs no window procedure or icon.
        WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };

        wcex.lpfnWndProc = DefWindowProcW;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wcex.lpszClassName = overlayWindowClass;

        if (!RegisterClassExW(&wcex))
            fatal("RegisterClassExW() for the drop overlay window failed");
    }

    {
        WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };

        // A drop shadow, as on the tooltips of the system.
        wcex.style = CS_DROPSHADOW;
        wcex.lpfnWndProc = HoverCardWndProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.lpszClassName = hoverCardWindowClass;

        if (!RegisterClassExW(&wcex))
            fatal("RegisterClassExW() for the tab hover card failed");
    }

    // Open the first document here, then open the rest one by one once the previous has loaded.
    if (mode == DocumentMode::STARTER)
        openCOOLWindow({ }, mode);
    else
        load_next_document();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
