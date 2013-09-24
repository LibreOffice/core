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

#include <com/sun/star/drawing/DashStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>

#include <sax/tools/converter.hxx>

#include "xmloff/DashStyle.hxx"
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <xmloff/xmltkmap.hxx>

using namespace ::com::sun::star;

using namespace ::xmloff::token;

enum SvXMLTokenMapAttrs
{
    XML_TOK_DASH_NAME,
    XML_TOK_DASH_DISPLAY_NAME,
    XML_TOK_DASH_STYLE,
    XML_TOK_DASH_DOTS1,
    XML_TOK_DASH_DOTS1LEN,
    XML_TOK_DASH_DOTS2,
    XML_TOK_DASH_DOTS2LEN,
    XML_TOK_DASH_DISTANCE,
    XML_TOK_DASH_END=XML_TOK_UNKNOWN
};

static SvXMLTokenMapEntry aDashStyleAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, XML_NAME,             XML_TOK_DASH_NAME },
    { XML_NAMESPACE_DRAW, XML_DISPLAY_NAME,     XML_TOK_DASH_DISPLAY_NAME },
    { XML_NAMESPACE_DRAW, XML_STYLE,            XML_TOK_DASH_STYLE },
    { XML_NAMESPACE_DRAW, XML_DOTS1,            XML_TOK_DASH_DOTS1 },
    { XML_NAMESPACE_DRAW, XML_DOTS1_LENGTH,     XML_TOK_DASH_DOTS1LEN },
    { XML_NAMESPACE_DRAW, XML_DOTS2,            XML_TOK_DASH_DOTS2 },
    { XML_NAMESPACE_DRAW, XML_DOTS2_LENGTH,     XML_TOK_DASH_DOTS2LEN },
    { XML_NAMESPACE_DRAW, XML_DISTANCE,         XML_TOK_DASH_DISTANCE },
    XML_TOKEN_MAP_END
};

SvXMLEnumMapEntry const pXML_DashStyle_Enum[] =
{
    { XML_RECT,         drawing::DashStyle_RECT },
    { XML_ROUND,        drawing::DashStyle_ROUND },
    { XML_RECT,         drawing::DashStyle_RECTRELATIVE },
    { XML_ROUND,        drawing::DashStyle_ROUNDRELATIVE },
    { XML_TOKEN_INVALID, 0 }
};

// Import

XMLDashStyleImport::XMLDashStyleImport( SvXMLImport& rImp )
    : rImport(rImp)
{
}

XMLDashStyleImport::~XMLDashStyleImport()
{
}

sal_Bool XMLDashStyleImport::importXML(
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    uno::Any& rValue,
    OUString& rStrName )
{
    drawing::LineDash aLineDash;
    aLineDash.Style = drawing::DashStyle_RECT;
    aLineDash.Dots = 0;
    aLineDash.DotLen = 0;
    aLineDash.Dashes = 0;
    aLineDash.DashLen = 0;
    aLineDash.Distance = 20;
    OUString aDisplayName;

    bool bIsRel = false;

    SvXMLNamespaceMap& rNamespaceMap = rImport.GetNamespaceMap();
    SvXMLUnitConverter& rUnitConverter = rImport.GetMM100UnitConverter();

    SvXMLTokenMap aTokenMap( aDashStyleAttrTokenMap );

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aStrAttrName;
        sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrName( rFullAttrName, &aStrAttrName );
        const OUString& rStrValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aStrAttrName ) )
        {
        case XML_TOK_DASH_NAME:
            {
                rStrName = rStrValue;
            }
            break;
        case XML_TOK_DASH_DISPLAY_NAME:
            {
                aDisplayName = rStrValue;
            }
            break;
        case XML_TOK_DASH_STYLE:
            {
                sal_uInt16 eValue;
                if( rUnitConverter.convertEnum( eValue, rStrValue, pXML_DashStyle_Enum ) )
                {
                    aLineDash.Style = (drawing::DashStyle) eValue;
                }
            }
            break;
        case XML_TOK_DASH_DOTS1:
            aLineDash.Dots = (sal_Int16)rStrValue.toInt32();
            break;

        case XML_TOK_DASH_DOTS1LEN:
            {
                if( rStrValue.indexOf( sal_Unicode('%') ) != -1 ) // its a percentage
                {
                    bIsRel = true;
                    ::sax::Converter::convertPercent(aLineDash.DotLen, rStrValue);
                }
                else
                {
                    rUnitConverter.convertMeasureToCore( aLineDash.DotLen,
                            rStrValue );
                }
            }
            break;

        case XML_TOK_DASH_DOTS2:
            aLineDash.Dashes = (sal_Int16)rStrValue.toInt32();
            break;

        case XML_TOK_DASH_DOTS2LEN:
            {
                if( rStrValue.indexOf( sal_Unicode('%') ) != -1 ) // its a percentage
                {
                    bIsRel = true;
                    ::sax::Converter::convertPercent(aLineDash.DashLen, rStrValue);
                }
                else
                {
                    rUnitConverter.convertMeasureToCore( aLineDash.DashLen,
                            rStrValue );
                }
            }
            break;

        case XML_TOK_DASH_DISTANCE:
            {
                if( rStrValue.indexOf( sal_Unicode('%') ) != -1 ) // its a percentage
                {
                    bIsRel = true;
                    ::sax::Converter::convertPercent(aLineDash.Distance, rStrValue);
                }
                else
                {
                    rUnitConverter.convertMeasureToCore( aLineDash.Distance,
                            rStrValue );
                }
            }
            break;
        default:
            DBG_WARNING( "Unknown token at import gradient style" );
        }
    }

    if( bIsRel )
        aLineDash.Style = aLineDash.Style == drawing::DashStyle_RECT ? drawing::DashStyle_RECTRELATIVE : drawing::DashStyle_ROUNDRELATIVE;

    rValue <<= aLineDash;

    if( !aDisplayName.isEmpty() )
    {
        rImport.AddStyleDisplayName( XML_STYLE_FAMILY_SD_STROKE_DASH_ID,
                                     rStrName, aDisplayName );
        rStrName = aDisplayName;
    }

    return sal_True;
}

