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

#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
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

#include "LegendHelper.hxx"
#include "TitleHelper.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"

#include "ChartModel.hxx"

using namespace css;
using namespace css::uno;
using ::sfx2::sidebar::Theme;

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

class ChartSidebarModifyListener : public cppu::WeakImplHelper1<css::util::XModifyListener>
{
public:

    ChartSidebarModifyListener(ChartElementsPanel* pParent);
    virtual ~ChartSidebarModifyListener();

    virtual void SAL_CALL modified(const css::lang::EventObject& rEvent)
        throw (::css::uno::RuntimeException, ::std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent)
        throw (::css::uno::RuntimeException, ::std::exception) SAL_OVERRIDE;

private:
    ChartElementsPanel* mpParent;
};

ChartSidebarModifyListener::ChartSidebarModifyListener(ChartElementsPanel* pParent):
    mpParent(pParent)
{
}

ChartSidebarModifyListener::~ChartSidebarModifyListener()
{
}

void ChartSidebarModifyListener::modified(const css::lang::EventObject& /*rEvent*/)
        throw (::css::uno::RuntimeException, ::std::exception)
{
    mpParent->updateData();
}

void ChartSidebarModifyListener::disposing(const css::lang::EventObject& /*rEvent*/)
        throw (::css::uno::RuntimeException, ::std::exception)
{
    mpParent->modelInvalid();
}

ChartModel* getChartModel(css::uno::Reference<css::frame::XModel> xModel)
{
    ChartModel* pModel = dynamic_cast<ChartModel*>(xModel.get());

    return pModel;
}

bool isLegendVisible(css::uno::Reference<css::frame::XModel> xModel)
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

bool isTitleVisisble(css::uno::Reference<css::frame::XModel> xModel, TitleHelper::eTitleType eTitle)
{
    return TitleHelper::getTitle(eTitle, xModel).is();
}

bool isGridVisible(css::uno::Reference<css::frame::XModel> xModel, GridType eType)
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

bool isAxisVisible(css::uno::Reference<css::frame::XModel> xModel, AxisType eType)
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

}

ChartElementsPanel::ChartElementsPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    ChartController* pController)
  : PanelLayout(pParent, "ChartElementsPanel", "modules/schart/ui/sidebarelements.ui", rxFrame),
    mxFrame(rxFrame),
    maContext(),
    mpBindings(pBindings),
    mxModel(pController->getModel()),
    mxListener(new ChartSidebarModifyListener(this))
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
    get(mpCBGridVertical,  "checkbutton_gridline_vertical");
    get(mpCBGridHorizontal,  "checkbutton_gridline_horizontal");
    get(mpCBShowLabel,  "checkbutton_label");
    get(mpCBTrendline,  "checkbutton_trendline");
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
    mpCBGridVertical.clear();
    mpCBGridHorizontal.clear();
    mpCBShowLabel.clear();
    mpCBTrendline.clear();

    PanelLayout::dispose();
}

void ChartElementsPanel::Initialize()
{
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(mxModel, css::uno::UNO_QUERY_THROW);
    xBroadcaster->addModifyListener(mxListener);
    updateData();
}

void ChartElementsPanel::updateData()
{
    SolarMutexGuard aGuard;

    mpCBLegend->Check(isLegendVisible(mxModel));
    mpCBTitle->Check(isTitleVisisble(mxModel, TitleHelper::MAIN_TITLE));
    mpCBSubtitle->Check(isTitleVisisble(mxModel, TitleHelper::SUB_TITLE));
    mpCBXAxisTitle->Check(isTitleVisisble(mxModel, TitleHelper::X_AXIS_TITLE));
    mpCBYAxisTitle->Check(isTitleVisisble(mxModel, TitleHelper::Y_AXIS_TITLE));
    mpCBZAxisTitle->Check(isTitleVisisble(mxModel, TitleHelper::Z_AXIS_TITLE));
    mpCB2ndXAxisTitle->Check(isTitleVisisble(mxModel, TitleHelper::SECONDARY_X_AXIS_TITLE));
    mpCB2ndYAxisTitle->Check(isTitleVisisble(mxModel, TitleHelper::SECONDARY_Y_AXIS_TITLE));
    mpCBGridVertical->Check(isGridVisible(mxModel, GridType::VERT_MAJOR));
    mpCBGridHorizontal->Check(isGridVisible(mxModel, GridType::HOR_MAJOR));
    mpCBXAxis->Check(isAxisVisible(mxModel, AxisType::X_MAIN));
    mpCBYAxis->Check(isAxisVisible(mxModel, AxisType::Y_MAIN));
    mpCBZAxis->Check(isAxisVisible(mxModel, AxisType::Z_MAIN));
    mpCB2ndXAxis->Check(isAxisVisible(mxModel, AxisType::X_SECOND));
    mpCB2ndYAxis->Check(isAxisVisible(mxModel, AxisType::Y_SECOND));
}

VclPtr<vcl::Window> ChartElementsPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings, ChartController* pController)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to ChartElementsPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to ChartElementsPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException("no SfxBindings given to ChartElementsPanel::Create", NULL, 2);

    return  VclPtr<ChartElementsPanel>::Create(
                        pParent, rxFrame, pBindings, pController);
}

void ChartElementsPanel::DataChanged(
    const DataChangedEvent& )
{
    updateData();
}

void ChartElementsPanel::HandleContextChange(
    const ::sfx2::sidebar::EnumContext& rContext)
{
    if(maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
    updateData();
}

void ChartElementsPanel::NotifyItemUpdate(
    sal_uInt16 /*nSID*/,
    SfxItemState /*eState*/,
    const SfxPoolItem* /*pState*/,
    const bool )
{
}

void ChartElementsPanel::modelInvalid()
{

}

}} // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
