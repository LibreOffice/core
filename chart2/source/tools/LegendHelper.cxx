/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LegendHelper.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:00:28 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

// static
uno::Reference< chart2::XLegend > LegendHelper::getLegend(
      const uno::Reference< frame::XModel >& xModel
    , const uno::Reference< uno::XComponentContext >& xContext
    , bool bCreate )
{
    uno::Reference< chart2::XLegend > xResult;

    uno::Reference< chart2::XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
    if( xChartDoc.is())
    {
        try
        {
            uno::Reference< chart2::XDiagram > xDia( xChartDoc->getFirstDiagram());
            if( xDia.is() )
            {
                xResult.set( xDia->getLegend() );
                if( bCreate && !xResult.is() && xContext.is() )
                {
                    xResult.set( xContext->getServiceManager()->createInstanceWithContext(
                        C2U( "com.sun.star.chart2.Legend" ), xContext ), uno::UNO_QUERY );
                    xDia->setLegend( xResult );
                }
            }
            else if(bCreate)
            {
                DBG_ERROR("need diagram for creation of legend");
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return xResult;
}

// static
bool LegendHelper::hasLegend( const uno::Reference< chart2::XDiagram > & xDiagram )
{
    bool bReturn = false;
    if( xDiagram.is())
    {
        uno::Reference< beans::XPropertySet > xLegendProp( xDiagram->getLegend(), uno::UNO_QUERY );
        if( xLegendProp.is())
            xLegendProp->getPropertyValue( C2U("Show")) >>= bReturn;
    }

    return bReturn;
}

//.............................................................................
} //namespace chart
//.............................................................................

