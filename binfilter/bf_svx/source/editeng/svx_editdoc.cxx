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

#ifndef _EEITEM_HXX
#include "eeitem.hxx"
#endif
#ifndef _EEITEMID_HXX
#include "eeitemid.hxx"
#endif

#include <tstpitem.hxx>
#include <fontitem.hxx>
#include <crsditem.hxx>
#include <fhgtitem.hxx>
#include <postitem.hxx>
#include <kernitem.hxx>
#include <wrlmitem.hxx>
#include <wghtitem.hxx>
#include <udlnitem.hxx>
#include <cntritem.hxx>
#include <escpitem.hxx>
#include <shdditem.hxx>
#include <akrnitem.hxx>
#include <cscoitem.hxx>
#include <langitem.hxx>
#include <emphitem.hxx>
#include <charreliefitem.hxx>

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#include <editdoc.hxx>
#include <eerdll.hxx>
#include <eerdll2.hxx>

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#include <tools/shl.hxx>

#ifndef _COM_SUN_STAR_TEXT_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif

#include <stdlib.h>	// qsort
namespace binfilter {

using namespace ::com::sun::star;


// ------------------------------------------------------------

/*N*/ USHORT GetScriptItemId( USHORT nItemId, short nScriptType )
/*N*/ {
/*N*/ 	USHORT nId = nItemId;
/*N*/ 	
/*N*/ 	if ( ( nScriptType == i18n::ScriptType::ASIAN ) ||
/*N*/ 		 ( nScriptType == i18n::ScriptType::COMPLEX ) )
/*N*/ 	{
/*N*/ 		switch ( nItemId ) 
/*N*/ 		{
/*N*/ 			case EE_CHAR_LANGUAGE:
/*N*/ 				nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_LANGUAGE_CJK : EE_CHAR_LANGUAGE_CTL;
/*N*/ 			break;
/*N*/ 			case EE_CHAR_FONTINFO:
/*N*/ 				nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_FONTINFO_CJK : EE_CHAR_FONTINFO_CTL;
/*N*/ 			break;
/*N*/ 			case EE_CHAR_FONTHEIGHT:
/*N*/ 				nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_FONTHEIGHT_CJK : EE_CHAR_FONTHEIGHT_CTL;
/*N*/ 			break;
/*N*/ 			case EE_CHAR_WEIGHT:
/*N*/ 				nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_WEIGHT_CJK : EE_CHAR_WEIGHT_CTL;
/*N*/ 			break;
/*N*/ 			case EE_CHAR_ITALIC:
/*N*/ 				nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_ITALIC_CJK : EE_CHAR_ITALIC_CTL;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	
/*N*/ 	return nId;
/*N*/ }

/*N*/ BOOL IsScriptItemValid( USHORT nItemId, short nScriptType )
/*N*/ {
/*N*/ 	BOOL bValid = TRUE;
/*N*/ 	
/*N*/ 	switch ( nItemId ) 
/*N*/ 	{
/*N*/ 		case EE_CHAR_LANGUAGE:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::LATIN;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_LANGUAGE_CJK:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::ASIAN;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_LANGUAGE_CTL:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::COMPLEX;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_FONTINFO:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::LATIN;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_FONTINFO_CJK:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::ASIAN;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_FONTINFO_CTL:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::COMPLEX;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_FONTHEIGHT:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::LATIN;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_FONTHEIGHT_CJK:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::ASIAN;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_FONTHEIGHT_CTL:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::COMPLEX;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_WEIGHT:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::LATIN;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_WEIGHT_CJK:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::ASIAN;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_WEIGHT_CTL:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::COMPLEX;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_ITALIC:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::LATIN;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_ITALIC_CJK:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::ASIAN;
/*N*/ 		break;
/*N*/ 		case EE_CHAR_ITALIC_CTL:
/*N*/ 			bValid = nScriptType == i18n::ScriptType::COMPLEX;
/*N*/ 		break;
/*N*/ 	}
/*N*/ 
/*N*/ 	return bValid;
/*N*/ }


// ------------------------------------------------------------

// Sollte spaeter zentral nach TOOLS/STRING (Aktuell: 303)
// fuer Grep: WS_TARGET

/*N*/ DBG_NAME( EE_TextPortion )
/*N*/ DBG_NAME( EE_EditLine )
/*N*/ DBG_NAME( EE_ContentNode )
/*N*/ DBG_NAME( EE_CharAttribList )

/*N*/ SfxItemInfo aItemInfos[EDITITEMCOUNT] = {
/*N*/ 		{ SID_ATTR_FRAMEDIRECTION, SFX_ITEM_POOLABLE },         // EE_PARA_WRITINGDIR
/*N*/ 		{ 0, SFX_ITEM_POOLABLE },								// EE_PARA_XMLATTRIBS
/*N*/ 		{ SID_ATTR_PARA_HANGPUNCTUATION, SFX_ITEM_POOLABLE },	// EE_PARA_HANGINGPUNCTUATION
/*N*/ 		{ SID_ATTR_PARA_FORBIDDEN_RULES, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_PARA_SCRIPTSPACE, SFX_ITEM_POOLABLE },	    // EE_PARA_ASIANCJKSPACING
/*N*/ 		{ SID_ATTR_NUMBERING_RULE, SFX_ITEM_POOLABLE },		    // EE_PARA_NUMBULL
/*N*/ 		{ 0, SFX_ITEM_POOLABLE },							    // EE_PARA_HYPHENATE
/*N*/ 		{ 0, SFX_ITEM_POOLABLE },							    // EE_PARA_BULLETSTATE
/*N*/ 		{ 0, SFX_ITEM_POOLABLE },							    // EE_PARA_OUTLLRSPACE
/*N*/ 		{ SID_ATTR_PARA_OUTLLEVEL, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_PARA_BULLET, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_LRSPACE, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_ULSPACE, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_PARA_LINESPACE, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_PARA_ADJUST, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_TABSTOP, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_COLOR, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_FONT, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEM_POOLABLE },
/*N*/         { SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_WEIGHT, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_UNDERLINE, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_STRIKEOUT, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_POSTURE, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_CONTOUR, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_SHADOWED, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_AUTOKERN, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_KERNING, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_LANGUAGE, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_CJK_FONT, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_CTL_FONT, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEM_POOLABLE },
/*N*/ 		{ SID_ATTR_CHAR_RELIEF, SFX_ITEM_POOLABLE },
/*N*/ 		{ 0, SFX_ITEM_POOLABLE },							// EE_CHAR_RUBI_DUMMY
/*N*/ 		{ 0, SFX_ITEM_POOLABLE },							// EE_CHAR_XMLATTRIBS
/*N*/ 		{ 0, SFX_ITEM_POOLABLE },							// EE_FEATURE_TAB
/*N*/ 		{ 0, SFX_ITEM_POOLABLE },							// EE_FEATURE_LINEBR
/*N*/ 		{ SID_ATTR_CHAR_CHARSETCOLOR, SFX_ITEM_POOLABLE },	// EE_FEATURE_NOTCONV
/*N*/ 		{ SID_FIELD, SFX_ITEM_POOLABLE }
/*N*/ };

/*N*/ USHORT aV1Map[] = {
/*N*/ 	3999, 4001, 4002, 4003, 4004, 4005, 4006,
/*N*/ 	4007, 4008, 4009, 4010, 4011, 4012, 4013, 4017, 4018, 4019 // MI: 4019?
/*N*/ };

/*N*/ USHORT aV2Map[] = {
/*N*/ 	3999, 4000, 4001, 4002, 4003, 4004, 4005, 4006,	4007, 4008, 4009,
/*N*/ 	4010, 4011, 4012, 4013, 4014, 4015, 4016, 4018, 4019, 4020
/*N*/ };

/*N*/ USHORT aV3Map[] = {
/*N*/ 	3997, 3998, 3999, 4000, 4001, 4002, 4003, 4004, 4005, 4006,	4007,
/*N*/ 	4009, 4010, 4011, 4012, 4013, 4014, 4015, 4016, 4017, 4018, 4019,
/*N*/ 	4020, 4021
/*N*/ };

/*N*/ USHORT aV4Map[] = {
/*N*/ 	3994, 3995, 3996, 3997, 3998, 3999, 4000, 4001, 4002, 4003, 
/*N*/ 	4004, 4005, 4006, 4007, 4008, 4009, 4010, 4011, 4012, 4013, 
/*N*/ 	4014, 4015, 4016, 4017, 4018, 
/*N*/ 	/* CJK Items inserted here: EE_CHAR_LANGUAGE - EE_CHAR_XMLATTRIBS */
/*N*/ 	4034, 4035, 4036, 4037 
/*N*/ };

/*N*/ SV_IMPL_PTRARR( ContentList, ContentNode* );
/*N*/ SV_IMPL_VARARR( ScriptTypePosInfos, ScriptTypePosInfo );
/*N*/ SV_IMPL_VARARR( WritingDirectionInfos, WritingDirectionInfo );
// SV_IMPL_VARARR( ExtraCharInfos, ExtraCharInfo );


/*N*/ int SAL_CALL CompareStart( const void* pFirst, const void* pSecond )
/*N*/ {
/*N*/ 	if ( (*((EditCharAttrib**)pFirst))->GetStart() < (*((EditCharAttrib**)pSecond))->GetStart() )
/*N*/ 		return (-1);
/*N*/ 	else if ( (*((EditCharAttrib**)pFirst))->GetStart() > (*((EditCharAttrib**)pSecond))->GetStart() )
/*N*/ 		return (1);
/*N*/ 	return 0;
/*N*/ }

/*N*/ EditCharAttrib* MakeCharAttrib( SfxItemPool& rPool, const SfxPoolItem& rAttr, USHORT nS, USHORT nE )
/*N*/ {
/*N*/ 	// das neue Attribut im Pool anlegen
/*N*/ 	const SfxPoolItem& rNew = rPool.Put( rAttr );
/*N*/ 
/*N*/ 	EditCharAttrib* pNew = 0;
/*N*/ 	switch( rNew.Which() )
/*N*/ 	{
/*N*/ 		case EE_CHAR_LANGUAGE:
/*N*/ 		case EE_CHAR_LANGUAGE_CJK:
/*N*/ 		case EE_CHAR_LANGUAGE_CTL:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribLanguage( (const SvxLanguageItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_COLOR:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribColor( (const SvxColorItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_FONTINFO:
/*N*/ 		case EE_CHAR_FONTINFO_CJK:
/*N*/ 		case EE_CHAR_FONTINFO_CTL:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribFont( (const SvxFontItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_FONTHEIGHT:
/*N*/ 		case EE_CHAR_FONTHEIGHT_CJK:
/*N*/ 		case EE_CHAR_FONTHEIGHT_CTL:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribFontHeight( (const SvxFontHeightItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_FONTWIDTH:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribFontWidth( (const SvxCharScaleWidthItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_WEIGHT:
/*N*/ 		case EE_CHAR_WEIGHT_CJK:
/*N*/ 		case EE_CHAR_WEIGHT_CTL:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribWeight( (const SvxWeightItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_UNDERLINE:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribUnderline( (const SvxUnderlineItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_EMPHASISMARK:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribEmphasisMark( (const SvxEmphasisMarkItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_RELIEF:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribRelief( (const SvxCharReliefItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_STRIKEOUT:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribStrikeout( (const SvxCrossedOutItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_ITALIC:
/*N*/ 		case EE_CHAR_ITALIC_CJK:
/*N*/ 		case EE_CHAR_ITALIC_CTL:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribItalic( (const SvxPostureItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_OUTLINE:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribOutline( (const SvxContourItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_SHADOW:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribShadow( (const SvxShadowedItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_ESCAPEMENT:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribEscapement( (const SvxEscapementItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_PAIRKERNING:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribPairKerning( (const SvxAutoKernItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_KERNING:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribKerning( (const SvxKerningItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_WLM:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribWordLineMode( (const SvxWordLineModeItem&)rNew, nS, nE );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_CHAR_XMLATTRIBS:
/*N*/ 		{
/*?*/ 			pNew = new EditCharAttrib( rNew, nS, nE );	// Attrib is only for holding XML information...
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_FEATURE_TAB:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribTab( (const SfxVoidItem&)rNew, nS );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_FEATURE_LINEBR:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribLineBreak( (const SfxVoidItem&)rNew, nS );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case EE_FEATURE_FIELD:
/*N*/ 		{
/*N*/ 			pNew = new EditCharAttribField( (const SvxFieldItem&)rNew, nS );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		default:
/*N*/ 		{
/*N*/ 			DBG_ERROR( "Ungueltiges Attribut!" );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pNew;
/*N*/ }

// -------------------------------------------------------------------------
// class EditLine
// -------------------------------------------------------------------------

/*N*/ EditLine::EditLine()
/*N*/ {
/*N*/ 	DBG_CTOR( EE_EditLine, 0 );
/*N*/ 
/*N*/ 	nStart = nEnd = 0;
/*N*/ 	nStartPortion = 0;				// damit in ungueltiger Zeile ohne Portions von einer gueltigen Zeile mit der Portion Nr0 unterscieden werden kann.
/*N*/ 	nEndPortion = 0;
/*N*/ 	nHeight = 0;
/*N*/ 	nStartPosX = 0;
/*N*/ 	nTxtHeight = 0;
/*N*/     nTxtWidth = 0;
/*N*/ 	nCrsrHeight = 0;
/*N*/ 	nMaxAscent = 0;
/*N*/ 	bHangingPunctuation = FALSE;
/*N*/ 	bInvalid = TRUE;
/*N*/ }

/*N*/ EditLine::EditLine( const EditLine& r )
/*N*/ {
/*N*/ 	DBG_CTOR( EE_EditLine, 0 );
/*N*/ 
/*N*/ 	nEnd = r.nEnd;
/*N*/ 	nStart = r.nStart;
/*N*/ 	nStartPortion = r.nStartPortion;
/*N*/ 	nEndPortion = r.nEndPortion;
/*N*/ 	bHangingPunctuation = r.bHangingPunctuation;
/*N*/ 
/*N*/ 	nHeight = 0;
/*N*/ 	nStartPosX = 0;
/*N*/ 	nTxtHeight = 0;
/*N*/     nTxtWidth = 0;
/*N*/ 	nCrsrHeight = 0;
/*N*/ 	nMaxAscent = 0;
/*N*/ 	bInvalid = TRUE;
/*N*/ }

/*N*/ EditLine::~EditLine()
/*N*/ {
/*N*/ 	DBG_DTOR( EE_EditLine, 0 );
/*N*/ }


/*N*/ BOOL operator == ( const EditLine& r1,  const EditLine& r2  )
/*N*/ {
/*N*/ 	if ( r1.nStart != r2.nStart )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	if ( r1.nEnd != r2.nEnd )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	if ( r1.nStartPortion != r2.nStartPortion )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	if ( r1.nEndPortion != r2.nEndPortion )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ EditLine& EditLine::operator = ( const EditLine& r )
/*N*/ {
/*N*/ 	nEnd = r.nEnd;
/*N*/ 	nStart = r.nStart;
/*N*/ 	nEndPortion = r.nEndPortion;
/*N*/ 	nStartPortion = r.nStartPortion;
/*N*/ 	return *this;
/*N*/ }



/*N*/ Size EditLine::CalcTextSize( ParaPortion& rParaPortion )
/*N*/ {
/*N*/ 	Size aSz;
/*N*/ 	Size aTmpSz;
/*N*/ 	TextPortion* pPortion;
/*N*/ 
/*N*/ 	USHORT nIndex = GetStart();
/*N*/ 
/*N*/ 	DBG_ASSERT( rParaPortion.GetTextPortions().Count(), "GetTextSize vor CreatePortions !" );
/*N*/ 
/*N*/ 	for ( USHORT n = nStartPortion; n <= nEndPortion; n++ )
/*N*/ 	{
/*N*/ 		pPortion = rParaPortion.GetTextPortions().GetObject(n);
/*N*/ 		switch ( pPortion->GetKind() )
/*N*/ 		{
/*N*/ 			case PORTIONKIND_TEXT:
/*N*/ 			case PORTIONKIND_FIELD:
/*N*/ 			case PORTIONKIND_HYPHENATOR:
/*N*/ 			{
/*N*/ 				aTmpSz = pPortion->GetSize();
/*N*/ 				aSz.Width() += aTmpSz.Width();
/*N*/ 				if ( aSz.Height() < aTmpSz.Height() )
/*N*/ 					aSz.Height() = aTmpSz.Height();
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case PORTIONKIND_TAB:
/*N*/ //			case PORTIONKIND_EXTRASPACE:
/*N*/ 			{
/*N*/ 				aSz.Width() += pPortion->GetSize().Width();
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		nIndex += pPortion->GetLen();
/*N*/ 	}
/*N*/ 
/*N*/ 	SetHeight( (USHORT)aSz.Height() );
/*N*/ 	return aSz;
/*N*/ }

// -------------------------------------------------------------------------
// class EditLineList
// -------------------------------------------------------------------------
/*N*/ EditLineList::EditLineList()
/*N*/ {
/*N*/ }

/*N*/ EditLineList::~EditLineList()
/*N*/ {
/*N*/ 	Reset();
/*N*/ }

/*N*/ void EditLineList::Reset()
/*N*/ {
/*N*/ 	for ( USHORT nLine = 0; nLine < Count(); nLine++ )
/*N*/ 		delete GetObject(nLine);
/*N*/ 	Remove( 0, Count() );
/*N*/ }

/*N*/ void EditLineList::DeleteFromLine( USHORT nDelFrom )
/*N*/ {
/*N*/ 	DBG_ASSERT( nDelFrom <= (Count() - 1), "DeleteFromLine: Out of range" );
/*N*/ 	for ( USHORT nL = nDelFrom; nL < Count(); nL++ )
/*N*/ 		delete GetObject(nL);
/*N*/ 	Remove( nDelFrom, Count()-nDelFrom );
/*N*/ }


// -------------------------------------------------------------------------
// class EditSelection
// -------------------------------------------------------------------------

/*N*/ EditSelection::EditSelection()
/*N*/ {
/*N*/ }

/*N*/ EditSelection::EditSelection( const EditPaM& rStartAndAnd )
/*N*/ {
/*N*/ 	// koennte noch optimiert werden!
/*N*/ 	// nicht erst Def-CTOR vom PaM rufen!
/*N*/ 	aStartPaM = rStartAndAnd;
/*N*/ 	aEndPaM = rStartAndAnd;
/*N*/ }

/*N*/ EditSelection::EditSelection( const EditPaM& rStart, const EditPaM& rEnd )
/*N*/ {
/*N*/ 	// koennte noch optimiert werden!
/*N*/ 	aStartPaM = rStart;
/*N*/ 	aEndPaM = rEnd;
/*N*/ }

/*N*/ EditSelection& EditSelection::operator = ( const EditPaM& rPaM )
/*N*/ {
/*N*/ 	aStartPaM = rPaM;
/*N*/ 	aEndPaM = rPaM;
/*N*/ 	return *this;
/*N*/ }

/*N*/ BOOL EditSelection::Adjust( const ContentList& rNodes )
/*N*/ {
/*N*/ 	DBG_ASSERT( aStartPaM.GetIndex() <= aStartPaM.GetNode()->Len(), "Index im Wald in Adjust(1)" );
/*N*/ 	DBG_ASSERT( aEndPaM.GetIndex() <= aEndPaM.GetNode()->Len(), "Index im Wald in Adjust(2)" );
/*N*/ 
/*N*/ 	ContentNode* pStartNode = aStartPaM.GetNode();
/*N*/ 	ContentNode* pEndNode = aEndPaM.GetNode();
/*N*/ 
/*N*/ 	USHORT nStartNode = rNodes.GetPos( pStartNode );
/*N*/ 	USHORT nEndNode = rNodes.GetPos( pEndNode );
/*N*/ 
/*N*/ 	DBG_ASSERT( nStartNode != USHRT_MAX, "Node im Wald in Adjust(1)" );
/*N*/ 	DBG_ASSERT( nEndNode != USHRT_MAX, "Node im Wald in Adjust(2)" );
/*N*/ 
/*N*/ 	BOOL bSwap = FALSE;
/*N*/ 	if ( nStartNode > nEndNode )
/*N*/ 		bSwap = TRUE;
/*N*/ 	else if ( ( nStartNode == nEndNode ) && ( aStartPaM.GetIndex() > aEndPaM.GetIndex() ) )
/*N*/ 		bSwap = TRUE;
/*N*/ 
/*N*/ 	if ( bSwap )
/*N*/ 	{
/*?*/ 		EditPaM aTmpPaM( aStartPaM );
/*?*/ 		aStartPaM = aEndPaM;
/*?*/ 		aEndPaM = aTmpPaM;
/*N*/ 	}
/*N*/ 
/*N*/ 	return bSwap;
/*N*/ }


// -------------------------------------------------------------------------
// class EditPaM
// -------------------------------------------------------------------------
/*N*/ BOOL operator == ( const EditPaM& r1,  const EditPaM& r2  )
/*N*/ {
/*N*/ 	if ( r1.GetNode() != r2.GetNode() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	if ( r1.GetIndex() != r2.GetIndex() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ EditPaM& EditPaM::operator = ( const EditPaM& rPaM )
/*N*/ {
/*N*/ 	nIndex = rPaM.nIndex;
/*N*/ 	pNode = rPaM.pNode;
/*N*/ 	return *this;
/*N*/ }

/*N*/ BOOL operator != ( const EditPaM& r1,  const EditPaM& r2  )
/*N*/ {
/*N*/ 	return !( r1 == r2 );
/*N*/ }


// -------------------------------------------------------------------------
// class ContentNode
// -------------------------------------------------------------------------
/*N*/ ContentNode::ContentNode( SfxItemPool& rPool ) : aContentAttribs( rPool )
/*N*/ {
/*N*/ 	DBG_CTOR( EE_ContentNode, 0 );
/*N*/ }

/*N*/ ContentNode::ContentNode( const XubString& rStr, const ContentAttribs& rContentAttribs ) :
/*N*/ 	XubString( rStr ), aContentAttribs( rContentAttribs )
/*N*/ {
/*N*/ 	DBG_CTOR( EE_ContentNode, 0 );
/*N*/ }

/*N*/ ContentNode::~ContentNode()
/*N*/ {
/*N*/ 	DBG_DTOR( EE_ContentNode, 0 );
/*N*/ }

/*N*/ void ContentNode::ExpandAttribs( USHORT nIndex, USHORT nNew, SfxItemPool& rItemPool )
/*N*/ {
/*N*/ 	if ( !nNew )
/*N*/ 		return;
/*N*/ 
/*N*/ 	// Da Features anders behandelt werden als normale Zeichenattribute,
/*N*/ 	// kann sich hier auch die Sortierung der Start-Liste aendern!
/*N*/ 	// In jedem if..., in dem weiter (n) Moeglichkeiten aufgrund von
/*N*/ 	// bFeature oder Spezialfall existieren,
/*N*/ 	// muessen (n-1) Moeglichkeiten mit bResort versehen werden.
/*N*/ 	// Die wahrscheinlichste Moeglichkeit erhaelt kein bResort,
/*N*/ 	// so dass nicht neu sortiert wird, wenn sich alle Attribute
/*N*/ 	// gleich verhalten.
/*N*/ 	BOOL bResort = FALSE;
/*N*/ 
/*N*/ 	USHORT nAttr = 0;
/*N*/ 	EditCharAttrib* pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
/*N*/ 	while ( pAttrib )
/*N*/ 	{
/*N*/ 		if ( pAttrib->GetEnd() >= nIndex )
/*N*/ 		{
/*N*/ 			// Alle Attribute hinter der Einfuegeposition verschieben...
/*N*/ 			if ( pAttrib->GetStart() > nIndex )
/*N*/ 			{
/*N*/ 				pAttrib->MoveForward( nNew );
/*N*/ 			}
/*N*/ 			// 0: Leeres Attribut expandieren, wenn an Einfuegestelle
/*N*/ 			else if ( pAttrib->IsEmpty() )
/*N*/ 			{
/*?*/ 				// Index nicht pruefen, leeres durfte nur dort liegen.
/*?*/ 				// Wenn spaeter doch Ueberpruefung:
/*?*/ 				//   Spezialfall: Start == 0; AbsLen == 1, nNew = 1 => Expand, weil durch Absatzumbruch!
/*?*/ 				// Start <= nIndex, End >= nIndex => Start=End=nIndex!
/*?*/ //				if ( pAttrib->GetStart() == nIndex )
/*?*/ 					pAttrib->Expand( nNew );
/*N*/ 			}
/*N*/ 			// 1: Attribut startet davor, geht bis Index...
/*N*/ 			else if ( pAttrib->GetEnd() == nIndex ) // Start muss davor liegen
/*N*/ 			{
/*N*/ 				// Nur expandieren, wenn kein Feature,
/*N*/ 				// und wenn nicht in ExcludeListe!
/*N*/ 				// Sonst geht z.B. ein UL bis zum neuen ULDB, beide expandieren
/*N*/ //				if ( !pAttrib->IsFeature() && !rExclList.FindAttrib( pAttrib->Which() ) )
/*N*/ 				if ( !pAttrib->IsFeature() && !aCharAttribList.FindEmptyAttrib( pAttrib->Which(), nIndex ) )
/*N*/ 				{
/*?*/ 					if ( !pAttrib->IsEdge() )
/*?*/ 						pAttrib->Expand( nNew );
/*N*/ 				}
/*N*/ 				else
/*N*/ 					bResort = TRUE;
/*N*/ 			}
/*N*/ 			// 2: Attribut startet davor, geht hinter Index...
/*N*/ 			else if ( ( pAttrib->GetStart() < nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
/*N*/ 			{
/*?*/ 				DBG_ASSERT( !pAttrib->IsFeature(), "Grosses Feature?!" );
/*?*/ 				pAttrib->Expand( nNew );
/*N*/ 			}
/*N*/ 			// 3: Attribut startet auf Index...
/*N*/ 			else if ( pAttrib->GetStart() == nIndex )
/*N*/ 			{
/*?*/ 				if ( pAttrib->IsFeature() )
/*?*/ 				{
/*?*/ 					pAttrib->MoveForward( nNew );
/*?*/ 					bResort = TRUE;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					if ( nIndex == 0 )
/*?*/ 					{
/*?*/ 						pAttrib->Expand( nNew );
/*?*/ 						bResort = TRUE;
/*?*/ 					}
/*?*/ 					else
/*?*/ 						pAttrib->MoveForward( nNew );
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( pAttrib->IsEdge() )
/*?*/ 			pAttrib->SetEdge( FALSE );
/*N*/ 
/*N*/ 		DBG_ASSERT( !pAttrib->IsFeature() || ( pAttrib->GetLen() == 1 ), "Expand: FeaturesLen != 1" );
/*N*/ 
/*N*/ 		DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Expand: Attribut verdreht!" );
/*N*/ 		DBG_ASSERT( ( pAttrib->GetEnd() <= Len() ), "Expand: Attrib groesser als Absatz!" );
/*N*/ 		if ( pAttrib->IsEmpty() )
/*N*/ 		{
/*?*/ 			DBG_ERROR( "Leeres Attribut nach ExpandAttribs?" );
/*?*/ 			bResort = TRUE;
/*?*/ 			aCharAttribList.GetAttribs().Remove( nAttr );
/*?*/ 			rItemPool.Remove( *pAttrib->GetItem() );
/*?*/ 			delete pAttrib;
/*?*/ 			nAttr--;
/*N*/ 		}
/*N*/ 		nAttr++;
/*N*/ 		pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bResort )
/*N*/ 		aCharAttribList.ResortAttribs();
/*N*/ 
/*N*/ #ifdef EDITDEBUG
/*N*/ 	DBG_ASSERT( CheckOrderedList( aCharAttribList.GetAttribs(), TRUE ), "Expand: Start-Liste verdreht" );
/*N*/ #endif
/*N*/ }

/*N*/ void ContentNode::CollapsAttribs( USHORT nIndex, USHORT nDeleted, SfxItemPool& rItemPool )
/*N*/ {
/*N*/ 	if ( !nDeleted )
/*N*/ 		return;
/*N*/ 
/*N*/ 	// Da Features anders behandelt werden als normale Zeichenattribute,
/*N*/ 	// kann sich hier auch die Sortierung der Start-Liste aendern!
/*N*/ 	BOOL bResort = FALSE;
/*N*/ 	BOOL bDelAttr = FALSE;
/*N*/ 	USHORT nEndChanges = nIndex+nDeleted;
/*N*/ 
/*N*/ 	USHORT nAttr = 0;
/*N*/ 	EditCharAttrib* pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
/*N*/ 	while ( pAttrib )
/*N*/ 	{
/*N*/ 		bDelAttr = FALSE;
/*N*/ 		if ( pAttrib->GetEnd() >= nIndex )
/*N*/ 		{
/*N*/ 			// Alles Attribute hinter der Einfuegeposition verschieben...
/*N*/ 			if ( pAttrib->GetStart() >= nEndChanges )
/*N*/ 			{
/*N*/ 				pAttrib->MoveBackward( nDeleted );
/*N*/ 			}
/*N*/ 			// 1. Innenliegende Attribute loeschen...
/*N*/ 			else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() <= nEndChanges ) )
/*N*/ 			{
/*N*/ 				// Spezialfall: Attrubt deckt genau den Bereich ab
/*N*/ 				// => als leeres Attribut behalten.
/*N*/ 				if ( !pAttrib->IsFeature() && ( pAttrib->GetStart() == nIndex ) && ( pAttrib->GetEnd() == nEndChanges ) )
/*?*/ 					pAttrib->GetEnd() = nIndex;	// leer
/*N*/ 				else
/*N*/ 					bDelAttr = TRUE;
/*N*/ 			}
/*N*/ 			// 2. Attribut beginnt davor, endet drinnen oder dahinter...
/*N*/ 			else if ( ( pAttrib->GetStart() <= nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
/*N*/ 			{
/*?*/ 				DBG_ASSERT( !pAttrib->IsFeature(), "Collapsing Feature!" );
/*?*/ 				if ( pAttrib->GetEnd() <= nEndChanges )	// endet drinnen
/*?*/ 					pAttrib->GetEnd() = nIndex;
/*?*/ 				else
/*?*/ 					pAttrib->Collaps( nDeleted );		// endet dahinter
/*N*/ 			}
/*N*/ 			// 3. Attribut beginnt drinnen, endet dahinter...
/*N*/ 			else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() > nEndChanges ) )
/*N*/ 			{
/*?*/ 				// Features duerfen nicht expandieren!
/*?*/ 				if ( pAttrib->IsFeature() )
/*?*/ 				{
/*?*/ 					pAttrib->MoveBackward( nDeleted );
/*?*/ 					bResort = TRUE;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					pAttrib->GetStart() = nEndChanges;
/*?*/ 					pAttrib->MoveBackward( nDeleted );
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		DBG_ASSERT( !pAttrib->IsFeature() || ( pAttrib->GetLen() == 1 ), "Expand: FeaturesLen != 1" );
/*N*/ 
/*N*/ 		DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Collaps: Attribut verdreht!" );
/*N*/ 		DBG_ASSERT( ( pAttrib->GetEnd() <= Len()) || bDelAttr, "Collaps: Attrib groesser als Absatz!" );
/*N*/ 		if ( bDelAttr /* || pAttrib->IsEmpty() */ )
/*N*/ 		{
/*N*/ 			bResort = TRUE;
/*N*/ 			aCharAttribList.GetAttribs().Remove( nAttr );
/*N*/ 			rItemPool.Remove( *pAttrib->GetItem() );
/*N*/ 			delete pAttrib;
/*N*/ 			nAttr--;
/*N*/ 		}
/*N*/ 		else if ( pAttrib->IsEmpty() )
/*?*/ 			aCharAttribList.HasEmptyAttribs() = TRUE;
/*N*/ 
/*N*/ 		nAttr++;
/*N*/ 		pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bResort )
/*N*/ 		aCharAttribList.ResortAttribs();
/*N*/ 
/*N*/ #ifdef EDITDEBUG
/*N*/ 	DBG_ASSERT( CheckOrderedList( aCharAttribList.GetAttribs(), TRUE ), "Collaps: Start-Liste verdreht" );
/*N*/ #endif
/*N*/ }

/*N*/ void ContentNode::CopyAndCutAttribs( ContentNode* pPrevNode, SfxItemPool& rPool, BOOL bKeepEndingAttribs )
/*N*/ {
/*N*/ 	DBG_ASSERT( pPrevNode, "kopieren von Attributen auf einen NULL-Pointer ?" );
/*N*/ 
/*N*/ 	xub_StrLen nCut = pPrevNode->Len();
/*N*/ 
/*N*/ 	USHORT nAttr = 0;
/*N*/ 	EditCharAttrib* pAttrib = GetAttrib( pPrevNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 	while ( pAttrib )
/*N*/ 	{
/*N*/ 		if ( pAttrib->GetEnd() < nCut )
/*N*/ 		{
/*N*/ 			// bleiben unveraendert....
/*N*/ 			;
/*N*/ 		}
/*N*/ 		else if ( pAttrib->GetEnd() == nCut )
/*N*/ 		{
/*N*/ 			// muessen als leeres Attribut kopiert werden.
/*N*/ 			if ( bKeepEndingAttribs && !pAttrib->IsFeature() && !aCharAttribList.FindAttrib( pAttrib->GetItem()->Which(), 0 ) )
/*N*/ 			{
/*?*/ 				EditCharAttrib* pNewAttrib = MakeCharAttrib( rPool, *(pAttrib->GetItem()), 0, 0 );
/*?*/ 				DBG_ASSERT( pNewAttrib, "MakeCharAttrib fehlgeschlagen!" );
/*?*/ 				aCharAttribList.InsertAttrib( pNewAttrib );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if ( pAttrib->IsInside( nCut ) || ( !nCut && !pAttrib->GetStart() && !pAttrib->IsFeature() ) )
/*N*/ 		{
/*?*/ 			// Wenn ganz vorne gecuttet wird, muss das Attribut erhalten bleiben!
/*?*/ 			// muessen kopiert und geaendert werden
/*?*/ 			EditCharAttrib* pNewAttrib = MakeCharAttrib( rPool, *(pAttrib->GetItem()), 0, pAttrib->GetEnd()-nCut );
/*?*/ 			DBG_ASSERT( pNewAttrib, "MakeCharAttrib fehlgeschlagen!" );
/*?*/ 			aCharAttribList.InsertAttrib( pNewAttrib );
/*?*/ 			// stutzen:
/*?*/ 			pAttrib->GetEnd() = nCut;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			// alle dahinter verschieben in den neuen Node (this)
/*?*/ //			pPrevNode->GetCharAttribs().RemoveAttrib( pAttrib );
/*?*/ 			pPrevNode->GetCharAttribs().GetAttribs().Remove( nAttr );
/*?*/ 			aCharAttribList.InsertAttrib( pAttrib );
/*?*/ 			DBG_ASSERT( pAttrib->GetStart() >= nCut, "Start < nCut!" );
/*?*/ 			DBG_ASSERT( pAttrib->GetEnd() >= nCut, "End < nCut!" );
/*?*/ 			pAttrib->GetStart() -= nCut;
/*?*/ 			pAttrib->GetEnd() -= nCut;
/*?*/ 			nAttr--;
/*N*/ 		}
/*N*/ 		nAttr++;
/*N*/ 		pAttrib = GetAttrib( pPrevNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 	}
/*N*/ }

/*N*/ void ContentNode::AppendAttribs( ContentNode* pNextNode )
/*N*/ {
/*N*/ 	DBG_ASSERT( pNextNode, "kopieren von Attributen von einen NULL-Pointer ?" );
/*N*/ 
/*N*/ 	USHORT nNewStart = Len();
/*N*/ 
/*N*/ #ifdef EDITDEBUG
/*N*/ 	DBG_ASSERT( aCharAttribList.DbgCheckAttribs(), "Attribute VOR AppendAttribs kaputt" );
/*N*/ #endif
/*N*/ 
/*N*/ 	USHORT nAttr = 0;
/*N*/ 	EditCharAttrib* pAttrib = GetAttrib( pNextNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 	while ( pAttrib )
/*N*/ 	{
/*?*/ 		// alle Attribute verschieben in den aktuellen Node (this)
/*?*/ 		BOOL bMelted = FALSE;
/*?*/ 		if ( ( pAttrib->GetStart() == 0 ) && ( !pAttrib->IsFeature() ) )
/*?*/ 		{
/*?*/ 			// Evtl koennen Attribute zusammengefasst werden:
/*?*/ 			USHORT nTmpAttr = 0;
/*?*/ 			EditCharAttrib* pTmpAttrib = GetAttrib( aCharAttribList.GetAttribs(), nTmpAttr );
/*?*/ 			while ( !bMelted && pTmpAttrib )
/*?*/ 			{
/*?*/ 				if ( pTmpAttrib->GetEnd() == nNewStart )
/*?*/ 				{
/*?*/ 					if ( ( pTmpAttrib->Which() == pAttrib->Which() ) &&
/*?*/ 						 ( *(pTmpAttrib->GetItem()) == *(pAttrib->GetItem() ) ) )
/*?*/ 					{
/*?*/ 						pTmpAttrib->GetEnd() += pAttrib->GetLen();
/*?*/ 						pNextNode->GetCharAttribs().GetAttribs().Remove( nAttr );
/*?*/ 						// Vom Pool abmelden ?!
/*?*/ 						delete pAttrib;
/*?*/ 						bMelted = TRUE;
/*?*/ 					}
/*?*/ 				}
/*?*/ 				++nTmpAttr;
/*?*/ 				pTmpAttrib = GetAttrib( aCharAttribList.GetAttribs(), nTmpAttr );
/*?*/ 			}
/*?*/ 		}
/*?*/ 
/*?*/ 		if ( !bMelted )
/*?*/ 		{
/*?*/ 			pAttrib->GetStart() += nNewStart;
/*?*/ 			pAttrib->GetEnd() += nNewStart;
/*?*/ 			aCharAttribList.InsertAttrib( pAttrib );
/*?*/ 			++nAttr;
/*?*/ 		}
/*?*/ 		pAttrib = GetAttrib( pNextNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 	}
/*N*/ 	// Fuer die Attribute, die nur ruebergewandert sind:
/*N*/ 	pNextNode->GetCharAttribs().Clear();
/*N*/ 
/*N*/ #ifdef EDITDEBUG
/*N*/ 	DBG_ASSERT( aCharAttribList.DbgCheckAttribs(), "Attribute NACH AppendAttribs kaputt" );
/*N*/ #endif
/*N*/ }

/*N*/ void ContentNode::CreateDefFont()
/*N*/ {
/*N*/ 	// Erst alle Informationen aus dem Style verwenden...
/*N*/ 	SfxStyleSheet* pS = aContentAttribs.GetStyleSheet();
/*N*/ 	if ( pS )
/*N*/ 		CreateFont( GetCharAttribs().GetDefFont(), pS->GetItemSet() );
/*N*/ 	
/*N*/ 	// ... dann die harte Absatzformatierung rueberbuegeln...
/*N*/ 	CreateFont( GetCharAttribs().GetDefFont(),
/*N*/ 		GetContentAttribs().GetItems(), ( pS ? FALSE : TRUE ) );
/*N*/ }

/*N*/ void ContentNode::SetStyleSheet( SfxStyleSheet* pS, BOOL bRecalcFont )
/*N*/ {
/*N*/ 	aContentAttribs.SetStyleSheet( pS );
/*N*/ 	if ( bRecalcFont )
/*N*/ 		CreateDefFont();
/*N*/ }


// -------------------------------------------------------------------------
// class ContentAttribs
// -------------------------------------------------------------------------
/*N*/ ContentAttribs::ContentAttribs( SfxItemPool& rPool ) :
/*N*/ 					aAttribSet( rPool, EE_PARA_START, EE_CHAR_END )
/*N*/ {
/*N*/ 	pStyle = 0;
/*N*/ }

/*N*/ ContentAttribs::ContentAttribs( const ContentAttribs& rRef ) :
/*N*/ 					aAttribSet( rRef.aAttribSet )
/*N*/ {
/*N*/ 	pStyle = rRef.pStyle;
/*N*/ }

/*N*/ ContentAttribs::~ContentAttribs()
/*N*/ {
/*N*/ }

/*N*/ SvxTabStop ContentAttribs::FindTabStop( long nCurPos, USHORT nDefTab )
/*N*/ {
/*N*/ 	const SvxTabStopItem& rTabs = (const SvxTabStopItem&) GetItem( EE_PARA_TABS );
/*N*/ 	for ( USHORT i = 0; i < rTabs.Count(); i++ )
/*N*/ 	{
/*N*/ 		const SvxTabStop& rTab = rTabs[i];
/*N*/ 		if ( rTab.GetTabPos() > nCurPos  )
/*N*/ 			return rTab;
/*N*/ 	}
/*N*/ 
/*N*/ 	// DefTab ermitteln...
/*N*/ 	SvxTabStop aTabStop;
/*N*/ 	long x = nCurPos / nDefTab + 1;
/*N*/ 	aTabStop.GetTabPos() = nDefTab * x;
/*N*/ 	return aTabStop;
/*N*/ }

/*N*/ void ContentAttribs::SetStyleSheet( SfxStyleSheet* pS )
/*N*/ {
/*N*/     BOOL bStyleChanged = ( pStyle != pS );
/*N*/ 	pStyle = pS;
/*N*/     // #104799# Only when other style sheet, not when current style sheet modified
/*N*/ 	if ( pStyle && bStyleChanged )
/*N*/ 	{
/*N*/ 		// Gezielt die Attribute aus der Absatzformatierung entfernen, die im Style
/*N*/ 		// spezifiziert sind, damit die Attribute des Styles wirken koennen.
/*N*/ 		const SfxItemSet& rStyleAttribs = pStyle->GetItemSet();
/*N*/ 		for ( USHORT nWhich = EE_PARA_START; nWhich <= EE_CHAR_END; nWhich++ )
/*N*/ 		{
/*N*/             // #99635# Don't change bullet on/off
/*N*/ 			if ( ( nWhich != EE_PARA_BULLETSTATE ) && ( rStyleAttribs.GetItemState( nWhich ) == SFX_ITEM_ON ) )
/*N*/ 				aAttribSet.ClearItem( nWhich );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ const SfxPoolItem& ContentAttribs::GetItem( USHORT nWhich )
/*N*/ {
/*N*/ 	// Harte Absatzattribute haben Vorrang!
/*N*/ 	SfxItemSet* pTakeFrom = &aAttribSet;
/*N*/ 	if ( pStyle && ( aAttribSet.GetItemState( nWhich, FALSE ) != SFX_ITEM_ON  ) )
/*N*/ 		pTakeFrom = &pStyle->GetItemSet();
/*N*/ 
/*N*/ 	return pTakeFrom->Get( nWhich );
/*N*/ }

/*N*/ BOOL ContentAttribs::HasItem( USHORT nWhich )
/*N*/ {
/*N*/ 	BOOL bHasItem = FALSE;
/*N*/ 	if ( aAttribSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_ON  )
/*N*/ 		bHasItem = TRUE;
/*N*/ 	else if ( pStyle && pStyle->GetItemSet().GetItemState( nWhich ) == SFX_ITEM_ON )
/*N*/ 		bHasItem = TRUE;
/*N*/ 
/*N*/ 	return bHasItem;
/*N*/ }



//	----------------------------------------------------------------------
//	class ItemList
//	----------------------------------------------------------------------

// -------------------------------------------------------------------------
// class EditDoc
// -------------------------------------------------------------------------
/*N*/ EditDoc::EditDoc( SfxItemPool* pPool )
/*N*/ {
/*N*/ 	if ( pPool ) 
/*N*/ 	{
/*N*/ 		pItemPool = pPool;
/*N*/ 		bOwnerOfPool = FALSE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		pItemPool = new EditEngineItemPool( FALSE );
/*?*/ 		bOwnerOfPool = TRUE;
/*N*/ 	}
/*N*/ 	
/*N*/ 	nDefTab = DEFTAB;
/*N*/ 	bIsVertical = FALSE;
/*N*/ 
/*N*/ 	// Don't create a empty node, Clear() will be called in EditEngine-CTOR
/*N*/ 
/*N*/ 	SetModified( FALSE );
/*N*/ };

/*N*/ EditDoc::~EditDoc()
/*N*/ {
/*N*/ 	ImplDestroyContents();
/*N*/ 	if ( bOwnerOfPool ) 
/*?*/ 		delete pItemPool;
/*N*/ }

/*N*/ void EditDoc::ImplDestroyContents()
/*N*/ {
/*N*/ 	for ( USHORT nNode = Count(); nNode; )
/*N*/ 		RemoveItemsFromPool( GetObject( --nNode ) );
/*N*/ 	DeleteAndDestroy( 0, Count() );
/*N*/ }

/*N*/ void EditDoc::RemoveItemsFromPool( ContentNode* pNode )
/*N*/ {
/*N*/ 	for ( USHORT nAttr = 0; nAttr < pNode->GetCharAttribs().Count(); nAttr++ )
/*N*/ 	{
/*N*/ 		EditCharAttrib* pAttr = pNode->GetCharAttribs().GetAttribs()[nAttr];
/*N*/ 		GetItemPool().Remove( *pAttr->GetItem() );
/*N*/ 	}
/*N*/ }

/*N*/ void CreateFont( SvxFont& rFont, const SfxItemSet& rSet, BOOL bSearchInParent, short nScriptType )
/*N*/ {
/*N*/ 	Font aPrevFont( rFont );
/*N*/ 	rFont.SetAlign( ALIGN_BASELINE );
/*N*/ 	rFont.SetTransparent( TRUE );
/*N*/ 
/*N*/     USHORT nWhich_FontInfo = GetScriptItemId( EE_CHAR_FONTINFO, nScriptType );
/*N*/     USHORT nWhich_Language = GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType );
/*N*/     USHORT nWhich_FontHeight = GetScriptItemId( EE_CHAR_FONTHEIGHT, nScriptType );
/*N*/     USHORT nWhich_Weight = GetScriptItemId( EE_CHAR_WEIGHT, nScriptType );
/*N*/     USHORT nWhich_Italic = GetScriptItemId( EE_CHAR_ITALIC, nScriptType );
/*N*/ 
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( nWhich_FontInfo ) == SFX_ITEM_ON ) )
/*N*/ 	{
/*N*/ 		const SvxFontItem& rFontItem = (const SvxFontItem&)rSet.Get( nWhich_FontInfo );
/*N*/ 		rFont.SetName( rFontItem.GetFamilyName() );
/*N*/ 		rFont.SetFamily( rFontItem.GetFamily() );
/*N*/ 		rFont.SetPitch( rFontItem.GetPitch() );
/*N*/ 		rFont.SetCharSet( rFontItem.GetCharSet() );
/*N*/ 	}
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( nWhich_Language ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetLanguage( ((const SvxLanguageItem&)rSet.Get( nWhich_Language )).GetLanguage() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_COLOR ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetColor( ((const SvxColorItem&)rSet.Get( EE_CHAR_COLOR )).GetValue() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( nWhich_FontHeight ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetSize( Size( rFont.GetSize().Width(), ((const SvxFontHeightItem&)rSet.Get( nWhich_FontHeight ) ).GetHeight() ) );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( nWhich_Weight ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetWeight( ((const SvxWeightItem&)rSet.Get( nWhich_Weight )).GetWeight() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_UNDERLINE ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetUnderline( ((const SvxUnderlineItem&)rSet.Get( EE_CHAR_UNDERLINE )).GetUnderline() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_STRIKEOUT ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetStrikeout( ((const SvxCrossedOutItem&)rSet.Get( EE_CHAR_STRIKEOUT )).GetStrikeout() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( nWhich_Italic ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetItalic( ((const SvxPostureItem&)rSet.Get( nWhich_Italic )).GetPosture() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_OUTLINE ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetOutline( ((const SvxContourItem&)rSet.Get( EE_CHAR_OUTLINE )).GetValue() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_SHADOW ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetShadow( ((const SvxShadowedItem&)rSet.Get( EE_CHAR_SHADOW )).GetValue() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_ESCAPEMENT ) == SFX_ITEM_ON ) )
/*N*/ 	{
/*N*/         const SvxEscapementItem& rEsc = (const SvxEscapementItem&) rSet.Get( EE_CHAR_ESCAPEMENT );
/*N*/ 
/*N*/         USHORT nProp = rEsc.GetProp();
/*N*/ 		rFont.SetPropr( (BYTE)nProp );
/*N*/ 
/*N*/         short nEsc = rEsc.GetEsc();
/*N*/ 	    if ( nEsc == DFLT_ESC_AUTO_SUPER )
/*N*/ 		    nEsc = 100 - nProp;
/*N*/ 	    else if ( nEsc == DFLT_ESC_AUTO_SUB )
/*N*/ 		    nEsc = -( 100 - nProp );
/*N*/ 	    rFont.SetEscapement( nEsc );
/*N*/ 	}
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_PAIRKERNING ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetKerning( ((const SvxAutoKernItem&)rSet.Get( EE_CHAR_PAIRKERNING )).GetValue() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_KERNING ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetFixKerning( ((const SvxKerningItem&)rSet.Get( EE_CHAR_KERNING )).GetValue() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_WLM ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetWordLineMode( ((const SvxWordLineModeItem&)rSet.Get( EE_CHAR_WLM )).GetValue() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_EMPHASISMARK ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetEmphasisMark( ((const SvxEmphasisMarkItem&)rSet.Get( EE_CHAR_EMPHASISMARK )).GetValue() );
/*N*/ 	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_RELIEF ) == SFX_ITEM_ON ) )
/*N*/ 		rFont.SetRelief( (FontRelief)((const SvxCharReliefItem&)rSet.Get( EE_CHAR_RELIEF )).GetValue() );
/*N*/ 
/*N*/ 	// Ob ich jetzt den ganzen Font vergleiche, oder vor jeder Aenderung
/*N*/ 	// pruefe, ob der Wert sich aendert, bleibt sich relativ gleich.
/*N*/ 	// So ggf ein MakeUniqFont im Font mehr, dafuer bei Aenderung schnellerer
/*N*/ 	// Abbruch der Abfrage, oder ich musste noch jedesmal ein bChanged pflegen.
/*N*/ 	if ( rFont == aPrevFont  )
/*N*/ 		rFont = aPrevFont;	// => Gleicher ImpPointer fuer IsSameInstance
/*N*/ }

/*N*/ void EditDoc::CreateDefFont( BOOL bUseStyles )
/*N*/ {
/*N*/ 	SfxItemSet aTmpSet( GetItemPool(), EE_PARA_START, EE_CHAR_END );
/*N*/ 	CreateFont( aDefFont, aTmpSet );
/*N*/ 	aDefFont.SetVertical( IsVertical() );
/*N*/ 	aDefFont.SetOrientation( IsVertical() ? 2700 : 0 );
/*N*/ 
/*N*/ 	for ( USHORT nNode = 0; nNode < Count(); nNode++ )
/*N*/ 	{
/*N*/ 		ContentNode* pNode = GetObject( nNode );
/*N*/ 		pNode->GetCharAttribs().GetDefFont() = aDefFont;
/*N*/ 		if ( bUseStyles )
/*?*/ 			pNode->CreateDefFont();
/*N*/ 	}
/*N*/ }

/*N*/ static const sal_Unicode aCR[] = { 0x0d, 0x00 };
/*N*/ static const sal_Unicode aLF[] = { 0x0a, 0x00 };
/*N*/ static const sal_Unicode aCRLF[] = { 0x0d, 0x0a, 0x00 };

/*N*/ XubString EditDoc::GetSepStr( LineEnd eEnd )
/*N*/ {
/*N*/ 	XubString aSep;
/*N*/ 	if ( eEnd == LINEEND_CR )
/*N*/ 		aSep = aCR;
/*N*/ 	else if ( eEnd == LINEEND_LF )
/*N*/ 		aSep = aLF;
/*N*/ 	else
/*N*/ 		aSep = aCRLF;
/*N*/ 	return aSep;
/*N*/ }

/*N*/ XubString EditDoc::GetText( LineEnd eEnd ) const
/*N*/ {
/*N*/ 	ULONG nLen = GetTextLen();
/*N*/ 	USHORT nNodes = Count();
/*N*/ 
/*N*/ 	String aSep = EditDoc::GetSepStr( eEnd );
/*N*/ 	USHORT nSepSize = aSep.Len();
/*N*/ 
/*N*/ 	if ( nSepSize )
/*N*/ 		nLen += nNodes * nSepSize;
/*N*/ 	if ( nLen > 0xFFFb / sizeof(xub_Unicode) )
/*N*/ 	{
/*?*/ 		DBG_ERROR( "Text zu gross fuer String" );
/*?*/ 		return XubString();
/*N*/ 	}
/*N*/ 	xub_Unicode* pStr = new xub_Unicode[nLen+1];
/*N*/ 	xub_Unicode* pCur = pStr;
/*N*/ 	USHORT nLastNode = nNodes-1;
/*N*/ 	for ( USHORT nNode = 0; nNode < nNodes; nNode++ )
/*N*/ 	{
/*N*/ 		XubString aTmp( GetParaAsString( GetObject(nNode) ) );
/*N*/ 		memcpy( pCur, aTmp.GetBuffer(), aTmp.Len()*sizeof(sal_Unicode) );
/*N*/ 		pCur += aTmp.Len();
/*N*/ 		if ( nSepSize && ( nNode != nLastNode ) )
/*N*/ 		{
/*N*/ 			memcpy( pCur, aSep.GetBuffer(), nSepSize*sizeof(sal_Unicode ) );
/*N*/ 			pCur += nSepSize;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	*pCur = '\0';
/*N*/ 	XubString aASCIIText( pStr );
/*N*/ 	delete[] pStr;
/*N*/ 	return aASCIIText;
/*N*/ }

/*N*/ XubString EditDoc::GetParaAsString( USHORT nNode ) const
/*N*/ {
/*N*/ 	return GetParaAsString( SaveGetObject( nNode ) );
/*N*/ }

/*N*/ XubString EditDoc::GetParaAsString( ContentNode* pNode, USHORT nStartPos, USHORT nEndPos, BOOL bResolveFields ) const
/*N*/ {
/*N*/ 	if ( nEndPos > pNode->Len() )
/*N*/ 		nEndPos = pNode->Len();
/*N*/ 
/*N*/ 	DBG_ASSERT( nStartPos <= nEndPos, "Start und Ende vertauscht?" );
/*N*/ 
/*N*/ 	USHORT nIndex = nStartPos;
/*N*/ 	XubString aStr;
/*N*/ 	EditCharAttrib* pNextFeature = pNode->GetCharAttribs().FindFeature( nIndex );
/*N*/ 	while ( nIndex < nEndPos )
/*N*/ 	{
/*N*/ 		USHORT nEnd = nEndPos;
/*N*/ 		if ( pNextFeature && ( pNextFeature->GetStart() < nEnd ) )
/*N*/ 			nEnd = pNextFeature->GetStart();
/*N*/ 		else
/*N*/ 			pNextFeature = 0;	// Feature interessiert unten nicht
/*N*/ 
/*N*/ 
/*N*/ 		DBG_ASSERT( nEnd >= nIndex, "Ende vorm Index?" );
/*N*/ 		aStr += XubString( *pNode, nIndex, nEnd-nIndex );
/*N*/ 
/*N*/ 		if ( pNextFeature )
/*N*/ 		{
/*N*/ 			switch ( pNextFeature->GetItem()->Which() )
/*N*/ 			{
/*N*/ 				case EE_FEATURE_TAB:	aStr += '\t';
/*N*/ 				break;
/*N*/ 				case EE_FEATURE_LINEBR:	aStr += '\x0A';
/*N*/ 				break;
/*N*/ 				case EE_FEATURE_FIELD:	if ( bResolveFields )
/*N*/ 											aStr += ((EditCharAttribField*)pNextFeature)->GetFieldValue();
/*N*/ 				break;
/*N*/ 				default:	DBG_ERROR( "Was fuer ein Feature ?" );
/*N*/ 			}
/*N*/ 			pNextFeature = pNode->GetCharAttribs().FindFeature( ++nEnd );
/*N*/ 		}
/*N*/ 		nIndex = nEnd;
/*N*/ 	}
/*N*/ 	return aStr;
/*N*/ }

/*N*/ ULONG EditDoc::GetTextLen() const
/*N*/ {
/*N*/ 	ULONG nLen = 0;
/*N*/ 	for ( USHORT nNode = 0; nNode < Count(); nNode++ )
/*N*/ 	{
/*N*/ 		ContentNode* pNode = GetObject( nNode );
/*N*/ 		nLen += pNode->Len();
/*N*/ 		// Felder k”nnen laenger sein als der Platzhalter im Node.
/*N*/ 		const CharAttribArray& rAttrs = pNode->GetCharAttribs().GetAttribs();
/*N*/ 		for ( USHORT nAttr = rAttrs.Count(); nAttr; )
/*N*/ 		{
/*N*/ 			EditCharAttrib* pAttr = rAttrs[--nAttr];
/*N*/ 			if ( pAttr->Which() == EE_FEATURE_FIELD )
/*N*/ 			{
/*N*/ 				USHORT nFieldLen = ((EditCharAttribField*)pAttr)->GetFieldValue().Len();
/*N*/ 				if ( !nFieldLen )
/*N*/ 					nLen--;
/*N*/ 				else
/*N*/ 					nLen += nFieldLen-1;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nLen;
/*N*/ }

/*N*/ EditPaM EditDoc::Clear()
/*N*/ {
/*N*/ 	ImplDestroyContents();
/*N*/ 
/*N*/ 	ContentNode* pNode = new ContentNode( GetItemPool() );
/*N*/ 	Insert( pNode, 0 );
/*N*/ 
/*N*/ 	CreateDefFont( FALSE );
/*N*/ 
/*N*/ 	SetModified( FALSE );
/*N*/ 
/*N*/ 	EditPaM aPaM( pNode, 0 );
/*N*/ 	return aPaM;
/*N*/ }
/*N*/ 
/*N*/ void EditDoc::SetModified( BOOL b )	
/*N*/ { 
/*N*/     bModified = b;
/*N*/     if ( bModified )
/*N*/     {
/*N*/         aModifyHdl.Call( NULL );
/*N*/     }
/*N*/ }

/*N*/ EditPaM EditDoc::RemoveText()
/*N*/ {
/*N*/ 	// Das alte ItemSetmerken, damit z.B. im Chart Font behalten bleibt
/*N*/ 	ContentNode* pPrevFirstNode = GetObject(0);
/*N*/ 	SfxStyleSheet* pPrevStyle = pPrevFirstNode->GetStyleSheet();
/*N*/ 	SfxItemSet aPrevSet( pPrevFirstNode->GetContentAttribs().GetItems() );
/*N*/ 	Font aPrevFont( pPrevFirstNode->GetCharAttribs().GetDefFont() );
/*N*/ 
/*N*/ 	ImplDestroyContents();
/*N*/ 
/*N*/ 	ContentNode* pNode = new ContentNode( GetItemPool() );
/*N*/ 	Insert( pNode, 0 );
/*N*/ 
/*N*/ 	pNode->SetStyleSheet( pPrevStyle, FALSE );
/*N*/ 	pNode->GetContentAttribs().GetItems().Set( aPrevSet );
/*N*/ 	pNode->GetCharAttribs().GetDefFont() = aPrevFont;
/*N*/ 
/*N*/ 	SetModified( TRUE );
/*N*/ 
/*N*/ 	EditPaM aPaM( pNode, 0 );
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ EditPaM EditDoc::InsertText( EditPaM aPaM, const XubString& rStr )
/*N*/ {
/*N*/ 	DBG_ASSERT( rStr.Search( 0x0A ) == STRING_NOTFOUND, "EditDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );
/*N*/ 	DBG_ASSERT( rStr.Search( 0x0D ) == STRING_NOTFOUND, "EditDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );
/*N*/ 	DBG_ASSERT( rStr.Search( '\t' ) == STRING_NOTFOUND, "EditDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );
/*N*/ 	DBG_ASSERT( aPaM.GetNode(), "Blinder PaM in EditDoc::InsertText1" );
/*N*/ 
/*N*/ 	aPaM.GetNode()->Insert( rStr, aPaM.GetIndex() );
/*N*/ 	aPaM.GetNode()->ExpandAttribs( aPaM.GetIndex(), rStr.Len(), GetItemPool() );
/*N*/ 	aPaM.GetIndex() += rStr.Len();
/*N*/ 
/*N*/ 	SetModified( TRUE );
/*N*/ 
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ EditPaM EditDoc::InsertParaBreak( EditPaM aPaM, BOOL bKeepEndingAttribs )
/*N*/ {
/*N*/ 	DBG_ASSERT( aPaM.GetNode(), "Blinder PaM in EditDoc::InsertParaBreak" );
/*N*/ 	ContentNode* pCurNode = aPaM.GetNode();
/*N*/ 	USHORT nPos = GetPos( pCurNode );
/*N*/ 	XubString aStr = aPaM.GetNode()->Copy( aPaM.GetIndex() );
/*N*/ 	aPaM.GetNode()->Erase( aPaM.GetIndex() );
/*N*/ 
/*N*/ 	// ContenNode-CTOR kopiert auch die Absatzattribute
/*N*/ 	ContentNode* pNode = new ContentNode( aStr, aPaM.GetNode()->GetContentAttribs());
/*N*/ 	// Den Default-Font kopieren
/*N*/ 	pNode->GetCharAttribs().GetDefFont() = aPaM.GetNode()->GetCharAttribs().GetDefFont();
/*N*/ 	SfxStyleSheet* pStyle = aPaM.GetNode()->GetStyleSheet();
/*N*/ 	if ( pStyle )
/*N*/ 	{
/*N*/ 		XubString aFollow( pStyle->GetFollow() );
/*N*/ 		if ( aFollow.Len() && ( aFollow != pStyle->GetName() ) )
/*N*/ 		{
/*?*/ 			SfxStyleSheetBase* pNext = pStyle->GetPool().Find( aFollow, pStyle->GetFamily() );
/*?*/ 			pNode->SetStyleSheet( (SfxStyleSheet*)pNext );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Zeichenattribute muessen ggf. kopiert bzw gestutzt werden:
/*N*/ 	pNode->CopyAndCutAttribs( aPaM.GetNode(), GetItemPool(), bKeepEndingAttribs );
/*N*/ 
/*N*/ 	Insert( pNode, nPos+1 );
/*N*/ 
/*N*/ 	SetModified( TRUE );
/*N*/ 
/*N*/ 	aPaM.SetNode( pNode );
/*N*/ 	aPaM.SetIndex( 0 );
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ EditPaM EditDoc::InsertFeature( EditPaM aPaM, const SfxPoolItem& rItem  )
/*N*/ {
/*N*/ 	DBG_ASSERT( aPaM.GetNode(), "Blinder PaM in EditDoc::InsertFeature" );
/*N*/ 
/*N*/ 	aPaM.GetNode()->Insert( CH_FEATURE, aPaM.GetIndex() );
/*N*/ 	aPaM.GetNode()->ExpandAttribs( aPaM.GetIndex(), 1, GetItemPool() );
/*N*/ 
/*N*/ 	// Fuer das Feature ein Feature-Attribut anlegen...
/*N*/ 	EditCharAttrib* pAttrib = MakeCharAttrib( GetItemPool(), rItem, aPaM.GetIndex(), aPaM.GetIndex()+1 );
/*N*/ 	DBG_ASSERT( pAttrib, "Warum kann ich kein Feature anlegen ?" );
/*N*/ 	aPaM.GetNode()->GetCharAttribs().InsertAttrib( pAttrib );
/*N*/ 
/*N*/ 	SetModified( TRUE );
/*N*/ 
/*N*/ 	aPaM.GetIndex()++;
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ EditPaM EditDoc::ConnectParagraphs( ContentNode* pLeft, ContentNode* pRight )
/*N*/ {
/*N*/ 	const EditPaM aPaM( pLeft, pLeft->Len() );
/*N*/ 
/*N*/ 	// Erst die Attribute, da sonst nLen nicht stimmt!
/*N*/ 	pLeft->AppendAttribs( pRight );
/*N*/ 	// Dann den Text...
/*N*/ 	*pLeft += *pRight;
/*N*/ 
/*N*/ 	// der rechte verschwindet.
/*N*/ 	RemoveItemsFromPool( pRight );
/*N*/ 	USHORT nRight = GetPos( pRight );
/*N*/ 	Remove( nRight );
/*N*/ 	delete pRight;
/*N*/ 
/*N*/ 	SetModified( TRUE );
/*N*/ 
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ EditPaM EditDoc::RemoveChars( EditPaM aPaM, USHORT nChars )
/*N*/ {
/*N*/ 	// Evtl. Features entfernen!
/*N*/ 	aPaM.GetNode()->Erase( aPaM.GetIndex(), nChars );
/*N*/ 	aPaM.GetNode()->CollapsAttribs( aPaM.GetIndex(), nChars, GetItemPool() );
/*N*/ 
/*N*/ 	SetModified( TRUE );
/*N*/ 
/*N*/ 	return aPaM;
/*N*/ }

/*N*/ void EditDoc::InsertAttribInSelection( ContentNode* pNode, USHORT nStart, USHORT nEnd, const SfxPoolItem& rPoolItem )
/*N*/ {
/*N*/ 	DBG_ASSERT( pNode, "Wohin mit dem Attribut?" );
/*N*/ 	DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribut zu gross!" );
/*N*/ 
/*N*/ 	// fuer Optimierung:
/*N*/ 	// dieses endet am Anfang der Selektion => kann erweitert werden
/*N*/ 	EditCharAttrib* pEndingAttrib = 0;
/*N*/ 	// dieses startet am Ende der Selektion => kann erweitert werden
/*N*/ 	EditCharAttrib* pStartingAttrib = 0;
/*N*/ 
/*N*/ 	DBG_ASSERT( nStart <= nEnd, "Kleiner Rechenfehler in InsertAttribInSelection" );
/*N*/ 
/*N*/ 	RemoveAttribs( pNode, nStart, nEnd, pStartingAttrib, pEndingAttrib, rPoolItem.Which() );
/*N*/ 
/*N*/ 	if ( pStartingAttrib && pEndingAttrib &&
/*N*/ 		 ( *(pStartingAttrib->GetItem()) == rPoolItem ) &&
/*N*/ 		 ( *(pEndingAttrib->GetItem()) == rPoolItem ) )
/*N*/ 	{
/*?*/ 		// wird ein groesses Attribut.
/*?*/ 		pEndingAttrib->GetEnd() = pStartingAttrib->GetEnd();
/*?*/ 		GetItemPool().Remove( *(pStartingAttrib->GetItem()) );
/*?*/ 		pNode->GetCharAttribs().GetAttribs().Remove( pNode->GetCharAttribs().GetAttribs().GetPos( pStartingAttrib ) );
/*?*/ 		delete pStartingAttrib;
/*N*/ 	}
/*N*/ 	else if ( pStartingAttrib && ( *(pStartingAttrib->GetItem()) == rPoolItem ) )
/*?*/ 		pStartingAttrib->GetStart() = nStart;
/*N*/ 	else if ( pEndingAttrib && ( *(pEndingAttrib->GetItem()) == rPoolItem ) )
/*N*/ 		pEndingAttrib->GetEnd() = nEnd;
/*N*/ 	else
/*N*/ 		InsertAttrib( rPoolItem, pNode, nStart, nEnd );
/*N*/ 
/*N*/ 	if ( pStartingAttrib )
/*?*/ 		pNode->GetCharAttribs().ResortAttribs();
/*N*/ 
/*N*/ 	SetModified( TRUE );
/*N*/ }


/*N*/ BOOL EditDoc::RemoveAttribs( ContentNode* pNode, USHORT nStart, USHORT nEnd, EditCharAttrib*& rpStarting, EditCharAttrib*& rpEnding, USHORT nWhich )
/*N*/ {
/*N*/ 	DBG_ASSERT( pNode, "Wohin mit dem Attribut?" );
/*N*/ 	DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribut zu gross!" );
/*N*/ 
/*N*/ 	// dieses endet am Anfang der Selektion => kann erweitert werden
/*N*/ 	rpEnding = 0;
/*N*/ 	// dieses startet am Ende der Selektion => kann erweitert werden
/*N*/ 	rpStarting = 0;
/*N*/ 
/*N*/ 	BOOL bChanged = FALSE;
/*N*/ 
/*N*/ 	DBG_ASSERT( nStart <= nEnd, "Kleiner Rechenfehler in InsertAttribInSelection" );
/*N*/ 
/*N*/ 	// ueber die Attribute iterieren...
/*N*/ 	USHORT nAttr = 0;
/*N*/ 	EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 	while ( pAttr )
/*N*/ 	{
/*N*/ 		BOOL bRemoveAttrib = FALSE;
/*N*/ 		// MT 11.9.97:
/*N*/ 		// Ich denke dass in dieser Methode generell keine Features geloescht
/*N*/ 		// werden sollen.
/*N*/ 		// => Dann koennen die Feature-Abfragen weiter unten entfallen
/*N*/ 		USHORT nAttrWhich = pAttr->Which();
/*N*/ 		if ( ( nAttrWhich < EE_FEATURE_START ) && ( !nWhich || ( nAttrWhich == nWhich ) ) )
/*N*/ 		{
/*N*/ 			// Attribut beginnt in Selection
/*N*/ 			if ( ( pAttr->GetStart() >= nStart ) && ( pAttr->GetStart() <= nEnd ) )
/*N*/ 			{
/*?*/ 				bChanged = TRUE;
/*?*/ 				if ( pAttr->GetEnd() > nEnd )
/*?*/ 				{
/*?*/ 					pAttr->GetStart() = nEnd;	// dann faengt es dahinter an
/*?*/ 					rpStarting = pAttr;
/*?*/ 					break;	// es kann kein weiteres Attrib hier liegen
/*?*/ 				}
/*?*/ 				else if ( !pAttr->IsFeature() || ( pAttr->GetStart() == nStart ) )
/*?*/ 				{
/*?*/ 					// Feature nur loeschen, wenn genau an der Stelle
/*?*/ 					bRemoveAttrib = TRUE;
/*?*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			// Attribut endet in Selection
/*N*/ 			else if ( ( pAttr->GetEnd() >= nStart ) && ( pAttr->GetEnd() <= nEnd ) )
/*N*/ 			{
/*N*/ 				bChanged = TRUE;
/*N*/ 				if ( ( pAttr->GetStart() < nStart ) && !pAttr->IsFeature() )
/*N*/ 				{
/*N*/ 					pAttr->GetEnd() = nStart;	// dann hoert es hier auf
/*N*/ 					rpEnding = pAttr;
/*N*/ 				}
/*N*/ 				else if ( !pAttr->IsFeature() || ( pAttr->GetStart() == nStart ) )
/*N*/ 				{
/*N*/ 					// Feature nur loeschen, wenn genau an der Stelle
/*N*/ 					bRemoveAttrib = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			// Attribut ueberlappt die Selektion
/*N*/ 			else if ( ( pAttr->GetStart() <= nStart ) && ( pAttr->GetEnd() >= nEnd ) )
/*N*/ 			{
/*?*/ 				bChanged = TRUE;
/*?*/ 				if ( pAttr->GetStart() == nStart )
/*?*/ 				{
/*?*/ 					pAttr->GetStart() = nEnd;
/*?*/ 					rpStarting = pAttr;
/*?*/ 					break;	// es kann weitere Attribute geben!
/*?*/ 				}
/*?*/ 				else if ( pAttr->GetEnd() == nEnd )
/*?*/ 				{
/*?*/ 					pAttr->GetEnd() = nStart;
/*?*/ 					rpEnding = pAttr;
/*?*/ 					break;	// es kann weitere Attribute geben!
/*?*/ 				}
/*?*/ 				else // Attribut muss gesplittet werden...
/*?*/ 				{
/*?*/ 					USHORT nOldEnd = pAttr->GetEnd();
/*?*/ 					pAttr->GetEnd() = nStart;
/*?*/ 					rpEnding = pAttr;
/*?*/ 					InsertAttrib( *pAttr->GetItem(), pNode, nEnd, nOldEnd );
/*?*/ 					break;	// es kann weitere Attribute geben!
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bRemoveAttrib )
/*N*/ 		{
/*?*/ 			DBG_ASSERT( ( pAttr != rpStarting ) && ( pAttr != rpEnding ), "Loeschen und behalten des gleichen Attributs ?" );
/*?*/ 			DBG_ASSERT( !pAttr->IsFeature(), "RemoveAttribs: Remove a feature?!" );
/*?*/ 			pNode->GetCharAttribs().GetAttribs().Remove(nAttr);
/*?*/ 			GetItemPool().Remove( *pAttr->GetItem() );
/*?*/ 			delete pAttr;
/*?*/ 			nAttr--;
/*N*/ 		}
/*N*/ 		nAttr++;
/*N*/ 		pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( bChanged )
/*N*/ 		SetModified( TRUE );
/*N*/ 
/*N*/ 	return bChanged;
/*N*/ }

/*N*/ void EditDoc::InsertAttrib( const SfxPoolItem& rPoolItem, ContentNode* pNode, USHORT nStart, USHORT nEnd )
/*N*/ {
/*N*/ 	// Diese Methode prueft nicht mehr, ob ein entspr. Attribut
/*N*/ 	// schon an der Stelle existiert!
/*N*/ 
/*N*/ 	EditCharAttrib* pAttrib = MakeCharAttrib( GetItemPool(), rPoolItem, nStart, nEnd );
/*N*/ 	DBG_ASSERT( pAttrib, "MakeCharAttrib fehlgeschlagen!" );
/*N*/ 	pNode->GetCharAttribs().InsertAttrib( pAttrib );
/*N*/ 
/*N*/ 	SetModified( TRUE );
/*N*/ }

/*N*/ void EditDoc::InsertAttrib( ContentNode* pNode, USHORT nStart, USHORT nEnd, const SfxPoolItem& rPoolItem )
/*N*/ {
/*N*/ 	if ( nStart != nEnd )
/*N*/ 	{
/*N*/ 		InsertAttribInSelection( pNode, nStart, nEnd, rPoolItem );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Pruefen, ob schon ein neues Attribut mit der WhichId an der Stelle:
/*N*/ 		EditCharAttrib* pAttr = pNode->GetCharAttribs().FindEmptyAttrib( rPoolItem.Which(), nStart );
/*N*/ 		if ( pAttr )
/*N*/ 		{
/*?*/ 			// Attribut entfernen....
/*?*/ 			pNode->GetCharAttribs().GetAttribs().Remove(
/*?*/ 				pNode->GetCharAttribs().GetAttribs().GetPos( pAttr ) );
/*N*/ 		}
/*N*/ 
/*N*/ 		// pruefen, ob ein 'gleiches' Attribut an der Stelle liegt.
/*N*/ 		pAttr = pNode->GetCharAttribs().FindAttrib( rPoolItem.Which(), nStart );
/*N*/ 		if ( pAttr )
/*N*/ 		{
/*?*/ 			if ( pAttr->IsInside( nStart ) )	// splitten
/*?*/ 			{
/*?*/ 				// ???????????????????????????????
/*?*/ 				// eigentlich noch pruefen, ob wirklich splittet, oder return !
/*?*/ 				// ???????????????????????????????
/*?*/ 				USHORT nOldEnd = pAttr->GetEnd();
/*?*/ 				pAttr->GetEnd() = nStart;
/*?*/ 				pAttr = MakeCharAttrib( GetItemPool(), *(pAttr->GetItem()), nStart, nOldEnd );
/*?*/ 				pNode->GetCharAttribs().InsertAttrib( pAttr );
/*?*/ 			}
/*?*/ 			else if ( pAttr->GetEnd() == nStart )
/*?*/ 			{
/*?*/ 				DBG_ASSERT( !pAttr->IsEmpty(), "Doch noch ein leeres Attribut?" );
/*?*/ 				// pruefen, ob genau das gleiche Attribut
/*?*/ 				if ( *(pAttr->GetItem()) == rPoolItem )
/*?*/ 					return;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		InsertAttrib( rPoolItem, pNode, nStart, nStart );
/*N*/ 	}
/*N*/ 
/*N*/ 	SetModified( TRUE );
/*N*/ }

/*N*/ void EditDoc::FindAttribs( ContentNode* pNode, USHORT nStartPos, USHORT nEndPos, SfxItemSet& rCurSet )
/*N*/ {
/*N*/ 	DBG_ASSERT( pNode, "Wo soll ich suchen ?" );
/*N*/ 	DBG_ASSERT( nStartPos <= nEndPos, "Ungueltiger Bereich!" );
/*N*/ 
/*N*/ 	USHORT nAttr = 0;
/*N*/ 	EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 	// keine Selection...
/*N*/ 	if ( nStartPos == nEndPos )
/*N*/ 	{
/*N*/ 		while ( pAttr && ( pAttr->GetStart() <= nEndPos) )
/*N*/ 		{
/*N*/ 			const SfxPoolItem* pItem = 0;
/*N*/ 			// Attribut liegt dadrueber...
/*N*/ 			if ( ( pAttr->GetStart() < nStartPos ) && ( pAttr->GetEnd() > nStartPos ) )
/*?*/ 				pItem = pAttr->GetItem();
/*N*/ 			// Attribut endet hier, ist nicht leer
/*N*/ 			else if ( ( pAttr->GetStart() < nStartPos ) && ( pAttr->GetEnd() == nStartPos ) )
/*N*/ 			{
/*?*/ 				if ( !pNode->GetCharAttribs().FindEmptyAttrib( pAttr->GetItem()->Which(), nStartPos ) )
/*?*/ 					pItem = pAttr->GetItem();
/*N*/ 			}
/*N*/ 			// Attribut endet hier, ist leer
/*N*/ 			else if ( ( pAttr->GetStart() == nStartPos ) && ( pAttr->GetEnd() == nStartPos ) )
/*N*/ 			{
/*?*/ 				pItem = pAttr->GetItem();
/*N*/ 			}
/*N*/ 			// Attribut beginnt hier
/*N*/ 			else if ( ( pAttr->GetStart() == nStartPos ) && ( pAttr->GetEnd() > nStartPos ) )
/*N*/ 			{
/*N*/ 				if ( nStartPos == 0 ) 	// Sonderfall
/*N*/ 					pItem = pAttr->GetItem();
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( pItem )
/*N*/ 			{
/*N*/ 				USHORT nWhich = pItem->Which();
/*N*/ 				if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
/*N*/ 				{
/*N*/ 					rCurSet.Put( *pItem );
/*N*/ 				}
/*N*/ 				else if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
/*N*/ 				{
/*?*/ 					const SfxPoolItem& rItem = rCurSet.Get( nWhich );
/*?*/ 					if ( rItem != *pItem )
/*?*/ 					{
/*?*/ 						rCurSet.InvalidateItem( nWhich );
/*?*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			nAttr++;
/*N*/ 			pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else	// Selektion
/*N*/ 	{
/*N*/ 		while ( pAttr && ( pAttr->GetStart() < nEndPos) )
/*N*/ 		{
/*N*/ 			const SfxPoolItem* pItem = 0;
/*N*/ 			// Attribut liegt dadrueber...
/*N*/ 			if ( ( pAttr->GetStart() <= nStartPos ) && ( pAttr->GetEnd() >= nEndPos ) )
/*N*/ 				pItem = pAttr->GetItem();
/*N*/ 			// Attribut startet mitten drin...
/*N*/ 			else if ( pAttr->GetStart() >= nStartPos )
/*N*/ 			{
/*N*/ 				// !!! pItem = pAttr->GetItem();
/*N*/ 				// einfach nur pItem reicht nicht, da ich z.B. bei Shadow
/*N*/ 				// niemals ein ungleiches Item finden wuerde, da ein solche
/*N*/ 				// seine Anwesenheit durch Abwesenheit repraesentiert!
/*N*/ 				// if ( ... )
/*N*/ 				// Es muesste geprueft werden, on genau das gleiche Attribut
/*N*/ 				// an der Bruchstelle aufsetzt, was recht aufwendig ist.
/*N*/ 				// Da ich beim Einfuegen von Attributen aber etwas optimiere
/*N*/ 				// tritt der Fall nicht so schnell auf...
/*N*/ 				// Also aus Geschwindigkeitsgruenden:
/*N*/ 				rCurSet.InvalidateItem( pAttr->GetItem()->Which() );
/*N*/ 
/*N*/ 			}
/*N*/ 			// Attribut endet mitten drin...
/*N*/ 			else if ( pAttr->GetEnd() > nStartPos )
/*N*/ 			{
/*N*/ 				// pItem = pAttr->GetItem();
/*N*/ 				// s.o.
                /*-----------------31.05.95 16:01-------------------
                 Ist falsch, wenn das gleiche Attribut sofort wieder
                 eingestellt wird!
                 => Sollte am besten nicht vorkommen, also gleich beim
                    Setzen von Attributen richtig machen!
                --------------------------------------------------*/
/*?*/ 				rCurSet.InvalidateItem( pAttr->GetItem()->Which() );
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( pItem )
/*N*/ 			{
/*N*/ 				USHORT nWhich = pItem->Which();
/*N*/ 				if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
/*N*/ 				{
/*N*/ 					rCurSet.Put( *pItem );
/*N*/ 				}
/*N*/ 				else if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
/*N*/ 				{
/*?*/ 					const SfxPoolItem& rItem = rCurSet.Get( nWhich );
/*?*/ 					if ( rItem != *pItem )
/*?*/ 					{
/*?*/ 						rCurSet.InvalidateItem( nWhich );
/*?*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			nAttr++;
/*N*/ 			pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


// -------------------------------------------------------------------------
// class EditCharAttribList
// -------------------------------------------------------------------------

/*N*/ CharAttribList::CharAttribList()
/*N*/ {
/*N*/ 	DBG_CTOR( EE_CharAttribList, 0 );
/*N*/ 	bHasEmptyAttribs = FALSE;
/*N*/ }

/*N*/ CharAttribList::~CharAttribList()
/*N*/ {
/*N*/ 	DBG_DTOR( EE_CharAttribList, 0 );
/*N*/ 
/*N*/ 	USHORT nAttr = 0;
/*N*/ 	EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
/*N*/ 	while ( pAttr )
/*N*/ 	{
/*N*/ 		delete pAttr;
/*N*/ 		++nAttr;
/*N*/ 		pAttr = GetAttrib( aAttribs, nAttr );
/*N*/ 	}
/*N*/ 	Clear();
/*N*/ }

/*N*/ void CharAttribList::InsertAttrib( EditCharAttrib* pAttrib )
/*N*/ {
/*N*/ // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*N*/ // optimieren: binaere Suche ? !
/*N*/ // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*N*/ 
/*N*/ 	// MT: 26.11.98
/*N*/ 	// Vielleicht aber auch einfach nur rueckwaerts iterieren:
/*N*/ 	// Der haeufigste und kritischste Fall: Attribute kommen bereits
/*N*/ 	// sortiert an (InsertBinTextObject!)
/*N*/ 	// Hier waere auch binaere Suche nicht optimal.
/*N*/ 	// => Wuerde einiges bringen!
/*N*/ 
/*N*/ 	const USHORT nCount = Count();
/*N*/ 	const USHORT nStart = pAttrib->GetStart(); // vielleicht besser fuer Comp.Opt.
/*N*/ 
/*N*/ 	if ( pAttrib->IsEmpty() )
/*N*/ 		bHasEmptyAttribs = TRUE;
/*N*/ 
/*N*/ 	BOOL bInserted = FALSE;
/*N*/ 	for ( USHORT x = 0; x < nCount; x++ )
/*N*/ 	{
/*N*/ 		EditCharAttribPtr pCurAttrib = aAttribs[x];
/*N*/ 		if ( pCurAttrib->GetStart() > nStart )
/*N*/ 		{
/*N*/ 			aAttribs.Insert( pAttrib, x );
/*N*/ 			bInserted = TRUE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( !bInserted )
/*N*/ 		aAttribs.Insert( pAttrib, nCount );
/*N*/ }

/*N*/ void CharAttribList::ResortAttribs()
/*N*/ {
/*N*/ 	if ( Count() )
/*N*/ 	{
/*N*/ 		qsort( (void*)aAttribs.GetData(), aAttribs.Count(), sizeof( EditCharAttrib* ), CompareStart );
/*N*/ 	}
/*N*/ }

/*N*/ void CharAttribList::OptimizeRanges( SfxItemPool& rItemPool )
/*N*/ {
/*N*/ 	for ( USHORT n = 0; n < aAttribs.Count(); n++ )
/*N*/ 	{
/*N*/         EditCharAttrib* pAttr = aAttribs.GetObject( n );
/*N*/         for ( USHORT nNext = n+1; nNext < aAttribs.Count(); nNext++ )
/*N*/         {
/*N*/             EditCharAttrib* p = aAttribs.GetObject( nNext );
/*N*/             if ( !pAttr->IsFeature() && ( p->GetStart() == pAttr->GetEnd() ) && ( p->Which() == pAttr->Which() ) )
/*N*/             {
/*N*/                 if ( *p->GetItem() == *pAttr->GetItem() )
/*N*/                 {
/*N*/                     pAttr->GetEnd() = p->GetEnd();
/*N*/                     aAttribs.Remove( nNext );
/*N*/                     rItemPool.Remove( *p->GetItem() );
/*N*/                     delete p;
/*N*/                 }
/*N*/                 break;  // only 1 attr with same which can start here.
/*N*/             }
/*N*/             else if ( p->GetStart() > pAttr->GetEnd() )
/*N*/             {
/*N*/                 break;
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/ }

/*N*/ EditCharAttrib* CharAttribList::FindAttrib( USHORT nWhich, USHORT nPos )
/*N*/ {
/*N*/ 	// Rueckwaerts, falls eins dort endet, das naechste startet.
/*N*/ 	// => Das startende gilt...
/*N*/ 	USHORT nAttr = aAttribs.Count()-1;
/*N*/ 	EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
/*N*/ 	while ( pAttr )
/*N*/ 	{
/*N*/ 		if ( ( pAttr->Which() == nWhich ) && pAttr->IsIn(nPos) )
/*N*/ 			return pAttr;
/*N*/ 		pAttr = GetAttrib( aAttribs, --nAttr );
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ EditCharAttrib* CharAttribList::FindNextAttrib( USHORT nWhich, USHORT nFromPos ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( nWhich, "FindNextAttrib: Which?" );
/*N*/ 	const USHORT nAttribs = aAttribs.Count();
/*N*/ 	for ( USHORT nAttr = 0; nAttr < nAttribs; nAttr++ )
/*N*/ 	{
/*N*/ 		EditCharAttrib* pAttr = aAttribs[ nAttr ];
/*N*/ 		if ( ( pAttr->GetStart() >= nFromPos ) && ( pAttr->Which() == nWhich ) )
/*N*/ 			return pAttr;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }




/*N*/ BOOL CharAttribList::HasBoundingAttrib( USHORT nBound )
/*N*/ {
/*N*/ 	// Rueckwaerts, falls eins dort endet, das naechste startet.
/*N*/ 	// => Das startende gilt...
/*N*/ 	USHORT nAttr = aAttribs.Count()-1;
/*N*/ 	EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
/*N*/ 	while ( pAttr && ( pAttr->GetEnd() >= nBound ) )
/*N*/ 	{
/*?*/ 		if ( ( pAttr->GetStart() == nBound ) || ( pAttr->GetEnd() == nBound ) )
/*?*/ 			return TRUE;
/*?*/ 		pAttr = GetAttrib( aAttribs, --nAttr );
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ EditCharAttrib* CharAttribList::FindEmptyAttrib( USHORT nWhich, USHORT nPos )
/*N*/ {
/*N*/ 	if ( !bHasEmptyAttribs )
/*N*/ 		return 0;
/*N*/ 	USHORT nAttr = 0;
/*N*/ 	EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
/*N*/ 	while ( pAttr && ( pAttr->GetStart() <= nPos ) )
/*N*/ 	{
/*N*/ 		if ( ( pAttr->GetStart() == nPos ) && ( pAttr->GetEnd() == nPos ) && ( pAttr->Which() == nWhich ) )
/*N*/ 			return pAttr;
/*N*/ 		nAttr++;
/*N*/ 		pAttr = GetAttrib( aAttribs, nAttr );
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ EditCharAttrib*	CharAttribList::FindFeature( USHORT nPos ) const
/*N*/ {
/*N*/ 
/*N*/ 	USHORT nAttr = 0;
/*N*/ 	EditCharAttrib* pNextAttrib = GetAttrib( aAttribs, nAttr );
/*N*/ 
/*N*/ 	// erstmal zur gewuenschten Position...
/*N*/ 	while ( pNextAttrib && ( pNextAttrib->GetStart() < nPos ) )
/*N*/ 	{
/*N*/ 		nAttr++;
/*N*/ 		pNextAttrib = GetAttrib( aAttribs, nAttr );
/*N*/ 	}
/*N*/ 
/*N*/ 	// jetzt das Feature suchen...
/*N*/ 	while ( pNextAttrib && !pNextAttrib->IsFeature() )
/*N*/ 	{
/*N*/ 		nAttr++;
/*N*/ 		pNextAttrib = GetAttrib( aAttribs, nAttr );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pNextAttrib;
/*N*/ }












/*N*/ EditEngineItemPool::EditEngineItemPool( BOOL bPersistenRefCounts )
/*N*/ 	: SfxItemPool( String( "EditEngineItemPool", RTL_TEXTENCODING_ASCII_US ), EE_ITEMS_START, EE_ITEMS_END,
/*N*/ 					aItemInfos, 0, bPersistenRefCounts )
/*N*/ {
/*N*/ 	SetVersionMap( 1, 3999, 4015, aV1Map );
/*N*/ 	SetVersionMap( 2, 3999, 4019, aV2Map );
/*N*/ 	SetVersionMap( 3, 3997, 4020, aV3Map );
/*N*/ 	SetVersionMap( 4, 3994, 4022, aV4Map );
/*N*/ 
/*N*/ 	DBG_ASSERT( EE_DLL(), "EditDLL?!" );
/*N*/ 	SfxPoolItem** ppDefItems = EE_DLL()->GetGlobalData()->GetDefItems();
/*N*/ 	SetDefaults( ppDefItems );
/*N*/ }

/*N*/ EditEngineItemPool::~EditEngineItemPool()
/*N*/ {
/*N*/ }

/*N*/ SvStream& EditEngineItemPool::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	// Bei einem 3.1-Export muess ein Hack eingebaut werden, da BUG im
/*N*/ 	// SfxItemSet::Load, aber nicht nachtraeglich in 3.1 fixbar.
/*N*/ 
/*N*/ 	// Der eingestellte Range muss nach Store erhalten bleiben, weil dann
/*N*/ 	// erst die ItemSets gespeichert werden...
/*N*/ 
/*N*/ 	long nVersion = rStream.GetVersion();
/*N*/ 	BOOL b31Format = ( nVersion && ( nVersion <= SOFFICE_FILEFORMAT_31 ) )
/*N*/ 						? TRUE : FALSE;
/*N*/ 
/*N*/ 	EditEngineItemPool* pThis = (EditEngineItemPool*)this;
/*N*/ 	if ( b31Format )
/*N*/ 		pThis->SetStoringRange( 3997, 4022 );
/*N*/ 	else
/*N*/ 		pThis->SetStoringRange( EE_ITEMS_START, EE_ITEMS_END );
/*N*/ 
/*N*/ 	return SfxItemPool::Store( rStream );
/*N*/ }
}
