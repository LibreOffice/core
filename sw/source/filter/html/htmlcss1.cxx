/*************************************************************************
 *
 *  $RCSfile: htmlcss1.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:58:51 $
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


#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_FONTLIST         SID_ATTR_CHAR_FONTLIST
#include "hintids.hxx"

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include <svx/flstitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif

#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif

#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FRMATR_HXX
#include "frmatr.hxx"
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif


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

// Wie viele Zeilen/Zeichen sind fuer DropCaps erlaubt?
// (Gibt es vielleicht woanders entsprechende Werte?)
#define MAX_DROPCAP_LINES 9
#define MAX_DROPCAP_CHARS 9

void lcl_swcss1_setEncoding( SwFmt& rFmt, rtl_TextEncoding eEnc );

/*  */

// Implementierung des SwCSS1Parsers (eigentlich swcss1.cxx)
static struct SwCSS1ItemIds
{
    USHORT nFmtBreak;
    USHORT nFmtPageDesc;
    USHORT nFmtKeep;

    SwCSS1ItemIds() :
        nFmtBreak( RES_BREAK ),
        nFmtPageDesc( RES_PAGEDESC ),
        nFmtKeep( RES_KEEP )
    {}

} aItemIds;

void SwCSS1Parser::ChgPageDesc( const SwPageDesc *pPageDesc,
                                const SwPageDesc& rNewPageDesc )
{
    USHORT nPageDescs = pDoc->GetPageDescCnt();
    for( USHORT i=0; i<nPageDescs; i++ )
        if( pPageDesc == &(pDoc->GetPageDesc(i)) )
        {
            pDoc->ChgPageDesc( i, rNewPageDesc );
            return;
        }

    ASSERT( i<nPageDescs, "Seitenvorlage nicht gefunden" );
}

SwCSS1Parser::SwCSS1Parser( SwDoc *pD, ULONG aFHeights[7], BOOL bNewDoc ) :
    SvxCSS1Parser( pD->GetAttrPool(), MM50/2,
                   (USHORT*)&aItemIds, sizeof(aItemIds) / sizeof(USHORT) ),
    nDropCapCnt( 0 ), bIsNewDoc( bNewDoc ), pDoc( pD ),
    bBodyBGColorSet( FALSE ), bBodyBackgroundSet( FALSE ),
    bBodyTextSet( FALSE ), bBodyLinkSet( FALSE ), bBodyVLinkSet( FALSE ),
    bSetFirstPageDesc( FALSE ), bSetRightPageDesc( FALSE ),
    bTableHeaderTxtCollSet( FALSE ), bTableTxtCollSet( FALSE ),
    bLinkCharFmtsSet( FALSE )
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


/*  */

// Feature: PrintExt
BOOL SwCSS1Parser::SetFmtBreak( SfxItemSet& rItemSet,
                                const SvxCSS1PropertyInfo& rPropInfo )
{
    SvxBreak eBreak = SVX_BREAK_NONE;
    BOOL bKeep = FALSE;
    BOOL bSetKeep = FALSE, bSetBreak = FALSE, bSetPageDesc = FALSE;
    const SwPageDesc *pPageDesc = 0;
    switch( rPropInfo.ePageBreakBefore )
    {
    case SVX_CSS1_PBREAK_ALWAYS:
        eBreak = SVX_BREAK_PAGE_BEFORE;
        bSetBreak = TRUE;
        break;
    case SVX_CSS1_PBREAK_LEFT:
        pPageDesc = GetLeftPageDesc( TRUE );
        bSetPageDesc = TRUE;
        break;
    case SVX_CSS1_PBREAK_RIGHT:
        pPageDesc = GetRightPageDesc( TRUE );
        bSetPageDesc = TRUE;
        break;
    case SVX_CSS1_PBREAK_AUTO:
        bSetBreak = bSetPageDesc = TRUE;
        break;
//  case SVX_CSS1_PBREAK_AVOID:
        // Hier koennte man SvxKeepItem am Absatz davor einfuegen
//      break;
    }
    switch( rPropInfo.ePageBreakAfter )
    {
    case SVX_CSS1_PBREAK_ALWAYS:
    case SVX_CSS1_PBREAK_LEFT:
    case SVX_CSS1_PBREAK_RIGHT:
        // LEFT/RIGHT koennte man auch am Absatz davor setzen
        eBreak = SVX_BREAK_PAGE_AFTER;
        bSetBreak = TRUE;
        break;
    case SVX_CSS1_PBREAK_AUTO:
        bSetBreak = bSetKeep = bSetPageDesc = TRUE;
        break;
    case SVX_CSS1_PBREAK_AVOID:
        bKeep = bSetKeep = TRUE;
        break;
    }

    if( bSetBreak )
        rItemSet.Put( SvxFmtBreakItem( eBreak ) );
    if( bSetPageDesc )
        rItemSet.Put( SwFmtPageDesc( pPageDesc ) );
    if( bSetKeep )
        rItemSet.Put( SvxFmtKeepItem( bKeep ) );

    return bSetBreak;
}
// /Feature: PrintExt

static void SetCharFmtAttrs( SwCharFmt *pCharFmt, SfxItemSet& rItemSet )
{
    const SfxPoolItem *pItem;
    static USHORT aWhichIds[3] = { RES_CHRATR_FONTSIZE,RES_CHRATR_CJK_FONTSIZE,
                                   RES_CHRATR_CTL_FONTSIZE };
       for( USHORT i=0; i<3; i++ )
    {
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i], FALSE,
                                                   &pItem ) &&
            ((const SvxFontHeightItem *)pItem)->GetProp() != 100)
        {
            // %-Angaben beim FontHeight-Item werden nicht unterstuetzt
            rItemSet.ClearItem( aWhichIds[i] );
        }
    }

    pCharFmt->SetAttr( rItemSet );

    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, FALSE, &pItem ) )
    {
        // Ein Brush-Item mit RES_BACKGROUND muss noch in eines mit
        // RES_CHRATR_BACKGROUND gewandelt werden

        SvxBrushItem aBrushItem( *(const SvxBrushItem *)pItem );
        aBrushItem.SetWhich( RES_CHRATR_BACKGROUND );
        pCharFmt->SetAttr( aBrushItem );
    }
}

void SwCSS1Parser::SetLinkCharFmts()
{
    ASSERT( !bLinkCharFmtsSet, "Aufruf von SetLinkCharFmts unnoetig" );

    SvxCSS1MapEntry *pStyleEntry =
        GetTag( String::CreateFromAscii(sHTML_anchor) );
    SwCharFmt *pUnvisited = 0, *pVisited = 0;
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        BOOL bColorSet = (SFX_ITEM_SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              FALSE));
        pUnvisited = GetCharFmtFromPool( RES_POOLCHR_INET_NORMAL );
        SetCharFmtAttrs( pUnvisited, rItemSet );
        bBodyLinkSet |= bColorSet;

        pVisited = GetCharFmtFromPool( RES_POOLCHR_INET_VISIT );
        SetCharFmtAttrs( pVisited, rItemSet );
        bBodyVLinkSet |= bColorSet;
    }

    String sTmp( String::CreateFromAscii(sHTML_anchor) );
    sTmp.Append( ':' );
    sTmp.AppendAscii( sCSS1_link );
    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        BOOL bColorSet = (SFX_ITEM_SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              FALSE));
        if( !pUnvisited )
            pUnvisited = GetCharFmtFromPool( RES_POOLCHR_INET_NORMAL );
        SetCharFmtAttrs( pUnvisited, rItemSet );
        bBodyLinkSet |= bColorSet;
    }

    sTmp.AssignAscii( sHTML_anchor );
    sTmp.Assign( ':' );
    sTmp.AppendAscii( sCSS1_visited );
    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        BOOL bColorSet = (SFX_ITEM_SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              FALSE));
        if( !pVisited )
            pVisited = GetCharFmtFromPool( RES_POOLCHR_INET_VISIT );
        SetCharFmtAttrs( pVisited, rItemSet );
        bBodyVLinkSet |= bColorSet;
    }

    bLinkCharFmtsSet = TRUE;
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
        SFX_ITEM_SET == rCollItemSet.GetItemState(RES_LR_SPACE,TRUE,&pCollItem) &&
        SFX_ITEM_SET == rItemSet.GetItemState(RES_LR_SPACE,FALSE,&pItem) )
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
        SFX_ITEM_SET == rCollItemSet.GetItemState(RES_UL_SPACE,TRUE,
                                                  &pCollItem) &&
        SFX_ITEM_SET == rItemSet.GetItemState(RES_UL_SPACE,FALSE,&pItem) )
    {
        const SvxULSpaceItem *pULItem = (const SvxULSpaceItem *)pItem;

        SvxULSpaceItem aULItem( *((const SvxULSpaceItem *)pCollItem) );
        if( rPropInfo.bTopMargin )
            aULItem.SetUpper( pULItem->GetUpper() );
        if( rPropInfo.bBottomMargin )
            aULItem.SetLower( pULItem->GetLower() );

        rItemSet.Put( aULItem );
    }

    static USHORT aWhichIds[3] = { RES_CHRATR_FONTSIZE,RES_CHRATR_CJK_FONTSIZE,
                                   RES_CHRATR_CTL_FONTSIZE };
       for( USHORT i=0; i<3; i++ )
    {
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i], FALSE,
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

    pColl->SetAttr( rItemSet );
}

