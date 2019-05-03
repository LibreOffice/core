#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>

#include "ChartStylesPanel.hxx"

using namespace css;
using namespace css::uno;

namespace chart { namespace sidebar {

  VclPtr<vcl::Window> ChartStylesPanel::Create (
      vcl::Window* pParent,
      const css::uno::Reference<css::frame::XFrame>& rxFrame,
      ChartController* pController )
  {
      if (pParent == nullptr)
          throw lang::IllegalArgumentException("no parent Window given to ChartStylesPanel::Create", nullptr, 0);
      if ( ! rxFrame.is())
          throw lang::IllegalArgumentException("no XFrame given to ChartStylesPanel::Create", nullptr, 1);

      return  VclPtr<ChartStylesPanel>::Create(
                          pParent, rxFrame, pController);
  }

ChartStylesPanel::ChartStylesPanel ( vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController )
  : PanelLayout(pParent, "ChartStylesPanel", "modules/schart/ui/sidebarstyle.ui", rxFrame)
{
    get(aStyleList, "stylelist");
    get(aApplyButton, "setstyle");
    get(aNewButton, "createstyle");
    get(aDefaultButton, "setdefault");
    get(aDeleteButton, "deletestyle");

    aStyleList->set_height_request(45 * aStyleList->GetTextHeight());
    Link<ListBox&,void> aLink=LINK( this, ChartStylesPanel, SelHdl);

    aStyleList->SetSelectHdl(aLink);
    aStyleList->SetDoubleClickHdl(LINK( this, ChartStylesPanel, SetSelectionHdl ));
    aApplyButton->SetClickHdl(LINK( this, ChartStylesPanel, SetSelectionClickHdl ));
    aNewButton->SetClickHdl(LINK( this, ChartStylesPanel, SetSelectionClickHdl ));
    aDefaultButton->SetClickHdl(LINK( this, ChartStylesPanel, SetSelectionClickHdl ));
    aDeleteButton->SetClickHdl(LINK( this, ChartStylesPanel, SetSelectionClickHdl ));
}

ChartStylesPanel::~ChartStylesPanel()
{
    disposeOnce();
}

void ChartStylesPanel::dispose()
{
    aStyleList.clear();
    aDefaultButton.clear();
    aApplyButton.clear();
    aNewButton.clear();
    aDeleteButton.clear();
    PanelLayout::dispose();
}

void ChartStylesPanel::UpdateList()
{}

IMPL_LINK( ChartStylesPanel, SelHdl, ListBox&, rLb, void )
{}

IMPL_LINK_NOARG( ChartStylesPanel, SetSelectionClickHdl, Button*, void )
{}

IMPL_LINK_NOARG( ChartStylesPanel, SetSelectionHdl, ListBox&, void )
{}

}}
