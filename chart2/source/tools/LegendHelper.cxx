/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "LegendHelper.hxx"
#include "macros.hxx"
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <tools/debug.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

//.............................................................................
namespace chart
{
//.............................................................................


Reference< chart2::XLegend > LegendHelper::showLegend( const Reference< frame::XModel >& xModel
                                                    , const uno::Reference< uno::XComponentContext >& xContext )
{
    uno::Reference< chart2::XLegend > xLegend = LegendHelper::getLegend( xModel, xContext, true );
    uno::Reference< beans::XPropertySet > xProp( xLegend, uno::UNO_QUERY );
    if( xProp.is())
    {
        xProp->setPropertyValue( C2U("Show"), uno::makeAny(sal_True) );

        chart2::RelativePosition aRelativePosition;
        if( !(xProp->getPropertyValue( C2U( "RelativePosition" )) >>=  aRelativePosition) )
        {
            chart2::LegendPosition ePos = chart2::LegendPosition_LINE_END;
            if( !(xProp->getPropertyValue( C2U( "AnchorPosition" )) >>= ePos ) )
                xProp->setPropertyValue( C2U( "AnchorPosition" ), uno::makeAny( ePos ));

            ::com::sun::star::chart::ChartLegendExpansion eExpansion =
                    ( ePos == chart2::LegendPosition_LINE_END ||
                      ePos == chart2::LegendPosition_LINE_START )
                    ? ::com::sun::star::chart::ChartLegendExpansion_HIGH
                    : ::com::sun::star::chart::ChartLegendExpansion_WIDE;
            if( !(xProp->getPropertyValue( C2U( "Expansion" )) >>= eExpansion ) )
                xProp->setPropertyValue( C2U( "Expansion" ), uno::makeAny( eExpansion ));

            xProp->setPropertyValue( C2U( "RelativePosition" ), uno::Any());
        }

    }
    return xLegend;
}

void LegendHelper::hideLegend( const Reference< frame::XModel >& xModel )
{
    uno::Reference< chart2::XLegend > xLegend = LegendHelper::getLegend( xModel, 0, false );
    uno::Reference< beans::XPropertySet > xProp( xLegend, uno::UNO_QUERY );
    if( xProp.is())
    {
        xProp->setPropertyValue( C2U("Show"), uno::makeAny(sal_False) );
    }
}

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
                OSL_FAIL("need diagram for creation of legend");
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return xResult;
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