void SwCSS1Parser::SetTableTxtColl( BOOL bHeader )
{
    ASSERT( !(bHeader ? bTableHeaderTxtCollSet : bTableTxtCollSet),
            "Aufruf von SetTableTxtColl unnoetig" );

    USHORT nPoolId;
    String sTag;
    if( bHeader )
    {
        nPoolId = RES_POOLCOLL_TABLE_HDLN;
        sTag.AssignAscii( sHTML_tableheader );
    }
    else
    {
        nPoolId = RES_POOLCOLL_TABLE;
        sTag.AssignAscii( sHTML_tabledata );
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

    String sTmp( sTag );
    sTmp.Append( ' ' );
    sTmp.AppendAscii( sHTML_parabreak );
    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        if( !pColl )
            pColl = GetTxtFmtColl( nPoolId, aEmptyStr );
        SetTxtCollAttrs( pColl, pStyleEntry->GetItemSet(),
                         pStyleEntry->GetPropertyInfo(), this );
    }

    if( bHeader )
        bTableHeaderTxtCollSet = TRUE;
    else
        bTableTxtCollSet = TRUE;
}

void SwCSS1Parser::SetPageDescAttrs( const SvxBrushItem *pBrush,
                                     SfxItemSet *pItemSet )
{
    SvxBrushItem aBrushItem;
    SvxBoxItem aBoxItem;
    SvxFrameDirectionItem aFrmDirItem;
    BOOL bSetBrush = pBrush!=0, bSetBox = FALSE, bSetFrmDir = FALSE;
    if( pBrush )
        aBrushItem = *pBrush;

    if( pItemSet )
    {
        const SfxPoolItem *pItem = 0;
        if( SFX_ITEM_SET == pItemSet->GetItemState( RES_BACKGROUND, FALSE,
                                                   &pItem ) )
        {
            // ein Hintergrund wird gesetzt
            aBrushItem = *((const SvxBrushItem *)pItem);
            pItemSet->ClearItem( RES_BACKGROUND );
            bSetBrush = TRUE;
        }

        if( SFX_ITEM_SET == pItemSet->GetItemState( RES_BOX, FALSE, &pItem ) )
        {
            // eine Umrandung wird gesetzt
            aBoxItem = *((const SvxBoxItem *)pItem);
            pItemSet->ClearItem( RES_BOX );
            bSetBox = TRUE;
        }

        if( SFX_ITEM_SET == pItemSet->GetItemState( RES_BOX, FALSE, &pItem ) )
        {
            // eine Umrandung wird gesetzt
            aBoxItem = *((const SvxBoxItem *)pItem);
            pItemSet->ClearItem( RES_BOX );
            bSetBox = TRUE;
        }

        if( SFX_ITEM_SET == pItemSet->GetItemState( RES_FRAMEDIR, FALSE, &pItem ) )
        {
            // eine Umrandung wird gesetzt
            aFrmDirItem = *static_cast< const SvxFrameDirectionItem *>( pItem );
            pItemSet->ClearItem( RES_FRAMEDIR );
            bSetFrmDir = TRUE;
        }
    }

    if( bSetBrush || bSetBox || bSetFrmDir )
    {
        static USHORT aPoolIds[] = { RES_POOLPAGE_HTML, RES_POOLPAGE_FIRST,
                                     RES_POOLPAGE_LEFT, RES_POOLPAGE_RIGHT };
        for( USHORT i=0; i<4; i++ )
        {
            const SwPageDesc *pPageDesc = GetPageDesc( aPoolIds[i], FALSE );
            if( pPageDesc )
            {
                SwPageDesc aNewPageDesc( *pPageDesc );
                SwFrmFmt &rMaster = aNewPageDesc.GetMaster();
                if( bSetBrush )
                    rMaster.SetAttr( aBrushItem );
                if( bSetBox )
                    rMaster.SetAttr( aBoxItem );
                if( bSetFrmDir )
                    rMaster.SetAttr( aFrmDirItem );

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
    BOOL bChanged = FALSE;

    // linker, rechter Rand und Erstzeilen-Einzug
    if( (rPropInfo.bLeftMargin || rPropInfo.bRightMargin) &&
        SFX_ITEM_SET == rItemSet.GetItemState(RES_LR_SPACE,FALSE,&pItem) )
    {
        if( (!rPropInfo.bLeftMargin || !rPropInfo.bRightMargin) &&
            SFX_ITEM_SET == rPageItemSet.GetItemState(RES_LR_SPACE,
                                                      TRUE,&pPageItem) )
        {
            const SvxLRSpaceItem *pLRItem = (const SvxLRSpaceItem *)pItem;

            SvxLRSpaceItem aLRItem( *((const SvxLRSpaceItem *)pPageItem) );
            if( rPropInfo.bLeftMargin )
                aLRItem.SetLeft( pLRItem->GetLeft() );
            if( rPropInfo.bRightMargin )
                aLRItem.SetRight( pLRItem->GetRight() );

            rMaster.SetAttr( aLRItem );
        }
        else
        {
            rMaster.SetAttr( *pItem );
        }
        bChanged = TRUE;
    }

    // oberer und unterer Rand
    if( (rPropInfo.bTopMargin || rPropInfo.bBottomMargin) &&
        SFX_ITEM_SET == rItemSet.GetItemState(RES_UL_SPACE,FALSE,&pItem) )
    {
        if( (!rPropInfo.bTopMargin || !rPropInfo.bBottomMargin) &&
            SFX_ITEM_SET == rPageItemSet.GetItemState(RES_UL_SPACE,
                                                      TRUE,&pPageItem) )
        {
            const SvxULSpaceItem *pULItem = (const SvxULSpaceItem *)pItem;

            SvxULSpaceItem aULItem( *((const SvxULSpaceItem *)pPageItem) );
            if( rPropInfo.bTopMargin )
                aULItem.SetUpper( pULItem->GetUpper() );
            if( rPropInfo.bBottomMargin )
                aULItem.SetLower( pULItem->GetLower() );

            rMaster.SetAttr( aULItem );
        }
        else
        {
            rMaster.SetAttr( *pItem );
        }
        bChanged = TRUE;
    }

    // die Groesse
    if( rPropInfo.eSizeType != SVX_CSS1_STYPE_NONE )
    {
        if( rPropInfo.eSizeType == SVX_CSS1_STYPE_TWIP )
        {
            rMaster.SetAttr( SwFmtFrmSize( ATT_FIX_SIZE, rPropInfo.nWidth,
                                           rPropInfo.nHeight ) );
            bChanged = TRUE;
        }
        else
        {
            // Bei "size: auto|portrait|landscape" bleibt die bisherige
            // Groesse der Vorlage erhalten. Bei "landscape" und "portrait"
            // wird das Landscape-Flag gesetzt und evtl. die Breite/Hoehe
            // vertauscht.
            SwFmtFrmSize aFrmSz( rMaster.GetFrmSize() );
            BOOL bLandscape = aNewPageDesc.GetLandscape();
            if( ( bLandscape &&
                  rPropInfo.eSizeType == SVX_CSS1_STYPE_PORTRAIT ) ||
                ( !bLandscape &&
                  rPropInfo.eSizeType == SVX_CSS1_STYPE_LANDSCAPE ) )
            {
                SwTwips nTmp = aFrmSz.GetHeight();
                aFrmSz.SetHeight( aFrmSz.GetWidth() );
                aFrmSz.SetWidth( nTmp );
                rMaster.SetAttr( aFrmSz );
                aNewPageDesc.SetLandscape( !bLandscape );
                bChanged = TRUE;
            }
        }
    }

    // Geht das wirklich?
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND, FALSE, &pItem ) )
    {
        // eine Umrandung wird gesetzt
        rMaster.SetAttr( *pItem );
        rItemSet.ClearItem( RES_BACKGROUND );
        bChanged = TRUE;
    }

    if( bChanged )
        ChgPageDesc( pPageDesc, aNewPageDesc );
}
// /Feature: PrintExt

const SvxBrushItem& SwCSS1Parser::GetPageDescBackground() const
{
    return pDoc->GetPageDescFromPoolSimple( RES_POOLPAGE_HTML, FALSE )
        ->GetMaster().GetBackground();
}

sal_uInt16 SwCSS1Parser::GetScriptFromClass( String& rClass,
                                      sal_Bool bSubClassOnly )
{
    sal_uInt16 nScriptFlags = CSS1_SCRIPT_ALL;
    xub_StrLen nLen = rClass.Len();
    xub_StrLen nPos = nLen > 4 ? rClass.SearchBackward( '-' ) : STRING_NOTFOUND;

    if( STRING_NOTFOUND == nPos )
    {
        if( bSubClassOnly )
            return nScriptFlags;
        nPos = 0;
    }
    else
    {
        nPos++;
        nLen -= nPos;
    }

    switch( nLen )
    {
    case 3:
        if( rClass.EqualsIgnoreCaseAscii( "cjk", nPos, 3 ) )
        {
            nScriptFlags = CSS1_SCRIPT_CJK;
        }
        else if( rClass.EqualsIgnoreCaseAscii( "ctl", nPos, 3 ) )
        {
            nScriptFlags = CSS1_SCRIPT_CTL;
        }
        break;
    case 7:
        if( rClass.EqualsIgnoreCaseAscii( "western", nPos, 7 ) )
        {
            nScriptFlags = CSS1_SCRIPT_WESTERN;
        }
        break;
    }
    if( CSS1_SCRIPT_ALL != nScriptFlags )
    {
        if( nPos )
        {
            rClass.Erase( nPos-1 );
        }
        else
        {
            rClass.Erase();
        }
    }

    return nScriptFlags;
}

static CSS1SelectorType GetTokenAndClass( const CSS1Selector *pSelector,
                              String& rToken, String& rClass,
                              sal_uInt16& rScriptFlags )
{
    rToken = pSelector->GetString();
    rClass.Erase();
    rScriptFlags = CSS1_SCRIPT_ALL;

    CSS1SelectorType eType = pSelector->GetType();
    if( CSS1_SELTYPE_ELEM_CLASS==eType  )
    {
        xub_StrLen nPos = rToken.Search( '.' );
        ASSERT( nPos != STRING_NOTFOUND, "kein Punkt in Class-Selektor???" );
        if( nPos != STRING_NOTFOUND )
        {
            rClass = rToken.Copy( nPos+1 );
            rToken.Erase( nPos );

            rScriptFlags = SwCSS1Parser::GetScriptFromClass( rClass, sal_False );
            if( !rClass.Len() )
                eType = CSS1_SELTYPE_ELEMENT;
        }
    }

    rToken.ToUpperAscii();
    return eType;
}

extern BOOL lcl_css1atr_equalFontItems( const SfxPoolItem& r1, const SfxPoolItem& r2 );

static void RemoveScriptItems( SfxItemSet& rItemSet, sal_uInt16 nScript,
                               const SfxItemSet *pParentItemSet = 0 )
{
    static sal_uInt16 aWhichIds[3][5] =
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
        ASSERT( aClearItems[0], "unknown script type" );
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
                 (0==i ? lcl_css1atr_equalFontItems( *pItem, pParentItemSet->Get(nWhich, sal_True ) )
                        : *pItem == pParentItemSet->Get(nWhich, sal_True ) ) ) )
            {
                rItemSet.ClearItem( nWhich );
            }
        }
    }
}

