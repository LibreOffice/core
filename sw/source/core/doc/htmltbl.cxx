/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include "hintids.hxx"

//#define TEST_DELAYED_RESIZE

#ifdef TEST_DELAYED_RESIZE
#include <vcl/sound.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#include <sot/storage.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <frmfmt.hxx>
#include <docary.hxx>
#include "ndtxt.hxx"
#include "doc.hxx"
#include "swtable.hxx"
#include "rootfrm.hxx"
#include "docsh.hxx"
#include "flyfrm.hxx"
#include "poolfmt.hxx"
#include "viewsh.hxx"
#include "tabfrm.hxx"

#include "htmltbl.hxx"
#include "ndindex.hxx"

using namespace ::com::sun::star;


#define COLFUZZY 20
#define MAX_TABWIDTH (USHRT_MAX - 2001)


/*  */

class SwHTMLTableLayoutConstraints
{
    sal_uInt16 nRow;                    // Start-Zeile
    sal_uInt16 nCol;                    // Start-Spalte
    sal_uInt16 nColSpan;                // COLSPAN der Zelle

    SwHTMLTableLayoutConstraints *pNext;        // die naechste Bedingung

    sal_uLong nMinNoAlign, nMaxNoAlign; // Zwischenergebnisse AL-Pass 1

public:

    SwHTMLTableLayoutConstraints( sal_uLong nMin, sal_uLong nMax, sal_uInt16 nRow,
                                sal_uInt16 nCol, sal_uInt16 nColSp );
    ~SwHTMLTableLayoutConstraints();

    sal_uLong GetMinNoAlign() const { return nMinNoAlign; }
    sal_uLong GetMaxNoAlign() const { return nMaxNoAlign; }

    SwHTMLTableLayoutConstraints *InsertNext( SwHTMLTableLayoutConstraints *pNxt );
    SwHTMLTableLayoutConstraints* GetNext() const { return pNext; }

    sal_uInt16 GetRow() const { return nRow; }

    sal_uInt16 GetColSpan() const { return nColSpan; }
    sal_uInt16 GetColumn() const { return nCol; }
};

/*  */

SwHTMLTableLayoutCnts::SwHTMLTableLayoutCnts( const SwStartNode *pSttNd,
                                          SwHTMLTableLayout* pTab,
                                          sal_Bool bNoBrTag,
                                          SwHTMLTableLayoutCnts* pNxt ) :
    pNext( pNxt ), pBox( 0 ), pTable( pTab ), pStartNode( pSttNd ),
    nPass1Done( 0 ), nWidthSet( 0 ), bNoBreakTag( bNoBrTag )
{}

SwHTMLTableLayoutCnts::~SwHTMLTableLayoutCnts()
{
    delete pNext;
    delete pTable;
}

const SwStartNode *SwHTMLTableLayoutCnts::GetStartNode() const
{
    return pBox ? pBox->GetSttNd() : pStartNode;
}


/*  */

SwHTMLTableLayoutCell::SwHTMLTableLayoutCell( SwHTMLTableLayoutCnts *pCnts,
                                          sal_uInt16 nRSpan, sal_uInt16 nCSpan,
                                          sal_uInt16 nWidth, sal_Bool bPrcWidth,
                                          sal_Bool bNWrapOpt ) :
    pContents( pCnts ),
    nRowSpan( nRSpan ), nColSpan( nCSpan ),
    nWidthOption( nWidth ), bPrcWidthOption( bPrcWidth ),
    bNoWrapOption( bNWrapOpt )
{}

SwHTMLTableLayoutCell::~SwHTMLTableLayoutCell()
{
    if( nRowSpan==1 && nColSpan==1 )
    {
        delete pContents;
    }
}

/*  */

SwHTMLTableLayoutColumn::SwHTMLTableLayoutColumn( sal_uInt16 nWidth,
                                                  sal_Bool bRelWidth,
                                                  sal_Bool bLBorder ) :
    nMinNoAlign(MINLAY), nMaxNoAlign(MINLAY), nAbsMinNoAlign(MINLAY),
    nMin(0), nMax(0),
    nAbsColWidth(0), nRelColWidth(0),
    nWidthOption( nWidth ), bRelWidthOption( bRelWidth ),
    bLeftBorder( bLBorder )
{}


/*  */

SwHTMLTableLayoutConstraints::SwHTMLTableLayoutConstraints(
    sal_uLong nMin, sal_uLong nMax, sal_uInt16 nRw, sal_uInt16 nColumn, sal_uInt16 nColSp ):
    nRow( nRw ), nCol( nColumn ), nColSpan( nColSp ),
    pNext( 0 ),
    nMinNoAlign( nMin ), nMaxNoAlign( nMax )
{}

SwHTMLTableLayoutConstraints::~SwHTMLTableLayoutConstraints()
{
    delete pNext;
}

SwHTMLTableLayoutConstraints *SwHTMLTableLayoutConstraints::InsertNext(
    SwHTMLTableLayoutConstraints *pNxt )
{
    SwHTMLTableLayoutConstraints *pPrev = 0;
    SwHTMLTableLayoutConstraints *pConstr = this;
    while( pConstr )
    {
        if( pConstr->GetRow() > pNxt->GetRow() ||
            pConstr->GetColumn() > pNxt->GetColumn() )
            break;
        pPrev = pConstr;
        pConstr = pConstr->GetNext();
    }

    if( pPrev )
    {
        pNxt->pNext = pPrev->GetNext();
        pPrev->pNext = pNxt;
        pConstr = this;
    }
    else
    {
        pNxt->pNext = this;
        pConstr = pNxt;
    }

    return pConstr;
}

/*  */

typedef SwHTMLTableLayoutColumn *SwHTMLTableLayoutColumnPtr;
typedef SwHTMLTableLayoutCell *SwHTMLTableLayoutCellPtr;

SwHTMLTableLayout::SwHTMLTableLayout(
                        const SwTable * pSwTbl,
                        sal_uInt16 nRws, sal_uInt16 nCls, sal_Bool bColsOpt, sal_Bool bColTgs,
                        sal_uInt16 nWdth, sal_Bool bPrcWdth, sal_uInt16 nBorderOpt,
                        sal_uInt16 nCellPad, sal_uInt16 nCellSp, SvxAdjust eAdjust,
                        sal_uInt16 nLMargin, sal_uInt16 nRMargin,
                        sal_uInt16 nBWidth, sal_uInt16 nLeftBWidth,
                        sal_uInt16 nRightBWidth,
                        sal_uInt16 nInhLeftBWidth, sal_uInt16 nInhRightBWidth ) :
    aColumns( new SwHTMLTableLayoutColumnPtr[nCls] ),
    aCells( new SwHTMLTableLayoutCellPtr[nRws*nCls] ),
    pSwTable( pSwTbl ), pLeftFillerBox( 0 ), pRightFillerBox( 0 ),
    nMin( 0 ), nMax( 0 ),
    nRows( nRws ), nCols( nCls ),
    nLeftMargin( nLMargin ), nRightMargin( nRMargin ),
    nInhAbsLeftSpace( 0 ), nInhAbsRightSpace( 0 ),
    nRelLeftFill( 0 ), nRelRightFill( 0 ),
    nRelTabWidth( 0 ), nWidthOption( nWdth ),
    nCellPadding( nCellPad ), nCellSpacing( nCellSp ), nBorder( nBorderOpt ),
    nLeftBorderWidth( nLeftBWidth ), nRightBorderWidth( nRightBWidth ),
    nInhLeftBorderWidth( nInhLeftBWidth ),
    nInhRightBorderWidth( nInhRightBWidth ),
    nBorderWidth( nBWidth ),
    nDelayedResizeAbsAvail( 0 ), nLastResizeAbsAvail( 0 ),
    nPass1Done( 0 ), nWidthSet( 0 ), eTableAdjust( eAdjust ),
    bColsOption( bColsOpt ), bColTags( bColTgs ),
    bPrcWidthOption( bPrcWdth ), bUseRelWidth( sal_False ),
    bMustResize( sal_True ), bExportable( sal_True ), bBordersChanged( sal_False ),
    bMustNotResize( sal_False ), bMustNotRecalc( sal_False )
{
    aResizeTimer.SetTimeoutHdl( STATIC_LINK( this, SwHTMLTableLayout,
                                             DelayedResize_Impl ) );
}

SwHTMLTableLayout::~SwHTMLTableLayout()
{
    sal_uInt16 i;

    for( i = 0; i < nCols; i++ )
        delete aColumns[i];
    delete[] aColumns;

    sal_uInt16 nCount = nRows*nCols;
    for( i=0; i<nCount; i++ )
        delete aCells[i];
    delete[] aCells;
}

// Die Breiten der Umrandung werden zunaechst wie in Netscape berechnet:
// Aussere Umrandung: BORDER + CELLSPACING + CELLPADDING
// Innere Umrandung: CELLSPACING + CELLPADDING
// Allerdings wird die Breite der Umrandung im SW trotzdem beachtet, wenn
// bSwBorders gesetzt ist, damit nicht faellschlich umgebrochen wird.
// MIB 27.6.97: Dabei muss auch der Abstand zum Inhalt berueckichtigt werden,
// und zwar auch dann, wenn wenn nur die gegenueberliegende Seite
// eine Umrandung hat.
sal_uInt16 SwHTMLTableLayout::GetLeftCellSpace( sal_uInt16 nCol, sal_uInt16 nColSpan,
                                            sal_Bool bSwBorders ) const
{
    sal_uInt16 nSpace = nCellSpacing + nCellPadding;

    if( nCol == 0 )
    {
        nSpace = nSpace + nBorder;

        if( bSwBorders && nSpace < nLeftBorderWidth )
            nSpace = nLeftBorderWidth;
    }
    else if( bSwBorders )
    {
        if( GetColumn(nCol)->HasLeftBorder() )
        {
            if( nSpace < nBorderWidth )
                nSpace = nBorderWidth;
        }
        else if( nCol+nColSpan == nCols && nRightBorderWidth &&
                 nSpace < MIN_BORDER_DIST )
        {
            ASSERT( !nCellPadding, "GetLeftCellSpace: CELLPADDING!=0" );
            // Wenn die Gegenueberliegende Seite umrandet ist muessen
            // wir zumindest den minimalen Abstand zum Inhalt
            // beruecksichtigen. (Koennte man zusaetzlich auch an
            // nCellPadding festmachen.)
            nSpace = MIN_BORDER_DIST;
        }
    }

    return nSpace;
}

