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
#include <tuple>

#include <SwStyleNameMapper.hxx>
#include <poolfmt.hxx>
#include <rcid.hrc>

#ifdef _NEED_TO_DEBUG_MAPPING
#include <stdlib.h>
#endif

// Initialise programmatic names to 0
std::vector<OUString>
                *SwStyleNameMapper::s_pTextProgNameArray = nullptr,
                *SwStyleNameMapper::s_pListsProgNameArray = nullptr,
                *SwStyleNameMapper::s_pExtraProgNameArray = nullptr,
                *SwStyleNameMapper::s_pRegisterProgNameArray = nullptr,
                *SwStyleNameMapper::s_pDocProgNameArray = nullptr,
                *SwStyleNameMapper::s_pHTMLProgNameArray = nullptr,
                *SwStyleNameMapper::s_pFrameFormatProgNameArray = nullptr,
                *SwStyleNameMapper::s_pChrFormatProgNameArray = nullptr,
                *SwStyleNameMapper::s_pHTMLChrFormatProgNameArray = nullptr,
                *SwStyleNameMapper::s_pPageDescProgNameArray = nullptr,
                *SwStyleNameMapper::s_pNumRuleProgNameArray = nullptr,
                *SwStyleNameMapper::s_pTableStyleProgNameArray = nullptr,
                *SwStyleNameMapper::s_pCellStyleProgNameArray = nullptr;

NameToIdHash    *SwStyleNameMapper::s_pParaUIMap = nullptr,
                *SwStyleNameMapper::s_pCharUIMap = nullptr,
                *SwStyleNameMapper::s_pPageUIMap = nullptr,
                *SwStyleNameMapper::s_pFrameUIMap = nullptr,
                *SwStyleNameMapper::s_pNumRuleUIMap = nullptr,
                *SwStyleNameMapper::s_pTableStyleUIMap = nullptr,
                *SwStyleNameMapper::s_pCellStyleUIMap = nullptr,

                *SwStyleNameMapper::s_pParaProgMap = nullptr,
                *SwStyleNameMapper::s_pCharProgMap = nullptr,
                *SwStyleNameMapper::s_pPageProgMap = nullptr,
                *SwStyleNameMapper::s_pFrameProgMap = nullptr,
                *SwStyleNameMapper::s_pNumRuleProgMap = nullptr,
                *SwStyleNameMapper::s_pTableStyleProgMap = nullptr,
                *SwStyleNameMapper::s_pCellStyleProgMap = nullptr;

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
    { 0, nullptr }
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
    { 0, nullptr }
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
    { 0, nullptr }
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
    { 0, nullptr }
};

const struct SwTableEntry DocProgNameTable [] =
{
    ENTRY( "Title" ), // STR_POCO_PRGM_DOC_TITEL
    ENTRY( "Subtitle" ),
    { 0, nullptr }
};

const struct SwTableEntry HTMLProgNameTable [] =
{
    ENTRY( "Quotations" ),
    ENTRY( "Preformatted Text" ),
    ENTRY( "Horizontal Line" ),
    ENTRY( "List Contents" ),
    ENTRY( "List Heading" ), // STR_POCO_PRGM_HTML_DT
    { 0, nullptr }
};

const struct SwTableEntry FrameFormatProgNameTable [] =
{
    ENTRY( "Frame" ), // RES_POOLFRM_FRAME
    ENTRY( "Graphics" ),
    ENTRY( "OLE" ),
    ENTRY( "Formula" ),
    ENTRY( "Marginalia" ),
    ENTRY( "Watermark" ),
    ENTRY( "Labels" ), // RES_POOLFRM_LABEL
    { 0, nullptr }
};

const struct SwTableEntry ChrFormatProgNameTable [] =
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
    { 0, nullptr }
};

const struct SwTableEntry HTMLChrFormatProgNameTable [] =
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
    { 0, nullptr }
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
    { 0, nullptr }
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
    { 0, nullptr }
};

