/*************************************************************************
 *
 *  $RCSfile: XMLIndexTemplateContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-14 14:42:50 $
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

#ifndef _XMLOFF_XMLINDEXBIBLIOGRAPHYENTRYCONTEXT_HXX_
#include "XMLIndexBibliographyEntryContext.hxx"
#endif

#ifndef _XMLOFF_XMLINDEXCHAPTERINFOENTRYCONTEXT_HXX_
#include "XMLIndexChapterInfoEntryContext.hxx"
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
    const OUString& rLocalName,
    const SvXMLEnumMapEntry* pLevelNameMap,
    const sal_Char* pLevelAttrName,
    const sal_Char** pLevelStylePropMap,
    const sal_Bool* pAllowedTokenTypes) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rPropertySet(rPropSet),
        sStyleName(),
        nOutlineLevel(1),   // all indices have level 1 (0 is for header)
        bStyleNameOK(sal_False),
        bOutlineLevelOK(sal_False),
        pOutlineLevelAttrName(pLevelAttrName),
        pOutlineLevelNameMap(pLevelNameMap),
        pOutlineLevelStylePropMap(pLevelStylePropMap),
        pAllowedTokenTypesMap(pAllowedTokenTypes),
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
        sText(RTL_CONSTASCII_USTRINGPARAM("Text")),
        sBibliographyDataField(RTL_CONSTASCII_USTRINGPARAM(
            "BibliographyDataField")),
        sChapterFormat(RTL_CONSTASCII_USTRINGPARAM("ChapterFormat"))
{
    DBG_ASSERT( ((NULL != pLevelAttrName) &&  (NULL != pLevelNameMap))
                || ((NULL == pLevelAttrName) &&  (NULL == pLevelNameMap)),
                "need both, attribute name and value map, or neither" );
    DBG_ASSERT( NULL != pOutlineLevelStylePropMap, "need property name map" );
    DBG_ASSERT( NULL != pAllowedTokenTypes, "need allowed tokens map" );

    // no map for outline-level? then use 1
    if (NULL == pLevelNameMap)
    {
        nOutlineLevel = 1;
        bOutlineLevelOK = sal_True;
    }
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
            else if (NULL != pOutlineLevelAttrName)
            {
                // we have an attr name! Then see if we have the attr, too.
                if (0 == sLocalName.compareToAscii(pOutlineLevelAttrName))
                {
                    // outline level
                    sal_uInt16 nTmp;
                    if (SvXMLUnitConverter::convertEnum(
                        nTmp, xAttrList->getValueByIndex(nAttr),
                        pOutlineLevelNameMap))
                    {
                        nOutlineLevel = nTmp;
                        bOutlineLevelOK = sal_True;
                    }
                    // else: illegal value -> ignore
                }
                // else: unknown attribute -> ignore
            }
            // else: we don't care about outline-level -> ignore
        }
        // else: attribute not in text namespace -> ignore
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
            const sal_Char* pStyleProperty =
                pOutlineLevelStylePropMap[nOutlineLevel];

            DBG_ASSERT(NULL != pStyleProperty, "need property name");
            if (NULL != pStyleProperty)
            {
                aAny <<= sStyleName;
                rPropertySet->setPropertyValue(
                    OUString::createFromAscii(pStyleProperty), aAny);
            }
        }
    }
}



/// template token types; used for aTokenTypeMap parameter
enum TemplateTokenType
{
    XML_TOK_INDEX_TYPE_ENTRY_TEXT = 0,
    XML_TOK_INDEX_TYPE_TAB_STOP,
    XML_TOK_INDEX_TYPE_TEXT,
    XML_TOK_INDEX_TYPE_PAGE_NUMBER,
    XML_TOK_INDEX_TYPE_CHAPTER,
    XML_TOK_INDEX_TYPE_CHAPTER_NUMBER,
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
    { sXML_index_entry_chapter_number, XML_TOK_INDEX_TYPE_CHAPTER_NUMBER },
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
            // can this index accept this kind of token?
            if (pAllowedTokenTypesMap[nToken])
            {
                switch ((TemplateTokenType)nToken)
                {
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

                    case XML_TOK_INDEX_TYPE_CHAPTER_NUMBER:
                        pContext = new XMLIndexSimpleEntryContext(
                            GetImport(), sTokenEntryNumber, *this,
                            nPrefix, rLocalName);
                        break;

                    case XML_TOK_INDEX_TYPE_BIBLIOGRAPHY:
                        pContext = new XMLIndexBibliographyEntryContext(
                            GetImport(), *this, nPrefix, rLocalName);
                        break;

                    case XML_TOK_INDEX_TYPE_CHAPTER:
                        pContext = new XMLIndexChapterInfoEntryContext(
                            GetImport(), *this, nPrefix, rLocalName);
                        break;

                    default:
                        // ignore!
                        break;
                }
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



//
// maps for the XMLIndexTemplateContext constructor
//


// table of content and user defined index:

const SvXMLEnumMapEntry aLevelNameTOCMap[] =
{
    { sXML_1, 1 },
    { sXML_2, 2 },
    { sXML_3, 3 },
    { sXML_4, 4 },
    { sXML_5, 5 },
    { sXML_6, 6 },
    { sXML_7, 7 },
    { sXML_8, 8 },
    { sXML_9, 9 },
    { sXML_10, 10 },
    { NULL, NULL }
};

const sal_Char* aLevelStylePropNameTOCMap[] =
    { NULL, "ParaStyleLevel1", "ParaStyleLevel2", "ParaStyleLevel3",
          "ParaStyleLevel4", "ParaStyleLevel5", "ParaStyleLevel6",
          "ParaStyleLevel7", "ParaStyleLevel8", "ParaStyleLevel9",
          "ParaStyleLevel10", NULL };

const sal_Bool aAllowedTokenTypesTOC[] =
{
    sal_True,       // XML_TOK_INDEX_TYPE_ENTRY_TEXT =
    sal_True,       // XML_TOK_INDEX_TYPE_TAB_STOP,
    sal_True,       // XML_TOK_INDEX_TYPE_TEXT,
    sal_True,       // XML_TOK_INDEX_TYPE_PAGE_NUMBER,
    sal_False,      // XML_TOK_INDEX_TYPE_CHAPTER,
    sal_True,       // XML_TOK_INDEX_TYPE_CHAPTER_NUMBER
    sal_True,       // XML_TOK_INDEX_TYPE_LINK_START,
    sal_True,       // XML_TOK_INDEX_TYPE_LINK_END,
    sal_False       // XML_TOK_INDEX_TYPE_BIBLIOGRAPHY
};

const sal_Bool aAllowedTokenTypesUser[] =
{
    sal_True,       // XML_TOK_INDEX_TYPE_ENTRY_TEXT =
    sal_True,       // XML_TOK_INDEX_TYPE_TAB_STOP,
    sal_True,       // XML_TOK_INDEX_TYPE_TEXT,
    sal_True,       // XML_TOK_INDEX_TYPE_PAGE_NUMBER,
    sal_False,      // XML_TOK_INDEX_TYPE_CHAPTER,
    sal_True,       // XML_TOK_INDEX_TYPE_CHAPTER_NUMBER
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_START,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_END,
    sal_False       // XML_TOK_INDEX_TYPE_BIBLIOGRAPHY
};


// alphabetical index

const SvXMLEnumMapEntry aLevelNameAlphaMap[] =
{
    { sXML_separator, 1 },
    { sXML_1, 2 },
    { sXML_2, 3 },
    { sXML_3, 4 },
    { NULL, NULL }
};

const sal_Char* aLevelStylePropNameAlphaMap[] =
    { NULL, "ParaStyleSeparator", "ParaStyleLevel1", "ParaStyleLevel2",
          "ParaStyleLevel3", NULL };

const sal_Bool aAllowedTokenTypesAlpha[] =
{
    sal_True,       // XML_TOK_INDEX_TYPE_ENTRY_TEXT =
    sal_True,       // XML_TOK_INDEX_TYPE_TAB_STOP,
    sal_True,       // XML_TOK_INDEX_TYPE_TEXT,
    sal_True,       // XML_TOK_INDEX_TYPE_PAGE_NUMBER,
    sal_True,       // XML_TOK_INDEX_TYPE_CHAPTER,
    sal_False,      // XML_TOK_INDEX_TYPE_CHAPTER_NUMBER
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_START,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_END,
    sal_False       // XML_TOK_INDEX_TYPE_BIBLIOGRAPHY
};


// bibliography index:

const SvXMLEnumMapEntry aLevelNameBibliographyMap[] =
{
    { sXML_article, 1 },
    { sXML_book, 2 },
    { sXML_booklet, 3 },
    { sXML_conference, 4 },
    { sXML_custom1, 5 },
    { sXML_custom2, 6 },
    { sXML_custom3, 7 },
    { sXML_custom4, 8 },
    { sXML_custom5, 9 },
    { sXML_email, 10 },
    { sXML_inbook, 11 },
    { sXML_incollection, 12 },
    { sXML_inproceedings, 13 },
    { sXML_journal, 14 },
    { sXML_manual, 15 },
    { sXML_mastersthesis, 16 },
    { sXML_misc, 17 },
    { sXML_phdthesis, 18 },
    { sXML_proceedings, 19 },
    { sXML_techreport, 20 },
    { sXML_unpublished, 21 },
    { sXML_www, 22 },
    { NULL, NULL }
};

// TODO: replace with real property names, when available
const sal_Char* aLevelStylePropNameBibliographyMap[] =
{
    NULL, "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", NULL };

const sal_Bool aAllowedTokenTypesBibliography[] =
{
    sal_True,       // XML_TOK_INDEX_TYPE_ENTRY_TEXT =
    sal_True,       // XML_TOK_INDEX_TYPE_TAB_STOP,
    sal_True,       // XML_TOK_INDEX_TYPE_TEXT,
    sal_True,       // XML_TOK_INDEX_TYPE_PAGE_NUMBER,
    sal_False,      // XML_TOK_INDEX_TYPE_CHAPTER,
    sal_False,      // XML_TOK_INDEX_TYPE_CHAPTER_NUMBER
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_START,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_END,
    sal_True        // XML_TOK_INDEX_TYPE_BIBLIOGRAPHY
};


// table, illustration and object index

// no name map
const SvXMLEnumMapEntry* aLevelNameTableMap = NULL;

const sal_Char* aLevelStylePropNameTableMap[] =
    { NULL, "ParaStyleLevel1", NULL };

const sal_Bool aAllowedTokenTypesTable[] =
{
    sal_True,       // XML_TOK_INDEX_TYPE_ENTRY_TEXT =
    sal_True,       // XML_TOK_INDEX_TYPE_TAB_STOP,
    sal_True,       // XML_TOK_INDEX_TYPE_TEXT,
    sal_True,       // XML_TOK_INDEX_TYPE_PAGE_NUMBER,
    sal_True,       // XML_TOK_INDEX_TYPE_CHAPTER,
    sal_False,      // XML_TOK_INDEX_TYPE_CHAPTER_NUMBER
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_START,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_END,
    sal_False       // XML_TOK_INDEX_TYPE_BIBLIOGRAPHY
};

