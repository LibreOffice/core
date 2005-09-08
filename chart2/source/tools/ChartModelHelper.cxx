/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartModelHelper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:26:59 $
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
#include "ChartModelHelper.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPEGROUP_HPP_
#include <com/sun/star/chart2/XChartTypeGroup.hpp>
#endif

// header for define DBG_ASSERT
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//static
uno::Reference< XDiagram > ChartModelHelper::findDiagram( const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
    if( xChartDoc.is())
        return xChartDoc->getDiagram();
    return NULL;
}

//static
uno::Reference< XChartType > ChartModelHelper::getFirstChartType( const uno::Reference< XDiagram >& xDiagram )
{
    if(!xDiagram.is())
        return 0;
    uno::Reference< XDataSeriesTreeParent > xTree = xDiagram->getTree();
    if(!xTree.is())
        return 0;
    uno::Sequence< uno::Reference< XDataSeriesTreeNode > >  aChartTypes( xTree->getChildren() );
    for( sal_Int32 i = 0; i < aChartTypes.getLength(); ++i )
    {
        uno::Reference< XChartTypeGroup > xChartTypeGroup( aChartTypes[i], uno::UNO_QUERY );
        DBG_ASSERT(xChartTypeGroup.is(),"First node at the diagram tree needs to be a ChartTypeGroup");
        if( xChartTypeGroup.is() )
        {
            uno::Reference< XChartType > xChartType = xChartTypeGroup->getChartType();
            if( xChartType.is() )
                return xChartType;
        }
    }
    return 0;
}

namespace
{
void lcl_addSeries( uno::Reference< XDataSeriesTreeParent > xParent,
                    ::std::vector< uno::Reference< XDataSeries > > & rOutSeriesVec )
{
    if( xParent.is())
    {
        uno::Sequence< uno::Reference< XDataSeriesTreeNode > > aChildren( xParent->getChildren());
        for( sal_Int32 i = 0; i < aChildren.getLength(); ++i )
        {
            uno::Reference< XDataSeries > aDataSeries( aChildren[ i ], uno::UNO_QUERY );
            if( aDataSeries.is())
            {
                rOutSeriesVec.push_back( aDataSeries );
            }
            else
            {
                uno::Reference< XDataSeriesTreeParent > xNewParent( aChildren[ i ], uno::UNO_QUERY );
                if( xNewParent.is())
                    lcl_addSeries( xNewParent, rOutSeriesVec );
            }
        }
    }
}
} // anonymous namespace

// static
::std::vector< uno::Reference< XDataSeries > > ChartModelHelper::getDataSeries(
    const uno::Reference< frame::XModel > & xModel )
{
    ::std::vector< uno::Reference< XDataSeries > > aResult;
    uno::Reference< XChartDocument > xDocument( xModel, uno::UNO_QUERY );
    if( xDocument.is())
    {
        uno::Reference< XDiagram > xDia( xDocument->getDiagram());
        if( xDia.is())
            lcl_addSeries( xDia->getTree(), aResult );
    }

    return aResult;
}

//static
uno::Reference< XDataSeries > ChartModelHelper::getSeriesByIdentifier(
        const rtl::OUString& rIdentifier
        , const uno::Reference< frame::XModel > xModel )
{
    if(!rIdentifier.getLength())
        return NULL;

    ::std::vector< uno::Reference< XDataSeries > > aSeriesList(
        getDataSeries( xModel ));
    uno::Reference< XDataSeries > xRet;
    uno::Reference< beans::XPropertySet > xProp;
    rtl::OUString aIdentifier;

    ::std::vector< uno::Reference< XDataSeries > >::const_iterator aIt;
    for( aIt = aSeriesList.begin(); aIt != aSeriesList.end(); ++aIt )
    {
        xProp = uno::Reference< beans::XPropertySet >( *aIt, uno::UNO_QUERY );
        if(!xProp.is())
            continue;
        uno::Any aAIdentifier = xProp->getPropertyValue( C2U( "Identifier" ) );
        aAIdentifier >>= aIdentifier;
        if(aIdentifier.getLength() && rIdentifier.equals(aIdentifier))
        {
            xRet = *aIt;
            break;
        }
    }
    return xRet;
}

uno::Reference< XChartType > ChartModelHelper::getChartTypeOfSeries(
                                const uno::Reference< frame::XModel >& xModel
                              , const uno::Reference< XDataSeries >&   xGivenDataSeries )
{
    uno::Reference< XChartType > xRet(NULL);

    //iterate through the nmodel to find the given xSeries in the tree
    //the found parent indicates the charttype
    if( !xGivenDataSeries.is() )
        return xRet;
    uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram( xModel );
    if(!xDiagram.is())
        return xRet;
    uno::Reference< XDataSeriesTreeParent > xTree = xDiagram->getTree();
    if(!xTree.is())
        return xRet;
    uno::Sequence< uno::Reference< XDataSeriesTreeNode > >  aChartTypes( xTree->getChildren() );
    for( sal_Int32 i = 0; i < aChartTypes.getLength(); ++i )
    {
        uno::Reference< XChartTypeGroup > xChartTypeGroup( aChartTypes[i], uno::UNO_QUERY );
        DBG_ASSERT(xChartTypeGroup.is(),"First node at the diagram tree needs to be a ChartTypeGroup");
        if( !xChartTypeGroup.is() )
            continue;
        uno::Sequence< uno::Reference< XDataSeriesTreeNode > > aXSlots( xChartTypeGroup->getChildren() );
        for( sal_Int32 nX = 0; nX < aXSlots.getLength(); ++nX )
        {
            uno::Reference< XDataSeriesTreeParent > xXSlot = uno::Reference< XDataSeriesTreeParent >::query( aXSlots[nX] );
            DBG_ASSERT( xXSlot.is(), "a node for the first dimension of a chart tree should always be a parent" );
            if(!xXSlot.is())
                continue;
            uno::Sequence< uno::Reference< XDataSeriesTreeNode > > aYSlots( xXSlot->getChildren() );
            for( sal_Int32 nY = 0; nY < aYSlots.getLength(); ++nY )
            {
                uno::Reference< XDataSeriesTreeParent > xYSlot = uno::Reference< XDataSeriesTreeParent >::query( aYSlots[nY] );
                DBG_ASSERT( xYSlot.is(), "a node for the second dimension of a chart tree should always be a parent" );
                if(!xYSlot.is())
                    continue;
                uno::Sequence< uno::Reference< XDataSeriesTreeNode > > aSeriesList( xYSlot->getChildren() );
                for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
                {
                    uno::Reference< XDataSeries > xDataSeries( aSeriesList[nS], uno::UNO_QUERY );
                    if( xGivenDataSeries==xDataSeries )
                    {
                        xRet = xChartTypeGroup->getChartType();
                        return xRet;
                    }
                }
            }
        }
    }
    return xRet;
}

//.............................................................................
} //namespace chart
//.............................................................................
