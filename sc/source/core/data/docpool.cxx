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

SfxItemInfo const  aItemInfos[] =
{
    // _nSID, _bNeedsPoolRegistration, _bShareable
    { SID_ATTR_CHAR_FONT,                   true,  true },    // ATTR_FONT
    { SID_ATTR_CHAR_FONTHEIGHT,             false, true },    // ATTR_FONT_HEIGHT
    { SID_ATTR_CHAR_WEIGHT,                 false, true },    // ATTR_FONT_WEIGHT
    { SID_ATTR_CHAR_POSTURE,                false, true },    // ATTR_FONT_POSTURE
    { SID_ATTR_CHAR_UNDERLINE,              false, true },    // ATTR_FONT_UNDERLINE
    { SID_ATTR_CHAR_OVERLINE,               false, true },    // ATTR_FONT_OVERLINE
    { SID_ATTR_CHAR_STRIKEOUT,              false, true },    // ATTR_FONT_CROSSEDOUT
    { SID_ATTR_CHAR_CONTOUR,                false, true },    // ATTR_FONT_CONTOUR
    { SID_ATTR_CHAR_SHADOWED,               false, true },    // ATTR_FONT_SHADOWED
    { SID_ATTR_CHAR_COLOR,                  true,  true },    // ATTR_FONT_COLOR
    { SID_ATTR_CHAR_LANGUAGE,               false, true },    // ATTR_FONT_LANGUAGE
    { SID_ATTR_CHAR_CJK_FONT,               true,  true },    // ATTR_CJK_FONT            from 614
    { SID_ATTR_CHAR_CJK_FONTHEIGHT,         false, true },    // ATTR_CJK_FONT_HEIGHT     from 614
    { SID_ATTR_CHAR_CJK_WEIGHT,             false, true },    // ATTR_CJK_FONT_WEIGHT     from 614
    { SID_ATTR_CHAR_CJK_POSTURE,            false, true },    // ATTR_CJK_FONT_POSTURE    from 614
    { SID_ATTR_CHAR_CJK_LANGUAGE,           false, true },    // ATTR_CJK_FONT_LANGUAGE   from 614
    { SID_ATTR_CHAR_CTL_FONT,               true,  true },    // ATTR_CTL_FONT            from 614
    { SID_ATTR_CHAR_CTL_FONTHEIGHT,         false, true },    // ATTR_CTL_FONT_HEIGHT     from 614
    { SID_ATTR_CHAR_CTL_WEIGHT,             false, true },    // ATTR_CTL_FONT_WEIGHT     from 614
    { SID_ATTR_CHAR_CTL_POSTURE,            false, true },    // ATTR_CTL_FONT_POSTURE    from 614
    { SID_ATTR_CHAR_CTL_LANGUAGE,           false, true },    // ATTR_CTL_FONT_LANGUAGE   from 614
    { SID_ATTR_CHAR_EMPHASISMARK,           false, true },    // ATTR_FONT_EMPHASISMARK   from 614
    { 0,                                    true,  true },    // ATTR_USERDEF             from 614 / 641c
    { SID_ATTR_CHAR_WORDLINEMODE,           false, true },    // ATTR_FONT_WORDLINE       from 632b
    { SID_ATTR_CHAR_RELIEF,                 false, true },    // ATTR_FONT_RELIEF         from 632b
    { SID_ATTR_ALIGN_HYPHENATION,           false, true },    // ATTR_HYPHENATE           from 632b
    { 0,                                    false, true },    // ATTR_SCRIPTSPACE         from 614d
    { 0,                                    false, true },    // ATTR_HANGPUNCTUATION     from 614d
    { SID_ATTR_PARA_FORBIDDEN_RULES,        false, true },    // ATTR_FORBIDDEN_RULES     from 614d
    { SID_ATTR_ALIGN_HOR_JUSTIFY,           false, true },    // ATTR_HOR_JUSTIFY
    { SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD,    false, true }, // ATTR_HOR_JUSTIFY_METHOD
    { SID_ATTR_ALIGN_INDENT,                false, true },    // ATTR_INDENT          from 350
    { SID_ATTR_ALIGN_VER_JUSTIFY,           false, true },    // ATTR_VER_JUSTIFY
    { SID_ATTR_ALIGN_VER_JUSTIFY_METHOD,    false, true }, // ATTR_VER_JUSTIFY_METHOD
    { SID_ATTR_ALIGN_STACKED,               false, true },    // ATTR_STACKED         from 680/dr14 (replaces ATTR_ORIENTATION)
    { SID_ATTR_ALIGN_DEGREES,               true,  true },    // ATTR_ROTATE_VALUE    from 367
    { SID_ATTR_ALIGN_LOCKPOS,               false, true },    // ATTR_ROTATE_MODE     from 367
    { SID_ATTR_ALIGN_ASIANVERTICAL,         false, true },    // ATTR_VERTICAL_ASIAN  from 642
    { SID_ATTR_FRAMEDIRECTION,              false, true },    // ATTR_WRITINGDIR      from 643
    { SID_ATTR_ALIGN_LINEBREAK,             false, true },    // ATTR_LINEBREAK
    { SID_ATTR_ALIGN_SHRINKTOFIT,           false, true },    // ATTR_SHRINKTOFIT     from 680/dr14
    { SID_ATTR_BORDER_DIAG_TLBR,            false, true },    // ATTR_BORDER_TLBR     from 680/dr14
    { SID_ATTR_BORDER_DIAG_BLTR,            false, true },    // ATTR_BORDER_BLTR     from 680/dr14
    { SID_ATTR_ALIGN_MARGIN,                false, true },    // ATTR_MARGIN
    { 0,                                    false, true },    // ATTR_MERGE
    { 0,                                    false, true },    // ATTR_MERGE_FLAG
    { SID_ATTR_NUMBERFORMAT_VALUE,          true,  true },    // ATTR_VALUE_FORMAT
    { 0,                                    false, true },    // ATTR_LANGUAGE_FORMAT from 329, is combined with SID_ATTR_NUMBERFORMAT_VALUE in the dialog
    { SID_ATTR_BRUSH,                       true,  true },    // ATTR_BACKGROUND
    { SID_SCATTR_PROTECTION,                false, true },    // ATTR_PROTECTION
    { SID_ATTR_BORDER_OUTER,                false, true },    // ATTR_BORDER
    { SID_ATTR_BORDER_INNER,                false, true },    // ATTR_BORDER_INNER
    { SID_ATTR_BORDER_SHADOW,               false, true },    // ATTR_SHADOW
    { 0,                                    false, true },    // ATTR_VALIDDATA
    { 0,                                    false, true },    // ATTR_CONDITIONAL
    { 0,                                    false, true },    // ATTR_HYPERLINK
    { 0,                                    true,  true },    // ATTR_PATTERN
    { SID_ATTR_LRSPACE,                     false, true },    // ATTR_LRSPACE
    { SID_ATTR_ULSPACE,                     false, true },    // ATTR_ULSPACE
    { SID_ATTR_PAGE,                        false, true },    // ATTR_PAGE
    { SID_ATTR_PAGE_PAPERBIN,               false, true },    // ATTR_PAGE_PAPERBIN
    { SID_ATTR_PAGE_SIZE,                   false, true },    // ATTR_PAGE_SIZE
    { SID_ATTR_PAGE_EXT1,                   false, true },    // ATTR_PAGE_HORCENTER
    { SID_ATTR_PAGE_EXT2,                   false, true },    // ATTR_PAGE_VERCENTER
    { SID_ATTR_PAGE_ON,                     false, true },    // ATTR_PAGE_ON
    { SID_ATTR_PAGE_DYNAMIC,                false, true },    // ATTR_PAGE_DYNAMIC
    { SID_ATTR_PAGE_SHARED,                 false, true },    // ATTR_PAGE_SHARED
    { SID_ATTR_PAGE_SHARED_FIRST,           false, true },    // ATTR_PAGE_SHARED_FIRST
    { 0,                                    false, true },    // ATTR_PAGE_NOTES aka. SID_SCATTR_PAGE_NOTES
    { 0,                                    false, true },    // ATTR_PAGE_GRID aka. SID_SCATTR_PAGE_GRID
    { 0,                                    false, true },    // ATTR_PAGE_HEADERS aka. SID_SCATTR_PAGE_HEADERS
    { 0,                                    false, true },    // ATTR_PAGE_CHARTS aka. SID_SCATTR_PAGE_CHARTS
    { 0,                                    false, true },    // ATTR_PAGE_OBJECTS aka. SID_SCATTR_PAGE_OBJECTS
    { 0,                                    false, true },    // ATTR_PAGE_DRAWINGS aka. SID_SCATTR_PAGE_DRAWINGS
    { 0,                                    false, true },    // ATTR_PAGE_TOPDOWN aka. SID_SCATTR_PAGE_TOPDOWN
    { 0,                                    false, true },    // ATTR_PAGE_SCALE aka SID_SCATTR_PAGE_SCALE
    { 0,                                    false, true },    // ATTR_PAGE_SCALETOPAGES aka SID_SCATTR_PAGE_SCALETOPAGES
    { 0,                                    false, true },    // ATTR_PAGE_FIRSTPAGENO aka SID_SCATTR_PAGE_FIRSTPAGENO
    { 0,                                    true,  true },    // ATTR_PAGE_HEADERLEFT aka SID_SCATTR_PAGE_HEADERLEFT
    { 0,                                    true,  true },    // ATTR_PAGE_FOOTERLEFT aka SID_SCATTR_PAGE_FOOTERLEFT
    { 0,                                    true,  true },    // ATTR_PAGE_HEADERRIGHT aka SID_SCATTR_PAGE_HEADERRIGHT
    { 0,                                    true,  true },    // ATTR_PAGE_FOOTERRIGHT aka. SID_SCATTR_PAGE_FOOTERRIGHT
    { 0,                                    true,  true },    // ATTR_PAGE_HEADERFIRST aka. SID_SCATTR_PAGE_HEADERFIRST
    { 0,                                    true,  true },    // ATTR_PAGE_FOOTERFIRST aka. SID_SCATTR_PAGE_FOOTERFIRST`
    { SID_ATTR_PAGE_HEADERSET,              false, true },    // ATTR_PAGE_HEADERSET
    { SID_ATTR_PAGE_FOOTERSET,              false, true },    // ATTR_PAGE_FOOTERSET
    { 0,                                    false, true },    // ATTR_PAGE_FORMULAS aka. SID_SCATTR_PAGE_FORMULAS
    { 0,                                    false, true },    // ATTR_PAGE_NULLVALS aka. SID_SCATTR_PAGE_NULLVALS
    { 0,                                    false, true },    // ATTR_PAGE_SCALETO aka. SID_SCATTR_PAGE_SCALETO
    { 0,                                    false, true }     // ATTR_HIDDEN
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
    ScModule::GetSpellSettings( nDefLang, nCjkLang, nCtlLang );

    //  latin font from GetDefaultFonts is not used, DEFAULTFONT_LATIN_SPREADSHEET instead
    SvxFontItem* pStdFont = getDefaultFontItem(nDefLang, DefaultFontType::LATIN_SPREADSHEET, ATTR_FONT);
    SvxFontItem* pCjkFont = getDefaultFontItem(nCjkLang, DefaultFontType::CJK_SPREADSHEET, ATTR_CJK_FONT);
    SvxFontItem* pCtlFont = getDefaultFontItem(nCtlLang, DefaultFontType::CTL_SPREADSHEET, ATTR_CTL_FONT);

    SvxBoxInfoItem* pGlobalBorderInnerAttr = new SvxBoxInfoItem( ATTR_BORDER_INNER );
    SfxItemSet      aSetItemItemSet( *this,
                                     svl::Items<ATTR_BACKGROUND, ATTR_BACKGROUND,
                                     ATTR_BORDER,     ATTR_SHADOW,
                                     ATTR_LRSPACE,    ATTR_ULSPACE,
                                     ATTR_PAGE_SIZE,  ATTR_PAGE_SIZE,
                                     ATTR_PAGE_ON,    ATTR_PAGE_SHARED_FIRST> );

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
    mvPoolDefaults[ ATTR_HYPHENATE       - ATTR_STARTINDEX ] = new ScHyphenateCell();
    mvPoolDefaults[ ATTR_SCRIPTSPACE     - ATTR_STARTINDEX ] = new SvxScriptSpaceItem( false, ATTR_SCRIPTSPACE);
    mvPoolDefaults[ ATTR_HANGPUNCTUATION - ATTR_STARTINDEX ] = new SvxHangingPunctuationItem( false, ATTR_HANGPUNCTUATION);
    mvPoolDefaults[ ATTR_FORBIDDEN_RULES - ATTR_STARTINDEX ] = new SvxForbiddenRuleItem( false, ATTR_FORBIDDEN_RULES);
    mvPoolDefaults[ ATTR_HOR_JUSTIFY     - ATTR_STARTINDEX ] = new SvxHorJustifyItem( SvxCellHorJustify::Standard, ATTR_HOR_JUSTIFY);
    mvPoolDefaults[ ATTR_HOR_JUSTIFY_METHOD - ATTR_STARTINDEX ] = new SvxJustifyMethodItem( SvxCellJustifyMethod::Auto, ATTR_HOR_JUSTIFY_METHOD);
    mvPoolDefaults[ ATTR_INDENT          - ATTR_STARTINDEX ] = new ScIndentItem( 0 );
    mvPoolDefaults[ ATTR_VER_JUSTIFY     - ATTR_STARTINDEX ] = new SvxVerJustifyItem( SvxCellVerJustify::Standard, ATTR_VER_JUSTIFY);
    mvPoolDefaults[ ATTR_VER_JUSTIFY_METHOD - ATTR_STARTINDEX ] = new SvxJustifyMethodItem( SvxCellJustifyMethod::Auto, ATTR_VER_JUSTIFY_METHOD);
    mvPoolDefaults[ ATTR_STACKED         - ATTR_STARTINDEX ] = new ScVerticalStackCell(false);
    mvPoolDefaults[ ATTR_ROTATE_VALUE    - ATTR_STARTINDEX ] = new ScRotateValueItem( 0_deg100 );
    mvPoolDefaults[ ATTR_ROTATE_MODE     - ATTR_STARTINDEX ] = new SvxRotateModeItem( SVX_ROTATE_MODE_BOTTOM, ATTR_ROTATE_MODE );
    mvPoolDefaults[ ATTR_VERTICAL_ASIAN  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_VERTICAL_ASIAN );
    //  The default for the ATTR_WRITINGDIR cell attribute must by SvxFrameDirection::Environment,
    //  so that value is returned when asking for a default cell's attributes.
    //  The value from the page style is set as DefaultHorizontalTextDirection for the EditEngine.
    mvPoolDefaults[ ATTR_WRITINGDIR      - ATTR_STARTINDEX ] = new SvxFrameDirectionItem( SvxFrameDirection::Environment, ATTR_WRITINGDIR );
    mvPoolDefaults[ ATTR_LINEBREAK       - ATTR_STARTINDEX ] = new ScLineBreakCell();
    mvPoolDefaults[ ATTR_SHRINKTOFIT     - ATTR_STARTINDEX ] = new ScShrinkToFitCell();
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

        mvPoolDefaults[ ATTR_PATTERN     - ATTR_STARTINDEX ] =
            new ScPatternAttr( SfxItemSetFixed<ATTR_PATTERN_START, ATTR_PATTERN_END>( *this ),
                               ScResId(STR_STYLENAME_STANDARD) );
    else
        mvPoolDefaults[ ATTR_PATTERN     - ATTR_STARTINDEX ] =
            new ScPatternAttr( SfxItemSetFixed<ATTR_PATTERN_START, ATTR_PATTERN_END>( *this ),
                               STRING_STANDARD ); // FIXME: without name?

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
    mvPoolDefaults[ ATTR_PAGE_SHARED_FIRST- ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_SHARED_FIRST, true );
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
    mvPoolDefaults[ ATTR_PAGE_HEADERFIRST- ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_HEADERFIRST );
    mvPoolDefaults[ ATTR_PAGE_FOOTERFIRST- ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_FOOTERFIRST );
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
    SetSecondaryPool(nullptr);

    for ( sal_uInt16 i=0; i < ATTR_ENDINDEX-ATTR_STARTINDEX+1; i++ )
    {
        ClearRefCount( *mvPoolDefaults[i] );
        delete mvPoolDefaults[i];
    }
}

