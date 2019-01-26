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

#include <sal/config.h>

#include <utility>

#include <scitems.hxx>

#include <comphelper/string.hxx>
#include <i18nutil/unicode.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svl/itemiter.hxx>
#include <svl/stritem.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>
#include <editeng/langitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/postitem.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/justifyitem.hxx>

#include <docpool.hxx>
#include <global.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <scmod.hxx>
#include <sc.hrc>

// ATTR_FONT_TWOLINES (not used) was changed to ATTR_USERDEF (not saved in binary format) in 641c

namespace {

SvxFontItem* getDefaultFontItem(LanguageType eLang, DefaultFontType nFontType, sal_uInt16 nItemId)
{
    vcl::Font aDefFont = OutputDevice::GetDefaultFont( nFontType, eLang, GetDefaultFontFlags::OnlyOne );
    SvxFontItem* pNewItem = new SvxFontItem( aDefFont.GetFamilyType(), aDefFont.GetFamilyName(), aDefFont.GetStyleName(),
            aDefFont.GetPitch(), aDefFont.GetCharSet(), nItemId );

    return pNewItem;
}

}

static SfxItemInfo const  aItemInfos[] =
{
    { SID_ATTR_CHAR_FONT,           true },    // ATTR_FONT
    { SID_ATTR_CHAR_FONTHEIGHT,     true },    // ATTR_FONT_HEIGHT
    { SID_ATTR_CHAR_WEIGHT,         true },    // ATTR_FONT_WEIGHT
    { SID_ATTR_CHAR_POSTURE,        true },    // ATTR_FONT_POSTURE
    { SID_ATTR_CHAR_UNDERLINE,      true },    // ATTR_FONT_UNDERLINE
    { SID_ATTR_CHAR_OVERLINE,       true },    // ATTR_FONT_OVERLINE
    { SID_ATTR_CHAR_STRIKEOUT,      true },    // ATTR_FONT_CROSSEDOUT
    { SID_ATTR_CHAR_CONTOUR,        true },    // ATTR_FONT_CONTOUR
    { SID_ATTR_CHAR_SHADOWED,       true },    // ATTR_FONT_SHADOWED
    { SID_ATTR_CHAR_COLOR,          true },    // ATTR_FONT_COLOR
    { SID_ATTR_CHAR_LANGUAGE,       true },    // ATTR_FONT_LANGUAGE
    { SID_ATTR_CHAR_CJK_FONT,       true },    // ATTR_CJK_FONT            from 614
    { SID_ATTR_CHAR_CJK_FONTHEIGHT, true },    // ATTR_CJK_FONT_HEIGHT     from 614
    { SID_ATTR_CHAR_CJK_WEIGHT,     true },    // ATTR_CJK_FONT_WEIGHT     from 614
    { SID_ATTR_CHAR_CJK_POSTURE,    true },    // ATTR_CJK_FONT_POSTURE    from 614
    { SID_ATTR_CHAR_CJK_LANGUAGE,   true },    // ATTR_CJK_FONT_LANGUAGE   from 614
    { SID_ATTR_CHAR_CTL_FONT,       true },    // ATTR_CTL_FONT            from 614
    { SID_ATTR_CHAR_CTL_FONTHEIGHT, true },    // ATTR_CTL_FONT_HEIGHT     from 614
    { SID_ATTR_CHAR_CTL_WEIGHT,     true },    // ATTR_CTL_FONT_WEIGHT     from 614
    { SID_ATTR_CHAR_CTL_POSTURE,    true },    // ATTR_CTL_FONT_POSTURE    from 614
    { SID_ATTR_CHAR_CTL_LANGUAGE,   true },    // ATTR_CTL_FONT_LANGUAGE   from 614
    { SID_ATTR_CHAR_EMPHASISMARK,   true },    // ATTR_FONT_EMPHASISMARK   from 614
    { 0,                            true },    // ATTR_USERDEF             from 614 / 641c
    { SID_ATTR_CHAR_WORDLINEMODE,   true },    // ATTR_FONT_WORDLINE       from 632b
    { SID_ATTR_CHAR_RELIEF,         true },    // ATTR_FONT_RELIEF         from 632b
    { SID_ATTR_ALIGN_HYPHENATION,   true },    // ATTR_HYPHENATE           from 632b
    { 0,                            true },    // ATTR_SCRIPTSPACE         from 614d
    { 0,                            true },    // ATTR_HANGPUNCTUATION     from 614d
    { SID_ATTR_PARA_FORBIDDEN_RULES,true },    // ATTR_FORBIDDEN_RULES     from 614d
    { SID_ATTR_ALIGN_HOR_JUSTIFY,   true },    // ATTR_HOR_JUSTIFY
    { SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD, true }, // ATTR_HOR_JUSTIFY_METHOD
    { SID_ATTR_ALIGN_INDENT,        true },    // ATTR_INDENT          from 350
    { SID_ATTR_ALIGN_VER_JUSTIFY,   true },    // ATTR_VER_JUSTIFY
    { SID_ATTR_ALIGN_VER_JUSTIFY_METHOD, true }, // ATTR_VER_JUSTIFY_METHOD
    { SID_ATTR_ALIGN_STACKED,       true },    // ATTR_STACKED         from 680/dr14 (replaces ATTR_ORIENTATION)
    { SID_ATTR_ALIGN_DEGREES,       true },    // ATTR_ROTATE_VALUE    from 367
    { SID_ATTR_ALIGN_LOCKPOS,       true },    // ATTR_ROTATE_MODE     from 367
    { SID_ATTR_ALIGN_ASIANVERTICAL, true },    // ATTR_VERTICAL_ASIAN  from 642
    { SID_ATTR_FRAMEDIRECTION,      true },    // ATTR_WRITINGDIR      from 643
    { SID_ATTR_ALIGN_LINEBREAK,     true },    // ATTR_LINEBREAK
    { SID_ATTR_ALIGN_SHRINKTOFIT,   true },    // ATTR_SHRINKTOFIT     from 680/dr14
    { SID_ATTR_BORDER_DIAG_TLBR,    true },    // ATTR_BORDER_TLBR     from 680/dr14
    { SID_ATTR_BORDER_DIAG_BLTR,    true },    // ATTR_BORDER_BLTR     from 680/dr14
    { SID_ATTR_ALIGN_MARGIN,        true },    // ATTR_MARGIN
    { 0,                            true },    // ATTR_MERGE
    { 0,                            true },    // ATTR_MERGE_FLAG
    { SID_ATTR_NUMBERFORMAT_VALUE,  true },    // ATTR_VALUE_FORMAT
    { ATTR_LANGUAGE_FORMAT,         true },    // ATTR_LANGUAGE_FORMAT from 329, is combined with SID_ATTR_NUMBERFORMAT_VALUE in the dialog
    { SID_ATTR_BRUSH,               true },    // ATTR_BACKGROUND
    { SID_SCATTR_PROTECTION,        true },    // ATTR_PROTECTION
    { SID_ATTR_BORDER_OUTER,        true },    // ATTR_BORDER
    { SID_ATTR_BORDER_INNER,        true },    // ATTR_BORDER_INNER
    { SID_ATTR_BORDER_SHADOW,       true },    // ATTR_SHADOW
    { 0,                            true },    // ATTR_VALIDDATA
    { 0,                            true },    // ATTR_CONDITIONAL
    { 0,                            true },    // ATTR_HYPERLINK
    { 0,                            true },    // ATTR_PATTERN
    { SID_ATTR_LRSPACE,             true },    // ATTR_LRSPACE
    { SID_ATTR_ULSPACE,             true },    // ATTR_ULSPACE
    { SID_ATTR_PAGE,                true },    // ATTR_PAGE
    { SID_ATTR_PAGE_PAPERBIN,       true },    // ATTR_PAGE_PAPERBIN
    { SID_ATTR_PAGE_SIZE,           true },    // ATTR_PAGE_SIZE
    { SID_ATTR_PAGE_EXT1,           true },    // ATTR_PAGE_HORCENTER
    { SID_ATTR_PAGE_EXT2,           true },    // ATTR_PAGE_VERCENTER
    { SID_ATTR_PAGE_ON,             true },    // ATTR_PAGE_ON
    { SID_ATTR_PAGE_DYNAMIC,        true },    // ATTR_PAGE_DYNAMIC
    { SID_ATTR_PAGE_SHARED,         true },    // ATTR_PAGE_SHARED
    { SID_SCATTR_PAGE_NOTES,        true },    // ATTR_PAGE_NOTES
    { SID_SCATTR_PAGE_GRID,         true },    // ATTR_PAGE_GRID
    { SID_SCATTR_PAGE_HEADERS,      true },    // ATTR_PAGE_HEADERS
    { SID_SCATTR_PAGE_CHARTS,       true },    // ATTR_PAGE_CHARTS
    { SID_SCATTR_PAGE_OBJECTS,      true },    // ATTR_PAGE_OBJECTS
    { SID_SCATTR_PAGE_DRAWINGS,     true },    // ATTR_PAGE_DRAWINGS
    { SID_SCATTR_PAGE_TOPDOWN,      true },    // ATTR_PAGE_TOPDOWN
    { SID_SCATTR_PAGE_SCALE,        true },    // ATTR_PAGE_SCALE
    { SID_SCATTR_PAGE_SCALETOPAGES, true },    // ATTR_PAGE_SCALETOPAGES
    { SID_SCATTR_PAGE_FIRSTPAGENO,  true },    // ATTR_PAGE_FIRSTPAGENO
    { SID_SCATTR_PAGE_HEADERLEFT,   true },    // ATTR_PAGE_HEADERLEFT
    { SID_SCATTR_PAGE_FOOTERLEFT,   true },    // ATTR_PAGE_FOOTERLEFT
    { SID_SCATTR_PAGE_HEADERRIGHT,  true },    // ATTR_PAGE_HEADERRIGHT
    { SID_SCATTR_PAGE_FOOTERRIGHT,  true },    // ATTR_PAGE_FOOTERRIGHT
    { SID_ATTR_PAGE_HEADERSET,      true },    // ATTR_PAGE_HEADERSET
    { SID_ATTR_PAGE_FOOTERSET,      true },    // ATTR_PAGE_FOOTERSET
    { SID_SCATTR_PAGE_FORMULAS,     true },    // ATTR_PAGE_FORMULAS
    { SID_SCATTR_PAGE_NULLVALS,     true },    // ATTR_PAGE_NULLVALS
    { SID_SCATTR_PAGE_SCALETO,      true },    // ATTR_PAGE_SCALETO
    { 0,                            true }     // ATTR_HIDDEN
};
static_assert(
    SAL_N_ELEMENTS(aItemInfos) == ATTR_ENDINDEX - ATTR_STARTINDEX + 1, "these must match");

