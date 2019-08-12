#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>

#include "ChartStylesPanel.hxx"

using namespace css;
using namespace css::uno;

namespace chart
{
namespace sidebar
{
VclPtr<vcl::Window> ChartStylesPanel::Create(vcl::Window* pParent,
                                             const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ChartStylesPanel::Create",
                                             nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartStylesPanel::Create", nullptr,
                                             1);

    return VclPtr<ChartStylesPanel>::Create(pParent, rxFrame);
}

ChartStylesPanel::ChartStylesPanel(vcl::Window* pParent,
                                   const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, "ChartStylesPanel", "modules/schart/ui/sidebarstyle.ui", rxFrame)
{
    get(aStyleList, "stylelist");
    get(aApplyButton, "setstyle");
    get(aNewButton, "createstyle");
    get(aDefaultButton, "setdefault");
    get(aDeleteButton, "deletestyle");

    aStyleList->set_height_request(45 * aStyleList->GetTextHeight());
    Link<ListBox&, void> aLink = LINK(this, ChartStylesPanel, SelHdl);
    Link<Button*, void> aLink2 = LINK(this, ChartStylesPanel, SetSelectionClickHdl);

    aStyleList->SetSelectHdl(aLink);
    aStyleList->SetDoubleClickHdl(LINK(this, ChartStylesPanel, SetSelectionHdl));
    aApplyButton->SetClickHdl(aLink2);
    aNewButton->SetClickHdl(aLink2);
    aDefaultButton->SetClickHdl(aLink2);
    aDeleteButton->SetClickHdl(aLink2);
}

ChartStylesPanel::~ChartStylesPanel() { disposeOnce(); }

void ChartStylesPanel::dispose()
{
    aStyleList.clear();
    aDefaultButton.clear();
    aApplyButton.clear();
    aNewButton.clear();
    aDeleteButton.clear();
    PanelLayout::dispose();
}

void ChartStylesPanel::UpdateList() {}

IMPL_LINK_NOARG(ChartStylesPanel, SelHdl, ListBox&, void) {}

IMPL_LINK_NOARG(ChartStylesPanel, SetSelectionClickHdl, Button*, void) {}

IMPL_LINK_NOARG(ChartStylesPanel, SetSelectionHdl, ListBox&, void) {}

} // namespace sidebar
} // namespace chart
