#pragma once

#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/weldutils.hxx>

namespace svx::sidebar{

class StylesPropertyPanel:
    public PanelLayout
{
private:
    std::unique_ptr<weld::Toolbar> m_xFontStyle;
    std::unique_ptr<ToolbarUnoDispatcher> m_xFontStyleDispatch;

    std::unique_ptr<weld::Toolbar> m_xStyle;
    std::unique_ptr<ToolbarUnoDispatcher> m_xStyleDispatch;

public:
    virtual ~StylesPropertyPanel() override;

    static std::unique_ptr<PanelLayout> Create (
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    StylesPropertyPanel(
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
};

} //end of namespace svx::sidebar
