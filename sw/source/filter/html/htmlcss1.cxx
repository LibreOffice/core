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

#include "hintids.hxx"
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svl/urihelper.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/svapp.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <fmtpdsc.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include "frmatr.hxx"
#include <charfmt.hxx>
#include <docary.hxx>
#include <svx/svxids.hrc>

#include "doc.hxx"
#include <IDocumentStylePoolAccess.hxx>
#include "pam.hxx"
#include "ndtxt.hxx"
#include "poolfmt.hxx"
#include "docsh.hxx"
#include "paratr.hxx"
#include "pagedesc.hxx"
#include "css1kywd.hxx"
#include "swcss1.hxx"
#include "htmlnum.hxx"
#include "swhtml.hxx"
#include <numrule.hxx>
#include <css1atr.hxx>

using namespace ::com::sun::star;

// Wie viele Zeilen/Zeichen sind fuer DropCaps erlaubt?
// (Gibt es vielleicht woanders entsprechende Werte?)
#define MAX_DROPCAP_LINES 9
#define MAX_DROPCAP_CHARS 9

static void lcl_swcss1_setEncoding( SwFormat& rFormat, rtl_TextEncoding eEnc );

// Implementierung des SwCSS1Parsers (eigentlich swcss1.cxx)
static struct SwCSS1ItemIds
{
    sal_uInt16 nFormatBreak;
    sal_uInt16 nFormatPageDesc;
    sal_uInt16 nFormatKeep;

    SwCSS1ItemIds() :
        nFormatBreak( RES_BREAK ),
        nFormatPageDesc( RES_PAGEDESC ),
        nFormatKeep( RES_KEEP )
    {}

} aItemIds;

void SwCSS1Parser::ChgPageDesc( const SwPageDesc *pPageDesc,
                                const SwPageDesc& rNewPageDesc )
{
    size_t pos;
    bool found = pDoc->ContainsPageDesc( pPageDesc, &pos );
    OSL_ENSURE( found, "Seitenvorlage nicht gefunden" );
    if (found)
        pDoc->ChgPageDesc( pos, rNewPageDesc );
}

SwCSS1Parser::SwCSS1Parser( SwDoc *pD, sal_uInt32 aFHeights[7], const OUString& rBaseURL, bool bNewDoc ) :
    SvxCSS1Parser( pD->GetAttrPool(), rBaseURL, MM50/2,
                   reinterpret_cast<sal_uInt16*>(&aItemIds), sizeof(aItemIds) / sizeof(sal_uInt16) ),
    pDoc( pD ),
    nDropCapCnt( 0 ),
    bIsNewDoc( bNewDoc ),
    bBodyBGColorSet( false ),
    bBodyBackgroundSet( false ),
    bBodyTextSet( false ),
    bBodyLinkSet( false ),
    bBodyVLinkSet( false ),
    bSetFirstPageDesc( false ),
    bSetRightPageDesc( false ),
    bTableHeaderTextCollSet( false ),
    bTableTextCollSet( false ),
    bLinkCharFormatsSet( false )
{
    aFontHeights[0] = aFHeights[0];
    aFontHeights[1] = aFHeights[1];
    aFontHeights[2] = aFHeights[2];
    aFontHeights[3] = aFHeights[3];
    aFontHeights[4] = aFHeights[4];
    aFontHeights[5] = aFHeights[5];
    aFontHeights[6] = aFHeights[6];
}

SwCSS1Parser::~SwCSS1Parser()
{
}

// Feature: PrintExt
bool SwCSS1Parser::SetFormatBreak( SfxItemSet& rItemSet,
                                const SvxCSS1PropertyInfo& rPropInfo )
{
    SvxBreak eBreak = SVX_BREAK_NONE;
    bool bKeep = false;
    bool bSetKeep = false, bSetBreak = false, bSetPageDesc = false;
    const SwPageDesc *pPageDesc = nullptr;
    switch( rPropInfo.ePageBreakBefore )
    {
    case SVX_CSS1_PBREAK_ALWAYS:
        eBreak = SVX_BREAK_PAGE_BEFORE;
        bSetBreak = true;
        break;
    case SVX_CSS1_PBREAK_LEFT:
        pPageDesc = GetLeftPageDesc( true );
        bSetPageDesc = true;
        break;
    case SVX_CSS1_PBREAK_RIGHT:
        pPageDesc = GetRightPageDesc( true );
        bSetPageDesc = true;
        break;
    case SVX_CSS1_PBREAK_AUTO:
        bSetBreak = bSetPageDesc = true;
        break;
    default:
        ;
    }
    switch( rPropInfo.ePageBreakAfter )
    {
    case SVX_CSS1_PBREAK_ALWAYS:
    case SVX_CSS1_PBREAK_LEFT:
    case SVX_CSS1_PBREAK_RIGHT:
        // LEFT/RIGHT koennte man auch am Absatz davor setzen
        eBreak = SVX_BREAK_PAGE_AFTER;
        bSetBreak = true;
        break;
    case SVX_CSS1_PBREAK_AUTO:
        bSetBreak = bSetKeep = bSetPageDesc = true;
        break;
    case SVX_CSS1_PBREAK_AVOID:
        bKeep = bSetKeep = true;
        break;
    default:
        ;
    }

    if( bSetBreak )
        rItemSet.Put( SvxFormatBreakItem( eBreak, RES_BREAK ) );
    if( bSetPageDesc )
        rItemSet.Put( SwFormatPageDesc( pPageDesc ) );
    if( bSetKeep )
        rItemSet.Put( SvxFormatKeepItem( bKeep, RES_KEEP ) );

    return bSetBreak;
}

static void SetCharFormatAttrs( SwCharFormat *pCharFormat, SfxItemSet& rItemSet )
{
    const SfxPoolItem *pItem;
    static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONTSIZE,RES_CHRATR_CJK_FONTSIZE,
                                   RES_CHRATR_CTL_FONTSIZE };
       for( size_t i=0; i<SAL_N_ELEMENTS(aWhichIds); ++i )
    {
        if( SfxItemState::SET == rItemSet.GetItemState( aWhichIds[i], false,
                                                   &pItem ) &&
            static_cast<const SvxFontHeightItem *>(pItem)->GetProp() != 100)
        {
            // %-Angaben beim FontHeight-Item werden nicht unterstuetzt
            rItemSet.ClearItem( aWhichIds[i] );
        }
    }

    pCharFormat->SetFormatAttr( rItemSet );

    if( SfxItemState::SET == rItemSet.GetItemState( RES_BACKGROUND, false, &pItem ) )
    {
        // Ein Brush-Item mit RES_BACKGROUND muss noch in eines mit
        // RES_CHRATR_BACKGROUND gewandelt werden

        SvxBrushItem aBrushItem( *static_cast<const SvxBrushItem *>(pItem) );
        aBrushItem.SetWhich( RES_CHRATR_BACKGROUND );
        pCharFormat->SetFormatAttr( aBrushItem );
    }

    if( SfxItemState::SET == rItemSet.GetItemState( RES_BOX, false, &pItem ) )
    {
        SvxBoxItem aBoxItem( *static_cast<const SvxBoxItem *>(pItem) );
        aBoxItem.SetWhich( RES_CHRATR_BOX );
        pCharFormat->SetFormatAttr( aBoxItem );
    }
}

void SwCSS1Parser::SetLinkCharFormats()
{
    OSL_ENSURE( !bLinkCharFormatsSet, "Aufruf von SetLinkCharFormats unnoetig" );

    SvxCSS1MapEntry *pStyleEntry =
        GetTag( OOO_STRING_SVTOOLS_HTML_anchor );
    SwCharFormat *pUnvisited = nullptr, *pVisited = nullptr;
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        bool bColorSet = (SfxItemState::SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              false));
        pUnvisited = GetCharFormatFromPool( RES_POOLCHR_INET_NORMAL );
        SetCharFormatAttrs( pUnvisited, rItemSet );
        bBodyLinkSet |= bColorSet;

        pVisited = GetCharFormatFromPool( RES_POOLCHR_INET_VISIT );
        SetCharFormatAttrs( pVisited, rItemSet );
        bBodyVLinkSet |= bColorSet;
    }

    OUString sTmp = OOO_STRING_SVTOOLS_HTML_anchor ":link";

    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        bool bColorSet = (SfxItemState::SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              false));
        if( !pUnvisited )
            pUnvisited = GetCharFormatFromPool( RES_POOLCHR_INET_NORMAL );
        SetCharFormatAttrs( pUnvisited, rItemSet );
        bBodyLinkSet |= bColorSet;
    }

    sTmp = OOO_STRING_SVTOOLS_HTML_anchor ":visited";

    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        bool bColorSet = (SfxItemState::SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              false));
        if( !pVisited )
            pVisited = GetCharFormatFromPool( RES_POOLCHR_INET_VISIT );
        SetCharFormatAttrs( pVisited, rItemSet );
        bBodyVLinkSet |= bColorSet;
    }

    bLinkCharFormatsSet = true;
}

static void SetTextCollAttrs( SwTextFormatColl *pColl, SfxItemSet& rItemSet,
                             SvxCSS1PropertyInfo& rPropInfo,
                             SwCSS1Parser *pCSS1Parser )
{
    const SfxItemSet& rCollItemSet = pColl->GetAttrSet();
    const SfxPoolItem *pCollItem, *pItem;

    // linker, rechter Rand und Erstzeilen-Einzug
    if( (rPropInfo.bLeftMargin || rPropInfo.bRightMargin ||
         rPropInfo.bTextIndent) &&
        (!rPropInfo.bLeftMargin || !rPropInfo.bRightMargin ||
         !rPropInfo.bTextIndent) &&
        SfxItemState::SET == rCollItemSet.GetItemState(RES_LR_SPACE,true,&pCollItem) &&
        SfxItemState::SET == rItemSet.GetItemState(RES_LR_SPACE,false,&pItem) )
    {
        const SvxLRSpaceItem *pLRItem = static_cast<const SvxLRSpaceItem *>(pItem);

        SvxLRSpaceItem aLRItem( *static_cast<const SvxLRSpaceItem *>(pCollItem) );
        if( rPropInfo.bLeftMargin )
            aLRItem.SetTextLeft( pLRItem->GetTextLeft() );
        if( rPropInfo.bRightMargin )
            aLRItem.SetRight( pLRItem->GetRight() );
        if( rPropInfo.bTextIndent )
            aLRItem.SetTextFirstLineOfst( pLRItem->GetTextFirstLineOfst() );

        rItemSet.Put( aLRItem );
    }

    // oberer und unterer Rand
    if( (rPropInfo.bTopMargin || rPropInfo.bBottomMargin) &&
        (!rPropInfo.bTopMargin || !rPropInfo.bBottomMargin) &&
        SfxItemState::SET == rCollItemSet.GetItemState(RES_UL_SPACE,true,
                                                  &pCollItem) &&
        SfxItemState::SET == rItemSet.GetItemState(RES_UL_SPACE,false,&pItem) )
    {
        const SvxULSpaceItem *pULItem = static_cast<const SvxULSpaceItem *>(pItem);

        SvxULSpaceItem aULItem( *static_cast<const SvxULSpaceItem *>(pCollItem) );
        if( rPropInfo.bTopMargin )
            aULItem.SetUpper( pULItem->GetUpper() );
        if( rPropInfo.bBottomMargin )
            aULItem.SetLower( pULItem->GetLower() );

        rItemSet.Put( aULItem );
    }

    static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONTSIZE,RES_CHRATR_CJK_FONTSIZE,
                                   RES_CHRATR_CTL_FONTSIZE };
       for( size_t i=0; i<SAL_N_ELEMENTS(aWhichIds); ++i )
    {
        if( SfxItemState::SET == rItemSet.GetItemState( aWhichIds[i], false,
                                                   &pItem ) &&
            static_cast<const SvxFontHeightItem *>(pItem)->GetProp() != 100)
        {
            // %-Angaben beim FontHeight-Item werden nicht unterstuetzt
            rItemSet.ClearItem( aWhichIds[i] );
        }
    }

    pCSS1Parser->SetFormatBreak( rItemSet, rPropInfo );

    pColl->SetFormatAttr( rItemSet );
}

