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


#include <XMLFootnoteConfigurationImportContext.hxx>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlement.hxx>

#include <xmloff/families.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/FootnoteNumbering.hpp>
#include <com/sun/star/style/NumberingType.hpp>


using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;


//  XMLFootnoteConfigHelper

namespace {

/// local helper class for import of quo-vadis and ergo-sum elements
class XMLFootnoteConfigHelper : public SvXMLImportContext
{
    OUStringBuffer sBuffer;
    XMLFootnoteConfigurationImportContext& rConfig;
    bool bIsBegin;

public:

    XMLFootnoteConfigHelper(
        SvXMLImport& rImport,
        XMLFootnoteConfigurationImportContext& rConfigImport,
        bool bBegin);

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual void SAL_CALL characters( const OUString& rChars ) override;
};

}

XMLFootnoteConfigHelper::XMLFootnoteConfigHelper(
    SvXMLImport& rImport,
    XMLFootnoteConfigurationImportContext& rConfigImport,
    bool bBegin)
:   SvXMLImportContext(rImport)
,   rConfig(rConfigImport)
,   bIsBegin(bBegin)
{
}

void XMLFootnoteConfigHelper::endFastElement(sal_Int32 )
{
    if (bIsBegin)
    {
        rConfig.SetBeginNotice(sBuffer.makeStringAndClear());
    }
    else
    {
        rConfig.SetEndNotice(sBuffer.makeStringAndClear());
    }
//  rConfig = NULL; // import contexts are ref-counted
}

void XMLFootnoteConfigHelper::characters( const OUString& rChars )
{
    sBuffer.append(rChars);
}


// XMLFootnoteConfigurationImportContext

constexpr OUStringLiteral gsPropertyAnchorCharStyleName(u"AnchorCharStyleName");
constexpr OUStringLiteral gsPropertyCharStyleName(u"CharStyleName");
constexpr OUStringLiteral gsPropertyNumberingType(u"NumberingType");
constexpr OUStringLiteral gsPropertyPageStyleName(u"PageStyleName");
constexpr OUStringLiteral gsPropertyParagraphStyleName(u"ParaStyleName");
constexpr OUStringLiteral gsPropertyPrefix(u"Prefix");
constexpr OUStringLiteral gsPropertyStartAt(u"StartAt");
constexpr OUStringLiteral gsPropertySuffix(u"Suffix");
constexpr OUStringLiteral gsPropertyPositionEndOfDoc(u"PositionEndOfDoc");
constexpr OUStringLiteral gsPropertyFootnoteCounting(u"FootnoteCounting");
constexpr OUStringLiteral gsPropertyEndNotice(u"EndNotice");
constexpr OUStringLiteral gsPropertyBeginNotice(u"BeginNotice");

XMLFootnoteConfigurationImportContext::XMLFootnoteConfigurationImportContext(
    SvXMLImport& rImport,
    sal_Int32 /*nElement*/,
    const Reference<XFastAttributeList> & xAttrList)
:   SvXMLStyleContext(rImport, XmlStyleFamily::TEXT_FOOTNOTECONFIG)
,   sNumFormat(u"1"_ustr)
,   sNumSync(u"false"_ustr)
,   nOffset(0)
,   nNumbering(FootnoteNumbering::PER_PAGE)
,   bPosition(false)
,   bIsEndnote(false)
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if( aIter.getToken() == XML_ELEMENT(TEXT, XML_NOTE_CLASS) )
        {
            if( IsXMLToken(aIter, XML_ENDNOTE ) )
            {
                bIsEndnote = true;
                SetFamily( XmlStyleFamily::TEXT_FOOTNOTECONFIG );
            }
            break;
        }
    }

}
XMLFootnoteConfigurationImportContext::~XMLFootnoteConfigurationImportContext()
{
}

SvXMLEnumMapEntry<sal_Int16> const aFootnoteNumberingMap[] =
{
    { XML_PAGE,             FootnoteNumbering::PER_PAGE },
    { XML_CHAPTER,          FootnoteNumbering::PER_CHAPTER },
    { XML_DOCUMENT,         FootnoteNumbering::PER_DOCUMENT },
    { XML_TOKEN_INVALID,    0 },
};

