/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSourceHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:15:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CHART2_DATASOURCEHELPER_HXX
#define _CHART2_DATASOURCEHELPER_HXX

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASOURCE_HPP_
#include <com/sun/star/chart2/data/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XLABELEDDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#include <vector>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class DataSourceHelper
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

        static bool migrateData(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument > & xChartDoc,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataProvider > & xNewDataProvider );

};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
