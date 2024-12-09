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
#include <comphelper/processfactory.hxx>
#include <doc.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
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
#include <editeng/postitem.hxx>
#include <editeng/rsiditem.hxx>
#include <svl/grabbagitem.hxx>
#include <svl/voiditem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/swafopt.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <i18nutil/transliteration.hxx>
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
#include <formatlinebreak.hxx>
#include <fmthdft.hxx>
#include <fmtinfmt.hxx>
#include <fmtline.hxx>
#include <fmtlsplt.hxx>
#include <fmtmeta.hxx>
#include <formatcontentcontrol.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <fmtrfmrk.hxx>
#include <fmtrowsplt.hxx>
#include <formatflysplit.hxx>
#include <formatwraptextatflystart.hxx>
#include <fmtruby.hxx>
#include <fmtsrnd.hxx>
#include <fmturl.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <fntcache.hxx>
#include <grfatr.hxx>
#include <hfspacingitem.hxx>
#include <hintids.hxx>
#include <init.hxx>
#include <paratr.hxx>
#include <proofreadingiterator.hxx>
#include <editeng/editids.hrc>
#include <svl/macitem.hxx>
#include <svx/sdtaitm.hxx>
#include <swcalwrp.hxx>
#include <SwStyleNameMapper.hxx>
#include <tblafmt.hxx>
#include <tgrditem.hxx>
#include <tools/globname.hxx>
#include <tox.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/configuration.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <salhelper/singletonref.hxx>
#include <viscrs.hxx>

using namespace ::com::sun::star;

// some ranges for sets in collections/ nodes

// AttrSet range for the 2 break attributes
WhichRangesContainer const aBreakSetRange(svl::Items<
    RES_PAGEDESC, RES_BREAK
>);

// AttrSet range for TextFormatColl
// list attributes ( RES_PARATR_LIST_BEGIN - RES_PARATR_LIST_END ) are not
// included in the paragraph style's itemset.
WhichRangesContainer const aTextFormatCollSetRange(svl::Items<
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    RES_PARATR_LIST_LEVEL, RES_PARATR_LIST_LEVEL,
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,

    // FillAttribute support
    XATTR_FILL_FIRST, XATTR_FILL_LAST

>);

// AttrSet range for GrfFormatColl
WhichRangesContainer const aGrfFormatCollSetRange(svl::Items<
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_GRFATR_BEGIN, RES_GRFATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1
>);

// AttrSet range for TextNode
WhichRangesContainer const aTextNodeSetRange(svl::Items<
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_PARATR_BEGIN, RES_PARATR_END-1,
    RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END-1,
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,

    // FillAttribute support (paragraph FillStyle)
    XATTR_FILL_FIRST, XATTR_FILL_LAST

>);

// AttrSet range for NoTextNode
WhichRangesContainer const aNoTextNodeSetRange(svl::Items<
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_GRFATR_BEGIN, RES_GRFATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1
>);

WhichRangesContainer const aTableSetRange(svl::Items<
    RES_FILL_ORDER,     RES_FRM_SIZE,
    RES_LR_SPACE,       RES_BREAK,
    RES_HORI_ORIENT,    RES_HORI_ORIENT,
    RES_BACKGROUND,     RES_SHADOW,
    RES_KEEP,           RES_KEEP,
    RES_LAYOUT_SPLIT,   RES_LAYOUT_SPLIT,
    RES_FRAMEDIR,       RES_FRAMEDIR,
    // #i29550#
    RES_COLLAPSING_BORDERS, RES_COLLAPSING_BORDERS,
    // <-- collapsing
    RES_FRMATR_GRABBAG, RES_FRMATR_GRABBAG,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1
>);

WhichRangesContainer const aTableLineSetRange(svl::Items<
    RES_FILL_ORDER,     RES_FRM_SIZE,
    RES_LR_SPACE,       RES_UL_SPACE,
    // HasTextChangesOnly
    RES_PRINT,          RES_PRINT,
    RES_PROTECT,        RES_PROTECT,
    RES_VERT_ORIENT,    RES_VERT_ORIENT,
    RES_BACKGROUND,     RES_SHADOW,
    RES_ROW_SPLIT,      RES_ROW_SPLIT,
    RES_FRMATR_GRABBAG, RES_FRMATR_GRABBAG,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1
>);

