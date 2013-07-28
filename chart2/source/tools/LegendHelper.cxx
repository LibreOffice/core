/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "LegendHelper.hxx"
#include "macros.hxx"
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace chart
{

Reference< chart2::XLegend > LegendHelper::showLegend( const Reference< frame::XModel >& xModel
                                                    , const uno::Reference< uno::XComponentContext >& xContext )
{
    uno::Reference< chart2::XLegend > xLegend = LegendHelper::getLegend( xModel, xContext, true );
    uno::Reference< beans::XPropertySet > xProp( xLegend, uno::UNO_QUERY );
    if( xProp.is())
    {
        xProp->setPropertyValue( "Show", uno::makeAny(sal_True) );

        chart2::RelativePosition aRelativePosition;
        if( !(xProp->getPropertyValue( "RelativePosition") >>=  aRelativePosition) )
        {
            chart2::LegendPosition ePos = chart2::LegendPosition_LINE_END;
            if( !(xProp->getPropertyValue( "AnchorPosition") >>= ePos ) )
                xProp->setPropertyValue( "AnchorPosition", uno::makeAny( ePos ));

            ::com::sun::star::chart::ChartLegendExpansion eExpansion =
                    ( ePos == chart2::LegendPosition_LINE_END ||
                      ePos == chart2::LegendPosition_LINE_START )
                    ? ::com::sun::star::chart::ChartLegendExpansion_HIGH
                    : ::com::sun::star::chart::ChartLegendExpansion_WIDE;
            if( !(xProp->getPropertyValue( "Expansion") >>= eExpansion ) )
                xProp->setPropertyValue( "Expansion", uno::makeAny( eExpansion ));

            xProp->setPropertyValue( "RelativePosition", uno::Any());
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
        xProp->setPropertyValue( "Show", uno::makeAny(sal_False) );
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
                        "com.sun.star.chart2.Legend", xContext ), uno::UNO_QUERY );
                    xDia->setLegend( xResult );
                }
            }
            else if(bCreate)
            {
                OSL_FAIL("need diagram for creation of legend");
            }
        }
        catch( const uno::Exception & ex )
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
            xLegendProp->getPropertyValue( "Show") >>= bReturn;
    }

    return bReturn;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
