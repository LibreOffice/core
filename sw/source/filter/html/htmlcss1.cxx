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

using namespace ::com::sun::star;


// Wie viele Zeilen/Zeichen sind fuer DropCaps erlaubt?
// (Gibt es vielleicht woanders entsprechende Werte?)
#define MAX_DROPCAP_LINES 9
#define MAX_DROPCAP_CHARS 9

static void lcl_swcss1_setEncoding( SwFmt& rFmt, rtl_TextEncoding eEnc );


// Implementierung des SwCSS1Parsers (eigentlich swcss1.cxx)
static struct SwCSS1ItemIds
{
    sal_uInt16 nFmtBreak;
    sal_uInt16 nFmtPageDesc;
    sal_uInt16 nFmtKeep;

    SwCSS1ItemIds() :
        nFmtBreak( RES_BREAK ),
        nFmtPageDesc( RES_PAGEDESC ),
        nFmtKeep( RES_KEEP )
    {}

} aItemIds;

void SwCSS1Parser::ChgPageDesc( const SwPageDesc *pPageDesc,
                                const SwPageDesc& rNewPageDesc )
{
    sal_uInt16 nPageDescs = pDoc->GetPageDescCnt();
    sal_uInt16 i;
    for( i=0; i<nPageDescs; i++ )
        if( pPageDesc == &pDoc->GetPageDesc(i) )
        {
            pDoc->ChgPageDesc( i, rNewPageDesc );
            return;
        }

    OSL_ENSURE( i<nPageDescs, "Seitenvorlage nicht gefunden" );
}

SwCSS1Parser::SwCSS1Parser( SwDoc *pD, sal_uInt32 aFHeights[7], const OUString& rBaseURL, sal_Bool bNewDoc ) :
    SvxCSS1Parser( pD->GetAttrPool(), rBaseURL, MM50/2,
                   (sal_uInt16*)&aItemIds, sizeof(aItemIds) / sizeof(sal_uInt16) ),
    pDoc( pD ),
    nDropCapCnt( 0 ),
    bIsNewDoc( bNewDoc ),
    bBodyBGColorSet( sal_False ),
    bBodyBackgroundSet( sal_False ),
    bBodyTextSet( sal_False ),
    bBodyLinkSet( sal_False ),
    bBodyVLinkSet( sal_False ),
    bSetFirstPageDesc( sal_False ),
    bSetRightPageDesc( sal_False ),
    bTableHeaderTxtCollSet( sal_False ),
    bTableTxtCollSet( sal_False ),
    bLinkCharFmtsSet( sal_False )
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
sal_Bool SwCSS1Parser::SetFmtBreak( SfxItemSet& rItemSet,
                                const SvxCSS1PropertyInfo& rPropInfo )
{
    SvxBreak eBreak = SVX_BREAK_NONE;
    sal_Bool bKeep = sal_False;
    sal_Bool bSetKeep = sal_False, bSetBreak = sal_False, bSetPageDesc = sal_False;
    const SwPageDesc *pPageDesc = 0;
    switch( rPropInfo.ePageBreakBefore )
    {
    case SVX_CSS1_PBREAK_ALWAYS:
        eBreak = SVX_BREAK_PAGE_BEFORE;
        bSetBreak = sal_True;
        break;
    case SVX_CSS1_PBREAK_LEFT:
        pPageDesc = GetLeftPageDesc( sal_True );
        bSetPageDesc = sal_True;
        break;
    case SVX_CSS1_PBREAK_RIGHT:
        pPageDesc = GetRightPageDesc( sal_True );
        bSetPageDesc = sal_True;
        break;
    case SVX_CSS1_PBREAK_AUTO:
        bSetBreak = bSetPageDesc = sal_True;
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
        bSetBreak = sal_True;
        break;
    case SVX_CSS1_PBREAK_AUTO:
        bSetBreak = bSetKeep = bSetPageDesc = sal_True;
        break;
    case SVX_CSS1_PBREAK_AVOID:
        bKeep = bSetKeep = sal_True;
        break;
    default:
        ;
    }

    if( bSetBreak )
        rItemSet.Put( SvxFmtBreakItem( eBreak, RES_BREAK ) );
    if( bSetPageDesc )
        rItemSet.Put( SwFmtPageDesc( pPageDesc ) );
    if( bSetKeep )
        rItemSet.Put( SvxFmtKeepItem( bKeep, RES_KEEP ) );

    return bSetBreak;
}
// /Feature: PrintExt

static void SetCharFmtAttrs( SwCharFmt *pCharFmt, SfxItemSet& rItemSet )
{
    const SfxPoolItem *pItem;
    static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONTSIZE,RES_CHRATR_CJK_FONTSIZE,
                                   RES_CHRATR_CTL_FONTSIZE };
       for( sal_uInt16 i=0; i<3; i++ )
    {
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i], sal_False,
                                                   &pItem ) &&
            ((const SvxFontHeightItem *)pItem)->GetProp() != 100)
        {
            // %-Angaben beim FontHeight-Item werden nicht unterstuetzt
            rItemSet.ClearItem( aWhichIds[i] );
        }
    }

    pCharFmt->SetFmtAttr( rItemSet );

    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, sal_False, &pItem ) )
    {
        // Ein Brush-Item mit RES_BACKGROUND muss noch in eines mit
        // RES_CHRATR_BACKGROUND gewandelt werden

        SvxBrushItem aBrushItem( *(const SvxBrushItem *)pItem );
        aBrushItem.SetWhich( RES_CHRATR_BACKGROUND );
        pCharFmt->SetFmtAttr( aBrushItem );
    }
}

void SwCSS1Parser::SetLinkCharFmts()
{
    OSL_ENSURE( !bLinkCharFmtsSet, "Aufruf von SetLinkCharFmts unnoetig" );

    SvxCSS1MapEntry *pStyleEntry =
        GetTag( OUString(OOO_STRING_SVTOOLS_HTML_anchor) );
    SwCharFmt *pUnvisited = 0, *pVisited = 0;
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        sal_Bool bColorSet = (SFX_ITEM_SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              sal_False));
        pUnvisited = GetCharFmtFromPool( RES_POOLCHR_INET_NORMAL );
        SetCharFmtAttrs( pUnvisited, rItemSet );
        bBodyLinkSet |= bColorSet;

        pVisited = GetCharFmtFromPool( RES_POOLCHR_INET_VISIT );
        SetCharFmtAttrs( pVisited, rItemSet );
        bBodyVLinkSet |= bColorSet;
    }

    OUString sTmp = OUString( OOO_STRING_SVTOOLS_HTML_anchor ) + ":" + "link";

    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        sal_Bool bColorSet = (SFX_ITEM_SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              sal_False));
        if( !pUnvisited )
            pUnvisited = GetCharFmtFromPool( RES_POOLCHR_INET_NORMAL );
        SetCharFmtAttrs( pUnvisited, rItemSet );
        bBodyLinkSet |= bColorSet;
    }

    sTmp = OUString( OOO_STRING_SVTOOLS_HTML_anchor ) + ":" + "visited";

    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        sal_Bool bColorSet = (SFX_ITEM_SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              sal_False));
        if( !pVisited )
            pVisited = GetCharFmtFromPool( RES_POOLCHR_INET_VISIT );
        SetCharFmtAttrs( pVisited, rItemSet );
        bBodyVLinkSet |= bColorSet;
    }

    bLinkCharFmtsSet = sal_True;
}

