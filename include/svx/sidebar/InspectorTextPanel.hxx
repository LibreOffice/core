
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/weldutils.hxx>
#include <vcl/EnumContext.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>

namespace svx
{
namespace sidebar
{
class InspectorTextPanel : public PanelLayout, public ::sfx2::sidebar::IContextChangeReceiver
{
public:
    virtual ~InspectorTextPanel() override;
    virtual void dispose() override;

    static VclPtr<vcl::Window> Create(vcl::Window* pParent,
                                      const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual void HandleContextChange(const vcl::EnumContext& rContext) override;

    InspectorTextPanel(vcl::Window* pParent,
                       const css::uno::Reference<css::frame::XFrame>& rxFrame);

private:
    std::unique_ptr<weld::TreeView> mxListBoxStyles;

    vcl::EnumContext maContext;
};
}
} // end of namespace svx::sidebar
