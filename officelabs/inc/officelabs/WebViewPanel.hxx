/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs WebView Panel
 *
 * CEF-based sidebar panel that hosts a React chat UI.
 * Replaces AIAssistantPanel when HAVE_FEATURE_CEF is defined.
 *
 * IPC:
 *   C++ -> JS: postMessageToJS() calls window.officelabs.__onMessage(json)
 *   JS -> C++: window.cefQuery() routed to WebViewMessageHandler
 */

#ifndef INCLUDED_OFFICELABS_WEBVIEWPANEL_HXX
#define INCLUDED_OFFICELABS_WEBVIEWPANEL_HXX

#ifdef HAVE_FEATURE_CEF

// prewin/postwin must come before ANY header that might pull in <windows.h>
// (vcl/sysdata.hxx, CEF headers, etc.)
#ifdef _WIN32
#include <prewin.h>
#include <windows.h>
#include <postwin.h>
#endif

#include <officelabs/officelabsdllapi.h>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/wrapper/cef_message_router.h>

#include <vcl/timer.hxx>
#include <memory>

class SfxBindings;

namespace officelabs {

class AgentConnection;
class DocumentController;
class WebViewMessageHandler;

class OFFICELABS_DLLPUBLIC WebViewPanel final : public PanelLayout
{
public:
    // Factory method matching AIAssistantPanel::Create() signature
    static std::unique_ptr<PanelLayout> Create(
        weld::Widget* pParent, SfxBindings* pBindings);

    WebViewPanel(weld::Widget* pParent, SfxBindings* pBindings);
    virtual ~WebViewPanel() override;

    // C++ -> JS: push JSON data to the React UI
    void postMessageToJS(const OUString& jsonMessage);

    // Access the browser (for message handler)
    CefRefPtr<CefBrowser> getBrowser() const { return m_browser; }

    // Called by CefLifeSpanHandler when browser is created
    void onBrowserCreated(CefRefPtr<CefBrowser> browser);

    // Reload after render process crash
    void reloadBrowser();

    // Detect/refresh the current Writer document for DocumentController
    void detectDocument();

    // Access agent and document controller (for message handler)
    AgentConnection* getAgent() const { return m_pAgent.get(); }
    DocumentController* getDocController() const { return m_pDocController.get(); }

private:
    void initCefBrowser();
    OUString getUIUrl() const;
    void syncCefWindowSize();

    DECL_LINK(ResizeTimerHdl, Timer*, void);

    SfxBindings* m_pBindings;

    // VclBin created via CreateChildFrame() inside the weld container
    VclPtr<vcl::Window> m_pBinWindow;

    // Raw Win32 child window hosting the CEF browser
    HWND m_hCefParentWnd = nullptr;
    HWND m_hFrameWnd = nullptr;

    // Resize tracking: timer polls for container size changes
    Timer m_aResizeTimer{ "officelabs::WebViewPanel resize" };
    Size m_aLastSize;
    Point m_aLastPos;

    // Backend connections
    std::unique_ptr<AgentConnection> m_pAgent;
    std::unique_ptr<DocumentController> m_pDocController;

    // CEF objects
    CefRefPtr<CefBrowser> m_browser;
    CefRefPtr<CefClient> m_client;
    CefRefPtr<CefMessageRouterBrowserSide> m_messageRouter;
    std::unique_ptr<WebViewMessageHandler> m_messageHandler;
};

} // namespace officelabs

#endif // HAVE_FEATURE_CEF
#endif // INCLUDED_OFFICELABS_WEBVIEWPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