static void SetTxtCollAttrs( SwTxtFmtColl *pColl, SfxItemSet& rItemSet,
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
        SFX_ITEM_SET == rCollItemSet.GetItemState(RES_LR_SPACE,sal_True,&pCollItem) &&
        SFX_ITEM_SET == rItemSet.GetItemState(RES_LR_SPACE,sal_False,&pItem) )
    {
        const SvxLRSpaceItem *pLRItem = (const SvxLRSpaceItem *)pItem;

        SvxLRSpaceItem aLRItem( *((const SvxLRSpaceItem *)pCollItem) );
        if( rPropInfo.bLeftMargin )
            aLRItem.SetTxtLeft( pLRItem->GetTxtLeft() );
        if( rPropInfo.bRightMargin )
            aLRItem.SetRight( pLRItem->GetRight() );
        if( rPropInfo.bTextIndent )
            aLRItem.SetTxtFirstLineOfst( pLRItem->GetTxtFirstLineOfst() );

        rItemSet.Put( aLRItem );
    }

    // oberer und unterer Rand
    if( (rPropInfo.bTopMargin || rPropInfo.bBottomMargin) &&
        (!rPropInfo.bTopMargin || !rPropInfo.bBottomMargin) &&
        SFX_ITEM_SET == rCollItemSet.GetItemState(RES_UL_SPACE,sal_True,
                                                  &pCollItem) &&
        SFX_ITEM_SET == rItemSet.GetItemState(RES_UL_SPACE,sal_False,&pItem) )
    {
        const SvxULSpaceItem *pULItem = (const SvxULSpaceItem *)pItem;

        SvxULSpaceItem aULItem( *((const SvxULSpaceItem *)pCollItem) );
        if( rPropInfo.bTopMargin )
            aULItem.SetUpper( pULItem->GetUpper() );
        if( rPropInfo.bBottomMargin )
            aULItem.SetLower( pULItem->GetLower() );

        rItemSet.Put( aULItem );
    }

    static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONTSIZE,RES_CHRATR_CJK_FONTSIZE,
                                   RES_CHRATR_CTL_FONTSIZE };
       for( sal_uInt16 i=0; i<3; i++ )
    {
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i], sal_False,
                                                   &pItem ) &&
            ((const SvxFontHeightItem *)pItem)->GetProp() != 100)
        {
            // %-Angaben beim FontHeight-Item werden nicht unterstuetzt
            rItemSet.ClearItem( aWhichIds[i] );
        }
    }

// Feature: PrintExt
    pCSS1Parser->SetFmtBreak( rItemSet, rPropInfo );
// /Feature: PrintExt

    pColl->SetFmtAttr( rItemSet );
}

void SwCSS1Parser::SetTableTxtColl( sal_Bool bHeader )
{
    OSL_ENSURE( !(bHeader ? bTableHeaderTxtCollSet : bTableTxtCollSet),
            "Aufruf von SetTableTxtColl unnoetig" );

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

    SwTxtFmtColl *pColl = 0;

    // The following entries will never be used again and may be changed.
    SvxCSS1MapEntry *pStyleEntry = GetTag( sTag );
    if( pStyleEntry )
    {
        pColl = GetTxtFmtColl( nPoolId, aEmptyStr );
        SetTxtCollAttrs( pColl, pStyleEntry->GetItemSet(),
                         pStyleEntry->GetPropertyInfo(), this );
    }

    OUString sTmp = sTag + " " + OOO_STRING_SVTOOLS_HTML_parabreak;
    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        if( !pColl )
            pColl = GetTxtFmtColl( nPoolId, aEmptyStr );
        SetTxtCollAttrs( pColl, pStyleEntry->GetItemSet(),
                         pStyleEntry->GetPropertyInfo(), this );
    }

    if( bHeader )
        bTableHeaderTxtCollSet = sal_True;
    else
        bTableTxtCollSet = sal_True;
}

void SwCSS1Parser::SetPageDescAttrs( const SvxBrushItem *pBrush,
                                     SfxItemSet *pItemSet2 )
{
    SvxBrushItem aBrushItem( RES_BACKGROUND );
    SvxBoxItem aBoxItem( RES_BOX );
    SvxFrameDirectionItem aFrmDirItem(FRMDIR_ENVIRONMENT, RES_FRAMEDIR);
    sal_Bool bSetBrush = pBrush!=0, bSetBox = sal_False, bSetFrmDir = sal_False;
    if( pBrush )
        aBrushItem = *pBrush;

    if( pItemSet2 )
    {
        const SfxPoolItem *pItem = 0;
        if( SFX_ITEM_SET == pItemSet2->GetItemState( RES_BACKGROUND, sal_False,
                                                   &pItem ) )
        {
            // ein Hintergrund wird gesetzt
            aBrushItem = *((const SvxBrushItem *)pItem);
            pItemSet2->ClearItem( RES_BACKGROUND );
            bSetBrush = sal_True;
        }

        if( SFX_ITEM_SET == pItemSet2->GetItemState( RES_BOX, sal_False, &pItem ) )
        {
            // eine Umrandung wird gesetzt
            aBoxItem = *((const SvxBoxItem *)pItem);
            pItemSet2->ClearItem( RES_BOX );
            bSetBox = sal_True;
        }

        if( SFX_ITEM_SET == pItemSet2->GetItemState( RES_BOX, sal_False, &pItem ) )
        {
            // eine Umrandung wird gesetzt
            aBoxItem = *((const SvxBoxItem *)pItem);
            pItemSet2->ClearItem( RES_BOX );
            bSetBox = sal_True;
        }

        if( SFX_ITEM_SET == pItemSet2->GetItemState( RES_FRAMEDIR, sal_False, &pItem ) )
        {
            // eine Umrandung wird gesetzt
            aFrmDirItem = *static_cast< const SvxFrameDirectionItem *>( pItem );
            pItemSet2->ClearItem( RES_FRAMEDIR );
            bSetFrmDir = sal_True;
        }
    }

    if( bSetBrush || bSetBox || bSetFrmDir )
    {
        static sal_uInt16 aPoolIds[] = { RES_POOLPAGE_HTML, RES_POOLPAGE_FIRST,
                                     RES_POOLPAGE_LEFT, RES_POOLPAGE_RIGHT };
        for( sal_uInt16 i=0; i<4; i++ )
        {
            const SwPageDesc *pPageDesc = GetPageDesc( aPoolIds[i], sal_False );
            if( pPageDesc )
            {
                SwPageDesc aNewPageDesc( *pPageDesc );
                SwFrmFmt &rMaster = aNewPageDesc.GetMaster();
                if( bSetBrush )
                    rMaster.SetFmtAttr( aBrushItem );
                if( bSetBox )
                    rMaster.SetFmtAttr( aBoxItem );
                if( bSetFrmDir )
                    rMaster.SetFmtAttr( aFrmDirItem );

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
    SwFrmFmt &rMaster = aNewPageDesc.GetMaster();
    const SfxItemSet& rPageItemSet = rMaster.GetAttrSet();
    const SfxPoolItem *pPageItem, *pItem;
    sal_Bool bChanged = sal_False;

    // linker, rechter Rand und Erstzeilen-Einzug
    if( (rPropInfo.bLeftMargin || rPropInfo.bRightMargin) &&
        SFX_ITEM_SET == rItemSet.GetItemState(RES_LR_SPACE,sal_False,&pItem) )
    {
        if( (!rPropInfo.bLeftMargin || !rPropInfo.bRightMargin) &&
            SFX_ITEM_SET == rPageItemSet.GetItemState(RES_LR_SPACE,
                                                      sal_True,&pPageItem) )
        {
            const SvxLRSpaceItem *pLRItem = (const SvxLRSpaceItem *)pItem;

            SvxLRSpaceItem aLRItem( *((const SvxLRSpaceItem *)pPageItem) );
            if( rPropInfo.bLeftMargin )
                aLRItem.SetLeft( pLRItem->GetLeft() );
            if( rPropInfo.bRightMargin )
                aLRItem.SetRight( pLRItem->GetRight() );

            rMaster.SetFmtAttr( aLRItem );
        }
        else
        {
            rMaster.SetFmtAttr( *pItem );
        }
        bChanged = sal_True;
    }

    // oberer und unterer Rand
    if( (rPropInfo.bTopMargin || rPropInfo.bBottomMargin) &&
        SFX_ITEM_SET == rItemSet.GetItemState(RES_UL_SPACE,sal_False,&pItem) )
    {
        if( (!rPropInfo.bTopMargin || !rPropInfo.bBottomMargin) &&
            SFX_ITEM_SET == rPageItemSet.GetItemState(RES_UL_SPACE,
                                                      sal_True,&pPageItem) )
        {
            const SvxULSpaceItem *pULItem = (const SvxULSpaceItem *)pItem;

            SvxULSpaceItem aULItem( *((const SvxULSpaceItem *)pPageItem) );
            if( rPropInfo.bTopMargin )
                aULItem.SetUpper( pULItem->GetUpper() );
            if( rPropInfo.bBottomMargin )
                aULItem.SetLower( pULItem->GetLower() );

            rMaster.SetFmtAttr( aULItem );
        }
        else
        {
            rMaster.SetFmtAttr( *pItem );
        }
        bChanged = sal_True;
    }

    // die Groesse
    if( rPropInfo.eSizeType != SVX_CSS1_STYPE_NONE )
    {
        if( rPropInfo.eSizeType == SVX_CSS1_STYPE_TWIP )
        {
            rMaster.SetFmtAttr( SwFmtFrmSize( ATT_FIX_SIZE, rPropInfo.nWidth,
                                           rPropInfo.nHeight ) );
            bChanged = sal_True;
        }
        else
        {
            // Bei "size: auto|portrait|landscape" bleibt die bisherige
            // Groesse der Vorlage erhalten. Bei "landscape" und "portrait"
            // wird das Landscape-Flag gesetzt und evtl. die Breite/Hoehe
            // vertauscht.
            SwFmtFrmSize aFrmSz( rMaster.GetFrmSize() );
            sal_Bool bLandscape = aNewPageDesc.GetLandscape();
            if( ( bLandscape &&
                  rPropInfo.eSizeType == SVX_CSS1_STYPE_PORTRAIT ) ||
                ( !bLandscape &&
                  rPropInfo.eSizeType == SVX_CSS1_STYPE_LANDSCAPE ) )
            {
                SwTwips nTmp = aFrmSz.GetHeight();
                aFrmSz.SetHeight( aFrmSz.GetWidth() );
                aFrmSz.SetWidth( nTmp );
                rMaster.SetFmtAttr( aFrmSz );
                aNewPageDesc.SetLandscape( !bLandscape );
                bChanged = sal_True;
            }
        }
    }

    // Geht das wirklich?
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, sal_False, &pItem ) )
    {
        // eine Umrandung wird gesetzt
        rMaster.SetFmtAttr( *pItem );
        rItemSet.ClearItem( RES_BACKGROUND );
        bChanged = sal_True;
    }

    if( bChanged )
        ChgPageDesc( pPageDesc, aNewPageDesc );
}
// /Feature: PrintExt

const SvxBrushItem& SwCSS1Parser::GetPageDescBackground() const
{
    return pDoc->GetPageDescFromPool( RES_POOLPAGE_HTML, false )
        ->GetMaster().GetBackground();
}

sal_uInt16 SwCSS1Parser::GetScriptFromClass( OUString& rClass,
                                      sal_Bool bSubClassOnly )
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
        if( rClass.matchIgnoreAsciiCaseAsciiL( "cjk", 3, nPos ) )
        {
            nScriptFlags = CSS1_SCRIPT_CJK;
        }
        else if( rClass.matchIgnoreAsciiCaseAsciiL( "ctl", 3, nPos ) )
        {
            nScriptFlags = CSS1_SCRIPT_CTL;
        }
        break;
    case 7:
        if( rClass.matchIgnoreAsciiCaseAsciiL( "western", 7, nPos ) )
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
            rClass = "";
        }
    }

    return nScriptFlags;
}

