/*************************************************************************
 *
 *  $RCSfile: markarr.cxx,v $
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

#include "markarr.hxx"
#include "global.hxx"

// STATIC DATA -----------------------------------------------------------

//------------------------------------------------------------------------

ScMarkArray::ScMarkArray()
{
    nCount = 1;
    pData = new ScMarkEntry[1];
    if (pData)
    {
        pData[0].nRow = MAXROW;
        pData[0].bMarked = FALSE;
    }
}

//------------------------------------------------------------------------

ScMarkArray::~ScMarkArray()
{
    if (pData)
        delete[] pData;
}

//------------------------------------------------------------------------

void ScMarkArray::Reset( BOOL bMarked )
{
    if (pData)
    {
        delete[] pData;

        nCount = 1;
        pData = new ScMarkEntry[1];
        if (pData)
        {
            pData[0].nRow = MAXROW;
            pData[0].bMarked = bMarked;
        }
    }
}

//------------------------------------------------------------------------

BOOL ScMarkArray::Search( USHORT nRow, short& nIndex ) const
{
    short   nLo         = 0;
    short   nHi         = ((short) nCount) - 1;
    short   nStartRow   = 0;
    short   nEndRow     = 0;
    short   i           = 0;
    BOOL    bFound      = (nCount == 1);
    if (pData)
    {
        while ( !bFound && nLo <= nHi )
        {
            i = (nLo + nHi) / 2;
            if (i > 0)
                nStartRow = (short) pData[i - 1].nRow;
            else
                nStartRow = -1;
            nEndRow = (short) pData[i].nRow;
            if (nEndRow < (short) nRow)
                nLo = ++i;
            else
                if (nStartRow >= (short) nRow)
                    nHi = --i;
                else
                    bFound = TRUE;
        }
    }
    else
        bFound = FALSE;

    if (bFound)
        nIndex=i;
    else
        nIndex=0;
    return bFound;
}

BOOL ScMarkArray::GetMark( USHORT nRow ) const
{
    short i;
    if (Search( nRow, i ))
        return pData[i].bMarked;
    else
        return FALSE;

}

//------------------------------------------------------------------------

void ScMarkArray::SetMark( USHORT nRow, BOOL bMarked )
{
    SetMarkArea( nRow, nRow, bMarked );
}

void ScMarkArray::SetMarkArea( USHORT nStartRow, USHORT nEndRow, BOOL bMarked )
{
    if ((nStartRow >= 0 && nStartRow <= MAXROW) && (nEndRow >= 0 && nEndRow <= MAXROW))
    {
        if ((nStartRow == 0) && (nEndRow == MAXROW))
        {
            Reset(bMarked);
        }
        else
        {
            USHORT nNewCount = 0;
            ScMarkEntry* pNewData = new ScMarkEntry[nCount + 2];
            if (pNewData)
            {
                USHORT ni = 0;
                if (nStartRow > 0)
                {
                    // Anfangsbereich kopieren
                    while (pData[ni].nRow < nStartRow)
                    {
                        pNewData[ni].nRow = pData[ni].nRow;
                        pNewData[ni].bMarked = pData[ni].bMarked;
                        nNewCount++;
                        ni++;
                    }
                    if ((pData[ni].bMarked != bMarked) &&
                        ((ni==0) ? TRUE : (pData[ni - 1].nRow < nStartRow - 1)))
                    {
                        // Eintrag splitten
                        pNewData[nNewCount].nRow = nStartRow - 1;
                        pNewData[nNewCount].bMarked = pData[ni].bMarked;
                        nNewCount++;
                        ni++;
                    }
                    if (ni > 0)
                        if (pData[ni-1].bMarked == bMarked)         // zusammenfassen
                        {
                            ni--;
                            nNewCount--;
                        }
                } // if StartRow > 0

                // Bereich setzen
                pNewData[nNewCount].nRow = nEndRow;
                pNewData[nNewCount].bMarked = bMarked;
                nNewCount++;

                USHORT nj = 0;
                if (nEndRow < MAXROW)
                {
                    // mittleren Bereich ueberspringen
                    while (pData[nj].nRow <= nEndRow) nj++;
                    if (pData[nj].bMarked == bMarked)
                    {
                        // Eintrag zusammenfassen
                        pNewData[nNewCount - 1].nRow = pData[nj].nRow;
                        nj++;
                    }
                    // Den Endbereich kopieren
                    while (nj < nCount)
                    {
                        pNewData[nNewCount].nRow = pData[nj].nRow;
                        pNewData[nNewCount].bMarked = pData[nj].bMarked;
                        nNewCount++;
                        nj++;
                    }
                } // if EndRow < MaxRow
                // Zeiger umsetzen
                delete[] pData;
                nCount = nNewCount;
                pData = pNewData;
            }
        }
    }
//  InfoBox(0, String(nCount) + String(" Eintraege") ).Execute();
}

void ScMarkArray::DeleteArea(USHORT nStartRow, USHORT nEndRow)
{
    SetMarkArea(nStartRow, nEndRow, FALSE);
}

BOOL ScMarkArray::IsAllMarked( USHORT nStartRow, USHORT nEndRow ) const
{
    short nStartIndex;
    short nEndIndex;

    if (Search( nStartRow, nStartIndex ))
        if (pData[nStartIndex].bMarked)
            if (Search( nEndRow, nEndIndex ))
                if (nEndIndex==nStartIndex)
                    return TRUE;

    return FALSE;
}

BOOL ScMarkArray::HasOneMark( USHORT& rStartRow, USHORT& rEndRow ) const
{
    BOOL bRet = FALSE;
    if ( nCount == 1 )
    {
        if ( pData[0].bMarked )
        {
            rStartRow = 0;
            rEndRow = MAXROW;
            bRet = TRUE;
        }
    }
    else if ( nCount == 2 )
    {
        if ( pData[0].bMarked )
        {
            rStartRow = 0;
            rEndRow = pData[0].nRow;
        }
        else
        {
            rStartRow = pData[0].nRow + 1;
            rEndRow = MAXROW;
        }
        bRet = TRUE;
    }
    else if ( nCount == 3 )
    {
        if ( pData[1].bMarked )
        {
            rStartRow = pData[0].nRow + 1;
            rEndRow = pData[1].nRow;
            bRet = TRUE;
        }
    }
    return bRet;
}

BOOL ScMarkArray::HasMarks() const
{
    return ( nCount > 1 || pData[0].bMarked );
}

void ScMarkArray::SwapCol(ScMarkArray& rMarkArray)
{
    USHORT nTemp = rMarkArray.nCount;
    rMarkArray.nCount = nCount;
    nCount = nTemp;

    ScMarkEntry* pTemp = rMarkArray.pData;
    rMarkArray.pData = pData;
    pData = pTemp;
}

void ScMarkArray::MoveTo(USHORT nStartRow, USHORT nEndRow, ScMarkArray& rMarkArray)
{
    USHORT nStart = nStartRow;
    for (USHORT i = 0; i < nCount; i++)
    {
        if ((pData[i].nRow >= nStartRow) && ((i==0) ? TRUE : pData[i-1].nRow < nEndRow))
        {
            rMarkArray.SetMarkArea(nStart, Min(pData[i].nRow,nEndRow), pData[i].bMarked);
        }
        nStart = Max((USHORT)nStart, (USHORT)(pData[i].nRow + 1) );
    }
    DeleteArea(nStartRow, nEndRow);
}

void ScMarkArray::CopyMarksTo( ScMarkArray& rDestMarkArray ) const
{
    if (rDestMarkArray.pData)
        delete rDestMarkArray.pData;

    if (pData)
    {
        rDestMarkArray.pData = new ScMarkEntry[nCount];
        memmove( rDestMarkArray.pData, pData, nCount * sizeof(ScMarkEntry) );
    }
    else
        rDestMarkArray.pData = NULL;

    rDestMarkArray.nCount = nCount;
}

short ScMarkArray::GetNextMarked( short nRow, BOOL bUp ) const
{
    short nRet = nRow;
    if (VALIDROW(nRow))
    {
        short nIndex;
        Search(nRow, nIndex);
        if (!pData[nIndex].bMarked)
        {
            if (bUp)
            {
                if (nIndex>0)
                    nRet = pData[nIndex-1].nRow;
                else
                    nRet = -1;
            }
            else
                nRet = pData[nIndex].nRow + 1;
        }
    }
    return nRet;
}

USHORT ScMarkArray::GetMarkEnd( USHORT nRow, BOOL bUp ) const
{
    USHORT nRet;
    short nIndex;
    Search(nRow, nIndex);
    DBG_ASSERT( pData[nIndex].bMarked, "GetMarkEnd ohne bMarked" );
    if (bUp)
    {
        if (nIndex>0)
            nRet = pData[nIndex-1].nRow + 1;
        else
            nRet = 0;
    }
    else
        nRet = pData[nIndex].nRow;

    return nRet;
}

//
//  -------------- Iterator ----------------------------------------------
//

ScMarkArrayIter::ScMarkArrayIter( const ScMarkArray* pNewArray ) :
    pArray( pNewArray ),
    nPos( 0 )
{
}

ScMarkArrayIter::~ScMarkArrayIter()
{
}

BOOL ScMarkArrayIter::Next( USHORT& rTop, USHORT& rBottom )
{
    if ( nPos >= pArray->nCount )
        return FALSE;
    while (!pArray->pData[nPos].bMarked)
    {
        ++nPos;
        if ( nPos >= pArray->nCount )
            return FALSE;
    }
    rBottom = pArray->pData[nPos].nRow;
    if (nPos==0)
        rTop = 0;
    else
        rTop = pArray->pData[nPos-1].nRow + 1;
    ++nPos;
    return TRUE;
}





