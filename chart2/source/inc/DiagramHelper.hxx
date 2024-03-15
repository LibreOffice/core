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
#pragma once

#include "StackMode.hxx"
#include "charttoolsdllapi.hxx"
#include "ChartTypeTemplate.hxx"
#include <com/sun/star/awt/Rectangle.hpp>
#include <rtl/ref.hxx>

namespace chart { class ChartModel; }
namespace com::sun::star::chart2 { class XAxis; }
namespace com::sun::star::chart2 { class XChartDocument; }
namespace com::sun::star::chart2 { class XChartType; }
namespace com::sun::star::chart2 { class XCoordinateSystem; }
namespace com::sun::star::chart2 { class XDiagram; }
namespace com::sun::star::chart2 { class XDataSeries; }
namespace com::sun::star::chart2::data { class XLabeledDataSequence; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::lang { class XMultiServiceFactory; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::util { class XNumberFormats; }
namespace com::sun::star::util { class XNumberFormatsSupplier; }

namespace chart
{
class Axis;
class BaseCoordinateSystem;
class ChartType;
class ChartTypeManager;
class ChartTypeTemplate;
class DataSeries;
class Diagram;
class LabeledDataSequence;

class UNLESS_MERGELIBS(OOO_DLLPUBLIC_CHARTTOOLS) DiagramHelper
{
public:

    /** Retrieves the stackmode of the first DataSeries or none. If the series have differing stack
        modes, rbAmbiguous is set to true. If no series is there rbFound is set to false.

        @param xCorrespondingCoordinateSystem
            The coordinate system in which the given chart type xChartType is
            located.  (This is needed for determining percent stacking.  If
            omitted, the result will just indicate "not stacked", "stacked" or
            "ambiguous")
     */
    static StackMode getStackModeFromChartType(
        const rtl::Reference< ::chart::ChartType > & xChartType,
        bool& rbFound, bool& rbAmbiguous,
        const rtl::Reference< ::chart::BaseCoordinateSystem > & xCorrespondingCoordinateSystem
        );

    static bool isSeriesAttachedToMainAxis(
        const rtl::Reference< ::chart::DataSeries >& xDataSeries );

    static css::uno::Sequence< OUString >
        getExplicitSimpleCategories( ChartModel& rModel );

    SAL_DLLPRIVATE static css::uno::Sequence< OUString >
        generateAutomaticCategoriesFromCooSys(
            const rtl::Reference< ::chart::BaseCoordinateSystem > & xCooSys );

    static void switchToDateCategories(
        const rtl::Reference<::chart::ChartModel> & xChartDoc );

    static void switchToTextCategories(
        const rtl::Reference<::chart::ChartModel> & xChartDoc );

    static bool isDateNumberFormat( sal_Int32 nNumberFormat, const css::uno::Reference< css::util::XNumberFormats >& xNumberFormats );
    static sal_Int32 getDateNumberFormat( const css::uno::Reference< css::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );
    static sal_Int32 getDateTimeInputNumberFormat( const css::uno::Reference< css::util::XNumberFormatsSupplier >& xNumberFormatsSupplier, double fNumber );

    static sal_Int32 getPercentNumberFormat( const css::uno::Reference<
                css::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

    SAL_DLLPRIVATE static bool areChartTypesCompatible(
                const rtl::Reference< ::chart::ChartType >& xFirstType,
                const rtl::Reference< ::chart::ChartType >& xSecondType );

    static bool setDiagramPositioning( const rtl::Reference<::chart::ChartModel>& xChartModel,
        const css::awt::Rectangle& rPosRect /*100th mm*/ );

    static css::awt::Rectangle getDiagramRectangleFromModel( const rtl::Reference<::chart::ChartModel>& xChartModel );

    static bool switchDiagramPositioningToExcludingPositioning( ChartModel& rModel
        , bool bResetModifiedState //set model back to unchanged if it was unchanged before
        , bool bConvertAlsoFromAutoPositioning );

private:
    DiagramHelper() = delete;

};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
