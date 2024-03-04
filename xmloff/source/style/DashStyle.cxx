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

#include <xmloff/DashStyle.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlement.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

SvXMLEnumMapEntry<drawing::DashStyle> const pXML_DashStyle_Enum[] =
{
    { XML_RECT,         drawing::DashStyle_RECT },
    { XML_ROUND,        drawing::DashStyle_ROUND },
    { XML_RECT,         drawing::DashStyle_RECTRELATIVE },
    { XML_ROUND,        drawing::DashStyle_ROUNDRELATIVE },
    { XML_TOKEN_INVALID, drawing::DashStyle(0) }
};

// Import

XMLDashStyleImport::XMLDashStyleImport( SvXMLImport& rImp )
    : m_rImport(rImp)
{
}

void XMLDashStyleImport::importXML(
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
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

    SvXMLUnitConverter& rUnitConverter = m_rImport.GetMM100UnitConverter();

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(DRAW, XML_NAME):
        case XML_ELEMENT(DRAW_OOO, XML_NAME):
            {
                rStrName = aIter.toString();
            }
            break;
        case XML_ELEMENT(DRAW, XML_DISPLAY_NAME):
        case XML_ELEMENT(DRAW_OOO, XML_DISPLAY_NAME):
            {
                aDisplayName = aIter.toString();
            }
            break;
        case XML_ELEMENT(DRAW, XML_STYLE):
        case XML_ELEMENT(DRAW_OOO, XML_STYLE):
            {
                SvXMLUnitConverter::convertEnum( aLineDash.Style, aIter.toView(), pXML_DashStyle_Enum );
            }
            break;
        case XML_ELEMENT(DRAW, XML_DOTS1):
        case XML_ELEMENT(DRAW_OOO, XML_DOTS1):
            aLineDash.Dots = static_cast<sal_Int16>(aIter.toInt32());
            break;

        case XML_ELEMENT(DRAW, XML_DOTS1_LENGTH):
        case XML_ELEMENT(DRAW_OOO, XML_DOTS1_LENGTH):
            {
                if( aIter.toView().find( '%' ) != std::string_view::npos ) // it's a percentage
                {
                    bIsRel = true;
                    ::sax::Converter::convertPercent(aLineDash.DotLen, aIter.toView());
                }
                else
                {
                    rUnitConverter.convertMeasureToCore( aLineDash.DotLen,
                            aIter.toView() );
                }
            }
            break;

        case XML_ELEMENT(DRAW, XML_DOTS2):
        case XML_ELEMENT(DRAW_OOO, XML_DOTS2):
            aLineDash.Dashes = static_cast<sal_Int16>(aIter.toInt32());
            break;

        case XML_ELEMENT(DRAW, XML_DOTS2_LENGTH):
        case XML_ELEMENT(DRAW_OOO, XML_DOTS2_LENGTH):
            {
                if( aIter.toView().find( '%' ) != std::string_view::npos ) // it's a percentage
                {
                    bIsRel = true;
                    ::sax::Converter::convertPercent(aLineDash.DashLen, aIter.toView());
                }
                else
                {
                    rUnitConverter.convertMeasureToCore( aLineDash.DashLen,
                            aIter.toView() );
                }
            }
            break;

        case XML_ELEMENT(DRAW, XML_DISTANCE):
        case XML_ELEMENT(DRAW_OOO, XML_DISTANCE):
            {
                if( aIter.toView().find( '%' ) != std::string_view::npos ) // it's a percentage
                {
                    bIsRel = true;
                    ::sax::Converter::convertPercent(aLineDash.Distance, aIter.toView());
                }
                else
                {
                    rUnitConverter.convertMeasureToCore( aLineDash.Distance,
                            aIter.toView() );
                }
            }
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff.style", aIter);
        }
    }

    if( bIsRel )
        aLineDash.Style = aLineDash.Style == drawing::DashStyle_RECT ? drawing::DashStyle_RECTRELATIVE : drawing::DashStyle_ROUNDRELATIVE;

    rValue <<= aLineDash;

    if( !aDisplayName.isEmpty() )
    {
        m_rImport.AddStyleDisplayName( XmlStyleFamily::SD_STROKE_DASH_ID,
                                     rStrName, aDisplayName );
        rStrName = aDisplayName;
    }
}

// Export

XMLDashStyleExport::XMLDashStyleExport( SvXMLExport& rExp )
    : m_rExport(rExp)
{
}

void XMLDashStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    SvXMLUnitConverter & rUnitConverter = m_rExport.GetMM100UnitConverter();

    drawing::LineDash aLineDash;

    if( rStrName.isEmpty() )
        return;

    if( !(rValue >>= aLineDash) )
        return;

    bool bIsRel = aLineDash.Style == drawing::DashStyle_RECTRELATIVE || aLineDash.Style == drawing::DashStyle_ROUNDRELATIVE;

    OUString aStrValue;
    OUStringBuffer aOut;

    // Name
    bool bEncoded = false;
    m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                          m_rExport.EncodeStyleName( rStrName,
                                                   &bEncoded ) );
    if( bEncoded )
        m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME,
                              rStrName );

    // Style
    SvXMLUnitConverter::convertEnum( aOut, aLineDash.Style, pXML_DashStyle_Enum );
    aStrValue = aOut.makeStringAndClear();
    m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE, aStrValue );

    // dots
    if( aLineDash.Dots )
    {
        m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DOTS1, OUString::number( aLineDash.Dots ) );

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
            m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DOTS1_LENGTH, aStrValue );
        }
    }

    // dashes
    if( aLineDash.Dashes )
    {
        m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DOTS2, OUString::number( aLineDash.Dashes ) );

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
            m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DOTS2_LENGTH, aStrValue );
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
    m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISTANCE, aStrValue );

    // do Write
    SvXMLElementExport rElem( m_rExport,
                              XML_NAMESPACE_DRAW, XML_STROKE_DASH,
                              true, false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
