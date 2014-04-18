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

#include <numeric>
#include <boost/tuple/tuple.hpp>

#include <SwStyleNameMapper.hxx>
#include <tools/resmgr.hxx>
#include <poolfmt.hxx>
#include <rcid.hrc>

#ifdef _NEED_TO_DEBUG_MAPPING
#include <stdlib.h>
#endif

extern ResMgr* pSwResMgr;
// Initialise UI names to 0
::std::vector<OUString> *SwStyleNameMapper::pTextUINameArray = 0,
                *SwStyleNameMapper::pListsUINameArray = 0,
                *SwStyleNameMapper::pExtraUINameArray = 0,
                *SwStyleNameMapper::pRegisterUINameArray = 0,
                *SwStyleNameMapper::pDocUINameArray = 0,
                *SwStyleNameMapper::pHTMLUINameArray = 0,
                *SwStyleNameMapper::pFrmFmtUINameArray = 0,
                *SwStyleNameMapper::pChrFmtUINameArray = 0,
                *SwStyleNameMapper::pHTMLChrFmtUINameArray = 0,
                *SwStyleNameMapper::pPageDescUINameArray = 0,
                *SwStyleNameMapper::pNumRuleUINameArray = 0,

// Initialise programmatic names to 0
                *SwStyleNameMapper::pTextProgNameArray = 0,
                *SwStyleNameMapper::pListsProgNameArray = 0,
                *SwStyleNameMapper::pExtraProgNameArray = 0,
                *SwStyleNameMapper::pRegisterProgNameArray = 0,
                *SwStyleNameMapper::pDocProgNameArray = 0,
                *SwStyleNameMapper::pHTMLProgNameArray = 0,
                *SwStyleNameMapper::pFrmFmtProgNameArray = 0,
                *SwStyleNameMapper::pChrFmtProgNameArray = 0,
                *SwStyleNameMapper::pHTMLChrFmtProgNameArray = 0,
                *SwStyleNameMapper::pPageDescProgNameArray = 0,
                *SwStyleNameMapper::pNumRuleProgNameArray = 0;

NameToIdHash    *SwStyleNameMapper::pParaUIMap = 0,
                *SwStyleNameMapper::pCharUIMap = 0,
                *SwStyleNameMapper::pPageUIMap = 0,
                *SwStyleNameMapper::pFrameUIMap = 0,
                *SwStyleNameMapper::pNumRuleUIMap = 0,

                *SwStyleNameMapper::pParaProgMap = 0,
                *SwStyleNameMapper::pCharProgMap = 0,
                *SwStyleNameMapper::pPageProgMap = 0,
                *SwStyleNameMapper::pFrameProgMap = 0,
                *SwStyleNameMapper::pNumRuleProgMap = 0;

// SwTableEntry so we can pass the length to the String CTOR
struct SwTableEntry
{
    sal_uInt8 nLength;
    const sal_Char *pChar;
};