static CSS1SelectorType GetTokenAndClass( const CSS1Selector *pSelector,
                              OUString& rToken, OUString& rClass,
                              sal_uInt16& rScriptFlags )
{
    rToken = pSelector->GetString();
    rClass = "";
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

            rScriptFlags = SwCSS1Parser::GetScriptFromClass( rClass, sal_False );
            if( rClass.isEmpty() )
                eType = CSS1_SELTYPE_ELEMENT;
        }
    }

    rToken = rToken.toAsciiUpperCase();
    return eType;
}

extern bool swhtml_css1atr_equalFontItems( const SfxPoolItem& r1, const SfxPoolItem& r2 );

static void RemoveScriptItems( SfxItemSet& rItemSet, sal_uInt16 nScript,
                               const SfxItemSet *pParentItemSet = 0 )
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

    sal_uInt16 aClearItems[3] = { sal_False, sal_False, sal_False };
    switch( nScript )
    {
    case CSS1_SCRIPT_WESTERN:
        aClearItems[1] = aClearItems[2] =  sal_True;
        break;
    case CSS1_SCRIPT_CJK:
        aClearItems[0] = aClearItems[2] =  sal_True;
        break;
    case CSS1_SCRIPT_CTL:
        aClearItems[0] = aClearItems[1] =  sal_True;
        break;
    case CSS1_SCRIPT_ALL:
        break;
    default:
        OSL_ENSURE( aClearItems[0], "unknown script type" );
        break;
       }

    for( sal_uInt16 j=0; j < 3; j++ )
    {
        for( sal_uInt16 i=0; i < 5; i++ )
        {
            sal_uInt16 nWhich = aWhichIds[j][i];
            const SfxPoolItem *pItem;
            if( aClearItems[j] ||
                (pParentItemSet &&
                 SFX_ITEM_SET == rItemSet.GetItemState( nWhich, sal_False, &pItem ) &&
                 (0==i ? swhtml_css1atr_equalFontItems( *pItem, pParentItemSet->Get(nWhich, sal_True ) )
                        : *pItem == pParentItemSet->Get(nWhich, sal_True ) ) ) )
            {
                rItemSet.ClearItem( nWhich );
            }
        }
    }
}

