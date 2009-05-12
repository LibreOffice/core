/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataSourceHelper.hxx,v $
 * $Revision: 1.3.44.1 $
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
#ifndef _COM_SUN_STAR_CHART2_XLABELEDDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#endif
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XModel.hpp>
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
        static ::com::sun::star::uno::Sequence<
               ::com::sun::star::beans::PropertyValue > createArguments(
                   bool bUseColumns, bool bFirstCellAsLabel, bool bHasCategories );

        static ::com::sun::star::uno::Sequence<
               ::com::sun::star::beans::PropertyValue > createArguments(
                   const ::rtl::OUString & rRangeRepresentation,
                   const ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping,
                   bool bUseColumns, bool bFirstCellAsLabel, bool bHasCategories );

        static void readArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArguments
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

        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getUsedDataRanges(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram > & xDiagram );

        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getUsedDataRanges(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel > & xChartModel );

        static ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSource > getUsedData(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xChartModel,
            bool bIncludeUnusedData = false );

        static ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSource > getUsedData(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument >& xChartDoc,
            bool bIncludeUnusedData = false );

        static bool detectRangeSegmentation(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xChartModel
            , ::rtl::OUString& rOutRangeString
            , ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping
            , bool& rOutUseColumns
            , bool& rOutFirstCellAsLabel
            , bool& rOutHasCategories
            , bool bIncludeUnusedData = false );

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

        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getRangesFromLabeledDataSequence(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XLabeledDataSequence > & xLSeq );

        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getRangesFromDataSource(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSource > & xSource );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
