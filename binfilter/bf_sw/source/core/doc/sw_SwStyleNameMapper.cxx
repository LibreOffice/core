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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#define _SVSTDARR_STRINGSDTOR
#include <bf_svtools/svstdarr.hxx>
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
namespace binfilter {
extern ResMgr* pSwResMgr;
// Initialise UI names to 0
SvStringsDtor	*SwStyleNameMapper::pTextUINameArray = 0,
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

NameToIdHash	*SwStyleNameMapper::pParaUIMap = 0,
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

/*N*/ sal_Bool SwStyleNameMapper::SuffixIsUser ( const String & rString )
/*N*/ {
/*N*/ 	const sal_Unicode *pChar = rString.GetBuffer();
/*N*/ 	sal_Int32 nLen = rString.Len();
/*N*/ 	return nLen <= 8 ? sal_False :
/*N*/ 		   pChar[nLen-7] == ' ' &&
/*N*/ 		   pChar[nLen-6] == '(' &&
/*N*/ 		   pChar[nLen-5] == 'u' &&
/*N*/ 		   pChar[nLen-4] == 's' &&
/*N*/ 		   pChar[nLen-3] == 'e' &&
/*N*/ 		   pChar[nLen-2] == 'r' &&
/*N*/ 		   pChar[nLen-1] == ')';
/*N*/ }
/*N*/ void SwStyleNameMapper::CheckSuffixAndDelete ( String & rString )
/*N*/ {
/*N*/ 	const sal_Unicode *pChar = rString.GetBuffer();
/*N*/ 	sal_Int32 nLen = rString.Len();
/*N*/ 	if (nLen > 8 &&
/*N*/ 		pChar[nLen-7] == ' ' &&
/*N*/ 		pChar[nLen-6] == '(' &&
/*N*/ 		pChar[nLen-5] == 'u' &&
/*N*/ 		pChar[nLen-4] == 's' &&
/*N*/ 		pChar[nLen-3] == 'e' &&
/*N*/ 		pChar[nLen-2] == 'r' &&
/*N*/ 		pChar[nLen-1] == ')')
/*N*/ 	{
/*?*/ 		rString.Erase ( nLen - 7, 7 );
/*N*/ 	}
/*N*/ }
/*N*/ const NameToIdHash & SwStyleNameMapper::getHashTable ( SwGetPoolIdFromName eFlags, sal_Bool bProgName )
/*N*/ {
/*N*/ 	NameToIdHash *pHash;
/*N*/ 	const SvStringsDtor *pStrings;
/*N*/ 
/*N*/ 	switch ( eFlags )
/*N*/ 	{
/*N*/ 		case GET_POOLID_TXTCOLL:
/*N*/ 		{
/*N*/ 			pHash = bProgName ? pParaProgMap : pParaUIMap;
/*N*/ 			if ( !pHash )
/*N*/ 			{
/*N*/ 				pHash = new NameToIdHash ( RES_POOLCOLL_TEXT_END - RES_POOLCOLL_TEXT_BEGIN +
/*N*/ 										   RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
/*N*/ 										   RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
/*N*/ 										   RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN +
/*N*/ 										   RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN +
/*N*/ 										   RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN );
/*N*/ 				pStrings = bProgName ? &GetTextProgNameArray() : &GetTextUINameArray();
                    sal_uInt16 nIndex=0;
                    sal_uInt16 nId;
/*N*/ 				for ( nIndex = 0, nId = RES_POOLCOLL_TEXT_BEGIN ; nId < RES_POOLCOLL_TEXT_END ; nId++,nIndex++ )
/*N*/ 					(*pHash)[(*pStrings)[nIndex]] = nId;
/*N*/ 				pStrings = bProgName ? &GetListsProgNameArray() : &GetListsUINameArray();
/*N*/ 				for ( nIndex = 0, nId = RES_POOLCOLL_LISTS_BEGIN ; nId < RES_POOLCOLL_LISTS_END ; nId++,nIndex++ )
/*N*/ 					(*pHash)[(*pStrings)[nIndex]] = nId;
/*N*/ 				pStrings = bProgName ? &GetExtraProgNameArray() : &GetExtraUINameArray();
/*N*/ 				for ( nIndex = 0, nId = RES_POOLCOLL_EXTRA_BEGIN ; nId < RES_POOLCOLL_EXTRA_END ; nId++,nIndex++ )
/*N*/ 					(*pHash)[(*pStrings)[nIndex]] = nId;
/*N*/ 				pStrings = bProgName ? &GetRegisterProgNameArray() : &GetRegisterUINameArray();
/*N*/ 				for ( nIndex = 0, nId = RES_POOLCOLL_REGISTER_BEGIN ; nId < RES_POOLCOLL_REGISTER_END ; nId++,nIndex++ )
/*N*/ 					(*pHash)[(*pStrings)[nIndex]] = nId;
/*N*/ 				pStrings = bProgName ? &GetDocProgNameArray() : &GetDocUINameArray();
/*N*/ 				for ( nIndex = 0, nId = RES_POOLCOLL_DOC_BEGIN ; nId < RES_POOLCOLL_DOC_END ; nId++,nIndex++ )
/*N*/ 					(*pHash)[(*pStrings)[nIndex]] = nId;
/*N*/ 				pStrings = bProgName ? &GetHTMLProgNameArray() : &GetHTMLUINameArray();
/*N*/ 				for ( nIndex = 0, nId = RES_POOLCOLL_HTML_BEGIN ; nId < RES_POOLCOLL_HTML_END ; nId++,nIndex++ )
/*N*/ 					(*pHash)[(*pStrings)[nIndex]] = nId;
/*N*/ 
/*N*/ 				if ( bProgName )
/*N*/ 					pParaProgMap = pHash;
/*N*/ 				else
/*N*/ 					pParaUIMap = pHash;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case GET_POOLID_CHRFMT:
/*N*/ 		{
/*N*/ 			pHash = bProgName ? pCharProgMap : pCharUIMap;
/*N*/ 			if ( !pHash )
/*N*/ 			{
/*N*/ 				pHash = new NameToIdHash ( RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN +
/*N*/ 										   RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN );
/*N*/ 				pStrings = bProgName ? &GetChrFmtProgNameArray() : &GetChrFmtUINameArray();
                    sal_uInt16 nIndex = 0;
                    sal_uInt16 nId;
/*N*/ 				for ( nIndex = 0, nId = RES_POOLCHR_NORMAL_BEGIN ; nId < RES_POOLCHR_NORMAL_END ; nId++,nIndex++ )
/*N*/ 					(*pHash)[(*pStrings)[nIndex]] = nId;
/*N*/ 				pStrings = bProgName ? &GetHTMLChrFmtProgNameArray() : &GetHTMLChrFmtUINameArray();
/*N*/ 				for ( nIndex = 0, nId = RES_POOLCHR_HTML_BEGIN ; nId < RES_POOLCHR_HTML_END ; nId++,nIndex++ )
/*N*/ 					(*pHash)[(*pStrings)[nIndex]] = nId;
/*N*/ 				if (bProgName )
/*N*/ 					pCharProgMap = pHash;
/*N*/ 				else
/*N*/ 					pCharUIMap = pHash;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case GET_POOLID_FRMFMT:
/*N*/ 		{
/*N*/ 			pHash = bProgName ? pFrameProgMap : pFrameUIMap;
/*N*/ 			if ( !pHash )
/*N*/ 			{
/*N*/ 				pHash = new NameToIdHash ( RES_POOLFRM_END - RES_POOLFRM_BEGIN );
/*N*/ 				pStrings = bProgName ? &GetFrmFmtProgNameArray() : &GetFrmFmtUINameArray();
/*N*/ 				for ( sal_uInt16 nIndex=0,nId = RES_POOLFRM_BEGIN ; nId < RES_POOLFRM_END ; nId++,nIndex++ )
/*N*/ 					(*pHash)[(*pStrings)[nIndex]] = nId;
/*N*/ 				if ( bProgName )
/*N*/ 					pFrameProgMap = pHash;
/*N*/ 				else
/*N*/ 					pFrameUIMap = pHash;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case GET_POOLID_PAGEDESC:
/*N*/ 		{
/*N*/ 			pHash = bProgName ? pPageProgMap : pPageUIMap;
/*N*/ 			if ( !pHash )
/*N*/ 			{
/*N*/ 				pHash = new NameToIdHash ( RES_POOLPAGE_END - RES_POOLPAGE_BEGIN );
/*N*/ 				pStrings = bProgName ? &GetPageDescProgNameArray() : &GetPageDescUINameArray();
/*N*/ 				for ( sal_uInt16 nIndex=0,nId = RES_POOLPAGE_BEGIN ; nId < RES_POOLPAGE_END ; nId++,nIndex++ )
/*N*/ 					(*pHash)[(*pStrings)[nIndex]] = nId;
/*N*/ 				if ( bProgName )
/*N*/ 					pPageProgMap = pHash;
/*N*/ 				else
/*N*/ 					pPageUIMap = pHash;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case GET_POOLID_NUMRULE:
/*N*/ 		{
/*N*/ 			pHash = bProgName ? pNumRuleProgMap : pNumRuleUIMap;
/*N*/ 			if ( !pHash )
/*N*/ 			{
/*N*/ 				pHash = new NameToIdHash ( RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN );
/*N*/ 				pStrings = bProgName ? &GetNumRuleProgNameArray() : &GetNumRuleUINameArray();
/*N*/ 				for ( sal_uInt16 nIndex=0,nId = RES_POOLNUMRULE_BEGIN ; nId < RES_POOLNUMRULE_END ; nId++,nIndex++ )
/*N*/ 					(*pHash)[(*pStrings)[nIndex]] = nId;
/*N*/ 				if ( bProgName )
/*N*/ 					pNumRuleProgMap = pHash;
/*N*/ 				else
/*N*/ 					pNumRuleUIMap = pHash;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ #ifdef _NEED_TO_DEBUG_MAPPING
/*N*/ 	static sal_Bool bTested = sal_False;
/*N*/ 	if ( !bTested )
/*N*/ 	{
/*N*/ 		bTested = sal_True;
/*N*/ 		{
/*N*/ 			for ( sal_uInt16 nIndex = 0, nId = RES_POOLCOLL_TEXT_BEGIN ; nId < RES_POOLCOLL_TEXT_END ; nId++,nIndex++ )
/*N*/ 			{
/*N*/ 				String aString, bString;
/*N*/ 				FillUIName ( nId, aString );
/*N*/ 				bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
/*N*/ 				sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
/*N*/ 				FillProgName ( nNewId, aString );
/*N*/ 				bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				if ( nNewId != nId )
/*N*/ 					*((sal_Int32*)0) = 42;
/*N*/ 			}
/*N*/ 			for ( nIndex = 0, nId = RES_POOLCOLL_LISTS_BEGIN ; nId < RES_POOLCOLL_LISTS_END ; nId++,nIndex++ )
/*N*/ 			{
/*N*/ 				String aString, bString;
/*N*/ 				FillUIName ( nId, aString );
/*N*/ 				bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
/*N*/ 				sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
/*N*/ 				FillProgName ( nNewId, aString );
/*N*/ 				bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				if ( nNewId != nId )
/*N*/ 					*((sal_Int32*)0) = 42;
/*N*/ 			}
/*N*/ 			for ( nIndex = 0, nId = RES_POOLCOLL_EXTRA_BEGIN ; nId < RES_POOLCOLL_EXTRA_END ; nId++,nIndex++ )
/*N*/ 			{
/*N*/ 				String aString, bString;
/*N*/ 				FillUIName ( nId, aString );
/*N*/ 				bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
/*N*/ 				sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
/*N*/ 				FillProgName ( nNewId, aString );
/*N*/ 				bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				if ( nNewId != nId )
/*N*/ 					*((sal_Int32*)0) = 42;
/*N*/ 			}
/*N*/ 			for ( nIndex = 0, nId = RES_POOLCOLL_REGISTER_BEGIN ; nId < RES_POOLCOLL_REGISTER_END ; nId++,nIndex++ )
/*N*/ 			{
/*N*/ 				String aString, bString;
/*N*/ 				FillUIName ( nId, aString );
/*N*/ 				bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
/*N*/ 				sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
/*N*/ 				FillProgName ( nNewId, aString );
/*N*/ 				bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				if ( nNewId != nId )
/*N*/ 					*((sal_Int32*)0) = 42;
/*N*/ 			}
/*N*/ 			for ( nIndex = 0, nId = RES_POOLCOLL_DOC_BEGIN ; nId < RES_POOLCOLL_DOC_END ; nId++,nIndex++ )
/*N*/ 			{
/*N*/ 				String aString, bString;
/*N*/ 				FillUIName ( nId, aString );
/*N*/ 				bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
/*N*/ 				sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
/*N*/ 				FillProgName ( nNewId, aString );
/*N*/ 				bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				if ( nNewId != nId )
/*N*/ 					*((sal_Int32*)0) = 42;
/*N*/ 			}
/*N*/ 			for ( nIndex = 0, nId = RES_POOLCOLL_HTML_BEGIN ; nId < RES_POOLCOLL_HTML_END ; nId++,nIndex++ )
/*N*/ 			{
/*N*/ 				String aString, bString;
/*N*/ 				FillUIName ( nId, aString );
/*N*/ 				bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
/*N*/ 				sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
/*N*/ 				FillProgName ( nNewId, aString );
/*N*/ 				bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				if ( nNewId != nId )
/*N*/ 					*((sal_Int32*)0) = 42;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		{
/*N*/ 			for ( sal_uInt16 nIndex = 0, nId = RES_POOLCHR_NORMAL_BEGIN ; nId < RES_POOLCHR_NORMAL_END ; nId++,nIndex++ )
/*N*/ 			{
/*N*/ 				String aString, bString;
/*N*/ 				FillUIName ( nId, aString );
/*N*/ 				bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
/*N*/ 				sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
/*N*/ 				FillProgName ( nNewId, aString );
/*N*/ 				bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				if ( nNewId != nId )
/*N*/ 					*((sal_Int32*)0) = 42;
/*N*/ 			}
/*N*/ 			for ( nIndex = 0, nId = RES_POOLCHR_HTML_BEGIN ; nId < RES_POOLCHR_HTML_END ; nId++,nIndex++ )
/*N*/ 			{
/*N*/ 				String aString, bString;
/*N*/ 				FillUIName ( nId, aString );
/*N*/ 				bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
/*N*/ 				sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
/*N*/ 				FillProgName ( nNewId, aString );
/*N*/ 				bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				if ( nNewId != nId )
/*N*/ 					*((sal_Int32*)0) = 42;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		{
/*N*/ 			for ( sal_uInt16 nIndex=0,nId = RES_POOLFRM_BEGIN ; nId < RES_POOLFRM_END ; nId++,nIndex++ )
/*N*/ 			{
/*N*/ 				String aString, bString;
/*N*/ 				FillUIName ( nId, aString );
/*N*/ 				bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
/*N*/ 				sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
/*N*/ 				FillProgName ( nNewId, aString );
/*N*/ 				bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				if ( nNewId != nId )
/*N*/ 					*((sal_Int32*)0) = 42;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		{
/*N*/ 			for ( sal_uInt16 nIndex=0,nId = RES_POOLPAGE_BEGIN ; nId < RES_POOLPAGE_END ; nId++,nIndex++ )
/*N*/ 			{
/*N*/ 				String aString, bString;
/*N*/ 				FillUIName ( nId, aString );
/*N*/ 				bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
/*N*/ 				sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
/*N*/ 				FillProgName ( nNewId, aString );
/*N*/ 				bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				if ( nNewId != nId )
/*N*/ 					*((sal_Int32*)0) = 42;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		{
/*N*/ 			for ( sal_uInt16 nIndex=0,nId = RES_POOLNUMRULE_BEGIN ; nId < RES_POOLNUMRULE_END ; nId++,nIndex++ )
/*N*/ 			{
/*N*/ 				String aString, bString;
/*N*/ 				FillUIName ( nId, aString );
/*N*/ 				bString = GetProgName ( GET_POOLID_TXTCOLL, aString );
/*N*/ 				sal_uInt16 nNewId = GetPoolIdFromProgName ( bString, GET_POOLID_TXTCOLL );
/*N*/ 				FillProgName ( nNewId, aString );
/*N*/ 				bString = GetUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				nNewId = GetPoolIdFromUIName ( aString, GET_POOLID_TXTCOLL );
/*N*/ 				if ( nNewId != nId )
/*N*/ 					*((sal_Int32*)0) = 42;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/ 	return *pHash;
/*N*/ }
// This gets the UI Name from the programmatic name
/*N*/ const String& SwStyleNameMapper::GetUIName ( const String& rName, SwGetPoolIdFromName eFlags )
/*N*/ {
/*N*/ 	sal_uInt16 nId = GetPoolIdFromProgName ( rName, eFlags );
/*N*/ 	return nId != USHRT_MAX ? GetUIName( nId, rName ) : rName;
/*N*/ }


// Get the programmatic Name from the UI name
/*N*/ const String& SwStyleNameMapper::GetProgName( const String& rName, SwGetPoolIdFromName eFlags )
/*N*/ {
/*N*/ 	sal_uInt16 nId = GetPoolIdFromUIName ( rName, eFlags );
/*N*/ 	return nId != USHRT_MAX ? GetProgName( nId, rName ) : rName;
/*N*/ }

// Get the programmatic name from the UI name in rName and put it into rFillName
/*N*/ void SwStyleNameMapper::FillProgName ( const String& rName, String& rFillName, SwGetPoolIdFromName eFlags, sal_Bool bDisambiguate )
/*N*/ {
/*N*/ 	sal_uInt16 nId = GetPoolIdFromUIName ( rName, eFlags );
/*N*/ 	if ( bDisambiguate && nId == USHRT_MAX )
/*N*/ 	{
/*N*/ 		// rName isn't in our UI name table...check if it's in the programmatic one
/*N*/ 		nId = GetPoolIdFromProgName ( rName, eFlags );
/*N*/ 
/*N*/ 		rFillName = rName;
/*N*/ 		if (nId == USHRT_MAX )
/*N*/ 		{
/*N*/ 			// It isn't ...make sure the suffix isn't already " (user)"...if it is,
/*N*/ 			// we need to add another one
/*N*/ 			if ( SuffixIsUser ( rFillName ) )
/*?*/ 				rFillName.AppendAscii ( RTL_CONSTASCII_STRINGPARAM ( " (user)" ) );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// It's in the programmatic name table...append suffix
/*N*/ 			rFillName.AppendAscii ( RTL_CONSTASCII_STRINGPARAM ( " (user)" ) );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// If we aren't trying to disambiguate, then just do a normal fill
/*?*/ 		fillNameFromId ( nId, rFillName, sal_True);
/*N*/ 	}
/*N*/ }
// Get the UI name from the programmatic name in rName and put it into rFillName
/*N*/ void SwStyleNameMapper::FillUIName ( const String& rName, String& rFillName, SwGetPoolIdFromName eFlags, sal_Bool bDisambiguate )
/*N*/ {
/*N*/ 	sal_uInt16 nId = GetPoolIdFromProgName ( rName, eFlags );
/*N*/ 	if ( bDisambiguate && nId == USHRT_MAX )
/*N*/ 	{
/*N*/ 		rFillName = rName;
/*N*/ 		// rName isn't in our Prog name table...check if it has a " (user)" suffix, if so remove it
/*N*/ 		CheckSuffixAndDelete ( rFillName );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// If we aren't trying to disambiguate, then just do a normal fill
/*N*/ 		fillNameFromId ( nId, rFillName, sal_False);
/*N*/ 	}
/*N*/ }

/*N*/ const String& SwStyleNameMapper::getNameFromId( sal_uInt16 nId, const String& rFillName, sal_Bool bProgName )
/*N*/ {
/*N*/ 	sal_uInt16 nStt = 0;
/*N*/ 	const SvStringsDtor* pStrArr = 0;
/*N*/ 
/*N*/ 	switch( (USER_FMT | COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID) & nId )
/*N*/ 	{
/*N*/ 	case COLL_TEXT_BITS:
/*?*/ 		if( RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END )
/*?*/ 		{
/*?*/ 			pStrArr = bProgName ? &GetTextProgNameArray() : &GetTextUINameArray();
/*?*/ 			nStt = RES_POOLCOLL_TEXT_BEGIN;
/*?*/ 		}
/*N*/ 		break;
/*N*/ 	case COLL_LISTS_BITS:
/*?*/ 		if( RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END )
/*?*/ 		{
/*?*/ 			pStrArr = bProgName ? &GetListsProgNameArray() : &GetListsUINameArray();
/*?*/ 			nStt = RES_POOLCOLL_LISTS_BEGIN;
/*?*/ 		}
/*N*/ 		break;
/*N*/ 	case COLL_EXTRA_BITS:
/*N*/ 		if( RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetExtraProgNameArray() : &GetExtraUINameArray();
/*N*/ 			nStt = RES_POOLCOLL_EXTRA_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case COLL_REGISTER_BITS:
/*?*/ 		if( RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END )
/*?*/ 		{
/*?*/ 			pStrArr = bProgName ? &GetRegisterProgNameArray() : &GetRegisterUINameArray();
/*?*/ 			nStt = RES_POOLCOLL_REGISTER_BEGIN;
/*?*/ 		}
/*N*/ 		break;
/*N*/ 	case COLL_DOC_BITS:
/*?*/ 		if( RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END )
/*?*/ 		{
/*?*/ 			pStrArr = bProgName ? &GetDocProgNameArray() : &GetDocUINameArray();
/*?*/ 			nStt = RES_POOLCOLL_DOC_BEGIN;
/*?*/ 		}
/*?*/ 		break;
/*N*/ 	case COLL_HTML_BITS:
/*?*/ 		if( RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END )
/*?*/ 		{
/*?*/ 			pStrArr = bProgName ? &GetHTMLProgNameArray() : &GetHTMLUINameArray();
/*?*/ 			nStt = RES_POOLCOLL_HTML_BEGIN;
/*?*/ 		}
/*?*/ 		break;
/*N*/ 	case POOLGRP_CHARFMT:
/*?*/ 		if( RES_POOLCHR_NORMAL_BEGIN <= nId && nId < RES_POOLCHR_NORMAL_END )
/*?*/ 		{
/*?*/ 			pStrArr = bProgName ? &GetChrFmtProgNameArray() : &GetChrFmtUINameArray();
/*?*/ 			nStt = RES_POOLCHR_NORMAL_BEGIN;
/*?*/ 		}
/*?*/ 		else if( RES_POOLCHR_HTML_BEGIN <= nId && nId < RES_POOLCHR_HTML_END )
/*?*/ 		{
/*?*/ 			pStrArr = bProgName ? &GetHTMLChrFmtProgNameArray() : &GetHTMLChrFmtUINameArray();
/*?*/ 			nStt = RES_POOLCHR_HTML_BEGIN;
/*?*/ 		}
/*?*/ 		break;
/*N*/ 	case POOLGRP_FRAMEFMT:
/*N*/ 		if( RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetFrmFmtProgNameArray() : &GetFrmFmtUINameArray();
/*N*/ 			nStt = RES_POOLFRM_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case POOLGRP_PAGEDESC:
/*?*/ 		if( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END )
/*?*/ 		{
/*?*/ 			pStrArr = bProgName ? &GetPageDescProgNameArray() : &GetPageDescUINameArray();
/*?*/ 			nStt = RES_POOLPAGE_BEGIN;
/*?*/ 		}
/*?*/ 		break;
/*N*/ 	case POOLGRP_NUMRULE:
/*?*/ 		if( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END )
/*?*/ 		{
/*?*/ 			pStrArr = bProgName ? &GetNumRuleProgNameArray() : &GetNumRuleUINameArray();
/*?*/ 			nStt = RES_POOLNUMRULE_BEGIN;
/*?*/ 		}
/*?*/ 		break;
/*?*/ 	}
/*N*/ 	return pStrArr ? *(pStrArr->operator[] ( nId - nStt ) ) : rFillName;
/*N*/ }
/*N*/ void SwStyleNameMapper::fillNameFromId( sal_uInt16 nId, String& rFillName, sal_Bool bProgName )
/*N*/ {
/*N*/ 	sal_uInt16 nStt = 0;
/*N*/ 	const SvStringsDtor* pStrArr = 0;
/*N*/ 
/*N*/ 	switch( (USER_FMT | COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID) & nId )
/*N*/ 	{
/*N*/ 	case COLL_TEXT_BITS:
/*N*/ 		if( RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetTextProgNameArray() : &GetTextUINameArray();
/*N*/ 			nStt = RES_POOLCOLL_TEXT_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case COLL_LISTS_BITS:
/*N*/ 		if( RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetListsProgNameArray() : &GetListsUINameArray();
/*N*/ 			nStt = RES_POOLCOLL_LISTS_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case COLL_EXTRA_BITS:
/*N*/ 		if( RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetExtraProgNameArray() : &GetExtraUINameArray();
/*N*/ 			nStt = RES_POOLCOLL_EXTRA_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case COLL_REGISTER_BITS:
/*N*/ 		if( RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetRegisterProgNameArray() : &GetRegisterUINameArray();
/*N*/ 			nStt = RES_POOLCOLL_REGISTER_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case COLL_DOC_BITS:
/*N*/ 		if( RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetDocProgNameArray() : &GetDocUINameArray();
/*N*/ 			nStt = RES_POOLCOLL_DOC_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case COLL_HTML_BITS:
/*N*/ 		if( RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetHTMLProgNameArray() : &GetHTMLUINameArray();
/*N*/ 			nStt = RES_POOLCOLL_HTML_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case POOLGRP_CHARFMT:
/*N*/ 		if( RES_POOLCHR_NORMAL_BEGIN <= nId && nId < RES_POOLCHR_NORMAL_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetChrFmtProgNameArray() : &GetChrFmtUINameArray();
/*N*/ 			nStt = RES_POOLCHR_NORMAL_BEGIN;
/*N*/ 		}
/*N*/ 		else if( RES_POOLCHR_HTML_BEGIN <= nId && nId < RES_POOLCHR_HTML_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetHTMLChrFmtProgNameArray() : &GetHTMLChrFmtUINameArray();
/*N*/ 			nStt = RES_POOLCHR_HTML_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case POOLGRP_FRAMEFMT:
/*N*/ 		if( RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetFrmFmtProgNameArray() : &GetFrmFmtUINameArray();
/*N*/ 			nStt = RES_POOLFRM_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case POOLGRP_PAGEDESC:
/*N*/ 		if( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetPageDescProgNameArray() : &GetPageDescUINameArray();
/*N*/ 			nStt = RES_POOLPAGE_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case POOLGRP_NUMRULE:
/*N*/ 		if( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END )
/*N*/ 		{
/*N*/ 			pStrArr = bProgName ? &GetNumRuleProgNameArray() : &GetNumRuleUINameArray();
/*N*/ 			nStt = RES_POOLNUMRULE_BEGIN;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	if (pStrArr)
/*N*/ 		rFillName = *(pStrArr->operator[] ( nId - nStt ) );
/*N*/ }
// Get the UI Name from the pool ID
/*N*/ void SwStyleNameMapper::FillUIName ( sal_uInt16 nId, String& rFillName )
/*N*/ {
/*N*/ 	fillNameFromId ( nId, rFillName, sal_False );
/*N*/ }
// Get the UI Name from the pool ID
/*N*/ const String& SwStyleNameMapper::GetUIName ( sal_uInt16 nId, const String& rName )
/*N*/ {
/*N*/ 	return getNameFromId ( nId, rName, sal_False );
/*N*/ }

// Get the programmatic Name from the pool ID
/*N*/ void SwStyleNameMapper::FillProgName ( sal_uInt16 nId, String& rFillName )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }
// Get the programmatic Name from the pool ID
/*N*/ const String& SwStyleNameMapper::GetProgName ( sal_uInt16 nId, const String& rName )
/*N*/ {
/*N*/ 	return getNameFromId ( nId, rName, sal_True );
/*N*/ }
// This gets the PoolId from the UI Name
/*N*/ sal_uInt16 SwStyleNameMapper::GetPoolIdFromUIName( const String& rName, SwGetPoolIdFromName eFlags )
/*N*/ {
/*N*/ 	const NameToIdHash & rHashMap = getHashTable ( eFlags, sal_False );
/*N*/ 	NameToIdHash::const_iterator aIter = rHashMap.find ( &rName );
/*N*/ 	return aIter != rHashMap.end() ? (*aIter).second : USHRT_MAX;
/*N*/ }
// Get the Pool ID from the programmatic name
/*N*/ sal_uInt16 SwStyleNameMapper::GetPoolIdFromProgName( const String& rName, SwGetPoolIdFromName eFlags )
/*N*/ {
/*N*/ 	const NameToIdHash & rHashMap = getHashTable ( eFlags, sal_True );
/*N*/ 	NameToIdHash::const_iterator aIter = rHashMap.find ( &rName );
/*N*/ 	return aIter != rHashMap.end() ? (*aIter).second : USHRT_MAX;
/*N*/ }

/*N*/ SvStringsDtor* SwStyleNameMapper::NewUINameArray( SvStringsDtor*& pNameArray, sal_uInt16 nStt, sal_uInt16 nEnd )
/*N*/ {
/*N*/ 	if( !pNameArray )
/*N*/ 	{
/*N*/ 		pNameArray = new SvStringsDtor( static_cast < sal_Int8 > (nEnd - nStt), 1 );
/*N*/ 		while( nStt < nEnd )
/*N*/ 		{
/*N*/ 			const ResId rRId( nStt, *pSwResMgr );
/*N*/ 			String* pStr = new String( rRId );
/*N*/ 			pNameArray->Insert( pStr, pNameArray->Count() );
/*N*/ 			++nStt;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pNameArray;
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetTextUINameArray()
/*N*/ {
/*N*/ 	return pTextUINameArray ? *pTextUINameArray :
/*N*/ 		   *NewUINameArray( pTextUINameArray, RC_POOLCOLL_TEXT_BEGIN,
/*N*/ 			( RC_POOLCOLL_TEXT_BEGIN +
/*N*/ 					(RES_POOLCOLL_TEXT_END - RES_POOLCOLL_TEXT_BEGIN )) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetListsUINameArray()
/*N*/ {
/*N*/ 	return pListsUINameArray ? *pListsUINameArray :
/*N*/ 		   *NewUINameArray( pListsUINameArray, RC_POOLCOLL_LISTS_BEGIN,
/*N*/ 			( RC_POOLCOLL_LISTS_BEGIN +
/*N*/ 					(RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN )) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetExtraUINameArray()
/*N*/ {
/*N*/ 	return pExtraUINameArray ? *pExtraUINameArray :
/*N*/ 		   *NewUINameArray( pExtraUINameArray, RC_POOLCOLL_EXTRA_BEGIN,
/*N*/ 				( RC_POOLCOLL_EXTRA_BEGIN +
/*N*/ 					(RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN )) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetRegisterUINameArray()
/*N*/ {
/*N*/ 	return pRegisterUINameArray ? *pRegisterUINameArray :
/*N*/ 		   *NewUINameArray( pRegisterUINameArray, RC_POOLCOLL_REGISTER_BEGIN,
/*N*/ 			( RC_POOLCOLL_REGISTER_BEGIN +
/*N*/ 				(RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN )) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetDocUINameArray()
/*N*/ {
/*N*/ 	return pDocUINameArray ? *pDocUINameArray :
/*N*/ 		   *NewUINameArray( pDocUINameArray, RC_POOLCOLL_DOC_BEGIN,
/*N*/ 					( RC_POOLCOLL_DOC_BEGIN +
/*N*/ 						(RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN )) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetHTMLUINameArray()
/*N*/ {
/*N*/ 	return pHTMLUINameArray ? *pHTMLUINameArray :
/*N*/ 		   *NewUINameArray( pHTMLUINameArray, RC_POOLCOLL_HTML_BEGIN,
/*N*/ 					( RC_POOLCOLL_HTML_BEGIN +
/*N*/ 						(RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN )) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetFrmFmtUINameArray()
/*N*/ {
/*N*/ 	return pFrmFmtUINameArray ? *pFrmFmtUINameArray :
/*N*/ 		   *NewUINameArray( pFrmFmtUINameArray, RC_POOLFRMFMT_BEGIN,
/*N*/ 					( RC_POOLFRMFMT_BEGIN +
/*N*/ 						(RES_POOLFRM_END - RES_POOLFRM_BEGIN )) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetChrFmtUINameArray()
/*N*/ {
/*N*/ 	return pChrFmtUINameArray ? *pChrFmtUINameArray :
/*N*/ 		   *NewUINameArray( pChrFmtUINameArray, RC_POOLCHRFMT_BEGIN,
/*N*/ 			( RC_POOLCHRFMT_BEGIN +
/*N*/ 					(RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN )) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetHTMLChrFmtUINameArray()
/*N*/ {
/*N*/ 	return pHTMLChrFmtUINameArray ? *pHTMLChrFmtUINameArray :
/*N*/ 		   *NewUINameArray( pHTMLChrFmtUINameArray, RC_POOLCHRFMT_HTML_BEGIN,
/*N*/ 			( RC_POOLCHRFMT_HTML_BEGIN +
/*N*/ 					(RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN )) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetPageDescUINameArray()
/*N*/ {
/*N*/ 	return pPageDescUINameArray ? *pPageDescUINameArray :
/*N*/ 		   *NewUINameArray( pPageDescUINameArray, RC_POOLPAGEDESC_BEGIN,
/*N*/ 			( RC_POOLPAGEDESC_BEGIN +
/*N*/ 					(RES_POOLPAGE_END - RES_POOLPAGE_BEGIN )) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetNumRuleUINameArray()
/*N*/ {
/*N*/ 	return pNumRuleUINameArray ? *pNumRuleUINameArray :
/*N*/ 	       *NewUINameArray( pNumRuleUINameArray, RC_POOLNUMRULE_BEGIN,
/*N*/ 			( RC_POOLNUMRULE_BEGIN +
/*N*/ 					(RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN )) );
/*N*/ }

/*N*/ SvStringsDtor* SwStyleNameMapper::NewProgNameArray( SvStringsDtor*& pProgNameArray, const SwTableEntry *pTable, sal_uInt8 nCount )
/*N*/ {
/*N*/ 	if( !pProgNameArray )
/*N*/ 	{
/*N*/ 		pProgNameArray = new SvStringsDtor( nCount, 1 );
/*N*/ 		while (pTable->nLength)
/*N*/ 		{
/*N*/ 			String* pStr = new String( pTable->pChar, pTable->nLength, RTL_TEXTENCODING_ASCII_US );
/*N*/ 			pProgNameArray->Insert( pStr, pProgNameArray->Count() );
/*N*/ 			pTable++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pProgNameArray;
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetTextProgNameArray()
/*N*/ {
/*N*/ 	return pTextProgNameArray ? *pTextProgNameArray :
/*N*/ 		   *NewProgNameArray( pTextProgNameArray, TextProgNameTable, 
/*N*/ 			sizeof ( TextProgNameTable ) / sizeof ( SwTableEntry ) ); 
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetListsProgNameArray()
/*N*/ {
/*N*/ 	return pListsProgNameArray ? *pListsProgNameArray : 
/*N*/ 		   *NewProgNameArray( pListsProgNameArray, ListsProgNameTable, 
/*N*/ 			sizeof ( ListsProgNameTable ) / sizeof ( SwTableEntry ) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetExtraProgNameArray()
/*N*/ {
/*N*/ 	return pExtraProgNameArray ? *pExtraProgNameArray : 
/*N*/ 		   *NewProgNameArray( pExtraProgNameArray, ExtraProgNameTable, 
/*N*/ 			sizeof ( ExtraProgNameTable ) / sizeof ( SwTableEntry ) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetRegisterProgNameArray()
/*N*/ {
/*N*/ 	return pRegisterProgNameArray ? *pRegisterProgNameArray : 
/*N*/ 		   *NewProgNameArray( pRegisterProgNameArray, RegisterProgNameTable, 
/*N*/ 			sizeof ( RegisterProgNameTable ) / sizeof ( SwTableEntry ) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetDocProgNameArray()
/*N*/ {
/*N*/ 	return pDocProgNameArray ? *pDocProgNameArray : 
/*N*/ 		   *NewProgNameArray( pDocProgNameArray, DocProgNameTable, 
/*N*/ 			sizeof ( DocProgNameTable ) / sizeof ( SwTableEntry ) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetHTMLProgNameArray()
/*N*/ {
/*N*/ 	return pHTMLProgNameArray ? *pHTMLProgNameArray : 
/*N*/ 		   *NewProgNameArray( pHTMLProgNameArray, HTMLProgNameTable, 
/*N*/ 			sizeof ( HTMLProgNameTable ) / sizeof ( SwTableEntry ) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetFrmFmtProgNameArray()
/*N*/ {
/*N*/ 	return pFrmFmtProgNameArray ? *pFrmFmtProgNameArray : 
/*N*/ 		   *NewProgNameArray( pFrmFmtProgNameArray, FrmFmtProgNameTable, 
/*N*/ 			sizeof ( FrmFmtProgNameTable ) / sizeof ( SwTableEntry ) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetChrFmtProgNameArray()
/*N*/ {
/*N*/ 	return pChrFmtProgNameArray ? *pChrFmtProgNameArray : 
/*N*/ 		   *NewProgNameArray( pChrFmtProgNameArray, ChrFmtProgNameTable, 
/*N*/ 			sizeof ( ChrFmtProgNameTable ) / sizeof ( SwTableEntry ) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetHTMLChrFmtProgNameArray()
/*N*/ {
/*N*/ 	return pHTMLChrFmtProgNameArray ? *pHTMLChrFmtProgNameArray : 
/*N*/ 		   *NewProgNameArray( pHTMLChrFmtProgNameArray, HTMLChrFmtProgNameTable, 
/*N*/ 			sizeof ( HTMLChrFmtProgNameTable ) / sizeof ( SwTableEntry ) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetPageDescProgNameArray()
/*N*/ {
/*N*/ 	return pPageDescProgNameArray ? *pPageDescProgNameArray : 
/*N*/ 		   *NewProgNameArray( pPageDescProgNameArray, PageDescProgNameTable, 
/*N*/ 			sizeof ( PageDescProgNameTable ) / sizeof ( SwTableEntry ) );
/*N*/ }

/*N*/ const SvStringsDtor& SwStyleNameMapper::GetNumRuleProgNameArray()
/*N*/ {
/*N*/ 	return pNumRuleProgNameArray ? *pNumRuleProgNameArray : 
/*N*/ 		   *NewProgNameArray( pNumRuleProgNameArray, NumRuleProgNameTable, 
/*N*/ 			sizeof ( NumRuleProgNameTable ) / sizeof ( SwTableEntry ) );
/*N*/ }


/*N*/ const String SwStyleNameMapper::GetSpecialExtraProgName( const String& rExtraUIName )
/*N*/ {
/*N*/     String aRes = rExtraUIName;
/*N*/     BOOL bChgName = FALSE;
/*N*/     const SvStringsDtor& rExtraArr = GetExtraUINameArray();
/*N*/     static sal_uInt16 nIds[] =
/*N*/     {
/*N*/         RES_POOLCOLL_LABEL_DRAWING - RES_POOLCOLL_EXTRA_BEGIN,
/*N*/         RES_POOLCOLL_LABEL_ABB - RES_POOLCOLL_EXTRA_BEGIN,
/*N*/         RES_POOLCOLL_LABEL_TABLE - RES_POOLCOLL_EXTRA_BEGIN,
/*N*/         RES_POOLCOLL_LABEL_FRAME- RES_POOLCOLL_EXTRA_BEGIN,
/*N*/         0
/*N*/     };
          const sal_uInt16 *pIds;
/*N*/     for ( pIds = nIds; *pIds; ++pIds)
/*N*/     {
/*N*/         if (aRes == *rExtraArr[ *pIds ])
/*N*/         {
/*N*/             bChgName = TRUE;
/*N*/             break;
/*N*/         }
/*N*/     }
/*N*/     if (bChgName)
/*N*/         aRes = *GetExtraProgNameArray()[*pIds];
/*N*/     return aRes;
/*N*/ }

/*N*/ const String SwStyleNameMapper::GetSpecialExtraUIName( const String& rExtraProgName )
/*N*/ {
/*?*/     String aRes = rExtraProgName;
/*?*/     BOOL bChgName = FALSE;
/*?*/     const SvStringsDtor& rExtraArr = GetExtraProgNameArray();
/*?*/     static sal_uInt16 nIds[] =
/*?*/     {
/*?*/         RES_POOLCOLL_LABEL_DRAWING - RES_POOLCOLL_EXTRA_BEGIN,
/*?*/         RES_POOLCOLL_LABEL_ABB - RES_POOLCOLL_EXTRA_BEGIN,
/*?*/         RES_POOLCOLL_LABEL_TABLE - RES_POOLCOLL_EXTRA_BEGIN,
/*?*/         RES_POOLCOLL_LABEL_FRAME- RES_POOLCOLL_EXTRA_BEGIN,
/*?*/         0
/*?*/     };
          const sal_uInt16 *pIds;
/*?*/     for ( pIds = nIds; *pIds; ++pIds)
/*?*/     {
/*?*/         if (aRes == *rExtraArr[ *pIds ])
/*?*/         {
/*?*/             bChgName = TRUE;
/*?*/             break;
/*?*/         }
/*?*/     }
/*?*/     if (bChgName)
/*?*/         aRes = *GetExtraUINameArray()[*pIds];
/*?*/     return aRes;
/*N*/ }

}
