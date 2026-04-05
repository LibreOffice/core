/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs WebView Message Handler
 *
 * Routes JS -> C++ requests from the React UI via cefQuery().
 * Only handles document operations (getDocument, getSelection, applyEdit).
 * Chat/streaming is handled directly by React via HTTP/SSE to the Python agent.
 *
 * THREADING: cefQuery callbacks arrive on the CEF IO thread.
 *            Document operations MUST be dispatched to the VCL main thread
 *            via Application::PostUserEvent().
 *            m_pPanel is atomic — read on CEF IO thread, written on VCL thread.
 */

#ifdef HAVE_FEATURE_CEF

#include <officelabs/WebViewMessageHandler.hxx>
#include <officelabs/WebViewPanel.hxx>
#include <officelabs/DocumentController.hxx>

#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <tools/link.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/task/XRestartManager.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <rtl/bootstrap.hxx>

#include <functional>
#include <cstdlib>

#ifdef _WIN32
#include <prewin.h>
#include <windows.h>
#include <postwin.h>
#endif

namespace {

// Helper: invoke a std::function on VCL thread via PostUserEvent
void VclDispatchCb(void* pData, void*)
{
    auto* pFn = static_cast<std::function<void()>*>(pData);
    (*pFn)();
    delete pFn;
}

void postToVclThread(std::function<void()> fn)
{
    auto* pData = new std::function<void()>(std::move(fn));
    Application::PostUserEvent(LINK_NONMEMBER(pData, VclDispatchCb));
}

} // anonymous namespace