BOOL SwCSS1Parser::StyleParsed( const CSS1Selector *pSelector,
                                SfxItemSet& rItemSet,
                                SvxCSS1PropertyInfo& rPropInfo )
{
    if( !bIsNewDoc )
        return TRUE;

    CSS1SelectorType eSelType = pSelector->GetType();
    const CSS1Selector *pNext = pSelector->GetNext();

    if( CSS1_SELTYPE_ID==eSelType && !pNext )
    {
        InsertId( pSelector->GetString(), rItemSet, rPropInfo );
    }
    else if( CSS1_SELTYPE_CLASS==eSelType && !pNext )
    {
        String aClass( pSelector->GetString() );
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
            (pNext->GetString().EqualsIgnoreCaseAscii(sCSS1_left) ||
             pNext->GetString().EqualsIgnoreCaseAscii(sCSS1_right) ||
             pNext->GetString().EqualsIgnoreCaseAscii(sCSS1_first)) ) )
            // || CSS1_SELTYPE_ELEMENT == pNext->GetType() )
        {
            String aName;
            if( pNext )
                aName = pNext->GetString();
            InsertPage( aName,
                        pNext != 0 /*CSS1_SELTYPE_PSEUDO == pNext->GetType()*/,
                        rItemSet, rPropInfo );
        }
    }

    if( CSS1_SELTYPE_ELEMENT != eSelType &&
        CSS1_SELTYPE_ELEM_CLASS != eSelType)
        return TRUE;

    // Token und Class zu dem Selektor holen
    String aToken, aClass;
    sal_uInt16 nScript;
    eSelType = GetTokenAndClass( pSelector, aToken, aClass, nScript );
    int nToken = GetHTMLToken( aToken );

    // und noch ein ganz par Infos zum naechsten Element
    CSS1SelectorType eNextType = pNext ? pNext->GetType()
                                       : CSS1_SELTYPE_ELEMENT;

    // Erstmal ein par Spezialfaelle
    if( CSS1_SELTYPE_ELEMENT==eSelType )
    {
        switch( nToken )
        {
        case HTML_ANCHOR_ON:
            if( !pNext )
            {
                InsertTag( aToken, rItemSet, rPropInfo );
                return FALSE;
            }
            else if( pNext && CSS1_SELTYPE_PSEUDO == eNextType )
            {
                // vielleicht A:visited oder A:link

                String aPseudo( pNext->GetString() );
                aPseudo.ToLowerAscii();

                BOOL bInsert = FALSE;
                switch( aPseudo.GetChar( 0 ))
                {
                    case 'l':
                        if( aPseudo.EqualsAscii(sCSS1_link) )
                        {
                            bInsert = TRUE;
                        }
                        break;
                    case 'v':
                        if( aPseudo.EqualsAscii(sCSS1_visited) )
                        {
                            bInsert = TRUE;
                        }
                        break;
                }
                if( bInsert )
                {
                    String sTmp( aToken );
                    (sTmp += ':') += aPseudo;
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
                    return FALSE;
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
                if( SFX_ITEM_SET==rItemSet.GetItemState(RES_BACKGROUND,FALSE,&pItem) )
                {
                    const SvxBrushItem *pBrushItem =
                        (const SvxBrushItem *)pItem;

                    /// OD 02.09.2002 #99657#
                    /// Body has a background color, if it is not "no fill"/"auto fill"
                    if( pBrushItem->GetColor() != COL_TRANSPARENT )
                        bBodyBGColorSet = TRUE;
                    if( GPOS_NONE != pBrushItem->GetGraphicPos() )
                        bBodyBackgroundSet = TRUE;
                }

                // Border and Padding
                rPropInfo.SetBoxItem( rItemSet, MIN_BORDER_DIST );

                // Ein par Attribute muessen an der Seitenvorlage gesetzt werden,
                // und zwar die, die nicht vererbt werden
                SetPageDescAttrs( 0, &rItemSet );

                // alle noch uebrigen Optionen koennen an der Standard-Vorlage
                // gesetzt werden und gelten dann automatisch als defaults
                if( SFX_ITEM_SET==rItemSet.GetItemState(RES_CHRATR_COLOR,FALSE) )
                    bBodyTextSet = TRUE;
                SetTxtCollAttrs(
                    GetTxtCollFromPool( RES_POOLCOLL_STANDARD ),
                    rItemSet, rPropInfo, this );

                return FALSE;
            }
            break;
        }
    }
    else if( CSS1_SELTYPE_ELEM_CLASS==eSelType &&  HTML_ANCHOR_ON==nToken &&
             !pNext && aClass.Len() >= 9 &&
             ('s' == aClass.GetChar(0) || 'S' == aClass.GetChar(0)) )
    {
        USHORT nPoolFmtId = 0;
        if( aClass.EqualsIgnoreCaseAscii(sHTML_sdendnote_sym) )
            nPoolFmtId = RES_POOLCHR_ENDNOTE;
        else if( aClass.EqualsIgnoreCaseAscii(sHTML_sdfootnote_sym) )
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
            return FALSE;
        }
    }

    // Jetzt werden die Selektoren verarbeitet, die zu einer Absatz-Vorlage
    // gehoehren
    USHORT nPoolCollId = 0;
    switch( nToken )
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
        if( aClass.Len() >= 9 &&
            ('s' == aClass.GetChar(0) || 'S' == aClass.GetChar(0)) )
        {
            if( aClass.EqualsIgnoreCaseAscii(sHTML_sdendnote) )
                nPoolCollId = RES_POOLCOLL_ENDNOTE;
            else if( aClass.EqualsIgnoreCaseAscii(sHTML_sdfootnote) )
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
            InsertTag( aToken, rItemSet, rPropInfo );
            return FALSE;
        }
        else if( CSS1_SELTYPE_ELEMENT==eSelType && pNext &&
                 (CSS1_SELTYPE_ELEMENT==eNextType ||
                  CSS1_SELTYPE_ELEM_CLASS==eNextType) )
        {
            // nicht TH und TD, aber TH P und TD P
            String aSubToken, aSubClass;
            GetTokenAndClass( pNext, aSubToken, aSubClass, nScript );
            if( HTML_PARABREAK_ON == GetHTMLToken( aSubToken ) )
            {
                aClass = aSubClass;
                pNext = pNext->GetNext();
                eNextType = pNext ? pNext->GetType() : CSS1_SELTYPE_ELEMENT;

                if( aClass.Len() || pNext )
                {
                    nPoolCollId =
                        HTML_TABLEHEADER_ON == nToken ? RES_POOLCOLL_TABLE_HDLN
                                                      : RES_POOLCOLL_TABLE;
                }
                else
                {
                    String sTmp( aToken );
                    sTmp += ' ';
                    sTmp.AppendAscii( sHTML_parabreak );

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

                    return FALSE;
                }
            }
        }
        break;
    }

    if( nPoolCollId )
    {
        if( !pNext ||
            (CSS1_SELTYPE_PSEUDO==eNextType &&
#ifdef FULL_FIRST_LETTER
             pNext->GetString().EqualsIgnoreCaseAscii(sCSS1_first_letter)) )
#else
             pNext->GetString().EqualsIgnoreCaseAscii(sCSS1_first_letter) &&
             SVX_ADJUST_LEFT == rPropInfo.eFloat) )
