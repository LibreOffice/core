#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_STYLES_STYLESPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_STYLES_STYLESPROPERTYPANEL_HXX

#include <svx/sidebar/PanelLayout.hxx>
#include <sfx2/weldutils.hxx>

namespace svx { namespace sidebar{

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

    static VclPtr<vcl::Window> Create (
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual void DataChanged( const DataChangedEvent& rEvent ) override;

    virtual void dispose() override;

    StylesPropertyPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
};

} } //end of namespace svx::sidebar

#endif
