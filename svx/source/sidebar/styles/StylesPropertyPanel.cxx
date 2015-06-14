#include "StylesPropertyPanel.hxx"
#include <boost/bind.hpp>

using namespace css;
using namespace css::uno;

namespace svx { namespace sidebar {

VclPtr<vcl::Window> StylesPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to StylesPropertyPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to StylesPropertyPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException("no SfxBindings given to StylesPropertyPanel::Create", NULL, 2);

    return VclPtr<StylesPropertyPanel>::Create(pParent,rxFrame,pBindings);
}

StylesPropertyPanel::StylesPropertyPanel ( vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame, SfxBindings* pBindings )
    : PanelLayout(pParent, "SidebarStylesPanel", "svx/ui/sidebarstylespanel.ui", rxFrame),
    mpBindings(pBindings)
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

void StylesPropertyPanel::NotifyItemUpdate (
    const sal_uInt16 /*nSId*/,
    const SfxItemState /*eState*/,
    const SfxPoolItem* /*pState*/,
    const bool /*bIsEnabled*/)
{

}

}}
