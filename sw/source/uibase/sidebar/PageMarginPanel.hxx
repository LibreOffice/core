#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEMARGINPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEMARGINPANEL_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XUndoManager.hpp>

#include <svx/sidebar/Popup.hxx>
#include <svx/sidebar/PanelLayout.hxx>

#include <sfx2/sidebar/ControllerItem.hxx>

#include <i18nutil/paper.hxx>

#include <svx/pageitem.hxx>
#include <svx/rulritem.hxx>
#include <editeng/sizeitem.hxx>

#include <vcl/ctrl.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <svl/intitem.hxx>
#include <tools/fldunit.hxx>
#include <svl/poolitem.hxx>
#include <svx/rulritem.hxx>
#include <boost/scoped_ptr.hpp>

namespace sw { namespace sidebar {

class PageMarginPanel
            : public PanelLayout,
              public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static VclPtr<vcl::Window> Create(
            vcl::Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
            SfxBindings* pBindings);

    virtual void NotifyItemUpdate(
            const sal_uInt16 nSID,
            const SfxItemState eState,
            const SfxPoolItem* pState,
            const bool bIsEnabled);

    SfxBindings* GetBindings() const { return mpBindings; }

    PageMarginPanel(
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
        SfxBindings* pBindings);
    virtual ~PageMarginPanel();
    virtual void dispose();

private:

    SfxBindings* mpBindings;

    VclPtr<MetricField> mpLeftMarginEdit;
    VclPtr<MetricField> mpRightMarginEdit;
    VclPtr<MetricField> mpTopMarginEdit;
    VclPtr<MetricField> mpBottomMarginEdit;

    FieldUnit meFUnit;
    SfxMapUnit meUnit;

    long mnPageLeftMargin;
    long mnPageRightMargin;
    long mnPageTopMargin;
    long mnPageBottomMargin;

    void Initialize();
    void ExecuteMarginLRChange( const long nPageLeftMargin, const long nPageRightMargin );
    void ExecuteMarginULChange( const long nPageTopMargin, const long  nPageBottomMargin);
    void MetricState( SfxItemState eState, const SfxPoolItem* pState );

    ::boost::scoped_ptr<SvxLongLRSpaceItem> mpPageLRMarginItem;
    ::boost::scoped_ptr<SvxLongULSpaceItem> mpPageULMarginItem;

    ::sfx2::sidebar::ControllerItem maSwPageLRControl;
    ::sfx2::sidebar::ControllerItem maSwPageULControl;
    ::sfx2::sidebar::ControllerItem maSwPageSizeControl;
    ::sfx2::sidebar::ControllerItem maSwPageMetricControl;

    DECL_LINK( ModifyLRMarginHdl, void* );
    DECL_LINK( ModifyULMarginHdl, void* );

};

} } // end of namespace sw::sidebar

#endif
