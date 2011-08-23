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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "scitems.hxx"
#include <vcl/outdev.hxx>
#include <bf_svtools/aeitem.hxx>
#include <bf_svx/algitem.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svx/charreliefitem.hxx>
#include <bf_svx/cntritem.hxx>
#include <bf_svx/colritem.hxx>
#include <bf_svx/crsditem.hxx>
#include <bf_svx/emphitem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/fontitem.hxx>
#include <bf_svx/forbiddenruleitem.hxx>
#include <bf_svx/frmdiritem.hxx>
#include <bf_svx/hngpnctitem.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/pageitem.hxx>
#include <bf_svx/pbinitem.hxx>
#include <bf_svx/postitem.hxx>
#include <bf_svx/rotmodit.hxx>
#include <bf_svx/scriptspaceitem.hxx>
#include <bf_svx/shaditem.hxx>
#include <bf_svx/shdditem.hxx>
#include <bf_svx/sizeitem.hxx>
#include <bf_svx/svxitems.hrc>
#include <bf_svx/udlnitem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/wghtitem.hxx>
#include <bf_svx/wrlmitem.hxx>
#include <bf_svx/xmlcnitm.hxx>

#include "docpool.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "globstr.hrc"
#include "bf_sc.hrc"			// Slot-IDs
namespace binfilter {


#define SC_MAX_POOLREF		(SFX_ITEMS_OLD_MAXREF - 39)
#define SC_SAFE_POOLREF		(SC_MAX_POOLREF + 20)

// STATIC DATA -----------------------------------------------------------

/*N*/ USHORT* ScDocumentPool::pVersionMap1 = 0;
/*N*/ USHORT* ScDocumentPool::pVersionMap2 = 0;
/*N*/ USHORT* ScDocumentPool::pVersionMap3 = 0;
/*N*/ USHORT* ScDocumentPool::pVersionMap4 = 0;
/*N*/ USHORT* ScDocumentPool::pVersionMap5 = 0;
/*N*/ USHORT* ScDocumentPool::pVersionMap6 = 0;
/*N*/ USHORT* ScDocumentPool::pVersionMap7 = 0;
/*N*/ USHORT* ScDocumentPool::pVersionMap8 = 0;
/*N*/ USHORT* ScDocumentPool::pVersionMap9 = 0;

// ATTR_FONT_TWOLINES (not used) was changed to ATTR_USERDEF (not saved in binary format) in 641c

/*N*/ static SfxItemInfo __READONLY_DATA  aItemInfos[] =
/*N*/ {
/*N*/ 	{ SID_ATTR_CHAR_FONT,			SFX_ITEM_POOLABLE },	// ATTR_FONT
/*N*/ 	{ SID_ATTR_CHAR_FONTHEIGHT,		SFX_ITEM_POOLABLE },	// ATTR_FONT_HEIGHT
/*N*/ 	{ SID_ATTR_CHAR_WEIGHT,			SFX_ITEM_POOLABLE },	// ATTR_FONT_WEIGHT
/*N*/ 	{ SID_ATTR_CHAR_POSTURE,		SFX_ITEM_POOLABLE },	// ATTR_FONT_POSTURE
/*N*/ 	{ SID_ATTR_CHAR_UNDERLINE,		SFX_ITEM_POOLABLE },	// ATTR_FONT_UNDERLINE
/*N*/ 	{ SID_ATTR_CHAR_STRIKEOUT,		SFX_ITEM_POOLABLE },	// ATTR_FONT_CROSSEDOUT
/*N*/ 	{ SID_ATTR_CHAR_CONTOUR,		SFX_ITEM_POOLABLE },	// ATTR_FONT_CONTOUR
/*N*/ 	{ SID_ATTR_CHAR_SHADOWED,		SFX_ITEM_POOLABLE },	// ATTR_FONT_SHADOWED
/*N*/ 	{ SID_ATTR_CHAR_COLOR,			SFX_ITEM_POOLABLE },	// ATTR_FONT_COLOR
/*N*/ 	{ SID_ATTR_CHAR_LANGUAGE,		SFX_ITEM_POOLABLE },	// ATTR_FONT_LANGUAGE
/*N*/ 	{ SID_ATTR_CHAR_CJK_FONT,		SFX_ITEM_POOLABLE },	// ATTR_CJK_FONT			from 614
/*N*/ 	{ SID_ATTR_CHAR_CJK_FONTHEIGHT,	SFX_ITEM_POOLABLE },	// ATTR_CJK_FONT_HEIGHT		from 614
/*N*/ 	{ SID_ATTR_CHAR_CJK_WEIGHT,		SFX_ITEM_POOLABLE },	// ATTR_CJK_FONT_WEIGHT		from 614
/*N*/ 	{ SID_ATTR_CHAR_CJK_POSTURE,	SFX_ITEM_POOLABLE },	// ATTR_CJK_FONT_POSTURE	from 614
/*N*/ 	{ SID_ATTR_CHAR_CJK_LANGUAGE,	SFX_ITEM_POOLABLE },	// ATTR_CJK_FONT_LANGUAGE	from 614
/*N*/ 	{ SID_ATTR_CHAR_CTL_FONT,		SFX_ITEM_POOLABLE },	// ATTR_CTL_FONT			from 614
/*N*/ 	{ SID_ATTR_CHAR_CTL_FONTHEIGHT,	SFX_ITEM_POOLABLE },	// ATTR_CTL_FONT_HEIGHT		from 614
/*N*/ 	{ SID_ATTR_CHAR_CTL_WEIGHT,		SFX_ITEM_POOLABLE },	// ATTR_CTL_FONT_WEIGHT		from 614
/*N*/ 	{ SID_ATTR_CHAR_CTL_POSTURE,	SFX_ITEM_POOLABLE },	// ATTR_CTL_FONT_POSTURE	from 614
/*N*/ 	{ SID_ATTR_CHAR_CTL_LANGUAGE,	SFX_ITEM_POOLABLE },	// ATTR_CTL_FONT_LANGUAGE	from 614
/*N*/ 	{ SID_ATTR_CHAR_EMPHASISMARK,	SFX_ITEM_POOLABLE },	// ATTR_FONT_EMPHASISMARK	from 614
/*N*/ 	{ 0,							SFX_ITEM_POOLABLE },	// ATTR_USERDEF				from 614 / 641c
/*N*/ 	{ SID_ATTR_CHAR_WORDLINEMODE,	SFX_ITEM_POOLABLE },	// ATTR_FONT_WORDLINE		from 632b
/*N*/ 	{ SID_ATTR_CHAR_RELIEF,			SFX_ITEM_POOLABLE },	// ATTR_FONT_RELIEF			from 632b
/*N*/ 	{ SID_ATTR_ALIGN_HYPHENATION,	SFX_ITEM_POOLABLE },	// ATTR_HYPHENATE			from 632b
/*N*/ 	{ 0,							SFX_ITEM_POOLABLE },	// ATTR_SCRIPTSPACE			from 614d
/*N*/ 	{ 0,							SFX_ITEM_POOLABLE },	// ATTR_HANGPUNCTUATION		from 614d
/*N*/ 	{ SID_ATTR_PARA_FORBIDDEN_RULES,SFX_ITEM_POOLABLE },	// ATTR_FORBIDDEN_RULES		from 614d
/*N*/ 	{ SID_ATTR_ALIGN_HOR_JUSTIFY,	SFX_ITEM_POOLABLE },	// ATTR_HOR_JUSTIFY
/*N*/ 	{ SID_ATTR_ALIGN_INDENT,		SFX_ITEM_POOLABLE },	// ATTR_INDENT			ab 350
/*N*/ 	{ SID_ATTR_ALIGN_VER_JUSTIFY,	SFX_ITEM_POOLABLE },	// ATTR_VER_JUSTIFY
/*N*/ 	{ SID_ATTR_ALIGN_ORIENTATION,	SFX_ITEM_POOLABLE },	// ATTR_ORIENTATION
/*N*/ 	{ SID_ATTR_ALIGN_DEGREES,		SFX_ITEM_POOLABLE },	// ATTR_ROTATE_VALUE	ab 367
/*N*/ 	{ SID_ATTR_ALIGN_LOCKPOS,		SFX_ITEM_POOLABLE },	// ATTR_ROTATE_MODE		ab 367
/*N*/     { SID_ATTR_ALIGN_ASIANVERTICAL, SFX_ITEM_POOLABLE },    // ATTR_VERTICAL_ASIAN  from 642
/*N*/ 	{ SID_ATTR_FRAMEDIRECTION,		SFX_ITEM_POOLABLE },    // ATTR_WRITINGDIR		from 643
/*N*/ 	{ SID_ATTR_ALIGN_LINEBREAK,		SFX_ITEM_POOLABLE },	// ATTR_LINEBREAK
/*N*/ 	{ SID_ATTR_ALIGN_MARGIN,		SFX_ITEM_POOLABLE },	// ATTR_MARGIN
/*N*/ 	{ 0,							SFX_ITEM_POOLABLE },	// ATTR_MERGE
/*N*/ 	{ 0,							SFX_ITEM_POOLABLE },	// ATTR_MERGE_FLAG
/*N*/ 	{ SID_ATTR_NUMBERFORMAT_VALUE,	SFX_ITEM_POOLABLE },	// ATTR_VALUE_FORMAT
/*N*/ 	{ ATTR_LANGUAGE_FORMAT,			SFX_ITEM_POOLABLE },	// ATTR_LANGUAGE_FORMAT ab 329, wird im Dialog mit SID_ATTR_NUMBERFORMAT_VALUE kombiniert
/*N*/ 	{ SID_ATTR_BRUSH,				SFX_ITEM_POOLABLE },	// ATTR_BACKGROUND
/*N*/ 	{ SID_SCATTR_PROTECTION,		SFX_ITEM_POOLABLE },	// ATTR_PROTECTION
/*N*/ 	{ SID_ATTR_BORDER_OUTER,		SFX_ITEM_POOLABLE },	// ATTR_BORDER
/*N*/ 	{ SID_ATTR_BORDER_INNER,		SFX_ITEM_POOLABLE },	// ATTR_BORDER_INNER
/*N*/ 	{ SID_ATTR_BORDER_SHADOW,		SFX_ITEM_POOLABLE },	// ATTR_SHADOW
/*N*/ 	{ 0,							SFX_ITEM_POOLABLE },	// ATTR_VALIDDATA
/*N*/ 	{ 0,							SFX_ITEM_POOLABLE },	// ATTR_CONDITIONAL
/*N*/ 	{ 0,							SFX_ITEM_POOLABLE },	// ATTR_PATTERN
/*N*/ 	{ SID_ATTR_LRSPACE,				SFX_ITEM_POOLABLE },	// ATTR_LRSPACE
/*N*/ 	{ SID_ATTR_ULSPACE,				SFX_ITEM_POOLABLE },	// ATTR_ULSPACE
/*N*/ 	{ SID_ATTR_PAGE,				SFX_ITEM_POOLABLE },	// ATTR_PAGE
/*N*/ 	{ 0,							SFX_ITEM_POOLABLE },	// ATTR_PAGE_PAPERTRAY, seit 303 nur noch dummy
/*N*/ 	{ SID_ATTR_PAGE_PAPERBIN,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_PAPERBIN
/*N*/ 	{ SID_ATTR_PAGE_SIZE,			SFX_ITEM_POOLABLE },	// ATTR_PAGE_SIZE
/*N*/ 	{ SID_ATTR_PAGE_MAXSIZE,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_MAXSIZE
/*N*/ 	{ SID_ATTR_PAGE_EXT1,			SFX_ITEM_POOLABLE },	// ATTR_PAGE_HORCENTER
/*N*/ 	{ SID_ATTR_PAGE_EXT2,			SFX_ITEM_POOLABLE },	// ATTR_PAGE_VERCENTER
/*N*/ 	{ SID_ATTR_PAGE_ON,				SFX_ITEM_POOLABLE },	// ATTR_PAGE_ON
/*N*/ 	{ SID_ATTR_PAGE_DYNAMIC,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_DYNAMIC
/*N*/ 	{ SID_ATTR_PAGE_SHARED,			SFX_ITEM_POOLABLE },	// ATTR_PAGE_SHARED
/*N*/ 	{ SID_SCATTR_PAGE_NOTES,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_NOTES
/*N*/ 	{ SID_SCATTR_PAGE_GRID,			SFX_ITEM_POOLABLE },	// ATTR_PAGE_GRID
/*N*/ 	{ SID_SCATTR_PAGE_HEADERS,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_HEADERS
/*N*/ 	{ SID_SCATTR_PAGE_CHARTS,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_CHARTS
/*N*/ 	{ SID_SCATTR_PAGE_OBJECTS,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_OBJECTS
/*N*/ 	{ SID_SCATTR_PAGE_DRAWINGS,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_DRAWINGS
/*N*/ 	{ SID_SCATTR_PAGE_TOPDOWN,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_TOPDOWN
/*N*/ 	{ SID_SCATTR_PAGE_SCALE,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_SCALE
/*N*/ 	{ SID_SCATTR_PAGE_SCALETOPAGES,	SFX_ITEM_POOLABLE },	// ATTR_PAGE_SCALETOPAGES
/*N*/ 	{ SID_SCATTR_PAGE_FIRSTPAGENO,	SFX_ITEM_POOLABLE },	// ATTR_PAGE_FIRSTPAGENO
/*N*/ 	{ SID_SCATTR_PAGE_PRINTAREA,	SFX_ITEM_POOLABLE },	// ATTR_PAGE_PRINTAREA
/*N*/ 	{ SID_SCATTR_PAGE_REPEATROW,	SFX_ITEM_POOLABLE },	// ATTR_PAGE_REPEATROW
/*N*/ 	{ SID_SCATTR_PAGE_REPEATCOL,	SFX_ITEM_POOLABLE },	// ATTR_PAGE_REPEATCOL
/*N*/ 	{ SID_SCATTR_PAGE_PRINTTABLES,	SFX_ITEM_POOLABLE },	// ATTR_PAGE_PRINTTABLES
/*N*/ 	{ SID_SCATTR_PAGE_HEADERLEFT,	SFX_ITEM_POOLABLE },	// ATTR_PAGE_HEADERLEFT
/*N*/ 	{ SID_SCATTR_PAGE_FOOTERLEFT,	SFX_ITEM_POOLABLE },	// ATTR_PAGE_FOOTERLEFT
/*N*/ 	{ SID_SCATTR_PAGE_HEADERRIGHT,	SFX_ITEM_POOLABLE },	// ATTR_PAGE_HEADERRIGHT
/*N*/ 	{ SID_SCATTR_PAGE_FOOTERRIGHT,	SFX_ITEM_POOLABLE },	// ATTR_PAGE_FOOTERRIGHT
/*N*/ 	{ SID_ATTR_PAGE_HEADERSET,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_HEADERSET
/*N*/ 	{ SID_ATTR_PAGE_FOOTERSET,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_FOOTERSET
/*N*/ 	{ SID_SCATTR_PAGE_FORMULAS,		SFX_ITEM_POOLABLE },	// ATTR_PAGE_FORMULAS
/*N*/ 	{ SID_SCATTR_PAGE_NULLVALS,		SFX_ITEM_POOLABLE }		// ATTR_PAGE_NULLVALS
/*N*/ };

// -----------------------------------------------------------------------

/*N*/ ScDocumentPool::ScDocumentPool( SfxItemPool* pSecPool, BOOL bLoadRefCounts )
/*N*/ 
/*N*/ 	:	SfxItemPool	( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScDocumentPool")),
/*N*/ 						ATTR_STARTINDEX, ATTR_ENDINDEX,
/*N*/ 						aItemInfos, NULL, bLoadRefCounts ),
/*N*/ 		pSecondary	( pSecPool )
/*N*/ {
/*N*/ 	//	latin font from GetDefaultFonts is not used, DEFAULTFONT_LATIN_SPREADSHEET instead
/*N*/ 	Font aStdFont = OutputDevice::GetDefaultFont( DEFAULTFONT_LATIN_SPREADSHEET, LANGUAGE_ENGLISH_US,
/*N*/ 													DEFAULTFONT_FLAGS_ONLYONE );
/*N*/ 	SvxFontItem* pStdFont = new SvxFontItem( aStdFont.GetFamily(),
/*N*/ 											aStdFont.GetName(), aStdFont.GetStyleName(),
/*N*/ 											aStdFont.GetPitch(), aStdFont.GetCharSet(),
/*N*/ 											ATTR_FONT );
/*N*/ 
/*N*/ 	SvxFontItem* pCjkFont = new SvxFontItem( ATTR_CJK_FONT );
/*N*/ 	SvxFontItem* pCtlFont = new SvxFontItem( ATTR_CTL_FONT );
/*N*/ 	SvxFontItem aDummy;
/*N*/ 	GetDefaultFonts( aDummy, *pCjkFont, *pCtlFont );
/*N*/ 
/*N*/ 	SvxBoxInfoItem*	pGlobalBorderInnerAttr = new SvxBoxInfoItem( ATTR_BORDER_INNER );
/*N*/ 	SfxItemSet*		pSet = new SfxItemSet( *this, ATTR_PATTERN_START, ATTR_PATTERN_END );
/*N*/ 	SfxItemSet	 	aSetItemItemSet( *this,
/*N*/ 									 ATTR_BACKGROUND, ATTR_BACKGROUND,
/*N*/ 									 ATTR_BORDER,	  ATTR_SHADOW,
/*N*/ 									 ATTR_LRSPACE,	  ATTR_ULSPACE,
/*N*/ 									 ATTR_PAGE_SIZE,  ATTR_PAGE_SIZE,
/*N*/ 									 ATTR_PAGE_ON,	  ATTR_PAGE_SHARED,
/*N*/ 									 0 );
/*N*/ 
/*N*/ 	pGlobalBorderInnerAttr->SetLine(NULL, BOXINFO_LINE_HORI);
/*N*/ 	pGlobalBorderInnerAttr->SetLine(NULL, BOXINFO_LINE_VERT);
/*N*/ 	pGlobalBorderInnerAttr->SetTable(TRUE);
/*N*/ 	pGlobalBorderInnerAttr->SetDist((BOOL)FALSE);
/*N*/ 	pGlobalBorderInnerAttr->SetMinDist(FALSE);
/*N*/ 
/*N*/ 	ppPoolDefaults = new SfxPoolItem*[ATTR_ENDINDEX-ATTR_STARTINDEX+1];
/*N*/ 
/*N*/ 	ppPoolDefaults[ ATTR_FONT			 - ATTR_STARTINDEX ] = pStdFont;
/*N*/ 	ppPoolDefaults[ ATTR_FONT_HEIGHT	 - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200 );		// 10 pt;
/*N*/ 	ppPoolDefaults[ ATTR_FONT_WEIGHT	 - ATTR_STARTINDEX ] = new SvxWeightItem;
/*N*/ 	ppPoolDefaults[ ATTR_FONT_POSTURE	 - ATTR_STARTINDEX ] = new SvxPostureItem;
/*N*/ 	ppPoolDefaults[ ATTR_FONT_UNDERLINE	 - ATTR_STARTINDEX ] = new SvxUnderlineItem;
/*N*/ 	ppPoolDefaults[ ATTR_FONT_CROSSEDOUT - ATTR_STARTINDEX ] = new SvxCrossedOutItem;
/*N*/ 	ppPoolDefaults[ ATTR_FONT_CONTOUR	 - ATTR_STARTINDEX ] = new SvxContourItem;
/*N*/ 	ppPoolDefaults[ ATTR_FONT_SHADOWED	 - ATTR_STARTINDEX ] = new SvxShadowedItem;
/*N*/ 	ppPoolDefaults[ ATTR_FONT_COLOR		 - ATTR_STARTINDEX ] = new SvxColorItem( Color(COL_AUTO) );
/*N*/ 	ppPoolDefaults[	ATTR_FONT_LANGUAGE	 - ATTR_STARTINDEX ] = new SvxLanguageItem(	LanguageType(LANGUAGE_DONTKNOW), ATTR_FONT_LANGUAGE );
/*N*/ 	ppPoolDefaults[ ATTR_CJK_FONT		 - ATTR_STARTINDEX ] = pCjkFont;
/*N*/ 	ppPoolDefaults[ ATTR_CJK_FONT_HEIGHT - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200, 100, ATTR_CJK_FONT_HEIGHT );
/*N*/ 	ppPoolDefaults[ ATTR_CJK_FONT_WEIGHT - ATTR_STARTINDEX ] = new SvxWeightItem( WEIGHT_NORMAL, ATTR_CJK_FONT_WEIGHT );
/*N*/ 	ppPoolDefaults[ ATTR_CJK_FONT_POSTURE- ATTR_STARTINDEX ] = new SvxPostureItem( ITALIC_NONE, ATTR_CJK_FONT_POSTURE );
/*N*/ 	ppPoolDefaults[	ATTR_CJK_FONT_LANGUAGE-ATTR_STARTINDEX ] = new SvxLanguageItem(	LanguageType(LANGUAGE_DONTKNOW),
/*N*/ 																	ATTR_CJK_FONT_LANGUAGE );
/*N*/ 	ppPoolDefaults[ ATTR_CTL_FONT		 - ATTR_STARTINDEX ] = pCtlFont;
/*N*/ 	ppPoolDefaults[ ATTR_CTL_FONT_HEIGHT - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200, 100, ATTR_CTL_FONT_HEIGHT );
/*N*/ 	ppPoolDefaults[ ATTR_CTL_FONT_WEIGHT - ATTR_STARTINDEX ] = new SvxWeightItem( WEIGHT_NORMAL, ATTR_CTL_FONT_WEIGHT );
/*N*/ 	ppPoolDefaults[ ATTR_CTL_FONT_POSTURE- ATTR_STARTINDEX ] = new SvxPostureItem( ITALIC_NONE, ATTR_CTL_FONT_POSTURE );
/*N*/ 	ppPoolDefaults[	ATTR_CTL_FONT_LANGUAGE-ATTR_STARTINDEX ] = new SvxLanguageItem(	LanguageType(LANGUAGE_DONTKNOW),
/*N*/ 																	ATTR_CTL_FONT_LANGUAGE );
/*N*/ 	ppPoolDefaults[	ATTR_FONT_EMPHASISMARK-ATTR_STARTINDEX ] = new SvxEmphasisMarkItem;
/*N*/ 	ppPoolDefaults[	ATTR_USERDEF		 - ATTR_STARTINDEX ] = new SvXMLAttrContainerItem( ATTR_USERDEF );
/*N*/ 	ppPoolDefaults[	ATTR_FONT_WORDLINE	 - ATTR_STARTINDEX ] = new SvxWordLineModeItem;
/*N*/ 	ppPoolDefaults[	ATTR_FONT_RELIEF	 - ATTR_STARTINDEX ] = new SvxCharReliefItem;
/*N*/ 	ppPoolDefaults[	ATTR_HYPHENATE		 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_HYPHENATE );
/*N*/ 	ppPoolDefaults[	ATTR_SCRIPTSPACE	 - ATTR_STARTINDEX ] = new SvxScriptSpaceItem;
/*N*/ 	ppPoolDefaults[	ATTR_HANGPUNCTUATION - ATTR_STARTINDEX ] = new SvxHangingPunctuationItem;
/*N*/ 	ppPoolDefaults[	ATTR_FORBIDDEN_RULES - ATTR_STARTINDEX ] = new SvxForbiddenRuleItem;
/*N*/ 	ppPoolDefaults[	ATTR_HOR_JUSTIFY	 - ATTR_STARTINDEX ] = new SvxHorJustifyItem;
/*N*/ 	ppPoolDefaults[	ATTR_INDENT			 - ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_INDENT, 0 );
/*N*/ 	ppPoolDefaults[	ATTR_VER_JUSTIFY	 - ATTR_STARTINDEX ] = new SvxVerJustifyItem;
/*N*/ 	ppPoolDefaults[	ATTR_ORIENTATION	 - ATTR_STARTINDEX ] = new SvxOrientationItem;
/*N*/ 	ppPoolDefaults[	ATTR_ROTATE_VALUE	 - ATTR_STARTINDEX ] = new SfxInt32Item( ATTR_ROTATE_VALUE, 0 );
/*N*/ 	ppPoolDefaults[	ATTR_ROTATE_MODE	 - ATTR_STARTINDEX ] = new SvxRotateModeItem( SVX_ROTATE_MODE_BOTTOM, ATTR_ROTATE_MODE );
/*N*/ 	ppPoolDefaults[	ATTR_VERTICAL_ASIAN	 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_VERTICAL_ASIAN );
    //	The default for the ATTR_WRITINGDIR cell attribute must by FRMDIR_ENVIRONMENT,
    //	so that value is returned when asking for a default cell's attributes.
    //	The value from the page style is set as DefaultHorizontalTextDirection for the EditEngine.
/*N*/ 	ppPoolDefaults[	ATTR_WRITINGDIR		 - ATTR_STARTINDEX ] = new SvxFrameDirectionItem( FRMDIR_ENVIRONMENT, ATTR_WRITINGDIR );
/*N*/ 	ppPoolDefaults[	ATTR_LINEBREAK		 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_LINEBREAK );
/*N*/ 	ppPoolDefaults[	ATTR_MARGIN			 - ATTR_STARTINDEX ] = new SvxMarginItem;
/*N*/ 	ppPoolDefaults[	ATTR_MERGE			 - ATTR_STARTINDEX ] = new ScMergeAttr;
/*N*/ 	ppPoolDefaults[	ATTR_MERGE_FLAG		 - ATTR_STARTINDEX ] = new ScMergeFlagAttr;
/*N*/ 	ppPoolDefaults[	ATTR_VALUE_FORMAT	 - ATTR_STARTINDEX ] = new SfxUInt32Item( ATTR_VALUE_FORMAT, 0 );
/*N*/ 	ppPoolDefaults[	ATTR_LANGUAGE_FORMAT - ATTR_STARTINDEX ] = new SvxLanguageItem(	ScGlobal::eLnge, ATTR_LANGUAGE_FORMAT );
/*N*/ 	ppPoolDefaults[	ATTR_BACKGROUND		 - ATTR_STARTINDEX ] = new SvxBrushItem( Color(COL_TRANSPARENT), ATTR_BACKGROUND );
/*N*/ 	ppPoolDefaults[	ATTR_PROTECTION		 - ATTR_STARTINDEX ] = new ScProtectionAttr;
/*N*/ 	ppPoolDefaults[	ATTR_BORDER			 - ATTR_STARTINDEX ] = new SvxBoxItem( ATTR_BORDER );
/*N*/ 	ppPoolDefaults[ ATTR_BORDER_INNER	 - ATTR_STARTINDEX ] = pGlobalBorderInnerAttr;
/*N*/ 	ppPoolDefaults[	ATTR_SHADOW			 - ATTR_STARTINDEX ] = new SvxShadowItem( ATTR_SHADOW );
/*N*/ 	ppPoolDefaults[	ATTR_VALIDDATA		 - ATTR_STARTINDEX ] = new SfxUInt32Item( ATTR_VALIDDATA, 0 );
/*N*/ 	ppPoolDefaults[	ATTR_CONDITIONAL	 - ATTR_STARTINDEX ] = new SfxUInt32Item( ATTR_CONDITIONAL, 0 );
/*N*/ 
/*N*/ 	//	GetRscString funktioniert erst nach ScGlobal::Init, zu erkennen am EmptyBrushItem
/*N*/ 	//!	zusaetzliche Methode ScGlobal::IsInit() oder so...
/*N*/ 	//!	oder erkennen, ob dies der Secondary-Pool fuer einen MessagePool ist
/*N*/ 	if ( ScGlobal::GetEmptyBrushItem() )
/*N*/ 		ppPoolDefaults[ ATTR_PATTERN	 - ATTR_STARTINDEX ] = new ScPatternAttr( pSet, ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );
/*N*/ 	else
/*N*/ 		ppPoolDefaults[ ATTR_PATTERN	 - ATTR_STARTINDEX ] = new ScPatternAttr( pSet,
/*N*/ 			String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_STANDARD)) );	//! without name?
/*N*/ 
/*N*/ 	ppPoolDefaults[	ATTR_LRSPACE		 - ATTR_STARTINDEX ] = new SvxLRSpaceItem( ATTR_LRSPACE );
/*N*/ 	ppPoolDefaults[	ATTR_ULSPACE		 - ATTR_STARTINDEX ] = new SvxULSpaceItem( ATTR_ULSPACE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE			 - ATTR_STARTINDEX ] = new SvxPageItem( ATTR_PAGE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_PAPERTRAY	 - ATTR_STARTINDEX ] = new SfxAllEnumItem( ATTR_PAGE_PAPERTRAY );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_PAPERBIN	 - ATTR_STARTINDEX ] = new SvxPaperBinItem( ATTR_PAGE_PAPERBIN );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_SIZE		 - ATTR_STARTINDEX ] = new SvxSizeItem( ATTR_PAGE_SIZE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_MAXSIZE	 - ATTR_STARTINDEX ] = new SvxSizeItem( ATTR_PAGE_MAXSIZE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_HORCENTER	 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_HORCENTER );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_VERCENTER	 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_VERCENTER );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_ON		 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_ON, TRUE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_DYNAMIC	 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_DYNAMIC, TRUE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_SHARED	 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_SHARED, TRUE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_NOTES		 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_NOTES, FALSE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_GRID		 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_GRID, FALSE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_HEADERS	 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_HEADERS, FALSE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_CHARTS	 - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_CHARTS );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_OBJECTS	 - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_OBJECTS );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_DRAWINGS	 - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_DRAWINGS );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_TOPDOWN	 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_TOPDOWN, TRUE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_SCALE		 - ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_SCALE, 100 );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_SCALETOPAGES-ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, 1 );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_FIRSTPAGENO- ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, 1 );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_PRINTAREA	 - ATTR_STARTINDEX ] = new ScRangeItem( ATTR_PAGE_PRINTAREA );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_REPEATROW	 - ATTR_STARTINDEX ] = new ScRangeItem( ATTR_PAGE_REPEATROW );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_REPEATCOL	 - ATTR_STARTINDEX ] = new ScRangeItem( ATTR_PAGE_REPEATCOL );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_PRINTTABLES- ATTR_STARTINDEX ] = new ScTableListItem( ATTR_PAGE_PRINTTABLES );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_HEADERLEFT - ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_HEADERLEFT );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_FOOTERLEFT - ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_FOOTERLEFT );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_HEADERRIGHT- ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_HEADERRIGHT );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_FOOTERRIGHT- ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_FOOTERRIGHT );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_HEADERSET	 - ATTR_STARTINDEX ] = new SvxSetItem( ATTR_PAGE_HEADERSET, aSetItemItemSet );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_FOOTERSET	 - ATTR_STARTINDEX ] = new SvxSetItem( ATTR_PAGE_FOOTERSET, aSetItemItemSet );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_FORMULAS	 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_FORMULAS, FALSE );
/*N*/ 	ppPoolDefaults[	ATTR_PAGE_NULLVALS	 - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_NULLVALS, TRUE );
/*N*/ //	ppPoolDefaults[	ATTR_ITEM_DOUBLE	 - ATTR_STARTINDEX ] = new ScDoubleItem( ATTR_ITEM_DOUBLE, 0 );
/*N*/ 
/*N*/ 	SetDefaults( ppPoolDefaults );
/*N*/ 
/*N*/ 	if ( pSecondary )
/*?*/ 		SetSecondaryPool( pSecondary );
/*N*/ 
/*N*/ 	// ATTR_LANGUAGE_FORMAT ab sv329 eingefuegt, VersionMap in _ScGlobal__Init
/*N*/ 	SetVersionMap( 1, 100, 157, pVersionMap1 );
/*N*/ 
/*N*/ 	// ATTR_VALIDDATA, ATTR_CONDITIONAL ab 341
/*N*/ 	SetVersionMap( 2, 100, 158, pVersionMap2 );
/*N*/ 
/*N*/ 	// ATTR_INDENT ab 350
/*N*/ 	SetVersionMap( 3, 100, 160, pVersionMap3 );
/*N*/ 
/*N*/ 	// ATTR_ROTATE_VALUE, ATTR_ROTATE_MODE ab 367
/*N*/ 	SetVersionMap( 4, 100, 161, pVersionMap4 );
/*N*/ 
/*N*/ 	// CJK, CTL, EMPHASISMARK, TWOLINES from 614
/*N*/ 	SetVersionMap( 5, 100, 163, pVersionMap5 );
/*N*/ 
/*N*/ 	// ATTR_SCRIPTSPACE, ATTR_HANGPUNCTUATION, ATTR_FORBIDDEN_RULES from 614d
/*N*/ 	SetVersionMap( 6, 100, 175, pVersionMap6 );
/*N*/ 
/*N*/ 	// ATTR_FONT_WORDLINE, ATTR_FONT_RELIEF, ATTR_HYPHENATE from 632b
/*N*/ 	SetVersionMap( 7, 100, 178, pVersionMap7 );
/*N*/ 
/*N*/ 	// ATTR_VERTICAL_ASIAN from 642q
/*N*/ 	SetVersionMap( 8, 100, 181, pVersionMap8 );

    // ATTR_WRITINGDIR from 643y
