/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs CEF Subprocess Entry Point
 *
 * Handles renderer, GPU, utility, and other helper processes.
 * This is NOT the main LibreOffice process.
 *
 * IMPORTANT: The renderer process must set up CefMessageRouterRendererSide
 * so that window.cefQuery() is injected into web pages. Without this,
 * the React UI cannot communicate with the C++ host.
 */

#ifdef HAVE_FEATURE_CEF

#include <include/cef_app.h>
#include <include/wrapper/cef_message_router.h>

// Renderer-side CefApp: sets up the message router in the render process
// so that window.cefQuery / window.cefQueryCancel are available in JS.
class OfficelabsRendererApp : public CefApp,
                               public CefRenderProcessHandler
{
public:
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override
    {
        return this;
    }

    void OnWebKitInitialized() override
    {
        // Create the renderer-side message router with the same config
        // as the browser-side router in WebViewPanel::initCefBrowser().
        CefMessageRouterConfig config;
        config.js_query_function = "cefQuery";
        config.js_cancel_function = "cefQueryCancel";
        m_messageRouter = CefMessageRouterRendererSide::Create(config);
    }

    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override
    {
        // This injects window.cefQuery into the page's JS context
        m_messageRouter->OnContextCreated(browser, frame, context);
    }

    void OnContextReleased(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefV8Context> context) override
    {
        m_messageRouter->OnContextReleased(browser, frame, context);
    }

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefProcessId source_process,
                                   CefRefPtr<CefProcessMessage> message) override
    {
        return m_messageRouter->OnProcessMessageReceived(
            browser, frame, source_process, message);
    }

    IMPLEMENT_REFCOUNTING(OfficelabsRendererApp);

private:
    CefRefPtr<CefMessageRouterRendererSide> m_messageRouter;
};

#ifdef _WIN32
#include <windows.h>

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                      LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    CefMainArgs main_args(hInstance);
    CefRefPtr<OfficelabsRendererApp> app(new OfficelabsRendererApp);
    return CefExecuteProcess(main_args, app, nullptr);
}

#else

int main(int argc, char* argv[])
{
    CefMainArgs main_args(argc, argv);
    CefRefPtr<OfficelabsRendererApp> app(new OfficelabsRendererApp);
    return CefExecuteProcess(main_args, app, nullptr);
}

#endif

#endif // HAVE_FEATURE_CEF

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
