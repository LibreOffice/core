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

#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>

#include <vcl/svapp.hxx>

#include "ChartElementsPanel.hxx"
#include <ChartController.hxx>
#include <comphelper/processfactory.hxx>

#include <Legend.hxx>
#include <LegendHelper.hxx>
#include <AxisHelper.hxx>
#include <Diagram.hxx>
#include <ChartType.hxx>
#include <ChartTypeHelper.hxx>
#include <ChartModel.hxx>
#include <BaseCoordinateSystem.hxx>


using namespace css;
using namespace css::uno;

namespace chart::sidebar {

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

    rtl::Reference< Legend > xLegendProp = LegendHelper::getLegend(*pModel);
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

bool isLegendOverlay(const css::uno::Reference<css::frame::XModel>& xModel)
{
    ChartModel* pModel = getChartModel(xModel);
    if (!pModel)
        return false;

    rtl::Reference< Legend > xLegendProp = LegendHelper::getLegend(*pModel);
    if( xLegendProp.is())
    {
        try
        {
            bool bOverlay = false;
            if(xLegendProp->getPropertyValue("Overlay") >>= bOverlay)
            {
                return bOverlay;
            }
        }
        catch(const uno::Exception &)
        {
        }
    }

    return false;
}

void setLegendOverlay(const css::uno::Reference<css::frame::XModel>& xModel, bool bOverlay)
{
    ChartModel* pModel = getChartModel(xModel);
    if (!pModel)
        return;

    rtl::Reference<Legend> xLegendProp = LegendHelper::getLegend(*pModel);
    if (!xLegendProp.is())
        return;

    xLegendProp->setPropertyValue("Overlay", css::uno::Any(bOverlay));
}

bool isTitleVisible(const rtl::Reference<::chart::ChartModel>& xModel, TitleHelper::eTitleType eTitle)
{
    rtl::Reference<Title> xTitle = TitleHelper::getTitle(eTitle, xModel);
    if (!xTitle.is())
        return false;

    css::uno::Any aAny = xTitle->getPropertyValue("Visible");
    bool bVisible = aAny.get<bool>();
    return bVisible;
}

bool isGridVisible(const rtl::Reference<::chart::ChartModel>& xModel, GridType eType)
{
    rtl::Reference< Diagram > xDiagram(xModel->getFirstChartDiagram());
    if(xDiagram.is())
    {
        sal_Int32 nDimensionIndex = 0;
        if (eType == GridType::HOR_MAJOR || eType == GridType::HOR_MINOR)
            nDimensionIndex = 1;

        bool bMajor = (eType == GridType::HOR_MAJOR || eType == GridType::VERT_MAJOR);

        bool bHasGrid = AxisHelper::isGridShown(nDimensionIndex, 0, bMajor, xDiagram);
        return bHasGrid;
    }
    return false;
}

void setGridVisible(const rtl::Reference<::chart::ChartModel>& xModel, GridType eType, bool bVisible)
{
    rtl::Reference< Diagram > xDiagram(xModel->getFirstChartDiagram());
    if(!xDiagram.is())
        return;

    sal_Int32 nDimensionIndex = 0;
    if (eType == GridType::HOR_MAJOR || eType == GridType::HOR_MINOR)
        nDimensionIndex = 1;
    sal_Int32 nCooSysIndex = 0;

    bool bMajor = (eType == GridType::HOR_MAJOR || eType == GridType::VERT_MAJOR);

    if (bVisible)
        AxisHelper::showGrid(nDimensionIndex, nCooSysIndex, bMajor,
                xDiagram);
    else
        AxisHelper::hideGrid(nDimensionIndex, nCooSysIndex, bMajor, xDiagram);
}

bool isAxisVisible(const rtl::Reference<::chart::ChartModel>& xModel, AxisType eType)
{
    rtl::Reference< Diagram > xDiagram(xModel->getFirstChartDiagram());
    if(xDiagram.is())
    {
        sal_Int32 nDimensionIndex = 0;
        if (eType == AxisType::Y_MAIN || eType == AxisType::Y_SECOND)
            nDimensionIndex = 1;
        else if (eType == AxisType::Z_MAIN)
            nDimensionIndex = 2;

        bool bMajor = (eType != AxisType::X_SECOND && eType != AxisType::Y_SECOND);

        bool bHasAxis = AxisHelper::isAxisShown(nDimensionIndex, bMajor, xDiagram);
        return bHasAxis;
    }
    return false;
}

void setAxisVisible(const rtl::Reference<::chart::ChartModel>& xModel, AxisType eType, bool bVisible)
{
    rtl::Reference< Diagram > xDiagram(xModel->getFirstChartDiagram());
    if(!xDiagram.is())
        return;

    sal_Int32 nDimensionIndex = 0;
    if (eType == AxisType::Y_MAIN || eType == AxisType::Y_SECOND)
        nDimensionIndex = 1;
    else if (eType == AxisType::Z_MAIN)
        nDimensionIndex = 2;

    bool bMajor = (eType != AxisType::X_SECOND && eType != AxisType::Y_SECOND);

    if (bVisible)
        AxisHelper::showAxis(nDimensionIndex, bMajor, xDiagram, comphelper::getProcessComponentContext());
    else
        AxisHelper::hideAxis(nDimensionIndex, bMajor, xDiagram);
}

sal_Int32 getLegendPos(const css::uno::Reference<css::frame::XModel>& xModel)
{
    ChartModel* pModel = getChartModel(xModel);
    if (!pModel)
        return -1;

    rtl::Reference< Legend > xLegendProp = LegendHelper::getLegend(*pModel);
    if (!xLegendProp.is())
        return -1;

    chart2::LegendPosition eLegendPos = chart2::LegendPosition_LINE_END;
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
            return -1;
    }
}

