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

#include <sal/config.h>

#include <cstddef>

#include "SeriesPlotterContainer.hxx"

#include <ChartView.hxx>
#include <Diagram.hxx>
#include <ChartType.hxx>
#include <DataSeries.hxx>
#include <ChartModel.hxx>
#include <ChartTypeHelper.hxx>
#include <ObjectIdentifier.hxx>
#include <DiagramHelper.hxx>
#include <Axis.hxx>
#include <AxisIndexDefines.hxx>
#include <DataSeriesHelper.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/PieChartSubType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <comphelper/classids.hxx>
#include <servicenames_charttypes.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace chart
{
using namespace ::css;
using namespace ::css::chart2;

using ::css::uno::Reference;
using ::css::uno::Sequence;
using ::css::uno::Any;

SeriesPlotterContainer::SeriesPlotterContainer(
    std::vector<std::unique_ptr<VCoordinateSystem>>& rVCooSysList)
    : m_rVCooSysList(rVCooSysList)
    , m_nMaxAxisIndex(0)
    , m_bChartTypeUsesShiftedCategoryPositionPerDefault(false)
    , m_nDefaultDateNumberFormat(0)
{
}

SeriesPlotterContainer::~SeriesPlotterContainer()
{
    // - remove plotter from coordinatesystems
    for (auto& nC : m_rVCooSysList)
        nC->clearMinimumAndMaximumSupplierList();
}

std::vector<LegendEntryProvider*> SeriesPlotterContainer::getLegendEntryProviderList()
{
    std::vector<LegendEntryProvider*> aRet(m_aSeriesPlotterList.size());
    sal_Int32 nN = 0;
    for (const std::unique_ptr<VSeriesPlotter>& aPlotter : m_aSeriesPlotterList)
        aRet[nN++] = aPlotter.get();
    return aRet;
}

VCoordinateSystem* SeriesPlotterContainer::findInCooSysList(
    const std::vector<std::unique_ptr<VCoordinateSystem>>& rVCooSysList,
    const rtl::Reference<BaseCoordinateSystem>& xCooSys)
{
    for (auto& pVCooSys : rVCooSysList)
    {
        if (pVCooSys->getModel() == xCooSys)
            return pVCooSys.get();
    }
    return nullptr;
}

VCoordinateSystem* SeriesPlotterContainer::getCooSysForPlotter(
    const std::vector<std::unique_ptr<VCoordinateSystem>>& rVCooSysList,
    MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier)
{
    if (!pMinimumAndMaximumSupplier)
        return nullptr;
    for (auto& pVCooSys : rVCooSysList)
    {
        if (pVCooSys->hasMinimumAndMaximumSupplier(pMinimumAndMaximumSupplier))
            return pVCooSys.get();
    }
    return nullptr;
}

VCoordinateSystem* SeriesPlotterContainer::addCooSysToList(
    std::vector<std::unique_ptr<VCoordinateSystem>>& rVCooSysList,
    const rtl::Reference<BaseCoordinateSystem>& xCooSys, ChartModel& rChartModel)
{
    VCoordinateSystem* pExistingVCooSys
        = SeriesPlotterContainer::findInCooSysList(rVCooSysList, xCooSys);
    if (pExistingVCooSys)
        return pExistingVCooSys;

    std::unique_ptr<VCoordinateSystem> pVCooSys
        = VCoordinateSystem::createCoordinateSystem(xCooSys);
    if (!pVCooSys)
        return nullptr;

    OUString aCooSysParticle(
        ObjectIdentifier::createParticleForCoordinateSystem(xCooSys, &rChartModel));
    pVCooSys->setParticle(aCooSysParticle);

    pVCooSys->setExplicitCategoriesProvider(new ExplicitCategoriesProvider(xCooSys, rChartModel));
    rVCooSysList.push_back(std::move(pVCooSys));
    return rVCooSysList.back().get();
}

void SeriesPlotterContainer::initializeCooSysAndSeriesPlotter(ChartModel& rChartModel)
{
    rtl::Reference<Diagram> xDiagram = rChartModel.getFirstChartDiagram();
    if (!xDiagram.is())
        return;

    uno::Reference<util::XNumberFormatsSupplier> xNumberFormatsSupplier(&rChartModel);
    if (rChartModel.hasInternalDataProvider() && xDiagram->isSupportingDateAxis())
        m_nDefaultDateNumberFormat = DiagramHelper::getDateNumberFormat(xNumberFormatsSupplier);

    sal_Int32 nDimensionCount = xDiagram->getDimension();
    if (!nDimensionCount)
    {
        //@todo handle mixed dimension
        nDimensionCount = 2;
    }

    bool bSortByXValues = false;
    bool bConnectBars = false;
    bool bGroupBarsPerAxis = true;
    bool bIncludeHiddenCells = true;
    bool bSecondaryYaxisVisible = true;
    sal_Int32 nStartingAngle = 90;
    sal_Int32 n3DRelativeHeight = 100;
    PieChartSubType ePieChartSubType = PieChartSubType_NONE;
    try
    {
        xDiagram->getPropertyValue(CHART_UNONAME_SORT_BY_XVALUES) >>= bSortByXValues;
        xDiagram->getPropertyValue("ConnectBars") >>= bConnectBars;
        xDiagram->getPropertyValue("GroupBarsPerAxis") >>= bGroupBarsPerAxis;
        xDiagram->getPropertyValue("IncludeHiddenCells") >>= bIncludeHiddenCells;
        xDiagram->getPropertyValue("StartingAngle") >>= nStartingAngle;

        if (nDimensionCount == 3)
        {
            xDiagram->getPropertyValue("3DRelativeHeight") >>= n3DRelativeHeight;
        }
        xDiagram->getPropertyValue("SubPieType") >>= ePieChartSubType;
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    if (xDiagram->getDataTable().is())
        m_bTableShiftPosition = true;

    //prepare for autoscaling and shape creation
    // - create plotter for charttypes (for each first scale group at each plotter, as they are independent)
    // - add series to plotter (thus each charttype can provide minimum and maximum values for autoscaling)
    // - add plotter to coordinate systems

    //iterate through all coordinate systems
    uno::Reference<XColorScheme> xColorScheme(xDiagram->getDefaultColorScheme());
    auto aCooSysList = xDiagram->getBaseCoordinateSystems();
    sal_Int32 nGlobalSeriesIndex = 0; //for automatic symbols
    for (std::size_t nCS = 0; nCS < aCooSysList.size(); ++nCS)
    {
        rtl::Reference<BaseCoordinateSystem> xCooSys(aCooSysList[nCS]);
        VCoordinateSystem* pVCooSys
            = SeriesPlotterContainer::addCooSysToList(m_rVCooSysList, xCooSys, rChartModel);
        // Let's check whether the secondary Y axis is visible
        try
        {
            if (xCooSys->getMaximumAxisIndexByDimension(1) > 0)
            {
                rtl::Reference<Axis> xAxisProp = xCooSys->getAxisByDimension2(1, 1);
                xAxisProp->getPropertyValue("Show") >>= bSecondaryYaxisVisible;
            }
        }
        catch (const lang::IndexOutOfBoundsException&)
        {
            TOOLS_WARN_EXCEPTION("chart2", "");
        }
        //iterate through all chart types in the current coordinate system
        std::vector<rtl::Reference<ChartType>> aChartTypeList(xCooSys->getChartTypes2());
        for (std::size_t nT = 0; nT < aChartTypeList.size(); ++nT)
        {
            rtl::Reference<ChartType> xChartType(aChartTypeList[nT]);
            if (nDimensionCount == 3
                && xChartType->getChartType().equalsIgnoreAsciiCase(
                       CHART2_SERVICE_NAME_CHARTTYPE_PIE))
            {
                try
                {
                    sal_Int32 n3DRelativeHeightOldValue(100);
                    uno::Any aAny = xChartType->getFastPropertyValue(
                        PROP_PIECHARTTYPE_3DRELATIVEHEIGHT); // "3DRelativeHeight"
                    aAny >>= n3DRelativeHeightOldValue;
                    if (n3DRelativeHeightOldValue != n3DRelativeHeight)
                        xChartType->setFastPropertyValue(
                            PROP_PIECHARTTYPE_3DRELATIVEHEIGHT, // "3DRelativeHeight"
                            uno::Any(n3DRelativeHeight));
                }
                catch (const uno::Exception&)
                {
                }
            }

            if (ePieChartSubType != PieChartSubType_NONE)
            {
                xChartType->setFastPropertyValue(PROP_PIECHARTTYPE_SUBTYPE,
                                                 uno::Any(ePieChartSubType));
            }

            if (nT == 0)
                m_bChartTypeUsesShiftedCategoryPositionPerDefault
                    = ChartTypeHelper::shiftCategoryPosAtXAxisPerDefault(xChartType);

            bool bExcludingPositioning
                = xDiagram->getDiagramPositioningMode() == DiagramPositioningMode::Excluding;
            VSeriesPlotter* pPlotter = VSeriesPlotter::createSeriesPlotter(
                xChartType, nDimensionCount, bExcludingPositioning);
            if (!pPlotter)
                continue;

            m_aSeriesPlotterList.push_back(std::unique_ptr<VSeriesPlotter>(pPlotter));
            pPlotter->setNumberFormatsSupplier(xNumberFormatsSupplier);
            pPlotter->setColorScheme(xColorScheme);
            if (pVCooSys)
                pPlotter->setExplicitCategoriesProvider(pVCooSys->getExplicitCategoriesProvider());
            sal_Int32 nMissingValueTreatment
                = xDiagram->getCorrectedMissingValueTreatment(xChartType);

            if (pVCooSys)
                pVCooSys->addMinimumAndMaximumSupplier(pPlotter);

            sal_Int32 zSlot = -1;
            sal_Int32 xSlot = -1;
            sal_Int32 ySlot = -1;
            const std::vector<rtl::Reference<DataSeries>>& aSeriesList
                = xChartType->getDataSeries2();
            for (std::size_t nS = 0; nS < aSeriesList.size(); ++nS)
            {
                rtl::Reference<DataSeries> const& xDataSeries = aSeriesList[nS];
                if (!bIncludeHiddenCells && !xDataSeries->hasUnhiddenData())
                    continue;

                std::unique_ptr<VDataSeries> pSeries(new VDataSeries(xDataSeries));

                pSeries->setGlobalSeriesIndex(nGlobalSeriesIndex);
                nGlobalSeriesIndex++;

                if (bSortByXValues)
                    pSeries->doSortByXValues();

                pSeries->setConnectBars(bConnectBars);
                pSeries->setGroupBarsPerAxis(bGroupBarsPerAxis);
                pSeries->setStartingAngle(nStartingAngle);

                pSeries->setMissingValueTreatment(nMissingValueTreatment);

                OUString aSeriesParticle(ObjectIdentifier::createParticleForSeries(0, nCS, nT, nS));
                pSeries->setParticle(aSeriesParticle);

                OUString aRole(ChartTypeHelper::getRoleOfSequenceForDataLabelNumberFormatDetection(
                    xChartType));
                pSeries->setRoleOfSequenceForDataLabelNumberFormatDetection(aRole);

                //ignore secondary axis for charttypes that do not support them
                if (pSeries->getAttachedAxisIndex() != MAIN_AXIS_INDEX
                    && (!ChartTypeHelper::isSupportingSecondaryAxis(xChartType, nDimensionCount)
                        || !bSecondaryYaxisVisible))
                {
                    pSeries->setAttachedAxisIndex(MAIN_AXIS_INDEX);
                }

                StackingDirection eDirection = pSeries->getStackingDirection();
                switch (eDirection)
                {
                    case StackingDirection_NO_STACKING:
                        xSlot++;
                        ySlot = -1;
                        if (zSlot < 0)
                            zSlot = 0;
                        break;
                    case StackingDirection_Y_STACKING:
                        ySlot++;
                        if (xSlot < 0)
                            xSlot = 0;
                        if (zSlot < 0)
                            zSlot = 0;
                        break;
                    case StackingDirection_Z_STACKING:
                        zSlot++;
                        xSlot = -1;
                        ySlot = -1;
                        break;
                    default:
                        // UNO enums have one additional auto-generated case
                        break;
                }
                pPlotter->addSeries(std::move(pSeries), zSlot, xSlot, ySlot);
            }
        }
    }

    auto order
        = [](const std::unique_ptr<VSeriesPlotter>& a, const std::unique_ptr<VSeriesPlotter>& b) {
              return a->getRenderOrder() < b->getRenderOrder();
          };

    std::stable_sort(m_aSeriesPlotterList.begin(), m_aSeriesPlotterList.end(), order);

    //transport seriesnames to the coordinatesystems if needed
    if (m_aSeriesPlotterList.empty())
        return;

    uno::Sequence<OUString> aSeriesNames;
    bool bSeriesNamesInitialized = false;
    for (auto& pVCooSys : m_rVCooSysList)
    {
        if (pVCooSys->needSeriesNamesForAxis())
        {
            if (!bSeriesNamesInitialized)
            {
                aSeriesNames = m_aSeriesPlotterList[0]->getSeriesNames();
                bSeriesNamesInitialized = true;
            }
            pVCooSys->setSeriesNamesForAxis(aSeriesNames);
        }
    }
}

bool SeriesPlotterContainer::isCategoryPositionShifted(const chart2::ScaleData& rSourceScale,
                                                       bool bHasComplexCategories)
{
    if (rSourceScale.AxisType == AxisType::CATEGORY)
        return bHasComplexCategories || rSourceScale.ShiftedCategoryPosition
               || m_bTableShiftPosition || m_bChartTypeUsesShiftedCategoryPositionPerDefault;

    if (rSourceScale.AxisType == AxisType::DATE)
        return rSourceScale.ShiftedCategoryPosition;

    return rSourceScale.AxisType == AxisType::SERIES;
}

void SeriesPlotterContainer::initAxisUsageList(const Date& rNullDate)
{
    m_aAxisUsageList.clear();

    // Loop through coordinate systems in the diagram (though for now
    // there should only be one coordinate system per diagram).
    for (auto& pVCooSys : m_rVCooSysList)
    {
        rtl::Reference<BaseCoordinateSystem> xCooSys = pVCooSys->getModel();
        sal_Int32 nDimCount = xCooSys->getDimension();
        bool bComplexCategoryAllowed = ChartTypeHelper::isSupportingComplexCategory(
            AxisHelper::getChartTypeByIndex(xCooSys, 0));

        for (sal_Int32 nDimIndex = 0; nDimIndex < nDimCount; ++nDimIndex)
        {
            bool bDateAxisAllowed = ChartTypeHelper::isSupportingDateAxis(
                AxisHelper::getChartTypeByIndex(xCooSys, 0), nDimIndex);

            // Each dimension may have primary and secondary axes.
            const sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimIndex);
            for (sal_Int32 nAxisIndex = 0; nAxisIndex <= nMaxAxisIndex; ++nAxisIndex)
            {
                rtl::Reference<Axis> xAxis = xCooSys->getAxisByDimension2(nDimIndex, nAxisIndex);

                if (!xAxis.is())
                    continue;

                if (m_aAxisUsageList.find(xAxis) == m_aAxisUsageList.end())
                {
                    // Create axis usage object for this axis.

                    chart2::ScaleData aSourceScale = xAxis->getScaleData();
                    ExplicitCategoriesProvider* pCatProvider
                        = pVCooSys->getExplicitCategoriesProvider();
                    if (nDimIndex == 0)
                        AxisHelper::checkDateAxis(aSourceScale, pCatProvider, bDateAxisAllowed);

                    bool bHasComplexCat = pCatProvider && pCatProvider->hasComplexCategories()
                                          && bComplexCategoryAllowed;
                    aSourceScale.ShiftedCategoryPosition
                        = isCategoryPositionShifted(aSourceScale, bHasComplexCat);

                    m_aAxisUsageList[xAxis].aAutoScaling = ScaleAutomatism(aSourceScale, rNullDate);
                }

                AxisUsage& rAxisUsage = m_aAxisUsageList[xAxis];
                rAxisUsage.addCoordinateSystem(pVCooSys.get(), nDimIndex, nAxisIndex);
            }
        }
    }

    // Determine the highest axis index of all dimensions.
    m_nMaxAxisIndex = 0;
    for (const auto& pVCooSys : m_rVCooSysList)
    {
        uno::Reference<XCoordinateSystem> xCooSys = pVCooSys->getModel();
        sal_Int32 nDimCount = xCooSys->getDimension();

        for (sal_Int32 nDimIndex = 0; nDimIndex < nDimCount; ++nDimIndex)
        {
            for (auto& axisUsage : m_aAxisUsageList)
            {
                sal_Int32 nLocalMax = axisUsage.second.getMaxAxisIndexForDimension(nDimIndex);
                if (m_nMaxAxisIndex < nLocalMax)
                    m_nMaxAxisIndex = nLocalMax;
            }
        }
    }
}

void SeriesPlotterContainer::setScalesFromCooSysToPlotter()
{
    //set scales to plotter to enable them to provide the preferred scene AspectRatio
    for (const std::unique_ptr<VSeriesPlotter>& aPlotter : m_aSeriesPlotterList)
    {
        VSeriesPlotter* pSeriesPlotter = aPlotter.get();
        VCoordinateSystem* pVCooSys
            = SeriesPlotterContainer::getCooSysForPlotter(m_rVCooSysList, pSeriesPlotter);
        if (pVCooSys)
        {
            pSeriesPlotter->setScales(pVCooSys->getExplicitScales(0, 0),
                                      pVCooSys->getPropertySwapXAndYAxis());
            sal_Int32 nMaxAxisIndex = pVCooSys->getMaximumAxisIndexByDimension(
                1); //only additional value axis are relevant for series plotter
            for (sal_Int32 nI = 1; nI <= nMaxAxisIndex; nI++)
                pSeriesPlotter->addSecondaryValueScale(pVCooSys->getExplicitScale(1, nI), nI);
        }
    }
}

void SeriesPlotterContainer::setNumberFormatsFromAxes()
{
    //set numberformats to plotter to enable them to display the data labels in the numberformat of the axis
    for (const std::unique_ptr<VSeriesPlotter>& aPlotter : m_aSeriesPlotterList)
    {
        VSeriesPlotter* pSeriesPlotter = aPlotter.get();
        VCoordinateSystem* pVCooSys
            = SeriesPlotterContainer::getCooSysForPlotter(m_rVCooSysList, pSeriesPlotter);
        if (pVCooSys)
        {
            AxesNumberFormats aAxesNumberFormats;
            const rtl::Reference<BaseCoordinateSystem>& xCooSys = pVCooSys->getModel();
            sal_Int32 nDimensionCount = xCooSys->getDimension();
            for (sal_Int32 nDimensionIndex = 0; nDimensionIndex < nDimensionCount;
                 ++nDimensionIndex)
            {
                const sal_Int32 nMaximumAxisIndex
                    = xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
                for (sal_Int32 nAxisIndex = 0; nAxisIndex <= nMaximumAxisIndex; ++nAxisIndex)
                {
                    try
                    {
                        rtl::Reference<Axis> xAxisProp
                            = xCooSys->getAxisByDimension2(nDimensionIndex, nAxisIndex);
                        if (xAxisProp.is())
                        {
                            sal_Int32 nNumberFormatKey(0);
                            if (xAxisProp->getPropertyValue(CHART_UNONAME_NUMFMT)
                                >>= nNumberFormatKey)
                            {
                                aAxesNumberFormats.setFormat(nNumberFormatKey, nDimensionIndex,
                                                             nAxisIndex);
                            }
                            else if (nDimensionIndex == 0)
                            {
                                //provide a default date format for date axis with own data
                                aAxesNumberFormats.setFormat(m_nDefaultDateNumberFormat,
                                                             nDimensionIndex, nAxisIndex);
                            }
                        }
                    }
                    catch (const lang::IndexOutOfBoundsException&)
                    {
                        TOOLS_WARN_EXCEPTION("chart2", "");
                    }
                }
            }
        }
    }
}

void SeriesPlotterContainer::updateScalesAndIncrementsOnAxes()
{
    for (auto& nC : m_rVCooSysList)
        nC->updateScalesAndIncrementsOnAxes();
}

void SeriesPlotterContainer::doAutoScaling(ChartModel& rChartModel)
{
    if (m_aSeriesPlotterList.empty() || m_aAxisUsageList.empty())
        // We need these two containers populated to do auto-scaling.  Bail out.
        return;

    //iterate over the main scales first than secondary axis
    for (sal_Int32 nAxisIndex = 0; nAxisIndex <= m_nMaxAxisIndex; ++nAxisIndex)
    {
        // - first do autoscale for all x and z scales (because they are treated independent)
        for (auto & [ rAxis, rAxisUsage ] : m_aAxisUsageList)
        {
            (void)rAxis;
            rAxisUsage.prepareAutomaticAxisScaling(rAxisUsage.aAutoScaling, 0, nAxisIndex);
            rAxisUsage.prepareAutomaticAxisScaling(rAxisUsage.aAutoScaling, 2, nAxisIndex);

            ExplicitScaleData aExplicitScale;
            ExplicitIncrementData aExplicitIncrement;
            rAxisUsage.aAutoScaling.calculateExplicitScaleAndIncrement(aExplicitScale,
                                                                       aExplicitIncrement);

            rAxisUsage.setExplicitScaleAndIncrement(0, nAxisIndex, aExplicitScale,
                                                    aExplicitIncrement);
            rAxisUsage.setExplicitScaleAndIncrement(2, nAxisIndex, aExplicitScale,
                                                    aExplicitIncrement);
        }

        // - second do autoscale for the dependent y scales (the coordinate systems are prepared with x and z scales already )
        for (auto & [ rAxis, rAxisUsage ] : m_aAxisUsageList)
        {
            (void)rAxis;
            rAxisUsage.prepareAutomaticAxisScaling(rAxisUsage.aAutoScaling, 1, nAxisIndex);

            ExplicitScaleData aExplicitScale;
            ExplicitIncrementData aExplicitIncrement;
            rAxisUsage.aAutoScaling.calculateExplicitScaleAndIncrement(aExplicitScale,
                                                                       aExplicitIncrement);

            rAxisUsage.setExplicitScaleAndIncrement(0, nAxisIndex, aExplicitScale,
                                                    aExplicitIncrement);
            rAxisUsage.setExplicitScaleAndIncrement(1, nAxisIndex, aExplicitScale,
                                                    aExplicitIncrement);
            rAxisUsage.setExplicitScaleAndIncrement(2, nAxisIndex, aExplicitScale,
                                                    aExplicitIncrement);
        }
    }
    AdaptScaleOfYAxisWithoutAttachedSeries(rChartModel);
}

void SeriesPlotterContainer::AdaptScaleOfYAxisWithoutAttachedSeries(ChartModel& rModel)
{
    //issue #i80518#
    for (sal_Int32 nAxisIndex = 0; nAxisIndex <= m_nMaxAxisIndex; nAxisIndex++)
    {
        for (auto & [ rAxis, rAxisUsage ] : m_aAxisUsageList)
        {
            (void)rAxis;
            std::vector<VCoordinateSystem*> aVCooSysList_Y
                = rAxisUsage.getCoordinateSystems(1, nAxisIndex);
            if (aVCooSysList_Y.empty())
                continue;

            rtl::Reference<Diagram> xDiagram(rModel.getFirstChartDiagram());
            if (!xDiagram.is())
                continue;

            bool bSeriesAttachedToThisAxis = false;
            sal_Int32 nAttachedAxisIndex = -1;
            {
                std::vector<rtl::Reference<DataSeries>> aSeriesVector = xDiagram->getDataSeries();
                for (auto const& series : aSeriesVector)
                {
                    sal_Int32 nCurrentIndex = DataSeriesHelper::getAttachedAxisIndex(series);
                    if (nAxisIndex == nCurrentIndex)
                    {
                        bSeriesAttachedToThisAxis = true;
                        break;
                    }
                    else if (nAttachedAxisIndex < 0 || nAttachedAxisIndex > nCurrentIndex)
                        nAttachedAxisIndex = nCurrentIndex;
                }
            }

            if (bSeriesAttachedToThisAxis || nAttachedAxisIndex < 0)
                continue;

            for (VCoordinateSystem* nC : aVCooSysList_Y)
            {
                nC->prepareAutomaticAxisScaling(rAxisUsage.aAutoScaling, 1, nAttachedAxisIndex);

                ExplicitScaleData aExplicitScaleSource
                    = nC->getExplicitScale(1, nAttachedAxisIndex);
                ExplicitIncrementData aExplicitIncrementSource
                    = nC->getExplicitIncrement(1, nAttachedAxisIndex);

                ExplicitScaleData aExplicitScaleDest = nC->getExplicitScale(1, nAxisIndex);
                ExplicitIncrementData aExplicitIncrementDest
                    = nC->getExplicitIncrement(1, nAxisIndex);

                aExplicitScaleDest.Orientation = aExplicitScaleSource.Orientation;
                aExplicitScaleDest.Scaling = aExplicitScaleSource.Scaling;
                aExplicitScaleDest.AxisType = aExplicitScaleSource.AxisType;

                aExplicitIncrementDest.BaseValue = aExplicitIncrementSource.BaseValue;

                ScaleData aScale(rAxisUsage.aAutoScaling.getScale());
                if (!aScale.Minimum.hasValue())
                {
                    bool bNewMinOK = true;
                    double fMax = 0.0;
                    if (aScale.Maximum >>= fMax)
                        bNewMinOK = (aExplicitScaleSource.Minimum <= fMax);
                    if (bNewMinOK)
                        aExplicitScaleDest.Minimum = aExplicitScaleSource.Minimum;
                }
                else
                    aExplicitIncrementDest.BaseValue = aExplicitScaleDest.Minimum;

                if (!aScale.Maximum.hasValue())
                {
                    bool bNewMaxOK = true;
                    double fMin = 0.0;
                    if (aScale.Minimum >>= fMin)
                        bNewMaxOK = (fMin <= aExplicitScaleSource.Maximum);
                    if (bNewMaxOK)
                        aExplicitScaleDest.Maximum = aExplicitScaleSource.Maximum;
                }
                if (!aScale.Origin.hasValue())
                    aExplicitScaleDest.Origin = aExplicitScaleSource.Origin;

                if (!aScale.IncrementData.Distance.hasValue())
                    aExplicitIncrementDest.Distance = aExplicitIncrementSource.Distance;

                bool bAutoMinorInterval = true;
                if (aScale.IncrementData.SubIncrements.hasElements())
                    bAutoMinorInterval
                        = !(aScale.IncrementData.SubIncrements[0].IntervalCount.hasValue());
                if (bAutoMinorInterval)
                {
                    if (!aExplicitIncrementDest.SubIncrements.empty()
                        && !aExplicitIncrementSource.SubIncrements.empty())
                        aExplicitIncrementDest.SubIncrements[0].IntervalCount
                            = aExplicitIncrementSource.SubIncrements[0].IntervalCount;
                }

                nC->setExplicitScaleAndIncrement(1, nAxisIndex, aExplicitScaleDest,
                                                 aExplicitIncrementDest);
            }
        }
    }

    if (!AxisHelper::isAxisPositioningEnabled())
        return;

    //correct origin for y main axis (the origin is where the other main axis crosses)
    sal_Int32 nAxisIndex = 0;
    sal_Int32 nDimensionIndex = 1;
    for (auto & [ rAxis, rAxisUsage ] : m_aAxisUsageList)
    {
        (void)rAxis;
        std::vector<VCoordinateSystem*> aVCooSysList
            = rAxisUsage.getCoordinateSystems(nDimensionIndex, nAxisIndex);
        size_t nC;
        for (nC = 0; nC < aVCooSysList.size(); nC++)
        {
            ExplicitScaleData aExplicitScale(
                aVCooSysList[nC]->getExplicitScale(nDimensionIndex, nAxisIndex));
            ExplicitIncrementData aExplicitIncrement(
                aVCooSysList[nC]->getExplicitIncrement(nDimensionIndex, nAxisIndex));

            rtl::Reference<BaseCoordinateSystem> xCooSys(aVCooSysList[nC]->getModel());
            rtl::Reference<Axis> xAxis = xCooSys->getAxisByDimension2(nDimensionIndex, nAxisIndex);
            rtl::Reference<Axis> xCrossingMainAxis
                = AxisHelper::getCrossingMainAxis(xAxis, xCooSys);

            if (xCrossingMainAxis.is())
            {
                css::chart::ChartAxisPosition eCrossingMainAxisPos(
                    css::chart::ChartAxisPosition_ZERO);
                xCrossingMainAxis->getPropertyValue("CrossoverPosition") >>= eCrossingMainAxisPos;
                if (eCrossingMainAxisPos == css::chart::ChartAxisPosition_VALUE)
                {
                    double fValue = 0.0;
                    xCrossingMainAxis->getPropertyValue("CrossoverValue") >>= fValue;
                    aExplicitScale.Origin = fValue;
                }
                else if (eCrossingMainAxisPos == css::chart::ChartAxisPosition_ZERO)
                    aExplicitScale.Origin = 0.0;
                else if (eCrossingMainAxisPos == css::chart::ChartAxisPosition_START)
                    aExplicitScale.Origin = aExplicitScale.Minimum;
                else if (eCrossingMainAxisPos == css::chart::ChartAxisPosition_END)
                    aExplicitScale.Origin = aExplicitScale.Maximum;
            }

            aVCooSysList[nC]->setExplicitScaleAndIncrement(nDimensionIndex, nAxisIndex,
                                                           aExplicitScale, aExplicitIncrement);
        }
    }
}

drawing::Direction3D SeriesPlotterContainer::getPreferredAspectRatio()
{
    drawing::Direction3D aPreferredAspectRatio(1.0, 1.0, 1.0);

    //get a list of all preferred aspect ratios and combine them
    //first with special demands wins (less or equal zero <-> arbitrary)
    double fx, fy, fz;
    fx = fy = fz = -1.0;
    for (const std::unique_ptr<VSeriesPlotter>& aPlotter : m_aSeriesPlotterList)
    {
        drawing::Direction3D aSingleRatio(aPlotter->getPreferredDiagramAspectRatio());
        if (fx < 0 && aSingleRatio.DirectionX > 0)
            fx = aSingleRatio.DirectionX;

        if (fy < 0 && aSingleRatio.DirectionY > 0)
        {
            if (fx > 0 && aSingleRatio.DirectionX > 0)
                fy = fx * aSingleRatio.DirectionY / aSingleRatio.DirectionX;
            else if (fz > 0 && aSingleRatio.DirectionZ > 0)
                fy = fz * aSingleRatio.DirectionY / aSingleRatio.DirectionZ;
            else
                fy = aSingleRatio.DirectionY;
        }

        if (fz < 0 && aSingleRatio.DirectionZ > 0)
        {
            if (fx > 0 && aSingleRatio.DirectionX > 0)
                fz = fx * aSingleRatio.DirectionZ / aSingleRatio.DirectionX;
            else if (fy > 0 && aSingleRatio.DirectionY > 0)
                fz = fy * aSingleRatio.DirectionZ / aSingleRatio.DirectionY;
            else
                fz = aSingleRatio.DirectionZ;
        }

        if (fx > 0 && fy > 0 && fz > 0)
            break;
    }
    aPreferredAspectRatio = drawing::Direction3D(fx, fy, fz);
    return aPreferredAspectRatio;
}

} //end chart2 namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
