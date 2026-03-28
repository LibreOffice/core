/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs WebView Panel Implementation
 *
 * Embeds a CEF browser inside a popup window tracked to the sidebar.
 * The browser loads a React chat app (from Vite dev server or bundled files).
 *
 * PER-FRAME BROWSER ARCHITECTURE
 * --------------------------------
 * Each LibreOffice document frame (Writer, Calc, Impress, ...) gets its
 * OWN independent CEF browser + popup window.  State is kept in a
 * std::map<HWND, PerFrameCefState> keyed by the LO frame HWND.
 *
 * BROWSER PERSISTENCE (within a frame)
 * -------------------------------------
 * The CEF browser, popup HWND, message router, and client for a frame live in
 * PerFrameCefState and survive panel destruction/recreation within that frame.
 * This handles OLE in-place activation (charts, equations) which temporarily
 * destroys and recreates the sidebar panel.
 *
 * When the sidebar destroys the panel for a frame, the destructor only hides
 * the popup and nulls the handler's panel pointer.  When the sidebar recreates
 * the panel for the same frame, the constructor reattaches to the existing
 * browser — no page reload, no localStorage loss.
 *
 * VISIBILITY MANAGEMENT
 * ----------------------
 * syncCefWindowSize() shows a frame's popup ONLY when that frame is the
 * foreground window.  Non-active frames' popups are automatically hidden.
 * When a frame is activated (WM_ACTIVATE), all live panels are sync'd
 * immediately so the transition is instant.
 */

#ifdef HAVE_FEATURE_CEF

#include <officelabs/WebViewPanel.hxx>
#include <officelabs/CefInit.hxx>
#include <officelabs/WebViewMessageHandler.hxx>
#include <officelabs/DocumentController.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <sal/log.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/file.hxx>
#include <tools/link.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <sfx2/docfac.hxx>

#include <include/cef_app.h>

#include <functional>
#include <cstdlib>
#include <map>
#include <vector>
#include <algorithm>

// ============================================================
// Per-frame CEF state — one entry per LO document frame HWND
// ============================================================
namespace {

/// State that belongs to a single document frame (Writer/Calc/Impress window).
/// Created when the AI panel is first opened in that frame.
/// Survives panel destroy/recreate within the same frame (e.g. OLE activation).
/// Destroyed (popup auto-destroyed by Windows) when the frame HWND is destroyed.
struct PerFrameCefState
{
    CefRefPtr<CefBrowser>                        browser;
    HWND                                         hCefParentWnd = nullptr;
    CefRefPtr<CefMessageRouterBrowserSide>       messageRouter;
    std::unique_ptr<officelabs::WebViewMessageHandler> messageHandler;
    CefRefPtr<CefClient>                         client;
    SfxObjectShell*                              lastDocShell = nullptr;
    bool                                         browserCreated = false;
};

/// All per-frame state, keyed by LO frame HWND.
static std::map<HWND, PerFrameCefState> s_perFrameState;

/// All currently alive WebViewPanel instances.
/// Used to broadcast syncCefWindowSize() to all panels on WM_ACTIVATE.
static std::vector<officelabs::WebViewPanel*> s_allPanels;

} // anonymous namespace

// ============================================================
// Helpers
// ============================================================
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

