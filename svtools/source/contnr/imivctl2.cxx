/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_svtools.hxx"
#include "imivctl.hxx"

IcnCursor_Impl::IcnCursor_Impl( SvxIconChoiceCtrl_Impl* pOwner )
{
    pView       = pOwner;
    pColumns    = 0;
    pRows       = 0;
    pCurEntry   = 0;
    nDeltaWidth = 0;
    nDeltaHeight= 0;
    nCols       = 0;
    nRows       = 0;
}

IcnCursor_Impl::~IcnCursor_Impl()
{
    delete[] pColumns;
    delete[] pRows;
}

sal_uInt16 IcnCursor_Impl::GetSortListPos( SvPtrarr* pList, long nValue,
    int bVertical )
{
    sal_uInt16 nCount = (sal_uInt16)pList->Count();
    if( !nCount )
        return 0;

    sal_uInt16 nCurPos = 0;
    long nPrevValue = LONG_MIN;
    while( nCount )
    {
        const Rectangle& rRect=
            pView->GetEntryBoundRect((SvxIconChoiceCtrlEntry*)(pList->GetObject(nCurPos)));
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
    return pList->Count();
}

void IcnCursor_Impl::ImplCreate()
{
    pView->CheckBoundingRects();
    DBG_ASSERT(pColumns==0&&pRows==0,"ImplCreate: Not cleared");

    SetDeltas();

    pColumns = new SvPtrarr[ nCols ];
    pRows = new SvPtrarr[ nRows ];

    sal_uLong nCount = pView->aEntries.Count();
    for( sal_uLong nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( nCur );
        // const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
        Rectangle rRect( pView->CalcBmpRect( pEntry,0 ) );
        short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / nDeltaHeight );
        short nX = (short)( ((rRect.Left()+rRect.Right())/2) / nDeltaWidth );

        // Rundungsfehler abfangen
        if( nY >= nRows )
            nY = sal::static_int_cast< short >(nRows - 1);
        if( nX >= nCols )
            nX = sal::static_int_cast< short >(nCols - 1);

        sal_uInt16 nIns = GetSortListPos( &pColumns[nX], rRect.Top(), sal_True );
        pColumns[ nX ].Insert( pEntry, nIns );

        nIns = GetSortListPos( &pRows[nY], rRect.Left(), sal_False );
        pRows[ nY ].Insert( pEntry, nIns );

        pEntry->nX = nX;
        pEntry->nY = nY;
    }
}




void IcnCursor_Impl::Clear()
{
    if( pColumns )
    {
        delete[] pColumns;
        delete[] pRows;
        pColumns = 0;
        pRows = 0;
        pCurEntry = 0;
        nDeltaWidth = 0;
        nDeltaHeight = 0;
    }
}

