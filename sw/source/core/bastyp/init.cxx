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

#include <memory>
#include <acmplwrd.hxx>
#include <breakit.hxx>
#include <cellatr.hxx>
#include <checkit.hxx>
#include <cmdid.h>
#include <fesh.hxx>
#include <comphelper/processfactory.hxx>
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
#include <editeng/opaqitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/pgrditem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/protitem.hxx>
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
#include <proofreadingiterator.hxx>
#include <editeng/editids.hrc>
#include <svx/svxids.hrc>
#include <rtl/instance.hxx>
#include <svl/macitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <svx/sdtaitm.hxx>
#include <swcalwrp.hxx>
#include <SwStyleNameMapper.hxx>
#include <tblafmt.hxx>
#include <tgrditem.hxx>
#include <tools/globname.hxx>
#include <tox.hxx>
#include <unotools/charclass.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <salhelper/singletonref.hxx>
#include <viscrs.hxx>

using namespace ::com::sun::star;

// some ranges for sets in collections/ nodes

// AttrSet range for the 2 break attributes
sal_uInt16 aBreakSetRange[] = {
    RES_PAGEDESC, RES_BREAK,
    0
};

// AttrSet range for TextFormatColl
// list attributes ( RES_PARATR_LIST_BEGIN - RES_PARATR_LIST_END ) are not
// included in the paragraph style's itemset.
sal_uInt16 aTextFormatCollSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,

    // FillAttribute support
    XATTR_FILL_FIRST, XATTR_FILL_LAST,

    0
};

// AttrSet range for GrfFormatColl
sal_uInt16 aGrfFormatCollSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_GRFATR_BEGIN, RES_GRFATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet range for TextNode
sal_uInt16 aTextNodeSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,

    // FillAttribute support (paragraph FillStyle)
    XATTR_FILL_FIRST, XATTR_FILL_LAST,

    0
};

// AttrSet range for NoTextNode
sal_uInt16 aNoTextNodeSetRange[] = {
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
    RES_FRMATR_GRABBAG, RES_FRMATR_GRABBAG,
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
    RES_FRMATR_GRABBAG, RES_FRMATR_GRABBAG,
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
    RES_FRMATR_GRABBAG, RES_FRMATR_GRABBAG,
    0
};

// AttrSet range for SwFrameFormat
sal_uInt16 aFrameFormatSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,

    // FillAttribute support (TextFrame, OLE, Writer GraphicObject)
    XATTR_FILL_FIRST, XATTR_FILL_LAST,

    0
};