ScDocumentPool::ScDocumentPool()

    :   SfxItemPool ( "ScDocumentPool",
                        ATTR_STARTINDEX, ATTR_ENDINDEX,
                        aItemInfos, nullptr ),
    mvPoolDefaults(ATTR_ENDINDEX-ATTR_STARTINDEX+1),
    mnCurrentMaxKey(0)
{

    LanguageType nDefLang, nCjkLang, nCtlLang;
    bool bAutoSpell;
    ScModule::GetSpellSettings( nDefLang, nCjkLang, nCtlLang, bAutoSpell );

    //  latin font from GetDefaultFonts is not used, DEFAULTFONT_LATIN_SPREADSHEET instead
    SvxFontItem* pStdFont = getDefaultFontItem(nDefLang, DefaultFontType::LATIN_SPREADSHEET, ATTR_FONT);
    SvxFontItem* pCjkFont = getDefaultFontItem(nCjkLang, DefaultFontType::CJK_SPREADSHEET, ATTR_CJK_FONT);
    SvxFontItem* pCtlFont = getDefaultFontItem(nCtlLang, DefaultFontType::CTL_SPREADSHEET, ATTR_CTL_FONT);

    SvxBoxInfoItem* pGlobalBorderInnerAttr = new SvxBoxInfoItem( ATTR_BORDER_INNER );
    auto pSet = std::make_unique<SfxItemSet>( *this, svl::Items<ATTR_PATTERN_START, ATTR_PATTERN_END>{} );
    SfxItemSet      aSetItemItemSet( *this,
                                     svl::Items<ATTR_BACKGROUND, ATTR_BACKGROUND,
                                     ATTR_BORDER,     ATTR_SHADOW,
                                     ATTR_LRSPACE,    ATTR_ULSPACE,
                                     ATTR_PAGE_SIZE,  ATTR_PAGE_SIZE,
                                     ATTR_PAGE_ON,    ATTR_PAGE_SHARED>{} );

    pGlobalBorderInnerAttr->SetLine(nullptr, SvxBoxInfoItemLine::HORI);
    pGlobalBorderInnerAttr->SetLine(nullptr, SvxBoxInfoItemLine::VERT);
    pGlobalBorderInnerAttr->SetTable(true);
    pGlobalBorderInnerAttr->SetDist(true);
    pGlobalBorderInnerAttr->SetMinDist(false);

    mvPoolDefaults[ ATTR_FONT            - ATTR_STARTINDEX ] = pStdFont;
    mvPoolDefaults[ ATTR_FONT_HEIGHT     - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200, 100, ATTR_FONT_HEIGHT );       // 10 pt;
    mvPoolDefaults[ ATTR_FONT_WEIGHT     - ATTR_STARTINDEX ] = new SvxWeightItem( WEIGHT_NORMAL, ATTR_FONT_WEIGHT );
    mvPoolDefaults[ ATTR_FONT_POSTURE    - ATTR_STARTINDEX ] = new SvxPostureItem( ITALIC_NONE, ATTR_FONT_POSTURE );
    mvPoolDefaults[ ATTR_FONT_UNDERLINE  - ATTR_STARTINDEX ] = new SvxUnderlineItem( LINESTYLE_NONE, ATTR_FONT_UNDERLINE );
    mvPoolDefaults[ ATTR_FONT_OVERLINE   - ATTR_STARTINDEX ] = new SvxOverlineItem( LINESTYLE_NONE, ATTR_FONT_OVERLINE );
    mvPoolDefaults[ ATTR_FONT_CROSSEDOUT - ATTR_STARTINDEX ] = new SvxCrossedOutItem( STRIKEOUT_NONE, ATTR_FONT_CROSSEDOUT );
    mvPoolDefaults[ ATTR_FONT_CONTOUR    - ATTR_STARTINDEX ] = new SvxContourItem( false, ATTR_FONT_CONTOUR );
    mvPoolDefaults[ ATTR_FONT_SHADOWED   - ATTR_STARTINDEX ] = new SvxShadowedItem( false, ATTR_FONT_SHADOWED );
    mvPoolDefaults[ ATTR_FONT_COLOR      - ATTR_STARTINDEX ] = new SvxColorItem( COL_AUTO, ATTR_FONT_COLOR );
    mvPoolDefaults[ ATTR_FONT_LANGUAGE   - ATTR_STARTINDEX ] = new SvxLanguageItem( LANGUAGE_DONTKNOW, ATTR_FONT_LANGUAGE );
    mvPoolDefaults[ ATTR_CJK_FONT        - ATTR_STARTINDEX ] = pCjkFont;
    mvPoolDefaults[ ATTR_CJK_FONT_HEIGHT - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200, 100, ATTR_CJK_FONT_HEIGHT );
    mvPoolDefaults[ ATTR_CJK_FONT_WEIGHT - ATTR_STARTINDEX ] = new SvxWeightItem( WEIGHT_NORMAL, ATTR_CJK_FONT_WEIGHT );
    mvPoolDefaults[ ATTR_CJK_FONT_POSTURE- ATTR_STARTINDEX ] = new SvxPostureItem( ITALIC_NONE, ATTR_CJK_FONT_POSTURE );
    mvPoolDefaults[ ATTR_CJK_FONT_LANGUAGE-ATTR_STARTINDEX ] = new SvxLanguageItem( LANGUAGE_DONTKNOW, ATTR_CJK_FONT_LANGUAGE );
    mvPoolDefaults[ ATTR_CTL_FONT        - ATTR_STARTINDEX ] = pCtlFont;
    mvPoolDefaults[ ATTR_CTL_FONT_HEIGHT - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200, 100, ATTR_CTL_FONT_HEIGHT );
    mvPoolDefaults[ ATTR_CTL_FONT_WEIGHT - ATTR_STARTINDEX ] = new SvxWeightItem( WEIGHT_NORMAL, ATTR_CTL_FONT_WEIGHT );
    mvPoolDefaults[ ATTR_CTL_FONT_POSTURE- ATTR_STARTINDEX ] = new SvxPostureItem( ITALIC_NONE, ATTR_CTL_FONT_POSTURE );
    mvPoolDefaults[ ATTR_CTL_FONT_LANGUAGE-ATTR_STARTINDEX ] = new SvxLanguageItem( LANGUAGE_DONTKNOW, ATTR_CTL_FONT_LANGUAGE );
    mvPoolDefaults[ ATTR_FONT_EMPHASISMARK-ATTR_STARTINDEX ] = new SvxEmphasisMarkItem( FontEmphasisMark::NONE, ATTR_FONT_EMPHASISMARK );
    mvPoolDefaults[ ATTR_USERDEF         - ATTR_STARTINDEX ] = new SvXMLAttrContainerItem( ATTR_USERDEF );
    mvPoolDefaults[ ATTR_FONT_WORDLINE   - ATTR_STARTINDEX ] = new SvxWordLineModeItem(false, ATTR_FONT_WORDLINE );
    mvPoolDefaults[ ATTR_FONT_RELIEF     - ATTR_STARTINDEX ] = new SvxCharReliefItem( FontRelief::NONE, ATTR_FONT_RELIEF );
    mvPoolDefaults[ ATTR_HYPHENATE       - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_HYPHENATE );
    mvPoolDefaults[ ATTR_SCRIPTSPACE     - ATTR_STARTINDEX ] = new SvxScriptSpaceItem( false, ATTR_SCRIPTSPACE);
    mvPoolDefaults[ ATTR_HANGPUNCTUATION - ATTR_STARTINDEX ] = new SvxHangingPunctuationItem( false, ATTR_HANGPUNCTUATION);
    mvPoolDefaults[ ATTR_FORBIDDEN_RULES - ATTR_STARTINDEX ] = new SvxForbiddenRuleItem( false, ATTR_FORBIDDEN_RULES);
    mvPoolDefaults[ ATTR_HOR_JUSTIFY     - ATTR_STARTINDEX ] = new SvxHorJustifyItem( SvxCellHorJustify::Standard, ATTR_HOR_JUSTIFY);
    mvPoolDefaults[ ATTR_HOR_JUSTIFY_METHOD - ATTR_STARTINDEX ] = new SvxJustifyMethodItem( SvxCellJustifyMethod::Auto, ATTR_HOR_JUSTIFY_METHOD);
    mvPoolDefaults[ ATTR_INDENT          - ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_INDENT, 0 );
    mvPoolDefaults[ ATTR_VER_JUSTIFY     - ATTR_STARTINDEX ] = new SvxVerJustifyItem( SvxCellVerJustify::Standard, ATTR_VER_JUSTIFY);
    mvPoolDefaults[ ATTR_VER_JUSTIFY_METHOD - ATTR_STARTINDEX ] = new SvxJustifyMethodItem( SvxCellJustifyMethod::Auto, ATTR_VER_JUSTIFY_METHOD);
    mvPoolDefaults[ ATTR_STACKED         - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_STACKED, false );
    mvPoolDefaults[ ATTR_ROTATE_VALUE    - ATTR_STARTINDEX ] = new SfxInt32Item( ATTR_ROTATE_VALUE, 0 );
    mvPoolDefaults[ ATTR_ROTATE_MODE     - ATTR_STARTINDEX ] = new SvxRotateModeItem( SVX_ROTATE_MODE_BOTTOM, ATTR_ROTATE_MODE );
    mvPoolDefaults[ ATTR_VERTICAL_ASIAN  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_VERTICAL_ASIAN );
    //  The default for the ATTR_WRITINGDIR cell attribute must by SvxFrameDirection::Environment,
    //  so that value is returned when asking for a default cell's attributes.
    //  The value from the page style is set as DefaultHorizontalTextDirection for the EditEngine.
    mvPoolDefaults[ ATTR_WRITINGDIR      - ATTR_STARTINDEX ] = new SvxFrameDirectionItem( SvxFrameDirection::Environment, ATTR_WRITINGDIR );
    mvPoolDefaults[ ATTR_LINEBREAK       - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_LINEBREAK );
    mvPoolDefaults[ ATTR_SHRINKTOFIT     - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_SHRINKTOFIT );
    mvPoolDefaults[ ATTR_BORDER_TLBR     - ATTR_STARTINDEX ] = new SvxLineItem( ATTR_BORDER_TLBR );
    mvPoolDefaults[ ATTR_BORDER_BLTR     - ATTR_STARTINDEX ] = new SvxLineItem( ATTR_BORDER_BLTR );
    mvPoolDefaults[ ATTR_MARGIN          - ATTR_STARTINDEX ] = new SvxMarginItem( ATTR_MARGIN );
    mvPoolDefaults[ ATTR_MERGE           - ATTR_STARTINDEX ] = new ScMergeAttr;
    mvPoolDefaults[ ATTR_MERGE_FLAG      - ATTR_STARTINDEX ] = new ScMergeFlagAttr;
    mvPoolDefaults[ ATTR_VALUE_FORMAT    - ATTR_STARTINDEX ] = new SfxUInt32Item( ATTR_VALUE_FORMAT, 0 );
    mvPoolDefaults[ ATTR_LANGUAGE_FORMAT - ATTR_STARTINDEX ] = new SvxLanguageItem( ScGlobal::eLnge, ATTR_LANGUAGE_FORMAT );
    mvPoolDefaults[ ATTR_BACKGROUND      - ATTR_STARTINDEX ] = new SvxBrushItem( COL_TRANSPARENT, ATTR_BACKGROUND );
    mvPoolDefaults[ ATTR_PROTECTION      - ATTR_STARTINDEX ] = new ScProtectionAttr;
    mvPoolDefaults[ ATTR_BORDER          - ATTR_STARTINDEX ] = new SvxBoxItem( ATTR_BORDER );
    mvPoolDefaults[ ATTR_BORDER_INNER    - ATTR_STARTINDEX ] = pGlobalBorderInnerAttr;
    mvPoolDefaults[ ATTR_SHADOW          - ATTR_STARTINDEX ] = new SvxShadowItem( ATTR_SHADOW );
    mvPoolDefaults[ ATTR_VALIDDATA       - ATTR_STARTINDEX ] = new SfxUInt32Item( ATTR_VALIDDATA, 0 );
    mvPoolDefaults[ ATTR_CONDITIONAL     - ATTR_STARTINDEX ] = new ScCondFormatItem;
    mvPoolDefaults[ ATTR_HYPERLINK       - ATTR_STARTINDEX ] = new SfxStringItem( ATTR_HYPERLINK, OUString() ) ;

    // GetRscString only works after ScGlobal::Init (indicated by the EmptyBrushItem)
    // TODO: Write additional method ScGlobal::IsInit() or somesuch
    //       or detect whether this is the Secondary Pool for a MessagePool
    if ( ScGlobal::GetEmptyBrushItem() )
        mvPoolDefaults[ ATTR_PATTERN     - ATTR_STARTINDEX ] = new ScPatternAttr( std::move(pSet), ScResId(STR_STYLENAME_STANDARD) );
    else
        mvPoolDefaults[ ATTR_PATTERN     - ATTR_STARTINDEX ] = new ScPatternAttr( std::move(pSet), STRING_STANDARD ); // FIXME: without name?

    mvPoolDefaults[ ATTR_LRSPACE         - ATTR_STARTINDEX ] = new SvxLRSpaceItem( ATTR_LRSPACE );
    mvPoolDefaults[ ATTR_ULSPACE         - ATTR_STARTINDEX ] = new SvxULSpaceItem( ATTR_ULSPACE );
    mvPoolDefaults[ ATTR_PAGE            - ATTR_STARTINDEX ] = new SvxPageItem( ATTR_PAGE );
    mvPoolDefaults[ ATTR_PAGE_PAPERBIN   - ATTR_STARTINDEX ] = new SvxPaperBinItem( ATTR_PAGE_PAPERBIN );
    mvPoolDefaults[ ATTR_PAGE_SIZE       - ATTR_STARTINDEX ] = new SvxSizeItem( ATTR_PAGE_SIZE );
    mvPoolDefaults[ ATTR_PAGE_HORCENTER  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_HORCENTER );
    mvPoolDefaults[ ATTR_PAGE_VERCENTER  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_VERCENTER );
    mvPoolDefaults[ ATTR_PAGE_ON         - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_ON, true );
    mvPoolDefaults[ ATTR_PAGE_DYNAMIC    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_DYNAMIC, true );
    mvPoolDefaults[ ATTR_PAGE_SHARED     - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_SHARED, true );
    mvPoolDefaults[ ATTR_PAGE_NOTES      - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_NOTES, false );
    mvPoolDefaults[ ATTR_PAGE_GRID       - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_GRID, false );
    mvPoolDefaults[ ATTR_PAGE_HEADERS    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_HEADERS, false );
    mvPoolDefaults[ ATTR_PAGE_CHARTS     - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_CHARTS );
    mvPoolDefaults[ ATTR_PAGE_OBJECTS    - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_OBJECTS );
    mvPoolDefaults[ ATTR_PAGE_DRAWINGS   - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_DRAWINGS );
    mvPoolDefaults[ ATTR_PAGE_TOPDOWN    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_TOPDOWN, true );
    mvPoolDefaults[ ATTR_PAGE_SCALE      - ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_SCALE, 100 );
    mvPoolDefaults[ ATTR_PAGE_SCALETOPAGES-ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, 1 );
    mvPoolDefaults[ ATTR_PAGE_FIRSTPAGENO- ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, 1 );
    mvPoolDefaults[ ATTR_PAGE_HEADERLEFT - ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_HEADERLEFT );
    mvPoolDefaults[ ATTR_PAGE_FOOTERLEFT - ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_FOOTERLEFT );
    mvPoolDefaults[ ATTR_PAGE_HEADERRIGHT- ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_HEADERRIGHT );
    mvPoolDefaults[ ATTR_PAGE_FOOTERRIGHT- ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_FOOTERRIGHT );
    mvPoolDefaults[ ATTR_PAGE_HEADERSET  - ATTR_STARTINDEX ] = new SvxSetItem( ATTR_PAGE_HEADERSET, aSetItemItemSet );
    mvPoolDefaults[ ATTR_PAGE_FOOTERSET  - ATTR_STARTINDEX ] = new SvxSetItem( ATTR_PAGE_FOOTERSET, aSetItemItemSet );
    mvPoolDefaults[ ATTR_PAGE_FORMULAS   - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_FORMULAS, false );
    mvPoolDefaults[ ATTR_PAGE_NULLVALS   - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_NULLVALS, true );
    mvPoolDefaults[ ATTR_PAGE_SCALETO    - ATTR_STARTINDEX ] = new ScPageScaleToItem( 1, 1 );
    mvPoolDefaults[ ATTR_HIDDEN          - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_HIDDEN, false );

    SetDefaults( &mvPoolDefaults );
}

