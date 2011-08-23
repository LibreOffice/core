/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "XMLFootnoteConfigurationImportContext.hxx"

#include "rtl/ustring.hxx"

#include <rtl/ustrbuf.hxx>


#include "nmspmap.hxx"

#include "xmlnmspe.hxx"



#include "xmluconv.hxx"

#include "xmlimp.hxx"





#include <com/sun/star/text/XFootnotesSupplier.hpp>

#include <com/sun/star/text/XEndnotesSupplier.hpp>

#include <com/sun/star/text/FootnoteNumbering.hpp>
#include <com/sun/star/style/NumberingType.hpp>
namespace binfilter {


using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::xml::sax;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;
using rtl::OUStringBuffer;

//
//  XMLFootnoteConfigHelper
//

/// local helper class for import of quo-vadis and ergo-sum elements
class XMLFootnoteConfigHelper : public SvXMLImportContext
{
    OUStringBuffer sBuffer;
    XMLFootnoteConfigurationImportContext& rConfig;
    sal_Bool bIsBegin;

public:
    TYPEINFO();

    XMLFootnoteConfigHelper(
        SvXMLImport& rImport, 
        sal_uInt16 nPrfx,
        const OUString& rLName,
        XMLFootnoteConfigurationImportContext& rConfigImport,
        sal_Bool bBegin);

    virtual void EndElement();

    virtual void Characters( const OUString& rChars );
};

TYPEINIT1( XMLFootnoteConfigHelper, SvXMLImportContext );

XMLFootnoteConfigHelper::XMLFootnoteConfigHelper(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLName,
    XMLFootnoteConfigurationImportContext& rConfigImport,
    sal_Bool bBegin) :
        SvXMLImportContext(rImport, nPrfx, rLName),
        rConfig(rConfigImport),
        sBuffer(),
        bIsBegin(bBegin)
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
//	rConfig = NULL; // import contexts are ref-counted
}

void XMLFootnoteConfigHelper::Characters( const OUString& rChars )
{
    sBuffer.append(rChars);
}


//
// XMLFootnoteConfigurationImportContext
//


TYPEINIT1( XMLFootnoteConfigurationImportContext, SvXMLStyleContext );

XMLFootnoteConfigurationImportContext::XMLFootnoteConfigurationImportContext( 
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList,
    sal_Bool bEnd) :
        SvXMLStyleContext(rImport, nPrfx, rLocalName, xAttrList, bEnd ? 
               XML_STYLE_FAMILY_TEXT_ENDNOTECONFIG :
               XML_STYLE_FAMILY_TEXT_FOOTNOTECONFIG),
        bIsEndnote(bEnd),
        pAttrTokenMap(NULL),
        sCitationStyle(),
        sAnchorStyle(),
        sDefaultStyle(),
        sPageStyle(),
        sPrefix(),
        sSuffix(),
        sBeginNotice(),
        sEndNotice(),
        sNumFormat(RTL_CONSTASCII_USTRINGPARAM("1")),
        sNumSync(RTL_CONSTASCII_USTRINGPARAM("false")),
        nOffset(0),
        nNumbering(FootnoteNumbering::PER_PAGE),
        bPosition(sal_False),
        sPropertyCharStyleName(RTL_CONSTASCII_USTRINGPARAM("CharStyleName")),
        sPropertyAnchorCharStyleName(
            RTL_CONSTASCII_USTRINGPARAM("AnchorCharStyleName")),
        sPropertyNumberingType(RTL_CONSTASCII_USTRINGPARAM("NumberingType")),
        sPropertyPageStyleName(RTL_CONSTASCII_USTRINGPARAM("PageStyleName")),
        sPropertyParagraphStyleName(
            RTL_CONSTASCII_USTRINGPARAM("ParaStyleName")),
        sPropertyPrefix(RTL_CONSTASCII_USTRINGPARAM("Prefix")),
        sPropertyStartAt(RTL_CONSTASCII_USTRINGPARAM("StartAt")),
        sPropertySuffix(RTL_CONSTASCII_USTRINGPARAM("Suffix")),
        sPropertyPositionEndOfDoc(
            RTL_CONSTASCII_USTRINGPARAM("PositionEndOfDoc")),
        sPropertyFootnoteCounting(
            RTL_CONSTASCII_USTRINGPARAM("FootnoteCounting")),
        sPropertyEndNotice(RTL_CONSTASCII_USTRINGPARAM("EndNotice")),
        sPropertyBeginNotice(RTL_CONSTASCII_USTRINGPARAM("BeginNotice"))
{
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

static __FAR_DATA SvXMLTokenMapEntry aTextFieldAttrTokenMap[] =
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
    if (NULL == pAttrTokenMap) 
    {
        pAttrTokenMap = new SvXMLTokenMap(aTextFieldAttrTokenMap);
    }

    return *pAttrTokenMap;
}

