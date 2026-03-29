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
 *
 * BROWSER PERSISTENCE:
 *   The CEF browser and its popup HWND live in static storage and survive
 *   panel destruction/recreation.  This prevents the sidebar from losing
 *   state when OLE in-place activation (charts, equations) temporarily
 *   changes the frame context.
 *   On panel destroy: popup is hidden, handler panel-pointer is nulled.
 *   On panel create:  popup is shown/repositioned, handler is re-wired.
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

class DocumentController;
class WebViewMessageHandler;

class OFFICELABS_DLLPUBLIC WebViewPanel final : public PanelLayout
{
public:
    // Factory method matching AIAssistantPanel::Create() signature
    static std::unique_ptr<PanelLayout> Create(
        weld::Widget* pParent, SfxBindings* pBindings);

    /// Release static CEF resources before CefShutdown(). Called by CefInit.
    static void cleanupPersistentBrowser();

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

    // Access document controller (for message handler)
    DocumentController* getDocController() const { return m_pDocController.get(); }

private:
    void initOrReattachCefBrowser();
    void initCefBrowser();
    void reattachCefBrowser();
    OUString getUIUrl() const;
    void syncCefWindowSize();

    // One-time registration of the "OfficeLabsCefHost" window class
    static bool registerCefHostClass();

    // Subclass proc installed on m_hFrameWnd for instant position tracking
    static LRESULT CALLBACK FrameSubclassProc(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
        UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

    DECL_LINK(ResizeTimerHdl, Timer*, void);

    SfxBindings* m_pBindings;

    // VclBin created via CreateChildFrame() inside the weld container
    VclPtr<vcl::Window> m_pBinWindow;

    // Instance copies of persistent static state (for convenience)
    HWND m_hCefParentWnd = nullptr;
    CefRefPtr<CefBrowser> m_browser;

    // Per-instance frame tracking
    HWND m_hFrameWnd = nullptr;

    // Resize tracking: timer polls for container size changes (fallback)
    Timer m_aResizeTimer{ "officelabs::WebViewPanel resize" };
    Size m_aLastSize;
    Point m_aLastPos;

    // Subclass tracking
    bool m_bInSizeMove = false;
    bool m_bSubclassed = false;

    // Grace period after reattach: number of timer ticks during which
    // syncCefWindowSize() will NOT hide the popup.  This prevents the
    // "black sidebar" after OLE in-place activation (chart insert etc.)
    // where the VCL parent isn't laid out yet and IsReallyVisible()
    // returns false, causing an immediate SW_HIDE.
    int m_nReattachGraceTicks = 0;

    // Deferred CEF init timer (removed — now uses postToVclThread + synthetic WM_SIZE)

    // Backend document bridge (per-panel — rebuilt on each attach)
    std::unique_ptr<DocumentController> m_pDocController;

    // NOTE: CefClient, CefMessageRouter, WebViewMessageHandler are NOT
    // per-instance — they live in static storage in WebViewPanel.cxx
    // and survive panel destruction/recreation.
};

} // namespace officelabs

#endif // HAVE_FEATURE_CEF
#endif // INCLUDED_OFFICELABS_WEBVIEWPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
