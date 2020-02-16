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

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlement.hxx>

#include <xmloff/families.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
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
    bool const bIsBegin;

public:

    XMLFootnoteConfigHelper(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        XMLFootnoteConfigurationImportContext& rConfigImport,
        bool bBegin);

    virtual void EndElement() override;

    virtual void Characters( const OUString& rChars ) override;
};

}

XMLFootnoteConfigHelper::XMLFootnoteConfigHelper(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    XMLFootnoteConfigurationImportContext& rConfigImport,
    bool bBegin)
:   SvXMLImportContext(rImport, nPrfx, rLName)
,   sBuffer()
,   rConfig(rConfigImport)
,   bIsBegin(bBegin)
{
}

void XMLFootnoteConfigHelper::EndElement()
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

void XMLFootnoteConfigHelper::Characters( const OUString& rChars )
{
    sBuffer.append(rChars);
}


// XMLFootnoteConfigurationImportContext

static const OUStringLiteral gsPropertyAnchorCharStyleName("AnchorCharStyleName");
static const OUStringLiteral gsPropertyCharStyleName("CharStyleName");
static const OUStringLiteral gsPropertyNumberingType("NumberingType");
static const OUStringLiteral gsPropertyPageStyleName("PageStyleName");
static const OUStringLiteral gsPropertyParagraphStyleName("ParaStyleName");
static const OUStringLiteral gsPropertyPrefix("Prefix");
static const OUStringLiteral gsPropertyStartAt("StartAt");
static const OUStringLiteral gsPropertySuffix("Suffix");
static const OUStringLiteral gsPropertyPositionEndOfDoc("PositionEndOfDoc");
static const OUStringLiteral gsPropertyFootnoteCounting("FootnoteCounting");
static const OUStringLiteral gsPropertyEndNotice("EndNotice");
static const OUStringLiteral gsPropertyBeginNotice("BeginNotice");

XMLFootnoteConfigurationImportContext::XMLFootnoteConfigurationImportContext(
    SvXMLImport& rImport,
    const Reference<XFastAttributeList> & xAttrList)
:   SvXMLStyleContext(rImport, XmlStyleFamily::TEXT_FOOTNOTECONFIG)
,   sNumFormat("1")
,   sNumSync("false")
,   nOffset(0)
,   nNumbering(FootnoteNumbering::PER_PAGE)
,   bPosition(false)
,   bIsEndnote(false)
{
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );
    for (auto &aIter : *pAttribList)
    {
        OUString sValue = aIter.toString();
        if( aIter.getToken() == XML_ELEMENT(TEXT, XML_NOTE_CLASS) )
        {
            if( IsXMLToken( sValue, XML_ENDNOTE ) )
            {
                bIsEndnote = true;
                SetFamily( XmlStyleFamily::TEXT_FOOTNOTECONFIG );
            }
            break;
        }
        else
            SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << aIter.toString());
    }

}
XMLFootnoteConfigurationImportContext::~XMLFootnoteConfigurationImportContext()
{
}

namespace {

enum XMLFtnConfigToken
{
    XML_TOK_FTNCONFIG_CITATION_STYLENAME,
    XML_TOK_FTNCONFIG_ANCHOR_STYLENAME,
    XML_TOK_FTNCONFIG_DEFAULT_STYLENAME,
    XML_TOK_FTNCONFIG_PAGE_STYLENAME,
    XML_TOK_FTNCONFIG_OFFSET,
    XML_TOK_FTNCONFIG_NUM_PREFIX,
    XML_TOK_FTNCONFIG_NUM_SUFFIX,
    XML_TOK_FTNCONFIG_NUM_FORMAT,
    XML_TOK_FTNCONFIG_NUM_SYNC,
    XML_TOK_FTNCONFIG_START_AT,
    XML_TOK_FTNCONFIG_POSITION
};

}

static const SvXMLTokenMapEntry aTextFieldAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, XML_CITATION_STYLE_NAME,      XML_TOK_FTNCONFIG_CITATION_STYLENAME },
    { XML_NAMESPACE_TEXT, XML_CITATION_BODY_STYLE_NAME, XML_TOK_FTNCONFIG_ANCHOR_STYLENAME },
    { XML_NAMESPACE_TEXT, XML_DEFAULT_STYLE_NAME,       XML_TOK_FTNCONFIG_DEFAULT_STYLENAME },
    { XML_NAMESPACE_TEXT, XML_MASTER_PAGE_NAME,         XML_TOK_FTNCONFIG_PAGE_STYLENAME },
    { XML_NAMESPACE_TEXT, XML_START_VALUE, XML_TOK_FTNCONFIG_OFFSET },
    { XML_NAMESPACE_STYLE, XML_NUM_PREFIX, XML_TOK_FTNCONFIG_NUM_PREFIX },
    { XML_NAMESPACE_STYLE, XML_NUM_SUFFIX, XML_TOK_FTNCONFIG_NUM_SUFFIX },
    { XML_NAMESPACE_STYLE, XML_NUM_FORMAT, XML_TOK_FTNCONFIG_NUM_FORMAT },
    { XML_NAMESPACE_STYLE, XML_NUM_LETTER_SYNC, XML_TOK_FTNCONFIG_NUM_SYNC },
    { XML_NAMESPACE_TEXT, XML_START_NUMBERING_AT, XML_TOK_FTNCONFIG_START_AT},
    { XML_NAMESPACE_TEXT, XML_FOOTNOTES_POSITION, XML_TOK_FTNCONFIG_POSITION},

    // for backwards compatibility with SRC630 & earlier
    { XML_NAMESPACE_TEXT, XML_NUM_PREFIX, XML_TOK_FTNCONFIG_NUM_PREFIX },
    { XML_NAMESPACE_TEXT, XML_NUM_SUFFIX, XML_TOK_FTNCONFIG_NUM_SUFFIX },
    { XML_NAMESPACE_TEXT, XML_OFFSET, XML_TOK_FTNCONFIG_OFFSET },
    XML_TOKEN_MAP_END
};

