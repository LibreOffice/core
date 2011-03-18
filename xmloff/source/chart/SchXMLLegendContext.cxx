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
#include "precompiled_xmloff.hxx"
#include "SchXMLLegendContext.hxx"
#include "SchXMLEnumConverter.hxx"

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>

#include <tools/debug.hxx>

#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

using namespace ::xmloff::token;
using namespace com::sun::star;

using rtl::OUString;
using com::sun::star::uno::Reference;

//----------------------------------------

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

//----------------------------------------

SchXMLLegendContext::SchXMLLegendContext( SchXMLImportHelper& rImpHelper, SvXMLImport& rImport, const rtl::OUString& rLocalName ) :
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
            xDocProp->setPropertyValue( rtl::OUString::createFromAscii( "HasLegend" ), uno::makeAny( sal_True ) );
        }
        catch( beans::UnknownPropertyException )
        {
            OSL_TRACE( "Property HasLegend not found" );
        }
    }

    uno::Reference< drawing::XShape > xLegendShape( xDoc->getLegend(), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xLegendProps( xLegendShape, uno::UNO_QUERY );
    if( !xLegendShape.is() || !xLegendProps.is() )
    {
        OSL_TRACE( "legend could not be created" );
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

    rtl::OUString sAutoStyleName;
    uno::Any aAny;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_LEGEND_POSITION:
                {
                    try
                    {
                        if( SchXMLEnumConverter::getLegendPositionConverter().importXML( aValue, aAny, GetImport().GetMM100UnitConverter() ) )
                            xLegendProps->setPropertyValue( rtl::OUString::createFromAscii( "Alignment" ), aAny );
                    }
                    catch( beans::UnknownPropertyException )
                    {
                        OSL_TRACE( "Property Alignment (legend) not found" );
                    }
                }
                break;

            case XML_TOK_LEGEND_X:
                GetImport().GetMM100UnitConverter().convertMeasure( aLegendPos.X, aValue );
                bHasXPosition = true;
                break;
            case XML_TOK_LEGEND_Y:
                GetImport().GetMM100UnitConverter().convertMeasure( aLegendPos.Y, aValue );
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
                GetImport().GetMM100UnitConverter().convertMeasure( aLegendSize.Width, aValue );
                bHasWidth = true;
                break;
            case XML_TOK_LEGEND_HEIGHT:
            case XML_TOK_LEGEND_HEIGHT_EXT:
                GetImport().GetMM100UnitConverter().convertMeasure( aLegendSize.Height, aValue );
                bHasHeight = true;
                break;
            default:
                break;
        }
    }

    if( bHasXPosition && bHasYPosition )
        xLegendShape->setPosition( aLegendPos );

    if( bHasExpansion && nLegendExpansion!= chart::ChartLegendExpansion_CUSTOM )
        xLegendProps->setPropertyValue( rtl::OUString::createFromAscii( "Expansion" ), uno::makeAny(nLegendExpansion) );
    else if( bHasHeight && bHasWidth )
        xLegendShape->setSize( aLegendSize );

    // the fill style has the default "none" in XML, but "solid" in the model.
    xLegendProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" )), uno::makeAny( drawing::FillStyle_NONE ));

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