ScDocumentPool::~ScDocumentPool()
{
    Delete();

    for ( sal_uInt16 i=0; i < ATTR_ENDINDEX-ATTR_STARTINDEX+1; i++ )
    {
        ClearRefCount( *mvPoolDefaults[i] );
        delete mvPoolDefaults[i];
    }
}

const SfxPoolItem& ScDocumentPool::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
{
    if ( rItem.Which() != ATTR_PATTERN ) // Only Pattern is special
        return SfxItemPool::Put( rItem, nWhich );

    // Don't copy the default pattern of this Pool
    if (&rItem == mvPoolDefaults[ ATTR_PATTERN - ATTR_STARTINDEX ])
        return rItem;

    // Else Put must always happen, because it could be another Pool
    const SfxPoolItem& rNew = SfxItemPool::Put( rItem, nWhich );
    sal_uInt32 nRef = rNew.GetRefCount();
    if (nRef == 1)
    {
        ++mnCurrentMaxKey;
        const_cast<ScPatternAttr&>(static_cast<const ScPatternAttr&>(rNew)).SetKey(mnCurrentMaxKey);
    }
    return rNew;
}

void ScDocumentPool::StyleDeleted( const ScStyleSheet* pStyle )
{
    sal_uInt32 nCount = GetItemCount2(ATTR_PATTERN);
    for (sal_uInt32 i=0; i<nCount; i++)
    {
        ScPatternAttr* pPattern = const_cast<ScPatternAttr*>(GetItem2(ATTR_PATTERN, i));
        if ( pPattern && pPattern->GetStyleSheet() == pStyle )
            pPattern->StyleToName();
    }
}

