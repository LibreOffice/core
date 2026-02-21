/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs WebView Panel Implementation
 *
 * Embeds a CEF browser inside a SystemChildWindow in the sidebar.
 * The browser loads a React chat app (from Vite dev server or bundled files).
 */

#ifdef HAVE_FEATURE_CEF

#include <officelabs/WebViewPanel.hxx>
#include <officelabs/CefInit.hxx>
#include <officelabs/WebViewMessageHandler.hxx>
#include <officelabs/AgentConnection.hxx>
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

#include <com/sun/star/text/XTextDocument.hpp>

#include <include/cef_app.h>

#include <functional>
#include <cstdlib>

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
        if (pPanel && pPanel->m_hCefParentWnd)
        {
            WORD wActivate = LOWORD(wParam);
            if (wActivate == WA_ACTIVE || wActivate == WA_CLICKACTIVE)
            {
                // Frame activated: bring popup to top immediately.
                // Fixes the Z-order gap during focus transitions.
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
            pPanel->m_bSubclassed = false;
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

    // CefClient
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

    // CefLifeSpanHandler
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override
    {
        if (m_pPanel)
            m_pPanel->onBrowserCreated(browser);
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

        if (m_pPanel)
        {
            WebViewPanel* panel = m_pPanel;
            postToVclThread([panel]() {
                panel->reloadBrowser();
            });
        }
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
    WebViewPanel* m_pPanel;
    CefRefPtr<CefMessageRouterBrowserSide> m_messageRouter;
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

WebViewPanel::WebViewPanel(weld::Widget* pParent, SfxBindings* pBindings)
    : PanelLayout(pParent, u"WebViewPanel"_ustr, u"officelabs/ui/webviewpanel.ui"_ustr)
    , m_pBindings(pBindings)
{
    SAL_INFO("officelabs.cef", "WebViewPanel created");

    // Create backend connections
    m_pAgent = std::make_unique<AgentConnection>();
    m_pDocController = std::make_unique<DocumentController>();

    // Ensure CEF is initialized (lazy init on first panel creation)
    bool bCefOk = CefInit::instance().initialize();

    if (!bCefOk)
    {
        SAL_WARN("officelabs.cef", "CEF init failed - panel will be blank");
        return;
    }

    // Defer browser creation until after sidebar layout has happened.
    // At construction time the panel has 0x0 size; PostUserEvent
    // fires after the current event processing, giving layout a chance.
    postToVclThread([this]() {
        initCefBrowser();
    });
}

WebViewPanel::~WebViewPanel()
{
    SAL_INFO("officelabs.cef", "WebViewPanel destroyed");

    m_aResizeTimer.Stop();

    // Remove frame subclass FIRST, before destroying any windows.
    // This prevents the subclass proc from firing with a stale pointer.
    if (m_bSubclassed && m_hFrameWnd && IsWindow(m_hFrameWnd))
    {
        RemoveWindowSubclass(m_hFrameWnd, FrameSubclassProc, CEFHOST_SUBCLASS_ID);
        m_bSubclassed = false;
    }

    if (m_browser)
    {
        m_browser->GetHost()->CloseBrowser(true);
        m_browser = nullptr;
    }

    if (m_hCefParentWnd)
    {
        DestroyWindow(m_hCefParentWnd);
        m_hCefParentWnd = nullptr;
    }

    m_pBinWindow.disposeAndClear();
}

void WebViewPanel::initCefBrowser()
{
    if (!CefInit::instance().isInitialized())
        return;

    // --- Set up CefMessageRouter for JS <-> C++ IPC ---
    CefMessageRouterConfig config;
    config.js_query_function = "cefQuery";
    config.js_cancel_function = "cefQueryCancel";
    m_messageRouter = CefMessageRouterBrowserSide::Create(config);

    m_messageHandler = std::make_unique<WebViewMessageHandler>(this);
    m_messageRouter->AddHandler(m_messageHandler.get(), true);

    m_client = new WebViewCefClient(this, m_messageRouter);

    // CreateChildFrame gives us a VclBin inside the weld container
    css::uno::Reference<css::awt::XWindow> xChildFrame = m_xContainer->CreateChildFrame();
    m_pBinWindow = VCLUnoHelper::GetWindow(xChildFrame);
    if (!m_pBinWindow)
        return;

    // Get the sidebar panel parent for sizing reference
    vcl::Window* pSizedParent = m_pBinWindow->GetParent();
    Size aParentSize = pSizedParent ? pSizedParent->GetSizePixel() : Size(0, 0);
    int w = aParentSize.Width();
    int h = aParentSize.Height();
    if (w <= 0 || h <= 0) { w = 300; h = 800; }

    // Get the frame HWND (all lightweight VCL widgets share this)
    const SystemEnvData* pFrameData = pSizedParent->GetSystemData();
    HWND hFrameWnd = pFrameData ? static_cast<HWND>(pFrameData->hWnd) : nullptr;
    if (!hFrameWnd)
        return;

    m_hFrameWnd = hFrameWnd;

    // Get the sidebar position in SCREEN coordinates.
    // We use a POPUP window with NO OWNER so CEF lives entirely outside
    // Win32's window hierarchy. This prevents crashes during:
    //   - DrawPage paint (shapes, arrows)
    //   - OLE in-place activation (charts, equations)
    // An owned window receives WM_NCACTIVATE/WM_ACTIVATE during OLE
    // activation, which causes CEF to interfere and crash LO.
    // Without an owner, we manually track frame visibility/minimize state.
    auto aScrPos = pSizedParent->OutputToAbsoluteScreenPixel(Point(0, 0));
    int scrX = static_cast<int>(aScrPos.X());
    int scrY = static_cast<int>(aScrPos.Y());

    // Register custom window class (once) — no background brush = no flash.
    registerCefHostClass();

    // Create an OWNED popup window at the sidebar's screen position.
    // WS_EX_TOOLWINDOW: hidden from taskbar/alt-tab
    // Owner = hFrameWnd: popup is captured with the frame during screen
    // sharing (Teams, Zoom, etc.) and auto-hides when frame is minimized.
    // WM_ACTIVATE/WM_NCACTIVATE are suppressed in CefHostWndProc to
    // prevent OLE in-place activation crashes (the original reason for
    // using an unowned popup).
    HWND hCefParent = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        L"OfficeLabsCefHost", L"",
        WS_POPUP | WS_CLIPCHILDREN,
        scrX, scrY, w, h,
        hFrameWnd, nullptr, GetModuleHandle(nullptr), nullptr);

    if (hCefParent)
    {
        // Show at top of Z-order without activating.
        // HWND_TOP puts popup IN FRONT of the frame (hFrameWnd would put it behind).
        // The resize timer maintains Z-order afterward.
        SetWindowPos(hCefParent, HWND_TOP,
                     0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }

    // Store HWNDs for cleanup and resize tracking
    m_hCefParentWnd = hCefParent;
    m_aLastSize = Size(w, h);
    m_aLastPos = Point(scrX, scrY);

    // Subclass the frame HWND for instant move/resize/activate notifications.
    // This replaces the timer as the primary position-tracking mechanism.
    if (m_hFrameWnd)
    {
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
    }

    if (!hCefParent)
        return;

    // --- Create CEF browser ---
    CefWindowInfo windowInfo;
    CefBrowserSettings browserSettings;
    windowInfo.SetAsChild(hCefParent, CefRect(0, 0, w, h));

    // Load the React UI
    OUString url = getUIUrl();
    OString utf8Url = OUStringToOString(url, RTL_TEXTENCODING_UTF8);
    SAL_INFO("officelabs.cef", "Creating CEF browser with URL: " << utf8Url);

    CefBrowserHost::CreateBrowser(
        windowInfo,
        m_client,
        CefString(utf8Url.getStr()),
        browserSettings,
        nullptr,
        nullptr
    );

    // Start resize tracking timer
    m_aResizeTimer.SetInvokeHandler(LINK(this, WebViewPanel, ResizeTimerHdl));
    m_aResizeTimer.SetTimeout(500);
    m_aResizeTimer.Start();
}

void WebViewPanel::syncCefWindowSize()
{
    if (!m_hCefParentWnd || !m_pBinWindow)
        return;

    vcl::Window* pParent = m_pBinWindow->GetParent();
    if (!pParent)
        return;

    // --- Visibility management (no owner, so we do this manually) ---

    // Should the popup be visible?
    // Hidden if: sidebar panel invisible, OR frame is minimized
    bool bSidebarVisible = pParent->IsReallyVisible();
    bool bFrameMinimized = m_hFrameWnd && IsIconic(m_hFrameWnd);
    bool bShouldShow = bSidebarVisible && !bFrameMinimized;

    bool bIsShown = IsWindowVisible(m_hCefParentWnd);
    if (bShouldShow && !bIsShown)
        ShowWindow(m_hCefParentWnd, SW_SHOWNA);
    else if (!bShouldShow && bIsShown)
        ShowWindow(m_hCefParentWnd, SW_HIDE);

    if (!bShouldShow)
        return;

    // --- Z-order: keep popup in front of the LO frame ---
    // Adjust when frame or popup is foreground (avoid popping over other apps).
    // With the frame subclass, WM_ACTIVATE handles the primary Z-order fixup
    // instantly. This timer-based check is a fallback.
    HWND hFg = GetForegroundWindow();
    if (m_hFrameWnd && (hFg == m_hFrameWnd || hFg == m_hCefParentWnd))
    {
        SetWindowPos(m_hCefParentWnd, HWND_TOP,
                     0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }

    // --- Position and size tracking ---
    Size aSize = pParent->GetSizePixel();

    // Get the sidebar position in SCREEN coordinates (popup uses screen coords)
    auto aScrPos = pParent->OutputToAbsoluteScreenPixel(Point(0, 0));
    Point aPos(static_cast<int>(aScrPos.X()), static_cast<int>(aScrPos.Y()));

    if (aSize == m_aLastSize && aPos == m_aLastPos)
        return;

    m_aLastSize = aSize;
    m_aLastPos = aPos;

    // Move the popup to the sidebar's current screen position
    SetWindowPos(m_hCefParentWnd, nullptr,
                 aPos.X(), aPos.Y(),
                 aSize.Width(), aSize.Height(),
                 SWP_NOZORDER | SWP_NOACTIVATE);

    // Also resize the CEF browser window inside the popup
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

void WebViewPanel::detectDocument()
{
    // Already have a document? Skip re-detection unless it changed.
    // (Remove this early-return if you need to support document switching.)

    // Strategy 1: Use SfxBindings -> Dispatcher -> ViewFrame -> ObjectShell
    // This is the most reliable path because m_pBindings is tied to the specific
    // frame that owns this sidebar panel.
    if (m_pBindings)
    {
        SfxDispatcher* pDisp = m_pBindings->GetDispatcher();
        if (pDisp)
        {
            SfxViewFrame* pViewFrame = pDisp->GetFrame();
            if (pViewFrame)
            {
                SfxObjectShell* pShell = pViewFrame->GetObjectShell();
                if (pShell)
                {
                    css::uno::Reference<css::text::XTextDocument> xTextDoc(
                        pShell->GetModel(), css::uno::UNO_QUERY);
                    if (xTextDoc.is())
                    {
                        m_pDocController->setDocument(xTextDoc);
                        SAL_INFO("officelabs.cef",
                                 "detectDocument: bound via SfxBindings");
                        return;
                    }
                }
            }
        }
        SAL_INFO("officelabs.cef",
                 "detectDocument: SfxBindings path failed, trying fallback");
    }

    // Strategy 2: Fallback to SfxObjectShell::Current()
    SfxObjectShell* pShell = SfxObjectShell::Current();
    if (pShell)
    {
        css::uno::Reference<css::text::XTextDocument> xTextDoc(
            pShell->GetModel(), css::uno::UNO_QUERY);
        if (xTextDoc.is())
        {
            m_pDocController->setDocument(xTextDoc);
            SAL_INFO("officelabs.cef",
                     "detectDocument: bound via SfxObjectShell::Current()");
            return;
        }
    }

    SAL_WARN("officelabs.cef", "detectDocument: could not find any Writer document");
}

void WebViewPanel::onBrowserCreated(CefRefPtr<CefBrowser> browser)
{
    m_browser = browser;
    SAL_INFO("officelabs.cef", "CEF browser created successfully");

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
