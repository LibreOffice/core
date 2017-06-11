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

#include "scitems.hxx"

#include <comphelper/string.hxx>
#include <i18nutil/unicode.hxx>
#include <o3tl/make_unique.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svl/aeitem.hxx>
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
#include <svx/dialmgr.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/itemtype.hxx>
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
#include <svx/svxitems.hrc>
#include <editeng/udlnitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/justifyitem.hxx>

#include "docpool.hxx"
#include "global.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "globstr.hrc"
#include "document.hxx"
#include "sc.hrc"

sal_uInt16* ScDocumentPool::pVersionMap1 = nullptr;
sal_uInt16* ScDocumentPool::pVersionMap2 = nullptr;
sal_uInt16* ScDocumentPool::pVersionMap3 = nullptr;
sal_uInt16* ScDocumentPool::pVersionMap4 = nullptr;
sal_uInt16* ScDocumentPool::pVersionMap5 = nullptr;
sal_uInt16* ScDocumentPool::pVersionMap6 = nullptr;
sal_uInt16* ScDocumentPool::pVersionMap7 = nullptr;
sal_uInt16* ScDocumentPool::pVersionMap8 = nullptr;
sal_uInt16* ScDocumentPool::pVersionMap9 = nullptr;
sal_uInt16* ScDocumentPool::pVersionMap10 = nullptr;
sal_uInt16* ScDocumentPool::pVersionMap11 = nullptr;
sal_uInt16* ScDocumentPool::pVersionMap12 = nullptr;