WhichRangesContainer const aTableBoxSetRange(svl::Items<
    RES_FILL_ORDER,     RES_FRM_SIZE,
    RES_LR_SPACE,       RES_UL_SPACE,
    // HasTextChangesOnly
    RES_PRINT,          RES_PRINT,
    RES_PROTECT,        RES_PROTECT,
    RES_VERT_ORIENT,    RES_VERT_ORIENT,
    RES_BACKGROUND,     RES_SHADOW,
    RES_FRAMEDIR,       RES_FRAMEDIR,
    RES_FRMATR_GRABBAG, RES_FRMATR_GRABBAG,
    RES_BOXATR_BEGIN,   RES_BOXATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1
>);

// AttrSet range for SwFrameFormat
WhichRangesContainer const aFrameFormatSetRange(svl::Items<
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,

    // FillAttribute support (TextFrame, OLE, Writer GraphicObject)
    XATTR_FILL_FIRST, XATTR_FILL_LAST

>);

// AttrSet range for SwCharFormat
WhichRangesContainer const aCharFormatSetRange(svl::Items<
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1
>);

// AttrSet range for character autostyles
WhichRangesContainer const aCharAutoFormatSetRange(svl::Items<
    RES_CHRATR_BEGIN, RES_CHRATR_END-1,
    RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1
>);

// AttrSet range for SwPageDescFormat
WhichRangesContainer const aPgFrameFormatSetRange(svl::Items<
    RES_FRMATR_BEGIN, RES_FRMATR_END-1,
    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1
>);

SwFormatField* createSwFormatFieldForItemInfoPackage(sal_uInt16 nWhich)
{
    return new SwFormatField(nWhich);
}

SwTOXMark* createSwTOXMarkForItemInfoPackage()
{
    return new SwTOXMark();
}

