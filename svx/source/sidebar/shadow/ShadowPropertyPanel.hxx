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
#include <boost/scoped_ptr.hpp>
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
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<vcl::Window> Create(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings);

    virtual void DataChanged(
    const DataChangedEvent& rEvent) SAL_OVERRIDE;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) SAL_OVERRIDE;

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
    bool bDisabled;

    ::sfx2::sidebar::ControllerItem maShadowController;
    ::sfx2::sidebar::ControllerItem maShadowTransController;
    ::sfx2::sidebar::ControllerItem maShadowColorController;
    ::sfx2::sidebar::ControllerItem maShadowXDistanceController;
    ::sfx2::sidebar::ControllerItem maShadowYDistanceController;

    css::uno::Reference<css::frame::XFrame> mxFrame;

    SfxBindings* mpBindings;
    long nX,nY,nXY;

    static void SetupIcons();
    void InsertDistanceValues();
    void InsertAngleValues();
    void SetTransparencyValue(long);
    void UpdateControls();
    sal_uInt32 ParseText(OUString pStr);
    DECL_LINK(ClickShadowHdl, void*);
    DECL_LINK(ModifyShadowColorHdl, void*);
    DECL_LINK(ModifyShadowTransMetricHdl, void*);
    DECL_LINK(ModifyShadowDistanceHdl, void*);
    DECL_LINK(ModifyShadowTransSliderHdl, void*);
};
}
}

#endif

