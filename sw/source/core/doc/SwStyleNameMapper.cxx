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

#include <SwStyleNameMapper.hxx>
#include <tools/resmgr.hxx>
#include <poolfmt.hxx>
#include <rcid.hrc>
#ifdef _NEED_TO_DEBUG_MAPPING
#include <stdlib.h>
#endif

extern ResMgr* pSwResMgr;
// Initialise UI names to 0
boost::ptr_vector<String> *SwStyleNameMapper::pTextUINameArray = 0,
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

bool SwStyleNameMapper::SuffixIsUser ( const String & rString )
{
    const sal_Unicode *pChar = rString.GetBuffer();
    sal_Int32 nLen = rString.Len();
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
void SwStyleNameMapper::CheckSuffixAndDelete ( String & rString )
{
    const sal_Unicode *pChar = rString.GetBuffer();
    xub_StrLen nLen = rString.Len();
    if (nLen > 8 &&
        pChar[nLen-7] == ' ' &&
        pChar[nLen-6] == '(' &&
        pChar[nLen-5] == 'u' &&
        pChar[nLen-4] == 's' &&
        pChar[nLen-3] == 'e' &&
        pChar[nLen-2] == 'r' &&
        pChar[nLen-1] == ')')
    {
        rString.Erase ( nLen - 7, 7 );
    }
}
const NameToIdHash & SwStyleNameMapper::getHashTable ( SwGetPoolIdFromName eFlags, sal_Bool bProgName )
{
    NameToIdHash *pHash = 0;
    const boost::ptr_vector<String> *pStrings;

    switch ( eFlags )
    {
        case nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL:
        {
            sal_uInt16 nIndex;
            sal_uInt16 nId;

            pHash = bProgName ? pParaProgMap : pParaUIMap;
            if ( !pHash )
            {
                pHash = new NameToIdHash ( RES_POOLCOLL_TEXT_END - RES_POOLCOLL_TEXT_BEGIN +
                                           RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                           RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                           RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN +
                                           RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN +
                                           RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN );
                pStrings = bProgName ? &GetTextProgNameArray() : &GetTextUINameArray();
                for ( nIndex = 0, nId = RES_POOLCOLL_TEXT_BEGIN ; nId < RES_POOLCOLL_TEXT_END ; nId++,nIndex++ )
                    (*pHash)[&((*pStrings)[nIndex])] = nId;
                pStrings = bProgName ? &GetListsProgNameArray() : &GetListsUINameArray();
                for ( nIndex = 0, nId = RES_POOLCOLL_LISTS_BEGIN ; nId < RES_POOLCOLL_LISTS_END ; nId++,nIndex++ )
                    (*pHash)[&((*pStrings)[nIndex])] = nId;
                pStrings = bProgName ? &GetExtraProgNameArray() : &GetExtraUINameArray();
                for ( nIndex = 0, nId = RES_POOLCOLL_EXTRA_BEGIN ; nId < RES_POOLCOLL_EXTRA_END ; nId++,nIndex++ )
                    (*pHash)[&((*pStrings)[nIndex])] = nId;
                pStrings = bProgName ? &GetRegisterProgNameArray() : &GetRegisterUINameArray();
                for ( nIndex = 0, nId = RES_POOLCOLL_REGISTER_BEGIN ; nId < RES_POOLCOLL_REGISTER_END ; nId++,nIndex++ )
                    (*pHash)[&((*pStrings)[nIndex])] = nId;
                pStrings = bProgName ? &GetDocProgNameArray() : &GetDocUINameArray();
                for ( nIndex = 0, nId = RES_POOLCOLL_DOC_BEGIN ; nId < RES_POOLCOLL_DOC_END ; nId++,nIndex++ )
                    (*pHash)[&((*pStrings)[nIndex])] = nId;
                pStrings = bProgName ? &GetHTMLProgNameArray() : &GetHTMLUINameArray();
                for ( nIndex = 0, nId = RES_POOLCOLL_HTML_BEGIN ; nId < RES_POOLCOLL_HTML_END ; nId++,nIndex++ )
                    (*pHash)[&((*pStrings)[nIndex])] = nId;

                if ( bProgName )
                    pParaProgMap = pHash;
                else
                    pParaUIMap = pHash;
            }
        }
        break;
        case nsSwGetPoolIdFromName::GET_POOLID_CHRFMT:
        {
            pHash = bProgName ? pCharProgMap : pCharUIMap;
            if ( !pHash )
            {
                sal_uInt16 nIndex;
                sal_uInt16 nId;

                pHash = new NameToIdHash ( RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN +
                                           RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN );
                pStrings = bProgName ? &GetChrFmtProgNameArray() : &GetChrFmtUINameArray();
                for ( nIndex = 0, nId = RES_POOLCHR_NORMAL_BEGIN ; nId < RES_POOLCHR_NORMAL_END ; nId++,nIndex++ )
                    (*pHash)[&((*pStrings)[nIndex])] = nId;
                pStrings = bProgName ? &GetHTMLChrFmtProgNameArray() : &GetHTMLChrFmtUINameArray();
                for ( nIndex = 0, nId = RES_POOLCHR_HTML_BEGIN ; nId < RES_POOLCHR_HTML_END ; nId++,nIndex++ )
                    (*pHash)[&((*pStrings)[nIndex])] = nId;
                if (bProgName )
                    pCharProgMap = pHash;
                else
                    pCharUIMap = pHash;
            }
        }
        break;
        case nsSwGetPoolIdFromName::GET_POOLID_FRMFMT:
        {
            pHash = bProgName ? pFrameProgMap : pFrameUIMap;
            if ( !pHash )
            {
                pHash = new NameToIdHash ( RES_POOLFRM_END - RES_POOLFRM_BEGIN );
                pStrings = bProgName ? &GetFrmFmtProgNameArray() : &GetFrmFmtUINameArray();
                for ( sal_uInt16 nIndex=0,nId = RES_POOLFRM_BEGIN ; nId < RES_POOLFRM_END ; nId++,nIndex++ )
                    (*pHash)[&((*pStrings)[nIndex])] = nId;
                if ( bProgName )
                    pFrameProgMap = pHash;
                else
                    pFrameUIMap = pHash;
            }
        }
        break;
        case nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC:
        {
            pHash = bProgName ? pPageProgMap : pPageUIMap;
            if ( !pHash )
            {
                pHash = new NameToIdHash ( RES_POOLPAGE_END - RES_POOLPAGE_BEGIN );
                pStrings = bProgName ? &GetPageDescProgNameArray() : &GetPageDescUINameArray();
                for ( sal_uInt16 nIndex=0,nId = RES_POOLPAGE_BEGIN ; nId < RES_POOLPAGE_END ; nId++,nIndex++ )
                    (*pHash)[&((*pStrings)[nIndex])] = nId;
                if ( bProgName )
                    pPageProgMap = pHash;
                else
                    pPageUIMap = pHash;
            }
        }
        break;
        case nsSwGetPoolIdFromName::GET_POOLID_NUMRULE:
        {
            pHash = bProgName ? pNumRuleProgMap : pNumRuleUIMap;
            if ( !pHash )
            {
                pHash = new NameToIdHash ( RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN );
                pStrings = bProgName ? &GetNumRuleProgNameArray() : &GetNumRuleUINameArray();
                for ( sal_uInt16 nIndex=0,nId = RES_POOLNUMRULE_BEGIN ; nId < RES_POOLNUMRULE_END ; nId++,nIndex++ )
                {
                    (*pHash)[&((*pStrings)[nIndex])] = nId;
                }
                if ( bProgName )
                    pNumRuleProgMap = pHash;
                else
                    pNumRuleUIMap = pHash;
            }
        }
        break;
    }
#ifdef _NEED_TO_DEBUG_MAPPING
    static bool bTested = false;
    if ( !bTested )
    {
        bTested = true;
        {
            for ( sal_uInt16 nIndex = 0, nId = RES_POOLCOLL_TEXT_BEGIN ; nId < RES_POOLCOLL_TEXT_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    abort();
            }
            for ( nIndex = 0, nId = RES_POOLCOLL_LISTS_BEGIN ; nId < RES_POOLCOLL_LISTS_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    abort();
            }
            for ( nIndex = 0, nId = RES_POOLCOLL_EXTRA_BEGIN ; nId < RES_POOLCOLL_EXTRA_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    abort();
            }
            for ( nIndex = 0, nId = RES_POOLCOLL_REGISTER_BEGIN ; nId < RES_POOLCOLL_REGISTER_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    abort();
            }
            for ( nIndex = 0, nId = RES_POOLCOLL_DOC_BEGIN ; nId < RES_POOLCOLL_DOC_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    abort();
            }
            for ( nIndex = 0, nId = RES_POOLCOLL_HTML_BEGIN ; nId < RES_POOLCOLL_HTML_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    abort();
            }
        }
        {
            for ( sal_uInt16 nIndex = 0, nId = RES_POOLCHR_NORMAL_BEGIN ; nId < RES_POOLCHR_NORMAL_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    abort();
            }
            for ( nIndex = 0, nId = RES_POOLCHR_HTML_BEGIN ; nId < RES_POOLCHR_HTML_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    abort();
            }
        }
        {
            for ( sal_uInt16 nIndex=0,nId = RES_POOLFRM_BEGIN ; nId < RES_POOLFRM_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    abort();
            }
        }
        {
            for ( sal_uInt16 nIndex=0,nId = RES_POOLPAGE_BEGIN ; nId < RES_POOLPAGE_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    abort();
            }
        }
        {
            for ( sal_uInt16 nIndex=0,nId = RES_POOLNUMRULE_BEGIN ; nId < RES_POOLNUMRULE_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    abort();
            }
        }
    }
#endif
    return *pHash;
}
// This gets the UI Name from the programmatic name
const String& SwStyleNameMapper::GetUIName ( const String& rName, SwGetPoolIdFromName eFlags )
{
    sal_uInt16 nId = GetPoolIdFromProgName ( rName, eFlags );
    return nId != USHRT_MAX ? GetUIName( nId, rName ) : rName;
}


// Get the programmatic Name from the UI name
const String& SwStyleNameMapper::GetProgName( const String& rName, SwGetPoolIdFromName eFlags )
{
    sal_uInt16 nId = GetPoolIdFromUIName ( rName, eFlags );
    return nId != USHRT_MAX ? GetProgName( nId, rName ) : rName;
}

// Get the programmatic name from the UI name in rName and put it into rFillName
void SwStyleNameMapper::FillProgName ( const String& rName, String& rFillName, SwGetPoolIdFromName eFlags, sal_Bool bDisambiguate )
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
            if ( SuffixIsUser ( rFillName ) )
                rFillName.AppendAscii ( RTL_CONSTASCII_STRINGPARAM ( " (user)" ) );
        }
        else
        {
            // It's in the programmatic name table...append suffix
            rFillName.AppendAscii ( RTL_CONSTASCII_STRINGPARAM ( " (user)" ) );
        }
    }
    else
    {
        // If we aren't trying to disambiguate, then just do a normal fill
        fillNameFromId ( nId, rFillName, sal_True);
    }
}
// Get the UI name from the programmatic name in rName and put it into rFillName
void SwStyleNameMapper::FillUIName ( const String& rName, String& rFillName, SwGetPoolIdFromName eFlags, sal_Bool bDisambiguate )
{
    sal_uInt16 nId = GetPoolIdFromProgName ( rName, eFlags );
    if ( bDisambiguate && nId == USHRT_MAX )
    {
        rFillName = rName;
        // rName isn't in our Prog name table...check if it has a " (user)" suffix, if so remove it
        CheckSuffixAndDelete ( rFillName );
    }
    else
    {
        // If we aren't trying to disambiguate, then just do a normal fill
        fillNameFromId ( nId, rFillName, sal_False);
    }
}

