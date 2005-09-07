/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MultipleChartConverters.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:29:56 $
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
#include "MultipleChartConverters.hxx"

#include "SchSfxItemIds.hxx"
#include "SchWhichPairs.hxx"
#include "AxisItemConverter.hxx"
#include "StatisticsItemConverter.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "DataPointItemConverter.hxx"
#include "ChartModelHelper.hxx"
#include "TitleHelper.hxx"
#include "TitleItemConverter.hxx"

#ifndef _COM_SUN_STAR_CHART2_XAXISCONTAINER_HPP_
#include <com/sun/star/chart2/XAxisContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XGRIDCONTAINER_HPP_
#include <com/sun/star/chart2/XGridContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif

using namespace ::com::sun::star;

namespace chart
{
namespace wrapper
{

//-----------------------------------------------------------------------------

AllAxisItemConverter::AllAxisItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    ::std::auto_ptr< awt::Size > pRefSize )
        : MultipleItemConverter( rItemPool )
{
    uno::Reference< chart2::XChartDocument > xChartDocument( xChartModel, uno::UNO_QUERY );
    if(!xChartDocument.is())
        return;
    uno::Reference< chart2::XDiagram > xDiagram = xChartDocument->getDiagram();
    uno::Reference< chart2::XAxisContainer > xContainer( xDiagram, uno::UNO_QUERY );
    if(!xContainer.is())
        return;
    uno::Sequence< uno::Reference< chart2::XAxis > > aElementList = xContainer->getAxes();
    for( sal_Int32 nA = 0; nA < aElementList.getLength(); nA++ )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties(aElementList[nA], uno::UNO_QUERY);
        if( pRefSize.get())
            m_aConverters.push_back( new ::chart::wrapper::AxisItemConverter(
                                         xObjectProperties, rItemPool, rDrawModel, 0, 0, 0, 0,
                                         ::std::auto_ptr< awt::Size >( new awt::Size( *pRefSize )) ));
        else
            m_aConverters.push_back( new ::chart::wrapper::AxisItemConverter(
                                         xObjectProperties, rItemPool, rDrawModel, 0, 0, 0, 0 ) );
    }
}

AllAxisItemConverter::~AllAxisItemConverter()
{
}

const USHORT * AllAxisItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nAllAxisWhichPairs;
}

//-----------------------------------------------------------------------------

AllGridItemConverter::AllGridItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel  )
        : MultipleItemConverter( rItemPool )
{
    uno::Reference< chart2::XChartDocument > xChartDocument( xChartModel, uno::UNO_QUERY );
    if(!xChartDocument.is())
        return;
    uno::Reference< chart2::XDiagram > xDiagram = xChartDocument->getDiagram();
    uno::Reference< chart2::XGridContainer > xContainer( xDiagram, uno::UNO_QUERY );
    if(!xContainer.is())
        return;
    uno::Sequence< uno::Reference< chart2::XGrid > > aElementList = xContainer->getGrids();
    for( sal_Int32 nA = 0; nA < aElementList.getLength(); nA++ )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties(aElementList[nA], uno::UNO_QUERY);
        m_aConverters.push_back( new ::chart::wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rItemPool, rDrawModel,
                                        ::chart::wrapper::GraphicPropertyItemConverter::LINE_PROPERTIES ) );
    }
}

AllGridItemConverter::~AllGridItemConverter()
{
}

const USHORT * AllGridItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nGridWhichPairs;
}

//-----------------------------------------------------------------------------

AllDataLabelItemConverter::AllDataLabelItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    ::std::auto_ptr< awt::Size > pRefSize )
        : MultipleItemConverter( rItemPool )
{
    ::std::vector< uno::Reference< chart2::XDataSeries > > aSeriesList(
        ::chart::ChartModelHelper::getDataSeries( xChartModel ));

    ::std::vector< uno::Reference< chart2::XDataSeries > >::const_iterator aIt;
    for( aIt = aSeriesList.begin(); aIt != aSeriesList.end(); ++aIt )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties( *aIt, uno::UNO_QUERY);
        if( pRefSize.get())
            m_aConverters.push_back( new ::chart::wrapper::DataPointItemConverter(
                                         xChartModel,
                                         xObjectProperties, rItemPool, rDrawModel, NULL,
                                         GraphicPropertyItemConverter::FILLED_DATA_POINT,
                                         ::std::auto_ptr< awt::Size >( new awt::Size( *pRefSize )) ));
        else
            m_aConverters.push_back( new ::chart::wrapper::DataPointItemConverter(
                                         xChartModel, xObjectProperties, rItemPool, rDrawModel, NULL ));
    }
}

AllDataLabelItemConverter::~AllDataLabelItemConverter()
{
}

const USHORT * AllDataLabelItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nDataLabelWhichPairs;
}

//-----------------------------------------------------------------------------

AllTitleItemConverter::AllTitleItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    ::std::auto_ptr< awt::Size > pRefSize )
        : MultipleItemConverter( rItemPool )
{
    for(sal_Int32 nTitle = TitleHelper::TITLE_BEGIN; nTitle < TitleHelper::TITLE_END; nTitle++ )
    {
        uno::Reference< chart2::XTitle > xTitle( TitleHelper::getTitle( TitleHelper::eTitleType(nTitle), xChartModel ) );
        if(!xTitle.is())
            return;
        uno::Reference< beans::XPropertySet > xObjectProperties( xTitle, uno::UNO_QUERY);
        ::std::auto_ptr< awt::Size > pSingleRefSize(0);
        if( pRefSize.get())
            pSingleRefSize = ::std::auto_ptr< awt::Size >( new awt::Size( *pRefSize ));
        m_aConverters.push_back( new ::chart::wrapper::TitleItemConverter(
                                        xObjectProperties, rItemPool, rDrawModel, pSingleRefSize ));
    }
}

AllTitleItemConverter::~AllTitleItemConverter()
{
}

const USHORT * AllTitleItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nTitleWhichPairs;
}

//-----------------------------------------------------------------------------

AllSeriesStatisticsConverter::AllSeriesStatisticsConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool )
        : MultipleItemConverter( rItemPool )
{
    ::std::vector< uno::Reference< chart2::XDataSeries > > aSeriesList(
        ::chart::ChartModelHelper::getDataSeries( xChartModel ));

    ::std::vector< uno::Reference< chart2::XDataSeries > >::const_iterator aIt;
    for( aIt = aSeriesList.begin(); aIt != aSeriesList.end(); ++aIt )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties( *aIt, uno::UNO_QUERY);
        m_aConverters.push_back( new ::chart::wrapper::StatisticsItemConverter(
                                     xChartModel, xObjectProperties, rItemPool ));
    }
}

AllSeriesStatisticsConverter::~AllSeriesStatisticsConverter()
{}

const USHORT * AllSeriesStatisticsConverter::GetWhichPairs() const
{
    // must span all used items!
    return nStatWhichPairs;
}

} //  namespace wrapper
} //  namespace chart