sal_uInt16 SwHTMLTableLayout::GetRightCellSpace( sal_uInt16 nCol, sal_uInt16 nColSpan,
                                             sal_Bool bSwBorders ) const
{
    sal_uInt16 nSpace = nCellPadding;

    if( nCol+nColSpan == nCols )
    {
        nSpace += nBorder + nCellSpacing;
        if( bSwBorders && nSpace < nRightBorderWidth )
            nSpace = nRightBorderWidth;
    }
    else if( bSwBorders && GetColumn(nCol)->HasLeftBorder() &&
             nSpace < MIN_BORDER_DIST )
    {
        ASSERT( !nCellPadding, "GetRightCellSpace: CELLPADDING!=0" );
        // Wenn die Gegenueberliegende Seite umrandet ist muessen
        // wir zumindest den minimalen Abstand zum Inhalt
        // beruecksichtigen. (Koennte man zusaetzlich auch an
        // nCellPadding festmachen.)
        nSpace = MIN_BORDER_DIST;
    }

    return nSpace;
}

void SwHTMLTableLayout::AddBorderWidth( sal_uLong &rMin, sal_uLong &rMax,
                                        sal_uLong &rAbsMin,
                                        sal_uInt16 nCol, sal_uInt16 nColSpan,
                                        sal_Bool bSwBorders ) const
{
    sal_uLong nAdd = GetLeftCellSpace( nCol, nColSpan, bSwBorders ) +
                 GetRightCellSpace( nCol, nColSpan, bSwBorders );

    rMin += nAdd;
    rMax += nAdd;
    rAbsMin += nAdd;
}

void SwHTMLTableLayout::SetBoxWidth( SwTableBox *pBox, sal_uInt16 nCol,
                             sal_uInt16 nColSpan ) const
{
    SwFrmFmt *pFrmFmt = pBox->GetFrmFmt();

    // die Breite der Box berechnen
    SwTwips nFrmWidth = 0;
    while( nColSpan-- )
        nFrmWidth += GetColumn( nCol++ )->GetRelColWidth();

    // und neu setzen

    pFrmFmt->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nFrmWidth, 0 ));
}

void SwHTMLTableLayout::GetAvail( sal_uInt16 nCol, sal_uInt16 nColSpan,
                                  sal_uInt16& rAbsAvail, sal_uInt16& rRelAvail ) const
{
    rAbsAvail = 0;
    rRelAvail = 0;
    for( sal_uInt16 i=nCol; i<nCol+nColSpan;i++ )
    {
        const SwHTMLTableLayoutColumn *pColumn = GetColumn(i);
        rAbsAvail = rAbsAvail + pColumn->GetAbsColWidth();
        rRelAvail = rRelAvail + pColumn->GetRelColWidth();
    }
}

sal_uInt16 SwHTMLTableLayout::GetBrowseWidthByVisArea( const SwDoc& rDoc )
{
    ViewShell *pVSh = 0;
    rDoc.GetEditShell( &pVSh );
    if( pVSh )
    {
        return (sal_uInt16)pVSh->GetBrowseWidth();
    }

    return 0;
}

sal_uInt16 SwHTMLTableLayout::GetBrowseWidth( const SwDoc& rDoc )
{
    // Wenn ein Layout da ist, koennen wir die Breite dort herholen.
    const SwRootFrm *pRootFrm = rDoc.GetRootFrm();
    if( pRootFrm )
    {
        const SwFrm *pPageFrm = pRootFrm->GetLower();
        if( pPageFrm )
            return (sal_uInt16)pPageFrm->Prt().Width();
    }

    // --> OD 2010-05-12 #i91658#
    // Assertion removed which state that no browse width is available.
    // Investigation reveals that all calls can handle the case that no browse
    // width is provided.
    return GetBrowseWidthByVisArea( rDoc );
    // <--
}

sal_uInt16 SwHTMLTableLayout::GetBrowseWidthByTabFrm(
    const SwTabFrm& rTabFrm ) const
{
    SwTwips nWidth = 0;

    const SwFrm *pUpper = rTabFrm.GetUpper();
    if( MayBeInFlyFrame() && pUpper->IsFlyFrm() &&
        ((const SwFlyFrm *)pUpper)->GetAnchorFrm() )
    {
        // Wenn die Tabelle in einem selbst angelegten Rahmen steht, dann ist
        // die Breite Ankers und nicht die Breite Rahmens von Bedeutung.
        // Bei Absatz-gebundenen Rahmen werden Absatz-Einzuege nicht beachtet.
        const SwFrm *pAnchor = ((const SwFlyFrm *)pUpper)->GetAnchorFrm();
        if( pAnchor->IsTxtFrm() )
            nWidth = pAnchor->Frm().Width();
        else
            nWidth = pAnchor->Prt().Width();
    }
    else
    {
        nWidth = pUpper->Prt().Width();
    }

    SwTwips nUpperDummy = 0;
    long nRightOffset = 0,
         nLeftOffset  = 0;
    rTabFrm.CalcFlyOffsets( nUpperDummy, nLeftOffset, nRightOffset );
    nWidth -= (nLeftOffset + nRightOffset);

    return nWidth < USHRT_MAX ? static_cast<sal_uInt16>(nWidth) : USHRT_MAX;
}

sal_uInt16 SwHTMLTableLayout::GetBrowseWidthByTable( const SwDoc& rDoc ) const
{
    sal_uInt16 nBrowseWidth = 0;
    SwClientIter aIter( *(SwModify*)pSwTable->GetFrmFmt() );
    SwClient* pCli = aIter.First( TYPE( SwTabFrm ));
    if( pCli )
    {
        nBrowseWidth = GetBrowseWidthByTabFrm( *(SwTabFrm*)pCli );
    }
    else
    {
        nBrowseWidth = SwHTMLTableLayout::GetBrowseWidth( rDoc );
    }

    return nBrowseWidth;
}

const SwStartNode *SwHTMLTableLayout::GetAnyBoxStartNode() const
{
    const SwStartNode *pBoxSttNd;

    const SwTableBox* pBox = pSwTable->GetTabLines()[0]->GetTabBoxes()[0];
    while( 0 == (pBoxSttNd = pBox->GetSttNd()) )
    {
        ASSERT( pBox->GetTabLines().Count() > 0,
                "Box ohne Start-Node und Lines" );
        ASSERT( pBox->GetTabLines()[0]->GetTabBoxes().Count() > 0,
                "Line ohne Boxen" );
        pBox = pBox->GetTabLines()[0]->GetTabBoxes()[0];
    }

    return pBoxSttNd;
}

SwFrmFmt *SwHTMLTableLayout::FindFlyFrmFmt() const
{
    const SwTableNode *pTblNd = GetAnyBoxStartNode()->FindTableNode();
    ASSERT( pTblNd, "Kein Table-Node?" );
    return pTblNd->GetFlyFmt();
}

static void lcl_GetMinMaxSize( sal_uLong& rMinNoAlignCnts, sal_uLong& rMaxNoAlignCnts,
                        sal_uLong& rAbsMinNoAlignCnts,
#ifdef FIX41370
                        sal_Bool& rHR,
#endif
                        SwTxtNode *pTxtNd, sal_uLong nIdx, sal_Bool bNoBreak )
{
    pTxtNd->GetMinMaxSize( nIdx, rMinNoAlignCnts, rMaxNoAlignCnts,
                           rAbsMinNoAlignCnts );
    ASSERT( rAbsMinNoAlignCnts <= rMinNoAlignCnts,
            "GetMinMaxSize: absmin > min" );
    ASSERT( rMinNoAlignCnts <= rMaxNoAlignCnts,
            "GetMinMaxSize: max > min" );

    //Bei einen <PRE>-Absatz entspricht die maximale Breite der
    // minimalen breite
    const SwFmtColl *pColl = &pTxtNd->GetAnyFmtColl();
    while( pColl && !pColl->IsDefault() &&
            (USER_FMT & pColl->GetPoolFmtId()) )
    {
        pColl = (const SwFmtColl *)pColl->DerivedFrom();
    }

    // <NOBR> in der gesamten Zelle bezieht sich auf Text, aber nicht
    // auf Tabellen. Netscape beruecksichtigt dies nur fuer Grafiken.
    if( (pColl && RES_POOLCOLL_HTML_PRE==pColl->GetPoolFmtId()) || bNoBreak )
    {
        rMinNoAlignCnts = rMaxNoAlignCnts;
        rAbsMinNoAlignCnts = rMaxNoAlignCnts;
    }
#ifdef FIX41370
    else if( pColl && RES_POOLCOLL_HTML_HR==pColl->GetPoolFmtId() )
    {
        rHR |= !pTxtNd->HasSwAttrSet() ||
                SFX_ITEM_SET != pTxtNd->GetpSwAttrSet()
                                      ->GetItemState( RES_LR_SPACE, sal_False );
    }
#endif
}

