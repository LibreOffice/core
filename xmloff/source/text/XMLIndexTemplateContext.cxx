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

#include "XMLIndexTemplateContext.hxx"
#include "XMLIndexSimpleEntryContext.hxx"
#include "XMLIndexSpanEntryContext.hxx"
#include "XMLIndexTabStopEntryContext.hxx"
#include "XMLIndexBibliographyEntryContext.hxx"
#include "XMLIndexChapterInfoEntryContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <tools/debug.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/container/XIndexReplace.hpp>


using namespace ::std;
using namespace ::xmloff::token;

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


XMLIndexTemplateContext::XMLIndexTemplateContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const SvXMLEnumMapEntry* pLevelNameMap,
    enum XMLTokenEnum eLevelAttrName,
    const sal_Char** pLevelStylePropMap,
    const sal_Bool* pAllowedTokenTypes,
    bool bT )
:   SvXMLImportContext(rImport, nPrfx, rLocalName)
,   pOutlineLevelNameMap(pLevelNameMap)
,   eOutlineLevelAttrName(eLevelAttrName)
,   pOutlineLevelStylePropMap(pLevelStylePropMap)
,   pAllowedTokenTypesMap(pAllowedTokenTypes)
,   nOutlineLevel(1)    // all indices have level 1 (0 is for header)
,   bStyleNameOK(false)
,   bOutlineLevelOK(false)
,   bTOC( bT )
,   rPropertySet(rPropSet)
,   sTokenEntryNumber(sAPI_TokenEntryNumber)
,   sTokenEntryText(sAPI_TokenEntryText)
,   sTokenTabStop(sAPI_TokenTabStop)
,   sTokenText(sAPI_TokenText)
,   sTokenPageNumber(sAPI_TokenPageNumber)
,   sTokenChapterInfo(sAPI_TokenChapterInfo)
,   sTokenHyperlinkStart(sAPI_TokenHyperlinkStart)
,   sTokenHyperlinkEnd(sAPI_TokenHyperlinkEnd)
,   sTokenBibliographyDataField(sAPI_TokenBibliographyDataField)

,   sCharacterStyleName("CharacterStyleName")
,   sTokenType("TokenType")
,   sText("Text")
,   sTabStopRightAligned("TabStopRightAligned")
,   sTabStopPosition("TabStopPosition")
,   sTabStopFillCharacter("TabStopFillCharacter")
,   sBibliographyDataField("BibliographyDataField")
,   sChapterFormat("ChapterFormat")
,   sChapterLevel("ChapterLevel") //#i53420

