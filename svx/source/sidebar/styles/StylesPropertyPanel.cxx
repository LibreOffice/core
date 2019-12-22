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
    : PanelLayout(pParent, "SidebarStylesPanel", "svx/ui/sidebarstylespanel.ui", rxFrame, true)
    , m_xFontStyle(m_xBuilder->weld_toolbar("fontstyletoolbox"))
    , m_xFontStyleDispatch(new weld::ToolbarUnoDispatcher(*m_xFontStyle, rxFrame))
    , m_xStyle(m_xBuilder->weld_toolbar("styleupdate"))
    , m_xStyleDispatch(new weld::ToolbarUnoDispatcher(*m_xStyle, rxFrame))
    , m_xStyleNew(m_xBuilder->weld_toolbar("stylenew"))
    , m_xStyleNewDispatch(new weld::ToolbarUnoDispatcher(*m_xStyleNew, rxFrame))
{
}

StylesPropertyPanel::~StylesPropertyPanel()
{
    disposeOnce();
}

void StylesPropertyPanel::dispose()
{
    m_xStyleNewDispatch.reset();
    m_xStyleNew.reset();
    m_xStyleDispatch.reset();
    m_xStyle.reset();
    m_xFontStyleDispatch.reset();
    m_xFontStyle.reset();

    PanelLayout::dispose();
}

void StylesPropertyPanel::DataChanged( const DataChangedEvent& /*rEvent*/)
{

}

}}
