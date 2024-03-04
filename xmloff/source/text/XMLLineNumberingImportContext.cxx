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

#include <XMLLineNumberingImportContext.hxx>
#include "XMLLineNumberingSeparatorImportContext.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/style/LineNumberPosition.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlement.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::text::XLineNumberingProperties;


constexpr OUStringLiteral gsCharStyleName(u"CharStyleName");
constexpr OUStringLiteral gsCountEmptyLines(u"CountEmptyLines");
constexpr OUStringLiteral gsCountLinesInFrames(u"CountLinesInFrames");
constexpr OUStringLiteral gsDistance(u"Distance");
constexpr OUStringLiteral gsInterval(u"Interval");
constexpr OUStringLiteral gsSeparatorText(u"SeparatorText");
constexpr OUStringLiteral gsNumberPosition(u"NumberPosition");
constexpr OUStringLiteral gsNumberingType(u"NumberingType");
constexpr OUStringLiteral gsIsOn(u"IsOn");
constexpr OUStringLiteral gsRestartAtEachPage(u"RestartAtEachPage");
constexpr OUStringLiteral gsSeparatorInterval(u"SeparatorInterval");

XMLLineNumberingImportContext::XMLLineNumberingImportContext(
    SvXMLImport& rImport)
:   SvXMLStyleContext(rImport, XmlStyleFamily::TEXT_LINENUMBERINGCONFIG)
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

void XMLLineNumberingImportContext::SetAttribute( sal_Int32 nElement,
                               const OUString& rValue )
{
    bool bTmp(false);
    sal_Int32 nTmp;

    switch (nElement)
    {
        case XML_ELEMENT(TEXT, XML_STYLE_NAME):
            sStyleName = rValue;
            break;

        case XML_ELEMENT(TEXT, XML_NUMBER_LINES):
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bNumberLines = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_COUNT_EMPTY_LINES):
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bCountEmptyLines = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_COUNT_IN_TEXT_BOXES):
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bCountInFloatingFrames = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_RESTART_ON_PAGE):
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bRestartNumbering = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_OFFSET):
            if (GetImport().GetMM100UnitConverter().
                    convertMeasureToCore(nTmp, rValue))
            {
                nOffset = nTmp;
            }
            break;

        case XML_ELEMENT(STYLE, XML_NUM_FORMAT):
            sNumFormat = rValue;
            break;

        case XML_ELEMENT(STYLE, XML_NUM_LETTER_SYNC):
            sNumLetterSync = rValue;
            break;

        case XML_ELEMENT(TEXT, XML_NUMBER_POSITION):
        {
            static const SvXMLEnumMapEntry<sal_Int16> aLineNumberPositionMap[] =
            {
                { XML_LEFT,     style::LineNumberPosition::LEFT },
                { XML_RIGHT,    style::LineNumberPosition::RIGHT },
                { XML_INSIDE,   style::LineNumberPosition::INSIDE },
                { XML_OUTSIDE,  style::LineNumberPosition::OUTSIDE },
                { XML_TOKEN_INVALID, 0 }
            };

            (void)SvXMLUnitConverter::convertEnum(nNumberPosition, rValue,
                                                  aLineNumberPositionMap);
            break;
        }

        case XML_ELEMENT(TEXT, XML_INCREMENT):
            if (::sax::Converter::convertNumber(nTmp, rValue, 0))
            {
                nIncrement = static_cast<sal_Int16>(nTmp);
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
    if (!xSupplier.is())
        return;

    Reference<XPropertySet> xLineNumbering =
        xSupplier->getLineNumberingProperties();

    if (!xLineNumbering.is())
        return;

    Any aAny;

    // set style name (if it exists)
    if ( GetImport().GetStyles()->FindStyleChildContext(
                    XmlStyleFamily::TEXT_TEXT, sStyleName ) != nullptr )
    {
        aAny <<= GetImport().GetStyleDisplayName(
                    XmlStyleFamily::TEXT_TEXT, sStyleName );
        xLineNumbering->setPropertyValue(gsCharStyleName, aAny);
    }

    xLineNumbering->setPropertyValue(gsSeparatorText, Any(sSeparator));
    xLineNumbering->setPropertyValue(gsDistance, Any(nOffset));
    xLineNumbering->setPropertyValue(gsNumberPosition, Any(nNumberPosition));

    if (nIncrement >= 0)
    {
        xLineNumbering->setPropertyValue(gsInterval, Any(nIncrement));
    }

    if (nSeparatorIncrement >= 0)
    {
        xLineNumbering->setPropertyValue(gsSeparatorInterval, Any(nSeparatorIncrement));
    }

    xLineNumbering->setPropertyValue(gsIsOn, Any(bNumberLines));
    xLineNumbering->setPropertyValue(gsCountEmptyLines, Any(bCountEmptyLines));
    xLineNumbering->setPropertyValue(gsCountLinesInFrames, Any(bCountInFloatingFrames));
    xLineNumbering->setPropertyValue(gsRestartAtEachPage, Any(bRestartNumbering));

    sal_Int16 nNumType = NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                            sNumFormat,
                                            sNumLetterSync );
    xLineNumbering->setPropertyValue(gsNumberingType, Any(nNumType));
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLLineNumberingImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if ( nElement == XML_ELEMENT(TEXT, XML_LINENUMBERING_SEPARATOR) )
        return new XMLLineNumberingSeparatorImportContext(GetImport(), *this);
    return nullptr;
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
