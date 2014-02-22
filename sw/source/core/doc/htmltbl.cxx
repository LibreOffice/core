/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "hintids.hxx"

#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
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
#include "viewopt.hxx"
#include "htmltbl.hxx"
#include "ndindex.hxx"
#include "switerator.hxx"
#include <boost/foreach.hpp>

using namespace ::com::sun::star;

#define COLFUZZY 20
#define MAX_TABWIDTH (USHRT_MAX - 2001)

class SwHTMLTableLayoutConstraints
{
    sal_uInt16 nRow;                    
    sal_uInt16 nCol;                    
    sal_uInt16 nColSpan;                

    SwHTMLTableLayoutConstraints *pNext;        

    sal_uLong nMinNoAlign, nMaxNoAlign; 

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

SwHTMLTableLayoutCnts::SwHTMLTableLayoutCnts( const SwStartNode *pSttNd,
                                          SwHTMLTableLayout* pTab,
                                          bool bNoBrTag,
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

SwHTMLTableLayoutCell::SwHTMLTableLayoutCell( SwHTMLTableLayoutCnts *pCnts,
                                          sal_uInt16 nRSpan, sal_uInt16 nCSpan,
                                          sal_uInt16 nWidth, bool bPrcWidth,
                                          bool bNWrapOpt ) :
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

SwHTMLTableLayoutColumn::SwHTMLTableLayoutColumn( sal_uInt16 nWidth,
                                                  bool bRelWidth,
                                                  bool bLBorder ) :
    nMinNoAlign(MINLAY), nMaxNoAlign(MINLAY), nAbsMinNoAlign(MINLAY),
    nMin(0), nMax(0),
    nAbsColWidth(0), nRelColWidth(0),
    nWidthOption( nWidth ), bRelWidthOption( bRelWidth ),
    bLeftBorder( bLBorder )
{}

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

typedef SwHTMLTableLayoutColumn *SwHTMLTableLayoutColumnPtr;
typedef SwHTMLTableLayoutCell *SwHTMLTableLayoutCellPtr;

SwHTMLTableLayout::SwHTMLTableLayout( const SwTable * pSwTbl,
                                      sal_uInt16 nRws, sal_uInt16 nCls,
                                      bool bColsOpt, bool bColTgs,
                                      sal_uInt16 nWdth, bool bPrcWdth,
                                      sal_uInt16 nBorderOpt, sal_uInt16 nCellPad,
                                      sal_uInt16 nCellSp, SvxAdjust eAdjust,
                                      sal_uInt16 nLMargin, sal_uInt16 nRMargin,
                                      sal_uInt16 nBWidth, sal_uInt16 nLeftBWidth,
                                      sal_uInt16 nRightBWidth,
                                      sal_uInt16 nInhLeftBWidth,
                                      sal_uInt16 nInhRightBWidth )
    : aColumns( new SwHTMLTableLayoutColumnPtr[nCls] )
    , aCells( new SwHTMLTableLayoutCellPtr[nRws*nCls] )
    , pSwTable( pSwTbl )
    , pLeftFillerBox( 0 )
    , pRightFillerBox( 0 )
    , nMin( 0 )
    , nMax( 0 )
    , nRows( nRws )
    , nCols( nCls )
    , nLeftMargin( nLMargin )
    , nRightMargin( nRMargin )
    , nInhAbsLeftSpace( 0 )
    , nInhAbsRightSpace( 0 )
    , nRelLeftFill( 0 )
    , nRelRightFill( 0 )
    , nRelTabWidth( 0 )
    , nWidthOption( nWdth )
    , nCellPadding( nCellPad )
    , nCellSpacing( nCellSp )
    , nBorder( nBorderOpt )
    , nLeftBorderWidth( nLeftBWidth )
    , nRightBorderWidth( nRightBWidth )
    , nInhLeftBorderWidth( nInhLeftBWidth )
    , nInhRightBorderWidth( nInhRightBWidth )
    , nBorderWidth( nBWidth )
    , nDelayedResizeAbsAvail( 0 )
    , nLastResizeAbsAvail( 0 )
    , nPass1Done( 0 )
    , nWidthSet( 0 )
    , eTableAdjust( eAdjust )
    , bColsOption( bColsOpt )
    , bColTags( bColTgs )
    , bPrcWidthOption( bPrcWdth )
    , bUseRelWidth( false )
    , bMustResize( sal_True )
    , bExportable( sal_True )
    , bBordersChanged( sal_False )
    , bMayBeInFlyFrame( sal_False )
    , bDelayedResizeRecalc( sal_False)
    , bMustNotResize( sal_False )
    , bMustNotRecalc( sal_False )
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
            OSL_ENSURE( !nCellPadding, "GetLeftCellSpace: CELLPADDING!=0" );
            
            
            
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
        OSL_ENSURE( !nCellPadding, "GetRightCellSpace: CELLPADDING!=0" );
        
        
        
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

    
    SwTwips nFrmWidth = 0;
    while( nColSpan-- )
        nFrmWidth += GetColumn( nCol++ )->GetRelColWidth();

    
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
    SwViewShell *pVSh = 0;
    rDoc.GetEditShell( &pVSh );
    if( pVSh )
    {
        return (sal_uInt16)pVSh->GetBrowseWidth();
    }

    return 0;
}

sal_uInt16 SwHTMLTableLayout::GetBrowseWidth( const SwDoc& rDoc )
{
    
    const SwRootFrm *pRootFrm = rDoc.GetCurrentLayout();
    if( pRootFrm )
    {
        const SwFrm *pPageFrm = pRootFrm->GetLower();
        if( pPageFrm )
            return (sal_uInt16)pPageFrm->Prt().Width();
    }

    
    
    
    
    return GetBrowseWidthByVisArea( rDoc );
}

sal_uInt16 SwHTMLTableLayout::GetBrowseWidthByTabFrm(
    const SwTabFrm& rTabFrm ) const
{
    SwTwips nWidth = 0;

    const SwFrm *pUpper = rTabFrm.GetUpper();
    if( MayBeInFlyFrame() && pUpper->IsFlyFrm() &&
        ((const SwFlyFrm *)pUpper)->GetAnchorFrm() )
    {
        
        
        
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
    SwTabFrm* pFrm = SwIterator<SwTabFrm,SwFmt>::FirstElement( *pSwTable->GetFrmFmt() );
    if( pFrm )
    {
        nBrowseWidth = GetBrowseWidthByTabFrm( *pFrm );
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
        OSL_ENSURE( pBox->GetTabLines().size() > 0,
                "Box without start node and lines" );
        OSL_ENSURE( pBox->GetTabLines().front()->GetTabBoxes().size() > 0,
                "Line without boxes" );
        pBox = pBox->GetTabLines().front()->GetTabBoxes().front();
    }

    return pBoxSttNd;
}

SwFrmFmt *SwHTMLTableLayout::FindFlyFrmFmt() const
{
    const SwTableNode *pTblNd = GetAnyBoxStartNode()->FindTableNode();
    OSL_ENSURE( pTblNd, "Kein Table-Node?" );
    return pTblNd->GetFlyFmt();
}

static void lcl_GetMinMaxSize( sal_uLong& rMinNoAlignCnts, sal_uLong& rMaxNoAlignCnts,
                        sal_uLong& rAbsMinNoAlignCnts,
                        SwTxtNode *pTxtNd, sal_uLong nIdx, bool bNoBreak )
{
    pTxtNd->GetMinMaxSize( nIdx, rMinNoAlignCnts, rMaxNoAlignCnts,
                           rAbsMinNoAlignCnts );
    OSL_ENSURE( rAbsMinNoAlignCnts <= rMinNoAlignCnts,
            "GetMinMaxSize: absmin > min" );
    OSL_ENSURE( rMinNoAlignCnts <= rMaxNoAlignCnts,
            "GetMinMaxSize: max > min" );

    
    const SwFmtColl *pColl = &pTxtNd->GetAnyFmtColl();
    while( pColl && !pColl->IsDefault() &&
            (USER_FMT & pColl->GetPoolFmtId()) )
    {
        pColl = (const SwFmtColl *)pColl->DerivedFrom();
    }

    
    
    if( (pColl && RES_POOLCOLL_HTML_PRE==pColl->GetPoolFmtId()) || bNoBreak )
    {
        rMinNoAlignCnts = rMaxNoAlignCnts;
        rAbsMinNoAlignCnts = rMaxNoAlignCnts;
    }
}

void SwHTMLTableLayout::AutoLayoutPass1()
{
    nPass1Done++;

    ClearPass1Info();

    bool bFixRelWidths = false;
    sal_uInt16 i;

    SwHTMLTableLayoutConstraints *pConstraints = 0;

    for( i=0; i<nCols; i++ )
    {
        SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
        pColumn->ClearPass1Info( !HasColTags() );
        sal_uInt16 nMinColSpan = USHRT_MAX; 
        sal_uInt16 nColSkip = USHRT_MAX;    

        for( sal_uInt16 j=0; j<nRows; j++ )
        {
            SwHTMLTableLayoutCell *pCell = GetCell(j,i);
            SwHTMLTableLayoutCnts *pCnts = pCell->GetContents();

            
            
            sal_uInt16 nColSpan = pCell->GetColSpan();
            if( nColSpan < nColSkip )
                nColSkip = nColSpan;

            if( !pCnts || (pCnts && !pCnts->IsPass1Done(nPass1Done)) )
            {
                
                if( nColSpan < nMinColSpan )
                    nMinColSpan = nColSpan;

                sal_uLong nMinNoAlignCell = 0;
                sal_uLong nMaxNoAlignCell = 0;
                sal_uLong nAbsMinNoAlignCell = 0;
                sal_uLong nMaxTableCell = 0;
                sal_uLong nAbsMinTableCell = 0;

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
                        OSL_ENSURE( !this, "Sub tables in HTML import?" );
                        SwHTMLTableLayout *pChild = pCnts->GetTable();
                        pChild->AutoLayoutPass1();
                        sal_uLong nMaxTableCnts = pChild->nMax;
                        sal_uLong nAbsMinTableCnts = pChild->nMin;

                        
                        
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


                sal_Bool bRelWidth = pCell->IsPrcWidthOption();
                sal_uInt16 nWidth = pCell->GetWidthOption();

                
                
                
                
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

                
                if( nMinNoAlignCell < MINLAY )
                    nMinNoAlignCell = MINLAY;
                if( nMaxNoAlignCell < MINLAY )
                    nMaxNoAlignCell = MINLAY;
                if( nAbsMinNoAlignCell < MINLAY )
                    nAbsMinNoAlignCell = MINLAY;

                
                AddBorderWidth( nMinNoAlignCell, nMaxNoAlignCell,
                                nAbsMinNoAlignCell, i, nColSpan );

                if( 1==nColSpan )
                {
                    
                    pColumn->MergeMinMaxNoAlign( nMinNoAlignCell,
                                                 nMaxNoAlignCell,
                                                 nAbsMinNoAlignCell );

                    
                    if( !HasColTags() )
                        pColumn->MergeCellWidthOption( nWidth, bRelWidth );
                }
                else
                {
                    

                    
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

        OSL_ENSURE( nMinColSpan>0 && nColSkip>0 && nColSkip <= nMinColSpan,
                "Layout pass 1: Columns are being forgotten!" );
        OSL_ENSURE( nMinColSpan!=USHRT_MAX,
                "Layout pass 1: unnecessary pass through the loop or a bug" );

        if( 1==nMinColSpan )
        {
            
            

            
            //
            
            //
            
            
            //
            
            
            //
            
            
            //
            
            
            //
            
            
            

            if( pColumn->GetWidthOption() && !pColumn->IsRelWidthOption() )
            {
                
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
            
            pColumn->SetMinMax( MINLAY, MINLAY );

            
            i += (nColSkip-1);
        }

        nMin += pColumn->GetMin();
        nMax += pColumn->GetMax();
        if (pColumn->IsRelWidthOption()) bFixRelWidths = true;
    }

    
    SwHTMLTableLayoutConstraints *pConstr = pConstraints;
    while( pConstr )
    {
        
        
        sal_uInt16 nCol = pConstr->GetColumn();
        sal_uInt16 nColSpan = pConstr->GetColSpan();
        sal_uLong nConstrMin = pConstr->GetMinNoAlign();
        sal_uLong nConstrMax = pConstr->GetMaxNoAlign();

        
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
            
            sal_uLong nMinD = nConstrMin-nColsMin;

            if( nConstrMin > nColsMax )
            {
                
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
                    OSL_ENSURE( nDiff >= nAdd, "Ooops: nDiff is not correct anymore" );
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
                
                for( sal_uInt16 ic=nCol; ic<nCol+nColSpan; ic++ )
                {
                    SwHTMLTableLayoutColumn *pColumn = GetColumn( ic );

                    sal_uLong nDiff = pColumn->GetMax()-pColumn->GetMin();
                    if( nMinD < nDiff )
                        nDiff = nMinD;

                    pColumn->AddToMin( nDiff );

                    OSL_ENSURE( pColumn->GetMax() >= pColumn->GetMin(),
                            "Why is the Column suddenly too narrow?" );

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
            
            
            
            
            //
            
            
            //
            
            
            
            

            sal_uLong nAbsMin = 0;  
            sal_uLong nRel = 0;     
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
            OSL_ENSURE( 0==nRel || nQuot!=ULONG_MAX,
                    "Where did the relative columns go?" );
            for( i=0; i<nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() )
                {
                    if( pColumn->GetWidthOption() )
                        pColumn->SetMax( pColumn->GetMax() / nQuot );
                    else
                        pColumn->SetMax( pColumn->GetMin() );
                    OSL_ENSURE( pColumn->GetMax() >= pColumn->GetMin(),
                            "Maximum column width is lower than the minimum column width" );
                    nMax += pColumn->GetMax();
                }
            }
        }
        else
        {
            sal_uInt16 nRel = 0;        
            sal_uInt16 nRelCols = 0;    
            sal_uLong nRelMax = 0;      
            for( i=0; i<nCols; i++ )
            {
                OSL_ENSURE( nRel<=100, "relative width of all columns > 100%" );
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() )
                {
                    
                    sal_uInt16 nColWidth = pColumn->GetWidthOption();
                    if( nRel+nColWidth > 100 )
                    {
                        nColWidth = 100 - nRel;
                        pColumn->SetWidthOption( nColWidth, true, false );
                    }
                    nRelMax += pColumn->GetMax();
                    nRel = nRel + nColWidth;
                    nRelCols++;
                }
                else if( !pColumn->GetMin() )
                {
                    
                    
                    nRelCols++;
                }
            }

            
            
            
            
            
            
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
                        
                        sal_uInt16 nColWidth =
                            (sal_uInt16)((pColumn->GetMax() * nRelLeft) / nFixMax);
                        pColumn->SetWidthOption( nColWidth, true, false );
                    }
                }
            }

            
            sal_uLong nQuotMax = ULONG_MAX;
            sal_uLong nOldMax = nMax;
            nMax = 0;
            for( i=0; i<nCols; i++ )
            {
                
                
                
                
                
                
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
            
            OSL_ENSURE( nQuotMax!=ULONG_MAX, "Where did the relative columns go?" );
            for( i=0; i<nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() )
                {
                    if( pColumn->GetWidthOption() )
                    {
                        pColumn->SetMax( pColumn->GetMax() / nQuotMax );
                        OSL_ENSURE( pColumn->GetMax() >= pColumn->GetMin(),
                                "Minimum width is one column bigger than maximum" );
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


/**

    @param nAbsAvail available space in TWIPS.
    @param nRelAvail available space related to USHRT_MAX or 0
    @param nAbsSpace fraction of nAbsAvail, which is reserved by the surrounding
                     cell for the border and the distance to the paragraph.
*/
void SwHTMLTableLayout::AutoLayoutPass2( sal_uInt16 nAbsAvail, sal_uInt16 nRelAvail,
                                         sal_uInt16 nAbsLeftSpace,
                                         sal_uInt16 nAbsRightSpace,
                                         sal_uInt16 nParentInhAbsSpace )
{
    

    
    OSL_ENSURE( nAbsAvail, "AutoLayout pass 2: No absolute width given" );

    
    OSL_ENSURE( IsTopTable() == (nRelAvail==0),
            "AutoLayout pass 2: Relative width at table in table or the other way around" );

    
    OSL_ENSURE( nMin<=nMax, "AutoLayout pass 2: nMin > nMax" );

    
    
    
    nLastResizeAbsAvail = nAbsAvail;

    
    

    
    sal_uInt16 nAbsLeftFill = 0, nAbsRightFill = 0;
    if( !IsTopTable() &&
        GetMin() + nAbsLeftSpace + nAbsRightSpace <= nAbsAvail )
    {
        nAbsLeftFill = nAbsLeftSpace;
        nAbsRightFill = nAbsRightSpace;
    }

    
    if( nLeftMargin || nRightMargin )
    {
        if( IsTopTable() )
        {
            
            
            nAbsAvail -= (nLeftMargin + nRightMargin);
        }
        else if( GetMin() + nLeftMargin + nRightMargin <= nAbsAvail )
        {
            
            
            nAbsLeftFill = nAbsLeftFill + nLeftMargin;
            nAbsRightFill = nAbsRightFill + nRightMargin;
        }
    }

    
    if( !IsTopTable() )
    {
        if( pLeftFillerBox && nAbsLeftFill<MINLAY+nInhLeftBorderWidth )
            nAbsLeftFill = MINLAY+nInhLeftBorderWidth;
        if( pRightFillerBox && nAbsRightFill<MINLAY+nInhRightBorderWidth )
            nAbsRightFill = MINLAY+nInhRightBorderWidth;
    }

    
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

    
    sal_uInt16 nAbsTabWidth = 0;
    bUseRelWidth = false;
    if( nWidthOption )
    {
        if( bPrcWidthOption )
        {
            OSL_ENSURE( nWidthOption<=100, "Percentage value too high" );
            if( nWidthOption > 100 )
                nWidthOption = 100;

            
            
            
            
            //
            
            
            
            //
            
            
            //
            
            
            nAbsTabWidth = (sal_uInt16)( ((sal_uLong)nAbsAvail * nWidthOption) / 100 );
            if( IsTopTable() &&
                ( /*MayBeInFlyFrame() ||*/ (sal_uLong)nAbsTabWidth > nMin ) )
            {
                nRelAvail = USHRT_MAX;
                bUseRelWidth = true;
            }
        }
        else
        {
            nAbsTabWidth = nWidthOption;
            if( nAbsTabWidth > MAX_TABWIDTH )
                nAbsTabWidth = MAX_TABWIDTH;

            
            
            if( !IsTopTable() && nAbsTabWidth > nAbsAvail )
                nAbsTabWidth = nAbsAvail;
        }
    }

    OSL_ENSURE( IsTopTable() || nAbsTabWidth<=nAbsAvail,
            "AutoLayout pass 2: nAbsTabWidth > nAbsAvail for table in table" );
    OSL_ENSURE( !nRelAvail || nAbsTabWidth<=nAbsAvail,
            "AutoLayout pass 2: nAbsTabWidth > nAbsAvail for relative width" );

    
    if( (!IsTopTable() || nRelAvail>0) && nAbsTabWidth>nAbsAvail )
        nAbsTabWidth = nAbsAvail;

    
    
    if( (!IsTopTable() && nMin > (sal_uLong)nAbsAvail) ||
        nMin > MAX_TABWIDTH )
    {
        
        
        
        
        

        nAbsTabWidth = IsTopTable() ? MAX_TABWIDTH : nAbsAvail;
        nRelTabWidth = (nRelAvail ? nRelAvail : nAbsTabWidth );

        
        
        

        sal_uLong nRealMin = 0;
        for( sal_uInt16 i=0; i<nCols; i++ )
        {
            sal_uLong nRealColMin = MINLAY, nDummy1 = 0, nDummy2 = 0;
            AddBorderWidth( nRealColMin, nDummy1, nDummy2, i, 1 );
            nRealMin += nRealColMin;
        }
        if( (nRealMin >= nAbsTabWidth) || (nRealMin >= nMin) )
        {
            
            

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
                sal_uLong nRealColMin = MINLAY, nDummy1 = 0, nDummy2 = 0;
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
        
        
        
        
        
        
        

        
        if( !nAbsTabWidth )
            nAbsTabWidth = (sal_uInt16)nMax;

        
        if( nAbsTabWidth > nAbsAvail )
        {
            OSL_ENSURE( IsTopTable(),
                    "Table in table should get wider than the surrounding cell." );
            nAbsAvail = nAbsTabWidth;
        }

        
        
        sal_uLong nAbsTabWidthL = nAbsTabWidth;
        nRelTabWidth =
            ( nRelAvail ? (sal_uInt16)((nAbsTabWidthL * nRelAvail) / nAbsAvail)
                        : nAbsTabWidth );

        
        sal_uLong nFixMax = nMax;
        for( sal_uInt16 i=0; i<nCols; i++ )
        {
            const SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
            if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption()>0 )
                nFixMax -= pColumn->GetMax();
        }

        if( nFixMax > 0 && nFixMax < nMax )
        {
            
            

            
            
            
            
            
            

            sal_uInt16 nAbs = 0, nRel = 0;
            sal_uInt16 nFixedCols = 0;
            sal_uInt16 i;

            for( i = 0; i < nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( !pColumn->IsRelWidthOption() || !pColumn->GetWidthOption() )
                {
                    
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

            
            
            
            
            OSL_ENSURE( nFixMax == nAbs, "Two loops, two sums?" );
            sal_uLong nDistMax = nMax - nFixMax;
            sal_uInt16 nDistAbsTabWidth = nAbsTabWidth - nAbs;
            sal_uInt16 nDistRelTabWidth = nRelTabWidth - nRel;

            for( i=0; i<nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() > 0 )
                {
                    
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
            OSL_ENSURE( nCols==nFixedCols, "Missed a column!" );
        }
        else
        {
            
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
        
        
        if( !nAbsTabWidth )
            nAbsTabWidth = nAbsAvail;
        if( nAbsTabWidth < nMin )
            nAbsTabWidth = (sal_uInt16)nMin;

        if( nAbsTabWidth > nAbsAvail )
        {
            OSL_ENSURE( IsTopTable(),
                    "A nested table should become wider than the available space." );
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

    
    
    nInhAbsLeftSpace = 0;
    nInhAbsRightSpace = 0;
    if( !IsTopTable() && (nRelLeftFill>0 || nRelRightFill>0 ||
                          nAbsTabWidth<nAbsAvail) )
    {
        
        
        sal_uInt16 nAbsDist = (sal_uInt16)(nAbsAvail-nAbsTabWidth);
        sal_uInt16 nRelDist = (sal_uInt16)(nRelAvail-nRelTabWidth);
        sal_uInt16 nParentInhAbsLeftSpace = 0, nParentInhAbsRightSpace = 0;

        
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

        OSL_ENSURE( !pLeftFillerBox || nRelLeftFill>0,
                "We don't have a width for the left filler box!" );
        OSL_ENSURE( !pRightFillerBox || nRelRightFill>0,
                "We don't have a width for the right filler box!" );

        
        
        
        
        
        if( nRelLeftFill && !pLeftFillerBox &&
            ( nWidthSet>0 || nAbsLeftFill<MINLAY+nInhLeftBorderWidth ||
              (HasColTags() && nAbsLeftFill < nAbsLeftSpace+nParentInhAbsLeftSpace+20) ) )
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
        {
            SwHTMLTableLayoutColumn *pColumn = GetColumn( nCols-1 );
            pColumn->SetAbsColWidth( pColumn->GetAbsColWidth()+nAbsRightFill );
            pColumn->SetRelColWidth( pColumn->GetRelColWidth()+nRelRightFill );
            nRelRightFill = 0;
            nInhAbsRightSpace = nAbsRightSpace + nParentInhAbsRightSpace;
        }
    }
}

static void lcl_ResizeLine( const SwTableLine* pLine, sal_uInt16 *pWidth );

static void lcl_ResizeBox( const SwTableBox* pBox, sal_uInt16* pWidth )
{
    if( !pBox->GetSttNd() )
    {
        sal_uInt16 nWidth = 0;
        BOOST_FOREACH( const SwTableLine *pLine, pBox->GetTabLines() )
            lcl_ResizeLine( pLine, &nWidth );
        pBox->GetFrmFmt()->SetFmtAttr( SwFmtFrmSize( ATT_VAR_SIZE, nWidth, 0 ));
        *pWidth = *pWidth + nWidth;
    }
    else
    {
        *pWidth = *pWidth + (sal_uInt16)pBox->GetFrmFmt()->GetFrmSize().GetSize().Width();
    }
}

static void lcl_ResizeLine( const SwTableLine* pLine, sal_uInt16 *pWidth )
{
    sal_uInt16 nOldWidth = *pWidth;
    *pWidth = 0;
    BOOST_FOREACH( const SwTableBox* pBox, pLine->GetTabBoxes() )
        lcl_ResizeBox(pBox, pWidth );

    SAL_WARN_IF( nOldWidth && std::abs(*pWidth-nOldWidth) >= COLFUZZY, "sw.core",
                 "A box's rows have all a different length" );
}

void SwHTMLTableLayout::SetWidths( sal_Bool bCallPass2, sal_uInt16 nAbsAvail,
                                   sal_uInt16 nRelAvail, sal_uInt16 nAbsLeftSpace,
                                   sal_uInt16 nAbsRightSpace,
                                   sal_uInt16 nParentInhAbsSpace )
{
    
    
    nWidthSet++;

    
    if( bCallPass2 )
        AutoLayoutPass2( nAbsAvail, nRelAvail, nAbsLeftSpace, nAbsRightSpace,
                         nParentInhAbsSpace );

    
    
    
    
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

    
    
    
    
    
    
    if( IsTopTable() )
    {
        sal_uInt16 nCalcTabWidth = 0;
        BOOST_FOREACH( const SwTableLine *pLine, pSwTable->GetTabLines() )
            lcl_ResizeLine( pLine, &nCalcTabWidth );
        SAL_WARN_IF( std::abs( nRelTabWidth-nCalcTabWidth ) >= COLFUZZY, "sw.core",
                     "Table width is not equal to the row width" );

        
        
        
        SwFrmFmt *pFrmFmt = pSwTable->GetFrmFmt();
        ((SwTable *)pSwTable)->LockModify();
        SwFmtFrmSize aFrmSize( pFrmFmt->GetFrmSize() );
        aFrmSize.SetWidth( nRelTabWidth );
        bool bRel = bUseRelWidth &&
                    text::HoriOrientation::FULL!=pFrmFmt->GetHoriOrient().GetHoriOrient();
        aFrmSize.SetWidthPercent( (sal_uInt8)(bRel ? nWidthOption : 0) );
        pFrmFmt->SetFmtAttr( aFrmSize );
        ((SwTable *)pSwTable)->UnlockModify();

        
        
        if( MayBeInFlyFrame() )
        {
            SwFrmFmt *pFlyFrmFmt = FindFlyFrmFmt();
            if( pFlyFrmFmt )
            {
                SwFmtFrmSize aFlyFrmSize( ATT_VAR_SIZE, nRelTabWidth, MINLAY );

                if( bUseRelWidth )
                {
                    
                    aFlyFrmSize.SetWidth(  nMin > USHRT_MAX ? USHRT_MAX
                                                            : nMin );
                    aFlyFrmSize.SetWidthPercent( (sal_uInt8)nWidthOption );
                }
                pFlyFrmFmt->SetFmtAttr( aFlyFrmSize );
            }
        }

#ifdef DBG_UTIL
        {
            
            extern void _CheckBoxWidth( const SwTableLine&, SwTwips );

            
            SwTwips nSize = pSwTable->GetFrmFmt()->GetFrmSize().GetWidth();
            const SwTableLines& rLines = pSwTable->GetTabLines();
            for (size_t n = 0; n < rLines.size(); ++n)
            {
                _CheckBoxWidth( *rLines[ n ], nSize );
            }
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
    
    
    if( bRecalc )
        AutoLayoutPass1();

    SwRootFrm *pRoot = (SwRootFrm*)GetDoc()->GetCurrentViewShell()->GetLayout();
    if ( pRoot && pRoot->IsCallbackActionEnabled() )
        pRoot->StartAllAction();

    
    SetWidths( sal_True, nAbsAvail );

    if ( pRoot && pRoot->IsCallbackActionEnabled() )
        pRoot->EndAllAction( sal_True );    
}

IMPL_STATIC_LINK( SwHTMLTableLayout, DelayedResize_Impl, void*, EMPTYARG )
{
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
    OSL_ENSURE( IsTopTable(), "Resize must only be called for top tables!" );

    
    if( bMustNotResize && !bForce )
        return sal_False;

    
    if( bMustNotRecalc && !bForce )
        bRecalc = sal_False;

    const SwDoc *pDoc = GetDoc();

    
    
    
    
    if( pDoc->GetCurrentViewShell() && pDoc->GetCurrentViewShell()->GetViewOptions()->getBrowseMode() )
    {
        const sal_uInt16 nVisAreaWidth = GetBrowseWidthByVisArea( *pDoc );
        if( nVisAreaWidth < nAbsAvail && !FindFlyFrmFmt() )
            nAbsAvail = nVisAreaWidth;
    }

    if( nDelay==0 && aResizeTimer.IsActive() )
    {
        
        
        bRecalc |= bDelayedResizeRecalc;
        nDelayedResizeAbsAvail = nAbsAvail;
        return sal_False;
    }

    
    
    
    
    
    
    
    
    
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
