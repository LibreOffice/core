/*************************************************************************
 *
 *  $RCSfile: docpool.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-25 12:13:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "scitems.hxx"
#include <tools/shl.hxx>
#include <vcl/system.hxx>
#include <svtools/aeitem.hxx>
#include <svtools/itemiter.hxx>
#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/cntritem.hxx>
#include <svx/colritem.hxx>
#include <svx/crsditem.hxx>
#include <svx/dialmgr.hxx>
#include <svx/emphitem.hxx>
#include <svx/fhgtitem.hxx>
#include <svx/fontitem.hxx>
#include <svx/itemtype.hxx>
#include <svx/langitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/pbinitem.hxx>
#include <svx/postitem.hxx>
#include <svx/rotmodit.hxx>
#include <svx/shaditem.hxx>
#include <svx/shdditem.hxx>
#include <svx/sizeitem.hxx>
#include <svx/svxitems.hrc>
#include <svx/twolinesitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/wghtitem.hxx>

#include "docpool.hxx"
#include "global.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "globstr.hrc"
#include "sc.hrc"           // Slot-IDs


#define SC_MAX_POOLREF      (SFX_ITEMS_MAXREF - 39)
#define SC_SAFE_POOLREF     (SC_MAX_POOLREF + 20)

// STATIC DATA -----------------------------------------------------------

USHORT* ScDocumentPool::pVersionMap1 = 0;
USHORT* ScDocumentPool::pVersionMap2 = 0;
USHORT* ScDocumentPool::pVersionMap3 = 0;
USHORT* ScDocumentPool::pVersionMap4 = 0;
USHORT* ScDocumentPool::pVersionMap5 = 0;

static SfxItemInfo __READONLY_DATA  aItemInfos[] =
{
    { SID_ATTR_CHAR_FONT,           SFX_ITEM_POOLABLE },    // ATTR_FONT
    { SID_ATTR_CHAR_FONTHEIGHT,     SFX_ITEM_POOLABLE },    // ATTR_FONT_HEIGHT
    { SID_ATTR_CHAR_WEIGHT,         SFX_ITEM_POOLABLE },    // ATTR_FONT_WEIGHT
    { SID_ATTR_CHAR_POSTURE,        SFX_ITEM_POOLABLE },    // ATTR_FONT_POSTURE
    { SID_ATTR_CHAR_UNDERLINE,      SFX_ITEM_POOLABLE },    // ATTR_FONT_UNDERLINE
    { SID_ATTR_CHAR_STRIKEOUT,      SFX_ITEM_POOLABLE },    // ATTR_FONT_CROSSEDOUT
    { SID_ATTR_CHAR_CONTOUR,        SFX_ITEM_POOLABLE },    // ATTR_FONT_CONTOUR
    { SID_ATTR_CHAR_SHADOWED,       SFX_ITEM_POOLABLE },    // ATTR_FONT_SHADOWED
    { SID_ATTR_CHAR_COLOR,          SFX_ITEM_POOLABLE },    // ATTR_FONT_COLOR
    { SID_ATTR_CHAR_LANGUAGE,       SFX_ITEM_POOLABLE },    // ATTR_FONT_LANGUAGE
    { SID_ATTR_CHAR_CJK_FONT,       SFX_ITEM_POOLABLE },    // ATTR_CJK_FONT            from 614
    { SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEM_POOLABLE },    // ATTR_CJK_FONT_HEIGHT     from 614
    { SID_ATTR_CHAR_CJK_WEIGHT,     SFX_ITEM_POOLABLE },    // ATTR_CJK_FONT_WEIGHT     from 614
    { SID_ATTR_CHAR_CJK_POSTURE,    SFX_ITEM_POOLABLE },    // ATTR_CJK_FONT_POSTURE    from 614
    { SID_ATTR_CHAR_CJK_LANGUAGE,   SFX_ITEM_POOLABLE },    // ATTR_CJK_FONT_LANGUAGE   from 614
    { SID_ATTR_CHAR_CTL_FONT,       SFX_ITEM_POOLABLE },    // ATTR_CTL_FONT            from 614
    { SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEM_POOLABLE },    // ATTR_CTL_FONT_HEIGHT     from 614
    { SID_ATTR_CHAR_CTL_WEIGHT,     SFX_ITEM_POOLABLE },    // ATTR_CTL_FONT_WEIGHT     from 614
    { SID_ATTR_CHAR_CTL_POSTURE,    SFX_ITEM_POOLABLE },    // ATTR_CTL_FONT_POSTURE    from 614
    { SID_ATTR_CHAR_CTL_LANGUAGE,   SFX_ITEM_POOLABLE },    // ATTR_CTL_FONT_LANGUAGE   from 614
    { 0,                            SFX_ITEM_POOLABLE },    // ATTR_FONT_EMPHASISMARK   from 614
    { 0,                            SFX_ITEM_POOLABLE },    // ATTR_FONT_TWOLINES       from 614
    { SID_ATTR_ALIGN_HOR_JUSTIFY,   SFX_ITEM_POOLABLE },    // ATTR_HOR_JUSTIFY
    { SID_ATTR_ALIGN_INDENT,        SFX_ITEM_POOLABLE },    // ATTR_INDENT          ab 350
    { SID_ATTR_ALIGN_VER_JUSTIFY,   SFX_ITEM_POOLABLE },    // ATTR_VER_JUSTIFY
    { SID_ATTR_ALIGN_ORIENTATION,   SFX_ITEM_POOLABLE },    // ATTR_ORIENTATION
    { SID_ATTR_ALIGN_DEGREES,       SFX_ITEM_POOLABLE },    // ATTR_ROTATE_VALUE    ab 367
    { SID_ATTR_ALIGN_LOCKPOS,       SFX_ITEM_POOLABLE },    // ATTR_ROTATE_MODE     ab 367
    { SID_ATTR_ALIGN_LINEBREAK,     SFX_ITEM_POOLABLE },    // ATTR_LINEBREAK
    { SID_ATTR_ALIGN_MARGIN,        SFX_ITEM_POOLABLE },    // ATTR_MARGIN
    { 0,                            SFX_ITEM_POOLABLE },    // ATTR_MERGE
    { 0,                            SFX_ITEM_POOLABLE },    // ATTR_MERGE_FLAG
    { SID_ATTR_NUMBERFORMAT_VALUE,  SFX_ITEM_POOLABLE },    // ATTR_VALUE_FORMAT
    { ATTR_LANGUAGE_FORMAT,         SFX_ITEM_POOLABLE },    // ATTR_LANGUAGE_FORMAT ab 329, wird im Dialog mit SID_ATTR_NUMBERFORMAT_VALUE kombiniert
    { SID_ATTR_BRUSH,               SFX_ITEM_POOLABLE },    // ATTR_BACKGROUND
    { SID_SCATTR_PROTECTION,        SFX_ITEM_POOLABLE },    // ATTR_PROTECTION
    { SID_ATTR_BORDER_OUTER,        SFX_ITEM_POOLABLE },    // ATTR_BORDER
    { SID_ATTR_BORDER_INNER,        SFX_ITEM_POOLABLE },    // ATTR_BORDER_INNER
    { SID_ATTR_BORDER_SHADOW,       SFX_ITEM_POOLABLE },    // ATTR_SHADOW
    { 0,                            SFX_ITEM_POOLABLE },    // ATTR_VALIDDATA
    { 0,                            SFX_ITEM_POOLABLE },    // ATTR_CONDITIONAL
    { 0,                            SFX_ITEM_POOLABLE },    // ATTR_PATTERN
    { SID_ATTR_LRSPACE,             SFX_ITEM_POOLABLE },    // ATTR_LRSPACE
    { SID_ATTR_ULSPACE,             SFX_ITEM_POOLABLE },    // ATTR_ULSPACE
    { SID_ATTR_PAGE,                SFX_ITEM_POOLABLE },    // ATTR_PAGE
    { 0,                            SFX_ITEM_POOLABLE },    // ATTR_PAGE_PAPERTRAY, seit 303 nur noch dummy
    { SID_ATTR_PAGE_PAPERBIN,       SFX_ITEM_POOLABLE },    // ATTR_PAGE_PAPERBIN
    { SID_ATTR_PAGE_SIZE,           SFX_ITEM_POOLABLE },    // ATTR_PAGE_SIZE
    { SID_ATTR_PAGE_MAXSIZE,        SFX_ITEM_POOLABLE },    // ATTR_PAGE_MAXSIZE
    { SID_ATTR_PAGE_EXT1,           SFX_ITEM_POOLABLE },    // ATTR_PAGE_HORCENTER
    { SID_ATTR_PAGE_EXT2,           SFX_ITEM_POOLABLE },    // ATTR_PAGE_VERCENTER
    { SID_ATTR_PAGE_ON,             SFX_ITEM_POOLABLE },    // ATTR_PAGE_ON
    { SID_ATTR_PAGE_DYNAMIC,        SFX_ITEM_POOLABLE },    // ATTR_PAGE_DYNAMIC
    { SID_ATTR_PAGE_SHARED,         SFX_ITEM_POOLABLE },    // ATTR_PAGE_SHARED
    { SID_SCATTR_PAGE_NOTES,        SFX_ITEM_POOLABLE },    // ATTR_PAGE_NOTES
    { SID_SCATTR_PAGE_GRID,         SFX_ITEM_POOLABLE },    // ATTR_PAGE_GRID
    { SID_SCATTR_PAGE_HEADERS,      SFX_ITEM_POOLABLE },    // ATTR_PAGE_HEADERS
    { SID_SCATTR_PAGE_CHARTS,       SFX_ITEM_POOLABLE },    // ATTR_PAGE_CHARTS
    { SID_SCATTR_PAGE_OBJECTS,      SFX_ITEM_POOLABLE },    // ATTR_PAGE_OBJECTS
    { SID_SCATTR_PAGE_DRAWINGS,     SFX_ITEM_POOLABLE },    // ATTR_PAGE_DRAWINGS
    { SID_SCATTR_PAGE_TOPDOWN,      SFX_ITEM_POOLABLE },    // ATTR_PAGE_TOPDOWN
    { SID_SCATTR_PAGE_SCALE,        SFX_ITEM_POOLABLE },    // ATTR_PAGE_SCALE
    { SID_SCATTR_PAGE_SCALETOPAGES, SFX_ITEM_POOLABLE },    // ATTR_PAGE_SCALETOPAGES
    { SID_SCATTR_PAGE_FIRSTPAGENO,  SFX_ITEM_POOLABLE },    // ATTR_PAGE_FIRSTPAGENO
    { SID_SCATTR_PAGE_PRINTAREA,    SFX_ITEM_POOLABLE },    // ATTR_PAGE_PRINTAREA
    { SID_SCATTR_PAGE_REPEATROW,    SFX_ITEM_POOLABLE },    // ATTR_PAGE_REPEATROW
    { SID_SCATTR_PAGE_REPEATCOL,    SFX_ITEM_POOLABLE },    // ATTR_PAGE_REPEATCOL
    { SID_SCATTR_PAGE_PRINTTABLES,  SFX_ITEM_POOLABLE },    // ATTR_PAGE_PRINTTABLES
    { SID_SCATTR_PAGE_HEADERLEFT,   SFX_ITEM_POOLABLE },    // ATTR_PAGE_HEADERLEFT
    { SID_SCATTR_PAGE_FOOTERLEFT,   SFX_ITEM_POOLABLE },    // ATTR_PAGE_FOOTERLEFT
    { SID_SCATTR_PAGE_HEADERRIGHT,  SFX_ITEM_POOLABLE },    // ATTR_PAGE_HEADERRIGHT
    { SID_SCATTR_PAGE_FOOTERRIGHT,  SFX_ITEM_POOLABLE },    // ATTR_PAGE_FOOTERRIGHT
    { SID_ATTR_PAGE_HEADERSET,      SFX_ITEM_POOLABLE },    // ATTR_PAGE_HEADERSET
    { SID_ATTR_PAGE_FOOTERSET,      SFX_ITEM_POOLABLE },    // ATTR_PAGE_FOOTERSET
    { SID_SCATTR_PAGE_FORMULAS,     SFX_ITEM_POOLABLE },    // ATTR_PAGE_FORMULAS
    { SID_SCATTR_PAGE_NULLVALS,     SFX_ITEM_POOLABLE }     // ATTR_PAGE_NULLVALS
};

// -----------------------------------------------------------------------

ScDocumentPool::ScDocumentPool( SfxItemPool* pSecPool, BOOL bLoadRefCounts )

    :   SfxItemPool ( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScDocumentPool")),
                        ATTR_STARTINDEX, ATTR_ENDINDEX,
                        aItemInfos, NULL, bLoadRefCounts ),
        pSecondary  ( pSecPool )
{
    //  latin font from GetDefaultFonts is not used, STDFONT_SWISS instead
    Font aStdFont = System::GetStandardFont( STDFONT_SWISS );
    SvxFontItem* pStdFont = new SvxFontItem( aStdFont.GetFamily(),
                                            aStdFont.GetName(), aStdFont.GetStyleName(),
                                            aStdFont.GetPitch(), aStdFont.GetCharSet(),
                                            ATTR_FONT );

    SvxFontItem* pCjkFont = new SvxFontItem( ATTR_CJK_FONT );
    SvxFontItem* pCtlFont = new SvxFontItem( ATTR_CTL_FONT );
    SvxFontItem aDummy;
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

    pGlobalBorderInnerAttr->SetLine(NULL, BOXINFO_LINE_HORI);
    pGlobalBorderInnerAttr->SetLine(NULL, BOXINFO_LINE_VERT);
    pGlobalBorderInnerAttr->SetTable(TRUE);
    pGlobalBorderInnerAttr->SetDist((BOOL)FALSE);
    pGlobalBorderInnerAttr->SetMinDist(FALSE);

    ppPoolDefaults = new SfxPoolItem*[ATTR_ENDINDEX-ATTR_STARTINDEX+1];

    ppPoolDefaults[ ATTR_FONT            - ATTR_STARTINDEX ] = pStdFont;
    ppPoolDefaults[ ATTR_FONT_HEIGHT     - ATTR_STARTINDEX ] = new SvxFontHeightItem( 200 );        // 10 pt;
    ppPoolDefaults[ ATTR_FONT_WEIGHT     - ATTR_STARTINDEX ] = new SvxWeightItem;
    ppPoolDefaults[ ATTR_FONT_POSTURE    - ATTR_STARTINDEX ] = new SvxPostureItem;
    ppPoolDefaults[ ATTR_FONT_UNDERLINE  - ATTR_STARTINDEX ] = new SvxUnderlineItem;
    ppPoolDefaults[ ATTR_FONT_CROSSEDOUT - ATTR_STARTINDEX ] = new SvxCrossedOutItem;
    ppPoolDefaults[ ATTR_FONT_CONTOUR    - ATTR_STARTINDEX ] = new SvxContourItem;
    ppPoolDefaults[ ATTR_FONT_SHADOWED   - ATTR_STARTINDEX ] = new SvxShadowedItem;
    ppPoolDefaults[ ATTR_FONT_COLOR      - ATTR_STARTINDEX ] = new SvxColorItem;
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
    ppPoolDefaults[ ATTR_FONT_EMPHASISMARK-ATTR_STARTINDEX ] = new SvxEmphasisMarkItem;
    ppPoolDefaults[ ATTR_FONT_TWOLINES   - ATTR_STARTINDEX ] = new SvxTwoLinesItem;
    ppPoolDefaults[ ATTR_HOR_JUSTIFY     - ATTR_STARTINDEX ] = new SvxHorJustifyItem;
    ppPoolDefaults[ ATTR_INDENT          - ATTR_STARTINDEX ] = new SfxUInt16Item( ATTR_INDENT, 0 );
    ppPoolDefaults[ ATTR_VER_JUSTIFY     - ATTR_STARTINDEX ] = new SvxVerJustifyItem;
    ppPoolDefaults[ ATTR_ORIENTATION     - ATTR_STARTINDEX ] = new SvxOrientationItem;
    ppPoolDefaults[ ATTR_ROTATE_VALUE    - ATTR_STARTINDEX ] = new SfxInt32Item( ATTR_ROTATE_VALUE, 0 );
    ppPoolDefaults[ ATTR_ROTATE_MODE     - ATTR_STARTINDEX ] = new SvxRotateModeItem( SVX_ROTATE_MODE_BOTTOM, ATTR_ROTATE_MODE );
    ppPoolDefaults[ ATTR_LINEBREAK       - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_LINEBREAK );
    ppPoolDefaults[ ATTR_MARGIN          - ATTR_STARTINDEX ] = new SvxMarginItem;
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
    ppPoolDefaults[ ATTR_CONDITIONAL     - ATTR_STARTINDEX ] = new SfxUInt32Item( ATTR_CONDITIONAL, 0 );

    //  GetRscString funktioniert erst nach ScGlobal::Init, zu erkennen am EmptyBrushItem
    //! zusaetzliche Methode ScGlobal::IsInit() oder so...
    //! oder erkennen, ob dies der Secondary-Pool fuer einen MessagePool ist
    if ( ScGlobal::GetEmptyBrushItem() )
        ppPoolDefaults[ ATTR_PATTERN     - ATTR_STARTINDEX ] = new ScPatternAttr( pSet, ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );
    else
        ppPoolDefaults[ ATTR_PATTERN     - ATTR_STARTINDEX ] = new ScPatternAttr( pSet,
            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_STANDARD)) ); //! without name?

    ppPoolDefaults[ ATTR_LRSPACE         - ATTR_STARTINDEX ] = new SvxLRSpaceItem( ATTR_LRSPACE );
    ppPoolDefaults[ ATTR_ULSPACE         - ATTR_STARTINDEX ] = new SvxULSpaceItem( ATTR_ULSPACE );
    ppPoolDefaults[ ATTR_PAGE            - ATTR_STARTINDEX ] = new SvxPageItem( ATTR_PAGE );
    ppPoolDefaults[ ATTR_PAGE_PAPERTRAY  - ATTR_STARTINDEX ] = new SfxAllEnumItem( ATTR_PAGE_PAPERTRAY );
    ppPoolDefaults[ ATTR_PAGE_PAPERBIN   - ATTR_STARTINDEX ] = new SvxPaperBinItem( ATTR_PAGE_PAPERBIN );
    ppPoolDefaults[ ATTR_PAGE_SIZE       - ATTR_STARTINDEX ] = new SvxSizeItem( ATTR_PAGE_SIZE );
    ppPoolDefaults[ ATTR_PAGE_MAXSIZE    - ATTR_STARTINDEX ] = new SvxSizeItem( ATTR_PAGE_MAXSIZE );
    ppPoolDefaults[ ATTR_PAGE_HORCENTER  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_HORCENTER );
    ppPoolDefaults[ ATTR_PAGE_VERCENTER  - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_VERCENTER );
    ppPoolDefaults[ ATTR_PAGE_ON         - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_ON, TRUE );
    ppPoolDefaults[ ATTR_PAGE_DYNAMIC    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_DYNAMIC, TRUE );
    ppPoolDefaults[ ATTR_PAGE_SHARED     - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_SHARED, TRUE );
    ppPoolDefaults[ ATTR_PAGE_NOTES      - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_NOTES, FALSE );
    ppPoolDefaults[ ATTR_PAGE_GRID       - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_GRID, FALSE );
    ppPoolDefaults[ ATTR_PAGE_HEADERS    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_HEADERS, FALSE );
    ppPoolDefaults[ ATTR_PAGE_CHARTS     - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_CHARTS );
    ppPoolDefaults[ ATTR_PAGE_OBJECTS    - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_OBJECTS );
    ppPoolDefaults[ ATTR_PAGE_DRAWINGS   - ATTR_STARTINDEX ] = new ScViewObjectModeItem( ATTR_PAGE_DRAWINGS );
    ppPoolDefaults[ ATTR_PAGE_TOPDOWN    - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_TOPDOWN, TRUE );
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
    ppPoolDefaults[ ATTR_PAGE_FORMULAS   - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_FORMULAS, FALSE );
    ppPoolDefaults[ ATTR_PAGE_NULLVALS   - ATTR_STARTINDEX ] = new SfxBoolItem( ATTR_PAGE_NULLVALS, TRUE );
//  ppPoolDefaults[ ATTR_ITEM_DOUBLE     - ATTR_STARTINDEX ] = new ScDoubleItem( ATTR_ITEM_DOUBLE, 0 );

    SetDefaults( ppPoolDefaults );

    if ( pSecondary )
        SetSecondaryPool( pSecondary );

    // ATTR_LANGUAGE_FORMAT ab sv329 eingefuegt, VersionMap in _ScGlobal__Init
    SetVersionMap( 1, 100, 157, pVersionMap1 );

    // ATTR_VALIDDATA, ATTR_CONDITIONAL ab 341
    SetVersionMap( 2, 100, 158, pVersionMap2 );

    // ATTR_INDENT ab 350
    SetVersionMap( 3, 100, 160, pVersionMap3 );

    // ATTR_ROTATE_VALUE, ATTR_ROTATE_MODE ab 367
    SetVersionMap( 4, 100, 161, pVersionMap4 );

    // CJK, CTL, EMPHASISMARK, TWOLINES from 614
    SetVersionMap( 5, 100, 163, pVersionMap5 );
}

__EXPORT ScDocumentPool::~ScDocumentPool()
{
    Delete();

    for ( USHORT i=0; i < ATTR_ENDINDEX-ATTR_STARTINDEX+1; i++ )
    {
        SetRef( *ppPoolDefaults[i], 0 );
        delete ppPoolDefaults[i];
    }

    delete[] ppPoolDefaults;
    delete pSecondary;
}

void ScDocumentPool::InitVersionMaps()
{
    DBG_ASSERT( !pVersionMap1 && !pVersionMap2 &&
                !pVersionMap3 && !pVersionMap4 &&
                !pVersionMap5, "InitVersionMaps call multiple times" );

    // alte WhichId's mappen
    // nicht mit ATTR_* zaehlen, falls die sich nochmal aendern

    //  erste Map: ATTR_LANGUAGE_FORMAT ab sv329 eingefuegt

    const USHORT nMap1Start = 100;  // alter ATTR_STARTINDEX
    const USHORT nMap1End   = 157;  // alter ATTR_ENDINDEX
    const USHORT nMap1Count = nMap1End - nMap1Start + 1;
    const USHORT nMap1New   = 18;   // ATTR_LANGUAGE_FORMAT - ATTR_STARTINDEX
    pVersionMap1 = new USHORT [ nMap1Count ];
    USHORT i, j;
    for ( i=0, j=nMap1Start; i < nMap1New; i++, j++ )
        pVersionMap1[i] = j;
    // ein Eintrag eingefuegt...
    for ( i=nMap1New, j=nMap1Start+nMap1New+1; i < nMap1Count; i++, j++ )
        pVersionMap1[i] = j;

    //  zweite Map: ATTR_VALIDDATA und ATTR_CONDITIONAL ab 341 eingefuegt

    const USHORT nMap2Start = 100;  // ATTR_STARTINDEX
    const USHORT nMap2End   = 158;  // ATTR_ENDINDEX
    const USHORT nMap2Count = nMap2End - nMap2Start + 1;
    const USHORT nMap2New   = 24;   // ATTR_VALIDDATA - ATTR_STARTINDEX
    pVersionMap2 = new USHORT [ nMap2Count ];
    for ( i=0, j=nMap2Start; i < nMap2New; i++, j++ )
        pVersionMap2[i] = j;
    // zwei Eintraege eingefuegt...
    for ( i=nMap2New, j=nMap2Start+nMap2New+2; i < nMap2Count; i++, j++ )
        pVersionMap2[i] = j;

    //  dritte Map: ATTR_INDENT ab 350 eingefuegt

    const USHORT nMap3Start = 100;  // ATTR_STARTINDEX
    const USHORT nMap3End   = 160;  // ATTR_ENDINDEX
    const USHORT nMap3Count = nMap3End - nMap3Start + 1;
    const USHORT nMap3New   = 11;   // ATTR_INDENT - ATTR_STARTINDEX
    pVersionMap3 = new USHORT [ nMap3Count ];
    for ( i=0, j=nMap3Start; i < nMap3New; i++, j++ )
        pVersionMap3[i] = j;
    // ein Eintrag eingefuegt...
    for ( i=nMap3New, j=nMap3Start+nMap3New+1; i < nMap3Count; i++, j++ )
        pVersionMap3[i] = j;

    //  vierte Map: ATTR_ROTATE_VALUE und ATTR_ROTATE_MODE ab 367 eingefuegt

    const USHORT nMap4Start = 100;  // ATTR_STARTINDEX
    const USHORT nMap4End   = 161;  // ATTR_ENDINDEX
    const USHORT nMap4Count = nMap4End - nMap4Start + 1;
    const USHORT nMap4New   = 14;   // ATTR_ROTATE_VALUE - ATTR_STARTINDEX
    pVersionMap4 = new USHORT [ nMap4Count ];
    for ( i=0, j=nMap4Start; i < nMap4New; i++, j++ )
        pVersionMap4[i] = j;
    // zwei Eintraege eingefuegt...
    for ( i=nMap4New, j=nMap4Start+nMap4New+2; i < nMap4Count; i++, j++ )
        pVersionMap4[i] = j;

    //  fifth map: CJK..., CTL..., EMPHASISMARK, TWOLINES (12 items) added in 614

    const USHORT nMap5Start = 100;  // ATTR_STARTINDEX
    const USHORT nMap5End   = 163;  // ATTR_ENDINDEX
    const USHORT nMap5Count = nMap5End - nMap5Start + 1;
    const USHORT nMap5New   = 10;   // ATTR_CJK_FONT - ATTR_STARTINDEX
    pVersionMap5 = new USHORT [ nMap5Count ];
    for ( i=0, j=nMap5Start; i < nMap5New; i++, j++ )
        pVersionMap5[i] = j;
    // 12 entries inserted
    for ( i=nMap5New, j=nMap5Start+nMap5New+12; i < nMap5Count; i++, j++ )
        pVersionMap5[i] = j;
}

void ScDocumentPool::DeleteVersionMaps()
{
    DBG_ASSERT( pVersionMap1 && pVersionMap2 &&
                pVersionMap3 && pVersionMap4 &&
                pVersionMap5, "DeleteVersionMaps without maps" );

    delete[] pVersionMap5;
    pVersionMap5 = 0;
    delete[] pVersionMap4;
    pVersionMap4 = 0;
    delete[] pVersionMap3;
    pVersionMap3 = 0;
    delete[] pVersionMap2;
    pVersionMap2 = 0;
    delete[] pVersionMap1;
    pVersionMap1 = 0;
}

// ----------------------------------------------------------------------------------------
//
//  Fuer die Pattern-Attribute (SetItems) kann der USHORT RefCount leicht ueberlaufen
//  (z.B. 600 ganze Zeilen abwechselnd formatieren).
//  Darum wird der RefCount bei SC_MAX_POOLREF festgehalten und nicht mehr hoch- oder
//  heruntergezaehlt. Dieser RefCount wird dann erst beim naechsten Laden neu gezaehlt.
//  Die Differenz zwischen SC_MAX_POOLREF und SC_SAFE_POOLREF ist ein wenig groesser
//  als noetig, um zu erkennen, wenn der RefCount aus Versehen doch "normal" veraendert
//  wird (Assertions).
//

const SfxPoolItem& __EXPORT ScDocumentPool::Put( const SfxPoolItem& rItem, USHORT nWhich )
{
    if ( rItem.Which() != ATTR_PATTERN )                // nur Pattern ist special
        return SfxItemPool::Put( rItem, nWhich );

    //  das Default-Pattern dieses Pools nicht kopieren
    if (&rItem == ppPoolDefaults[ ATTR_PATTERN - ATTR_STARTINDEX ])
        return rItem;

    //  ansonsten muss Put immer passieren, weil es ein anderer Pool sein kann
    const SfxPoolItem& rNew = SfxItemPool::Put( rItem, nWhich );
    CheckRef( rNew );
    return rNew;
}

void __EXPORT ScDocumentPool::Remove( const SfxPoolItem& rItem )
{
    if ( rItem.Which() == ATTR_PATTERN )                // nur Pattern ist special
    {
        USHORT nRef = rItem.GetRef();
        if ( nRef >= SC_MAX_POOLREF && nRef <= SFX_ITEMS_MAXREF )
        {
            if ( nRef != SC_SAFE_POOLREF )
            {
                DBG_ERROR("Wer fummelt da an meinen Ref-Counts herum");
                SetRef( (SfxPoolItem&)rItem, SC_SAFE_POOLREF );
            }
            return;                 // nicht herunterzaehlen
        }
    }
    SfxItemPool::Remove( rItem );
}

void ScDocumentPool::CheckRef( const SfxPoolItem& rItem )   // static
{
    USHORT nRef = rItem.GetRef();
    if ( nRef >= SC_MAX_POOLREF && nRef <= SFX_ITEMS_MAXREF )
    {
        // beim Apply vom Cache wird evtl. um 2 hochgezaehlt (auf MAX+1 oder SAFE+2),
        // heruntergezaehlt wird nur einzeln (in LoadCompleted)
        DBG_ASSERT( nRef<=SC_MAX_POOLREF+1 || (nRef>=SC_SAFE_POOLREF-1 && nRef<=SC_SAFE_POOLREF+2),
                    "ScDocumentPool::CheckRef" );

        SetRef( (SfxPoolItem&)rItem, SC_SAFE_POOLREF );
    }
}

void ScDocumentPool::MyLoadCompleted()
{
    LoadCompleted();

    USHORT nCount = GetItemCount(ATTR_PATTERN);
    for (USHORT i=0; i<nCount; i++)
    {
        const SfxPoolItem* pItem = GetItem(ATTR_PATTERN, i);
        if (pItem)
            CheckRef(*pItem);
    }
}

// ----------------------------------------------------------------------------------------

void ScDocumentPool::StyleDeleted( ScStyleSheet* pStyle )
{
    USHORT nCount = GetItemCount(ATTR_PATTERN);
    for (USHORT i=0; i<nCount; i++)
    {
        ScPatternAttr* pPattern = (ScPatternAttr*)GetItem(ATTR_PATTERN, i);
        if ( pPattern && pPattern->GetStyleSheet() == pStyle )
            pPattern->StyleToName();
    }
}

SfxItemPool* __EXPORT ScDocumentPool::Clone() const
{
    return new SfxItemPool (*this, TRUE);
}

SfxItemPresentation lcl_HFPresentation
(
    const SfxPoolItem&  rItem,
    SfxItemPresentation ePresentation,
    SfxMapUnit          eCoreMetric,
    SfxMapUnit          ePresentationMetric,
    String&             rText,
    const International* pIntl
)
{
    const SfxItemSet& rSet = ((const SfxSetItem&)rItem).GetItemSet();
    const SfxPoolItem* pItem;

    if ( SFX_ITEM_SET == rSet.GetItemState(ATTR_PAGE_ON,FALSE,&pItem) )
    {
        if( FALSE == ((const SfxBoolItem*)pItem)->GetValue() )
            return SFX_ITEM_PRESENTATION_NONE;
    }

    SfxItemIter aIter( rSet );
    pItem = aIter.FirstItem();
    String  aText;
    String  aDel = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( " + " ));

    while( pItem )
    {
        USHORT nWhich = pItem->Which();

        aText.Erase();

        switch( nWhich )
        {
            case ATTR_PAGE_ON:
            case ATTR_PAGE_DYNAMIC:
            case ATTR_PAGE_SHARED:
            break;

            case ATTR_LRSPACE:
            {
                SvxLRSpaceItem& rLRItem = (SvxLRSpaceItem&)*pItem;
                USHORT nPropLeftMargin  = rLRItem.GetPropLeft();
                USHORT nPropRightMargin = rLRItem.GetPropRight();
                USHORT nLeftMargin, nRightMargin;
                long nTmp;
                nTmp = rLRItem.GetLeft();
                nLeftMargin = nTmp < 0 ? 0 : USHORT(nTmp);
                nTmp = rLRItem.GetRight();
                nRightMargin = nTmp < 0 ? 0 : USHORT(nTmp);

                aText = SVX_RESSTR(RID_SVXITEMS_LRSPACE_LEFT);
                if ( 100 != nPropLeftMargin )
                {
                    aText += String::CreateFromInt32( nPropLeftMargin );
                    aText += '%';
                }
                else
                {
                    aText += GetMetricText( (long)nLeftMargin,
                                           eCoreMetric, ePresentationMetric );
                    aText += SVX_RESSTR(GetMetricId(ePresentationMetric));
                }
                aText += cpDelim;

                // nPropFirstLineOfst haben wir nicht

                aText += SVX_RESSTR(RID_SVXITEMS_LRSPACE_RIGHT);
                if ( 100 != nPropRightMargin )
                {
                    aText += String::CreateFromInt32( nPropRightMargin );
                    aText += '%';
                }
                else
                {
                    aText += GetMetricText( (long)nRightMargin,
                                            eCoreMetric, ePresentationMetric );
                    aText += SVX_RESSTR(GetMetricId(ePresentationMetric));
                }
            }
            break;

            default:
                if ( !pIntl )
                    pIntl = ScGlobal::pScInternational;
                pItem->GetPresentation( ePresentation, eCoreMetric, ePresentationMetric, aText, pIntl );

        }

        if ( aText.Len() )
        {
            rText += aText;
            rText += aDel;
        }

        pItem = aIter.NextItem();
    }

    rText.EraseTrailingChars();
    rText.EraseTrailingChars( '+' );
    rText.EraseTrailingChars();

    return ePresentation;
}

SfxItemPresentation __EXPORT ScDocumentPool::GetPresentation(
    const SfxPoolItem&  rItem,
    SfxItemPresentation ePresentation,
    SfxMapUnit          ePresentationMetric,
    String&             rText,
    const International* pIntl ) const
{
    USHORT  nW = rItem.Which();
    String aStrYes  ( ScGlobal::GetRscString(STR_YES) );
    String aStrNo   ( ScGlobal::GetRscString(STR_NO) );
    String aStrSep = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(": "));

    switch( nW )
    {
        case ATTR_PAGE_TOPDOWN:
        switch ( ePresentation )
        {
            case SFX_ITEM_PRESENTATION_COMPLETE:
            rText  = ScGlobal::GetRscString(STR_SCATTR_PAGE_PRINTDIR);
            rText += aStrSep;
//          break; // DURCHFALLEN!!!
            case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += ((const SfxBoolItem&)rItem).GetValue() ?
                ScGlobal::GetRscString(STR_SCATTR_PAGE_TOPDOWN) :
                ScGlobal::GetRscString(STR_SCATTR_PAGE_LEFTRIGHT) ;
            break;
        }
        break;

        case ATTR_PAGE_HEADERS:
        switch ( ePresentation )
        {
            case SFX_ITEM_PRESENTATION_COMPLETE:
            rText  = ScGlobal::GetRscString(STR_SCATTR_PAGE_HEADERS);
            rText += aStrSep;
//          break; // DURCHFALLEN!!!
            case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += ((const SfxBoolItem&)rItem).GetValue() ? aStrYes : aStrNo ;
            break;
        }
        break;

        case ATTR_PAGE_NULLVALS:
        switch ( ePresentation )
        {
            case SFX_ITEM_PRESENTATION_COMPLETE:
            rText  = ScGlobal::GetRscString(STR_SCATTR_PAGE_NULLVALS);
            rText += aStrSep;
//          break; // DURCHFALLEN!!!
            case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += ((const SfxBoolItem&)rItem).GetValue() ? aStrYes : aStrNo ;
            break;
        }
        break;

        case ATTR_PAGE_FORMULAS:
        switch ( ePresentation )
        {
            case SFX_ITEM_PRESENTATION_COMPLETE:
            rText  = ScGlobal::GetRscString(STR_SCATTR_PAGE_NULLVALS);
            rText += aStrSep;
//          break; // DURCHFALLEN!!!
            case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += ((const SfxBoolItem&)rItem).GetValue() ? aStrYes : aStrNo ;
            break;
        }
        break;

        case ATTR_PAGE_NOTES:
        switch ( ePresentation )
        {
            case SFX_ITEM_PRESENTATION_COMPLETE:
            rText  = ScGlobal::GetRscString(STR_SCATTR_PAGE_NOTES);
            rText += aStrSep;
//          break; // DURCHFALLEN!!!
            case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += ((const SfxBoolItem&)rItem).GetValue() ? aStrYes : aStrNo ;
            break;
        }
        break;

        case ATTR_PAGE_GRID:
        switch ( ePresentation )
        {
            case SFX_ITEM_PRESENTATION_COMPLETE:
            rText  = ScGlobal::GetRscString(STR_SCATTR_PAGE_GRID);
            rText += aStrSep;
//          break; // DURCHFALLEN!!!
            case SFX_ITEM_PRESENTATION_NAMELESS:
            rText += ((const SfxBoolItem&)rItem).GetValue() ? aStrYes : aStrNo ;
            break;
        }
        break;

        case ATTR_PAGE_SCALETOPAGES:
        {
            USHORT  nPagNo = ((const SfxUInt16Item&)rItem).GetValue();

            if( nPagNo )
            {
                switch ( ePresentation )
                {
                    case SFX_ITEM_PRESENTATION_COMPLETE:
                    rText  = ScGlobal::GetRscString(STR_SCATTR_PAGE_SCALETOPAGES);
                    rText += aStrSep;
//                  break; // DURCHFALLEN!!!
                    case SFX_ITEM_PRESENTATION_NAMELESS:
                    rText += String::CreateFromInt32( nPagNo );
                    break;
                }
            }
            else
            {
                ePresentation = SFX_ITEM_PRESENTATION_NONE;
            }
        }
        break;

        case ATTR_PAGE_FIRSTPAGENO:
        {
            USHORT  nPagNo = ((const SfxUInt16Item&)rItem).GetValue();

            if( nPagNo )
            {
                switch ( ePresentation )
                {
                    case SFX_ITEM_PRESENTATION_COMPLETE:
                    rText  = ScGlobal::GetRscString(STR_SCATTR_PAGE_FIRSTPAGENO);
                    rText += aStrSep;
//                  break; // DURCHFALLEN!!!
                    case SFX_ITEM_PRESENTATION_NAMELESS:
                    rText += String::CreateFromInt32( nPagNo );
                    break;
                }
            }
            else
            {
                ePresentation = SFX_ITEM_PRESENTATION_NONE;
            }
        }
        break;

        case ATTR_PAGE_SCALE:
        {
            USHORT  nPercent = ((const SfxUInt16Item&)rItem).GetValue();

            if( nPercent )
            {
                switch ( ePresentation )
                {
                    case SFX_ITEM_PRESENTATION_COMPLETE:
                    rText  = ScGlobal::GetRscString(STR_SCATTR_PAGE_SCALE);
                    rText += aStrSep;
//                  break; // DURCHFALLEN!!!
                    case SFX_ITEM_PRESENTATION_NAMELESS:
                    rText += String::CreateFromInt32( nPercent );
                    rText += '%';
                    break;
                }
            }
            else
            {
                ePresentation = SFX_ITEM_PRESENTATION_NONE;
            }
        }
        break;

        case ATTR_PAGE_HEADERSET:
        {
            String  aBuffer;

            if( lcl_HFPresentation( rItem, ePresentation, GetMetric( nW ), ePresentationMetric, aBuffer, pIntl ) != SFX_ITEM_PRESENTATION_NONE )
            {
                rText  = ScGlobal::GetRscString(STR_HEADER);
                rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " ( " ));
                rText += aBuffer;
                rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " ) " ));
            }
        }
        break;

        case ATTR_PAGE_FOOTERSET:
        {
            String  aBuffer;

            if( lcl_HFPresentation( rItem, ePresentation, GetMetric( nW ), ePresentationMetric, aBuffer, pIntl ) != SFX_ITEM_PRESENTATION_NONE )
            {
                rText  = ScGlobal::GetRscString(STR_FOOTER);
                rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " ( " ));
                rText += aBuffer;
                rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " ) " ));
            }
        }
        break;

/*
        case ATTR_PAGE_HEADERLEFT:
        rText = "SID_SCATTR_PAGE_HEADERLEFT";
        break;

        case ATTR_PAGE_FOOTERLEFT:
        rText = "SID_SCATTR_PAGE_FOOTERLEFT";
        break;

        case ATTR_PAGE_HEADERRIGHT:
        rText = "SID_SCATTR_PAGE_HEADERRIGHT";
        break;

        case ATTR_PAGE_FOOTERRIGHT:
        rText = "SID_SCATTR_PAGE_FOOTERRIGHT";
        break;
*/

        default:
            if ( !pIntl )
                pIntl = ScGlobal::pScInternational;
            ePresentation = rItem.GetPresentation( ePresentation, GetMetric( nW ), ePresentationMetric, rText, pIntl );
        break;
    }

    return ePresentation;
}

SfxMapUnit __EXPORT ScDocumentPool::GetMetric( USHORT nWhich ) const
{
    //  eigene Attribute: Twips, alles andere 1/100 mm

    if ( nWhich >= ATTR_STARTINDEX && nWhich <= ATTR_ENDINDEX )
        return SFX_MAPUNIT_TWIP;
    else
        return SFX_MAPUNIT_100TH_MM;
}





