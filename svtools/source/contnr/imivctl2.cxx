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

#include "imivctl.hxx"

IcnCursor_Impl::IcnCursor_Impl( SvxIconChoiceCtrl_Impl* pOwner )
{
    pView       = pOwner;
    pCurEntry   = 0;
    nDeltaWidth = 0;
    nDeltaHeight= 0;
    nCols       = 0;
    nRows       = 0;
}

IcnCursor_Impl::~IcnCursor_Impl()
{
}

sal_uInt16 IcnCursor_Impl::GetSortListPos( SvxIconChoiceCtrlEntryPtrVec& rList, long nValue,
    bool bVertical )
{
    sal_uInt16 nCount = rList.size();
    if( !nCount )
        return 0;

    sal_uInt16 nCurPos = 0;
    long nPrevValue = LONG_MIN;
    while( nCount )
    {
        const Rectangle& rRect = pView->GetEntryBoundRect( rList[nCurPos] );
        long nCurValue;
        if( bVertical )
            nCurValue = rRect.Top();
        else
            nCurValue = rRect.Left();
        if( nValue >= nPrevValue && nValue <= nCurValue )
            return (sal_uInt16)nCurPos;
        nPrevValue = nCurValue;
        nCount--;
        nCurPos++;
    }
    return rList.size();
}

void IcnCursor_Impl::ImplCreate()
{
    pView->CheckBoundingRects();
    DBG_ASSERT(xColumns==0&&xRows==0,"ImplCreate: Not cleared");

    SetDeltas();

    xColumns.reset(new IconChoiceMap);
    xRows.reset(new IconChoiceMap);

    size_t nCount = pView->aEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = pView->aEntries[ nCur ];
        // const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
        Rectangle rRect( pView->CalcBmpRect( pEntry,0 ) );
        short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / nDeltaHeight );
        short nX = (short)( ((rRect.Left()+rRect.Right())/2) / nDeltaWidth );

        // capture rounding errors
        if( nY >= nRows )
            nY = sal::static_int_cast< short >(nRows - 1);
        if( nX >= nCols )
            nX = sal::static_int_cast< short >(nCols - 1);

        SvxIconChoiceCtrlEntryPtrVec& rColEntry = (*xColumns)[nX];
        sal_uInt16 nIns = GetSortListPos( rColEntry, rRect.Top(), true );
        rColEntry.insert( rColEntry.begin() + nIns, pEntry );

        SvxIconChoiceCtrlEntryPtrVec& rRowEntry = (*xRows)[nY];
        nIns = GetSortListPos( rRowEntry, rRect.Left(), false );
        rRowEntry.insert( rRowEntry.begin() + nIns, pEntry );

        pEntry->nX = nX;
        pEntry->nY = nY;
    }
}




void IcnCursor_Impl::Clear()
{
    if( xColumns )
    {
        xColumns.reset();
        xRows.reset();
        pCurEntry = 0;
        nDeltaWidth = 0;
        nDeltaHeight = 0;
    }
}

SvxIconChoiceCtrlEntry* IcnCursor_Impl::SearchCol(sal_uInt16 nCol, sal_uInt16 nTop, sal_uInt16 nBottom,
    sal_uInt16, bool bDown, bool bSimple )
{
    DBG_ASSERT(pCurEntry, "SearchCol: No reference entry");
    IconChoiceMap::iterator mapIt = xColumns->find( nCol );
    if ( mapIt == xColumns->end() )
        return 0;
    SvxIconChoiceCtrlEntryPtrVec const & rList = mapIt->second;
    const sal_uInt16 nCount = rList.size();
    if( !nCount )
        return 0;

    const Rectangle& rRefRect = pView->GetEntryBoundRect(pCurEntry);

    if( bSimple )
    {
        SvxIconChoiceCtrlEntryPtrVec::const_iterator it = std::find( rList.begin(), rList.end(), pCurEntry );

        assert(it != rList.end()); //Entry not in Col-List
        if (it == rList.end())
            return 0;

        if( bDown )
        {
            while( ++it != rList.end() )
            {
                SvxIconChoiceCtrlEntry* pEntry = *it;
                const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
                if( rRect.Top() > rRefRect.Top() )
                    return pEntry;
            }
            return 0;
        }
        else
        {
            SvxIconChoiceCtrlEntryPtrVec::const_reverse_iterator it2(it);
            while (it2 != rList.rend())
            {
                SvxIconChoiceCtrlEntry* pEntry = *it2;
                const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
                if( rRect.Top() < rRefRect.Top() )
                    return pEntry;
                ++it2;
            }
            return 0;
        }
    }

    if( nTop > nBottom )
    {
        sal_uInt16 nTemp = nTop;
        nTop = nBottom;
        nBottom = nTemp;
    }
    long nMinDistance = LONG_MAX;
    SvxIconChoiceCtrlEntry* pResult = 0;
    for( sal_uInt16 nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = rList[ nCur ];
        if( pEntry != pCurEntry )
        {
            sal_uInt16 nY = pEntry->nY;
            if( nY >= nTop && nY <= nBottom )
            {
                const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
                long nDistance = rRect.Top() - rRefRect.Top();
                if( nDistance < 0 )
                    nDistance *= -1;
                if( nDistance && nDistance < nMinDistance )
                {
                    nMinDistance = nDistance;
                    pResult = pEntry;
                }
            }
        }
    }
    return pResult;
}

