/*************************************************************************
 *
 *  $RCSfile: htmltbl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:16 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"

//#define TEST_DELAYED_RESIZE

#ifdef TEST_DELAYED_RESIZE
#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SOT_STORAGE_HXX //autogen
#include <sot/storage.hxx>
#endif

#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
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

#define COLFUZZY 20
#define MAX_TABWIDTH (USHRT_MAX - 2001)


/*  */

class SwHTMLTableLayoutConstraints
{
    USHORT nRow;                    // Start-Zeile
    USHORT nCol;                    // Start-Spalte
    USHORT nColSpan;                // COLSPAN der Zelle

    SwHTMLTableLayoutConstraints *pNext;        // die naechste Bedingung

    ULONG nMinNoAlign, nMaxNoAlign; // Zwischenergebnisse AL-Pass 1

public:

    SwHTMLTableLayoutConstraints( ULONG nMin, ULONG nMax, USHORT nRow,
                                USHORT nCol, USHORT nColSp );
    ~SwHTMLTableLayoutConstraints();

    ULONG GetMinNoAlign() const { return nMinNoAlign; }
    ULONG GetMaxNoAlign() const { return nMaxNoAlign; }

    SwHTMLTableLayoutConstraints *InsertNext( SwHTMLTableLayoutConstraints *pNxt );
    SwHTMLTableLayoutConstraints* GetNext() const { return pNext; }

    USHORT GetRow() const { return nRow; }

    USHORT GetColSpan() const { return nColSpan; }
    USHORT GetColumn() const { return nCol; }
};

/*  */

SwHTMLTableLayoutCnts::SwHTMLTableLayoutCnts( const SwStartNode *pSttNd,
                                          SwHTMLTableLayout* pTab,
                                          BOOL bNoBrTag,
                                          SwHTMLTableLayoutCnts* pNxt ) :
    pNext( pNxt ), pBox( 0 ), pTable( pTab ), pStartNode( pSttNd ),
    nWidthSet( 0 ), nPass1Done( 0 ), bNoBreakTag( bNoBrTag )
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
                                          USHORT nRSpan, USHORT nCSpan,
                                          USHORT nWidth, BOOL bPrcWidth,
                                          BOOL bNWrapOpt ) :
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

SwHTMLTableLayoutColumn::SwHTMLTableLayoutColumn( USHORT nWidth,
                                                  BOOL bRelWidth,
                                                  BOOL bLBorder ) :
    nMinNoAlign(MINLAY), nMaxNoAlign(MINLAY), nAbsMinNoAlign(MINLAY),
    nMin(0), nMax(0),
    nAbsColWidth(0), nRelColWidth(0),
    nWidthOption( nWidth ), bRelWidthOption( bRelWidth ),
    bLeftBorder( bLBorder )
{}


/*  */

SwHTMLTableLayoutConstraints::SwHTMLTableLayoutConstraints(
    ULONG nMin, ULONG nMax, USHORT nRw, USHORT nColumn, USHORT nColSp ):
    nMinNoAlign( nMin ), nMaxNoAlign( nMax ),
    nRow( nRw ), nCol( nColumn ), nColSpan( nColSp ),
    pNext( 0 )
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
                        USHORT nRws, USHORT nCls, BOOL bColsOpt, BOOL bColTgs,
                        USHORT nWdth, BOOL bPrcWdth, USHORT nBorderOpt,
                        USHORT nCellPad, USHORT nCellSp, SvxAdjust eAdjust,
                        USHORT nLMargin, USHORT nRMargin,
                        USHORT nBWidth, USHORT nLeftBWidth,
                        USHORT nRightBWidth,
                        USHORT nInhLeftBWidth, USHORT nInhRightBWidth ) :
    aColumns( new SwHTMLTableLayoutColumnPtr[nCls] ),
    aCells( new SwHTMLTableLayoutCellPtr[nRws*nCls] ),
    pSwTable( pSwTbl ), pLeftFillerBox( 0 ), pRightFillerBox( 0 ),
    nMin( 0 ), nMax( 0 ),
    nRows( nRws ), nCols( nCls ),
    nLeftMargin( nLMargin ), nRightMargin( nRMargin ),
    nCellPadding( nCellPad ), nCellSpacing( nCellSp ), nBorder( nBorderOpt ),
    nBorderWidth( nBWidth ),
    nLeftBorderWidth( nLeftBWidth ), nRightBorderWidth( nRightBWidth ),
    nInhLeftBorderWidth( nInhLeftBWidth ),
    nInhRightBorderWidth( nInhRightBWidth ),
    nRelLeftFill( 0 ), nRelRightFill( 0 ),
    nInhAbsLeftSpace( 0 ), nInhAbsRightSpace( 0 ),
    nRelTabWidth( 0 ), nWidthOption( nWdth ),
    nDelayedResizeAbsAvail( 0 ), nLastResizeAbsAvail( 0 ),
    nPass1Done( 0 ), nWidthSet( 0 ), eTableAdjust( eAdjust ),
    bColsOption( bColsOpt ), bColTags( bColTgs ),
    bPrcWidthOption( bPrcWdth ), bUseRelWidth( FALSE ),
    bMustResize( TRUE ), bExportable( TRUE ), bBordersChanged( FALSE ),
    bMustNotResize( FALSE ), bMustNotRecalc( FALSE )
{
    aResizeTimer.SetTimeoutHdl( STATIC_LINK( this, SwHTMLTableLayout,
                                             DelayedResize_Impl ) );
}