#endif
        {
            // Entweder kein zusammengesetzter Selektor oder
            // ein X:first-line { float: left; ... }

            // Die Vorlage Suchen bzw. Anlegen
            SwTxtFmtColl *pColl = GetTxtFmtColl( nPoolCollId, aEmptyStr );
            SwTxtFmtColl* pParentColl = 0;
            if( aClass.Len() )
            {
                String aName( pColl->GetName() );
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
                        pColl->GetAttrSet().GetItemState(RES_BOX,TRUE,&pItem) )
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
                    FillDropCap( aDrop, rItemSet, &pColl->GetName() );
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
                    FillDropCap( aDrop, aScriptItemSet, &pColl->GetName() );
                }

                // Das Attribut nur setzen, wenn float: left angegeben wurde
                // und das Initial ueber mehrere Zeilen geht. Sonst wird die
                // ggf. angelegte Zeichen-Vorlage spaeter ueber den Namen
                // gesucht und gesetzt.
                if( aDrop.GetLines() > 1 &&
                    (SVX_ADJUST_LEFT == rPropInfo.eFloat  ||
                     CSS1_SCRIPT_ALL == nScript) )
                {
                    pColl->SetAttr( aDrop );
                }
            }

            return FALSE;
        }

        return TRUE;
    }

    // Jetzt werden die Selektoten verarbeitet, die zu einer Zechenvorlage
    // gehoehren. Zusammengesetzte gibt es hier allerdings nich nicht.
    if( pNext )
        return TRUE;

    SwCharFmt *pCFmt = GetChrFmt( nToken, aEmptyStr );
    if( pCFmt )
    {
        SwCharFmt *pParentCFmt = 0;
        if( aClass.Len() )
        {
            String aName( pCFmt->GetName() );
            AddClassName( aName, aClass );
            pParentCFmt = pCFmt;

            pCFmt = pDoc->FindCharFmtByName( aName );
            if( !pCFmt )
            {
                pCFmt = pDoc->MakeCharFmt( aName, pParentCFmt );
                pCFmt->SetAuto( FALSE );
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
        return FALSE;
    }

    return TRUE;
}

#ifdef USED
void SwCSS1Parser::SetFmtAttrs( const SfxItemSet& rItemSet,
                                SwFmt *pChildFmt1,
                                SwFmt *pChildFmt2 )
{
    SfxItemIter aIter( rItemSet );

    const SfxItemSet& rItemSet1 = pChildFmt1->GetAttrSet();
    const SfxItemSet* pItemSet2 = pChildFmt2 ? &pChildFmt2->GetAttrSet() : 0;
    const SfxPoolItem *pPItem = aIter.FirstItem();
    while( pPItem )
    {
        // wenn das Attribut an einer der abhaengigen Vorlagen nicht gesetzt
        // ist, muss es dort explizit auf den Wrt gesetzt werden, der
        // dort bereits aus der Parent-Vorlage geerbt wird.

        USHORT nWhich = pPItem->Which();
        const SfxPoolItem *pCItem;
        if( SFX_ITEM_SET != rItemSet1.GetItemState( nWhich, FALSE, &pCItem ) )
            pChildFmt1->SetAttr( pChildFmt1->GetAttr(nWhich) );

        if( pChildFmt2 &&
            SFX_ITEM_SET != pItemSet2->GetItemState( nWhich, FALSE, &pCItem ) )
            pChildFmt2->SetAttr( pChildFmt2->GetAttr(nWhich) );

        // auf zum naechsten Item
        pPItem = aIter.NextItem();
    }
}
#endif

ULONG SwCSS1Parser::GetFontHeight( USHORT nSize ) const
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

/*  */

SwCharFmt* SwCSS1Parser::GetChrFmt( USHORT nToken, const String& rClass ) const
{
    // die entsprechende Vorlage suchen
    USHORT nPoolId = 0;
    const sal_Char* sName = 0;
    switch( nToken )
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

    case HTML_SHORTQUOTE_ON:    sName = sHTML_shortquote;   break;
    case HTML_LANGUAGE_ON:      sName = sHTML_language;     break;
    case HTML_AUTHOR_ON:        sName = sHTML_author;       break;
    case HTML_PERSON_ON:        sName = sHTML_person;       break;
    case HTML_ACRONYM_ON:       sName = sHTML_acronym;      break;
    case HTML_ABBREVIATION_ON:  sName = sHTML_abbreviation; break;
    case HTML_INSERTEDTEXT_ON:  sName = sHTML_insertedtext; break;
    case HTML_DELETEDTEXT_ON:   sName = sHTML_deletedtext;  break;
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
        String sCName( String::CreateFromAscii(sName) );
        pCFmt = pDoc->FindCharFmtByName( sCName );
        if( !pCFmt )
        {
            pCFmt = pDoc->MakeCharFmt( sCName, pDoc->GetDfltCharFmt() );
            pCFmt->SetAuto( FALSE );
        }
    }

    ASSERT( pCFmt, "Keine Zeichen-Vorlage???" );

    // Wenn es eine Klasse gibt, die Klassen-Vorlage suchen aber nicht
    // neu anlegen.
    String aClass( rClass );
    GetScriptFromClass( aClass, sal_False );
    if( aClass.Len() )
    {
        String aTmp( pCFmt->GetName() );
        AddClassName( aTmp, aClass );
        SwCharFmt *pClassCFmt = pDoc->FindCharFmtByName( aTmp );
        if( pClassCFmt )
        {
            pCFmt = pClassCFmt;
        }
        else
        {
            SvxCSS1MapEntry *pClass = GetClass( aClass );
            if( pClass )
            {
                pCFmt = pDoc->MakeCharFmt( aTmp, pCFmt );
                pCFmt->SetAuto( FALSE );
                SfxItemSet aItemSet( pClass->GetItemSet() );
                SetCharFmtAttrs( pCFmt, aItemSet );
            }
        }
    }

    return pCFmt;
}