void SwHTMLTableLayout::AutoLayoutPass1()
{
    nPass1Done++;

    ClearPass1Info();

    sal_Bool bFixRelWidths = sal_False;
    sal_uInt16 i;

    SwHTMLTableLayoutConstraints *pConstraints = 0;

    for( i=0; i<nCols; i++ )
    {
        SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
        pColumn->ClearPass1Info( !HasColTags() );
        sal_uInt16 nMinColSpan = USHRT_MAX; // Spaltenzahl, auf die sich dir
                                        // berechnete Breite bezieht
        sal_uInt16 nColSkip = USHRT_MAX;    // Wie viele Spalten muessen
                                        // uebersprungen werden

        for( sal_uInt16 j=0; j<nRows; j++ )
        {
            SwHTMLTableLayoutCell *pCell = GetCell(j,i);
            SwHTMLTableLayoutCnts *pCnts = pCell->GetContents();

            // fix #31488#: Zum Ermitteln der naechsten zu berechnenden
            // Spalte muessen alle Zeilen herangezogen werden
            sal_uInt16 nColSpan = pCell->GetColSpan();
            if( nColSpan < nColSkip )
                nColSkip = nColSpan;

            if( !pCnts || (pCnts && !pCnts->IsPass1Done(nPass1Done)) )
            {
                // die Zelle ist leer oder ihr Inhalt wurde nich nicht
                // bearbeitet
                if( nColSpan < nMinColSpan )
                    nMinColSpan = nColSpan;

                sal_uLong nMinNoAlignCell = 0;
                sal_uLong nMaxNoAlignCell = 0;
                sal_uLong nAbsMinNoAlignCell = 0;
                sal_uLong nMaxTableCell = 0;
                sal_uLong nAbsMinTableCell = 0;
#ifdef FIX41370
                sal_Bool bHR = sal_False;
#endif

                while( pCnts )
                {
                    const SwStartNode *pSttNd = pCnts->GetStartNode();
                    if( pSttNd )
                    {
                        const SwDoc *pDoc = pSttNd->GetDoc();
                        sal_uLong nIdx = pSttNd->GetIndex();
                        while( !(pDoc->GetNodes()[nIdx])->IsEndNode() )
                        {
                            SwTxtNode *pTxtNd = (pDoc->GetNodes()[nIdx])->GetTxtNode();
                            if( pTxtNd )
                            {
                                sal_uLong nMinNoAlignCnts = 0;
                                sal_uLong nMaxNoAlignCnts = 0;
                                sal_uLong nAbsMinNoAlignCnts = 0;

                                lcl_GetMinMaxSize( nMinNoAlignCnts,
                                                   nMaxNoAlignCnts,
                                                   nAbsMinNoAlignCnts,
#ifdef FIX41370
                                                   bHR,
#endif
                                                   pTxtNd, nIdx,
                                                   pCnts->HasNoBreakTag() );

                                if( nMinNoAlignCnts > nMinNoAlignCell )
                                    nMinNoAlignCell = nMinNoAlignCnts;
                                if( nMaxNoAlignCnts > nMaxNoAlignCell )
                                    nMaxNoAlignCell = nMaxNoAlignCnts;
                                if( nAbsMinNoAlignCnts > nAbsMinNoAlignCell )
                                    nAbsMinNoAlignCell = nAbsMinNoAlignCnts;
                            }
                            else
                            {
                                SwTableNode *pTabNd = (pDoc->GetNodes()[nIdx])->GetTableNode();
                                if( pTabNd )
                                {
                                    SwHTMLTableLayout *pChild = pTabNd->GetTable().GetHTMLTableLayout();
                                    if( pChild )
                                    {
                                        pChild->AutoLayoutPass1();
                                        sal_uLong nMaxTableCnts = pChild->nMax;
                                        sal_uLong nAbsMinTableCnts = pChild->nMin;

                                        // Eine feste Tabellen-Breite wird als Minimum
                                        // und Maximum gleichzeitig uebernommen
                                        if( !pChild->bPrcWidthOption && pChild->nWidthOption )
                                        {
                                            sal_uLong nTabWidth = pChild->nWidthOption;
                                            if( nTabWidth >= nAbsMinTableCnts  )
                                            {
                                                nMaxTableCnts = nTabWidth;
                                                nAbsMinTableCnts = nTabWidth;
                                            }
                                            else
                                            {
                                                nMaxTableCnts = nAbsMinTableCnts;
                                            }
                                        }

                                        if( nMaxTableCnts > nMaxTableCell )
                                            nMaxTableCell = nMaxTableCnts;
                                        if( nAbsMinTableCnts > nAbsMinTableCell )
                                            nAbsMinTableCell = nAbsMinTableCnts;
                                    }
                                    nIdx = pTabNd->EndOfSectionNode()->GetIndex();
                                }
                            }
                            nIdx++;
                        }
                    }
                    else
                    {
                        ASSERT( !this, "Sub tables in HTML import?" )
                        SwHTMLTableLayout *pChild = pCnts->GetTable();
                        pChild->AutoLayoutPass1();
                        sal_uLong nMaxTableCnts = pChild->nMax;
                        sal_uLong nAbsMinTableCnts = pChild->nMin;

                        // Eine feste Tabellen-Breite wird als Minimum
                        // und Maximum gleichzeitig uebernommen
                        if( !pChild->bPrcWidthOption && pChild->nWidthOption )
                        {
                            sal_uLong nTabWidth = pChild->nWidthOption;
                            if( nTabWidth >= nAbsMinTableCnts  )
                            {
                                nMaxTableCnts = nTabWidth;
                                nAbsMinTableCnts = nTabWidth;
                            }
                            else
                            {
                                nMaxTableCnts = nAbsMinTableCnts;
                            }
                        }

                        if( nMaxTableCnts > nMaxTableCell )
                            nMaxTableCell = nMaxTableCnts;
                        if( nAbsMinTableCnts > nAbsMinTableCell )
                            nAbsMinTableCell = nAbsMinTableCnts;
                    }
                    pCnts->SetPass1Done( nPass1Done );
                    pCnts = pCnts->GetNext();
                }

// War frueher hinter AddBorderWidth
                // Wenn die Breite einer Tabelle in der Zelle breiter ist als
                // das, was wir fuer sonstigen Inhalt berechnet haben, mussen
                // wir die Breite der Tabelle nutzen
                if( nMaxTableCell > nMaxNoAlignCell )
                    nMaxNoAlignCell = nMaxTableCell;
                if( nAbsMinTableCell > nAbsMinNoAlignCell )
                {
                    nAbsMinNoAlignCell = nAbsMinTableCell;
                    if( nMinNoAlignCell < nAbsMinNoAlignCell )
                        nMinNoAlignCell = nAbsMinNoAlignCell;
                    if( nMaxNoAlignCell < nMinNoAlignCell )
                        nMaxNoAlignCell = nMinNoAlignCell;
                }
// War frueher hinter AddBorderWidth

                sal_Bool bRelWidth = pCell->IsPrcWidthOption();
                sal_uInt16 nWidth = pCell->GetWidthOption();

                // Eine NOWRAP-Option bezieht sich auf Text und auf
                // Tabellen, wird aber bei fester Zellenbreite
                // nicht uebernommen. Stattdessen wirkt die angegebene
                // Zellenbreite wie eine Mindestbreite.
                if( pCell->HasNoWrapOption() )
                {
                    if( nWidth==0 || bRelWidth )
                    {
                        nMinNoAlignCell = nMaxNoAlignCell;
                        nAbsMinNoAlignCell = nMaxNoAlignCell;
                    }
                    else
                    {
                        if( nWidth>nMinNoAlignCell )
                            nMinNoAlignCell = nWidth;
                        if( nWidth>nAbsMinNoAlignCell )
                            nAbsMinNoAlignCell = nWidth;
                    }
                }
#ifdef FIX41370
                else if( bHR && nWidth>0 && !bRelWidth )
                {
                    // Ein kleiner Hack, um einen Bug in Netscape 4.0
                    // nachzubilden (siehe #41370#). Wenn eine Zelle eine
                    // fixe Breite besitzt und gleichzeitig ein HR, wird
                    // sie nie schmaler als die angegebene Breite.
                    // (Genaugenomen scheint die Zelle nie schmaler zu werden
                    // als die HR-Linie, denn wenn man fuer die Linie eine
                    // Breite angibt, die breiter ist als die der Zelle, dann
                    // wird die Zelle so breit wie die Linie. Das bekommen wir
                    // natuerlich nicht hin.)
                    if( nWidth>nMinNoAlignCell )
                        nMinNoAlignCell = nWidth;
                    if( nWidth>nAbsMinNoAlignCell )
                        nAbsMinNoAlignCell = nWidth;
                }
#endif

                // Mindestbreite fuer Inhalt einhalten
                if( nMinNoAlignCell < MINLAY )
                    nMinNoAlignCell = MINLAY;
                if( nMaxNoAlignCell < MINLAY )
                    nMaxNoAlignCell = MINLAY;
                if( nAbsMinNoAlignCell < MINLAY )
                    nAbsMinNoAlignCell = MINLAY;

                // Umrandung und Abstand zum Inhalt beachten.
                AddBorderWidth( nMinNoAlignCell, nMaxNoAlignCell,
                                nAbsMinNoAlignCell, i, nColSpan );

                if( 1==nColSpan )
                {
                    // die Werte direkt uebernehmen
                    pColumn->MergeMinMaxNoAlign( nMinNoAlignCell,
                                                 nMaxNoAlignCell,
                                                 nAbsMinNoAlignCell );

                    // bei den WIDTH angaben gewinnt die breiteste
                    if( !HasColTags() )
                        pColumn->MergeCellWidthOption( nWidth, bRelWidth );
                }
                else
                {
                    // die Angaben erst am Ende, und zwar zeilenweise von
                    // links nach rechts bearbeiten

                    // Wann welche Werte wie uebernommen werden ist weiter
                    // unten erklaert.
                    if( !HasColTags() && nWidth && !bRelWidth )
                    {
                        sal_uLong nAbsWidth = nWidth, nDummy = 0, nDummy2 = 0;
                        AddBorderWidth( nAbsWidth, nDummy, nDummy2,
                                        i, nColSpan, sal_False );

                        if( nAbsWidth >= nMinNoAlignCell )
                        {
                            nMaxNoAlignCell = nAbsWidth;
                            if( HasColsOption() )
                                nMinNoAlignCell = nAbsWidth;
                        }
                        else if( nAbsWidth >= nAbsMinNoAlignCell )
                        {
                            nMaxNoAlignCell = nAbsWidth;
                            nMinNoAlignCell = nAbsWidth;
                        }
                        else
                        {
                            nMaxNoAlignCell = nAbsMinNoAlignCell;
                            nMinNoAlignCell = nAbsMinNoAlignCell;
                        }
                    }
                    else if( HasColsOption() || HasColTags() )
                        nMinNoAlignCell = nAbsMinNoAlignCell;

                    SwHTMLTableLayoutConstraints *pConstr =
                        new SwHTMLTableLayoutConstraints( nMinNoAlignCell,
                            nMaxNoAlignCell, j, i, nColSpan );
                    if( pConstraints )
                        pConstraints = pConstraints->InsertNext( pConstr );
                    else
                        pConstraints = pConstr;
                }
            }
        }

        ASSERT( nMinColSpan>0 && nColSkip>0 && nColSkip <= nMinColSpan,
                "Layout Pass 1: Da werden Spalten vergessen!" );
        ASSERT( nMinColSpan!=USHRT_MAX,
                "Layout Pass 1: unnoetiger Schleifendurchlauf oder Bug" );

        if( 1==nMinColSpan )
        {
            // es gibt Zellen mit COLSPAN 1 und demnach auch sinnvolle
            // Werte in pColumn

            // Werte anhand folgender Tabelle (Netscape 4.0 pv 3) uebernehmen:
            //
            // WIDTH:           kein COLS       COLS
            //
            // keine            min = min       min = absmin
            //                  max = max       max = max
            //
            // >= min           min = min       min = width
            //                  max = width     max = width
            //
            // >= absmin        min = wdith(*)  min = width
            //                  max = width     max = width
            //
            // < absmin         min = absmin    min = absmin
            //                  max = absmin    max = absmin
            //
            // (*) Netscape benutzt hier die Mindestbreite ohne einen
            //     Umbruch vor der letzten Grafik. Haben wir (noch?) nicht,
            //     also belassen wir es bei width.^

            if( pColumn->GetWidthOption() && !pColumn->IsRelWidthOption() )
            {
                // absolute Breiten als Minimal- und Maximalbreite
                // uebernehmen.
                sal_uLong nAbsWidth = pColumn->GetWidthOption();
                sal_uLong nDummy = 0, nDummy2 = 0;
                AddBorderWidth( nAbsWidth, nDummy, nDummy2, i, 1, sal_False );

                if( nAbsWidth >= pColumn->GetMinNoAlign() )
                {
                    pColumn->SetMinMax( HasColsOption() ? nAbsWidth
                                                   : pColumn->GetMinNoAlign(),
                                        nAbsWidth );
                }
                else if( nAbsWidth >= pColumn->GetAbsMinNoAlign() )
                {
                    pColumn->SetMinMax( nAbsWidth, nAbsWidth );
                }
                else
                {
                    pColumn->SetMinMax( pColumn->GetAbsMinNoAlign(),
                                        pColumn->GetAbsMinNoAlign() );
                }
            }
            else
            {
                pColumn->SetMinMax( HasColsOption() ? pColumn->GetAbsMinNoAlign()
                                               : pColumn->GetMinNoAlign(),
                                    pColumn->GetMaxNoAlign() );
            }
        }
        else if( USHRT_MAX!=nMinColSpan )
        {
            // kann irgendwas !=0 sein, weil es durch die Constraints
            // angepasst wird.
            pColumn->SetMinMax( MINLAY, MINLAY );

            // die naechsten Spalten muessen nicht bearbeitet werden
            i += (nColSkip-1);
        }

        nMin += pColumn->GetMin();
        nMax += pColumn->GetMax();
        bFixRelWidths |= pColumn->IsRelWidthOption();
    }

    // jetzt noch die Constrains verarbeiten
    SwHTMLTableLayoutConstraints *pConstr = pConstraints;
    while( pConstr )
    {
        // Erstmal muss die Breite analog zu den den Spaltenbreiten
        // aufbereitet werden
        sal_uInt16 nCol = pConstr->GetColumn();
        sal_uInt16 nColSpan = pConstr->GetColSpan();
        sal_uLong nConstrMin = pConstr->GetMinNoAlign();
        sal_uLong nConstrMax = pConstr->GetMaxNoAlign();

        // jetzt holen wir uns die bisherige Breite der ueberspannten
        // Spalten
        sal_uLong nColsMin = 0;
        sal_uLong nColsMax = 0;
        for( sal_uInt16 j=nCol; j<nCol+nColSpan; j++ )
        {
            SwHTMLTableLayoutColumn *pColumn = GetColumn( j );
            nColsMin += pColumn->GetMin();
            nColsMax += pColumn->GetMax();
        }

        if( nColsMin<nConstrMin )
        {
            // den Minimalwert anteilig auf die Spalten verteilen
            sal_uLong nMinD = nConstrMin-nColsMin;

            if( nConstrMin > nColsMax )
            {
                // Anteilig anhand der Mindestbreiten
                sal_uInt16 nEndCol = nCol+nColSpan;
                sal_uLong nDiff = nMinD;
                for( sal_uInt16 ic=nCol; ic<nEndCol; ic++ )
                {
                    SwHTMLTableLayoutColumn *pColumn = GetColumn( ic );

                    sal_uLong nColMin = pColumn->GetMin();
                    sal_uLong nColMax = pColumn->GetMax();

                    nMin -= nColMin;
                    sal_uLong nAdd = ic<nEndCol-1 ? (nColMin * nMinD) / nColsMin
                                             : nDiff;
                    nColMin += nAdd;
                    nMin += nColMin;
                    ASSERT( nDiff >= nAdd, "Ooops: nDiff stimmt nicht mehr" );
                    nDiff -= nAdd;

                    if( nColMax < nColMin )
                    {
                        nMax -= nColMax;
                        nColsMax -= nColMax;
                        nColMax = nColMin;
                        nMax += nColMax;
                        nColsMax += nColMax;
                    }

                    pColumn->SetMinMax( nColMin, nColMax );
                }
            }
            else
            {
                // Anteilig anhand der Differenz zwischen Max und Min
                for( sal_uInt16 ic=nCol; ic<nCol+nColSpan; ic++ )
                {
                    SwHTMLTableLayoutColumn *pColumn = GetColumn( ic );

                    sal_uLong nDiff = pColumn->GetMax()-pColumn->GetMin();
                    if( nMinD < nDiff )
                        nDiff = nMinD;

                    pColumn->AddToMin( nDiff );

                    ASSERT( pColumn->GetMax() >= pColumn->GetMin(),
                            "Wieso ist die SPalte auf einmal zu schmal?" )

                    nMin += nDiff;
                    nMinD -= nDiff;
                }
            }
        }

        if( !HasColTags() && nColsMax<nConstrMax )
        {
            sal_uLong nMaxD = nConstrMax-nColsMax;

            for( sal_uInt16 ic=nCol; ic<nCol+nColSpan; ic++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( ic );

                nMax -= pColumn->GetMax();

                pColumn->AddToMax( (pColumn->GetMax() * nMaxD) / nColsMax );

                nMax += pColumn->GetMax();
            }
        }

        pConstr = pConstr->GetNext();
    }


    if( bFixRelWidths )
    {
        if( HasColTags() )
        {
            // Zum Anpassen der relativen Breiten werden im 1. Schritt die
            // Minmalbreiten aller anzupassenden Zellen jeweils mit der
            // relativen Breite einer Spalte multipliziert. Dadurch stimmen
            // dann die Breitenverhaeltnisse der Spalten untereinander.
            // Ausserdem wird der Faktor berechnet, um den die Zelle dadurch
            // breiter gworden ist als die Minmalbreite.
            // Im 2. Schritt werden dann die berechneten Breiten durch diesen
            // Faktor geteilt. Dadurch bleibt die Breite (nimd.) einer Zelle
            // erhalten und dient als Ausgangsbasis fuer die andern Breiten.
            // Es werden auch hier nur die Maximalbreiten beeinflusst!

            sal_uLong nAbsMin = 0;  // absolte Min-Breite alter Spalten mit
                                // relativer Breite
            sal_uLong nRel = 0;     // Summe der relativen Breiten aller Spalten
            for( i=0; i<nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() )
                {
                    nAbsMin += pColumn->GetMin();
                    nRel += pColumn->GetWidthOption();
                }
            }

            sal_uLong nQuot = ULONG_MAX;
            for( i=0; i<nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() )
                {
                    nMax -= pColumn->GetMax();
                    if( pColumn->GetWidthOption() && pColumn->GetMin() )
                    {
                        pColumn->SetMax( nAbsMin * pColumn->GetWidthOption() );
                        sal_uLong nColQuot = pColumn->GetMax() / pColumn->GetMin();
                        if( nColQuot<nQuot )
                            nQuot = nColQuot;
                    }
                }
            }
            ASSERT( 0==nRel || nQuot!=ULONG_MAX,
                    "Wo sind die relativen Spalten geblieben?" );
            for( i=0; i<nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() )
                {
                    if( pColumn->GetWidthOption() )
                        pColumn->SetMax( pColumn->GetMax() / nQuot );
                    else
                        pColumn->SetMax( pColumn->GetMin() );
                    ASSERT( pColumn->GetMax() >= pColumn->GetMin(),
                            "Maximale Spaltenbreite kleiner als Minimale" );
                    nMax += pColumn->GetMax();
                }
            }
        }
        else
        {
            sal_uInt16 nRel = 0;        // Summe der relativen Breiten aller Spalten
            sal_uInt16 nRelCols = 0;    // Anzahl Spalten mit relativer Angabe
            sal_uLong nRelMax = 0;      // Anteil am Maximum dieser Spalten
            for( i=0; i<nCols; i++ )
            {
                ASSERT( nRel<=100, "relative Breite aller Spalten>100%" );
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() )
                {
                    // Sicherstellen, dass die relativen breiten nicht
                    // ueber 100% landen
                    sal_uInt16 nColWidth = pColumn->GetWidthOption();
                    if( nRel+nColWidth > 100 )
                    {
                        nColWidth = 100 - nRel;
                        pColumn->SetWidthOption( nColWidth, sal_True, sal_False );
                    }
                    nRelMax += pColumn->GetMax();
                    nRel = nRel + nColWidth;
                    nRelCols++;
                }
                else if( !pColumn->GetMin() )
                {
                    // Die Spalte ist leer (wurde also auschliesslich
                    // durch COLSPAN erzeugt) und darf deshalb auch
                    // keine %-Breite zugewiesen bekommen.
                    nRelCols++;
                }
            }

            // Eventuell noch vorhandene Prozente werden auf die Spalten ohne
            // eine Breiten-Angabe verteilt. Wie in Netscape werden die
            // verbleibenden Prozente enstprechend der Verhaeltnisse
            // der Maximalbreiten der in Frage kommenden Spalten
            // untereinander verteilt.
            // ??? Wie beruecksichtigen bei den Maximalbreiten auch Spalten
            // mit fester Breite. Ist das richtig???
            if( nRel < 100 && nRelCols < nCols )
            {
                sal_uInt16 nRelLeft = 100 - nRel;
                sal_uLong nFixMax = nMax - nRelMax;
                for( i=0; i<nCols; i++ )
                {
                    SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                    if( !pColumn->IsRelWidthOption() &&
                        !pColumn->GetWidthOption() &&
                        pColumn->GetMin() )
                    {
                        // den Rest bekommt die naechste Spalte
                        sal_uInt16 nColWidth =
                            (sal_uInt16)((pColumn->GetMax() * nRelLeft) / nFixMax);
                        pColumn->SetWidthOption( nColWidth, sal_True, sal_False );
                    }
                }
            }

            // nun die Maximalbreiten entsprechend anpassen
            sal_uLong nQuotMax = ULONG_MAX;
            sal_uLong nOldMax = nMax;
            nMax = 0;
            for( i=0; i<nCols; i++ )
            {
                // Spalten mit %-Angaben werden enstprechend angepasst.
                // Spalten, die
                // - keine %-Angabe besitzen und in einer Tabelle mit COLS
                //   oder WIDTH vorkommen, oder
                // - als Breite 0% angegeben haben erhalten die Minimalbreite
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() )
                {
                    sal_uLong nNewMax;
                    sal_uLong nColQuotMax;
                    if( !nWidthOption )
                    {
                        nNewMax = nOldMax * pColumn->GetWidthOption();
                        nColQuotMax = nNewMax / pColumn->GetMax();
                    }
                    else
                    {
                        nNewMax = nMin * pColumn->GetWidthOption();
                        nColQuotMax = nNewMax / pColumn->GetMin();
                    }
                    pColumn->SetMax( nNewMax );
                    if( nColQuotMax < nQuotMax )
                        nQuotMax = nColQuotMax;
                }
                else if( HasColsOption() || nWidthOption ||
                         (pColumn->IsRelWidthOption() &&
                          !pColumn->GetWidthOption()) )
                    pColumn->SetMax( pColumn->GetMin() );
            }
            // und durch den Quotienten teilen
            ASSERT( nQuotMax!=ULONG_MAX, "Wo sind die relativen Spalten geblieben?" );
            for( i=0; i<nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() )
                {
                    if( pColumn->GetWidthOption() )
                    {
                        pColumn->SetMax( pColumn->GetMax() / nQuotMax );
                        ASSERT( pColumn->GetMax() >= pColumn->GetMin(),
                                "Minimalbreite ein Spalte Groesser Maximum" );
                        if( pColumn->GetMax() < pColumn->GetMin() )
                            pColumn->SetMax( pColumn->GetMin() );
                    }
                }
                nMax += pColumn->GetMax();
            }
        }
    }

    delete pConstraints;
}

