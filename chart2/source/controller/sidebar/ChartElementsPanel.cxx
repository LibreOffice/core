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

#include <sfx2/sidebar/ControlFactory.hxx>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>

#include "ChartElementsPanel.hxx"
#include "ChartController.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/imagemgr.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/toolbox.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <comphelper/processfactory.hxx>

#include "LegendHelper.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"
#include "DiagramHelper.hxx"
#include "ChartTypeHelper.hxx"

#include "ChartModel.hxx"


using namespace css;
using namespace css::uno;

namespace chart { namespace sidebar {

namespace {

enum class GridType
{
    VERT_MAJOR,
    VERT_MINOR,
    HOR_MAJOR,
    HOR_MINOR
};

enum class AxisType
{
    X_MAIN,
    Y_MAIN,
    Z_MAIN,
    X_SECOND,
    Y_SECOND
};

ChartModel* getChartModel(const css::uno::Reference<css::frame::XModel>& xModel)
{
    ChartModel* pModel = dynamic_cast<ChartModel*>(xModel.get());

    return pModel;
}

bool isLegendVisible(const css::uno::Reference<css::frame::XModel>& xModel)
{
    ChartModel* pModel = getChartModel(xModel);
    if (!pModel)
        return false;

    Reference< beans::XPropertySet > xLegendProp( LegendHelper::getLegend(*pModel), uno::UNO_QUERY );
    if( xLegendProp.is())
    {
        try
        {
            bool bShow = false;
            if( xLegendProp->getPropertyValue( "Show") >>= bShow )
            {
                return bShow;
            }
        }
        catch(const uno::Exception &)
        {
        }
    }

    return false;
}

void setLegendVisible(const css::uno::Reference<css::frame::XModel>& xModel, bool bVisible)
{
    ChartModel* pModel = getChartModel(xModel);
    if (!pModel)
        return;

    if (bVisible)
        LegendHelper::showLegend(*pModel, comphelper::getProcessComponentContext());
    else
        LegendHelper::hideLegend(*pModel);
}

bool isTitleVisisble(const css::uno::Reference<css::frame::XModel>& xModel, TitleHelper::eTitleType eTitle)
{
    css::uno::Reference<css::uno::XInterface> xTitle = TitleHelper::getTitle(eTitle, xModel);
    if (!xTitle.is())
        return false;

    css::uno::Reference<css::beans::XPropertySet> xPropSet(xTitle, css::uno::UNO_QUERY_THROW);
    css::uno::Any aAny = xPropSet->getPropertyValue("Visible");
    bool bVisible = aAny.get<bool>();
    return bVisible;
}

bool isGridVisible(const css::uno::Reference<css::frame::XModel>& xModel, GridType eType)
{
    Reference< chart2::XDiagram > xDiagram(ChartModelHelper::findDiagram(xModel));
    if(xDiagram.is())
    {
        sal_Int32 nDimensionIndex = 0;
        if (eType == GridType::HOR_MAJOR || eType == GridType::HOR_MINOR)
            nDimensionIndex = 1;
        sal_Int32 nCooSysIndex = 0;

        bool bMajor = (eType == GridType::HOR_MAJOR || eType == GridType::VERT_MAJOR);

        bool bHasGrid = AxisHelper::isGridShown(nDimensionIndex, nCooSysIndex, bMajor, xDiagram);
        return bHasGrid;
    }
    return false;
}

void setGridVisible(const css::uno::Reference<css::frame::XModel>& xModel, GridType eType, bool bVisible)
{
    Reference< chart2::XDiagram > xDiagram(ChartModelHelper::findDiagram(xModel));
    if(xDiagram.is())
    {
        sal_Int32 nDimensionIndex = 0;
        if (eType == GridType::HOR_MAJOR || eType == GridType::HOR_MINOR)
            nDimensionIndex = 1;
        sal_Int32 nCooSysIndex = 0;

        bool bMajor = (eType == GridType::HOR_MAJOR || eType == GridType::VERT_MAJOR);

        if (bVisible)
            AxisHelper::showGrid(nDimensionIndex, nCooSysIndex, bMajor,
                    xDiagram, comphelper::getProcessComponentContext());
        else
            AxisHelper::hideGrid(nDimensionIndex, nCooSysIndex, bMajor, xDiagram);
    }
}

bool isAxisVisible(const css::uno::Reference<css::frame::XModel>& xModel, AxisType eType)
{
    Reference< chart2::XDiagram > xDiagram(ChartModelHelper::findDiagram(xModel));
    if(xDiagram.is())
    {
        sal_Int32 nDimensionIndex = 0;
        if (eType == AxisType::Y_MAIN || eType == AxisType::Y_SECOND)
            nDimensionIndex = 1;
        else if (eType == AxisType::Z_MAIN)
            nDimensionIndex = 2;

        bool bMajor = !(eType == AxisType::X_SECOND || eType == AxisType::Y_SECOND);

        bool bHasAxis = AxisHelper::isAxisShown(nDimensionIndex, bMajor, xDiagram);
        return bHasAxis;
    }
    return false;
}

void setAxisVisible(const css::uno::Reference<css::frame::XModel>& xModel, AxisType eType, bool bVisible)
{
    Reference< chart2::XDiagram > xDiagram(ChartModelHelper::findDiagram(xModel));
    if(xDiagram.is())
    {
        sal_Int32 nDimensionIndex = 0;
        if (eType == AxisType::Y_MAIN || eType == AxisType::Y_SECOND)
            nDimensionIndex = 1;
        else if (eType == AxisType::Z_MAIN)
            nDimensionIndex = 2;

        bool bMajor = !(eType == AxisType::X_SECOND || eType == AxisType::Y_SECOND);

        if (bVisible)
            AxisHelper::showAxis(nDimensionIndex, bMajor, xDiagram, comphelper::getProcessComponentContext());
        else
            AxisHelper::hideAxis(nDimensionIndex, bMajor, xDiagram);
    }
}

sal_Int32 getLegendPos(const css::uno::Reference<css::frame::XModel>& xModel)
{
    ChartModel* pModel = getChartModel(xModel);
    if (!pModel)
        return 4;

    Reference< beans::XPropertySet > xLegendProp( LegendHelper::getLegend(*pModel), uno::UNO_QUERY );
    if (!xLegendProp.is())
        return 4;

    chart2::LegendPosition eLegendPos = chart2::LegendPosition_CUSTOM;
    xLegendProp->getPropertyValue("AnchorPosition") >>= eLegendPos;
    switch(eLegendPos)
    {
        case chart2::LegendPosition_LINE_START:
            return 3;
        case chart2::LegendPosition_LINE_END:
            return 0;
        case chart2::LegendPosition_PAGE_START:
            return 1;
        case chart2::LegendPosition_PAGE_END:
            return 2;
        default:
            return 4;
    }
}

void setLegendPos(const css::uno::Reference<css::frame::XModel>& xModel, sal_Int32 nPos)
{
    ChartModel* pModel = getChartModel(xModel);
    if (!pModel)
        return;

    Reference< beans::XPropertySet > xLegendProp( LegendHelper::getLegend(*pModel), uno::UNO_QUERY );
    if (!xLegendProp.is())
        return;

    chart2::LegendPosition eLegendPos = chart2::LegendPosition_CUSTOM;
    css::chart::ChartLegendExpansion eExpansion = css::chart::ChartLegendExpansion_HIGH;
    switch(nPos)
    {
        case 1:
            eLegendPos = chart2::LegendPosition_PAGE_START;
            eExpansion = css::chart::ChartLegendExpansion_WIDE;
            break;
        case 3:
            eLegendPos = chart2::LegendPosition_LINE_START;
            break;
        case 0:
            eLegendPos = chart2::LegendPosition_LINE_END;
            break;
        case 2:
            eLegendPos = chart2::LegendPosition_PAGE_END;
            eExpansion = css::chart::ChartLegendExpansion_WIDE;
            break;
        case 4:
            eLegendPos = chart2::LegendPosition_CUSTOM;
            break;
        default:
            assert(false);
    }

    xLegendProp->setPropertyValue("AnchorPosition", css::uno::Any(eLegendPos));
    xLegendProp->setPropertyValue("Expansion", css::uno::Any(eExpansion));

    if (eLegendPos != chart2::LegendPosition_CUSTOM)
    {
        xLegendProp->setPropertyValue("RelativePosition", uno::Any());
    }
}

}

ChartElementsPanel::ChartElementsPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
  : PanelLayout(pParent, "ChartElementsPanel", "modules/schart/ui/sidebarelements.ui", rxFrame),
    maContext(),
    mxModel(pController->getModel()),
    mxListener(new ChartSidebarModifyListener(this)),
    mbModelValid(true)
{
    get(mpCBTitle,  "checkbutton_title");
    get(mpCBSubtitle,  "checkbutton_subtitle");
    get(mpCBXAxis,  "checkbutton_x_axis");
    get(mpCBXAxisTitle,  "checkbutton_x_axis_title");
    get(mpCBYAxis,  "checkbutton_y_axis");
    get(mpCBYAxisTitle,  "checkbutton_y_axis_title");
    get(mpCBZAxis,  "checkbutton_z_axis");
    get(mpCBZAxisTitle,  "checkbutton_z_axis_title");
    get(mpCB2ndXAxis,  "checkbutton_2nd_x_axis");
    get(mpCB2ndXAxisTitle,  "checkbutton_2nd_x_axis_title");
    get(mpCB2ndYAxis,  "checkbutton_2nd_y_axis");
    get(mpCB2ndYAxisTitle,  "checkbutton_2nd_y_axis_title");
    get(mpCBLegend,  "checkbutton_legend");
    get(mpCBGridVerticalMajor,  "checkbutton_gridline_vertical_major");
    get(mpCBGridHorizontalMajor,  "checkbutton_gridline_horizontal_major");
    get(mpCBGridVerticalMinor,  "checkbutton_gridline_vertical_minor");
    get(mpCBGridHorizontalMinor,  "checkbutton_gridline_horizontal_minor");

    get(mpLBAxis, "label_axes");
    get(mpLBGrid, "label_gri");

    get(mpLBLegendPosition, "comboboxtext_legend");
    get(mpBoxLegend, "box_legend");

    get(mpTextTitle, "text_title");
    get(mpTextSubTitle, "text_subtitle");

    maTextTitle = mpTextTitle->GetText();
    maTextSubTitle = mpTextSubTitle->GetText();

    Initialize();
}

ChartElementsPanel::~ChartElementsPanel()
{
    disposeOnce();
}

void ChartElementsPanel::dispose()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->removeModifyListener(mxListener);
    mpCBTitle.clear();
    mpCBSubtitle.clear();
    mpCBXAxis.clear();
    mpCBXAxisTitle.clear();
    mpCBYAxis.clear();
    mpCBYAxisTitle.clear();
    mpCBZAxis.clear();
    mpCBZAxisTitle.clear();
    mpCB2ndXAxis.clear();
    mpCB2ndXAxisTitle.clear();
    mpCB2ndYAxis.clear();
    mpCB2ndYAxisTitle.clear();
    mpCBLegend.clear();
    mpCBGridVerticalMajor.clear();
    mpCBGridHorizontalMajor.clear();
    mpCBGridVerticalMinor.clear();
    mpCBGridHorizontalMinor.clear();