/*  */

SwTxtFmtColl *SwCSS1Parser::GetTxtCollFromPool( USHORT nPoolId ) const
{
    USHORT nOldArrLen = pDoc->GetTxtFmtColls()->Count();

    SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPoolSimple( nPoolId, FALSE );

    if( bIsNewDoc )
    {
        USHORT nArrLen = pDoc->GetTxtFmtColls()->Count();
        for( USHORT i=nOldArrLen; i<nArrLen; i++ )
            lcl_swcss1_setEncoding( *(*pDoc->GetTxtFmtColls())[i],
                                    GetDfltEncoding() );
    }

    return pColl;
}

SwCharFmt *SwCSS1Parser::GetCharFmtFromPool( USHORT nPoolId ) const
{
    USHORT nOldArrLen = pDoc->GetCharFmts()->Count();

    SwCharFmt *pCharFmt = pDoc->GetCharFmtFromPool( nPoolId );

    if( bIsNewDoc )
    {
        USHORT nArrLen = pDoc->GetCharFmts()->Count();

        for( USHORT i=nOldArrLen; i<nArrLen; i++ )
            lcl_swcss1_setEncoding( *(*pDoc->GetCharFmts())[i],
                                    GetDfltEncoding() );
    }

    return pCharFmt;
}

SwTxtFmtColl *SwCSS1Parser::GetTxtFmtColl( USHORT nTxtColl,
                                           const String& rClass )
{
    SwTxtFmtColl* pColl = 0;

    String aClass( rClass );
    GetScriptFromClass( aClass, sal_False );
    if( RES_POOLCOLL_TEXT == nTxtColl && aClass.Len() >= 9 &&
        ('s' == aClass.GetChar(0) || 'S' == aClass.GetChar(0) ) )
    {
        if( aClass.EqualsIgnoreCaseAscii(sHTML_sdendnote) )
        {
            nTxtColl = RES_POOLCOLL_ENDNOTE;
            aClass = aEmptyStr;
        }
        else if( aClass.EqualsIgnoreCaseAscii(sHTML_sdfootnote) )
        {
            nTxtColl = RES_POOLCOLL_FOOTNOTE;
            aClass = aEmptyStr;
        }
    }

    String sName;
    if( USER_FMT & nTxtColl )       // eine vom Reader angelegte
    {
        ASSERT( !this, "Wo kommt die Benutzer-Vorlage her?" );
        pColl = GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
    }
    else
    {
        pColl = GetTxtCollFromPool( nTxtColl );
    }

    ASSERT( pColl, "Keine Absatz-Vorlage???" );
    if( aClass.Len() )
    {
        String aTmp( pColl->GetName() );
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
                BOOL bPositioned = MayBePositioned( pClass->GetPropertyInfo() );
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
    return pDoc->GetPageDescFromPoolSimple( RES_POOLPAGE_HTML, FALSE );
}

static SwPageDesc *FindPageDesc( SwDoc *pDoc, USHORT nPoolId, USHORT& rPage )
{
    USHORT nPageDescs = pDoc->GetPageDescCnt();
    for( rPage=0; rPage < nPageDescs &&
         pDoc->GetPageDesc(rPage).GetPoolFmtId() != nPoolId; rPage++ )
         ;

    return rPage < nPageDescs ? &pDoc->_GetPageDesc( rPage ) : 0;
}

const SwPageDesc *SwCSS1Parser::GetPageDesc( USHORT nPoolId, BOOL bCreate )
{
    if( RES_POOLPAGE_HTML == nPoolId )
        return pDoc->GetPageDescFromPoolSimple( RES_POOLPAGE_HTML, FALSE );

    USHORT nPage;
    const SwPageDesc *pPageDesc = FindPageDesc( pDoc, nPoolId, nPage );
    if( !pPageDesc && bCreate )
    {
        // Die erste Seite wird aus der rechten Seite erzeugt, wenn es die
        // gibt.
        SwPageDesc *pMasterPageDesc = 0;
        if( RES_POOLPAGE_FIRST == nPoolId )
            pMasterPageDesc = FindPageDesc( pDoc, RES_POOLPAGE_RIGHT, nPage );
        if( !pMasterPageDesc )
            pMasterPageDesc = pDoc->GetPageDescFromPoolSimple
                ( RES_POOLPAGE_HTML, FALSE );

        // Die neue Seitenvorlage entsteht aus dem Master durch kopieren.
        SwPageDesc *pNewPageDesc = pDoc->
            GetPageDescFromPoolSimple( nPoolId, FALSE );

        // dazu brauchen wir auch die Nummer der neuen Vorlage
        pPageDesc = FindPageDesc( pDoc, nPoolId, nPage );
        ASSERT( pPageDesc==pNewPageDesc, "Seitenvorlage nicht gefunden" );

        pDoc->CopyPageDesc( *pMasterPageDesc, *pNewPageDesc, FALSE );

        // Die Vorlagen an ihren neuen Zweck anpassen.
        const SwPageDesc *pFollow = 0;
        BOOL bSetFollowFollow = FALSE;
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
            GetLeftPageDesc( TRUE );
            break;

        case RES_POOLPAGE_LEFT:
            // Die rechte Vorlage wird angelegt, wenn sie noch nicht existiert.
            // Es findet aber keine Verkettung statt.
            // Wenn schon eine erste Seitenvorlage existiert, wird die linke
            // Vorlage die Folge-Vorlage der ersten Seite.
            pFollow = GetRightPageDesc( TRUE );
            bSetFollowFollow = TRUE;
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


BOOL SwCSS1Parser::MayBePositioned( const SvxCSS1PropertyInfo& rPropInfo,
                                    BOOL bAutoWidth )
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


/*  */

void SwCSS1Parser::AddClassName( String& rFmtName, const String& rClass )
{
    ASSERT( rClass.Len(), "Style-Klasse ohne Laenge?" );

// ??????????
//  String aTmp( rClass );
//  GetpApp()->GetAppInternational().ToLower( aTmp );

    (rFmtName += '.') += rClass;
}

/*  */

void SwCSS1Parser::FillDropCap( SwFmtDrop& rDrop,
                                SfxItemSet& rItemSet,
                                const String *pName )
{
    // die Anzahl der Zeilen entspricht in etwa einer %-Angabe
    // fuer die Hoehe (was passiert mit absoluten Hoehen???)
    BYTE nLines = rDrop.GetLines();
    const SfxPoolItem *pItem;
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_CHRATR_FONTSIZE, FALSE, &pItem ) )
    {
        USHORT nProp = ((const SvxFontHeightItem *)pItem)->GetProp();
        nLines = (BYTE)((nProp + 50) / 100);
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
#ifdef FULL_FIRST_LETTER
    if( nLines<=1 && !pName )
#else
    if( nLines<=1 )
#endif
        return;

    rDrop.GetLines() = nLines;

    // ein rechter Rand wird der Abstand zum Text!
    if( SFX_ITEM_SET == rItemSet.GetItemState( RES_LR_SPACE, FALSE, &pItem ) )
    {
        rDrop.GetDistance() =
            ((const SvxLRSpaceItem *)pItem)->GetRight();
        rItemSet.ClearItem( RES_LR_SPACE );
    }

    // Fuer alle anderen Attribute eine Zeichen-Vorlage anlegen
    if( rItemSet.Count() )
    {
        SwCharFmt *pCFmt = 0;
        String aName;
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
                aName.AssignAscii( sCSS1_first_letter );
                aName.Append( ' ' );
                aName.Append(
                    String::CreateFromInt32( (sal_Int32)(++nDropCapCnt) ) );
            }
            while( pDoc->FindCharFmtByName(aName) );
        }

        if( !pCFmt )
        {
            pCFmt = pDoc->MakeCharFmt( aName, pDoc->GetDfltCharFmt() );
            pCFmt->SetAuto( FALSE );
        }
        SetCharFmtAttrs( pCFmt, rItemSet );

        // Die Zeichenvorlage braucht nur im Attribut gesetzt werden, wenn
        // auch das Attribut gesetzt wird.
        if( nLines > 1 )
            rDrop.SetCharFmt( pCFmt );
    }
}

