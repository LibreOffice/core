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


#include "XMLFootnoteConfigurationImportContext.hxx"

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>

#include <xmloff/families.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XFootnote.hpp>
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


/// local helper class for import of quo-vadis and ergo-sum elements
class XMLFootnoteConfigHelper : public SvXMLImportContext
{
    OUStringBuffer sBuffer;
    XMLFootnoteConfigurationImportContext& rConfig;
    bool bIsBegin;

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


XMLFootnoteConfigurationImportContext::XMLFootnoteConfigurationImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLocalName, xAttrList, XML_STYLE_FAMILY_TEXT_FOOTNOTECONFIG)
,   sPropertyAnchorCharStyleName("AnchorCharStyleName")
,   sPropertyCharStyleName("CharStyleName")
,   sPropertyNumberingType("NumberingType")
,   sPropertyPageStyleName("PageStyleName")
,   sPropertyParagraphStyleName("ParaStyleName")
,   sPropertyPrefix("Prefix")
,   sPropertyStartAt("StartAt")
,   sPropertySuffix("Suffix")
,   sPropertyPositionEndOfDoc("PositionEndOfDoc")
,   sPropertyFootnoteCounting("FootnoteCounting")
,   sPropertyEndNotice("EndNotice")
,   sPropertyBeginNotice("BeginNotice")
,   sNumFormat("1")
,   sNumSync("false")
,   pAttrTokenMap(nullptr)
,   nOffset(0)
,   nNumbering(FootnoteNumbering::PER_PAGE)
,   bPosition(false)
,   bIsEndnote(false)
{
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if( XML_NAMESPACE_TEXT == nPrefix && IsXMLToken( sLocalName,
                                                        XML_NOTE_CLASS ) )
        {
            const OUString& rValue = xAttrList->getValueByIndex( nAttr );
            if( IsXMLToken( rValue, XML_ENDNOTE ) )
            {
                bIsEndnote = true;
                SetFamily( XML_STYLE_FAMILY_TEXT_FOOTNOTECONFIG );
            }
            break;
        }
    }

}
XMLFootnoteConfigurationImportContext::~XMLFootnoteConfigurationImportContext()
{
    delete pAttrTokenMap;
}

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
    if (nullptr == pAttrTokenMap)
    {
        pAttrTokenMap = new SvXMLTokenMap(aTextFieldAttrTokenMap);
    }

    return *pAttrTokenMap;
}

static SvXMLEnumMapEntry const aFootnoteNumberingMap[] =
{
    { XML_PAGE,             FootnoteNumbering::PER_PAGE },
    { XML_CHAPTER,          FootnoteNumbering::PER_CHAPTER },
    { XML_DOCUMENT,         FootnoteNumbering::PER_DOCUMENT },
    { XML_TOKEN_INVALID,    0 },
};

void XMLFootnoteConfigurationImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList )
{
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        OUString sValue = xAttrList->getValueByIndex(nAttr);
        switch (GetFtnConfigAttrTokenMap().Get(nPrefix, sLocalName))
        {
            case XML_TOK_FTNCONFIG_CITATION_STYLENAME:
                sCitationStyle = sValue;
                break;
            case XML_TOK_FTNCONFIG_ANCHOR_STYLENAME:
                sAnchorStyle = sValue;
                break;
            case XML_TOK_FTNCONFIG_DEFAULT_STYLENAME:
                sDefaultStyle = sValue;
                break;
            case XML_TOK_FTNCONFIG_PAGE_STYLENAME:
                sPageStyle = sValue;
                break;
            case XML_TOK_FTNCONFIG_OFFSET:
            {
                sal_Int32 nTmp;
                if (::sax::Converter::convertNumber(nTmp, sValue))
                {
                    nOffset = (sal_uInt16)nTmp;
                }
                break;
            }
            case XML_TOK_FTNCONFIG_NUM_PREFIX:
                sPrefix = sValue;
                break;
            case XML_TOK_FTNCONFIG_NUM_SUFFIX:
                sSuffix = sValue;
                break;
            case XML_TOK_FTNCONFIG_NUM_FORMAT:
                sNumFormat = sValue;
                break;
            case XML_TOK_FTNCONFIG_NUM_SYNC:
                sNumSync = sValue;
                break;
            case XML_TOK_FTNCONFIG_START_AT:
            {
                sal_uInt16 nTmp;
                if (SvXMLUnitConverter::convertEnum(nTmp, sValue,
                                                    aFootnoteNumberingMap))
                {
                    nNumbering = nTmp;
                }
                break;
            }
            case XML_TOK_FTNCONFIG_POSITION:
                bPosition = IsXMLToken( sValue, XML_DOCUMENT );
                break;
            default:
                ; // ignore
        }
    }
}

