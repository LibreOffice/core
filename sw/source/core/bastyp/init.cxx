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
#include <proofreadingiterator.hxx>
#include <rtl/instance.hxx>
#include <svl/macitem.hxx>
#include <svx/dialogs.hrc>
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

    //UUUU FillAttribute support
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

    //UUUU FillAttribute support (paragraph FillStyle)
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

    //UUUU FillAttribute support (TextFrame, OLE, Writer GraphicObject)
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
SwDfltAttrTab aAttrTab;

SfxItemInfo aSlotTab[] =
{
    { SID_ATTR_CHAR_CASEMAP, SfxItemPoolFlags::POOLABLE },       // RES_CHRATR_CASEMAP
    { SID_ATTR_CHAR_CHARSETCOLOR, SfxItemPoolFlags::POOLABLE },  // RES_CHRATR_CHARSETCOLOR
    { SID_ATTR_CHAR_COLOR, SfxItemPoolFlags::POOLABLE },         // RES_CHRATR_COLOR
    { SID_ATTR_CHAR_CONTOUR, SfxItemPoolFlags::POOLABLE },       // RES_CHRATR_CONTOUR
    { SID_ATTR_CHAR_STRIKEOUT, SfxItemPoolFlags::POOLABLE },     // RES_CHRATR_CROSSEDOUT
    { SID_ATTR_CHAR_ESCAPEMENT, SfxItemPoolFlags::POOLABLE },    // RES_CHRATR_ESCAPEMENT
    { SID_ATTR_CHAR_FONT, SfxItemPoolFlags::POOLABLE },          // RES_CHRATR_FONT
    { SID_ATTR_CHAR_FONTHEIGHT, SfxItemPoolFlags::POOLABLE },    // RES_CHRATR_FONTSIZE
    { SID_ATTR_CHAR_KERNING, SfxItemPoolFlags::POOLABLE },       // RES_CHRATR_KERNING
    { SID_ATTR_CHAR_LANGUAGE, SfxItemPoolFlags::POOLABLE },      // RES_CHRATR_LANGUAGE
    { SID_ATTR_CHAR_POSTURE, SfxItemPoolFlags::POOLABLE },       // RES_CHRATR_POSTURE
    { SID_ATTR_CHAR_PROPSIZE, SfxItemPoolFlags::POOLABLE },      // RES_CHRATR_PROPORTIONALFONTSIZE
    { SID_ATTR_CHAR_SHADOWED, SfxItemPoolFlags::POOLABLE },      // RES_CHRATR_SHADOWED
    { SID_ATTR_CHAR_UNDERLINE, SfxItemPoolFlags::POOLABLE },     // RES_CHRATR_UNDERLINE
    { SID_ATTR_CHAR_WEIGHT, SfxItemPoolFlags::POOLABLE },        // RES_CHRATR_WEIGHT
    { SID_ATTR_CHAR_WORDLINEMODE, SfxItemPoolFlags::POOLABLE },  // RES_CHRATR_WORDLINEMODE
    { SID_ATTR_CHAR_AUTOKERN, SfxItemPoolFlags::POOLABLE },      // RES_CHRATR_AUTOKERN
    { SID_ATTR_FLASH, SfxItemPoolFlags::POOLABLE },              // RES_CHRATR_BLINK
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_CHRATR_NOLINEBREAK
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_CHRATR_NOHYPHEN
    { SID_ATTR_BRUSH_CHAR, SfxItemPoolFlags::POOLABLE },         // RES_CHRATR_BACKGROUND
    { SID_ATTR_CHAR_CJK_FONT, SfxItemPoolFlags::POOLABLE },      // RES_CHRATR_CJK_FONT
    { SID_ATTR_CHAR_CJK_FONTHEIGHT, SfxItemPoolFlags::POOLABLE },// RES_CHRATR_CJK_FONTSIZE
    { SID_ATTR_CHAR_CJK_LANGUAGE, SfxItemPoolFlags::POOLABLE },  // RES_CHRATR_CJK_LANGUAGE
    { SID_ATTR_CHAR_CJK_POSTURE, SfxItemPoolFlags::POOLABLE },   // RES_CHRATR_CJK_POSTURE
    { SID_ATTR_CHAR_CJK_WEIGHT, SfxItemPoolFlags::POOLABLE },    // RES_CHRATR_CJK_WEIGHT
    { SID_ATTR_CHAR_CTL_FONT, SfxItemPoolFlags::POOLABLE },      // RES_CHRATR_CTL_FONT
    { SID_ATTR_CHAR_CTL_FONTHEIGHT, SfxItemPoolFlags::POOLABLE },// RES_CHRATR_CTL_FONTSIZE
    { SID_ATTR_CHAR_CTL_LANGUAGE, SfxItemPoolFlags::POOLABLE },  // RES_CHRATR_CTL_LANGUAGE
    { SID_ATTR_CHAR_CTL_POSTURE, SfxItemPoolFlags::POOLABLE },   // RES_CHRATR_CTL_POSTURE
    { SID_ATTR_CHAR_CTL_WEIGHT, SfxItemPoolFlags::POOLABLE },    // RES_CHRATR_CTL_WEIGHT
    { SID_ATTR_CHAR_ROTATED, SfxItemPoolFlags::POOLABLE },       // RES_CHRATR_ROTATE
    { SID_ATTR_CHAR_EMPHASISMARK, SfxItemPoolFlags::POOLABLE },  // RES_CHRATR_EMPHASIS_MARK
    { SID_ATTR_CHAR_TWO_LINES, SfxItemPoolFlags::POOLABLE },     // RES_CHRATR_TWO_LINES
    { SID_ATTR_CHAR_SCALEWIDTH, SfxItemPoolFlags::POOLABLE },    // RES_CHRATR_SCALEW
    { SID_ATTR_CHAR_RELIEF, SfxItemPoolFlags::POOLABLE },        // RES_CHRATR_RELIEF
    { SID_ATTR_CHAR_HIDDEN, SfxItemPoolFlags::POOLABLE },        // RES_CHRATR_HIDDEN
    { SID_ATTR_CHAR_OVERLINE, SfxItemPoolFlags::POOLABLE },      // RES_CHRATR_OVERLINE
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_CHRATR_RSID
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_CHRATR_BOX
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_CHRATR_SHADOW
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_CHRATR_HIGHLIGHT
    { SID_ATTR_CHAR_GRABBAG, SfxItemPoolFlags::POOLABLE },       // RES_CHRATR_GRABBAG
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_CHRATR_BIDIRTL
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_CHRATR_IDCTHINT

    { 0, SfxItemPoolFlags::NONE },                               // RES_TXTATR_REFMARK
    { 0, SfxItemPoolFlags::NONE },                               // RES_TXTATR_TOXMARK
    { 0, SfxItemPoolFlags::NONE },                               // RES_TXTATR_META
    { 0, SfxItemPoolFlags::NONE },                               // RES_TXTATR_METAFIELD
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_TXTATR_AUTOFMT
    { FN_TXTATR_INET, SfxItemPoolFlags::NONE },                  // RES_TXTATR_INETFMT
    { 0, SfxItemPoolFlags::NONE },                               // RES_TXTATR_CHARFMT
    { SID_ATTR_CHAR_CJK_RUBY, SfxItemPoolFlags::NONE },          // RES_TXTATR_CJK_RUBY
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_TXTATR_UNKNOWN_CONTAINER
    { 0, SfxItemPoolFlags::NONE },                               // RES_TXTATR_INPUTFIELD

    { 0, SfxItemPoolFlags::NONE },                               // RES_TXTATR_FIELD
    { 0, SfxItemPoolFlags::NONE },                               // RES_TXTATR_FLYCNT
    { 0, SfxItemPoolFlags::NONE },                               // RES_TXTATR_FTN
    { 0, SfxItemPoolFlags::NONE },                               // RES_TXTATR_ANNOTATION
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_TXTATR_DUMMY3
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_TXTATR_DUMMY1
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_TXTATR_DUMMY2

    { SID_ATTR_PARA_LINESPACE, SfxItemPoolFlags::POOLABLE },     // RES_PARATR_LINESPACING
    { SID_ATTR_PARA_ADJUST, SfxItemPoolFlags::POOLABLE },        // RES_PARATR_ADJUST
    { SID_ATTR_PARA_SPLIT, SfxItemPoolFlags::POOLABLE },         // RES_PARATR_SPLIT
    { SID_ATTR_PARA_ORPHANS, SfxItemPoolFlags::POOLABLE },       // RES_PARATR_ORPHANS
    { SID_ATTR_PARA_WIDOWS, SfxItemPoolFlags::POOLABLE },        // RES_PARATR_WIDOWS
    { SID_ATTR_TABSTOP, SfxItemPoolFlags::POOLABLE },            // RES_PARATR_TABSTOP
    { SID_ATTR_PARA_HYPHENZONE, SfxItemPoolFlags::POOLABLE },    // RES_PARATR_HYPHENZONE
    { FN_FORMAT_DROPCAPS, SfxItemPoolFlags::NONE },              // RES_PARATR_DROP
    { SID_ATTR_PARA_REGISTER, SfxItemPoolFlags::POOLABLE },      // RES_PARATR_REGISTER
    { SID_ATTR_PARA_NUMRULE, SfxItemPoolFlags::POOLABLE },       // RES_PARATR_NUMRULE
    { SID_ATTR_PARA_SCRIPTSPACE, SfxItemPoolFlags::POOLABLE },   // RES_PARATR_SCRIPTSPACE
    { SID_ATTR_PARA_HANGPUNCTUATION, SfxItemPoolFlags::POOLABLE },// RES_PARATR_HANGINGPUNCTUATION

    { SID_ATTR_PARA_FORBIDDEN_RULES, SfxItemPoolFlags::POOLABLE },// RES_PARATR_FORBIDDEN_RULES
    { SID_PARA_VERTALIGN, SfxItemPoolFlags::POOLABLE },          // RES_PARATR_VERTALIGN
    { SID_ATTR_PARA_SNAPTOGRID, SfxItemPoolFlags::POOLABLE },    // RES_PARATR_SNAPTOGRID
    { SID_ATTR_BORDER_CONNECT, SfxItemPoolFlags::POOLABLE },     // RES_PARATR_CONNECT_BORDER

    { SID_ATTR_PARA_OUTLINE_LEVEL, SfxItemPoolFlags::POOLABLE }, // RES_PARATR_OUTLINELEVEL //#outline level
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_PARATR_RSID
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_PARATR_GRABBAG
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_PARATR_LIST_ID
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_PARATR_LIST_LEVEL
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_PARATR_LIST_ISRESTART
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_PARATR_LIST_RESTARTVALUE
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_PARATR_LIST_ISCOUNTED

    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_FILL_ORDER
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_FRM_SIZE
    { SID_ATTR_PAGE_PAPERBIN, SfxItemPoolFlags::POOLABLE },      // RES_PAPER_BIN
    { SID_ATTR_LRSPACE, SfxItemPoolFlags::POOLABLE },            // RES_LR_SPACE
    { SID_ATTR_ULSPACE, SfxItemPoolFlags::POOLABLE },            // RES_UL_SPACE
    { 0, SfxItemPoolFlags::NONE },                               // RES_PAGEDESC
    { SID_ATTR_PARA_PAGEBREAK, SfxItemPoolFlags::POOLABLE },     // RES_BREAK
    { 0, SfxItemPoolFlags::NONE },                               // RES_CNTNT
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_HEADER
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_FOOTER
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_PRINT
    { FN_OPAQUE, SfxItemPoolFlags::POOLABLE },                   // RES_OPAQUE
    { FN_SET_PROTECT, SfxItemPoolFlags::POOLABLE },              // RES_PROTECT
    { FN_SURROUND, SfxItemPoolFlags::POOLABLE },                 // RES_SURROUND
    { FN_VERT_ORIENT, SfxItemPoolFlags::POOLABLE },              // RES_VERT_ORIENT
    { FN_HORI_ORIENT, SfxItemPoolFlags::POOLABLE },              // RES_HORI_ORIENT
    { 0, SfxItemPoolFlags::NONE },                               // RES_ANCHOR
    { SID_ATTR_BRUSH, SfxItemPoolFlags::POOLABLE },              // RES_BACKGROUND
    { SID_ATTR_BORDER_OUTER, SfxItemPoolFlags::POOLABLE },       // RES_BOX
    { SID_ATTR_BORDER_SHADOW, SfxItemPoolFlags::POOLABLE },      // RES_SHADOW
    { SID_ATTR_MACROITEM, SfxItemPoolFlags::POOLABLE },          // RES_FRMMACRO
    { FN_ATTR_COLUMNS, SfxItemPoolFlags::POOLABLE },             // RES_COL
    { SID_ATTR_PARA_KEEP, SfxItemPoolFlags::POOLABLE },          // RES_KEEP
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_URL
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_EDIT_IN_READONLY

    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_LAYOUT_SPLIT
    { 0, SfxItemPoolFlags::NONE },                               // RES_CHAIN
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_TEXTGRID
    { FN_FORMAT_LINENUMBER, SfxItemPoolFlags::POOLABLE },        // RES_LINENUMBER
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_FTN_AT_TXTEND
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_END_AT_TXTEND
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_COLUMNBALANCE

    { SID_ATTR_FRAMEDIRECTION, SfxItemPoolFlags::POOLABLE },     // RES_FRAMEDIR

    { SID_ATTR_HDFT_DYNAMIC_SPACING, SfxItemPoolFlags::POOLABLE },// RES_HEADER_FOOTER_EAT_SPACING
    { FN_TABLE_ROW_SPLIT, SfxItemPoolFlags::POOLABLE },          // RES_ROW_SPLIT
    // #i18732# - use slot-id define in svx
    { SID_SW_FOLLOW_TEXT_FLOW, SfxItemPoolFlags::POOLABLE },     // RES_FOLLOW_TEXT_FLOW
    // #i29550#
    { SID_SW_COLLAPSING_BORDERS, SfxItemPoolFlags::POOLABLE },   // RES_COLLAPSING_BORDERS
    // #i28701#
    { SID_SW_WRAP_INFLUENCE_ON_OBJPOS, SfxItemPoolFlags::POOLABLE },// RES_WRAP_INFLUENCE_ON_OBJPOS
    { 0, SfxItemPoolFlags::NONE },                               // RES_AUTO_STYLE
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_FRMATR_STYLE_NAME
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_FRMATR_CONDITIONAL_STYLE_NAME
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_FRMATR_GRABBAG
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_TEXT_VERT_ADJUST

    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_MIRRORGRF
    { SID_ATTR_GRAF_CROP, SfxItemPoolFlags::POOLABLE },          // RES_GRFATR_CROPGRF
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_ROTATION,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_LUMINANCE,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_CONTRAST,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_CHANNELR,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_CHANNELG,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_CHANNELB,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_GAMMA,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_INVERT,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_TRANSPARENCY,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_DUMMY1,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_DUMMY2,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_DUMMY3,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_DUMMY4,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_DUMMY5,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_GRFATR_DUMMY6,

    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_BOXATR_FORMAT
    { 0, SfxItemPoolFlags::NONE },                               // RES_BOXATR_FORMULA,
    { 0, SfxItemPoolFlags::POOLABLE },                           // RES_BOXATR_VALUE

    { 0, SfxItemPoolFlags::POOLABLE }                            // RES_UNKNOWNATR_CONTAINER
};