void SwCSS1Parser::SetTableTextColl( bool bHeader )
{
    OSL_ENSURE( !(bHeader ? bTableHeaderTextCollSet : bTableTextCollSet),
            "Aufruf von SetTableTextColl unnoetig" );

    sal_uInt16 nPoolId;
    OUString sTag;
    if( bHeader )
    {
        nPoolId = RES_POOLCOLL_TABLE_HDLN;
        sTag = OOO_STRING_SVTOOLS_HTML_tableheader;
    }
    else
    {
        nPoolId = RES_POOLCOLL_TABLE;
        sTag = OOO_STRING_SVTOOLS_HTML_tabledata;
    }

    SwTextFormatColl *pColl = nullptr;

    // The following entries will never be used again and may be changed.
    SvxCSS1MapEntry *pStyleEntry = GetTag( sTag );
    if( pStyleEntry )
    {
        pColl = GetTextFormatColl( nPoolId, aEmptyOUStr );
        SetTextCollAttrs( pColl, pStyleEntry->GetItemSet(),
                         pStyleEntry->GetPropertyInfo(), this );
    }

    OUString sTmp = sTag + " " OOO_STRING_SVTOOLS_HTML_parabreak;
    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        if( !pColl )
            pColl = GetTextFormatColl( nPoolId, aEmptyOUStr );
        SetTextCollAttrs( pColl, pStyleEntry->GetItemSet(),
                         pStyleEntry->GetPropertyInfo(), this );
    }

    if( bHeader )
        bTableHeaderTextCollSet = true;
    else
        bTableTextCollSet = true;
}

void SwCSS1Parser::SetPageDescAttrs( const SvxBrushItem *pBrush,
                                     SfxItemSet *pItemSet2 )
{
    SvxBrushItem aBrushItem( RES_BACKGROUND );
    SvxBoxItem aBoxItem( RES_BOX );
    SvxFrameDirectionItem aFrameDirItem(FRMDIR_ENVIRONMENT, RES_FRAMEDIR);
    bool bSetBrush = pBrush!=nullptr, bSetBox = false, bSetFrameDir = false;
    if( pBrush )
        aBrushItem = *pBrush;

    if( pItemSet2 )
    {
        const SfxPoolItem *pItem = nullptr;
        if( SfxItemState::SET == pItemSet2->GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            // ein Hintergrund wird gesetzt
            aBrushItem = *static_cast<const SvxBrushItem *>(pItem);
            pItemSet2->ClearItem( RES_BACKGROUND );
            bSetBrush = true;
        }

        if( SfxItemState::SET == pItemSet2->GetItemState( RES_BOX, false, &pItem ) )
        {
            // eine Umrandung wird gesetzt
            aBoxItem = *static_cast<const SvxBoxItem *>(pItem);
            pItemSet2->ClearItem( RES_BOX );
            bSetBox = true;
        }

        if( SfxItemState::SET == pItemSet2->GetItemState( RES_FRAMEDIR, false, &pItem ) )
        {
            // eine Umrandung wird gesetzt
            aFrameDirItem = *static_cast< const SvxFrameDirectionItem *>( pItem );
            pItemSet2->ClearItem( RES_FRAMEDIR );
            bSetFrameDir = true;
        }
    }

    if( bSetBrush || bSetBox || bSetFrameDir )
    {
        static sal_uInt16 aPoolIds[] = { RES_POOLPAGE_HTML, RES_POOLPAGE_FIRST,
                                     RES_POOLPAGE_LEFT, RES_POOLPAGE_RIGHT };
        for( size_t i=0; i<SAL_N_ELEMENTS(aPoolIds); i++ )
        {
            const SwPageDesc *pPageDesc = GetPageDesc( aPoolIds[i], false );
            if( pPageDesc )
            {
                SwPageDesc aNewPageDesc( *pPageDesc );
                SwFrameFormat &rMaster = aNewPageDesc.GetMaster();
                if( bSetBrush )
                    rMaster.SetFormatAttr( aBrushItem );
                if( bSetBox )
                    rMaster.SetFormatAttr( aBoxItem );
                if( bSetFrameDir )
                    rMaster.SetFormatAttr( aFrameDirItem );

                ChgPageDesc( pPageDesc, aNewPageDesc );
            }
        }
    }
}

// Feature: PrintExt
void SwCSS1Parser::SetPageDescAttrs( const SwPageDesc *pPageDesc,
                                     SfxItemSet& rItemSet,
                                     const SvxCSS1PropertyInfo& rPropInfo )
{
    if( !pPageDesc )
        return;

    SwPageDesc aNewPageDesc( *pPageDesc );
    SwFrameFormat &rMaster = aNewPageDesc.GetMaster();
    const SfxItemSet& rPageItemSet = rMaster.GetAttrSet();
    const SfxPoolItem *pPageItem, *pItem;
    bool bChanged = false;

    // linker, rechter Rand und Erstzeilen-Einzug
    if( (rPropInfo.bLeftMargin || rPropInfo.bRightMargin) &&
        SfxItemState::SET == rItemSet.GetItemState(RES_LR_SPACE,false,&pItem) )
    {
        if( (!rPropInfo.bLeftMargin || !rPropInfo.bRightMargin) &&
            SfxItemState::SET == rPageItemSet.GetItemState(RES_LR_SPACE,
                                                      true,&pPageItem) )
        {
            const SvxLRSpaceItem *pLRItem = static_cast<const SvxLRSpaceItem *>(pItem);

            SvxLRSpaceItem aLRItem( *static_cast<const SvxLRSpaceItem *>(pPageItem) );
            if( rPropInfo.bLeftMargin )
                aLRItem.SetLeft( pLRItem->GetLeft() );
            if( rPropInfo.bRightMargin )
                aLRItem.SetRight( pLRItem->GetRight() );

            rMaster.SetFormatAttr( aLRItem );
        }
        else
        {
            rMaster.SetFormatAttr( *pItem );
        }
        bChanged = true;
    }

    // oberer und unterer Rand
    if( (rPropInfo.bTopMargin || rPropInfo.bBottomMargin) &&
        SfxItemState::SET == rItemSet.GetItemState(RES_UL_SPACE,false,&pItem) )
    {
        if( (!rPropInfo.bTopMargin || !rPropInfo.bBottomMargin) &&
            SfxItemState::SET == rPageItemSet.GetItemState(RES_UL_SPACE,
                                                      true,&pPageItem) )
        {
            const SvxULSpaceItem *pULItem = static_cast<const SvxULSpaceItem *>(pItem);

            SvxULSpaceItem aULItem( *static_cast<const SvxULSpaceItem *>(pPageItem) );
            if( rPropInfo.bTopMargin )
                aULItem.SetUpper( pULItem->GetUpper() );
            if( rPropInfo.bBottomMargin )
                aULItem.SetLower( pULItem->GetLower() );

            rMaster.SetFormatAttr( aULItem );
        }
        else
        {
            rMaster.SetFormatAttr( *pItem );
        }
        bChanged = true;
    }

    // die Groesse
    if( rPropInfo.eSizeType != SVX_CSS1_STYPE_NONE )
    {
        if( rPropInfo.eSizeType == SVX_CSS1_STYPE_TWIP )
        {
            rMaster.SetFormatAttr( SwFormatFrameSize( ATT_FIX_SIZE, rPropInfo.nWidth,
                                           rPropInfo.nHeight ) );
            bChanged = true;
        }
        else
        {
            // Bei "size: auto|portrait|landscape" bleibt die bisherige
            // Groesse der Vorlage erhalten. Bei "landscape" und "portrait"
            // wird das Landscape-Flag gesetzt und evtl. die Breite/Hoehe
            // vertauscht.
            SwFormatFrameSize aFrameSz( rMaster.GetFrameSize() );
            bool bLandscape = aNewPageDesc.GetLandscape();
            if( ( bLandscape &&
                  rPropInfo.eSizeType == SVX_CSS1_STYPE_PORTRAIT ) ||
                ( !bLandscape &&
                  rPropInfo.eSizeType == SVX_CSS1_STYPE_LANDSCAPE ) )
            {
                SwTwips nTmp = aFrameSz.GetHeight();
                aFrameSz.SetHeight( aFrameSz.GetWidth() );
                aFrameSz.SetWidth( nTmp );
                rMaster.SetFormatAttr( aFrameSz );
                aNewPageDesc.SetLandscape( !bLandscape );
                bChanged = true;
            }
        }
    }

    // Geht das wirklich?
    if( SfxItemState::SET == rItemSet.GetItemState( RES_BACKGROUND, false, &pItem ) )
    {
        // eine Umrandung wird gesetzt
        rMaster.SetFormatAttr( *pItem );
        rItemSet.ClearItem( RES_BACKGROUND );
        bChanged = true;
    }

    if( bChanged )
        ChgPageDesc( pPageDesc, aNewPageDesc );
}

SvxBrushItem SwCSS1Parser::makePageDescBackground() const
{
    return pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_HTML, false )
        ->GetMaster().makeBackgroundBrushItem();
}

sal_uInt16 SwCSS1Parser::GetScriptFromClass( OUString& rClass,
                                      bool bSubClassOnly )
{
    sal_uInt16 nScriptFlags = CSS1_SCRIPT_ALL;
    sal_Int32 nLen = rClass.getLength();
    sal_Int32 nPos = nLen > 4 ? rClass.lastIndexOf( '-' ) : -1;

    if( nPos == -1 )
    {
        if( bSubClassOnly )
            return nScriptFlags;
        nPos = 0;
    }
    else
    {
        nPos++;
        nLen = nLen - nPos;
    }

    switch( nLen )
    {
    case 3:
        if( rClass.matchIgnoreAsciiCase( "cjk", nPos ) )
        {
            nScriptFlags = CSS1_SCRIPT_CJK;
        }
        else if( rClass.matchIgnoreAsciiCase( "ctl", nPos ) )
        {
            nScriptFlags = CSS1_SCRIPT_CTL;
        }
        break;
    case 7:
        if( rClass.matchIgnoreAsciiCase( "western", nPos ) )
        {
            nScriptFlags = CSS1_SCRIPT_WESTERN;
        }
        break;
    }
    if( CSS1_SCRIPT_ALL != nScriptFlags )
    {
        if( nPos )
        {
            rClass = rClass.copy( 0, nPos-1 );
        }
        else
        {
            rClass.clear();
        }
    }

    return nScriptFlags;
}