void setLegendPos(const css::uno::Reference<css::frame::XModel>& xModel, sal_Int32 nPos)
{
    ChartModel* pModel = getChartModel(xModel);
    if (!pModel)
        return;

    rtl::Reference< Legend > xLegendProp = LegendHelper::getLegend(*pModel);
    if (!xLegendProp.is())
        return;

    chart2::LegendPosition eLegendPos = chart2::LegendPosition_LINE_END;
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
        default:
            assert(false);
    }

    xLegendProp->setPropertyValue("AnchorPosition", css::uno::Any(eLegendPos));
    xLegendProp->setPropertyValue("Expansion", css::uno::Any(eExpansion));
    xLegendProp->setPropertyValue("RelativePosition", uno::Any());
}

}

ChartElementsPanel::ChartElementsPanel(
    weld::Widget* pParent, ChartController* pController)
    : PanelLayout(pParent, "ChartElementsPanel", "modules/schart/ui/sidebarelements.ui")
    , mxCBTitle(m_xBuilder->weld_check_button("checkbutton_title"))
    , mxEditTitle(m_xBuilder->weld_entry("edit_title"))
    , mxCBSubtitle(m_xBuilder->weld_check_button("checkbutton_subtitle"))
    , mxEditSubtitle(m_xBuilder->weld_entry("edit_subtitle"))
    , mxCBXAxis(m_xBuilder->weld_check_button("checkbutton_x_axis"))
    , mxCBXAxisTitle(m_xBuilder->weld_check_button("checkbutton_x_axis_title"))
    , mxCBYAxis(m_xBuilder->weld_check_button("checkbutton_y_axis"))
    , mxCBYAxisTitle(m_xBuilder->weld_check_button("checkbutton_y_axis_title"))
    , mxCBZAxis(m_xBuilder->weld_check_button("checkbutton_z_axis"))
    , mxCBZAxisTitle(m_xBuilder->weld_check_button("checkbutton_z_axis_title"))
    , mxCB2ndXAxis(m_xBuilder->weld_check_button("checkbutton_2nd_x_axis"))
    , mxCB2ndXAxisTitle(m_xBuilder->weld_check_button("checkbutton_2nd_x_axis_title"))
    , mxCB2ndYAxis(m_xBuilder->weld_check_button("checkbutton_2nd_y_axis"))
    , mxCB2ndYAxisTitle(m_xBuilder->weld_check_button("checkbutton_2nd_y_axis_title"))
    , mxCBLegend(m_xBuilder->weld_check_button("checkbutton_legend"))
    , mxCBLegendNoOverlay(m_xBuilder->weld_check_button("checkbutton_no_overlay"))
    , mxCBGridVerticalMajor(m_xBuilder->weld_check_button("checkbutton_gridline_vertical_major"))
    , mxCBGridHorizontalMajor(m_xBuilder->weld_check_button("checkbutton_gridline_horizontal_major"))
    , mxCBGridVerticalMinor(m_xBuilder->weld_check_button("checkbutton_gridline_vertical_minor"))
    , mxCBGridHorizontalMinor(m_xBuilder->weld_check_button("checkbutton_gridline_horizontal_minor"))
    , mxTextTitle(m_xBuilder->weld_label("text_title"))
    , mxTextSubTitle(m_xBuilder->weld_label("text_subtitle"))
    , mxLBAxis(m_xBuilder->weld_label("label_axes"))
    , mxLBGrid(m_xBuilder->weld_label("label_gri"))
    , mxLBLegendPosition(m_xBuilder->weld_combo_box("comboboxtext_legend"))
    , mxBoxLegend(m_xBuilder->weld_widget("box_legend"))
    , mxModel(pController->getChartModel())
    , mxListener(new ChartSidebarModifyListener(this))
    , mbModelValid(true)
{
    maTextTitle = mxTextTitle->get_label();
    maTextSubTitle = mxTextSubTitle->get_label();

    Initialize();
}

