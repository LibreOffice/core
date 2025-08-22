#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <ViewShellBase.hxx>
#include <sfx2/viewsh.hxx>
#include "MasterPageContainer.hxx"
#include "AllMasterPagesSelector.hxx"
#include "SlideMasterPagesAllToolBoxControl.hxx"

SlideMasterPagesAllToolBoxControl::SlideMasterPagesAllToolBoxControl() {}

SlideMasterPagesAllToolBoxControl::~SlideMasterPagesAllToolBoxControl() {}

void SAL_CALL
SlideMasterPagesAllToolBoxControl::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
{
    SAL_INFO("sd", "initialize called for SlideMasterPagesAllToolBoxControl");

    svt::ToolboxController::initialize(rArguments);
}

void SAL_CALL
SlideMasterPagesAllToolBoxControl::statusChanged(const css::frame::FeatureStateEvent& /*rEvent*/)
{
    // Handle status changes if needed
}

void SAL_CALL SlideMasterPagesAllToolBoxControl::dispose()
{
    SolarMutexGuard aSolarMutexGuard;
    m_xVclBox.disposeAndClear();
    svt::ToolboxController::dispose();
}

css::uno::Reference<css::awt::XWindow> SlideMasterPagesAllToolBoxControl::createItemWindow(
    const css::uno::Reference<css::awt::XWindow>& rParent)
{
    css::uno::Reference<css::awt::XWindow> xItemWindow;
    SAL_INFO("sd", "createItemWindow called for SlideMasterPagesAllToolBoxControl");

    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow(rParent);
    if (pParent)
    {
        SolarMutexGuard aSolarMutexGuard;

        sd::ViewShellBase* pViewShellBase = nullptr;
        if (SfxViewShell* pViewShell = SfxViewShell::Current())
        {
            pViewShellBase = dynamic_cast<sd::ViewShellBase*>(pViewShell);
        }

        if (pViewShellBase)
        {
            m_xVclBox = VclPtr<SlideMasterPagesAllWrapper>::Create(pParent, *pViewShellBase);
            xItemWindow = VCLUnoHelper::GetInterface(m_xVclBox.get());
        }
    }
    SAL_INFO("sd", "createItemWindow called for SlideMasterPagesAllToolBoxControl");

    return xItemWindow;
}

OUString SAL_CALL SlideMasterPagesAllToolBoxControl::getImplementationName()
{
    return u"com.sun.star.comp.sd.SlideMasterPagesAllToolBoxControl"_ustr;
}

css::uno::Sequence<OUString> SAL_CALL SlideMasterPagesAllToolBoxControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}

sal_Bool SAL_CALL SlideMasterPagesAllToolBoxControl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sd_SlideMasterPagesAllToolBoxControl_get_implementation(
    css::uno::XComponentContext* /*rxContext*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SlideMasterPagesAllToolBoxControl());
}

SlideMasterPagesAllWrapper::SlideMasterPagesAllWrapper(vcl::Window* pParent,
                                                       sd::ViewShellBase& rBase)
    : InterimItemWindow(pParent, u"modules/simpress/ui/masterpagepanelall.ui"_ustr,
                        u"MasterPagePanel"_ustr, true,
                        reinterpret_cast<sal_uInt64>(SfxViewShell::Current()))
{
    SdDrawDocument* pDocument = rBase.GetDocument();
    if (pDocument == nullptr)
        SAL_INFO("sd", "SlideMasterPagesAllWrapper got null SdDrawDocument");

    m_xPane = std::make_unique<sd::sidebar::AllMasterPagesSelector>(
        m_xContainer.get(), *pDocument, rBase,
        std::make_shared<sd::sidebar::MasterPageContainer>());
    SetOptimalSize();
    SAL_INFO("sd", "SlideMasterPagesAllWrapper created");
}

SlideMasterPagesAllWrapper::~SlideMasterPagesAllWrapper() { disposeOnce(); }
void SlideMasterPagesAllWrapper::SetOptimalSize() { SetSizePixel(GetOptimalSize()); }

void SlideMasterPagesAllWrapper::dispose()
{
    SAL_INFO("sd", "SlideMasterPagesAllWrapper dispose ");
    m_xPane.reset();
    InterimItemWindow::dispose();
}