SvxIconChoiceCtrlEntry* IcnCursor_Impl::SearchCol(sal_uInt16 nCol,sal_uInt16 nTop,sal_uInt16 nBottom,
    sal_uInt16, sal_Bool bDown, sal_Bool bSimple  )
{
    DBG_ASSERT(pCurEntry,"SearchCol: No reference entry");
    SvPtrarr* pList = &(pColumns[ nCol ]);
    const sal_uInt16 nCount = pList->Count();
    if( !nCount )
        return 0;

    const Rectangle& rRefRect = pView->GetEntryBoundRect(pCurEntry);

    if( bSimple )
    {
        sal_uInt16 nListPos = pList->GetPos( pCurEntry );
        DBG_ASSERT(nListPos!=0xffff,"Entry not in Col-List");
        if( bDown )
        {
            while( nListPos < nCount-1 )
            {
                nListPos++;
                SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)pList->GetObject( nListPos );
                const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
                if( rRect.Top() > rRefRect.Top() )
                    return pEntry;
            }
            return 0;
        }
        else
        {
            while( nListPos )
            {
                nListPos--;
                if( nListPos < nCount )
                {
                    SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)pList->GetObject( nListPos );
                    const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
                    if( rRect.Top() < rRefRect.Top() )
                        return pEntry;
                }
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
        SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)(pList->GetObject( nCur ));
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

SvxIconChoiceCtrlEntry* IcnCursor_Impl::SearchRow(sal_uInt16 nRow,sal_uInt16 nLeft,sal_uInt16 nRight,
    sal_uInt16, sal_Bool bRight, sal_Bool bSimple )
{
    DBG_ASSERT(pCurEntry,"SearchRow: No reference entry");
    SvPtrarr* pList = &(pRows[ nRow ]);
    const sal_uInt16 nCount = pList->Count();
    if( !nCount )
        return 0;

    const Rectangle& rRefRect = pView->GetEntryBoundRect(pCurEntry);

    if( bSimple )
    {
        sal_uInt16 nListPos = pList->GetPos( pCurEntry );
        DBG_ASSERT(nListPos!=0xffff,"Entry not in Row-List");
        if( bRight )
        {
            while( nListPos < nCount-1 )
            {
                nListPos++;
                SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)pList->GetObject( nListPos );
                const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
                if( rRect.Left() > rRefRect.Left() )
                    return pEntry;
            }
            return 0;
        }
        else
        {
            while( nListPos )
            {
                nListPos--;
                if( nListPos < nCount )
                {
                    SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)pList->GetObject( nListPos );
                    const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
                    if( rRect.Left() < rRefRect.Left() )
                        return pEntry;
                }
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
        SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)(pList->GetObject( nCur ));
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
    Sucht ab dem uebergebenen Eintrag den naechsten rechts- bzw.
    linksstehenden. Suchverfahren am Beispiel bRight = sal_True:

                  c
                b c
              a b c
            S 1 1 1      ====> Suchrichtung
              a b c
                b c
                  c

    S : Startposition
    1 : erstes Suchrechteck
    a,b,c : 2., 3., 4. Suchrechteck
*/

SvxIconChoiceCtrlEntry* IcnCursor_Impl::GoLeftRight( SvxIconChoiceCtrlEntry* pCtrlEntry, sal_Bool bRight )
{
    SvxIconChoiceCtrlEntry* pResult;
    pCurEntry = pCtrlEntry;
    Create();
    sal_uInt16 nY = pCtrlEntry->nY;
    sal_uInt16 nX = pCtrlEntry->nX;
    DBG_ASSERT(nY< nRows,"GoLeftRight:Bad column");
    DBG_ASSERT(nX< nCols,"GoLeftRight:Bad row");
    // Nachbar auf gleicher Zeile ?
    if( bRight )
        pResult = SearchRow(
            nY, nX, sal::static_int_cast< sal_uInt16 >(nCols-1), nX, sal_True, sal_True );
    else
        pResult = SearchRow( nY, nX ,0, nX, sal_False, sal_True );
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
        SvxIconChoiceCtrlEntry* pEntry = SearchCol((sal_uInt16)nCurCol,nRowMin,nRowMax,nY,sal_True, sal_False);
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

SvxIconChoiceCtrlEntry* IcnCursor_Impl::GoPageUpDown( SvxIconChoiceCtrlEntry* pStart, sal_Bool bDown)
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
            if( nNewPos >= (long)pView->aEntries.Count() )
                nNewPos = pView->aEntries.Count() - 1;
        }
        else
        {
            nNewPos -= nEntriesInView;
            if( nNewPos < 0 )
                nNewPos = 0;
        }
        if( nPos != nNewPos )
            return (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( (sal_uLong)nNewPos );
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

SvxIconChoiceCtrlEntry* IcnCursor_Impl::GoUpDown( SvxIconChoiceCtrlEntry* pCtrlEntry, sal_Bool bDown)
{
    if( pView->IsAutoArrange() && !(pView->nWinBits & WB_ALIGN_TOP) )
    {
        sal_uLong nPos = pView->GetEntryListPos( pCtrlEntry );
        if( bDown && nPos < (pView->aEntries.Count() - 1) )
            return (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( nPos + 1 );
        else if( !bDown && nPos > 0 )
            return (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( nPos - 1 );
        return 0;
    }

    SvxIconChoiceCtrlEntry* pResult;
    pCurEntry = pCtrlEntry;
    Create();
    sal_uInt16 nY = pCtrlEntry->nY;
    sal_uInt16 nX = pCtrlEntry->nX;
    DBG_ASSERT(nY<nRows,"GoUpDown:Bad column");
    DBG_ASSERT(nX<nCols,"GoUpDown:Bad row");

    // Nachbar in gleicher Spalte ?
    if( bDown )
        pResult = SearchCol(
            nX, nY, sal::static_int_cast< sal_uInt16 >(nRows-1), nY, sal_True, sal_True );
    else
        pResult = SearchCol( nX, nY ,0, nY, sal_False, sal_True );
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
        SvxIconChoiceCtrlEntry* pEntry = SearchRow((sal_uInt16)nCurRow,nColMin,nColMax,nX,sal_True, sal_False);
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

void IcnCursor_Impl::CreateGridAjustData( SvPtrarr& rLists, SvxIconChoiceCtrlEntry* pRefEntry)
{
    if( !pRefEntry )
    {
        sal_uInt16 nGridRows = (sal_uInt16)(pView->aVirtOutputSize.Height() / pView->nGridDY);
        nGridRows++; // wg. Abrundung!

        if( !nGridRows )
            return;
        for( sal_uInt16 nCurList = 0; nCurList < nGridRows; nCurList++ )
        {
            SvPtrarr* pRow = new SvPtrarr;
            rLists.Insert( (void*)pRow, nCurList );
        }
        const sal_uLong nCount = pView->aEntries.Count();
        for( sal_uLong nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( nCur );
            const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
            short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / pView->nGridDY );
            sal_uInt16 nIns = GetSortListPos((SvPtrarr*)rLists[nY],rRect.Left(),sal_False);
            ((SvPtrarr*)rLists[ nY ])->Insert( pEntry, nIns );
        }
    }
    else
    {
        // Aufbau eines hor. "Schlauchs" auf der RefEntry-Zeile
        // UEBERLEGEN: BoundingRect nehmen wg. Ueberlappungen???
        Rectangle rRefRect( pView->CalcBmpRect( pRefEntry ) );
        //const Rectangle& rRefRect = pView->GetEntryBoundRect( pRefEntry );
        short nRefRow = (short)( ((rRefRect.Top()+rRefRect.Bottom())/2) / pView->nGridDY );
        SvPtrarr* pRow = new SvPtrarr;
        rLists.Insert( (void*)pRow, 0 );
        sal_uLong nCount = pView->aEntries.Count();
        for( sal_uLong nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( nCur );
            Rectangle rRect( pView->CalcBmpRect(pEntry) );
            //const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
            short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / pView->nGridDY );
            if( nY == nRefRow )
            {
                sal_uInt16 nIns = GetSortListPos( pRow, rRect.Left(), sal_False );
                pRow->Insert( pEntry, nIns );
            }
        }
    }
}

//static
void IcnCursor_Impl::DestroyGridAdjustData( SvPtrarr& rLists )
{
    const sal_uInt16 nCount = rLists.Count();
    for( sal_uInt16 nCur = 0; nCur < nCount; nCur++ )
    {
        SvPtrarr* pArr = (SvPtrarr*)rLists[ nCur ];
        delete pArr;
    }
    rLists.Remove( 0, rLists.Count() );
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

        sal_Bool* pNewGridMap = new sal_Bool[nNewGridRows*nNewGridCols];
        memset( pNewGridMap, 0, nNewGridRows * nNewGridCols * sizeof(sal_Bool) );
        memcpy( pNewGridMap, _pGridMap, _nGridRows * _nGridCols * sizeof(sal_Bool) );
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

    _pGridMap = new sal_Bool[ _nGridRows * _nGridCols];
    memset( (void*)_pGridMap, 0, _nGridRows * _nGridCols );

    const sal_uLong nCount = _pView->aEntries.Count();
    for( sal_uLong nCur=0; nCur < nCount; nCur++ )
        OccupyGrids( (SvxIconChoiceCtrlEntry*)_pView->aEntries.GetObject( nCur ));
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
        return nGridX + ( nGridY * _nGridCols );
    else
        return nGridY + ( nGridX * _nGridRows );
}

GridId IcnGridMap_Impl::GetGrid( const Point& rDocPos, sal_Bool* pbClipped )
{
    Create();

    long nX = rDocPos.X();
    long nY = rDocPos.Y();
    nX -= LROFFS_WINBORDER;
    nY -= TBOFFS_WINBORDER;
    nX /= _pView->nGridDX;
    nY /= _pView->nGridDY;
    sal_Bool bClipped = sal_False;
    if( nX >= _nGridCols )
    {
        nX = _nGridCols - 1;
        bClipped = sal_True;
    }
    if( nY >= _nGridRows )
    {
        nY = _nGridRows - 1;
        bClipped = sal_True;
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

GridId IcnGridMap_Impl::GetUnoccupiedGrid( sal_Bool bOccupyFound )
{
    Create();
    sal_uLong nStart = 0;
    sal_Bool bExpanded = sal_False;

    while( 1 )
    {
        const sal_uLong nCount = (sal_uInt16)(_nGridCols * _nGridRows);
        for( sal_uLong nCur = nStart; nCur < nCount; nCur++ )
        {
            if( !_pGridMap[ nCur ] )
            {
                if( bOccupyFound )
                    _pGridMap[ nCur ] = sal_True;
                return (GridId)nCur;
            }
        }
        DBG_ASSERT(!bExpanded,"ExpandGrid failed");
        if( bExpanded )
            return 0; // prevent never ending loop
        bExpanded = sal_True;
        Expand();
        nStart = nCount;
    }
}

// ein Eintrag belegt nur das unter seinem Zentrum liegende GridRect
// diese Variante ist bedeutend schneller als die Belegung ueber das
// Bounding-Rect, kann aber zu kleinen Ueberlappungen fuehren
#define OCCUPY_CENTER

void IcnGridMap_Impl::OccupyGrids( const SvxIconChoiceCtrlEntry* pEntry, sal_Bool bOccupy )
{
    if( !_pGridMap || !_pView->IsBoundingRectValid( pEntry->aRect ))
        return;
#ifndef OCCUPY_CENTER
    OccupyGrids( pEntry->aRect, bOccupy );
#else
    OccupyGrid( GetGrid( pEntry->aRect.Center()), bOccupy );
#endif

}

void IcnGridMap_Impl::OccupyGrids( const Rectangle& rRect, sal_Bool bUsed )
{
    if( !_pGridMap )
        return;

    if( bUsed )
    {
        if( _aLastOccupiedGrid == rRect )
            return;
        _aLastOccupiedGrid = rRect;
    }
    else
        _aLastOccupiedGrid.SetEmpty();

    sal_Bool bTopLeftClipped, bBottomRightClipped;
    GridId nIdTL = GetGrid( rRect.TopLeft(), &bTopLeftClipped );
    GridId nIdBR = GetGrid( rRect.BottomRight(), &bBottomRightClipped );

    if( bTopLeftClipped && bBottomRightClipped )
        return;

    sal_uInt16 nX1,nX2,nY1,nY2;
    GetGridCoord( nIdTL, nX1, nY1 );
    GetGridCoord( nIdBR, nX2, nY2 );
    sal_uInt16 nTemp;
    if( nX1 > nX2 )
    {
        nTemp = nX1;
        nX1 = nX2;
        nX2 = nTemp;
    }
    if( nY1 > nY2 )
    {
        nTemp = nY1;
        nY1 = nY2;
        nY2 = nTemp;
    }
    for( ; nX1 <= nX2; nX1++ )
        for( ; nY1 <= nY2; nY1++ )
            OccupyGrid( GetGrid( nX1, nY1 ) );
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

// Independendly of the views alignment (TOP or LEFT) the gridmap
// should contain the data in a continues region, to make it possible
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