SwHTMLTableLayout::~SwHTMLTableLayout()
{
    for( USHORT i=0; i<nCols; i++ )
        delete aColumns[i];
    delete[] aColumns;

    USHORT nCount = nRows*nCols;
    for( i=0; i<nCount; i++ )
        delete aCells[i];
    delete[] aCells;
}

#ifdef USED
USHORT SwHTMLTableLayout::GetLeftBorderWidth( USHORT nCol ) const
{
    SwHTMLTableLayoutColumn *pColumn = GetColumn( nCol );

    USHORT nBWidth = 0;
    if( pColumn->HasLeftBorder() )
    {
        nBWidth = nCol==0 ? nLeftBorderWidth : nBorderWidth;
    }
    if( nCol==0 && nInhLeftBorderWidth > nBWidth )
    {
        nBWidth = nInhLeftBorderWidth;
    }

    return nBWidth;
}

USHORT SwHTMLTableLayout::GetRightBorderWidth( USHORT nCol, USHORT nColSpan ) const
{
    USHORT nBWidth = 0;

    if( nCol+nColSpan==nCols )
    {
        nBWidth = nInhRightBorderWidth > nRightBorderWidth
            ? nInhRightBorderWidth
            : nRightBorderWidth;
    }

    return nBWidth;
}
#endif

