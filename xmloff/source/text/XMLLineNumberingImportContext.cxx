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

#include "XMLLineNumberingImportContext.hxx"
#include "XMLLineNumberingSeparatorImportContext.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/style/LineNumberPosition.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/token/tokens.hxx>
#include <xmloff/xmlnumi.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace com::sun::star::xml::sax;
using namespace ::xmloff::token;
using namespace xmloff;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::text::XLineNumberingProperties;

TYPEINIT1( XMLLineNumberingImportContext, SvXMLStyleContext );


XMLLineNumberingImportContext::XMLLineNumberingImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLocalName, xAttrList, XML_STYLE_FAMILY_TEXT_LINENUMBERINGCONFIG)
,   sCharStyleName("CharStyleName")
,   sCountEmptyLines("CountEmptyLines")
,   sCountLinesInFrames("CountLinesInFrames")
,   sDistance("Distance")
,   sInterval("Interval")
,   sSeparatorText("SeparatorText")
,   sNumberPosition("NumberPosition")
,   sNumberingType("NumberingType")
,   sIsOn("IsOn")
,   sRestartAtEachPage("RestartAtEachPage")
,   sSeparatorInterval("SeparatorInterval")
,   sNumFormat(GetXMLToken(XML_1))
,   sNumLetterSync(GetXMLToken(XML_FALSE))
,   nOffset(-1)
,   nNumberPosition(style::LineNumberPosition::LEFT)
,   nIncrement(-1)
,   nSeparatorIncrement(-1)
,   bNumberLines(true)
,   bCountEmptyLines(true)
,   bCountInFloatingFrames(false)
,   bRestartNumbering(false)
{
}

XMLLineNumberingImportContext::XMLLineNumberingImportContext(
    SvXMLImport& rImport,
    sal_Int32 Element,
    const Reference< xml::sax::XFastAttributeList > & xAttrList)
:   SvXMLStyleContext(rImport, Element, xAttrList, XML_STYLE_FAMILY_TEXT_LINENUMBERINGCONFIG)
,   sCharStyleName("CharStyleName")
,   sCountEmptyLines("CountEmptyLines")
,   sCountLinesInFrames("CountLinesInFrames")
,   sDistance("Distance")
,   sInterval("Interval")
,   sSeparatorText("SeparatorText")
,   sNumberPosition("NumberPosition")
,   sNumberingType("NumberingType")
,   sIsOn("IsOn")
,   sRestartAtEachPage("RestartAtEachPage")
,   sSeparatorInterval("SeparatorInterval")
,   sNumFormat(GetXMLToken(XML_1))
,   sNumLetterSync(GetXMLToken(XML_FALSE))
,   nOffset(-1)
,   nNumberPosition(style::LineNumberPosition::LEFT)
,   nIncrement(-1)
,   nSeparatorIncrement(-1)
,   bNumberLines(true)
,   bCountEmptyLines(true)
,   bCountInFloatingFrames(false)
,   bRestartNumbering(false)
{
}

XMLLineNumberingImportContext::~XMLLineNumberingImportContext()
{
}

void XMLLineNumberingImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    static const SvXMLTokenMapEntry aLineNumberingTokenMap[] =
    {
        { XML_NAMESPACE_TEXT, XML_STYLE_NAME, XML_TOK_LINENUMBERING_STYLE_NAME,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_style_name) },
        { XML_NAMESPACE_TEXT, XML_NUMBER_LINES, XML_TOK_LINENUMBERING_NUMBER_LINES,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_number_lines) },
        { XML_NAMESPACE_TEXT, XML_COUNT_EMPTY_LINES, XML_TOK_LINENUMBERING_COUNT_EMPTY_LINES,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_count_empty_lines) },
        { XML_NAMESPACE_TEXT, XML_COUNT_IN_TEXT_BOXES, XML_TOK_LINENUMBERING_COUNT_IN_TEXT_BOXES,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_count_in_text_boxes) },
        { XML_NAMESPACE_TEXT, XML_RESTART_ON_PAGE, XML_TOK_LINENUMBERING_RESTART_NUMBERING,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_restart_on_page) },
        { XML_NAMESPACE_TEXT, XML_OFFSET, XML_TOK_LINENUMBERING_OFFSET,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_offset) },
        { XML_NAMESPACE_STYLE, XML_NUM_FORMAT, XML_TOK_LINENUMBERING_NUM_FORMAT,
            (FastToken::NAMESPACE | XML_NAMESPACE_STYLE | XML_num_format) },
        { XML_NAMESPACE_STYLE, XML_NUM_LETTER_SYNC, XML_TOK_LINENUMBERING_NUM_LETTER_SYNC,
            (FastToken::NAMESPACE | XML_NAMESPACE_STYLE | XML_num_letter_sync) },
        { XML_NAMESPACE_TEXT, XML_NUMBER_POSITION, XML_TOK_LINENUMBERING_NUMBER_POSITION,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_number_position) },
        { XML_NAMESPACE_TEXT, XML_INCREMENT, XML_TOK_LINENUMBERING_INCREMENT,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_increment) },
    //  { XML_NAMESPACE_TEXT, XML_LINENUMBERING_CONFIGURATION, XML_TOK_LINENUMBERING_LINENUMBERING_CONFIGURATION
    //      (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_linenumbering_configuration) },
    //  { XML_NAMESPACE_TEXT, XML_INCREMENT, XML_TOK_LINENUMBERING_INCREMENT,
    //      (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_increment) },
    //  { XML_NAMESPACE_TEXT, XML_LINENUMBERING_SEPARATOR, XML_TOK_LINENUMBERING_LINENUMBERING_SEPARATOR
    //      (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_linenumbering_separator) },
        XML_TOKEN_MAP_END
    };

    SvXMLTokenMap aTokenMap(aLineNumberingTokenMap);

    // process attributes
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 i=0; i<nLength; i++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        ProcessAttribute(
            (enum LineNumberingToken)aTokenMap.Get(nPrefix, sLocalName),
            xAttrList->getValueByIndex(i));
    }
}

void SAL_CALL XMLLineNumberingImportContext::startFastElement(
    sal_Int32 /*Element*/, const Reference< xml::sax::XFastAttributeList >& xAttrList )
    throw(uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    static const SvXMLTokenMapEntry aLineNumberingTokenMap[] =
    {
        { XML_NAMESPACE_TEXT, XML_STYLE_NAME, XML_TOK_LINENUMBERING_STYLE_NAME,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_style_name) },
        { XML_NAMESPACE_TEXT, XML_NUMBER_LINES, XML_TOK_LINENUMBERING_NUMBER_LINES,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_number_lines) },
        { XML_NAMESPACE_TEXT, XML_COUNT_EMPTY_LINES, XML_TOK_LINENUMBERING_COUNT_EMPTY_LINES,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_count_empty_lines) },
        { XML_NAMESPACE_TEXT, XML_COUNT_IN_TEXT_BOXES, XML_TOK_LINENUMBERING_COUNT_IN_TEXT_BOXES,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_count_in_text_boxes) },
        { XML_NAMESPACE_TEXT, XML_RESTART_ON_PAGE, XML_TOK_LINENUMBERING_RESTART_NUMBERING,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_restart_on_page) },
        { XML_NAMESPACE_TEXT, XML_OFFSET, XML_TOK_LINENUMBERING_OFFSET,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_offset) },
        { XML_NAMESPACE_STYLE, XML_NUM_FORMAT, XML_TOK_LINENUMBERING_NUM_FORMAT,
            (FastToken::NAMESPACE | XML_NAMESPACE_STYLE | XML_num_format) },
        { XML_NAMESPACE_STYLE, XML_NUM_LETTER_SYNC, XML_TOK_LINENUMBERING_NUM_LETTER_SYNC,
            (FastToken::NAMESPACE | XML_NAMESPACE_STYLE | XML_num_letter_sync) },
        { XML_NAMESPACE_TEXT, XML_NUMBER_POSITION, XML_TOK_LINENUMBERING_NUMBER_POSITION,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_number_position) },
        { XML_NAMESPACE_TEXT, XML_INCREMENT, XML_TOK_LINENUMBERING_INCREMENT,
            (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_increment) },
    //  { XML_NAMESPACE_TEXT, XML_LINENUMBERING_CONFIGURATION, XML_TOK_LINENUMBERING_LINENUMBERING_CONFIGURATION
    //      (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_linenumbering_configuration) },
    //  { XML_NAMESPACE_TEXT, XML_INCREMENT, XML_TOK_LINENUMBERING_INCREMENT,
    //      (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_increment) },
    //  { XML_NAMESPACE_TEXT, XML_LINENUMBERING_SEPARATOR, XML_TOK_LINENUMBERING_LINENUMBERING_SEPARATOR
    //      (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_linenumbering_separator) },
        XML_TOKEN_MAP_END
    };

    SvXMLTokenMap aTokenMap(aLineNumberingTokenMap);

    // process attributes
    Sequence< css::xml::FastAttribute > attributes = xAttrList->getFastAttributes();
    for(css::xml::FastAttribute* attr = attributes.begin();
            attr != attributes.end(); attr++ )
    {
        ProcessAttribute(
            (enum LineNumberingToken)aTokenMap.Get(attr->Token), attr->Value);
    }
}