    mpLBLegendPosition.clear();
    mpBoxLegend.clear();

    mpLBAxis.clear();
    mpLBGrid.clear();

    mpTextTitle.clear();
    mpTextSubTitle.clear();

    PanelLayout::dispose();
}

void ChartElementsPanel::Initialize()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->addModifyListener(mxListener);
    updateData();

    Link<Button*,void> aLink = LINK(this, ChartElementsPanel, CheckBoxHdl);
    mpCBTitle->SetClickHdl(aLink);
    mpCBSubtitle->SetClickHdl(aLink);
    mpCBXAxis->SetClickHdl(aLink);
    mpCBXAxisTitle->SetClickHdl(aLink);
    mpCBYAxis->SetClickHdl(aLink);
    mpCBYAxisTitle->SetClickHdl(aLink);
    mpCBZAxis->SetClickHdl(aLink);
    mpCBZAxisTitle->SetClickHdl(aLink);
    mpCB2ndXAxis->SetClickHdl(aLink);
    mpCB2ndXAxisTitle->SetClickHdl(aLink);
    mpCB2ndYAxis->SetClickHdl(aLink);
    mpCB2ndYAxisTitle->SetClickHdl(aLink);
    mpCBLegend->SetClickHdl(aLink);
    mpCBGridVerticalMajor->SetClickHdl(aLink);
    mpCBGridHorizontalMajor->SetClickHdl(aLink);
    mpCBGridVerticalMinor->SetClickHdl(aLink);
    mpCBGridHorizontalMinor->SetClickHdl(aLink);

