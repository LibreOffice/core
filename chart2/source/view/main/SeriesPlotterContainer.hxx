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

#include <config_feature_desktop.h>
#include <VSeriesPlotter.hxx>
#include <BaseCoordinateSystem.hxx>
#include "AxisUsage.hxx"

namespace chart
{
/** This class is a container of `SeriesPlotter` objects (such as `PieChart`
 *  instances). It is used for initializing coordinate systems, axes and scales
 *  of all series plotters which belongs to the container.
 */
class SeriesPlotterContainer
{
public:
    explicit SeriesPlotterContainer(std::vector<std::unique_ptr<VCoordinateSystem>>& rVCooSysList);
    ~SeriesPlotterContainer();

    /** It is used to set coordinate systems (`m_rVCooSysList`), this method
     *  is invoked by `ChartView::createShapes2D` before of
     *  `ChartView::impl_createDiagramAndContent`.
     *  Coordinate systems are retrieved through the `XCoordinateSystemContainer`
     *  interface implemented by a diagram object which is provided by the
     *  `ChartModel` object passed to the method (`rChartModel.getFirstDiagram()`).
     *
     *  It is used for creating series plotters and appending them
     *  to `m_aSeriesPlotterList`. The created series plotters are initialized
     *  through data (number formats supplier, color scheme, data series),
     *  extracted from the chart model or the diagram objects. An exception is
     *  the explicit category provider that is retrieved through the
     *  `VCoordinateSystem` object used by the series plotter.
     *
     *  It sets the minimum-maximum supplier for a coordinate system:
     *  this supplier is the series plotter itself which utilizes the given
     *  coordinate system. In fact `VSeriesPlotter` has `MinimumMaximumSupplier`
     *  as one of its base classes.
     *  Hence, for instance, a `PieChart`, which is a series plotter, is
     *  a `MinimumMaximumSupplier`, too.
     */
    void initializeCooSysAndSeriesPlotter(ChartModel& rModel);

    /** This method is invoked by `ChartView::impl_createDiagramAndContent`.
     *  It iterates on every axis of every coordinate systems, and if the axis
     *  is not yet present in `m_aAxisUsageList` it creates a new `AxisUsage`
     *  object and initialize its `aAutoScaling` member to the `ScaleData`
     *  object of the current axis.
     */
    void initAxisUsageList(const Date& rNullDate);

    /**
     * Perform automatic axis scaling and determine the amount and spacing of
     * increments.  It assumes that the caller has determined the size of the
     * largest axis label text object prior to calling this method.
     *
     * The new axis scaling data will be stored in the VCoordinateSystem
     * objects.  The label alignment direction for each axis will also get
     * determined during this process, and stored in VAxis.
     *
     * This method is invoked by `ChartView::impl_createDiagramAndContent`
     * soon after `initAxisUsageList`.
     * It initializes explicit scale and increment objects for all coordinate
     * systems in `m_rVCooSysList`.
     * This action is achieved by iterating on the `m_aAxisUsageList` container,
     * and performing 3 steps:
     *   1- call `VCoordinateSystem::prepareAutomaticAxisScaling` for setting
     *      scaling parameters of the `aAutoScaling` member (a `ScaleAutomatism`
     *      object) for the current `AxisUsage` instance
     *      (see `VCoordinateSystem::prepareAutomaticAxisScaling`);
     *   2- calculate the explicit scale and increment objects
     *      (see ScaleAutomatism::calculateExplicitScaleAndIncrement);
     *   3- set the explicit scale and increment objects for each coordinate
     *      system.
     */
    void doAutoScaling(ChartModel& rModel);

    /**
     * After auto-scaling is performed, call this method to set the explicit
     * scaling and increment data to all relevant VAxis objects.
     */
    void updateScalesAndIncrementsOnAxes();

    /**
     * After auto-scaling is performed, call this method to set the explicit
     * scaling data to all the plotters.
     */
    void setScalesFromCooSysToPlotter();

    void setNumberFormatsFromAxes();
    css::drawing::Direction3D getPreferredAspectRatio();

    std::vector<std::unique_ptr<VSeriesPlotter>>& getSeriesPlotterList()
    {
        return m_aSeriesPlotterList;
    }
    std::vector<std::unique_ptr<VCoordinateSystem>>& getCooSysList() { return m_rVCooSysList; }
    std::vector<LegendEntryProvider*> getLegendEntryProviderList();

    void AdaptScaleOfYAxisWithoutAttachedSeries(ChartModel& rModel);

    bool isCategoryPositionShifted(const css::chart2::ScaleData& rSourceScale,
                                   bool bHasComplexCategories);

    static VCoordinateSystem*
    getCooSysForPlotter(const std::vector<std::unique_ptr<VCoordinateSystem>>& rVCooSysList,
                        MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier);
    static VCoordinateSystem*
    addCooSysToList(std::vector<std::unique_ptr<VCoordinateSystem>>& rVCooSysList,
                    const rtl::Reference<BaseCoordinateSystem>& xCooSys, ChartModel& rChartModel);
    static VCoordinateSystem*
    findInCooSysList(const std::vector<std::unique_ptr<VCoordinateSystem>>& rVCooSysList,
                     const rtl::Reference<BaseCoordinateSystem>& xCooSys);

private:
    /** A vector of series plotters.
     */
    std::vector<std::unique_ptr<VSeriesPlotter>> m_aSeriesPlotterList;

    /** A vector of coordinate systems.
     */
    std::vector<std::unique_ptr<VCoordinateSystem>>& m_rVCooSysList;

    /** A map whose key is a `XAxis` interface and the related value is
     *  an object of `AxisUsage` type.
     */
    std::map<rtl::Reference<Axis>, AxisUsage> m_aAxisUsageList;

    /**
     * Max axis index of all dimensions.  Currently this can be either 0 or 1
     * since we only support primary and secondary axes per dimension.  The
     * value of 0 means all dimensions have only primary axis, while 1 means
     * at least one dimension has a secondary axis.
     */
    sal_Int32 m_nMaxAxisIndex;

    bool m_bChartTypeUsesShiftedCategoryPositionPerDefault;
    bool m_bTableShiftPosition = false;
    sal_Int32 m_nDefaultDateNumberFormat;
};

} //end chart2 namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