// AttrSet range for SwCharFormat
sal_uInt16 aCharFormatSetRange[] = {
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet range for character autostyles
sal_uInt16 aCharAutoFormatSetRange[] = {
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// AttrSet range for SwPageDescFormat
sal_uInt16 aPgFrameFormatSetRange[] = {
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
    0
};

// create table for accessing default format attributes
SwDfltAttrTab aAttrTab( POOLATTR_END - POOLATTR_BEGIN, nullptr );

SfxItemInfo aSlotTab[] =
{
    { SID_ATTR_CHAR_CASEMAP, true },       // RES_CHRATR_CASEMAP
    { SID_ATTR_CHAR_CHARSETCOLOR, true },  // RES_CHRATR_CHARSETCOLOR
    { SID_ATTR_CHAR_COLOR, true },         // RES_CHRATR_COLOR
    { SID_ATTR_CHAR_CONTOUR, true },       // RES_CHRATR_CONTOUR
    { SID_ATTR_CHAR_STRIKEOUT, true },     // RES_CHRATR_CROSSEDOUT
    { SID_ATTR_CHAR_ESCAPEMENT, true },    // RES_CHRATR_ESCAPEMENT
    { SID_ATTR_CHAR_FONT, true },          // RES_CHRATR_FONT
    { SID_ATTR_CHAR_FONTHEIGHT, true },    // RES_CHRATR_FONTSIZE
    { SID_ATTR_CHAR_KERNING, true },       // RES_CHRATR_KERNING
    { SID_ATTR_CHAR_LANGUAGE, true },      // RES_CHRATR_LANGUAGE
    { SID_ATTR_CHAR_POSTURE, true },       // RES_CHRATR_POSTURE
    { 0, true },                           // RES_CHRATR_UNUSED1
    { SID_ATTR_CHAR_SHADOWED, true },      // RES_CHRATR_SHADOWED
    { SID_ATTR_CHAR_UNDERLINE, true },     // RES_CHRATR_UNDERLINE
    { SID_ATTR_CHAR_WEIGHT, true },        // RES_CHRATR_WEIGHT
    { SID_ATTR_CHAR_WORDLINEMODE, true },  // RES_CHRATR_WORDLINEMODE
    { SID_ATTR_CHAR_AUTOKERN, true },      // RES_CHRATR_AUTOKERN
    { SID_ATTR_FLASH, true },              // RES_CHRATR_BLINK
    { 0, true },                           // RES_CHRATR_UNUSED2
    { 0, true },                           // RES_CHRATR_NOHYPHEN
    { SID_ATTR_BRUSH_CHAR, true },         // RES_CHRATR_BACKGROUND
    { SID_ATTR_CHAR_CJK_FONT, true },      // RES_CHRATR_CJK_FONT
    { SID_ATTR_CHAR_CJK_FONTHEIGHT, true },// RES_CHRATR_CJK_FONTSIZE
    { SID_ATTR_CHAR_CJK_LANGUAGE, true },  // RES_CHRATR_CJK_LANGUAGE
    { SID_ATTR_CHAR_CJK_POSTURE, true },   // RES_CHRATR_CJK_POSTURE
    { SID_ATTR_CHAR_CJK_WEIGHT, true },    // RES_CHRATR_CJK_WEIGHT
    { SID_ATTR_CHAR_CTL_FONT, true },      // RES_CHRATR_CTL_FONT
    { SID_ATTR_CHAR_CTL_FONTHEIGHT, true },// RES_CHRATR_CTL_FONTSIZE
    { SID_ATTR_CHAR_CTL_LANGUAGE, true },  // RES_CHRATR_CTL_LANGUAGE
    { SID_ATTR_CHAR_CTL_POSTURE, true },   // RES_CHRATR_CTL_POSTURE
    { SID_ATTR_CHAR_CTL_WEIGHT, true },    // RES_CHRATR_CTL_WEIGHT
    { SID_ATTR_CHAR_ROTATED, true },       // RES_CHRATR_ROTATE
    { SID_ATTR_CHAR_EMPHASISMARK, true },  // RES_CHRATR_EMPHASIS_MARK
    { SID_ATTR_CHAR_TWO_LINES, true },     // RES_CHRATR_TWO_LINES
    { SID_ATTR_CHAR_SCALEWIDTH, true },    // RES_CHRATR_SCALEW
    { SID_ATTR_CHAR_RELIEF, true },        // RES_CHRATR_RELIEF
    { SID_ATTR_CHAR_HIDDEN, true },        // RES_CHRATR_HIDDEN
    { SID_ATTR_CHAR_OVERLINE, true },      // RES_CHRATR_OVERLINE
    { 0, true },                           // RES_CHRATR_RSID
    { SID_ATTR_CHAR_BOX, true },           // RES_CHRATR_BOX
    { SID_ATTR_CHAR_SHADOW, true },        // RES_CHRATR_SHADOW
    { 0, true },                           // RES_CHRATR_HIGHLIGHT
    { SID_ATTR_CHAR_GRABBAG, true },       // RES_CHRATR_GRABBAG
    { 0, true },                           // RES_CHRATR_BIDIRTL
    { 0, true },                           // RES_CHRATR_IDCTHINT

    { 0, false },                               // RES_TXTATR_REFMARK
    { 0, false },                               // RES_TXTATR_TOXMARK
    { 0, false },                               // RES_TXTATR_META
    { 0, false },                               // RES_TXTATR_METAFIELD
    { 0, true },                           // RES_TXTATR_AUTOFMT
    { FN_TXTATR_INET, false },                  // RES_TXTATR_INETFMT
    { 0, false },                               // RES_TXTATR_CHARFMT
    { SID_ATTR_CHAR_CJK_RUBY, false },          // RES_TXTATR_CJK_RUBY
    { 0, true },                           // RES_TXTATR_UNKNOWN_CONTAINER
    { 0, false },                               // RES_TXTATR_INPUTFIELD

    { 0, false },                               // RES_TXTATR_FIELD
    { 0, false },                               // RES_TXTATR_FLYCNT
    { 0, false },                               // RES_TXTATR_FTN
    { 0, false },                               // RES_TXTATR_ANNOTATION
    { 0, true },                           // RES_TXTATR_DUMMY3
    { 0, true },                           // RES_TXTATR_DUMMY1
    { 0, true },                           // RES_TXTATR_DUMMY2

    { SID_ATTR_PARA_LINESPACE, true },     // RES_PARATR_LINESPACING
    { SID_ATTR_PARA_ADJUST, true },        // RES_PARATR_ADJUST
    { SID_ATTR_PARA_SPLIT, true },         // RES_PARATR_SPLIT
    { SID_ATTR_PARA_ORPHANS, true },       // RES_PARATR_ORPHANS
    { SID_ATTR_PARA_WIDOWS, true },        // RES_PARATR_WIDOWS
    { SID_ATTR_TABSTOP, true },            // RES_PARATR_TABSTOP
    { SID_ATTR_PARA_HYPHENZONE, true },    // RES_PARATR_HYPHENZONE
    { FN_FORMAT_DROPCAPS, false },              // RES_PARATR_DROP
    { SID_ATTR_PARA_REGISTER, true },      // RES_PARATR_REGISTER
    { SID_ATTR_PARA_NUMRULE, true },       // RES_PARATR_NUMRULE
    { SID_ATTR_PARA_SCRIPTSPACE, true },   // RES_PARATR_SCRIPTSPACE
    { SID_ATTR_PARA_HANGPUNCTUATION, true },// RES_PARATR_HANGINGPUNCTUATION

    { SID_ATTR_PARA_FORBIDDEN_RULES, true },// RES_PARATR_FORBIDDEN_RULES
    { SID_PARA_VERTALIGN, true },          // RES_PARATR_VERTALIGN
    { SID_ATTR_PARA_SNAPTOGRID, true },    // RES_PARATR_SNAPTOGRID
    { SID_ATTR_BORDER_CONNECT, true },     // RES_PARATR_CONNECT_BORDER

    { SID_ATTR_PARA_OUTLINE_LEVEL, true }, // RES_PARATR_OUTLINELEVEL //#outline level
    { 0, true },                           // RES_PARATR_RSID
    { 0, true },                           // RES_PARATR_GRABBAG
    { 0, true },                           // RES_PARATR_LIST_ID
    { 0, true },                           // RES_PARATR_LIST_LEVEL
    { 0, true },                           // RES_PARATR_LIST_ISRESTART
    { 0, true },                           // RES_PARATR_LIST_RESTARTVALUE
    { 0, true },                           // RES_PARATR_LIST_ISCOUNTED

    { 0, true },                           // RES_FILL_ORDER
    { 0, true },                           // RES_FRM_SIZE
    { SID_ATTR_PAGE_PAPERBIN, true },      // RES_PAPER_BIN
    { SID_ATTR_LRSPACE, true },            // RES_LR_SPACE
    { SID_ATTR_ULSPACE, true },            // RES_UL_SPACE
    { 0, false },                               // RES_PAGEDESC
    { SID_ATTR_PARA_PAGEBREAK, true },     // RES_BREAK
    { 0, false },                               // RES_CNTNT
    { 0, true },                           // RES_HEADER
    { 0, true },                           // RES_FOOTER
    { 0, true },                           // RES_PRINT
    { FN_OPAQUE, true },                   // RES_OPAQUE
    { FN_SET_PROTECT, true },              // RES_PROTECT
    { FN_SURROUND, true },                 // RES_SURROUND
    { FN_VERT_ORIENT, true },              // RES_VERT_ORIENT
    { FN_HORI_ORIENT, true },              // RES_HORI_ORIENT
    { 0, false },                               // RES_ANCHOR
    { SID_ATTR_BRUSH, true },              // RES_BACKGROUND
    { SID_ATTR_BORDER_OUTER, true },       // RES_BOX
    { SID_ATTR_BORDER_SHADOW, true },      // RES_SHADOW
    { SID_ATTR_MACROITEM, true },          // RES_FRMMACRO
    { FN_ATTR_COLUMNS, true },             // RES_COL
    { SID_ATTR_PARA_KEEP, true },          // RES_KEEP
    { 0, true },                           // RES_URL
    { 0, true },                           // RES_EDIT_IN_READONLY

    { 0, true },                           // RES_LAYOUT_SPLIT
    { 0, false },                               // RES_CHAIN
    { 0, true },                           // RES_TEXTGRID
    { FN_FORMAT_LINENUMBER, true },        // RES_LINENUMBER
    { 0, true },                           // RES_FTN_AT_TXTEND
    { 0, true },                           // RES_END_AT_TXTEND
    { 0, true },                           // RES_COLUMNBALANCE

    { SID_ATTR_FRAMEDIRECTION, true },     // RES_FRAMEDIR

    { SID_ATTR_HDFT_DYNAMIC_SPACING, true },// RES_HEADER_FOOTER_EAT_SPACING
    { FN_TABLE_ROW_SPLIT, true },          // RES_ROW_SPLIT
    // #i18732# - use slot-id define in svx
    { SID_SW_FOLLOW_TEXT_FLOW, true },     // RES_FOLLOW_TEXT_FLOW
    // #i29550#
    { SID_SW_COLLAPSING_BORDERS, true },   // RES_COLLAPSING_BORDERS
    // #i28701#
    { SID_SW_WRAP_INFLUENCE_ON_OBJPOS, true },// RES_WRAP_INFLUENCE_ON_OBJPOS
    { 0, false },                               // RES_AUTO_STYLE
    { 0, true },                           // RES_FRMATR_STYLE_NAME
    { 0, true },                           // RES_FRMATR_CONDITIONAL_STYLE_NAME
    { 0, true },                           // RES_FRMATR_GRABBAG
    { 0, true },                           // RES_TEXT_VERT_ADJUST

    { 0, true },                           // RES_GRFATR_MIRRORGRF
    { SID_ATTR_GRAF_CROP, true },          // RES_GRFATR_CROPGRF
    { 0, true },                           // RES_GRFATR_ROTATION,
    { 0, true },                           // RES_GRFATR_LUMINANCE,
    { 0, true },                           // RES_GRFATR_CONTRAST,
    { 0, true },                           // RES_GRFATR_CHANNELR,
    { 0, true },                           // RES_GRFATR_CHANNELG,
    { 0, true },                           // RES_GRFATR_CHANNELB,
    { 0, true },                           // RES_GRFATR_GAMMA,
    { 0, true },                           // RES_GRFATR_INVERT,
    { 0, true },                           // RES_GRFATR_TRANSPARENCY,
    { 0, true },                           // RES_GRFATR_DUMMY1,
    { 0, true },                           // RES_GRFATR_DUMMY2,
    { 0, true },                           // RES_GRFATR_DUMMY3,
    { 0, true },                           // RES_GRFATR_DUMMY4,
    { 0, true },                           // RES_GRFATR_DUMMY5,
    { 0, true },                           // RES_GRFATR_DUMMY6,

    { 0, true },                           // RES_BOXATR_FORMAT
    { 0, false },                               // RES_BOXATR_FORMULA,
    { 0, true },                           // RES_BOXATR_VALUE

    { 0, true }                            // RES_UNKNOWNATR_CONTAINER
};

std::vector<SvGlobalName> *pGlobalOLEExcludeList = nullptr;

SwAutoCompleteWord* SwDoc::s_pAutoCompleteWords = nullptr;

SwCheckIt* pCheckIt = nullptr;
static CharClass* pAppCharClass = nullptr;

static CollatorWrapper* pCollator = nullptr,
                *pCaseCollator = nullptr;

salhelper::SingletonRef<SwCalendarWrapper>* s_getCalendarWrapper()
{
    static salhelper::SingletonRef<SwCalendarWrapper> aCalendarWrapper;
    return &aCalendarWrapper;
}

void InitCore()
{
    SfxPoolItem* pItem;

    aAttrTab[ RES_CHRATR_CASEMAP- POOLATTR_BEGIN ] =        new SvxCaseMapItem( SvxCaseMap::NotMapped, RES_CHRATR_CASEMAP);
    aAttrTab[ RES_CHRATR_CHARSETCOLOR- POOLATTR_BEGIN ] =   new SvxColorItem(RES_CHRATR_CHARSETCOLOR);
    aAttrTab[ RES_CHRATR_COLOR- POOLATTR_BEGIN ] =          new SvxColorItem(RES_CHRATR_COLOR);
    aAttrTab[ RES_CHRATR_CONTOUR- POOLATTR_BEGIN ] =        new SvxContourItem( false, RES_CHRATR_CONTOUR );
    aAttrTab[ RES_CHRATR_CROSSEDOUT- POOLATTR_BEGIN ] =     new SvxCrossedOutItem( STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT );
    aAttrTab[ RES_CHRATR_ESCAPEMENT- POOLATTR_BEGIN ] =     new SvxEscapementItem( RES_CHRATR_ESCAPEMENT );
    aAttrTab[ RES_CHRATR_FONT- POOLATTR_BEGIN ] =           new SvxFontItem( RES_CHRATR_FONT );

    aAttrTab[ RES_CHRATR_FONTSIZE- POOLATTR_BEGIN ] =       new SvxFontHeightItem( 240, 100, RES_CHRATR_FONTSIZE );
    aAttrTab[ RES_CHRATR_KERNING- POOLATTR_BEGIN ] =        new SvxKerningItem( 0, RES_CHRATR_KERNING );
    aAttrTab[ RES_CHRATR_LANGUAGE- POOLATTR_BEGIN ] =       new SvxLanguageItem(LANGUAGE_DONTKNOW, RES_CHRATR_LANGUAGE );
    aAttrTab[ RES_CHRATR_POSTURE- POOLATTR_BEGIN ] =        new SvxPostureItem( ITALIC_NONE, RES_CHRATR_POSTURE );
    aAttrTab[ RES_CHRATR_UNUSED1- POOLATTR_BEGIN ] =        new SfxVoidItem( RES_CHRATR_UNUSED1 );
    aAttrTab[ RES_CHRATR_SHADOWED- POOLATTR_BEGIN ] =       new SvxShadowedItem( false, RES_CHRATR_SHADOWED );
    aAttrTab[ RES_CHRATR_UNDERLINE- POOLATTR_BEGIN ] =      new SvxUnderlineItem( LINESTYLE_NONE, RES_CHRATR_UNDERLINE );
    aAttrTab[ RES_CHRATR_WEIGHT- POOLATTR_BEGIN ] =         new SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_WEIGHT );
    aAttrTab[ RES_CHRATR_RSID - POOLATTR_BEGIN ] =          new SvxRsidItem( 0, RES_CHRATR_RSID );
    aAttrTab[ RES_CHRATR_WORDLINEMODE- POOLATTR_BEGIN ] =   new SvxWordLineModeItem( false, RES_CHRATR_WORDLINEMODE );
    aAttrTab[ RES_CHRATR_AUTOKERN- POOLATTR_BEGIN ] =       new SvxAutoKernItem( false, RES_CHRATR_AUTOKERN );
    aAttrTab[ RES_CHRATR_BLINK - POOLATTR_BEGIN ] =         new SvxBlinkItem( false, RES_CHRATR_BLINK );
    aAttrTab[ RES_CHRATR_NOHYPHEN - POOLATTR_BEGIN ] =      new SvxNoHyphenItem( RES_CHRATR_NOHYPHEN );
    aAttrTab[ RES_CHRATR_UNUSED2- POOLATTR_BEGIN ] =        new SfxVoidItem( RES_CHRATR_UNUSED2 );
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

    aAttrTab[ RES_CHRATR_ROTATE - POOLATTR_BEGIN ] =        new SvxCharRotateItem( 0, false, RES_CHRATR_ROTATE );
    aAttrTab[ RES_CHRATR_EMPHASIS_MARK - POOLATTR_BEGIN ] = new SvxEmphasisMarkItem( FontEmphasisMark::NONE, RES_CHRATR_EMPHASIS_MARK );
    aAttrTab[ RES_CHRATR_TWO_LINES - POOLATTR_BEGIN ] =     new SvxTwoLinesItem( false, 0, 0, RES_CHRATR_TWO_LINES );
    aAttrTab[ RES_CHRATR_SCALEW - POOLATTR_BEGIN ] =        new SvxCharScaleWidthItem( 100, RES_CHRATR_SCALEW );
    aAttrTab[ RES_CHRATR_RELIEF - POOLATTR_BEGIN ] =        new SvxCharReliefItem( FontRelief::NONE, RES_CHRATR_RELIEF );
    aAttrTab[ RES_CHRATR_HIDDEN - POOLATTR_BEGIN ] =        new SvxCharHiddenItem( false, RES_CHRATR_HIDDEN );
    aAttrTab[ RES_CHRATR_OVERLINE- POOLATTR_BEGIN ] =       new SvxOverlineItem( LINESTYLE_NONE, RES_CHRATR_OVERLINE );
    aAttrTab[ RES_CHRATR_BOX - POOLATTR_BEGIN ] =           new SvxBoxItem( RES_CHRATR_BOX );
    aAttrTab[ RES_CHRATR_SHADOW - POOLATTR_BEGIN ] =        new SvxShadowItem( RES_CHRATR_SHADOW );
    aAttrTab[ RES_CHRATR_HIGHLIGHT - POOLATTR_BEGIN ] =     new SvxBrushItem( RES_CHRATR_HIGHLIGHT );
    aAttrTab[ RES_CHRATR_GRABBAG - POOLATTR_BEGIN ] =       new SfxGrabBagItem( RES_CHRATR_GRABBAG );

// CharacterAttr - MSWord weak char direction/script override emulation
    aAttrTab[ RES_CHRATR_BIDIRTL - POOLATTR_BEGIN ] = new SfxInt16Item( RES_CHRATR_BIDIRTL, sal_Int16(-1) );
    aAttrTab[ RES_CHRATR_IDCTHINT - POOLATTR_BEGIN ] = new SfxInt16Item( RES_CHRATR_IDCTHINT, sal_Int16(-1) );

    aAttrTab[ RES_TXTATR_REFMARK - POOLATTR_BEGIN ] =       new SwFormatRefMark( OUString() );
    aAttrTab[ RES_TXTATR_TOXMARK - POOLATTR_BEGIN ] =       new SwTOXMark;
    aAttrTab[ RES_TXTATR_META - POOLATTR_BEGIN ] =          SwFormatMeta::CreatePoolDefault(RES_TXTATR_META);
    aAttrTab[ RES_TXTATR_METAFIELD - POOLATTR_BEGIN ] =     SwFormatMeta::CreatePoolDefault(RES_TXTATR_METAFIELD);
    aAttrTab[ RES_TXTATR_AUTOFMT- POOLATTR_BEGIN ] =        new SwFormatAutoFormat;
    aAttrTab[ RES_TXTATR_INETFMT - POOLATTR_BEGIN ] =       new SwFormatINetFormat( OUString(), OUString() );
    aAttrTab[ RES_TXTATR_CHARFMT- POOLATTR_BEGIN ] =        new SwFormatCharFormat( nullptr );
    aAttrTab[ RES_TXTATR_CJK_RUBY - POOLATTR_BEGIN ] =      new SwFormatRuby( OUString() );
    aAttrTab[ RES_TXTATR_UNKNOWN_CONTAINER - POOLATTR_BEGIN ] = new SvXMLAttrContainerItem( RES_TXTATR_UNKNOWN_CONTAINER );
    aAttrTab[ RES_TXTATR_INPUTFIELD - POOLATTR_BEGIN ] = new SwFormatField( RES_TXTATR_INPUTFIELD );

    aAttrTab[ RES_TXTATR_FIELD- POOLATTR_BEGIN ] =          new SwFormatField( RES_TXTATR_FIELD );
    aAttrTab[ RES_TXTATR_FLYCNT - POOLATTR_BEGIN ] =        new SwFormatFlyCnt( nullptr );
    aAttrTab[ RES_TXTATR_FTN - POOLATTR_BEGIN ] =           new SwFormatFootnote;
    aAttrTab[ RES_TXTATR_ANNOTATION - POOLATTR_BEGIN ] = new SwFormatField( RES_TXTATR_ANNOTATION );

// TextAttr - Dummies
    aAttrTab[ RES_TXTATR_DUMMY1 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_TXTATR_DUMMY1 );
    aAttrTab[ RES_TXTATR_DUMMY2 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_TXTATR_DUMMY2 );
    aAttrTab[ RES_TXTATR_DUMMY3 - POOLATTR_BEGIN ] =        new SfxBoolItem( RES_TXTATR_DUMMY3 );

    aAttrTab[ RES_PARATR_LINESPACING- POOLATTR_BEGIN ] =    new SvxLineSpacingItem( LINE_SPACE_DEFAULT_HEIGHT, RES_PARATR_LINESPACING );
    aAttrTab[ RES_PARATR_ADJUST- POOLATTR_BEGIN ] =         new SvxAdjustItem( SvxAdjust::Left, RES_PARATR_ADJUST );
    aAttrTab[ RES_PARATR_SPLIT- POOLATTR_BEGIN ] =          new SvxFormatSplitItem( true, RES_PARATR_SPLIT );
    aAttrTab[ RES_PARATR_WIDOWS- POOLATTR_BEGIN ] =         new SvxWidowsItem( 0, RES_PARATR_WIDOWS );
    aAttrTab[ RES_PARATR_ORPHANS- POOLATTR_BEGIN ] =        new SvxOrphansItem( 0, RES_PARATR_ORPHANS );
    aAttrTab[ RES_PARATR_TABSTOP- POOLATTR_BEGIN ] =        new SvxTabStopItem( 1, SVX_TAB_DEFDIST, SvxTabAdjust::Default, RES_PARATR_TABSTOP );

    pItem = new SvxHyphenZoneItem( false, RES_PARATR_HYPHENZONE );
    static_cast<SvxHyphenZoneItem*>(pItem)->GetMaxHyphens() = 0; // Default: 0
    aAttrTab[ RES_PARATR_HYPHENZONE- POOLATTR_BEGIN ] =     pItem;

    aAttrTab[ RES_PARATR_DROP- POOLATTR_BEGIN ] =           new SwFormatDrop;
    aAttrTab[ RES_PARATR_REGISTER - POOLATTR_BEGIN ] =      new SwRegisterItem( false );
    aAttrTab[ RES_PARATR_NUMRULE - POOLATTR_BEGIN ] =       new SwNumRuleItem( OUString() );

    aAttrTab[ RES_PARATR_SCRIPTSPACE - POOLATTR_BEGIN ] =   new SvxScriptSpaceItem( true, RES_PARATR_SCRIPTSPACE );
    aAttrTab[ RES_PARATR_HANGINGPUNCTUATION - POOLATTR_BEGIN ] = new SvxHangingPunctuationItem( true, RES_PARATR_HANGINGPUNCTUATION );
    aAttrTab[ RES_PARATR_FORBIDDEN_RULES - POOLATTR_BEGIN ] = new SvxForbiddenRuleItem( true, RES_PARATR_FORBIDDEN_RULES );
    aAttrTab[ RES_PARATR_VERTALIGN - POOLATTR_BEGIN ] =     new SvxParaVertAlignItem( SvxParaVertAlignItem::Align::Automatic, RES_PARATR_VERTALIGN );
    aAttrTab[ RES_PARATR_SNAPTOGRID - POOLATTR_BEGIN ] =    new SvxParaGridItem( true, RES_PARATR_SNAPTOGRID );
    aAttrTab[ RES_PARATR_CONNECT_BORDER - POOLATTR_BEGIN ] = new SwParaConnectBorderItem;

    aAttrTab[ RES_PARATR_OUTLINELEVEL - POOLATTR_BEGIN ] =  new SfxUInt16Item( RES_PARATR_OUTLINELEVEL, 0 );
    aAttrTab[ RES_PARATR_RSID - POOLATTR_BEGIN ] =          new SvxRsidItem( 0, RES_PARATR_RSID );
    aAttrTab[ RES_PARATR_GRABBAG - POOLATTR_BEGIN ] =       new SfxGrabBagItem( RES_PARATR_GRABBAG );

    aAttrTab[ RES_PARATR_LIST_ID - POOLATTR_BEGIN ] =       new SfxStringItem( RES_PARATR_LIST_ID, OUString() );
    aAttrTab[ RES_PARATR_LIST_LEVEL - POOLATTR_BEGIN ] =    new SfxInt16Item( RES_PARATR_LIST_LEVEL, 0 );
    aAttrTab[ RES_PARATR_LIST_ISRESTART - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_LIST_ISRESTART, false );
    aAttrTab[ RES_PARATR_LIST_RESTARTVALUE - POOLATTR_BEGIN ] = new SfxInt16Item( RES_PARATR_LIST_RESTARTVALUE, 1 );
    aAttrTab[ RES_PARATR_LIST_ISCOUNTED - POOLATTR_BEGIN ] = new SfxBoolItem( RES_PARATR_LIST_ISCOUNTED, true );

    aAttrTab[ RES_FILL_ORDER- POOLATTR_BEGIN ] =            new SwFormatFillOrder;
    aAttrTab[ RES_FRM_SIZE- POOLATTR_BEGIN ] =              new SwFormatFrameSize;
    aAttrTab[ RES_PAPER_BIN- POOLATTR_BEGIN ] =             new SvxPaperBinItem( RES_PAPER_BIN );
    aAttrTab[ RES_LR_SPACE- POOLATTR_BEGIN ] =              new SvxLRSpaceItem( RES_LR_SPACE );
    aAttrTab[ RES_UL_SPACE- POOLATTR_BEGIN ] =              new SvxULSpaceItem( RES_UL_SPACE );
    aAttrTab[ RES_PAGEDESC- POOLATTR_BEGIN ] =              new SwFormatPageDesc;
    aAttrTab[ RES_BREAK- POOLATTR_BEGIN ] =                 new SvxFormatBreakItem( SvxBreak::NONE, RES_BREAK);
    aAttrTab[ RES_CNTNT- POOLATTR_BEGIN ] =                 new SwFormatContent;
    aAttrTab[ RES_HEADER- POOLATTR_BEGIN ] =                new SwFormatHeader;
    aAttrTab[ RES_FOOTER- POOLATTR_BEGIN ] =                new SwFormatFooter;
    aAttrTab[ RES_PRINT- POOLATTR_BEGIN ] =                 new SvxPrintItem( RES_PRINT );
    aAttrTab[ RES_OPAQUE- POOLATTR_BEGIN ] =                new SvxOpaqueItem( RES_OPAQUE );
    aAttrTab[ RES_PROTECT- POOLATTR_BEGIN ] =               new SvxProtectItem( RES_PROTECT );
    aAttrTab[ RES_SURROUND- POOLATTR_BEGIN ] =              new SwFormatSurround;
    aAttrTab[ RES_VERT_ORIENT- POOLATTR_BEGIN ] =           new SwFormatVertOrient;
    aAttrTab[ RES_HORI_ORIENT- POOLATTR_BEGIN ] =           new SwFormatHoriOrient;
    aAttrTab[ RES_ANCHOR- POOLATTR_BEGIN ] =                new SwFormatAnchor;
    aAttrTab[ RES_BACKGROUND- POOLATTR_BEGIN ] =            new SvxBrushItem( RES_BACKGROUND );
    aAttrTab[ RES_BOX- POOLATTR_BEGIN ] =                   new SvxBoxItem( RES_BOX );
    aAttrTab[ RES_SHADOW- POOLATTR_BEGIN ] =                new SvxShadowItem( RES_SHADOW );
    aAttrTab[ RES_FRMMACRO- POOLATTR_BEGIN ] =              new SvxMacroItem( RES_FRMMACRO );
    aAttrTab[ RES_COL- POOLATTR_BEGIN ] =                   new SwFormatCol;
    aAttrTab[ RES_KEEP - POOLATTR_BEGIN ] =                 new SvxFormatKeepItem( false, RES_KEEP );
    aAttrTab[ RES_URL - POOLATTR_BEGIN ] =                  new SwFormatURL();
    aAttrTab[ RES_EDIT_IN_READONLY - POOLATTR_BEGIN ] =     new SwFormatEditInReadonly;
    aAttrTab[ RES_LAYOUT_SPLIT - POOLATTR_BEGIN ] =         new SwFormatLayoutSplit;
    aAttrTab[ RES_CHAIN - POOLATTR_BEGIN ] =                new SwFormatChain;
    aAttrTab[ RES_TEXTGRID - POOLATTR_BEGIN ] =             new SwTextGridItem;
    aAttrTab[ RES_HEADER_FOOTER_EAT_SPACING - POOLATTR_BEGIN ] = new SwHeaderAndFooterEatSpacingItem;
    aAttrTab[ RES_LINENUMBER - POOLATTR_BEGIN ] =           new SwFormatLineNumber;
    aAttrTab[ RES_FTN_AT_TXTEND - POOLATTR_BEGIN ] =        new SwFormatFootnoteAtTextEnd;
    aAttrTab[ RES_END_AT_TXTEND - POOLATTR_BEGIN ] =        new SwFormatEndAtTextEnd;
    aAttrTab[ RES_COLUMNBALANCE - POOLATTR_BEGIN ] =        new SwFormatNoBalancedColumns;
    aAttrTab[ RES_FRAMEDIR - POOLATTR_BEGIN ] =             new SvxFrameDirectionItem( SvxFrameDirection::Environment, RES_FRAMEDIR );
    aAttrTab[ RES_ROW_SPLIT - POOLATTR_BEGIN ] =            new SwFormatRowSplit;

    // #i18732#
    aAttrTab[ RES_FOLLOW_TEXT_FLOW - POOLATTR_BEGIN ] =     new SwFormatFollowTextFlow(false);
    // collapsing borders #i29550#
    aAttrTab[ RES_COLLAPSING_BORDERS - POOLATTR_BEGIN ] =   new SfxBoolItem( RES_COLLAPSING_BORDERS, false );
    // #i28701#
    // #i35017# - constant name has changed
    aAttrTab[ RES_WRAP_INFLUENCE_ON_OBJPOS - POOLATTR_BEGIN ] = new SwFormatWrapInfluenceOnObjPos( text::WrapInfluenceOnPosition::ONCE_CONCURRENT );

    aAttrTab[ RES_AUTO_STYLE - POOLATTR_BEGIN ] =           new SwFormatAutoFormat( RES_AUTO_STYLE );
    aAttrTab[ RES_FRMATR_STYLE_NAME - POOLATTR_BEGIN ] =    new SfxStringItem( RES_FRMATR_STYLE_NAME, OUString());
    aAttrTab[ RES_FRMATR_CONDITIONAL_STYLE_NAME - POOLATTR_BEGIN ] = new SfxStringItem( RES_FRMATR_CONDITIONAL_STYLE_NAME, OUString() );
    aAttrTab[ RES_FRMATR_GRABBAG - POOLATTR_BEGIN ] = new SfxGrabBagItem(RES_FRMATR_GRABBAG);
    aAttrTab[ RES_TEXT_VERT_ADJUST - POOLATTR_BEGIN ] = new SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP,RES_TEXT_VERT_ADJUST);

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

    aAttrTab[ RES_BOXATR_FORMAT- POOLATTR_BEGIN ] =         new SwTableBoxNumFormat;
    aAttrTab[ RES_BOXATR_FORMULA- POOLATTR_BEGIN ] =        new SwTableBoxFormula( OUString() );
    aAttrTab[ RES_BOXATR_VALUE- POOLATTR_BEGIN ] =          new SwTableBoxValue;

    aAttrTab[ RES_UNKNOWNATR_CONTAINER- POOLATTR_BEGIN ] =
                new SvXMLAttrContainerItem( RES_UNKNOWNATR_CONTAINER );

    // get the correct fonts:
    ::GetDefaultFonts( *static_cast<SvxFontItem*>(aAttrTab[ RES_CHRATR_FONT- POOLATTR_BEGIN ]),
                       *static_cast<SvxFontItem*>(aAttrTab[ RES_CHRATR_CJK_FONT - POOLATTR_BEGIN ]),
                       *static_cast<SvxFontItem*>(aAttrTab[ RES_CHRATR_CTL_FONT - POOLATTR_BEGIN ]) );

    SwBreakIt::Create_( ::comphelper::getProcessComponentContext() );
    pCheckIt = nullptr;

    FrameInit();
    TextInit_();

    SwSelPaintRects::s_pMapMode = new MapMode;
    SwFntObj::pPixMap = new MapMode;

    pGlobalOLEExcludeList = new std::vector<SvGlobalName>;

    if (!utl::ConfigManager::IsFuzzing())
    {
        const SvxSwAutoFormatFlags& rAFlags = SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags();
        SwDoc::s_pAutoCompleteWords = new SwAutoCompleteWord( rAFlags.nAutoCmpltListLen,
                                            rAFlags.nAutoCmpltWordLen );
    }
    else
    {
        SwDoc::s_pAutoCompleteWords = new SwAutoCompleteWord( 0, 0 );
    }
}

