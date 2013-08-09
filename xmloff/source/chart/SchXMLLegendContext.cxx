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

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>

#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

using namespace ::xmloff::token;
using namespace com::sun::star;

using com::sun::star::uno::Reference;

namespace
{

enum LegendAttributeTokens
{
    XML_TOK_LEGEND_POSITION,
    XML_TOK_LEGEND_X,
    XML_TOK_LEGEND_Y,
    XML_TOK_LEGEND_STYLE_NAME,
    XML_TOK_LEGEND_EXPANSION,
    XML_TOK_LEGEND_EXPANSION_ASPECT_RATIO,
    XML_TOK_LEGEND_WIDTH,
    XML_TOK_LEGEND_WIDTH_EXT,
    XML_TOK_LEGEND_HEIGHT,
    XML_TOK_LEGEND_HEIGHT_EXT
};

SvXMLTokenMapEntry aLegendAttributeTokenMap[] =
{
    { XML_NAMESPACE_CHART,      XML_LEGEND_POSITION,    XML_TOK_LEGEND_POSITION     },
    { XML_NAMESPACE_SVG,        XML_X,                  XML_TOK_LEGEND_X            },
    { XML_NAMESPACE_SVG,        XML_Y,                  XML_TOK_LEGEND_Y            },
    { XML_NAMESPACE_CHART,      XML_STYLE_NAME,         XML_TOK_LEGEND_STYLE_NAME   },
    { XML_NAMESPACE_STYLE,      XML_LEGEND_EXPANSION,   XML_TOK_LEGEND_EXPANSION    },
    { XML_NAMESPACE_STYLE,      XML_LEGEND_EXPANSION_ASPECT_RATIO,   XML_TOK_LEGEND_EXPANSION_ASPECT_RATIO    },
    { XML_NAMESPACE_SVG,        XML_WIDTH,              XML_TOK_LEGEND_WIDTH        },
    { XML_NAMESPACE_CHART_EXT,  XML_WIDTH,              XML_TOK_LEGEND_WIDTH_EXT    },
    { XML_NAMESPACE_SVG,        XML_HEIGHT,             XML_TOK_LEGEND_HEIGHT       },
    { XML_NAMESPACE_CHART_EXT,  XML_HEIGHT,             XML_TOK_LEGEND_HEIGHT_EXT   },
    XML_TOKEN_MAP_END
};

class LegendAttributeTokenMap : public SvXMLTokenMap
{
public:
    LegendAttributeTokenMap(): SvXMLTokenMap( aLegendAttributeTokenMap ) {}
    virtual ~LegendAttributeTokenMap() {}
};

//a LegendAttributeTokenMap Singleton
struct theLegendAttributeTokenMap : public rtl::Static< LegendAttributeTokenMap, theLegendAttributeTokenMap > {};

}//end anonymous namespace

SchXMLLegendContext::SchXMLLegendContext( SchXMLImportHelper& rImpHelper, SvXMLImport& rImport, const OUString& rLocalName ) :
    SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
    mrImportHelper( rImpHelper )
{
}

void SchXMLLegendContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
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
            xDocProp->setPropertyValue("HasLegend", uno::makeAny( sal_True ) );
        }
        catch(const beans::UnknownPropertyException&)
        {
            SAL_INFO("xmloff.chart", "Property HasLegend not found" );
        }
    }

    uno::Reference< drawing::XShape > xLegendShape( xDoc->getLegend(), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xLegendProps( xLegendShape, uno::UNO_QUERY );
    if( !xLegendShape.is() || !xLegendProps.is() )
    {
        SAL_INFO("xmloff.chart", "legend could not be created" );
        return;
    }

    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    const SvXMLTokenMap& rAttrTokenMap = theLegendAttributeTokenMap::get();

    awt::Point aLegendPos;
    bool bHasXPosition=false;
    bool bHasYPosition=false;
    awt::Size aLegendSize;
    bool bHasWidth=false;
    bool bHasHeight=false;
    chart::ChartLegendExpansion nLegendExpansion = chart::ChartLegendExpansion_HIGH;
    bool bHasExpansion=false;

    OUString sAutoStyleName;
    uno::Any aAny;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_LEGEND_POSITION:
                {
                    try
                    {
                        if( SchXMLEnumConverter::getLegendPositionConverter().importXML( aValue, aAny, GetImport().GetMM100UnitConverter() ) )
                            xLegendProps->setPropertyValue("Alignment", aAny );
                    }
                    catch(const beans::UnknownPropertyException&)
                    {
                        SAL_INFO("xmloff.chart", "Property Alignment (legend) not found" );
                    }
                }
                break;

            case XML_TOK_LEGEND_X:
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aLegendPos.X, aValue );
                bHasXPosition = true;
                break;
            case XML_TOK_LEGEND_Y:
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aLegendPos.Y, aValue );
                bHasYPosition = true;
                break;
            case XML_TOK_LEGEND_STYLE_NAME:
                sAutoStyleName = aValue;
                break;
            case XML_TOK_LEGEND_EXPANSION:
                SchXMLEnumConverter::getLegendPositionConverter().importXML( aValue, aAny, GetImport().GetMM100UnitConverter() );
                bHasExpansion = (aAny>>=nLegendExpansion);
                break;
            case XML_TOK_LEGEND_EXPANSION_ASPECT_RATIO:
                break;
            case XML_TOK_LEGEND_WIDTH:
            case XML_TOK_LEGEND_WIDTH_EXT:
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aLegendSize.Width, aValue );
                bHasWidth = true;
                break;
            case XML_TOK_LEGEND_HEIGHT:
            case XML_TOK_LEGEND_HEIGHT_EXT:
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aLegendSize.Height, aValue );
                bHasHeight = true;
                break;
            default:
                break;
        }
    }

    if( bHasXPosition && bHasYPosition )
        xLegendShape->setPosition( aLegendPos );

    if( bHasExpansion && nLegendExpansion!= chart::ChartLegendExpansion_CUSTOM )
        xLegendProps->setPropertyValue("Expansion", uno::makeAny(nLegendExpansion) );
    else if( bHasHeight && bHasWidth )
        xLegendShape->setSize( aLegendSize );

    // the fill style has the default "none" in XML, but "solid" in the model.
    xLegendProps->setPropertyValue("FillStyle", uno::makeAny( drawing::FillStyle_NONE ));

    // set auto-styles for Legend
    const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
    if( pStylesCtxt )
    {
        const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
            mrImportHelper.GetChartFamilyID(), sAutoStyleName );

        if( pStyle && pStyle->ISA( XMLPropStyleContext ))
            (( XMLPropStyleContext* )pStyle )->FillPropertySet( xLegendProps );
    }
}

SchXMLLegendContext::~SchXMLLegendContext()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
