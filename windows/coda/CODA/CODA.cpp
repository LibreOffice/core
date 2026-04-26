// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <map>
#include <regex>
#include <thread>
#include <vector>

#include <Windows.h>
#include <appmodel.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <shobjidl_core.h>

#include <wincrypt.h>

#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>

#include <wrl.h>
#include <wil/com.h>

#include <Poco/MemoryStream.h>

#include "litecask.h"

#include <common/Clipboard.hpp>
#include <common/LangUtil.hpp>
#include <common/Protocol.hpp>
#include <common/Log.hpp>
#include <common/MobileApp.hpp>
#include <common/RecentFiles.hpp>
#include <common/SettingsStorage.hpp>
#include <common/StringVector.hpp>
#include <common/Uri.hpp>
#include <net/FakeSocket.hpp>
#include <wsd/COOLWSD.hpp>

#include "Resource.h"
#include "windows.hpp"

// Note that all pathnames in this code that are plain narrow strings (std::string) are in UTF-8 and
// can thus *not* be used for actual file system operations. They must always be converted to UTF-16
// first with Util::string_to_wide_string(). URIs one the other hand are valid as such as narrow
// strings.

enum class ClipboardOp
{
    CUT,
    COPY,
    PASTE,
    PASTEUNFORMATTED,
    READ
};

enum class DocumentType
{
    TEXT,
    SPREADSHEET,
    PRESENTATION,
};

enum class DocumentMode { EDIT, NEW, WELCOME, STARTER };

struct FilenameAndUri
{
    // Just the basename (and extension), without folder
    std::string filename;

    // Complete file: URI
    std::string uri;
};

// Various document window specific data
struct WindowData
{
    HWND hWnd;
    HWND hConsoleWnd = 0;
    HWND hParentWnd = 0;
    int numMonitors = 0;
    RECT originalRect;
    LONG originalStyle;
    POINT previousSize; // After a WM_SIZE
    bool isFullScreen = false;
    bool isPresFullScreen = false;
    bool isConsole = false;
    int fakeClientFd;
    int closeNotificationPipeForForwardingThread[2];
    FilenameAndUri filenameAndUri;
    DocumentMode mode;
    int appDocId;
    DWORD lastOwnClipboardModification;
    DWORD lastAnyonesClipboardModification;
    wil::com_ptr<ICoreWebView2Controller> webViewController;
    wil::com_ptr<ICoreWebView2> webView;
    std::thread app2js;
};

struct PersistedDocumentWindowSize
{
    POINT size;
    WPARAM resizeType;
};

static std::map<HWND, WindowData> windowData;

static bool enableWebDriver = false;

static HINSTANCE appInstance;
static int appShowMode;

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

// The file open dialog dummy owner window class name.
static const wchar_t dummyWindowClass[] = L"CODADummyFileDialogOwnerWindow";
// The handle of that dummy window.
static HWND hiddenOwnerWindow;

static const int CODA_WM_EXECUTESCRIPT = WM_APP + 1;
static const int CODA_WM_LOADNEXTDOCUMENT = WM_APP + 2;

static HMONITOR primaryMonitor;

static litecask::Datastore persistentWindowSizeStore;
static bool persistentWindowSizeStoreOK;

static RecentFiles recentFiles;

static FilenameAndUri fileSaveDialog(const std::string& name,
                                     const std::string& folder,
                                     const std::vector<COMDLG_FILTERSPEC>& extensions);

static void openCOOLWindow(const FilenameAndUri& filenameAndUri, DocumentMode mode);

static std::set<std::string> currentlyOpenDocumens()
{
    std::set<std::string> result;

    for (const auto& i : windowData)
        result.insert(i.second.filenameAndUri.uri);

    return result;
}

// Vector of documents to open passed on the command line, or multiple documents to open selected in
// a file open dialog. We open the next one only as soon as the previous one has finished loading.
static std::deque<FilenameAndUri> filenamesAndUrisToOpen;

void load_next_document()
{
    if (filenamesAndUrisToOpen.size() > 0)
    {
        // Open the next document (from the command line or selected in the file open dialog), if
        // any.
        if (windowData.size() > 0)
        {
            // Post a message to one randomly selected window that can be a starter backstage window
            // or a document window, it doesn't matter, they use the same window procedure.
            PostMessageW(windowData.begin()->second.hWnd, CODA_WM_LOADNEXTDOCUMENT, 0, 0);
        }
        else
        {
            // We have no window open, so we can just call openCOOLWindow() directly.
            auto nextDocument = filenamesAndUrisToOpen.front();
            filenamesAndUrisToOpen.pop_front();
            openCOOLWindow(nextDocument, DocumentMode::EDIT);
        }
    }
}

static void processMessage(WindowData& data, wil::unique_cotaskmem_string& message);

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

static void send2JS(const HWND hWnd, const char* buffer, int length)
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

    PostMessageW(hWnd, CODA_WM_EXECUTESCRIPT, (WPARAM)wparam, 0);
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

    auto URI = Poco::URI(suggestedURI);
    auto path = URI.getPath();
    if (path.size() > 4 && path[0] == '/' && path[2] == ':' && path[3] == '/')
        path = path.substr(1);
    auto lastSlash = path.find_last_of('/');
    auto filename = path.substr(lastSlash + 1);
    auto folder = path.substr(0, lastSlash);
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

