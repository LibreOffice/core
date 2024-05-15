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

#include "SchXMLLegendContext.hxx"
#include "SchXMLEnumConverter.hxx"

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>

#include <sal/log.hxx>

#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

using namespace ::xmloff::token;
using namespace com::sun::star;

SchXMLLegendContext::SchXMLLegendContext( SchXMLImportHelper& rImpHelper, SvXMLImport& rImport ) :
    SvXMLImportContext( rImport ),
    mrImportHelper( rImpHelper )
{
}

void SchXMLLegendContext::startFastElement( sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    uno::Reference< chart::XChartDocument > xDoc = mrImportHelper.GetChartDocument();
    if( !xDoc.is() )
        return;

    // turn on legend
    uno::Reference< beans::XPropertySet > xDocProp( xDoc, uno::UNO_QUERY );
    if( xDocProp.is() )
    {
        try
        {
            xDocProp->setPropertyValue(u"HasLegend"_ustr, uno::Any( true ) );
        }
        catch(const beans::UnknownPropertyException&)
        {
            SAL_INFO("xmloff.chart", "Property HasLegend not found" );
        }
    }

    uno::Reference< drawing::XShape > xLegendShape = xDoc->getLegend();
    uno::Reference< beans::XPropertySet > xLegendProps( xLegendShape, uno::UNO_QUERY );
    if( !xLegendShape.is() || !xLegendProps.is() )
    {
        SAL_INFO("xmloff.chart", "legend could not be created" );
        return;
    }

    // parse attributes
    awt::Point aLegendPos;
    bool bOverlay = false;
    bool bHasXPosition=false;
    bool bHasYPosition=false;
    awt::Size aLegendSize;
    bool bHasWidth=false;
    bool bHasHeight=false;
    chart::ChartLegendExpansion nLegendExpansion = chart::ChartLegendExpansion_HIGH;
    bool bHasExpansion=false;

    OUString sAutoStyleName;
    uno::Any aAny;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(CHART, XML_LEGEND_POSITION):
                try
                {
                    if( SchXMLEnumConverter::getLegendPositionConverter().importXML( aIter.toString(), aAny, GetImport().GetMM100UnitConverter() ) )
                        xLegendProps->setPropertyValue(u"Alignment"_ustr, aAny );
                }
                catch(const beans::UnknownPropertyException&)
                {
                    SAL_INFO("xmloff.chart", "Property Alignment (legend) not found" );
                }
                break;
            case  XML_ELEMENT(LO_EXT, XML_OVERLAY):
                try
                {
                    bOverlay = aIter.toBoolean();
                    xLegendProps->setPropertyValue(u"Overlay"_ustr, uno::Any(bOverlay));
                }
                catch(const beans::UnknownPropertyException&)
                {
                    SAL_INFO("xmloff.chart", "Property Overlay (legend) not found" );
                }
                break;
            case XML_ELEMENT(SVG, XML_X):
            case XML_ELEMENT(SVG_COMPAT, XML_X):
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aLegendPos.X, aIter.toView() );
                bHasXPosition = true;
                break;
            case XML_ELEMENT(SVG, XML_Y):
            case XML_ELEMENT(SVG_COMPAT, XML_Y):
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aLegendPos.Y, aIter.toView() );
                bHasYPosition = true;
                break;
            case XML_ELEMENT(CHART, XML_STYLE_NAME):
                sAutoStyleName = aIter.toString();
                break;
            case  XML_ELEMENT(STYLE, XML_LEGEND_EXPANSION):
                SchXMLEnumConverter::getLegendPositionConverter().importXML( aIter.toString(), aAny, GetImport().GetMM100UnitConverter() );
                bHasExpansion = (aAny>>=nLegendExpansion);
                break;
            case XML_ELEMENT(STYLE, XML_LEGEND_EXPANSION_ASPECT_RATIO):
                break;
            case XML_ELEMENT(SVG, XML_WIDTH):
            case XML_ELEMENT(SVG_COMPAT, XML_WIDTH):
            case XML_ELEMENT(CHART_EXT, XML_WIDTH):
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aLegendSize.Width, aIter.toView() );
                bHasWidth = true;
                break;
            case XML_ELEMENT(SVG, XML_HEIGHT):
            case XML_ELEMENT(SVG_COMPAT, XML_HEIGHT):
            case XML_ELEMENT(CHART_EXT, XML_HEIGHT):
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aLegendSize.Height, aIter.toView() );
                bHasHeight = true;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
                break;
        }
    }

    if( bHasExpansion && nLegendExpansion!= chart::ChartLegendExpansion_CUSTOM )
        xLegendProps->setPropertyValue(u"Expansion"_ustr, uno::Any(nLegendExpansion) );
    else if( bHasHeight && bHasWidth )
        xLegendShape->setSize( aLegendSize );

    if( bHasXPosition && bHasYPosition )
        xLegendShape->setPosition( aLegendPos );

    // the fill style has the default "none" in XML, but "solid" in the model.
    xLegendProps->setPropertyValue(u"FillStyle"_ustr, uno::Any( drawing::FillStyle_NONE ));

    // set auto-styles for Legend
    mrImportHelper.FillAutoStyle(sAutoStyleName, xLegendProps);
}

SchXMLLegendContext::~SchXMLLegendContext()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
