/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTSTYLESPANEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTSTYLESPANEL_HXX

#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <vcl/EnumContext.hxx>
#include <svtools/ctrlbox.hxx>
#include <editeng/fhgtitem.hxx>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XSidebar.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/chart2/XChartStyle.hpp>

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <svx/sidebar/PanelLayout.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace util
{
class XModifyListener;
}
} // namespace star
} // namespace sun
} // namespace com
namespace com
{
namespace sun
{
namespace star
{
namespace view
{
class XSelectionChangeListener;
}
} // namespace star
} // namespace sun
} // namespace com

namespace chart
{
class ChartController;

namespace sidebar
{
class ChartStylesPanel : public PanelLayout
{
private:
    css::uno::Reference<css::frame::XModel>             m_xModel;
    css::uno::Reference<css::container::XNameContainer> m_xChartStyles;

    VclPtr<ListBox> aStyleList;
    VclPtr<PushButton> aApplyButton;
    VclPtr<PushButton> aNewButton;
    VclPtr<PushButton> aDefaultButton;
    VclPtr<PushButton> aDeleteButton;

    DECL_LINK(SetSelectionHdl, ListBox&, void);
    DECL_LINK(SetSelectionClickHdl, Button*, void);
    DECL_LINK(SelHdl, ListBox&, void);

    void UpdateList();
    void Initialize();

    css::uno::Reference<css::chart2::XChartStyle> updateStyleFromChart(css::uno::Reference<css::chart2::XChartStyle>& xChartStyle);
    void createNewStyle(const OUString& sName);

public:
    static VclPtr<vcl::Window> Create(vcl::Window* pParent,
                                      const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                      const css::uno::Reference<css::frame::XController>& rxController);

    // constructor/destructor
    ChartStylesPanel(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                           const css::uno::Reference<css::frame::XController>& pController);

    virtual ~ChartStylesPanel() override;
    virtual void dispose() override;
};

} // namespace sidebar
} // namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