SvxIconChoiceCtrlEntry* IcnCursor_Impl::SearchRow(sal_uInt16 nRow, sal_uInt16 nLeft, sal_uInt16 nRight,
    sal_uInt16, bool bRight, bool bSimple )
{
    DBG_ASSERT(pCurEntry,"SearchRow: No reference entry");
    IconChoiceMap::iterator mapIt = xRows->find( nRow );
    if ( mapIt == xRows->end() )
        return 0;
    SvxIconChoiceCtrlEntryPtrVec const & rList = mapIt->second;
    const sal_uInt16 nCount = rList.size();
    if( !nCount )
        return 0;

    const Rectangle& rRefRect = pView->GetEntryBoundRect(pCurEntry);

    if( bSimple )
    {
        SvxIconChoiceCtrlEntryPtrVec::const_iterator it = std::find( rList.begin(), rList.end(), pCurEntry );

        assert(it != rList.end()); //Entry not in Row-List
        if (it == rList.end())
            return 0;

        if( bRight )
        {
            while( ++it != rList.end() )
            {
                SvxIconChoiceCtrlEntry* pEntry = *it;
                const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
                if( rRect.Left() > rRefRect.Left() )
                    return pEntry;
            }
            return 0;
        }
        else
        {
            SvxIconChoiceCtrlEntryPtrVec::const_reverse_iterator it2(it);
            while (it2 != rList.rend())
            {
                SvxIconChoiceCtrlEntry* pEntry = *it2;
                const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
                if( rRect.Left() < rRefRect.Left() )
                    return pEntry;
                ++it2;
            }
            return 0;
        }

    }
    if( nRight < nLeft )
    {
        sal_uInt16 nTemp = nRight;
        nRight = nLeft;
        nLeft = nTemp;
    }
    long nMinDistance = LONG_MAX;
    SvxIconChoiceCtrlEntry* pResult = 0;
    for( sal_uInt16 nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = rList[ nCur ];
        if( pEntry != pCurEntry )
        {
            sal_uInt16 nX = pEntry->nX;
            if( nX >= nLeft && nX <= nRight )
            {
                const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
                long nDistance = rRect.Left() - rRefRect.Left();
                if( nDistance < 0 )
                    nDistance *= -1;
                if( nDistance && nDistance < nMinDistance )
                {
                    nMinDistance = nDistance;
                    pResult = pEntry;
                }
            }
        }
    }
    return pResult;
}



/*
    Searches, starting from the passed value, the next entry to the left/to the
    right. Example for bRight = sal_True:

                  c
                b c
              a b c
            S 1 1 1      ====> search direction
              a b c
                b c
                  c

    S : starting position
    1 : first searched rectangle
    a,b,c : 2nd, 3rd, 4th searched rectangle
*/

