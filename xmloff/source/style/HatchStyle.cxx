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

#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <xmloff/xmlement.hxx>

using namespace ::com::sun::star;

using namespace ::xmloff::token;

SvXMLEnumMapEntry<drawing::HatchStyle> const pXML_HatchStyle_Enum[] =
{
    { XML_SINGLE,               drawing::HatchStyle_SINGLE },
    { XML_DOUBLE,               drawing::HatchStyle_DOUBLE },
    { XML_HATCHSTYLE_TRIPLE,    drawing::HatchStyle_TRIPLE },
    { XML_TOKEN_INVALID, drawing::HatchStyle(0) }
};

// Import

XMLHatchStyleImport::XMLHatchStyleImport( SvXMLImport& rImp )
    : m_rImport(rImp)
{
}

void XMLHatchStyleImport::importXML(
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
    uno::Any& rValue,
    OUString& rStrName )
{
    OUString aDisplayName;

    drawing::Hatch aHatch;
    aHatch.Style = drawing::HatchStyle_SINGLE;
    aHatch.Color = 0;
    aHatch.Distance = 0;
    aHatch.Angle = 0;

    SvXMLUnitConverter& rUnitConverter = m_rImport.GetMM100UnitConverter();

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch( aIter.getToken() )
        {
            case XML_ELEMENT(DRAW, XML_NAME):
            case XML_ELEMENT(DRAW_OOO, XML_NAME):
                rStrName = aIter.toString();
                break;
            case XML_ELEMENT(DRAW, XML_DISPLAY_NAME):
            case XML_ELEMENT(DRAW_OOO, XML_DISPLAY_NAME):
                aDisplayName = aIter.toString();
                break;
            case XML_ELEMENT(DRAW, XML_STYLE):
            case XML_ELEMENT(DRAW_OOO, XML_STYLE):
                SvXMLUnitConverter::convertEnum( aHatch.Style, aIter.toView(), pXML_HatchStyle_Enum );
                break;
            case XML_ELEMENT(DRAW, XML_COLOR):
            case XML_ELEMENT(DRAW_OOO, XML_COLOR):
                ::sax::Converter::convertColor(aHatch.Color, aIter.toView());
                break;
            case XML_ELEMENT(DRAW, XML_DISTANCE):
            case XML_ELEMENT(DRAW_OOO, XML_DISTANCE):
                rUnitConverter.convertMeasureToCore(aHatch.Distance, aIter.toView());
                break;
            case XML_ELEMENT(DRAW, XML_ROTATION):
            case XML_ELEMENT(DRAW_OOO, XML_ROTATION):
            {
                sal_Int32 nValue;
                if (::sax::Converter::convertNumber(nValue, aIter.toView(), 0, 3600))
                    aHatch.Angle = sal_Int16(nValue);
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff.style", aIter);
        }
    }

    rValue <<= aHatch;

    if( !aDisplayName.isEmpty() )
    {
        m_rImport.AddStyleDisplayName( XmlStyleFamily::SD_HATCH_ID, rStrName,
                                     aDisplayName );
        rStrName = aDisplayName;
    }
}

// Export

XMLHatchStyleExport::XMLHatchStyleExport( SvXMLExport& rExp )
    : m_rExport(rExp)
{
}

void XMLHatchStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    drawing::Hatch aHatch;

    if( rStrName.isEmpty() )
        return;

    if( !(rValue >>= aHatch) )
        return;

    OUString aStrValue;
    OUStringBuffer aOut;

    SvXMLUnitConverter& rUnitConverter =
        m_rExport.GetMM100UnitConverter();

    // Style
    if( !SvXMLUnitConverter::convertEnum( aOut, aHatch.Style, pXML_HatchStyle_Enum ) )
        return;

    // Name
    bool bEncoded = false;
    m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                          m_rExport.EncodeStyleName( rStrName,
                                                    &bEncoded ) );
    if( bEncoded )
        m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME,
                                rStrName );

    aStrValue = aOut.makeStringAndClear();
    m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE, aStrValue );

    // Color
    ::sax::Converter::convertColor(aOut, aHatch.Color);
    aStrValue = aOut.makeStringAndClear();
    m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_COLOR, aStrValue );

    // Distance
    rUnitConverter.convertMeasureToXML( aOut, aHatch.Distance );
    aStrValue = aOut.makeStringAndClear();
    m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISTANCE, aStrValue );

    // Angle
    m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_ROTATION, OUString::number(aHatch.Angle) );

    // Do Write
    SvXMLElementExport rElem( m_rExport, XML_NAMESPACE_DRAW, XML_HATCH,
                              true, false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
