/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NetChartTypeTemplate.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:22:28 $
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
#include "NetChartTypeTemplate.hxx"
#include "NetChartType.hxx"
#include "macros.hxx"
#include "algohelper.hxx"
#include "DataSeriesTreeHelper.hxx"
#include "PolarCoordinateSystem.hxx"
#include "BoundedCoordinateSystem.hxx"
#include "Scaling.hxx"
#include "Scale.hxx"

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
    chart2::StackMode eStackMode,
    bool bSymbols ) :
        ChartTypeTemplate( xContext, rServiceName ),
        m_eStackMode( eStackMode ),
        m_bHasSymbols( bSymbols )
{}

NetChartTypeTemplate::~NetChartTypeTemplate()
{}

chart2::StackMode NetChartTypeTemplate::getYStackMode() const
{
    return m_eStackMode;
}

Reference< chart2::XBoundedCoordinateSystem > NetChartTypeTemplate::createCoordinateSystem(
    const Reference< chart2::XBoundedCoordinateSystemContainer > & xCoordSysCnt )
{
    Reference< chart2::XCoordinateSystem > xCoordSys(
        new PolarCoordinateSystem( getDimension() ));
    Reference< chart2::XBoundedCoordinateSystem > xResult(
        new BoundedCoordinateSystem( xCoordSys ));

    chart2::ScaleData aScale;
    aScale.Scaling = new LinearScaling( 1.0, 0.0 );

    aScale.Orientation = chart2::AxisOrientation_REVERSE;
    xResult->setScaleByDimension(
        0, Reference< chart2::XScale >( new Scale( GetComponentContext(), aScale ) ));
    aScale.Orientation = chart2::AxisOrientation_MATHEMATICAL;
    xResult->setScaleByDimension(
        1, Reference< chart2::XScale >( new Scale( GetComponentContext(), aScale ) ));

    try
    {
        if( xCoordSys.is())
            xCoordSysCnt->addCoordinateSystem( xResult );
    }
    catch( lang::IllegalArgumentException ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< chart2::XChartType > NetChartTypeTemplate::getDefaultChartType()
    throw (uno::RuntimeException)
{
    return new NetChartType();
}

// ____ XChartTypeTemplate ____
Reference< chart2::XDiagram > SAL_CALL
    NetChartTypeTemplate::createDiagram(
        const uno::Sequence< Reference< chart2::XDataSeries > >& aSeriesSeq )
    throw (uno::RuntimeException)
{
    // set symbol type at data series
    chart2::SymbolStyle eStyle = m_bHasSymbols
        ? chart2::SymbolStyle_STANDARD
        : chart2::SymbolStyle_NONE;

    for( sal_Int32 i = 0; i < aSeriesSeq.getLength(); ++i )
    {
        try
        {
            chart2::Symbol aSymbProp;
            Reference< beans::XPropertySet > xProp( aSeriesSeq[i], uno::UNO_QUERY_THROW );
            if( (xProp->getPropertyValue( C2U( "Symbol" )) >>= aSymbProp ) )
            {
                aSymbProp.aStyle = eStyle;
                if( m_bHasSymbols )
                    aSymbProp.nStandardSymbol = i;
                xProp->setPropertyValue( C2U( "Symbol" ), uno::makeAny( aSymbProp ));
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    // todo: set symbol type at data points

    return ChartTypeTemplate::createDiagram( aSeriesSeq );
}

sal_Bool SAL_CALL NetChartTypeTemplate::matchesTemplate(
    const Reference< chart2::XDiagram >& xDiagram )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram );

    // check symbol-style
    if( bResult )
    {
        uno::Sequence< Reference< chart2::XDataSeries > > aSeriesSeq(
            helper::DataSeriesTreeHelper::getDataSeriesFromDiagram( xDiagram ));
        chart2::SymbolStyle eStyle = m_bHasSymbols
            ? chart2::SymbolStyle_STANDARD
            : chart2::SymbolStyle_NONE;

        for( sal_Int32 i = 0; i < aSeriesSeq.getLength(); ++i )
        {
            try
            {
                chart2::Symbol aSymbProp;
                Reference< beans::XPropertySet > xProp( aSeriesSeq[i], uno::UNO_QUERY_THROW );
                if( (xProp->getPropertyValue( C2U( "Symbol" )) >>= aSymbProp ) )
                {
                    if( aSymbProp.aStyle != eStyle )
                    {
                        bResult = false;
                        break;
                    }
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }

    return bResult;
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