// WndProc for the "OfficeLabsCefHost" window class.
// Unlike "Static", this class has no background brush and suppresses
// WM_ERASEBKGND. This prevents the black flash when Windows invalidates
// the popup during focus transitions between CEF and the document.
LRESULT CALLBACK CefHostWndProc(HWND hWnd, UINT uMsg,
                                WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ERASEBKGND:
        // Do NOT erase the background. CEF paints the entire client area.
        // Returning 1 tells Windows "I handled it" so it won't paint a
        // default background (which causes the black flash).
        return 1;

    case WM_PAINT:
    {
        // Validate the dirty region without drawing anything.
        // Without BeginPaint/EndPaint, Windows would keep sending WM_PAINT.
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_NCHITTEST:
        // The popup should never receive mouse input itself --
        // all input goes to the CEF child HWND inside it.
        return HTTRANSPARENT;

    case WM_ACTIVATE:
        // Suppress activation processing. During OLE in-place activation
        // (charts, equations), Windows sends WM_ACTIVATE to owned popups.
        // If DefWindowProc processes this, CEF interferes with the OLE
        // operation and crashes LibreOffice. Return 0 = "handled".
        return 0;

    case WM_NCACTIVATE:
        // Same rationale: suppress non-client activation visual updates.
        // Return TRUE to accept the state change without visual update.
        return TRUE;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

} // anonymous namespace

namespace officelabs {

// Subclass ID — arbitrary unique value for our frame subclass
static constexpr UINT_PTR CEFHOST_SUBCLASS_ID = 0x4F4C4345; // "OLCE"

bool WebViewPanel::registerCefHostClass()
{
    static bool bRegistered = false;
    if (bRegistered)
        return true;

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.style         = 0;
    wc.lpfnWndProc   = CefHostWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = GetModuleHandle(nullptr);
    wc.hIcon         = nullptr;
    wc.hIconSm       = nullptr;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;   // KEY: no background brush = no flash
    wc.lpszMenuName  = nullptr;
    wc.lpszClassName = L"OfficeLabsCefHost";

    if (!RegisterClassExW(&wc))
    {
        DWORD err = GetLastError();
        if (err != ERROR_CLASS_ALREADY_EXISTS)
        {
            SAL_WARN("officelabs.cef",
                     "RegisterClassExW(OfficeLabsCefHost) failed, error=" << err);
            return false;
        }
    }

    bRegistered = true;
    SAL_INFO("officelabs.cef", "Registered OfficeLabsCefHost window class");
    return true;
}

LRESULT CALLBACK WebViewPanel::FrameSubclassProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    auto* pPanel = reinterpret_cast<WebViewPanel*>(dwRefData);

    switch (uMsg)
    {
    case WM_WINDOWPOSCHANGED:
    {
        // Fires on every frame move/resize, including during drag.
        // This is the primary fix for the lag: instant position sync.
        LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        if (pPanel)
            pPanel->syncCefWindowSize();
        return result;
    }

    case WM_ACTIVATE:
    {
        LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        WORD wActivate = LOWORD(wParam);
        if (wActivate == WA_ACTIVE || wActivate == WA_CLICKACTIVE)
        {
            // This frame was activated. Immediately sync ALL live panels:
            //   - this panel: shows its popup at this sidebar's position
            //   - other panels: hide their popups (bFrameActive = false)
            // This makes the transition instant instead of waiting for timer.
            for (auto* p : s_allPanels)
                p->syncCefWindowSize();

            // Bring this panel's popup to top of Z-order.
            if (pPanel && pPanel->m_hCefParentWnd)
            {
                SetWindowPos(pPanel->m_hCefParentWnd, HWND_TOP,
                             0, 0, 0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
        }
        return result;
    }

    case WM_NCACTIVATE:
    {
        // When the CEF popup takes focus, the frame gets WM_NCACTIVATE(FALSE).
        // This dims the title bar. We want the frame to still LOOK active
        // when the user is interacting with the sidebar (it's part of LO).
        if (!wParam && pPanel && pPanel->m_hCefParentWnd)
        {
            HWND hFg = GetForegroundWindow();
            if (hFg == pPanel->m_hCefParentWnd)
            {
                // Force visual "active" state on the frame
                return DefWindowProcW(hWnd, WM_NCACTIVATE, TRUE, lParam);
            }
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    case WM_ENTERMENULOOP:
    case WM_INITMENUPOPUP:
    {
        // A menu or popup is opening — temporarily push CEF behind so
        // dropdown menus render on top of the AI sidebar.
        if (pPanel && pPanel->m_hCefParentWnd)
        {
            SetWindowPos(pPanel->m_hCefParentWnd, HWND_BOTTOM,
                         0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    case WM_EXITMENULOOP:
    case WM_UNINITMENUPOPUP:
    {
        // Menu closed — restore CEF to top of Z-order.
        if (pPanel && pPanel->m_hCefParentWnd)
        {
            SetWindowPos(pPanel->m_hCefParentWnd, HWND_TOP,
                         0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    case WM_ENTERSIZEMOVE:
    {
        if (pPanel)
            pPanel->m_bInSizeMove = true;
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    case WM_EXITSIZEMOVE:
    {
        if (pPanel)
        {
            pPanel->m_bInSizeMove = false;
            pPanel->syncCefWindowSize();
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    case WM_NCDESTROY:
    {
        RemoveWindowSubclass(hWnd, FrameSubclassProc, uIdSubclass);
        if (pPanel)
        {
            pPanel->m_bSubclassed = false;

            // The LO frame HWND is being destroyed. Remove this frame's entry
            // from the per-frame state map. The popup is an owned window of
            // hWnd, so Windows destroys it automatically — we just need to
            // clean up our map entry to avoid dangling references.
            s_perFrameState.erase(hWnd);
            SAL_INFO("officelabs.cef", "FrameSubclassProc WM_NCDESTROY: "
                     "removed per-frame CEF state for frame " << hWnd);
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    default:
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }
}

// ============================================================
// CefClient implementation for this panel
// ============================================================
class WebViewCefClient final : public CefClient,
                                public CefLifeSpanHandler,
                                public CefRequestHandler
{
public:
    WebViewCefClient(WebViewPanel* pPanel,
                     CefRefPtr<CefMessageRouterBrowserSide> router)
        : m_pPanel(pPanel)
        , m_messageRouter(router)
    {
    }

    /// Update the panel pointer (called on attach/detach within same frame).
    void setPanel(WebViewPanel* p) { m_pPanel.store(p, std::memory_order_release); }

    /// Cache the browser for use in crash recovery (when panel pointer may be null).
    void setBrowser(CefRefPtr<CefBrowser> b) { m_browser = b; }

    // CefClient
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

    // CefLifeSpanHandler
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override
    {
        WebViewPanel* panel = m_pPanel.load(std::memory_order_acquire);
        if (panel)
            panel->onBrowserCreated(browser);
    }

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override
    {
        m_messageRouter->OnBeforeClose(browser);
    }

    // CefRequestHandler
    bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefRequest> /*request*/,
                        bool /*user_gesture*/,
                        bool /*is_redirect*/) override
    {
        m_messageRouter->OnBeforeBrowse(browser, frame);
        return false;  // Allow the navigation
    }

    void OnRenderProcessTerminated(CefRefPtr<CefBrowser> /*browser*/,
                                    TerminationStatus status,
                                    int error_code,
                                    const CefString& /*error_string*/) override
    {
        SAL_WARN("officelabs.cef", "Render process terminated, status="
                 << static_cast<int>(status) << " code=" << error_code);

        // Use cached browser ref instead of going through the panel pointer.
        // During OLE activation (doVerb), the panel pointer may be null.
        CefRefPtr<CefBrowser> browser = m_browser;
        postToVclThread([browser]() {
            if (browser)
            {
                SAL_INFO("officelabs.cef", "Reloading browser after render crash");
                browser->Reload();
            }
        });
    }

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefProcessId source_process,
                                   CefRefPtr<CefProcessMessage> message) override
    {
        return m_messageRouter->OnProcessMessageReceived(
            browser, frame, source_process, message);
    }

    IMPLEMENT_REFCOUNTING(WebViewCefClient);

private:
    std::atomic<WebViewPanel*> m_pPanel;
    CefRefPtr<CefMessageRouterBrowserSide> m_messageRouter;
    CefRefPtr<CefBrowser> m_browser;  // cached for crash recovery
};

// ============================================================
// WebViewPanel
// ============================================================

std::unique_ptr<PanelLayout> WebViewPanel::Create(
    weld::Widget* pParent, SfxBindings* pBindings)
{
    if (!pParent)
        throw css::lang::IllegalArgumentException(
            u"No parent given to WebViewPanel::Create"_ustr, nullptr, 0);

    return std::make_unique<WebViewPanel>(pParent, pBindings);
}

void WebViewPanel::cleanupPersistentBrowser()
{
    SAL_INFO("officelabs.cef", "cleanupPersistentBrowser: releasing all per-frame CEF state");

    for (auto& [hwnd, state] : s_perFrameState)
    {
        if (state.browser)
        {
            state.browser->GetHost()->CloseBrowser(true);
            state.browser = nullptr;
        }
        state.client = nullptr;
        state.messageRouter = nullptr;
        state.messageHandler.reset();

        if (state.hCefParentWnd && IsWindow(state.hCefParentWnd))
        {
            DestroyWindow(state.hCefParentWnd);
            state.hCefParentWnd = nullptr;
        }
    }

    s_perFrameState.clear();
}

WebViewPanel::WebViewPanel(weld::Widget* pParent, SfxBindings* pBindings)
    : PanelLayout(pParent, u"WebViewPanel"_ustr, u"officelabs/ui/webviewpanel.ui"_ustr)
    , m_pBindings(pBindings)
{
    SAL_INFO("officelabs.cef", "WebViewPanel created, active frames="
             << s_perFrameState.size());

    // Register this instance for broadcast sync
    s_allPanels.push_back(this);

    // Create per-panel document bridge
    m_pDocController = std::make_unique<DocumentController>();

    // Initialize CEF if needed, then determine whether to init a new browser
    // or reattach to an existing one for this frame. The frame HWND is not
    // yet known here — initOrReattachCefBrowser() resolves it after layout.
    bool bCefOk = CefInit::instance().initialize();
    if (!bCefOk)
    {
        SAL_WARN("officelabs.cef", "CEF init failed - panel will be blank");
        return;
    }

    // Defer until after sidebar layout so the panel has a valid size/position.
    postToVclThread([this]() {
        initOrReattachCefBrowser();
    });
}

WebViewPanel::~WebViewPanel()
{
    SAL_INFO("officelabs.cef", "WebViewPanel destroyed (detaching, NOT closing browser)");

    m_aResizeTimer.Stop();

    // Unregister from the broadcast list
    auto it = std::find(s_allPanels.begin(), s_allPanels.end(), this);
    if (it != s_allPanels.end())
        s_allPanels.erase(it);

    // Remove frame subclass FIRST, before destroying any windows.
    // This prevents the subclass proc from firing with a stale pointer.
    if (m_bSubclassed && m_hFrameWnd && IsWindow(m_hFrameWnd))
    {
        RemoveWindowSubclass(m_hFrameWnd, FrameSubclassProc, CEFHOST_SUBCLASS_ID);
        m_bSubclassed = false;
    }

    // DETACH: null out the panel pointer in the handler and client so
    // they don't call back into a deleted panel. Do NOT close the browser
    // or destroy the popup — they persist in per-frame state.
    auto stateIt = s_perFrameState.find(m_hFrameWnd);
    if (stateIt != s_perFrameState.end())
    {
        auto& state = stateIt->second;
        if (state.messageHandler)
            state.messageHandler->setPanel(nullptr);
        if (state.client)
            static_cast<WebViewCefClient*>(state.client.get())->setPanel(nullptr);
    }

    // Keep the popup WHERE IT IS during the destroy/recreate transition.
    // DO NOT hide (SW_HIDE) or move off-screen — both cause the popup to
    // not re-show because syncCefWindowSize() fights with the visibility
    // state before the VCL parent is fully laid out.

    // Clear instance copies (per-frame state keeps the real references alive)
    m_browser = nullptr;
    m_hCefParentWnd = nullptr;

    m_pBinWindow.disposeAndClear();
}

void WebViewPanel::initOrReattachCefBrowser()
{
    if (!CefInit::instance().isInitialized())
        return;

    // Get the VclBin widget for position/size reference
    css::uno::Reference<css::awt::XWindow> xChildFrame = m_xContainer->CreateChildFrame();
    m_pBinWindow = VCLUnoHelper::GetWindow(xChildFrame);
    if (!m_pBinWindow)
    {
        SAL_WARN("officelabs.cef", "initOrReattach: CreateChildFrame failed");
        return;
    }

    vcl::Window* pSizedParent = m_pBinWindow->GetParent();
    if (!pSizedParent)
    {
        SAL_WARN("officelabs.cef", "initOrReattach: no parent window");
        return;
    }

    const SystemEnvData* pFrameData = pSizedParent->GetSystemData();
    HWND hFrameWnd = pFrameData ? static_cast<HWND>(pFrameData->hWnd) : nullptr;
    if (!hFrameWnd)
    {
        SAL_WARN("officelabs.cef", "initOrReattach: could not get frame HWND");
        return;
    }

    m_hFrameWnd = hFrameWnd;

    // Check if this frame already has an active browser
    auto it = s_perFrameState.find(m_hFrameWnd);
    if (it != s_perFrameState.end() && it->second.browserCreated)
    {
        SAL_INFO("officelabs.cef", "initOrReattach: frame already has browser — reattaching");
        reattachCefBrowser();
    }
    else
    {
        SAL_INFO("officelabs.cef", "initOrReattach: new frame — creating browser");
        initCefBrowser();
    }
}

void WebViewPanel::initCefBrowser()
{
    // m_hFrameWnd and m_pBinWindow are set by initOrReattachCefBrowser()
    if (!m_hFrameWnd || !m_pBinWindow)
        return;

    // Create per-frame state entry
    auto& state = s_perFrameState[m_hFrameWnd];

    // --- Set up CefMessageRouter for JS <-> C++ IPC ---
    CefMessageRouterConfig config;
    config.js_query_function = "cefQuery";
    config.js_cancel_function = "cefQueryCancel";
    state.messageRouter = CefMessageRouterBrowserSide::Create(config);

    state.messageHandler = std::make_unique<WebViewMessageHandler>(this);
    state.messageRouter->AddHandler(state.messageHandler.get(), true);

    state.client = new WebViewCefClient(this, state.messageRouter);

    // Get sidebar size and position
    vcl::Window* pSizedParent = m_pBinWindow->GetParent();
    Size aParentSize = pSizedParent->GetSizePixel();
    int w = aParentSize.Width();
    int h = aParentSize.Height();
    if (w <= 0 || h <= 0) { w = 300; h = 800; }

    auto aScrPos = pSizedParent->OutputToAbsoluteScreenPixel(Point(0, 0));
    int scrX = static_cast<int>(aScrPos.X());
    int scrY = static_cast<int>(aScrPos.Y());

    // Register custom window class (once) — no background brush = no flash.
    registerCefHostClass();

    // Create an OWNED popup window at the sidebar's screen position.
    // Owner = m_hFrameWnd: popup is bound to this specific document frame.
    // When m_hFrameWnd is destroyed, Windows auto-destroys this popup.
    HWND hCefParent = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        L"OfficeLabsCefHost", L"",
        WS_POPUP | WS_CLIPCHILDREN,
        scrX, scrY, w, h,
        m_hFrameWnd, nullptr, GetModuleHandle(nullptr), nullptr);

    if (hCefParent)
    {
        SetWindowPos(hCefParent, HWND_TOP,
                     0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }

    state.hCefParentWnd = hCefParent;
    m_hCefParentWnd = hCefParent;
    m_aLastSize = Size(w, h);
    m_aLastPos = Point(scrX, scrY);

    // Subclass the frame HWND for instant move/resize/activate notifications.
    if (SetWindowSubclass(m_hFrameWnd, FrameSubclassProc,
                          CEFHOST_SUBCLASS_ID,
                          reinterpret_cast<DWORD_PTR>(this)))
    {
        m_bSubclassed = true;
        SAL_INFO("officelabs.cef", "Frame HWND subclassed for instant tracking");
    }
    else
    {
        SAL_WARN("officelabs.cef",
                 "SetWindowSubclass failed, falling back to timer-only tracking");
    }

    if (!hCefParent)
        return;

    // --- Create CEF browser ---
    CefWindowInfo windowInfo;
    CefBrowserSettings browserSettings;
    windowInfo.SetAsChild(hCefParent, CefRect(0, 0, w, h));

    OUString url = getUIUrl();
    OString utf8Url = OUStringToOString(url, RTL_TEXTENCODING_UTF8);
    SAL_INFO("officelabs.cef", "Creating CEF browser for frame "
             << reinterpret_cast<sal_uIntPtr>(m_hFrameWnd)
             << " URL: " << utf8Url);

    CefBrowserHost::CreateBrowser(
        windowInfo,
        state.client,
        CefString(utf8Url.getStr()),
        browserSettings,
        nullptr,
        nullptr
    );

    state.browserCreated = true;

    // Start resize tracking timer
    m_aResizeTimer.SetInvokeHandler(LINK(this, WebViewPanel, ResizeTimerHdl));
    m_aResizeTimer.SetTimeout(500);
    m_aResizeTimer.Start();
}

void WebViewPanel::reattachCefBrowser()
{
    // m_hFrameWnd and m_pBinWindow are set by initOrReattachCefBrowser()
    if (!m_hFrameWnd || !m_pBinWindow)
        return;

    auto it = s_perFrameState.find(m_hFrameWnd);
    if (it == s_perFrameState.end())
    {
        SAL_WARN("officelabs.cef", "reattach: no per-frame state for this HWND — init instead");
        initCefBrowser();
        return;
    }

    auto& state = it->second;
    SAL_INFO("officelabs.cef", "reattachCefBrowser: wiring persistent browser to new panel");

    // Copy per-frame refs into instance members
    m_browser = state.browser;
    m_hCefParentWnd = state.hCefParentWnd;

    // Re-wire handler and client to point to this (new) panel
    if (state.messageHandler)
        state.messageHandler->setPanel(this);
    if (state.client)
        static_cast<WebViewCefClient*>(state.client.get())->setPanel(this);

    // Install frame subclass on the (possibly new) frame HWND
    if (!m_bSubclassed)
    {
        if (SetWindowSubclass(m_hFrameWnd, FrameSubclassProc,
                              CEFHOST_SUBCLASS_ID,
                              reinterpret_cast<DWORD_PTR>(this)))
        {
            m_bSubclassed = true;
        }
    }

    // Show the popup and bring it to the top of Z-order.
    if (state.hCefParentWnd)
    {
        SetWindowPos(state.hCefParentWnd, HWND_TOP,
                     0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);

        if (m_browser)
        {
            m_browser->GetHost()->NotifyMoveOrResizeStarted();
            m_browser->GetHost()->WasResized();
        }
    }

    // GRACE PERIOD: prevent syncCefWindowSize() from hiding the popup
    // during the first few seconds while VCL lays out the sidebar.
    // 10 ticks × 500ms timer = 5 seconds max.
    m_nReattachGraceTicks = 10;
    SAL_INFO("officelabs.cef", "reattach: grace period started (10 ticks)");

    // Start resize tracking timer
    m_aResizeTimer.SetInvokeHandler(LINK(this, WebViewPanel, ResizeTimerHdl));
    m_aResizeTimer.SetTimeout(500);
    m_aResizeTimer.Start();

    // Re-detect the document and check for document switch.
    SfxObjectShell* pPrevShell = state.lastDocShell;
    detectDocument();

    if (pPrevShell != nullptr && state.lastDocShell != pPrevShell)
    {
        SAL_INFO("officelabs.cef",
                 "reattach: document changed — clearing chat in React");
        postMessageToJS(u"{\"type\":\"session:newDocument\"}"_ustr);
    }

    SAL_INFO("officelabs.cef", "reattachCefBrowser: done");
}

void WebViewPanel::syncCefWindowSize()
{
    if (!m_hCefParentWnd || !m_pBinWindow)
        return;

    vcl::Window* pParent = m_pBinWindow->GetParent();
    if (!pParent)
        return;

    // --- Visibility management ---
    // Show this panel's popup whenever its sidebar is visible and its frame
    // is not minimized. Each frame owns its own popup window (positioned at
    // its own sidebar), so multiple panels can be visible simultaneously
    // across different document windows without overlapping each other.
    HWND hFg = GetForegroundWindow();
    bool bFrameActive = (hFg == m_hFrameWnd || hFg == m_hCefParentWnd);

    bool bSidebarVisible = pParent->IsReallyVisible();
    bool bFrameMinimized = m_hFrameWnd && IsIconic(m_hFrameWnd);
    bool bShouldShow = bSidebarVisible && !bFrameMinimized;

    bool bIsShown = IsWindowVisible(m_hCefParentWnd);

    // GRACE PERIOD after reattach (same as before — handles OLE activation).
    if (m_nReattachGraceTicks > 0)
    {
        if (bShouldShow)
        {
            m_nReattachGraceTicks = 0;
            SAL_INFO("officelabs.cef", "syncCef: grace period ended (VCL ready)");
        }
        else
        {
            --m_nReattachGraceTicks;
            if (m_nReattachGraceTicks == 0)
                SAL_INFO("officelabs.cef", "syncCef: grace period expired");
            return;
        }
    }

    if (bShouldShow && !bIsShown)
        ShowWindow(m_hCefParentWnd, SW_SHOWNA);
    else if (!bShouldShow && bIsShown)
        ShowWindow(m_hCefParentWnd, SW_HIDE);

    if (!bShouldShow)
        return;

    // --- Z-order ---
    // Only boost to HWND_TOP when this frame is the active one. Inactive
    // panels stay at their natural Z-order (above their owner frame but
    // below the active application's windows), preventing them from
    // covering unrelated foreground content.
    if (bFrameActive)
    {
        SetWindowPos(m_hCefParentWnd, HWND_TOP,
                     0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }

    // --- Position and size tracking ---
    Size aSize = pParent->GetSizePixel();
    auto aScrPos = pParent->OutputToAbsoluteScreenPixel(Point(0, 0));
    Point aPos(static_cast<int>(aScrPos.X()), static_cast<int>(aScrPos.Y()));

    if (aSize == m_aLastSize && aPos == m_aLastPos)
        return;

    m_aLastSize = aSize;
    m_aLastPos = aPos;

    SetWindowPos(m_hCefParentWnd, nullptr,
                 aPos.X(), aPos.Y(),
                 aSize.Width(), aSize.Height(),
                 SWP_NOZORDER | SWP_NOACTIVATE);

    if (m_browser)
    {
        HWND cefHwnd = m_browser->GetHost()->GetWindowHandle();
        if (cefHwnd)
        {
            SetWindowPos(cefHwnd, nullptr, 0, 0,
                         aSize.Width(), aSize.Height(),
                         SWP_NOZORDER | SWP_NOMOVE);
        }
    }
}

IMPL_LINK_NOARG(WebViewPanel, ResizeTimerHdl, Timer*, void)
{
    syncCefWindowSize();
    m_aResizeTimer.Start();  // Restart for next check
}

OUString WebViewPanel::getUIUrl() const
{
    // Check for dev mode: OFFICELABS_UI_DEV_URL=http://localhost:5173
    const char* devUrl = std::getenv("OFFICELABS_UI_DEV_URL");
    if (devUrl && *devUrl)
    {
        SAL_INFO("officelabs.cef", "Dev mode: loading UI from " << devUrl);
        return OUString::fromUtf8(devUrl);
    }

    // Production: load from bundled files in instdir/program/officelabs-ui/
    OUString sInstDir(u"$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/.."_ustr);
    rtl::Bootstrap::expandMacros(sInstDir);

    OUString sFileUrl;
    osl::FileBase::getAbsoluteFileURL(OUString(), sInstDir, sFileUrl);

    return sFileUrl + "/program/officelabs-ui/index.html";
}

void WebViewPanel::postMessageToJS(const OUString& jsonMessage)
{
    if (!m_browser || !m_browser->GetMainFrame())
        return;

    OString utf8 = OUStringToOString(jsonMessage, RTL_TEXTENCODING_UTF8);
    std::string js = "window.officelabs && window.officelabs.__onMessage("
                   + std::string(utf8.getStr()) + ");";

    m_browser->GetMainFrame()->ExecuteJavaScript(
        CefString(js), m_browser->GetMainFrame()->GetURL(), 0);
}

// Helper: determine app type string from factory name
static OUString appTypeFromFactory(const OUString& rFactoryName)
{
    if (rFactoryName == "swriter")
        return u"writer"_ustr;
    if (rFactoryName == "scalc")
        return u"calc"_ustr;
    if (rFactoryName == "simpress")
        return u"impress"_ustr;
    if (rFactoryName == "sdraw")
        return u"draw"_ustr;
    return u"writer"_ustr;  // safe fallback
}

void WebViewPanel::detectDocument()
{
    SfxObjectShell* pShell = nullptr;

    // Strategy 1: Use SfxBindings -> Dispatcher -> ViewFrame -> ObjectShell
    if (m_pBindings)
    {
        SfxDispatcher* pDisp = m_pBindings->GetDispatcher();
        if (pDisp)
        {
            SfxViewFrame* pViewFrame = pDisp->GetFrame();
            if (pViewFrame)
                pShell = pViewFrame->GetObjectShell();
        }
    }

    // Strategy 2: Fallback to SfxObjectShell::Current()
    if (!pShell)
        pShell = SfxObjectShell::Current();

    if (!pShell)
    {
        SAL_WARN("officelabs.cef", "detectDocument: no document shell found");
        return;
    }

    // Determine application type from factory name
    const OUString sFactory = pShell->GetFactory().GetFactoryName();
    const OUString sAppType = appTypeFromFactory(sFactory);
    m_pDocController->setAppType(sAppType);

    // Update per-frame lastDocShell
    auto stateIt = s_perFrameState.find(m_hFrameWnd);
    if (stateIt != s_perFrameState.end())
        stateIt->second.lastDocShell = pShell;

    SAL_INFO("officelabs.cef",
             "detectDocument: factory=" << sFactory << " appType=" << sAppType);

    // Bind the appropriate document interface based on app type
    if (sAppType == "writer")
    {
        css::uno::Reference<css::text::XTextDocument> xTextDoc(
            pShell->GetModel(), css::uno::UNO_QUERY);
        if (xTextDoc.is())
            m_pDocController->setDocument(xTextDoc);
    }
    else if (sAppType == "calc")
    {
        css::uno::Reference<css::sheet::XSpreadsheetDocument> xCalcDoc(
            pShell->GetModel(), css::uno::UNO_QUERY);
        if (xCalcDoc.is())
            m_pDocController->setCalcDocument(xCalcDoc);
    }
    else if (sAppType == "impress" || sAppType == "draw")
    {
        css::uno::Reference<css::drawing::XDrawPagesSupplier> xDrawDoc(
            pShell->GetModel(), css::uno::UNO_QUERY);
        if (xDrawDoc.is())
            m_pDocController->setImpressDocument(xDrawDoc);
    }
}

void WebViewPanel::onBrowserCreated(CefRefPtr<CefBrowser> browser)
{
    m_browser = browser;

    // Store in per-frame state
    auto stateIt = s_perFrameState.find(m_hFrameWnd);
    if (stateIt != s_perFrameState.end())
    {
        stateIt->second.browser = browser;
        // Give the client a direct reference for crash recovery
        if (stateIt->second.client)
            static_cast<WebViewCefClient*>(stateIt->second.client.get())->setBrowser(browser);
    }

    SAL_INFO("officelabs.cef", "CEF browser created for frame "
             << reinterpret_cast<sal_uIntPtr>(m_hFrameWnd));

    // Now that the browser is ready, detect and bind the current document
    postToVclThread([this]() {
        detectDocument();
    });
}

void WebViewPanel::reloadBrowser()
{
    if (m_browser)
    {
        SAL_INFO("officelabs.cef", "Reloading CEF browser after crash");
        m_browser->Reload();
    }
}

} // namespace officelabs

#endif // HAVE_FEATURE_CEF

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
