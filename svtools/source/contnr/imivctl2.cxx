/*************************************************************************
 *
 *  $RCSfile: imivctl2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 15:41:30 $
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

#ifndef _IMPICNVW_HXX
#include "imivctl.hxx"
#endif

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

USHORT IcnCursor_Impl::GetSortListPos( SvPtrarr* pList, long nValue,
    int bVertical )
{
    USHORT nCount = (USHORT)pList->Count();
    if( !nCount )
        return 0;

    USHORT nCurPos = 0;
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
            return (USHORT)nCurPos;
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

    ULONG nCount = pView->aEntries.Count();
    for( ULONG nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( nCur );
        // const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
        Rectangle rRect( pView->CalcBmpRect( pEntry,0 ) );
        short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / nDeltaHeight );
        short nX = (short)( ((rRect.Left()+rRect.Right())/2) / nDeltaWidth );

        // Rundungsfehler abfangen
        if( nY >= nRows )
            nY = nRows - 1;
        if( nX >= nCols )
            nX = nCols - 1;

        USHORT nIns = GetSortListPos( &pColumns[nX], rRect.Top(), TRUE );
        pColumns[ nX ].Insert( pEntry, nIns );

        nIns = GetSortListPos( &pRows[nY], rRect.Left(), FALSE );
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

SvxIconChoiceCtrlEntry* IcnCursor_Impl::SearchCol(USHORT nCol,USHORT nTop,USHORT nBottom,
    USHORT nPref, BOOL bDown, BOOL bSimple  )
{
    DBG_ASSERT(pCurEntry,"SearchCol: No reference entry");
    SvPtrarr* pList = &(pColumns[ nCol ]);
    const USHORT nCount = pList->Count();
    if( !nCount )
        return 0;

    const Rectangle& rRefRect = pView->GetEntryBoundRect(pCurEntry);

    if( bSimple )
    {
        USHORT nListPos = pList->GetPos( pCurEntry );
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
        USHORT nTemp = nTop;
        nTop = nBottom;
        nBottom = nTemp;
    }
    long nMinDistance = LONG_MAX;
    SvxIconChoiceCtrlEntry* pResult = 0;
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)(pList->GetObject( nCur ));
        if( pEntry != pCurEntry )
        {
            USHORT nY = pEntry->nY;
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

SvxIconChoiceCtrlEntry* IcnCursor_Impl::SearchRow(USHORT nRow,USHORT nLeft,USHORT nRight,
    USHORT nPref, BOOL bRight, BOOL bSimple )
{
    DBG_ASSERT(pCurEntry,"SearchRow: No reference entry");
    SvPtrarr* pList = &(pRows[ nRow ]);
    const USHORT nCount = pList->Count();
    if( !nCount )
        return 0;

    const Rectangle& rRefRect = pView->GetEntryBoundRect(pCurEntry);

    if( bSimple )
    {
        USHORT nListPos = pList->GetPos( pCurEntry );
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
        USHORT nTemp = nRight;
        nRight = nLeft;
        nLeft = nTemp;
    }
    long nMinDistance = LONG_MAX;
    SvxIconChoiceCtrlEntry* pResult = 0;
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)(pList->GetObject( nCur ));
        if( pEntry != pCurEntry )
        {
            USHORT nX = pEntry->nX;
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
    linksstehenden. Suchverfahren am Beispiel bRight = TRUE:

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

SvxIconChoiceCtrlEntry* IcnCursor_Impl::GoLeftRight( SvxIconChoiceCtrlEntry* pEntry, BOOL bRight )
{
    SvxIconChoiceCtrlEntry* pResult;
    pCurEntry = pEntry;
    Create();
    USHORT nY = pEntry->nY;
    USHORT nX = pEntry->nX;
    DBG_ASSERT(nY< nRows,"GoLeftRight:Bad column");
    DBG_ASSERT(nX< nCols,"GoLeftRight:Bad row");
    // Nachbar auf gleicher Zeile ?
    if( bRight )
        pResult = SearchRow( nY, nX ,nCols-1, nX, TRUE, TRUE );
    else
        pResult = SearchRow( nY, nX ,0, nX, FALSE, TRUE );
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

    USHORT nRowMin = nY;
    USHORT nRowMax = nY;
    do
    {
        SvxIconChoiceCtrlEntry* pEntry = SearchCol((USHORT)nCurCol,nRowMin,nRowMax,nY,TRUE, FALSE);
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

SvxIconChoiceCtrlEntry* IcnCursor_Impl::GoPageUpDown( SvxIconChoiceCtrlEntry* pStart, BOOL bDown)
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
            return (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( (ULONG)nNewPos );
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

SvxIconChoiceCtrlEntry* IcnCursor_Impl::GoUpDown( SvxIconChoiceCtrlEntry* pEntry, BOOL bDown)
{
    if( pView->IsAutoArrange() && !(pView->nWinBits & WB_ALIGN_TOP) )
    {
        ULONG nPos = pView->GetEntryListPos( pEntry );
        if( bDown && nPos < (pView->aEntries.Count() - 1) )
            return (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( nPos + 1 );
        else if( !bDown && nPos > 0 )
            return (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( nPos - 1 );
        return 0;
    }

    SvxIconChoiceCtrlEntry* pResult;
    pCurEntry = pEntry;
    Create();
    USHORT nY = pEntry->nY;
    USHORT nX = pEntry->nX;
    DBG_ASSERT(nY<nRows,"GoUpDown:Bad column");
    DBG_ASSERT(nX<nCols,"GoUpDown:Bad row");

    // Nachbar in gleicher Spalte ?
    if( bDown )
        pResult = SearchCol( nX, nY ,nRows-1, nY, TRUE, TRUE );
    else
        pResult = SearchCol( nX, nY ,0, nY, FALSE, TRUE );
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

    USHORT nColMin = nX;
    USHORT nColMax = nX;
    do
    {
        SvxIconChoiceCtrlEntry* pEntry = SearchRow((USHORT)nCurRow,nColMin,nColMax,nX,TRUE, FALSE);
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
        USHORT nRows = (USHORT)(pView->aVirtOutputSize.Height() / pView->nGridDY);
        nRows++; // wg. Abrundung!

        if( !nRows )
            return;
        for( USHORT nCurList = 0; nCurList < nRows; nCurList++ )
        {
            SvPtrarr* pRow = new SvPtrarr;
            rLists.Insert( (void*)pRow, nCurList );
        }
        const ULONG nCount = pView->aEntries.Count();
        for( ULONG nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( nCur );
            const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
            short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / pView->nGridDY );
            USHORT nIns = GetSortListPos((SvPtrarr*)rLists[nY],rRect.Left(),FALSE);
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
        ULONG nCount = pView->aEntries.Count();
        for( ULONG nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = (SvxIconChoiceCtrlEntry*)pView->aEntries.GetObject( nCur );
            Rectangle rRect( pView->CalcBmpRect(pEntry) );
            //const Rectangle& rRect = pView->GetEntryBoundRect( pEntry );
            short nY = (short)( ((rRect.Top()+rRect.Bottom())/2) / pView->nGridDY );
            if( nY == nRefRow )
            {
                USHORT nIns = GetSortListPos( pRow, rRect.Left(), FALSE );
                pRow->Insert( pEntry, nIns );
            }
        }
    }
}

//static
void IcnCursor_Impl::DestroyGridAdjustData( SvPtrarr& rLists )
{
    const USHORT nCount = rLists.Count();
    for( USHORT nCur = 0; nCur < nCount; nCur++ )
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
        USHORT nNewGridRows = _nGridRows;
        USHORT nNewGridCols = _nGridCols;
        if( _pView->nWinBits & WB_ALIGN_TOP )
            nNewGridRows += 50;
        else
            nNewGridCols += 50;

        BOOL* pNewGridMap = new BOOL[nNewGridRows*nNewGridCols];
        memset( pNewGridMap, 0, nNewGridRows * nNewGridCols * sizeof(BOOL) );
        memcpy( pNewGridMap, _pGridMap, _nGridRows * _nGridCols * sizeof(BOOL) );
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

    _pGridMap = new BOOL[ _nGridRows * _nGridCols];
    memset( (void*)_pGridMap, 0, _nGridRows * _nGridCols );

    const ULONG nCount = _pView->aEntries.Count();
    for( ULONG nCur=0; nCur < nCount; nCur++ )
        OccupyGrids( (SvxIconChoiceCtrlEntry*)_pView->aEntries.GetObject( nCur ));
}

void IcnGridMap_Impl::GetMinMapSize( USHORT& rDX, USHORT& rDY ) const
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

    rDX = (USHORT)nDX;
    rDY = (USHORT)nDY;
}

GridId IcnGridMap_Impl::GetGrid( USHORT nGridX, USHORT nGridY )
{
    Create();
    if( _pView->nWinBits & WB_ALIGN_TOP )
        return nGridX + ( nGridY * _nGridCols );
    else
        return nGridY + ( nGridX * _nGridRows );
}

GridId IcnGridMap_Impl::GetGrid( const Point& rDocPos, BOOL* pbClipped )
{
    Create();

    long nX = rDocPos.X();
    long nY = rDocPos.Y();
    nX -= LROFFS_WINBORDER;
    nY -= TBOFFS_WINBORDER;
    nX /= _pView->nGridDX;
    nY /= _pView->nGridDY;
    BOOL bClipped = FALSE;
    if( nX >= _nGridCols )
    {
        nX = _nGridCols - 1;
        bClipped = TRUE;
    }
    if( nY >= _nGridRows )
    {
        nY = _nGridRows - 1;
        bClipped = TRUE;
    }
    GridId nId = GetGrid( (USHORT)nX, (USHORT)nY );
    if( pbClipped )
        *pbClipped = bClipped;
    DBG_ASSERT(nId <(ULONG)(_nGridCols*_nGridRows),"GetGrid failed");
    return nId;
}

Rectangle IcnGridMap_Impl::GetGridRect( GridId nId )
{
    Create();
    USHORT nGridX, nGridY;
    GetGridCoord( nId, nGridX, nGridY );
    const long nLeft = nGridX * _pView->nGridDX+ LROFFS_WINBORDER;
    const long nTop = nGridY * _pView->nGridDY + TBOFFS_WINBORDER;
    return Rectangle(
        nLeft, nTop,
        nLeft + _pView->nGridDX,
        nTop + _pView->nGridDY );
}

GridId IcnGridMap_Impl::GetUnoccupiedGrid( BOOL bOccupyFound )
{
    Create();
    ULONG nStart = 0;
    BOOL bExpanded = FALSE;

    while( 1 )
    {
        const ULONG nCount = (USHORT)(_nGridCols * _nGridRows);
        for( ULONG nCur = nStart; nCur < nCount; nCur++ )
        {
            if( !_pGridMap[ nCur ] )
            {
                if( bOccupyFound )
                    _pGridMap[ nCur ] = TRUE;
                return (GridId)nCur;
            }
        }
        DBG_ASSERT(!bExpanded,"ExpandGrid failed");
        if( bExpanded )
            return 0; // prevent never ending loop
        bExpanded = TRUE;
        Expand();
        nStart = nCount;
    }
    return 0;
}

// ein Eintrag belegt nur das unter seinem Zentrum liegende GridRect
// diese Variante ist bedeutend schneller als die Belegung ueber das
// Bounding-Rect, kann aber zu kleinen Ueberlappungen fuehren
#define OCCUPY_CENTER

void IcnGridMap_Impl::OccupyGrids( const SvxIconChoiceCtrlEntry* pEntry, BOOL bOccupy )
{
    if( !_pGridMap || !_pView->IsBoundingRectValid( pEntry->aRect ))
        return;
#ifndef OCCUPY_CENTER
    OccupyGrids( pEntry->aRect, bOccupy );
#else
    OccupyGrid( GetGrid( pEntry->aRect.Center()), bOccupy );
#endif

}

void IcnGridMap_Impl::OccupyGrids( const Rectangle& rRect, BOOL bUsed )
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

    BOOL bTopLeftClipped, bBottomRightClipped;
    GridId nIdTL = GetGrid( rRect.TopLeft(), &bTopLeftClipped );
    GridId nIdBR = GetGrid( rRect.BottomRight(), &bBottomRightClipped );

    if( bTopLeftClipped && bBottomRightClipped )
        return;

    USHORT nX1,nX2,nY1,nY2;
    GetGridCoord( nIdTL, nX1, nY1 );
    GetGridCoord( nIdBR, nX2, nY2 );
    USHORT nTemp;
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

ULONG IcnGridMap_Impl::GetGridCount( const Size& rSizePixel, USHORT nDX, USHORT nDY)
{
    long ndx = (rSizePixel.Width() - LROFFS_WINBORDER) / nDX;
    if( ndx < 0 ) ndx *= -1;
    long ndy = (rSizePixel.Height() - TBOFFS_WINBORDER) / nDY;
    if( ndy < 0 ) ndy *= -1;
    return (ULONG)(ndx * ndy);
}

void IcnGridMap_Impl::OutputSizeChanged()
{
    if( _pGridMap )
    {
        USHORT nCols, nRows;
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
void IcnGridMap_Impl::GetGridCoord( GridId nId, USHORT& rGridX, USHORT& rGridY )
{
    Create();
    if( _pView->nWinBits & WB_ALIGN_TOP )
    {
        rGridX = (USHORT)(nId % _nGridCols);
        rGridY = (USHORT)(nId / _nGridCols);
    }
    else
    {
        rGridX = (USHORT)(nId / _nGridRows);
        rGridY = (USHORT)(nId % _nGridRows);
    }
}