sal_uInt16* SwAttrPool::pVersionMap1 = nullptr;
sal_uInt16* SwAttrPool::pVersionMap2 = nullptr;
sal_uInt16* SwAttrPool::pVersionMap3 = nullptr;
sal_uInt16* SwAttrPool::pVersionMap4 = nullptr;
// #i18732#
sal_uInt16* SwAttrPool::pVersionMap5 = nullptr;
sal_uInt16* SwAttrPool::pVersionMap6 = nullptr;
sal_uInt16* SwAttrPool::pVersionMap7 = nullptr;

std::vector<SvGlobalName*> *pGlobalOLEExcludeList = nullptr;

SwAutoCompleteWord* SwDoc::mpACmpltWords = nullptr;

SwCheckIt* pCheckIt = nullptr;
CharClass* pAppCharClass = nullptr;

CollatorWrapper* pCollator = nullptr,
                *pCaseCollator = nullptr;

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
    aAttrTab[ RES_CHRATR_CONTOUR- POOLATTR_BEGIN ] =        new SvxContourItem( false, RES_CHRATR_CONTOUR );
    aAttrTab[ RES_CHRATR_CROSSEDOUT- POOLATTR_BEGIN ] =     new SvxCrossedOutItem( STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT );
    aAttrTab[ RES_CHRATR_ESCAPEMENT- POOLATTR_BEGIN ] =     new SvxEscapementItem( RES_CHRATR_ESCAPEMENT );
    aAttrTab[ RES_CHRATR_FONT- POOLATTR_BEGIN ] =           new SvxFontItem( RES_CHRATR_FONT );

    aAttrTab[ RES_CHRATR_FONTSIZE- POOLATTR_BEGIN ] =       new SvxFontHeightItem( 240, 100, RES_CHRATR_FONTSIZE );
    aAttrTab[ RES_CHRATR_KERNING- POOLATTR_BEGIN ] =        new SvxKerningItem( 0, RES_CHRATR_KERNING );
    aAttrTab[ RES_CHRATR_LANGUAGE- POOLATTR_BEGIN ] =       new SvxLanguageItem(LANGUAGE_DONTKNOW, RES_CHRATR_LANGUAGE );
    aAttrTab[ RES_CHRATR_POSTURE- POOLATTR_BEGIN ] =        new SvxPostureItem( ITALIC_NONE, RES_CHRATR_POSTURE );
    aAttrTab[ RES_CHRATR_PROPORTIONALFONTSIZE- POOLATTR_BEGIN ] = new SvxPropSizeItem( 100, RES_CHRATR_PROPORTIONALFONTSIZE );
    aAttrTab[ RES_CHRATR_SHADOWED- POOLATTR_BEGIN ] =       new SvxShadowedItem( false, RES_CHRATR_SHADOWED );
    aAttrTab[ RES_CHRATR_UNDERLINE- POOLATTR_BEGIN ] =      new SvxUnderlineItem( UNDERLINE_NONE, RES_CHRATR_UNDERLINE );
    aAttrTab[ RES_CHRATR_WEIGHT- POOLATTR_BEGIN ] =         new SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_WEIGHT );
    aAttrTab[ RES_CHRATR_RSID - POOLATTR_BEGIN ] =          new SvxRsidItem( 0, RES_CHRATR_RSID );
    aAttrTab[ RES_CHRATR_WORDLINEMODE- POOLATTR_BEGIN ] =   new SvxWordLineModeItem( false, RES_CHRATR_WORDLINEMODE );
    aAttrTab[ RES_CHRATR_AUTOKERN- POOLATTR_BEGIN ] =       new SvxAutoKernItem( false, RES_CHRATR_AUTOKERN );
    aAttrTab[ RES_CHRATR_BLINK - POOLATTR_BEGIN ] =         new SvxBlinkItem( false, RES_CHRATR_BLINK );
    aAttrTab[ RES_CHRATR_NOHYPHEN - POOLATTR_BEGIN ] =      new SvxNoHyphenItem( true, RES_CHRATR_NOHYPHEN );
    aAttrTab[ RES_CHRATR_NOLINEBREAK- POOLATTR_BEGIN ] =    new SvxNoLinebreakItem( true, RES_CHRATR_NOLINEBREAK );
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
    aAttrTab[ RES_CHRATR_EMPHASIS_MARK - POOLATTR_BEGIN ] = new SvxEmphasisMarkItem( EMPHASISMARK_NONE, RES_CHRATR_EMPHASIS_MARK );
    aAttrTab[ RES_CHRATR_TWO_LINES - POOLATTR_BEGIN ] =     new SvxTwoLinesItem( false, 0, 0, RES_CHRATR_TWO_LINES );
    aAttrTab[ RES_CHRATR_SCALEW - POOLATTR_BEGIN ] =        new SvxCharScaleWidthItem( 100, RES_CHRATR_SCALEW );
    aAttrTab[ RES_CHRATR_RELIEF - POOLATTR_BEGIN ] =        new SvxCharReliefItem( RELIEF_NONE, RES_CHRATR_RELIEF );
    aAttrTab[ RES_CHRATR_HIDDEN - POOLATTR_BEGIN ] =        new SvxCharHiddenItem( false, RES_CHRATR_HIDDEN );
    aAttrTab[ RES_CHRATR_OVERLINE- POOLATTR_BEGIN ] =       new SvxOverlineItem( UNDERLINE_NONE, RES_CHRATR_OVERLINE );
    aAttrTab[ RES_CHRATR_BOX - POOLATTR_BEGIN ] =           new SvxBoxItem( RES_CHRATR_BOX );
    aAttrTab[ RES_CHRATR_SHADOW - POOLATTR_BEGIN ] =        new SvxShadowItem( RES_CHRATR_SHADOW );
    aAttrTab[ RES_CHRATR_HIGHLIGHT - POOLATTR_BEGIN ] =     new SvxBrushItem( RES_CHRATR_HIGHLIGHT );
    aAttrTab[ RES_CHRATR_GRABBAG - POOLATTR_BEGIN ] =       new SfxGrabBagItem( RES_CHRATR_GRABBAG );