SvxIconChoiceCtrlEntry* IcnCursor_Impl::GoLeftRight( SvxIconChoiceCtrlEntry* pCtrlEntry, bool bRight )
{
    SvxIconChoiceCtrlEntry* pResult;
    pCurEntry = pCtrlEntry;
    Create();
    sal_uInt16 nY = pCtrlEntry->nY;
    sal_uInt16 nX = pCtrlEntry->nX;
    DBG_ASSERT(nY< nRows,"GoLeftRight:Bad column");
    DBG_ASSERT(nX< nCols,"GoLeftRight:Bad row");
    // neighbor in same row?
    if( bRight )
        pResult = SearchRow(
            nY, nX, sal::static_int_cast< sal_uInt16 >(nCols-1), nX, true, true );
    else
        pResult = SearchRow( nY, nX ,0, nX, false, true );
    if( pResult )
        return pResult;

    long nCurCol = nX;

    long nColOffs, nLastCol;
    if( bRight )
    {
        nColOffs = 1;
        nLastCol = nCols;
    }
    else
    {
        nColOffs = -1;
        nLastCol = -1;   // 0-1
    }

    sal_uInt16 nRowMin = nY;
    sal_uInt16 nRowMax = nY;
    do
    {
        SvxIconChoiceCtrlEntry* pEntry = SearchCol((sal_uInt16)nCurCol,nRowMin,nRowMax,nY,true, false);
        if( pEntry )
            return pEntry;
        if( nRowMin )
            nRowMin--;
        if( nRowMax < (nRows-1))
            nRowMax++;
        nCurCol += nColOffs;
    } while( nCurCol != nLastCol );
    return 0;
}

SvxIconChoiceCtrlEntry* IcnCursor_Impl::GoPageUpDown( SvxIconChoiceCtrlEntry* pStart, bool bDown)
{
    if( pView->IsAutoArrange() && !(pView->nWinBits & WB_ALIGN_TOP) )
    {
        const long nPos = (long)pView->GetEntryListPos( pStart );
        long nEntriesInView = (pView->aOutputSize.Height() / pView->nGridDY);
        nEntriesInView *=
            ((pView->aOutputSize.Width()+(pView->nGridDX/2)) / pView->nGridDX );
        long nNewPos = nPos;
        if( bDown )
        {
            nNewPos += nEntriesInView;
            if( nNewPos >= (long)pView->aEntries.size() )
                nNewPos = pView->aEntries.size() - 1;
        }
        else
        {
            nNewPos -= nEntriesInView;
            if( nNewPos < 0 )
                nNewPos = 0;
        }
        if( nPos != nNewPos )
            return pView->aEntries[ (size_t)nNewPos ];
        return 0;
    }
    long nOpt = pView->GetEntryBoundRect( pStart ).Top();
    if( bDown )
    {
        nOpt += pView->aOutputSize.Height();
        nOpt -= pView->nGridDY;
    }
    else
    {
        nOpt -= pView->aOutputSize.Height();
        nOpt += pView->nGridDY;
    }
    if( nOpt < 0 )
        nOpt = 0;

    long nPrevErr = LONG_MAX;

    SvxIconChoiceCtrlEntry* pPrev = pStart;
    SvxIconChoiceCtrlEntry* pNext = GoUpDown( pStart, bDown );
    while( pNext )
    {
        long nCur = pView->GetEntryBoundRect( pNext ).Top();
        long nErr = nOpt - nCur;
        if( nErr < 0 )
            nErr *= -1;
        if( nErr > nPrevErr )
            return pPrev;
        nPrevErr = nErr;
        pPrev = pNext;
        pNext = GoUpDown( pNext, bDown );
    }
    if( pPrev != pStart )
        return pPrev;
    return 0;
}