namespace
{

#define ENTRY( s ) { sizeof (s)-1, s }

// Hard coded Programmatic Name tables
const struct SwTableEntry TextProgNameTable [] =
{
    ENTRY( "Standard" ), // RES_POOLCOLL_STANDARD
    ENTRY( "Text body" ),
    ENTRY( "First line indent" ),
    ENTRY( "Hanging indent" ),
    ENTRY( "Text body indent" ),
    ENTRY( "Salutation" ),
    ENTRY( "Signature" ),
    ENTRY( "List Indent" ), // RES_POOLCOLL_CONFRONTATION
    ENTRY( "Marginalia" ),
    ENTRY( "Heading" ),
    ENTRY( "Heading 1" ),
    ENTRY( "Heading 2" ),
    ENTRY( "Heading 3" ),
    ENTRY( "Heading 4" ),
    ENTRY( "Heading 5" ),
    ENTRY( "Heading 6" ),
    ENTRY( "Heading 7" ),
    ENTRY( "Heading 8" ),
    ENTRY( "Heading 9" ),
    ENTRY( "Heading 10" ), // RES_POOLCOLL_TEXT_END
    { 0, NULL }
};

const struct SwTableEntry ListsProgNameTable [] =
{
    ENTRY( "List" ), // STR_POCO_PRGM_NUMBUL_BASE
    ENTRY( "Numbering 1 Start" ), // STR_POCO_PRGM_NUM_LEVEL1S
    ENTRY( "Numbering 1" ),
    ENTRY( "Numbering 1 End" ),
    ENTRY( "Numbering 1 Cont." ),
    ENTRY( "Numbering 2 Start" ),
    ENTRY( "Numbering 2" ),
    ENTRY( "Numbering 2 End" ),
    ENTRY( "Numbering 2 Cont." ),
    ENTRY( "Numbering 3 Start" ),
    ENTRY( "Numbering 3" ),
    ENTRY( "Numbering 3 End" ),
    ENTRY( "Numbering 3 Cont." ),
    ENTRY( "Numbering 4 Start" ),
    ENTRY( "Numbering 4" ),
    ENTRY( "Numbering 4 End" ),
    ENTRY( "Numbering 4 Cont." ),
    ENTRY( "Numbering 5 Start" ),
    ENTRY( "Numbering 5" ),
    ENTRY( "Numbering 5 End" ),
    ENTRY( "Numbering 5 Cont." ),
    ENTRY( "List 1 Start" ),
    ENTRY( "List 1" ),
    ENTRY( "List 1 End" ),
    ENTRY( "List 1 Cont." ),
    ENTRY( "List 2 Start" ),
    ENTRY( "List 2" ),
    ENTRY( "List 2 End" ),
    ENTRY( "List 2 Cont." ),
    ENTRY( "List 3 Start" ),
    ENTRY( "List 3" ),
    ENTRY( "List 3 End" ),
    ENTRY( "List 3 Cont." ),
    ENTRY( "List 4 Start" ),
    ENTRY( "List 4" ),
    ENTRY( "List 4 End" ),
    ENTRY( "List 4 Cont." ),
    ENTRY( "List 5 Start" ),
    ENTRY( "List 5" ),
    ENTRY( "List 5 End" ),
    ENTRY( "List 5 Cont." ), // STR_POCO_PRGM_BUL_NONUM5
    { 0, NULL }
};

const struct SwTableEntry ExtraProgNameTable [] =
{
    ENTRY( "Header" ), // RES_POOLCOLL_EXTRA_BEGIN
    ENTRY( "Header left" ),
    ENTRY( "Header right" ),
    ENTRY( "Footer" ),
    ENTRY( "Footer left" ),
    ENTRY( "Footer right" ),
    ENTRY( "Table Contents" ),
    ENTRY( "Table Heading" ),
    ENTRY( "Caption" ),
    ENTRY( "Illustration" ),
    ENTRY( "Table" ),
    ENTRY( "Text" ),
    ENTRY( "Frame contents" ),
    ENTRY( "Footnote" ),
    ENTRY( "Addressee" ),
    ENTRY( "Sender" ),
    ENTRY( "Endnote" ),
    ENTRY( "Drawing" ), // RES_POOLCOLL_LABEL_DRAWING
    { 0, NULL }
};

const struct SwTableEntry RegisterProgNameTable [] =
{
    ENTRY( "Index" ), // STR_POCO_PRGM_REGISTER_BASE
    ENTRY( "Index Heading" ), // STR_POCO_PRGM_TOX_IDXH
    ENTRY( "Index 1" ),
    ENTRY( "Index 2" ),
    ENTRY( "Index 3" ),
    ENTRY( "Index Separator" ),
    ENTRY( "Contents Heading" ),
    ENTRY( "Contents 1" ),
    ENTRY( "Contents 2" ),
    ENTRY( "Contents 3" ),
    ENTRY( "Contents 4" ),
    ENTRY( "Contents 5" ),
    ENTRY( "User Index Heading" ),
    ENTRY( "User Index 1" ),
    ENTRY( "User Index 2" ),
    ENTRY( "User Index 3" ),
    ENTRY( "User Index 4" ),
    ENTRY( "User Index 5" ),
    ENTRY( "Contents 6" ),
    ENTRY( "Contents 7" ),
    ENTRY( "Contents 8" ),
    ENTRY( "Contents 9" ),
    ENTRY( "Contents 10" ),
    ENTRY( "Illustration Index Heading" ),
    ENTRY( "Illustration Index 1" ),
    ENTRY( "Object index heading" ),
    ENTRY( "Object index 1" ),
    ENTRY( "Table index heading" ),
    ENTRY( "Table index 1" ),
    ENTRY( "Bibliography Heading" ),
    ENTRY( "Bibliography 1" ),
    ENTRY( "User Index 6" ),
    ENTRY( "User Index 7" ),
    ENTRY( "User Index 8" ),
    ENTRY( "User Index 9" ),
    ENTRY( "User Index 10" ), // STR_POCO_PRGM_TOX_USER10
    { 0, NULL }
};

const struct SwTableEntry DocProgNameTable [] =
{
    ENTRY( "Title" ), // STR_POCO_PRGM_DOC_TITEL
    ENTRY( "Subtitle" ),
    { 0, NULL }
};

const struct SwTableEntry HTMLProgNameTable [] =
{
    ENTRY( "Quotations" ),
    ENTRY( "Preformatted Text" ),
    ENTRY( "Horizontal Line" ),
    ENTRY( "List Contents" ),
    ENTRY( "List Heading" ), // STR_POCO_PRGM_HTML_DT
    { 0, NULL }
};

const struct SwTableEntry FrmFmtProgNameTable [] =
{
    ENTRY( "Frame" ), // RES_POOLFRM_FRAME
    ENTRY( "Graphics" ),
    ENTRY( "OLE" ),
    ENTRY( "Formula" ),
    ENTRY( "Marginalia" ),
    ENTRY( "Watermark" ),
    ENTRY( "Labels" ), // RES_POOLFRM_LABEL
    { 0, NULL }
};

const struct SwTableEntry ChrFmtProgNameTable [] =
{
    ENTRY( "Footnote Symbol" ), // RES_POOLCHR_FOOTNOTE
    ENTRY( "Page Number" ),
    ENTRY( "Caption characters" ),
    ENTRY( "Drop Caps" ),
    ENTRY( "Numbering Symbols" ),
    ENTRY( "Bullet Symbols" ),
    ENTRY( "Internet link" ),
    ENTRY( "Visited Internet Link" ),
    ENTRY( "Placeholder" ),
    ENTRY( "Index Link" ),
    ENTRY( "Endnote Symbol" ),
    ENTRY( "Line numbering" ),
    ENTRY( "Main index entry" ),
    ENTRY( "Footnote anchor" ),
    ENTRY( "Endnote anchor" ),
    ENTRY( "Rubies" ), // RES_POOLCHR_RUBYTEXT
    ENTRY( "Vertical Numbering Symbols" ), // RES_POOLCHR_VERT_NUMBER
    { 0, NULL }
};

const struct SwTableEntry HTMLChrFmtProgNameTable [] =
{
    ENTRY( "Emphasis" ), // RES_POOLCHR_HTML_EMPHASIS
    ENTRY( "Citation" ),
    ENTRY( "Strong Emphasis" ),
    ENTRY( "Source Text" ),
    ENTRY( "Example" ),
    ENTRY( "User Entry" ),
    ENTRY( "Variable" ),
    ENTRY( "Definition" ),
    ENTRY( "Teletype" ), // RES_POOLCHR_HTML_TELETYPE
    { 0, NULL }
};

const struct SwTableEntry PageDescProgNameTable [] =
{
    ENTRY( "Standard" ), // STR_POOLPAGE_PRGM_STANDARD
    ENTRY( "First Page" ),
    ENTRY( "Left Page" ),
    ENTRY( "Right Page" ),
    ENTRY( "Envelope" ),
    ENTRY( "Index" ),
    ENTRY( "HTML" ),
    ENTRY( "Footnote" ),
    ENTRY( "Endnote" ), // STR_POOLPAGE_PRGM_ENDNOTE
    ENTRY( "Landscape" ),
    { 0, NULL }
};

const struct SwTableEntry NumRuleProgNameTable [] =
{
    ENTRY( "Numbering 1" ), // STR_POOLNUMRULE_PRGM_NUM1
    ENTRY( "Numbering 2" ),
    ENTRY( "Numbering 3" ),
    ENTRY( "Numbering 4" ),
    ENTRY( "Numbering 5" ),
    ENTRY( "List 1" ),
    ENTRY( "List 2" ),
    ENTRY( "List 3" ),
    ENTRY( "List 4" ),
    ENTRY( "List 5" ), // STR_POOLNUMRULE_PRGM_BUL5
    { 0, NULL }
};
#undef ENTRY

static ::std::vector<OUString>*
lcl_NewUINameArray(sal_uInt16 nStt, sal_uInt16 const nEnd)
{
    ::std::vector<OUString> *const pNameArray = new ::std::vector<OUString>;
    pNameArray->reserve(nEnd - nStt);
    while( nStt < nEnd )
    {
        const ResId aRId( nStt, *pSwResMgr );
        pNameArray->push_back(OUString(aRId));
        ++nStt;
    }
    return pNameArray;
}

static ::std::vector<OUString>*
lcl_NewProgNameArray(const SwTableEntry *pTable, sal_uInt8 const nCount)
{
    ::std::vector<OUString> *const pProgNameArray = new ::std::vector<OUString>;
    pProgNameArray->reserve(nCount);
    while (pTable->nLength)
    {
        pProgNameArray->push_back(OUString(
                pTable->pChar, pTable->nLength, RTL_TEXTENCODING_ASCII_US));
        pTable++;
    }
    return pProgNameArray;
}

static OUString
lcl_GetSpecialExtraName(const OUString& rExtraName, const bool bIsUIName )
{
    const ::std::vector<OUString>& rExtraArr = bIsUIName
        ? SwStyleNameMapper::GetExtraUINameArray()
        : SwStyleNameMapper::GetExtraProgNameArray();
    static const sal_uInt16 nIds[] =
    {
        RES_POOLCOLL_LABEL_DRAWING - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_ABB - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_TABLE - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_FRAME- RES_POOLCOLL_EXTRA_BEGIN,
        0
    };
    const sal_uInt16 * pIds;
    for ( pIds = nIds; *pIds; ++pIds)
    {
        if (rExtraName == rExtraArr[ *pIds ])
        {
            return bIsUIName
                ? SwStyleNameMapper::GetExtraProgNameArray()[*pIds]
                : SwStyleNameMapper::GetExtraUINameArray()[*pIds];
        }
    }
    return rExtraName;
}

static bool lcl_SuffixIsUser(const OUString & rString)
{
    const sal_Unicode *pChar = rString.getStr();
    sal_Int32 nLen = rString.getLength();
    bool bRet = false;
    if( nLen > 8 &&
        pChar[nLen-7] == ' ' &&
        pChar[nLen-6] == '(' &&
        pChar[nLen-5] == 'u' &&
        pChar[nLen-4] == 's' &&
        pChar[nLen-3] == 'e' &&
        pChar[nLen-2] == 'r' &&
        pChar[nLen-1] == ')' )
        bRet = true;
    return bRet;
}

static void lcl_CheckSuffixAndDelete(OUString & rString)
{
    if (lcl_SuffixIsUser(rString))
    {
        rString = rString.copy(0, rString.getLength() - 7);
    }
}

typedef boost::tuple<sal_uInt16, sal_uInt16, const ::std::vector<OUString>& (*)() > NameArrayIndexTuple_t;

static sal_uInt16 lcl_AccumulateIndexCount( sal_uInt16 nSum, NameArrayIndexTuple_t const tuple ){
    // Return running sum + (index end) - (index start)
    return nSum + boost::get<1>( tuple ) - boost::get<0>( tuple );
}
}