static SvXMLEnumMapEntry __READONLY_DATA aFootnoteNumberingMap[] =
{
    { XML_PAGE,		        FootnoteNumbering::PER_PAGE },
    { XML_CHAPTER,		    FootnoteNumbering::PER_CHAPTER },
    { XML_DOCUMENT,	        FootnoteNumbering::PER_DOCUMENT },
    { XML_TOKEN_INVALID, 	0 },
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
                if (SvXMLUnitConverter::convertNumber(nTmp, sValue))
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
    USHORT nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if (!bIsEndnote)
    {
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if ( IsXMLToken( rLocalName, 
                             XML_FOOTNOTE_CONTINUATION_NOTICE_FORWARD ) )
            {
                pContext = new XMLFootnoteConfigHelper(GetImport(),
                                                       nPrefix, rLocalName,
                                                       *this, sal_False);
            }
            else if ( IsXMLToken( rLocalName, 
                                  XML_FOOTNOTE_CONTINUATION_NOTICE_BACKWARD ) )
            {
                pContext = new XMLFootnoteConfigHelper(GetImport(),
                                                       nPrefix, rLocalName,
                                                       *this, sal_True);
            }
            // else: default context
        }
        // else: unknown namespace -> default context
    }
    // else: endnote -> default context

    if (pContext == NULL)
    {
        // default: delegate to super class
        pContext = SvXMLStyleContext::CreateChildContext(nPrefix, 
                                                         rLocalName, 
                                                         xAttrList);
    }

    return pContext;
}


void XMLFootnoteConfigurationImportContext::CreateAndInsertLate( 
    sal_Bool bOverwrite )
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

    if (sCitationStyle.getLength() > 0)
    {
        aAny <<= sCitationStyle;
        rConfig->setPropertyValue(sPropertyCharStyleName, aAny);
    }

    if (sAnchorStyle.getLength() > 0)
    {
        aAny <<= sAnchorStyle;
        rConfig->setPropertyValue(sPropertyAnchorCharStyleName, aAny);
    }

    if (sPageStyle.getLength() > 0)
    {
        aAny <<= sPageStyle;
        rConfig->setPropertyValue(sPropertyPageStyleName, aAny);
    }

    if (sDefaultStyle.getLength() > 0)
    {
        aAny <<= sDefaultStyle;
        rConfig->setPropertyValue(sPropertyParagraphStyleName, aAny);
    }

    aAny <<= sPrefix;
    rConfig->setPropertyValue(sPropertyPrefix, aAny);

    aAny <<= sSuffix;
    rConfig->setPropertyValue(sPropertySuffix, aAny);

    sal_Int16 nNumType = NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType, sNumFormat,
                                                     sNumSync );
    aAny <<=  nNumType;
    rConfig->setPropertyValue(sPropertyNumberingType, aAny);

    aAny <<= nOffset;
    rConfig->setPropertyValue(sPropertyStartAt, aAny);

    if (!bIsEndnote)
    {
        aAny.setValue(&bPosition, ::getBooleanCppuType());
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
    OUString sText)
{
    sBeginNotice = sText;
}

void XMLFootnoteConfigurationImportContext::SetEndNotice(
    OUString sText)
{
    sEndNotice = sText;
}


}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
