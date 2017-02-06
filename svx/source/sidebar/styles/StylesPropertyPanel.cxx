#include <sal/config.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include "StylesPropertyPanel.hxx"

using namespace css;
using namespace css::uno;

namespace svx { namespace sidebar {

VclPtr<vcl::Window> StylesPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to StylesPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to StylesPropertyPanel::Create", nullptr, 1);

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

void StylesPropertyPanel::DataChanged( const DataChangedEvent& /*rEvent*/)
{

}

}}
