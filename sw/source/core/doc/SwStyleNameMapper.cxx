/*************************************************************************
 *
 *  $RCSfile: SwStyleNameMapper.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:39:34 $
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
 *  Contributor(s): Martin Gallwey ( gallwey@sun.com )
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif
#pragma hdrstop

#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _RDIC_HRC
#include <rcid.hrc>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
extern ResMgr* pSwResMgr;
// Initialise UI names to 0
SvStringsDtor   *SwStyleNameMapper::pTextUINameArray = 0,
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

sal_Bool SwStyleNameMapper::SuffixIsUser ( const String & rString )
{
    const sal_Unicode *pChar = rString.GetBuffer();
    sal_Int32 nLen = rString.Len();
    return nLen <= 8 ? sal_False :
           pChar[nLen-7] == ' ' &&
           pChar[nLen-6] == '(' &&
           pChar[nLen-5] == 'u' &&
           pChar[nLen-4] == 's' &&
           pChar[nLen-3] == 'e' &&
           pChar[nLen-2] == 'r' &&
           pChar[nLen-1] == ')';
}
void SwStyleNameMapper::CheckSuffixAndDelete ( String & rString )
{
    const sal_Unicode *pChar = rString.GetBuffer();
    sal_Int32 nLen = rString.Len();
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
    NameToIdHash *pHash;
    const SvStringsDtor *pStrings;

    switch ( eFlags )
    {
        case GET_POOLID_TXTCOLL:
        {
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
                for ( sal_uInt16 nIndex = 0, nId = RES_POOLCOLL_TEXT_BEGIN ; nId < RES_POOLCOLL_TEXT_END ; nId++,nIndex++ )
                    (*pHash)[(*pStrings)[nIndex]] = nId;
                pStrings = bProgName ? &GetListsProgNameArray() : &GetListsUINameArray();
                for ( nIndex = 0, nId = RES_POOLCOLL_LISTS_BEGIN ; nId < RES_POOLCOLL_LISTS_END ; nId++,nIndex++ )
                    (*pHash)[(*pStrings)[nIndex]] = nId;
                pStrings = bProgName ? &GetExtraProgNameArray() : &GetExtraUINameArray();
                for ( nIndex = 0, nId = RES_POOLCOLL_EXTRA_BEGIN ; nId < RES_POOLCOLL_EXTRA_END ; nId++,nIndex++ )
                    (*pHash)[(*pStrings)[nIndex]] = nId;
                pStrings = bProgName ? &GetRegisterProgNameArray() : &GetRegisterUINameArray();
                for ( nIndex = 0, nId = RES_POOLCOLL_REGISTER_BEGIN ; nId < RES_POOLCOLL_REGISTER_END ; nId++,nIndex++ )
                    (*pHash)[(*pStrings)[nIndex]] = nId;
                pStrings = bProgName ? &GetDocProgNameArray() : &GetDocUINameArray();
                for ( nIndex = 0, nId = RES_POOLCOLL_DOC_BEGIN ; nId < RES_POOLCOLL_DOC_END ; nId++,nIndex++ )
                    (*pHash)[(*pStrings)[nIndex]] = nId;
                pStrings = bProgName ? &GetHTMLProgNameArray() : &GetHTMLUINameArray();
                for ( nIndex = 0, nId = RES_POOLCOLL_HTML_BEGIN ; nId < RES_POOLCOLL_HTML_END ; nId++,nIndex++ )
                    (*pHash)[(*pStrings)[nIndex]] = nId;

                if ( bProgName )
                    pParaProgMap = pHash;
                else
                    pParaUIMap = pHash;
            }
        }
        break;
        case GET_POOLID_CHRFMT:
        {
            pHash = bProgName ? pCharProgMap : pCharUIMap;
            if ( !pHash )
            {
                pHash = new NameToIdHash ( RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN +
                                           RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN );
                pStrings = bProgName ? &GetChrFmtProgNameArray() : &GetChrFmtUINameArray();
                for ( sal_uInt16 nIndex = 0, nId = RES_POOLCHR_NORMAL_BEGIN ; nId < RES_POOLCHR_NORMAL_END ; nId++,nIndex++ )
                    (*pHash)[(*pStrings)[nIndex]] = nId;
                pStrings = bProgName ? &GetHTMLChrFmtProgNameArray() : &GetHTMLChrFmtUINameArray();
                for ( nIndex = 0, nId = RES_POOLCHR_HTML_BEGIN ; nId < RES_POOLCHR_HTML_END ; nId++,nIndex++ )
                    (*pHash)[(*pStrings)[nIndex]] = nId;
                if (bProgName )
                    pCharProgMap = pHash;
                else
                    pCharUIMap = pHash;
            }
        }
        break;
        case GET_POOLID_FRMFMT:
        {
            pHash = bProgName ? pFrameProgMap : pFrameUIMap;
            if ( !pHash )
            {
                pHash = new NameToIdHash ( RES_POOLFRM_END - RES_POOLFRM_BEGIN );
                pStrings = bProgName ? &GetFrmFmtProgNameArray() : &GetFrmFmtUINameArray();
                for ( sal_uInt16 nIndex=0,nId = RES_POOLFRM_BEGIN ; nId < RES_POOLFRM_END ; nId++,nIndex++ )
                    (*pHash)[(*pStrings)[nIndex]] = nId;
                if ( bProgName )
                    pFrameProgMap = pHash;
                else
                    pFrameUIMap = pHash;
            }
        }
        break;
        case GET_POOLID_PAGEDESC:
        {
            pHash = bProgName ? pPageProgMap : pPageUIMap;
            if ( !pHash )
            {
                pHash = new NameToIdHash ( RES_POOLPAGE_END - RES_POOLPAGE_BEGIN );
                pStrings = bProgName ? &GetPageDescProgNameArray() : &GetPageDescUINameArray();
                for ( sal_uInt16 nIndex=0,nId = RES_POOLPAGE_BEGIN ; nId < RES_POOLPAGE_END ; nId++,nIndex++ )
                    (*pHash)[(*pStrings)[nIndex]] = nId;
                if ( bProgName )
                    pPageProgMap = pHash;
                else
                    pPageUIMap = pHash;
            }
        }
        break;
        case GET_POOLID_NUMRULE:
        {
            pHash = bProgName ? pNumRuleProgMap : pNumRuleUIMap;
            if ( !pHash )
            {
                pHash = new NameToIdHash ( RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN );
                pStrings = bProgName ? &GetNumRuleProgNameArray() : &GetNumRuleUINameArray();
                for ( sal_uInt16 nIndex=0,nId = RES_POOLNUMRULE_BEGIN ; nId < RES_POOLNUMRULE_END ; nId++,nIndex++ )
                    (*pHash)[(*pStrings)[nIndex]] = nId;
                if ( bProgName )
                    pNumRuleProgMap = pHash;
                else
                    pNumRuleUIMap = pHash;
            }
        }
        break;
    }
#ifdef _NEED_TO_DEBUG_MAPPING
    static sal_Bool bTested = sal_False;
    if ( !bTested )
    {
        bTested = sal_True;
        {
            for ( sal_uInt16 nIndex = 0, nId = RES_POOLCOLL_TEXT_BEGIN ; nId < RES_POOLCOLL_TEXT_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    *((sal_Int32*)0) = 42;
            }
            for ( nIndex = 0, nId = RES_POOLCOLL_LISTS_BEGIN ; nId < RES_POOLCOLL_LISTS_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    *((sal_Int32*)0) = 42;
            }
            for ( nIndex = 0, nId = RES_POOLCOLL_EXTRA_BEGIN ; nId < RES_POOLCOLL_EXTRA_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    *((sal_Int32*)0) = 42;
            }
            for ( nIndex = 0, nId = RES_POOLCOLL_REGISTER_BEGIN ; nId < RES_POOLCOLL_REGISTER_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    *((sal_Int32*)0) = 42;
            }
            for ( nIndex = 0, nId = RES_POOLCOLL_DOC_BEGIN ; nId < RES_POOLCOLL_DOC_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    *((sal_Int32*)0) = 42;
            }
            for ( nIndex = 0, nId = RES_POOLCOLL_HTML_BEGIN ; nId < RES_POOLCOLL_HTML_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    *((sal_Int32*)0) = 42;
            }
        }
        {
            for ( sal_uInt16 nIndex = 0, nId = RES_POOLCHR_NORMAL_BEGIN ; nId < RES_POOLCHR_NORMAL_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    *((sal_Int32*)0) = 42;
            }
            for ( nIndex = 0, nId = RES_POOLCHR_HTML_BEGIN ; nId < RES_POOLCHR_HTML_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    *((sal_Int32*)0) = 42;
            }
        }
        {
            for ( sal_uInt16 nIndex=0,nId = RES_POOLFRM_BEGIN ; nId < RES_POOLFRM_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    *((sal_Int32*)0) = 42;
            }
        }
        {
            for ( sal_uInt16 nIndex=0,nId = RES_POOLPAGE_BEGIN ; nId < RES_POOLPAGE_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    *((sal_Int32*)0) = 42;
            }
        }
        {
            for ( sal_uInt16 nIndex=0,nId = RES_POOLNUMRULE_BEGIN ; nId < RES_POOLNUMRULE_END ; nId++,nIndex++ )
            {
                String aString, bString;
                FillUIName ( nId, aString );
                bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
                sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
                FillProgName ( nNewId, aString );
                bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
                nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
                if ( nNewId != nId )
                    *((sal_Int32*)0) = 42;
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
    const SvStringsDtor* pStrArr = 0;

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
    return pStrArr ? *(pStrArr->operator[] ( nId - nStt ) ) : rFillName;
}
void SwStyleNameMapper::fillNameFromId( sal_uInt16 nId, String& rFillName, sal_Bool bProgName )
{
    sal_uInt16 nStt = 0;
    const SvStringsDtor* pStrArr = 0;

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
        rFillName = *(pStrArr->operator[] ( nId - nStt ) );
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

SvStringsDtor* SwStyleNameMapper::NewUINameArray( SvStringsDtor*& pNameArray, sal_uInt16 nStt, sal_uInt16 nEnd )
{
    if( !pNameArray )
    {
        pNameArray = new SvStringsDtor( static_cast < sal_Int8 > (nEnd - nStt), 1 );
        while( nStt < nEnd )
        {
            const ResId rRId( nStt, pSwResMgr );
            String* pStr = new String( rRId );
            pNameArray->Insert( pStr, pNameArray->Count() );
            ++nStt;
        }
    }
    return pNameArray;
}

const SvStringsDtor& SwStyleNameMapper::GetTextUINameArray()
{
    return pTextUINameArray ? *pTextUINameArray :
           *NewUINameArray( pTextUINameArray, RC_POOLCOLL_TEXT_BEGIN,
            ( RC_POOLCOLL_TEXT_BEGIN +
                    (RES_POOLCOLL_TEXT_END - RES_POOLCOLL_TEXT_BEGIN )) );
}

const SvStringsDtor& SwStyleNameMapper::GetListsUINameArray()
{
    return pListsUINameArray ? *pListsUINameArray :
           *NewUINameArray( pListsUINameArray, RC_POOLCOLL_LISTS_BEGIN,
            ( RC_POOLCOLL_LISTS_BEGIN +
                    (RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN )) );
}

const SvStringsDtor& SwStyleNameMapper::GetExtraUINameArray()
{
    return pExtraUINameArray ? *pExtraUINameArray :
           *NewUINameArray( pExtraUINameArray, RC_POOLCOLL_EXTRA_BEGIN,
                ( RC_POOLCOLL_EXTRA_BEGIN +
                    (RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN )) );
}

const SvStringsDtor& SwStyleNameMapper::GetRegisterUINameArray()
{
    return pRegisterUINameArray ? *pRegisterUINameArray :
           *NewUINameArray( pRegisterUINameArray, RC_POOLCOLL_REGISTER_BEGIN,
            ( RC_POOLCOLL_REGISTER_BEGIN +
                (RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN )) );
}

const SvStringsDtor& SwStyleNameMapper::GetDocUINameArray()
{
    return pDocUINameArray ? *pDocUINameArray :
           *NewUINameArray( pDocUINameArray, RC_POOLCOLL_DOC_BEGIN,
                    ( RC_POOLCOLL_DOC_BEGIN +
                        (RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN )) );
}

const SvStringsDtor& SwStyleNameMapper::GetHTMLUINameArray()
{
    return pHTMLUINameArray ? *pHTMLUINameArray :
           *NewUINameArray( pHTMLUINameArray, RC_POOLCOLL_HTML_BEGIN,
                    ( RC_POOLCOLL_HTML_BEGIN +
                        (RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN )) );
}

const SvStringsDtor& SwStyleNameMapper::GetFrmFmtUINameArray()
{
    return pFrmFmtUINameArray ? *pFrmFmtUINameArray :
           *NewUINameArray( pFrmFmtUINameArray, RC_POOLFRMFMT_BEGIN,
                    ( RC_POOLFRMFMT_BEGIN +
                        (RES_POOLFRM_END - RES_POOLFRM_BEGIN )) );
}

const SvStringsDtor& SwStyleNameMapper::GetChrFmtUINameArray()
{
    return pChrFmtUINameArray ? *pChrFmtUINameArray :
           *NewUINameArray( pChrFmtUINameArray, RC_POOLCHRFMT_BEGIN,
            ( RC_POOLCHRFMT_BEGIN +
                    (RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN )) );
}

const SvStringsDtor& SwStyleNameMapper::GetHTMLChrFmtUINameArray()
{
    return pHTMLChrFmtUINameArray ? *pHTMLChrFmtUINameArray :
           *NewUINameArray( pHTMLChrFmtUINameArray, RC_POOLCHRFMT_HTML_BEGIN,
            ( RC_POOLCHRFMT_HTML_BEGIN +
                    (RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN )) );
}

const SvStringsDtor& SwStyleNameMapper::GetPageDescUINameArray()
{
    return pPageDescUINameArray ? *pPageDescUINameArray :
           *NewUINameArray( pPageDescUINameArray, RC_POOLPAGEDESC_BEGIN,
            ( RC_POOLPAGEDESC_BEGIN +
                    (RES_POOLPAGE_END - RES_POOLPAGE_BEGIN )) );
}

const SvStringsDtor& SwStyleNameMapper::GetNumRuleUINameArray()
{
    return pNumRuleUINameArray ? *pNumRuleUINameArray :
           *NewUINameArray( pNumRuleUINameArray, RC_POOLNUMRULE_BEGIN,
            ( RC_POOLNUMRULE_BEGIN +
                    (RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN )) );
}

SvStringsDtor* SwStyleNameMapper::NewProgNameArray( SvStringsDtor*& pProgNameArray, const SwTableEntry *pTable, sal_uInt8 nCount )
{
    if( !pProgNameArray )
    {
        pProgNameArray = new SvStringsDtor( nCount, 1 );
        while (pTable->nLength)
        {
            String* pStr = new String( pTable->pChar, pTable->nLength, RTL_TEXTENCODING_ASCII_US );
            pProgNameArray->Insert( pStr, pProgNameArray->Count() );
            pTable++;
        }
    }
    return pProgNameArray;
}

const SvStringsDtor& SwStyleNameMapper::GetTextProgNameArray()
{
    return pTextProgNameArray ? *pTextProgNameArray :
           *NewProgNameArray( pTextProgNameArray, TextProgNameTable,
            sizeof ( TextProgNameTable ) / sizeof ( SwTableEntry ) );
}

const SvStringsDtor& SwStyleNameMapper::GetListsProgNameArray()
{
    return pListsProgNameArray ? *pListsProgNameArray :
           *NewProgNameArray( pListsProgNameArray, ListsProgNameTable,
            sizeof ( ListsProgNameTable ) / sizeof ( SwTableEntry ) );
}

const SvStringsDtor& SwStyleNameMapper::GetExtraProgNameArray()
{
    return pExtraProgNameArray ? *pExtraProgNameArray :
           *NewProgNameArray( pExtraProgNameArray, ExtraProgNameTable,
            sizeof ( ExtraProgNameTable ) / sizeof ( SwTableEntry ) );
}

const SvStringsDtor& SwStyleNameMapper::GetRegisterProgNameArray()
{
    return pRegisterProgNameArray ? *pRegisterProgNameArray :
           *NewProgNameArray( pRegisterProgNameArray, RegisterProgNameTable,
            sizeof ( RegisterProgNameTable ) / sizeof ( SwTableEntry ) );
}

const SvStringsDtor& SwStyleNameMapper::GetDocProgNameArray()
{
    return pDocProgNameArray ? *pDocProgNameArray :
           *NewProgNameArray( pDocProgNameArray, DocProgNameTable,
            sizeof ( DocProgNameTable ) / sizeof ( SwTableEntry ) );
}

const SvStringsDtor& SwStyleNameMapper::GetHTMLProgNameArray()
{
    return pHTMLProgNameArray ? *pHTMLProgNameArray :
           *NewProgNameArray( pHTMLProgNameArray, HTMLProgNameTable,
            sizeof ( HTMLProgNameTable ) / sizeof ( SwTableEntry ) );
}

const SvStringsDtor& SwStyleNameMapper::GetFrmFmtProgNameArray()
{
    return pFrmFmtProgNameArray ? *pFrmFmtProgNameArray :
           *NewProgNameArray( pFrmFmtProgNameArray, FrmFmtProgNameTable,
            sizeof ( FrmFmtProgNameTable ) / sizeof ( SwTableEntry ) );
}

const SvStringsDtor& SwStyleNameMapper::GetChrFmtProgNameArray()
{
    return pChrFmtProgNameArray ? *pChrFmtProgNameArray :
           *NewProgNameArray( pChrFmtProgNameArray, ChrFmtProgNameTable,
            sizeof ( ChrFmtProgNameTable ) / sizeof ( SwTableEntry ) );
}

const SvStringsDtor& SwStyleNameMapper::GetHTMLChrFmtProgNameArray()
{
    return pHTMLChrFmtProgNameArray ? *pHTMLChrFmtProgNameArray :
           *NewProgNameArray( pHTMLChrFmtProgNameArray, HTMLChrFmtProgNameTable,
            sizeof ( HTMLChrFmtProgNameTable ) / sizeof ( SwTableEntry ) );
}

const SvStringsDtor& SwStyleNameMapper::GetPageDescProgNameArray()
{
    return pPageDescProgNameArray ? *pPageDescProgNameArray :
           *NewProgNameArray( pPageDescProgNameArray, PageDescProgNameTable,
            sizeof ( PageDescProgNameTable ) / sizeof ( SwTableEntry ) );
}

const SvStringsDtor& SwStyleNameMapper::GetNumRuleProgNameArray()
{
    return pNumRuleProgNameArray ? *pNumRuleProgNameArray :
           *NewProgNameArray( pNumRuleProgNameArray, NumRuleProgNameTable,
            sizeof ( NumRuleProgNameTable ) / sizeof ( SwTableEntry ) );
}


const String SwStyleNameMapper::GetSpecialExtraProgName( const String& rExtraUIName )
{
    String aRes = rExtraUIName;
    BOOL bChgName = FALSE;
    const SvStringsDtor& rExtraArr = GetExtraUINameArray();
    static sal_uInt16 nIds[] =
    {
        RES_POOLCOLL_LABEL_DRAWING - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_ABB - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_TABLE - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_FRAME- RES_POOLCOLL_EXTRA_BEGIN,
        0
    };
    for (const sal_uInt16 * pIds = nIds; *pIds; ++pIds)
    {
        if (aRes == *rExtraArr[ *pIds ])
        {
            bChgName = TRUE;
            break;
        }
    }
    if (bChgName)
        aRes = *GetExtraProgNameArray()[*pIds];
    return aRes;
}

const String SwStyleNameMapper::GetSpecialExtraUIName( const String& rExtraProgName )
{
    String aRes = rExtraProgName;
    BOOL bChgName = FALSE;
    const SvStringsDtor& rExtraArr = GetExtraProgNameArray();
    static sal_uInt16 nIds[] =
    {
        RES_POOLCOLL_LABEL_DRAWING - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_ABB - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_TABLE - RES_POOLCOLL_EXTRA_BEGIN,
        RES_POOLCOLL_LABEL_FRAME- RES_POOLCOLL_EXTRA_BEGIN,
        0
    };
    for (const sal_uInt16 * pIds = nIds; *pIds; ++pIds)
    {
        if (aRes == *rExtraArr[ *pIds ])
        {
            bChgName = TRUE;
            break;
        }
    }
    if (bChgName)
        aRes = *GetExtraUINameArray()[*pIds];
    return aRes;
}

