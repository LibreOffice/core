/*************************************************************************
 *
 *  $RCSfile: MultipleItemConverter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-07 15:39:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "MultipleItemConverter.hxx"
#include "ItemPropertyMap.hxx"
#include "SchSfxItemIds.hxx"
#include "SchWhichPairs.hxx"
#include "AxisItemConverter.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "DataPointItemConverter.hxx"
#include "ChartModelHelper.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XAXISCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XAxisContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XGRIDCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XGridContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <drafts/com/sun/star/chart2/XChartDocument.hpp>
#endif

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace chart
{
namespace wrapper
{
MultipleItemConverter::MultipleItemConverter( SfxItemPool& rItemPool )
        : ItemConverter( NULL, rItemPool )
{
}
MultipleItemConverter::~MultipleItemConverter()
{
    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     DeleteItemConverterPtr() );
}

void MultipleItemConverter::FillItemSet( SfxItemSet & rOutItemSet ) const
{
    ::std::vector< ItemConverter* >::const_iterator       aIter = m_aConverters.begin();
    const ::std::vector< ItemConverter* >::const_iterator aEnd  = m_aConverters.end();
    if( aIter != aEnd )
    {
        (*aIter)->FillItemSet( rOutItemSet );
        aIter++;
    }
    for( ; aIter != aEnd; aIter++ )
    {
        SfxItemSet aSet = this->CreateEmptyItemSet();
        (*aIter)->FillItemSet( aSet );
        InvalidateUnequalItems( rOutItemSet, aSet );
    }
    // no own items
}

bool MultipleItemConverter::ApplyItemSet( const SfxItemSet & rItemSet )
{
    bool bResult = false;

    ::std::for_each( m_aConverters.begin(), m_aConverters.end(),
                     ApplyItemSetFunc( rItemSet, bResult ));

    // no own items
    return bResult;
}

bool MultipleItemConverter::GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const
{
    return false;
}

//-----------------------------------------------------------------------------

AllAxisItemConverter::AllAxisItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel )
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
        m_aConverters.push_back( new wrapper::AxisItemConverter(
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
        m_aConverters.push_back( new wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rItemPool, rDrawModel,
                                        wrapper::GraphicPropertyItemConverter::LINE_PROPERTIES ) );
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
    SdrModel& rDrawModel  )
        : MultipleItemConverter( rItemPool )
{
    ::std::vector< uno::Reference< chart2::XDataSeries > > aSeriesList(
        ChartModelHelper::getDataSeries( xChartModel ));

    ::std::vector< uno::Reference< chart2::XDataSeries > >::const_iterator aIt;
    for( aIt = aSeriesList.begin(); aIt != aSeriesList.end(); ++aIt )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties( *aIt, uno::UNO_QUERY);
        m_aConverters.push_back( new wrapper::DataPointItemConverter(
                                        xObjectProperties, rItemPool, rDrawModel, NULL ) );
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

} //  namespace wrapper
} //  namespace chart