const String& SwStyleNameMapper::getNameFromId( sal_uInt16 nId, const String& rFillName, sal_Bool bProgName )
{
    sal_uInt16 nStt = 0;
    const boost::ptr_vector<String>* pStrArr = 0;

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
    return pStrArr ? (pStrArr->operator[] ( nId - nStt ) ) : rFillName;
}
void SwStyleNameMapper::fillNameFromId( sal_uInt16 nId, String& rFillName, sal_Bool bProgName )
{
    sal_uInt16 nStt = 0;
    const boost::ptr_vector<String>* pStrArr = 0;

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
    if (pStrArr)
        rFillName = (pStrArr->operator[] ( nId - nStt ) );
}
// Get the UI Name from the pool ID
void SwStyleNameMapper::FillUIName ( sal_uInt16 nId, String& rFillName )
{
    fillNameFromId ( nId, rFillName, sal_False );
}
// Get the UI Name from the pool ID
const String& SwStyleNameMapper::GetUIName ( sal_uInt16 nId, const String& rName )
{
    return getNameFromId ( nId, rName, sal_False );
}

// Get the programmatic Name from the pool ID
void SwStyleNameMapper::FillProgName ( sal_uInt16 nId, String& rFillName )
{
    fillNameFromId ( nId, rFillName, sal_True );
}
// Get the programmatic Name from the pool ID
const String& SwStyleNameMapper::GetProgName ( sal_uInt16 nId, const String& rName )
{
    return getNameFromId ( nId, rName, sal_True );
}
// This gets the PoolId from the UI Name
sal_uInt16 SwStyleNameMapper::GetPoolIdFromUIName( const String& rName, SwGetPoolIdFromName eFlags )
{
    const NameToIdHash & rHashMap = getHashTable ( eFlags, sal_False );
    NameToIdHash::const_iterator aIter = rHashMap.find ( &rName );
    return aIter != rHashMap.end() ? (*aIter).second : USHRT_MAX;
}
// Get the Pool ID from the programmatic name
sal_uInt16 SwStyleNameMapper::GetPoolIdFromProgName( const String& rName, SwGetPoolIdFromName eFlags )
{
    const NameToIdHash & rHashMap = getHashTable ( eFlags, sal_True );
    NameToIdHash::const_iterator aIter = rHashMap.find ( &rName );
    return aIter != rHashMap.end() ? (*aIter).second : USHRT_MAX;
}