SvxIconChoiceCtrlEntry* IcnCursor_Impl::GoUpDown( SvxIconChoiceCtrlEntry* pCtrlEntry, bool bDown)
{
    if( pView->IsAutoArrange() && !(pView->nWinBits & WB_ALIGN_TOP) )
    {
        sal_uLong nPos = pView->GetEntryListPos( pCtrlEntry );
        if( bDown && nPos < (pView->aEntries.size() - 1) )
            return pView->aEntries[ nPos + 1 ];
        else if( !bDown && nPos > 0 )
            return pView->aEntries[ nPos - 1 ];
        return 0;
    }

    SvxIconChoiceCtrlEntry* pResult;
    pCurEntry = pCtrlEntry;
    Create();
    sal_uInt16 nY = pCtrlEntry->nY;
    sal_uInt16 nX = pCtrlEntry->nX;
    DBG_ASSERT(nY<nRows,"GoUpDown:Bad column");
    DBG_ASSERT(nX<nCols,"GoUpDown:Bad row");

    // neighbor in same column?
    if( bDown )
        pResult = SearchCol(
            nX, nY, sal::static_int_cast< sal_uInt16 >(nRows-1), nY, true, true );
    else
        pResult = SearchCol( nX, nY ,0, nY, false, true );
    if( pResult )
        return pResult;

    long nCurRow = nY;

    long nRowOffs, nLastRow;
    if( bDown )
    {
        nRowOffs = 1;
        nLastRow = nRows;
    }
    else
    {
        nRowOffs = -1;
        nLastRow = -1;   // 0-1
    }

    sal_uInt16 nColMin = nX;
    sal_uInt16 nColMax = nX;
    do
    {
        SvxIconChoiceCtrlEntry* pEntry = SearchRow((sal_uInt16)nCurRow,nColMin,nColMax,nX,true, false);
        if( pEntry )
            return pEntry;
        if( nColMin )
            nColMin--;
        if( nColMax < (nCols-1))
            nColMax++;
        nCurRow += nRowOffs;
    } while( nCurRow != nLastRow );
    return 0;
}

void IcnCursor_Impl::SetDeltas()
{
    const Size& rSize = pView->aVirtOutputSize;
    nCols = rSize.Width() / pView->nGridDX;
    if( !nCols )
        nCols = 1;
    nRows = rSize.Height() / pView->nGridDY;
    if( (nRows * pView->nGridDY) < rSize.Height() )
        nRows++;
    if( !nRows )
        nRows = 1;

    nDeltaWidth = (short)(rSize.Width() / nCols);
    nDeltaHeight = (short)(rSize.Height() / nRows);
    if( !nDeltaHeight )
    {
        nDeltaHeight = 1;
        DBG_WARNING("SetDeltas:Bad height");
    }
    if( !nDeltaWidth )
    {
        nDeltaWidth = 1;
        DBG_WARNING("SetDeltas:Bad width");
    }
}

void IcnCursor_Impl::CreateGridAjustData( IconChoiceMap& rLists, SvxIconChoiceCtrlEntry* pRefEntry)
{
    if( !pRefEntry )
    {
        sal_uInt16 nGridRows = (sal_uInt16)(pView->aVirtOutputSize.Height() / pView->nGridDY);
        nGridRows++; // because we round down later!

        if( !nGridRows )
            return;
        const size_t nCount = pView->aEntries.size();
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = pView->aEntries[ nCur ];
            const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
            short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / pView->nGridDY );
            sal_uInt16 nIns = GetSortListPos( rLists[nY], rRect.Left(), false );
            rLists[ nY ].insert( rLists[ nY ].begin() + nIns, pEntry );
        }
    }
    else
    {
        // build a horizontal "tube" in the RefEntry line
        // STOP AND THINK: maybe use bounding rectangle because of overlaps?
        Rectangle rRefRect( pView->CalcBmpRect( pRefEntry ) );
        //const Rectangle& rRefRect = pView->GetEntryBoundRect( pRefEntry );
        short nRefRow = (short)( ((rRefRect.Top()+rRefRect.Bottom())/2) / pView->nGridDY );
        SvxIconChoiceCtrlEntryPtrVec& rRow = rLists[0];
        size_t nCount = pView->aEntries.size();
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = pView->aEntries[ nCur ];
            Rectangle rRect( pView->CalcBmpRect(pEntry) );
            //const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
            short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / pView->nGridDY );
            if( nY == nRefRow )
            {
                sal_uInt16 nIns = GetSortListPos( rRow, rRect.Left(), false );
                rRow.insert( rRow.begin() + nIns, pEntry );
            }
        }
    }
}

//static
void IcnCursor_Impl::DestroyGridAdjustData( IconChoiceMap& rLists )
{
    rLists.clear();
}

IcnGridMap_Impl::IcnGridMap_Impl(SvxIconChoiceCtrl_Impl* pView)
{
    _pView = pView;
    _pGridMap = 0;
    _nGridCols = 0;
    _nGridRows = 0;
}

IcnGridMap_Impl::~IcnGridMap_Impl()
{
    delete[] _pGridMap, _pGridMap=0;
}