ChartElementsPanel::~ChartElementsPanel()
{
    doUpdateModel(nullptr);

    mxCBTitle.reset();
    mxEditTitle.reset();
    mxCBSubtitle.reset();
    mxEditSubtitle.reset();
    mxCBXAxis.reset();
    mxCBXAxisTitle.reset();
    mxCBYAxis.reset();
    mxCBYAxisTitle.reset();
    mxCBZAxis.reset();
    mxCBZAxisTitle.reset();
    mxCB2ndXAxis.reset();
    mxCB2ndXAxisTitle.reset();
    mxCB2ndYAxis.reset();
    mxCB2ndYAxisTitle.reset();
    mxCBLegend.reset();
    mxCBLegendNoOverlay.reset();
    mxCBGridVerticalMajor.reset();
    mxCBGridHorizontalMajor.reset();
    mxCBGridVerticalMinor.reset();
    mxCBGridHorizontalMinor.reset();

    mxLBLegendPosition.reset();
    mxBoxLegend.reset();

    mxLBAxis.reset();
    mxLBGrid.reset();

    mxTextTitle.reset();
    mxTextSubTitle.reset();
}

void ChartElementsPanel::Initialize()
{
    mxModel->addModifyListener(mxListener);
    updateData();

    Link<weld::Toggleable&,void> aLink = LINK(this, ChartElementsPanel, CheckBoxHdl);
    mxCBTitle->connect_toggled(aLink);
    mxCBSubtitle->connect_toggled(aLink);
    mxCBXAxis->connect_toggled(aLink);
    mxCBXAxisTitle->connect_toggled(aLink);
    mxCBYAxis->connect_toggled(aLink);
    mxCBYAxisTitle->connect_toggled(aLink);
    mxCBZAxis->connect_toggled(aLink);
    mxCBZAxisTitle->connect_toggled(aLink);
    mxCB2ndXAxis->connect_toggled(aLink);
    mxCB2ndXAxisTitle->connect_toggled(aLink);
    mxCB2ndYAxis->connect_toggled(aLink);
    mxCB2ndYAxisTitle->connect_toggled(aLink);
    mxCBLegend->connect_toggled(aLink);
    mxCBLegendNoOverlay->connect_toggled(aLink);
    mxCBGridVerticalMajor->connect_toggled(aLink);
    mxCBGridHorizontalMajor->connect_toggled(aLink);
    mxCBGridVerticalMinor->connect_toggled(aLink);
    mxCBGridHorizontalMinor->connect_toggled(aLink);

    mxLBLegendPosition->connect_changed(LINK(this, ChartElementsPanel, LegendPosHdl));

    Link<weld::Entry&, void> aEditLink = LINK(this, ChartElementsPanel, EditHdl);
    mxEditTitle->connect_changed(aEditLink);
    mxEditSubtitle->connect_changed(aEditLink);
}

