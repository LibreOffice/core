/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LegendHelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:29:57 $
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
#include "LegendHelper.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XLEGEND_HPP_
#include <com/sun/star/chart2/XLegend.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

// static
rtl::OUString LegendHelper::getIdentifierForLegend()
{
    static rtl::OUString aIdentifier( C2U( "@legend" ) );
    return aIdentifier;
}

// static
uno::Reference< chart2::XLegend > LegendHelper::getLegend(
    const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< chart2::XLegend > xResult;

    uno::Reference< chart2::XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
    if( xChartDoc.is())
    {
        try
        {
            uno::Reference< chart2::XDiagram > xDia( xChartDoc->getDiagram());
            if( xDia.is())
                xResult.set( xDia->getLegend() );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }

    }

    return xResult;
}

// static
void LegendHelper::defaultFillEmptyLegend(
    const uno::Reference< chart2::XLegend > & xLegend,
    const uno::Reference< chart2::XDiagram > & xDiagram )
{
    if( xLegend.is() &&
        xDiagram.is() )
    {
        try
        {
            uno::Reference< chart2::XDataSeriesTreeParent > xRoot( xDiagram->getTree());

            uno::Sequence< uno::Reference< chart2::XDataSeriesTreeNode > > aChildren( xRoot->getChildren());
            for( sal_Int32 i = 0; i < aChildren.getLength(); ++i )
            {
                uno::Reference< lang::XServiceInfo > xInfo( aChildren[ i ], uno::UNO_QUERY );
                if( xInfo.is() &&
                    xInfo->supportsService( C2U( "com.sun.star.chart2.ChartTypeGroup" )))
                {
                    uno::Reference< chart2::XLegendEntry > xEntry( xInfo, uno::UNO_QUERY );
                    if( xEntry.is())
                        xLegend->registerEntry( xEntry );
                }
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

// static
void LegendHelper::flushLegend( const uno::Reference< chart2::XLegend > & xLegend )
{
    if( xLegend.is())
    {
        uno::Sequence< uno::Reference< chart2::XLegendEntry > > aEntries( xLegend->getEntries());
        for( sal_Int32 i = 0; i < aEntries.getLength(); ++i )
        {
            xLegend->revokeEntry( aEntries[ i ] );
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................

