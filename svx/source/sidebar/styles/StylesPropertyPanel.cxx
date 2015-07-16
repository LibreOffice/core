#include "StylesPropertyPanel.hxx"
#include <boost/bind.hpp>

using namespace css;
using namespace css::uno;

namespace svx { namespace sidebar {

VclPtr<vcl::Window> StylesPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to StylesPropertyPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to StylesPropertyPanel::Create", NULL, 1);

    return VclPtr<StylesPropertyPanel>::Create(pParent,rxFrame);
}

StylesPropertyPanel::StylesPropertyPanel ( vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame )
    : PanelLayout(pParent, "SidebarStylesPanel", "svx/ui/sidebarstylespanel.ui", rxFrame)
{

}

StylesPropertyPanel::~StylesPropertyPanel()
{
    disposeOnce();
}

void StylesPropertyPanel::dispose()
{
    PanelLayout::dispose();
}

void StylesPropertyPanel::DataChanged( const DataChangedEvent& /*rEvent*/)
{

}



}}