boost::ptr_vector<String>* SwStyleNameMapper::NewUINameArray( boost::ptr_vector<String>*& pNameArray, sal_uInt16 nStt, sal_uInt16 nEnd )
{
    if( !pNameArray )
    {
        pNameArray = new boost::ptr_vector<String>;
        pNameArray->reserve(nEnd - nStt);
        while( nStt < nEnd )
        {
            const ResId rRId( nStt, *pSwResMgr );
            pNameArray->push_back(new String(rRId));
            ++nStt;
        }
    }
    return pNameArray;
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetTextUINameArray()
{
    return pTextUINameArray ? *pTextUINameArray :
           *NewUINameArray( pTextUINameArray, RC_POOLCOLL_TEXT_BEGIN,
            ( RC_POOLCOLL_TEXT_BEGIN +
                    (RES_POOLCOLL_TEXT_END - RES_POOLCOLL_TEXT_BEGIN )) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetListsUINameArray()
{
    return pListsUINameArray ? *pListsUINameArray :
           *NewUINameArray( pListsUINameArray, RC_POOLCOLL_LISTS_BEGIN,
            ( RC_POOLCOLL_LISTS_BEGIN +
                    (RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN )) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetExtraUINameArray()
{
    return pExtraUINameArray ? *pExtraUINameArray :
           *NewUINameArray( pExtraUINameArray, RC_POOLCOLL_EXTRA_BEGIN,
                ( RC_POOLCOLL_EXTRA_BEGIN +
                    (RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN )) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetRegisterUINameArray()
{
    return pRegisterUINameArray ? *pRegisterUINameArray :
           *NewUINameArray( pRegisterUINameArray, RC_POOLCOLL_REGISTER_BEGIN,
            ( RC_POOLCOLL_REGISTER_BEGIN +
                (RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN )) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetDocUINameArray()
{
    return pDocUINameArray ? *pDocUINameArray :
           *NewUINameArray( pDocUINameArray, RC_POOLCOLL_DOC_BEGIN,
                    ( RC_POOLCOLL_DOC_BEGIN +
                        (RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN )) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetHTMLUINameArray()
{
    return pHTMLUINameArray ? *pHTMLUINameArray :
           *NewUINameArray( pHTMLUINameArray, RC_POOLCOLL_HTML_BEGIN,
                    ( RC_POOLCOLL_HTML_BEGIN +
                        (RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN )) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetFrmFmtUINameArray()
{
    return pFrmFmtUINameArray ? *pFrmFmtUINameArray :
           *NewUINameArray( pFrmFmtUINameArray, RC_POOLFRMFMT_BEGIN,
                    ( RC_POOLFRMFMT_BEGIN +
                        (RES_POOLFRM_END - RES_POOLFRM_BEGIN )) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetChrFmtUINameArray()
{
    return pChrFmtUINameArray ? *pChrFmtUINameArray :
           *NewUINameArray( pChrFmtUINameArray, RC_POOLCHRFMT_BEGIN,
            ( RC_POOLCHRFMT_BEGIN +
                    (RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN )) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetHTMLChrFmtUINameArray()
{
    return pHTMLChrFmtUINameArray ? *pHTMLChrFmtUINameArray :
           *NewUINameArray( pHTMLChrFmtUINameArray, RC_POOLCHRFMT_HTML_BEGIN,
            ( RC_POOLCHRFMT_HTML_BEGIN +
                    (RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN )) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetPageDescUINameArray()
{
    return pPageDescUINameArray ? *pPageDescUINameArray :
           *NewUINameArray( pPageDescUINameArray, RC_POOLPAGEDESC_BEGIN,
            ( RC_POOLPAGEDESC_BEGIN +
                    (RES_POOLPAGE_END - RES_POOLPAGE_BEGIN )) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetNumRuleUINameArray()
{
    return pNumRuleUINameArray ? *pNumRuleUINameArray :
           *NewUINameArray( pNumRuleUINameArray, RC_POOLNUMRULE_BEGIN,
            ( RC_POOLNUMRULE_BEGIN +
                    (RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN )) );
}

boost::ptr_vector<String>* SwStyleNameMapper::NewProgNameArray( boost::ptr_vector<String>*& pProgNameArray, const SwTableEntry *pTable, sal_uInt8 nCount )
{
    if( !pProgNameArray )
    {
        pProgNameArray = new boost::ptr_vector<String>;
        pProgNameArray->reserve(nCount);
        while (pTable->nLength)
        {
            pProgNameArray->push_back(new String( pTable->pChar, pTable->nLength, RTL_TEXTENCODING_ASCII_US ));
            pTable++;
        }
    }
    return pProgNameArray;
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetTextProgNameArray()
{
    return pTextProgNameArray ? *pTextProgNameArray :
           *NewProgNameArray( pTextProgNameArray, TextProgNameTable,
            sizeof ( TextProgNameTable ) / sizeof ( SwTableEntry ) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetListsProgNameArray()
{
    return pListsProgNameArray ? *pListsProgNameArray :
           *NewProgNameArray( pListsProgNameArray, ListsProgNameTable,
            sizeof ( ListsProgNameTable ) / sizeof ( SwTableEntry ) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetExtraProgNameArray()
{
    return pExtraProgNameArray ? *pExtraProgNameArray :
           *NewProgNameArray( pExtraProgNameArray, ExtraProgNameTable,
            sizeof ( ExtraProgNameTable ) / sizeof ( SwTableEntry ) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetRegisterProgNameArray()
{
    return pRegisterProgNameArray ? *pRegisterProgNameArray :
           *NewProgNameArray( pRegisterProgNameArray, RegisterProgNameTable,
            sizeof ( RegisterProgNameTable ) / sizeof ( SwTableEntry ) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetDocProgNameArray()
{
    return pDocProgNameArray ? *pDocProgNameArray :
           *NewProgNameArray( pDocProgNameArray, DocProgNameTable,
            sizeof ( DocProgNameTable ) / sizeof ( SwTableEntry ) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetHTMLProgNameArray()
{
    return pHTMLProgNameArray ? *pHTMLProgNameArray :
           *NewProgNameArray( pHTMLProgNameArray, HTMLProgNameTable,
            sizeof ( HTMLProgNameTable ) / sizeof ( SwTableEntry ) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetFrmFmtProgNameArray()
{
    return pFrmFmtProgNameArray ? *pFrmFmtProgNameArray :
           *NewProgNameArray( pFrmFmtProgNameArray, FrmFmtProgNameTable,
            sizeof ( FrmFmtProgNameTable ) / sizeof ( SwTableEntry ) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetChrFmtProgNameArray()
{
    return pChrFmtProgNameArray ? *pChrFmtProgNameArray :
           *NewProgNameArray( pChrFmtProgNameArray, ChrFmtProgNameTable,
            sizeof ( ChrFmtProgNameTable ) / sizeof ( SwTableEntry ) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetHTMLChrFmtProgNameArray()
{
    return pHTMLChrFmtProgNameArray ? *pHTMLChrFmtProgNameArray :
           *NewProgNameArray( pHTMLChrFmtProgNameArray, HTMLChrFmtProgNameTable,
            sizeof ( HTMLChrFmtProgNameTable ) / sizeof ( SwTableEntry ) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetPageDescProgNameArray()
{
    return pPageDescProgNameArray ? *pPageDescProgNameArray :
           *NewProgNameArray( pPageDescProgNameArray, PageDescProgNameTable,
            sizeof ( PageDescProgNameTable ) / sizeof ( SwTableEntry ) );
}

const boost::ptr_vector<String>& SwStyleNameMapper::GetNumRuleProgNameArray()
{
    return pNumRuleProgNameArray ? *pNumRuleProgNameArray :
           *NewProgNameArray( pNumRuleProgNameArray, NumRuleProgNameTable,
            sizeof ( NumRuleProgNameTable ) / sizeof ( SwTableEntry ) );
}


const String SwStyleNameMapper::GetSpecialExtraProgName( const String& rExtraUIName )
{
    String aRes = rExtraUIName;
    bool bChgName = false;
    const boost::ptr_vector<String>& rExtraArr(GetExtraUINameArray());
    static sal_uInt16 nIds[] =
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
        if (aRes == rExtraArr[ *pIds ])
        {
            bChgName = true;
            break;
        }
    }
    if (bChgName)
        aRes = GetExtraProgNameArray()[*pIds];
    return aRes;
}

const String SwStyleNameMapper::GetSpecialExtraUIName( const String& rExtraProgName )
{
    String aRes = rExtraProgName;
    bool bChgName = false;
    const boost::ptr_vector<String>& rExtraArr(GetExtraProgNameArray());
    static sal_uInt16 nIds[] =
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
        if (aRes == rExtraArr[ *pIds ])
        {
            bChgName = true;
            break;
        }
    }
    if (bChgName)
        aRes = GetExtraUINameArray()[*pIds];
    return aRes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