#ifdef _NEED_TO_DEBUG_MAPPING
void SwStyleNameMapper::testNameTable( SwGetPoolIdFromName const nFamily, sal_uInt16 const nStartIndex, sal_uInt16 const nEndIndex )
{
    sal_uInt16 nIndex;
    sal_uInt16 nId;

    for ( nIndex = 0, nId = nStartIndex ; nId < nEndIndex ; nId++,nIndex++ )
    {
        OUString aString, bString;
        FillUIName ( nId, aString );
        bString = GetProgName ( nFamily, aString );
        sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nFamily );
        FillProgName ( nNewId, aString );
        bString = GetUIName ( aString, nFamily );
        nNewId = GetPoolIdFromUIName ( aString, nFamily );
        if ( nNewId != nId )
            abort();
    }
}
#endif

const NameToIdHash & SwStyleNameMapper::getHashTable ( SwGetPoolIdFromName eFlags, bool bProgName )
{
    // pHashPointer is a pointer to a pointer which stores the UI/prog name array
    NameToIdHash **pHashPointer = 0;
    // Stores tuples representing (index start, index end, pointer to function which returns ref to name array)
    ::std::vector<NameArrayIndexTuple_t> vIndexes;

    switch ( eFlags )
    {
        case nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL:
        {
            pHashPointer = bProgName ? &pParaProgMap : &pParaUIMap;
            vIndexes.push_back( boost::make_tuple(RES_POOLCOLL_TEXT_BEGIN, RES_POOLCOLL_TEXT_END, bProgName ? &GetTextProgNameArray : &GetTextUINameArray) );
            vIndexes.push_back( boost::make_tuple(RES_POOLCOLL_LISTS_BEGIN, RES_POOLCOLL_LISTS_END, bProgName ? &GetListsProgNameArray : &GetListsUINameArray) );
            vIndexes.push_back( boost::make_tuple(RES_POOLCOLL_EXTRA_BEGIN, RES_POOLCOLL_EXTRA_END, bProgName ? &GetExtraProgNameArray : &GetExtraUINameArray) );
            vIndexes.push_back( boost::make_tuple(RES_POOLCOLL_REGISTER_BEGIN, RES_POOLCOLL_REGISTER_END, bProgName ? &GetRegisterProgNameArray : &GetRegisterUINameArray) );
            vIndexes.push_back( boost::make_tuple(RES_POOLCOLL_DOC_BEGIN, RES_POOLCOLL_DOC_END, bProgName ? &GetDocProgNameArray : &GetDocUINameArray) );
            vIndexes.push_back( boost::make_tuple(RES_POOLCOLL_HTML_BEGIN, RES_POOLCOLL_HTML_END, bProgName ? &GetHTMLProgNameArray : &GetHTMLUINameArray) );
        }
        break;
        case nsSwGetPoolIdFromName::GET_POOLID_CHRFMT:
        {
            pHashPointer = bProgName ? &pCharProgMap : &pCharUIMap;
            vIndexes.push_back( boost::make_tuple(RES_POOLCHR_NORMAL_BEGIN, RES_POOLCHR_NORMAL_END, bProgName ? &GetChrFmtProgNameArray : &GetChrFmtUINameArray) );
            vIndexes.push_back( boost::make_tuple(RES_POOLCHR_HTML_BEGIN, RES_POOLCHR_HTML_END, bProgName ? &GetHTMLChrFmtProgNameArray : &GetHTMLChrFmtUINameArray) );
        }
        break;
        case nsSwGetPoolIdFromName::GET_POOLID_FRMFMT:
        {
            pHashPointer = bProgName ? &pFrameProgMap : &pFrameUIMap;
            vIndexes.push_back( boost::make_tuple(RES_POOLFRM_BEGIN, RES_POOLFRM_END, bProgName ? &GetFrmFmtProgNameArray : &GetFrmFmtUINameArray) );
        }
        break;
        case nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC:
        {
            pHashPointer = bProgName ? &pPageProgMap : &pPageUIMap;
            vIndexes.push_back( boost::make_tuple(RES_POOLPAGE_BEGIN, RES_POOLPAGE_END, bProgName ? &GetPageDescProgNameArray : &GetPageDescUINameArray) );
        }
        break;
        case nsSwGetPoolIdFromName::GET_POOLID_NUMRULE:
        {
            pHashPointer = bProgName ? &pNumRuleProgMap : &pNumRuleUIMap;
            vIndexes.push_back( boost::make_tuple(RES_POOLNUMRULE_BEGIN, RES_POOLNUMRULE_END, bProgName ? &GetNumRuleProgNameArray : &GetNumRuleUINameArray) );
        }
        break;
        default:
        {
            // TODO: Is there a better way of failing here?
            *pHashPointer = new NameToIdHash( 0 );
            return **pHashPointer;
        }
        break;
    }

    // Proceed if we have a pointer to a hash, and the hash hasn't already been populated
    if ( pHashPointer && !*pHashPointer )
    {
        // Compute the size of the hash we need to build
        sal_uInt16 nSize = std::accumulate( vIndexes.begin(), vIndexes.end(), 0, lcl_AccumulateIndexCount );

        NameToIdHash *pHash = new NameToIdHash( nSize );
        for ( ::std::vector<NameArrayIndexTuple_t>::iterator entry = vIndexes.begin(); entry != vIndexes.end(); ++entry )
        {
            // Get a pointer to the function which will populate pStrings
            const ::std::vector<OUString>& (*pStringsFetchFunc)() = boost::get<2>( *entry );
            if ( pStringsFetchFunc )
            {
                const ::std::vector<OUString>& pStrings = pStringsFetchFunc();
                sal_uInt16 nIndex, nId;
                for ( nIndex = 0, nId = boost::get<0>( *entry ) ; nId < boost::get<1>( *entry ) ; nId++, nIndex++ )
                    (*pHash)[pStrings[nIndex]] = nId;
            }
        }

        *pHashPointer = pHash;
    }

#ifdef _NEED_TO_DEBUG_MAPPING
    static bool bTested = false;
    if ( !bTested )
    {
        bTested = true;

        testNameTable( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, RES_POOLCOLL_TEXT_BEGIN, RES_POOLCOLL_TEXT_END );
        testNameTable( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, RES_POOLCOLL_LISTS_BEGIN, RES_POOLCOLL_LISTS_END );
        testNameTable( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, RES_POOLCOLL_EXTRA_BEGIN, RES_POOLCOLL_EXTRA_END );
        testNameTable( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, RES_POOLCOLL_REGISTER_BEGIN, RES_POOLCOLL_REGISTER_END );
        testNameTable( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, RES_POOLCOLL_DOC_BEGIN, RES_POOLCOLL_DOC_END );
        testNameTable( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, RES_POOLCOLL_HTML_BEGIN, RES_POOLCOLL_HTML_END );
        testNameTable( nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, RES_POOLCHR_NORMAL_BEGIN, RES_POOLCHR_NORMAL_END );
        testNameTable( nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, RES_POOLCHR_HTML_BEGIN, RES_POOLCHR_HTML_END );
        testNameTable( nsSwGetPoolIdFromName::GET_POOLID_FRMFMT, RES_POOLFRM_BEGIN, RES_POOLFRM_END );
        testNameTable( nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, RES_POOLPAGE_BEGIN, RES_POOLPAGE_END );
        testNameTable( nsSwGetPoolIdFromName::GET_POOLID_NUMRULE, RES_POOLNUMRULE_BEGIN, RES_POOLNUMRULE_END );
    }
#endif
    return **pHashPointer;
}