/*N*/ 	SetVersionMap( 9, 100, 182, pVersionMap9 );
/*N*/ }

/*N*/ __EXPORT ScDocumentPool::~ScDocumentPool()
/*N*/ {
/*N*/ 	Delete();
/*N*/ 
/*N*/ 	for ( USHORT i=0; i < ATTR_ENDINDEX-ATTR_STARTINDEX+1; i++ )
/*N*/ 	{
/*N*/ 		SetRefCount( *ppPoolDefaults[i], 0 );
/*N*/ 		delete ppPoolDefaults[i];
/*N*/ 	}
/*N*/ 
/*N*/ 	delete[] ppPoolDefaults;
/*N*/ 	delete pSecondary;
/*N*/ }

/*N*/ void ScDocumentPool::InitVersionMaps()
/*N*/ {
/*N*/ 	DBG_ASSERT( !pVersionMap1 && !pVersionMap2 &&
/*N*/ 				!pVersionMap3 && !pVersionMap4 &&
/*N*/ 				!pVersionMap5 && !pVersionMap6 &&
/*N*/ 				!pVersionMap7 && !pVersionMap8 &&
/*N*/ 				!pVersionMap9, "InitVersionMaps call multiple times" );
/*N*/ 
/*N*/ 	// alte WhichId's mappen
/*N*/ 	// nicht mit ATTR_* zaehlen, falls die sich nochmal aendern
/*N*/ 
/*N*/ 	//	erste Map: ATTR_LANGUAGE_FORMAT ab sv329 eingefuegt
/*N*/ 
/*N*/ 	const USHORT nMap1Start	= 100;	// alter ATTR_STARTINDEX
/*N*/ 	const USHORT nMap1End	= 157;	// alter ATTR_ENDINDEX
/*N*/ 	const USHORT nMap1Count = nMap1End - nMap1Start + 1;
/*N*/ 	const USHORT nMap1New	= 18;	// ATTR_LANGUAGE_FORMAT - ATTR_STARTINDEX
/*N*/ 	pVersionMap1 = new USHORT [ nMap1Count ];
/*N*/ 	USHORT i, j;
/*N*/ 	for ( i=0, j=nMap1Start; i < nMap1New; i++, j++ )
/*N*/ 		pVersionMap1[i] = j;
/*N*/ 	// ein Eintrag eingefuegt...
/*N*/ 	for ( i=nMap1New, j=nMap1Start+nMap1New+1; i < nMap1Count; i++, j++ )
/*N*/ 		pVersionMap1[i] = j;
/*N*/ 
/*N*/ 	//	zweite Map: ATTR_VALIDDATA und ATTR_CONDITIONAL ab 341 eingefuegt
/*N*/ 
/*N*/ 	const USHORT nMap2Start	= 100;	// ATTR_STARTINDEX
/*N*/ 	const USHORT nMap2End	= 158;	// ATTR_ENDINDEX
/*N*/ 	const USHORT nMap2Count = nMap2End - nMap2Start + 1;
/*N*/ 	const USHORT nMap2New   = 24;	// ATTR_VALIDDATA - ATTR_STARTINDEX
/*N*/ 	pVersionMap2 = new USHORT [ nMap2Count ];
/*N*/ 	for ( i=0, j=nMap2Start; i < nMap2New; i++, j++ )
/*N*/ 		pVersionMap2[i] = j;
/*N*/ 	// zwei Eintraege eingefuegt...
/*N*/ 	for ( i=nMap2New, j=nMap2Start+nMap2New+2; i < nMap2Count; i++, j++ )
/*N*/ 		pVersionMap2[i] = j;
/*N*/ 
/*N*/ 	//	dritte Map: ATTR_INDENT ab 350 eingefuegt
/*N*/ 
/*N*/ 	const USHORT nMap3Start	= 100;	// ATTR_STARTINDEX
/*N*/ 	const USHORT nMap3End	= 160;	// ATTR_ENDINDEX
/*N*/ 	const USHORT nMap3Count = nMap3End - nMap3Start + 1;
/*N*/ 	const USHORT nMap3New   = 11;	// ATTR_INDENT - ATTR_STARTINDEX
/*N*/ 	pVersionMap3 = new USHORT [ nMap3Count ];
/*N*/ 	for ( i=0, j=nMap3Start; i < nMap3New; i++, j++ )
/*N*/ 		pVersionMap3[i] = j;
/*N*/ 	// ein Eintrag eingefuegt...
/*N*/ 	for ( i=nMap3New, j=nMap3Start+nMap3New+1; i < nMap3Count; i++, j++ )
/*N*/ 		pVersionMap3[i] = j;
/*N*/ 
/*N*/ 	//	vierte Map: ATTR_ROTATE_VALUE und ATTR_ROTATE_MODE ab 367 eingefuegt
/*N*/ 
/*N*/ 	const USHORT nMap4Start	= 100;	// ATTR_STARTINDEX
/*N*/ 	const USHORT nMap4End	= 161;	// ATTR_ENDINDEX
/*N*/ 	const USHORT nMap4Count = nMap4End - nMap4Start + 1;
/*N*/ 	const USHORT nMap4New   = 14;	// ATTR_ROTATE_VALUE - ATTR_STARTINDEX
/*N*/ 	pVersionMap4 = new USHORT [ nMap4Count ];
/*N*/ 	for ( i=0, j=nMap4Start; i < nMap4New; i++, j++ )
/*N*/ 		pVersionMap4[i] = j;
/*N*/ 	// zwei Eintraege eingefuegt...
/*N*/ 	for ( i=nMap4New, j=nMap4Start+nMap4New+2; i < nMap4Count; i++, j++ )
/*N*/ 		pVersionMap4[i] = j;
/*N*/ 
/*N*/ 	//	fifth map: CJK..., CTL..., EMPHASISMARK, TWOLINES (12 items) added in 614
/*N*/ 
/*N*/ 	const USHORT nMap5Start	= 100;	// ATTR_STARTINDEX
/*N*/ 	const USHORT nMap5End	= 163;	// ATTR_ENDINDEX
/*N*/ 	const USHORT nMap5Count = nMap5End - nMap5Start + 1;
/*N*/ 	const USHORT nMap5New   = 10;	// ATTR_CJK_FONT - ATTR_STARTINDEX
/*N*/ 	pVersionMap5 = new USHORT [ nMap5Count ];
/*N*/ 	for ( i=0, j=nMap5Start; i < nMap5New; i++, j++ )
/*N*/ 		pVersionMap5[i] = j;
/*N*/ 	// 12 entries inserted
/*N*/ 	for ( i=nMap5New, j=nMap5Start+nMap5New+12; i < nMap5Count; i++, j++ )
/*N*/ 		pVersionMap5[i] = j;
/*N*/ 
/*N*/ 	// sixth map: ATTR_SCRIPTSPACE, ATTR_HANGPUNCTUATION, ATTR_FORBIDDEN_RULES added in 614d
/*N*/ 
/*N*/ 	const USHORT nMap6Start	= 100;	// ATTR_STARTINDEX
/*N*/ 	const USHORT nMap6End	= 175;	// ATTR_ENDINDEX
/*N*/ 	const USHORT nMap6Count = nMap6End - nMap6Start + 1;
/*N*/ 	const USHORT nMap6New   = 22;	// ATTR_SCRIPTSPACE - ATTR_STARTINDEX
/*N*/ 	pVersionMap6 = new USHORT [ nMap6Count ];
/*N*/ 	for ( i=0, j=nMap6Start; i < nMap6New; i++, j++ )
/*N*/ 		pVersionMap6[i] = j;
/*N*/ 	// 3 entries inserted
/*N*/ 	for ( i=nMap6New, j=nMap6Start+nMap6New+3; i < nMap6Count; i++, j++ )
/*N*/ 		pVersionMap6[i] = j;
/*N*/ 
/*N*/ 	//	seventh map: ATTR_FONT_WORDLINE, ATTR_FONT_RELIEF, ATTR_HYPHENATE added in 632b
/*N*/ 
/*N*/ 	const USHORT nMap7Start	= 100;	// ATTR_STARTINDEX
/*N*/ 	const USHORT nMap7End	= 178;	// ATTR_ENDINDEX
/*N*/ 	const USHORT nMap7Count = nMap7End - nMap7Start + 1;
/*N*/ 	const USHORT nMap7New   = 22;	// ATTR_FONT_WORDLINE - ATTR_STARTINDEX
/*N*/ 	pVersionMap7 = new USHORT [ nMap7Count ];
/*N*/ 	for ( i=0, j=nMap7Start; i < nMap7New; i++, j++ )
/*N*/ 		pVersionMap7[i] = j;
/*N*/ 	// 3 entries inserted
/*N*/ 	for ( i=nMap7New, j=nMap7Start+nMap7New+3; i < nMap7Count; i++, j++ )
/*N*/ 		pVersionMap7[i] = j;
/*N*/ 
/*N*/ 	//	eighth map: ATTR_VERTICAL_ASIAN added in 642q
/*N*/ 
/*N*/ 	const USHORT nMap8Start	= 100;	// ATTR_STARTINDEX
/*N*/ 	const USHORT nMap8End	= 181;	// ATTR_ENDINDEX
/*N*/ 	const USHORT nMap8Count = nMap8End - nMap8Start + 1;
/*N*/ 	const USHORT nMap8New   = 34;	// ATTR_VERTICAL_ASIAN - ATTR_STARTINDEX
/*N*/ 	pVersionMap8 = new USHORT [ nMap8Count ];
/*N*/ 	for ( i=0, j=nMap8Start; i < nMap8New; i++, j++ )
/*N*/ 		pVersionMap8[i] = j;
/*N*/ 	// 1 entry inserted
/*N*/ 	for ( i=nMap8New, j=nMap8Start+nMap8New+1; i < nMap8Count; i++, j++ )
/*N*/ 		pVersionMap8[i] = j;

/*M*/ 	//	9th map: ATTR_WRITINGDIR added in 643y
/*M*/ 
/*M*/ 	const USHORT nMap9Start	= 100;	// ATTR_STARTINDEX
/*M*/ 	const USHORT nMap9End	= 182;	// ATTR_ENDINDEX
/*M*/ 	const USHORT nMap9Count = nMap9End - nMap9Start + 1;
/*M*/ 	const USHORT nMap9New   = 35;	// ATTR_WRITINGDIR - ATTR_STARTINDEX
/*M*/ 	pVersionMap9 = new USHORT [ nMap9Count ];
/*M*/ 	for ( i=0, j=nMap9Start; i < nMap9New; i++, j++ )
/*M*/ 		pVersionMap9[i] = j;
/*M*/ 	// 1 entry inserted
/*M*/ 	for ( i=nMap9New, j=nMap9Start+nMap9New+1; i < nMap9Count; i++, j++ )
/*M*/ 		pVersionMap9[i] = j;
/*N*/ }