void ScDocumentPool::newItem_Callback(const SfxPoolItem& rItem) const
{
    if (ATTR_PATTERN == rItem.Which() && 1 == rItem.GetRefCount())
    {
        const_cast<ScDocumentPool*>(this)->mnCurrentMaxKey++;
        const_cast<ScPatternAttr&>(static_cast<const ScPatternAttr&>(rItem)).SetPAKey(mnCurrentMaxKey);
    }
}

bool ScDocumentPool::newItem_UseDirect(const SfxPoolItem& rItem) const
{
    // I have evaluated that this is currently needed for ATTR_PATTERN/ScPatternAttr to work,
    // so this needs to stay at ptr-compare
    return (ATTR_PATTERN == rItem.Which() && areSfxPoolItemPtrsEqual(&rItem, mvPoolDefaults[ATTR_PATTERN - ATTR_STARTINDEX]));
}

void ScDocumentPool::StyleDeleted( const ScStyleSheet* pStyle )
{
    for (const SfxPoolItem* pItem : GetItemSurrogates( ATTR_PATTERN ))
    {
        ScPatternAttr* pPattern = const_cast<ScPatternAttr*>(dynamic_cast<const ScPatternAttr*>(pItem));
        if ( pPattern && pPattern->GetStyleSheet() == pStyle )
            pPattern->StyleToName();
    }
}

