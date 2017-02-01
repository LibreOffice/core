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

#include <xmloff/HatchStyle.hxx>

#include <com/sun/star/drawing/Hatch.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnmspe.hxx>
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
    XML_TOK_HATCH_NAME,
    XML_TOK_HATCH_DISPLAY_NAME,
    XML_TOK_HATCH_STYLE,
    XML_TOK_HATCH_COLOR,
    XML_TOK_HATCH_DISTANCE,
    XML_TOK_HATCH_ROTATION,
    XML_TOK_TABSTOP_END=XML_TOK_UNKNOWN
};

SvXMLEnumMapEntry const pXML_HatchStyle_Enum[] =
{
    { XML_HATCHSTYLE_SINGLE,    drawing::HatchStyle_SINGLE },
    { XML_HATCHSTYLE_DOUBLE,    drawing::HatchStyle_DOUBLE },
    { XML_HATCHSTYLE_TRIPLE,    drawing::HatchStyle_TRIPLE },
    { XML_TOKEN_INVALID, 0 }
};

// Import

XMLHatchStyleImport::XMLHatchStyleImport( SvXMLImport& rImp )
    : rImport(rImp)
{
}

XMLHatchStyleImport::~XMLHatchStyleImport()
{
}

void XMLHatchStyleImport::importXML(
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    uno::Any& rValue,
    OUString& rStrName )
{
    static const SvXMLTokenMapEntry aHatchAttrTokenMap[] =
    {
        { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_HATCH_NAME },
        { XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, XML_TOK_HATCH_DISPLAY_NAME },
        { XML_NAMESPACE_DRAW, XML_STYLE, XML_TOK_HATCH_STYLE },
        { XML_NAMESPACE_DRAW, XML_COLOR, XML_TOK_HATCH_COLOR },
        { XML_NAMESPACE_DRAW, XML_HATCH_DISTANCE, XML_TOK_HATCH_DISTANCE,
            NAMESPACE_TOKEN( XML_NAMESPACE_DRAW ) | XML_DISTANCE },
        //  XML_HATCH_DISTANCE is a duplicate of XML_DISTANCE
        { XML_NAMESPACE_DRAW, XML_ROTATION, XML_TOK_HATCH_ROTATION },
        XML_TOKEN_MAP_END
    };

    bool bHasStyle = false;
    OUString aDisplayName;

    drawing::Hatch aHatch;
    aHatch.Style = drawing::HatchStyle_SINGLE;
    aHatch.Color = 0;
    aHatch.Distance = 0;
    aHatch.Angle = 0;

    SvXMLTokenMap aTokenMap( aHatchAttrTokenMap );
    SvXMLNamespaceMap rNamespaceMap = rImport.GetNamespaceMap();
    SvXMLUnitConverter& rUnitConverter = rImport.GetMM100UnitConverter();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aStrAttrName;
        sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrName( rFullAttrName, &aStrAttrName );
        const OUString& rStrValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aStrAttrName ) )
        {
            case XML_TOK_HATCH_NAME:
                rStrName = rStrValue;
                break;
            case XML_TOK_HATCH_DISPLAY_NAME:
                aDisplayName = rStrValue;
                break;
            case XML_TOK_HATCH_STYLE:
                {
                    sal_uInt16 eValue;
                    bHasStyle = SvXMLUnitConverter::convertEnum( eValue, rStrValue, pXML_HatchStyle_Enum );
                    if( bHasStyle )
                        aHatch.Style = (drawing::HatchStyle) eValue;
                }
                break;
            case XML_TOK_HATCH_COLOR:
                ::sax::Converter::convertColor(aHatch.Color, rStrValue);
                break;
            case XML_TOK_HATCH_DISTANCE:
                rUnitConverter.convertMeasureToCore(
                        (sal_Int32&)aHatch.Distance, rStrValue );
                break;
            case XML_TOK_HATCH_ROTATION:
                {
                    sal_Int32 nValue;
                    ::sax::Converter::convertNumber(nValue, rStrValue, 0, 3600);
                    aHatch.Angle = sal_Int16( nValue );
                }
                break;

            default:
                SAL_INFO("xmloff.style", "Unknown token at import hatch style");
        }
    }

    rValue <<= aHatch;

    if( !aDisplayName.isEmpty() )
    {
        rImport.AddStyleDisplayName( XML_STYLE_FAMILY_SD_HATCH_ID, rStrName,
                                     aDisplayName );
        rStrName = aDisplayName;
    }
}

// Export

XMLHatchStyleExport::XMLHatchStyleExport( SvXMLExport& rExp )
    : rExport(rExp)
{
}

XMLHatchStyleExport::~XMLHatchStyleExport()
{
}

void XMLHatchStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    drawing::Hatch aHatch;

    if( !rStrName.isEmpty() )
    {
        if( rValue >>= aHatch )
        {
            OUString aStrValue;
            OUStringBuffer aOut;

            SvXMLUnitConverter& rUnitConverter =
                rExport.GetMM100UnitConverter();

            // Style
            if( SvXMLUnitConverter::convertEnum( aOut, aHatch.Style, pXML_HatchStyle_Enum ) )
            {
                // Name
                bool bEncoded = false;
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                                      rExport.EncodeStyleName( rStrName,
                                                                &bEncoded ) );
                if( bEncoded )
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME,
                                            rStrName );

                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE, aStrValue );

                // Color
                ::sax::Converter::convertColor(aOut, aHatch.Color);
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_COLOR, aStrValue );

                // Distance
                rUnitConverter.convertMeasureToXML( aOut, aHatch.Distance );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HATCH_DISTANCE, aStrValue );

                // Angle
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_ROTATION, OUString::number(aHatch.Angle) );

                // Do Write
                SvXMLElementExport rElem( rExport, XML_NAMESPACE_DRAW, XML_HATCH,
                                          true, false );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