static void stopServer()
{
    SigUtil::requestShutdown();

    // Wait until coolwsdThread is torn down, so that we don't start cleaning up too early.
    coolwsdThread.join();
}

static void createAndStartMessagePumpThread(WindowData& data)
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
                        send2JS(data.hWnd, buf.data(), n);
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

static void do_hullo_handling_things(WindowData& data)
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

static void do_welcome_handling_things(WindowData& data)
{
    const auto welcomeSlideshow = Poco::Path(app_installation_path + "..\\cool\\welcome\\welcome-slideshow.odp");

    if (!Poco::File(welcomeSlideshow).exists())
        return;

    openCOOLWindow({ welcomeSlideshow.getFileName(), Poco::URI(welcomeSlideshow).toString() }, DocumentMode::WELCOME);
}

static void enter_full_screen(WindowData& data, HMONITOR monitor, bool saveRestoreInfo)
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
}

static void leave_full_screen(WindowData& data)
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
}

static void do_bye_handling_things(const WindowData& data)
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

    DocumentData::get(appDocId).loKitDocument->saveAs(tempFileUri.c_str(), "pdf", nullptr);

    STARTUPINFOW startupInfo{ sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION processInformation;

    if (!CreateProcessW(
            Util::string_to_wide_string(app_installation_path + "..\\PrintPDFAndDelete.exe").c_str(),
            Util::string_to_wide_string("PrintPDFAndDelete " + tempFileUri).data(), NULL, NULL,
            TRUE, 0, NULL, NULL, &startupInfo, &processInformation))
        LOG_ERR("CreateProcess failed: " << GetLastError());
}

static void do_other_message_handling_things(const WindowData& data, const char* message)
{
    LOG_TRC_NOFILE("Handling other message:'" << message << "'");

    fakeSocketWriteQueue(data.fakeClientFd, message, strlen(message));
}

static void do_getrecentdocs(const WindowData& data, int id)
{
    PostMessageW(data.hWnd, CODA_WM_EXECUTESCRIPT,
                 (WPARAM)_strdup(("window.replyFromNativeToCall(" +
                                  std::to_string(id) +
                                  ", '" + recentFiles.serialiseFiltered(currentlyOpenDocumens()) + "')").c_str()), 0);
}