void ScDocumentPool::CellStyleCreated( std::u16string_view rName, const ScDocument& rDoc )
{
    // If a style was created, don't keep any pattern with its name string in the pool,
    // because it would compare equal to a pattern with a pointer to the new style.
    // Calling StyleSheetChanged isn't enough because the pool may still contain items
    // for undo or clipboard content.

    for (const SfxPoolItem* pItem : GetItemSurrogates( ATTR_PATTERN ))
    {
        auto pPattern = const_cast<ScPatternAttr*>(dynamic_cast<const ScPatternAttr*>(pItem));
        if ( pPattern && pPattern->GetStyleSheet() == nullptr )
        {
            const OUString* pStyleName = pPattern->GetStyleName();
            if ( pStyleName && *pStyleName == rName )
                pPattern->UpdateStyleSheet(rDoc); // find and store style pointer
        }
    }
}

rtl::Reference<SfxItemPool> ScDocumentPool::Clone() const
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

    if ( const SfxBoolItem* pItem = rSet.GetItemIfSet(ATTR_PAGE_ON,false) )
    {
        if( !pItem->GetValue() )
            return false;
    }

    SfxItemIter aIter( rSet );

    for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
    {
        sal_uInt16 nWhich = pItem->Which();

        OUString aText;

        switch( nWhich )
        {
            case ATTR_PAGE_ON:
            case ATTR_PAGE_DYNAMIC:
            case ATTR_PAGE_SHARED:
            case ATTR_PAGE_SHARED_FIRST:
            break;

            case ATTR_LRSPACE:
            {
                const SvxLRSpaceItem& rLRItem = static_cast<const SvxLRSpaceItem&>(*pItem);
                sal_uInt16 nPropLeftMargin  = rLRItem.GetPropLeft();
                sal_uInt16 nPropRightMargin = rLRItem.GetPropRight();
                sal_uInt16 nLeftMargin, nRightMargin;
                tools::Long nTmp;
                nTmp = rLRItem.GetLeft();
                nLeftMargin = nTmp < 0 ? 0 : sal_uInt16(nTmp);
                nTmp = rLRItem.GetRight();
                nRightMargin = nTmp < 0 ? 0 : sal_uInt16(nTmp);

                aText = EditResId(RID_SVXITEMS_LRSPACE_LEFT);
                if ( 100 != nPropLeftMargin )
                {
                    aText += unicode::formatPercent(nPropLeftMargin,
                        Application::GetSettings().GetUILanguageTag());
                }
                else
                {
                    aText += GetMetricText( static_cast<tools::Long>(nLeftMargin),
                                           eCoreMetric, ePresentationMetric, &rIntl ) +
                        " " + EditResId(GetMetricId(ePresentationMetric));
                }
                aText += cpDelim +
                    // We don't have a nPropFirstLineOffset
                    EditResId(RID_SVXITEMS_LRSPACE_RIGHT);
                if ( 100 != nPropRightMargin )
                {
                    aText += unicode::formatPercent(nPropLeftMargin,
                        Application::GetSettings().GetUILanguageTag());
                }
                else
                {
                    aText += GetMetricText( static_cast<tools::Long>(nRightMargin),
                                            eCoreMetric, ePresentationMetric, &rIntl ) +
                        " " + EditResId(GetMetricId(ePresentationMetric));
                }
            }
            break;

            default:
                pItem->GetPresentation( SfxItemPresentation::Complete, eCoreMetric, ePresentationMetric, aText, rIntl );

        }

        if ( aText.getLength() )
        {
            rText += aText + " + ";
        }
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
                rText += unicode::formatPercent(nPercent,
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
