#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_SLIDEBACKGROUND_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_SLIDEBACKGROUND_HXX

#include <vcl/ctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <svx/pagectrl.hxx>
#include "ViewShellBase.hxx"
#include <svx/itemwin.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "fupage.hxx"
#include <svx/xflclit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xbitmap.hxx>
#include <svx/xflbckit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflhtit.hxx>

namespace sd { namespace sidebar {

class SlideBackground :
    public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    SlideBackground(
        Window * pParent,
        ViewShellBase& rBase,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings );
    virtual ~SlideBackground();
    virtual void dispose();
    SfxBindings* GetBindings() { return mpBindings; }
    // Window
    virtual void DataChanged (const DataChangedEvent& rEvent);
    virtual void NotifyItemUpdate(
        const sal_uInt16 nSID,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled);

private:

    ViewShellBase& mrBase;

    VclPtr<ListBox> mpPaperSizeBox;
    VclPtr<ListBox> mpPaperOrientation;
    VclPtr<ListBox> mpMasterSlide;
    VclPtr<ListBox> mpFillStyle;
    VclPtr<ColorLB> mpFillLB;
    VclPtr<SvxFillAttrBox> mpFillAttr;
    VclPtr<ColorLB> mpFillGrad;
    VclPtr<CheckBox> mpDspMasterBackground;
    VclPtr<CheckBox> mpDspMasterObjects;

    ::sfx2::sidebar::ControllerItem maPaperController;
    ::sfx2::sidebar::ControllerItem maBckColorController;
    ::sfx2::sidebar::ControllerItem maBckGradientController;
    ::sfx2::sidebar::ControllerItem maBckHatchController;
    ::sfx2::sidebar::ControllerItem maBckBitmapController;
    ::sfx2::sidebar::ControllerItem maBckFillStyleController;
    ::sfx2::sidebar::ControllerItem maBckImageController;
    ::sfx2::sidebar::ControllerItem maDspBckController;
    ::sfx2::sidebar::ControllerItem maDspObjController;
    ::sfx2::sidebar::ControllerItem maMetricController;

    std::unique_ptr< XFillStyleItem > mpFillStyleItem;
    std::unique_ptr< XFillColorItem > mpColorItem;
    std::unique_ptr< XFillGradientItem > mpGradientItem;
    std::unique_ptr< XFillHatchItem > mpHatchItem;
    std::unique_ptr< XFillBitmapItem > mpBitmapItem;

    SfxBindings* mpBindings;

    SfxMapUnit meUnit;

    DECL_LINK_TYPED(FillBackgroundHdl, ListBox&, void);
    DECL_LINK_TYPED(FillStyleModifyHdl, ListBox&, void);
    DECL_LINK_TYPED(PaperSizeModifyHdl, ListBox&, void);
    DECL_LINK_TYPED(FillColorHdl, ListBox&, void);
    DECL_LINK_TYPED(AssignMasterPage, ListBox&, void);
    DECL_LINK_TYPED(DspBackground, Button*, void);
    DECL_LINK_TYPED(DspObjects, Button*, void);

    void Initialize();
    void Update();
};

}}


#endif