static void do_cut_or_copy(ClipboardOp op, WindowData& data)
{
    // Tell core to copy the selection into its internal clipboard
    DocumentData::get(data.appDocId).loKitDocument->postUnoCommand(".uno:Copy");

    size_t count = 0;
    char** mimeTypes = nullptr;
    size_t* sizes = nullptr;
    char** streams = nullptr;

    // Get core's internal clipboard
    DocumentData::get(data.appDocId).loKitDocument->getClipboard(nullptr, &count, &mimeTypes, &sizes,
                                                            &streams);
    if (!OpenClipboard(NULL))
        return;

    if (!EmptyClipboard())
    {
        CloseClipboard();
        return;
    }

    for (int i = 0; i < count; i++)
    {
        // We check whether there is a corresponding standard or well-known Windows clipboard
        // format.
        int format = 0;
        int format2 = 0;
        int wformat = 0;

        if (strcmp(mimeTypes[i], "text/plain;charset=utf-8") == 0)
            wformat = CF_UNICODETEXT;
        else if (strcmp(mimeTypes[i], "text/rtf") == 0)
            format = RegisterClipboardFormatW(L"Rich Text Format");
        else if (strcmp(mimeTypes[i], "text/html") == 0)
            format = RegisterClipboardFormatW(L"HTML (HyperText Markup Language)");
        else if (strcmp(mimeTypes[i], "image/png") == 0)
        {
            // For PNG two clipboard formats seem to occur
            format = RegisterClipboardFormatW(L"image/png");
            format2 = RegisterClipboardFormatW(L"PNG");
        }
        else if (strcmp(mimeTypes[i], "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"") == 0)
            format = RegisterClipboardFormatW(L"Star Embed Source (XML)");
        else if (std::string(mimeTypes[i]).starts_with("application/x-openoffice-objectdescriptor-xml;"))
            format = RegisterClipboardFormatW(L"Star Object Descriptor (XML)");

        if (wformat == CF_UNICODETEXT)
        {
            std::wstring wtext =
                sizes[i] ? Util::string_to_wide_string(std::string(streams[i], sizes[i])) : L"";
            // CF_UNICODETEXT *must* have a terminating zero wchar_t
            const int byteSize = wtext.size() * 2 + 2;
            HANDLE hglData = GlobalAlloc(GMEM_MOVEABLE, byteSize);
            if (hglData)
            {
                wchar_t* wcopy = (wchar_t*)GlobalLock(hglData);
                memcpy(wcopy, wtext.c_str(), byteSize - 2);
                wcopy[wtext.size()] = 0;
                GlobalUnlock(hglData);
                SetClipboardData(CF_UNICODETEXT, hglData);
            }
        }
        else if (format)
        {
            HANDLE hglData = GlobalAlloc(GMEM_MOVEABLE, sizes[i]);
            if (hglData)
            {
                char* copy = (char*)GlobalLock(hglData);
                memcpy(copy, streams[i], sizes[i]);
                GlobalUnlock(hglData);
                SetClipboardData(format, hglData);
            }
            if (format2)
            {
                // Must allocate another handle for the other format
                HANDLE hglData2 = GlobalAlloc(GMEM_MOVEABLE, sizes[i]);
                if (hglData2)
                {
                    char* copy2 = (char*)GlobalLock(hglData2);
                    memcpy(copy2, streams[i], sizes[i]);
                    GlobalUnlock(hglData2);
                    SetClipboardData(format2, hglData2);
                }
            }
        }
    }
    CloseClipboard();

    data.lastOwnClipboardModification = GetClipboardSequenceNumber();

    if (op == ClipboardOp::CUT)
        DocumentData::get(data.appDocId).loKitDocument->postUnoCommand(".uno:Cut");
}

static std::wstring get_clipboard_format_name(UINT format)
{
    const int NNAME{ 1000 };
    wchar_t name[NNAME];
    int nwc = GetClipboardFormatNameW(format, name, NNAME);
    if (nwc)
    {
        name[nwc] = 0;
        return name;
    }
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

static void do_paste_or_read(ClipboardOp op, WindowData& data)
{
    if (data.lastAnyonesClipboardModification > data.lastOwnClipboardModification)
    {
        if (!OpenClipboard(NULL))
            return;

        std::vector<const char*> mimeTypes;
        std::vector<size_t> sizes;
        std::vector<const char*> streams;

        UINT format = 0;

        std::set<std::string> doneMimeTypes;

        while ((format = EnumClipboardFormats(format)) != 0)
        {
            if (format == CF_UNICODETEXT)
            {
                HANDLE data = GetClipboardData(format);
                if (!data)
                    continue;
                wchar_t* wtext = (wchar_t*)GlobalLock(data);
                if (!wtext)
                    continue;
                std::string text = Util::wide_string_to_string(std::wstring(wtext));
                GlobalUnlock(data);

                mimeTypes.push_back(_strdup("text/plain;charset=utf-8"));
                doneMimeTypes.insert("text/plain;charset=utf-8");
                sizes.push_back(text.size());
                streams.push_back(_strdup(text.c_str()));
            }
            else
            {
                auto name = get_clipboard_format_name(format);

                std::string mimeType;

                if (name == L"Star Embed Source (XML)")
                    mimeType = "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
                else if (name == L"Star Object Descriptor (XML)")
                    mimeType = "application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"";
                else if (name == L"PNG")
                    mimeType = "image/png";
                else if (name == L"Rich Text Format")
                    mimeType = "text/rtf";
                else if (name == L"text/rtf" || name == L"image/png" ||
                         // Not handled yet if ever by the rest of the code here and in core, I think,
                         // but why not be future-safe.
                         name == L"image/svg+xml")
                    mimeType = Util::wide_string_to_string(name);
                else if (name == L"HTML (HyperText Markup Language)" ||
                         name == L"HTML Format")
                    mimeType = "text/html";

                if (mimeType != "" && doneMimeTypes.count(mimeType) == 0)
                {
                    HANDLE data = GetClipboardData(format);
                    if (!data)
                        continue;
                    size_t size = GlobalSize(data);
                    const char* source = (const char*)GlobalLock(data);
                    if (!source)
                        continue;

                    std::string fragment;
                    if (name == L"HTML Format")
                    {
                        fragment = get_html_clipboard_fragment(source).c_str();
                        source = fragment.c_str();
                        size = strlen(source);
                    }

                    doneMimeTypes.insert(mimeType);

                    char* copy = (char*)std::malloc(size);
                    std::memcpy(copy, source, size);

                    GlobalUnlock(data);

                    mimeTypes.push_back(_strdup(mimeType.c_str()));
                    sizes.push_back(size);
                    streams.push_back(copy);
                }
            }
        }

        // Populate core's internal clipboard
        DocumentData::get(data.appDocId).loKitDocument->setClipboard(mimeTypes.size(), mimeTypes.data(),
                                                                     sizes.data(), streams.data());

        for (int i = 0; i < mimeTypes.size(); i++)
        {
            std::free((void*)mimeTypes[i]);
            std::free((void*)streams[i]);
        }

        CloseClipboard();
    }

    if (op == ClipboardOp::PASTE)
    {
        DocumentData::get(data.appDocId).loKitDocument->postUnoCommand(".uno:Paste");
    }
    else if (op == ClipboardOp::PASTEUNFORMATTED)
    {
        DocumentData::get(data.appDocId).loKitDocument->postUnoCommand(".uno:PasteUnformatted");
    }
}

static void do_clipboard_set(int appDocId, const char* text)
{
    size_t nData;
    std::vector<size_t> sizes;
    std::vector<const char*> mimeTypes;
    std::vector<const char*> streams;
    ClipboardData data;

    if (memcmp(text, "<!DOCTYPE html>", 15) == 0)
    {
        nData = 1;
        sizes.resize(1);
        sizes[0] = strlen(text);
        mimeTypes.resize(1);
        mimeTypes[0] = "text/html";
        streams.resize(1);
        streams[0] = text;
    }
    else
    {
        Poco::MemoryInputStream stream(text, strlen(text));
        data.read(stream);

        nData = data.size();
        sizes.resize(nData);
        mimeTypes.resize(nData);
        streams.resize(nData);

        for (size_t i = 0; i < nData; ++i)
        {
            sizes[i] = data._content[i].length();
            streams[i] = data._content[i].c_str();
            mimeTypes[i] = data._mimeTypes[i].c_str();
        }
    }

    DocumentData::get(appDocId).loKitDocument->setClipboard(nData, mimeTypes.data(), sizes.data(),
                                                            streams.data());
}

static void do_open_hyperlink(HWND hWnd, std::wstring url)
{
    ShellExecuteW(hWnd, NULL, url.c_str(), NULL, NULL, SW_SHOW);
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

static void exchangeMonitors(WindowData& data)
{
    Monitors monitors(getMonitors());
    if (monitors.size() < 2)
        return;

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

    leave_full_screen(data);

    size_t newPresentationMonitor = origPresentationMonitor;

    if (data.hConsoleWnd)
    {
        leave_full_screen(windowData[data.hConsoleWnd]);

        size_t newConsoleMonitor = (origConsoleMonitor + 1) % monitors.size();
        if (newConsoleMonitor == newPresentationMonitor)
            newPresentationMonitor = (newPresentationMonitor + 1) % monitors.size();

        enter_full_screen(windowData[data.hConsoleWnd], monitors[newConsoleMonitor].hMonitor, false);
    }
    else
    {
        newPresentationMonitor = (newPresentationMonitor + 1) % monitors.size();
    }

    enter_full_screen(data, monitors[newPresentationMonitor].hMonitor, false);
}

static std::string pathToURI(const Poco::Path& path)
{
    auto uri = Poco::URI(path);

    if (path.getNode() == "")
        return uri.toString();

    uri.setHost(path.getNode());
    return uri.toString();
}

static std::vector<FilenameAndUri> fileOpenDialog()
{
    IFileOpenDialog* dialog;

    if (!SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IFileOpenDialog, reinterpret_cast<void**>(&dialog))))
        fatal("CoCreateInstance(CLSID_FileOpenDialog) failed");

    COMDLG_FILTERSPEC filter[] = {
        { L"",
          L"*.odt;*.docx;*.doc;*.rtf;*.txt;*.md;*.ods;*.xlsx;*.xls;*.odp;*.pptx;*.ppt" },
        { L"", L"*.*" }
    };

    if (!SUCCEEDED(dialog->SetFileTypes(sizeof(filter) / sizeof(filter[0]), &filter[0])))
        fatal("dialog->SetFileTypes() failed");

    FILEOPENDIALOGOPTIONS options;
    if (SUCCEEDED(dialog->GetOptions(&options)))
    {
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

static std::vector<COMDLG_FILTERSPEC>getSaveAsFormats(int docType)
{
    std::vector<COMDLG_FILTERSPEC> result;

    if (docType == KIT_DOCTYPE_TEXT)
    {
        result.push_back({L"ODT", L"*.odt"});
        result.push_back({L"RTF", L"*.rtf"});
        result.push_back({L"DOCX", L"*.docx"});
        result.push_back({L"DOC", L"*.doc"});
    }
    else if (docType == KIT_DOCTYPE_SPREADSHEET)
    {
        result.push_back({L"ODS", L"*.ods"});
        result.push_back({L"XLSX", L"*.xlsx"});
        result.push_back({L"XLS", L"*.xls"});
    }
    else if (docType == KIT_DOCTYPE_PRESENTATION)
    {
        result.push_back({L"ODP", L"*.odp"});
        result.push_back({L"PPTX", L"*.pptx"});
        result.push_back({L"PPT", L"*.ppt"});
    }
    else if (docType == KIT_DOCTYPE_DRAWING)
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

static void arrangePresentationWindows(WindowData& data)
{
    Monitors monitors(getMonitors());
    data.numMonitors = monitors.size();

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

    leave_full_screen(data);
    if (data.hConsoleWnd)
        leave_full_screen(windowData[data.hConsoleWnd]);

    HMONITOR presenterMonitor = externalMonitor ? externalMonitor : laptopMonitor;

    enter_full_screen(data, presenterMonitor, true);

    if (data.hConsoleWnd)
    {
        if (externalMonitor)
            enter_full_screen(windowData[data.hConsoleWnd], laptopMonitor, true);
        else
            BringWindowToTop(data.hConsoleWnd);
    }
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
            {
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
            if (windowData[hWnd].webViewController != nullptr)
            {
                RECT bounds;
                GetClientRect(hWnd, &bounds);
                windowData[hWnd].webViewController->put_Bounds(bounds);

                if (!windowData[hWnd].isFullScreen &&
                    persistentWindowSizeStoreOK &&
                    (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED))
                {
                    std::vector<uint8_t> value(sizeof(PersistedDocumentWindowSize));
                    PersistedDocumentWindowSize* p = reinterpret_cast<PersistedDocumentWindowSize*>(value.data());
                    if (wParam == SIZE_RESTORED)
                    {
                        p->size.x = LOWORD(lParam);
                        p->size.y = HIWORD(lParam);
                        windowData[hWnd].previousSize = p->size;
                    }
                    else
                    {
                        p->size = windowData[hWnd].previousSize;
                    }
                    p->resizeType = wParam;
                    persistentWindowSizeStore.put(windowData[hWnd].filenameAndUri.uri.c_str(), value);
                }
            };
            break;

        case WM_SETFOCUS:
            if (windowData.count(hWnd) && windowData[hWnd].webViewController)
                windowData[hWnd].webViewController->MoveFocus(
                    COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
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
            auto& data = windowData[hWnd];
            if (data.hConsoleWnd || data.isPresFullScreen)
            {
                int numMonitors = getMonitors().size();
                if (data.numMonitors != numMonitors)
                    arrangePresentationWindows(data);
            }
        }
        break;

        case WM_CLOSE:
            if (windowData[hWnd].mode == DocumentMode::STARTER)
                ; // Nothing
            else if (!windowData[hWnd].isConsole)
            {
                do_bye_handling_things(windowData[hWnd]);

                DocumentData::deallocate(windowData[hWnd].appDocId);
            }
            else
            {
                auto& parent = windowData[windowData[hWnd].hParentWnd];
                leave_full_screen(parent);
                parent.hConsoleWnd = 0;
            }
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            if (windowData[hWnd].app2js.joinable())
                windowData[hWnd].app2js.join();
            if (DocumentData::count() == 0)
            {
                if (persistentWindowSizeStoreOK)
                {
                    persistentWindowSizeStoreOK = false;
                    persistentWindowSizeStore.close();
                }
                stopServer();
            }
            break;

        case WM_NCDESTROY:
        {
            auto it = windowData.find(hWnd);
            if (it != windowData.end())
            {
                if (it->second.isConsole)
                {
                    auto& data = it->second;
                    if (data.webViewController)
                    {
                        data.webViewController->Close();
                        data.webViewController = nullptr;
                    }
                    data.webView = nullptr;
                }
                windowData.erase(hWnd);
            }
            break;
        }

        case WM_CLIPBOARDUPDATE:
            windowData[hWnd].lastAnyonesClipboardModification = GetClipboardSequenceNumber();
            break;

        case CODA_WM_EXECUTESCRIPT:
            windowData[hWnd].webView->ExecuteScript(
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

        case CODA_WM_LOADNEXTDOCUMENT:
            if (filenamesAndUrisToOpen.size() > 0)
            {
                auto nextDocument = filenamesAndUrisToOpen.front();
                filenamesAndUrisToOpen.pop_front();
                openCOOLWindow(nextDocument, DocumentMode::EDIT);
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

static void openCOOLWindow(const FilenameAndUri& filenameAndUri, DocumentMode mode)
{
    bool havePersistedSize = false;

    int width, height;
    int welcomeX = CW_USEDEFAULT, welcomeY = CW_USEDEFAULT;
    bool maximize = false;

    if (mode != DocumentMode::WELCOME && mode != DocumentMode::STARTER && persistentWindowSizeStoreOK)
    {
        std::vector<uint8_t> value;
        if (persistentWindowSizeStore.get(filenameAndUri.uri.c_str(), value) == litecask::Status::Ok)
        {
            if (value.size() == sizeof(POINT))
            {
                // We used to store just the size
                const POINT* p = reinterpret_cast<POINT*>(value.data());
                width = p->x;
                height = p->y;
                havePersistedSize = true;
            }
            else if (value.size() == sizeof(PersistedDocumentWindowSize))
            {
                // Currently we also store the last wParam in the WM_SIZE message
                const PersistedDocumentWindowSize* p = reinterpret_cast<PersistedDocumentWindowSize*>(value.data());
                width = p->size.x;
                height = p->size.y;
                if (p->resizeType == SIZE_MAXIMIZED)
                    maximize = true;
                havePersistedSize = true;
            }
        }
    }

    if (!havePersistedSize)
    {
        // Set size of document window to be 90% of monitor width and height. For the welcome
        // slideshow always set width:height to 16:9 because we know it is that aspect ratio.

        // The welcome slideshow is displayed without decorations.

        // FIXME: Should we actually, at least for text documents, ideally peek into the document and
        // check what its page size is, and in the common case of a portrait orientation text document,
        // make the document window also (if the monitor is large enough) higher than wider? On small
        // monitors (1280x768 or less?) we should probably default to making the document window
        // full-screen?

        // FIXME: My initial assumption that the COOL window would open up on the monitor where the
        // file section dialog was is incorrect.

        MONITORINFO monitorInfo;

        monitorInfo.cbSize = sizeof(monitorInfo);
        if (GetMonitorInfoW(primaryMonitor, &monitorInfo))
        {
            if (mode == DocumentMode::WELCOME)
            {
                double aspectRatio =
                    (double)(monitorInfo.rcWork.right - monitorInfo.rcWork.left) / (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);
                if (aspectRatio < 16.0/9.0)
                {
                    width = 0.9 * (monitorInfo.rcWork.right - monitorInfo.rcWork.left);
                    welcomeX = monitorInfo.rcWork.left + 0.05 * (monitorInfo.rcWork.right - monitorInfo.rcWork.left);
                    height = width / (16.0/9.0);
                    welcomeY = monitorInfo.rcWork.top + ((monitorInfo.rcWork.bottom - monitorInfo.rcWork.top) - height) / 2;
                }
                else
                {
                    height = 0.9 * (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);
                    welcomeY = monitorInfo.rcWork.top + 0.05 * (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);
                    width = (16.0/9.0) * height;
                    welcomeX = monitorInfo.rcWork.left + ((monitorInfo.rcWork.right - monitorInfo.rcWork.left) - width) / 2;
                }
            }
            else
            {
                width = 0.9 * (monitorInfo.rcWork.right - monitorInfo.rcWork.left);
                height = 0.9 * (monitorInfo.rcWork.bottom - monitorInfo.rcWork.top);
            }
        }
        else
        {
            if (mode == DocumentMode::WELCOME)
            {
                width = 1280;
                height = 720;
            }
            else
            {
                width = 1200;
                height = 900;
            }
        }
    }

    HWND hWnd;
    if (mode == DocumentMode::WELCOME)
        hWnd = CreateWindowW(
            windowClass, Util::string_to_wide_string(APP_NAME).c_str(),
            WS_POPUP, welcomeX, welcomeY, width, height, NULL, NULL, appInstance,
            NULL);
    else if (mode == DocumentMode::STARTER)
        hWnd = CreateWindowW(
            windowClass, Util::string_to_wide_string(APP_NAME).c_str(),
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, appInstance,
            NULL);
    else
        hWnd = CreateWindowW(
            windowClass, Util::string_to_wide_string(filenameAndUri.filename + " - " APP_NAME).c_str(),
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, appInstance,
            NULL);

    auto& data = windowData[hWnd];
    data.hWnd = hWnd;
    data.previousSize.x = width;
    data.previousSize.y = height;
    data.isFullScreen = false;
    if (mode == DocumentMode::STARTER)
    {
        data.fakeClientFd = -1;
        data.appDocId = 0;
    }
    else
    {
        data.fakeClientFd = fakeSocketSocket();
        data.appDocId = generate_new_app_doc_id();
    }
    data.lastOwnClipboardModification = 0;
    data.lastAnyonesClipboardModification = 1;
    data.filenameAndUri = filenameAndUri;
    data.mode = mode;

    if (maximize)
        ShowWindow(hWnd, SW_MAXIMIZE);
    else
        ShowWindow(hWnd, appShowMode);
    UpdateWindow(hWnd);

    AddClipboardFormatListener(hWnd);

    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();

    // Required for instantiating new Web Workers, which otherwise fail with a
    // cross-origin SecurityError because file:// gets origin 'null'.
    std::wstring additionalArgs = L"--allow-file-access-from-files";
    if (enableWebDriver)
        additionalArgs += L" --remote-debugging-port=9222";
    options->put_AdditionalBrowserArguments(additionalArgs.c_str());

    CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        (Util::string_to_wide_string(localAppData) + L"\\UDF").c_str(),
        options.Get(),
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [&data](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
            {
                // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
                env->CreateCoreWebView2Controller(
                    data.hWnd,
                    Microsoft::WRL::Callback<
                        ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [&data, env](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (!controller)
                                return E_FAIL;

                            ICoreWebView2* webView;
                            controller->get_CoreWebView2(&webView);
                            data.webView = wil::com_ptr<ICoreWebView2>(webView);
                            data.webViewController = controller;

                            // Add a few settings for the webview
                            // The demo step is redundant since the values are the default settings
                            wil::com_ptr<ICoreWebView2Settings> settings;
                            webView->get_Settings(&settings);
                            settings->put_IsScriptEnabled(TRUE);
                            settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                            settings->put_IsWebMessageEnabled(TRUE);

                            // Resize WebView to fit the bounds of the parent window
                            RECT bounds;
                            GetClientRect(data.hWnd, &bounds);
                            data.webViewController->put_Bounds(bounds);

                            EventRegistrationToken token;

                            // Communication between host and web content
                            // Set an event handler for the host to return received message back to the web content
                            webView->add_WebMessageReceived(
                                Microsoft::WRL::Callback<
                                    ICoreWebView2WebMessageReceivedEventHandler>(
                                    [&data](
                                        ICoreWebView2* webView,
                                        ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
                                    {
                                        wil::unique_cotaskmem_string message;
                                        args->TryGetWebMessageAsString(&message);
                                        processMessage(data, message);
                                        return S_OK;
                                    })
                                    .Get(),
                                &token);

                            webView->add_ContainsFullScreenElementChanged(
                                Microsoft::WRL::Callback<ICoreWebView2ContainsFullScreenElementChangedEventHandler>(
                                    [&data](ICoreWebView2* sender, IUnknown* args) -> HRESULT
                                    {
                                        BOOL containsFullscreenElement;
                                        sender->get_ContainsFullScreenElement(&containsFullscreenElement);
                                        if (containsFullscreenElement)
                                        {
                                            HMONITOR monitor = MonitorFromWindow(data.hWnd, MONITOR_DEFAULTTONEAREST);
                                            enter_full_screen(data, monitor, true);
                                        }
                                        else
                                            leave_full_screen(data);
                                        return S_OK;
                                    })
                                    .Get(),
                                nullptr);

                            // New windows appear to need to reuse the original env of the parent, a good explanation
                            // of use at: https://github.com/MicrosoftEdge/WebView2Feedback/discussions/4501#discussioncomment-9215801
                            webView->add_NewWindowRequested(
                                Microsoft::WRL::Callback<ICoreWebView2NewWindowRequestedEventHandler>(
                                    [env, &data](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args)
                                    {
                                        wil::com_ptr<ICoreWebView2Deferral> deferral;
                                        args->GetDeferral(&deferral);

                                        data.hConsoleWnd = CreateWindowW(windowClass,
                                                Util::string_to_wide_string(APP_NAME).c_str(),
                                                WS_OVERLAPPEDWINDOW,
                                                CW_USEDEFAULT, CW_USEDEFAULT,
                                                800, 640, NULL, NULL, appInstance, NULL);

                                        auto& consoleData = windowData[data.hConsoleWnd];
                                        consoleData.hWnd = data.hConsoleWnd;
                                        consoleData.hParentWnd = data.hWnd;
                                        consoleData.isConsole = true;
                                        consoleData.previousSize.x = 800;
                                        consoleData.previousSize.y = 640;

                                        ShowWindow(data.hConsoleWnd, appShowMode);

                                        env->CreateCoreWebView2Controller(
                                            data.hConsoleWnd,
                                            Microsoft::WRL::Callback<
                                                ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                                                [&consoleData, &data, args, deferral](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                                                {
                                                    if (!controller)
                                                        return E_FAIL;

                                                    ICoreWebView2* webView;
                                                    controller->get_CoreWebView2(&webView);
                                                    consoleData.webView = wil::com_ptr<ICoreWebView2>(webView);

                                                    webView->add_WindowCloseRequested(
                                                        Microsoft::WRL::Callback<ICoreWebView2WindowCloseRequestedEventHandler>(
                                                            [&consoleData](ICoreWebView2* sender, IUnknown* args)
                                                            {
                                                                PostMessageW(consoleData.hWnd, WM_CLOSE, 0, 0);
                                                                return S_OK;
                                                            })
                                                            .Get(),
                                                        nullptr);

                                                    controller->put_IsVisible(TRUE);

                                                    consoleData.webViewController = controller;

                                                    // Resize WebView to fit the bounds of the parent window
                                                    RECT bounds;
                                                    GetClientRect(consoleData.hWnd, &bounds);
                                                    controller->put_Bounds(bounds);

                                                    args->put_NewWindow(consoleData.webView.get());
                                                    args->put_Handled(TRUE);
                                                    deferral->Complete();

                                                    arrangePresentationWindows(data);

                                                    return S_OK;
                                                })
                                                .Get());
                                            return S_OK;

                                        return S_OK;
                                    })
                                    .Get(),
                                nullptr);

                            std::string coolURL =
                                app_installation_uri + "../cool/cool.html?";
                            if (data.mode == DocumentMode::STARTER)
                                coolURL += "starterMode=true";
                            else
                            {
                                if (data.mode != DocumentMode::WELCOME)
                                    recentFiles.add(data.filenameAndUri.uri);
                                coolURL +=
                                    "file_path=" + data.filenameAndUri.uri +
                                    std::string("&permission=edit") +
                                    std::string("&appdocid=") + std::to_string(data.appDocId) +
                                    std::string("&userinterfacemode=notebookbar");
                            }

                            coolURL += "&lang=" + uiLanguage;
                            coolURL += "&dir=" + std::string(LangUtil::isRtlLanguage(uiLanguage) ? "rtl" : "");

                            if (!isLightTheme())
                                coolURL +=
                                    "&darkTheme=true";

                            if (data.mode != DocumentMode::STARTER)
                                coolURL +=
                                    std::string((data.mode == DocumentMode::NEW ? "&isnewdocument=true" : "")) +
                                    std::string((data.mode == DocumentMode::WELCOME ? "&welcome=true" : ""));

                            webView->Navigate(Util::string_to_wide_string(coolURL).c_str());
                            controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);

                            return S_OK;
                        })
                        .Get());
                return S_OK;
            })
            .Get());
}

static void processMessage(WindowData& data, wil::unique_cotaskmem_string& message)
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
        else if (s == L"BYE")
        {
            do_bye_handling_things(data);
        }
        else if (s == L"PRINT")
        {
            do_print(data.appDocId);
        }
        else if (s == L"CUT")
        {
            do_cut_or_copy(ClipboardOp::CUT, data);
        }
        else if (s == L"COPY" || s == L"CLIPBOARDWRITE")
        {
            do_cut_or_copy(ClipboardOp::COPY, data);
        }
        else if (s == L"PASTE")
        {
            do_paste_or_read(ClipboardOp::PASTE, data);
        }
        else if (s == L"PASTEUNFORMATTED")
        {
            do_paste_or_read(ClipboardOp::PASTEUNFORMATTED, data);
        }
        else if (s == L"CLIPBOARDREAD")
        {
            do_paste_or_read(ClipboardOp::READ, data);
        }
        else if (s.starts_with(L"TEXTCLIPBOARD "))
        {
            std::wstring text = s.substr(14);
            if (OpenClipboard(NULL))
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
        else if (s.starts_with(L"CLIPBOARDSET "))
        {
            do_clipboard_set(data.appDocId, Util::wide_string_to_string(s.substr(13)).c_str());
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
            data.isPresFullScreen = s.substr(23) == L"true";
            if (data.isPresFullScreen)
                arrangePresentationWindows(data);
            else
                leave_full_screen(data);
        }
        else if (s == L"SYNCSETTINGS")
        {
            Desktop::syncSettings([&data](const std::vector<char>& buf) {
                send2JS(data.hWnd, buf.data(), buf.size());
            });
        }
        else if (s.starts_with(L"UPLOADSETTINGS "))
        {
            Desktop::uploadSettings(Util::wide_string_to_string(s.substr(strlen("UPLOADSETTINGS "))));
        }
        else if (s.starts_with(L"PROCESSINTEGRATORADMINFILE "))
        {
            Desktop::processIntegratorAdminFile(Util::wide_string_to_string(s.substr(strlen("PROCESSINTEGRATORADMINFILE "))));
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
            data.filenameAndUri = { filename, Poco::URI(url).toString() } ;

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

            // Update window title with new filename
            SetWindowTextW(data.hWnd, Util::string_to_wide_string(data.filenameAndUri.filename + " - " APP_NAME).c_str());
        }
        else if (s == L"uno .uno:Open")
        {
            auto openResult = fileOpenDialog();
            if (openResult.size() > 0)
            {
                for (const auto& i: openResult)
                    filenamesAndUrisToOpen.push_back(i);

                load_next_document();
            }
            // Close the starter window
            if (data.mode == DocumentMode::STARTER)
                PostMessage(data.hWnd, WM_CLOSE, 0, 0);
        }
        else if (s == L"uno .uno:SaveAs")
        {
            auto loKitDoc = DocumentData::get(data.appDocId).loKitDocument;
            const int docType = loKitDoc->getDocumentType();
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
            PostMessageW(data.hWnd, WM_CLOSE, 0, 0);
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
                PostMessage(data.hWnd, WM_CLOSE, 0, 0);
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
                PostMessage(data.hWnd, WM_CLOSE, 0, 0);
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
                std::string resultAsJavaScriptData =
                    "{"
                      "fileName: '" + result.fileName + "'"
                      ","
                      "mimeType: '" + result.mimeType + "'"
                      ","
                      "content: '" + result.content + "'"
                    "}";

                PostMessageW(data.hWnd, CODA_WM_EXECUTESCRIPT,
                             (WPARAM)_strdup(("window.replyFromNativeToCall(" +
                                              std::to_string(id) +
                                              ", '" + resultAsJavaScriptData + "')").c_str()), 0);
            }
        }
        else if (s == L"FETCHSETTINGSCONFIG")
        {
            PostMessageW(data.hWnd, CODA_WM_EXECUTESCRIPT,
                         (WPARAM)_strdup(("window.replyFromNativeToCall(" +
                                          std::to_string(id) +
                                          ", '" + Desktop::fetchSettingsConfig() + "')").c_str()), 0);
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

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    appName = Util::string_to_wide_string(APP_NAME);

    if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
        fatal("CoInitializeEx() failed");

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

    persistentWindowSizeStoreOK =
        (persistentWindowSizeStore.open
         (Util::string_to_wide_string(localAppData +
                                      "\\persistentWindowSizes")) == litecask::Status::Ok);

    recentFiles.load(localAppData + "\\recentFiles.txt", 10);

    // Create a dummy hidden owner window so that the file open dialog can inherit its icon for the
    // task switcher (Alt-Tab) from it.

    {
        WNDCLASSEXW wcex;

        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = 0;
        wcex.lpfnWndProc = DefWindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CODA));
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = dummyWindowClass;
        wcex.hIconSm = NULL;

        if (!RegisterClassExW(&wcex))
        {
            MessageBoxW(NULL, L"Call to RegisterClassExW failed", Util::string_to_wide_string(APP_NAME).c_str(), NULL);
            return 1;
        }

        hiddenOwnerWindow = CreateWindowW(dummyWindowClass, L"CODAHiddenOwnerWindow",
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
            while (true)
            {
                coolwsd = new COOLWSD();
                coolwsd->run(1, argv);
                delete coolwsd;
                LOG_TRC("One run of COOLWSD completed");
            }
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
        if (isLightTheme())
            wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        else
            wcex.hbrBackground = CreateSolidBrush(RGB(0x12, 0x12, 0x12));
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = windowClass;
        wcex.hIconSm = NULL;

        if (!RegisterClassExW(&wcex))
        {
            MessageBoxW(NULL, L"Call to RegisterClassExW failed", Util::string_to_wide_string(APP_NAME).c_str(), NULL);
            return 1;
        }
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