static CSS1SelectorType GetTokenAndClass( const CSS1Selector *pSelector,
                              OUString& rToken, OUString& rClass,
                              sal_uInt16& rScriptFlags )
{
    rToken = pSelector->GetString();
    rClass.clear();
    rScriptFlags = CSS1_SCRIPT_ALL;

    CSS1SelectorType eType = pSelector->GetType();
    if( CSS1_SELTYPE_ELEM_CLASS==eType  )
    {
        sal_Int32 nPos = rToken.indexOf( '.' );
        OSL_ENSURE( nPos >= 0, "kein Punkt in Class-Selektor???" );
        if( nPos >= 0 )
        {
            rClass = rToken.copy( nPos+1 );
            rToken = rToken.copy( 0, nPos );

            rScriptFlags = SwCSS1Parser::GetScriptFromClass( rClass, false );
            if( rClass.isEmpty() )
                eType = CSS1_SELTYPE_ELEMENT;
        }
    }

    rToken = rToken.toAsciiLowerCase();
    return eType;
}

static void RemoveScriptItems( SfxItemSet& rItemSet, sal_uInt16 nScript,
                               const SfxItemSet *pParentItemSet = nullptr )
{
    static const sal_uInt16 aWhichIds[3][5] =
    {
        { RES_CHRATR_FONT, RES_CHRATR_FONTSIZE, RES_CHRATR_LANGUAGE,
            RES_CHRATR_POSTURE, RES_CHRATR_WEIGHT },
        { RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONTSIZE, RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CJK_POSTURE, RES_CHRATR_CJK_WEIGHT },
        { RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONTSIZE, RES_CHRATR_CTL_LANGUAGE,
            RES_CHRATR_CTL_POSTURE, RES_CHRATR_CTL_WEIGHT }
    };

    bool aClearItems[3] = { false, false, false };
    switch( nScript )
    {
    case CSS1_SCRIPT_WESTERN:
        aClearItems[1] = aClearItems[2] =  true;
        break;
    case CSS1_SCRIPT_CJK:
        aClearItems[0] = aClearItems[2] =  true;
        break;
    case CSS1_SCRIPT_CTL:
        aClearItems[0] = aClearItems[1] =  true;
        break;
    case CSS1_SCRIPT_ALL:
        break;
    default:
        OSL_ENSURE( aClearItems[0], "unknown script type" );
        break;
       }

    for( size_t j=0; j < SAL_N_ELEMENTS(aWhichIds); ++j )
    {
        for( size_t i=0; i < SAL_N_ELEMENTS(aWhichIds[0]); ++i )
        {
            sal_uInt16 nWhich = aWhichIds[j][i];
            const SfxPoolItem *pItem;
            if( aClearItems[j] ||
                (pParentItemSet &&
                 SfxItemState::SET == rItemSet.GetItemState( nWhich, false, &pItem ) &&
                 (0==i ? swhtml_css1atr_equalFontItems( *pItem, pParentItemSet->Get(nWhich ) )
                        : *pItem == pParentItemSet->Get(nWhich ) ) ) )
            {
                rItemSet.ClearItem( nWhich );
            }
        }
    }
}