const SvXMLTokenMap&
    XMLFootnoteConfigurationImportContext::GetFtnConfigAttrTokenMap()
{
    if (!pAttrTokenMap)
    {
        pAttrTokenMap.reset( new SvXMLTokenMap(aTextFieldAttrTokenMap) );
    }

    return *pAttrTokenMap;
}

static SvXMLEnumMapEntry<sal_Int16> const aFootnoteNumberingMap[] =
{
    { XML_PAGE,             FootnoteNumbering::PER_PAGE },
    { XML_CHAPTER,          FootnoteNumbering::PER_CHAPTER },
    { XML_DOCUMENT,         FootnoteNumbering::PER_DOCUMENT },
    { XML_TOKEN_INVALID,    0 },
};

void XMLFootnoteConfigurationImportContext::startFastElement(
    sal_Int32 nElement,
    const Reference<XFastAttributeList> & xAttrList )
{
    sax_fastparser::FastAttributeList *pAttribList =
                    sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );
    for (auto &aIter : *pAttribList)
    {
        OUString sValue = aIter.toString();
        switch (aIter.getToken())
        {
            case XML_ELEMENT(TEXT, XML_CITATION_STYLE_NAME):
                sCitationStyle = sValue;
                break;
            case XML_ELEMENT(TEXT, XML_CITATION_BODY_STYLE_NAME):
                sAnchorStyle = sValue;
                break;
            case XML_ELEMENT(TEXT, XML_DEFAULT_STYLE_NAME):
                sDefaultStyle = sValue;
                break;
            case XML_ELEMENT(TEXT, XML_MASTER_PAGE_NAME):
                sPageStyle = sValue;
                break;
            case XML_ELEMENT(TEXT, XML_START_VALUE):
            case XML_ELEMENT(TEXT, XML_OFFSET): // for backwards compatibility with SRC630 & earlier
            {
                sal_Int32 nTmp;
                if (::sax::Converter::convertNumber(nTmp, sValue))
                {
                    nOffset = static_cast<sal_uInt16>(nTmp);
                }
                break;
            }
            case XML_ELEMENT(STYLE, XML_NUM_PREFIX):
            case XML_ELEMENT(TEXT, XML_NUM_PREFIX): // for backwards compatibility with SRC630 & earlier
                sPrefix = sValue;
                break;
            case XML_ELEMENT(STYLE, XML_NUM_SUFFIX):
            case XML_ELEMENT(TEXT, XML_NUM_SUFFIX): // for backwards compatibility with SRC630 & earlier
                sSuffix = sValue;
                break;
            case XML_ELEMENT(STYLE, XML_NUM_FORMAT):
                sNumFormat = sValue;
                break;
            case XML_ELEMENT(STYLE, XML_NUM_LETTER_SYNC):
                sNumSync = sValue;
                break;
            case XML_ELEMENT(TEXT, XML_START_NUMBERING_AT):
            {
                (void)SvXMLUnitConverter::convertEnum(nNumbering, sValue,
                                                      aFootnoteNumberingMap);
                break;
            }
            case XML_ELEMENT(TEXT, XML_FOOTNOTES_POSITION):
                bPosition = IsXMLToken( sValue, XML_DOCUMENT );
                break;
            default:
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(nElement) << "=" << sValue);
        }
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLFootnoteConfigurationImportContext::createFastChildContext(
        sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    return nullptr;
}

SvXMLImportContextRef XMLFootnoteConfigurationImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContextRef xContext;

    if (!bIsEndnote)
    {
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if ( IsXMLToken( rLocalName,
                             XML_FOOTNOTE_CONTINUATION_NOTICE_FORWARD ) )
            {
                xContext = new XMLFootnoteConfigHelper(GetImport(),
                                                       nPrefix, rLocalName,
                                                       *this, false);
            }
            else if ( IsXMLToken( rLocalName,
                                  XML_FOOTNOTE_CONTINUATION_NOTICE_BACKWARD ) )
            {
                xContext = new XMLFootnoteConfigHelper(GetImport(),
                                                       nPrefix, rLocalName,
                                                       *this, true);
            }
            // else: default context
        }
        // else: unknown namespace -> default context
    }
    // else: endnote -> default context

    if (!xContext)
    {
        // default: delegate to super class
        xContext = SvXMLStyleContext::CreateChildContext(nPrefix,
                                                         rLocalName,
                                                         xAttrList);
    }

    return xContext;
}

// Rename method <CreateAndInsertLate(..)> to <Finish(..)> (#i40597#)
void XMLFootnoteConfigurationImportContext::Finish( bool bOverwrite )
{

    if (bOverwrite)
    {
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