// CharakterAttr - MSWord weak char direction/script override emulation
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
    aAttrTab[ RES_PARATR_ADJUST- POOLATTR_BEGIN ] =         new SvxAdjustItem( SVX_ADJUST_LEFT, RES_PARATR_ADJUST );
    aAttrTab[ RES_PARATR_SPLIT- POOLATTR_BEGIN ] =          new SvxFormatSplitItem( true, RES_PARATR_SPLIT );
    aAttrTab[ RES_PARATR_WIDOWS- POOLATTR_BEGIN ] =         new SvxWidowsItem( 0, RES_PARATR_WIDOWS );
    aAttrTab[ RES_PARATR_ORPHANS- POOLATTR_BEGIN ] =        new SvxOrphansItem( 0, RES_PARATR_ORPHANS );
    aAttrTab[ RES_PARATR_TABSTOP- POOLATTR_BEGIN ] =        new SvxTabStopItem( 1, SVX_TAB_DEFDIST, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );

    pItem = new SvxHyphenZoneItem( false, RES_PARATR_HYPHENZONE );
    static_cast<SvxHyphenZoneItem*>(pItem)->GetMaxHyphens() = 0; // Default: 0
    aAttrTab[ RES_PARATR_HYPHENZONE- POOLATTR_BEGIN ] =     pItem;

    aAttrTab[ RES_PARATR_DROP- POOLATTR_BEGIN ] =           new SwFormatDrop;
    aAttrTab[ RES_PARATR_REGISTER - POOLATTR_BEGIN ] =      new SwRegisterItem( false );
    aAttrTab[ RES_PARATR_NUMRULE - POOLATTR_BEGIN ] =       new SwNumRuleItem( OUString() );

    aAttrTab[ RES_PARATR_SCRIPTSPACE - POOLATTR_BEGIN ] =   new SvxScriptSpaceItem( true, RES_PARATR_SCRIPTSPACE );
    aAttrTab[ RES_PARATR_HANGINGPUNCTUATION - POOLATTR_BEGIN ] = new SvxHangingPunctuationItem( true, RES_PARATR_HANGINGPUNCTUATION );
    aAttrTab[ RES_PARATR_FORBIDDEN_RULES - POOLATTR_BEGIN ] = new SvxForbiddenRuleItem( true, RES_PARATR_FORBIDDEN_RULES );
    aAttrTab[ RES_PARATR_VERTALIGN - POOLATTR_BEGIN ] =     new SvxParaVertAlignItem( 0, RES_PARATR_VERTALIGN );
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
    aAttrTab[ RES_FRM_SIZE- POOLATTR_BEGIN ] =              new SwFormatFrmSize;
    aAttrTab[ RES_PAPER_BIN- POOLATTR_BEGIN ] =             new SvxPaperBinItem( RES_PAPER_BIN );
    aAttrTab[ RES_LR_SPACE- POOLATTR_BEGIN ] =              new SvxLRSpaceItem( RES_LR_SPACE );
    aAttrTab[ RES_UL_SPACE- POOLATTR_BEGIN ] =              new SvxULSpaceItem( RES_UL_SPACE );
    aAttrTab[ RES_PAGEDESC- POOLATTR_BEGIN ] =              new SwFormatPageDesc;
    aAttrTab[ RES_BREAK- POOLATTR_BEGIN ] =                 new SvxFormatBreakItem( SVX_BREAK_NONE, RES_BREAK);
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
    aAttrTab[ RES_FRAMEDIR - POOLATTR_BEGIN ] =             new SvxFrameDirectionItem( FRMDIR_ENVIRONMENT, RES_FRAMEDIR );
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

    // 7. version:
    // New character attribute for character box shadow plus 3 dummies
    SwAttrPool::pVersionMap7 = new sal_uInt16[ 144 ];
    for( i = 1; i <= 40; ++i )
        SwAttrPool::pVersionMap7[ i-1 ] = i;
    for ( i = 41; i <= 144; ++i )
        SwAttrPool::pVersionMap7[ i-1 ] = i + 4;

    SwBreakIt::_Create( ::comphelper::getProcessComponentContext() );
    pCheckIt = nullptr;

    _FrmInit();
    _TextInit();

    SwSelPaintRects::s_pMapMode = new MapMode;
    SwFntObj::pPixMap = new MapMode;

    pGlobalOLEExcludeList = new std::vector<SvGlobalName*>;

    if (!utl::ConfigManager::IsAvoidConfig())
    {
        const SvxSwAutoFormatFlags& rAFlags = SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags();
        SwDoc::mpACmpltWords = new SwAutoCompleteWord( rAFlags.nAutoCmpltListLen,
                                            rAFlags.nAutoCmpltWordLen );
    }
    else
    {
        SwDoc::mpACmpltWords = new SwAutoCompleteWord( 0, 0 );
    }
}