// ATTR_FONT_TWOLINES (not used) was changed to ATTR_USERDEF (not saved in binary format) in 641c

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
    { 0,                            true },    // ATTR_PAGE_PAPERTRAY, since 303 just a dummy
    { SID_ATTR_PAGE_PAPERBIN,       true },    // ATTR_PAGE_PAPERBIN
    { SID_ATTR_PAGE_SIZE,           true },    // ATTR_PAGE_SIZE
    { SID_ATTR_PAGE_MAXSIZE,        true },    // ATTR_PAGE_MAXSIZE
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
    { SID_SCATTR_PAGE_PRINTAREA,    true },    // ATTR_PAGE_PRINTAREA
    { SID_SCATTR_PAGE_REPEATROW,    true },    // ATTR_PAGE_REPEATROW
    { SID_SCATTR_PAGE_REPEATCOL,    true },    // ATTR_PAGE_REPEATCOL
    { SID_SCATTR_PAGE_PRINTTABLES,  true },    // ATTR_PAGE_PRINTTABLES
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
                        aItemInfos, nullptr, false/*bLoadRefCounts*/ ),
    mnCurrentMaxKey(0)
{
    // this is a hack for unit tests that need to have a ScDocument
    // but don't want to depend on BootstrapFixture
    if (!pVersionMap1)
        InitVersionMaps();

    //  latin font from GetDefaultFonts is not used, DEFAULTFONT_LATIN_SPREADSHEET instead
    vcl::Font aStdFont = OutputDevice::GetDefaultFont( DefaultFontType::LATIN_SPREADSHEET, LANGUAGE_ENGLISH_US,
                                                    GetDefaultFontFlags::OnlyOne );
    SvxFontItem* pStdFont = new SvxFontItem( aStdFont.GetFamilyType(),
                                            aStdFont.GetFamilyName(), aStdFont.GetStyleName(),
                                            aStdFont.GetPitch(), aStdFont.GetCharSet(),
                                            ATTR_FONT );

    SvxFontItem* pCjkFont = new SvxFontItem( ATTR_CJK_FONT );
    SvxFontItem* pCtlFont = new SvxFontItem( ATTR_CTL_FONT );
    SvxFontItem aDummy( ATTR_FONT );
    GetDefaultFonts( aDummy, *pCjkFont, *pCtlFont );

    SvxBoxInfoItem* pGlobalBorderInnerAttr = new SvxBoxInfoItem( ATTR_BORDER_INNER );
    auto pSet = o3tl::make_unique<SfxItemSet>( *this, svl::Items<ATTR_PATTERN_START, ATTR_PATTERN_END>{} );
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

    mpPoolDefaults = new std::vector<SfxPoolItem*>(ATTR_ENDINDEX-ATTR_STARTINDEX+1);
    std::vector<SfxPoolItem*>& rPoolDefaults = *mpPoolDefaults;

    rPoolDefaults[ ATTR_FONT            - ATTR_STARTINDEX ] = pStdFont;
    rPoolDefaults[ ATTR_FONT_HEIGHT     - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200, 100, ATTR_FONT_HEIGHT );       // 10 pt;
    rPoolDefaults[ ATTR_FONT_WEIGHT     - ATTR_STARTINDEX ] = new SvxWeightItem( WEIGHT_NORMAL, ATTR_FONT_WEIGHT );
    rPoolDefaults[ ATTR_FONT_POSTURE    - ATTR_STARTINDEX ] = new SvxPostureItem( ITALIC_NONE, ATTR_FONT_POSTURE );
    rPoolDefaults[ ATTR_FONT_UNDERLINE  - ATTR_STARTINDEX ] = new SvxUnderlineItem( LINESTYLE_NONE, ATTR_FONT_UNDERLINE );
    rPoolDefaults[ ATTR_FONT_OVERLINE   - ATTR_STARTINDEX ] = new SvxOverlineItem( LINESTYLE_NONE, ATTR_FONT_OVERLINE );
    rPoolDefaults[ ATTR_FONT_CROSSEDOUT - ATTR_STARTINDEX ] = new SvxCrossedOutItem( STRIKEOUT_NONE, ATTR_FONT_CROSSEDOUT );
    rPoolDefaults[ ATTR_FONT_CONTOUR    - ATTR_STARTINDEX ] = new SvxContourItem( false, ATTR_FONT_CONTOUR );
    rPoolDefaults[ ATTR_FONT_SHADOWED   - ATTR_STARTINDEX ] = new SvxShadowedItem( false, ATTR_FONT_SHADOWED );
    rPoolDefaults[ ATTR_FONT_COLOR      - ATTR_STARTINDEX ] = new SvxColorItem( Color(COL_AUTO), ATTR_FONT_COLOR );
    rPoolDefaults[ ATTR_FONT_LANGUAGE   - ATTR_STARTINDEX ] = new SvxLanguageItem( LANGUAGE_DONTKNOW, ATTR_FONT_LANGUAGE );
    rPoolDefaults[ ATTR_CJK_FONT        - ATTR_STARTINDEX ] = pCjkFont;
    rPoolDefaults[ ATTR_CJK_FONT_HEIGHT - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200, 100, ATTR_CJK_FONT_HEIGHT );
    rPoolDefaults[ ATTR_CJK_FONT_WEIGHT - ATTR_STARTINDEX ] = new SvxWeightItem( WEIGHT_NORMAL, ATTR_CJK_FONT_WEIGHT );
    rPoolDefaults[ ATTR_CJK_FONT_POSTURE- ATTR_STARTINDEX ] = new SvxPostureItem( ITALIC_NONE, ATTR_CJK_FONT_POSTURE );
    rPoolDefaults[ ATTR_CJK_FONT_LANGUAGE-ATTR_STARTINDEX ] = new SvxLanguageItem( LANGUAGE_DONTKNOW, ATTR_CJK_FONT_LANGUAGE );
    rPoolDefaults[ ATTR_CTL_FONT        - ATTR_STARTINDEX ] = pCtlFont;
    rPoolDefaults[ ATTR_CTL_FONT_HEIGHT - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200, 100, ATTR_CTL_FONT_HEIGHT );
    rPoolDefaults[ ATTR_CTL_FONT_WEIGHT - ATTR_STARTINDEX ] = new SvxWeightItem( WEIGHT_NORMAL, ATTR_CTL_FONT_WEIGHT );
    rPoolDefaults[ ATTR_CTL_FONT_POSTURE- ATTR_STARTINDEX ] = new SvxPostureItem( ITALIC_NONE, ATTR_CTL_FONT_POSTURE );
    rPoolDefaults[ ATTR_CTL_FONT_LANGUAGE-ATTR_STARTINDEX ] = new SvxLanguageItem( LANGUAGE_DONTKNOW, ATTR_CTL_FONT_LANGUAGE );
    rPoolDefaults[ ATTR_FONT_EMPHASISMARK-ATTR_STARTINDEX ] = new SvxEmphasisMarkItem( FontEmphasisMark::NONE, ATTR_FONT_EMPHASISMARK );
    rPoolDefaults[ ATTR_USERDEF         - ATTR_STARTINDEX ] = new SvXMLAttrContainerItem( ATTR_USERDEF );
    rPoolDefaults[ ATTR_FONT_WORDLINE   - ATTR_STARTINDEX ] = new SvxWordLineModeItem(false, ATTR_FONT_WORDLINE );
    rPoolDefaults[ ATTR_FONT_RELIEF     - ATTR_STARTINDEX ] = new SvxCharReliefItem( FontRelief::NONE, ATTR_FONT_RELIEF );
    rPoolDefaults[ ATTR_HYPHENATE       - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_HYPHENATE );
    rPoolDefaults[ ATTR_SCRIPTSPACE     - ATTR_STARTINDEX ] = new SvxScriptSpaceItem( false, ATTR_SCRIPTSPACE);
    rPoolDefaults[ ATTR_HANGPUNCTUATION - ATTR_STARTINDEX ] = new SvxHangingPunctuationItem( false, ATTR_HANGPUNCTUATION);
    rPoolDefaults[ ATTR_FORBIDDEN_RULES - ATTR_STARTINDEX ] = new SvxForbiddenRuleItem( false, ATTR_FORBIDDEN_RULES);
    rPoolDefaults[ ATTR_HOR_JUSTIFY     - ATTR_STARTINDEX ] = new SvxHorJustifyItem( SvxCellHorJustify::Standard, ATTR_HOR_JUSTIFY);
    rPoolDefaults[ ATTR_HOR_JUSTIFY_METHOD - ATTR_STARTINDEX ] = new SvxJustifyMethodItem( SvxCellJustifyMethod::Auto, ATTR_HOR_JUSTIFY_METHOD);
    rPoolDefaults[ ATTR_INDENT          - ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_INDENT, 0 );
    rPoolDefaults[ ATTR_VER_JUSTIFY     - ATTR_STARTINDEX ] = new SvxVerJustifyItem( SVX_VER_JUSTIFY_STANDARD, ATTR_VER_JUSTIFY);
    rPoolDefaults[ ATTR_VER_JUSTIFY_METHOD - ATTR_STARTINDEX ] = new SvxJustifyMethodItem( SvxCellJustifyMethod::Auto, ATTR_VER_JUSTIFY_METHOD);
    rPoolDefaults[ ATTR_STACKED         - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_STACKED, false );
    rPoolDefaults[ ATTR_ROTATE_VALUE    - ATTR_STARTINDEX ] = new SfxInt32Item( ATTR_ROTATE_VALUE, 0 );
    rPoolDefaults[ ATTR_ROTATE_MODE     - ATTR_STARTINDEX ] = new SvxRotateModeItem( SVX_ROTATE_MODE_BOTTOM, ATTR_ROTATE_MODE );
    rPoolDefaults[ ATTR_VERTICAL_ASIAN  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_VERTICAL_ASIAN );
    //  The default for the ATTR_WRITINGDIR cell attribute must by SvxFrameDirection::Environment,
    //  so that value is returned when asking for a default cell's attributes.
    //  The value from the page style is set as DefaultHorizontalTextDirection for the EditEngine.
    rPoolDefaults[ ATTR_WRITINGDIR      - ATTR_STARTINDEX ] = new SvxFrameDirectionItem( SvxFrameDirection::Environment, ATTR_WRITINGDIR );
    rPoolDefaults[ ATTR_LINEBREAK       - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_LINEBREAK );
    rPoolDefaults[ ATTR_SHRINKTOFIT     - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_SHRINKTOFIT );
    rPoolDefaults[ ATTR_BORDER_TLBR     - ATTR_STARTINDEX ] = new SvxLineItem( ATTR_BORDER_TLBR );
    rPoolDefaults[ ATTR_BORDER_BLTR     - ATTR_STARTINDEX ] = new SvxLineItem( ATTR_BORDER_BLTR );
    rPoolDefaults[ ATTR_MARGIN          - ATTR_STARTINDEX ] = new SvxMarginItem( ATTR_MARGIN );
    rPoolDefaults[ ATTR_MERGE           - ATTR_STARTINDEX ] = new ScMergeAttr;
    rPoolDefaults[ ATTR_MERGE_FLAG      - ATTR_STARTINDEX ] = new ScMergeFlagAttr;
    rPoolDefaults[ ATTR_VALUE_FORMAT    - ATTR_STARTINDEX ] = new SfxUInt32Item( ATTR_VALUE_FORMAT, 0 );
    rPoolDefaults[ ATTR_LANGUAGE_FORMAT - ATTR_STARTINDEX ] = new SvxLanguageItem( ScGlobal::eLnge, ATTR_LANGUAGE_FORMAT );
    rPoolDefaults[ ATTR_BACKGROUND      - ATTR_STARTINDEX ] = new SvxBrushItem( Color(COL_TRANSPARENT), ATTR_BACKGROUND );
    rPoolDefaults[ ATTR_PROTECTION      - ATTR_STARTINDEX ] = new ScProtectionAttr;
    rPoolDefaults[ ATTR_BORDER          - ATTR_STARTINDEX ] = new SvxBoxItem( ATTR_BORDER );
    rPoolDefaults[ ATTR_BORDER_INNER    - ATTR_STARTINDEX ] = pGlobalBorderInnerAttr;
    rPoolDefaults[ ATTR_SHADOW          - ATTR_STARTINDEX ] = new SvxShadowItem( ATTR_SHADOW );
    rPoolDefaults[ ATTR_VALIDDATA       - ATTR_STARTINDEX ] = new SfxUInt32Item( ATTR_VALIDDATA, 0 );
    rPoolDefaults[ ATTR_CONDITIONAL     - ATTR_STARTINDEX ] = new ScCondFormatItem;
    rPoolDefaults[ ATTR_HYPERLINK       - ATTR_STARTINDEX ] = new SfxStringItem( ATTR_HYPERLINK, OUString() ) ;

    // GetRscString only works after ScGlobal::Init (indicated by the EmptyBrushItem)
    // TODO: Write additional method ScGlobal::IsInit() or somesuch
    //       or detect whether this is the Secondary Pool for a MessagePool
    if ( ScGlobal::GetEmptyBrushItem() )
        rPoolDefaults[ ATTR_PATTERN     - ATTR_STARTINDEX ] = new ScPatternAttr( std::move(pSet), ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );
    else
        rPoolDefaults[ ATTR_PATTERN     - ATTR_STARTINDEX ] = new ScPatternAttr( std::move(pSet), STRING_STANDARD ); // FIXME: without name?

    rPoolDefaults[ ATTR_LRSPACE         - ATTR_STARTINDEX ] = new SvxLRSpaceItem( ATTR_LRSPACE );
    rPoolDefaults[ ATTR_ULSPACE         - ATTR_STARTINDEX ] = new SvxULSpaceItem( ATTR_ULSPACE );
    rPoolDefaults[ ATTR_PAGE            - ATTR_STARTINDEX ] = new SvxPageItem( ATTR_PAGE );
    rPoolDefaults[ ATTR_PAGE_PAPERTRAY  - ATTR_STARTINDEX ] = new SfxAllEnumItem( ATTR_PAGE_PAPERTRAY );
    rPoolDefaults[ ATTR_PAGE_PAPERBIN   - ATTR_STARTINDEX ] = new SvxPaperBinItem( ATTR_PAGE_PAPERBIN );
    rPoolDefaults[ ATTR_PAGE_SIZE       - ATTR_STARTINDEX ] = new SvxSizeItem( ATTR_PAGE_SIZE );
    rPoolDefaults[ ATTR_PAGE_MAXSIZE    - ATTR_STARTINDEX ] = new SvxSizeItem( ATTR_PAGE_MAXSIZE );
    rPoolDefaults[ ATTR_PAGE_HORCENTER  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_HORCENTER );
    rPoolDefaults[ ATTR_PAGE_VERCENTER  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_VERCENTER );
    rPoolDefaults[ ATTR_PAGE_ON         - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_ON, true );
    rPoolDefaults[ ATTR_PAGE_DYNAMIC    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_DYNAMIC, true );
    rPoolDefaults[ ATTR_PAGE_SHARED     - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_SHARED, true );
    rPoolDefaults[ ATTR_PAGE_NOTES      - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_NOTES, false );
    rPoolDefaults[ ATTR_PAGE_GRID       - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_GRID, false );
    rPoolDefaults[ ATTR_PAGE_HEADERS    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_HEADERS, false );
    rPoolDefaults[ ATTR_PAGE_CHARTS     - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_CHARTS );
    rPoolDefaults[ ATTR_PAGE_OBJECTS    - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_OBJECTS );
    rPoolDefaults[ ATTR_PAGE_DRAWINGS   - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_DRAWINGS );
    rPoolDefaults[ ATTR_PAGE_TOPDOWN    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_TOPDOWN, true );
    rPoolDefaults[ ATTR_PAGE_SCALE      - ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_SCALE, 100 );
    rPoolDefaults[ ATTR_PAGE_SCALETOPAGES-ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, 1 );
    rPoolDefaults[ ATTR_PAGE_FIRSTPAGENO- ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, 1 );
    rPoolDefaults[ ATTR_PAGE_PRINTAREA  - ATTR_STARTINDEX ] = new ScRangeItem( ATTR_PAGE_PRINTAREA );
    rPoolDefaults[ ATTR_PAGE_REPEATROW  - ATTR_STARTINDEX ] = new ScRangeItem( ATTR_PAGE_REPEATROW );
    rPoolDefaults[ ATTR_PAGE_REPEATCOL  - ATTR_STARTINDEX ] = new ScRangeItem( ATTR_PAGE_REPEATCOL );
    rPoolDefaults[ ATTR_PAGE_PRINTTABLES- ATTR_STARTINDEX ] = new ScTableListItem;
    rPoolDefaults[ ATTR_PAGE_HEADERLEFT - ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_HEADERLEFT );
    rPoolDefaults[ ATTR_PAGE_FOOTERLEFT - ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_FOOTERLEFT );
    rPoolDefaults[ ATTR_PAGE_HEADERRIGHT- ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_HEADERRIGHT );
    rPoolDefaults[ ATTR_PAGE_FOOTERRIGHT- ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_FOOTERRIGHT );
    rPoolDefaults[ ATTR_PAGE_HEADERSET  - ATTR_STARTINDEX ] = new SvxSetItem( ATTR_PAGE_HEADERSET, aSetItemItemSet );
    rPoolDefaults[ ATTR_PAGE_FOOTERSET  - ATTR_STARTINDEX ] = new SvxSetItem( ATTR_PAGE_FOOTERSET, aSetItemItemSet );
    rPoolDefaults[ ATTR_PAGE_FORMULAS   - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_FORMULAS, false );
    rPoolDefaults[ ATTR_PAGE_NULLVALS   - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_NULLVALS, true );
    rPoolDefaults[ ATTR_PAGE_SCALETO    - ATTR_STARTINDEX ] = new ScPageScaleToItem( 1, 1 );
    rPoolDefaults[ ATTR_HIDDEN          - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_HIDDEN, false );

    SetDefaults( mpPoolDefaults );

    // ATTR_LANGUAGE_FORMAT from sv329 inserted, VersionMap in _ScGlobal__Init
    SetVersionMap( 1, 100, 157, pVersionMap1 );

    // ATTR_VALIDDATA, ATTR_CONDITIONAL from 341
    SetVersionMap( 2, 100, 158, pVersionMap2 );

    // ATTR_INDENT from 350
    SetVersionMap( 3, 100, 160, pVersionMap3 );

    // ATTR_ROTATE_VALUE, ATTR_ROTATE_MODE from 367
    SetVersionMap( 4, 100, 161, pVersionMap4 );

    // CJK, CTL, EMPHASISMARK, TWOLINES from 614
    SetVersionMap( 5, 100, 163, pVersionMap5 );

    // ATTR_SCRIPTSPACE, ATTR_HANGPUNCTUATION, ATTR_FORBIDDEN_RULES from 614d
    SetVersionMap( 6, 100, 175, pVersionMap6 );

    // ATTR_FONT_WORDLINE, ATTR_FONT_RELIEF, ATTR_HYPHENATE from 632b
    SetVersionMap( 7, 100, 178, pVersionMap7 );

    // ATTR_VERTICAL_ASIAN from 642q
    SetVersionMap( 8, 100, 181, pVersionMap8 );

    // ATTR_WRITINGDIR from 643y
    SetVersionMap( 9, 100, 182, pVersionMap9 );

    // ATTR_PAGE_SCALETO added in 680/sab008
    // new version map not required

    // ATTR_SHRINKTOFIT, ATTR_BORDER_TL_BR, ATTR_BORDER_BL_TR added in 680/dr14
    SetVersionMap( 10, 100, 184, pVersionMap10 );

    // ATTR_FONT_OVERLINE added in DEV300/overline2
    SetVersionMap( 11, 100, 187, pVersionMap11 );
    // ATTR_HYERLINK added
    SetVersionMap( 12, 100, 192, pVersionMap12 );
}