/*N*/ void ScDocumentPool::DeleteVersionMaps()
/*N*/ {
/*N*/ 	DBG_ASSERT( pVersionMap1 && pVersionMap2 &&
/*N*/ 				pVersionMap3 && pVersionMap4 &&
/*N*/ 				pVersionMap5 && pVersionMap6 &&
/*M*/ 				pVersionMap7 && pVersionMap8 &&
/*M*/ 				pVersionMap9, "DeleteVersionMaps without maps" );
/*M*/ 
/*M*/ 	delete[] pVersionMap9;
/*M*/ 	pVersionMap9 = 0;
/*N*/ 	delete[] pVersionMap8;
/*N*/ 	pVersionMap8 = 0;
/*N*/ 	delete[] pVersionMap7;
/*N*/ 	pVersionMap7 = 0;
/*N*/ 	delete[] pVersionMap6;
/*N*/ 	pVersionMap6 = 0;
/*N*/ 	delete[] pVersionMap5;
/*N*/ 	pVersionMap5 = 0;
/*N*/ 	delete[] pVersionMap4;
/*N*/ 	pVersionMap4 = 0;
/*N*/ 	delete[] pVersionMap3;
/*N*/ 	pVersionMap3 = 0;
/*N*/ 	delete[] pVersionMap2;
/*N*/ 	pVersionMap2 = 0;
/*N*/ 	delete[] pVersionMap1;
/*N*/ 	pVersionMap1 = 0;
/*N*/ }

