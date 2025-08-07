#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <SlideTransitionPane.hxx>
#include <ViewShellBase.hxx>
#include <sfx2/viewsh.hxx>
#include "SlideTransitionsToolBoxControl.hxx"

SlideTransitionsToolBoxControl::SlideTransitionsToolBoxControl() {}

SlideTransitionsToolBoxControl::~SlideTransitionsToolBoxControl() {}

void SAL_CALL
SlideTransitionsToolBoxControl::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
{
    SAL_INFO("sd", "initialize called for TransitionToolBoxControl");

    svt::ToolboxController::initialize(rArguments);
}

void SAL_CALL SlideTransitionsToolBoxControl::dispose()
{
    SolarMutexGuard aSolarMutexGuard;
    m_xVclBox.disposeAndClear();
    svt::ToolboxController::dispose();
}

void SAL_CALL
SlideTransitionsToolBoxControl::statusChanged(const css::frame::FeatureStateEvent& /*rEvent*/)
{
    // Handle status changes if needed
}

css::uno::Reference<css::awt::XWindow> SlideTransitionsToolBoxControl::createItemWindow(
    const css::uno::Reference<css::awt::XWindow>& rParent)
{
    css::uno::Reference<css::awt::XWindow> xItemWindow;
    SAL_INFO("sd", "createItemWindow called for SlideTransitionsToolBoxControl");

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
            m_xVclBox = VclPtr<SlideTransitionsPaneWrapper>::Create(pParent, *pViewShellBase);
            xItemWindow = VCLUnoHelper::GetInterface(m_xVclBox.get());
        }
    }
    SAL_INFO("sd", "createItemWindow called for TransitionToolBoxControl");

    return xItemWindow;
}

void SAL_CALL SlideTransitionsToolBoxControl::update()
{
    // Update logic if needed
}

OUString SAL_CALL SlideTransitionsToolBoxControl::getImplementationName()
{
    return u"com.sun.star.comp.sd.SlideTransitionsToolBoxControl"_ustr;
}

sal_Bool SAL_CALL SlideTransitionsToolBoxControl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL SlideTransitionsToolBoxControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}

// Export function for service registration
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sd_SlideTransitionsToolBoxControl_get_implementation(
    css::uno::XComponentContext* /*rxContext*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SlideTransitionsToolBoxControl());
}

SlideTransitionsPaneWrapper::SlideTransitionsPaneWrapper(vcl::Window* pParent,
                                                         sd::ViewShellBase& rBase)
    : InterimItemWindow(pParent, u"modules/simpress/ui/slidetransitionspanel.ui"_ustr,
                        u"SlideTransitionsPanel"_ustr, true,
                        reinterpret_cast<sal_uInt64>(SfxViewShell::Current()))
{
    // Create SlideTransitionsPane with the container widget
    m_xTransitionPane = std::make_unique<sd::SlideTransitionPane>(m_xContainer.get(), rBase);
    SetOptimalSize();
    SAL_INFO("sd", "SlideTransitionsPaneWrapper created with SlideTransitionsPane");
}

SlideTransitionsPaneWrapper::~SlideTransitionsPaneWrapper() { disposeOnce(); }
void SlideTransitionsPaneWrapper::SetOptimalSize() { SetSizePixel(GetOptimalSize()); }

void SlideTransitionsPaneWrapper::dispose()
{
    SAL_INFO("sd", "SlideTransitionsPaneWrapper dispose ");
    m_xTransitionPane.reset();
    InterimItemWindow::dispose();
}