void ScDocumentPool::CellStyleCreated( const OUString& rName, const ScDocument* pDoc )
{
    // If a style was created, don't keep any pattern with its name string in the pool,
    // because it would compare equal to a pattern with a pointer to the new style.
    // Calling StyleSheetChanged isn't enough because the pool may still contain items
    // for undo or clipboard content.

    sal_uInt32 nCount = GetItemCount2(ATTR_PATTERN);
    for (sal_uInt32 i=0; i<nCount; i++)
    {
        ScPatternAttr *const pPattern =
            const_cast<ScPatternAttr*>(GetItem2(ATTR_PATTERN, i));
        if ( pPattern && pPattern->GetStyleSheet() == nullptr )
        {
            const OUString* pStyleName = pPattern->GetStyleName();
            if ( pStyleName && *pStyleName == rName )
                pPattern->UpdateStyleSheet(pDoc); // find and store style pointer
        }
    }
}

SfxItemPool* ScDocumentPool::Clone() const
{
    return new SfxItemPool (*this, true);
}

static bool lcl_HFPresentation
(
    const SfxPoolItem&  rItem,
    MapUnit             eCoreMetric,
    MapUnit             ePresentationMetric,
    OUString&           rText,
    const IntlWrapper& rIntl
)
{
    const SfxItemSet& rSet = static_cast<const SfxSetItem&>(rItem).GetItemSet();
    const SfxPoolItem* pItem;

    if ( SfxItemState::SET == rSet.GetItemState(ATTR_PAGE_ON,false,&pItem) )
    {
        if( !static_cast<const SfxBoolItem*>(pItem)->GetValue() )
            return false;
    }

    SfxItemIter aIter( rSet );
    pItem = aIter.FirstItem();

    while( pItem )
    {
        sal_uInt16 nWhich = pItem->Which();

        OUString aText;

        switch( nWhich )
        {
            case ATTR_PAGE_ON:
            case ATTR_PAGE_DYNAMIC:
            case ATTR_PAGE_SHARED:
            break;

            case ATTR_LRSPACE:
            {
                const SvxLRSpaceItem& rLRItem = static_cast<const SvxLRSpaceItem&>(*pItem);
                sal_uInt16 nPropLeftMargin  = rLRItem.GetPropLeft();
                sal_uInt16 nPropRightMargin = rLRItem.GetPropRight();
                sal_uInt16 nLeftMargin, nRightMargin;
                long nTmp;
                nTmp = rLRItem.GetLeft();
                nLeftMargin = nTmp < 0 ? 0 : sal_uInt16(nTmp);
                nTmp = rLRItem.GetRight();
                nRightMargin = nTmp < 0 ? 0 : sal_uInt16(nTmp);

                aText = EditResId(RID_SVXITEMS_LRSPACE_LEFT);
                if ( 100 != nPropLeftMargin )
                {
                    aText = aText + unicode::formatPercent(nPropLeftMargin,
                        Application::GetSettings().GetUILanguageTag());
                }
                else
                {
                    aText += GetMetricText( static_cast<long>(nLeftMargin),
                                           eCoreMetric, ePresentationMetric, &rIntl );
                    aText += " " + EditResId(GetMetricId(ePresentationMetric));
                }
                aText += cpDelim;

                // We don't have a nPropFirstLineOfst
                aText += EditResId(RID_SVXITEMS_LRSPACE_RIGHT);
                if ( 100 != nPropRightMargin )
                {
                    aText = aText + unicode::formatPercent(nPropLeftMargin,
                        Application::GetSettings().GetUILanguageTag());
                }
                else
                {
                    aText += GetMetricText( static_cast<long>(nRightMargin),
                                            eCoreMetric, ePresentationMetric, &rIntl );
                    aText += " " + EditResId(GetMetricId(ePresentationMetric));
                }
            }
            break;

            default:
                pItem->GetPresentation( SfxItemPresentation::Complete, eCoreMetric, ePresentationMetric, aText, rIntl );

        }

        if ( aText.getLength() )
        {
            rText = rText + aText + " + ";
        }

        pItem = aIter.NextItem();
    }

    rText = comphelper::string::stripEnd(rText, ' ');
    rText = comphelper::string::stripEnd(rText, '+');
    rText = comphelper::string::stripEnd(rText, ' ');
    return true;
}