void IcnGridMap_Impl::Expand()
{
    if( !_pGridMap )
        Create_Impl();
    else
    {
        sal_uInt16 nNewGridRows = _nGridRows;
        sal_uInt16 nNewGridCols = _nGridCols;
        if( _pView->nWinBits & WB_ALIGN_TOP )
            nNewGridRows += 50;
        else
            nNewGridCols += 50;

        size_t nNewCellCount = static_cast<size_t>(nNewGridRows) * nNewGridCols;
        bool* pNewGridMap = new bool[nNewCellCount];
        size_t nOldCellCount = static_cast<size_t>(_nGridRows) * _nGridCols;
        memcpy(pNewGridMap, _pGridMap, nOldCellCount * sizeof(bool));
        memset(pNewGridMap + nOldCellCount, 0, (nNewCellCount-nOldCellCount) * sizeof(bool));
        delete[] _pGridMap;
        _pGridMap = pNewGridMap;
        _nGridRows = nNewGridRows;
        _nGridCols = nNewGridCols;
    }
}

void IcnGridMap_Impl::Create_Impl()
{
    DBG_ASSERT(!_pGridMap,"Unnecessary call to IcnGridMap_Impl::Create_Impl()");
    if( _pGridMap )
        return;
    GetMinMapSize( _nGridCols, _nGridRows );
    if( _pView->nWinBits & WB_ALIGN_TOP )
        _nGridRows += 50;  // avoid resize of gridmap too often
    else
        _nGridCols += 50;

    size_t nCellCount = static_cast<size_t>(_nGridRows) * _nGridCols;
    _pGridMap = new bool[nCellCount];
    memset(_pGridMap, 0, nCellCount * sizeof(bool));

    const size_t nCount = _pView->aEntries.size();
    for( size_t nCur=0; nCur < nCount; nCur++ )
        OccupyGrids( _pView->aEntries[ nCur ] );
}

void IcnGridMap_Impl::GetMinMapSize( sal_uInt16& rDX, sal_uInt16& rDY ) const
{
    long nX, nY;
    if( _pView->nWinBits & WB_ALIGN_TOP )
    {
        // The view grows in vertical direction. Its max. width is _pView->nMaxVirtWidth
        nX = _pView->nMaxVirtWidth;
        if( !nX )
            nX = _pView->pView->GetOutputSizePixel().Width();
        if( !(_pView->nFlags & F_ARRANGING) )
            nX -= _pView->nVerSBarWidth;

        nY = _pView->aVirtOutputSize.Height();
    }
    else
    {
        // The view grows in horizontal direction. Its max. height is _pView->nMaxVirtHeight
        nY = _pView->nMaxVirtHeight;
        if( !nY )
            nY = _pView->pView->GetOutputSizePixel().Height();
        if( !(_pView->nFlags & F_ARRANGING) )
            nY -= _pView->nHorSBarHeight;
        nX = _pView->aVirtOutputSize.Width();
    }

    if( !nX )
        nX = DEFAULT_MAX_VIRT_WIDTH;
    if( !nY )
        nY = DEFAULT_MAX_VIRT_HEIGHT;

    long nDX = nX / _pView->nGridDX;
    long nDY = nY / _pView->nGridDY;

    if( !nDX )
        nDX++;
    if( !nDY )
        nDY++;

    rDX = (sal_uInt16)nDX;
    rDY = (sal_uInt16)nDY;
}

GridId IcnGridMap_Impl::GetGrid( sal_uInt16 nGridX, sal_uInt16 nGridY )
{
    Create();
    if( _pView->nWinBits & WB_ALIGN_TOP )
        return nGridX + ( static_cast<GridId>(nGridY) * _nGridCols );
    else
        return nGridY + ( static_cast<GridId>(nGridX) * _nGridRows );
}

GridId IcnGridMap_Impl::GetGrid( const Point& rDocPos, bool* pbClipped )
{
    Create();

    long nX = rDocPos.X();
    long nY = rDocPos.Y();
    nX -= LROFFS_WINBORDER;
    nY -= TBOFFS_WINBORDER;
    nX /= _pView->nGridDX;
    nY /= _pView->nGridDY;
    bool bClipped = false;
    if( nX >= _nGridCols )
    {
        nX = _nGridCols - 1;
        bClipped = true;
    }
    if( nY >= _nGridRows )
    {
        nY = _nGridRows - 1;
        bClipped = true;
    }
    GridId nId = GetGrid( (sal_uInt16)nX, (sal_uInt16)nY );
    if( pbClipped )
        *pbClipped = bClipped;
    DBG_ASSERT(nId <(sal_uLong)(_nGridCols*_nGridRows),"GetGrid failed");
    return nId;
}

