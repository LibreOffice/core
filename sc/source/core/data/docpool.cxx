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

#include "scitems.hxx"

#include <comphelper/string.hxx>
#include <i18nutil/unicode.hxx>
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

#define SC_MAX_POOLREF      (SFX_ITEMS_OLD_MAXREF - 39)
#define SC_SAFE_POOLREF     (SC_MAX_POOLREF + 20)

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
    { SID_ATTR_CHAR_FONT,           SfxItemPoolFlags::POOLABLE },    // ATTR_FONT
    { SID_ATTR_CHAR_FONTHEIGHT,     SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_HEIGHT
    { SID_ATTR_CHAR_WEIGHT,         SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_WEIGHT
    { SID_ATTR_CHAR_POSTURE,        SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_POSTURE
    { SID_ATTR_CHAR_UNDERLINE,      SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_UNDERLINE
    { SID_ATTR_CHAR_OVERLINE,       SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_OVERLINE
    { SID_ATTR_CHAR_STRIKEOUT,      SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_CROSSEDOUT
    { SID_ATTR_CHAR_CONTOUR,        SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_CONTOUR
    { SID_ATTR_CHAR_SHADOWED,       SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_SHADOWED
    { SID_ATTR_CHAR_COLOR,          SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_COLOR
    { SID_ATTR_CHAR_LANGUAGE,       SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_LANGUAGE
    { SID_ATTR_CHAR_CJK_FONT,       SfxItemPoolFlags::POOLABLE },    // ATTR_CJK_FONT            from 614
    { SID_ATTR_CHAR_CJK_FONTHEIGHT, SfxItemPoolFlags::POOLABLE },    // ATTR_CJK_FONT_HEIGHT     from 614
    { SID_ATTR_CHAR_CJK_WEIGHT,     SfxItemPoolFlags::POOLABLE },    // ATTR_CJK_FONT_WEIGHT     from 614
    { SID_ATTR_CHAR_CJK_POSTURE,    SfxItemPoolFlags::POOLABLE },    // ATTR_CJK_FONT_POSTURE    from 614
    { SID_ATTR_CHAR_CJK_LANGUAGE,   SfxItemPoolFlags::POOLABLE },    // ATTR_CJK_FONT_LANGUAGE   from 614
    { SID_ATTR_CHAR_CTL_FONT,       SfxItemPoolFlags::POOLABLE },    // ATTR_CTL_FONT            from 614
    { SID_ATTR_CHAR_CTL_FONTHEIGHT, SfxItemPoolFlags::POOLABLE },    // ATTR_CTL_FONT_HEIGHT     from 614
    { SID_ATTR_CHAR_CTL_WEIGHT,     SfxItemPoolFlags::POOLABLE },    // ATTR_CTL_FONT_WEIGHT     from 614
    { SID_ATTR_CHAR_CTL_POSTURE,    SfxItemPoolFlags::POOLABLE },    // ATTR_CTL_FONT_POSTURE    from 614
    { SID_ATTR_CHAR_CTL_LANGUAGE,   SfxItemPoolFlags::POOLABLE },    // ATTR_CTL_FONT_LANGUAGE   from 614
    { SID_ATTR_CHAR_EMPHASISMARK,   SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_EMPHASISMARK   from 614
    { 0,                            SfxItemPoolFlags::POOLABLE },    // ATTR_USERDEF             from 614 / 641c
    { SID_ATTR_CHAR_WORDLINEMODE,   SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_WORDLINE       from 632b
    { SID_ATTR_CHAR_RELIEF,         SfxItemPoolFlags::POOLABLE },    // ATTR_FONT_RELIEF         from 632b
    { SID_ATTR_ALIGN_HYPHENATION,   SfxItemPoolFlags::POOLABLE },    // ATTR_HYPHENATE           from 632b
    { 0,                            SfxItemPoolFlags::POOLABLE },    // ATTR_SCRIPTSPACE         from 614d
    { 0,                            SfxItemPoolFlags::POOLABLE },    // ATTR_HANGPUNCTUATION     from 614d
    { SID_ATTR_PARA_FORBIDDEN_RULES,SfxItemPoolFlags::POOLABLE },    // ATTR_FORBIDDEN_RULES     from 614d
    { SID_ATTR_ALIGN_HOR_JUSTIFY,   SfxItemPoolFlags::POOLABLE },    // ATTR_HOR_JUSTIFY
    { SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD, SfxItemPoolFlags::POOLABLE }, // ATTR_HOR_JUSTIFY_METHOD
    { SID_ATTR_ALIGN_INDENT,        SfxItemPoolFlags::POOLABLE },    // ATTR_INDENT          from 350
    { SID_ATTR_ALIGN_VER_JUSTIFY,   SfxItemPoolFlags::POOLABLE },    // ATTR_VER_JUSTIFY
    { SID_ATTR_ALIGN_VER_JUSTIFY_METHOD, SfxItemPoolFlags::POOLABLE }, // ATTR_VER_JUSTIFY_METHOD
    { SID_ATTR_ALIGN_STACKED,       SfxItemPoolFlags::POOLABLE },    // ATTR_STACKED         from 680/dr14 (replaces ATTR_ORIENTATION)
    { SID_ATTR_ALIGN_DEGREES,       SfxItemPoolFlags::POOLABLE },    // ATTR_ROTATE_VALUE    from 367
    { SID_ATTR_ALIGN_LOCKPOS,       SfxItemPoolFlags::POOLABLE },    // ATTR_ROTATE_MODE     from 367
    { SID_ATTR_ALIGN_ASIANVERTICAL, SfxItemPoolFlags::POOLABLE },    // ATTR_VERTICAL_ASIAN  from 642
    { SID_ATTR_FRAMEDIRECTION,      SfxItemPoolFlags::POOLABLE },    // ATTR_WRITINGDIR      from 643
    { SID_ATTR_ALIGN_LINEBREAK,     SfxItemPoolFlags::POOLABLE },    // ATTR_LINEBREAK
    { SID_ATTR_ALIGN_SHRINKTOFIT,   SfxItemPoolFlags::POOLABLE },    // ATTR_SHRINKTOFIT     from 680/dr14
    { SID_ATTR_BORDER_DIAG_TLBR,    SfxItemPoolFlags::POOLABLE },    // ATTR_BORDER_TLBR     from 680/dr14
    { SID_ATTR_BORDER_DIAG_BLTR,    SfxItemPoolFlags::POOLABLE },    // ATTR_BORDER_BLTR     from 680/dr14
    { SID_ATTR_ALIGN_MARGIN,        SfxItemPoolFlags::POOLABLE },    // ATTR_MARGIN
    { 0,                            SfxItemPoolFlags::POOLABLE },    // ATTR_MERGE
    { 0,                            SfxItemPoolFlags::POOLABLE },    // ATTR_MERGE_FLAG
    { SID_ATTR_NUMBERFORMAT_VALUE,  SfxItemPoolFlags::POOLABLE },    // ATTR_VALUE_FORMAT
    { ATTR_LANGUAGE_FORMAT,         SfxItemPoolFlags::POOLABLE },    // ATTR_LANGUAGE_FORMAT from 329, is combined with SID_ATTR_NUMBERFORMAT_VALUE in the dialog
    { SID_ATTR_BRUSH,               SfxItemPoolFlags::POOLABLE },    // ATTR_BACKGROUND
    { SID_SCATTR_PROTECTION,        SfxItemPoolFlags::POOLABLE },    // ATTR_PROTECTION
    { SID_ATTR_BORDER_OUTER,        SfxItemPoolFlags::POOLABLE },    // ATTR_BORDER
    { SID_ATTR_BORDER_INNER,        SfxItemPoolFlags::POOLABLE },    // ATTR_BORDER_INNER
    { SID_ATTR_BORDER_SHADOW,       SfxItemPoolFlags::POOLABLE },    // ATTR_SHADOW
    { 0,                            SfxItemPoolFlags::POOLABLE },    // ATTR_VALIDDATA
    { 0,                            SfxItemPoolFlags::POOLABLE },    // ATTR_CONDITIONAL
    { 0,                            SfxItemPoolFlags::POOLABLE },    // ATTR_HYPERLINK
    { 0,                            SfxItemPoolFlags::POOLABLE },    // ATTR_PATTERN
    { SID_ATTR_LRSPACE,             SfxItemPoolFlags::POOLABLE },    // ATTR_LRSPACE
    { SID_ATTR_ULSPACE,             SfxItemPoolFlags::POOLABLE },    // ATTR_ULSPACE
    { SID_ATTR_PAGE,                SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE
    { 0,                            SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_PAPERTRAY, since 303 just a dummy
    { SID_ATTR_PAGE_PAPERBIN,       SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_PAPERBIN
    { SID_ATTR_PAGE_SIZE,           SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_SIZE
    { SID_ATTR_PAGE_MAXSIZE,        SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_MAXSIZE
    { SID_ATTR_PAGE_EXT1,           SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_HORCENTER
    { SID_ATTR_PAGE_EXT2,           SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_VERCENTER
    { SID_ATTR_PAGE_ON,             SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_ON
    { SID_ATTR_PAGE_DYNAMIC,        SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_DYNAMIC
    { SID_ATTR_PAGE_SHARED,         SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_SHARED
    { SID_SCATTR_PAGE_NOTES,        SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_NOTES
    { SID_SCATTR_PAGE_GRID,         SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_GRID
    { SID_SCATTR_PAGE_HEADERS,      SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_HEADERS
    { SID_SCATTR_PAGE_CHARTS,       SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_CHARTS
    { SID_SCATTR_PAGE_OBJECTS,      SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_OBJECTS
    { SID_SCATTR_PAGE_DRAWINGS,     SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_DRAWINGS
    { SID_SCATTR_PAGE_TOPDOWN,      SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_TOPDOWN
    { SID_SCATTR_PAGE_SCALE,        SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_SCALE
    { SID_SCATTR_PAGE_SCALETOPAGES, SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_SCALETOPAGES
    { SID_SCATTR_PAGE_FIRSTPAGENO,  SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_FIRSTPAGENO
    { SID_SCATTR_PAGE_PRINTAREA,    SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_PRINTAREA
    { SID_SCATTR_PAGE_REPEATROW,    SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_REPEATROW
    { SID_SCATTR_PAGE_REPEATCOL,    SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_REPEATCOL
    { SID_SCATTR_PAGE_PRINTTABLES,  SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_PRINTTABLES
    { SID_SCATTR_PAGE_HEADERLEFT,   SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_HEADERLEFT
    { SID_SCATTR_PAGE_FOOTERLEFT,   SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_FOOTERLEFT
    { SID_SCATTR_PAGE_HEADERRIGHT,  SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_HEADERRIGHT
    { SID_SCATTR_PAGE_FOOTERRIGHT,  SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_FOOTERRIGHT
    { SID_ATTR_PAGE_HEADERSET,      SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_HEADERSET
    { SID_ATTR_PAGE_FOOTERSET,      SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_FOOTERSET
    { SID_SCATTR_PAGE_FORMULAS,     SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_FORMULAS
    { SID_SCATTR_PAGE_NULLVALS,     SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_NULLVALS
    { SID_SCATTR_PAGE_SCALETO,      SfxItemPoolFlags::POOLABLE },    // ATTR_PAGE_SCALETO
    { 0,                            SfxItemPoolFlags::POOLABLE }     // ATTR_HIDDEN
};
static_assert(
    SAL_N_ELEMENTS(aItemInfos) == ATTR_ENDINDEX - ATTR_STARTINDEX + 1, "these must match");

ScDocumentPool::ScDocumentPool( SfxItemPool* pSecPool)

    :   SfxItemPool ( OUString("ScDocumentPool"),
                        ATTR_STARTINDEX, ATTR_ENDINDEX,
                        aItemInfos, nullptr, false/*bLoadRefCounts*/ ),
        pSecondary  ( pSecPool )
{
    //  latin font from GetDefaultFonts is not used, DEFAULTFONT_LATIN_SPREADSHEET instead
    vcl::Font aStdFont = OutputDevice::GetDefaultFont( DefaultFontType::LATIN_SPREADSHEET, LANGUAGE_ENGLISH_US,
                                                    GetDefaultFontFlags::OnlyOne );
    SvxFontItem* pStdFont = new SvxFontItem( aStdFont.GetFamily(),
                                            aStdFont.GetFamilyName(), aStdFont.GetStyleName(),
                                            aStdFont.GetPitch(), aStdFont.GetCharSet(),
                                            ATTR_FONT );

    SvxFontItem* pCjkFont = new SvxFontItem( ATTR_CJK_FONT );
    SvxFontItem* pCtlFont = new SvxFontItem( ATTR_CTL_FONT );
    SvxFontItem aDummy( ATTR_FONT );
    GetDefaultFonts( aDummy, *pCjkFont, *pCtlFont );

    SvxBoxInfoItem* pGlobalBorderInnerAttr = new SvxBoxInfoItem( ATTR_BORDER_INNER );
    SfxItemSet*     pSet = new SfxItemSet( *this, ATTR_PATTERN_START, ATTR_PATTERN_END );
    SfxItemSet      aSetItemItemSet( *this,
                                     ATTR_BACKGROUND, ATTR_BACKGROUND,
                                     ATTR_BORDER,     ATTR_SHADOW,
                                     ATTR_LRSPACE,    ATTR_ULSPACE,
                                     ATTR_PAGE_SIZE,  ATTR_PAGE_SIZE,
                                     ATTR_PAGE_ON,    ATTR_PAGE_SHARED,
                                     0 );

    pGlobalBorderInnerAttr->SetLine(nullptr, SvxBoxInfoItemLine::HORI);
    pGlobalBorderInnerAttr->SetLine(nullptr, SvxBoxInfoItemLine::VERT);
    pGlobalBorderInnerAttr->SetTable(true);
    pGlobalBorderInnerAttr->SetDist(true);
    pGlobalBorderInnerAttr->SetMinDist(false);

    ppPoolDefaults = new SfxPoolItem*[ATTR_ENDINDEX-ATTR_STARTINDEX+1];

    ppPoolDefaults[ ATTR_FONT            - ATTR_STARTINDEX ] = pStdFont;
    ppPoolDefaults[ ATTR_FONT_HEIGHT     - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200, 100, ATTR_FONT_HEIGHT );       // 10 pt;
    ppPoolDefaults[ ATTR_FONT_WEIGHT     - ATTR_STARTINDEX ] = new SvxWeightItem( WEIGHT_NORMAL, ATTR_FONT_WEIGHT );
    ppPoolDefaults[ ATTR_FONT_POSTURE    - ATTR_STARTINDEX ] = new SvxPostureItem( ITALIC_NONE, ATTR_FONT_POSTURE );
    ppPoolDefaults[ ATTR_FONT_UNDERLINE  - ATTR_STARTINDEX ] = new SvxUnderlineItem( UNDERLINE_NONE, ATTR_FONT_UNDERLINE );
    ppPoolDefaults[ ATTR_FONT_OVERLINE   - ATTR_STARTINDEX ] = new SvxOverlineItem( UNDERLINE_NONE, ATTR_FONT_OVERLINE );
    ppPoolDefaults[ ATTR_FONT_CROSSEDOUT - ATTR_STARTINDEX ] = new SvxCrossedOutItem( STRIKEOUT_NONE, ATTR_FONT_CROSSEDOUT );
    ppPoolDefaults[ ATTR_FONT_CONTOUR    - ATTR_STARTINDEX ] = new SvxContourItem( false, ATTR_FONT_CONTOUR );
    ppPoolDefaults[ ATTR_FONT_SHADOWED   - ATTR_STARTINDEX ] = new SvxShadowedItem( false, ATTR_FONT_SHADOWED );
    ppPoolDefaults[ ATTR_FONT_COLOR      - ATTR_STARTINDEX ] = new SvxColorItem( Color(COL_AUTO), ATTR_FONT_COLOR );
    ppPoolDefaults[ ATTR_FONT_LANGUAGE   - ATTR_STARTINDEX ] = new SvxLanguageItem( LanguageType(LANGUAGE_DONTKNOW), ATTR_FONT_LANGUAGE );
    ppPoolDefaults[ ATTR_CJK_FONT        - ATTR_STARTINDEX ] = pCjkFont;
    ppPoolDefaults[ ATTR_CJK_FONT_HEIGHT - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200, 100, ATTR_CJK_FONT_HEIGHT );
    ppPoolDefaults[ ATTR_CJK_FONT_WEIGHT - ATTR_STARTINDEX ] = new SvxWeightItem( WEIGHT_NORMAL, ATTR_CJK_FONT_WEIGHT );
    ppPoolDefaults[ ATTR_CJK_FONT_POSTURE- ATTR_STARTINDEX ] = new SvxPostureItem( ITALIC_NONE, ATTR_CJK_FONT_POSTURE );
    ppPoolDefaults[ ATTR_CJK_FONT_LANGUAGE-ATTR_STARTINDEX ] = new SvxLanguageItem( LanguageType(LANGUAGE_DONTKNOW),
                                                                    ATTR_CJK_FONT_LANGUAGE );
    ppPoolDefaults[ ATTR_CTL_FONT        - ATTR_STARTINDEX ] = pCtlFont;
    ppPoolDefaults[ ATTR_CTL_FONT_HEIGHT - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200, 100, ATTR_CTL_FONT_HEIGHT );
    ppPoolDefaults[ ATTR_CTL_FONT_WEIGHT - ATTR_STARTINDEX ] = new SvxWeightItem( WEIGHT_NORMAL, ATTR_CTL_FONT_WEIGHT );
    ppPoolDefaults[ ATTR_CTL_FONT_POSTURE- ATTR_STARTINDEX ] = new SvxPostureItem( ITALIC_NONE, ATTR_CTL_FONT_POSTURE );
    ppPoolDefaults[ ATTR_CTL_FONT_LANGUAGE-ATTR_STARTINDEX ] = new SvxLanguageItem( LanguageType(LANGUAGE_DONTKNOW),
                                                                    ATTR_CTL_FONT_LANGUAGE );
    ppPoolDefaults[ ATTR_FONT_EMPHASISMARK-ATTR_STARTINDEX ] = new SvxEmphasisMarkItem( EMPHASISMARK_NONE, ATTR_FONT_EMPHASISMARK );
    ppPoolDefaults[ ATTR_USERDEF         - ATTR_STARTINDEX ] = new SvXMLAttrContainerItem( ATTR_USERDEF );
    ppPoolDefaults[ ATTR_FONT_WORDLINE   - ATTR_STARTINDEX ] = new SvxWordLineModeItem(false, ATTR_FONT_WORDLINE );
    ppPoolDefaults[ ATTR_FONT_RELIEF     - ATTR_STARTINDEX ] = new SvxCharReliefItem( RELIEF_NONE, ATTR_FONT_RELIEF );
    ppPoolDefaults[ ATTR_HYPHENATE       - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_HYPHENATE );
    ppPoolDefaults[ ATTR_SCRIPTSPACE     - ATTR_STARTINDEX ] = new SvxScriptSpaceItem( false, ATTR_SCRIPTSPACE);
    ppPoolDefaults[ ATTR_HANGPUNCTUATION - ATTR_STARTINDEX ] = new SvxHangingPunctuationItem( false, ATTR_HANGPUNCTUATION);
    ppPoolDefaults[ ATTR_FORBIDDEN_RULES - ATTR_STARTINDEX ] = new SvxForbiddenRuleItem( false, ATTR_FORBIDDEN_RULES);
    ppPoolDefaults[ ATTR_HOR_JUSTIFY     - ATTR_STARTINDEX ] = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_STANDARD, ATTR_HOR_JUSTIFY);
    ppPoolDefaults[ ATTR_HOR_JUSTIFY_METHOD - ATTR_STARTINDEX ] = new SvxJustifyMethodItem( SVX_JUSTIFY_METHOD_AUTO, ATTR_HOR_JUSTIFY_METHOD);
    ppPoolDefaults[ ATTR_INDENT          - ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_INDENT, 0 );
    ppPoolDefaults[ ATTR_VER_JUSTIFY     - ATTR_STARTINDEX ] = new SvxVerJustifyItem( SVX_VER_JUSTIFY_STANDARD, ATTR_VER_JUSTIFY);
    ppPoolDefaults[ ATTR_VER_JUSTIFY_METHOD - ATTR_STARTINDEX ] = new SvxJustifyMethodItem( SVX_JUSTIFY_METHOD_AUTO, ATTR_VER_JUSTIFY_METHOD);
    ppPoolDefaults[ ATTR_STACKED         - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_STACKED, false );
    ppPoolDefaults[ ATTR_ROTATE_VALUE    - ATTR_STARTINDEX ] = new SfxInt32Item( ATTR_ROTATE_VALUE, 0 );
    ppPoolDefaults[ ATTR_ROTATE_MODE     - ATTR_STARTINDEX ] = new SvxRotateModeItem( SVX_ROTATE_MODE_BOTTOM, ATTR_ROTATE_MODE );
    ppPoolDefaults[ ATTR_VERTICAL_ASIAN  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_VERTICAL_ASIAN );
    //  The default for the ATTR_WRITINGDIR cell attribute must by FRMDIR_ENVIRONMENT,
    //  so that value is returned when asking for a default cell's attributes.
    //  The value from the page style is set as DefaultHorizontalTextDirection for the EditEngine.
    ppPoolDefaults[ ATTR_WRITINGDIR      - ATTR_STARTINDEX ] = new SvxFrameDirectionItem( FRMDIR_ENVIRONMENT, ATTR_WRITINGDIR );
    ppPoolDefaults[ ATTR_LINEBREAK       - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_LINEBREAK );
    ppPoolDefaults[ ATTR_SHRINKTOFIT     - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_SHRINKTOFIT );
    ppPoolDefaults[ ATTR_BORDER_TLBR     - ATTR_STARTINDEX ] = new SvxLineItem( ATTR_BORDER_TLBR );
    ppPoolDefaults[ ATTR_BORDER_BLTR     - ATTR_STARTINDEX ] = new SvxLineItem( ATTR_BORDER_BLTR );
    ppPoolDefaults[ ATTR_MARGIN          - ATTR_STARTINDEX ] = new SvxMarginItem( ATTR_MARGIN );
    ppPoolDefaults[ ATTR_MERGE           - ATTR_STARTINDEX ] = new ScMergeAttr;
    ppPoolDefaults[ ATTR_MERGE_FLAG      - ATTR_STARTINDEX ] = new ScMergeFlagAttr;
    ppPoolDefaults[ ATTR_VALUE_FORMAT    - ATTR_STARTINDEX ] = new SfxUInt32Item( ATTR_VALUE_FORMAT, 0 );
    ppPoolDefaults[ ATTR_LANGUAGE_FORMAT - ATTR_STARTINDEX ] = new SvxLanguageItem( ScGlobal::eLnge, ATTR_LANGUAGE_FORMAT );
    ppPoolDefaults[ ATTR_BACKGROUND      - ATTR_STARTINDEX ] = new SvxBrushItem( Color(COL_TRANSPARENT), ATTR_BACKGROUND );
    ppPoolDefaults[ ATTR_PROTECTION      - ATTR_STARTINDEX ] = new ScProtectionAttr;
    ppPoolDefaults[ ATTR_BORDER          - ATTR_STARTINDEX ] = new SvxBoxItem( ATTR_BORDER );
    ppPoolDefaults[ ATTR_BORDER_INNER    - ATTR_STARTINDEX ] = pGlobalBorderInnerAttr;
    ppPoolDefaults[ ATTR_SHADOW          - ATTR_STARTINDEX ] = new SvxShadowItem( ATTR_SHADOW );
    ppPoolDefaults[ ATTR_VALIDDATA       - ATTR_STARTINDEX ] = new SfxUInt32Item( ATTR_VALIDDATA, 0 );
    ppPoolDefaults[ ATTR_CONDITIONAL     - ATTR_STARTINDEX ] = new ScCondFormatItem;
    ppPoolDefaults[ ATTR_HYPERLINK       - ATTR_STARTINDEX ] = new SfxStringItem( ATTR_HYPERLINK, OUString() ) ;

    // GetRscString only works after ScGlobal::Init (indicated by the EmptyBrushItem)
    // TODO: Write additional method ScGlobal::IsInit() or somesuch
    //       or detect whether this is the Secondary Pool for a MessagePool
    if ( ScGlobal::GetEmptyBrushItem() )
        ppPoolDefaults[ ATTR_PATTERN     - ATTR_STARTINDEX ] = new ScPatternAttr( pSet, ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );
    else
        ppPoolDefaults[ ATTR_PATTERN     - ATTR_STARTINDEX ] = new ScPatternAttr( pSet,
            OUString(STRING_STANDARD) ); // FIXME: without name?

    ppPoolDefaults[ ATTR_LRSPACE         - ATTR_STARTINDEX ] = new SvxLRSpaceItem( ATTR_LRSPACE );
    ppPoolDefaults[ ATTR_ULSPACE         - ATTR_STARTINDEX ] = new SvxULSpaceItem( ATTR_ULSPACE );
    ppPoolDefaults[ ATTR_PAGE            - ATTR_STARTINDEX ] = new SvxPageItem( ATTR_PAGE );
    ppPoolDefaults[ ATTR_PAGE_PAPERTRAY  - ATTR_STARTINDEX ] = new SfxAllEnumItem( ATTR_PAGE_PAPERTRAY );
    ppPoolDefaults[ ATTR_PAGE_PAPERBIN   - ATTR_STARTINDEX ] = new SvxPaperBinItem( ATTR_PAGE_PAPERBIN );
    ppPoolDefaults[ ATTR_PAGE_SIZE       - ATTR_STARTINDEX ] = new SvxSizeItem( ATTR_PAGE_SIZE );
    ppPoolDefaults[ ATTR_PAGE_MAXSIZE    - ATTR_STARTINDEX ] = new SvxSizeItem( ATTR_PAGE_MAXSIZE );
    ppPoolDefaults[ ATTR_PAGE_HORCENTER  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_HORCENTER );
    ppPoolDefaults[ ATTR_PAGE_VERCENTER  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_VERCENTER );
    ppPoolDefaults[ ATTR_PAGE_ON         - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_ON, true );
    ppPoolDefaults[ ATTR_PAGE_DYNAMIC    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_DYNAMIC, true );
    ppPoolDefaults[ ATTR_PAGE_SHARED     - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_SHARED, true );
    ppPoolDefaults[ ATTR_PAGE_NOTES      - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_NOTES, false );
    ppPoolDefaults[ ATTR_PAGE_GRID       - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_GRID, false );
    ppPoolDefaults[ ATTR_PAGE_HEADERS    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_HEADERS, false );
    ppPoolDefaults[ ATTR_PAGE_CHARTS     - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_CHARTS );
    ppPoolDefaults[ ATTR_PAGE_OBJECTS    - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_OBJECTS );
    ppPoolDefaults[ ATTR_PAGE_DRAWINGS   - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_DRAWINGS );
    ppPoolDefaults[ ATTR_PAGE_TOPDOWN    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_TOPDOWN, true );
    ppPoolDefaults[ ATTR_PAGE_SCALE      - ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_SCALE, 100 );
    ppPoolDefaults[ ATTR_PAGE_SCALETOPAGES-ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, 1 );
    ppPoolDefaults[ ATTR_PAGE_FIRSTPAGENO- ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, 1 );
    ppPoolDefaults[ ATTR_PAGE_PRINTAREA  - ATTR_STARTINDEX ] = new ScRangeItem( ATTR_PAGE_PRINTAREA );
    ppPoolDefaults[ ATTR_PAGE_REPEATROW  - ATTR_STARTINDEX ] = new ScRangeItem( ATTR_PAGE_REPEATROW );
    ppPoolDefaults[ ATTR_PAGE_REPEATCOL  - ATTR_STARTINDEX ] = new ScRangeItem( ATTR_PAGE_REPEATCOL );
    ppPoolDefaults[ ATTR_PAGE_PRINTTABLES- ATTR_STARTINDEX ] = new ScTableListItem( ATTR_PAGE_PRINTTABLES );
    ppPoolDefaults[ ATTR_PAGE_HEADERLEFT - ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_HEADERLEFT );
    ppPoolDefaults[ ATTR_PAGE_FOOTERLEFT - ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_FOOTERLEFT );
    ppPoolDefaults[ ATTR_PAGE_HEADERRIGHT- ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_HEADERRIGHT );
    ppPoolDefaults[ ATTR_PAGE_FOOTERRIGHT- ATTR_STARTINDEX ] = new ScPageHFItem( ATTR_PAGE_FOOTERRIGHT );
    ppPoolDefaults[ ATTR_PAGE_HEADERSET  - ATTR_STARTINDEX ] = new SvxSetItem( ATTR_PAGE_HEADERSET, aSetItemItemSet );
    ppPoolDefaults[ ATTR_PAGE_FOOTERSET  - ATTR_STARTINDEX ] = new SvxSetItem( ATTR_PAGE_FOOTERSET, aSetItemItemSet );
    ppPoolDefaults[ ATTR_PAGE_FORMULAS   - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_FORMULAS, false );
    ppPoolDefaults[ ATTR_PAGE_NULLVALS   - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_NULLVALS, true );
    ppPoolDefaults[ ATTR_PAGE_SCALETO    - ATTR_STARTINDEX ] = new ScPageScaleToItem( 1, 1 );
    ppPoolDefaults[ ATTR_HIDDEN          - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_HIDDEN, false );

    SetDefaults( ppPoolDefaults );

    if ( pSecondary )
        SetSecondaryPool( pSecondary );

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
        SetRefCount( *ppPoolDefaults[i], 0 );
        delete ppPoolDefaults[i];
    }

    delete[] ppPoolDefaults;
    SfxItemPool::Free(pSecondary);
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

/**
 * The sal_uInt16 RefCount can overflow easily for the pattern attributes (SetItems):
 * E.g. Alternate formatting for 600 whole cells.
 * The RefCount is kept at SC_MAX_POOLREF and not increased/decreased anymore.
 * This RefCount is recalculated not until the next load.
 * The difference between SC_MAX_POOLREF and SC_SAFE_POOLREF is a little larger than it needs
 * to be, to allow for detecting accidental "normal" changes to the RefCount (assertions).
 */
const SfxPoolItem& ScDocumentPool::Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
{
    if ( rItem.Which() != ATTR_PATTERN ) // Only Pattern is special
        return SfxItemPool::Put( rItem, nWhich );

    // Don't copy the default pattern of this Pool
    if (&rItem == ppPoolDefaults[ ATTR_PATTERN - ATTR_STARTINDEX ])
        return rItem;

    // Else Put must always happen, because it could be another Pool
    const SfxPoolItem& rNew = SfxItemPool::Put( rItem, nWhich );
    CheckRef( rNew );
    return rNew;
}

void ScDocumentPool::Remove( const SfxPoolItem& rItem )
{
    if ( rItem.Which() == ATTR_PATTERN ) // Only Pattern is special
    {
        sal_uLong nRef = rItem.GetRefCount();
        if ( nRef >= (sal_uLong) SC_MAX_POOLREF && nRef <= (sal_uLong) SFX_ITEMS_OLD_MAXREF )
        {
            if ( nRef != (sal_uLong) SC_SAFE_POOLREF )
            {
                OSL_FAIL("Who fiddles with my ref counts?");
                SetRefCount( (SfxPoolItem&)rItem, (sal_uLong) SC_SAFE_POOLREF );
            }
            return; // Do not decrement
        }
    }
    SfxItemPool::Remove( rItem );
}

void ScDocumentPool::CheckRef( const SfxPoolItem& rItem )
{
    sal_uLong nRef = rItem.GetRefCount();
    if ( nRef >= (sal_uLong) SC_MAX_POOLREF && nRef <= (sal_uLong) SFX_ITEMS_OLD_MAXREF )
    {
        // At the Apply of the Cache we might increase by 2 (to MAX+1 or SAFE+2)
        // We only decrease by 1 (in LoadCompleted)
        OSL_ENSURE( nRef<=(sal_uLong)SC_MAX_POOLREF+1 || (nRef>=(sal_uLong)SC_SAFE_POOLREF-1 && nRef<=(sal_uLong)SC_SAFE_POOLREF+2),
                "ScDocumentPool::CheckRef" );
        SetRefCount( (SfxPoolItem&)rItem, (sal_uLong) SC_SAFE_POOLREF );
    }
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
    SfxMapUnit          eCoreMetric,
    SfxMapUnit          ePresentationMetric,
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

                aText = EE_RESSTR(RID_SVXITEMS_LRSPACE_LEFT);
                if ( 100 != nPropLeftMargin )
                {
                    aText = aText + unicode::formatPercent(nPropLeftMargin,
                        Application::GetSettings().GetUILanguageTag());
                }
                else
                {
                    aText += GetMetricText( (long)nLeftMargin,
                                           eCoreMetric, ePresentationMetric, pIntl );
                    aText += " " + EE_RESSTR(GetMetricId(ePresentationMetric));
                }
                aText += cpDelim;

                // We don't have a nPropFirstLineOfst
                aText += EE_RESSTR(RID_SVXITEMS_LRSPACE_RIGHT);
                if ( 100 != nPropRightMargin )
                {
                    aText = aText + unicode::formatPercent(nPropLeftMargin,
                        Application::GetSettings().GetUILanguageTag());
                }
                else
                {
                    aText += GetMetricText( (long)nRightMargin,
                                            eCoreMetric, ePresentationMetric, pIntl );
                    aText += " " + EE_RESSTR(GetMetricId(ePresentationMetric));
                }
            }
            break;

            default:
                if ( !pIntl )
                    pIntl = ScGlobal::GetScIntlWrapper();
                pItem->GetPresentation( SFX_ITEM_PRESENTATION_COMPLETE, eCoreMetric, ePresentationMetric, aText, pIntl );

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
    SfxMapUnit          ePresentationMetric,
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
            ePresentationRet = rItem.GetPresentation( SFX_ITEM_PRESENTATION_COMPLETE, GetMetric( nW ), ePresentationMetric, rText, pIntl );
        break;
    }

    return ePresentationRet;
}

SfxMapUnit ScDocumentPool::GetMetric( sal_uInt16 nWhich ) const
{
    // Own attributes in Twips, everything else in 1/100 mm
    if ( nWhich >= ATTR_STARTINDEX && nWhich <= ATTR_ENDINDEX )
        return SFX_MAPUNIT_TWIP;
    else
        return SFX_MAPUNIT_100TH_MM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
