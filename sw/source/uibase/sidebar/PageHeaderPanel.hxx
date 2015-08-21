#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEHEADERPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEHEADERPANEL_HXX

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

class PageHeaderPanel
            : public PanelLayout,
              public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static VclPtr<vcl::Window> Create(
            vcl::Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
            SfxBindings* pBindings);

    virtual void NotifyItemUpdate(
            const sal_uInt16 nSId,
            const SfxItemState eState,
            const SfxPoolItem* pState,
            const bool bIsEnabled);

    SfxBindings* GetBindings() const { return mpBindings; }
    PageHeaderPanel(
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
        SfxBindings* pBindings);
    virtual ~PageHeaderPanel();
    virtual void dispose();

private:

    SfxBindings* mpBindings;

    VclPtr<CheckBox> mpHeaderToggle;
    VclPtr<MetricField> mpHeaderHeightField;
    VclPtr<MetricField> mpHeaderLMargin;
    VclPtr<MetricField> mpHeaderRMargin;
    VclPtr<MetricField> mpHeaderSpacing;
    VclPtr<ListBox> mpSameContentLB;

    FieldUnit meFUnit;
    SfxMapUnit meUnit;

    void Initialize();
    void UpdateControls();

    DECL_LINK( HeaderToggleHdl, void* );

};

} } //end of namespace sw::sidebar

#endif