// nAbsAvail ist der verfuegbare Platz in TWIPS.
// nRelAvail ist der auf USHRT_MAX bezogene verfuegbare Platz oder 0
// nAbsSpace ist der Anteil von nAbsAvail, der durch der umgebende Zelle
//           fur die Umrandung und den Abstand zum Inhalt reserviert ist.
void SwHTMLTableLayout::AutoLayoutPass2( sal_uInt16 nAbsAvail, sal_uInt16 nRelAvail,
                                         sal_uInt16 nAbsLeftSpace,
                                         sal_uInt16 nAbsRightSpace,
                                         sal_uInt16 nParentInhAbsSpace )
{
    // Erstmal fuehren wie jede Menge Plausibilaets-Test durch

    // Eine abolute zur Verfuegung stehende Breite muss immer uebergeben
    // werden.
    ASSERT( nAbsAvail, "AutoLayout Pass 2: Keine absolute Breite gegeben" );

    // Eine realtive zur Verfuegung stehende Breite darf nur und muss fuer
    // Tabellen in Tabellen uebergeben
    ASSERT( IsTopTable() == (nRelAvail==0),
            "AutoLayout Pass 2: Rel. Breite bei Tab in Tab oder umgekehrt" );

    // Die Minimalbreite der Tabelle darf natuerlich nie groesser sein
    // als das die Maximalbreite.
    ASSERT( nMin<=nMax, "AutoLayout Pass2: nMin > nMax" );

    // Die verfuegbare Breite, fuer die die Tabelle berechnet wurde, merken.
    // (Dies ist ein guter Ort, denn hier kommer wir bei der Erstberechnung
    // der Tabelle aus dem Parser und bei jedem _Resize-Aufruf vorbei.)
    nLastResizeAbsAvail = nAbsAvail;

    // Schritt 1: Der verfuegbar Platz wird an linke/rechte Raender,
    // vorhandene Filler-Zellen und Abstande angepasst

    // Abstand zum Inhalt und Unrandung
    sal_uInt16 nAbsLeftFill = 0, nAbsRightFill = 0;
    if( !IsTopTable() &&
        GetMin() + nAbsLeftSpace + nAbsRightSpace <= nAbsAvail )
    {
        nAbsLeftFill = nAbsLeftSpace;
        nAbsRightFill = nAbsRightSpace;
    }

    // Linker und rechter Abstand
    if( nLeftMargin || nRightMargin )
    {
        if( IsTopTable() )
        {
            // fuer die Top-Table beruecksichtigen wir die Raender immer,
            // den die Minimalbreite der Tabelle wird hier nie unterschritten
            nAbsAvail -= (nLeftMargin + nRightMargin);
        }
        else if( GetMin() + nLeftMargin + nRightMargin <= nAbsAvail )
        {
            // sonst beruecksichtigen wir die Raender nur, wenn auch Platz
            // fuer sie da ist (nMin ist hier bereits berechnet!)
            nAbsLeftFill = nAbsLeftFill + nLeftMargin;
            nAbsRightFill = nAbsRightFill + nRightMargin;
        }
    }

    // Filler-Zellen
    if( !IsTopTable() )
    {
        if( pLeftFillerBox && nAbsLeftFill<MINLAY+nInhLeftBorderWidth )
            nAbsLeftFill = MINLAY+nInhLeftBorderWidth;
        if( pRightFillerBox && nAbsRightFill<MINLAY+nInhRightBorderWidth )
            nAbsRightFill = MINLAY+nInhRightBorderWidth;
    }

    // Anpassen des verfuegbaren Platzes.
    nRelLeftFill = 0;
    nRelRightFill = 0;
    if( !IsTopTable() && (nAbsLeftFill>0 || nAbsRightFill) )
    {
        sal_uLong nAbsLeftFillL = nAbsLeftFill, nAbsRightFillL = nAbsRightFill;

        nRelLeftFill = (sal_uInt16)((nAbsLeftFillL * nRelAvail) / nAbsAvail);
        nRelRightFill = (sal_uInt16)((nAbsRightFillL * nRelAvail) / nAbsAvail);

        nAbsAvail -= (nAbsLeftFill + nAbsRightFill);
        if( nRelAvail )
            nRelAvail -= (nRelLeftFill + nRelRightFill);
    }


    // Schritt 2: Die absolute Tabellenbreite wird berechnet.
    sal_uInt16 nAbsTabWidth = 0;
    bUseRelWidth = sal_False;
    if( nWidthOption )
    {
        if( bPrcWidthOption )
        {
            ASSERT( nWidthOption<=100, "Prozentangabe zu gross" );
            if( nWidthOption > 100 )
                nWidthOption = 100;

            // Die absolute Breite entspricht den angegeben Prozent der
            // zur Verfuegung stehenden Breite.
            // Top-Tabellen bekommen nur eine relative Breite, wenn der
            // verfuegbare Platz *echt groesser* ist als die Minimalbreite.
            // ACHTUNG: Das "echte groesser" ist noetig, weil der Wechsel
            // von einer relativen Breite zu einer absoluten Breite durch
            // Resize sonst zu einer Endlosschleife fuehrt.
            // Weil bei Tabellen in Rahmen kein Resize aufgerufen wird,
            // wenn der Rahmen eine nicht-relative Breite besitzt, koennen
            // wir da solche Spielchen nicht spielen
            // MIB 19.2.98: Wegen fix #47394# spielen wir solche Spielchen
            // jetzt doch. Dort war eine Grafik in einer 1%-breiten
            // Tabelle und hat da natuerlich nicht hineingepasst.
            nAbsTabWidth = (sal_uInt16)( ((sal_uLong)nAbsAvail * nWidthOption) / 100 );
            if( IsTopTable() &&
                ( /*MayBeInFlyFrame() ||*/ (sal_uLong)nAbsTabWidth > nMin ) )
            {
                nRelAvail = USHRT_MAX;
                bUseRelWidth = sal_True;
            }
        }
        else
        {
            nAbsTabWidth = nWidthOption;
            if( nAbsTabWidth > MAX_TABWIDTH )
                nAbsTabWidth = MAX_TABWIDTH;

            // Tabellen in Tabellen duerfen niemals breiter werden als der
            // verfuegbare Platz.
            if( !IsTopTable() && nAbsTabWidth > nAbsAvail )
                nAbsTabWidth = nAbsAvail;
        }
    }

    ASSERT( IsTopTable() || nAbsTabWidth<=nAbsAvail,
            "AutoLayout Pass2: nAbsTabWidth > nAbsAvail fuer Tab in Tab" );
    ASSERT( !nRelAvail || nAbsTabWidth<=nAbsAvail,
            "AutoLayout Pass2: nAbsTabWidth > nAbsAvail fuer relative Breite" );

    // Catch fuer die beiden Asserts von oben (man weiss ja nie!)
    if( (!IsTopTable() || nRelAvail>0) && nAbsTabWidth>nAbsAvail )
        nAbsTabWidth = nAbsAvail;


    // Schritt 3: Bestimmen der Spaltenbreiten und ggf. auch der
    // absoluten und relativen Tabellenbreiten.
    if( (!IsTopTable() && nMin > (sal_uLong)nAbsAvail) ||
        nMin > MAX_TABWIDTH )
    {
        // Wenn
        // - das Minumum einer inneren Tabelle groesser ist als der
        //   verfuegbare Platz, oder
        // - das Minumum einer Top-Table groesser ist als USHRT_MAX
        // muss die Tabelle an den verfuegbaren Platz bzw. USHRT_MAX
        // abgepasst werden. Dabei bleiben die Verhaeltnisse der Breiten
        // untereinander erhalten.

        nAbsTabWidth = IsTopTable() ? MAX_TABWIDTH : nAbsAvail;
        nRelTabWidth = (nRelAvail ? nRelAvail : nAbsTabWidth );

        // First of all, we check wether we can fit the layout constrains,
        // that are: Every cell's width excluding the borders must be at least
        // MINLAY:

        sal_uLong nRealMin = 0;
        for( sal_uInt16 i=0; i<nCols; i++ )
        {
            sal_uLong nRealColMin = MINLAY, nDummy1, nDummy2;
            AddBorderWidth( nRealColMin, nDummy1, nDummy2, i, 1 );
            nRealMin += nRealColMin;
        }
        if( (nRealMin >= nAbsTabWidth) || (nRealMin >= nMin) )
        {
            // "Nichts geht mehr". We cannot get the minimum column widths
            // the layout wants to have.

            sal_uInt16 nAbs = 0, nRel = 0;
            SwHTMLTableLayoutColumn *pColumn;
            for( sal_uInt16 i=0; i<nCols-1; i++ )
            {
                pColumn = GetColumn( i );
                sal_uLong nColMin = pColumn->GetMin();
                if( nColMin <= USHRT_MAX )
                {
                    pColumn->SetAbsColWidth(
                        (sal_uInt16)((nColMin * nAbsTabWidth) / nMin) );
                    pColumn->SetRelColWidth(
                        (sal_uInt16)((nColMin * nRelTabWidth) / nMin) );
                }
                else
                {
                    double nColMinD = nColMin;
                    pColumn->SetAbsColWidth(
                        (sal_uInt16)((nColMinD * nAbsTabWidth) / nMin) );
                    pColumn->SetRelColWidth(
                        (sal_uInt16)((nColMinD * nRelTabWidth) / nMin) );
                }

                nAbs = nAbs + (sal_uInt16)pColumn->GetAbsColWidth();
                nRel = nRel + (sal_uInt16)pColumn->GetRelColWidth();
            }
            pColumn = GetColumn( nCols-1 );
            pColumn->SetAbsColWidth( nAbsTabWidth - nAbs );
            pColumn->SetRelColWidth( nRelTabWidth - nRel );
        }
        else
        {
            sal_uLong nDistAbs = nAbsTabWidth - nRealMin;
            sal_uLong nDistRel = nRelTabWidth - nRealMin;
            sal_uLong nDistMin = nMin - nRealMin;
            sal_uInt16 nAbs = 0, nRel = 0;
            SwHTMLTableLayoutColumn *pColumn;
            for( sal_uInt16 i=0; i<nCols-1; i++ )
            {
                pColumn = GetColumn( i );
                sal_uLong nColMin = pColumn->GetMin();
                sal_uLong nRealColMin = MINLAY, nDummy1, nDummy2;
                AddBorderWidth( nRealColMin, nDummy1, nDummy2, i, 1 );

                if( nColMin <= USHRT_MAX )
                {
                    pColumn->SetAbsColWidth(
                        (sal_uInt16)((((nColMin-nRealColMin) * nDistAbs) / nDistMin) + nRealColMin) );
                    pColumn->SetRelColWidth(
                        (sal_uInt16)((((nColMin-nRealColMin) * nDistRel) / nDistMin) + nRealColMin) );
                }
                else
                {
                    double nColMinD = nColMin;
                    pColumn->SetAbsColWidth(
                        (sal_uInt16)((((nColMinD-nRealColMin) * nDistAbs) / nDistMin) + nRealColMin) );
                    pColumn->SetRelColWidth(
                        (sal_uInt16)((((nColMinD-nRealColMin) * nDistRel) / nDistMin) + nRealColMin) );
                }

                nAbs = nAbs + (sal_uInt16)pColumn->GetAbsColWidth();
                nRel = nRel + (sal_uInt16)pColumn->GetRelColWidth();
            }
            pColumn = GetColumn( nCols-1 );
            pColumn->SetAbsColWidth( nAbsTabWidth - nAbs );
            pColumn->SetRelColWidth( nRelTabWidth - nRel );
        }
    }
    else if( nMax <= (sal_uLong)(nAbsTabWidth ? nAbsTabWidth : nAbsAvail) )
    {
        // Wenn
        // - die Tabelle eine fixe Breite besitzt und das Maximum der
        //   Tabelle kleiner ist, oder
        // - das Maximum kleiner ist als der verfuegbare Platz
        // kann das Maximum direkt uebernommen werden bzw. die Tabelle nur
        // unter Beruecksichtigung des Maxumums an die fixe Breite
        // angepasst werden.

        // Keine fixe Breite, dann das Maximum nehmen.
        if( !nAbsTabWidth )
            nAbsTabWidth = (sal_uInt16)nMax;

        // Eine Top-Table darf auch beriter werden als der verfuegbare Platz.
        if( nAbsTabWidth > nAbsAvail )
        {
            ASSERT( IsTopTable(),
                    "Tabelle in Tabelle soll breiter werden als umgebende Zelle" );
            nAbsAvail = nAbsTabWidth;
        }

        // Nur den Anteil der relativen Breite verwenden, der auch fuer
        // die absolute Breite verwendet wuerde.
        sal_uLong nAbsTabWidthL = nAbsTabWidth;
        nRelTabWidth =
            ( nRelAvail ? (sal_uInt16)((nAbsTabWidthL * nRelAvail) / nAbsAvail)
                        : nAbsTabWidth );

        // Gibt es Spalten mit und Spalten ohne %-Angabe?
        sal_uLong nFixMax = nMax;
        for( sal_uInt16 i=0; i<nCols; i++ )
        {
            const SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
            if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption()>0 )
                nFixMax -= pColumn->GetMax();
        }

        if( nFixMax > 0 && nFixMax < nMax )
        {
            // ja, dann den zu verteilenden Platz nur auf die Spalten
            // mit %-Angabe verteilen.

            // In diesem (und nur in diesem) Fall gibt es Spalten,
            // die ihre Maximalbreite genau einhalten, also weder
            // schmaler noch breiter werden. Beim zurueckrechnen der
            // absoluten Breite aus der relativen Breite kann es
            // zu Rundungsfehlern kommen (bug #45598#). Um die auszugeleichen
            // werden zuerst die fixen Breiten entsprechend korrigiert
            // eingestellt und erst danach die relativen.

            sal_uInt16 nAbs = 0, nRel = 0;
            sal_uInt16 nFixedCols = 0;
            sal_uInt16 i;

            for( i = 0; i < nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( !pColumn->IsRelWidthOption() || !pColumn->GetWidthOption() )
                {
                    // Die Spalte behaelt ihre Breite bei.
                    nFixedCols++;
                    sal_uLong nColMax = pColumn->GetMax();
                    pColumn->SetAbsColWidth( (sal_uInt16)nColMax );

                    sal_uLong nRelColWidth =
                        (nColMax * nRelTabWidth) / nAbsTabWidth;
                    sal_uLong nChkWidth =
                        (nRelColWidth * nAbsTabWidth) / nRelTabWidth;
                    if( nChkWidth < nColMax )
                        nRelColWidth++;
                    else if( nChkWidth > nColMax )
                        nRelColWidth--;
                    pColumn->SetRelColWidth( (sal_uInt16)nRelColWidth );

                    nAbs = nAbs + (sal_uInt16)nColMax;
                    nRel = nRel + (sal_uInt16)nRelColWidth;
                }
            }

            // Zu verteilende Anteile des Maximums und der relativen und
            // absoluten Breiten. nFixMax entspricht an dieser Stelle
            // nAbs, so dass man gleich nFixMax haette nehmen koennen.
            // Der Code ist so aber verstaendlicher.
            ASSERT( nFixMax == nAbs, "Zwei Schleifen, zwei Summen?" )
            sal_uLong nDistMax = nMax - nFixMax;
            sal_uInt16 nDistAbsTabWidth = nAbsTabWidth - nAbs;
            sal_uInt16 nDistRelTabWidth = nRelTabWidth - nRel;

            for( i=0; i<nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() > 0 )
                {
                    // Die Spalte wird anteilig breiter.
                    nFixedCols++;
                    if( nFixedCols == nCols )
                    {
                        pColumn->SetAbsColWidth( nAbsTabWidth-nAbs );
                        pColumn->SetRelColWidth( nRelTabWidth-nRel );
                    }
                    else
                    {
                        sal_uLong nColMax = pColumn->GetMax();
                        pColumn->SetAbsColWidth(
                            (sal_uInt16)((nColMax * nDistAbsTabWidth) / nDistMax) );
                        pColumn->SetRelColWidth(
                            (sal_uInt16)((nColMax * nDistRelTabWidth) / nDistMax) );
                    }
                    nAbs = nAbs + pColumn->GetAbsColWidth();
                    nRel = nRel + pColumn->GetRelColWidth();
                }
            }
            ASSERT( nCols==nFixedCols, "Spalte vergessen!" );
        }
        else
        {
            // nein, dann den zu verteilenden Platz auf alle Spalten
            // gleichmaessig vertilen.
            for( sal_uInt16 i=0; i<nCols; i++ )
            {
                sal_uLong nColMax = GetColumn( i )->GetMax();
                GetColumn( i )->SetAbsColWidth(
                    (sal_uInt16)((nColMax * nAbsTabWidth) / nMax) );
                GetColumn( i )->SetRelColWidth(
                    (sal_uInt16)((nColMax * nRelTabWidth) / nMax) );
            }
        }
    }
    else
    {
        // den ueber die Minimalbreite herausgehenden Platz entsprechend
        // den einzelnen Spalten anteilig zuschlagen
        if( !nAbsTabWidth )
            nAbsTabWidth = nAbsAvail;
        if( nAbsTabWidth < nMin )
            nAbsTabWidth = (sal_uInt16)nMin;

        if( nAbsTabWidth > nAbsAvail )
        {
            ASSERT( IsTopTable(),
                    "Tabelle in Tabelle soll breiter werden als Platz da ist" );
            nAbsAvail = nAbsTabWidth;
        }

        sal_uLong nAbsTabWidthL = nAbsTabWidth;
        nRelTabWidth =
            ( nRelAvail ? (sal_uInt16)((nAbsTabWidthL * nRelAvail) / nAbsAvail)
                        : nAbsTabWidth );
        double nW = nAbsTabWidth - nMin;
        double nD = (nMax==nMin ? 1 : nMax-nMin);
        sal_uInt16 nAbs = 0, nRel = 0;
        for( sal_uInt16 i=0; i<nCols-1; i++ )
        {
            double nd = GetColumn( i )->GetMax() - GetColumn( i )->GetMin();
            sal_uLong nAbsColWidth = GetColumn( i )->GetMin() + (sal_uLong)((nd*nW)/nD);
            sal_uLong nRelColWidth = nRelAvail
                                    ? (nAbsColWidth * nRelTabWidth) / nAbsTabWidth
                                    : nAbsColWidth;

            GetColumn( i )->SetAbsColWidth( (sal_uInt16)nAbsColWidth );
            GetColumn( i )->SetRelColWidth( (sal_uInt16)nRelColWidth );
            nAbs = nAbs + (sal_uInt16)nAbsColWidth;
            nRel = nRel + (sal_uInt16)nRelColWidth;
        }
        GetColumn( nCols-1 )->SetAbsColWidth( nAbsTabWidth - nAbs );
        GetColumn( nCols-1 )->SetRelColWidth( nRelTabWidth - nRel );

    }

    // Schritt 4: Fuer Tabellen in Tabellen kann es links und/oder rechts
    // noch Ausgleichzellen geben. Deren Breite wird jetzt berechnet.
    nInhAbsLeftSpace = 0;
    nInhAbsRightSpace = 0;
    if( !IsTopTable() && (nRelLeftFill>0 || nRelRightFill>0 ||
                          nAbsTabWidth<nAbsAvail) )
    {
        // Die Breite von zusaetzlichen Zellen zur Ausrichtung der
        // inneren Tabelle bestimmen
        sal_uInt16 nAbsDist = (sal_uInt16)(nAbsAvail-nAbsTabWidth);
        sal_uInt16 nRelDist = (sal_uInt16)(nRelAvail-nRelTabWidth);
        sal_uInt16 nParentInhAbsLeftSpace = 0, nParentInhAbsRightSpace = 0;

        // Groesse und Position der zusaetzlichen Zellen bestimmen
        switch( eTableAdjust )
        {
        case SVX_ADJUST_RIGHT:
            nAbsLeftFill = nAbsLeftFill + nAbsDist;
            nRelLeftFill = nRelLeftFill + nRelDist;
            nParentInhAbsLeftSpace = nParentInhAbsSpace;
            break;
        case SVX_ADJUST_CENTER:
            {
                sal_uInt16 nAbsLeftDist = nAbsDist / 2;
                nAbsLeftFill = nAbsLeftFill + nAbsLeftDist;
                nAbsRightFill += nAbsDist - nAbsLeftDist;
                sal_uInt16 nRelLeftDist = nRelDist / 2;
                nRelLeftFill = nRelLeftFill + nRelLeftDist;
                nRelRightFill += nRelDist - nRelLeftDist;
                nParentInhAbsLeftSpace = nParentInhAbsSpace / 2;
                nParentInhAbsRightSpace = nParentInhAbsSpace -
                                          nParentInhAbsLeftSpace;
            }
            break;
        case SVX_ADJUST_LEFT:
        default:
            nAbsRightFill = nAbsRightFill + nAbsDist;
            nRelRightFill = nRelRightFill + nRelDist;
            nParentInhAbsRightSpace = nParentInhAbsSpace;
            break;
        }

        ASSERT( !pLeftFillerBox || nRelLeftFill>0,
                "Fuer linke Filler-Box ist keine Breite da!" );
        ASSERT( !pRightFillerBox || nRelRightFill>0,
                "Fuer rechte Filler-Box ist keine Breite da!" );

        // Filler-Breiten werden auf die ausseren Spalten geschlagen, wenn
        // es nach dem ersten Durchlauf keine Boxen fuer sie gibt (nWidth>0)
        // oder ihre Breite zu klein wuerde oder wenn es COL-Tags gibt und
        // die Filler-Breite der Umrandung-Breite entspricht (dann haben wir
        // die Tabelle wahrscheinlich selbst exportiert)
        if( nRelLeftFill && !pLeftFillerBox &&
            ( nWidthSet>0 || nAbsLeftFill<MINLAY+nInhLeftBorderWidth ||
              (HasColTags() && nAbsLeftFill < nAbsLeftSpace+nParentInhAbsLeftSpace+20) ) )
//          (nAbsLeftFill<MINLAY || nAbsLeftFill<=nAbsLeftSpace) )
        {
            SwHTMLTableLayoutColumn *pColumn = GetColumn( 0 );
            pColumn->SetAbsColWidth( pColumn->GetAbsColWidth()+nAbsLeftFill );
            pColumn->SetRelColWidth( pColumn->GetRelColWidth()+nRelLeftFill );
            nRelLeftFill = 0;
            nInhAbsLeftSpace = nAbsLeftSpace + nParentInhAbsLeftSpace;
        }
        if( nRelRightFill && !pRightFillerBox &&
            ( nWidthSet>0 || nAbsRightFill<MINLAY+nInhRightBorderWidth ||
              (HasColTags() && nAbsRightFill < nAbsRightSpace+nParentInhAbsRightSpace+20) ) )
//          (nAbsRightFill<MINLAY || nAbsRightFill<=nAbsRightSpace) )
        {
            SwHTMLTableLayoutColumn *pColumn = GetColumn( nCols-1 );
            pColumn->SetAbsColWidth( pColumn->GetAbsColWidth()+nAbsRightFill );
            pColumn->SetRelColWidth( pColumn->GetRelColWidth()+nRelRightFill );
            nRelRightFill = 0;
            nInhAbsRightSpace = nAbsRightSpace + nParentInhAbsRightSpace;
        }
    }
}