namespace {

rtl::Reference<ChartType> getChartType(const rtl::Reference<ChartModel>& xModel)
{
    rtl::Reference<Diagram > xDiagram = xModel->getFirstChartDiagram();
    if (!xDiagram.is())
        return nullptr;

    const std::vector<rtl::Reference<BaseCoordinateSystem>> & xCooSysSequence(xDiagram->getBaseCoordinateSystems());

    if (xCooSysSequence.empty())
        return nullptr;

    const std::vector<rtl::Reference<ChartType>> & xChartTypeSequence(xCooSysSequence[0]->getChartTypes2());

    if (xChartTypeSequence.empty())
        return nullptr;

    return xChartTypeSequence[0];
}

}

void ChartElementsPanel::updateData()
{
    if (!mbModelValid)
        return;

    rtl::Reference< Diagram > xDiagram(mxModel->getFirstChartDiagram());
    sal_Int32 nDimension = 0;
    if (xDiagram)
        nDimension = xDiagram->getDimension();
    SolarMutexGuard aGuard;

    mxCBLegend->set_active(isLegendVisible(mxModel));
    mxCBLegendNoOverlay->set_sensitive(isLegendVisible(mxModel));
    mxCBLegendNoOverlay->set_active(!isLegendOverlay(mxModel));
    mxBoxLegend->set_sensitive(isLegendVisible(mxModel));

    bool hasTitle = isTitleVisible(mxModel, TitleHelper::MAIN_TITLE);
    mxCBTitle->set_active(hasTitle);

    OUString title = mxEditTitle->get_text();
    OUString newTitle = TitleHelper::getCompleteString(TitleHelper::getTitle(TitleHelper::MAIN_TITLE, mxModel));
    if (title != newTitle)
        mxEditTitle->set_text(newTitle);
    if (mxEditTitle->get_sensitive() != hasTitle)
         mxEditTitle->set_sensitive(hasTitle);

    bool hasSubtitle = isTitleVisible(mxModel, TitleHelper::SUB_TITLE);
    mxCBSubtitle->set_active(hasSubtitle);

    OUString subtitle = mxEditSubtitle->get_text();
    OUString newSubtitle = TitleHelper::getCompleteString(TitleHelper::getTitle(TitleHelper::SUB_TITLE, mxModel));
    if (subtitle != newSubtitle)
        mxEditSubtitle->set_text(newSubtitle);
    if (mxEditSubtitle->get_sensitive() != hasSubtitle)
         mxEditSubtitle->set_sensitive(hasSubtitle);

    mxCBXAxisTitle->set_active(isTitleVisible(mxModel, TitleHelper::X_AXIS_TITLE));
    mxCBYAxisTitle->set_active(isTitleVisible(mxModel, TitleHelper::Y_AXIS_TITLE));
    mxCBZAxisTitle->set_active(isTitleVisible(mxModel, TitleHelper::Z_AXIS_TITLE));
    mxCB2ndXAxisTitle->set_active(isTitleVisible(mxModel, TitleHelper::SECONDARY_X_AXIS_TITLE));
    mxCB2ndYAxisTitle->set_active(isTitleVisible(mxModel, TitleHelper::SECONDARY_Y_AXIS_TITLE));
    mxCBGridVerticalMajor->set_active(isGridVisible(mxModel, GridType::VERT_MAJOR));
    mxCBGridHorizontalMajor->set_active(isGridVisible(mxModel, GridType::HOR_MAJOR));
    mxCBGridVerticalMinor->set_active(isGridVisible(mxModel, GridType::VERT_MINOR));
    mxCBGridHorizontalMinor->set_active(isGridVisible(mxModel, GridType::HOR_MINOR));
    mxCBXAxis->set_active(isAxisVisible(mxModel, AxisType::X_MAIN));
    mxCBYAxis->set_active(isAxisVisible(mxModel, AxisType::Y_MAIN));
    mxCBZAxis->set_active(isAxisVisible(mxModel, AxisType::Z_MAIN));
    mxCB2ndXAxis->set_active(isAxisVisible(mxModel, AxisType::X_SECOND));
    mxCB2ndYAxis->set_active(isAxisVisible(mxModel, AxisType::Y_SECOND));

    bool bSupportsMainAxis = ChartTypeHelper::isSupportingMainAxis(
            getChartType(mxModel), 0, 0);
    if (bSupportsMainAxis)
    {
        mxCBXAxis->show();
        mxCBYAxis->show();
        mxCBZAxis->show();
        mxCBXAxisTitle->show();
        mxCBYAxisTitle->show();
        mxCBZAxisTitle->show();
        mxCBGridVerticalMajor->show();
        mxCBGridVerticalMinor->show();
        mxCBGridHorizontalMajor->show();
        mxCBGridHorizontalMinor->show();
        mxLBAxis->show();
        mxLBGrid->show();
    }
    else
    {
        mxCBXAxis->hide();
        mxCBYAxis->hide();
        mxCBZAxis->hide();
        mxCBXAxisTitle->hide();
        mxCBYAxisTitle->hide();
        mxCBZAxisTitle->hide();
        mxCBGridVerticalMajor->hide();
        mxCBGridVerticalMinor->hide();
        mxCBGridHorizontalMajor->hide();
        mxCBGridHorizontalMinor->hide();
        mxLBAxis->hide();
        mxLBGrid->hide();
    }

    if (nDimension == 3)
    {
        mxCBZAxis->set_sensitive(true);
        mxCBZAxisTitle->set_sensitive(true);
    }
    else
    {
        mxCBZAxis->set_sensitive(false);
        mxCBZAxisTitle->set_sensitive(false);
    }

    mxLBLegendPosition->set_active(getLegendPos(mxModel));
}