ScDocumentPool::~ScDocumentPool()
{
    Delete();

    for ( sal_uInt16 i=0; i < ATTR_ENDINDEX-ATTR_STARTINDEX+1; i++ )
    {
        ClearRefCount( *(*mpPoolDefaults)[i] );
        delete (*mpPoolDefaults)[i];
    }

    delete mpPoolDefaults;
}

void ScDocumentPool::InitVersionMaps()
{
    OSL_PRECOND( !pVersionMap1 && !pVersionMap2 &&
                !pVersionMap3 && !pVersionMap4 &&
                !pVersionMap5 && !pVersionMap6 &&
                !pVersionMap7 && !pVersionMap8 &&
                !pVersionMap9 && !pVersionMap10 &&
                !pVersionMap11 && !pVersionMap12 , "InitVersionMaps call multiple times" );

    // Map old WhichId's
    // Do not count with ATTR_*, if they change again

    // First Map: ATTR_LANGUAGE_FORMAT from sv329 inserted

    const sal_uInt16 nMap1Start = 100;  // Old ATTR_STARTINDEX
    const sal_uInt16 nMap1End   = 157;  // Old ATTR_ENDINDEX
    const sal_uInt16 nMap1Count = nMap1End - nMap1Start + 1;
    const sal_uInt16 nMap1New   = 18;   // ATTR_LANGUAGE_FORMAT - ATTR_STARTINDEX
    pVersionMap1 = new sal_uInt16 [ nMap1Count ];
    sal_uInt16 i, j;
    for ( i=0, j=nMap1Start; i < nMap1New; i++, j++ )
        pVersionMap1[i] = j;
    // An entry inserted ...
    for ( i=nMap1New, j=nMap1Start+nMap1New+1; i < nMap1Count; i++, j++ )
        pVersionMap1[i] = j;

    // Second Map: ATTR_VALIDDATA and ATTR_CONDITIONAL from 341 inserted

    const sal_uInt16 nMap2Start = 100;  // ATTR_STARTINDEX
    const sal_uInt16 nMap2End   = 158;  // ATTR_ENDINDEX
    const sal_uInt16 nMap2Count = nMap2End - nMap2Start + 1;
    const sal_uInt16 nMap2New   = 24;   // ATTR_VALIDDATA - ATTR_STARTINDEX
    pVersionMap2 = new sal_uInt16 [ nMap2Count ];
    for ( i=0, j=nMap2Start; i < nMap2New; i++, j++ )
        pVersionMap2[i] = j;
    // Two entries inserted ...
    for ( i=nMap2New, j=nMap2Start+nMap2New+2; i < nMap2Count; i++, j++ )
        pVersionMap2[i] = j;

    // Third Map: ATTR_INDENT from 350 inserted

    const sal_uInt16 nMap3Start = 100;  // ATTR_STARTINDEX
    const sal_uInt16 nMap3End   = 160;  // ATTR_ENDINDEX
    const sal_uInt16 nMap3Count = nMap3End - nMap3Start + 1;
    const sal_uInt16 nMap3New   = 11;   // ATTR_INDENT - ATTR_STARTINDEX
    pVersionMap3 = new sal_uInt16 [ nMap3Count ];
    for ( i=0, j=nMap3Start; i < nMap3New; i++, j++ )
        pVersionMap3[i] = j;
    // An entry inserted...
    for ( i=nMap3New, j=nMap3Start+nMap3New+1; i < nMap3Count; i++, j++ )
        pVersionMap3[i] = j;

    // Fourth Map: ATTR_ROTATE_VALUE and ATTR_ROTATE_MODE from 367 inserted

    const sal_uInt16 nMap4Start = 100;  // ATTR_STARTINDEX
    const sal_uInt16 nMap4End   = 161;  // ATTR_ENDINDEX
    const sal_uInt16 nMap4Count = nMap4End - nMap4Start + 1;
    const sal_uInt16 nMap4New   = 14;   // ATTR_ROTATE_VALUE - ATTR_STARTINDEX
    pVersionMap4 = new sal_uInt16 [ nMap4Count ];
    for ( i=0, j=nMap4Start; i < nMap4New; i++, j++ )
        pVersionMap4[i] = j;
    // Two entries inserted ...
    for ( i=nMap4New, j=nMap4Start+nMap4New+2; i < nMap4Count; i++, j++ )
        pVersionMap4[i] = j;

    // fifth map: CJK..., CTL..., EMPHASISMARK, TWOLINES (12 items) added in 614

    const sal_uInt16 nMap5Start = 100;  // ATTR_STARTINDEX
    const sal_uInt16 nMap5End   = 163;  // ATTR_ENDINDEX
    const sal_uInt16 nMap5Count = nMap5End - nMap5Start + 1;
    const sal_uInt16 nMap5New   = 10;   // ATTR_CJK_FONT - ATTR_STARTINDEX
    pVersionMap5 = new sal_uInt16 [ nMap5Count ];
    for ( i=0, j=nMap5Start; i < nMap5New; i++, j++ )
        pVersionMap5[i] = j;
    // 12 entries inserted
    for ( i=nMap5New, j=nMap5Start+nMap5New+12; i < nMap5Count; i++, j++ )
        pVersionMap5[i] = j;

    // sixth map: ATTR_SCRIPTSPACE, ATTR_HANGPUNCTUATION, ATTR_FORBIDDEN_RULES added in 614d

    const sal_uInt16 nMap6Start = 100;  // ATTR_STARTINDEX
    const sal_uInt16 nMap6End   = 175;  // ATTR_ENDINDEX
    const sal_uInt16 nMap6Count = nMap6End - nMap6Start + 1;
    const sal_uInt16 nMap6New   = 22;   // ATTR_SCRIPTSPACE - ATTR_STARTINDEX
    pVersionMap6 = new sal_uInt16 [ nMap6Count ];
    for ( i=0, j=nMap6Start; i < nMap6New; i++, j++ )
        pVersionMap6[i] = j;
    // 3 entries inserted
    for ( i=nMap6New, j=nMap6Start+nMap6New+3; i < nMap6Count; i++, j++ )
        pVersionMap6[i] = j;

    // seventh map: ATTR_FONT_WORDLINE, ATTR_FONT_RELIEF, ATTR_HYPHENATE added in 632b

    const sal_uInt16 nMap7Start = 100;  // ATTR_STARTINDEX
    const sal_uInt16 nMap7End   = 178;  // ATTR_ENDINDEX
    const sal_uInt16 nMap7Count = nMap7End - nMap7Start + 1;
    const sal_uInt16 nMap7New   = 22;   // ATTR_FONT_WORDLINE - ATTR_STARTINDEX
    pVersionMap7 = new sal_uInt16 [ nMap7Count ];
    for ( i=0, j=nMap7Start; i < nMap7New; i++, j++ )
        pVersionMap7[i] = j;
    // 3 entries inserted
    for ( i=nMap7New, j=nMap7Start+nMap7New+3; i < nMap7Count; i++, j++ )
        pVersionMap7[i] = j;

    // eighth map: ATTR_VERTICAL_ASIAN added in 642q

    const sal_uInt16 nMap8Start = 100;  // ATTR_STARTINDEX
    const sal_uInt16 nMap8End   = 181;  // ATTR_ENDINDEX
    const sal_uInt16 nMap8Count = nMap8End - nMap8Start + 1;
    const sal_uInt16 nMap8New   = 34;   // ATTR_VERTICAL_ASIAN - ATTR_STARTINDEX
    pVersionMap8 = new sal_uInt16 [ nMap8Count ];
    for ( i=0, j=nMap8Start; i < nMap8New; i++, j++ )
        pVersionMap8[i] = j;
    // 1 entry inserted
    for ( i=nMap8New, j=nMap8Start+nMap8New+1; i < nMap8Count; i++, j++ )
        pVersionMap8[i] = j;

    // 9th map: ATTR_WRITINGDIR added in 643y

    const sal_uInt16 nMap9Start = 100;  // ATTR_STARTINDEX
    const sal_uInt16 nMap9End   = 182;  // ATTR_ENDINDEX
    const sal_uInt16 nMap9Count = nMap9End - nMap9Start + 1;
    const sal_uInt16 nMap9New   = 35;   // ATTR_WRITINGDIR - ATTR_STARTINDEX
    pVersionMap9 = new sal_uInt16 [ nMap9Count ];
    for ( i=0, j=nMap9Start; i < nMap9New; i++, j++ )
        pVersionMap9[i] = j;
    // 1 entry inserted
    for ( i=nMap9New, j=nMap9Start+nMap9New+1; i < nMap9Count; i++, j++ )
        pVersionMap9[i] = j;

    // ATTR_PAGE_SCALETO added in 680/sab008

    // 10th map: ATTR_SHRINKTOFIT, ATTR_BORDER_TL_BR, ATTR_BORDER_BL_TR added in 680/dr14

    const sal_uInt16 nMap10Start = 100;  // ATTR_STARTINDEX
    const sal_uInt16 nMap10End   = 184;  // ATTR_ENDINDEX
    const sal_uInt16 nMap10Count = nMap10End - nMap10Start + 1;
    const sal_uInt16 nMap10New   = 37;   // ATTR_SHRINKTOFIT - ATTR_STARTINDEX
    pVersionMap10 = new sal_uInt16 [ nMap10Count ];
    for ( i=0, j=nMap10Start; i < nMap10New; i++, j++ )
        pVersionMap10[i] = j;
    // 3 entries inserted
    for ( i=nMap10New, j=nMap10Start+nMap10New+3; i < nMap10Count; i++, j++ )
        pVersionMap10[i] = j;

    // 11th map: ATTR_FONT_OVERLINE added in DEV300/overline2

    const sal_uInt16 nMap11Start = 100;  // ATTR_STARTINDEX
    const sal_uInt16 nMap11End   = 187;  // ATTR_ENDINDEX
    const sal_uInt16 nMap11Count = nMap11End - nMap11Start + 1;
    const sal_uInt16 nMap11New   = 5;    // ATTR_FONT_OVERLINE - ATTR_STARTINDEX
    pVersionMap11 = new sal_uInt16 [ nMap11Count ];
    for ( i=0, j=nMap11Start; i < nMap11New; i++, j++ )
        pVersionMap11[i] = j;
    // 1 entry inserted
    for ( i=nMap11New, j=nMap11Start+nMap11New+1; i < nMap11Count; i++, j++ )
        pVersionMap11[i] = j;

    const sal_uInt16 nMap12Start = 100;  // ATTR_STARTINDEX
    const sal_uInt16 nMap12End   = 192;  // ATTR_ENDINDEX
    const sal_uInt16 nMap12Count = nMap12End - nMap12Start + 1;
    const sal_uInt16 nMap12New   = 55;    // ATTR_HYPERLINK - ATTR_STARTINDEX
    pVersionMap12 = new sal_uInt16 [ nMap12Count ];
    for ( i=0, j=nMap12Start; i < nMap12New; i++, j++ )
        pVersionMap12[i] = j;
    // 1 entry inserted
    for ( i=nMap12New, j=nMap12Start+nMap12New+1; i < nMap12Count; i++, j++ )
        pVersionMap12[i] = j;
}