namespace officelabs {

namespace {

// Simple JSON string escaping (for building response JSON)
std::string escapeJson(const OUString& s)
{
    OString utf8 = OUStringToOString(s, RTL_TEXTENCODING_UTF8);
    std::string result;
    result.reserve(utf8.getLength() + 16);
    for (sal_Int32 i = 0; i < utf8.getLength(); ++i)
    {
        char c = utf8[i];
        switch (c)
        {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:   result += c;      break;
        }
    }
    return result;
}

// Extract a string value from JSON by key (simple, no nesting)
std::string extractJsonString(const std::string& json, const std::string& key)
{
    std::string searchKey = "\"" + key + "\"";
    auto pos = json.find(searchKey);
    if (pos == std::string::npos)
        return "";

    pos = json.find(':', pos + searchKey.length());
    if (pos == std::string::npos)
        return "";

    pos++;
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t'))
        pos++;

    if (pos >= json.length() || json[pos] != '"')
        return "";

    pos++; // skip opening quote
    std::string value;
    while (pos < json.length() && json[pos] != '"')
    {
        if (json[pos] == '\\' && pos + 1 < json.length())
        {
            pos++;
            switch (json[pos])
            {
                case 'n':  value += '\n'; break;
                case 'r':  value += '\r'; break;
                case 't':  value += '\t'; break;
                case '"':  value += '"';  break;
                case '\\': value += '\\'; break;
                default:   value += json[pos]; break;
            }
        }
        else
        {
            value += json[pos];
        }
        pos++;
    }
    return value;
}

} // anonymous namespace

WebViewMessageHandler::WebViewMessageHandler(WebViewPanel* pPanel)
    : m_pPanel(pPanel)
{
}

void WebViewMessageHandler::setPanel(WebViewPanel* pPanel)
{
    m_pPanel.store(pPanel, std::memory_order_release);
}

bool WebViewMessageHandler::OnQuery(
    CefRefPtr<CefBrowser> /*browser*/,
    CefRefPtr<CefFrame> /*frame*/,
    int64_t /*query_id*/,
    const CefString& request,
    bool /*persistent*/,
    CefRefPtr<Callback> callback)
{
    std::string req = request.ToString();
    SAL_INFO("officelabs.cef", "cefQuery received: " << req.substr(0, 100));

    // Route based on "type" field in JSON
    if (req.find("\"type\":\"getDocument\"") != std::string::npos
        || req.find("\"type\": \"getDocument\"") != std::string::npos)
    {
        handleGetDocument(callback);
        return true;
    }

    if (req.find("\"type\":\"getSelection\"") != std::string::npos
        || req.find("\"type\": \"getSelection\"") != std::string::npos)
    {
        handleGetSelection(callback);
        return true;
    }

    if (req.find("\"type\":\"applyEdit\"") != std::string::npos
        || req.find("\"type\": \"applyEdit\"") != std::string::npos)
    {
        handleApplyEdit(req, callback);
        return true;
    }

    if (req.find("\"type\":\"getAppType\"") != std::string::npos
        || req.find("\"type\": \"getAppType\"") != std::string::npos)
    {
        handleGetAppType(callback);
        return true;
    }

    if (req.find("\"type\":\"switchTheme\"") != std::string::npos
        || req.find("\"type\": \"switchTheme\"") != std::string::npos)
    {
        handleSwitchTheme(req, callback);
        return true;
    }

    // Unknown request type
    SAL_WARN("officelabs.cef", "Unknown cefQuery type: " << req.substr(0, 50));
    callback->Failure(404, "Unknown request type");
    return true;
}

void WebViewMessageHandler::OnQueryCanceled(
    CefRefPtr<CefBrowser> /*browser*/,
    CefRefPtr<CefFrame> /*frame*/,
    int64_t /*query_id*/)
{
    SAL_INFO("officelabs.cef", "cefQuery canceled");
}

void WebViewMessageHandler::handleGetDocument(CefRefPtr<Callback> callback)
{
    WebViewPanel* panel = m_pPanel.load(std::memory_order_acquire);
    if (!panel)
    {
        callback->Failure(500, "Panel not available");
        return;
    }

    CefRefPtr<Callback> cb = callback;

    // Re-read panel pointer inside VCL lambda (panel may have been
    // swapped between the CEF IO thread check and VCL dispatch).
    postToVclThread([this, cb]() {
        WebViewPanel* p = m_pPanel.load(std::memory_order_acquire);
        if (!p)
        {
            cb->Failure(500, "Panel destroyed during dispatch");
            return;
        }

        // Always refresh document reference (handles document switches)
        p->detectDocument();

        DocumentController* dc = p->getDocController();
        if (!dc || !dc->hasDocument())
        {
            cb->Success("{\"text\":\"\"}");
            return;
        }

        OUString text = dc->getDocumentText();
        std::string escaped = escapeJson(text);
        std::string response = "{\"text\":\"" + escaped + "\"}";
        cb->Success(response);
    });
}

void WebViewMessageHandler::handleGetSelection(CefRefPtr<Callback> callback)
{
    WebViewPanel* panel = m_pPanel.load(std::memory_order_acquire);
    if (!panel)
    {
        callback->Failure(500, "Panel not available");
        return;
    }

    CefRefPtr<Callback> cb = callback;

    postToVclThread([this, cb]() {
        WebViewPanel* p = m_pPanel.load(std::memory_order_acquire);
        if (!p)
        {
            cb->Failure(500, "Panel destroyed during dispatch");
            return;
        }

        p->detectDocument();

        DocumentController* dc = p->getDocController();
        if (!dc || !dc->hasDocument())
        {
            cb->Success("{\"selection\":\"\"}");
            return;
        }

        OUString selection = dc->getSelectedText();
        std::string escaped = escapeJson(selection);
        std::string response = "{\"selection\":\"" + escaped + "\"}";
        cb->Success(response);
    });
}

void WebViewMessageHandler::handleApplyEdit(
    const std::string& json, CefRefPtr<Callback> callback)
{
    WebViewPanel* panel = m_pPanel.load(std::memory_order_acquire);
    if (!panel)
    {
        callback->Failure(500, "Panel not available");
        return;
    }

    std::string editId = extractJsonString(json, "editId");
    std::string action = extractJsonString(json, "action");

    SAL_INFO("officelabs.cef", "applyEdit: id=" << editId << " action=" << action);

    CefRefPtr<Callback> cb = callback;

    postToVclThread([this, cb, editId, action]() {
        WebViewPanel* p = m_pPanel.load(std::memory_order_acquire);
        if (!p)
        {
            cb->Failure(500, "Panel destroyed during dispatch");
            return;
        }

        p->detectDocument();

        DocumentController* dc = p->getDocController();
        if (!dc || !dc->hasDocument())
        {
            cb->Failure(400, "No document open");
            return;
        }

        if (action == "approve")
        {
            cb->Success("{\"status\":\"approved\"}");
        }
        else if (action == "reject")
        {
            cb->Success("{\"status\":\"rejected\"}");
        }
        else
        {
            cb->Failure(400, "Unknown action");
        }
    });
}

void WebViewMessageHandler::handleGetAppType(CefRefPtr<Callback> callback)
{
    WebViewPanel* panel = m_pPanel.load(std::memory_order_acquire);
    if (!panel)
    {
        callback->Success("{\"appType\":\"writer\"}");
        return;
    }

    CefRefPtr<Callback> cb = callback;

    postToVclThread([this, cb]() {
        WebViewPanel* p = m_pPanel.load(std::memory_order_acquire);
        if (!p)
        {
            cb->Success("{\"appType\":\"writer\"}");
            return;
        }

        p->detectDocument();

        DocumentController* dc = p->getDocController();
        OUString appType = dc ? dc->getApplicationType() : u"writer"_ustr;

        OString utf8AppType = OUStringToOString(appType, RTL_TEXTENCODING_UTF8);
        std::string response = "{\"appType\":\"" + std::string(utf8AppType.getStr()) + "\"}";
        cb->Success(response);
    });
}

void WebViewMessageHandler::handleSwitchTheme(
    const std::string& req,
    CefRefPtr<Callback> callback)
{
    // Extract theme from JSON: {"type":"switchTheme","theme":"light"|"dark"}
    std::string theme = "dark";
    auto pos = req.find("\"theme\"");
    if (pos != std::string::npos)
    {
        auto valStart = req.find(':', pos);
        auto qStart = req.find('"', valStart + 1);
        auto qEnd = req.find('"', qStart + 1);
        if (qStart != std::string::npos && qEnd != std::string::npos)
            theme = req.substr(qStart + 1, qEnd - qStart - 1);
    }

    SAL_INFO("officelabs.cef", "switchTheme requested: " << theme);

    CefRefPtr<Callback> cb = callback;
    std::string themeCopy = theme;

    postToVclThread([cb, themeCopy]() {
        // 1. Run switch-theme.ps1 to re-point junctions
        // Find the script relative to soffice.exe
        OUString sProgPath(u"$BRAND_BASE_DIR/program"_ustr);
        rtl::Bootstrap::expandMacros(sProgPath);
        OUString sCorePath;
        // Navigate from instdir/program/ up to officelabs-core/
        // instdir is inside libreoffice-fork, core is sibling at repo level
        OUString sBaseDir(u"$BRAND_BASE_DIR"_ustr);
        rtl::Bootstrap::expandMacros(sBaseDir);

#ifdef _WIN32
        // Build path to switch-theme.ps1
        // sBaseDir = .../libreoffice-fork/instdir
        // script = .../officelabs-core/switch-theme.ps1
        OString baseUtf8 = OUStringToOString(sBaseDir, RTL_TEXTENCODING_UTF8);
        std::string basePath(baseUtf8.getStr());

        // Go up from instdir to libreoffice-fork, then up to suite root, then into officelabs-core
        std::string scriptPath = basePath + "/../../officelabs-core/switch-theme.ps1";

        std::string cmd = "powershell.exe -ExecutionPolicy Bypass -File \""
                          + scriptPath + "\" -Theme " + themeCopy;

        SAL_INFO("officelabs.cef", "Running: " << cmd);

        // Run script (non-blocking would be better, but synchronous is fine for a quick script)
        int ret = std::system(cmd.c_str());
        SAL_INFO("officelabs.cef", "switch-theme.ps1 returned: " << ret);

        // 2. Set env var for current process (in case restart is delayed)
        _putenv_s("OFFICELABS_THEME", themeCopy.c_str());
#endif

        // 3. Respond to JS with success
        cb->Success("{\"success\":true,\"theme\":\"" + themeCopy + "\",\"restartRequired\":true}");

        // 4. Request restart via LO's built-in restart manager
        try
        {
            auto xContext = comphelper::getProcessComponentContext();
            css::uno::Reference<css::uno::XInterface> xService(
                xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.comp.task.OfficeRestartManager"_ustr, xContext));
            css::uno::Reference<css::task::XRestartManager> xRestart(xService, css::uno::UNO_QUERY);
            if (xRestart.is())
            {
                xRestart->requestRestart(
                    css::uno::Reference<css::task::XInteractionHandler>());
                SAL_INFO("officelabs.cef", "Restart requested via XRestartManager");
            }
        }
        catch (const css::uno::Exception& e)
        {
            SAL_WARN("officelabs.cef", "Failed to request restart: " << e.Message);
        }
    });
}

} // namespace officelabs

#endif // HAVE_FEATURE_CEF

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