,   sLevelFormat("LevelFormat")
{
    DBG_ASSERT( ((XML_TOKEN_INVALID != eLevelAttrName) &&  (nullptr != pLevelNameMap))
                || ((XML_TOKEN_INVALID == eLevelAttrName) &&  (nullptr == pLevelNameMap)),
                "need both, attribute name and value map, or neither" );
    DBG_ASSERT( nullptr != pOutlineLevelStylePropMap, "need property name map" );
    DBG_ASSERT( nullptr != pAllowedTokenTypes, "need allowed tokens map" );

    // no map for outline-level? then use 1
    if (nullptr == pLevelNameMap)
    {
        nOutlineLevel = 1;
        bOutlineLevelOK = true;
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
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if ( IsXMLToken( sLocalName, XML_STYLE_NAME ) )
            {
                // style name
                sStyleName = xAttrList->getValueByIndex(nAttr);
                bStyleNameOK = true;
            }
            else if (eOutlineLevelAttrName != XML_TOKEN_INVALID)
            {
                // we have an attr name! Then see if we have the attr, too.
                if (IsXMLToken(sLocalName, eOutlineLevelAttrName))
                {
                    // outline level
                    sal_uInt16 nTmp;
                    if (SvXMLUnitConverter::convertEnum(
                        nTmp, xAttrList->getValueByIndex(nAttr),
                        pOutlineLevelNameMap))
                    {
                        nOutlineLevel = nTmp;
                        bOutlineLevelOK = true;
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
        const sal_Int32 nCount = aValueVector.size();
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
        xIndexReplace->replaceByIndex(nOutlineLevel, Any(aValueSequence));

        if (bStyleNameOK)
        {
            const sal_Char* pStyleProperty =
                pOutlineLevelStylePropMap[nOutlineLevel];

            DBG_ASSERT(nullptr != pStyleProperty, "need property name");
            if (nullptr != pStyleProperty)
            {
                OUString sDisplayStyleName =
                        GetImport().GetStyleDisplayName(
                        XML_STYLE_FAMILY_TEXT_PARAGRAPH,
                        sStyleName );
                // #i50288#: Check if style exists
                const Reference < ::com::sun::star::container::XNameContainer > & rStyles =
                    GetImport().GetTextImport()->GetParaStyles();
                if( rStyles.is() &&
                    rStyles->hasByName( sDisplayStyleName ) )
                {
                    rPropertySet->setPropertyValue(
                        OUString::createFromAscii(pStyleProperty), css::uno::Any(sDisplayStyleName));
                }
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
    XML_TOK_INDEX_TYPE_LINK_START,
    XML_TOK_INDEX_TYPE_LINK_END,
    XML_TOK_INDEX_TYPE_BIBLIOGRAPHY
};


SvXMLEnumMapEntry aTemplateTokenTypeMap[] =
{
    { XML_INDEX_ENTRY_TEXT,         XML_TOK_INDEX_TYPE_ENTRY_TEXT },
    { XML_INDEX_ENTRY_TAB_STOP,     XML_TOK_INDEX_TYPE_TAB_STOP },
    { XML_INDEX_ENTRY_SPAN,         XML_TOK_INDEX_TYPE_TEXT },
    { XML_INDEX_ENTRY_PAGE_NUMBER,  XML_TOK_INDEX_TYPE_PAGE_NUMBER },
    { XML_INDEX_ENTRY_CHAPTER,      XML_TOK_INDEX_TYPE_CHAPTER },
    { XML_INDEX_ENTRY_LINK_START,   XML_TOK_INDEX_TYPE_LINK_START },
    { XML_INDEX_ENTRY_LINK_END,     XML_TOK_INDEX_TYPE_LINK_END },
    { XML_INDEX_ENTRY_BIBLIOGRAPHY, XML_TOK_INDEX_TYPE_BIBLIOGRAPHY },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLImportContext *XMLIndexTemplateContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = nullptr;

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

                    case XML_TOK_INDEX_TYPE_BIBLIOGRAPHY:
                        pContext = new XMLIndexBibliographyEntryContext(
                            GetImport(), *this, nPrefix, rLocalName);
                        break;

                    case XML_TOK_INDEX_TYPE_CHAPTER:
                        pContext = new XMLIndexChapterInfoEntryContext(
                            GetImport(), *this, nPrefix, rLocalName, bTOC );
                        break;

                    default:
                        // ignore!
                        break;
                }
            }
        }
    }

    // ignore unknown
    if (nullptr == pContext)
    {
        return SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                      xAttrList);
    }

    return pContext;
}


// maps for the XMLIndexTemplateContext constructor


// table of content and user defined index:

const SvXMLEnumMapEntry aSvLevelNameTOCMap[] =
{
    { XML_1, 1 },
    { XML_2, 2 },
    { XML_3, 3 },
    { XML_4, 4 },
    { XML_5, 5 },
    { XML_6, 6 },
    { XML_7, 7 },
    { XML_8, 8 },
    { XML_9, 9 },
    { XML_10, 10 },
    { XML_TOKEN_INVALID, 0 }
};

const sal_Char* aLevelStylePropNameTOCMap[] =
    { nullptr, "ParaStyleLevel1", "ParaStyleLevel2", "ParaStyleLevel3",
          "ParaStyleLevel4", "ParaStyleLevel5", "ParaStyleLevel6",
          "ParaStyleLevel7", "ParaStyleLevel8", "ParaStyleLevel9",
          "ParaStyleLevel10", nullptr };

const sal_Bool aAllowedTokenTypesTOC[] =
{
    sal_True,       // XML_TOK_INDEX_TYPE_ENTRY_TEXT =
    sal_True,       // XML_TOK_INDEX_TYPE_TAB_STOP,
    sal_True,       // XML_TOK_INDEX_TYPE_TEXT,
    sal_True,       // XML_TOK_INDEX_TYPE_PAGE_NUMBER,
    sal_True,       // XML_TOK_INDEX_TYPE_CHAPTER,
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
    sal_True,       // XML_TOK_INDEX_TYPE_CHAPTER,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_START,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_END,
    sal_False       // XML_TOK_INDEX_TYPE_BIBLIOGRAPHY
};


// alphabetical index

const SvXMLEnumMapEntry aLevelNameAlphaMap[] =
{
    { XML_SEPARATOR, 1 },
    { XML_1, 2 },
    { XML_2, 3 },
    { XML_3, 4 },
    { XML_TOKEN_INVALID, 0 }
};

const sal_Char* aLevelStylePropNameAlphaMap[] =
    { nullptr, "ParaStyleSeparator", "ParaStyleLevel1", "ParaStyleLevel2",
          "ParaStyleLevel3", nullptr };

const sal_Bool aAllowedTokenTypesAlpha[] =
{
    sal_True,       // XML_TOK_INDEX_TYPE_ENTRY_TEXT =
    sal_True,       // XML_TOK_INDEX_TYPE_TAB_STOP,
    sal_True,       // XML_TOK_INDEX_TYPE_TEXT,
    sal_True,       // XML_TOK_INDEX_TYPE_PAGE_NUMBER,
    sal_True,       // XML_TOK_INDEX_TYPE_CHAPTER,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_START,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_END,
    sal_False       // XML_TOK_INDEX_TYPE_BIBLIOGRAPHY
};


// bibliography index:

const SvXMLEnumMapEntry aLevelNameBibliographyMap[] =
{
    { XML_ARTICLE, 1 },
    { XML_BOOK, 2 },
    { XML_BOOKLET, 3 },
    { XML_CONFERENCE, 4 },
    { XML_CUSTOM1, 5 },
    { XML_CUSTOM2, 6 },
    { XML_CUSTOM3, 7 },
    { XML_CUSTOM4, 8 },
    { XML_CUSTOM5, 9 },
    { XML_EMAIL, 10 },
    { XML_INBOOK, 11 },
    { XML_INCOLLECTION, 12 },
    { XML_INPROCEEDINGS, 13 },
    { XML_JOURNAL, 14 },
    { XML_MANUAL, 15 },
    { XML_MASTERSTHESIS, 16 },
    { XML_MISC, 17 },
    { XML_PHDTHESIS, 18 },
    { XML_PROCEEDINGS, 19 },
    { XML_TECHREPORT, 20 },
    { XML_UNPUBLISHED, 21 },
    { XML_WWW, 22 },
    { XML_TOKEN_INVALID, 0 }
};

// TODO: replace with real property names, when available
const sal_Char* aLevelStylePropNameBibliographyMap[] =
{
    nullptr, "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
    "ParaStyleLevel1", nullptr };

const sal_Bool aAllowedTokenTypesBibliography[] =
{
    sal_True,       // XML_TOK_INDEX_TYPE_ENTRY_TEXT =
    sal_True,       // XML_TOK_INDEX_TYPE_TAB_STOP,
    sal_True,       // XML_TOK_INDEX_TYPE_TEXT,
    sal_True,       // XML_TOK_INDEX_TYPE_PAGE_NUMBER,
    sal_False,      // XML_TOK_INDEX_TYPE_CHAPTER,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_START,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_END,
    sal_True        // XML_TOK_INDEX_TYPE_BIBLIOGRAPHY
};


// table, illustration and object index

// no name map
const SvXMLEnumMapEntry* aLevelNameTableMap = nullptr;

const sal_Char* aLevelStylePropNameTableMap[] =
    { nullptr, "ParaStyleLevel1", nullptr };

const sal_Bool aAllowedTokenTypesTable[] =
{
    sal_True,       // XML_TOK_INDEX_TYPE_ENTRY_TEXT =
    sal_True,       // XML_TOK_INDEX_TYPE_TAB_STOP,
    sal_True,       // XML_TOK_INDEX_TYPE_TEXT,
    sal_True,       // XML_TOK_INDEX_TYPE_PAGE_NUMBER,
    sal_True,       // XML_TOK_INDEX_TYPE_CHAPTER,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_START,
    sal_False,      // XML_TOK_INDEX_TYPE_LINK_END,
    sal_False       // XML_TOK_INDEX_TYPE_BIBLIOGRAPHY
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