// Export

XMLDashStyleExport::XMLDashStyleExport( SvXMLExport& rExp )
    : rExport(rExp)
{
}

XMLDashStyleExport::~XMLDashStyleExport()
{
}

sal_Bool XMLDashStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    sal_Bool bRet = sal_False;

    SvXMLUnitConverter & rUnitConverter = rExport.GetMM100UnitConverter();

    drawing::LineDash aLineDash;

    if( !rStrName.isEmpty() )
    {
        if( rValue >>= aLineDash )
        {
            bool bIsRel = aLineDash.Style == drawing::DashStyle_RECTRELATIVE || aLineDash.Style == drawing::DashStyle_ROUNDRELATIVE;

            OUString aStrValue;
            OUStringBuffer aOut;

            // Name
            sal_Bool bEncoded = sal_False;
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                                  rExport.EncodeStyleName( rStrName,
                                                           &bEncoded ) );
            if( bEncoded )
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME,
                                      rStrName );

            // Style
            rUnitConverter.convertEnum( aOut, aLineDash.Style, pXML_DashStyle_Enum );
            aStrValue = aOut.makeStringAndClear();
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE, aStrValue );

            // dots
            if( aLineDash.Dots )
            {
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DOTS1, OUString::number( aLineDash.Dots ) );

                if( aLineDash.DotLen )
                {
                    // dashes length
                    if( bIsRel )
                    {
                        ::sax::Converter::convertPercent(aOut, aLineDash.DotLen);
                    }
                    else
                    {
                        rUnitConverter.convertMeasureToXML( aOut,
                                aLineDash.DotLen );
                    }
                    aStrValue = aOut.makeStringAndClear();
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DOTS1_LENGTH, aStrValue );
                }
            }

            // dashes
            if( aLineDash.Dashes )
            {
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DOTS2, OUString::number( aLineDash.Dashes ) );

                if( aLineDash.DashLen )
                {
                    // dashes length
                    if( bIsRel )
                    {
                        ::sax::Converter::convertPercent(aOut, aLineDash.DashLen);
                    }
                    else
                    {
                        rUnitConverter.convertMeasureToXML( aOut,
                                aLineDash.DashLen );
                    }
                    aStrValue = aOut.makeStringAndClear();
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DOTS2_LENGTH, aStrValue );
                }
            }

            // distance
            if( bIsRel )
            {
                ::sax::Converter::convertPercent( aOut, aLineDash.Distance );
            }
            else
            {
                rUnitConverter.convertMeasureToXML( aOut,
                        aLineDash.Distance );
            }
            aStrValue = aOut.makeStringAndClear();
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISTANCE, aStrValue );

            // do Write
            SvXMLElementExport rElem( rExport,
                                      XML_NAMESPACE_DRAW, XML_STROKE_DASH,
                                      sal_True, sal_False );
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