void FinitCore()
{
    FrameFinit();
    TextFinit();

    sw::proofreadingiterator::dispose();
    SwBreakIt::Delete_();
    delete pCheckIt;
    delete pAppCharClass;
    delete pCollator;
    delete pCaseCollator;

    // destroy default TableAutoFormat
    delete SwTableAutoFormat::pDfltBoxAutoFormat;

    delete SwSelPaintRects::s_pMapMode;
    delete SwFntObj::pPixMap;

    delete SwEditShell::s_pAutoFormatFlags;

#if OSL_DEBUG_LEVEL > 0
    // free defaults to prevent assertions
    if ( aAttrTab[0]->GetRefCount() )
        SfxItemPool::ReleaseDefaults( &aAttrTab );
#endif
    delete SwDoc::s_pAutoCompleteWords;

    delete SwStyleNameMapper::s_pTextUINameArray;
    delete SwStyleNameMapper::s_pListsUINameArray;
    delete SwStyleNameMapper::s_pExtraUINameArray;
    delete SwStyleNameMapper::s_pRegisterUINameArray;
    delete SwStyleNameMapper::s_pDocUINameArray;
    delete SwStyleNameMapper::s_pHTMLUINameArray;
    delete SwStyleNameMapper::s_pFrameFormatUINameArray;
    delete SwStyleNameMapper::s_pChrFormatUINameArray;
    delete SwStyleNameMapper::s_pHTMLChrFormatUINameArray;
    delete SwStyleNameMapper::s_pPageDescUINameArray;
    delete SwStyleNameMapper::s_pNumRuleUINameArray;

    // Delete programmatic name arrays also
    delete SwStyleNameMapper::s_pTextProgNameArray;
    delete SwStyleNameMapper::s_pListsProgNameArray;
    delete SwStyleNameMapper::s_pExtraProgNameArray;
    delete SwStyleNameMapper::s_pRegisterProgNameArray;
    delete SwStyleNameMapper::s_pDocProgNameArray;
    delete SwStyleNameMapper::s_pHTMLProgNameArray;
    delete SwStyleNameMapper::s_pFrameFormatProgNameArray;
    delete SwStyleNameMapper::s_pChrFormatProgNameArray;
    delete SwStyleNameMapper::s_pHTMLChrFormatProgNameArray;
    delete SwStyleNameMapper::s_pPageDescProgNameArray;
    delete SwStyleNameMapper::s_pNumRuleProgNameArray;

    // And finally, any hash tables that we used
    delete SwStyleNameMapper::s_pParaUIMap;
    delete SwStyleNameMapper::s_pCharUIMap;
    delete SwStyleNameMapper::s_pPageUIMap;
    delete SwStyleNameMapper::s_pFrameUIMap;
    delete SwStyleNameMapper::s_pNumRuleUIMap;

    delete SwStyleNameMapper::s_pParaProgMap;
    delete SwStyleNameMapper::s_pCharProgMap;
    delete SwStyleNameMapper::s_pPageProgMap;
    delete SwStyleNameMapper::s_pFrameProgMap;
    delete SwStyleNameMapper::s_pNumRuleProgMap;

    // delete all default attributes
    for(SfxPoolItem* pHt : aAttrTab)
    {
        delete pHt;
    }

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

void SwCalendarWrapper::LoadDefaultCalendar( LanguageType eLang )
{
    if( eLang != m_nLang )
    {
        m_nLang = eLang;
        loadDefaultCalendar( LanguageTag::convertToLocale( m_nLang ));
    }
}

LanguageType GetAppLanguage()
{
    if (!utl::ConfigManager::IsFuzzing())
        return Application::GetSettings().GetLanguageTag().getLanguageType();
    return LANGUAGE_ENGLISH_US;
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
        std::unique_ptr<utl::TransliterationWrapper> m_xTransWrp;
    public:
        TransWrp()
        {
            uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

            m_xTransWrp.reset(new ::utl::TransliterationWrapper( xContext,
                    TransliterationFlags::IGNORE_CASE |
                    TransliterationFlags::IGNORE_KANA |
                    TransliterationFlags::IGNORE_WIDTH ));

            m_xTransWrp->loadModuleIfNeeded( GetAppLanguage() );
        }
        const ::utl::TransliterationWrapper& getTransliterationWrapper() const
        {
            return *m_xTransWrp;
        }
    };

    class theTransWrp : public rtl::Static<TransWrp, theTransWrp> {};
}

const ::utl::TransliterationWrapper& GetAppCmpStrIgnore()
{
    return theTransWrp::get().getTransliterationWrapper();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
