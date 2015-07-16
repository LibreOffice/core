#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_STYLES_STYLESPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_STYLES_STYLESPROPERTYPANEL_HXX

#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svtools/ctrlbox.hxx>
#include <editeng/fhgtitem.hxx>
#include <boost/scoped_ptr.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XSidebar.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>

#include <svx/sidebar/PanelLayout.hxx>

namespace svx { namespace sidebar{

class StylesPropertyPanel:
    public PanelLayout
{
public:
    virtual ~StylesPropertyPanel();
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<vcl::Window> Create (
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual void DataChanged( const DataChangedEvent& rEvent ) SAL_OVERRIDE;

    StylesPropertyPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
};

} } //end of namespace svx::sidebar

#endif