/*  */

// CSS1-sezifisches des SwHTMLParsers

_HTMLAttr **SwHTMLParser::GetAttrTabEntry( USHORT nWhich )
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

    const HTMLOptions *pOptions = GetOptions();
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[--i];
        if( HTML_O_TYPE==pOption->GetToken() )
            sType = pOption->GetString();
    }

    bIgnoreRawData = sType.Len() &&
                     !sType.GetToken(0,';').EqualsAscii(sCSS_mimetype);
}

void SwHTMLParser::EndStyle()
{
    bIgnoreRawData = FALSE;

    if( aStyleSource.Len() )
    {
        pCSS1Parser->ParseStyleSheet( aStyleSource );
        aStyleSource.Erase();
    }
}

BOOL SwHTMLParser::FileDownload( const String& rURL,
                                 String& rStr )
{
    // View wegschmeissen (wegen Reschedule)
    ViewShell *pOldVSh = CallEndAction();
    SetSaveBaseURL();

    // Ein Medium anlegen
    SfxMedium aDLMedium( rURL, STREAM_READ | STREAM_SHARE_DENYWRITE, FALSE );
    aDLMedium.SetTransferPriority( SFX_TFPRIO_SYNCHRON );

    // Medium registrieren, damit abgebrochen werden kann
    if( pDoc->GetDocShell() )
        pDoc->GetDocShell()->RegisterTransfer( aDLMedium );

    SvStream* pStream = aDLMedium.GetInStream();
    if( pStream )
    {
        SvMemoryStream aStream;
        aStream << *pStream;

        aStream.Seek( STREAM_SEEK_TO_END );
        DBG_ASSERT( aStream.Tell() < STRING_MAXLEN,
                    "File zu lang fuer einen String, Ende abgeschnitten" );
        xub_StrLen nLen = aStream.Tell() < STRING_MAXLEN
                        ? (xub_StrLen)aStream.Tell()
                        : STRING_MAXLEN;

        rStr = String( (const sal_Char *)aStream.GetData(), nLen,
                       GetSrcEncoding() );
    }

    GetSaveAndSetOwnBaseURL();

    // wurde abgebrochen?
    if( ( pDoc->GetDocShell() && pDoc->GetDocShell()->IsAbortingImport() )
        || 1 == pDoc->GetLinkCnt() )
    {
        // wurde der Import vom SFX abgebrochen?
        eState = SVPAR_ERROR;
        pStream = 0;
    }

    // View wieder anlgen
#if OSL_DEBUG_LEVEL > 1
    ViewShell *pVSh =
#endif
        CallStartAction( pOldVSh );
#if OSL_DEBUG_LEVEL > 1
    ASSERT( pOldVSh == pVSh, "FileDownload: ViewShell wurde ausgetauscht" );
#endif

    return pStream!=0;
}

