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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#include <hintids.hxx>

#include <tools/globname.hxx>
#include <vcl/mapmod.hxx>

#include <bf_svx/xmlcnitm.hxx>

#include <bf_svtools/macitem.hxx>
#include <bf_svx/pbinitem.hxx>
#include <bf_svx/keepitem.hxx>
#include <bf_svx/nlbkitem.hxx>
#include <bf_svx/hyznitem.hxx>
#include <bf_svx/protitem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/prszitem.hxx>
#include <bf_svx/opaqitem.hxx>
#include <bf_svx/shaditem.hxx>
#include <bf_svx/prntitem.hxx>
#include <bf_svx/brkitem.hxx>
#include <bf_svx/tstpitem.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/wrlmitem.hxx>
#include <bf_svx/kernitem.hxx>
#include <bf_svx/escpitem.hxx>
#include <bf_svx/cscoitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/orphitem.hxx>
#include <bf_svx/widwitem.hxx>
#include <bf_svx/nhypitem.hxx>
#include <bf_svx/spltitem.hxx>
#include <bf_svx/lspcitem.hxx>
#include <bf_svx/blnkitem.hxx>
#include <bf_svx/akrnitem.hxx>
#include <bf_svx/emphitem.hxx>
#include <bf_svx/twolinesitem.hxx>
#include <bf_svx/scriptspaceitem.hxx>
#include <bf_svx/hngpnctitem.hxx>
#include <bf_svx/cmapitem.hxx>
#include <bf_svx/charscaleitem.hxx>
#include <bf_svx/charrotateitem.hxx>
#include <bf_svx/charreliefitem.hxx>
#include <bf_svx/frmdiritem.hxx>
#include <bf_svx/dialogs.hrc>
#include <bf_offmgr/app.hxx>
#include <bf_svx/forbiddenruleitem.hxx>
#include <bf_svx/paravertalignitem.hxx>
#include <bf_svx/pgrditem.hxx>


#include <fmthbsh.hxx>
#include <fmtanchr.hxx>

#include <horiornt.hxx>

#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include <fmtfld.hxx>
#include <fmtrfmrk.hxx>
#include <fmtlsplt.hxx>
#include <fmteiro.hxx>
#include <fmtclds.hxx>
#include <fmturl.hxx>
#include <fmtcntnt.hxx>
#include <fmthdft.hxx>
#include <fmtpdsc.hxx>
#include <fmtftn.hxx>
#include <fmtfordr.hxx>
#include <fmtflcnt.hxx>
#include <fchrfmt.hxx>
#include <fmtinfmt.hxx>
#include <fmtcnct.hxx>
#include <fmtline.hxx>
#include <fmtftntx.hxx>
#include <fmtruby.hxx>
#include <tgrditem.hxx>
#include <hfspacingitem.hxx>
#include <editsh.hxx>
#include <init.hxx>
#include <paratr.hxx>
#include <grfatr.hxx>
#include <tox.hxx>
#include <cellatr.hxx>
#include <tblafmt.hxx>
#include <viscrs.hxx>
#include <fntcache.hxx>
#include <doc.hxx>
#include <acmplwrd.hxx>
#include <fmtclbl.hxx>
#include <cmdid.h>
#include <breakit.hxx>
#include <checkit.hxx>

#include <swcalwrp.hxx>
#include <SwStyleNameMapper.hxx>
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
namespace binfilter {

extern void _FrmFinit();
extern void ClearFEShellTabCols();

/*************************************************************************
|*	einige Bereiche fuer die Set in Collections / Nodes
|*************************************************************************/
    // AttrSet-Range fuer die 2 Break-Attribute
USHORT __FAR_DATA aBreakSetRange[] = {
    RES_PAGEDESC, RES_BREAK,
    0 };

    // AttrSet-Range fuer die TxtFmtColl
USHORT __FAR_DATA aTxtFmtCollSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

    // AttrSet-Range fuer die GrfFmtColl
USHORT __FAR_DATA aGrfFmtCollSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_GRFATR_BEGIN, RES_GRFATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

    // AttrSet-Range fuer die TextNode
USHORT __FAR_DATA aTxtNodeSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