// This gets the UI name from the programmatic name
const OUString& SwStyleNameMapper::GetUIName(const OUString& rName,
                                             SwGetPoolIdFromName const eFlags)
{
    sal_uInt16 nId = GetPoolIdFromProgName ( rName, eFlags );
    return nId != USHRT_MAX ? GetUIName( nId, rName ) : rName;
}

// Get the programmatic name from the UI name
const OUString& SwStyleNameMapper::GetProgName(
        const OUString& rName, SwGetPoolIdFromName const eFlags)
{
    sal_uInt16 nId = GetPoolIdFromUIName ( rName, eFlags );
    return nId != USHRT_MAX ? GetProgName( nId, rName ) : rName;
}

// Get the programmatic name from the UI name in rName and put it into rFillName
void SwStyleNameMapper::FillProgName(
        const OUString& rName, OUString& rFillName,
        SwGetPoolIdFromName const eFlags, bool const bDisambiguate)
{
    sal_uInt16 nId = GetPoolIdFromUIName ( rName, eFlags );
    if ( bDisambiguate && nId == USHRT_MAX )
    {
        // rName isn't in our UI name table...check if it's in the programmatic one
        nId = GetPoolIdFromProgName ( rName, eFlags );

        rFillName = rName;
        if (nId == USHRT_MAX )
        {
            // It isn't ...make sure the suffix isn't already " (user)"...if it is,
            // we need to add another one
            if (lcl_SuffixIsUser(rFillName))
                rFillName += " (user)";
        }
        else
        {
            // It's in the programmatic name table...append suffix
            rFillName += " (user)";
        }
    }
    else
    {
        // If we aren't trying to disambiguate, then just do a normal fill
        fillNameFromId(nId, rFillName, true);
    }
}