const struct SwTableEntry TableStyleProgNameTable [] =
{
    ENTRY( "Default Style" ),     // RES_POOLTABLESTYLE_DEFAULT
    ENTRY( "3D" ),                // RES_POOLTABLESTYLE_3D
    ENTRY( "Black 1" ),           // RES_POOLTABLESTYLE_BLACK1
    ENTRY( "Black 2" ),           // RES_POOLTABLESTYLE_BLACK2
    ENTRY( "Blue" ),              // RES_POOLTABLESTYLE_BLUE
    ENTRY( "Brown" ),             // RES_POOLTABLESTYLE_BROWN
    ENTRY( "Currency" ),          // RES_POOLTABLESTYLE_CURRENCY
    ENTRY( "Currency 3D" ),       // RES_POOLTABLESTYLE_CURRENCY_3D
    ENTRY( "Currency Gray" ),     // RES_POOLTABLESTYLE_CURRENCY_GRAY
    ENTRY( "Currency Lavender" ), // RES_POOLTABLESTYLE_CURRENCY_LAVENDER
    ENTRY( "Currency Turquoise" ),// RES_POOLTABLESTYLE_CURRENCY_TURQUOISE
    ENTRY( "Gray" ),              // RES_POOLTABLESTYLE_GRAY
    ENTRY( "Green" ),             // RES_POOLTABLESTYLE_GREEN
    ENTRY( "Lavender" ),          // RES_POOLTABLESTYLE_LAVENDER
    ENTRY( "Red" ),               // RES_POOLTABLESTYLE_RED
    ENTRY( "Turquoise" ),         // RES_POOLTABLESTYLE_TURQUOISE
    ENTRY( "Yellow" ),            // RES_POOLTABLESTYLE_YELLOW
    { 0, nullptr }
};
#undef ENTRY

std::vector<OUString>*
lcl_NewProgNameArray(const SwTableEntry *pTable, sal_uInt8 const nCount)
{
    std::vector<OUString> *const pProgNameArray = new std::vector<OUString>;
    pProgNameArray->reserve(nCount);
    while (pTable->nLength)
    {
        pProgNameArray->push_back(OUString(
                pTable->pChar, pTable->nLength, RTL_TEXTENCODING_ASCII_US));
        pTable++;
    }
    return pProgNameArray;
}

