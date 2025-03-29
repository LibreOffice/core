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

#include <config_options.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <rtl/ref.hxx>

namespace chart { class ChartModel; }
namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::chart2::data { class XDataSequence; }
namespace com::sun::star::chart2::data { class XDataSource; }
namespace com::sun::star::chart2::data { class XLabeledDataSequence; }

namespace chart
{
class DataSource;
class Diagram;
class LabeledDataSequence;

class DataSourceHelper
{
public:
        static css::uno::Reference< css::chart2::data::XDataSequence >
            createCachedDataSequence();

        static css::uno::Reference< css::chart2::data::XDataSequence >
            createCachedDataSequence( const OUString & rSingleText );

        static rtl::Reference< LabeledDataSequence >
            createLabeledDataSequence(
                const css::uno::Reference< css::chart2::data::XDataSequence >& xValues ,
                const css::uno::Reference< css::chart2::data::XDataSequence >& xLabels );

        static rtl::Reference< LabeledDataSequence >
            createLabeledDataSequence(
                const css::uno::Reference< css::chart2::data::XDataSequence >& xValues );

        static rtl::Reference< LabeledDataSequence >
            createLabeledDataSequence();

        static css::uno::Sequence< css::beans::PropertyValue >
            createArguments(
                   bool bUseColumns, bool bFirstCellAsLabel, bool bHasCategories );

        static css::uno::Sequence<
               css::beans::PropertyValue > createArguments(
                   const OUString & rRangeRepresentation,
                   const css::uno::Sequence< sal_Int32 >& rSequenceMapping,
                   bool bUseColumns, bool bFirstCellAsLabel, bool bHasCategories );

        static void readArguments( const css::uno::Sequence< css::beans::PropertyValue >& rArguments
            , OUString & rRangeRepresentation, css::uno::Sequence< sal_Int32 >& rSequenceMapping
            , bool& bUseColumns, bool& bFirstCellAsLabel, bool& bHasCategories );

        static rtl::Reference< ::chart::DataSource >
            pressUsedDataIntoRectangularFormat( const rtl::Reference< ::chart::ChartModel >& xChartDoc );

        static css::uno::Sequence< OUString > getUsedDataRanges(
            const rtl::Reference< ::chart::Diagram > & xDiagram );

        static css::uno::Sequence< OUString > getUsedDataRanges(
            const rtl::Reference<::chart::ChartModel> & xChartModel );

        static rtl::Reference< ::chart::DataSource > getUsedData(
                ChartModel& rModel );

        static bool detectRangeSegmentation(
            const rtl::Reference<::chart::ChartModel>& xChartModel
            , OUString& rOutRangeString
            , css::uno::Sequence< sal_Int32 >& rSequenceMapping
            , bool& rOutUseColumns
            , bool& rOutFirstCellAsLabel
            , bool& rOutHasCategories );

        static void setRangeSegmentation(
            const rtl::Reference<::chart::ChartModel>& xChartModel
            , const css::uno::Sequence< sal_Int32 >& rSequenceMapping
            , bool bUseColumns
            , bool bFirstCellAsLabel
            , bool bUseCategories );

        /** Returns true, if all arguments necessary for getting all data by a
            rectangular region are returned by detectArguments at the given
            document's data provider.

            Currently, this is: CellRangeRepresentation, DataRowSource,
            HasCategories and FirstCellAsLabel.
         */
        static bool allArgumentsForRectRangeDetected(
            const rtl::Reference<::chart::ChartModel>& xChartDocument );

        static css::uno::Sequence< OUString > getRangesFromLabeledDataSequence(
            const css::uno::Reference< css::chart2::data::XLabeledDataSequence > & xLSeq );

        static OUString getRangeFromValues(
            const css::uno::Reference< css::chart2::data::XLabeledDataSequence > & xLSeq );

        static css::uno::Sequence< OUString > getRangesFromDataSource(
            const css::uno::Reference< css::chart2::data::XDataSource > & xSource );
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
