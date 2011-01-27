/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
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
            createCachedDataSequence( const ::rtl::OUString & rSingleText );

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
                   const ::rtl::OUString & rRangeRepresentation,
                   const ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping,
                   bool bUseColumns, bool bFirstCellAsLabel, bool bHasCategories );

        SAL_DLLPRIVATE static void readArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArguments
            , ::rtl::OUString & rRangeRepresentation, ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping
            , bool& bUseColumns, bool& bFirstCellAsLabel, bool& bHasCategories );

        static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >
            pressUsedDataIntoRectangularFormat( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument >& xChartDoc,
                bool bWithCategories = true );

        static void addRangeRepresentationsFromLabeledDataSequence(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XLabeledDataSequence >& xLabeledDataSequence
            ,  ::std::vector< ::rtl::OUString >& rOutRangeRepresentations );

        SAL_DLLPRIVATE static ::com::sun::star::uno::Sequence< ::rtl::OUString > getUsedDataRanges(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram > & xDiagram );

        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getUsedDataRanges(
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
            , ::rtl::OUString& rOutRangeString
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

        SAL_DLLPRIVATE static ::com::sun::star::uno::Sequence< ::rtl::OUString > getRangesFromLabeledDataSequence(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XLabeledDataSequence > & xLSeq );

        SAL_DLLPRIVATE static ::rtl::OUString getRangeFromValues(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XLabeledDataSequence > & xLSeq );

        SAL_DLLPRIVATE static ::com::sun::star::uno::Sequence< ::rtl::OUString > getRangesFromDataSource(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSource > & xSource );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
