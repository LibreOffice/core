#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <vcl/vclptr.hxx>
#include "AllMasterPagesSelector.hxx"
#include <vcl/InterimItemWindow.hxx>

// Used to put transition pane to the notebookbar

class SlideMasterPagesAllWrapper final : public InterimItemWindow
{
private:
    std::unique_ptr<sd::sidebar::AllMasterPagesSelector> m_xPane;

public:
    SlideMasterPagesAllWrapper(vcl::Window* pParent, sd::ViewShellBase& rBase);
    virtual ~SlideMasterPagesAllWrapper() override;
    virtual void dispose() override;
    void SetOptimalSize();
};

class SlideMasterPagesAllToolBoxControl final
    : public cppu::ImplInheritanceHelper<svt::ToolboxController, css::lang::XServiceInfo>
{
    VclPtr<SlideMasterPagesAllWrapper> m_xVclBox;

public:
    SlideMasterPagesAllToolBoxControl();
    virtual ~SlideMasterPagesAllToolBoxControl() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    // XToolbarController
    virtual css::uno::Reference<css::awt::XWindow>
        SAL_CALL createItemWindow(const css::uno::Reference<css::awt::XWindow>& rParent) override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};