sal_Bool SwCSS1Parser::StyleParsed( const CSS1Selector *pSelector,
                                SfxItemSet& rItemSet,
                                SvxCSS1PropertyInfo& rPropInfo )
{
    if( !bIsNewDoc )
        return sal_True;

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
                        pNext != 0,
                        rItemSet, rPropInfo );
        }
    }

    if( CSS1_SELTYPE_ELEMENT != eSelType &&
        CSS1_SELTYPE_ELEM_CLASS != eSelType)
        return sal_True;

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
                return sal_False;
            }
            else if( pNext && CSS1_SELTYPE_PSEUDO == eNextType )
            {
                // vielleicht A:visited oder A:link

                OUString aPseudo( pNext->GetString() );
                aPseudo = aPseudo.toAsciiLowerCase();
                sal_Bool bInsert = sal_False;
                switch( aPseudo[0] )
                {
                    case 'l':
                        if( aPseudo == "link" )
                        {
                            bInsert = sal_True;
                        }
                        break;
                    case 'v':
                        if( aPseudo == "visited" )
                        {
                            bInsert = sal_True;
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
                    return sal_False;
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
                if( SFX_ITEM_SET==rItemSet.GetItemState(RES_BACKGROUND,sal_False,&pItem) )
                {
                    const SvxBrushItem *pBrushItem =
                        (const SvxBrushItem *)pItem;

                    /// Body has a background color, if it is not "no fill"/"auto fill"
                    if( pBrushItem->GetColor() != COL_TRANSPARENT )
                        bBodyBGColorSet = sal_True;
                    if( GPOS_NONE != pBrushItem->GetGraphicPos() )
                        bBodyBackgroundSet = sal_True;
                }

                // Border and Padding
                rPropInfo.SetBoxItem( rItemSet, MIN_BORDER_DIST );

                // Ein par Attribute muessen an der Seitenvorlage gesetzt werden,
                // und zwar die, die nicht vererbt werden
                SetPageDescAttrs( 0, &rItemSet );

                // alle noch uebrigen Optionen koennen an der Standard-Vorlage
                // gesetzt werden und gelten dann automatisch als defaults
                if( SFX_ITEM_SET==rItemSet.GetItemState(RES_CHRATR_COLOR,sal_False) )
                    bBodyTextSet = sal_True;
                SetTxtCollAttrs(
                    GetTxtCollFromPool( RES_POOLCOLL_STANDARD ),
                    rItemSet, rPropInfo, this );

                return sal_False;
            }
            break;
        }
    }
    else if( CSS1_SELTYPE_ELEM_CLASS==eSelType &&  HTML_ANCHOR_ON==nToken2 &&
             !pNext && aClass.getLength() >= 9 &&
             ('s' == aClass[0] || 'S' == aClass[0]) )
    {
        sal_uInt16 nPoolFmtId = 0;
        if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdendnote_sym) )
            nPoolFmtId = RES_POOLCHR_ENDNOTE;
        else if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdfootnote_sym) )
            nPoolFmtId = RES_POOLCHR_FOOTNOTE;
        if( nPoolFmtId )
        {
            if( CSS1_SCRIPT_ALL == nScript )
            {
                SetCharFmtAttrs( GetCharFmtFromPool(nPoolFmtId), rItemSet );
            }
            else
            {
                SfxItemSet aScriptItemSet( rItemSet );
                RemoveScriptItems( aScriptItemSet, nScript );
                SetCharFmtAttrs( GetCharFmtFromPool(nPoolFmtId),
                                 aScriptItemSet);
            }
            return sal_False;
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
                aClass = aEmptyStr;
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
            return sal_False;
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
                    OUString sTmp = aToken2 + " " + OOO_STRING_SVTOOLS_HTML_parabreak;

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

                    return sal_False;
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
            SwTxtFmtColl *pColl = GetTxtFmtColl( nPoolCollId, aEmptyStr );
            SwTxtFmtColl* pParentColl = 0;
            if( !aClass.isEmpty() )
            {
                OUString aName( pColl->GetName() );
                AddClassName( aName, aClass );

                pParentColl = pColl;
                pColl = pDoc->FindTxtFmtCollByName( aName );
                if( !pColl )
                    pColl = pDoc->MakeTxtFmtColl( aName, pParentColl );
            }
            if( !pNext )
            {
                // nur die Attribute an der Vorlage setzen
                const SfxPoolItem *pItem;
                const SvxBoxItem *pBoxItem = 0;
                if( SFX_ITEM_SET ==
                        pColl->GetAttrSet().GetItemState(RES_BOX,sal_True,&pItem) )
                    pBoxItem = (const SvxBoxItem *)pItem;
                rPropInfo.SetBoxItem( rItemSet, MIN_BORDER_DIST, pBoxItem );
                if( CSS1_SCRIPT_ALL == nScript && !pParentColl )
                {
                    SetTxtCollAttrs( pColl, rItemSet, rPropInfo, this );
                }
                else
                {
                    SfxItemSet aScriptItemSet( rItemSet );
                    RemoveScriptItems( aScriptItemSet, nScript,
                                       pParentColl ? &pParentColl->GetAttrSet() : 0 );
                    SetTxtCollAttrs( pColl, aScriptItemSet, rPropInfo, this );
                }
            }
            else
            {
                // ein Drop-Cap-Attribut basteln
                SwFmtDrop aDrop( pColl->GetDrop() );
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
                    pColl->SetFmtAttr( aDrop );
                }
            }

            return sal_False;
        }

        return sal_True;
    }

    // Jetzt werden die Selektoten verarbeitet, die zu einer Zechenvorlage
    // gehoehren. Zusammengesetzte gibt es hier allerdings nich nicht.
    if( pNext )
        return sal_True;

    SwCharFmt *pCFmt = GetChrFmt( static_cast< sal_uInt16 >(nToken2), aEmptyStr );
    if( pCFmt )
    {
        SwCharFmt *pParentCFmt = 0;
        if( !aClass.isEmpty() )
        {
            OUString aName( pCFmt->GetName() );
            AddClassName( aName, aClass );
            pParentCFmt = pCFmt;

            pCFmt = pDoc->FindCharFmtByName( aName );
            if( !pCFmt )
            {
                pCFmt = pDoc->MakeCharFmt( aName, pParentCFmt );
                pCFmt->SetAuto( false );
            }
        }

        if( CSS1_SCRIPT_ALL == nScript && !pParentCFmt )
        {
            SetCharFmtAttrs( pCFmt, rItemSet );
        }
        else
        {
            SfxItemSet aScriptItemSet( rItemSet );
            RemoveScriptItems( aScriptItemSet, nScript,
                               pParentCFmt ? &pParentCFmt->GetAttrSet() : 0 );
            SetCharFmtAttrs( pCFmt, aScriptItemSet );
        }
        return sal_False;
    }

    return sal_True;
}

sal_uInt32 SwCSS1Parser::GetFontHeight( sal_uInt16 nSize ) const
{
    return aFontHeights[ nSize>6 ? 6 : nSize ];
}

const FontList *SwCSS1Parser::GetFontList() const
{
    const FontList *pFList = 0;
    SwDocShell *pDocSh = pDoc->GetDocShell();
    if( pDocSh )
    {
        const SvxFontListItem *pFListItem =
            (const SvxFontListItem *)pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST);
        if( pFListItem )
            pFList = pFListItem->GetFontList();
    }

    return pFList;
}


SwCharFmt* SwCSS1Parser::GetChrFmt( sal_uInt16 nToken2, const OUString& rClass ) const
{
    // die entsprechende Vorlage suchen
    sal_uInt16 nPoolId = 0;
    const sal_Char* sName = 0;
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
        return 0;

    // Die Vorlage (ohne Class) suchen oder anlegen
    SwCharFmt *pCFmt = 0;
    if( nPoolId )
    {
        pCFmt = GetCharFmtFromPool( nPoolId );
    }
    else
    {
        OUString sCName( OUString::createFromAscii(sName) );
        pCFmt = pDoc->FindCharFmtByName( sCName );
        if( !pCFmt )
        {
            pCFmt = pDoc->MakeCharFmt( sCName, pDoc->GetDfltCharFmt() );
            pCFmt->SetAuto( false );
        }
    }

    OSL_ENSURE( pCFmt, "Keine Zeichen-Vorlage???" );

    // Wenn es eine Klasse gibt, die Klassen-Vorlage suchen aber nicht
    // neu anlegen.
    OUString aClass( rClass );
    GetScriptFromClass( aClass, sal_False );
    if( !aClass.isEmpty() )
    {
        OUString aTmp( pCFmt->GetName() );
        AddClassName( aTmp, aClass );
        SwCharFmt *pClassCFmt = pDoc->FindCharFmtByName( aTmp );
        if( pClassCFmt )
        {
            pCFmt = pClassCFmt;
        }
        else
        {
            const SvxCSS1MapEntry *pClass = GetClass( aClass );
            if( pClass )
            {
                pCFmt = pDoc->MakeCharFmt( aTmp, pCFmt );
                pCFmt->SetAuto( false );
                SfxItemSet aItemSet( pClass->GetItemSet() );
                SetCharFmtAttrs( pCFmt, aItemSet );
            }
        }
    }

    return pCFmt;
}



SwTxtFmtColl *SwCSS1Parser::GetTxtCollFromPool( sal_uInt16 nPoolId ) const
{
    sal_uInt16 nOldArrLen = pDoc->GetTxtFmtColls()->size();

    SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool( nPoolId, false );

    if( bIsNewDoc )
    {
        sal_uInt16 nArrLen = pDoc->GetTxtFmtColls()->size();
        for( sal_uInt16 i=nOldArrLen; i<nArrLen; i++ )
            lcl_swcss1_setEncoding( *(*pDoc->GetTxtFmtColls())[i],
                                    GetDfltEncoding() );
    }

    return pColl;
}