// Get the UI name from the programmatic name in rName and put it into rFillName
void SwStyleNameMapper::FillUIName(
        const OUString& rName, OUString& rFillName,
        SwGetPoolIdFromName const eFlags, bool const bDisambiguate)
{
    sal_uInt16 nId = GetPoolIdFromProgName ( rName, eFlags );
    if ( bDisambiguate && nId == USHRT_MAX )
    {
        rFillName = rName;
        // rName isn't in our Prog name table...check if it has a " (user)" suffix, if so remove it
        lcl_CheckSuffixAndDelete ( rFillName );
    }
    else
    {
        // If we aren't trying to disambiguate, then just do a normal fill
        fillNameFromId(nId, rFillName, false);
    }
}

const OUString& SwStyleNameMapper::getNameFromId(
        sal_uInt16 const nId, const OUString& rFillName, bool const bProgName)
{
    sal_uInt16 nStt = 0;
    const ::std::vector<OUString>* pStrArr = 0;

    switch( (USER_FMT | COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID) & nId )
    {
    case COLL_TEXT_BITS:
        if( RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END )
        {
            pStrArr = bProgName ? &GetTextProgNameArray() : &GetTextUINameArray();
            nStt = RES_POOLCOLL_TEXT_BEGIN;
        }
        break;
    case COLL_LISTS_BITS:
        if( RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END )
        {
            pStrArr = bProgName ? &GetListsProgNameArray() : &GetListsUINameArray();
            nStt = RES_POOLCOLL_LISTS_BEGIN;
        }
        break;
    case COLL_EXTRA_BITS:
        if( RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END )
        {
            pStrArr = bProgName ? &GetExtraProgNameArray() : &GetExtraUINameArray();
            nStt = RES_POOLCOLL_EXTRA_BEGIN;
        }
        break;
    case COLL_REGISTER_BITS:
        if( RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END )
        {
            pStrArr = bProgName ? &GetRegisterProgNameArray() : &GetRegisterUINameArray();
            nStt = RES_POOLCOLL_REGISTER_BEGIN;
        }
        break;
    case COLL_DOC_BITS:
        if( RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END )
        {
            pStrArr = bProgName ? &GetDocProgNameArray() : &GetDocUINameArray();
            nStt = RES_POOLCOLL_DOC_BEGIN;
        }
        break;
    case COLL_HTML_BITS:
        if( RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END )
        {
            pStrArr = bProgName ? &GetHTMLProgNameArray() : &GetHTMLUINameArray();
            nStt = RES_POOLCOLL_HTML_BEGIN;
        }
        break;
    case POOLGRP_CHARFMT:
        if( RES_POOLCHR_NORMAL_BEGIN <= nId && nId < RES_POOLCHR_NORMAL_END )
        {
            pStrArr = bProgName ? &GetChrFmtProgNameArray() : &GetChrFmtUINameArray();
            nStt = RES_POOLCHR_NORMAL_BEGIN;
        }
        else if( RES_POOLCHR_HTML_BEGIN <= nId && nId < RES_POOLCHR_HTML_END )
        {
            pStrArr = bProgName ? &GetHTMLChrFmtProgNameArray() : &GetHTMLChrFmtUINameArray();
            nStt = RES_POOLCHR_HTML_BEGIN;
        }
        break;
    case POOLGRP_FRAMEFMT:
        if( RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END )
        {
            pStrArr = bProgName ? &GetFrmFmtProgNameArray() : &GetFrmFmtUINameArray();
            nStt = RES_POOLFRM_BEGIN;
        }
        break;
    case POOLGRP_PAGEDESC:
        if( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END )
        {
            pStrArr = bProgName ? &GetPageDescProgNameArray() : &GetPageDescUINameArray();
            nStt = RES_POOLPAGE_BEGIN;
        }
        break;
    case POOLGRP_NUMRULE:
        if( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END )
        {
            pStrArr = bProgName ? &GetNumRuleProgNameArray() : &GetNumRuleUINameArray();
            nStt = RES_POOLNUMRULE_BEGIN;
        }
        break;
    }
    return pStrArr ? pStrArr->operator[](nId - nStt) : rFillName;
}