void SwHTMLParser::InsertLink()
{
    BOOL bFinishDownload = FALSE;
    if( pPendStack )
    {
        ASSERT( ShouldFinishFileDownload(),
                "Pending-Stack ohne File-Download?" );

        SwPendingStack* pTmp = pPendStack->pNext;
        delete pPendStack;
        pPendStack = pTmp;
        ASSERT( !pPendStack, "Wo kommt der Pending-Stack her?" );

        bFinishDownload = TRUE;
    }
    else
    {
        String sRel, sHRef, sType;

        const HTMLOptions *pOptions = GetOptions();
        for( USHORT i = pOptions->Count(); i; )
        {
            const HTMLOption *pOption = (*pOptions)[--i];
            switch( pOption->GetToken() )
            {
                case HTML_O_REL:
                    sRel = pOption->GetString();
                    break;
                case HTML_O_HREF:
                    ASSERT( INetURLObject::GetBaseURL() == sBaseURL,
                            "<LINK>: Base URL ist zerschossen" );
                    sHRef = INetURLObject::RelToAbs( pOption->GetString() );
                    break;
                case HTML_O_TYPE:
                    sType = pOption->GetString();
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
                StartFileDownload( sHRef, 0, pDoc->GetDocShell() );
                if( IsParserWorking() )
                {
                    // Der Style wurde synchron geladen und wir koennen
                    // es direkt aufrufen.
                    bFinishDownload = TRUE;
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
                String sSource;
                if( FileDownload( sHRef, sSource ) )
                    pCSS1Parser->ParseStyleSheet( sSource );
            }
        }
    }

    if( bFinishDownload )
    {
        String sSource;
        if( FinishFileDownload(sSource) && sSource.Len() )
            pCSS1Parser->ParseStyleSheet( sSource );
    }
}

BOOL SwCSS1Parser::ParseStyleSheet( const String& rIn )
{
    if( !SvxCSS1Parser::ParseStyleSheet( rIn ) )
        return FALSE;

    SwPageDesc *pMasterPageDesc =
        pDoc->GetPageDescFromPoolSimple( RES_POOLPAGE_HTML, FALSE );

    SvxCSS1MapEntry *pPageEntry = GetPage( aEmptyStr, FALSE );
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
//      if( pNamedPageDescs )
//      {
//          for( USHORT i=0; i<pNamedPageDescs->Count(); i++ )
//              SetPageDescAttrs( (*pNamedPageDescs)[i],
//                                pPageEntry->GetItemSet(),
//                                pPageEntry->GetPropertyInfo() );
//      }

    }

    pPageEntry = GetPage( String::CreateFromAscii(sCSS1_first), TRUE );
    if( pPageEntry )
    {
        SetPageDescAttrs( GetFirstPageDesc(TRUE), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        bSetFirstPageDesc = TRUE;
    }

    pPageEntry = GetPage( String::CreateFromAscii(sCSS1_right), TRUE );
    if( pPageEntry )
    {
        SetPageDescAttrs( GetRightPageDesc(TRUE), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        bSetRightPageDesc = TRUE;
    }

    pPageEntry = GetPage( String::CreateFromAscii(sCSS1_left), TRUE );
    if( pPageEntry )
        SetPageDescAttrs( GetLeftPageDesc(TRUE), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );

    // und jetzt noch die benannten Vorlagen
//  for( USHORT i=0; i < GetPageCount(); i++ )
//  {
//      pPageEntry = GetPage( i );
//      const String& rKey = pPageEntry->GetKey();
//      if( !rKey.Len() || rKey.GetChar(0) == ':' )
//          continue;
//
//      String aName( rKey );
//      GetpApp()->GetAppInternational().ToLower( aName );
//      USHORT nPage = pDoc->MakePageDesc( aName );
//      SwPageDesc *pPageDesc = &pDoc->_GetPageDesc( nPage );
//
//      // Die neue Seitenvorlage entsteht aus dem Master durch kopieren.
//      pDoc->CopyPageDesc( *pMasterPageDesc, *pPageDesc );
//      SetPageDescAttrs( pPageDesc, pPageEntry->GetItemSet(),
//                        pPageEntry->GetPropertyInfo() );
//
//      if( !pNamedPageDescs )
//          pNamedPageDescs = new SwHTMLPageDescs;
//      pNamedPageDescs->Insert( pPageDesc, pNamedPageDescs->Count() );
//  }

    return TRUE;
}

BOOL SwHTMLParser::ParseStyleOptions( const String &rStyle,
                                      const String &rId,
                                      const String &rClass,
                                      SfxItemSet &rItemSet,
                                      SvxCSS1PropertyInfo &rPropInfo,
                                         const String *pLang,
                                         const String *pDir )
{
    BOOL bRet = FALSE;

    if( rClass.Len() )
    {
        String aClass( rClass );
        SwCSS1Parser::GetScriptFromClass( aClass );
        SvxCSS1MapEntry *pClass = pCSS1Parser->GetClass( aClass );
        if( pClass )
        {
            pCSS1Parser->MergeStyles( pClass->GetItemSet(),
                                      pClass->GetPropertyInfo(),
                                      rItemSet, rPropInfo, FALSE );
            bRet = TRUE;
        }
    }

    if( rId.Len() )
    {
        SvxCSS1MapEntry *pId = pCSS1Parser->GetId( rId );
        if( pId )
            pCSS1Parser->MergeStyles( pId->GetItemSet(),
                                      pId->GetPropertyInfo(),
                                      rItemSet, rPropInfo, rClass.Len()!=0 );
        rPropInfo.aId = rId;
        bRet = TRUE;
    }

    if( rStyle.Len() )
    {
        pCSS1Parser->ParseStyleOption( rStyle, rItemSet, rPropInfo );
        bRet = TRUE;
    }

    if( bRet )
        rPropInfo.SetBoxItem( rItemSet, MIN_BORDER_DIST );

    if( pLang && pLang->Len() )
    {
        LanguageType eLang = ConvertIsoStringToLanguage( *pLang );
        if( LANGUAGE_DONTKNOW != eLang )
        {
            SvxLanguageItem aLang( eLang );
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
            SvxFrameDirectionItem aDir( eDir );
            rItemSet.Put( aDir );

            bRet = sal_True;
        }
    }

    return bRet;
}

void SwHTMLParser::SetAnchorAndAdjustment( const SfxItemSet &rItemSet,
                                           const SvxCSS1PropertyInfo &rPropInfo,
                                           SfxItemSet &rFrmItemSet )
{
    SwFmtAnchor aAnchor;

    SwHoriOrient eHoriOri = HORI_NONE;
    SwVertOrient eVertOri = VERT_NONE;
    SwRelationOrient eHoriRel = FRAME;
    SwRelationOrient eVertRel = FRAME;
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
                pDoc->GetNodes()[pPam->GetPoint()->nNode]->FindFlyStartNode();
            if( pFlySttNd )
            {
                aAnchor.SetType( FLY_AT_FLY );
                SwPosition aPos( *pFlySttNd );
                aAnchor.SetAnchor( &aPos );
            }
            else
            {
                aAnchor.SetType( FLY_PAGE );
                aAnchor.SetPageNum( 1 );
            }
            nHoriPos = rPropInfo.nLeft;
            nVertPos = rPropInfo.nTop;
        }
        else
        {
            aAnchor.SetType( FLY_AT_CNTNT );
            aAnchor.SetAnchor( pPam->GetPoint() );
            eVertOri = VERT_TOP;
            eVertRel = REL_CHAR;
            if( SVX_CSS1_LTYPE_TWIP == rPropInfo.eLeftType )
            {
                eHoriOri = HORI_NONE;
                eHoriRel = REL_PG_FRAME;
                nHoriPos = rPropInfo.nLeft;
            }
            else
            {
                eHoriOri = HORI_LEFT;
                eHoriRel = FRAME;   // wird noch umgeschossen
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
            aAnchor.SetType( FLY_AUTO_CNTNT );
            pPam->Move( fnMoveBackward );
            eVertOri = VERT_CHAR_BOTTOM;
            eVertRel = REL_CHAR;
        }
        else
        {
            aAnchor.SetType( FLY_AT_CNTNT );
            eVertOri = VERT_TOP;
            eVertRel = PRTAREA;
        }

        aAnchor.SetAnchor( pPam->GetPoint() );

        if( nCntnt )
            pPam->Move( fnMoveForward );

        USHORT nLeftSpace = 0, nRightSpace = 0;
        short nIndent = 0;
        GetMarginsFromContextWithNumBul( nLeftSpace, nRightSpace, nIndent );

        if( SVX_ADJUST_RIGHT==rPropInfo.eFloat )
        {
            eHoriOri = HORI_RIGHT;
            eHoriRel = nRightSpace ? PRTAREA : FRAME;
            eSurround = SURROUND_LEFT;
        }
        else
        {
            eHoriOri = HORI_LEFT;
            eHoriRel = nLeftSpace ? PRTAREA : FRAME;
            eSurround = SURROUND_RIGHT;
        }
    }
    rFrmItemSet.Put( aAnchor );

    // Absolut Positioniert mit Durchlauf
    rFrmItemSet.Put( SwFmtHoriOrient( nHoriPos, eHoriOri, eHoriRel ) );
    rFrmItemSet.Put( SwFmtVertOrient( nVertPos, eVertOri, eVertRel ) );
    rFrmItemSet.Put( SwFmtSurround( eSurround ) );
}

void SwHTMLParser::SetVarSize( SfxItemSet &rItemSet,
                               SvxCSS1PropertyInfo &rPropInfo,
                               SfxItemSet &rFrmItemSet,
                               SwTwips nDfltWidth, BYTE nDfltPrcWidth )
{
    SwFrmSize eSize = ATT_MIN_SIZE;
    SwTwips nWidth = nDfltWidth, nHeight = MINFLY;
    BYTE nPrcWidth = nDfltPrcWidth, nPrcHeight = 0;
    switch( rPropInfo.eWidthType )
    {
    case SVX_CSS1_LTYPE_PERCENTAGE:
        nPrcWidth = rPropInfo.nWidth > 0 ? (BYTE)rPropInfo.nWidth : 1;
        nWidth = MINFLY;
        break;
    case SVX_CSS1_LTYPE_TWIP:
        nWidth = rPropInfo.nWidth > MINFLY ? rPropInfo.nWidth : MINFLY;
        nPrcWidth = 0;
        break;
    }
    switch( rPropInfo.eHeightType )
    {
    case SVX_CSS1_LTYPE_PERCENTAGE:
        nPrcHeight = rPropInfo.nHeight > 0 ? (BYTE)rPropInfo.nHeight : 1;
        break;
    case SVX_CSS1_LTYPE_TWIP:
        // Netscape und MS-IE interpretieren die Hoehe regelwiedrig
        // als Mindest-Hoehe, also machwn wir das auch so.
        nHeight = rPropInfo.nHeight > MINFLY ? rPropInfo.nHeight : MINFLY;
        break;
    }

    SwFmtFrmSize aFrmSize( eSize, nWidth, nHeight );
    aFrmSize.SetWidthPercent( nPrcWidth );
    aFrmSize.SetHeightPercent( nPrcHeight );
    rFrmItemSet.Put( aFrmSize );
}

void SwHTMLParser::SetFrmFmtAttrs( SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo &rPropInfo,
                                   USHORT nFlags,
                                   SfxItemSet &rFrmItemSet )
{
    const SfxPoolItem *pItem;
    if( (nFlags & HTML_FF_BOX) != 0 &&
        SFX_ITEM_SET==rItemSet.GetItemState( RES_BOX, TRUE, &pItem ) )
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
        SFX_ITEM_SET==rItemSet.GetItemState( RES_BACKGROUND, TRUE, &pItem ) )
    {
        rFrmItemSet.Put( *pItem );
        rItemSet.ClearItem( RES_BACKGROUND );
    }

    if( (nFlags & HTML_FF_DIRECTION) != 0 &&
        SFX_ITEM_SET==rItemSet.GetItemState( RES_FRAMEDIR, TRUE, &pItem ) )
    {
        rFrmItemSet.Put( *pItem );
        rItemSet.ClearItem( RES_FRAMEDIR );
    }
}