// ----------------------------------------------------------------------------------------
//
//	Fuer die Pattern-Attribute (SetItems) kann der USHORT RefCount leicht ueberlaufen
//	(z.B. 600 ganze Zeilen abwechselnd formatieren).
//	Darum wird der RefCount bei SC_MAX_POOLREF festgehalten und nicht mehr hoch- oder
//	heruntergezaehlt. Dieser RefCount wird dann erst beim naechsten Laden neu gezaehlt.
//	Die Differenz zwischen SC_MAX_POOLREF und SC_SAFE_POOLREF ist ein wenig groesser
//	als noetig, um zu erkennen, wenn der RefCount aus Versehen doch "normal" veraendert
//	wird (Assertions).
//

/*N*/ const SfxPoolItem& __EXPORT ScDocumentPool::Put( const SfxPoolItem& rItem, USHORT nWhich )
/*N*/ {
/*N*/ 	if ( rItem.Which() != ATTR_PATTERN )				// nur Pattern ist special
/*N*/ 		return SfxItemPool::Put( rItem, nWhich );
/*N*/ 
/*N*/ 	//	das Default-Pattern dieses Pools nicht kopieren
/*N*/ 	if (&rItem == ppPoolDefaults[ ATTR_PATTERN - ATTR_STARTINDEX ])
/*N*/ 		return rItem;
/*N*/ 
/*N*/ 	//	ansonsten muss Put immer passieren, weil es ein anderer Pool sein kann
/*N*/ 	const SfxPoolItem& rNew = SfxItemPool::Put( rItem, nWhich );
/*N*/ 	CheckRef( rNew );
/*N*/ 	return rNew;
/*N*/ }

