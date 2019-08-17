/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
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
    PanelLayout::dispose();
}

css::uno::Reference<css::chart2::XChartStyle> ChartStylesPanel::updateStyleFromChart( css::uno::Reference<css::chart2::XChartStyle>& xChartStyle )
{
    ChartModel* pModel = getChartModel(m_xModel);
    css::uno::Reference<css::beans::XPropertyAccess> xPropAccess(pModel->getTitleObject(), uno::UNO_QUERY);
    if (xPropAccess.is())
        xChartStyle->updateStyle(css::chart2::ChartObjectType::TITLE, xPropAccess->getPropertyValues());

    xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>(pModel->getPageBackground(), uno::UNO_QUERY);
    if (xPropAccess.is())
        xChartStyle->updateStyle(css::chart2::ChartObjectType::PAGE, xPropAccess->getPropertyValues());

    xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>(pModel->getFirstDiagram(), uno::UNO_QUERY);
    if (xPropAccess.is())
        xChartStyle->updateStyle(css::chart2::ChartObjectType::DIAGRAM, xPropAccess->getPropertyValues());

    css::uno::Reference<css::chart2::XDiagram> xDiagram = pModel->getFirstDiagram();

    xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>( xDiagram->getWall(), uno::UNO_QUERY);
    if (xPropAccess.is())
        xChartStyle->updateStyle(css::chart2::ChartObjectType::WALL, xPropAccess->getPropertyValues());

    xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>( xDiagram->getLegend(), uno::UNO_QUERY);
    if (xPropAccess.is())
        xChartStyle->updateStyle(css::chart2::ChartObjectType::LEGEND, xPropAccess->getPropertyValues());

    css::uno::Reference<css::chart2::XTitled> xTitled( xDiagram, uno::UNO_QUERY);
    xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>( xTitled->getTitleObject(), uno::UNO_QUERY);
    if (xPropAccess.is())
        xChartStyle->updateStyle(css::chart2::ChartObjectType::TITLE, xPropAccess->getPropertyValues());

    css::uno::Reference<css::chart2::XCoordinateSystemContainer> xCooSysCont( xDiagram, uno::UNO_QUERY);

    css::uno::Sequence<css::uno::Reference<css::chart2::XCoordinateSystem>> aCooSysSeq(
        xCooSysCont->getCoordinateSystems());

    for (sal_Int32 nCooSysIdx = 0; nCooSysIdx < aCooSysSeq.getLength(); ++nCooSysIdx)
    {
        css::uno::Reference<css::chart2::XCoordinateSystem> xCooSys(aCooSysSeq[nCooSysIdx],
                                                                    css::uno::UNO_QUERY);
        sal_Int16 nDimCount = xCooSys->getDimension();
        for (sal_Int16 nDimIdx = 0; nDimIdx <= nDimCount; nDimIdx++)
        {
            css::uno::Reference<css::chart2::XAxis> xAxis = xCooSys->getAxisByDimension(nDimIdx, 0);
            xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>( xAxis, uno::UNO_QUERY);
            if (xPropAccess.is())
                xChartStyle->updateStyle(css::chart2::ChartObjectType::AXIS, xPropAccess->getPropertyValues());

            if (xCooSys->getMaximumAxisIndexByDimension(nDimIdx))
            {
                xAxis = xCooSys->getAxisByDimension(nDimIdx, 1);
                xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>( xAxis, uno::UNO_QUERY);
                if (xPropAccess.is())
                    xChartStyle->updateStyle(css::chart2::ChartObjectType::AXIS, xPropAccess->getPropertyValues());
            }
        }
    }

    return xChartStyle;
}

void ChartStylesPanel::createNewStyle(const OUString& sName)
{
    css::uno::Reference<css::chart2::XChartStyle> xCurrentStyle = getCurrentStyle(m_xModel);

    css::uno::Reference<css::chart2::XChartStyle> xCopyStyle = CloneHelper::CreateRefClone<css::chart2::XChartStyle>()(xCurrentStyle);
    css::uno::Reference<css::chart2::XChartStyle> xNewStyle = updateStyleFromChart( xCopyStyle );

    m_xChartStyles->insertByName(sName, css::uno::Any(xNewStyle));

    UpdateList();
}

void ChartStylesPanel::UpdateList()
{
    aStyleList->Clear();
    css::uno::Sequence<OUString> aStyleNames = m_xChartStyles->getElementNames();
    for ( auto& rStyle : aStyleNames )
    {
        aStyleList->InsertEntry( rStyle );
    }
    ChartModel* pModel = getChartModel(m_xModel);
    OUString aCurrentChartStyleName = css::uno::Reference<css::style::XStyle>(pModel->getChartStyle(),
                                                                css::uno::UNO_QUERY_THROW)->getName();
    aStyleList->SelectEntry(aCurrentChartStyleName);
}

void ChartStylesPanel::Initialize()
{
    UpdateList();
}

IMPL_LINK_NOARG(ChartStylesPanel, SelHdl, ListBox&, void) {}

IMPL_LINK_NOARG(ChartStylesPanel, SetSelectionClickHdl, Button*, void)
{
    // code for pressing button goes here

}

IMPL_LINK_NOARG(ChartStylesPanel, SetSelectionHdl, ListBox&, void) {}

} // namespace sidebar
} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