static sal_Bool lcl_ResizeLine( const SwTableLine*& rpLine, void* pPara );

static sal_Bool lcl_ResizeBox( const SwTableBox*& rpBox, void* pPara )
{
    sal_uInt16 *pWidth = (sal_uInt16 *)pPara;

    if( !rpBox->GetSttNd() )
    {
        sal_uInt16 nWidth = 0;
        ((SwTableBox *)rpBox)->GetTabLines().ForEach( &lcl_ResizeLine, &nWidth );
        rpBox->GetFrmFmt()->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth, 0 ));
        *pWidth = *pWidth + nWidth;
    }
    else
    {
        *pWidth = *pWidth + (sal_uInt16)rpBox->GetFrmFmt()->GetFrmSize().GetSize().Width();
    }

    return sal_True;
}

static sal_Bool lcl_ResizeLine( const SwTableLine*& rpLine, void* pPara )
{
    sal_uInt16 *pWidth = (sal_uInt16 *)pPara;
#ifdef DBG_UTIL
    sal_uInt16 nOldWidth = *pWidth;
#endif
    *pWidth = 0;
    ((SwTableLine *)rpLine)->GetTabBoxes().ForEach( &lcl_ResizeBox, pWidth );

#ifdef DBG_UTIL
    ASSERT( !nOldWidth || Abs(*pWidth-nOldWidth) < COLFUZZY,
            "Zeilen einer Box sind unterschiedlich lang" );
#endif

    return sal_True;
}

