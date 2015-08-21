#include <sal/config.h>
#include <swtypes.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/paperinf.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>
#include "PageHeaderPanel.hxx"
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include "cmdid.h"
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

namespace sw { namespace sidebar{

VclPtr<vcl::Window> PageHeaderPanel::Create(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if( pParent == NULL )
        throw ::com::sun::star::lang::IllegalArgumentException("no parent window given to PageHeaderPanel::Create", NULL, 0);
    if( !rxFrame.is() )
        throw ::com::sun::star::lang::IllegalArgumentException("no XFrame given to PageHeaderPanel::Create", NULL, 0);
    if( pBindings == NULL )
        throw ::com::sun::star::lang::IllegalArgumentException("no SfxBindings given to PageHeaderPanel::Create", NULL, 0);

    return VclPtr<PageHeaderPanel>::Create(
                pParent,
                rxFrame,
                pBindings);
}

PageHeaderPanel::PageHeaderPanel(
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
        SfxBindings* pBindings)
    :PanelLayout(pParent, "PageHeaderPanel", "modules/swriter/ui/pageheaderpanel.ui", rxFrame)
    , mpBindings( pBindings )
    , meFUnit()
    , meUnit()
{
    get(mpHeaderToggle, "headertoggle");
    get(mpHeaderHeightField, "heightspinfield");
    get(mpHeaderLMargin, "leftmargin");
    get(mpHeaderRMargin, "rightmargin");
    get(mpHeaderSpacing, "spacingspinfield");
    get(mpSameContentLB, "samecontentLB");

    Initialize();
}

PageHeaderPanel::~PageHeaderPanel()
{
    disposeOnce();
}

void PageHeaderPanel::dispose()
{
    mpHeaderToggle.disposeAndClear();
    mpHeaderHeightField.disposeAndClear();
    mpHeaderLMargin.disposeAndClear();
    mpHeaderRMargin.disposeAndClear();
    mpHeaderSpacing.disposeAndClear();
    mpSameContentLB.disposeAndClear();

    PanelLayout::dispose();
}

void PageHeaderPanel::Initialize()
{
    mpHeaderToggle->SetClickHdl( LINK(this, PageHeaderPanel, HeaderToggleHdl) );
}

void PageHeaderPanel::UpdateControls()
{
    bool IsEnabled = (bool)mpHeaderToggle->IsChecked();
    if(IsEnabled)
    {
        mpHeaderHeightField->Enable();
        mpHeaderLMargin->Enable();
        mpHeaderRMargin->Enable();
        mpHeaderSpacing->Enable();
        mpSameContentLB->Enable();
    }
    else
    {
        mpHeaderHeightField->Disable();
        mpHeaderLMargin->Disable();
        mpHeaderRMargin->Disable();
        mpHeaderSpacing->Disable();
        mpSameContentLB->Disable();
    }
}

void PageHeaderPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
}

IMPL_LINK_NOARG( PageHeaderPanel, HeaderToggleHdl )
{
    bool IsChecked = mpHeaderToggle->IsChecked();
    SfxBoolItem aItem(SID_ATTR_PAGE_ON, IsChecked);
    GetBindings()->GetDispatcher()->Execute( SID_ATTR_PAGE_HEADERSET, SfxCallMode::RECORD, &aItem, 0l );
    UpdateControls();
    return 0;
}



} }
