/*************************************************************************
 *
 *  $RCSfile: XMLIndexTemplateContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-02 15:51:18 $
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


#ifndef _XMLOFF_XMLINDEXTEMPLATECONTEXT_HXX_
#include "XMLIndexTemplateContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXSIMPLEENTRYCONTEXT_HXX_
#include "XMLIndexSimpleEntryContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXSPANENTRYCONTEXT_HXX_
#include "XMLIndexSpanEntryContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXTABSTOPENTRYCONTEXT_HXX_
#include "XMLIndexTabStopEntryContext.hxx"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_TEXTIMP_HXX_
#include "txtimp.hxx"
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

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif


using namespace ::std;
//using namespace ::com::sun::star::text;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::PropertyValues;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::container::XIndexReplace;

const sal_Char sAPI_TokenEntryNumber[] =    "TokenEntryNumber";
const sal_Char sAPI_TokenEntryText[] =      "TokenEntryText";
const sal_Char sAPI_TokenTabStop[] =        "TokenTabStop";
const sal_Char sAPI_TokenText[] =           "TokenText";
const sal_Char sAPI_TokenPageNumber[] =     "TokenPageNumber";
const sal_Char sAPI_TokenChapterInfo[] =    "TokenChapterInfo";
const sal_Char sAPI_TokenHyperlinkStart[] = "TokenHyperlinkStart";
const sal_Char sAPI_TokenHyperlinkEnd[] =   "TokenHyperlinkEnd";
const sal_Char sAPI_TokenBibliographyDataField[] =
                                            "TokenBibliographyDataField";


TYPEINIT1( XMLIndexTemplateContext, SvXMLImportContext);

XMLIndexTemplateContext::XMLIndexTemplateContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet,
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rPropertySet(rPropSet),
        sTokenEntryNumber(RTL_CONSTASCII_USTRINGPARAM(sAPI_TokenEntryNumber)),
        sTokenEntryText(RTL_CONSTASCII_USTRINGPARAM(sAPI_TokenEntryText)),
        sTokenTabStop(RTL_CONSTASCII_USTRINGPARAM(sAPI_TokenTabStop)),
        sTokenText(RTL_CONSTASCII_USTRINGPARAM(sAPI_TokenText)),
        sTokenPageNumber(RTL_CONSTASCII_USTRINGPARAM(sAPI_TokenPageNumber)),
        sTokenChapterInfo(RTL_CONSTASCII_USTRINGPARAM(sAPI_TokenChapterInfo)),
        sTokenHyperlinkStart(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_TokenHyperlinkStart)),
        sTokenHyperlinkEnd(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_TokenHyperlinkEnd)),
        sTokenBibliographyDataField(RTL_CONSTASCII_USTRINGPARAM(
            sAPI_TokenBibliographyDataField)),
        sLevelFormat(RTL_CONSTASCII_USTRINGPARAM("LevelFormat")),
        sParaStyleLevel(RTL_CONSTASCII_USTRINGPARAM("ParaStyleLevel")),
        sTabStopRightAligned(RTL_CONSTASCII_USTRINGPARAM(
            "TabStopRightAligned")),
        sTabStopPosition(RTL_CONSTASCII_USTRINGPARAM("TabStopPosition")),
        sTabStopFillCharacter(RTL_CONSTASCII_USTRINGPARAM(
            "TabStopFillCharacter")),
        sCharacterStyleName(RTL_CONSTASCII_USTRINGPARAM("CharacterStyleName")),
        sTokenType(RTL_CONSTASCII_USTRINGPARAM("TokenType")),
        sText(RTL_CONSTASCII_USTRINGPARAM("Text"))
{
}

XMLIndexTemplateContext::~XMLIndexTemplateContext()
{
}


void XMLIndexTemplateContext::addTemplateEntry(
    const PropertyValues& aValues)
{
    aValueVector.push_back(aValues);
}

void XMLIndexTemplateContext::StartElement(
        const Reference<XAttributeList> & xAttrList)
{
    // process two attributes: style-name, outline-level
    sal_Int32 nLength = xAttrList->getLength();
    for(sal_Int32 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if (sLocalName.equalsAsciiL(sXML_style_name,
                                        sizeof(sXML_style_name)-1))
            {
                // style name
                sStyleName = xAttrList->getValueByIndex(nAttr);
                bStyleNameOK = sal_True;
            }
            else if (sLocalName.equalsAsciiL(sXML_outline_level,
                                             sizeof(sXML_outline_level)-1))
            {
                // outline level
                sal_Int32 nTmp;
                if (SvXMLUnitConverter::convertNumber(
                    nTmp, xAttrList->getValueByIndex(nAttr), 1,
                    GetImport().GetTextImport()->
                        GetChapterNumbering()->getCount() ))
                {
                    nOutlineLevel = nTmp;
                    bOutlineLevelOK = sal_True;
                }
            }
            // else: unknown attribute
        }
        // else: attribute not in text namespace
    }
}

void XMLIndexTemplateContext::EndElement()
{
    if (bOutlineLevelOK)
    {
        sal_Int32 nCount = aValueVector.size();
        Sequence<PropertyValues> aValueSequence(nCount);
        for(sal_Int32 i = 0; i<nCount; i++)
        {
            aValueSequence[i] = aValueVector[i];
        }

        // get LevelFormat IndexReplace ...
        Any aAny = rPropertySet->getPropertyValue(sLevelFormat);
        Reference<XIndexReplace> xIndexReplace;
        aAny >>= xIndexReplace;

        // ... and insert
        aAny <<= aValueSequence;
        xIndexReplace->replaceByIndex(nOutlineLevel, aAny);

        if (bStyleNameOK)
        {
            // create property name and set
            OUStringBuffer sBuf;
            sBuf.append(sParaStyleLevel);
            sBuf.append(nOutlineLevel);

            aAny <<= sStyleName;
            rPropertySet->setPropertyValue(sBuf.makeStringAndClear(), aAny);
        }
    }
}

enum TemplateTokenType
{
    XML_TOK_INDEX_TYPE_ENTRY_TEXT,
    XML_TOK_INDEX_TYPE_TAB_STOP,
    XML_TOK_INDEX_TYPE_TEXT,
    XML_TOK_INDEX_TYPE_PAGE_NUMBER,
    XML_TOK_INDEX_TYPE_CHAPTER,
    XML_TOK_INDEX_TYPE_LINK_START,
    XML_TOK_INDEX_TYPE_LINK_END,
    XML_TOK_INDEX_TYPE_BIBLIOGRAPHY
};

SvXMLEnumMapEntry aTemplateTokenTypeMap[] =
{
    { sXML_index_entry_chapter, XML_TOK_INDEX_TYPE_CHAPTER },
    { sXML_index_entry_text, XML_TOK_INDEX_TYPE_ENTRY_TEXT },
    { sXML_index_entry_tab_stop, XML_TOK_INDEX_TYPE_TAB_STOP },
    { sXML_index_entry_span, XML_TOK_INDEX_TYPE_TEXT },
    { sXML_index_entry_page_number, XML_TOK_INDEX_TYPE_PAGE_NUMBER },
    { sXML_index_entry_chapter, XML_TOK_INDEX_TYPE_CHAPTER },
    { sXML_index_entry_link_start, XML_TOK_INDEX_TYPE_LINK_START },
    { sXML_index_entry_link_end, XML_TOK_INDEX_TYPE_LINK_END },
    { sXML_index_entry_bibliography, XML_TOK_INDEX_TYPE_BIBLIOGRAPHY },
    { 0, 0 }
};

SvXMLImportContext *XMLIndexTemplateContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if (XML_NAMESPACE_TEXT == nPrefix)
    {
        sal_uInt16 nToken;
        if (SvXMLUnitConverter::convertEnum(nToken, rLocalName,
                                            aTemplateTokenTypeMap))
        {
            switch ((TemplateTokenType)nToken)
            {
// TODO: only allow entries for the appropriate index types!
                case XML_TOK_INDEX_TYPE_ENTRY_TEXT:
                    pContext = new XMLIndexSimpleEntryContext(
                        GetImport(), sTokenEntryText, *this,
                        nPrefix, rLocalName);
                    break;

                case XML_TOK_INDEX_TYPE_PAGE_NUMBER:
                    pContext = new XMLIndexSimpleEntryContext(
                        GetImport(), sTokenPageNumber, *this,
                        nPrefix, rLocalName);
                    break;

                case XML_TOK_INDEX_TYPE_LINK_START:
                    pContext = new XMLIndexSimpleEntryContext(
                        GetImport(), sTokenHyperlinkStart, *this,
                        nPrefix, rLocalName);
                    break;

                case XML_TOK_INDEX_TYPE_LINK_END:
                    pContext = new XMLIndexSimpleEntryContext(
                        GetImport(), sTokenHyperlinkEnd, *this,
                        nPrefix, rLocalName);
                    break;

                case XML_TOK_INDEX_TYPE_TEXT:
                    pContext = new XMLIndexSpanEntryContext(
                        GetImport(), *this, nPrefix, rLocalName);
                    break;

                case XML_TOK_INDEX_TYPE_TAB_STOP:
                    pContext = new XMLIndexTabStopEntryContext(
                        GetImport(), *this, nPrefix, rLocalName);
                    break;

                case XML_TOK_INDEX_TYPE_CHAPTER:
                    // TODO: implement "real" chapter context that
                    //       parses chapter attributes
                    pContext = new XMLIndexSimpleEntryContext(
                        GetImport(), sTokenEntryNumber, *this,
                        nPrefix, rLocalName);
                    break;

                case XML_TOK_INDEX_TYPE_BIBLIOGRAPHY:
                    // TODO: bibliography context
                default:
                    break;
            }
        }
    }

    // ignore unknown
    if (NULL == pContext)
    {
        return SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                      xAttrList);
    }

    return pContext;
}
