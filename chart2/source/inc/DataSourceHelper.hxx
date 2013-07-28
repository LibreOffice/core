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

#ifndef _CHART2_DATASOURCEHELPER_HXX
#define _CHART2_DATASOURCEHELPER_HXX

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "charttoolsdllapi.hxx"

#include <vector>

namespace chart
{

/**
*/

class OOO_DLLPUBLIC_CHARTTOOLS DataSourceHelper
{
public:
        static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >
            createDataSource( const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::data::XLabeledDataSequence > > & rSequences );

        static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >
            createCachedDataSequence();

        static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >
            createCachedDataSequence( const OUString & rSingleText );

        static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
            createLabeledDataSequence(
                const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xValues ,
                const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xLabels );

        static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
            createLabeledDataSequence(
                const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xValues );

        static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
            createLabeledDataSequence(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );

        static ::com::sun::star::uno::Sequence<
               ::com::sun::star::beans::PropertyValue > createArguments(
                   bool bUseColumns, bool bFirstCellAsLabel, bool bHasCategories );

        static ::com::sun::star::uno::Sequence<
               ::com::sun::star::beans::PropertyValue > createArguments(
                   const OUString & rRangeRepresentation,
                   const ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping,
                   bool bUseColumns, bool bFirstCellAsLabel, bool bHasCategories );

        SAL_DLLPRIVATE static void readArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArguments
            , OUString & rRangeRepresentation, ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping
            , bool& bUseColumns, bool& bFirstCellAsLabel, bool& bHasCategories );

        static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >
            pressUsedDataIntoRectangularFormat( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument >& xChartDoc,
                bool bWithCategories = true );

        static void addRangeRepresentationsFromLabeledDataSequence(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XLabeledDataSequence >& xLabeledDataSequence
            ,  ::std::vector< OUString >& rOutRangeRepresentations );

        SAL_DLLPRIVATE static ::com::sun::star::uno::Sequence< OUString > getUsedDataRanges(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram > & xDiagram );

        static ::com::sun::star::uno::Sequence< OUString > getUsedDataRanges(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel > & xChartModel );

        static ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSource > getUsedData(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xChartModel );

        static ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSource > getUsedData(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument >& xChartDoc );

        static bool detectRangeSegmentation(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xChartModel
            , OUString& rOutRangeString
            , ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping
            , bool& rOutUseColumns
            , bool& rOutFirstCellAsLabel
            , bool& rOutHasCategories );

        static void setRangeSegmentation(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xChartModel
            , const ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping
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
            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDocument );

        SAL_DLLPRIVATE static ::com::sun::star::uno::Sequence< OUString > getRangesFromLabeledDataSequence(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XLabeledDataSequence > & xLSeq );

        SAL_DLLPRIVATE static OUString getRangeFromValues(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XLabeledDataSequence > & xLSeq );

        SAL_DLLPRIVATE static ::com::sun::star::uno::Sequence< OUString > getRangesFromDataSource(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSource > & xSource );
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