    mpLBLegendPosition->SetSelectHdl(LINK(this, ChartElementsPanel, LegendPosHdl));
}

namespace {

css::uno::Reference<css::chart2::XChartType> getChartType(const css::uno::Reference<css::frame::XModel>& xModel)
{
    css::uno::Reference<css::chart2::XChartDocument> xChartDoc(xModel, css::uno::UNO_QUERY_THROW);
    css::uno::Reference<chart2::XDiagram > xDiagram = xChartDoc->getFirstDiagram();
    if (!xDiagram.is()) {
        return css::uno::Reference<css::chart2::XChartType>();
    }

    css::uno::Reference<css::chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, css::uno::UNO_QUERY_THROW );

    css::uno::Sequence<css::uno::Reference<css::chart2::XCoordinateSystem>> xCooSysSequence(xCooSysContainer->getCoordinateSystems());

    css::uno::Reference<css::chart2::XChartTypeContainer> xChartTypeContainer(xCooSysSequence[0], css::uno::UNO_QUERY_THROW);

    css::uno::Sequence<css::uno::Reference<css::chart2::XChartType>> xChartTypeSequence(xChartTypeContainer->getChartTypes());

    return xChartTypeSequence[0];
}

}

void ChartElementsPanel::updateData()
{
    if (!mbModelValid)
        return;

    Reference< chart2::XDiagram > xDiagram(ChartModelHelper::findDiagram(mxModel));
    sal_Int32 nDimension = DiagramHelper::getDimension(xDiagram);
    SolarMutexGuard aGuard;

    mpCBLegend->Check(isLegendVisible(mxModel));
    mpCBTitle->Check(isTitleVisisble(mxModel, TitleHelper::MAIN_TITLE));
    mpCBSubtitle->Check(isTitleVisisble(mxModel, TitleHelper::SUB_TITLE));
    mpCBXAxisTitle->Check(isTitleVisisble(mxModel, TitleHelper::X_AXIS_TITLE));
    mpCBYAxisTitle->Check(isTitleVisisble(mxModel, TitleHelper::Y_AXIS_TITLE));
    mpCBZAxisTitle->Check(isTitleVisisble(mxModel, TitleHelper::Z_AXIS_TITLE));
    mpCB2ndXAxisTitle->Check(isTitleVisisble(mxModel, TitleHelper::SECONDARY_X_AXIS_TITLE));
    mpCB2ndYAxisTitle->Check(isTitleVisisble(mxModel, TitleHelper::SECONDARY_Y_AXIS_TITLE));
    mpCBGridVerticalMajor->Check(isGridVisible(mxModel, GridType::VERT_MAJOR));
    mpCBGridHorizontalMajor->Check(isGridVisible(mxModel, GridType::HOR_MAJOR));
    mpCBGridVerticalMinor->Check(isGridVisible(mxModel, GridType::VERT_MINOR));
    mpCBGridHorizontalMinor->Check(isGridVisible(mxModel, GridType::HOR_MINOR));
    mpCBXAxis->Check(isAxisVisible(mxModel, AxisType::X_MAIN));
    mpCBYAxis->Check(isAxisVisible(mxModel, AxisType::Y_MAIN));
    mpCBZAxis->Check(isAxisVisible(mxModel, AxisType::Z_MAIN));
    mpCB2ndXAxis->Check(isAxisVisible(mxModel, AxisType::X_SECOND));
    mpCB2ndYAxis->Check(isAxisVisible(mxModel, AxisType::Y_SECOND));


    bool bSupportsMainAxis = ChartTypeHelper::isSupportingMainAxis(
            getChartType(mxModel), 0, 0);
    if (bSupportsMainAxis)
    {
        mpCBXAxis->Show();
        mpCBYAxis->Show();
        mpCBZAxis->Show();
        mpCBXAxisTitle->Show();
        mpCBYAxisTitle->Show();
        mpCBZAxisTitle->Show();
        mpCBGridVerticalMajor->Show();
        mpCBGridVerticalMinor->Show();
        mpCBGridHorizontalMajor->Show();
        mpCBGridHorizontalMinor->Show();
        mpLBAxis->Show();
        mpLBGrid->Show();
    }
    else
    {
        mpCBXAxis->Hide();
        mpCBYAxis->Hide();
        mpCBZAxis->Hide();
        mpCBXAxisTitle->Hide();
        mpCBYAxisTitle->Hide();
        mpCBZAxisTitle->Hide();
        mpCBGridVerticalMajor->Hide();
        mpCBGridVerticalMinor->Hide();
        mpCBGridHorizontalMajor->Hide();
        mpCBGridHorizontalMinor->Hide();
        mpLBAxis->Hide();
        mpLBGrid->Hide();
    }

    if (nDimension == 3)
    {
        mpCBZAxis->Enable();
        mpCBZAxisTitle->Enable();
    }
    else
    {
        mpCBZAxis->Disable();
        mpCBZAxisTitle->Disable();
    }

    mpLBLegendPosition->SelectEntryPos(getLegendPos(mxModel));
}

