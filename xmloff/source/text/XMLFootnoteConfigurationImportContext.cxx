/*************************************************************************
 *
 *  $RCSfile: XMLFootnoteConfigurationImportContext.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-17 18:54:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLFOOTNOTECONFIGURATIONIMPORTCONTEXT_HXX
#include "XMLFootnoteConfigurationImportContext.hxx"
#endif

#ifndef _RTL_USTRING
#include <rtl/ustring>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX
#include "families.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_XMLNUMI_HXX
#include "xmlnumi.hxx"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTE_HPP_
#include <com/sun/star/text/XFootnote.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTESSUPPLIER_HPP_
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XENDNOTESSUPPLIER_HPP_
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_FOOTNOTENUMBERING_HPP_
#include <com/sun/star/text/FootnoteNumbering.hpp>
#endif




using namespace ::rtl;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;


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
//  rConfig = NULL; // import contexts are ref-counted
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
        SvXMLStyleContext(rImport, nPrfx, rLocalName, xAttrList),
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
    SetFamily( bEnd ?
               XML_STYLE_FAMILY_TEXT_ENDNOTECONFIG :
               XML_STYLE_FAMILY_TEXT_FOOTNOTECONFIG );
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
    { XML_NAMESPACE_TEXT, sXML_citation_style_name,
          XML_TOK_FTNCONFIG_CITATION_STYLENAME },
    { XML_NAMESPACE_TEXT, sXML_citation_body_style_name,
          XML_TOK_FTNCONFIG_ANCHOR_STYLENAME },
    { XML_NAMESPACE_TEXT, sXML_default_style_name,
          XML_TOK_FTNCONFIG_DEFAULT_STYLENAME },
    { XML_NAMESPACE_TEXT, sXML_master_page_name,
          XML_TOK_FTNCONFIG_PAGE_STYLENAME },
    { XML_NAMESPACE_TEXT, sXML_offset, XML_TOK_FTNCONFIG_OFFSET },
    { XML_NAMESPACE_TEXT, sXML_num_prefix, XML_TOK_FTNCONFIG_NUM_PREFIX },
    { XML_NAMESPACE_TEXT, sXML_num_suffix, XML_TOK_FTNCONFIG_NUM_SUFFIX },
    { XML_NAMESPACE_STYLE, sXML_num_format, XML_TOK_FTNCONFIG_NUM_FORMAT },
    { XML_NAMESPACE_STYLE, sXML_num_letter_sync, XML_TOK_FTNCONFIG_NUM_SYNC },
    { XML_NAMESPACE_TEXT, sXML_start_numbering_at, XML_TOK_FTNCONFIG_START_AT},
    { XML_NAMESPACE_TEXT, sXML_footnotes_position, XML_TOK_FTNCONFIG_POSITION},
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
    { sXML_page,        FootnoteNumbering::PER_PAGE },
    { sXML_chapter,     FootnoteNumbering::PER_CHAPTER },
    { sXML_document,    FootnoteNumbering::PER_DOCUMENT },
    { 0,                0 },
};

void XMLFootnoteConfigurationImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList )
{
    sal_Int32 nLength = xAttrList->getLength();
    for(sal_Int32 nAttr = 0; nAttr < nLength; nAttr++)
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
                bPosition = (0 == sValue.compareToAscii(sXML_document));
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
            if (rLocalName.equalsAsciiL(
                    sXML_footnote_continuation_notice_forward,
                    sizeof(sXML_footnote_continuation_notice_forward)-1))
            {
                pContext = new XMLFootnoteConfigHelper(GetImport(),
                                                       nPrefix, rLocalName,
                                                       *this, sal_False);
            }
            else if (rLocalName.equalsAsciiL(
                    sXML_footnote_continuation_notice_backward,
                    sizeof(sXML_footnote_continuation_notice_backward)-1))
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

    aAny <<=  SvxXMLListStyleContext::GetNumType( sNumFormat, sNumSync );
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