bool SwCSS1Parser::StyleParsed( const CSS1Selector *pSelector,
                                SfxItemSet& rItemSet,
                                SvxCSS1PropertyInfo& rPropInfo )
{
    if( !bIsNewDoc )
        return true;

    CSS1SelectorType eSelType = pSelector->GetType();
    const CSS1Selector *pNext = pSelector->GetNext();

    if( CSS1_SELTYPE_ID==eSelType && !pNext )
    {
        InsertId( pSelector->GetString(), rItemSet, rPropInfo );
    }
    else if( CSS1_SELTYPE_CLASS==eSelType && !pNext )
    {
        OUString aClass( pSelector->GetString() );
        sal_uInt16 nScript = GetScriptFromClass( aClass );
        if( CSS1_SCRIPT_ALL != nScript )
        {
            SfxItemSet aScriptItemSet( rItemSet );
            RemoveScriptItems( aScriptItemSet, nScript );
            InsertClass( aClass, aScriptItemSet, rPropInfo );
        }
        else
        {
            InsertClass( aClass, rItemSet, rPropInfo );
        }
    }
    else if( CSS1_SELTYPE_PAGE==eSelType )
    {
        if( !pNext ||
            (CSS1_SELTYPE_PSEUDO == pNext->GetType() &&
            (pNext->GetString().equalsIgnoreAsciiCase( "left" ) ||
             pNext->GetString().equalsIgnoreAsciiCase( "right" ) ||
             pNext->GetString().equalsIgnoreAsciiCase( "first" ) ) ) )
        {
            OUString aName;
            if( pNext )
                aName = pNext->GetString();
            InsertPage( aName,
                        pNext != nullptr,
                        rItemSet, rPropInfo );
        }
    }

    if( CSS1_SELTYPE_ELEMENT != eSelType &&
        CSS1_SELTYPE_ELEM_CLASS != eSelType)
        return true;

    // Token und Class zu dem Selektor holen
    OUString aToken2;
    OUString aClass;
    sal_uInt16 nScript;
    eSelType = GetTokenAndClass( pSelector, aToken2, aClass, nScript );
    int nToken2 = GetHTMLToken( aToken2 );

    // und noch ein ganz par Infos zum naechsten Element
    CSS1SelectorType eNextType = pNext ? pNext->GetType()
                                       : CSS1_SELTYPE_ELEMENT;

    // Erstmal ein par Spezialfaelle
    if( CSS1_SELTYPE_ELEMENT==eSelType )
    {
        switch( nToken2 )
        {
        case HTML_ANCHOR_ON:
            if( !pNext )
            {
                InsertTag( aToken2, rItemSet, rPropInfo );
                return false;
            }
            else if( pNext && CSS1_SELTYPE_PSEUDO == eNextType )
            {
                // vielleicht A:visited oder A:link

                OUString aPseudo( pNext->GetString() );
                aPseudo = aPseudo.toAsciiLowerCase();
                bool bInsert = false;
                switch( aPseudo[0] )
                {
                    case 'l':
                        if( aPseudo == "link" )
                        {
                            bInsert = true;
                        }
                        break;
                    case 'v':
                        if( aPseudo == "visited" )
                        {
                            bInsert = true;
                        }
                        break;
                }
                if( bInsert )
                {
                    OUString sTmp = aToken2 + ":" + aPseudo;
                    if( CSS1_SCRIPT_ALL != nScript )
                    {
                        SfxItemSet aScriptItemSet( rItemSet );
                        RemoveScriptItems( aScriptItemSet, nScript );
                        InsertTag( sTmp, aScriptItemSet, rPropInfo );
                    }
                    else
                    {
                        InsertTag( sTmp, rItemSet, rPropInfo );
                    }
                    return false;
                }
            }
            break;
        case HTML_BODY_ON:
            if( !pNext )
            {
                // BODY

                // Den Hintergrund muessen wir vor dem Setzen abfragen,
                // denn in SetPageDescAttrs wird er geloescht.
                const SfxPoolItem *pItem;
                if( SfxItemState::SET==rItemSet.GetItemState(RES_BACKGROUND,false,&pItem) )
                {
                    const SvxBrushItem *pBrushItem =
                        static_cast<const SvxBrushItem *>(pItem);

                    /// Body has a background color, if it is not "no fill"/"auto fill"
                    if( pBrushItem->GetColor() != COL_TRANSPARENT )
                        bBodyBGColorSet = true;
                    if( GPOS_NONE != pBrushItem->GetGraphicPos() )
                        bBodyBackgroundSet = true;
                }

                // Border and Padding
                rPropInfo.SetBoxItem( rItemSet, MIN_BORDER_DIST );

                // Ein par Attribute muessen an der Seitenvorlage gesetzt werden,
                // und zwar die, die nicht vererbt werden
                SetPageDescAttrs( nullptr, &rItemSet );

                // alle noch uebrigen Optionen koennen an der Standard-Vorlage
                // gesetzt werden und gelten dann automatisch als defaults
                if( SfxItemState::SET==rItemSet.GetItemState(RES_CHRATR_COLOR,false) )
                    bBodyTextSet = true;
                SetTextCollAttrs(
                    GetTextCollFromPool( RES_POOLCOLL_STANDARD ),
                    rItemSet, rPropInfo, this );

                return false;
            }
            break;
        }
    }
    else if( CSS1_SELTYPE_ELEM_CLASS==eSelType &&  HTML_ANCHOR_ON==nToken2 &&
             !pNext && aClass.getLength() >= 9 &&
             ('s' == aClass[0] || 'S' == aClass[0]) )
    {
        sal_uInt16 nPoolFormatId = 0;
        if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdendnote_sym) )
            nPoolFormatId = RES_POOLCHR_ENDNOTE;
        else if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdfootnote_sym) )
            nPoolFormatId = RES_POOLCHR_FOOTNOTE;
        if( nPoolFormatId )
        {
            if( CSS1_SCRIPT_ALL == nScript )
            {
                SetCharFormatAttrs( GetCharFormatFromPool(nPoolFormatId), rItemSet );
            }
            else
            {
                SfxItemSet aScriptItemSet( rItemSet );
                RemoveScriptItems( aScriptItemSet, nScript );
                SetCharFormatAttrs( GetCharFormatFromPool(nPoolFormatId),
                                 aScriptItemSet);
            }
            return false;
        }
    }

    // Jetzt werden die Selektoren verarbeitet, die zu einer Absatz-Vorlage
    // gehoehren
    sal_uInt16 nPoolCollId = 0;
    switch( nToken2 )
    {
    case HTML_HEAD1_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE1;
        break;
    case HTML_HEAD2_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE2;
        break;
    case HTML_HEAD3_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE3;
        break;
    case HTML_HEAD4_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE4;
        break;
    case HTML_HEAD5_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE5;
        break;
    case HTML_HEAD6_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE6;
        break;
    case HTML_PARABREAK_ON:
        if( aClass.getLength() >= 9 &&
            ('s' == aClass[0] || 'S' == aClass[0]) )
        {
            if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdendnote) )
                nPoolCollId = RES_POOLCOLL_ENDNOTE;
            else if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdfootnote) )
                nPoolCollId = RES_POOLCOLL_FOOTNOTE;

            if( nPoolCollId )
                aClass = aEmptyOUStr;
            else
                nPoolCollId = RES_POOLCOLL_TEXT;
        }
        else
        {
            nPoolCollId = RES_POOLCOLL_TEXT;
        }
        break;
    case HTML_ADDRESS_ON:
        nPoolCollId = RES_POOLCOLL_SENDADRESS;
        break;
    case HTML_BLOCKQUOTE_ON:
        nPoolCollId = RES_POOLCOLL_HTML_BLOCKQUOTE;
        break;
    case HTML_DT_ON:
        nPoolCollId = RES_POOLCOLL_HTML_DT;
        break;
    case HTML_DD_ON:
        nPoolCollId = RES_POOLCOLL_HTML_DD;
        break;
    case HTML_PREFORMTXT_ON:
        nPoolCollId = RES_POOLCOLL_HTML_PRE;
        break;
    case HTML_TABLEHEADER_ON:
    case HTML_TABLEDATA_ON:
        if( CSS1_SELTYPE_ELEMENT==eSelType && !pNext )
        {
            InsertTag( aToken2, rItemSet, rPropInfo );
            return false;
        }
        else if( CSS1_SELTYPE_ELEMENT==eSelType && pNext &&
                 (CSS1_SELTYPE_ELEMENT==eNextType ||
                  CSS1_SELTYPE_ELEM_CLASS==eNextType) )
        {
            // nicht TH und TD, aber TH P und TD P
            OUString aSubToken, aSubClass;
            GetTokenAndClass( pNext, aSubToken, aSubClass, nScript );
            if( HTML_PARABREAK_ON == GetHTMLToken( aSubToken ) )
            {
                aClass = aSubClass;
                pNext = pNext->GetNext();
                eNextType = pNext ? pNext->GetType() : CSS1_SELTYPE_ELEMENT;

                if( !aClass.isEmpty() || pNext )
                {
                    nPoolCollId = static_cast< sal_uInt16 >(
                        HTML_TABLEHEADER_ON == nToken2 ? RES_POOLCOLL_TABLE_HDLN
                                                      : RES_POOLCOLL_TABLE );
                }
                else
                {
                    OUString sTmp = aToken2 + " " OOO_STRING_SVTOOLS_HTML_parabreak;

                    if( CSS1_SCRIPT_ALL == nScript )
                    {
                        InsertTag( sTmp, rItemSet, rPropInfo );
                    }
                    else
                    {
                        SfxItemSet aScriptItemSet( rItemSet );
                        RemoveScriptItems( aScriptItemSet, nScript );
                        InsertTag( sTmp, aScriptItemSet, rPropInfo );
                    }

                    return false;
                }
            }
        }
        break;

        default:
            ;
    }

    if( nPoolCollId )
    {
        if( !pNext ||
            (CSS1_SELTYPE_PSEUDO==eNextType &&
             pNext->GetString().equalsIgnoreAsciiCase( "first-letter" ) &&
             SVX_ADJUST_LEFT == rPropInfo.eFloat) )
        {
            // Entweder kein zusammengesetzter Selektor oder
            // ein X:first-line { float: left; ... }

            // Die Vorlage Suchen bzw. Anlegen
            SwTextFormatColl *pColl = GetTextFormatColl( nPoolCollId, aEmptyOUStr );
            SwTextFormatColl* pParentColl = nullptr;
            if( !aClass.isEmpty() )
            {
                OUString aName( pColl->GetName() );
                AddClassName( aName, aClass );

                pParentColl = pColl;
                pColl = pDoc->FindTextFormatCollByName( aName );
                if( !pColl )
                    pColl = pDoc->MakeTextFormatColl( aName, pParentColl );
            }
            if( !pNext )
            {
                // nur die Attribute an der Vorlage setzen
                const SfxPoolItem *pItem;
                const SvxBoxItem *pBoxItem = nullptr;
                if( SfxItemState::SET ==
                        pColl->GetAttrSet().GetItemState(RES_BOX,true,&pItem) )
                    pBoxItem = static_cast<const SvxBoxItem *>(pItem);
                rPropInfo.SetBoxItem( rItemSet, MIN_BORDER_DIST, pBoxItem );
                if( CSS1_SCRIPT_ALL == nScript && !pParentColl )
                {
                    SetTextCollAttrs( pColl, rItemSet, rPropInfo, this );
                }
                else
                {
                    SfxItemSet aScriptItemSet( rItemSet );
                    RemoveScriptItems( aScriptItemSet, nScript,
                                       pParentColl ? &pParentColl->GetAttrSet() : nullptr );
                    SetTextCollAttrs( pColl, aScriptItemSet, rPropInfo, this );
                }
            }
            else
            {
                // ein Drop-Cap-Attribut basteln
                SwFormatDrop aDrop( pColl->GetDrop() );
                aDrop.GetChars() = 1;

                // die Attribute in das DropCap-Attribut einfuegen
                if( CSS1_SCRIPT_ALL == nScript )
                {
                    OUString sName(pColl->GetName());
                    FillDropCap( aDrop, rItemSet, &sName );
                }
                else
                {
                    SfxItemSet aScriptItemSet( rItemSet );
                    if( CSS1_SCRIPT_WESTERN != nScript )
                    {
                        aScriptItemSet.ClearItem( RES_CHRATR_FONT );
                        aScriptItemSet.ClearItem( RES_CHRATR_LANGUAGE );
                        aScriptItemSet.ClearItem( RES_CHRATR_POSTURE );
                        aScriptItemSet.ClearItem( RES_CHRATR_WEIGHT );
                    }
                    if( CSS1_SCRIPT_CJK != nScript )
                    {
                        aScriptItemSet.ClearItem( RES_CHRATR_CJK_FONT );
                        aScriptItemSet.ClearItem( RES_CHRATR_CJK_LANGUAGE );
                        aScriptItemSet.ClearItem( RES_CHRATR_CJK_POSTURE );
                        aScriptItemSet.ClearItem( RES_CHRATR_CJK_WEIGHT );
                    }
                    if( CSS1_SCRIPT_CTL != nScript )
                    {
                        aScriptItemSet.ClearItem( RES_CHRATR_CTL_FONT );
                        aScriptItemSet.ClearItem( RES_CHRATR_CTL_LANGUAGE );
                        aScriptItemSet.ClearItem( RES_CHRATR_CTL_POSTURE );
                        aScriptItemSet.ClearItem( RES_CHRATR_CTL_WEIGHT );
                    }
                    OUString sName(pColl->GetName());
                    FillDropCap( aDrop, aScriptItemSet, &sName );
                }

                // Das Attribut nur setzen, wenn float: left angegeben wurde
                // und das Initial ueber mehrere Zeilen geht. Sonst wird die
                // ggf. angelegte Zeichen-Vorlage spaeter ueber den Namen
                // gesucht und gesetzt.
                if( aDrop.GetLines() > 1 &&
                    (SVX_ADJUST_LEFT == rPropInfo.eFloat  ||
                     CSS1_SCRIPT_ALL == nScript) )
                {
                    pColl->SetFormatAttr( aDrop );
                }
            }

            return false;
        }

        return true;
    }

    // Jetzt werden die Selektoten verarbeitet, die zu einer Zechenvorlage
    // gehoehren. Zusammengesetzte gibt es hier allerdings nich nicht.
    if( pNext )
        return true;

    SwCharFormat *pCFormat = GetChrFormat( static_cast< sal_uInt16 >(nToken2), aEmptyOUStr );
    if( pCFormat )
    {
        SwCharFormat *pParentCFormat = nullptr;
        if( !aClass.isEmpty() )
        {
            OUString aName( pCFormat->GetName() );
            AddClassName( aName, aClass );
            pParentCFormat = pCFormat;

            pCFormat = pDoc->FindCharFormatByName( aName );
            if( !pCFormat )
            {
                pCFormat = pDoc->MakeCharFormat( aName, pParentCFormat );
                pCFormat->SetAuto(false);
            }
        }

        if( CSS1_SCRIPT_ALL == nScript && !pParentCFormat )
        {
            SetCharFormatAttrs( pCFormat, rItemSet );
        }
        else
        {
            SfxItemSet aScriptItemSet( rItemSet );
            RemoveScriptItems( aScriptItemSet, nScript,
                               pParentCFormat ? &pParentCFormat->GetAttrSet() : nullptr );
            SetCharFormatAttrs( pCFormat, aScriptItemSet );
        }
        return false;
    }

    return true;
}

sal_uInt32 SwCSS1Parser::GetFontHeight( sal_uInt16 nSize ) const
{
    return aFontHeights[ nSize>6 ? 6 : nSize ];
}

const FontList *SwCSS1Parser::GetFontList() const
{
    const FontList *pFList = nullptr;
    SwDocShell *pDocSh = pDoc->GetDocShell();
    if( pDocSh )
    {
        const SvxFontListItem *pFListItem =
            static_cast<const SvxFontListItem *>(pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST));
        if( pFListItem )
            pFList = pFListItem->GetFontList();
    }

    return pFList;
}

SwCharFormat* SwCSS1Parser::GetChrFormat( sal_uInt16 nToken2, const OUString& rClass ) const
{
    // die entsprechende Vorlage suchen
    sal_uInt16 nPoolId = 0;
    const sal_Char* sName = nullptr;
    switch( nToken2 )
    {
    case HTML_EMPHASIS_ON:      nPoolId = RES_POOLCHR_HTML_EMPHASIS;    break;
    case HTML_CITIATION_ON:     nPoolId = RES_POOLCHR_HTML_CITIATION;   break;
    case HTML_STRONG_ON:        nPoolId = RES_POOLCHR_HTML_STRONG;      break;
    case HTML_CODE_ON:          nPoolId = RES_POOLCHR_HTML_CODE;        break;
    case HTML_SAMPLE_ON:        nPoolId = RES_POOLCHR_HTML_SAMPLE;      break;
    case HTML_KEYBOARD_ON:      nPoolId = RES_POOLCHR_HTML_KEYBOARD;    break;
    case HTML_VARIABLE_ON:      nPoolId = RES_POOLCHR_HTML_VARIABLE;    break;
    case HTML_DEFINSTANCE_ON:   nPoolId = RES_POOLCHR_HTML_DEFINSTANCE; break;
    case HTML_TELETYPE_ON:      nPoolId = RES_POOLCHR_HTML_TELETYPE;    break;

    case HTML_SHORTQUOTE_ON:    sName = OOO_STRING_SVTOOLS_HTML_shortquote;     break;
    case HTML_LANGUAGE_ON:      sName = OOO_STRING_SVTOOLS_HTML_language;   break;
    case HTML_AUTHOR_ON:        sName = OOO_STRING_SVTOOLS_HTML_author;         break;
    case HTML_PERSON_ON:        sName = OOO_STRING_SVTOOLS_HTML_person;         break;
    case HTML_ACRONYM_ON:       sName = OOO_STRING_SVTOOLS_HTML_acronym;        break;
    case HTML_ABBREVIATION_ON:  sName = OOO_STRING_SVTOOLS_HTML_abbreviation;   break;
    case HTML_INSERTEDTEXT_ON:  sName = OOO_STRING_SVTOOLS_HTML_insertedtext;   break;
    case HTML_DELETEDTEXT_ON:   sName = OOO_STRING_SVTOOLS_HTML_deletedtext;    break;
    }

    // die Vorlage suchen oder anlegen (geht nur mit Namen)
    if( !nPoolId && !sName )
        return nullptr;

    // Die Vorlage (ohne Class) suchen oder anlegen
    SwCharFormat *pCFormat = nullptr;
    if( nPoolId )
    {
        pCFormat = GetCharFormatFromPool( nPoolId );
    }
    else
    {
        OUString sCName( OUString::createFromAscii(sName) );
        pCFormat = pDoc->FindCharFormatByName( sCName );
        if( !pCFormat )
        {
            pCFormat = pDoc->MakeCharFormat( sCName, pDoc->GetDfltCharFormat() );
            pCFormat->SetAuto(false);
        }
    }

    OSL_ENSURE( pCFormat, "Keine Zeichen-Vorlage???" );

    // Wenn es eine Klasse gibt, die Klassen-Vorlage suchen aber nicht
    // neu anlegen.
    OUString aClass( rClass );
    GetScriptFromClass( aClass, false );
    if( !aClass.isEmpty() )
    {
        OUString aTmp( pCFormat->GetName() );
        AddClassName( aTmp, aClass );
        SwCharFormat *pClassCFormat = pDoc->FindCharFormatByName( aTmp );
        if( pClassCFormat )
        {
            pCFormat = pClassCFormat;
        }
        else
        {
            const SvxCSS1MapEntry *pClass = GetClass( aClass );
            if( pClass )
            {
                pCFormat = pDoc->MakeCharFormat( aTmp, pCFormat );
                pCFormat->SetAuto(false);
                SfxItemSet aItemSet( pClass->GetItemSet() );
                SetCharFormatAttrs( pCFormat, aItemSet );
            }
        }
    }

    return pCFormat;
}

