/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NetChartTypeTemplate.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 12:02:19 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "NetChartTypeTemplate.hxx"
#include "macros.hxx"
#include "PolarCoordinateSystem.hxx"
#include "Scaling.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "DataSeriesHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_SYMBOLSTYLE_HPP_
#include <com/sun/star/chart2/SymbolStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_SYMBOL_HPP_
#include <com/sun/star/chart2/Symbol.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;


namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.NetChartTypeTemplate" ));
} // anonymous namespace

namespace chart
{

NetChartTypeTemplate::NetChartTypeTemplate(
    Reference< uno::XComponentContext > const & xContext,
    const ::rtl::OUString & rServiceName,
    StackMode eStackMode,
    bool bSymbols,
    bool bHasLines ) :
        ChartTypeTemplate( xContext, rServiceName ),
        m_eStackMode( eStackMode ),
        m_bHasSymbols( bSymbols ),
        m_bHasLines( bHasLines )
{}

NetChartTypeTemplate::~NetChartTypeTemplate()
{}

StackMode NetChartTypeTemplate::getStackMode( sal_Int32 /* nChartTypeIndex */ ) const
{
    return m_eStackMode;
}

void SAL_CALL NetChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
    throw (uno::RuntimeException)
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );

    try
    {
        Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY_THROW );

        DataSeriesHelper::switchSymbolsOnOrOff( xProp, m_bHasSymbols, nSeriesIndex );
        DataSeriesHelper::switchLinesOnOrOff( xProp, m_bHasLines );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL NetChartTypeTemplate::matchesTemplate(
    const Reference< chart2::XDiagram >& xDiagram,
    sal_Bool bAdaptProperties )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram, bAdaptProperties );

    // check symbol-style
    // for a template with symbols it is ok, if there is at least one series
    // with symbols, otherwise an unknown template is too easy to achieve
    bool bSymbolsFound = false;
    if( bResult )
    {
        ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

        for( ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aIt =
                 aSeriesVec.begin(); aIt != aSeriesVec.end(); ++aIt )
        {
            try
            {
                chart2::Symbol aSymbProp;
                drawing::LineStyle eLineStyle;
                Reference< beans::XPropertySet > xProp( *aIt, uno::UNO_QUERY_THROW );
                if( (xProp->getPropertyValue( C2U( "Symbol" )) >>= aSymbProp) &&
                    (aSymbProp.Style != chart2::SymbolStyle_NONE ) &&
                    (! m_bHasSymbols) )
                {
                    bResult = false;
                    break;
                }
                if( m_bHasSymbols )
                    bSymbolsFound = bSymbolsFound || (aSymbProp.Style != chart2::SymbolStyle_NONE);

                if( (xProp->getPropertyValue( C2U( "LineStyle" )) >>= eLineStyle) &&
                    (m_bHasLines != ( eLineStyle != drawing::LineStyle_NONE )) )
                {
                    bResult = false;
                    break;
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }

        if( m_bHasSymbols )
            bResult = bResult && bSymbolsFound;
    }

    return bResult;
}

Reference< chart2::XChartType > NetChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_NET ), uno::UNO_QUERY_THROW );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< chart2::XChartType > SAL_CALL NetChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
    throw (uno::RuntimeException)
{
    Reference< chart2::XChartType > xResult( getChartTypeForIndex( 0 ) );
    ChartTypeTemplate::copyPropertiesFromOldToNewCoordianteSystem( aFormerlyUsedChartTypes, xResult );
    return xResult;
}

// ----------------------------------------

Sequence< OUString > NetChartTypeTemplate::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartTypeTemplate" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( NetChartTypeTemplate, lcl_aServiceName );

} //  namespace chart
