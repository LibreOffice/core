/*************************************************************************
 *
 *  $RCSfile: markdata.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:15 $
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

// INCLUDE ---------------------------------------------------------------

#include <tools/debug.hxx>

#include "markdata.hxx"
#include "markarr.hxx"
#include "rangelst.hxx"

// STATIC DATA -----------------------------------------------------------

//------------------------------------------------------------------------

ScMarkData::ScMarkData() :
    pMultiSel( NULL )
{
    for (USHORT i=0; i<=MAXTAB; i++)
        bTabMarked[i] = FALSE;

    ResetMark();
}

ScMarkData::ScMarkData(const ScMarkData& rData) :
    pMultiSel( NULL ),
    aMarkRange( rData.aMarkRange ),
    aMultiRange( rData.aMultiRange )
{
    bMarked      = rData.bMarked;
    bMultiMarked = rData.bMultiMarked;
    bMarking     = rData.bMarking;
    bMarkIsNeg   = rData.bMarkIsNeg;

    USHORT i;
    for (i=0; i<=MAXTAB; i++)
        bTabMarked[i] = rData.bTabMarked[i];

    if (rData.pMultiSel)
    {
        pMultiSel = new ScMarkArray[MAXCOL+1];
        for (i=0; i<=MAXCOL; i++)
            rData.pMultiSel[i].CopyMarksTo( pMultiSel[i] );
    }
}

ScMarkData& ScMarkData::operator=(const ScMarkData& rData)
{
    if ( &rData == this )
        return *this;

    delete[] pMultiSel;
    pMultiSel = NULL;

    aMarkRange   = rData.aMarkRange;
    aMultiRange  = rData.aMultiRange;
    bMarked      = rData.bMarked;
    bMultiMarked = rData.bMultiMarked;
    bMarking     = rData.bMarking;
    bMarkIsNeg   = rData.bMarkIsNeg;

    USHORT i;
    for (i=0; i<=MAXTAB; i++)
        bTabMarked[i] = rData.bTabMarked[i];

    if (rData.pMultiSel)
    {
        pMultiSel = new ScMarkArray[MAXCOL+1];
        for (i=0; i<=MAXCOL; i++)
            rData.pMultiSel[i].CopyMarksTo( pMultiSel[i] );
    }

    return *this;
}

ScMarkData::~ScMarkData()
{
    delete[] pMultiSel;
}

void ScMarkData::ResetMark()
{
    delete[] pMultiSel;
    pMultiSel = NULL;

    bMarked = bMultiMarked = FALSE;
    bMarking = bMarkIsNeg = FALSE;
}

void ScMarkData::SetMarkArea( const ScRange& rRange )
{
    aMarkRange = rRange;
    aMarkRange.Justify();
    if ( !bMarked )
    {
        // #77987# Upon creation of a document ScFormatShell GetTextAttrState
        // may query (default) attributes although no sheet is marked yet.
        // => mark that one.
        if ( !GetSelectCount() )
            bTabMarked[ aMarkRange.aStart.Tab() ] = TRUE;
        bMarked = TRUE;
    }
}

void ScMarkData::GetMarkArea( ScRange& rRange ) const
{
    rRange = aMarkRange;        //! inline ?
}

void ScMarkData::GetMultiMarkArea( ScRange& rRange ) const
{
    rRange = aMultiRange;
}

void ScMarkData::SetMultiMarkArea( const ScRange& rRange, BOOL bMark )
{
    if (!pMultiSel)
        pMultiSel = new ScMarkArray[MAXCOL+1];

    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartCol, nEndCol );

    USHORT nCol;
    for (nCol=nStartCol; nCol<=nEndCol; nCol++)
        pMultiSel[nCol].SetMarkArea( nStartRow, nEndRow, bMark );

    if ( bMultiMarked )                 // aMultiRange updaten
    {
        if ( nStartCol < aMultiRange.aStart.Col() )
            aMultiRange.aStart.SetCol( nStartCol );
        if ( nStartRow < aMultiRange.aStart.Row() )
            aMultiRange.aStart.SetRow( nStartRow );
        if ( nEndCol > aMultiRange.aEnd.Col() )
            aMultiRange.aEnd.SetCol( nEndCol );
        if ( nEndRow > aMultiRange.aEnd.Row() )
            aMultiRange.aEnd.SetRow( nEndRow );
    }
    else
    {
        aMultiRange = rRange;           // neu
        bMultiMarked = TRUE;
    }
}

void ScMarkData::SetAreaTab( USHORT nTab )
{
    aMarkRange.aStart.SetTab(nTab);
    aMarkRange.aEnd.SetTab(nTab);
    aMultiRange.aStart.SetTab(nTab);
    aMultiRange.aEnd.SetTab(nTab);
}

void ScMarkData::SelectOneTable( USHORT nTab )
{
    for (USHORT i=0; i<=MAXTAB; i++)
        bTabMarked[i] = ( nTab == i );
}

USHORT ScMarkData::GetSelectCount() const
{
    USHORT nCount = 0;
    for (USHORT i=0; i<=MAXTAB; i++)
        if (bTabMarked[i])
            ++nCount;

    return nCount;
}

USHORT ScMarkData::GetFirstSelected() const
{
    for (USHORT i=0; i<=MAXTAB; i++)
        if (bTabMarked[i])
            return i;

    DBG_ERROR("GetFirstSelected: keine markiert");
    return 0;
}

void ScMarkData::MarkToMulti()
{
    if ( bMarked && !bMarking )
    {
        SetMultiMarkArea( aMarkRange, !bMarkIsNeg );
        bMarked = FALSE;
    }
}

void ScMarkData::MarkToSimple()
{
    if ( bMultiMarked && !bMarking )
    {
        DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

        if (bMarked)
            MarkToMulti();

        ScRange aNew = aMultiRange;

        BOOL bOk = FALSE;
        USHORT nStartCol = aNew.aStart.Col();
        USHORT nEndCol   = aNew.aEnd.Col();

        while ( nStartCol < nEndCol && !pMultiSel[nStartCol].HasMarks() )
            ++nStartCol;
        while ( nStartCol < nEndCol && !pMultiSel[nEndCol].HasMarks() )
            --nEndCol;

        //  Zeilen werden nur aus MarkArray genommen
        USHORT nStartRow, nEndRow;
        if ( pMultiSel[nStartCol].HasOneMark( nStartRow, nEndRow ) )
        {
            bOk = TRUE;
            USHORT nCmpStart, nCmpEnd;
            for (USHORT nCol=nStartCol+1; nCol<=nEndCol && bOk; nCol++)
                if ( !pMultiSel[nCol].HasOneMark( nCmpStart, nCmpEnd )
                        || nCmpStart != nStartRow || nCmpEnd != nEndRow )
                    bOk = FALSE;
        }

        if (bOk)
        {
            aNew.aStart.SetCol(nStartCol);
            aNew.aStart.SetRow(nStartRow);
            aNew.aEnd.SetCol(nEndCol);
            aNew.aEnd.SetRow(nEndRow);

            ResetMark();
            aMarkRange = aNew;
            bMarked = TRUE;
            bMarkIsNeg = FALSE;
        }
    }
}

BOOL ScMarkData::IsCellMarked( USHORT nCol, USHORT nRow, BOOL bNoSimple ) const
{
    if ( bMarked && !bNoSimple && !bMarkIsNeg )
        if ( aMarkRange.aStart.Col() <= nCol && aMarkRange.aEnd.Col() >= nCol &&
             aMarkRange.aStart.Row() <= nRow && aMarkRange.aEnd.Row() >= nRow )
            return TRUE;

    if (bMultiMarked)
    {
        //! hier auf negative Markierung testen ?

        DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");
        return pMultiSel[nCol].GetMark( nRow );
    }

    return FALSE;
}

BOOL ScMarkData::IsColumnMarked( USHORT nCol ) const
{
    //  bMarkIsNeg inzwischen auch fuer Spaltenkoepfe
    //! GetMarkColumnRanges fuer komplett markierte Spalten

    if ( bMarked && !bMarkIsNeg &&
                    aMarkRange.aStart.Col() <= nCol && aMarkRange.aEnd.Col() >= nCol &&
                    aMarkRange.aStart.Row() == 0    && aMarkRange.aEnd.Row() == MAXROW )
        return TRUE;

    if ( bMultiMarked && pMultiSel[nCol].IsAllMarked(0,MAXROW) )
        return TRUE;

    return FALSE;
}

BOOL ScMarkData::IsRowMarked( USHORT nRow ) const
{
    //  bMarkIsNeg inzwischen auch fuer Zeilenkoepfe
    //! GetMarkRowRanges fuer komplett markierte Zeilen

    if ( bMarked && !bMarkIsNeg &&
                    aMarkRange.aStart.Col() == 0    && aMarkRange.aEnd.Col() == MAXCOL &&
                    aMarkRange.aStart.Row() <= nRow && aMarkRange.aEnd.Row() >= nRow )
        return TRUE;

    if ( bMultiMarked )
    {
        DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");
        for (USHORT nCol=0; nCol<=MAXCOL; nCol++)
            if (!pMultiSel[nCol].GetMark(nRow))
                return FALSE;
        return TRUE;
    }

    return FALSE;
}

void ScMarkData::MarkFromRangeList( const ScRangeList& rList, BOOL bReset )
{
    if (bReset)
    {
        for (USHORT i=0; i<=MAXTAB; i++)
            bTabMarked[i] = FALSE;              // Tabellen sind nicht in ResetMark
        ResetMark();
    }

    ULONG nCount = rList.Count();
    if ( nCount == 1 && !bMarked && !bMultiMarked )
    {
        ScRange aRange = *rList.GetObject(0);
        SetMarkArea( aRange );
        SelectTable( aRange.aStart.Tab(), TRUE );
    }
    else
    {
        for (ULONG i=0; i<nCount; i++)
        {
            ScRange aRange = *rList.GetObject(i);
            SetMultiMarkArea( aRange, TRUE );
            SelectTable( aRange.aStart.Tab(), TRUE );
        }
    }
}

void ScMarkData::FillRangeListWithMarks( ScRangeList* pList, BOOL bClear ) const
{
    if (!pList)
        return;

    if (bClear)
        pList->RemoveAll();

    //!     bei mehreren selektierten Tabellen mehrere Ranges eintragen !!!

    if ( bMultiMarked )
    {
        DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

        USHORT nTab = aMultiRange.aStart.Tab();

        USHORT nStartCol = aMultiRange.aStart.Col();
        USHORT nEndCol = aMultiRange.aEnd.Col();
        for (USHORT nCol=nStartCol; nCol<=nEndCol; nCol++)
            if (pMultiSel[nCol].HasMarks())
            {
                USHORT nTop, nBottom;
                ScRange aRange( nCol, 0, nTab );
                ScMarkArrayIter aMarkIter( &pMultiSel[nCol] );
                while ( aMarkIter.Next( nTop, nBottom ) )
                {
                    aRange.aStart.SetRow( nTop );
                    aRange.aEnd.SetRow( nBottom );
                    pList->Join( aRange );
                }
            }
    }

    if ( bMarked )
        pList->Append( aMarkRange );
}

void ScMarkData::ExtendRangeListTables( ScRangeList* pList ) const
{
    if (!pList)
        return;

    ScRangeList aOldList(*pList);
    pList->RemoveAll();                 //! oder die vorhandenen unten weglassen

    for (USHORT nTab=0; nTab<=MAXTAB; nTab++)
        if (bTabMarked[nTab])
        {
            ULONG nCount = aOldList.Count();
            for (ULONG i=0; i<nCount; i++)
            {
                ScRange aRange = *aOldList.GetObject(i);
                aRange.aStart.SetTab(nTab);
                aRange.aEnd.SetTab(nTab);
                pList->Append( aRange );
            }
        }
}

USHORT ScMarkData::GetMarkColumnRanges( USHORT* pRanges )
{
    if (bMarked)
        MarkToMulti();

    if (!bMultiMarked)
        return 0;

    DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

    USHORT nRangeCnt = 0;
    USHORT nStart = 0;
    while (nStart<=MAXCOL)
    {
        while (nStart<MAXCOL && !pMultiSel[nStart].HasMarks())
            ++nStart;
        if (pMultiSel[nStart].HasMarks())
        {
            USHORT nEnd = nStart;
            while (nEnd<MAXCOL && pMultiSel[nEnd].HasMarks())
                ++nEnd;
            if (!pMultiSel[nEnd].HasMarks())
                --nEnd;
            pRanges[2*nRangeCnt  ] = nStart;
            pRanges[2*nRangeCnt+1] = nEnd;
            ++nRangeCnt;
            nStart = nEnd+1;
        }
        else
            nStart = MAXCOL+1;
    }

    return nRangeCnt;
}

USHORT ScMarkData::GetMarkRowRanges( USHORT* pRanges )
{
    if (bMarked)
        MarkToMulti();

    if (!bMultiMarked)
        return 0;

    DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

    //  Welche Zeilen sind markiert?

    BOOL*   bMarked = new BOOL[MAXROW+1];
    USHORT  nRow;
    USHORT  nCol;
    for (nRow=0; nRow<=MAXROW; nRow++)
        bMarked[nRow] = FALSE;

    USHORT nTop;
    USHORT nBottom;
    for (nCol=0; nCol<=MAXCOL; nCol++)
    {
        ScMarkArrayIter aMarkIter( &pMultiSel[nCol] );
        while (aMarkIter.Next( nTop, nBottom ))
            for (nRow=nTop; nRow<=nBottom; nRow++)
                bMarked[nRow] = TRUE;
    }

    //  zu Bereichen zusammenfassen

    USHORT nRangeCnt = 0;
    USHORT nStart = 0;
    while (nStart<=MAXROW)
    {
        while (nStart<MAXROW && !bMarked[nStart])
            ++nStart;
        if (bMarked[nStart])
        {
            USHORT nEnd = nStart;
            while (nEnd<MAXROW && bMarked[nEnd])
                ++nEnd;
            if (!bMarked[nEnd])
                --nEnd;
            pRanges[2*nRangeCnt  ] = nStart;
            pRanges[2*nRangeCnt+1] = nEnd;
            ++nRangeCnt;
            nStart = nEnd+1;
        }
        else
            nStart = MAXROW+1;
    }

    delete[] bMarked;
    return nRangeCnt;
}

BOOL ScMarkData::IsAllMarked( const ScRange& rRange ) const
{
    if ( !bMultiMarked )
        return FALSE;

    DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    BOOL bOk = TRUE;
    for (USHORT nCol=nStartCol; nCol<=nEndCol && bOk; nCol++)
        if ( !pMultiSel[nCol].IsAllMarked( nStartRow, nEndRow ) )
            bOk = FALSE;

    return bOk;
}

short ScMarkData::GetNextMarked( USHORT nCol, short nRow, BOOL bUp ) const
{
    if ( !bMultiMarked )
        return nRow;

    DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

    return pMultiSel[nCol].GetNextMarked( nRow, bUp );
}

BOOL ScMarkData::HasMultiMarks( USHORT nCol ) const
{
    if ( !bMultiMarked )
        return FALSE;

    DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

    return pMultiSel[nCol].HasMarks();
}

BOOL ScMarkData::HasAnyMultiMarks() const
{
    if ( !bMultiMarked )
        return FALSE;

    DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

    for (USHORT nCol=0; nCol<=MAXCOL; nCol++)
        if ( pMultiSel[nCol].HasMarks() )
            return TRUE;

    return FALSE;       // nix
}

void ScMarkData::InsertTab( USHORT nTab )
{
    for (USHORT i=MAXTAB; i>nTab; i--)
        bTabMarked[i] = bTabMarked[i-1];
    bTabMarked[nTab] = FALSE;
}

void ScMarkData::DeleteTab( USHORT nTab )
{
    for (USHORT i=nTab; i<MAXTAB; i++)
        bTabMarked[i] = bTabMarked[i+1];
    bTabMarked[MAXTAB] = FALSE;
}