SwTextFormatColl *SwCSS1Parser::GetTextCollFromPool( sal_uInt16 nPoolId ) const
{
    const SwTextFormatColls::size_type nOldArrLen = pDoc->GetTextFormatColls()->size();

    SwTextFormatColl *pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( nPoolId, false );

    if( bIsNewDoc )
    {
        const SwTextFormatColls::size_type nArrLen = pDoc->GetTextFormatColls()->size();
        for( SwTextFormatColls::size_type i=nOldArrLen; i<nArrLen; ++i )
            lcl_swcss1_setEncoding( *(*pDoc->GetTextFormatColls())[i],
                                    GetDfltEncoding() );
    }

    return pColl;
}

SwCharFormat *SwCSS1Parser::GetCharFormatFromPool( sal_uInt16 nPoolId ) const
{
    const SwCharFormats::size_type nOldArrLen = pDoc->GetCharFormats()->size();

    SwCharFormat *pCharFormat = pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( nPoolId );

    if( bIsNewDoc )
    {
        const SwCharFormats::size_type nArrLen = pDoc->GetCharFormats()->size();

        for( SwCharFormats::size_type i=nOldArrLen; i<nArrLen; i++ )
            lcl_swcss1_setEncoding( *(*pDoc->GetCharFormats())[i],
                                    GetDfltEncoding() );
    }

    return pCharFormat;
}

SwTextFormatColl *SwCSS1Parser::GetTextFormatColl( sal_uInt16 nTextColl,
                                           const OUString& rClass )
{
    SwTextFormatColl* pColl = nullptr;

    OUString aClass( rClass );
    GetScriptFromClass( aClass, false );
    if( RES_POOLCOLL_TEXT == nTextColl && aClass.getLength() >= 9 &&
        ('s' == aClass[0] || 'S' == aClass[0] ) )
    {
        if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdendnote) )
        {
            nTextColl = RES_POOLCOLL_ENDNOTE;
            aClass = aEmptyOUStr;
        }
        else if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdfootnote) )
        {
            nTextColl = RES_POOLCOLL_FOOTNOTE;
            aClass = aEmptyOUStr;
        }
    }

    if( USER_FMT & nTextColl )       // eine vom Reader angelegte
    {
        OSL_ENSURE( false, "Wo kommt die Benutzer-Vorlage her?" );
        pColl = GetTextCollFromPool( RES_POOLCOLL_STANDARD );
    }
    else
    {
        pColl = GetTextCollFromPool( nTextColl );
    }

    OSL_ENSURE( pColl, "Keine Absatz-Vorlage???" );
    if( !aClass.isEmpty() )
    {
        OUString aTmp( pColl->GetName() );
        AddClassName( aTmp, aClass );
        SwTextFormatColl* pClassColl = pDoc->FindTextFormatCollByName( aTmp );

        if( !pClassColl &&
            (nTextColl==RES_POOLCOLL_TABLE ||
             nTextColl==RES_POOLCOLL_TABLE_HDLN) )
        {
            // Wenn dieser Fall eintritt, dann wurde ein <TD><P CLASS=foo>
            // gelesen, aber die TD.foo Vorlage nicht gefunden. Dann muessen
            // wir P.foo nehmen, wenn es sie gibt.
            SwTextFormatColl* pCollText =
                GetTextCollFromPool( RES_POOLCOLL_TEXT );
            aTmp = pCollText->GetName();
            AddClassName( aTmp, aClass );
            pClassColl = pDoc->FindTextFormatCollByName( aTmp );
        }

        if( pClassColl )
        {
            pColl = pClassColl;
        }
        else
        {
            const SvxCSS1MapEntry *pClass = GetClass( aClass );
            if( pClass )
            {
                pColl = pDoc->MakeTextFormatColl( aTmp, pColl );
                SfxItemSet aItemSet( pClass->GetItemSet() );
                SvxCSS1PropertyInfo aPropInfo( pClass->GetPropertyInfo() );
                aPropInfo.SetBoxItem( aItemSet, MIN_BORDER_DIST );
                bool bPositioned = MayBePositioned( pClass->GetPropertyInfo() );
                if( bPositioned )
                    aItemSet.ClearItem( RES_BACKGROUND );
                SetTextCollAttrs( pColl, aItemSet, aPropInfo,
                                 this );
            }
        }

    }

    if( pColl )
        lcl_swcss1_setEncoding( *pColl, GetDfltEncoding() );

    return pColl;
}

SwPageDesc *SwCSS1Parser::GetMasterPageDesc()
{
    return pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_HTML, false );
}

static SwPageDesc *FindPageDesc(SwDoc *pDoc, sal_uInt16 nPoolId)
{
    size_t nPageDescs = pDoc->GetPageDescCnt();
    size_t nPage;
    for (nPage=0; nPage < nPageDescs &&
         pDoc->GetPageDesc(nPage).GetPoolFormatId() != nPoolId; ++nPage)
         ;

    return nPage < nPageDescs ? &pDoc->GetPageDesc(nPage) : nullptr;
}

const SwPageDesc *SwCSS1Parser::GetPageDesc( sal_uInt16 nPoolId, bool bCreate )
{
    if( RES_POOLPAGE_HTML == nPoolId )
        return pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_HTML, false );

    const SwPageDesc *pPageDesc = FindPageDesc(pDoc, nPoolId);
    if( !pPageDesc && bCreate )
    {
        // Die erste Seite wird aus der rechten Seite erzeugt, wenn es die
        // gibt.
        SwPageDesc *pMasterPageDesc = nullptr;
        if( RES_POOLPAGE_FIRST == nPoolId )
            pMasterPageDesc = FindPageDesc(pDoc, RES_POOLPAGE_RIGHT);
        if( !pMasterPageDesc )
            pMasterPageDesc = pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_HTML, false );

        // Die neue Seitenvorlage entsteht aus dem Master durch kopieren.
        SwPageDesc *pNewPageDesc = pDoc->
            getIDocumentStylePoolAccess().GetPageDescFromPool( nPoolId, false );

        // dazu brauchen wir auch die Nummer der neuen Vorlage
        OSL_ENSURE(pNewPageDesc == FindPageDesc(pDoc, nPoolId), "Seitenvorlage nicht gefunden");

        pDoc->CopyPageDesc( *pMasterPageDesc, *pNewPageDesc, false );

        // Die Vorlagen an ihren neuen Zweck anpassen.
        const SwPageDesc *pFollow = nullptr;
        bool bSetFollowFollow = false;
        switch( nPoolId )
        {
        case RES_POOLPAGE_FIRST:
            // Wenn es schon eine linke Seite gibt, dann ist das die
            // Folge-Vorlage, sonst ist es die HTML-Vorlage.
            pFollow = GetLeftPageDesc();
            if( !pFollow )
                pFollow = pMasterPageDesc;
            break;

        case RES_POOLPAGE_RIGHT:
            // Wenn die linke Vorlage schon angelegt ist, passiert hier gar
            // nichts. Sonst wird die linke Vorlage angelegt und sorgt auch
            // fuer die richtige Verkettung mit der rechten Voralge.
            GetLeftPageDesc( true );
            break;

        case RES_POOLPAGE_LEFT:
            // Die rechte Vorlage wird angelegt, wenn sie noch nicht existiert.
            // Es findet aber keine Verkettung statt.
            // Wenn schon eine erste Seitenvorlage existiert, wird die linke
            // Vorlage die Folge-Vorlage der ersten Seite.
            pFollow = GetRightPageDesc( true );
            bSetFollowFollow = true;
            {
                const SwPageDesc *pFirstPageDesc = GetFirstPageDesc();
                if( pFirstPageDesc )
                {
                    SwPageDesc aNewFirstPageDesc( *pFirstPageDesc );
                    aNewFirstPageDesc.SetFollow( pNewPageDesc );
                    ChgPageDesc( pFirstPageDesc, aNewFirstPageDesc );
                }
            }
            break;
        }

        if( pFollow )
        {
            SwPageDesc aNewPageDesc( *pNewPageDesc );
            aNewPageDesc.SetFollow( pFollow );
            ChgPageDesc( pNewPageDesc, aNewPageDesc );

            if( bSetFollowFollow )
            {
                SwPageDesc aNewFollowPageDesc( *pFollow );
                aNewFollowPageDesc.SetFollow( pNewPageDesc );
                ChgPageDesc( pFollow, aNewFollowPageDesc );
            }
        }
        pPageDesc = pNewPageDesc;
    }

    return pPageDesc;
}

bool SwCSS1Parser::MayBePositioned( const SvxCSS1PropertyInfo& rPropInfo,
                                    bool bAutoWidth )
{
    // abs-pos
    // left/top none    auto    twip    perc

    // none     Z       Z       -       -
    // auto     Z       Z       -       -
    // twip     Z       Z       S/R     -
    // perc     -       -       -       -

    // - das Tag wird absolut positioniert und left/top sind beide
    //   gegeben und enthalten auch keine %-Angabe, oder
    // - das Tag soll fliessen, und
    // - es wurde eine Breite angegeben (in beiden Faellen noetig)
    return ( ( SVX_CSS1_POS_ABSOLUTE     == rPropInfo.ePosition &&
               SVX_CSS1_LTYPE_PERCENTAGE != rPropInfo.eLeftType &&
               SVX_CSS1_LTYPE_PERCENTAGE != rPropInfo.eTopType &&
              (SVX_CSS1_LTYPE_TWIP       == rPropInfo.eLeftType ||
               SVX_CSS1_LTYPE_TWIP       != rPropInfo.eTopType) ) ||
             ( SVX_ADJUST_END            != rPropInfo.eFloat  ) ) &&
           ( bAutoWidth ||
             SVX_CSS1_LTYPE_TWIP         == rPropInfo.eWidthType ||
             SVX_CSS1_LTYPE_PERCENTAGE   == rPropInfo.eWidthType );
}

