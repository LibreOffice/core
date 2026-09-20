#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <vcl/vclptr.hxx>
#include "AllMasterPagesSelector.hxx"
#include "CurrentMasterPagesSelector.hxx"
#include <vcl/InterimItemWindow.hxx>

// Used to put transition pane to the notebookbar

class SlideMasterPagesWrapper final : public InterimItemWindow
{
private:
    std::unique_ptr<sd::sidebar::CurrentMasterPagesSelector> m_xCurrentSelector;
    std::unique_ptr<sd::sidebar::AllMasterPagesSelector> m_xAllSelector;

public:
    SlideMasterPagesWrapper(vcl::Window* pParent, sd::ViewShellBase& rBase);
    virtual ~SlideMasterPagesWrapper() override;
    virtual void dispose() override;
    void SetOptimalSize();
};

using SlideMasterPagesToolBoxControl_Base
    = cppu::ImplInheritanceHelper<svt::ToolboxController, css::lang::XServiceInfo>;
class SlideMasterPagesToolBoxControl final : public SlideMasterPagesToolBoxControl_Base
{
    VclPtr<SlideMasterPagesWrapper> m_xVclBox;

public:
    SlideMasterPagesToolBoxControl();
    virtual ~SlideMasterPagesToolBoxControl() override;

    // XStatusListener
    virtual void statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    // XToolbarController
    virtual cpo::uno::Reference<css::awt::XWindow>
        createItemWindow(const cpo::uno::Reference<css::awt::XWindow>& rParent) override;

    // XInitialization
    virtual void initialize(const cpo::uno::Sequence<cpo::uno::Any>& rArguments) override;

    // WeakComponentImplHelperBase
    using SlideMasterPagesToolBoxControl_Base::disposing;
    virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& rServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};