SwCharFmt *SwCSS1Parser::GetCharFmtFromPool( sal_uInt16 nPoolId ) const
{
    sal_uInt16 nOldArrLen = pDoc->GetCharFmts()->size();

    SwCharFmt *pCharFmt = pDoc->GetCharFmtFromPool( nPoolId );

    if( bIsNewDoc )
    {
        sal_uInt16 nArrLen = pDoc->GetCharFmts()->size();

        for( sal_uInt16 i=nOldArrLen; i<nArrLen; i++ )
            lcl_swcss1_setEncoding( *(*pDoc->GetCharFmts())[i],
                                    GetDfltEncoding() );
    }

    return pCharFmt;
}

SwTxtFmtColl *SwCSS1Parser::GetTxtFmtColl( sal_uInt16 nTxtColl,
                                           const OUString& rClass )
{
    SwTxtFmtColl* pColl = 0;

    OUString aClass( rClass );
    GetScriptFromClass( aClass, sal_False );
    if( RES_POOLCOLL_TEXT == nTxtColl && aClass.getLength() >= 9 &&
        ('s' == aClass[0] || 'S' == aClass[0] ) )
    {
        if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdendnote) )
        {
            nTxtColl = RES_POOLCOLL_ENDNOTE;
            aClass = aEmptyStr;
        }
        else if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdfootnote) )
        {
            nTxtColl = RES_POOLCOLL_FOOTNOTE;
            aClass = aEmptyStr;
        }
    }

    if( USER_FMT & nTxtColl )       // eine vom Reader angelegte
    {
        OSL_ENSURE( !this, "Wo kommt die Benutzer-Vorlage her?" );
        pColl = GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
    }
    else
    {
        pColl = GetTxtCollFromPool( nTxtColl );
    }

    OSL_ENSURE( pColl, "Keine Absatz-Vorlage???" );
    if( !aClass.isEmpty() )
    {
        OUString aTmp( pColl->GetName() );
        AddClassName( aTmp, aClass );
        SwTxtFmtColl* pClassColl = pDoc->FindTxtFmtCollByName( aTmp );

        if( !pClassColl &&
            (nTxtColl==RES_POOLCOLL_TABLE ||
             nTxtColl==RES_POOLCOLL_TABLE_HDLN) )
        {
            // Wenn dieser Fall eintritt, dann wurde ein <TD><P CLASS=foo>
            // gelesen, aber die TD.foo Vorlage nicht gefunden. Dann muessen
            // wir P.foo nehmen, wenn es sie gibt.
            SwTxtFmtColl* pCollText =
                GetTxtCollFromPool( RES_POOLCOLL_TEXT );
            aTmp = pCollText->GetName();
            AddClassName( aTmp, aClass );
            pClassColl = pDoc->FindTxtFmtCollByName( aTmp );
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
                pColl = pDoc->MakeTxtFmtColl( aTmp, pColl );
                SfxItemSet aItemSet( pClass->GetItemSet() );
                SvxCSS1PropertyInfo aPropInfo( pClass->GetPropertyInfo() );
                aPropInfo.SetBoxItem( aItemSet, MIN_BORDER_DIST );
                sal_Bool bPositioned = MayBePositioned( pClass->GetPropertyInfo() );
                if( bPositioned )
                    aItemSet.ClearItem( RES_BACKGROUND );
                SetTxtCollAttrs( pColl, aItemSet, aPropInfo,
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
    return pDoc->GetPageDescFromPool( RES_POOLPAGE_HTML, false );
}

static SwPageDesc *FindPageDesc( SwDoc *pDoc, sal_uInt16 nPoolId, sal_uInt16& rPage )
{
    sal_uInt16 nPageDescs = pDoc->GetPageDescCnt();
    for( rPage=0; rPage < nPageDescs &&
         pDoc->GetPageDesc(rPage).GetPoolFmtId() != nPoolId; rPage++ )
         ;

    return rPage < nPageDescs ? &pDoc->GetPageDesc( rPage ) : 0;
}

const SwPageDesc *SwCSS1Parser::GetPageDesc( sal_uInt16 nPoolId, sal_Bool bCreate )
{
    if( RES_POOLPAGE_HTML == nPoolId )
        return pDoc->GetPageDescFromPool( RES_POOLPAGE_HTML, false );

    sal_uInt16 nPage;
    const SwPageDesc *pPageDesc = FindPageDesc( pDoc, nPoolId, nPage );
    if( !pPageDesc && bCreate )
    {
        // Die erste Seite wird aus der rechten Seite erzeugt, wenn es die
        // gibt.
        SwPageDesc *pMasterPageDesc = 0;
        if( RES_POOLPAGE_FIRST == nPoolId )
            pMasterPageDesc = FindPageDesc( pDoc, RES_POOLPAGE_RIGHT, nPage );
        if( !pMasterPageDesc )
            pMasterPageDesc = pDoc->GetPageDescFromPool( RES_POOLPAGE_HTML, false );

        // Die neue Seitenvorlage entsteht aus dem Master durch kopieren.
        SwPageDesc *pNewPageDesc = pDoc->
            GetPageDescFromPool( nPoolId, false );

        // dazu brauchen wir auch die Nummer der neuen Vorlage
        pPageDesc = FindPageDesc( pDoc, nPoolId, nPage );
        OSL_ENSURE( pPageDesc==pNewPageDesc, "Seitenvorlage nicht gefunden" );

        pDoc->CopyPageDesc( *pMasterPageDesc, *pNewPageDesc, false );

        // Die Vorlagen an ihren neuen Zweck anpassen.
        const SwPageDesc *pFollow = 0;
        sal_Bool bSetFollowFollow = sal_False;
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
            GetLeftPageDesc( sal_True );
            break;

        case RES_POOLPAGE_LEFT:
            // Die rechte Vorlage wird angelegt, wenn sie noch nicht existiert.
            // Es findet aber keine Verkettung statt.
            // Wenn schon eine erste Seitenvorlage existiert, wird die linke
            // Vorlage die Folge-Vorlage der ersten Seite.
            pFollow = GetRightPageDesc( sal_True );
            bSetFollowFollow = sal_True;
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


sal_Bool SwCSS1Parser::MayBePositioned( const SvxCSS1PropertyInfo& rPropInfo,
                                    sal_Bool bAutoWidth )
{
    // abs-pos
    // left/top none    auto    twip    perc
    //
    // none     Z       Z       -       -
    // auto     Z       Z       -       -
    // twip     Z       Z       S/R     -
    // perc     -       -       -       -
    //
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



void SwCSS1Parser::AddClassName( OUString& rFmtName, const OUString& rClass )
{
    OSL_ENSURE( !rClass.isEmpty(), "Style-Klasse ohne Laenge?" );

    rFmtName += "." + rClass;
}


void SwCSS1Parser::FillDropCap( SwFmtDrop& rDrop,
                                SfxItemSet& rItemSet,
                                const OUString *pName )
{
    // die Anzahl der Zeilen entspricht in etwa einer %-Angabe
    // fuer die Hoehe (was passiert mit absoluten Hoehen???)
    sal_uInt8 nLines = rDrop.GetLines();
    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_CHRATR_FONTSIZE, sal_False, &pItem ) )
    {
        sal_uInt16 nProp = ((const SvxFontHeightItem *)pItem)->GetProp();
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
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_LR_SPACE, sal_False, &pItem ) )
    {
        rDrop.GetDistance() = static_cast< sal_uInt16 >(
            ((const SvxLRSpaceItem *)pItem)->GetRight() );
        rItemSet.ClearItem( RES_LR_SPACE );
    }

    // Fuer alle anderen Attribute eine Zeichen-Vorlage anlegen
    if( rItemSet.Count() )
    {
        SwCharFmt *pCFmt = 0;
        OUString aName;
        if( pName )
        {
            aName = *pName;
            AddFirstLetterExt( aName );
            pCFmt = pDoc->FindCharFmtByName( aName );
        }
        else
        {
            do
            {
                aName = OUString( "first-letter" ) + " " + OUString::number( (sal_Int32)(++nDropCapCnt) );
            }
            while( pDoc->FindCharFmtByName(aName) );
        }

        if( !pCFmt )
        {
            pCFmt = pDoc->MakeCharFmt( aName, pDoc->GetDfltCharFmt() );
            pCFmt->SetAuto( false );
        }
        SetCharFmtAttrs( pCFmt, rItemSet );

        // Die Zeichenvorlage braucht nur im Attribut gesetzt werden, wenn
        // auch das Attribut gesetzt wird.
        if( nLines > 1 )
            rDrop.SetCharFmt( pCFmt );
    }
}


// CSS1-sezifisches des SwHTMLParsers

_HTMLAttr **SwHTMLParser::GetAttrTabEntry( sal_uInt16 nWhich )
{
    // den zu dem Item gehoehrenden Tabellen-Eintrag ermitteln ...
    _HTMLAttr **ppAttr = 0;
    switch( nWhich )
    {
    case RES_CHRATR_BLINK:
        ppAttr = &aAttrTab.pBlink;
        break;
    case RES_CHRATR_CASEMAP:
        ppAttr = &aAttrTab.pCaseMap;
        break;
    case RES_CHRATR_COLOR:
        ppAttr = &aAttrTab.pFontColor;
        break;
    case RES_CHRATR_CROSSEDOUT:
        ppAttr = &aAttrTab.pStrike;
        break;
    case RES_CHRATR_ESCAPEMENT:
        ppAttr = &aAttrTab.pEscapement;
        break;
    case RES_CHRATR_FONT:
        ppAttr = &aAttrTab.pFont;
        break;
    case RES_CHRATR_CJK_FONT:
        ppAttr = &aAttrTab.pFontCJK;
        break;
    case RES_CHRATR_CTL_FONT:
        ppAttr = &aAttrTab.pFontCTL;
        break;
    case RES_CHRATR_FONTSIZE:
        ppAttr = &aAttrTab.pFontHeight;
        break;
    case RES_CHRATR_CJK_FONTSIZE:
        ppAttr = &aAttrTab.pFontHeightCJK;
        break;
    case RES_CHRATR_CTL_FONTSIZE:
        ppAttr = &aAttrTab.pFontHeightCTL;
        break;
    case RES_CHRATR_KERNING:
        ppAttr = &aAttrTab.pKerning;
        break;
    case RES_CHRATR_POSTURE:
        ppAttr = &aAttrTab.pItalic;
        break;
    case RES_CHRATR_CJK_POSTURE:
        ppAttr = &aAttrTab.pItalicCJK;
        break;
    case RES_CHRATR_CTL_POSTURE:
        ppAttr = &aAttrTab.pItalicCTL;
        break;
    case RES_CHRATR_UNDERLINE:
        ppAttr = &aAttrTab.pUnderline;
        break;
    case RES_CHRATR_WEIGHT:
        ppAttr = &aAttrTab.pBold;
        break;
    case RES_CHRATR_CJK_WEIGHT:
        ppAttr = &aAttrTab.pBoldCJK;
        break;
    case RES_CHRATR_CTL_WEIGHT:
        ppAttr = &aAttrTab.pBoldCTL;
        break;
    case RES_CHRATR_BACKGROUND:
        ppAttr = &aAttrTab.pCharBrush;
        break;

    case RES_PARATR_LINESPACING:
        ppAttr = &aAttrTab.pLineSpacing;
        break;
    case RES_PARATR_ADJUST:
        ppAttr = &aAttrTab.pAdjust;
        break;

    case RES_LR_SPACE:
        ppAttr = &aAttrTab.pLRSpace;
        break;
    case RES_UL_SPACE:
        ppAttr = &aAttrTab.pULSpace;
        break;
    case RES_BOX:
        ppAttr = &aAttrTab.pBox;
        break;
    case RES_BACKGROUND:
        ppAttr = &aAttrTab.pBrush;
        break;
    case RES_BREAK:
        ppAttr = &aAttrTab.pBreak;
        break;
    case RES_PAGEDESC:
        ppAttr = &aAttrTab.pPageDesc;
        break;
    case RES_PARATR_SPLIT:
        ppAttr = &aAttrTab.pSplit;
        break;
    case RES_PARATR_WIDOWS:
        ppAttr = &aAttrTab.pWidows;
        break;
    case RES_PARATR_ORPHANS:
        ppAttr = &aAttrTab.pOrphans;
        break;
    case RES_KEEP:
        ppAttr = &aAttrTab.pKeep;
        break;

    case RES_CHRATR_LANGUAGE:
        ppAttr = &aAttrTab.pLanguage;
        break;
    case RES_CHRATR_CJK_LANGUAGE:
        ppAttr = &aAttrTab.pLanguageCJK;
        break;
    case RES_CHRATR_CTL_LANGUAGE:
        ppAttr = &aAttrTab.pLanguageCTL;
        break;

    case RES_FRAMEDIR:
        ppAttr = &aAttrTab.pDirection;
        break;
    }

    return ppAttr;
}

void SwHTMLParser::NewStyle()
{
    String sType;

    const HTMLOptions& rOptions2 = GetOptions();
    for (size_t i = rOptions2.size(); i; )
    {
        const HTMLOption& rOption = rOptions2[--i];
        if( HTML_O_TYPE == rOption.GetToken() )
            sType = rOption.GetString();
    }

    bIgnoreRawData = sType.Len() &&
                     !sType.GetToken(0,';').EqualsAscii(sCSS_mimetype);
}

void SwHTMLParser::EndStyle()
{
    bIgnoreRawData = sal_False;

    if( aStyleSource.Len() )
    {
        pCSS1Parser->ParseStyleSheet( aStyleSource );
        aStyleSource.Erase();
    }
}

sal_Bool SwHTMLParser::FileDownload( const String& rURL,
                                 OUString& rStr )
{
    // View wegschmeissen (wegen Reschedule)
    ViewShell *pOldVSh = CallEndAction();

    // Ein Medium anlegen
    SfxMedium aDLMedium( rURL, STREAM_READ | STREAM_SHARE_DENYWRITE );

    SvStream* pStream = aDLMedium.GetInStream();
    if( pStream )
    {
        SvMemoryStream aStream;
        aStream << *pStream;

        aStream.Seek( STREAM_SEEK_TO_END );
        OSL_ENSURE( aStream.Tell() < STRING_MAXLEN,
                    "File zu lang fuer einen String, Ende abgeschnitten" );
        xub_StrLen nLen = aStream.Tell() < STRING_MAXLEN
                        ? (xub_StrLen)aStream.Tell()
                        : STRING_MAXLEN;

        rStr = OUString( (const sal_Char *)aStream.GetData(), nLen
                            , GetSrcEncoding() );
    }


    // wurde abgebrochen?
    if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == pDoc->getReferenceCount() )
    {
        // wurde der Import vom SFX abgebrochen?
        eState = SVPAR_ERROR;
        pStream = 0;
    }

    // recreate View
    ViewShell *const pVSh = CallStartAction( pOldVSh );
    OSL_ENSURE( pOldVSh == pVSh, "FileDownload: ViewShell changed on us" );
    (void) pVSh;

    return pStream!=0;
}

void SwHTMLParser::InsertLink()
{
    sal_Bool bFinishDownload = sal_False;
    if( pPendStack )
    {
        OSL_ENSURE( ShouldFinishFileDownload(),
                "Pending-Stack ohne File-Download?" );

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        OSL_ENSURE( !pPendStack, "Wo kommt der Pending-Stack her?" );

        bFinishDownload = sal_True;
    }
    else
    {
        String sRel, sHRef, sType;

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
                    sHRef = URIHelper::SmartRel2Abs( INetURLObject( sBaseURL ), rOption.GetString(), Link(), false );
                    break;
                case HTML_O_TYPE:
                    sType = rOption.GetString();
                    break;
            }
        }

        if( sHRef.Len() && sRel.EqualsIgnoreCaseAscii( "STYLESHEET" ) &&
            ( !sType.Len() ||
              sType.GetToken(0,';').EqualsAscii(sCSS_mimetype) ) )
        {
            if( GetMedium() )
            {
                // Download des Style-Source starten
                StartFileDownload(sHRef);
                if( IsParserWorking() )
                {
                    // Der Style wurde synchron geladen und wir koennen
                    // es direkt aufrufen.
                    bFinishDownload = sal_True;
                }
                else
                {
                    // Der Style wird asynchron geladen und ist erst beim
                    // naechsten Continue-Aufruf da. Wir muessen deshalb einen
                    // Pending-Stack anlegen, damit wir hierher zurueckkehren
                    pPendStack = new SwPendingStack( HTML_LINK, pPendStack );
                }
            }
            else
            {
                // File synchron holen
                OUString sSource;
                if( FileDownload( sHRef, sSource ) )
                    pCSS1Parser->ParseStyleSheet( sSource );
            }
        }
    }

    if( bFinishDownload )
    {
        OUString sSource;
        if( FinishFileDownload( sSource ) && !sSource.isEmpty() )
            pCSS1Parser->ParseStyleSheet( sSource );
    }
}