void SwHTMLTableLayout::SetWidths( sal_Bool bCallPass2, sal_uInt16 nAbsAvail,
                                   sal_uInt16 nRelAvail, sal_uInt16 nAbsLeftSpace,
                                   sal_uInt16 nAbsRightSpace,
                                   sal_uInt16 nParentInhAbsSpace )
{
    // SetWidth muss am Ende einmal mehr fuer jede Zelle durchlaufen
    // worden sein.
    nWidthSet++;

    // Schritt 0: Wenn noetig, wird hier noch der Pass2 des Layout-Alogithmus
    // aufgerufen.
    if( bCallPass2 )
        AutoLayoutPass2( nAbsAvail, nRelAvail, nAbsLeftSpace, nAbsRightSpace,
                         nParentInhAbsSpace );

    // Schritt 1: Setzten der neuen Breite an allen Content-Boxen.
    // Da die Boxen nichts von der HTML-Tabellen-Struktur wissen, wird
    // ueber die HTML-Tabellen-Struktur iteriert. Fuer Tabellen in Tabellen
    // in Tabellen wird rekursiv SetWidth aufgerufen.
    for( sal_uInt16 i=0; i<nRows; i++ )
    {
        for( sal_uInt16 j=0; j<nCols; j++ )
        {
            SwHTMLTableLayoutCell *pCell = GetCell( i, j );

            SwHTMLTableLayoutCnts* pCntnts = pCell->GetContents();
            while( pCntnts && !pCntnts->IsWidthSet(nWidthSet) )
            {
                SwTableBox *pBox = pCntnts->GetTableBox();
                if( pBox )
                {
                    SetBoxWidth( pBox, j, pCell->GetColSpan() );
                }
                else
                {
                    sal_uInt16 nAbs = 0, nRel = 0, nLSpace = 0, nRSpace = 0,
                           nInhSpace = 0;
                    if( bCallPass2 )
                    {
                        sal_uInt16 nColSpan = pCell->GetColSpan();
                        GetAvail( j, nColSpan, nAbs, nRel );
                        nLSpace = GetLeftCellSpace( j, nColSpan );
                        nRSpace = GetRightCellSpace( j, nColSpan );
                        nInhSpace = GetInhCellSpace( j, nColSpan );
                    }
                    pCntnts->GetTable()->SetWidths( bCallPass2, nAbs, nRel,
                                                    nLSpace, nRSpace,
                                                    nInhSpace );
                }

                pCntnts->SetWidthSet( nWidthSet );
                pCntnts = pCntnts->GetNext();
            }
        }
    }

    // Schritt 2: Wenn eine Top-Tabelle vorliegt, werden jetzt die Formate
    // der Nicht-Content-Boxen angepasst. Da diese aufgrund der
    // Garbage-Collection in der HTML-Tabelle nicht bekannt sind, muessen
    // wir hier ueber die Tabelle iterieren. Bei der Gelegenheit wird auch
    // das Tabellen-Frameformat angepasst. Fuer Tabellen in Tabellen werden
    // stattdessen die Breiten der Filler-Zellen gesetzt.
    if( IsTopTable() )
    {
        sal_uInt16 nCalcTabWidth = 0;
        ((SwTable *)pSwTable)->GetTabLines().ForEach( &lcl_ResizeLine,
                                                      &nCalcTabWidth );
        ASSERT( Abs( nRelTabWidth-nCalcTabWidth ) < COLFUZZY,
                "Tabellebreite stimmt nicht mit Zeilenbreite ueberein." );

        // Beim Anpassen des Tabellen-Formats dieses locken, weil sonst
        // die Boxformate erneut angepasst werden. Ausserdem muss eine
        // evtl. vorhandene %-Angabe in jedem Fall erhalten bleiben.
        SwFrmFmt *pFrmFmt = pSwTable->GetFrmFmt();
        ((SwTable *)pSwTable)->LockModify();
        SwFmtFrmSize aFrmSize( pFrmFmt->GetFrmSize() );
        aFrmSize.SetWidth( nRelTabWidth );
        sal_Bool bRel = bUseRelWidth &&
                    text::HoriOrientation::FULL!=pFrmFmt->GetHoriOrient().GetHoriOrient();
        aFrmSize.SetWidthPercent( (sal_uInt8)(bRel ? nWidthOption : 0) );
        pFrmFmt->SetFmtAttr( aFrmSize );
        ((SwTable *)pSwTable)->UnlockModify();

        // Wenn die Tabelle in einem Rahmen steht, muss auch noch dessen
        // breite angepasst werden.
        if( MayBeInFlyFrame() )
        {
            SwFrmFmt *pFlyFrmFmt = FindFlyFrmFmt();
            if( pFlyFrmFmt )
            {
                SwFmtFrmSize aFlyFrmSize( ATT_VAR_SIZE, nRelTabWidth, MINLAY );

                if( bUseRelWidth )
                {
                    // Bei %-Angaben wird die Breite auf das Minimum gesetzt.
                    aFlyFrmSize.SetWidth(  nMin > USHRT_MAX ? USHRT_MAX
                                                            : nMin );
                    aFlyFrmSize.SetWidthPercent( (sal_uInt8)nWidthOption );
                }
                pFlyFrmFmt->SetFmtAttr( aFlyFrmSize );
            }
        }

#ifdef DBG_UTIL
        {
            // steht im tblrwcl.cxx
            extern void _CheckBoxWidth( const SwTableLine&, SwTwips );

            // checke doch mal ob die Tabellen korrekte Breiten haben
            SwTwips nSize = pSwTable->GetFrmFmt()->GetFrmSize().GetWidth();
            const SwTableLines& rLines = pSwTable->GetTabLines();
            for( sal_uInt16 n = 0; n < rLines.Count(); ++n  )
                _CheckBoxWidth( *rLines[ n ], nSize );
        }
#endif

    }
    else
    {
        if( pLeftFillerBox )
        {
            pLeftFillerBox->GetFrmFmt()->SetFmtAttr(
                SwFmtFrmSize( ATT_VAR_SIZE, nRelLeftFill, 0 ));
        }
        if( pRightFillerBox )
        {
            pRightFillerBox->GetFrmFmt()->SetFmtAttr(
                SwFmtFrmSize( ATT_VAR_SIZE, nRelRightFill, 0 ));
        }
    }
}