// Die Breiten der Umrandung werden zunaechst wie in Netscape berechnet:
// Aussere Umrandung: BORDER + CELLSPACING + CELLPADDING
// Innere Umrandung: CELLSPACING + CELLPADDING
// Allerdings wird die Breite der Umrandung im SW trotzdem beachtet, wenn
// bSwBorders gesetzt ist, damit nicht faellschlich umgebrochen wird.
// MIB 27.6.97: Dabei muss auch der Abstand zum Inhalt berueckichtigt werden,
// und zwar auch dann, wenn wenn nur die gegenueberliegende Seite
// eine Umrandung hat.
USHORT SwHTMLTableLayout::GetLeftCellSpace( USHORT nCol, USHORT nColSpan,
                                            BOOL bSwBorders ) const
{
    USHORT nSpace = nCellSpacing + nCellPadding;

    if( nCol == 0 )
    {
        nSpace += nBorder;

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

USHORT SwHTMLTableLayout::GetRightCellSpace( USHORT nCol, USHORT nColSpan,
                                             BOOL bSwBorders ) const
{
    USHORT nSpace = nCellPadding;

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

void SwHTMLTableLayout::AddBorderWidth( ULONG &rMin, ULONG &rMax,
                                        ULONG &rAbsMin,
                                        USHORT nCol, USHORT nColSpan,
                                        BOOL bSwBorders ) const
{
    ULONG nAdd = GetLeftCellSpace( nCol, nColSpan, bSwBorders ) +
                 GetRightCellSpace( nCol, nColSpan, bSwBorders );

    rMin += nAdd;
    rMax += nAdd;
    rAbsMin += nAdd;

#if 0
    // Diese Breiten-Berchnung orientiert sich an den SW-Umrandungen!!!
    USHORT nBDist = nCellPadding;

    // linke Umrandung beruecksichtigen
    USHORT nBorderWidth = GetLeftBorderWidth( nCol );
    if( nBorderWidth )
    {
        rMin += nBorderWidth;
        rMax += nBorderWidth;
        rAbsMin += nBorderWidth;
        if( !nBDist )
            nBDist = MIN_BORDER_DIST;
    }

    // rechte Umrandung beruecksichtigen
    nBorderWidth = GetRightBorderWidth( nCol, nColSpan );
    if( nBorderWidth )
    {
        rMin += nBorderWidth;
        rMax += nBorderWidth;
        rAbsMin += nBorderWidth;
        if( !nBDist )
            nBDist = MIN_BORDER_DIST;
    }

    // Abstand zum Inhalt beruecksichtigen
    rMin += 2*nBDist;
    rMax += 2*nBDist;
    rAbsMin += 2*nBDist;
#endif
}

void SwHTMLTableLayout::SetBoxWidth( SwTableBox *pBox, USHORT nCol,
                             USHORT nColSpan ) const
{
    SwFrmFmt *pFrmFmt = pBox->GetFrmFmt();

    // die Breite der Box berechnen
    SwTwips nFrmWidth = 0;
    while( nColSpan-- )
        nFrmWidth += GetColumn( nCol++ )->GetRelColWidth();

    // und neu setzen

    pFrmFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nFrmWidth, 0 ));
}

void SwHTMLTableLayout::GetAvail( USHORT nCol, USHORT nColSpan,
                                  USHORT& rAbsAvail, USHORT& rRelAvail ) const
{
    rAbsAvail = 0;
    rRelAvail = 0;
    for( USHORT i=nCol; i<nCol+nColSpan;i++ )
    {
        const SwHTMLTableLayoutColumn *pColumn = GetColumn(i);
        rAbsAvail += pColumn->GetAbsColWidth();
        rRelAvail += pColumn->GetRelColWidth();
    }

#if 0
    rAbsSpace = GetLeftBorderWidth( nCol ) +
                GetRightBorderWidth( nCol, nColSpan ) +
                2*nCellPadding;
#endif
}

USHORT SwHTMLTableLayout::GetBrowseWidthByVisArea( const SwDoc& rDoc )
{
    ViewShell *pVSh = 0;
    rDoc.GetEditShell( &pVSh );
    if( pVSh )
    {
        return (USHORT)( pVSh->VisArea().Width() -
           2*pVSh->GetOut()->PixelToLogic( pVSh->GetBrowseBorder() ).Width() );
    }

    return 0;
}

USHORT SwHTMLTableLayout::GetBrowseWidth( const SwDoc& rDoc )
{
    // Wenn ein Layout da ist, koennen wir die Breite dort herholen.
    const SwRootFrm *pRootFrm = rDoc.GetRootFrm();
    if( pRootFrm )
    {
        const SwFrm *pPageFrm = pRootFrm->GetLower();
        if( pPageFrm )
            return (USHORT)pPageFrm->Prt().Width();
    }

    // Sonst versuchen wir es ueber die ViewShell
    USHORT nWidth = GetBrowseWidthByVisArea( rDoc );
    if( !nWidth )
    {
        // Und wenn das auch nicht geht, gibt es noch die ActualSize an der
        // DocShell.
        if( rDoc.GetDocShell() && GetpApp() && GetpApp()->GetDefaultDevice() )
        {
            nWidth = (USHORT)Application::GetDefaultDevice()
                    ->PixelToLogic( rDoc.GetDocShell()->GetActualSize(),
                                    MapMode( MAP_TWIP ) ).Width();

            ASSERT( nWidth, "GetActualSize liefert 0" );
        }
#ifndef PRODUCT
        else
        {
            // und wenn das auch nicht klappt, gibt es zur Zeit keine Breite
            ASSERT( nWidth, "Nix da um eine Browse-Breite zu berechnen" );
        }
#endif
    }
    return nWidth;
}

USHORT SwHTMLTableLayout::GetBrowseWidthByTabFrm(
    const SwTabFrm& rTabFrm ) const
{
    SwTwips nWidth = 0;

    const SwFrm *pUpper = rTabFrm.GetUpper();
    if( MayBeInFlyFrame() && pUpper->IsFlyFrm() &&
        ((const SwFlyFrm *)pUpper)->GetAnchor() )
    {
        // Wenn die Tabelle in einem selbst angelegten Rahmen steht, dann ist
        // die Breite Ankers und nicht die Breite Rahmens von Bedeutung.
        // Bei Absatz-gebundenen Rahmen werden Absatz-Einzuege nicht beachtet.
        const SwFrm *pAnchor = ((const SwFlyFrm *)pUpper)->GetAnchor();
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

    return nWidth < USHRT_MAX ? nWidth : USHRT_MAX;
}

USHORT SwHTMLTableLayout::GetBrowseWidthByTable( const SwDoc& rDoc ) const
{
    USHORT nBrowseWidth = 0;
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

void lcl_GetMinMaxSize( ULONG& rMinNoAlignCnts, ULONG& rMaxNoAlignCnts,
                        ULONG& rAbsMinNoAlignCnts,
#ifdef FIX41370
                        BOOL& rHR,
#endif
                        SwTxtNode *pTxtNd, ULONG nIdx, BOOL bNoBreak )
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
        rHR |= !pTxtNd->GetpSwAttrSet() ||
                SFX_ITEM_SET != pTxtNd->GetpSwAttrSet()
                                      ->GetItemState( RES_LR_SPACE, FALSE );
    }
#endif
}

void SwHTMLTableLayout::AutoLayoutPass1()
{
    nPass1Done++;

    ClearPass1Info();

    BOOL bFixRelWidths = FALSE;
    USHORT i;

    SwHTMLTableLayoutConstraints *pConstraints = 0;

    for( i=0; i<nCols; i++ )
    {
        SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
        pColumn->ClearPass1Info( !HasColTags() );
        USHORT nMinColSpan = USHRT_MAX; // Spaltenzahl, auf die sich dir
                                        // berechnete Breite bezieht
        USHORT nColSkip = USHRT_MAX;    // Wie viele Spalten muessen
                                        // uebersprungen werden

        for( USHORT j=0; j<nRows; j++ )
        {
            SwHTMLTableLayoutCell *pCell = GetCell(j,i);
            SwHTMLTableLayoutCnts *pCnts = pCell->GetContents();

            // fix #31488#: Zum Ermitteln der naechsten zu berechnenden
            // Spalte muessen alle Zeilen herangezogen werden
            USHORT nColSpan = pCell->GetColSpan();
            if( nColSpan < nColSkip )
                nColSkip = nColSpan;

            if( !pCnts || (pCnts && !pCnts->IsPass1Done(nPass1Done)) )
            {
                // die Zelle ist leer oder ihr Inhalt wurde nich nicht
                // bearbeitet
                if( nColSpan < nMinColSpan )
                    nMinColSpan = nColSpan;

                ULONG nMinNoAlignCell = 0;
                ULONG nMaxNoAlignCell = 0;
                ULONG nAbsMinNoAlignCell = 0;
                ULONG nMaxTableCell = 0;
                ULONG nAbsMinTableCell = 0;
#ifdef FIX41370
                BOOL bHR = FALSE;
#endif

                while( pCnts )
                {
                    const SwStartNode *pSttNd = pCnts->GetStartNode();
                    if( pSttNd )
                    {
                        const SwDoc *pDoc = pSttNd->GetDoc();
                        ULONG nIdx = pSttNd->GetIndex();
                        while( !(pDoc->GetNodes()[nIdx])->IsEndNode() )
                        {
                            SwTxtNode *pTxtNd = (pDoc->GetNodes()[nIdx])->GetTxtNode();
                            if( pTxtNd )
                            {
                                ULONG nMinNoAlignCnts;
                                ULONG nMaxNoAlignCnts;
                                ULONG nAbsMinNoAlignCnts;

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
                            nIdx++;
                        }
                    }
                    else
                    {
                        SwHTMLTableLayout *pChild = pCnts->GetTable();
                        pChild->AutoLayoutPass1();
                        ULONG nMaxTableCnts = pChild->nMax;
                        ULONG nAbsMinTableCnts = pChild->nMin;

                        // Eine feste Tabellen-Breite wird als Minimum
                        // und Maximum gleichzeitig uebernommen
                        if( !pChild->bPrcWidthOption && pChild->nWidthOption )
                        {
                            ULONG nTabWidth = pChild->nWidthOption;
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

                BOOL bRelWidth = pCell->IsPrcWidthOption();
                USHORT nWidth = pCell->GetWidthOption();

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
                        ULONG nAbsWidth = nWidth, nDummy = 0, nDummy2 = 0;
                        AddBorderWidth( nAbsWidth, nDummy, nDummy2,
                                        i, nColSpan, FALSE );

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
                ULONG nAbsWidth = pColumn->GetWidthOption();
                ULONG nDummy = 0, nDummy2 = 0;
                AddBorderWidth( nAbsWidth, nDummy, nDummy2, i, 1, FALSE );

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
        USHORT nCol = pConstr->GetColumn();
        USHORT nColSpan = pConstr->GetColSpan();
        ULONG nConstrMin = pConstr->GetMinNoAlign();
        ULONG nConstrMax = pConstr->GetMaxNoAlign();

        // jetzt holen wir uns die bisherige Breite der ueberspannten
        // Spalten
        ULONG nColsMin = 0;
        ULONG nColsMax = 0;
        for( USHORT i=nCol; i<nCol+nColSpan; i++ )
        {
            SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
            nColsMin += pColumn->GetMin();
            nColsMax += pColumn->GetMax();
        }

        if( nColsMin<nConstrMin )
        {
            // den Minimalwert anteilig auf die Spalten verteilen
            ULONG nMinD = nConstrMin-nColsMin;

            if( nConstrMin > nColsMax )
            {
                // Anteilig anhand der Mindestbreiten
                USHORT nEndCol = nCol+nColSpan;
                ULONG nDiff = nMinD;
                for( USHORT i=nCol; i<nEndCol; i++ )
                {
                    SwHTMLTableLayoutColumn *pColumn = GetColumn( i );

                    ULONG nColMin = pColumn->GetMin();
                    ULONG nColMax = pColumn->GetMax();

                    nMin -= nColMin;
                    ULONG nAdd = i<nEndCol-1 ? (nColMin * nMinD) / nColsMin
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
                for( USHORT i=nCol; i<nCol+nColSpan; i++ )
                {
                    SwHTMLTableLayoutColumn *pColumn = GetColumn( i );

                    ULONG nDiff = pColumn->GetMax()-pColumn->GetMin();
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
            ULONG nMaxD = nConstrMax-nColsMax;

            for( USHORT i=nCol; i<nCol+nColSpan; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );

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

            ULONG nAbsMin = 0;  // absolte Min-Breite alter Spalten mit
                                // relativer Breite
            ULONG nRel = 0;     // Summe der relativen Breiten aller Spalten
            for( i=0; i<nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() )
                {
                    nAbsMin += pColumn->GetMin();
                    nRel += pColumn->GetWidthOption();
                }
            }

            ULONG nQuot = ULONG_MAX;
            for( i=0; i<nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() )
                {
                    nMax -= pColumn->GetMax();
                    if( pColumn->GetWidthOption() && pColumn->GetMin() )
                    {
                        pColumn->SetMax( nAbsMin * pColumn->GetWidthOption() );
                        ULONG nColQuot = pColumn->GetMax() / pColumn->GetMin();
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
            USHORT nRel = 0;        // Summe der relativen Breiten aller Spalten
            USHORT nRelCols = 0;    // Anzahl Spalten mit relativer Angabe
            ULONG nRelMax = 0;      // Anteil am Maximum dieser Spalten
            for( i=0; i<nCols; i++ )
            {
                ASSERT( nRel<=100, "relative Breite aller Spalten>100%" );
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() )
                {
                    // Sicherstellen, dass die relativen breiten nicht
                    // ueber 100% landen
                    USHORT nColWidth = pColumn->GetWidthOption();
                    if( nRel+nColWidth > 100 )
                    {
                        nColWidth = 100 - nRel;
                        pColumn->SetWidthOption( nColWidth, TRUE, FALSE );
                    }
                    nRelMax += pColumn->GetMax();
                    nRel += nColWidth;
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
                USHORT nRelLeft = 100 - nRel;
                ULONG nFixMax = nMax - nRelMax;
                for( i=0; i<nCols; i++ )
                {
                    SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                    if( !pColumn->IsRelWidthOption() &&
                        !pColumn->GetWidthOption() &&
                        pColumn->GetMin() )
                    {
                        // den Rest bekommt die naechste Spalte
                        USHORT nColWidth =
                            (USHORT)((pColumn->GetMax() * nRelLeft) / nFixMax);
                        pColumn->SetWidthOption( nColWidth, TRUE, FALSE );
                    }
                }
            }

            // nun die Maximalbreiten entsprechend anpassen
            ULONG nQuotMax = ULONG_MAX;
            ULONG nOldMax = nMax;
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
                    ULONG nNewMax;
                    ULONG nColQuotMax;
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
void SwHTMLTableLayout::AutoLayoutPass2( USHORT nAbsAvail, USHORT nRelAvail,
                                         USHORT nAbsLeftSpace,
                                         USHORT nAbsRightSpace,
                                         USHORT nParentInhAbsSpace )
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
    USHORT nAbsLeftFill = 0, nAbsRightFill = 0;
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
            nAbsLeftFill += nLeftMargin;
            nAbsRightFill += nRightMargin;
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
        ULONG nAbsLeftFillL = nAbsLeftFill, nAbsRightFillL = nAbsRightFill;

        nRelLeftFill = (USHORT)((nAbsLeftFillL * nRelAvail) / nAbsAvail);
        nRelRightFill = (USHORT)((nAbsRightFillL * nRelAvail) / nAbsAvail);

        nAbsAvail -= (nAbsLeftFill + nAbsRightFill);
        if( nRelAvail )
            nRelAvail -= (nRelLeftFill + nRelRightFill);
    }


    // Schritt 2: Die absolute Tabellenbreite wird berechnet.
    USHORT nAbsTabWidth = 0;
    bUseRelWidth = FALSE;
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
            nAbsTabWidth = (USHORT)( ((ULONG)nAbsAvail * nWidthOption) / 100 );
            if( IsTopTable() &&
                ( /*MayBeInFlyFrame() ||*/ (ULONG)nAbsTabWidth > nMin ) )
            {
                nRelAvail = USHRT_MAX;
                bUseRelWidth = TRUE;
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
    if( (!IsTopTable() && nMin > (ULONG)nAbsAvail) ||
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

        USHORT nAbs = 0, nRel = 0;
        SwHTMLTableLayoutColumn *pColumn;
        for( USHORT i=0; i<nCols-1; i++ )
        {
            pColumn = GetColumn( i );
            ULONG nColMin = pColumn->GetMin();
            if( nColMin <= USHRT_MAX )
            {
                pColumn->SetAbsColWidth(
                    (USHORT)((nColMin * nAbsTabWidth) / nMin) );
                pColumn->SetRelColWidth(
                    (USHORT)((nColMin * nRelTabWidth) / nMin) );
            }
            else
            {
                double nColMinD = nColMin;
                pColumn->SetAbsColWidth(
                    (USHORT)((nColMinD * nAbsTabWidth) / nMin) );
                pColumn->SetRelColWidth(
                    (USHORT)((nColMinD * nRelTabWidth) / nMin) );
            }

            nAbs += (USHORT)pColumn->GetAbsColWidth();
            nRel += (USHORT)pColumn->GetRelColWidth();
        }
        pColumn = GetColumn( nCols-1 );
        pColumn->SetAbsColWidth( nAbsTabWidth - nAbs );
        pColumn->SetRelColWidth( nRelTabWidth - nRel );
    }
    else if( nMax <= (ULONG)(nAbsTabWidth ? nAbsTabWidth : nAbsAvail) )
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
            nAbsTabWidth = (USHORT)nMax;

        // Eine Top-Table darf auch beriter werden als der verfuegbare Platz.
        if( nAbsTabWidth > nAbsAvail )
        {
            ASSERT( IsTopTable(),
                    "Tabelle in Tabelle soll breiter werden als umgebende Zelle" );
            nAbsAvail = nAbsTabWidth;
        }

        // Nur den Anteil der relativen Breite verwenden, der auch fuer
        // die absolute Breite verwendet wuerde.
        ULONG nAbsTabWidthL = nAbsTabWidth;
        nRelTabWidth =
            ( nRelAvail ? (USHORT)((nAbsTabWidthL * nRelAvail) / nAbsAvail)
                        : nAbsTabWidth );

        // Gibt es Spalten mit und Spalten ohne %-Angabe?
        ULONG nFixMax = nMax;
        for( USHORT i=0; i<nCols; i++ )
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

            USHORT nAbs = 0, nRel = 0;
            USHORT nFixedCols = 0;
            for( USHORT i=0; i<nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( !pColumn->IsRelWidthOption() || !pColumn->GetWidthOption() )
                {
                    // Die Spalte behaelt ihre Breite bei.
                    nFixedCols++;
                    ULONG nColMax = pColumn->GetMax();
                    pColumn->SetAbsColWidth( (USHORT)nColMax );

                    ULONG nRelColWidth =
                        (nColMax * nRelTabWidth) / nAbsTabWidth;
                    ULONG nChkWidth =
                        (nRelColWidth * nAbsTabWidth) / nRelTabWidth;
                    if( nChkWidth < nColMax )
                        nRelColWidth++;
                    else if( nChkWidth > nColMax )
                        nRelColWidth--;
                    pColumn->SetRelColWidth( (USHORT)nRelColWidth );

                    nAbs += (USHORT)nColMax;
                    nRel += (USHORT)nRelColWidth;
                }
            }

            // Zu verteilende Anteile des Maximums und der relativen und
            // absoluten Breiten. nFixMax entspricht an dieser Stelle
            // nAbs, so dass man gleich nFixMax haette nehmen koennen.
            // Der Code ist so aber verstaendlicher.
            ASSERT( nFixMax == nAbs, "Zwei Schleifen, zwei Summen?" )
            ULONG nDistMax = nMax - nFixMax;
            USHORT nDistAbsTabWidth = nAbsTabWidth - nAbs;
            USHORT nDistRelTabWidth = nRelTabWidth - nRel;

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
                        ULONG nColMax = pColumn->GetMax();
                        pColumn->SetAbsColWidth(
                            (USHORT)((nColMax * nDistAbsTabWidth) / nDistMax) );
                        pColumn->SetRelColWidth(
                            (USHORT)((nColMax * nDistRelTabWidth) / nDistMax) );
                    }
                    nAbs += pColumn->GetAbsColWidth();
                    nRel += pColumn->GetRelColWidth();
                }
            }
            ASSERT( nCols==nFixedCols, "Spalte vergessen!" );
        }
        else
        {
            // nein, dann den zu verteilenden Platz auf alle Spalten
            // gleichmaessig vertilen.
            for( USHORT i=0; i<nCols; i++ )
            {
                ULONG nColMax = GetColumn( i )->GetMax();
                GetColumn( i )->SetAbsColWidth(
                    (USHORT)((nColMax * nAbsTabWidth) / nMax) );
                GetColumn( i )->SetRelColWidth(
                    (USHORT)((nColMax * nRelTabWidth) / nMax) );
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
            nAbsTabWidth = (USHORT)nMin;

        if( nAbsTabWidth > nAbsAvail )
        {
            ASSERT( IsTopTable(),
                    "Tabelle in Tabelle soll breiter werden als Platz da ist" );
            nAbsAvail = nAbsTabWidth;
        }

        ULONG nAbsTabWidthL = nAbsTabWidth;
        nRelTabWidth =
            ( nRelAvail ? (USHORT)((nAbsTabWidthL * nRelAvail) / nAbsAvail)
                        : nAbsTabWidth );
        double nW = nAbsTabWidth - nMin;
        double nD = (nMax==nMin ? 1 : nMax-nMin);
        USHORT nAbs = 0, nRel = 0;
        for( USHORT i=0; i<nCols-1; i++ )
        {
            double nd = GetColumn( i )->GetMax() - GetColumn( i )->GetMin();
            ULONG nAbsColWidth = GetColumn( i )->GetMin() + (ULONG)((nd*nW)/nD);
            ULONG nRelColWidth = nRelAvail
                                    ? (nAbsColWidth * nRelTabWidth) / nAbsTabWidth
                                    : nAbsColWidth;

            GetColumn( i )->SetAbsColWidth( (USHORT)nAbsColWidth );
            GetColumn( i )->SetRelColWidth( (USHORT)nRelColWidth );
            nAbs += (USHORT)nAbsColWidth;
            nRel += (USHORT)nRelColWidth;
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
        USHORT nAbsDist = (USHORT)(nAbsAvail-nAbsTabWidth);
        USHORT nRelDist = (USHORT)(nRelAvail-nRelTabWidth);
        USHORT nParentInhAbsLeftSpace = 0, nParentInhAbsRightSpace = 0;

        // Groesse und Position der zusaetzlichen Zellen bestimmen
        switch( eTableAdjust )
        {
        case SVX_ADJUST_RIGHT:
            nAbsLeftFill += nAbsDist;
            nRelLeftFill += nRelDist;
            nParentInhAbsLeftSpace = nParentInhAbsSpace;
            break;
        case SVX_ADJUST_CENTER:
            {
                USHORT nAbsLeftDist = nAbsDist / 2;
                nAbsLeftFill += nAbsLeftDist;
                nAbsRightFill += nAbsDist - nAbsLeftDist;
                USHORT nRelLeftDist = nRelDist / 2;
                nRelLeftFill += nRelLeftDist;
                nRelRightFill += nRelDist - nRelLeftDist;
                nParentInhAbsLeftSpace = nParentInhAbsSpace / 2;
                nParentInhAbsRightSpace = nParentInhAbsSpace -
                                          nParentInhAbsLeftSpace;
            }
            break;
        case SVX_ADJUST_LEFT:
        default:
            nAbsRightFill += nAbsDist;
            nRelRightFill += nRelDist;
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

BOOL lcl_ResizeLine( const SwTableLine*& rpLine, void* pPara );

BOOL lcl_ResizeBox( const SwTableBox*& rpBox, void* pPara )
{
    USHORT *pWidth = (USHORT *)pPara;

    if( !rpBox->GetSttNd() )
    {
        USHORT nWidth = 0;
        ((SwTableBox *)rpBox)->GetTabLines().ForEach( &lcl_ResizeLine, &nWidth );
        rpBox->GetFrmFmt()->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth, 0 ));
        *pWidth += nWidth;
    }
    else
    {
        *pWidth += (USHORT)rpBox->GetFrmFmt()->GetFrmSize().GetSize().Width();
    }

    return TRUE;
}

BOOL lcl_ResizeLine( const SwTableLine*& rpLine, void* pPara )
{
    USHORT *pWidth = (USHORT *)pPara;
#ifndef PRODUCT
    USHORT nOldWidth = *pWidth;
#endif;
    *pWidth = 0;
    ((SwTableLine *)rpLine)->GetTabBoxes().ForEach( &lcl_ResizeBox, pWidth );

#ifndef PRODUCT
    ASSERT( !nOldWidth || Abs(*pWidth-nOldWidth) < COLFUZZY,
            "Zeilen einer Box sind unterschiedlich lang" );
#endif;

    return TRUE;
}

void SwHTMLTableLayout::SetWidths( BOOL bCallPass2, USHORT nAbsAvail,
                                   USHORT nRelAvail, USHORT nAbsLeftSpace,
                                   USHORT nAbsRightSpace,
                                   USHORT nParentInhAbsSpace )
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
    for( USHORT i=0; i<nRows; i++ )
    {
        for( USHORT j=0; j<nCols; j++ )
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
                    USHORT nAbs = 0, nRel = 0, nLSpace = 0, nRSpace = 0,
                           nInhSpace = 0;
                    if( bCallPass2 )
                    {
                        USHORT nColSpan = pCell->GetColSpan();
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
        USHORT nCalcTabWidth = 0;
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
        BOOL bRel = bUseRelWidth &&
                    HORI_FULL!=pFrmFmt->GetHoriOrient().GetHoriOrient();
        aFrmSize.SetWidthPercent( (BYTE)(bRel ? nWidthOption : 0) );
        pFrmFmt->SetAttr( aFrmSize );
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
                    aFlyFrmSize.SetWidthPercent( (BYTE)nWidthOption );
                }
                pFlyFrmFmt->SetAttr( aFlyFrmSize );
            }
        }

#ifndef PRODUCT
        {
            // steht im tblrwcl.cxx
            extern void _CheckBoxWidth( const SwTableLine&, SwTwips );

            // checke doch mal ob die Tabellen korrekte Breiten haben
            SwTwips nSize = pSwTable->GetFrmFmt()->GetFrmSize().GetWidth();
            const SwTableLines& rLines = pSwTable->GetTabLines();
            for( USHORT n = 0; n < rLines.Count(); ++n  )
                _CheckBoxWidth( *rLines[ n ], nSize );
        }
#endif

    }
    else
    {
        if( pLeftFillerBox )
        {
            pLeftFillerBox->GetFrmFmt()->SetAttr(
                SwFmtFrmSize( ATT_VAR_SIZE, nRelLeftFill, 0 ));
        }
        if( pRightFillerBox )
        {
            pRightFillerBox->GetFrmFmt()->SetAttr(
                SwFmtFrmSize( ATT_VAR_SIZE, nRelRightFill, 0 ));
        }
    }
}

void SwHTMLTableLayout::_Resize( USHORT nAbsAvail, BOOL bRecalc )
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
    SetWidths( TRUE, nAbsAvail );

    if ( pRoot && pRoot->IsCallbackActionEnabled() )
        pRoot->EndAllAction( TRUE );    //True per VirDev (Browsen ruhiger)
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


BOOL SwHTMLTableLayout::Resize( USHORT nAbsAvail, BOOL bRecalc,
                                BOOL bForce, ULONG nDelay )
{
    ASSERT( IsTopTable(), "Resize darf nur an Top-Tabellen aufgerufen werden" );

    // Darf die Tabelle uberhaupt Resized werden oder soll sie es trotzdem?
    if( bMustNotResize && !bForce )
        return FALSE;

    // Darf ein Recalc der Tabelle durchgefuehrt werden?
    if( bMustNotRecalc && !bForce )
        bRecalc = FALSE;

    const SwDoc *pDoc = GetDoc();

    // Wenn es ein Layout gibt, wurde evtl. die Groesse der Root-Frames
    // und nicht die der VisArea uebergeben. Wenn wir nicht in einem Rahmen
    // stehen, muss die Tabelle allerdings fuer die VisArea berechnet werden,
    // weil sond die Umschaltung von relativ nach absolut nicht funktioniert.
    if( pDoc->GetRootFrm() && pDoc->IsBrowseMode() )
    {
        USHORT nVisAreaWidth = GetBrowseWidthByVisArea( *pDoc );
        if( nVisAreaWidth < nAbsAvail && !FindFlyFrmFmt() )
            nAbsAvail = nVisAreaWidth;
    }

    if( nDelay==0 && aResizeTimer.IsActive() )
    {
        // Wenn beim Aufruf eines synchronen Resize noch ein asynchrones
        // Resize aussteht, dann werden nur die neuen Werte uebernommen.

        bRecalc |= bDelayedResizeRecalc;
        nDelayedResizeAbsAvail = nAbsAvail;
        return FALSE;
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
        return FALSE;

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

    return TRUE;
}

void SwHTMLTableLayout::BordersChanged( USHORT nAbsAvail, BOOL bRecalc )
{
    bBordersChanged = TRUE;

    Resize( nAbsAvail, bRecalc );
}