void ScDocumentPool::DeleteVersionMaps()
{
    OSL_PRECOND( pVersionMap1 && pVersionMap2 &&
                pVersionMap3 && pVersionMap4 &&
                pVersionMap5 && pVersionMap6 &&
                pVersionMap7 && pVersionMap8 &&
                pVersionMap9 && pVersionMap10 &&
                pVersionMap11 && pVersionMap12 , "DeleteVersionMaps without maps" );

    delete[] pVersionMap12;
    pVersionMap12 = nullptr;
    delete[] pVersionMap11;
    pVersionMap11 = nullptr;
    delete[] pVersionMap10;
    pVersionMap10 = nullptr;
    delete[] pVersionMap9;
    pVersionMap9 = nullptr;
    delete[] pVersionMap8;
    pVersionMap8 = nullptr;
    delete[] pVersionMap7;
    pVersionMap7 = nullptr;
    delete[] pVersionMap6;
    pVersionMap6 = nullptr;
    delete[] pVersionMap5;
    pVersionMap5 = nullptr;
    delete[] pVersionMap4;
    pVersionMap4 = nullptr;
    delete[] pVersionMap3;
    pVersionMap3 = nullptr;
    delete[] pVersionMap2;
    pVersionMap2 = nullptr;
    delete[] pVersionMap1;
    pVersionMap1 = nullptr;
}