void SwHTMLTableLayout::_Resize( sal_uInt16 nAbsAvail, sal_Bool bRecalc )
{
    // Wenn bRecalc gestzt ist, hat sich am Inhalt der Tabelle etwas
    // geaendert. Es muss dann der erste Pass noch einmal durchgefuehrt
    // werden.
    if( bRecalc )
        AutoLayoutPass1();

    SwRootFrm *pRoot = (SwRootFrm*)GetDoc()->GetRootFrm();
    if ( pRoot && pRoot->IsCallbackActionEnabled() )
        pRoot->StartAllAction();

    // Sonst koennen die Breiten gesetzt werden, wobei zuvor aber jewils
    // noch der Pass 2 laufen muss.
    SetWidths( sal_True, nAbsAvail );

    if ( pRoot && pRoot->IsCallbackActionEnabled() )
        pRoot->EndAllAction( sal_True );    //True per VirDev (Browsen ruhiger)
}

IMPL_STATIC_LINK( SwHTMLTableLayout, DelayedResize_Impl, void*, EMPTYARG )
{
#ifdef TEST_DELAYED_RESIZE
    Sound::Beep( SOUND_WARNING );
#endif
    pThis->aResizeTimer.Stop();
    pThis->_Resize( pThis->nDelayedResizeAbsAvail,
                    pThis->bDelayedResizeRecalc );

    return 0;
}