void XMLLineNumberingImportContext::ProcessAttribute(
    enum LineNumberingToken eToken,
    const OUString& sValue)
{
    bool bTmp(false);
    sal_Int32 nTmp;

    switch (eToken)
    {
        case XML_TOK_LINENUMBERING_STYLE_NAME:
            sStyleName = sValue;
            break;

        case XML_TOK_LINENUMBERING_NUMBER_LINES:
            if (::sax::Converter::convertBool(bTmp, sValue))
            {
                bNumberLines = bTmp;
            }
            break;

        case XML_TOK_LINENUMBERING_COUNT_EMPTY_LINES:
            if (::sax::Converter::convertBool(bTmp, sValue))
            {
                bCountEmptyLines = bTmp;
            }
            break;

        case XML_TOK_LINENUMBERING_COUNT_IN_TEXT_BOXES:
            if (::sax::Converter::convertBool(bTmp, sValue))
            {
                bCountInFloatingFrames = bTmp;
            }
            break;

        case XML_TOK_LINENUMBERING_RESTART_NUMBERING:
            if (::sax::Converter::convertBool(bTmp, sValue))
            {
                bRestartNumbering = bTmp;
            }
            break;

        case XML_TOK_LINENUMBERING_OFFSET:
            if (GetImport().GetMM100UnitConverter().
                    convertMeasureToCore(nTmp, sValue))
            {
                nOffset = nTmp;
            }
            break;

        case XML_TOK_LINENUMBERING_NUM_FORMAT:
            sNumFormat = sValue;
            break;

        case XML_TOK_LINENUMBERING_NUM_LETTER_SYNC:
            sNumLetterSync = sValue;
            break;

        case XML_TOK_LINENUMBERING_NUMBER_POSITION:
        {
            static const SvXMLEnumMapEntry aLineNumberPositionMap[] =
            {
                { XML_LEFT,     style::LineNumberPosition::LEFT },
                { XML_RIGHT,    style::LineNumberPosition::RIGHT },
                { XML_INSIDE,   style::LineNumberPosition::INSIDE },
                { XML_OUTSIDE,  style::LineNumberPosition::OUTSIDE },
                { xmloff::token::XML_TOKEN_INVALID, 0 }
            };

            sal_uInt16 nTmp16;
            if (SvXMLUnitConverter::convertEnum(nTmp16, sValue,
                                                aLineNumberPositionMap))
            {
                nNumberPosition = nTmp16;
            }
            break;
        }

        case XML_TOK_LINENUMBERING_INCREMENT:
            if (::sax::Converter::convertNumber(nTmp, sValue, 0))
            {
                nIncrement = (sal_Int16)nTmp;
            }
            break;
    }
}

