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

#include <acmplwrd.hxx>
#include <boost/scoped_ptr.hpp>
#include <breakit.hxx>
#include <cellatr.hxx>
#include <checkit.hxx>
#include <cmdid.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <doc.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/charsetcoloritem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/nhypitem.hxx>
#include <editeng/nlbkitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/pgrditem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/prszitem.hxx>
#include <editeng/rsiditem.hxx>
#include <svl/grabbagitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/swafopt.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editsh.hxx>
#include <fchrfmt.hxx>
#include <fmtanchr.hxx>
#include <fmtautofmt.hxx>
#include <fmtclbl.hxx>
#include <fmtclds.hxx>
#include <fmtcnct.hxx>
#include <fmtcntnt.hxx>
#include <fmteiro.hxx>
#include <fmtflcnt.hxx>
#include <fmtfld.hxx>
#include <fmtfollowtextflow.hxx>
#include <fmtfordr.hxx>
#include <fmtfsize.hxx>
#include <fmtftn.hxx>
#include <fmtftntx.hxx>
#include <fmthdft.hxx>
#include <fmtinfmt.hxx>
#include <fmtline.hxx>
#include <fmtlsplt.hxx>
#include <fmtmeta.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <fmtrfmrk.hxx>
#include <fmtrowsplt.hxx>
#include <fmtruby.hxx>
#include <fmtsrnd.hxx>
#include <fmturl.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <fntcache.hxx>
#include <grfatr.hxx>
#include <hfspacingitem.hxx>
#include <hintids.hxx>
#include <init.hxx>
#include <pam.hxx>
#include <paratr.hxx>
#include <rtl/instance.hxx>
#include <svl/macitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <swcalwrp.hxx>
#include <SwStyleNameMapper.hxx>
#include <tblafmt.hxx>
#include <tgrditem.hxx>
#include <tools/globname.hxx>
#include <tox.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/svapp.hxx>
#include <viscrs.hxx>

using namespace ::com::sun::star;

extern void _FrmFinit();
extern void ClearFEShellTabCols();

// some ranges for sets in collections/ nodes

// AttrSet range for the 2 break attributes
sal_uInt16 aBreakSetRange[] = {
    RES_PAGEDESC, RES_BREAK,
    0
};

// AttrSet range for TxtFmtColl
// list attributes ( RES_PARATR_LIST_BEGIN - RES_PARATR_LIST_END ) are not
// included in the paragraph style's itemset.
sal_uInt16 aTxtFmtCollSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet range for GrfFmtColl
sal_uInt16 aGrfFmtCollSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_GRFATR_BEGIN, RES_GRFATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet range for TextNode
sal_uInt16 aTxtNodeSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet range for NoTxtNode
sal_uInt16 aNoTxtNodeSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_GRFATR_BEGIN, RES_GRFATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