std::unique_ptr<PanelLayout> ChartElementsPanel::Create (
    weld::Widget* pParent,
    ChartController* pController)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to ChartElementsPanel::Create", nullptr, 0);
    return std::make_unique<ChartElementsPanel>(pParent, pController);
}

void ChartElementsPanel::DataChanged(const DataChangedEvent& rEvent)
{
    PanelLayout::DataChanged(rEvent);
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

void ChartElementsPanel::doUpdateModel(const rtl::Reference<::chart::ChartModel>& xModel)
{
    if (mbModelValid)
    {
        mxModel->removeModifyListener(mxListener);
    }

    mxModel = xModel;
    mbModelValid = mxModel.is();

    if (!mbModelValid)
        return;

    mxModel->addModifyListener(mxListener);
}

void ChartElementsPanel::updateModel(css::uno::Reference<css::frame::XModel> xModel)
{
    ::chart::ChartModel* pModel = dynamic_cast<::chart::ChartModel*>(xModel.get());
    assert(!xModel || pModel);
    doUpdateModel(pModel);
}

IMPL_LINK(ChartElementsPanel, CheckBoxHdl, weld::Toggleable&, rCheckBox, void)
{
    bool bChecked = rCheckBox.get_active();
    if (&rCheckBox == mxCBTitle.get())
        setTitleVisible(TitleHelper::MAIN_TITLE, bChecked);
    else if (&rCheckBox == mxCBSubtitle.get())
        setTitleVisible(TitleHelper::SUB_TITLE, bChecked);
    else if (&rCheckBox == mxCBXAxis.get())
        setAxisVisible(mxModel, AxisType::X_MAIN, bChecked);
    else if (&rCheckBox == mxCBXAxisTitle.get())
        setTitleVisible(TitleHelper::X_AXIS_TITLE, bChecked);
    else if (&rCheckBox == mxCBYAxis.get())
        setAxisVisible(mxModel, AxisType::Y_MAIN, bChecked);
    else if (&rCheckBox == mxCBYAxisTitle.get())
        setTitleVisible(TitleHelper::Y_AXIS_TITLE, bChecked);
    else if (&rCheckBox == mxCBZAxis.get())
        setAxisVisible(mxModel, AxisType::Z_MAIN, bChecked);
    else if (&rCheckBox == mxCBZAxisTitle.get())
        setTitleVisible(TitleHelper::Z_AXIS_TITLE, bChecked);
    else if (&rCheckBox == mxCB2ndXAxis.get())
        setAxisVisible(mxModel, AxisType::X_SECOND, bChecked);
    else if (&rCheckBox == mxCB2ndXAxisTitle.get())
        setTitleVisible(TitleHelper::SECONDARY_X_AXIS_TITLE, bChecked);
    else if (&rCheckBox == mxCB2ndYAxis.get())
        setAxisVisible(mxModel, AxisType::Y_SECOND, bChecked);
    else if (&rCheckBox == mxCB2ndYAxisTitle.get())
        setTitleVisible(TitleHelper::SECONDARY_Y_AXIS_TITLE, bChecked);
    else if (&rCheckBox == mxCBLegend.get())
    {
        mxBoxLegend->set_sensitive(bChecked);
        mxCBLegendNoOverlay->set_sensitive(bChecked);
        setLegendVisible(mxModel, bChecked);
    }
    else if (&rCheckBox == mxCBLegendNoOverlay.get())
        setLegendOverlay(mxModel, !bChecked);
    else if (&rCheckBox == mxCBGridVerticalMajor.get())
        setGridVisible(mxModel, GridType::VERT_MAJOR, bChecked);
    else if (&rCheckBox == mxCBGridHorizontalMajor.get())
        setGridVisible(mxModel, GridType::HOR_MAJOR, bChecked);
    else if (&rCheckBox == mxCBGridVerticalMinor.get())
        setGridVisible(mxModel, GridType::VERT_MINOR, bChecked);
    else if (&rCheckBox == mxCBGridHorizontalMinor.get())
        setGridVisible(mxModel, GridType::HOR_MINOR, bChecked);

    updateData();
}

IMPL_LINK(ChartElementsPanel, EditHdl, weld::Entry&, rEdit, void)
{
    // title or subtitle?
    TitleHelper::eTitleType aTitleType = TitleHelper::MAIN_TITLE;
    if (&rEdit == mxEditSubtitle.get())
        aTitleType = TitleHelper::SUB_TITLE;

    // set it
    OUString aText(rEdit.get_text());
    TitleHelper::setCompleteString(aText, TitleHelper::getTitle(aTitleType, mxModel),
        comphelper::getProcessComponentContext(), nullptr, true);
}

IMPL_LINK_NOARG(ChartElementsPanel, LegendPosHdl, weld::ComboBox&, void)
{
    sal_Int32 nPos = mxLBLegendPosition->get_active();
    setLegendPos(mxModel, nPos);
}

void ChartElementsPanel::setTitleVisible(TitleHelper::eTitleType eTitle, bool bVisible)
{
    if (bVisible)
    {
        OUString aText = eTitle == TitleHelper::SUB_TITLE ? maTextSubTitle : maTextTitle;
        TitleHelper::createOrShowTitle(eTitle, aText, mxModel, comphelper::getProcessComponentContext());
    }
    else
    {
        TitleHelper::hideTitle(eTitle, mxModel);
    }
}

} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