/*N*/ void __EXPORT ScDocumentPool::Remove( const SfxPoolItem& rItem )
/*N*/ {
/*N*/ 	if ( rItem.Which() == ATTR_PATTERN )				// nur Pattern ist special
/*N*/ 	{
/*N*/ 		ULONG nRef = rItem.GetRefCount();
/*N*/ 		if ( nRef >= (ULONG) SC_MAX_POOLREF && nRef <= (ULONG) SFX_ITEMS_OLD_MAXREF )
/*N*/ 		{
/*?*/ 			if ( nRef != (ULONG) SC_SAFE_POOLREF )
/*?*/ 			{
/*?*/ 				DBG_ERROR("Wer fummelt da an meinen Ref-Counts herum");
/*?*/ 				SetRefCount( (SfxPoolItem&)rItem, (ULONG) SC_SAFE_POOLREF );
/*?*/ 			}
/*?*/ 			return;					// nicht herunterzaehlen
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SfxItemPool::Remove( rItem );
/*N*/ }

/*N*/ void ScDocumentPool::CheckRef( const SfxPoolItem& rItem )	// static
/*N*/ {
/*N*/ 	ULONG nRef = rItem.GetRefCount();
/*N*/ 	if ( nRef >= (ULONG) SC_MAX_POOLREF && nRef <= (ULONG) SFX_ITEMS_OLD_MAXREF )
/*N*/ 	{
/*?*/ 		// beim Apply vom Cache wird evtl. um 2 hochgezaehlt (auf MAX+1 oder SAFE+2),
/*?*/ 		// heruntergezaehlt wird nur einzeln (in LoadCompleted)
/*?*/ 		DBG_ASSERT( nRef<=(ULONG)SC_MAX_POOLREF+1 || (nRef>=(ULONG)SC_SAFE_POOLREF-1 && nRef<=(ULONG)SC_SAFE_POOLREF+2),
/*?*/ 				"ScDocumentPool::CheckRef" );
/*?*/ 		SetRefCount( (SfxPoolItem&)rItem, (ULONG) SC_SAFE_POOLREF );
/*N*/ 	}
/*N*/ }

/*N*/ void ScDocumentPool::MyLoadCompleted()
/*N*/ {
/*N*/ 	LoadCompleted();
/*N*/ 
/*N*/ 	USHORT nCount = GetItemCount(ATTR_PATTERN);
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pItem = GetItem(ATTR_PATTERN, i);
/*N*/ 		if (pItem)
/*N*/ 			CheckRef(*pItem);
/*N*/ 	}
/*N*/ }

// ----------------------------------------------------------------------------------------










}