VclPtr<vcl::Window> ChartElementsPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ChartElementsPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartElementsPanel::Create", nullptr, 1);
    return  VclPtr<ChartElementsPanel>::Create(
                        pParent, rxFrame, pController);
}

void ChartElementsPanel::DataChanged(
    const DataChangedEvent& )
{
    updateData();
}

void ChartElementsPanel::HandleContextChange(
    const vcl::EnumContext& rContext)
{
    if(maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
    updateData();
}

void ChartElementsPanel::modelInvalid()
{
    mbModelValid = false;
}

void ChartElementsPanel::updateModel(
        css::uno::Reference<css::frame::XModel> xModel)
{
    if (mbModelValid)
    {
        css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
        xBroadcaster->removeModifyListener(mxListener);
    }

    mxModel = xModel;
    mbModelValid = true;

    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcasterNew(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcasterNew->addModifyListener(mxListener);
}

IMPL_LINK(ChartElementsPanel, CheckBoxHdl, Button*, pButton, void)
{
    CheckBox* pCheckBox = static_cast<CheckBox*>(pButton);
    bool bChecked = pCheckBox->IsChecked();
    if (pCheckBox == mpCBTitle.get())
        setTitleVisible(TitleHelper::MAIN_TITLE, bChecked);
    else if (pCheckBox == mpCBSubtitle.get())
        setTitleVisible(TitleHelper::SUB_TITLE, bChecked);
    else if (pCheckBox == mpCBXAxis.get())
        setAxisVisible(mxModel, AxisType::X_MAIN, bChecked);
    else if (pCheckBox == mpCBXAxisTitle.get())
        setTitleVisible(TitleHelper::X_AXIS_TITLE, bChecked);
    else if (pCheckBox == mpCBYAxis.get())
        setAxisVisible(mxModel, AxisType::Y_MAIN, bChecked);
    else if (pCheckBox == mpCBYAxisTitle.get())
        setTitleVisible(TitleHelper::Y_AXIS_TITLE, bChecked);
    else if (pCheckBox == mpCBZAxis.get())
        setAxisVisible(mxModel, AxisType::Z_MAIN, bChecked);
    else if (pCheckBox == mpCBZAxisTitle.get())
        setTitleVisible(TitleHelper::Z_AXIS_TITLE, bChecked);
    else if (pCheckBox == mpCB2ndXAxis.get())
        setAxisVisible(mxModel, AxisType::X_SECOND, bChecked);
    else if (pCheckBox == mpCB2ndXAxisTitle.get())
        setTitleVisible(TitleHelper::SECONDARY_X_AXIS_TITLE, bChecked);
    else if (pCheckBox == mpCB2ndYAxis.get())
        setAxisVisible(mxModel, AxisType::Y_SECOND, bChecked);
    else if (pCheckBox == mpCB2ndYAxisTitle.get())
        setTitleVisible(TitleHelper::SECONDARY_Y_AXIS_TITLE, bChecked);
    else if (pCheckBox == mpCBLegend.get())
    {
        mpBoxLegend->Enable( bChecked );
        setLegendVisible(mxModel, bChecked);
    }
    else if (pCheckBox == mpCBGridVerticalMajor.get())
        setGridVisible(mxModel, GridType::VERT_MAJOR, bChecked);
    else if (pCheckBox == mpCBGridHorizontalMajor.get())
        setGridVisible(mxModel, GridType::HOR_MAJOR, bChecked);
    else if (pCheckBox == mpCBGridVerticalMinor.get())
        setGridVisible(mxModel, GridType::VERT_MINOR, bChecked);
    else if (pCheckBox == mpCBGridHorizontalMinor.get())
        setGridVisible(mxModel, GridType::HOR_MINOR, bChecked);
}

IMPL_LINK_NOARG(ChartElementsPanel, LegendPosHdl, ListBox&, void)
{
    sal_Int32 nPos = mpLBLegendPosition->GetSelectEntryPos();
    setLegendPos(mxModel, nPos);
}

void ChartElementsPanel::setTitleVisible(TitleHelper::eTitleType eTitle, bool bVisible)
{
    if (bVisible)
    {
        OUString aText = eTitle == TitleHelper::SUB_TITLE ? maTextSubTitle : maTextTitle;
        TitleHelper::createOrShowTitle(eTitle, aText, mxModel, comphelper::getProcessComponentContext(), nullptr);
    }
    else
    {
        TitleHelper::hideTitle(eTitle, mxModel);
    }
}

}} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