sal_Bool SwCSS1Parser::ParseStyleSheet( const OUString& rIn )
{
    if( !SvxCSS1Parser::ParseStyleSheet( rIn ) )
        return sal_False;

    SwPageDesc *pMasterPageDesc =
        pDoc->GetPageDescFromPool( RES_POOLPAGE_HTML, false );

    SvxCSS1MapEntry *pPageEntry = GetPage( aEmptyStr, false );
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

    pPageEntry = GetPage( "first", sal_True );
    if( pPageEntry )
    {
        SetPageDescAttrs( GetFirstPageDesc(sal_True), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        bSetFirstPageDesc = sal_True;
    }

    pPageEntry = GetPage( "right", sal_True );
    if( pPageEntry )
    {
        SetPageDescAttrs( GetRightPageDesc(sal_True), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        bSetRightPageDesc = sal_True;
    }

    pPageEntry = GetPage( "left", sal_True );
    if( pPageEntry )
        SetPageDescAttrs( GetLeftPageDesc(sal_True), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );

    return sal_True;
}

sal_Bool SwHTMLParser::ParseStyleOptions( const String &rStyle,
                                      const String &rId,
                                      const String &rClass,
                                      SfxItemSet &rItemSet,
                                      SvxCSS1PropertyInfo &rPropInfo,
                                         const String *pLang,
                                         const String *pDir )
{
    sal_Bool bRet = sal_False;

    if( rClass.Len() )
    {
        OUString aClass( rClass );
        SwCSS1Parser::GetScriptFromClass( aClass );
        const SvxCSS1MapEntry *pClass = pCSS1Parser->GetClass( aClass );
        if( pClass )
        {
            pCSS1Parser->MergeStyles( pClass->GetItemSet(),
                                      pClass->GetPropertyInfo(),
                                      rItemSet, rPropInfo, sal_False );
            bRet = sal_True;
        }
    }

    if( rId.Len() )
    {
        const SvxCSS1MapEntry *pId = pCSS1Parser->GetId( rId );
        if( pId )
            pCSS1Parser->MergeStyles( pId->GetItemSet(),
                                      pId->GetPropertyInfo(),
                                      rItemSet, rPropInfo, rClass.Len()!=0 );
        rPropInfo.aId = rId;
        bRet = sal_True;
    }

    if( rStyle.Len() )
    {
        pCSS1Parser->ParseStyleOption( rStyle, rItemSet, rPropInfo );
        bRet = sal_True;
    }

    if( bRet )
        rPropInfo.SetBoxItem( rItemSet, MIN_BORDER_DIST );

    if( pLang && pLang->Len() )
    {
        LanguageType eLang = LanguageTag::convertToLanguageType( *pLang );
        if( LANGUAGE_DONTKNOW != eLang )
        {
            SvxLanguageItem aLang( eLang, RES_CHRATR_LANGUAGE );
            rItemSet.Put( aLang );
            aLang.SetWhich( RES_CHRATR_CJK_LANGUAGE );
            rItemSet.Put( aLang );
            aLang.SetWhich( RES_CHRATR_CTL_LANGUAGE );
            rItemSet.Put( aLang );

            bRet = sal_True;
        }
    }
    if( pDir && pDir->Len() )
    {
        String aValue( *pDir );
        aValue.ToUpperAscii();
        SvxFrameDirection eDir = FRMDIR_ENVIRONMENT;
        if( aValue.EqualsAscii( "LTR" ) )
            eDir = FRMDIR_HORI_LEFT_TOP;
        else if( aValue.EqualsAscii( "RTL" ) )
            eDir = FRMDIR_HORI_RIGHT_TOP;

        if( FRMDIR_ENVIRONMENT != eDir )
        {
            SvxFrameDirectionItem aDir( eDir, RES_FRAMEDIR );
            rItemSet.Put( aDir );

            bRet = sal_True;
        }
    }

    return bRet;
}

void SwHTMLParser::SetAnchorAndAdjustment( const SfxItemSet & /*rItemSet*/,
                                           const SvxCSS1PropertyInfo &rPropInfo,
                                           SfxItemSet &rFrmItemSet )
{
    SwFmtAnchor aAnchor;

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
                pPam->GetPoint()->nNode.GetNode().FindFlyStartNode();
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
            aAnchor.SetAnchor( pPam->GetPoint() );
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
        xub_StrLen nCntnt = pPam->GetPoint()->nContent.GetIndex();
        if( nCntnt )
        {
            aAnchor.SetType( FLY_AT_CHAR );
            pPam->Move( fnMoveBackward );
            eVertOri = text::VertOrientation::CHAR_BOTTOM;
            eVertRel = text::RelOrientation::CHAR;
        }
        else
        {
            aAnchor.SetType( FLY_AT_PARA );
            eVertOri = text::VertOrientation::TOP;
            eVertRel = text::RelOrientation::PRINT_AREA;
        }

        aAnchor.SetAnchor( pPam->GetPoint() );

        if( nCntnt )
            pPam->Move( fnMoveForward );

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
    rFrmItemSet.Put( aAnchor );

    // Absolut Positioniert mit Durchlauf
    rFrmItemSet.Put( SwFmtHoriOrient( nHoriPos, eHoriOri, eHoriRel ) );
    rFrmItemSet.Put( SwFmtVertOrient( nVertPos, eVertOri, eVertRel ) );
    rFrmItemSet.Put( SwFmtSurround( eSurround ) );
}

void SwHTMLParser::SetVarSize( SfxItemSet & /*rItemSet*/,
                               SvxCSS1PropertyInfo &rPropInfo,
                               SfxItemSet &rFrmItemSet,
                               SwTwips nDfltWidth, sal_uInt8 nDfltPrcWidth )
{
    SwFrmSize eSize = ATT_MIN_SIZE;
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

    SwFmtFrmSize aFrmSize( eSize, nWidth, nHeight );
    aFrmSize.SetWidthPercent( nPrcWidth );
    aFrmSize.SetHeightPercent( nPrcHeight );
    rFrmItemSet.Put( aFrmSize );
}

void SwHTMLParser::SetFrmFmtAttrs( SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo & /*rPropInfo*/,
                                   sal_uInt16 nFlags,
                                   SfxItemSet &rFrmItemSet )
{
    const SfxPoolItem *pItem;
    if( (nFlags & HTML_FF_BOX) != 0 &&
        SFX_ITEM_SET==rItemSet.GetItemState( RES_BOX, sal_True, &pItem ) )
    {
        if( (nFlags & HTML_FF_PADDING) == 0 )
        {
            SvxBoxItem aBoxItem( *(const SvxBoxItem *)pItem );
            // Alle 4 Seiten gleichzeitig auf 0 setzen
            aBoxItem.SetDistance( 0 );
            rFrmItemSet.Put( aBoxItem );
        }
        else
        {
            rFrmItemSet.Put( *pItem );
        }
        rItemSet.ClearItem( RES_BOX );
    }

    if( (nFlags & HTML_FF_BACKGROUND) != 0 &&
        SFX_ITEM_SET==rItemSet.GetItemState( RES_BACKGROUND, sal_True, &pItem ) )
    {
        rFrmItemSet.Put( *pItem );
        rItemSet.ClearItem( RES_BACKGROUND );
    }

    if( (nFlags & HTML_FF_DIRECTION) != 0 &&
        SFX_ITEM_SET==rItemSet.GetItemState( RES_FRAMEDIR, sal_True, &pItem ) )
    {
        rFrmItemSet.Put( *pItem );
        rItemSet.ClearItem( RES_FRAMEDIR );
    }
}



_HTMLAttrContext *SwHTMLParser::PopContext( sal_uInt16 nToken, sal_uInt16 nLimit,
                                            sal_Bool bRemove )
{
    sal_uInt16 nPos = aContexts.size();
    if( nPos <= nContextStMin )
        return 0;

    sal_Bool bFound = 0==nToken;
    if( nToken )
    {
        // Stack-Eintrag zu dem Token suchen
        while( nPos > nContextStMin )
        {
            sal_uInt16 nCntxtToken = aContexts[--nPos]->GetToken();
            if( nCntxtToken == nToken )
            {
                bFound = sal_True;
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

    _HTMLAttrContext *pCntxt = 0;
    if( bFound )
    {
        pCntxt = aContexts[nPos];
        if( bRemove )
            aContexts.erase( aContexts.begin() + nPos );
    }

    return pCntxt;
}

sal_Bool SwHTMLParser::GetMarginsFromContext( sal_uInt16& nLeft,
                                          sal_uInt16& nRight,
                                          short& nIndent,
                                          sal_Bool bIgnoreTopContext ) const
{
    sal_uInt16 nPos = aContexts.size();
    if( bIgnoreTopContext )
    {
        if( !nPos )
            return sal_False;
        else
            nPos--;
    }

    while( nPos > nContextStAttrMin )
    {
        const _HTMLAttrContext *pCntxt = aContexts[--nPos];
        if( pCntxt->IsLRSpaceChanged() )
        {
            pCntxt->GetMargins( nLeft, nRight, nIndent );
            return sal_True;
        }
    }

    return sal_False;
}

sal_Bool SwHTMLParser::GetMarginsFromContextWithNumBul( sal_uInt16& nLeft,
                                                    sal_uInt16& nRight,
                                                    short& nIndent ) const
{
    sal_Bool bRet = GetMarginsFromContext( nLeft, nRight, nIndent );
    const SwHTMLNumRuleInfo& rInfo = ((SwHTMLParser*)this)->GetNumInfo();
    if( rInfo.GetDepth() )
    {
        sal_uInt8 nLevel = (sal_uInt8)( (rInfo.GetDepth() <= MAXLEVEL ? rInfo.GetDepth()
                                                            : MAXLEVEL) - 1 );
        const SwNumFmt& rNumFmt = rInfo.GetNumRule()->Get(nLevel);
        nLeft = nLeft + rNumFmt.GetAbsLSpace();
        nIndent = rNumFmt.GetFirstLineOffset();
    }

    return bRet;
}

void SwHTMLParser::GetULSpaceFromContext( sal_uInt16& nUpper,
                                          sal_uInt16& nLower ) const
{
    sal_uInt16 nDfltColl = 0;
    OUString aDfltClass;

    sal_uInt16 nPos = aContexts.size();
    while( nPos > nContextStAttrMin )
    {
        const _HTMLAttrContext *pCntxt = aContexts[--nPos];
        if( pCntxt->IsULSpaceChanged() )
        {
            pCntxt->GetULSpace( nUpper, nLower );
            return;
        }
        else if( !nDfltColl )
        {
            nDfltColl = pCntxt->GetDfltTxtFmtColl();
            if( nDfltColl )
                aDfltClass = pCntxt->GetClass();
        }
    }

    if( !nDfltColl )
        nDfltColl = RES_POOLCOLL_TEXT;

    const SwTxtFmtColl *pColl =
        pCSS1Parser->GetTxtFmtColl( nDfltColl, aDfltClass );
    const SvxULSpaceItem& rULSpace = pColl->GetULSpace();
    nUpper = rULSpace.GetUpper();
    nLower = rULSpace.GetLower();
}

void SwHTMLParser::EndContextAttrs( _HTMLAttrContext *pContext, sal_Bool bRemove )
{
    _HTMLAttrs &rAttrs = pContext->GetAttrs();
    for( sal_uInt16 i=0; i<rAttrs.size(); i++ )
    {
        _HTMLAttr *pAttr = rAttrs[i];

        if( RES_PARATR_DROP==pAttr->GetItem().Which() )
        {
            // Fuer DropCaps noch die Anzahl der Zeichen anpassen. Wenn
            // es am Ende 0 sind, wird das Attribut invalidiert und dann
            // von _SetAttr gar nicht erst gesetzt.
            xub_StrLen nChars = pPam->GetPoint()->nContent.GetIndex();
            if( nChars < 1 )
                pAttr->Invalidate();
            else if( nChars > MAX_DROPCAP_CHARS )
                nChars = MAX_DROPCAP_CHARS;
            ((SwFmtDrop&)pAttr->GetItem()).GetChars() = (sal_uInt8)nChars;
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
            aParaAttrs.push_back( *ppAttr );
            EndAttr( *ppAttr, 0, sal_False );
        }

        pItem = aIter.NextItem();
    }
}

static void lcl_swcss1_setEncoding( SwFmt& rFmt, rtl_TextEncoding eEnc )
{
    if( RTL_TEXTENCODING_DONTKNOW == eEnc )
        return;

    const SfxItemSet& rItemSet = rFmt.GetAttrSet();
    static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                   RES_CHRATR_CTL_FONT };
    const SfxPoolItem *pItem;
    for( sal_uInt16 i=0; i<3; i++ )
    {
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i], sal_False,&pItem ) )
        {
            const SvxFontItem& rFont = *(const SvxFontItem *)pItem;
            if( RTL_TEXTENCODING_SYMBOL != rFont.GetCharSet() )
            {
                SvxFontItem aFont( rFont.GetFamily(), rFont.GetFamilyName(),
                                   rFont.GetStyleName(), rFont.GetPitch(),
                                   eEnc, aWhichIds[i]);
                rFmt.SetFmtAttr( aFont );
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
            sal_uInt16 i;
            for( i=0; i<3; i++ )
            {
                const SvxFontItem& rDfltFont =
                    (const SvxFontItem&)pDoc->GetDefault( aWhichIds[i]);
                SvxFontItem aFont( rDfltFont.GetFamily(),
                                   rDfltFont.GetFamilyName(),
                                   rDfltFont.GetStyleName(),
                                   rDfltFont.GetPitch(),
                                   eEnc, aWhichIds[i] );
                pDoc->SetDefault( aFont );
            }

            // Change all paragraph styles that do specify a font.
            sal_uInt16 nArrLen = pDoc->GetTxtFmtColls()->size();
            for( i=1; i<nArrLen; i++ )
                lcl_swcss1_setEncoding( *(*pDoc->GetTxtFmtColls())[i], eEnc );

            // Change all character styles that do specify a font.
            nArrLen = pDoc->GetCharFmts()->size();
            for( i=1; i<nArrLen; i++ )
                lcl_swcss1_setEncoding( *(*pDoc->GetCharFmts())[i], eEnc );
        }

        SvxCSS1Parser::SetDfltEncoding( eEnc );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
