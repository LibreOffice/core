#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

class TheApplication : public Application
{
public:
    virtual int Main();
};
class TheWindow : public WorkWindow
{
public:
    TheWindow(Window* parent, WinBits windowStyle)
        : WorkWindow(parent, windowStyle)
    {
    }
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect);
};

void TheWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    rRenderContext.DrawText(Point(rRect.GetWidth() / 2, rRect.getHeight() / 2),
                            OUString(u"VCL module in LibreOffice"));
}

int TheApplication::Main()
{
    TheWindow aWindow(NULL, WB_APP | WB_STDWORK);
    aWindow.SetText(u"VCL");
    aWindow.Show();
    Execute();
    return 0;
}

int main()
{
    auto xContext = cppu::defaultBootstrap_InitialComponentContext();
    css::uno::Reference<css::lang::XMultiServiceFactory> xServiceManager(
        xContext->getServiceManager(), css::uno::UNO_QUERY);
    comphelper::setProcessServiceFactory(xServiceManager);

    TheApplication anApplication;
    InitVCL();
    int ret = anApplication.Main();
    DeInitVCL();

    return ret;
}
