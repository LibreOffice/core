#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <ViewShellBase.hxx>
#include <sfx2/viewsh.hxx>
#include "CurrentMasterPagesSelector.hxx"
#include "MasterPageContainer.hxx"
#include "AllMasterPagesSelector.hxx"
#include "SlideMasterPagesToolBoxControl.hxx"

SlideMasterPagesToolBoxControl::SlideMasterPagesToolBoxControl() {}

SlideMasterPagesToolBoxControl::~SlideMasterPagesToolBoxControl() {}

void SAL_CALL
SlideMasterPagesToolBoxControl::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
{
    SAL_INFO("sd", "initialize called for SlideMasterPagesToolBoxControl");

    svt::ToolboxController::initialize(rArguments);
}

void SAL_CALL
SlideMasterPagesToolBoxControl::statusChanged(const css::frame::FeatureStateEvent& /*rEvent*/)
{
    // Handle status changes if needed
}

void SAL_CALL SlideMasterPagesToolBoxControl::dispose()
{
    SolarMutexGuard aSolarMutexGuard;
    m_xVclBox.disposeAndClear();
    svt::ToolboxController::dispose();
}

css::uno::Reference<css::awt::XWindow> SlideMasterPagesToolBoxControl::createItemWindow(
    const css::uno::Reference<css::awt::XWindow>& rParent)
{
    css::uno::Reference<css::awt::XWindow> xItemWindow;
    SAL_INFO("sd", "createItemWindow called for SlideMasterPagesToolBoxControl");

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
            m_xVclBox = VclPtr<SlideMasterPagesWrapper>::Create(pParent, *pViewShellBase);
            xItemWindow = VCLUnoHelper::GetInterface(m_xVclBox.get());
        }
    }
    SAL_INFO("sd", "createItemWindow called for SlideMasterPagesToolBoxControl");

    return xItemWindow;
}

OUString SAL_CALL SlideMasterPagesToolBoxControl::getImplementationName()
{
    return u"com.sun.star.comp.sd.SlideMasterPagesToolBoxControl"_ustr;
}

css::uno::Sequence<OUString> SAL_CALL SlideMasterPagesToolBoxControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}

sal_Bool SAL_CALL SlideMasterPagesToolBoxControl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sd_SlideMasterPagesToolBoxControl_get_implementation(
    css::uno::XComponentContext* /*rxContext*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SlideMasterPagesToolBoxControl());
}

SlideMasterPagesWrapper::SlideMasterPagesWrapper(vcl::Window* pParent, sd::ViewShellBase& rBase)
    : InterimItemWindow(pParent, u"modules/simpress/ui/masterpagepanelall.ui"_ustr,
                        u"MasterPagePanel"_ustr, true,
                        reinterpret_cast<sal_uInt64>(SfxViewShell::Current()))
{
    SdDrawDocument* pDocument = rBase.GetDocument();
    if (pDocument == nullptr)
        SAL_INFO("sd", "SlideMasterPagesWrapper got null SdDrawDocument");

    auto pContainer = std::make_shared<sd::sidebar::MasterPageContainer>();

    m_xCurrentSelector = std::make_unique<sd::sidebar::CurrentMasterPagesSelector>(
        m_xContainer.get(), *pDocument, rBase, pContainer);
    m_xAllSelector = std::make_unique<sd::sidebar::AllMasterPagesSelector>(
        m_xContainer.get(), *pDocument, rBase, pContainer);
    SetOptimalSize();
    SAL_INFO("sd", "SlideMasterPagesWrapper created");
}

SlideMasterPagesWrapper::~SlideMasterPagesWrapper() { disposeOnce(); }
void SlideMasterPagesWrapper::SetOptimalSize() { SetSizePixel(GetOptimalSize()); }

void SlideMasterPagesWrapper::dispose()
{
    SAL_INFO("sd", "SlideMasterPagesWrapper dispose ");
    m_xCurrentSelector.reset();
    m_xAllSelector.reset();
    InterimItemWindow::dispose();
}