void XMLFootnoteConfigurationImportContext::SetAttribute( sal_Int32 nElement,
                               const OUString& rValue )
{
    switch (nElement)
    {
        case XML_ELEMENT(TEXT, XML_CITATION_STYLE_NAME):
            sCitationStyle = rValue;
            break;
        case XML_ELEMENT(TEXT, XML_CITATION_BODY_STYLE_NAME):
            sAnchorStyle = rValue;
            break;
        case XML_ELEMENT(TEXT, XML_DEFAULT_STYLE_NAME):
            sDefaultStyle = rValue;
            break;
        case XML_ELEMENT(TEXT, XML_MASTER_PAGE_NAME):
            sPageStyle = rValue;
            break;
        case XML_ELEMENT(TEXT, XML_START_VALUE):
        case XML_ELEMENT(TEXT, XML_OFFSET): // for backwards compatibility with SRC630 & earlier
        {
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber(nTmp, rValue))
            {
                nOffset = static_cast<sal_uInt16>(nTmp);
            }
            break;
        }
        case XML_ELEMENT(STYLE, XML_NUM_PREFIX):
        case XML_ELEMENT(TEXT, XML_NUM_PREFIX): // for backwards compatibility with SRC630 & earlier
            sPrefix = rValue;
            break;
        case XML_ELEMENT(STYLE, XML_NUM_SUFFIX):
        case XML_ELEMENT(TEXT, XML_NUM_SUFFIX): // for backwards compatibility with SRC630 & earlier
            sSuffix = rValue;
            break;
        case XML_ELEMENT(STYLE, XML_NUM_FORMAT):
            sNumFormat = rValue;
            break;
        case XML_ELEMENT(STYLE, XML_NUM_LETTER_SYNC):
            sNumSync = rValue;
            break;
        case XML_ELEMENT(TEXT, XML_START_NUMBERING_AT):
        {
            (void)SvXMLUnitConverter::convertEnum(nNumbering, rValue,
                                                  aFootnoteNumberingMap);
            break;
        }
        case XML_ELEMENT(TEXT, XML_FOOTNOTES_POSITION):
            bPosition = IsXMLToken( rValue, XML_DOCUMENT );
            break;
        default:
            ; // ignore
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLFootnoteConfigurationImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext;

    if (bIsEndnote)
        return nullptr;

    switch (nElement)
    {
        case XML_ELEMENT(TEXT, XML_FOOTNOTE_CONTINUATION_NOTICE_FORWARD):
            xContext = new XMLFootnoteConfigHelper(GetImport(), *this, false);
            break;
        case XML_ELEMENT(TEXT, XML_FOOTNOTE_CONTINUATION_NOTICE_BACKWARD):
            xContext = new XMLFootnoteConfigHelper(GetImport(), *this, true);
            break;
    }

    return xContext;
}

// Rename method <CreateAndInsertLate(..)> to <Finish(..)> (#i40597#)
void XMLFootnoteConfigurationImportContext::Finish( bool bOverwrite )
{

    if (!bOverwrite)
        return;

    if (bIsEndnote)
    {
        Reference<XEndnotesSupplier> xSupplier(
            GetImport().GetModel(), UNO_QUERY);
        if (xSupplier.is())
        {
            ProcessSettings(xSupplier->getEndnoteSettings());
        }
    }
    else
    {
        Reference<XFootnotesSupplier> xSupplier(
            GetImport().GetModel(), UNO_QUERY);
        if (xSupplier.is())
        {
            ProcessSettings(xSupplier->getFootnoteSettings());
        }
    }
    // else: ignore (there's only one configuration, so we can only overwrite)
}

void XMLFootnoteConfigurationImportContext::ProcessSettings(
    const Reference<XPropertySet> & rConfig)
{
    Any aAny;

    if (!sCitationStyle.isEmpty())
    {
        aAny <<= GetImport().GetStyleDisplayName(
                        XmlStyleFamily::TEXT_TEXT, sCitationStyle );
        rConfig->setPropertyValue(gsPropertyCharStyleName, aAny);
    }

    if (!sAnchorStyle.isEmpty())
    {
        aAny <<= GetImport().GetStyleDisplayName(
                        XmlStyleFamily::TEXT_TEXT, sAnchorStyle );
        rConfig->setPropertyValue(gsPropertyAnchorCharStyleName, aAny);
    }

    if (!sPageStyle.isEmpty())
    {
        aAny <<= GetImport().GetStyleDisplayName(
                        XmlStyleFamily::MASTER_PAGE, sPageStyle );
        rConfig->setPropertyValue(gsPropertyPageStyleName, aAny);
    }

    if (!sDefaultStyle.isEmpty())
    {
        aAny <<= GetImport().GetStyleDisplayName(
                        XmlStyleFamily::TEXT_PARAGRAPH, sDefaultStyle );
        rConfig->setPropertyValue(gsPropertyParagraphStyleName, aAny);
    }

    rConfig->setPropertyValue(gsPropertyPrefix, Any(sPrefix));

    rConfig->setPropertyValue(gsPropertySuffix, Any(sSuffix));

    sal_Int16 nNumType = NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType, sNumFormat,
                                                     sNumSync );
    // #i61399: Corrupt file? It contains "Bullet" as numbering style for footnotes.
    // Okay, even it seems to be corrupt, we will oversee this and set the style to ARABIC
    if( NumberingType::CHAR_SPECIAL == nNumType )
        nNumType = NumberingType::ARABIC;

    rConfig->setPropertyValue(gsPropertyNumberingType, Any(nNumType));

    rConfig->setPropertyValue(gsPropertyStartAt, Any(nOffset));

    if (!bIsEndnote)
    {
        rConfig->setPropertyValue(gsPropertyPositionEndOfDoc, Any(bPosition));
        rConfig->setPropertyValue(gsPropertyFootnoteCounting, Any(nNumbering));
        rConfig->setPropertyValue(gsPropertyEndNotice, Any(sEndNotice));
        rConfig->setPropertyValue(gsPropertyBeginNotice, Any(sBeginNotice));
    }
}

void XMLFootnoteConfigurationImportContext::SetBeginNotice(
    const OUString& sText)
{
    sBeginNotice = sText;
}

void XMLFootnoteConfigurationImportContext::SetEndNotice(
    const OUString& sText)
{
    sEndNotice = sText;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
