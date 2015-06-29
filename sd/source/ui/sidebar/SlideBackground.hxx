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
    virtual void Initialize();
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
    VclPtr<SvxPageWindow> mpBspWin;
    ::sfx2::sidebar::ControllerItem maPaperController;
    SfxBindings* mpBindings;
    DECL_LINK(FillBackgroundHdl, void*);
    DECL_LINK(FillStyleModifyHdl, void*);
    DECL_LINK(PaperSizeModifyHdl, void*);
    DECL_LINK(FillColorHdl, void*);
    DECL_LINK(AssignMasterPage, void*);
};

}}


#endif