sal_uInt16 aTableSetRange[] = {
    RES_FILL_ORDER,     RES_FRM_SIZE,
    RES_LR_SPACE,       RES_BREAK,
    RES_BACKGROUND,     RES_SHADOW,
    RES_HORI_ORIENT,    RES_HORI_ORIENT,
    RES_KEEP,           RES_KEEP,
    RES_LAYOUT_SPLIT,   RES_LAYOUT_SPLIT,
    RES_FRAMEDIR,       RES_FRAMEDIR,
    // #i29550#
    RES_COLLAPSING_BORDERS, RES_COLLAPSING_BORDERS,
    // <-- collapsing
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

sal_uInt16 aTableLineSetRange[] = {
    RES_FILL_ORDER,     RES_FRM_SIZE,
    RES_LR_SPACE,       RES_UL_SPACE,
    RES_BACKGROUND,     RES_SHADOW,
    RES_ROW_SPLIT,      RES_ROW_SPLIT,
    RES_PROTECT,        RES_PROTECT,
    RES_VERT_ORIENT,    RES_VERT_ORIENT,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

sal_uInt16 aTableBoxSetRange[] = {
    RES_FILL_ORDER,     RES_FRM_SIZE,
    RES_LR_SPACE,       RES_UL_SPACE,
    RES_BACKGROUND,     RES_SHADOW,
    RES_PROTECT,        RES_PROTECT,
    RES_VERT_ORIENT,    RES_VERT_ORIENT,
    RES_FRAMEDIR,       RES_FRAMEDIR,
    RES_BOXATR_BEGIN,   RES_BOXATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet range for SwFrmFmt
sal_uInt16 aFrmFmtSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet range for SwCharFmt
sal_uInt16 aCharFmtSetRange[] = {
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet range for character autostyles
sal_uInt16 aCharAutoFmtSetRange[] = {
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet range for SwPageDescFmt
sal_uInt16 aPgFrmFmtSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// create table for accessing default format attributes
SwDfltAttrTab aAttrTab;

SfxItemInfo aSlotTab[] =
{
    { SID_ATTR_CHAR_CASEMAP, SFX_ITEM_POOLABLE },       // RES_CHRATR_CASEMAP
    { SID_ATTR_CHAR_CHARSETCOLOR, SFX_ITEM_POOLABLE },  // RES_CHRATR_CHARSETCOLOR
    { SID_ATTR_CHAR_COLOR, SFX_ITEM_POOLABLE },         // RES_CHRATR_COLOR
    { SID_ATTR_CHAR_CONTOUR, SFX_ITEM_POOLABLE },       // RES_CHRATR_CONTOUR
    { SID_ATTR_CHAR_STRIKEOUT, SFX_ITEM_POOLABLE },     // RES_CHRATR_CROSSEDOUT
    { SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEM_POOLABLE },    // RES_CHRATR_ESCAPEMENT
    { SID_ATTR_CHAR_FONT, SFX_ITEM_POOLABLE },          // RES_CHRATR_FONT
    { SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEM_POOLABLE },    // RES_CHRATR_FONTSIZE
    { SID_ATTR_CHAR_KERNING, SFX_ITEM_POOLABLE },       // RES_CHRATR_KERNING
    { SID_ATTR_CHAR_LANGUAGE, SFX_ITEM_POOLABLE },      // RES_CHRATR_LANGUAGE
    { SID_ATTR_CHAR_POSTURE, SFX_ITEM_POOLABLE },       // RES_CHRATR_POSTURE
    { SID_ATTR_CHAR_PROPSIZE, SFX_ITEM_POOLABLE },      // RES_CHRATR_PROPORTIONALFONTSIZE
    { SID_ATTR_CHAR_SHADOWED, SFX_ITEM_POOLABLE },      // RES_CHRATR_SHADOWED
    { SID_ATTR_CHAR_UNDERLINE, SFX_ITEM_POOLABLE },     // RES_CHRATR_UNDERLINE
    { SID_ATTR_CHAR_WEIGHT, SFX_ITEM_POOLABLE },        // RES_CHRATR_WEIGHT
    { SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEM_POOLABLE },  // RES_CHRATR_WORDLINEMODE
    { SID_ATTR_CHAR_AUTOKERN, SFX_ITEM_POOLABLE },      // RES_CHRATR_AUTOKERN
    { SID_ATTR_FLASH, SFX_ITEM_POOLABLE },              // RES_CHRATR_BLINK
    { 0, SFX_ITEM_POOLABLE },                           // RES_CHRATR_NOLINEBREAK
    { 0, SFX_ITEM_POOLABLE },                           // RES_CHRATR_NOHYPHEN
    { SID_ATTR_BRUSH_CHAR, SFX_ITEM_POOLABLE },         // RES_CHRATR_BACKGROUND
    { SID_ATTR_CHAR_CJK_FONT, SFX_ITEM_POOLABLE },      // RES_CHRATR_CJK_FONT
    { SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEM_POOLABLE },// RES_CHRATR_CJK_FONTSIZE
    { SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEM_POOLABLE },  // RES_CHRATR_CJK_LANGUAGE
    { SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEM_POOLABLE },   // RES_CHRATR_CJK_POSTURE
    { SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEM_POOLABLE },    // RES_CHRATR_CJK_WEIGHT
    { SID_ATTR_CHAR_CTL_FONT, SFX_ITEM_POOLABLE },      // RES_CHRATR_CTL_FONT
    { SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEM_POOLABLE },// RES_CHRATR_CTL_FONTSIZE
    { SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEM_POOLABLE },  // RES_CHRATR_CTL_LANGUAGE
    { SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEM_POOLABLE },   // RES_CHRATR_CTL_POSTURE
    { SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEM_POOLABLE },    // RES_CHRATR_CTL_WEIGHT
    { SID_ATTR_CHAR_ROTATED, SFX_ITEM_POOLABLE },       // RES_CHRATR_ROTATE
    { SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEM_POOLABLE },  // RES_CHRATR_EMPHASIS_MARK
    { SID_ATTR_CHAR_TWO_LINES, SFX_ITEM_POOLABLE },     // RES_CHRATR_TWO_LINES
    { SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEM_POOLABLE },    // RES_CHRATR_SCALEW
    { SID_ATTR_CHAR_RELIEF, SFX_ITEM_POOLABLE },        // RES_CHRATR_RELIEF
    { SID_ATTR_CHAR_HIDDEN, SFX_ITEM_POOLABLE },        // RES_CHRATR_HIDDEN
    { SID_ATTR_CHAR_OVERLINE, SFX_ITEM_POOLABLE },      // RES_CHRATR_OVERLINE
    { 0, SFX_ITEM_POOLABLE },                           // RES_CHRATR_RSID
    { 0, SFX_ITEM_POOLABLE },                           // RES_CHRATR_BOX

    { 0, 0 },                                           // RES_TXTATR_REFMARK
    { 0, 0 },                                           // RES_TXTATR_TOXMARK
    { 0, 0 },                                           // RES_TXTATR_META
    { 0, 0 },                                           // RES_TXTATR_METAFIELD
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_AUTOFMT
    { FN_TXTATR_INET, 0 },                              // RES_TXTATR_INETFMT
    { 0, 0 },                                           // RES_TXTATR_CHARFMT
    { SID_ATTR_CHAR_CJK_RUBY, 0 },                      // RES_TXTATR_CJK_RUBY
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_UNKNOWN_CONTAINER
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_DUMMY5

    { 0, 0 },                                           // RES_TXTATR_FIELD
    { 0, 0 },                                           // RES_TXTATR_FLYCNT
    { 0, 0 },                                           // RES_TXTATR_FTN
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_DUMMY4
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_DUMMY3
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_DUMMY1
    { 0, SFX_ITEM_POOLABLE },                           // RES_TXTATR_DUMMY2

    { SID_ATTR_PARA_LINESPACE, SFX_ITEM_POOLABLE },     // RES_PARATR_LINESPACING
    { SID_ATTR_PARA_ADJUST, SFX_ITEM_POOLABLE },        // RES_PARATR_ADJUST
    { SID_ATTR_PARA_SPLIT, SFX_ITEM_POOLABLE },         // RES_PARATR_SPLIT
    { SID_ATTR_PARA_ORPHANS, SFX_ITEM_POOLABLE },       // RES_PARATR_ORPHANS
    { SID_ATTR_PARA_WIDOWS, SFX_ITEM_POOLABLE },        // RES_PARATR_WIDOWS
    { SID_ATTR_TABSTOP, SFX_ITEM_POOLABLE },            // RES_PARATR_TABSTOP
    { SID_ATTR_PARA_HYPHENZONE, SFX_ITEM_POOLABLE },    // RES_PARATR_HYPHENZONE
    { FN_FORMAT_DROPCAPS, 0 },                          // RES_PARATR_DROP
    { SID_ATTR_PARA_REGISTER, SFX_ITEM_POOLABLE },      // RES_PARATR_REGISTER
    { SID_ATTR_PARA_NUMRULE, SFX_ITEM_POOLABLE },       // RES_PARATR_NUMRULE
    { SID_ATTR_PARA_SCRIPTSPACE, SFX_ITEM_POOLABLE },   // RES_PARATR_SCRIPTSPACE
    { SID_ATTR_PARA_HANGPUNCTUATION, SFX_ITEM_POOLABLE },// RES_PARATR_HANGINGPUNCTUATION

    { SID_ATTR_PARA_FORBIDDEN_RULES, SFX_ITEM_POOLABLE },// RES_PARATR_FORBIDDEN_RULES
    { SID_PARA_VERTALIGN, SFX_ITEM_POOLABLE },          // RES_PARATR_VERTALIGN
    { SID_ATTR_PARA_SNAPTOGRID, SFX_ITEM_POOLABLE },    // RES_PARATR_SNAPTOGRID
    { SID_ATTR_BORDER_CONNECT, SFX_ITEM_POOLABLE },     // RES_PARATR_CONNECT_BORDER

    { SID_ATTR_PARA_OUTLINE_LEVEL, SFX_ITEM_POOLABLE }, // RES_PARATR_OUTLINELEVEL //#outline level
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_RSID
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_GRABBAG
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_LIST_ID
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_LIST_LEVEL
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_LIST_ISRESTART
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_LIST_RESTARTVALUE
    { 0, SFX_ITEM_POOLABLE },                           // RES_PARATR_LIST_ISCOUNTED

    { 0, SFX_ITEM_POOLABLE },                           // RES_FILL_ORDER
    { 0, SFX_ITEM_POOLABLE },                           // RES_FRM_SIZE
    { SID_ATTR_PAGE_PAPERBIN, SFX_ITEM_POOLABLE },      // RES_PAPER_BIN
    { SID_ATTR_LRSPACE, SFX_ITEM_POOLABLE },            // RES_LR_SPACE
    { SID_ATTR_ULSPACE, SFX_ITEM_POOLABLE },            // RES_UL_SPACE
    { 0, 0 },                                           // RES_PAGEDESC
    { SID_ATTR_PARA_PAGEBREAK, SFX_ITEM_POOLABLE },     // RES_BREAK
    { 0, 0 },                                           // RES_CNTNT
    { 0, SFX_ITEM_POOLABLE },                           // RES_HEADER
    { 0, SFX_ITEM_POOLABLE },                           // RES_FOOTER
    { 0, SFX_ITEM_POOLABLE },                           // RES_PRINT
    { FN_OPAQUE, SFX_ITEM_POOLABLE },                   // RES_OPAQUE
    { FN_SET_PROTECT, SFX_ITEM_POOLABLE },              // RES_PROTECT
    { FN_SURROUND, SFX_ITEM_POOLABLE },                 // RES_SURROUND
    { FN_VERT_ORIENT, SFX_ITEM_POOLABLE },              // RES_VERT_ORIENT
    { FN_HORI_ORIENT, SFX_ITEM_POOLABLE },              // RES_HORI_ORIENT
    { 0, 0 },                                           // RES_ANCHOR
    { SID_ATTR_BRUSH, SFX_ITEM_POOLABLE },              // RES_BACKGROUND
    { SID_ATTR_BORDER_OUTER, SFX_ITEM_POOLABLE },       // RES_BOX
    { SID_ATTR_BORDER_SHADOW, SFX_ITEM_POOLABLE },      // RES_SHADOW
    { SID_ATTR_MACROITEM, SFX_ITEM_POOLABLE },          // RES_FRMMACRO
    { FN_ATTR_COLUMNS, SFX_ITEM_POOLABLE },             // RES_COL
    { SID_ATTR_PARA_KEEP, SFX_ITEM_POOLABLE },          // RES_KEEP
    { 0, SFX_ITEM_POOLABLE },                           // RES_URL
    { 0, SFX_ITEM_POOLABLE },                           // RES_EDIT_IN_READONLY

    { 0, SFX_ITEM_POOLABLE },                           // RES_LAYOUT_SPLIT
    { 0, 0 },                                           // RES_CHAIN
    { 0, SFX_ITEM_POOLABLE },                           // RES_TEXTGRID
    { FN_FORMAT_LINENUMBER, SFX_ITEM_POOLABLE },        // RES_LINENUMBER
    { 0, SFX_ITEM_POOLABLE },                           // RES_FTN_AT_TXTEND
    { 0, SFX_ITEM_POOLABLE },                           // RES_END_AT_TXTEND
    { 0, SFX_ITEM_POOLABLE },                           // RES_COLUMNBALANCE

    { SID_ATTR_FRAMEDIRECTION, SFX_ITEM_POOLABLE },     // RES_FRAMEDIR

    { SID_ATTR_HDFT_DYNAMIC_SPACING, SFX_ITEM_POOLABLE },// RES_HEADER_FOOTER_EAT_SPACING
    { FN_TABLE_ROW_SPLIT, SFX_ITEM_POOLABLE },          // RES_ROW_SPLIT
    // #i18732# - use slot-id define in svx
    { SID_SW_FOLLOW_TEXT_FLOW, SFX_ITEM_POOLABLE },     // RES_FOLLOW_TEXT_FLOW
    // #i29550#
    { SID_SW_COLLAPSING_BORDERS, SFX_ITEM_POOLABLE },   // RES_COLLAPSING_BORDERS
    // #i28701#
    { SID_SW_WRAP_INFLUENCE_ON_OBJPOS, SFX_ITEM_POOLABLE },// RES_WRAP_INFLUENCE_ON_OBJPOS
    { 0, 0 },                                           // RES_AUTO_STYLE
    { 0, SFX_ITEM_POOLABLE },                           // RES_FRMATR_STYLE_NAME
    { 0, SFX_ITEM_POOLABLE },                           // RES_FRMATR_CONDITIONAL_STYLE_NAME
    { SID_ATTR_FILL_STYLE, SFX_ITEM_POOLABLE },                           // RES_FILL_STYLE
    { SID_ATTR_FILL_GRADIENT, SFX_ITEM_POOLABLE },                           // RES_FILL_GRADIENT

    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_MIRRORGRF
    { SID_ATTR_GRAF_CROP, SFX_ITEM_POOLABLE },          // RES_GRFATR_CROPGRF
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_ROTATION,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_LUMINANCE,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_CONTRAST,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_CHANNELR,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_CHANNELG,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_CHANNELB,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_GAMMA,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_INVERT,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_TRANSPARENCY,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY1,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY2,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY3,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY4,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY5,
    { 0, SFX_ITEM_POOLABLE },                           // RES_GRFATR_DUMMY6,

    { 0, SFX_ITEM_POOLABLE },                           // RES_BOXATR_FORMAT
    { 0, 0 },                                           // RES_BOXATR_FORMULA,
    { 0, SFX_ITEM_POOLABLE },                           // RES_BOXATR_VALUE

    { 0, SFX_ITEM_POOLABLE }                            // RES_UNKNOWNATR_CONTAINER
};


sal_uInt16* SwAttrPool::pVersionMap1 = 0;
sal_uInt16* SwAttrPool::pVersionMap2 = 0;
sal_uInt16* SwAttrPool::pVersionMap3 = 0;
sal_uInt16* SwAttrPool::pVersionMap4 = 0;
// #i18732#
sal_uInt16* SwAttrPool::pVersionMap5 = 0;
sal_uInt16* SwAttrPool::pVersionMap6 = 0;

const sal_Char* pMarkToTable    = "table";
const sal_Char* pMarkToFrame    = "frame";
const sal_Char* pMarkToRegion   = "region";
const sal_Char* pMarkToText     = "text";
const sal_Char* pMarkToOutline  = "outline";
const sal_Char* pMarkToGraphic  = "graphic";
const sal_Char* pMarkToOLE      = "ole";
const sal_Char* pMarkToSequence = "sequence";

std::vector<SvGlobalName*> *pGlobalOLEExcludeList = 0;

SwAutoCompleteWord* SwDoc::mpACmpltWords = 0;

SwCheckIt* pCheckIt = 0;
CharClass* pAppCharClass = 0;

CollatorWrapper* pCollator = 0,
                *pCaseCollator = 0;

salhelper::SingletonRef<SwCalendarWrapper>* s_getCalendarWrapper()
{
    static salhelper::SingletonRef<SwCalendarWrapper> aCalendarWrapper;
    return &aCalendarWrapper;
}

void _InitCore()
{
    SfxPoolItem* pItem;

    // first initialize all attribute pointers with 0
    memset( aAttrTab, 0, (POOLATTR_END - POOLATTR_BEGIN) * sizeof( SfxPoolItem* ) );

    aAttrTab[ RES_CHRATR_CASEMAP- POOLATTR_BEGIN ] =        new SvxCaseMapItem( SVX_CASEMAP_NOT_MAPPED, RES_CHRATR_CASEMAP);
    aAttrTab[ RES_CHRATR_CHARSETCOLOR- POOLATTR_BEGIN ] =   new SvxCharSetColorItem(RES_CHRATR_CHARSETCOLOR);
    aAttrTab[ RES_CHRATR_COLOR- POOLATTR_BEGIN ] =          new SvxColorItem(RES_CHRATR_COLOR);
    aAttrTab[ RES_CHRATR_CONTOUR- POOLATTR_BEGIN ] =        new SvxContourItem( sal_False, RES_CHRATR_CONTOUR );
    aAttrTab[ RES_CHRATR_CROSSEDOUT- POOLATTR_BEGIN ] =     new SvxCrossedOutItem( STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT );
    aAttrTab[ RES_CHRATR_ESCAPEMENT- POOLATTR_BEGIN ] =     new SvxEscapementItem( RES_CHRATR_ESCAPEMENT );
    aAttrTab[ RES_CHRATR_FONT- POOLATTR_BEGIN ] =           new SvxFontItem( RES_CHRATR_FONT );

    aAttrTab[ RES_CHRATR_FONTSIZE- POOLATTR_BEGIN ] =       new SvxFontHeightItem( 240, 100, RES_CHRATR_FONTSIZE );
    aAttrTab[ RES_CHRATR_KERNING- POOLATTR_BEGIN ] =        new SvxKerningItem( 0, RES_CHRATR_KERNING );
    aAttrTab[ RES_CHRATR_LANGUAGE- POOLATTR_BEGIN ] =       new SvxLanguageItem(LANGUAGE_DONTKNOW, RES_CHRATR_LANGUAGE );
    aAttrTab[ RES_CHRATR_POSTURE- POOLATTR_BEGIN ] =        new SvxPostureItem( ITALIC_NONE, RES_CHRATR_POSTURE );
    aAttrTab[ RES_CHRATR_PROPORTIONALFONTSIZE- POOLATTR_BEGIN ] = new SvxPropSizeItem( 100, RES_CHRATR_PROPORTIONALFONTSIZE );
    aAttrTab[ RES_CHRATR_SHADOWED- POOLATTR_BEGIN ] =       new SvxShadowedItem( sal_False, RES_CHRATR_SHADOWED );
    aAttrTab[ RES_CHRATR_UNDERLINE- POOLATTR_BEGIN ] =      new SvxUnderlineItem( UNDERLINE_NONE, RES_CHRATR_UNDERLINE );
    aAttrTab[ RES_CHRATR_WEIGHT- POOLATTR_BEGIN ] =         new SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_WEIGHT );
    aAttrTab[ RES_CHRATR_RSID - POOLATTR_BEGIN ] =          new SvxRsidItem( 0, RES_CHRATR_RSID );
    aAttrTab[ RES_CHRATR_WORDLINEMODE- POOLATTR_BEGIN ] =   new SvxWordLineModeItem( sal_False, RES_CHRATR_WORDLINEMODE );
    aAttrTab[ RES_CHRATR_AUTOKERN- POOLATTR_BEGIN ] =       new SvxAutoKernItem( sal_False, RES_CHRATR_AUTOKERN );
    aAttrTab[ RES_CHRATR_BLINK - POOLATTR_BEGIN ] =         new SvxBlinkItem( sal_False, RES_CHRATR_BLINK );
    aAttrTab[ RES_CHRATR_NOHYPHEN - POOLATTR_BEGIN ] =      new SvxNoHyphenItem( sal_True, RES_CHRATR_NOHYPHEN );
    aAttrTab[ RES_CHRATR_NOLINEBREAK- POOLATTR_BEGIN ] =    new SvxNoLinebreakItem( sal_True, RES_CHRATR_NOLINEBREAK );
    aAttrTab[ RES_CHRATR_BACKGROUND - POOLATTR_BEGIN ] =    new SvxBrushItem( RES_CHRATR_BACKGROUND );

    // CJK-Attributes
    aAttrTab[ RES_CHRATR_CJK_FONT - POOLATTR_BEGIN ] =      new SvxFontItem( RES_CHRATR_CJK_FONT );
    aAttrTab[ RES_CHRATR_CJK_FONTSIZE - POOLATTR_BEGIN ] =  new SvxFontHeightItem( 240, 100, RES_CHRATR_CJK_FONTSIZE );
    aAttrTab[ RES_CHRATR_CJK_LANGUAGE - POOLATTR_BEGIN ] =  new SvxLanguageItem(LANGUAGE_DONTKNOW, RES_CHRATR_CJK_LANGUAGE);
    aAttrTab[ RES_CHRATR_CJK_POSTURE - POOLATTR_BEGIN ] =   new SvxPostureItem(ITALIC_NONE, RES_CHRATR_CJK_POSTURE );
    aAttrTab[ RES_CHRATR_CJK_WEIGHT - POOLATTR_BEGIN ] =    new SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_CJK_WEIGHT );

    // CTL-Attributes
    aAttrTab[ RES_CHRATR_CTL_FONT - POOLATTR_BEGIN ] =      new SvxFontItem( RES_CHRATR_CTL_FONT );
    aAttrTab[ RES_CHRATR_CTL_FONTSIZE - POOLATTR_BEGIN ] =  new SvxFontHeightItem(  240, 100,  RES_CHRATR_CTL_FONTSIZE );
    aAttrTab[ RES_CHRATR_CTL_LANGUAGE - POOLATTR_BEGIN ] =  new SvxLanguageItem(LANGUAGE_DONTKNOW, RES_CHRATR_CTL_LANGUAGE);
    aAttrTab[ RES_CHRATR_CTL_POSTURE - POOLATTR_BEGIN ] =   new SvxPostureItem(ITALIC_NONE, RES_CHRATR_CTL_POSTURE );
    aAttrTab[ RES_CHRATR_CTL_WEIGHT - POOLATTR_BEGIN ] =    new SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_CTL_WEIGHT );

    aAttrTab[ RES_CHRATR_ROTATE - POOLATTR_BEGIN ] =        new SvxCharRotateItem( 0, sal_False, RES_CHRATR_ROTATE );
    aAttrTab[ RES_CHRATR_EMPHASIS_MARK - POOLATTR_BEGIN ] = new SvxEmphasisMarkItem( EMPHASISMARK_NONE, RES_CHRATR_EMPHASIS_MARK );
    aAttrTab[ RES_CHRATR_TWO_LINES - POOLATTR_BEGIN ] =     new SvxTwoLinesItem( sal_False, 0, 0, RES_CHRATR_TWO_LINES );
    aAttrTab[ RES_CHRATR_SCALEW - POOLATTR_BEGIN ] =        new SvxCharScaleWidthItem( 100, RES_CHRATR_SCALEW );
    aAttrTab[ RES_CHRATR_RELIEF - POOLATTR_BEGIN ] =        new SvxCharReliefItem( RELIEF_NONE, RES_CHRATR_RELIEF );
    aAttrTab[ RES_CHRATR_HIDDEN - POOLATTR_BEGIN ] =        new SvxCharHiddenItem( sal_False, RES_CHRATR_HIDDEN );
    aAttrTab[ RES_CHRATR_OVERLINE- POOLATTR_BEGIN ] =       new SvxOverlineItem( UNDERLINE_NONE, RES_CHRATR_OVERLINE );
    aAttrTab[ RES_CHRATR_BOX - POOLATTR_BEGIN ] =           new SvxBoxItem( RES_CHRATR_BOX );

    aAttrTab[ RES_TXTATR_AUTOFMT- POOLATTR_BEGIN ] =        new SwFmtAutoFmt;
    aAttrTab[ RES_TXTATR_INETFMT - POOLATTR_BEGIN ] =       new SwFmtINetFmt( OUString(), OUString() );
    aAttrTab[ RES_TXTATR_REFMARK - POOLATTR_BEGIN ] =       new SwFmtRefMark( aEmptyStr );
    aAttrTab[ RES_TXTATR_TOXMARK - POOLATTR_BEGIN ] =       new SwTOXMark;
    aAttrTab[ RES_TXTATR_CHARFMT- POOLATTR_BEGIN ] =        new SwFmtCharFmt( 0 );
    aAttrTab[ RES_TXTATR_CJK_RUBY - POOLATTR_BEGIN ] =      new SwFmtRuby( aEmptyStr );
    aAttrTab[ RES_TXTATR_UNKNOWN_CONTAINER - POOLATTR_BEGIN ] = new SvXMLAttrContainerItem( RES_TXTATR_UNKNOWN_CONTAINER );
    aAttrTab[ RES_TXTATR_META - POOLATTR_BEGIN ] =      SwFmtMeta::CreatePoolDefault(RES_TXTATR_META);
    aAttrTab[ RES_TXTATR_METAFIELD - POOLATTR_BEGIN ] = SwFmtMeta::CreatePoolDefault(RES_TXTATR_METAFIELD);

    aAttrTab[ RES_TXTATR_FIELD- POOLATTR_BEGIN ] =          new SwFmtFld;
    aAttrTab[ RES_TXTATR_FLYCNT - POOLATTR_BEGIN ] =        new SwFmtFlyCnt( 0 );
    aAttrTab[ RES_TXTATR_FTN - POOLATTR_BEGIN ] =           new SwFmtFtn;

// TextAttr - Dummies
    aAttrTab[ RES_TXTATR_DUMMY1 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_TXTATR_DUMMY1 );
    aAttrTab[ RES_TXTATR_DUMMY2 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_TXTATR_DUMMY2 );
    aAttrTab[ RES_TXTATR_DUMMY3 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_TXTATR_DUMMY3 );
    aAttrTab[ RES_TXTATR_DUMMY4 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_TXTATR_DUMMY4 );
    aAttrTab[ RES_TXTATR_DUMMY5 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_TXTATR_DUMMY5 );

    aAttrTab[ RES_PARATR_LINESPACING- POOLATTR_BEGIN ] =    new SvxLineSpacingItem( LINE_SPACE_DEFAULT_HEIGHT, RES_PARATR_LINESPACING );
    aAttrTab[ RES_PARATR_ADJUST- POOLATTR_BEGIN ] =         new SvxAdjustItem( SVX_ADJUST_LEFT, RES_PARATR_ADJUST );
    aAttrTab[ RES_PARATR_SPLIT- POOLATTR_BEGIN ] =          new SvxFmtSplitItem( sal_True, RES_PARATR_SPLIT );
    aAttrTab[ RES_PARATR_WIDOWS- POOLATTR_BEGIN ] =         new SvxWidowsItem( 0, RES_PARATR_WIDOWS );
    aAttrTab[ RES_PARATR_ORPHANS- POOLATTR_BEGIN ] =        new SvxOrphansItem( 0, RES_PARATR_ORPHANS );
    aAttrTab[ RES_PARATR_TABSTOP- POOLATTR_BEGIN ] =        new SvxTabStopItem( 1, SVX_TAB_DEFDIST, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );

    pItem = new SvxHyphenZoneItem( sal_False, RES_PARATR_HYPHENZONE );
    ((SvxHyphenZoneItem*)pItem)->GetMaxHyphens() = 0; // Default: 0
    aAttrTab[ RES_PARATR_HYPHENZONE- POOLATTR_BEGIN ] =     pItem;

    aAttrTab[ RES_PARATR_DROP- POOLATTR_BEGIN ] =           new SwFmtDrop;
    aAttrTab[ RES_PARATR_REGISTER - POOLATTR_BEGIN ] =      new SwRegisterItem( sal_False );
    aAttrTab[ RES_PARATR_NUMRULE - POOLATTR_BEGIN ] =       new SwNumRuleItem( aEmptyStr );

    aAttrTab[ RES_PARATR_SCRIPTSPACE - POOLATTR_BEGIN ] =   new SvxScriptSpaceItem( sal_True, RES_PARATR_SCRIPTSPACE );
    aAttrTab[ RES_PARATR_HANGINGPUNCTUATION - POOLATTR_BEGIN ] = new SvxHangingPunctuationItem( sal_True, RES_PARATR_HANGINGPUNCTUATION );
    aAttrTab[ RES_PARATR_FORBIDDEN_RULES - POOLATTR_BEGIN ] = new SvxForbiddenRuleItem( sal_True, RES_PARATR_FORBIDDEN_RULES );
    aAttrTab[ RES_PARATR_VERTALIGN - POOLATTR_BEGIN ] =     new SvxParaVertAlignItem( 0, RES_PARATR_VERTALIGN );
    aAttrTab[ RES_PARATR_SNAPTOGRID - POOLATTR_BEGIN ] =    new SvxParaGridItem( sal_True, RES_PARATR_SNAPTOGRID );
    aAttrTab[ RES_PARATR_CONNECT_BORDER - POOLATTR_BEGIN ] = new SwParaConnectBorderItem;

    aAttrTab[ RES_PARATR_OUTLINELEVEL - POOLATTR_BEGIN ] =  new SfxUInt16Item( RES_PARATR_OUTLINELEVEL, 0 );
    aAttrTab[ RES_PARATR_RSID - POOLATTR_BEGIN ] =          new SvxRsidItem( 0, RES_PARATR_RSID );
    aAttrTab[ RES_PARATR_GRABBAG - POOLATTR_BEGIN ] =       new SfxGrabBagItem( RES_PARATR_GRABBAG );

    aAttrTab[ RES_PARATR_LIST_ID - POOLATTR_BEGIN ] =       new SfxStringItem( RES_PARATR_LIST_ID, aEmptyStr );
    aAttrTab[ RES_PARATR_LIST_LEVEL - POOLATTR_BEGIN ] =    new SfxInt16Item( RES_PARATR_LIST_LEVEL, 0 );
    aAttrTab[ RES_PARATR_LIST_ISRESTART - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_LIST_ISRESTART, sal_False );
    aAttrTab[ RES_PARATR_LIST_RESTARTVALUE - POOLATTR_BEGIN ] = new SfxInt16Item( RES_PARATR_LIST_RESTARTVALUE, 1 );
    aAttrTab[ RES_PARATR_LIST_ISCOUNTED - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_LIST_ISCOUNTED, sal_True );

    aAttrTab[ RES_FILL_ORDER- POOLATTR_BEGIN ] =            new SwFmtFillOrder;
    aAttrTab[ RES_FRM_SIZE- POOLATTR_BEGIN ] =              new SwFmtFrmSize;
    aAttrTab[ RES_PAPER_BIN- POOLATTR_BEGIN ] =             new SvxPaperBinItem( RES_PAPER_BIN );
    aAttrTab[ RES_LR_SPACE- POOLATTR_BEGIN ] =              new SvxLRSpaceItem( RES_LR_SPACE );
    aAttrTab[ RES_UL_SPACE- POOLATTR_BEGIN ] =              new SvxULSpaceItem( RES_UL_SPACE );
    aAttrTab[ RES_PAGEDESC- POOLATTR_BEGIN ] =              new SwFmtPageDesc;
    aAttrTab[ RES_BREAK- POOLATTR_BEGIN ] =                 new SvxFmtBreakItem( SVX_BREAK_NONE, RES_BREAK);
    aAttrTab[ RES_CNTNT- POOLATTR_BEGIN ] =                 new SwFmtCntnt;
    aAttrTab[ RES_HEADER- POOLATTR_BEGIN ] =                new SwFmtHeader;
    aAttrTab[ RES_FOOTER- POOLATTR_BEGIN ] =                new SwFmtFooter;
    aAttrTab[ RES_PRINT- POOLATTR_BEGIN ] =                 new SvxPrintItem( RES_PRINT );
    aAttrTab[ RES_OPAQUE- POOLATTR_BEGIN ] =                new SvxOpaqueItem( RES_OPAQUE );
    aAttrTab[ RES_PROTECT- POOLATTR_BEGIN ] =               new SvxProtectItem( RES_PROTECT );
    aAttrTab[ RES_SURROUND- POOLATTR_BEGIN ] =              new SwFmtSurround;
    aAttrTab[ RES_VERT_ORIENT- POOLATTR_BEGIN ] =           new SwFmtVertOrient;
    aAttrTab[ RES_HORI_ORIENT- POOLATTR_BEGIN ] =           new SwFmtHoriOrient;
    aAttrTab[ RES_ANCHOR- POOLATTR_BEGIN ] =                new SwFmtAnchor;
    aAttrTab[ RES_BACKGROUND- POOLATTR_BEGIN ] =            new SvxBrushItem( RES_BACKGROUND );
    aAttrTab[ RES_BOX- POOLATTR_BEGIN ] =                   new SvxBoxItem( RES_BOX );
    aAttrTab[ RES_SHADOW- POOLATTR_BEGIN ] =                new SvxShadowItem( RES_SHADOW );
    aAttrTab[ RES_FRMMACRO- POOLATTR_BEGIN ] =              new SvxMacroItem( RES_FRMMACRO );
    aAttrTab[ RES_COL- POOLATTR_BEGIN ] =                   new SwFmtCol;
    aAttrTab[ RES_KEEP - POOLATTR_BEGIN ] =                 new SvxFmtKeepItem( sal_False, RES_KEEP );
    aAttrTab[ RES_URL - POOLATTR_BEGIN ] =                  new SwFmtURL();
    aAttrTab[ RES_EDIT_IN_READONLY - POOLATTR_BEGIN ] =     new SwFmtEditInReadonly;
    aAttrTab[ RES_LAYOUT_SPLIT - POOLATTR_BEGIN ] =         new SwFmtLayoutSplit;
    aAttrTab[ RES_CHAIN - POOLATTR_BEGIN ] =                new SwFmtChain;
    aAttrTab[ RES_TEXTGRID - POOLATTR_BEGIN ] =             new SwTextGridItem;
    aAttrTab[ RES_HEADER_FOOTER_EAT_SPACING - POOLATTR_BEGIN ] = new SwHeaderAndFooterEatSpacingItem;
    aAttrTab[ RES_LINENUMBER - POOLATTR_BEGIN ] =           new SwFmtLineNumber;
    aAttrTab[ RES_FTN_AT_TXTEND - POOLATTR_BEGIN ] =        new SwFmtFtnAtTxtEnd;
    aAttrTab[ RES_END_AT_TXTEND - POOLATTR_BEGIN ] =        new SwFmtEndAtTxtEnd;
    aAttrTab[ RES_COLUMNBALANCE - POOLATTR_BEGIN ] =        new SwFmtNoBalancedColumns;
    aAttrTab[ RES_FRAMEDIR - POOLATTR_BEGIN ] =             new SvxFrameDirectionItem( FRMDIR_ENVIRONMENT, RES_FRAMEDIR );
    aAttrTab[ RES_ROW_SPLIT - POOLATTR_BEGIN ] =            new SwFmtRowSplit;

    // #i18732#
    aAttrTab[ RES_FOLLOW_TEXT_FLOW - POOLATTR_BEGIN ] =     new SwFmtFollowTextFlow(false);
    // collapsing borders #i29550#
    aAttrTab[ RES_COLLAPSING_BORDERS - POOLATTR_BEGIN ] =   new SfxBoolItem( RES_COLLAPSING_BORDERS, sal_False );
    // #i28701#
    // #i35017# - constant name has changed
    aAttrTab[ RES_WRAP_INFLUENCE_ON_OBJPOS - POOLATTR_BEGIN ] =
            new SwFmtWrapInfluenceOnObjPos( text::WrapInfluenceOnPosition::ONCE_CONCURRENT );

    aAttrTab[ RES_AUTO_STYLE - POOLATTR_BEGIN ] =           new SwFmtAutoFmt( RES_AUTO_STYLE );
    aAttrTab[ RES_FRMATR_STYLE_NAME - POOLATTR_BEGIN ] =    new SfxStringItem( RES_FRMATR_STYLE_NAME, aEmptyStr );
    aAttrTab[ RES_FRMATR_CONDITIONAL_STYLE_NAME - POOLATTR_BEGIN ] = new SfxStringItem( RES_FRMATR_CONDITIONAL_STYLE_NAME, aEmptyStr );
    aAttrTab[ RES_FILL_STYLE - POOLATTR_BEGIN ] = new XFillStyleItem(XFILL_SOLID, RES_FILL_STYLE);
    aAttrTab[ RES_FILL_GRADIENT - POOLATTR_BEGIN ] = new XFillGradientItem(RES_FILL_GRADIENT);

    aAttrTab[ RES_GRFATR_MIRRORGRF- POOLATTR_BEGIN ] =      new SwMirrorGrf;
    aAttrTab[ RES_GRFATR_CROPGRF- POOLATTR_BEGIN ] =        new SwCropGrf;
    aAttrTab[ RES_GRFATR_ROTATION - POOLATTR_BEGIN ] =      new SwRotationGrf;
    aAttrTab[ RES_GRFATR_LUMINANCE - POOLATTR_BEGIN ] =     new SwLuminanceGrf;
    aAttrTab[ RES_GRFATR_CONTRAST - POOLATTR_BEGIN ] =      new SwContrastGrf;
    aAttrTab[ RES_GRFATR_CHANNELR - POOLATTR_BEGIN ] =      new SwChannelRGrf;
    aAttrTab[ RES_GRFATR_CHANNELG - POOLATTR_BEGIN ] =      new SwChannelGGrf;
    aAttrTab[ RES_GRFATR_CHANNELB - POOLATTR_BEGIN ] =      new SwChannelBGrf;
    aAttrTab[ RES_GRFATR_GAMMA - POOLATTR_BEGIN ] =         new SwGammaGrf;
    aAttrTab[ RES_GRFATR_INVERT - POOLATTR_BEGIN ] =        new SwInvertGrf;
    aAttrTab[ RES_GRFATR_TRANSPARENCY - POOLATTR_BEGIN ] =  new SwTransparencyGrf;
    aAttrTab[ RES_GRFATR_DRAWMODE - POOLATTR_BEGIN ] =      new SwDrawModeGrf;

// GraphicAttr - Dummies
    aAttrTab[ RES_GRFATR_DUMMY1 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_GRFATR_DUMMY1 );
    aAttrTab[ RES_GRFATR_DUMMY2 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_GRFATR_DUMMY2 );
    aAttrTab[ RES_GRFATR_DUMMY3 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_GRFATR_DUMMY3 );
    aAttrTab[ RES_GRFATR_DUMMY4 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_GRFATR_DUMMY4 );
    aAttrTab[ RES_GRFATR_DUMMY5 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_GRFATR_DUMMY5 );

    aAttrTab[ RES_BOXATR_FORMAT- POOLATTR_BEGIN ] =         new SwTblBoxNumFormat;
    aAttrTab[ RES_BOXATR_FORMULA- POOLATTR_BEGIN ] =        new SwTblBoxFormula( aEmptyStr );
    aAttrTab[ RES_BOXATR_VALUE- POOLATTR_BEGIN ] =          new SwTblBoxValue;

    aAttrTab[ RES_UNKNOWNATR_CONTAINER- POOLATTR_BEGIN ] =
                new SvXMLAttrContainerItem( RES_UNKNOWNATR_CONTAINER );

    // get the correct fonts:
    ::GetDefaultFonts( *(SvxFontItem*)aAttrTab[ RES_CHRATR_FONT- POOLATTR_BEGIN ],
                       *(SvxFontItem*)aAttrTab[ RES_CHRATR_CJK_FONT - POOLATTR_BEGIN ],
                       *(SvxFontItem*)aAttrTab[ RES_CHRATR_CTL_FONT - POOLATTR_BEGIN ] );

    // 1. version - new attributes:
    //      - RES_CHRATR_BLINK
    //      - RES_CHRATR_NOHYPHEN
    //      - RES_CHRATR_NOLINEBREAK
    //      - RES_PARATR_REGISTER
    //      + 2 dummies for the "ranges"
    SwAttrPool::pVersionMap1 = new sal_uInt16[ 60 ];
    sal_uInt16 i;
    for( i = 1; i <= 17; i++ )
        SwAttrPool::pVersionMap1[ i-1 ] = i;
    for ( i = 18; i <= 27; ++i )
        SwAttrPool::pVersionMap1[ i-1 ] = i + 5;
    for ( i = 28; i <= 35; ++i )
        SwAttrPool::pVersionMap1[ i-1 ] = i + 7;
    for ( i = 36; i <= 58; ++i )
        SwAttrPool::pVersionMap1[ i-1 ] = i + 10;
    for ( i = 59; i <= 60; ++i )
        SwAttrPool::pVersionMap1[ i-1 ] = i + 12;

    // 2. version - new attributes:
    //      10 dummies for the frame "range"
    SwAttrPool::pVersionMap2 = new sal_uInt16[ 75 ];
    for( i = 1; i <= 70; i++ )
        SwAttrPool::pVersionMap2[ i-1 ] = i;
    for ( i = 71; i <= 75; ++i )
        SwAttrPool::pVersionMap2[ i-1 ] = i + 10;

    // 3. version:
    //      new attributes and dummies for the CJK version and
    //      new graphics attributes
    SwAttrPool::pVersionMap3 = new sal_uInt16[ 86 ];
    for( i = 1; i <= 21; i++ )
        SwAttrPool::pVersionMap3[ i-1 ] = i;
    for ( i = 22; i <= 27; ++i )
        SwAttrPool::pVersionMap3[ i-1 ] = i + 15;
    for ( i = 28; i <= 82; ++i )
        SwAttrPool::pVersionMap3[ i-1 ] = i + 20;
    for ( i = 83; i <= 86; ++i )
        SwAttrPool::pVersionMap3[ i-1 ] = i + 35;

    // 4. version:
    //      new paragraph attributes for CJK version
    SwAttrPool::pVersionMap4 = new sal_uInt16[ 121 ];
    for( i = 1; i <= 65; i++ )
        SwAttrPool::pVersionMap4[ i-1 ] = i;
    for ( i = 66; i <= 121; ++i )
        SwAttrPool::pVersionMap4[ i-1 ] = i + 9;

    // 5. version
    // #i18732# - setup new version map due to extension of
    // the frame attributes (RES_FRMATR_*) for binary filters.
    SwAttrPool::pVersionMap5 = new sal_uInt16[ 130 ];
    for( i = 1; i <= 109; i++ )
        SwAttrPool::pVersionMap5[ i-1 ] = i;
    for ( i = 110; i <= 130; ++i )
        SwAttrPool::pVersionMap5[ i-1 ] = i + 6;

    // 6. version:
    //      RES_CHARATR_OVERLINE
    //      new character attribute for overlining plus 2 dummies
    //      1. dummy -> RES_CHRATR_RSID
    //      2. dummy -> RES_CHRATR_BOX
    SwAttrPool::pVersionMap6 = new sal_uInt16[ 136 ];
    for( i = 1; i <= 37; i++ )
        SwAttrPool::pVersionMap6[ i-1 ] = i;
    for ( i = 38; i <= 136; ++i )
        SwAttrPool::pVersionMap6[ i-1 ] = i + 3;

    SwBreakIt::_Create( ::comphelper::getProcessComponentContext() );
    pCheckIt = NULL;

    _FrmInit();
    _TextInit();

    SwSelPaintRects::pMapMode = new MapMode;
    SwFntObj::pPixMap = new MapMode;

    pGlobalOLEExcludeList = new std::vector<SvGlobalName*>;

    const SvxSwAutoFmtFlags& rAFlags = SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags();
    SwDoc::mpACmpltWords = new SwAutoCompleteWord( rAFlags.nAutoCmpltListLen,
                                            rAFlags.nAutoCmpltWordLen );
}

void _FinitCore()
{
    _FrmFinit();
    _TextFinit();

    SwBreakIt::_Delete();
    delete pCheckIt;
    delete pAppCharClass;
    delete pCollator;
    delete pCaseCollator;

    // destroy default TableAutoFormat
    delete SwTableAutoFmt::pDfltBoxAutoFmt;

    delete SwSelPaintRects::pMapMode;
    delete SwFntObj::pPixMap;

    delete SwEditShell::pAutoFmtFlags;

#if OSL_DEBUG_LEVEL > 0
    // free defaults to prevent assertions
    if ( aAttrTab[0]->GetRefCount() )
        SfxItemPool::ReleaseDefaults( aAttrTab, POOLATTR_END-POOLATTR_BEGIN, sal_False);
#endif
    delete SwDoc::mpACmpltWords;

    delete SwStyleNameMapper::pTextUINameArray;
    delete SwStyleNameMapper::pListsUINameArray;
    delete SwStyleNameMapper::pExtraUINameArray;
    delete SwStyleNameMapper::pRegisterUINameArray;
    delete SwStyleNameMapper::pDocUINameArray;
    delete SwStyleNameMapper::pHTMLUINameArray;
    delete SwStyleNameMapper::pFrmFmtUINameArray;
    delete SwStyleNameMapper::pChrFmtUINameArray;
    delete SwStyleNameMapper::pHTMLChrFmtUINameArray;
    delete SwStyleNameMapper::pPageDescUINameArray;
    delete SwStyleNameMapper::pNumRuleUINameArray;

    // Delete programmatic name arrays also
    delete SwStyleNameMapper::pTextProgNameArray;
    delete SwStyleNameMapper::pListsProgNameArray;
    delete SwStyleNameMapper::pExtraProgNameArray;
    delete SwStyleNameMapper::pRegisterProgNameArray;
    delete SwStyleNameMapper::pDocProgNameArray;
    delete SwStyleNameMapper::pHTMLProgNameArray;
    delete SwStyleNameMapper::pFrmFmtProgNameArray;
    delete SwStyleNameMapper::pChrFmtProgNameArray;
    delete SwStyleNameMapper::pHTMLChrFmtProgNameArray;
    delete SwStyleNameMapper::pPageDescProgNameArray;
    delete SwStyleNameMapper::pNumRuleProgNameArray;

    // And finally, any hash tables that we used
    delete SwStyleNameMapper::pParaUIMap;
    delete SwStyleNameMapper::pCharUIMap;
    delete SwStyleNameMapper::pPageUIMap;
    delete SwStyleNameMapper::pFrameUIMap;
    delete SwStyleNameMapper::pNumRuleUIMap;

    delete SwStyleNameMapper::pParaProgMap;
    delete SwStyleNameMapper::pCharProgMap;
    delete SwStyleNameMapper::pPageProgMap;
    delete SwStyleNameMapper::pFrameProgMap;
    delete SwStyleNameMapper::pNumRuleProgMap;


    // delete all default attributes
    SfxPoolItem* pHt;
    for( sal_uInt16 n = 0; n < POOLATTR_END - POOLATTR_BEGIN; n++ )
        if( 0 != ( pHt = aAttrTab[n] ))
            delete pHt;

    ::ClearFEShellTabCols();

    delete[] SwAttrPool::pVersionMap1;
    delete[] SwAttrPool::pVersionMap2;
    delete[] SwAttrPool::pVersionMap3;
    delete[] SwAttrPool::pVersionMap4;
    // #i18732#
    delete[] SwAttrPool::pVersionMap5;
    delete[] SwAttrPool::pVersionMap6;

    for ( sal_uInt16 i = 0; i < pGlobalOLEExcludeList->size(); ++i )
        delete (*pGlobalOLEExcludeList)[i];
    delete pGlobalOLEExcludeList;
}

// returns the APP - CharClass instance - used for all ToUpper/ToLower/...
CharClass& GetAppCharClass()
{
    if ( !pAppCharClass )
    {
        pAppCharClass = new CharClass(
            ::comphelper::getProcessComponentContext(),
            SwBreakIt::Get()->GetLanguageTag( GetAppLanguageTag() ));
    }
    return *pAppCharClass;
}

void SwCalendarWrapper::LoadDefaultCalendar( sal_uInt16 eLang )
{
    sUniqueId.Erase();
    if( eLang != nLang )
        loadDefaultCalendar( LanguageTag::convertToLocale( nLang = eLang ));
}

LanguageType GetAppLanguage()
{
    return Application::GetSettings().GetLanguageTag().getLanguageType();
}

const LanguageTag& GetAppLanguageTag()
{
    return Application::GetSettings().GetLanguageTag();
}

CollatorWrapper& GetAppCollator()
{
    if( !pCollator )
    {
        const lang::Locale& rLcl = g_pBreakIt->GetLocale( GetAppLanguage() );

        pCollator = new CollatorWrapper( ::comphelper::getProcessComponentContext() );
        pCollator->loadDefaultCollator( rLcl, SW_COLLATOR_IGNORES );
    }
    return *pCollator;
}
CollatorWrapper& GetAppCaseCollator()
{
    if( !pCaseCollator )
    {
        const lang::Locale& rLcl = g_pBreakIt->GetLocale( GetAppLanguage() );

        pCaseCollator = new CollatorWrapper( ::comphelper::getProcessComponentContext() );
        pCaseCollator->loadDefaultCollator( rLcl, 0 );
    }
    return *pCaseCollator;
}

namespace
{
    class TransWrp
    {
    private:
        boost::scoped_ptr< ::utl::TransliterationWrapper > xTransWrp;
    public:
        TransWrp()
        {
            uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

            xTransWrp.reset(new ::utl::TransliterationWrapper( xContext,
                    i18n::TransliterationModules_IGNORE_CASE |
                    i18n::TransliterationModules_IGNORE_KANA |
                    i18n::TransliterationModules_IGNORE_WIDTH ));

            xTransWrp->loadModuleIfNeeded( static_cast<sal_uInt16>(GetAppLanguage()) );
        }
        const ::utl::TransliterationWrapper& getTransliterationWrapper() const
        {
            return *xTransWrp;
        }
    };

    class theTransWrp : public rtl::Static<TransWrp, theTransWrp> {};
}

const ::utl::TransliterationWrapper& GetAppCmpStrIgnore()
{
    return theTransWrp::get().getTransliterationWrapper();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