Rectangle IcnGridMap_Impl::GetGridRect( GridId nId )
{
    Create();
    sal_uInt16 nGridX, nGridY;
    GetGridCoord( nId, nGridX, nGridY );
    const long nLeft = nGridX * _pView->nGridDX+ LROFFS_WINBORDER;
    const long nTop = nGridY * _pView->nGridDY + TBOFFS_WINBORDER;
    return Rectangle(
        nLeft, nTop,
        nLeft + _pView->nGridDX,
        nTop + _pView->nGridDY );
}

GridId IcnGridMap_Impl::GetUnoccupiedGrid( bool bOccupyFound )
{
    Create();
    sal_uLong nStart = 0;
    bool bExpanded = false;

    while( true )
    {
        const sal_uLong nCount = (sal_uInt16)(_nGridCols * _nGridRows);
        for( sal_uLong nCur = nStart; nCur < nCount; nCur++ )
        {
            if( !_pGridMap[ nCur ] )
            {
                if( bOccupyFound )
                    _pGridMap[ nCur ] = true;
                return (GridId)nCur;
            }
        }
        DBG_ASSERT(!bExpanded,"ExpandGrid failed");
        if( bExpanded )
            return 0; // prevent never ending loop
        bExpanded = true;
        Expand();
        nStart = nCount;
    }
}

// An entry only means that there's a GridRect lying under its center. This
// variant is much faster than allocating via the bounding rectangle but can
// lead to small overlaps.
void IcnGridMap_Impl::OccupyGrids( const SvxIconChoiceCtrlEntry* pEntry, bool bOccupy )
{
    if( !_pGridMap || !SvxIconChoiceCtrl_Impl::IsBoundingRectValid( pEntry->aRect ))
        return;
    OccupyGrid( GetGrid( pEntry->aRect.Center()), bOccupy );
}

void IcnGridMap_Impl::Clear()
{
    if( _pGridMap )
    {
        delete[] _pGridMap, _pGridMap=0;
        _nGridRows = 0;
        _nGridCols = 0;
        _aLastOccupiedGrid.SetEmpty();
    }
}

sal_uLong IcnGridMap_Impl::GetGridCount( const Size& rSizePixel, sal_uInt16 nDX, sal_uInt16 nDY)
{
    long ndx = (rSizePixel.Width() - LROFFS_WINBORDER) / nDX;
    if( ndx < 0 ) ndx *= -1;
    long ndy = (rSizePixel.Height() - TBOFFS_WINBORDER) / nDY;
    if( ndy < 0 ) ndy *= -1;
    return (sal_uLong)(ndx * ndy);
}

void IcnGridMap_Impl::OutputSizeChanged()
{
    if( _pGridMap )
    {
        sal_uInt16 nCols, nRows;
        GetMinMapSize( nCols, nRows );
        if( _pView->nWinBits & WB_ALIGN_TOP )
        {
            if( nCols != _nGridCols )
                Clear();
            else if( nRows >= _nGridRows )
                Expand();
        }
        else
        {
            if( nRows != _nGridRows )
                Clear();
            else if( nCols >= _nGridCols )
                Expand();
        }
    }
}

// Independently of the view's alignment (TOP or LEFT), the gridmap
// should contain the data in a continuous region, to make it possible
// to copy the whole block if the gridmap needs to be expanded.
void IcnGridMap_Impl::GetGridCoord( GridId nId, sal_uInt16& rGridX, sal_uInt16& rGridY )
{
    Create();
    if( _pView->nWinBits & WB_ALIGN_TOP )
    {
        rGridX = (sal_uInt16)(nId % _nGridCols);
        rGridY = (sal_uInt16)(nId / _nGridCols);
    }
    else
    {
        rGridX = (sal_uInt16)(nId / _nGridRows);
        rGridY = (sal_uInt16)(nId % _nGridRows);
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