ItemInfoPackage& getItemInfoPackageSwAttributes()
{
    class ItemInfoPackageSwAttributes : public ItemInfoPackage
    {
        typedef std::array<ItemInfoStatic, POOLATTR_END - POOLATTR_BEGIN> ItemInfoArrayWriter;
        ItemInfoArrayWriter maItemInfos {{
            // m_nWhich, m_pItem, m_nSlotID, m_nItemInfoFlags
            { RES_CHRATR_CASEMAP, new SvxCaseMapItem( SvxCaseMap::NotMapped, RES_CHRATR_CASEMAP), SID_ATTR_CHAR_CASEMAP, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_CHARSETCOLOR, new SvxColorItem(RES_CHRATR_CHARSETCOLOR), SID_ATTR_CHAR_CHARSETCOLOR, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_COLOR, new SvxColorItem(RES_CHRATR_COLOR), SID_ATTR_CHAR_COLOR, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_CONTOUR, new SvxContourItem( false, RES_CHRATR_CONTOUR ), SID_ATTR_CHAR_CONTOUR, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_CROSSEDOUT, new SvxCrossedOutItem( STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT ), SID_ATTR_CHAR_STRIKEOUT, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_ESCAPEMENT, new SvxEscapementItem( RES_CHRATR_ESCAPEMENT ), SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_FONT, nullptr, SID_ATTR_CHAR_FONT, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_FONTSIZE, new SvxFontHeightItem( 240, 100, RES_CHRATR_FONTSIZE ), SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_KERNING, new SvxKerningItem( 0, RES_CHRATR_KERNING ), SID_ATTR_CHAR_KERNING, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_LANGUAGE, new SvxLanguageItem(LANGUAGE_DONTKNOW, RES_CHRATR_LANGUAGE ), SID_ATTR_CHAR_LANGUAGE, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_POSTURE, new SvxPostureItem( ITALIC_NONE, RES_CHRATR_POSTURE ), SID_ATTR_CHAR_POSTURE, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_UNUSED1, new SfxVoidItem( RES_CHRATR_UNUSED1 ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_SHADOWED, new SvxShadowedItem( false, RES_CHRATR_SHADOWED ), SID_ATTR_CHAR_SHADOWED, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_UNDERLINE, new SvxUnderlineItem( LINESTYLE_NONE, RES_CHRATR_UNDERLINE ), SID_ATTR_CHAR_UNDERLINE, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_WEIGHT, new SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_WEIGHT ), SID_ATTR_CHAR_WEIGHT, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_WORDLINEMODE, new SvxWordLineModeItem( false, RES_CHRATR_WORDLINEMODE ), SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_AUTOKERN, new SvxAutoKernItem( false, RES_CHRATR_AUTOKERN ), SID_ATTR_CHAR_AUTOKERN, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_BLINK, new SvxBlinkItem( false, RES_CHRATR_BLINK ), SID_ATTR_FLASH, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_NOHYPHEN, new SvxNoHyphenItem( false, RES_CHRATR_NOHYPHEN ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_UNUSED2, new SfxVoidItem( RES_CHRATR_UNUSED2 ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_BACKGROUND, new SvxBrushItem( RES_CHRATR_BACKGROUND ), SID_ATTR_BRUSH_CHAR, SFX_ITEMINFOFLAG_NONE },

            // CJK-Attributes
            { RES_CHRATR_CJK_FONT, nullptr, SID_ATTR_CHAR_CJK_FONT, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_CJK_FONTSIZE, new SvxFontHeightItem( 240, 100, RES_CHRATR_CJK_FONTSIZE ), SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_CJK_LANGUAGE, new SvxLanguageItem(LANGUAGE_DONTKNOW, RES_CHRATR_CJK_LANGUAGE), SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_CJK_POSTURE, new SvxPostureItem(ITALIC_NONE, RES_CHRATR_CJK_POSTURE ), SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_CJK_WEIGHT, new SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_CJK_WEIGHT ), SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEMINFOFLAG_NONE },

            // CTL-Attributes
            { RES_CHRATR_CTL_FONT, nullptr, SID_ATTR_CHAR_CTL_FONT, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_CTL_FONTSIZE, new SvxFontHeightItem(  240, 100,  RES_CHRATR_CTL_FONTSIZE ), SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_CTL_LANGUAGE, new SvxLanguageItem(LANGUAGE_DONTKNOW, RES_CHRATR_CTL_LANGUAGE), SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_CTL_POSTURE, new SvxPostureItem(ITALIC_NONE, RES_CHRATR_CTL_POSTURE ), SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_CTL_WEIGHT, new SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_CTL_WEIGHT ), SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEMINFOFLAG_NONE },

            { RES_CHRATR_ROTATE, new SvxCharRotateItem( 0_deg10, false, RES_CHRATR_ROTATE ), SID_ATTR_CHAR_ROTATED, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_EMPHASIS_MARK, new SvxEmphasisMarkItem( FontEmphasisMark::NONE, RES_CHRATR_EMPHASIS_MARK ), SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_TWO_LINES, new SvxTwoLinesItem( false, 0, 0, RES_CHRATR_TWO_LINES ), SID_ATTR_CHAR_TWO_LINES, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_SCALEW, new SvxCharScaleWidthItem( 100, RES_CHRATR_SCALEW ), SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_RELIEF, new SvxCharReliefItem( FontRelief::NONE, RES_CHRATR_RELIEF ), SID_ATTR_CHAR_RELIEF, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_HIDDEN, new SvxCharHiddenItem( false, RES_CHRATR_HIDDEN ), SID_ATTR_CHAR_HIDDEN, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_OVERLINE, new SvxOverlineItem( LINESTYLE_NONE, RES_CHRATR_OVERLINE ), SID_ATTR_CHAR_OVERLINE, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_RSID, new SvxRsidItem( 0, RES_CHRATR_RSID ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_BOX, new SvxBoxItem( RES_CHRATR_BOX ), SID_ATTR_CHAR_BOX, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_SHADOW, new SvxShadowItem( RES_CHRATR_SHADOW ), SID_ATTR_CHAR_SHADOW, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_HIGHLIGHT, new SvxBrushItem( RES_CHRATR_HIGHLIGHT ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_GRABBAG, new SfxGrabBagItem( RES_CHRATR_GRABBAG ), SID_ATTR_CHAR_GRABBAG, SFX_ITEMINFOFLAG_NONE },

            // CharacterAttr - MSWord weak char direction/script override emulation
            { RES_CHRATR_BIDIRTL, new SfxInt16Item( RES_CHRATR_BIDIRTL, sal_Int16(-1) ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_CHRATR_IDCTHINT, new SfxInt16Item( RES_CHRATR_IDCTHINT, sal_Int16(-1) ), 0, SFX_ITEMINFOFLAG_NONE },

            { RES_TXTATR_REFMARK, new SwFormatRefMark( ReferenceMarkerName() ),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_TOXMARK, createSwTOXMarkForItemInfoPackage(),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_META, SwFormatMeta::CreatePoolDefault(RES_TXTATR_META),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_METAFIELD, SwFormatMeta::CreatePoolDefault(RES_TXTATR_METAFIELD),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_AUTOFMT, new SwFormatAutoFormat,  0, SFX_ITEMINFOFLAG_NONE },

            // We cannot yet create the SwFormatINetFormat, that would crash since SwModule::get
            // is not initialized and the translated resource strings would not be available.
            // Luckily this mechanism allows to also flag this ItemInfo as 'incomplete' using
            // a nullptr as ItemPtr and implementing on-demand creation, see ::getItemInfo
            { RES_TXTATR_INETFMT, nullptr,  FN_TXTATR_INET, SFX_ITEMINFOFLAG_NONE },

            { RES_TXTATR_CHARFMT, new SwFormatCharFormat( nullptr ),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_CJK_RUBY, new SwFormatRuby( OUString() ),  SID_ATTR_CHAR_CJK_RUBY, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_UNKNOWN_CONTAINER, new SvXMLAttrContainerItem( RES_TXTATR_UNKNOWN_CONTAINER ),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_INPUTFIELD, createSwFormatFieldForItemInfoPackage( RES_TXTATR_INPUTFIELD ),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_CONTENTCONTROL, new SwFormatContentControl( RES_TXTATR_CONTENTCONTROL ),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_FIELD, createSwFormatFieldForItemInfoPackage( RES_TXTATR_FIELD ),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_FLYCNT, new SwFormatFlyCnt( nullptr ),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_FTN, new SwFormatFootnote,  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_ANNOTATION, createSwFormatFieldForItemInfoPackage( RES_TXTATR_ANNOTATION ),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_LINEBREAK, new SwFormatLineBreak(SwLineBreakClear::NONE),  0, SFX_ITEMINFOFLAG_NONE },
            { RES_TXTATR_DUMMY1, new SfxBoolItem( RES_TXTATR_DUMMY1 ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_LINESPACING, new SvxLineSpacingItem( LINE_SPACE_DEFAULT_HEIGHT, RES_PARATR_LINESPACING ), SID_ATTR_PARA_LINESPACE, SFX_ITEMINFOFLAG_NONE  },
            { RES_PARATR_ADJUST, new SvxAdjustItem( SvxAdjust::Left, RES_PARATR_ADJUST ), SID_ATTR_PARA_ADJUST, SFX_ITEMINFOFLAG_NONE  },
            { RES_PARATR_SPLIT, new SvxFormatSplitItem( true, RES_PARATR_SPLIT ), SID_ATTR_PARA_SPLIT, SFX_ITEMINFOFLAG_NONE  },
            { RES_PARATR_ORPHANS, new SvxOrphansItem( 0, RES_PARATR_ORPHANS ), SID_ATTR_PARA_ORPHANS, SFX_ITEMINFOFLAG_NONE  },
            { RES_PARATR_WIDOWS, new SvxWidowsItem( 0, RES_PARATR_WIDOWS ), SID_ATTR_PARA_WIDOWS, SFX_ITEMINFOFLAG_NONE  },
            { RES_PARATR_TABSTOP, new SvxTabStopItem( 1, SVX_TAB_DEFDIST, SvxTabAdjust::Default, RES_PARATR_TABSTOP ), SID_ATTR_TABSTOP, SFX_ITEMINFOFLAG_NONE  },

            // for this at the Item GetMaxHyphens() = 0 was called, do this now on-demand at construction time
            // it will get added in constructor below once for LO runtime as static default
            { RES_PARATR_HYPHENZONE, nullptr, SID_ATTR_PARA_HYPHENZONE, SFX_ITEMINFOFLAG_NONE  },

            { RES_PARATR_DROP, new SwFormatDrop, FN_FORMAT_DROPCAPS, SFX_ITEMINFOFLAG_NONE  },
            { RES_PARATR_REGISTER, new SwRegisterItem( false ), SID_ATTR_PARA_REGISTER, SFX_ITEMINFOFLAG_NONE  },
            { RES_PARATR_NUMRULE, new SwNumRuleItem( UIName() ), SID_ATTR_PARA_NUMRULE, SFX_ITEMINFOFLAG_NONE  },
            { RES_PARATR_SCRIPTSPACE, new SvxScriptSpaceItem( true, RES_PARATR_SCRIPTSPACE ), SID_ATTR_PARA_SCRIPTSPACE, SFX_ITEMINFOFLAG_NONE  },
            { RES_PARATR_HANGINGPUNCTUATION, new SvxHangingPunctuationItem( true, RES_PARATR_HANGINGPUNCTUATION ), SID_ATTR_PARA_HANGPUNCTUATION, SFX_ITEMINFOFLAG_NONE  },
            { RES_PARATR_FORBIDDEN_RULES, new SvxForbiddenRuleItem( true, RES_PARATR_FORBIDDEN_RULES ), SID_ATTR_PARA_FORBIDDEN_RULES, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_VERTALIGN, new SvxParaVertAlignItem( SvxParaVertAlignItem::Align::Automatic, RES_PARATR_VERTALIGN ), SID_PARA_VERTALIGN, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_SNAPTOGRID, new SvxParaGridItem( true, RES_PARATR_SNAPTOGRID ), SID_ATTR_PARA_SNAPTOGRID, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_CONNECT_BORDER, new SwParaConnectBorderItem, SID_ATTR_BORDER_CONNECT, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_OUTLINELEVEL, new SfxUInt16Item( RES_PARATR_OUTLINELEVEL, 0 ), SID_ATTR_PARA_OUTLINE_LEVEL, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_RSID, new SvxRsidItem( 0, RES_PARATR_RSID ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_GRABBAG, new SfxGrabBagItem( RES_PARATR_GRABBAG ), SID_ATTR_PARA_GRABBAG, SFX_ITEMINFOFLAG_NONE },

            { RES_PARATR_LIST_ID, new SfxStringItem( RES_PARATR_LIST_ID, OUString() ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_LIST_LEVEL, new SfxInt16Item( RES_PARATR_LIST_LEVEL, 0 ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_LIST_ISRESTART, new SfxBoolItem( RES_PARATR_LIST_ISRESTART, false ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_LIST_RESTARTVALUE, new SfxInt16Item( RES_PARATR_LIST_RESTARTVALUE, 1 ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_LIST_ISCOUNTED, new SfxBoolItem( RES_PARATR_LIST_ISCOUNTED, true ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_PARATR_LIST_AUTOFMT, new SwFormatAutoFormat(RES_PARATR_LIST_AUTOFMT), 0, SFX_ITEMINFOFLAG_NONE },

            { RES_FILL_ORDER, new SwFormatFillOrder, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_FRM_SIZE, new SwFormatFrameSize, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_PAPER_BIN, new SvxPaperBinItem( RES_PAPER_BIN ), SID_ATTR_PAGE_PAPERBIN, SFX_ITEMINFOFLAG_NONE },
            { RES_MARGIN_FIRSTLINE, new SvxFirstLineIndentItem(RES_MARGIN_FIRSTLINE), SID_ATTR_PARA_FIRSTLINESPACE, SFX_ITEMINFOFLAG_NONE },
            { RES_MARGIN_TEXTLEFT, new SvxTextLeftMarginItem(RES_MARGIN_TEXTLEFT), SID_ATTR_PARA_LEFTSPACE, SFX_ITEMINFOFLAG_NONE },
            { RES_MARGIN_RIGHT, new SvxRightMarginItem(RES_MARGIN_RIGHT), SID_ATTR_PARA_RIGHTSPACE, SFX_ITEMINFOFLAG_NONE },
            { RES_MARGIN_LEFT, new SvxLeftMarginItem(RES_MARGIN_LEFT), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_MARGIN_GUTTER, new SvxGutterLeftMarginItem(RES_MARGIN_GUTTER), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_MARGIN_GUTTER_RIGHT, new SvxGutterRightMarginItem(RES_MARGIN_GUTTER_RIGHT), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_LR_SPACE, new SvxLRSpaceItem( RES_LR_SPACE ), SID_ATTR_LRSPACE, SFX_ITEMINFOFLAG_NONE },
            { RES_UL_SPACE, new SvxULSpaceItem( RES_UL_SPACE ), SID_ATTR_ULSPACE, SFX_ITEMINFOFLAG_NONE },
            { RES_PAGEDESC, new SwFormatPageDesc, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_BREAK, new SvxFormatBreakItem( SvxBreak::NONE, RES_BREAK), SID_ATTR_PARA_PAGEBREAK, SFX_ITEMINFOFLAG_NONE },
            { RES_CNTNT, new SwFormatContent, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_HEADER, new SwFormatHeader, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_FOOTER, new SwFormatFooter, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_PRINT, new SvxPrintItem( RES_PRINT ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_OPAQUE, new SvxOpaqueItem( RES_OPAQUE ), FN_OPAQUE, SFX_ITEMINFOFLAG_NONE },
            { RES_PROTECT, new SvxProtectItem( RES_PROTECT ), FN_SET_PROTECT, SFX_ITEMINFOFLAG_NONE },
            { RES_SURROUND, new SwFormatSurround, FN_SURROUND, SFX_ITEMINFOFLAG_NONE },
            { RES_VERT_ORIENT, new SwFormatVertOrient, FN_VERT_ORIENT, SFX_ITEMINFOFLAG_NONE },
            { RES_HORI_ORIENT, new SwFormatHoriOrient, FN_HORI_ORIENT, SFX_ITEMINFOFLAG_NONE },
            { RES_ANCHOR, new SwFormatAnchor, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_BACKGROUND, new SvxBrushItem( RES_BACKGROUND ), SID_ATTR_BRUSH, SFX_ITEMINFOFLAG_NONE },
            { RES_BOX, new SvxBoxItem( RES_BOX ), SID_ATTR_BORDER_OUTER, SFX_ITEMINFOFLAG_NONE },
            { RES_SHADOW, new SvxShadowItem( RES_SHADOW ), SID_ATTR_BORDER_SHADOW, SFX_ITEMINFOFLAG_NONE },
            { RES_FRMMACRO, new SvxMacroItem( RES_FRMMACRO ), SID_ATTR_MACROITEM, SFX_ITEMINFOFLAG_NONE },
            { RES_COL, new SwFormatCol, FN_ATTR_COLUMNS, SFX_ITEMINFOFLAG_NONE },
            { RES_KEEP, new SvxFormatKeepItem( false, RES_KEEP ), SID_ATTR_PARA_KEEP, SFX_ITEMINFOFLAG_NONE },
            { RES_URL, new SwFormatURL(), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_EDIT_IN_READONLY, new SwFormatEditInReadonly, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_LAYOUT_SPLIT, new SwFormatLayoutSplit, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_CHAIN, new SwFormatChain, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_TEXTGRID, new SwTextGridItem, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_LINENUMBER, new SwFormatLineNumber, FN_FORMAT_LINENUMBER, SFX_ITEMINFOFLAG_NONE },
            { RES_FTN_AT_TXTEND, new SwFormatFootnoteAtTextEnd, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_END_AT_TXTEND, new SwFormatEndAtTextEnd, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_COLUMNBALANCE, new SwFormatNoBalancedColumns, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_FRAMEDIR, new SvxFrameDirectionItem( SvxFrameDirection::Environment, RES_FRAMEDIR ), SID_ATTR_FRAMEDIRECTION, SFX_ITEMINFOFLAG_NONE },
            { RES_HEADER_FOOTER_EAT_SPACING, new SwHeaderAndFooterEatSpacingItem, SID_ATTR_HDFT_DYNAMIC_SPACING, SFX_ITEMINFOFLAG_NONE },
            { RES_ROW_SPLIT, new SwFormatRowSplit, FN_TABLE_ROW_SPLIT, SFX_ITEMINFOFLAG_NONE },
            { RES_FLY_SPLIT, new SwFormatFlySplit, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_FOLLOW_TEXT_FLOW, new SwFormatFollowTextFlow(false), SID_SW_FOLLOW_TEXT_FLOW, SFX_ITEMINFOFLAG_NONE },
            { RES_COLLAPSING_BORDERS, new SfxBoolItem( RES_COLLAPSING_BORDERS, false ), SID_SW_COLLAPSING_BORDERS, SFX_ITEMINFOFLAG_NONE },
            { RES_WRAP_INFLUENCE_ON_OBJPOS, new SwFormatWrapInfluenceOnObjPos( text::WrapInfluenceOnPosition::ONCE_CONCURRENT ), SID_SW_WRAP_INFLUENCE_ON_OBJPOS, SFX_ITEMINFOFLAG_NONE },
            { RES_AUTO_STYLE, new SwFormatAutoFormat( RES_AUTO_STYLE ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_FRMATR_STYLE_NAME, new SfxStringItem( RES_FRMATR_STYLE_NAME, OUString()), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_FRMATR_CONDITIONAL_STYLE_NAME, new SfxStringItem( RES_FRMATR_CONDITIONAL_STYLE_NAME, OUString() ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_FRMATR_GRABBAG, new SfxGrabBagItem(RES_FRMATR_GRABBAG), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_TEXT_VERT_ADJUST, new SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP,RES_TEXT_VERT_ADJUST), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_BACKGROUND_FULL_SIZE, new SfxBoolItem(RES_BACKGROUND_FULL_SIZE, true), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_RTL_GUTTER, new SfxBoolItem(RES_RTL_GUTTER, false), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_DECORATIVE, new SfxBoolItem(RES_DECORATIVE, false), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_WRAP_TEXT_AT_FLY_START, new SwFormatWrapTextAtFlyStart, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_MIRRORGRF, new SwMirrorGrf, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_CROPGRF, new SwCropGrf, SID_ATTR_GRAF_CROP, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_ROTATION, new SwRotationGrf, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_LUMINANCE, new SwLuminanceGrf, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_CONTRAST, new SwContrastGrf, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_CHANNELR, new SwChannelRGrf, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_CHANNELG, new SwChannelGGrf, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_CHANNELB, new SwChannelBGrf, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_GAMMA, new SwGammaGrf, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_INVERT, new SwInvertGrf, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_TRANSPARENCY, new SwTransparencyGrf, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_DRAWMODE, new SwDrawModeGrf, 0, SFX_ITEMINFOFLAG_NONE },

            // GraphicAttr - Dummies
            { RES_GRFATR_DUMMY4, new SfxBoolItem( RES_GRFATR_DUMMY4 ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_GRFATR_DUMMY5, new SfxBoolItem( RES_GRFATR_DUMMY5 ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_BOXATR_FORMAT, new SwTableBoxNumFormat, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_BOXATR_FORMULA, new SwTableBoxFormula( OUString() ), 0, SFX_ITEMINFOFLAG_NONE },
            { RES_BOXATR_VALUE, new SwTableBoxValue, 0, SFX_ITEMINFOFLAG_NONE },
            { RES_UNKNOWNATR_CONTAINER, new SvXMLAttrContainerItem( RES_UNKNOWNATR_CONTAINER ), 0, SFX_ITEMINFOFLAG_NONE }
        }};

        virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const override { return maItemInfos[nIndex]; }

    public:
        ItemInfoPackageSwAttributes()
        {
            SvxHyphenZoneItem* pSvxHyphenZoneItem(new SvxHyphenZoneItem(false, RES_PARATR_HYPHENZONE));
            pSvxHyphenZoneItem->GetMaxHyphens() = 0; // Default: 0
            setItemAtItemInfoStatic(pSvxHyphenZoneItem, maItemInfos[RES_PARATR_HYPHENZONE - POOLATTR_BEGIN]);

            SvxFontItem* pFont(new SvxFontItem(RES_CHRATR_FONT));
            SvxFontItem* pFontCJK(new SvxFontItem(RES_CHRATR_CJK_FONT));
            SvxFontItem* pFontCTL(new SvxFontItem(RES_CHRATR_CTL_FONT));

            // Init DefFonts:
            GetDefaultFonts(*pFont, *pFontCJK, *pFontCTL);

            setItemAtItemInfoStatic(pFont, maItemInfos[RES_CHRATR_FONT - POOLATTR_BEGIN]);
            setItemAtItemInfoStatic(pFontCJK, maItemInfos[RES_CHRATR_CJK_FONT - POOLATTR_BEGIN]);
            setItemAtItemInfoStatic(pFontCTL, maItemInfos[RES_CHRATR_CTL_FONT - POOLATTR_BEGIN]);

            // as described above, this Item gets created on-demand.
            setItemAtItemInfoStatic(
                new SwFormatINetFormat(OUString(), OUString()),
                maItemInfos[RES_TXTATR_INETFMT - POOLATTR_BEGIN]);
        }

        virtual size_t size() const override { return maItemInfos.size(); }
        virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& /*rPool*/) override { return maItemInfos[nIndex]; }
        virtual const ItemInfo& getExistingItemInfo(size_t nIndex) override
        {
            return maItemInfos[nIndex];
        }
    };


    static std::unique_ptr<ItemInfoPackageSwAttributes> g_aItemInfoPackageSwAttributes;
    if (!g_aItemInfoPackageSwAttributes)
        g_aItemInfoPackageSwAttributes.reset(new ItemInfoPackageSwAttributes);
    return *g_aItemInfoPackageSwAttributes;
}

std::vector<SvGlobalName> *pGlobalOLEExcludeList = nullptr;

SwAutoCompleteWord* SwDoc::s_pAutoCompleteWords = nullptr;
SwDoc* SwDoc::s_pLast = nullptr;

SwCheckIt* pCheckIt = nullptr;
static CharClass* pAppCharClass = nullptr;

static CollatorWrapper* pCollator = nullptr,
                *pCaseCollator = nullptr;

SwCalendarWrapper& s_getCalendarWrapper()
{
    static SwCalendarWrapper aCalendarWrapper;
    return aCalendarWrapper;
}

void InitCore()
{
    SwBreakIt::Create_( ::comphelper::getProcessComponentContext() );
    pCheckIt = nullptr;

    FrameInit();
    TextInit_();

    SwSelPaintRects::s_pMapMode = new MapMode;
    SwFntObj::s_pPixMap = new MapMode;

    pGlobalOLEExcludeList = new std::vector<SvGlobalName>;

    if (!comphelper::IsFuzzing())
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
    delete SwTableAutoFormat::s_pDefaultBoxAutoFormat;

    delete SwSelPaintRects::s_pMapMode;
    delete SwFntObj::s_pPixMap;

    delete SwEditShell::s_pAutoFormatFlags;

    delete SwDoc::s_pAutoCompleteWords;

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
    if (!comphelper::IsFuzzing())
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
            const uno::Reference< uno::XComponentContext >& xContext = ::comphelper::getProcessComponentContext();

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
}

const ::utl::TransliterationWrapper& GetAppCmpStrIgnore()
{
    static TransWrp theTransWrp;
    return theTransWrp.getTransliterationWrapper();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
