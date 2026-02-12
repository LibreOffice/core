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

} // anonymous namespace

namespace officelabs {

// ============================================================
// CefHostWindow: SystemChildWindow that resizes CEF on layout
// ============================================================
class CefHostWindow final : public SystemChildWindow
{
public:
    explicit CefHostWindow(vcl::Window* pParent)
        : SystemChildWindow(pParent, 0)
    {
    }

    void SetBrowser(CefRefPtr<CefBrowser> browser) { m_browser = browser; }

    virtual void Resize() override
    {
        SystemChildWindow::Resize();
        if (!m_browser)
            return;

        HWND cefHwnd = m_browser->GetHost()->GetWindowHandle();
        if (cefHwnd)
        {
            Size aSize = GetSizePixel();
            SetWindowPos(cefHwnd, nullptr, 0, 0,
                         aSize.Width(), aSize.Height(),
                         SWP_NOZORDER | SWP_NOMOVE);
        }
    }

private:
    CefRefPtr<CefBrowser> m_browser;
};

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

    // The VclBin starts at 0x0. Walk up to find a parent with actual size.
    vcl::Window* pSizedParent = m_pBinWindow->GetParent();
    Size aParentSize = pSizedParent ? pSizedParent->GetSizePixel() : Size(0, 0);
    int w = aParentSize.Width();
    int h = aParentSize.Height();
    if (w <= 0 || h <= 0) { w = 300; h = 800; }

    // All lightweight VCL widgets share the top-level frame HWND.
    // Create a raw Win32 child window directly inside the frame HWND
    // at the correct position for CEF to render into.
    const SystemEnvData* pFrameData = pSizedParent->GetSystemData();
    HWND hFrameWnd = pFrameData ? static_cast<HWND>(pFrameData->hWnd) : nullptr;
    if (!hFrameWnd)
        return;

    // Map the panel content area's position from screen coords to frame client coords
    auto aScrPos = pSizedParent->OutputToAbsoluteScreenPixel(Point(0, 0));
    POINT framePos = { static_cast<LONG>(aScrPos.X()), static_cast<LONG>(aScrPos.Y()) };
    ScreenToClient(hFrameWnd, &framePos);

    // Create a raw Win32 child window at the exact sidebar panel position
    HWND hCefParent = CreateWindowExW(
        0, L"Static", L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
        framePos.x, framePos.y, w, h,
        hFrameWnd, nullptr, GetModuleHandle(nullptr), nullptr);

    // Store HWNDs for cleanup and resize tracking
    m_hCefParentWnd = hCefParent;
    m_hFrameWnd = hFrameWnd;
    m_aLastSize = Size(w, h);
    m_aLastPos = Point(framePos.x, framePos.y);

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
    m_aResizeTimer.SetTimeout(200);
    m_aResizeTimer.Start();
}

void WebViewPanel::syncCefWindowSize()
{
    if (!m_hCefParentWnd || !m_hFrameWnd || !m_pBinWindow)
        return;

    vcl::Window* pParent = m_pBinWindow->GetParent();
    if (!pParent)
        return;

    Size aSize = pParent->GetSizePixel();
    auto aScrPos = pParent->OutputToAbsoluteScreenPixel(Point(0, 0));
    POINT pt = { static_cast<LONG>(aScrPos.X()), static_cast<LONG>(aScrPos.Y()) };
    ScreenToClient(m_hFrameWnd, &pt);
    Point aPos(pt.x, pt.y);

    if (aSize == m_aLastSize && aPos == m_aLastPos)
        return;

    m_aLastSize = aSize;
    m_aLastPos = aPos;

    SetWindowPos(m_hCefParentWnd, nullptr,
                 aPos.X(), aPos.Y(),
                 aSize.Width(), aSize.Height(),
                 SWP_NOZORDER);

    // Also resize the CEF browser window inside the parent
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