/*  */

_HTMLAttrContext *SwHTMLParser::PopContext( USHORT nToken, USHORT nLimit,
                                            BOOL bRemove )
{
    USHORT nPos = aContexts.Count();
    if( nPos <= nContextStMin )
        return 0;

    BOOL bFound = 0==nToken;
    if( nToken )
    {
        // Stack-Eintrag zu dem Token suchen
        while( nPos > nContextStMin )
        {
            USHORT nCntxtToken = aContexts[--nPos]->GetToken();
            if( nCntxtToken == nToken )
            {
                bFound = TRUE;
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
            aContexts.Remove( nPos, 1 );
    }

    return pCntxt;
}

#ifdef USED
USHORT SwHTMLParser::GetTxtFmtCollFromContext() const
{
    USHORT nPos = aContexts.Count();
    USHORT nPoolId = 0;

    while( !nPoolId && nPos>0 ) // nicht nMinStack wg. Tabellen!!!
        nPoolId = aContexts[--nPos]->GetTxtFmtColl();

    if( !nPoolId )
        nPoolId = aDfltContext.GetTxtFmtColl();

    return nPoolId;
}
#endif

BOOL SwHTMLParser::GetMarginsFromContext( USHORT& nLeft,
                                          USHORT& nRight,
                                          short& nIndent,
                                          BOOL bIgnoreTopContext ) const
{
    USHORT nPos = aContexts.Count();
    if( bIgnoreTopContext )
    {
        if( !nPos )
            return FALSE;
        else
            nPos--;
    }

    while( nPos > nContextStAttrMin )
    {
        const _HTMLAttrContext *pCntxt = aContexts[--nPos];
        if( pCntxt->IsLRSpaceChanged() )
        {
            pCntxt->GetMargins( nLeft, nRight, nIndent );
            return TRUE;
        }
    }

    return FALSE;
}

BOOL SwHTMLParser::GetMarginsFromContextWithNumBul( USHORT& nLeft,
                                                    USHORT& nRight,
                                                    short& nIndent ) const
{
    BOOL bRet = GetMarginsFromContext( nLeft, nRight, nIndent );
#ifdef NUM_RELSPACE
    const SwHTMLNumRuleInfo& rInfo = ((SwHTMLParser*)this)->GetNumInfo();
    if( rInfo.GetDepth() )
    {
        BYTE nLevel = (BYTE)( (rInfo.GetDepth() <= MAXLEVEL ? rInfo.GetDepth()
                                                            : MAXLEVEL) - 1 );
        const SwNumFmt& rNumFmt = rInfo.GetNumRule()->Get(nLevel);
        nLeft += rNumFmt.GetAbsLSpace();
        nIndent = rNumFmt.GetFirstLineOffset();
    }
#endif

    return bRet;
}

void SwHTMLParser::GetULSpaceFromContext( USHORT& nUpper,
                                          USHORT& nLower ) const
{
    USHORT nDfltColl = 0;
    String aDfltClass;

    USHORT nPos = aContexts.Count();
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

void SwHTMLParser::EndContextAttrs( _HTMLAttrContext *pContext, BOOL bRemove )
{
    _HTMLAttrs &rAttrs = pContext->GetAttrs();
    for( USHORT i=0; i<rAttrs.Count(); i++ )
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
            ((SwFmtDrop&)pAttr->GetItem()).GetChars() = (BYTE)nChars;
        }

        EndAttr( pAttr );
    }

    if( bRemove && rAttrs.Count() )
        rAttrs.Remove( 0, rAttrs.Count() );
}

void SwHTMLParser::InsertParaAttrs( const SfxItemSet& rItemSet )
{
    SfxItemIter aIter( rItemSet );

    const SfxPoolItem *pItem = aIter.FirstItem();
    while( pItem )
    {
        // den zu dem Item gehoehrenden Tabellen-Eintrag ermitteln ...
        USHORT nWhich = pItem->Which();
        _HTMLAttr **ppAttr = GetAttrTabEntry( nWhich );

        if( ppAttr )
        {
            NewAttr( ppAttr, *pItem );
            if( RES_PARATR_BEGIN > nWhich )
                (*ppAttr)->SetLikePara();
            aParaAttrs.Insert( *ppAttr, aParaAttrs.Count() );
            EndAttr( *ppAttr, 0, FALSE );
        }

        pItem = aIter.NextItem();
    }
}

void SwHTMLParser::AddFormatAttrs( SfxItemSet& rItemSet,
                                   const SwFmt& rFmt ) const
{
    const SfxItemSet& rFmtItemSet = rFmt.GetAttrSet();
    const SfxPoolItem *pFmtItem, *pItem;
    SfxWhichIter aIter( rItemSet );

    USHORT nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        if( SFX_ITEM_SET != rItemSet.GetItemState(nWhich,FALSE,&pItem) &&
            SFX_ITEM_SET == rFmtItemSet.GetItemState(nWhich,TRUE,&pFmtItem) )
        {
                rItemSet.Put( *pFmtItem );
        }

        nWhich = aIter.NextWhich();
    }
}

void SwHTMLParser::RemoveFormatAttrs( SfxItemSet& rItemSet,
                                      const SwFmt& rFmt ) const
{
    const SfxItemSet& rFmtItemSet = rFmt.GetAttrSet();
    const SfxPoolItem *pItem;
    SfxWhichIter aIter( rItemSet );

    USHORT nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        if( SFX_ITEM_SET == rItemSet.GetItemState(nWhich,FALSE,&pItem) &&
            SFX_ITEM_SET == rFmtItemSet.GetItemState(nWhich,TRUE,&pItem) )
        {
                rItemSet.ClearItem( nWhich );
        }

        nWhich = aIter.NextWhich();
    }
}

void lcl_swcss1_setEncoding( SwFmt& rFmt, rtl_TextEncoding eEnc )
{
    if( RTL_TEXTENCODING_DONTKNOW == eEnc )
        return;

    const SfxItemSet& rItemSet = rFmt.GetAttrSet();
    static USHORT aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                   RES_CHRATR_CTL_FONT };
    const SfxPoolItem *pItem;
    for( USHORT i=0; i<3; i++ )
    {
        if( SFX_ITEM_SET == rItemSet.GetItemState( aWhichIds[i], FALSE,&pItem ) )
        {
            const SvxFontItem& rFont = *(const SvxFontItem *)pItem;
            if( RTL_TEXTENCODING_SYMBOL != rFont.GetCharSet() )
            {
                SvxFontItem aFont( rFont.GetFamily(), rFont.GetFamilyName(),
                                   rFont.GetStyleName(), rFont.GetPitch(),
                                   eEnc, aWhichIds[i]);
                rFmt.SetAttr( aFont );
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
            static USHORT aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                           RES_CHRATR_CTL_FONT };
            USHORT i;
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
            USHORT nArrLen = pDoc->GetTxtFmtColls()->Count();
            for( i=1; i<nArrLen; i++ )
                lcl_swcss1_setEncoding( *(*pDoc->GetTxtFmtColls())[i], eEnc );

            // Change all character styles that do specify a font.
            nArrLen = pDoc->GetCharFmts()->Count();
            for( i=1; i<nArrLen; i++ )
                lcl_swcss1_setEncoding( *(*pDoc->GetCharFmts())[i], eEnc );
        }

        SvxCSS1Parser::SetDfltEncoding( eEnc );
    }
}
