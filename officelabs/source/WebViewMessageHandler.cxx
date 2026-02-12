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
 */

#ifdef HAVE_FEATURE_CEF

#include <officelabs/WebViewMessageHandler.hxx>
#include <officelabs/WebViewPanel.hxx>
#include <officelabs/DocumentController.hxx>

#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <tools/link.hxx>

#include <functional>

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
    if (!m_pPanel)
    {
        callback->Failure(500, "Panel not available");
        return;
    }

    CefRefPtr<Callback> cb = callback;
    WebViewPanel* panel = m_pPanel;

    postToVclThread([cb, panel]() {
        // Always refresh document reference (handles document switches)
        panel->detectDocument();

        DocumentController* dc = panel->getDocController();
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
    if (!m_pPanel)
    {
        callback->Failure(500, "Panel not available");
        return;
    }

    CefRefPtr<Callback> cb = callback;
    WebViewPanel* panel = m_pPanel;

    postToVclThread([cb, panel]() {
        // Always refresh document reference (handles document switches)
        panel->detectDocument();

        DocumentController* dc = panel->getDocController();
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
    if (!m_pPanel)
    {
        callback->Failure(500, "Panel not available");
        return;
    }

    std::string editId = extractJsonString(json, "editId");
    std::string action = extractJsonString(json, "action");

    SAL_INFO("officelabs.cef", "applyEdit: id=" << editId << " action=" << action);

    CefRefPtr<Callback> cb = callback;
    WebViewPanel* panel = m_pPanel;

    postToVclThread([cb, panel, editId, action]() {
        panel->detectDocument();

        DocumentController* dc = panel->getDocController();
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

} // namespace officelabs

#endif // HAVE_FEATURE_CEF

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