void _FinitCore()
{
    _FrmFinit();
    _TextFinit();

    sw::proofreadingiterator::dispose();
    SwBreakIt::_Delete();
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
        SfxItemPool::ReleaseDefaults( aAttrTab, POOLATTR_END-POOLATTR_BEGIN);
#endif
    delete SwDoc::mpACmpltWords;

    delete SwStyleNameMapper::m_pTextUINameArray;
    delete SwStyleNameMapper::m_pListsUINameArray;
    delete SwStyleNameMapper::m_pExtraUINameArray;
    delete SwStyleNameMapper::m_pRegisterUINameArray;
    delete SwStyleNameMapper::m_pDocUINameArray;
    delete SwStyleNameMapper::m_pHTMLUINameArray;
    delete SwStyleNameMapper::m_pFrameFormatUINameArray;
    delete SwStyleNameMapper::m_pChrFormatUINameArray;
    delete SwStyleNameMapper::m_pHTMLChrFormatUINameArray;
    delete SwStyleNameMapper::m_pPageDescUINameArray;
    delete SwStyleNameMapper::m_pNumRuleUINameArray;

    // Delete programmatic name arrays also
    delete SwStyleNameMapper::m_pTextProgNameArray;
    delete SwStyleNameMapper::m_pListsProgNameArray;
    delete SwStyleNameMapper::m_pExtraProgNameArray;
    delete SwStyleNameMapper::m_pRegisterProgNameArray;
    delete SwStyleNameMapper::m_pDocProgNameArray;
    delete SwStyleNameMapper::m_pHTMLProgNameArray;
    delete SwStyleNameMapper::m_pFrameFormatProgNameArray;
    delete SwStyleNameMapper::m_pChrFormatProgNameArray;
    delete SwStyleNameMapper::m_pHTMLChrFormatProgNameArray;
    delete SwStyleNameMapper::m_pPageDescProgNameArray;
    delete SwStyleNameMapper::m_pNumRuleProgNameArray;

    // And finally, any hash tables that we used
    delete SwStyleNameMapper::m_pParaUIMap;
    delete SwStyleNameMapper::m_pCharUIMap;
    delete SwStyleNameMapper::m_pPageUIMap;
    delete SwStyleNameMapper::m_pFrameUIMap;
    delete SwStyleNameMapper::m_pNumRuleUIMap;

    delete SwStyleNameMapper::m_pParaProgMap;
    delete SwStyleNameMapper::m_pCharProgMap;
    delete SwStyleNameMapper::m_pPageProgMap;
    delete SwStyleNameMapper::m_pFrameProgMap;
    delete SwStyleNameMapper::m_pNumRuleProgMap;

    // delete all default attributes
    for( sal_uInt16 n = 0; n < POOLATTR_END - POOLATTR_BEGIN; n++ )
    {
        SfxPoolItem* pHt;
        if( nullptr != ( pHt = aAttrTab[n] ))
            delete pHt;
    }

    ::ClearFEShellTabCols();

    delete[] SwAttrPool::pVersionMap1;
    delete[] SwAttrPool::pVersionMap2;
    delete[] SwAttrPool::pVersionMap3;
    delete[] SwAttrPool::pVersionMap4;
    // #i18732#
    delete[] SwAttrPool::pVersionMap5;
    delete[] SwAttrPool::pVersionMap6;
    delete[] SwAttrPool::pVersionMap7;

    for ( size_t i = 0; i < pGlobalOLEExcludeList->size(); ++i )
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
    sUniqueId.clear();
    if( eLang != nLang )
        loadDefaultCalendar( LanguageTag::convertToLocale( nLang = eLang ));
}

LanguageType GetAppLanguage()
{
    if (!utl::ConfigManager::IsAvoidConfig())
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
        std::unique_ptr<utl::TransliterationWrapper> xTransWrp;
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