bool ScDocumentPool::GetPresentation(
    const SfxPoolItem&  rItem,
    MapUnit             ePresentationMetric,
    OUString&           rText,
    const IntlWrapper& rIntl ) const
{
    sal_uInt16  nW = rItem.Which();
    OUString aStrYes  ( ScResId(STR_YES) );
    OUString aStrNo   ( ScResId(STR_NO) );
    OUString aStrSep(": ");

    bool ePresentationRet = true;
    switch( nW )
    {
        case ATTR_PAGE_TOPDOWN:
            rText = ScResId(STR_SCATTR_PAGE_PRINTDIR) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ?
                ScResId(STR_SCATTR_PAGE_TOPDOWN) :
                ScResId(STR_SCATTR_PAGE_LEFTRIGHT) ;
        break;

        case ATTR_PAGE_HEADERS:
            rText = ScResId(STR_SCATTR_PAGE_HEADERS) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_NULLVALS:
            rText = ScResId(STR_SCATTR_PAGE_NULLVALS) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_FORMULAS:
            rText = ScResId(STR_SCATTR_PAGE_FORMULAS) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_NOTES:
            rText = ScResId(STR_SCATTR_PAGE_NOTES) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_GRID:
            rText = ScResId(STR_SCATTR_PAGE_GRID) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_SCALETOPAGES:
        {
            sal_uInt16  nPagNo = static_cast<const SfxUInt16Item&>(rItem).GetValue();

            if( nPagNo )
            {
                rText = ScResId( STR_SCATTR_PAGE_SCALETOPAGES ) + aStrSep;
                OUString aPages(ScResId(STR_SCATTR_PAGE_SCALE_PAGES, nPagNo));
                aPages = aPages.replaceFirst( "%1", OUString::number( nPagNo ) );
                rText += aPages;
            }
            else
            {
                ePresentationRet = false;
            }
        }
        break;

        case ATTR_PAGE_FIRSTPAGENO:
        {
            sal_uInt16  nPagNo = static_cast<const SfxUInt16Item&>(rItem).GetValue();

            if( nPagNo )
            {
                rText = ScResId(STR_SCATTR_PAGE_FIRSTPAGENO) + aStrSep;
                rText += OUString::number( nPagNo );
            }
            else
            {
                ePresentationRet = false;
            }
        }
        break;

        case ATTR_PAGE_SCALE:
        {
            sal_uInt16  nPercent = static_cast<const SfxUInt16Item &>(rItem).GetValue();

            if( nPercent )
            {
                rText = ScResId(STR_SCATTR_PAGE_SCALE) + aStrSep;
                rText = rText + unicode::formatPercent(nPercent,
                    Application::GetSettings().GetUILanguageTag());
            }
            else
            {
                ePresentationRet = false;
            }
        }
        break;

        case ATTR_PAGE_HEADERSET:
        {
            OUString  aBuffer;

            if( lcl_HFPresentation( rItem, GetMetric( nW ), ePresentationMetric, aBuffer, rIntl ) )
            {
                rText = ScResId(STR_HEADER) + " ( " + aBuffer + " ) ";
            }
        }
        break;

        case ATTR_PAGE_FOOTERSET:
        {
            OUString  aBuffer;

            if( lcl_HFPresentation( rItem, GetMetric( nW ), ePresentationMetric, aBuffer, rIntl ) )
            {
                rText = ScResId(STR_FOOTER) + " ( " + aBuffer + " ) ";
            }
        }
        break;

        default:
            ePresentationRet = rItem.GetPresentation( SfxItemPresentation::Complete, GetMetric( nW ), ePresentationMetric, rText, rIntl );
        break;
    }

    return ePresentationRet;
}

MapUnit ScDocumentPool::GetMetric( sal_uInt16 nWhich ) const
{
    // Own attributes in Twips, everything else in 1/100 mm
    if ( nWhich >= ATTR_STARTINDEX && nWhich <= ATTR_ENDINDEX )
        return MapUnit::MapTwip;
    else
        return MapUnit::Map100thMM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