SvXMLImportContext *XMLFootnoteConfigurationImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = nullptr;

    if (!bIsEndnote)
    {
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if ( IsXMLToken( rLocalName,
                             XML_FOOTNOTE_CONTINUATION_NOTICE_FORWARD ) )
            {
                pContext = new XMLFootnoteConfigHelper(GetImport(),
                                                       nPrefix, rLocalName,
                                                       *this, false);
            }
            else if ( IsXMLToken( rLocalName,
                                  XML_FOOTNOTE_CONTINUATION_NOTICE_BACKWARD ) )
            {
                pContext = new XMLFootnoteConfigHelper(GetImport(),
                                                       nPrefix, rLocalName,
                                                       *this, true);
            }
            // else: default context
        }
        // else: unknown namespace -> default context
    }
    // else: endnote -> default context

    if (pContext == nullptr)
    {
        // default: delegate to super class
        pContext = SvXMLStyleContext::CreateChildContext(nPrefix,
                                                         rLocalName,
                                                         xAttrList);
    }

    return pContext;
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
                        XML_STYLE_FAMILY_TEXT_TEXT, sCitationStyle );
        rConfig->setPropertyValue(sPropertyCharStyleName, aAny);
    }

    if (!sAnchorStyle.isEmpty())
    {
        aAny <<= GetImport().GetStyleDisplayName(
                        XML_STYLE_FAMILY_TEXT_TEXT, sAnchorStyle );
        rConfig->setPropertyValue(sPropertyAnchorCharStyleName, aAny);
    }

    if (!sPageStyle.isEmpty())
    {
        aAny <<= GetImport().GetStyleDisplayName(
                        XML_STYLE_FAMILY_MASTER_PAGE, sPageStyle );
        rConfig->setPropertyValue(sPropertyPageStyleName, aAny);
    }

    if (!sDefaultStyle.isEmpty())
    {
        aAny <<= GetImport().GetStyleDisplayName(
                        XML_STYLE_FAMILY_TEXT_PARAGRAPH, sDefaultStyle );
        rConfig->setPropertyValue(sPropertyParagraphStyleName, aAny);
    }

    rConfig->setPropertyValue(sPropertyPrefix, Any(sPrefix));

    rConfig->setPropertyValue(sPropertySuffix, Any(sSuffix));

    sal_Int16 nNumType = NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType, sNumFormat,
                                                     sNumSync );
    // #i61399: Corrupt file? It contains "Bullet" as numbering style for footnotes.
    // Okay, even it seems to be corrupt, we will oversee this and set the style to ARABIC
    if( NumberingType::CHAR_SPECIAL == nNumType )
        nNumType = NumberingType::ARABIC;

    rConfig->setPropertyValue(sPropertyNumberingType, Any(nNumType));

    rConfig->setPropertyValue(sPropertyStartAt, Any(nOffset));

    if (!bIsEndnote)
    {
        aAny.setValue(&bPosition, cppu::UnoType<bool>::get());
        rConfig->setPropertyValue(sPropertyPositionEndOfDoc, aAny);

        aAny <<= nNumbering;
        rConfig->setPropertyValue(sPropertyFootnoteCounting, aAny);

        aAny <<= sEndNotice;
        rConfig->setPropertyValue(sPropertyEndNotice, aAny);

        aAny <<= sBeginNotice;
        rConfig->setPropertyValue(sPropertyBeginNotice, aAny);
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