void SwCSS1Parser::AddClassName( OUString& rFormatName, const OUString& rClass )
{
    OSL_ENSURE( !rClass.isEmpty(), "Style-Klasse ohne Laenge?" );

    rFormatName += "." + rClass;
}

void SwCSS1Parser::FillDropCap( SwFormatDrop& rDrop,
                                SfxItemSet& rItemSet,
                                const OUString *pName )
{
    // die Anzahl der Zeilen entspricht in etwa einer %-Angabe
    // fuer die Hoehe (was passiert mit absoluten Hoehen???)
    sal_uInt8 nLines = rDrop.GetLines();
    const SfxPoolItem *pItem;
    if( SfxItemState::SET == rItemSet.GetItemState( RES_CHRATR_FONTSIZE, false, &pItem ) )
    {
        sal_uInt16 nProp = static_cast<const SvxFontHeightItem *>(pItem)->GetProp();
        nLines = (sal_uInt8)((nProp + 50) / 100);
        if( nLines < 1 )
            nLines = 1;
        else if( nLines > MAX_DROPCAP_LINES )
            nLines = MAX_DROPCAP_LINES;

        // Nur wenn nLines>1 ist, wird das Attribut auch gesetzt. Dann
        // brauchen wir die Font-Hoehe aber auch nicht in der Zeichen-Vorlage.
        if( nLines > 1 )
        {
            rItemSet.ClearItem( RES_CHRATR_FONTSIZE );
            rItemSet.ClearItem( RES_CHRATR_CJK_FONTSIZE );
            rItemSet.ClearItem( RES_CHRATR_CTL_FONTSIZE );
        }
    }

    // Bei harter Attributierung (pName==0) koennen wir aufhoehren, wenn
    // das Initial nur ueber eine Zeile geht.
    if( nLines<=1 )
        return;

    rDrop.GetLines() = nLines;

    // ein rechter Rand wird der Abstand zum Text!
    if( SfxItemState::SET == rItemSet.GetItemState( RES_LR_SPACE, false, &pItem ) )
    {
        rDrop.GetDistance() = static_cast< sal_uInt16 >(
            static_cast<const SvxLRSpaceItem *>(pItem)->GetRight() );
        rItemSet.ClearItem( RES_LR_SPACE );
    }

    // Fuer alle anderen Attribute eine Zeichen-Vorlage anlegen
    if( rItemSet.Count() )
    {
        SwCharFormat *pCFormat = nullptr;
        OUString aName;
        if( pName )
        {
            aName = *pName;
            AddFirstLetterExt( aName );
            pCFormat = pDoc->FindCharFormatByName( aName );
        }
        else
        {
            do
            {
                aName = "first-letter " + OUString::number( (sal_Int32)(++nDropCapCnt) );
            }
            while( pDoc->FindCharFormatByName(aName) );
        }

        if( !pCFormat )
        {
            pCFormat = pDoc->MakeCharFormat( aName, pDoc->GetDfltCharFormat() );
            pCFormat->SetAuto(false);
        }
        SetCharFormatAttrs( pCFormat, rItemSet );

        // Die Zeichenvorlage braucht nur im Attribut gesetzt werden, wenn
        // auch das Attribut gesetzt wird.
        if( nLines > 1 )
            rDrop.SetCharFormat( pCFormat );
    }
}

// CSS1-sezifisches des SwHTMLParsers

_HTMLAttr **SwHTMLParser::GetAttrTabEntry( sal_uInt16 nWhich )
{
    // den zu dem Item gehoehrenden Tabellen-Eintrag ermitteln ...
    _HTMLAttr **ppAttr = nullptr;
    switch( nWhich )
    {
    case RES_CHRATR_BLINK:
        ppAttr = &m_aAttrTab.pBlink;
        break;
    case RES_CHRATR_CASEMAP:
        ppAttr = &m_aAttrTab.pCaseMap;
        break;
    case RES_CHRATR_COLOR:
        ppAttr = &m_aAttrTab.pFontColor;
        break;
    case RES_CHRATR_CROSSEDOUT:
        ppAttr = &m_aAttrTab.pStrike;
        break;
    case RES_CHRATR_ESCAPEMENT:
        ppAttr = &m_aAttrTab.pEscapement;
        break;
    case RES_CHRATR_FONT:
        ppAttr = &m_aAttrTab.pFont;
        break;
    case RES_CHRATR_CJK_FONT:
        ppAttr = &m_aAttrTab.pFontCJK;
        break;
    case RES_CHRATR_CTL_FONT:
        ppAttr = &m_aAttrTab.pFontCTL;
        break;
    case RES_CHRATR_FONTSIZE:
        ppAttr = &m_aAttrTab.pFontHeight;
        break;
    case RES_CHRATR_CJK_FONTSIZE:
        ppAttr = &m_aAttrTab.pFontHeightCJK;
        break;
    case RES_CHRATR_CTL_FONTSIZE:
        ppAttr = &m_aAttrTab.pFontHeightCTL;
        break;
    case RES_CHRATR_KERNING:
        ppAttr = &m_aAttrTab.pKerning;
        break;
    case RES_CHRATR_POSTURE:
        ppAttr = &m_aAttrTab.pItalic;
        break;
    case RES_CHRATR_CJK_POSTURE:
        ppAttr = &m_aAttrTab.pItalicCJK;
        break;
    case RES_CHRATR_CTL_POSTURE:
        ppAttr = &m_aAttrTab.pItalicCTL;
        break;
    case RES_CHRATR_UNDERLINE:
        ppAttr = &m_aAttrTab.pUnderline;
        break;
    case RES_CHRATR_WEIGHT:
        ppAttr = &m_aAttrTab.pBold;
        break;
    case RES_CHRATR_CJK_WEIGHT:
        ppAttr = &m_aAttrTab.pBoldCJK;
        break;
    case RES_CHRATR_CTL_WEIGHT:
        ppAttr = &m_aAttrTab.pBoldCTL;
        break;
    case RES_CHRATR_BACKGROUND:
        ppAttr = &m_aAttrTab.pCharBrush;
        break;
    case RES_CHRATR_BOX:
        ppAttr = &m_aAttrTab.pCharBox;
        break;

    case RES_PARATR_LINESPACING:
        ppAttr = &m_aAttrTab.pLineSpacing;
        break;
    case RES_PARATR_ADJUST:
        ppAttr = &m_aAttrTab.pAdjust;
        break;

    case RES_LR_SPACE:
        ppAttr = &m_aAttrTab.pLRSpace;
        break;
    case RES_UL_SPACE:
        ppAttr = &m_aAttrTab.pULSpace;
        break;
    case RES_BOX:
        ppAttr = &m_aAttrTab.pBox;
        break;
    case RES_BACKGROUND:
        ppAttr = &m_aAttrTab.pBrush;
        break;
    case RES_BREAK:
        ppAttr = &m_aAttrTab.pBreak;
        break;
    case RES_PAGEDESC:
        ppAttr = &m_aAttrTab.pPageDesc;
        break;
    case RES_PARATR_SPLIT:
        ppAttr = &m_aAttrTab.pSplit;
        break;
    case RES_PARATR_WIDOWS:
        ppAttr = &m_aAttrTab.pWidows;
        break;
    case RES_PARATR_ORPHANS:
        ppAttr = &m_aAttrTab.pOrphans;
        break;
    case RES_KEEP:
        ppAttr = &m_aAttrTab.pKeep;
        break;

    case RES_CHRATR_LANGUAGE:
        ppAttr = &m_aAttrTab.pLanguage;
        break;
    case RES_CHRATR_CJK_LANGUAGE:
        ppAttr = &m_aAttrTab.pLanguageCJK;
        break;
    case RES_CHRATR_CTL_LANGUAGE:
        ppAttr = &m_aAttrTab.pLanguageCTL;
        break;

    case RES_FRAMEDIR:
        ppAttr = &m_aAttrTab.pDirection;
        break;
    }

    return ppAttr;
}

void SwHTMLParser::NewStyle()
{
    OUString sType;

    const HTMLOptions& rOptions2 = GetOptions();
    for (size_t i = rOptions2.size(); i; )
    {
        const HTMLOption& rOption = rOptions2[--i];
        if( HTML_O_TYPE == rOption.GetToken() )
            sType = rOption.GetString();
    }

    m_bIgnoreRawData = sType.getLength() &&
                     !sType.getToken(0,';').equalsAscii(sCSS_mimetype);
}

void SwHTMLParser::EndStyle()
{
    m_bIgnoreRawData = false;

    if( !m_aStyleSource.isEmpty() )
    {
        m_pCSS1Parser->ParseStyleSheet( m_aStyleSource );
        m_aStyleSource.clear();
    }
}

