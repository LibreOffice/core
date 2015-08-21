#include <sal/config.h>
#include <swtypes.hxx>
#include <svl/intitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/paperinf.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>
#include "PageFooterPanel.hxx"
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

VclPtr<vcl::Window> PageFooterPanel::Create(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if( pParent == NULL )
        throw ::com::sun::star::lang::IllegalArgumentException("no parent window given to PageFooterPanel::Create", NULL, 0);
    if( !rxFrame.is() )
        throw ::com::sun::star::lang::IllegalArgumentException("no XFrame given to PageFooterPanel::Create", NULL, 0);
    if( pBindings == NULL )
        throw ::com::sun::star::lang::IllegalArgumentException("no SfxBindings given to PageFooterPanel::Create", NULL, 0);

    return VclPtr<PageFooterPanel>::Create(
                pParent,
                rxFrame,
                pBindings);
}

PageFooterPanel::PageFooterPanel(
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
        SfxBindings* pBindings)
    :PanelLayout(pParent, "PageFooterPanel", "modules/swriter/ui/pagefooterpanel.ui", rxFrame)
    , mpBindings( pBindings )
    , meFUnit()
    , meUnit()
{
    Initialize();
}

PageFooterPanel::~PageFooterPanel()
{
    disposeOnce();
}

void PageFooterPanel::dispose()
{
    PanelLayout::dispose();
}

void PageFooterPanel::Initialize()
{

}

void PageFooterPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{

}

} }

