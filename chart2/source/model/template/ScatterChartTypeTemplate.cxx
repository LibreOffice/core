/*************************************************************************
 *
 *  $RCSfile: ScatterChartTypeTemplate.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-20 17:02:46 $
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
#include "ScatterChartTypeTemplate.hxx"
#include "ScatterChartType.hxx"
#include "macros.hxx"
#include "DataSeriesTreeHelper.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_SYMBOLSTYLE_HPP_
#include <drafts/com/sun/star/chart2/SymbolStyle.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_SYMBOLPROPERTIES_HPP_
#include <drafts/com/sun/star/chart2/SymbolProperties.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace chart
{

ScatterChartTypeTemplate::ScatterChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    const ::rtl::OUString & rServiceName,
    chart2::CurveStyle eCurveStyle,
    bool bSymbols,
    sal_Int32 nDim /* = 2 */ ) :
        ChartTypeTemplate( xContext, rServiceName ),
        m_eCurveStyle( eCurveStyle ),
        m_bHasSymbols( bSymbols ),
        m_nDim( nDim )
{}

ScatterChartTypeTemplate::~ScatterChartTypeTemplate()
{}

sal_Int32 ScatterChartTypeTemplate::getDimension() const
{
    return m_nDim;
}

uno::Reference< chart2::XChartType > ScatterChartTypeTemplate::getDefaultChartType()
    throw (uno::RuntimeException)
{
    return new ScatterChartType( m_nDim, m_eCurveStyle );
}

// ____ XChartTypeTemplate ____
uno::Reference< chart2::XDiagram > SAL_CALL
    ScatterChartTypeTemplate::createDiagram(
        const uno::Sequence< uno::Reference< chart2::XDataSeries > >& aSeriesSeq )
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
            chart2::SymbolProperties aSymbProp;
            uno::Reference< beans::XPropertySet > xProp( aSeriesSeq[i], uno::UNO_QUERY_THROW );
            if( (xProp->getPropertyValue( C2U( "SymbolProperties" )) >>= aSymbProp ) )
            {
                aSymbProp.aStyle = eStyle;
                if( m_bHasSymbols )
                    aSymbProp.nStandardSymbol = i;
                xProp->setPropertyValue( C2U( "SymbolProperties" ), uno::makeAny( aSymbProp ));
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

sal_Bool SAL_CALL ScatterChartTypeTemplate::matchesTemplate(
    const uno::Reference< chart2::XDiagram >& xDiagram )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram );

    // check curve-style
    if( bResult )
    {
        try
        {
            uno::Reference< beans::XPropertySet > xCTProp(
                helper::DataSeriesTreeHelper::getChartTypeByIndex( xDiagram->getTree(), 0 ),
                uno::UNO_QUERY_THROW );
            chart2::CurveStyle eCurveStyle;
            if( xCTProp->getPropertyValue( C2U( "CurveStyle" )) >>= eCurveStyle )
            {
                bResult = ( eCurveStyle == m_eCurveStyle );
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    // check symbol-style
    if( bResult )
    {
        uno::Sequence< uno::Reference< chart2::XDataSeries > > aSeriesSeq(
            helper::DataSeriesTreeHelper::getDataSeriesFromDiagram( xDiagram ));
        chart2::SymbolStyle eStyle = m_bHasSymbols
            ? chart2::SymbolStyle_STANDARD
            : chart2::SymbolStyle_NONE;

        for( sal_Int32 i = 0; i < aSeriesSeq.getLength(); ++i )
        {
            try
            {
                chart2::SymbolProperties aSymbProp;
                uno::Reference< beans::XPropertySet > xProp( aSeriesSeq[i], uno::UNO_QUERY_THROW );
                if( (xProp->getPropertyValue( C2U( "SymbolProperties" )) >>= aSymbProp ) )
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

} //  namespace chart