const SfxPoolItem& ScDocumentPool::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
{
    if ( rItem.Which() != ATTR_PATTERN ) // Only Pattern is special
        return SfxItemPool::Put( rItem, nWhich );

    // Don't copy the default pattern of this Pool
    if (&rItem == (*mpPoolDefaults)[ ATTR_PATTERN - ATTR_STARTINDEX ])
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

void ScDocumentPool::StyleDeleted( ScStyleSheet* pStyle )
{
    sal_uInt32 nCount = GetItemCount2(ATTR_PATTERN);
    for (sal_uInt32 i=0; i<nCount; i++)
    {
        ScPatternAttr* pPattern = const_cast<ScPatternAttr*>(static_cast<const ScPatternAttr*>(GetItem2(ATTR_PATTERN, i)));
        if ( pPattern && pPattern->GetStyleSheet() == pStyle )
            pPattern->StyleToName();
    }
}

void ScDocumentPool::CellStyleCreated( const OUString& rName, ScDocument* pDoc )
{
    // If a style was created, don't keep any pattern with its name string in the pool,
    // because it would compare equal to a pattern with a pointer to the new style.
    // Calling StyleSheetChanged isn't enough because the pool may still contain items
    // for undo or clipboard content.

    sal_uInt32 nCount = GetItemCount2(ATTR_PATTERN);
    for (sal_uInt32 i=0; i<nCount; i++)
    {
        ScPatternAttr *const pPattern =
            const_cast<ScPatternAttr*>(
                static_cast<ScPatternAttr const*>(GetItem2(ATTR_PATTERN, i)));
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
    const IntlWrapper* pIntl
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
    OUString aText;

    while( pItem )
    {
        sal_uInt16 nWhich = pItem->Which();

        aText.clear();

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
                    aText += GetMetricText( (long)nLeftMargin,
                                           eCoreMetric, ePresentationMetric, pIntl );
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
                    aText += GetMetricText( (long)nRightMargin,
                                            eCoreMetric, ePresentationMetric, pIntl );
                    aText += " " + EditResId(GetMetricId(ePresentationMetric));
                }
            }
            break;

            default:
                if ( !pIntl )
                    pIntl = ScGlobal::GetScIntlWrapper();
                pItem->GetPresentation( SfxItemPresentation::Complete, eCoreMetric, ePresentationMetric, aText, pIntl );

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
    const IntlWrapper* pIntl ) const
{
    sal_uInt16  nW = rItem.Which();
    OUString aStrYes  ( ScGlobal::GetRscString(STR_YES) );
    OUString aStrNo   ( ScGlobal::GetRscString(STR_NO) );
    OUString aStrSep(": ");

    bool ePresentationRet = true;
    switch( nW )
    {
        case ATTR_PAGE_TOPDOWN:
            rText = ScGlobal::GetRscString(STR_SCATTR_PAGE_PRINTDIR) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ?
                ScGlobal::GetRscString(STR_SCATTR_PAGE_TOPDOWN) :
                ScGlobal::GetRscString(STR_SCATTR_PAGE_LEFTRIGHT) ;
        break;

        case ATTR_PAGE_HEADERS:
            rText = ScGlobal::GetRscString(STR_SCATTR_PAGE_HEADERS) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_NULLVALS:
            rText = ScGlobal::GetRscString(STR_SCATTR_PAGE_NULLVALS) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_FORMULAS:
            rText = ScGlobal::GetRscString(STR_SCATTR_PAGE_FORMULAS) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_NOTES:
            rText = ScGlobal::GetRscString(STR_SCATTR_PAGE_NOTES) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_GRID:
            rText = ScGlobal::GetRscString(STR_SCATTR_PAGE_GRID) + aStrSep;
            rText += static_cast<const SfxBoolItem&>(rItem).GetValue() ? aStrYes : aStrNo ;
        break;

        case ATTR_PAGE_SCALETOPAGES:
        {
            sal_uInt16  nPagNo = static_cast<const SfxUInt16Item&>(rItem).GetValue();

            if( nPagNo )
            {
                rText = ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALETOPAGES ) + aStrSep;
                OUString aPages( ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALE_PAGES ) );
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
                rText = ScGlobal::GetRscString(STR_SCATTR_PAGE_FIRSTPAGENO) + aStrSep;
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
                rText = ScGlobal::GetRscString(STR_SCATTR_PAGE_SCALE) + aStrSep;
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

            if( lcl_HFPresentation( rItem, GetMetric( nW ), ePresentationMetric, aBuffer, pIntl ) )
            {
                rText = ScGlobal::GetRscString(STR_HEADER) + " ( " + aBuffer + " ) ";
            }
        }
        break;

        case ATTR_PAGE_FOOTERSET:
        {
            OUString  aBuffer;

            if( lcl_HFPresentation( rItem, GetMetric( nW ), ePresentationMetric, aBuffer, pIntl ) )
            {
                rText = ScGlobal::GetRscString(STR_FOOTER) + " ( " + aBuffer + " ) ";
            }
        }
        break;

        default:
            if ( !pIntl )
                pIntl = ScGlobal::GetScIntlWrapper();
            ePresentationRet = rItem.GetPresentation( SfxItemPresentation::Complete, GetMetric( nW ), ePresentationMetric, rText, pIntl );
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
