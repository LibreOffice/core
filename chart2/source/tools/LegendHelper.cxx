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

#include <LegendHelper.hxx>
#include <Legend.hxx>
#include <ChartModel.hxx>
#include <Diagram.hxx>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace chart
{

rtl::Reference< Legend > LegendHelper::showLegend( ChartModel& rModel
                                                    , const uno::Reference< uno::XComponentContext >& xContext )
{
    rtl::Reference< Legend > xLegend = LegendHelper::getLegend( rModel, xContext, true );
    if( xLegend.is())
    {
        xLegend->setPropertyValue( u"Show"_ustr, uno::Any(true) );

        chart2::RelativePosition aRelativePosition;
        if( !(xLegend->getPropertyValue( u"RelativePosition"_ustr) >>=  aRelativePosition) )
        {
            chart2::LegendPosition ePos = chart2::LegendPosition_LINE_END;
            if( !(xLegend->getPropertyValue( u"AnchorPosition"_ustr) >>= ePos ) )
                xLegend->setPropertyValue( u"AnchorPosition"_ustr, uno::Any( ePos ));

            css::chart::ChartLegendExpansion eExpansion =
                    ( ePos == chart2::LegendPosition_LINE_END ||
                      ePos == chart2::LegendPosition_LINE_START )
                    ? css::chart::ChartLegendExpansion_HIGH
                    : css::chart::ChartLegendExpansion_WIDE;
            if( !(xLegend->getPropertyValue( u"Expansion"_ustr) >>= eExpansion ) )
                xLegend->setPropertyValue( u"Expansion"_ustr, uno::Any( eExpansion ));

            xLegend->setPropertyValue( u"RelativePosition"_ustr, uno::Any());
        }

    }
    return xLegend;
}

void LegendHelper::hideLegend( ChartModel& rModel )
{
    rtl::Reference< Legend > xLegend = LegendHelper::getLegend( rModel, nullptr );
    if( xLegend.is())
    {
        xLegend->setPropertyValue( u"Show"_ustr, uno::Any(false) );
    }
}

rtl::Reference< Legend > LegendHelper::getLegend(
      ChartModel& rModel
    , const uno::Reference< uno::XComponentContext >& xContext
    , bool bCreate )
{
    rtl::Reference< Legend > xResult;

    try
    {
        rtl::Reference< Diagram > xDia( rModel.getFirstChartDiagram());
        if( xDia.is() )
        {
            xResult = xDia->getLegend2();
            if( bCreate && !xResult.is() && xContext.is() )
            {
                xResult = new Legend();
                xDia->setLegend( xResult );
            }
        }
        else if(bCreate)
        {
            OSL_FAIL("need diagram for creation of legend");
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

bool LegendHelper::hasLegend( const rtl::Reference< Diagram > & xDiagram )
{
    bool bReturn = false;
    if( xDiagram.is())
    {
        uno::Reference< beans::XPropertySet > xLegendProp( xDiagram->getLegend(), uno::UNO_QUERY );
        if( xLegendProp.is())
            xLegendProp->getPropertyValue( u"Show"_ustr) >>= bReturn;
    }

    return bReturn;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