void SwStyleNameMapper::fillNameFromId(
        sal_uInt16 const nId, OUString& rFillName, bool bProgName)
{
    rFillName = getNameFromId(nId, rFillName, bProgName);
}

// Get the UI name from the pool ID
void SwStyleNameMapper::FillUIName(sal_uInt16 const nId, OUString& rFillName)
{
    fillNameFromId(nId, rFillName, false);
}

// Get the UI name from the pool ID
const OUString& SwStyleNameMapper::GetUIName(
        sal_uInt16 const nId, const OUString& rName)
{
    return getNameFromId(nId, rName, false);
}

// Get the programmatic name from the pool ID
void SwStyleNameMapper::FillProgName(sal_uInt16 nId, OUString& rFillName)
{
    fillNameFromId(nId, rFillName, true);
}

// Get the programmatic name from the pool ID
const OUString&
SwStyleNameMapper::GetProgName(sal_uInt16 const nId, const OUString& rName)
{
    return getNameFromId(nId, rName, true);
}

// This gets the PoolId from the UI Name
sal_uInt16 SwStyleNameMapper::GetPoolIdFromUIName(
        const OUString& rName, SwGetPoolIdFromName const eFlags)
{
    const NameToIdHash & rHashMap = getHashTable ( eFlags, false );
    NameToIdHash::const_iterator aIter = rHashMap.find(rName);
    return aIter != rHashMap.end() ? (*aIter).second : USHRT_MAX;
}

