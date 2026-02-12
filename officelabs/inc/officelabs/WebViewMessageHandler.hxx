/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs WebView Message Handler
 *
 * Handles JS -> C++ messages via cefQuery().
 * Only handles document operations - chat is direct HTTP/SSE to agent.
 *
 * Request types:
 *   getDocument  - Read document text via DocumentController
 *   getSelection - Get current selection
 *   applyEdit    - Apply/reject a pending edit
 */

#ifndef INCLUDED_OFFICELABS_WEBVIEWMESSAGEHANDLER_HXX
#define INCLUDED_OFFICELABS_WEBVIEWMESSAGEHANDLER_HXX

#ifdef HAVE_FEATURE_CEF

#include <officelabs/officelabsdllapi.h>

#ifdef _WIN32
#include <prewin.h>
#include <windows.h>
#include <postwin.h>
#endif

#include <include/wrapper/cef_message_router.h>
#include <string>

namespace officelabs {

class WebViewPanel;

class WebViewMessageHandler final : public CefMessageRouterBrowserSide::Handler
{
public:
    explicit WebViewMessageHandler(WebViewPanel* pPanel);

    bool OnQuery(CefRefPtr<CefBrowser> browser,
                 CefRefPtr<CefFrame> frame,
                 int64_t query_id,
                 const CefString& request,
                 bool persistent,
                 CefRefPtr<Callback> callback) override;

    void OnQueryCanceled(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         int64_t query_id) override;

private:
    void handleGetDocument(CefRefPtr<Callback> callback);
    void handleGetSelection(CefRefPtr<Callback> callback);
    void handleApplyEdit(const std::string& json,
                         CefRefPtr<Callback> callback);

    WebViewPanel* m_pPanel;  // non-owning, panel outlives handler
};

} // namespace officelabs

#endif // HAVE_FEATURE_CEF
#endif // INCLUDED_OFFICELABS_WEBVIEWMESSAGEHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
