/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <iostream>
#include "ChartStylesPanel.hxx"
#include <ChartStyle.hxx>
#include <ChartModel.hxx>
#include <CloneHelper.hxx>
#include <com/sun/star/chart2/ChartObjectType.hpp>

using namespace css;
using namespace css::uno;

namespace chart
{
namespace sidebar
{

namespace
{

ChartModel* getChartModel(const css::uno::Reference<css::frame::XModel>& xModel)
{
    ChartModel* pModel = dynamic_cast<ChartModel*>(xModel.get());
    return pModel;
}

css::uno::Reference<css::chart2::XChartStyle> getCurrentStyle(const css::uno::Reference<css::frame::XModel>& xModel)
{
    return getChartModel(xModel)->getChartStyle();
}

} // namespace anonymous

VclPtr<vcl::Window> ChartStylesPanel::Create(vcl::Window* pParent,
                                             const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                             const css::uno::Reference<css::frame::XController>& pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ChartStylesPanel::Create",
                                             nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartStylesPanel::Create", nullptr,
                                             1);

    return VclPtr<ChartStylesPanel>::Create(pParent, rxFrame, pController);
}

ChartStylesPanel::ChartStylesPanel(vcl::Window* pParent,
                                   const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                   const css::uno::Reference<css::frame::XController>& pController)
    : PanelLayout(pParent, "ChartStylesPanel", "modules/schart/ui/sidebarstyle.ui", rxFrame)
    , m_xModel( pController->getModel() )
    , m_xChartStyles( ::chart::getChartStyles() )
{
    get(aStyleList, "stylelist");
    get(aApplyButton, "setstyle");
    get(aNewButton, "createstyle");
    get(aDefaultButton, "setdefault");
    get(aDeleteButton, "deletestyle");
    get(aDefaultText, "default_style");

    Link<ListBox&, void> aLink = LINK(this, ChartStylesPanel, SelHdl);
    Link<Button*, void> aLink2 = LINK(this, ChartStylesPanel, SetSelectionClickHdl);

    aStyleList->SetSelectHdl(aLink);
    aStyleList->SetDoubleClickHdl(LINK(this, ChartStylesPanel, SetSelectionHdl));
    aApplyButton->SetClickHdl(aLink2);
    aNewButton->SetClickHdl(aLink2);
    aDefaultButton->SetClickHdl(aLink2);
    aDeleteButton->SetClickHdl(aLink2);

    Initialize();
}

ChartStylesPanel::~ChartStylesPanel() { disposeOnce(); }

void ChartStylesPanel::dispose()
{
    aStyleList.clear();
    aDefaultButton.clear();
    aApplyButton.clear();
    aNewButton.clear();
    aDeleteButton.clear();
    aDefaultText.clear();
    PanelLayout::dispose();
}

void ChartStylesPanel::createNewStyle(const OUString& sName)
{
    css::uno::Reference<css::chart2::XChartStyle> xCurrentStyle = getCurrentStyle(m_xModel);

    css::uno::Reference<css::chart2::XChartStyle> xNewStyle = CloneHelper::CreateRefClone<css::chart2::XChartStyle>()(xCurrentStyle);
    xNewStyle->updateChartStyle(css::uno::Reference<css::chart2::XChartDocument>(m_xModel, css::uno::UNO_QUERY));
    css::uno::Reference<css::style::XStyle> xStyle(xNewStyle, css::uno::UNO_QUERY);
    xStyle->setName(sName);
    m_xChartStyles->insertByName(sName, css::uno::Any(xNewStyle));

    UpdateList();
}

void ChartStylesPanel::updateDefaultStyle(const OUString& sName)
{
    m_xChartStyles->replaceByName("Default", css::uno::Any(m_xChartStyles->getByName(sName)));

    aDefaultText->SetText(sName);
}

void ChartStylesPanel::UpdateList()
{
    aStyleList->Clear();
    css::uno::Sequence<OUString> aStyleNames = m_xChartStyles->getElementNames();
    for ( auto& rStyle : aStyleNames )
    {
        if (rStyle != "Default")
            aStyleList->InsertEntry( rStyle );
        else
        {
            css::uno::Reference<css::style::XStyle> xStyle;
            m_xChartStyles->getByName("Default") >>= xStyle;
            aDefaultText->SetText(xStyle->getName());
        }
    }

    OUString aCurrentChartStyleName = css::uno::Reference<css::style::XStyle>(getCurrentStyle(m_xModel),
                                                                css::uno::UNO_QUERY_THROW)->getName();
    aStyleList->SelectEntry(aCurrentChartStyleName);
}

void ChartStylesPanel::Initialize()
{
    UpdateList();
}

IMPL_LINK_NOARG(ChartStylesPanel, SelHdl, ListBox&, void) {}

IMPL_LINK(ChartStylesPanel, SetSelectionClickHdl, Button*, pButton, void)
{
}

IMPL_LINK_NOARG(ChartStylesPanel, SetSelectionHdl, ListBox&, void) {}

} // namespace sidebar
} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