bool SwHTMLParser::FileDownload( const OUString& rURL,
                                 OUString& rStr )
{
    // View wegschmeissen (wegen Reschedule)
    SwViewShell *pOldVSh = CallEndAction();

    // Ein Medium anlegen
    SfxMedium aDLMedium( rURL, StreamMode::READ | StreamMode::SHARE_DENYWRITE );

    SvStream* pStream = aDLMedium.GetInStream();
    if( pStream )
    {
        SvMemoryStream aStream;
        aStream.WriteStream( *pStream );

        aStream.Seek( STREAM_SEEK_TO_END );
        rStr = OUString(static_cast<const sal_Char *>(aStream.GetData()), aStream.Tell(),
            GetSrcEncoding());
    }

    // wurde abgebrochen?
    if( ( m_pDoc->GetDocShell() && m_pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == m_pDoc->getReferenceCount() )
    {
        // wurde der Import vom SFX abgebrochen?
        eState = SVPAR_ERROR;
        pStream = nullptr;
    }

    // recreate View
    SwViewShell *const pVSh = CallStartAction( pOldVSh );
    OSL_ENSURE( pOldVSh == pVSh, "FileDownload: SwViewShell changed on us" );
    (void) pVSh;

    return pStream!=nullptr;
}

void SwHTMLParser::InsertLink()
{
    bool bFinishDownload = false;
    if( m_pPendStack )
    {
        OSL_ENSURE( ShouldFinishFileDownload(),
                "Pending-Stack ohne File-Download?" );

        SwPendingStack* pTmp = m_pPendStack->pNext;
        delete m_pPendStack;
        m_pPendStack = pTmp;
        OSL_ENSURE( !m_pPendStack, "Wo kommt der Pending-Stack her?" );

        bFinishDownload = true;
    }
    else
    {
        OUString sRel, sHRef, sType;

        const HTMLOptions& rOptions2 = GetOptions();
        for (size_t i = rOptions2.size(); i; )
        {
            const HTMLOption& rOption = rOptions2[--i];
            switch( rOption.GetToken() )
            {
                case HTML_O_REL:
                    sRel = rOption.GetString();
                    break;
                case HTML_O_HREF:
                    sHRef = URIHelper::SmartRel2Abs( INetURLObject( m_sBaseURL ), rOption.GetString(), Link<OUString *, bool>(), false );
                    break;
                case HTML_O_TYPE:
                    sType = rOption.GetString();
                    break;
            }
        }

        if( !sHRef.isEmpty() && sRel.equalsIgnoreAsciiCase( "STYLESHEET" ) &&
            ( sType.isEmpty() ||
              sType.getToken(0,';').equalsAscii(sCSS_mimetype) ) )
        {
            if( GetMedium() )
            {
                // Download des Style-Source starten
                StartFileDownload(sHRef);
                if( IsParserWorking() )
                {
                    // Der Style wurde synchron geladen und wir koennen
                    // es direkt aufrufen.
                    bFinishDownload = true;
                }
                else
                {
                    // Der Style wird asynchron geladen und ist erst beim
                    // naechsten Continue-Aufruf da. Wir muessen deshalb einen
                    // Pending-Stack anlegen, damit wir hierher zurueckkehren
                    m_pPendStack = new SwPendingStack( HTML_LINK, m_pPendStack );
                }
            }
            else
            {
                // File synchron holen
                OUString sSource;
                if( FileDownload( sHRef, sSource ) )
                    m_pCSS1Parser->ParseStyleSheet( sSource );
            }
        }
    }

    if( bFinishDownload )
    {
        OUString sSource;
        if( FinishFileDownload( sSource ) && !sSource.isEmpty() )
            m_pCSS1Parser->ParseStyleSheet( sSource );
    }
}

bool SwCSS1Parser::ParseStyleSheet( const OUString& rIn )
{
    if( !SvxCSS1Parser::ParseStyleSheet( rIn ) )
        return false;

    SwPageDesc *pMasterPageDesc =
        pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_HTML, false );

    SvxCSS1MapEntry *pPageEntry = GetPage( aEmptyOUStr, false );
    if( pPageEntry )
    {
        // @page (wirkt auf alle Seiten, die es schon gibt

        SetPageDescAttrs( pMasterPageDesc, pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );

        // Fuer alle anderen Seiten-Vorlagen, die es schon gibt,
        // muessen die Attribute auch noch gesetzt werden

        SetPageDescAttrs( GetFirstPageDesc(), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        SetPageDescAttrs( GetLeftPageDesc(), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        SetPageDescAttrs( GetRightPageDesc(), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );

    }

    pPageEntry = GetPage( "first", true );
    if( pPageEntry )
    {
        SetPageDescAttrs( GetFirstPageDesc(true), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        bSetFirstPageDesc = true;
    }

    pPageEntry = GetPage( "right", true );
    if( pPageEntry )
    {
        SetPageDescAttrs( GetRightPageDesc(true), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        bSetRightPageDesc = true;
    }

    pPageEntry = GetPage( "left", true );
    if( pPageEntry )
        SetPageDescAttrs( GetLeftPageDesc(true), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );

    return true;
}

bool SwHTMLParser::ParseStyleOptions( const OUString &rStyle,
                                          const OUString &rId,
                                          const OUString &rClass,
                                          SfxItemSet &rItemSet,
                                          SvxCSS1PropertyInfo &rPropInfo,
                                          const OUString *pLang,
                                          const OUString *pDir )
{
    bool bRet = false;

    if( !rClass.isEmpty() )
    {
        OUString aClass( rClass );
        SwCSS1Parser::GetScriptFromClass( aClass );
        const SvxCSS1MapEntry *pClass = m_pCSS1Parser->GetClass( aClass );
        if( pClass )
        {
            SvxCSS1Parser::MergeStyles( pClass->GetItemSet(),
                                      pClass->GetPropertyInfo(),
                                      rItemSet, rPropInfo, false );
            bRet = true;
        }
    }

    if( !rId.isEmpty() )
    {
        const SvxCSS1MapEntry *pId = m_pCSS1Parser->GetId( rId );
        if( pId )
            SvxCSS1Parser::MergeStyles( pId->GetItemSet(),
                                      pId->GetPropertyInfo(),
                                      rItemSet, rPropInfo, !rClass.isEmpty() );
        rPropInfo.aId = rId;
        bRet = true;
    }

    if( !rStyle.isEmpty() )
    {
        m_pCSS1Parser->ParseStyleOption( rStyle, rItemSet, rPropInfo );
        bRet = true;
    }

    if( bRet )
        rPropInfo.SetBoxItem( rItemSet, MIN_BORDER_DIST );

    if( pLang && !pLang->isEmpty() )
    {
        LanguageType eLang = LanguageTag::convertToLanguageTypeWithFallback( *pLang );
        if( LANGUAGE_DONTKNOW != eLang )
        {
            SvxLanguageItem aLang( eLang, RES_CHRATR_LANGUAGE );
            rItemSet.Put( aLang );
            aLang.SetWhich( RES_CHRATR_CJK_LANGUAGE );
            rItemSet.Put( aLang );
            aLang.SetWhich( RES_CHRATR_CTL_LANGUAGE );
            rItemSet.Put( aLang );

            bRet = true;
        }
    }
    if( pDir && !pDir->isEmpty() )
    {
        OUString aValue( *pDir );
        SvxFrameDirection eDir = FRMDIR_ENVIRONMENT;
        if (aValue.equalsIgnoreAsciiCase("LTR"))
            eDir = FRMDIR_HORI_LEFT_TOP;
        else if (aValue.equalsIgnoreAsciiCase("RTL"))
            eDir = FRMDIR_HORI_RIGHT_TOP;

        if( FRMDIR_ENVIRONMENT != eDir )
        {
            SvxFrameDirectionItem aDir( eDir, RES_FRAMEDIR );
            rItemSet.Put( aDir );

            bRet = true;
        }
    }

    return bRet;
}

void SwHTMLParser::SetAnchorAndAdjustment( const SfxItemSet & /*rItemSet*/,
                                           const SvxCSS1PropertyInfo &rPropInfo,
                                           SfxItemSet &rFrameItemSet )
{
    SwFormatAnchor aAnchor;

    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    sal_Int16 eVertOri = text::VertOrientation::NONE;
    sal_Int16 eHoriRel = text::RelOrientation::FRAME;
    sal_Int16 eVertRel = text::RelOrientation::FRAME;
    SwTwips nHoriPos = 0, nVertPos = 0;
    SwSurround eSurround = SURROUND_THROUGHT;
    if( SVX_CSS1_POS_ABSOLUTE == rPropInfo.ePosition )
    {
        if( SVX_CSS1_LTYPE_TWIP == rPropInfo.eLeftType &&
            SVX_CSS1_LTYPE_TWIP == rPropInfo.eTopType )
        {
            // Absolut positionierte Objekte sind seitengebunden, wenn
            // sie nicht schon in einem Rahmen stehen und sonst
            // Rahmengebunden.
            const SwStartNode *pFlySttNd =
                m_pPam->GetPoint()->nNode.GetNode().FindFlyStartNode();
            if( pFlySttNd )
            {
                aAnchor.SetType( FLY_AT_FLY );
                SwPosition aPos( *pFlySttNd );
                aAnchor.SetAnchor( &aPos );
            }
            else
            {
                aAnchor.SetType( FLY_AT_PAGE );
                aAnchor.SetPageNum( 1 );
            }
            nHoriPos = rPropInfo.nLeft;
            nVertPos = rPropInfo.nTop;
        }
        else
        {
            aAnchor.SetType( FLY_AT_PARA );
            aAnchor.SetAnchor( m_pPam->GetPoint() );
            eVertOri = text::VertOrientation::TOP;
            eVertRel = text::RelOrientation::CHAR;
            if( SVX_CSS1_LTYPE_TWIP == rPropInfo.eLeftType )
            {
                eHoriOri = text::HoriOrientation::NONE;
                eHoriRel = text::RelOrientation::PAGE_FRAME;
                nHoriPos = rPropInfo.nLeft;
            }
            else
            {
                eHoriOri = text::HoriOrientation::LEFT;
                eHoriRel = text::RelOrientation::FRAME;   // wird noch umgeschossen
            }
        }
    }
    else
    {
        // fliessende Objekte werden Absatzgebunden eingefuegt, wenn
        // der Absatz noch leer ist und sonst auto-gebunden.
        // Auto-gebundene Rahmen werden zunaechst an der Position davor
        // eingefuegt und erst spaeter verschoben.
        const sal_Int32 nContent = m_pPam->GetPoint()->nContent.GetIndex();
        if( nContent )
        {
            aAnchor.SetType( FLY_AT_CHAR );
            m_pPam->Move( fnMoveBackward );
            eVertOri = text::VertOrientation::CHAR_BOTTOM;
            eVertRel = text::RelOrientation::CHAR;
        }
        else
        {
            aAnchor.SetType( FLY_AT_PARA );
            eVertOri = text::VertOrientation::TOP;
            eVertRel = text::RelOrientation::PRINT_AREA;
        }

        aAnchor.SetAnchor( m_pPam->GetPoint() );

        if( nContent )
            m_pPam->Move( fnMoveForward );

        sal_uInt16 nLeftSpace = 0, nRightSpace = 0;
        short nIndent = 0;
        GetMarginsFromContextWithNumBul( nLeftSpace, nRightSpace, nIndent );

        if( SVX_ADJUST_RIGHT==rPropInfo.eFloat )
        {
            eHoriOri = text::HoriOrientation::RIGHT;
            eHoriRel = nRightSpace ? text::RelOrientation::PRINT_AREA : text::RelOrientation::FRAME;
            eSurround = SURROUND_LEFT;
        }
        else
        {
            eHoriOri = text::HoriOrientation::LEFT;
            eHoriRel = nLeftSpace ? text::RelOrientation::PRINT_AREA : text::RelOrientation::FRAME;
            eSurround = SURROUND_RIGHT;
        }
    }
    rFrameItemSet.Put( aAnchor );

    // Absolut Positioniert mit Durchlauf
    rFrameItemSet.Put( SwFormatHoriOrient( nHoriPos, eHoriOri, eHoriRel ) );
    rFrameItemSet.Put( SwFormatVertOrient( nVertPos, eVertOri, eVertRel ) );
    rFrameItemSet.Put( SwFormatSurround( eSurround ) );
}

void SwHTMLParser::SetVarSize( SfxItemSet & /*rItemSet*/,
                               SvxCSS1PropertyInfo &rPropInfo,
                               SfxItemSet &rFrameItemSet,
                               SwTwips nDfltWidth, sal_uInt8 nDfltPrcWidth )
{
    SwFrameSize eSize = ATT_MIN_SIZE;
    SwTwips nWidth = nDfltWidth, nHeight = MINFLY;
    sal_uInt8 nPrcWidth = nDfltPrcWidth, nPrcHeight = 0;
    switch( rPropInfo.eWidthType )
    {
    case SVX_CSS1_LTYPE_PERCENTAGE:
        nPrcWidth = rPropInfo.nWidth > 0 ? (sal_uInt8)rPropInfo.nWidth : 1;
        nWidth = MINFLY;
        break;
    case SVX_CSS1_LTYPE_TWIP:
        nWidth = rPropInfo.nWidth > MINFLY ? rPropInfo.nWidth : MINFLY;
        nPrcWidth = 0;
        break;
    default:
        ;
    }
    switch( rPropInfo.eHeightType )
    {
    case SVX_CSS1_LTYPE_PERCENTAGE:
        nPrcHeight = rPropInfo.nHeight > 0 ? (sal_uInt8)rPropInfo.nHeight : 1;
        break;
    case SVX_CSS1_LTYPE_TWIP:
        // Netscape und MS-IE interpretieren die Hoehe regelwiedrig
        // als Mindest-Hoehe, also machwn wir das auch so.
        nHeight = rPropInfo.nHeight > MINFLY ? rPropInfo.nHeight : MINFLY;
        break;
    default:
        ;
    }

    SwFormatFrameSize aFrameSize( eSize, nWidth, nHeight );
    aFrameSize.SetWidthPercent( nPrcWidth );
    aFrameSize.SetHeightPercent( nPrcHeight );
    rFrameItemSet.Put( aFrameSize );
}

void SwHTMLParser::SetFrameFormatAttrs( SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo & /*rPropInfo*/,
                                   sal_uInt16 nFlags,
                                   SfxItemSet &rFrameItemSet )
{
    const SfxPoolItem *pItem;
    if( (nFlags & HTML_FF_BOX) != 0 &&
        SfxItemState::SET==rItemSet.GetItemState( RES_BOX, true, &pItem ) )
    {
        if( (nFlags & HTML_FF_PADDING) == 0 )
        {
            SvxBoxItem aBoxItem( *static_cast<const SvxBoxItem *>(pItem) );
            // Alle 4 Seiten gleichzeitig auf 0 setzen
            aBoxItem.SetDistance( 0 );
            rFrameItemSet.Put( aBoxItem );
        }
        else
        {
            rFrameItemSet.Put( *pItem );
        }
        rItemSet.ClearItem( RES_BOX );
    }

    if( (nFlags & HTML_FF_BACKGROUND) != 0 &&
        SfxItemState::SET==rItemSet.GetItemState( RES_BACKGROUND, true, &pItem ) )
    {
        rFrameItemSet.Put( *pItem );
        rItemSet.ClearItem( RES_BACKGROUND );
    }

    if( (nFlags & HTML_FF_DIRECTION) != 0 &&
        SfxItemState::SET==rItemSet.GetItemState( RES_FRAMEDIR, true, &pItem ) )
    {
        rFrameItemSet.Put( *pItem );
        rItemSet.ClearItem( RES_FRAMEDIR );
    }
}

_HTMLAttrContext *SwHTMLParser::PopContext( sal_uInt16 nToken, sal_uInt16 nLimit,
                                            bool bRemove )
{
    _HTMLAttrContexts::size_type nPos = m_aContexts.size();
    if( nPos <= m_nContextStMin )
        return nullptr;

    bool bFound = 0==nToken;
    if( nToken )
    {
        // Stack-Eintrag zu dem Token suchen
        while( nPos > m_nContextStMin )
        {
            sal_uInt16 nCntxtToken = m_aContexts[--nPos]->GetToken();
            if( nCntxtToken == nToken )
            {
                bFound = true;
                break;
            }
            else if( nCntxtToken == nLimit ) // 0 als Token kommt nicht vor
            {
                break;
            }
        }
    }
    else
    {
        nPos--;
    }

    _HTMLAttrContext *pCntxt = nullptr;
    if( bFound )
    {
        pCntxt = m_aContexts[nPos];
        if( bRemove )
            m_aContexts.erase( m_aContexts.begin() + nPos );
    }

    return pCntxt;
}

bool SwHTMLParser::GetMarginsFromContext( sal_uInt16& nLeft,
                                          sal_uInt16& nRight,
                                          short& nIndent,
                                          bool bIgnoreTopContext ) const
{
    _HTMLAttrContexts::size_type nPos = m_aContexts.size();
    if( bIgnoreTopContext )
    {
        if( !nPos )
            return false;
        else
            nPos--;
    }

    while( nPos > m_nContextStAttrMin )
    {
        const _HTMLAttrContext *pCntxt = m_aContexts[--nPos];
        if( pCntxt->IsLRSpaceChanged() )
        {
            pCntxt->GetMargins( nLeft, nRight, nIndent );
            return true;
        }
    }

    return false;
}

bool SwHTMLParser::GetMarginsFromContextWithNumBul( sal_uInt16& nLeft,
                                                    sal_uInt16& nRight,
                                                    short& nIndent ) const
{
    bool bRet = GetMarginsFromContext( nLeft, nRight, nIndent );
    const SwHTMLNumRuleInfo& rInfo = const_cast<SwHTMLParser*>(this)->GetNumInfo();
    if( rInfo.GetDepth() )
    {
        sal_uInt8 nLevel = (sal_uInt8)( (rInfo.GetDepth() <= MAXLEVEL ? rInfo.GetDepth()
                                                            : MAXLEVEL) - 1 );
        const SwNumFormat& rNumFormat = rInfo.GetNumRule()->Get(nLevel);
        nLeft = nLeft + rNumFormat.GetAbsLSpace();
        nIndent = rNumFormat.GetFirstLineOffset();
    }

    return bRet;
}

void SwHTMLParser::GetULSpaceFromContext( sal_uInt16& nUpper,
                                          sal_uInt16& nLower ) const
{
    sal_uInt16 nDfltColl = 0;
    OUString aDfltClass;

    _HTMLAttrContexts::size_type nPos = m_aContexts.size();
    while( nPos > m_nContextStAttrMin )
    {
        const _HTMLAttrContext *pCntxt = m_aContexts[--nPos];
        if( pCntxt->IsULSpaceChanged() )
        {
            pCntxt->GetULSpace( nUpper, nLower );
            return;
        }
        else if( !nDfltColl )
        {
            nDfltColl = pCntxt->GetDfltTextFormatColl();
            if( nDfltColl )
                aDfltClass = pCntxt->GetClass();
        }
    }

    if( !nDfltColl )
        nDfltColl = RES_POOLCOLL_TEXT;

    const SwTextFormatColl *pColl =
        m_pCSS1Parser->GetTextFormatColl( nDfltColl, aDfltClass );
    const SvxULSpaceItem& rULSpace = pColl->GetULSpace();
    nUpper = rULSpace.GetUpper();
    nLower = rULSpace.GetLower();
}

void SwHTMLParser::EndContextAttrs( _HTMLAttrContext *pContext, bool bRemove )
{
    _HTMLAttrs &rAttrs = pContext->GetAttrs();
    for( auto pAttr : rAttrs )
    {
        if( RES_PARATR_DROP==pAttr->GetItem().Which() )
        {
            // Fuer DropCaps noch die Anzahl der Zeichen anpassen. Wenn
            // es am Ende 0 sind, wird das Attribut invalidiert und dann
            // von _SetAttr gar nicht erst gesetzt.
            sal_Int32 nChars = m_pPam->GetPoint()->nContent.GetIndex();
            if( nChars < 1 )
                pAttr->Invalidate();
            else if( nChars > MAX_DROPCAP_CHARS )
                nChars = MAX_DROPCAP_CHARS;
            static_cast<SwFormatDrop&>(pAttr->GetItem()).GetChars() = (sal_uInt8)nChars;
        }

        EndAttr( pAttr );
    }

    if( bRemove && !rAttrs.empty() )
        rAttrs.clear();
}

void SwHTMLParser::InsertParaAttrs( const SfxItemSet& rItemSet )
{
    SfxItemIter aIter( rItemSet );

    const SfxPoolItem *pItem = aIter.FirstItem();
    while( pItem )
    {
        // den zu dem Item gehoehrenden Tabellen-Eintrag ermitteln ...
        sal_uInt16 nWhich = pItem->Which();
        _HTMLAttr **ppAttr = GetAttrTabEntry( nWhich );

        if( ppAttr )
        {
            NewAttr( ppAttr, *pItem );
            if( RES_PARATR_BEGIN > nWhich )
                (*ppAttr)->SetLikePara();
            m_aParaAttrs.push_back( *ppAttr );
            bool bSuccess = EndAttr( *ppAttr, nullptr, false );
            if (!bSuccess)
                m_aParaAttrs.pop_back();
        }

        pItem = aIter.NextItem();
    }
}

static void lcl_swcss1_setEncoding( SwFormat& rFormat, rtl_TextEncoding eEnc )
{
    if( RTL_TEXTENCODING_DONTKNOW == eEnc )
        return;

    const SfxItemSet& rItemSet = rFormat.GetAttrSet();
    static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                   RES_CHRATR_CTL_FONT };
    const SfxPoolItem *pItem;
    for( size_t i=0; i<SAL_N_ELEMENTS(aWhichIds); ++i )
    {
        if( SfxItemState::SET == rItemSet.GetItemState( aWhichIds[i], false,&pItem ) )
        {
            const SvxFontItem& rFont = *static_cast<const SvxFontItem *>(pItem);
            if( RTL_TEXTENCODING_SYMBOL != rFont.GetCharSet() )
            {
                SvxFontItem aFont( rFont.GetFamily(), rFont.GetFamilyName(),
                                   rFont.GetStyleName(), rFont.GetPitch(),
                                   eEnc, aWhichIds[i]);
                rFormat.SetFormatAttr( aFont );
            }
        }
    }
}

void SwCSS1Parser::SetDfltEncoding( rtl_TextEncoding eEnc )
{
    if( eEnc != GetDfltEncoding() )
    {
        if( bIsNewDoc )
        {
            // Set new encoding as pool default
            static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                           RES_CHRATR_CTL_FONT };
            for( size_t i=0; i<SAL_N_ELEMENTS(aWhichIds); ++i )
            {
                const SvxFontItem& rDfltFont =
                    static_cast<const SvxFontItem&>(pDoc->GetDefault( aWhichIds[i]));
                SvxFontItem aFont( rDfltFont.GetFamily(),
                                   rDfltFont.GetFamilyName(),
                                   rDfltFont.GetStyleName(),
                                   rDfltFont.GetPitch(),
                                   eEnc, aWhichIds[i] );
                pDoc->SetDefault( aFont );
            }

            // Change all paragraph styles that do specify a font.
            for( auto pTextFormatColl : *pDoc->GetTextFormatColls() )
                lcl_swcss1_setEncoding( *pTextFormatColl, eEnc );

            // Change all character styles that do specify a font.
            for( auto pCharFormat : *pDoc->GetCharFormats() )
                lcl_swcss1_setEncoding( *pCharFormat, eEnc );
        }

        SvxCSS1Parser::SetDfltEncoding( eEnc );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