    // AttrSet-Range fuer die NoTxtNode
USHORT __FAR_DATA aNoTxtNodeSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_GRFATR_BEGIN, RES_GRFATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

USHORT __FAR_DATA aTableSetRange[] = {
    RES_FILL_ORDER, 	RES_FRM_SIZE,
    RES_LR_SPACE, 		RES_BREAK,
    RES_BACKGROUND, 	RES_SHADOW,
    RES_HORI_ORIENT,	RES_HORI_ORIENT,
    RES_KEEP,			RES_KEEP,
    RES_LAYOUT_SPLIT,	RES_LAYOUT_SPLIT,
    RES_FRAMEDIR,       RES_FRAMEDIR,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

USHORT __FAR_DATA aTableLineSetRange[] = {
    RES_FILL_ORDER, 	RES_FRM_SIZE,
    RES_LR_SPACE, 		RES_UL_SPACE,
    RES_BACKGROUND, 	RES_SHADOW,
    RES_PROTECT, 		RES_PROTECT,
    RES_VERT_ORIENT,	RES_VERT_ORIENT,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

USHORT __FAR_DATA aTableBoxSetRange[] = {
    RES_FILL_ORDER, 	RES_FRM_SIZE,
    RES_LR_SPACE, 		RES_UL_SPACE,
    RES_BACKGROUND, 	RES_SHADOW,
    RES_PROTECT, 		RES_PROTECT,
    RES_VERT_ORIENT,	RES_VERT_ORIENT,
    RES_BOXATR_BEGIN,	RES_BOXATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet-Range fuer die SwFrmFmt
USHORT __FAR_DATA aFrmFmtSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet-Range fuer die SwCharFmt
USHORT __FAR_DATA aCharFmtSetRange[] = {
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet-Range fuer die SwPageDescFmt
USHORT __FAR_DATA aPgFrmFmtSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

/******************************************************************************
 * lege eine Tabelle fuer einen Zugriff auf die
 * Default-Format-Attribute an
 ******************************************************************************/
SwDfltAttrTab __FAR_DATA aAttrTab;

SfxItemInfo __FAR_DATA aSlotTab[] =
{
    { SID_ATTR_CHAR_CASEMAP, SFX_ITEM_POOLABLE },   	// RES_CHRATR_CASEMAP
    { SID_ATTR_CHAR_CHARSETCOLOR, SFX_ITEM_POOLABLE },	// RES_CHRATR_CHARSETCOLOR
    { SID_ATTR_CHAR_COLOR, SFX_ITEM_POOLABLE },     	// RES_CHRATR_COLOR
    { SID_ATTR_CHAR_CONTOUR, SFX_ITEM_POOLABLE },    	// RES_CHRATR_CONTOUR
    { SID_ATTR_CHAR_STRIKEOUT, SFX_ITEM_POOLABLE },  	// RES_CHRATR_CROSSEDOUT
    { SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEM_POOLABLE }, 	// RES_CHRATR_ESCAPEMENT
    { SID_ATTR_CHAR_FONT, SFX_ITEM_POOLABLE },     		// RES_CHRATR_FONT
    { SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEM_POOLABLE }, 	// RES_CHRATR_FONTSIZE
    { SID_ATTR_CHAR_KERNING, SFX_ITEM_POOLABLE },    	// RES_CHRATR_KERNING
    { SID_ATTR_CHAR_LANGUAGE, SFX_ITEM_POOLABLE },   	// RES_CHRATR_LANGUAGE
    { SID_ATTR_CHAR_POSTURE, SFX_ITEM_POOLABLE },    	// RES_CHRATR_POSTURE
    { SID_ATTR_CHAR_PROPSIZE, SFX_ITEM_POOLABLE },   	// RES_CHRATR_PROPORTIONALFONTSIZE
    { SID_ATTR_CHAR_SHADOWED, SFX_ITEM_POOLABLE },   	// RES_CHRATR_SHADOWED
    { SID_ATTR_CHAR_UNDERLINE, SFX_ITEM_POOLABLE },  	// RES_CHRATR_UNDERLINE
    { SID_ATTR_CHAR_WEIGHT, SFX_ITEM_POOLABLE },     	// RES_CHRATR_WEIGHT
    { SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEM_POOLABLE },	// RES_CHRATR_WORDLINEMODE
    { SID_ATTR_CHAR_AUTOKERN, SFX_ITEM_POOLABLE },	   	// RES_CHRATR_AUTOKERN
    { SID_ATTR_FLASH, SFX_ITEM_POOLABLE },				// RES_CHRATR_BLINK
    { 0, SFX_ITEM_POOLABLE },							// RES_CHRATR_NOLINEBREAK
    { 0, SFX_ITEM_POOLABLE },							// RES_CHRATR_NOHYPHEN
    { SID_ATTR_BRUSH_CHAR, SFX_ITEM_POOLABLE },			// RES_CHRATR_BACKGROUND
    { SID_ATTR_CHAR_CJK_FONT, SFX_ITEM_POOLABLE },		// RES_CHRATR_CJK_FONT
    { SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEM_POOLABLE },// RES_CHRATR_CJK_FONTSIZE
    { SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEM_POOLABLE },	// RES_CHRATR_CJK_LANGUAGE
    { SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEM_POOLABLE },	// RES_CHRATR_CJK_POSTURE
    { SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEM_POOLABLE },	// RES_CHRATR_CJK_WEIGHT
    { SID_ATTR_CHAR_CTL_FONT, SFX_ITEM_POOLABLE },		// RES_CHRATR_CTL_FONT
    { SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEM_POOLABLE },// RES_CHRATR_CTL_FONTSIZE
    { SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEM_POOLABLE },	// RES_CHRATR_CTL_LANGUAGE
    { SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEM_POOLABLE },	// RES_CHRATR_CTL_POSTURE
    { SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEM_POOLABLE },	// RES_CHRATR_CTL_WEIGHT
    { SID_ATTR_CHAR_ROTATED, SFX_ITEM_POOLABLE },		// RES_CHRATR_ROTATE
    { SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEM_POOLABLE },	// RES_CHRATR_EMPHASIS_MARK
    { SID_ATTR_CHAR_TWO_LINES, SFX_ITEM_POOLABLE },		// RES_CHRATR_TWO_LINES
    { SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEM_POOLABLE },	// RES_CHRATR_SCALEW
    { SID_ATTR_CHAR_RELIEF, SFX_ITEM_POOLABLE },		// RES_CHRATR_RELIEF
    { 0, SFX_ITEM_POOLABLE },							// RES_CHRATR_DUMMY1

    { FN_TXTATR_INET, 0 }, 								// RES_TXTATR_INETFMT
    { 0, SFX_ITEM_POOLABLE },					 		// RES_TXTATR_DUMMY4
    { 0, 0 }, 											// RES_TXTATR_REFMARK
    { 0, 0 },											// RES_TXTATR_TOXMARK
    { 0, 0 },											// RES_TXTATR_CHARFMT
    { 0, SFX_ITEM_POOLABLE },					 		// RES_TXTATR_DUMMY5,
    { SID_ATTR_CHAR_CJK_RUBY, 0 },					 	// RES_TXTATR_CJK_RUBY,
    { 0, SFX_ITEM_POOLABLE },					 		// RES_TXTATR_UNKNOWN_CONTAINER,
    { 0, SFX_ITEM_POOLABLE },					 		// RES_TXTATR_DUMMY6,
    { 0, SFX_ITEM_POOLABLE },					 		// RES_TXTATR_DUMMY7,

    { 0, 0 },											// RES_TXTATR_FIELD
    { 0, 0 },											// RES_TXTATR_FLYCNT
    { 0, 0 },											// RES_TXTATR_FTN
    { 0, SFX_ITEM_POOLABLE },							// RES_TXTATR_SOFTHYPH
    { 0, SFX_ITEM_POOLABLE },							// RES_TXTATR_HARDBLANK
    { 0, SFX_ITEM_POOLABLE },							// RES_TXTATR_DUMMY1
    { 0, SFX_ITEM_POOLABLE },							// RES_TXTATR_DUMMY2

    { SID_ATTR_PARA_LINESPACE, SFX_ITEM_POOLABLE }, 	// RES_PARATR_LINESPACING
    { SID_ATTR_PARA_ADJUST, SFX_ITEM_POOLABLE }, 		// RES_PARATR_ADJUST
    { SID_ATTR_PARA_SPLIT, SFX_ITEM_POOLABLE }, 		// RES_PARATR_SPLIT
    { SID_ATTR_PARA_ORPHANS, SFX_ITEM_POOLABLE },  		// RES_PARATR_ORPHANS
    { SID_ATTR_PARA_WIDOWS, SFX_ITEM_POOLABLE }, 		// RES_PARATR_WIDOWS
    { SID_ATTR_TABSTOP, SFX_ITEM_POOLABLE }, 			// RES_PARATR_TABSTOP
    { SID_ATTR_PARA_HYPHENZONE, SFX_ITEM_POOLABLE }, 	// RES_PARATR_HYPHENZONE
    { FN_FORMAT_DROPCAPS, 0 },							// RES_PARATR_DROP
    { SID_ATTR_PARA_REGISTER, SFX_ITEM_POOLABLE },		// RES_PARATR_REGISTER
    { SID_ATTR_PARA_NUMRULE, 0 },						// RES_PARATR_NUMRULE
    { SID_ATTR_PARA_SCRIPTSPACE, SFX_ITEM_POOLABLE },	// RES_PARATR_SCRIPTSPACE
    { SID_ATTR_PARA_HANGPUNCTUATION, SFX_ITEM_POOLABLE },// RES_PARATR_HANGINGPUNCTUATION

    { SID_ATTR_PARA_FORBIDDEN_RULES, SFX_ITEM_POOLABLE },// RES_PARATR_FORBIDDEN_RULES
    { SID_PARA_VERTALIGN, SFX_ITEM_POOLABLE },          // RES_PARATR_VERTALIGN
    { SID_ATTR_PARA_SNAPTOGRID, SFX_ITEM_POOLABLE },    // RES_PARATR_SNAPTOGRID
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_CONNECT_BORDER
    { 0, SFX_ITEM_POOLABLE },							// RES_PARATR_DUMMY5
    { 0, SFX_ITEM_POOLABLE },							// RES_PARATR_DUMMY6
    { 0, SFX_ITEM_POOLABLE },							// RES_PARATR_DUMMY7
    { 0, SFX_ITEM_POOLABLE },							// RES_PARATR_DUMMY8

    { 0, SFX_ITEM_POOLABLE },							// RES_FILL_ORDER
    { 0, SFX_ITEM_POOLABLE }, 							// RES_FRM_SIZE
    { SID_ATTR_PAGE_PAPERBIN, SFX_ITEM_POOLABLE }, 		// RES_PAPER_BIN
    { SID_ATTR_LRSPACE, SFX_ITEM_POOLABLE }, 			// RES_LR_SPACE
    { SID_ATTR_ULSPACE, SFX_ITEM_POOLABLE }, 			// RES_UL_SPACE
    { 0, 0 },											// RES_PAGEDESC
    { SID_ATTR_PARA_PAGEBREAK, SFX_ITEM_POOLABLE }, 	// RES_BREAK
    { 0, 0 },											// RES_CNTNT
    { 0, SFX_ITEM_POOLABLE },							// RES_HEADER
    { 0, SFX_ITEM_POOLABLE },							// RES_FOOTER
    { 0, SFX_ITEM_POOLABLE },							// RES_PRINT
    { FN_OPAQUE, SFX_ITEM_POOLABLE },					// RES_OPAQUE
    { FN_SET_PROTECT, SFX_ITEM_POOLABLE },				// RES_PROTECT
    { FN_SURROUND, SFX_ITEM_POOLABLE },					// RES_SURROUND
    { FN_VERT_ORIENT, SFX_ITEM_POOLABLE },				// RES_VERT_ORIENT
    { FN_HORI_ORIENT, SFX_ITEM_POOLABLE },				// RES_HORI_ORIENT
    { 0, 0 },											// RES_ANCHOR
    { SID_ATTR_BRUSH, SFX_ITEM_POOLABLE },		 		// RES_BACKGROUND
    { SID_ATTR_BORDER_OUTER, SFX_ITEM_POOLABLE }, 		// RES_BOX
    { SID_ATTR_BORDER_SHADOW, SFX_ITEM_POOLABLE }, 		// RES_SHADOW
    { SID_ATTR_MACROITEM, SFX_ITEM_POOLABLE },			// RES_FRMMACRO
    { FN_ATTR_COLUMNS, SFX_ITEM_POOLABLE },				// RES_COL
    { SID_ATTR_PARA_KEEP, SFX_ITEM_POOLABLE },			// RES_KEEP
    { 0, SFX_ITEM_POOLABLE },							// RES_URL
    { 0, SFX_ITEM_POOLABLE },							// RES_EDIT_IN_READONLY

    { 0, SFX_ITEM_POOLABLE },							// RES_LAYOUT_SPLIT
    { 0, 0 },											// RES_CHAIN
    { 0, SFX_ITEM_POOLABLE },                           // RES_TEXTGRID
    { FN_FORMAT_LINENUMBER, SFX_ITEM_POOLABLE },        // RES_LINENUMBER
    { 0, SFX_ITEM_POOLABLE },							// RES_FTN_AT_TXTEND
    { 0, SFX_ITEM_POOLABLE },							// RES_END_AT_TXTEND
    { 0, SFX_ITEM_POOLABLE },							// RES_COLUMNBALANCE

#ifdef VERTICAL_LAYOUT
    { SID_ATTR_FRAMEDIRECTION, SFX_ITEM_POOLABLE },		// RES_FRAMEDIR
#else
    { 0, SFX_ITEM_POOLABLE },							// RES_FRAMEDIR
#endif

    { SID_ATTR_HDFT_DYNAMIC_SPACING, SFX_ITEM_POOLABLE }, // RES_HEADER_FOOTER_EAT_SPACING
    { 0, SFX_ITEM_POOLABLE },							// RES_FRMATR_DUMMY9

    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_MIRRORGRF
    { SID_ATTR_GRAF_CROP, SFX_ITEM_POOLABLE }, 			// RES_GRFATR_CROPGRF
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_ROTATION,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_LUMINANCE,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_CONTRAST,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_CHANNELR,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_CHANNELG,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_CHANNELB,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_GAMMA,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_INVERT,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_TRANSPARENCY,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_DUMMY1,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_DUMMY2,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_DUMMY3,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_DUMMY4,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_DUMMY5,
    { 0, SFX_ITEM_POOLABLE },							// RES_GRFATR_DUMMY6,

    { 0, SFX_ITEM_POOLABLE },							// RES_BOXATR_FORMAT
    { 0, 0 },											// RES_BOXATR_FORMULA,
    { 0, SFX_ITEM_POOLABLE },							// RES_BOXATR_VALUE

    { 0, SFX_ITEM_POOLABLE }							// RES_UNKNOWNATR_CONTAINER
};


USHORT* SwAttrPool::pVersionMap1 = 0;
USHORT* SwAttrPool::pVersionMap2 = 0;
USHORT* SwAttrPool::pVersionMap3 = 0;
USHORT* SwAttrPool::pVersionMap4 = 0;
SwIndexReg* SwIndexReg::pEmptyIndexArray = 0;

const sal_Char* __FAR_DATA pMarkToTable		= "table";
const sal_Char* __FAR_DATA pMarkToFrame		= "frame";
const sal_Char* __FAR_DATA pMarkToRegion	= "region";
const sal_Char* __FAR_DATA pMarkToText		= "text";
const sal_Char* __FAR_DATA pMarkToOutline	= "outline";
const sal_Char* __FAR_DATA pMarkToGraphic	= "graphic";
const sal_Char* __FAR_DATA pMarkToOLE		= "ole";

SvPtrarr *pGlobalOLEExcludeList = 0;

SwBreakIt* pBreakIt = 0;
SwCheckIt* pCheckIt = 0;
CharClass* pAppCharClass = 0;
SwCalendarWrapper* pCalendarWrapper = 0;
CollatorWrapper* pCollator = 0, *pCaseCollator = 0;
::utl::TransliterationWrapper* pTransWrp = 0;

/******************************************************************************
 *	void _InitCore()
 ******************************************************************************/
class SwDontWrite : public SfxBoolItem
{
public:
    SwDontWrite( USHORT nId ) : SfxBoolItem( nId ) {}

};





/*N*/ void _InitCore()
/*N*/ {
/*N*/ 	SfxPoolItem* pItem;
/*N*/ 
/*N*/ 	// erstmal alle Attribut-Pointer auf 0 setzen
/*N*/ 	memset( aAttrTab, 0, (POOLATTR_END - POOLATTR_BEGIN) *
/*N*/ 							sizeof( SfxPoolItem* ) );
/*N*/ 
/*N*/ 	aAttrTab[ RES_CHRATR_CASEMAP- POOLATTR_BEGIN ] = new SvxCaseMapItem;
/*N*/ 	aAttrTab[ RES_CHRATR_CHARSETCOLOR- POOLATTR_BEGIN ] = new SvxCharSetColorItem;
/*N*/ 	aAttrTab[ RES_CHRATR_COLOR- POOLATTR_BEGIN ] = new SvxColorItem;
/*N*/ 	aAttrTab[ RES_CHRATR_CONTOUR- POOLATTR_BEGIN ] = new SvxContourItem;
/*N*/ 	aAttrTab[ RES_CHRATR_CROSSEDOUT- POOLATTR_BEGIN ] = new SvxCrossedOutItem;
/*N*/ 	aAttrTab[ RES_CHRATR_ESCAPEMENT- POOLATTR_BEGIN ] = new SvxEscapementItem;
/*N*/ 	aAttrTab[ RES_CHRATR_FONT- POOLATTR_BEGIN ] =
/*N*/ 										new SvxFontItem( RES_CHRATR_FONT );
/*N*/ 
/*N*/ 	aAttrTab[ RES_CHRATR_FONTSIZE- POOLATTR_BEGIN ] = new SvxFontHeightItem;
/*N*/ 	aAttrTab[ RES_CHRATR_KERNING- POOLATTR_BEGIN ] = new SvxKerningItem;
/*N*/ 	aAttrTab[ RES_CHRATR_LANGUAGE- POOLATTR_BEGIN ] = new SvxLanguageItem(LANGUAGE_DONTKNOW);
/*N*/ 	aAttrTab[ RES_CHRATR_POSTURE- POOLATTR_BEGIN ] = new SvxPostureItem;
/*N*/ 	aAttrTab[ RES_CHRATR_PROPORTIONALFONTSIZE- POOLATTR_BEGIN ] = new SvxPropSizeItem;
/*N*/ 	aAttrTab[ RES_CHRATR_SHADOWED- POOLATTR_BEGIN ] = new SvxShadowedItem;
/*N*/ 	aAttrTab[ RES_CHRATR_UNDERLINE- POOLATTR_BEGIN ] = new SvxUnderlineItem;
/*N*/ 	aAttrTab[ RES_CHRATR_WEIGHT- POOLATTR_BEGIN ] = new SvxWeightItem;
/*N*/ 	aAttrTab[ RES_CHRATR_WORDLINEMODE- POOLATTR_BEGIN ] = new SvxWordLineModeItem;
/*N*/ 	aAttrTab[ RES_CHRATR_AUTOKERN- POOLATTR_BEGIN ] = new SvxAutoKernItem;
/*N*/ 	aAttrTab[ RES_CHRATR_BLINK - POOLATTR_BEGIN ]
/*N*/ 				= new SvxBlinkItem( FALSE, RES_CHRATR_BLINK );
/*N*/ 	aAttrTab[ RES_CHRATR_NOHYPHEN - POOLATTR_BEGIN ]
/*N*/ 				= new SvxNoHyphenItem( TRUE, RES_CHRATR_NOHYPHEN );
/*N*/ 	aAttrTab[ RES_CHRATR_NOLINEBREAK- POOLATTR_BEGIN ]
/*N*/ 				= new SvxNoLinebreakItem( TRUE, RES_CHRATR_NOLINEBREAK );
/*N*/ 	aAttrTab[ RES_CHRATR_BACKGROUND - POOLATTR_BEGIN ]
/*N*/ 				= new SvxBrushItem( RES_CHRATR_BACKGROUND );
/*N*/ 
/*N*/ 	// CJK-Attributes
/*N*/ 	aAttrTab[ RES_CHRATR_CJK_FONT - POOLATTR_BEGIN ] =
/*N*/ 									new SvxFontItem( RES_CHRATR_CJK_FONT );
/*N*/ 
/*N*/ 	pItem = new SvxFontHeightItem;
/*N*/ 	pItem->SetWhich( RES_CHRATR_CJK_FONTSIZE );
/*N*/ 	aAttrTab[ RES_CHRATR_CJK_FONTSIZE - POOLATTR_BEGIN ] = pItem;
/*N*/ 
/*N*/ 	pItem = new SvxLanguageItem(LANGUAGE_DONTKNOW);
/*N*/ 	pItem->SetWhich( RES_CHRATR_CJK_LANGUAGE );
/*N*/ 	aAttrTab[ RES_CHRATR_CJK_LANGUAGE - POOLATTR_BEGIN ] = pItem;
/*N*/ 
/*N*/ 	pItem = new SvxPostureItem;
/*N*/ 	pItem->SetWhich( RES_CHRATR_CJK_POSTURE );
/*N*/ 	aAttrTab[ RES_CHRATR_CJK_POSTURE - POOLATTR_BEGIN ] = pItem;
/*N*/ 
/*N*/ 	pItem = new SvxWeightItem;
/*N*/ 	pItem->SetWhich( RES_CHRATR_CJK_WEIGHT );
/*N*/ 	aAttrTab[ RES_CHRATR_CJK_WEIGHT - POOLATTR_BEGIN ] = pItem;
/*N*/ 
/*N*/ 	// CTL-Attributes
/*N*/ 	aAttrTab[ RES_CHRATR_CTL_FONT - POOLATTR_BEGIN ] =
/*N*/ 									new SvxFontItem( RES_CHRATR_CTL_FONT );
/*N*/ 
/*N*/ 	pItem = new SvxFontHeightItem;
/*N*/ 	pItem->SetWhich( RES_CHRATR_CTL_FONTSIZE );
/*N*/ 	aAttrTab[ RES_CHRATR_CTL_FONTSIZE - POOLATTR_BEGIN ] = pItem;
/*N*/ 
/*N*/ 	pItem = new SvxLanguageItem(LANGUAGE_DONTKNOW);
/*N*/ 	pItem->SetWhich( RES_CHRATR_CTL_LANGUAGE );
/*N*/ 	aAttrTab[ RES_CHRATR_CTL_LANGUAGE - POOLATTR_BEGIN ] = pItem;
/*N*/ 
/*N*/ 	pItem = new SvxPostureItem;
/*N*/ 	pItem->SetWhich( RES_CHRATR_CTL_POSTURE );
/*N*/ 	aAttrTab[ RES_CHRATR_CTL_POSTURE - POOLATTR_BEGIN ] = pItem;
/*N*/ 
/*N*/ 	pItem = new SvxWeightItem;
/*N*/ 	pItem->SetWhich( RES_CHRATR_CTL_WEIGHT );
/*N*/ 	aAttrTab[ RES_CHRATR_CTL_WEIGHT - POOLATTR_BEGIN ] = pItem;
/*N*/ 
/*N*/ 	aAttrTab[ RES_CHRATR_ROTATE - POOLATTR_BEGIN ] = new SvxCharRotateItem;
/*N*/ 	aAttrTab[ RES_CHRATR_EMPHASIS_MARK - POOLATTR_BEGIN ] =
/*N*/ 				new SvxEmphasisMarkItem;
/*N*/ 	aAttrTab[ RES_CHRATR_TWO_LINES - POOLATTR_BEGIN ] = new SvxTwoLinesItem( FALSE );
/*N*/ 	aAttrTab[ RES_CHRATR_SCALEW - POOLATTR_BEGIN ] = new SvxCharScaleWidthItem;
/*N*/ 	aAttrTab[ RES_CHRATR_RELIEF - POOLATTR_BEGIN ] = new SvxCharReliefItem;
/*N*/ 
/*N*/ // CharakterAttr - Dummies
/*N*/ 	aAttrTab[ RES_CHRATR_DUMMY1 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_CHRATR_DUMMY1 );
/*N*/ // CharakterAttr - Dummies
/*N*/ 
/*N*/ // TextAttr Ende - Dummies
/*N*/ 	aAttrTab[ RES_TXTATR_DUMMY4 - POOLATTR_BEGIN ]
/*N*/ 				= new SfxBoolItem( RES_TXTATR_DUMMY4 );
/*N*/ // TextAttr Ende - Dummies
/*N*/ 
/*N*/ 	aAttrTab[ RES_TXTATR_INETFMT - POOLATTR_BEGIN ]
/*N*/ 				= new SwFmtINetFmt( aEmptyStr, aEmptyStr );
/*N*/ 	aAttrTab[ RES_TXTATR_REFMARK - POOLATTR_BEGIN ] = new SwFmtRefMark( aEmptyStr );
/*N*/ 	aAttrTab[ RES_TXTATR_TOXMARK - POOLATTR_BEGIN ] = new SwTOXMark;
/*N*/ 	aAttrTab[ RES_TXTATR_CHARFMT- POOLATTR_BEGIN ] = new SwFmtCharFmt( 0 );
/*N*/ 
/*N*/ 	aAttrTab[ RES_TXTATR_CJK_RUBY - POOLATTR_BEGIN ] = new SwFmtRuby( aEmptyStr );
/*N*/ 	aAttrTab[ RES_TXTATR_UNKNOWN_CONTAINER - POOLATTR_BEGIN ] =
/*N*/ 				new SvXMLAttrContainerItem( RES_TXTATR_UNKNOWN_CONTAINER );
/*N*/ 
/*N*/ 	aAttrTab[ RES_TXTATR_FIELD- POOLATTR_BEGIN ] = new SwFmtFld;
/*N*/ 	aAttrTab[ RES_TXTATR_FLYCNT - POOLATTR_BEGIN ] = new SwFmtFlyCnt( 0 );
/*N*/ 	aAttrTab[ RES_TXTATR_FTN - POOLATTR_BEGIN ] = new SwFmtFtn;
/*N*/ 	aAttrTab[ RES_TXTATR_SOFTHYPH- POOLATTR_BEGIN ] = new SwFmtSoftHyph;
/*N*/ 	aAttrTab[ RES_TXTATR_HARDBLANK- POOLATTR_BEGIN ] = new SwFmtHardBlank( ' ', FALSE );
/*N*/ 
/*N*/ // TextAttr ohne Ende - Dummies
/*N*/ 	aAttrTab[ RES_TXTATR_DUMMY1 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_TXTATR_DUMMY1 );
/*N*/ 	aAttrTab[ RES_TXTATR_DUMMY2 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_TXTATR_DUMMY2 );
/*N*/ 	aAttrTab[ RES_TXTATR_DUMMY5 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_TXTATR_DUMMY5 );
/*N*/ 	aAttrTab[ RES_TXTATR_DUMMY6 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_TXTATR_DUMMY6 );
/*N*/ 	aAttrTab[ RES_TXTATR_DUMMY7 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_TXTATR_DUMMY7 );
/*N*/ // TextAttr ohne Ende - Dummies
/*N*/ 
/*N*/ 	aAttrTab[ RES_PARATR_LINESPACING- POOLATTR_BEGIN ] = new SvxLineSpacingItem;
/*N*/ 	aAttrTab[ RES_PARATR_ADJUST- POOLATTR_BEGIN ] = new SvxAdjustItem;
/*N*/ 	aAttrTab[ RES_PARATR_SPLIT- POOLATTR_BEGIN ] = new SvxFmtSplitItem;
/*N*/ 	aAttrTab[ RES_PARATR_WIDOWS- POOLATTR_BEGIN ] = new SvxWidowsItem;
/*N*/ 	aAttrTab[ RES_PARATR_ORPHANS- POOLATTR_BEGIN ] = new SvxOrphansItem;
/*N*/ 	aAttrTab[ RES_PARATR_TABSTOP- POOLATTR_BEGIN ] = new
/*N*/ 							SvxTabStopItem( 1, SVX_TAB_DEFDIST );
/*N*/ 
/*N*/ 	pItem = new SvxHyphenZoneItem;
/*N*/ 	((SvxHyphenZoneItem*)pItem)->GetMaxHyphens() = 0; // Default z.Z. auf 0
/*N*/ 	aAttrTab[ RES_PARATR_HYPHENZONE- POOLATTR_BEGIN ] = pItem;
/*N*/ 
/*N*/ 	aAttrTab[ RES_PARATR_DROP- POOLATTR_BEGIN ] = new SwFmtDrop;
/*N*/ 	aAttrTab[ RES_PARATR_REGISTER - POOLATTR_BEGIN ] = new SwRegisterItem( FALSE );
/*N*/ 	aAttrTab[ RES_PARATR_NUMRULE - POOLATTR_BEGIN ] = new SwNumRuleItem( aEmptyStr );
/*N*/ 
/*N*/ 	aAttrTab[ RES_PARATR_SCRIPTSPACE - POOLATTR_BEGIN ] =
/*N*/ 										new SvxScriptSpaceItem( TRUE );
/*N*/ 	aAttrTab[ RES_PARATR_HANGINGPUNCTUATION - POOLATTR_BEGIN ] =
/*N*/ 										new SvxHangingPunctuationItem( TRUE );
/*N*/ 	aAttrTab[ RES_PARATR_FORBIDDEN_RULES - POOLATTR_BEGIN ] =
/*N*/ 										new SvxForbiddenRuleItem( TRUE );
/*N*/ 	aAttrTab[ RES_PARATR_VERTALIGN - POOLATTR_BEGIN ] =
/*N*/ 							new SvxParaVertAlignItem( 0 );
/*N*/     aAttrTab[ RES_PARATR_SNAPTOGRID - POOLATTR_BEGIN ] = new SvxParaGridItem;
/*N*/     aAttrTab[ RES_PARATR_CONNECT_BORDER - POOLATTR_BEGIN ] = new SwParaConnectBorderItem;
/*N*/ // ParaAttr - Dummies
/*N*/ 	aAttrTab[ RES_PARATR_DUMMY5 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_DUMMY5 );
/*N*/ 	aAttrTab[ RES_PARATR_DUMMY6 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_DUMMY6 );
/*N*/ 	aAttrTab[ RES_PARATR_DUMMY7 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_DUMMY7 );
/*N*/ 	aAttrTab[ RES_PARATR_DUMMY8 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_DUMMY8 );
/*N*/ // ParatAttr - Dummies
/*N*/ 
/*N*/ 	aAttrTab[ RES_FILL_ORDER- POOLATTR_BEGIN ] = new SwFmtFillOrder;
/*N*/ 	aAttrTab[ RES_FRM_SIZE- POOLATTR_BEGIN ] = new SwFmtFrmSize;
/*N*/ 	aAttrTab[ RES_PAPER_BIN- POOLATTR_BEGIN ] = new SvxPaperBinItem;
/*N*/ 	aAttrTab[ RES_LR_SPACE- POOLATTR_BEGIN ] = new SvxLRSpaceItem;
/*N*/ 	aAttrTab[ RES_UL_SPACE- POOLATTR_BEGIN ] = new SvxULSpaceItem;
/*N*/ 	aAttrTab[ RES_PAGEDESC- POOLATTR_BEGIN ] = new SwFmtPageDesc;
/*N*/ 	aAttrTab[ RES_BREAK- POOLATTR_BEGIN ] = new SvxFmtBreakItem;
/*N*/ 	aAttrTab[ RES_CNTNT- POOLATTR_BEGIN ] = new SwFmtCntnt;
/*N*/ 	aAttrTab[ RES_HEADER- POOLATTR_BEGIN ] = new SwFmtHeader;
/*N*/ 	aAttrTab[ RES_FOOTER- POOLATTR_BEGIN ] = new SwFmtFooter;
/*N*/ 	aAttrTab[ RES_PRINT- POOLATTR_BEGIN ] = new SvxPrintItem;
/*N*/ 	aAttrTab[ RES_OPAQUE- POOLATTR_BEGIN ] = new SvxOpaqueItem;
/*N*/ 	aAttrTab[ RES_PROTECT- POOLATTR_BEGIN ] = new SvxProtectItem;
/*N*/ 	aAttrTab[ RES_SURROUND- POOLATTR_BEGIN ] = new SwFmtSurround;
/*N*/ 	aAttrTab[ RES_VERT_ORIENT- POOLATTR_BEGIN ] = new SwFmtVertOrient;
/*N*/ 	aAttrTab[ RES_HORI_ORIENT- POOLATTR_BEGIN ] = new SwFmtHoriOrient;
/*N*/ 	aAttrTab[ RES_ANCHOR- POOLATTR_BEGIN ] = new SwFmtAnchor;
/*N*/ 	aAttrTab[ RES_BACKGROUND- POOLATTR_BEGIN ] = new SvxBrushItem;
/*N*/ 	aAttrTab[ RES_BOX- POOLATTR_BEGIN ] = new SvxBoxItem;
/*N*/ 	aAttrTab[ RES_SHADOW- POOLATTR_BEGIN ] = new SvxShadowItem;
/*N*/ 	aAttrTab[ RES_FRMMACRO- POOLATTR_BEGIN ] = new SvxMacroItem(RES_FRMMACRO);
/*N*/ 	aAttrTab[ RES_COL- POOLATTR_BEGIN ] = new SwFmtCol;
/*N*/ 	aAttrTab[ RES_KEEP - POOLATTR_BEGIN ] = new SvxFmtKeepItem( FALSE );
/*N*/ 	aAttrTab[ RES_URL - POOLATTR_BEGIN ] = new SwFmtURL();
/*N*/ 	aAttrTab[ RES_EDIT_IN_READONLY - POOLATTR_BEGIN ] = new SwFmtEditInReadonly;
/*N*/ 	aAttrTab[ RES_LAYOUT_SPLIT - POOLATTR_BEGIN ] = new SwFmtLayoutSplit;
/*N*/ 	aAttrTab[ RES_CHAIN - POOLATTR_BEGIN ] = new SwFmtChain;
/*N*/   aAttrTab[ RES_TEXTGRID - POOLATTR_BEGIN ] = new SwTextGridItem;
/*M*/   aAttrTab[ RES_HEADER_FOOTER_EAT_SPACING - POOLATTR_BEGIN ] = new SwHeaderAndFooterEatSpacingItem;
/*N*/ 	aAttrTab[ RES_LINENUMBER - POOLATTR_BEGIN ] = new SwFmtLineNumber;
/*N*/ 	aAttrTab[ RES_FTN_AT_TXTEND - POOLATTR_BEGIN ] = new SwFmtFtnAtTxtEnd;
/*N*/ 	aAttrTab[ RES_END_AT_TXTEND - POOLATTR_BEGIN ] = new SwFmtEndAtTxtEnd;
/*N*/ 	aAttrTab[ RES_COLUMNBALANCE - POOLATTR_BEGIN ] = new SwFmtNoBalancedColumns;
/*N*/     aAttrTab[ RES_FRAMEDIR - POOLATTR_BEGIN ] = new SvxFrameDirectionItem(FRMDIR_ENVIRONMENT);
/*N*/ 
/*N*/ // FrameAttr - Dummies
/*N*/ 	aAttrTab[ RES_FRMATR_DUMMY9 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_FRMATR_DUMMY9 );
/*N*/ // FrameAttr - Dummies
/*N*/ 
/*N*/ 	aAttrTab[ RES_GRFATR_MIRRORGRF- POOLATTR_BEGIN ] = new SwMirrorGrf;
/*N*/ 	aAttrTab[ RES_GRFATR_CROPGRF- POOLATTR_BEGIN ] = new SwCropGrf;
/*N*/ 
/*N*/ 	aAttrTab[ RES_GRFATR_ROTATION - POOLATTR_BEGIN ] = new SwRotationGrf;
/*N*/ 	aAttrTab[ RES_GRFATR_LUMINANCE - POOLATTR_BEGIN ] = new SwLuminanceGrf;
/*N*/ 	aAttrTab[ RES_GRFATR_CONTRAST - POOLATTR_BEGIN ] = new SwContrastGrf;
/*N*/ 	aAttrTab[ RES_GRFATR_CHANNELR - POOLATTR_BEGIN ] = new SwChannelRGrf;
/*N*/ 	aAttrTab[ RES_GRFATR_CHANNELG - POOLATTR_BEGIN ] = new SwChannelGGrf;
/*N*/ 	aAttrTab[ RES_GRFATR_CHANNELB - POOLATTR_BEGIN ] = new SwChannelBGrf;
/*N*/ 	aAttrTab[ RES_GRFATR_GAMMA - POOLATTR_BEGIN ] = new SwGammaGrf;
/*N*/ 	aAttrTab[ RES_GRFATR_INVERT - POOLATTR_BEGIN ] = new SwInvertGrf;
/*N*/ 	aAttrTab[ RES_GRFATR_TRANSPARENCY - POOLATTR_BEGIN ] = new SwTransparencyGrf;
/*N*/ 	aAttrTab[ RES_GRFATR_DRAWMODE - POOLATTR_BEGIN ] = new SwDrawModeGrf;
/*N*/ 
/*N*/ // GraphicAttr - Dummies
/*N*/ 	aAttrTab[ RES_GRFATR_DUMMY1 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_GRFATR_DUMMY1 );
/*N*/ 	aAttrTab[ RES_GRFATR_DUMMY2 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_GRFATR_DUMMY2 );
/*N*/ 	aAttrTab[ RES_GRFATR_DUMMY3 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_GRFATR_DUMMY3 );
/*N*/ 	aAttrTab[ RES_GRFATR_DUMMY4 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_GRFATR_DUMMY4 );
/*N*/ 	aAttrTab[ RES_GRFATR_DUMMY5 - POOLATTR_BEGIN ] = new SfxBoolItem( RES_GRFATR_DUMMY5 );
/*N*/ // GraphicAttr - Dummies
/*N*/ 
/*N*/ 	aAttrTab[ RES_BOXATR_FORMAT- POOLATTR_BEGIN ] = new SwTblBoxNumFormat;
/*N*/ 	aAttrTab[ RES_BOXATR_FORMULA- POOLATTR_BEGIN ] = new SwTblBoxFormula( aEmptyStr );
/*N*/ 	aAttrTab[ RES_BOXATR_VALUE- POOLATTR_BEGIN ] = new SwTblBoxValue;
/*N*/ 
/*N*/ 	aAttrTab[ RES_UNKNOWNATR_CONTAINER- POOLATTR_BEGIN ] =
/*N*/ 				new SvXMLAttrContainerItem( RES_UNKNOWNATR_CONTAINER );
/*N*/ 
/*N*/ 	// get the correct fonts:
/*N*/ 	::binfilter::GetDefaultFonts( *(SvxFontItem*)aAttrTab[ RES_CHRATR_FONT- POOLATTR_BEGIN ],
/*N*/ 					   *(SvxFontItem*)aAttrTab[ RES_CHRATR_CJK_FONT - POOLATTR_BEGIN ],
/*N*/ 					   *(SvxFontItem*)aAttrTab[ RES_CHRATR_CTL_FONT - POOLATTR_BEGIN ] );
/*N*/ 
/*N*/ 	// 1. Version - neue Attribute:
/*N*/ 	//		- RES_CHRATR_BLINK
/*N*/ 	//		- RES_CHRATR_NOHYPHEN
/*N*/ 	//		- RES_CHRATR_NOLINEBREAK
/*N*/ 	// 		- RES_PARATR_REGISTER
/*N*/ 	// 		+ 2 Dummies fuer die einzelnen "Bereiche"
/*N*/ 	SwAttrPool::pVersionMap1 = new USHORT[ 60 ];
        USHORT i;
/*N*/ 	for( i = 1; i <= 17; i++ )
/*N*/ 		SwAttrPool::pVersionMap1[ i-1 ] = i;
/*N*/ 	for ( i = 18; i <= 27; ++i )
/*N*/ 		SwAttrPool::pVersionMap1[ i-1 ] = i + 5;
/*N*/ 	for ( i = 28; i <= 35; ++i )
/*N*/ 		SwAttrPool::pVersionMap1[ i-1 ] = i + 7;
/*N*/ 	for ( i = 36; i <= 58; ++i )
/*N*/ 		SwAttrPool::pVersionMap1[ i-1 ] = i + 10;
/*N*/ 	for ( i = 59; i <= 60; ++i )
/*N*/ 		SwAttrPool::pVersionMap1[ i-1 ] = i + 12;
/*N*/ 
/*N*/ 	// 2. Version - neue Attribute:
/*N*/ 	// 		10 Dummies fuer den Frame "Bereich"
/*N*/ 	SwAttrPool::pVersionMap2 = new USHORT[ 75 ];
/*N*/ 	for( i = 1; i <= 70; i++ )
/*N*/ 		SwAttrPool::pVersionMap2[ i-1 ] = i;
/*N*/ 	for ( i = 71; i <= 75; ++i )
/*N*/ 		SwAttrPool::pVersionMap2[ i-1 ] = i + 10;
/*N*/ 
/*N*/ 	// 3. Version - neue Attribute:
/*N*/ 	// 		neue Attribute und Dummies fuer die CJK-Version
/*N*/ 	// 		und neue Grafik-Attribute
/*N*/ 	SwAttrPool::pVersionMap3 = new USHORT[ 86 ];
/*N*/ 	for( i = 1; i <= 21; i++ )
/*N*/ 		SwAttrPool::pVersionMap3[ i-1 ] = i;
/*N*/ 	for ( i = 22; i <= 27; ++i )
/*N*/ 		SwAttrPool::pVersionMap3[ i-1 ] = i + 15;
/*N*/ 	for ( i = 28; i <= 82; ++i )
/*N*/ 		SwAttrPool::pVersionMap3[ i-1 ] = i + 20;
/*N*/ 	for ( i = 83; i <= 86; ++i )
/*N*/ 		SwAttrPool::pVersionMap3[ i-1 ] = i + 35;
/*N*/ 
/*N*/ 	// 4. Version - neue Paragraph Attribute fuer die CJK-Version
/*N*/ 	SwAttrPool::pVersionMap4 = new USHORT[ 121 ];
/*N*/ 	for( i = 1; i <= 65; i++ )
/*N*/ 		SwAttrPool::pVersionMap4[ i-1 ] = i;
/*N*/ 	for ( i = 66; i <= 121; ++i )
/*N*/ 		SwAttrPool::pVersionMap4[ i-1 ] = i + 9;
/*N*/ 
/*N*/ 	pBreakIt = new SwBreakIt;
/*N*/ 	const ::com::sun::star::lang::Locale& rLcl = pBreakIt->GetLocale(
/*N*/ 											(LanguageType)GetAppLanguage() );
/*M*/   pCheckIt = NULL;
/*N*/ 	::com::sun::star::uno::Reference<
/*N*/ 			::com::sun::star::lang::XMultiServiceFactory > xMSF =
/*N*/ 									::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/ 	pAppCharClass = new CharClass( xMSF, rLcl );
/*N*/ 	pCalendarWrapper = new SwCalendarWrapper( xMSF );
/*N*/ 
/*N*/ 	_FrmInit();
/*N*/ 	_TextInit();
/*N*/ 
/*N*/ 	SwSelPaintRects::pMapMode = new MapMode;
/*N*/ 	SwFntObj::pPixMap = new MapMode;
/*N*/ 
/*N*/ 	SwIndexReg::pEmptyIndexArray = new SwIndexReg;
/*N*/ 
/*N*/ 	pGlobalOLEExcludeList = new SvPtrarr;
/*N*/ 
/*N*/ }

/******************************************************************************
 *	void _FinitCore()
 ******************************************************************************/



/*N*/ void _FinitCore()
/*N*/ {
/*N*/ 	_FrmFinit();
/*N*/ 	_TextFinit();
/*N*/ 
/*N*/ 	delete pBreakIt;
/*M*/   delete pCheckIt;
/*N*/ 	delete pAppCharClass;
/*N*/ 	delete pCalendarWrapper;
/*N*/ 	delete pCollator;
/*N*/ 	delete pCaseCollator;
/*N*/ 
/*N*/ 	delete SwSelPaintRects::pMapMode;
/*N*/ 	delete SwFntObj::pPixMap;
/*N*/ 
/*N*/ 	delete SwEditShell::pAutoFmtFlags;
/*N*/ 	delete SwNumRule::pDefBulletFont;
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	//Defaultattribut freigeben lassen um asserts zu vermeiden.
/*N*/ 	if ( aAttrTab[0]->GetRefCount() )
/*N*/ 		SfxItemPool::ReleaseDefaults( aAttrTab, POOLATTR_END-POOLATTR_BEGIN, FALSE);
/*N*/ #endif
/*N*/ 
/*N*/ 	delete SwStyleNameMapper::pTextUINameArray;
/*N*/ 	delete SwStyleNameMapper::pListsUINameArray;
/*N*/ 	delete SwStyleNameMapper::pExtraUINameArray;
/*N*/ 	delete SwStyleNameMapper::pRegisterUINameArray;
/*N*/ 	delete SwStyleNameMapper::pDocUINameArray;
/*N*/ 	delete SwStyleNameMapper::pHTMLUINameArray;
/*N*/ 	delete SwStyleNameMapper::pFrmFmtUINameArray;
/*N*/ 	delete SwStyleNameMapper::pChrFmtUINameArray;
/*N*/ 	delete SwStyleNameMapper::pHTMLChrFmtUINameArray;
/*N*/ 	delete SwStyleNameMapper::pPageDescUINameArray;
/*N*/ 	delete SwStyleNameMapper::pNumRuleUINameArray;
/*N*/ 
/*N*/ 	// Delete programmatic name arrays also
/*N*/ 	delete SwStyleNameMapper::pTextProgNameArray;
/*N*/ 	delete SwStyleNameMapper::pListsProgNameArray;
/*N*/ 	delete SwStyleNameMapper::pExtraProgNameArray;
/*N*/ 	delete SwStyleNameMapper::pRegisterProgNameArray;
/*N*/ 	delete SwStyleNameMapper::pDocProgNameArray;
/*N*/ 	delete SwStyleNameMapper::pHTMLProgNameArray;
/*N*/ 	delete SwStyleNameMapper::pFrmFmtProgNameArray;
/*N*/ 	delete SwStyleNameMapper::pChrFmtProgNameArray;
/*N*/ 	delete SwStyleNameMapper::pHTMLChrFmtProgNameArray;
/*N*/ 	delete SwStyleNameMapper::pPageDescProgNameArray;
/*N*/ 	delete SwStyleNameMapper::pNumRuleProgNameArray;
/*N*/ 
/*N*/ 	// And finally, any hash tables that we used
/*N*/ 	delete SwStyleNameMapper::pParaUIMap;
/*N*/ 	delete SwStyleNameMapper::pCharUIMap;
/*N*/ 	delete SwStyleNameMapper::pPageUIMap;
/*N*/ 	delete SwStyleNameMapper::pFrameUIMap;
/*N*/ 	delete SwStyleNameMapper::pNumRuleUIMap;
/*N*/ 
/*N*/ 	delete SwStyleNameMapper::pParaProgMap;
/*N*/ 	delete SwStyleNameMapper::pCharProgMap;
/*N*/ 	delete SwStyleNameMapper::pPageProgMap;
/*N*/ 	delete SwStyleNameMapper::pFrameProgMap;
/*N*/ 	delete SwStyleNameMapper::pNumRuleProgMap;
/*N*/ 
/*N*/ 
/*N*/ 	// loesche alle default-Attribute
/*N*/ 	SfxPoolItem* pHt;
/*N*/ 	for( USHORT n = 0; n < POOLATTR_END - POOLATTR_BEGIN; n++ )
/*N*/ 		if( 0 != ( pHt = aAttrTab[n] ))
/*N*/ 			delete pHt;
/*N*/ 
/*N*/ 	::binfilter::ClearFEShellTabCols();
/*N*/ 
/*N*/ 	delete SwIndexReg::pEmptyIndexArray;
/*N*/     delete[] SwAttrPool::pVersionMap1;
/*N*/     delete[] SwAttrPool::pVersionMap2;
/*N*/     delete[] SwAttrPool::pVersionMap3;
/*N*/     delete[] SwAttrPool::pVersionMap4;
/*N*/ 
/*N*/ 	for ( USHORT i = 0; i < pGlobalOLEExcludeList->Count(); ++i )
/*N*/ 		delete (SvGlobalName*)(*pGlobalOLEExcludeList)[i];
/*N*/ 	delete pGlobalOLEExcludeList;
/*N*/ }

// returns the APP - CharClass instance - used for all ToUpper/ToLower/...
/*N*/ CharClass& GetAppCharClass()
/*N*/ {
/*N*/ 	return *pAppCharClass;
/*N*/ }

/*N*/ LocaleDataWrapper& GetAppLocaleData()
/*N*/ {
/*N*/ 	SvtSysLocale aSysLocale;
/*N*/ 	return (LocaleDataWrapper&)aSysLocale.GetLocaleData();
/*N*/ }




/*N*/ ULONG GetAppLanguage()
/*N*/ {
/*N*/ 	return Application::GetSettings().GetLanguage();
/*N*/ }

/*N*/ CollatorWrapper& GetAppCollator()
/*N*/ {
/*?*/ 	if( !pCollator )
/*?*/ 	{
/*?*/ 		const ::com::sun::star::lang::Locale& rLcl = pBreakIt->GetLocale(
/*?*/ 											(LanguageType)GetAppLanguage() );
/*?*/ 		::com::sun::star::uno::Reference<
/*?*/ 			::com::sun::star::lang::XMultiServiceFactory > xMSF =
/*?*/ 									::legacy_binfilters::getLegacyProcessServiceFactory();
/*?*/ 
/*?*/ 		pCollator = new ::CollatorWrapper( xMSF );
/*?*/ 		pCollator->loadDefaultCollator( rLcl, SW_COLLATOR_IGNORES );
/*?*/ 	}
/*?*/ 	return *pCollator;
/*N*/ }

/*N*/ const ::utl::TransliterationWrapper& GetAppCmpStrIgnore()
/*N*/ {
/*N*/ 	if( !pTransWrp )
/*N*/ 	{
/*N*/ 		::com::sun::star::uno::Reference<
/*N*/ 			::com::sun::star::lang::XMultiServiceFactory > xMSF =
/*N*/ 									::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/ 
/*N*/ 		pTransWrp = new ::utl::TransliterationWrapper( xMSF,
/*N*/ 				::com::sun::star::i18n::TransliterationModules_IGNORE_CASE |
/*N*/ 				::com::sun::star::i18n::TransliterationModules_IGNORE_KANA |
/*N*/ 				::com::sun::star::i18n::TransliterationModules_IGNORE_WIDTH );
/*N*/ 		pTransWrp->loadModuleIfNeeded( GetAppLanguage() );
/*N*/ 	}
/*N*/ 	return *pTransWrp;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