OUString
lcl_GetSpecialExtraName(const OUString& rExtraName, const bool bIsUIName )
{
    const std::vector<OUString>& rExtraArr = bIsUIName
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

bool lcl_SuffixIsUser(const OUString & rString)
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

void lcl_CheckSuffixAndDelete(OUString & rString)
{
    if (lcl_SuffixIsUser(rString))
    {
        rString = rString.copy(0, rString.getLength() - 7);
    }
}

typedef std::tuple<sal_uInt16, sal_uInt16, const std::vector<OUString>& (*)() > NameArrayIndexTuple_t;

sal_uInt16 lcl_AccumulateIndexCount( sal_uInt16 nSum, const NameArrayIndexTuple_t& tuple ){
    // Return running sum + (index end) - (index start)
    return nSum + std::get<1>( tuple ) - std::get<0>( tuple );
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
    NameToIdHash **pHashPointer = nullptr;
    // Stores tuples representing (index start, index end, pointer to function which returns ref to name array)
    std::vector<NameArrayIndexTuple_t> vIndexes;

    switch ( eFlags )
    {
        case SwGetPoolIdFromName::TxtColl:
        {
            pHashPointer = bProgName ? &s_pParaProgMap : &s_pParaUIMap;
            vIndexes.push_back( std::make_tuple(RES_POOLCOLL_TEXT_BEGIN, RES_POOLCOLL_TEXT_END, bProgName ? &GetTextProgNameArray : &GetTextUINameArray) );
            vIndexes.push_back( std::make_tuple(RES_POOLCOLL_LISTS_BEGIN, RES_POOLCOLL_LISTS_END, bProgName ? &GetListsProgNameArray : &GetListsUINameArray) );
            vIndexes.push_back( std::make_tuple(RES_POOLCOLL_EXTRA_BEGIN, RES_POOLCOLL_EXTRA_END, bProgName ? &GetExtraProgNameArray : &GetExtraUINameArray) );
            vIndexes.push_back( std::make_tuple(RES_POOLCOLL_REGISTER_BEGIN, RES_POOLCOLL_REGISTER_END, bProgName ? &GetRegisterProgNameArray : &GetRegisterUINameArray) );
            vIndexes.push_back( std::make_tuple(RES_POOLCOLL_DOC_BEGIN, RES_POOLCOLL_DOC_END, bProgName ? &GetDocProgNameArray : &GetDocUINameArray) );
            vIndexes.push_back( std::make_tuple(RES_POOLCOLL_HTML_BEGIN, RES_POOLCOLL_HTML_END, bProgName ? &GetHTMLProgNameArray : &GetHTMLUINameArray) );
        }
        break;
        case SwGetPoolIdFromName::ChrFmt:
        {
            pHashPointer = bProgName ? &s_pCharProgMap : &s_pCharUIMap;
            vIndexes.push_back( std::make_tuple(RES_POOLCHR_NORMAL_BEGIN, RES_POOLCHR_NORMAL_END, bProgName ? &GetChrFormatProgNameArray : &GetChrFormatUINameArray) );
            vIndexes.push_back( std::make_tuple(RES_POOLCHR_HTML_BEGIN, RES_POOLCHR_HTML_END, bProgName ? &GetHTMLChrFormatProgNameArray : &GetHTMLChrFormatUINameArray) );
        }
        break;
        case SwGetPoolIdFromName::FrmFmt:
        {
            pHashPointer = bProgName ? &s_pFrameProgMap : &s_pFrameUIMap;
            vIndexes.push_back( std::make_tuple(RES_POOLFRM_BEGIN, RES_POOLFRM_END, bProgName ? &GetFrameFormatProgNameArray : &GetFrameFormatUINameArray) );
        }
        break;
        case SwGetPoolIdFromName::PageDesc:
        {
            pHashPointer = bProgName ? &s_pPageProgMap : &s_pPageUIMap;
            vIndexes.push_back( std::make_tuple(RES_POOLPAGE_BEGIN, RES_POOLPAGE_END, bProgName ? &GetPageDescProgNameArray : &GetPageDescUINameArray) );
        }
        break;
        case SwGetPoolIdFromName::NumRule:
        {
            pHashPointer = bProgName ? &s_pNumRuleProgMap : &s_pNumRuleUIMap;
            vIndexes.push_back( std::make_tuple(RES_POOLNUMRULE_BEGIN, RES_POOLNUMRULE_END, bProgName ? &GetNumRuleProgNameArray : &GetNumRuleUINameArray) );
        }
        break;
        case SwGetPoolIdFromName::TabStyle:
        {
            pHashPointer = bProgName ? &s_pTableStyleProgMap : &s_pTableStyleUIMap;
            vIndexes.push_back( std::make_tuple(RES_POOLTABSTYLE_BEGIN, RES_POOLTABSTYLE_END, bProgName ? &GetTableStyleProgNameArray : &GetTableStyleUINameArray) );
        }
        break;
        case SwGetPoolIdFromName::CellStyle:
        {
            pHashPointer = bProgName ? &s_pCellStyleProgMap : &s_pCellStyleUIMap;
            vIndexes.push_back( std::make_tuple(RES_POOLCELLSTYLE_BEGIN, RES_POOLCELLSTYLE_END, bProgName ? &GetCellStyleProgNameArray : &GetCellStyleUINameArray) );
        }
        break;
        default:
            assert(false && "unknown pool family");
    }

    // Proceed if we have a pointer to a hash, and the hash hasn't already been populated
    assert(pHashPointer && "null hash pointer");
    if (!*pHashPointer )
    {
        // Compute the size of the hash we need to build
        sal_uInt16 nSize = std::accumulate( vIndexes.begin(), vIndexes.end(), 0, lcl_AccumulateIndexCount );

        NameToIdHash *pHash = new NameToIdHash( nSize );
        for ( std::vector<NameArrayIndexTuple_t>::iterator entry = vIndexes.begin(); entry != vIndexes.end(); ++entry )
        {
            // Get a pointer to the function which will populate pStrings
            const std::vector<OUString>& (*pStringsFetchFunc)() = std::get<2>( *entry );
            if ( pStringsFetchFunc )
            {
                const std::vector<OUString>& rStrings = pStringsFetchFunc();
                sal_uInt16 nIndex, nId;
                for ( nIndex = 0, nId = std::get<0>( *entry ) ; nId < std::get<1>( *entry ) ; nId++, nIndex++ )
                    (*pHash)[rStrings[nIndex]] = nId;
            }
        }

        *pHashPointer = pHash;
    }

#ifdef _NEED_TO_DEBUG_MAPPING
    static bool bTested = false;
    if ( !bTested )
    {
        bTested = true;

        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_TEXT_BEGIN, RES_POOLCOLL_TEXT_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_LISTS_BEGIN, RES_POOLCOLL_LISTS_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_EXTRA_BEGIN, RES_POOLCOLL_EXTRA_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_REGISTER_BEGIN, RES_POOLCOLL_REGISTER_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_DOC_BEGIN, RES_POOLCOLL_DOC_END );
        testNameTable( SwGetPoolIdFromName::TxtColl, RES_POOLCOLL_HTML_BEGIN, RES_POOLCOLL_HTML_END );
        testNameTable( SwGetPoolIdFromName::ChrFmt, RES_POOLCHR_NORMAL_BEGIN, RES_POOLCHR_NORMAL_END );
        testNameTable( SwGetPoolIdFromName::ChrFmt, RES_POOLCHR_HTML_BEGIN, RES_POOLCHR_HTML_END );
        testNameTable( SwGetPoolIdFromName::FrmFmt, RES_POOLFRM_BEGIN, RES_POOLFRM_END );
        testNameTable( SwGetPoolIdFromName::PageDesc, RES_POOLPAGE_BEGIN, RES_POOLPAGE_END );
        testNameTable( SwGetPoolIdFromName::NumRule, RES_POOLNUMRULE_BEGIN, RES_POOLNUMRULE_END );
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
    const std::vector<OUString>* pStrArr = nullptr;

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
            pStrArr = bProgName ? &GetChrFormatProgNameArray() : &GetChrFormatUINameArray();
            nStt = RES_POOLCHR_NORMAL_BEGIN;
        }
        else if( RES_POOLCHR_HTML_BEGIN <= nId && nId < RES_POOLCHR_HTML_END )
        {
            pStrArr = bProgName ? &GetHTMLChrFormatProgNameArray() : &GetHTMLChrFormatUINameArray();
            nStt = RES_POOLCHR_HTML_BEGIN;
        }
        break;
    case POOLGRP_FRAMEFMT:
        if( RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END )
        {
            pStrArr = bProgName ? &GetFrameFormatProgNameArray() : &GetFrameFormatUINameArray();
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
    case POOLGRP_TABSTYLE:
        if( RES_POOLTABSTYLE_BEGIN <= nId && nId < RES_POOLTABSTYLE_END )
        {
            pStrArr = bProgName ? &GetTableStyleProgNameArray() : &GetTableStyleUINameArray();
            nStt = RES_POOLTABSTYLE_BEGIN;
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

/// returns an empty array because Cell Names aren't translated
const std::vector<OUString>& SwStyleNameMapper::GetCellStyleUINameArray()
{
    if (!s_pCellStyleUINameArray)
        s_pCellStyleUINameArray = new std::vector<OUString>;
    return *s_pCellStyleUINameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetTextProgNameArray()
{
    if (!s_pTextProgNameArray)
        s_pTextProgNameArray = lcl_NewProgNameArray(TextProgNameTable,
            SAL_N_ELEMENTS ( TextProgNameTable ) );
    return *s_pTextProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetListsProgNameArray()
{
    if (!s_pListsProgNameArray)
        s_pListsProgNameArray = lcl_NewProgNameArray( ListsProgNameTable,
            SAL_N_ELEMENTS ( ListsProgNameTable ) );
    return *s_pListsProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetExtraProgNameArray()
{
    if (!s_pExtraProgNameArray)
        s_pExtraProgNameArray = lcl_NewProgNameArray( ExtraProgNameTable,
            SAL_N_ELEMENTS ( ExtraProgNameTable ) );
    return *s_pExtraProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetRegisterProgNameArray()
{
    if (!s_pRegisterProgNameArray)
        s_pRegisterProgNameArray = lcl_NewProgNameArray( RegisterProgNameTable,
            SAL_N_ELEMENTS ( RegisterProgNameTable ) );
    return *s_pRegisterProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetDocProgNameArray()
{
    if (!s_pDocProgNameArray)
        s_pDocProgNameArray = lcl_NewProgNameArray( DocProgNameTable,
            SAL_N_ELEMENTS ( DocProgNameTable ) );
    return *s_pDocProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetHTMLProgNameArray()
{
    if (!s_pHTMLProgNameArray)
        s_pHTMLProgNameArray = lcl_NewProgNameArray( HTMLProgNameTable,
            SAL_N_ELEMENTS ( HTMLProgNameTable ) );
    return *s_pHTMLProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetFrameFormatProgNameArray()
{
    if (!s_pFrameFormatProgNameArray)
        s_pFrameFormatProgNameArray = lcl_NewProgNameArray( FrameFormatProgNameTable,
            SAL_N_ELEMENTS ( FrameFormatProgNameTable ) );
    return *s_pFrameFormatProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetChrFormatProgNameArray()
{
    if (!s_pChrFormatProgNameArray)
        s_pChrFormatProgNameArray = lcl_NewProgNameArray( ChrFormatProgNameTable,
            SAL_N_ELEMENTS ( ChrFormatProgNameTable ) );
    return *s_pChrFormatProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetHTMLChrFormatProgNameArray()
{
    if (!s_pHTMLChrFormatProgNameArray)
        s_pHTMLChrFormatProgNameArray = lcl_NewProgNameArray( HTMLChrFormatProgNameTable,
            SAL_N_ELEMENTS ( HTMLChrFormatProgNameTable ) );
    return *s_pHTMLChrFormatProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetPageDescProgNameArray()
{
    if (!s_pPageDescProgNameArray)
        s_pPageDescProgNameArray = lcl_NewProgNameArray( PageDescProgNameTable,
            SAL_N_ELEMENTS ( PageDescProgNameTable ) );
    return *s_pPageDescProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetNumRuleProgNameArray()
{
    if (!s_pNumRuleProgNameArray)
        s_pNumRuleProgNameArray = lcl_NewProgNameArray( NumRuleProgNameTable,
            SAL_N_ELEMENTS ( NumRuleProgNameTable ) );
    return *s_pNumRuleProgNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetTableStyleProgNameArray()
{
    if (!s_pTableStyleProgNameArray)
        s_pTableStyleProgNameArray = lcl_NewProgNameArray( TableStyleProgNameTable,
            SAL_N_ELEMENTS ( TableStyleProgNameTable ) );
    return *s_pTableStyleProgNameArray;
}

/// returns an empty array because Cell Names aren't translated
const std::vector<OUString>& SwStyleNameMapper::GetCellStyleProgNameArray()
{
    if (!s_pCellStyleProgNameArray)
        s_pCellStyleProgNameArray = new std::vector<OUString>;
    return *s_pCellStyleProgNameArray;
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