sal_Bool SwHTMLTableLayout::Resize( sal_uInt16 nAbsAvail, sal_Bool bRecalc,
                                sal_Bool bForce, sal_uLong nDelay )
{
    if( 0 == nAbsAvail )
        return sal_False;
    ASSERT( IsTopTable(), "Resize darf nur an Top-Tabellen aufgerufen werden" );

    // Darf die Tabelle uberhaupt Resized werden oder soll sie es trotzdem?
    if( bMustNotResize && !bForce )
        return sal_False;

    // Darf ein Recalc der Tabelle durchgefuehrt werden?
    if( bMustNotRecalc && !bForce )
        bRecalc = sal_False;

    const SwDoc *pDoc = GetDoc();

    // Wenn es ein Layout gibt, wurde evtl. die Groesse der Root-Frames
    // und nicht die der VisArea uebergeben. Wenn wir nicht in einem Rahmen
    // stehen, muss die Tabelle allerdings fuer die VisArea berechnet werden,
    // weil sond die Umschaltung von relativ nach absolut nicht funktioniert.
    if( pDoc->GetRootFrm() && pDoc->get(IDocumentSettingAccess::BROWSE_MODE) )
    {
        const sal_uInt16 nVisAreaWidth = GetBrowseWidthByVisArea( *pDoc );
        if( nVisAreaWidth < nAbsAvail && !FindFlyFrmFmt() )
            nAbsAvail = nVisAreaWidth;
    }

    if( nDelay==0 && aResizeTimer.IsActive() )
    {
        // Wenn beim Aufruf eines synchronen Resize noch ein asynchrones
        // Resize aussteht, dann werden nur die neuen Werte uebernommen.

        bRecalc |= bDelayedResizeRecalc;
        nDelayedResizeAbsAvail = nAbsAvail;
        return sal_False;
    }

    // Optimierung:
    // Wenn die Minima/Maxima nicht neu berechnet werden sollen und
    // - die Breite der Tabelle nie neu berechnet werden muss, oder
    // - die Tabelle schon fuer die uebergebene Breite berechnet wurde, oder
    // - der verfuegbare Platz kleiner oder gleich der Minimalbreite ist
    //   und die Tabelle bereits die Minimalbreite besitzt, oder
    // - der verfuegbare Platz groesser ist als die Maximalbreite und
    //   die Tabelle bereits die Maximalbreite besitzt
    // wird sich an der Tabelle nichts aendern.
    if( !bRecalc && ( !bMustResize ||
                      (nLastResizeAbsAvail==nAbsAvail) ||
                      (nAbsAvail<=nMin && nRelTabWidth==nMin) ||
                      (!bPrcWidthOption && nAbsAvail>=nMax && nRelTabWidth==nMax) ) )
        return sal_False;

    if( nDelay==HTMLTABLE_RESIZE_NOW )
    {
        if( aResizeTimer.IsActive() )
            aResizeTimer.Stop();
        _Resize( nAbsAvail, bRecalc );
    }
    else if( nDelay > 0 )
    {
        nDelayedResizeAbsAvail = nAbsAvail;
        bDelayedResizeRecalc = bRecalc;
        aResizeTimer.SetTimeout( nDelay );
        aResizeTimer.Start();
#ifdef TEST_DELAYED_RESIZE
        Sound::Beep( SOUND_DEFAULT );
#endif
    }
    else
    {
        _Resize( nAbsAvail, bRecalc );
    }

    return sal_True;
}

void SwHTMLTableLayout::BordersChanged( sal_uInt16 nAbsAvail, sal_Bool bRecalc )
{
    bBordersChanged = sal_True;

    Resize( nAbsAvail, bRecalc );
}