// Get the Pool ID from the programmatic name
sal_uInt16 SwStyleNameMapper::GetPoolIdFromProgName(
            const OUString& rName, SwGetPoolIdFromName const eFlags)
{
    const NameToIdHash & rHashMap = getHashTable ( eFlags, true );
    NameToIdHash::const_iterator aIter = rHashMap.find(rName);
    return aIter != rHashMap.end() ? (*aIter).second : USHRT_MAX;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetTextUINameArray()
{
    if (!pTextUINameArray)
        pTextUINameArray = lcl_NewUINameArray( RC_POOLCOLL_TEXT_BEGIN,
            RC_POOLCOLL_TEXT_BEGIN + (RES_POOLCOLL_TEXT_END - RES_POOLCOLL_TEXT_BEGIN) );
    return *pTextUINameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetListsUINameArray()
{
    if (!pListsUINameArray)
        pListsUINameArray = lcl_NewUINameArray( RC_POOLCOLL_LISTS_BEGIN,
            RC_POOLCOLL_LISTS_BEGIN + (RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN) );
    return *pListsUINameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetExtraUINameArray()
{
    if (!pExtraUINameArray)
        pExtraUINameArray = lcl_NewUINameArray( RC_POOLCOLL_EXTRA_BEGIN,
            RC_POOLCOLL_EXTRA_BEGIN + (RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN) );
    return *pExtraUINameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetRegisterUINameArray()
{
    if (!pRegisterUINameArray)
        pRegisterUINameArray = lcl_NewUINameArray( RC_POOLCOLL_REGISTER_BEGIN,
            RC_POOLCOLL_REGISTER_BEGIN + (RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN) );
    return *pRegisterUINameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetDocUINameArray()
{
    if (!pDocUINameArray)
        pDocUINameArray = lcl_NewUINameArray( RC_POOLCOLL_DOC_BEGIN,
            RC_POOLCOLL_DOC_BEGIN + (RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN) );
    return *pDocUINameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetHTMLUINameArray()
{
    if (!pHTMLUINameArray)
        pHTMLUINameArray = lcl_NewUINameArray( RC_POOLCOLL_HTML_BEGIN,
            RC_POOLCOLL_HTML_BEGIN + (RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN) );
    return *pHTMLUINameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetFrmFmtUINameArray()
{
    if (!pFrmFmtUINameArray)
        pFrmFmtUINameArray = lcl_NewUINameArray( RC_POOLFRMFMT_BEGIN,
            RC_POOLFRMFMT_BEGIN + (RES_POOLFRM_END - RES_POOLFRM_BEGIN) );
    return *pFrmFmtUINameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetChrFmtUINameArray()
{
    if (!pChrFmtUINameArray)
        pChrFmtUINameArray = lcl_NewUINameArray( RC_POOLCHRFMT_BEGIN,
            RC_POOLCHRFMT_BEGIN + (RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN) );
    return *pChrFmtUINameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetHTMLChrFmtUINameArray()
{
    if (!pHTMLChrFmtUINameArray)
        pHTMLChrFmtUINameArray = lcl_NewUINameArray( RC_POOLCHRFMT_HTML_BEGIN,
            RC_POOLCHRFMT_HTML_BEGIN + (RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN) );
    return *pHTMLChrFmtUINameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetPageDescUINameArray()
{
    if (!pPageDescUINameArray)
        pPageDescUINameArray = lcl_NewUINameArray( RC_POOLPAGEDESC_BEGIN,
            RC_POOLPAGEDESC_BEGIN + (RES_POOLPAGE_END - RES_POOLPAGE_BEGIN) );
    return *pPageDescUINameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetNumRuleUINameArray()
{
    if (!pNumRuleUINameArray)
        pNumRuleUINameArray = lcl_NewUINameArray( RC_POOLNUMRULE_BEGIN,
            RC_POOLNUMRULE_BEGIN + (RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN) );
    return *pNumRuleUINameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetTextProgNameArray()
{
    if (!pTextProgNameArray)
        pTextProgNameArray = lcl_NewProgNameArray(TextProgNameTable,
            sizeof ( TextProgNameTable ) / sizeof ( SwTableEntry ) );
    return *pTextProgNameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetListsProgNameArray()
{
    if (!pListsProgNameArray)
        pListsProgNameArray = lcl_NewProgNameArray( ListsProgNameTable,
            sizeof ( ListsProgNameTable ) / sizeof ( SwTableEntry ) );
    return *pListsProgNameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetExtraProgNameArray()
{
    if (!pExtraProgNameArray)
        pExtraProgNameArray = lcl_NewProgNameArray( ExtraProgNameTable,
            sizeof ( ExtraProgNameTable ) / sizeof ( SwTableEntry ) );
    return *pExtraProgNameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetRegisterProgNameArray()
{
    if (!pRegisterProgNameArray)
        pRegisterProgNameArray = lcl_NewProgNameArray( RegisterProgNameTable,
            sizeof ( RegisterProgNameTable ) / sizeof ( SwTableEntry ) );
    return *pRegisterProgNameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetDocProgNameArray()
{
    if (!pDocProgNameArray)
        pDocProgNameArray = lcl_NewProgNameArray( DocProgNameTable,
            sizeof ( DocProgNameTable ) / sizeof ( SwTableEntry ) );
    return *pDocProgNameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetHTMLProgNameArray()
{
    if (!pHTMLProgNameArray)
        pHTMLProgNameArray = lcl_NewProgNameArray( HTMLProgNameTable,
            sizeof ( HTMLProgNameTable ) / sizeof ( SwTableEntry ) );
    return *pHTMLProgNameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetFrmFmtProgNameArray()
{
    if (!pFrmFmtProgNameArray)
        pFrmFmtProgNameArray = lcl_NewProgNameArray( FrmFmtProgNameTable,
            sizeof ( FrmFmtProgNameTable ) / sizeof ( SwTableEntry ) );
    return *pFrmFmtProgNameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetChrFmtProgNameArray()
{
    if (!pChrFmtProgNameArray)
        pChrFmtProgNameArray = lcl_NewProgNameArray( ChrFmtProgNameTable,
            sizeof ( ChrFmtProgNameTable ) / sizeof ( SwTableEntry ) );
    return *pChrFmtProgNameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetHTMLChrFmtProgNameArray()
{
    if (!pHTMLChrFmtProgNameArray)
        pHTMLChrFmtProgNameArray = lcl_NewProgNameArray( HTMLChrFmtProgNameTable,
            sizeof ( HTMLChrFmtProgNameTable ) / sizeof ( SwTableEntry ) );
    return *pHTMLChrFmtProgNameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetPageDescProgNameArray()
{
    if (!pPageDescProgNameArray)
        pPageDescProgNameArray = lcl_NewProgNameArray( PageDescProgNameTable,
            sizeof ( PageDescProgNameTable ) / sizeof ( SwTableEntry ) );
    return *pPageDescProgNameArray;
}

const ::std::vector<OUString>& SwStyleNameMapper::GetNumRuleProgNameArray()
{
    if (!pNumRuleProgNameArray)
        pNumRuleProgNameArray = lcl_NewProgNameArray( NumRuleProgNameTable,
            sizeof ( NumRuleProgNameTable ) / sizeof ( SwTableEntry ) );
    return *pNumRuleProgNameArray;
}

const OUString
SwStyleNameMapper::GetSpecialExtraProgName(const OUString& rExtraUIName)
{
    return lcl_GetSpecialExtraName( rExtraUIName, true );
}

const OUString
SwStyleNameMapper::GetSpecialExtraUIName(const OUString& rExtraProgName)
{
    return lcl_GetSpecialExtraName( rExtraProgName, false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