void XMLLineNumberingImportContext::CreateAndInsert(bool)
{
    // insert and block mode is handled in insertStyleFamily

    // we'll try to get the LineNumberingProperties
    Reference<XLineNumberingProperties> xSupplier(GetImport().GetModel(),
                                                  UNO_QUERY);
    if (xSupplier.is())
    {
        Reference<XPropertySet> xLineNumbering =
            xSupplier->getLineNumberingProperties();

        if (xLineNumbering.is())
        {
            Any aAny;

            // set style name (if it exists)
            if ( GetImport().GetStyles()->FindStyleChildContext(
                            XML_STYLE_FAMILY_TEXT_TEXT, sStyleName ) != NULL )
            {
                aAny <<= GetImport().GetStyleDisplayName(
                            XML_STYLE_FAMILY_TEXT_TEXT, sStyleName );
                xLineNumbering->setPropertyValue(sCharStyleName, aAny);
            }

            aAny <<= sSeparator;
            xLineNumbering->setPropertyValue(sSeparatorText, aAny);

            aAny <<= nOffset;
            xLineNumbering->setPropertyValue(sDistance, aAny);

            aAny <<= nNumberPosition;
            xLineNumbering->setPropertyValue(sNumberPosition, aAny);

            if (nIncrement >= 0)
            {
                aAny <<= nIncrement;
                xLineNumbering->setPropertyValue(sInterval, aAny);
            }

            if (nSeparatorIncrement >= 0)
            {
                aAny <<= nSeparatorIncrement;
                xLineNumbering->setPropertyValue(sSeparatorInterval, aAny);
            }

            aAny.setValue(&bNumberLines, cppu::UnoType<bool>::get());
            xLineNumbering->setPropertyValue(sIsOn, aAny);

            aAny.setValue(&bCountEmptyLines, cppu::UnoType<bool>::get());
            xLineNumbering->setPropertyValue(sCountEmptyLines, aAny);

            aAny.setValue(&bCountInFloatingFrames, cppu::UnoType<bool>::get());
            xLineNumbering->setPropertyValue(sCountLinesInFrames, aAny);

            aAny.setValue(&bRestartNumbering, cppu::UnoType<bool>::get());
            xLineNumbering->setPropertyValue(sRestartAtEachPage, aAny);

            sal_Int16 nNumType = NumberingType::ARABIC;
            GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumFormat,
                                                    sNumLetterSync );
            aAny <<= nNumType;
            xLineNumbering->setPropertyValue(sNumberingType, aAny);
        }
    }
}

SvXMLImportContext* XMLLineNumberingImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    if ( (nPrefix == XML_NAMESPACE_TEXT) &&
         IsXMLToken(rLocalName, XML_LINENUMBERING_SEPARATOR) )
    {
        return new XMLLineNumberingSeparatorImportContext(GetImport(),
                                                          nPrefix, rLocalName,
                                                          *this);
    }
    else
    {
        // unknown element: default context
        return SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                      xAttrList);
    }
}

Reference< xml::sax::XFastContextHandler > SAL_CALL
    XMLLineNumberingImportContext::createFastChildContext( sal_Int32 Element,
    const Reference< xml::sax::XFastAttributeList >& xAttrList )
    throw(uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    if( Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_linenumbering_separator) )
    {
        return new XMLLineNumberingSeparatorImportContext(
            GetImport(), Element, *this );
    }
    else
    {
        // unknown element: default context
        return SvXMLImportContext::createFastChildContext( Element, xAttrList );
    }
}

void XMLLineNumberingImportContext::SetSeparatorText(
    const OUString& sText)
{
    sSeparator = sText;
}

void XMLLineNumberingImportContext::SetSeparatorIncrement(
    sal_Int16 nIncr)
{
    nSeparatorIncrement = nIncr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
