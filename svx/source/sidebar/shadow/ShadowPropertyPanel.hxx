#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_AREA_SHADOWPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_AREA_SHADOWPROPERTYPANEL_HXX

#include <vcl/slider.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclptr.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <svl/intitem.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/slider.hxx>
#include <com/sun/star/ui/XUIElement.hpp>
#include <sfx2/request.hxx>
#include <svx/dlgctrl.hxx>



namespace svx { namespace sidebar {

class ShadowPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~ShadowPropertyPanel();
    virtual void dispose() override;

    static VclPtr<vcl::Window> Create(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings);

    virtual void DataChanged(
    const DataChangedEvent& rEvent) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) override;

    SfxBindings* GetBindings() { return mpBindings;}

    void Initialize();

    ShadowPropertyPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

private:
    VclPtr<CheckBox>   mpShowShadow;
    VclPtr<MetricBox>       mpShadowDistance;
    VclPtr<ColorLB>       mpLBShadowColor;
    VclPtr<MetricBox>       mpShadowAngle;
    VclPtr<FixedText>     mpFTAngle;
    VclPtr<FixedText>     mpFTDistance;
    VclPtr<FixedText>     mpFTTransparency;
    VclPtr<FixedText>     mpFTColor;
    VclPtr<Slider>        mpShadowTransSlider;
    VclPtr<MetricField>   mpShadowTransMetric;
    XColorListRef         pColorList;

    ::sfx2::sidebar::ControllerItem maShadowController;
    ::sfx2::sidebar::ControllerItem maShadowTransController;
    ::sfx2::sidebar::ControllerItem maShadowColorController;
    ::sfx2::sidebar::ControllerItem maShadowXDistanceController;
    ::sfx2::sidebar::ControllerItem maShadowYDistanceController;

    css::uno::Reference<css::frame::XFrame> mxFrame;

    SfxBindings* mpBindings;
    long nX,nY,nXY;

    void InsertDistanceValues();
    void InsertAngleValues();
    void SetTransparencyValue(long);
    void UpdateControls();
    DECL_LINK_TYPED(ClickShadowHdl, Button*, void);
    DECL_LINK_TYPED(ModifyShadowColorHdl, ListBox&, void);
    DECL_LINK(ModifyShadowTransMetricHdl, void*);
    DECL_LINK(ModifyShadowDistanceHdl, void*);
    DECL_LINK_TYPED(ModifyShadowTransSliderHdl, Slider*, void);
};
}
}

#endif

