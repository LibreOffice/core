/*************************************************************************
 *
 *  $RCSfile: index.cxx,v $
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

#include <stdlib.h>             // fuer qsort

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#include "errhdl.hxx"           // fuers ASSERT
#include "index.hxx"
#include "error.h"              // fuers ASSERT

#ifndef PRODUCT
int SwIndex::nSerial = 0;
#endif


TYPEINIT0(SwIndexReg);  // rtti


#ifdef CHK

#define IDX_CHK_ARRAY       pArray->ChhkArr();
#define ARR_CHK_ARRAY       ChhkArr();


void SwIndexReg::ChkArr()
{
    ASSERT( (pFirst && pLast) || (!pFirst && !pLast),
            "Array falsch Indiziert" );

    if( !pFirst )
        return;

    xub_StrLen nVal = 0;
    const SwIndex* pIdx = pFirst, *pPrev = 0;
    ASSERT( !pIdx->pPrev, "Array-pFirst nicht am Anfang" );

    while( pIdx != pLast )
    {
        ASSERT( pIdx->pPrev != pIdx && pIdx->pNext != pIdx,
                "Index zeigt auf sich selbst" )

        ASSERT( pIdx->nIndex >= nVal, "Reihenfolge stimmt nicht" );
        ASSERT( pPrev == pIdx->pPrev, "Verkettung stimmt nicht" );
        pPrev = pIdx;
        pIdx = pIdx->pNext;
        nVal = pPrev->nIndex;
    }
}

#else                                   // CHK

#define IDX_CHK_ARRAY
#define ARR_CHK_ARRAY

#endif                                  // CHK



SwIndex::SwIndex( SwIndexReg* pArr, xub_StrLen nIdx )
    : pArray( pArr ), nIndex( nIdx ), pNext( 0 ), pPrev( 0 )
{
    if( !pArray )
    {
        pArray = SwIndexReg::pEmptyIndexArray;
        nIndex = 0;     // steht immer auf 0 !!!
    }

    if( !pArray->pFirst )         // 1. Index ??
        pArray->pFirst = pArray->pLast = this;
    else if( pArray->pMiddle )
    {
        if( pArray->pMiddle->nIndex <= nIdx )
        {
            if( nIdx > ((pArray->pLast->nIndex - pArray->pMiddle->nIndex) / 2) )
                ChgValue( *pArray->pLast, nIdx );
            else
                ChgValue( *pArray->pMiddle, nIdx );
        }
        else if( nIdx > ((pArray->pMiddle->nIndex - pArray->pFirst->nIndex) / 2) )
            ChgValue( *pArray->pMiddle, nIdx );
        else
            ChgValue( *pArray->pFirst, nIdx );
    }
    else if( nIdx > ((pArray->pLast->nIndex - pArray->pFirst->nIndex) / 2) )
        ChgValue( *pArray->pLast, nIdx );
    else
        ChgValue( *pArray->pFirst, nIdx );

#ifndef PRODUCT
    MySerial = ++nSerial;       // nur in der nicht PRODUCT-Version
#endif
IDX_CHK_ARRAY
}


SwIndex::SwIndex( const SwIndex& rIdx, short nIdx )
    : pArray( rIdx.pArray ), pNext( 0 ), pPrev( 0 )
{
    ChgValue( rIdx, rIdx.nIndex + nIdx );

#ifndef PRODUCT
    MySerial = ++nSerial;       // nur in der nicht PRODUCT-Version
#endif
IDX_CHK_ARRAY
}


SwIndex::SwIndex( const SwIndex& rIdx )
    : pArray( rIdx.pArray ), nIndex( rIdx.nIndex ), pNext( 0 ), pPrev( 0 )
{
    ChgValue( rIdx, rIdx.nIndex );
#ifndef PRODUCT
    MySerial = ++nSerial;       // nur in der nicht PRODUCT-Version
#endif
IDX_CHK_ARRAY
}


SwIndex& SwIndex::ChgValue( const SwIndex& rIdx, xub_StrLen nNewValue )
{
    register SwIndex* pFnd = (SwIndex*)&rIdx;
    if( rIdx.nIndex > nNewValue )               // nach vorne versuchen
    {
        register SwIndex* pPrv;
        while( 0 != ( pPrv = pFnd->pPrev ) && pPrv->nIndex > nNewValue )
            pFnd = pPrv;

        if( pFnd != this )
        {
            // an alter Position ausketten
            // erstmal an alter Position ausketten
            if( pPrev )
                pPrev->pNext = pNext;
            else if( pArray->pFirst == this )
                pArray->pFirst = pNext;

            if( pNext )
                pNext->pPrev = pPrev;
            else if( pArray->pLast == this )
                pArray->pLast = pPrev;

            pNext = pFnd;
            pPrev = pFnd->pPrev;
            if( pPrev )
                pPrev->pNext = this;
            else
                pArray->pFirst = this;
            pFnd->pPrev = this;
        }
    }
    else if( rIdx.nIndex < nNewValue )
    {
        register SwIndex* pNxt;
        while( 0 != ( pNxt = pFnd->pNext ) && pNxt->nIndex < nNewValue )
            pFnd = pNxt;

        if( pFnd != this )
        {
            // erstmal an alter Position ausketten
            if( pPrev )
                pPrev->pNext = pNext;
            else if( pArray->pFirst == this )
                pArray->pFirst = pNext;

            if( pNext )
                pNext->pPrev = pPrev;
            else if( pArray->pLast == this )
                pArray->pLast = pPrev;

            pPrev = pFnd;
            pNext = pFnd->pNext;
            if( pNext )
                pNext->pPrev = this;
            else
                pArray->pLast = this;
            pFnd->pNext = this;
        }
    }
    else if( pFnd != this )
    {
        // erstmal an alter Position ausketten
        if( pPrev )
            pPrev->pNext = pNext;
        else if( pArray->pFirst == this )
            pArray->pFirst = pNext;

        if( pNext )
            pNext->pPrev = pPrev;
        else if( pArray->pLast == this )
            pArray->pLast = pPrev;

        pPrev = (SwIndex*)&rIdx;
        pNext = rIdx.pNext;
        pPrev->pNext = this;

        if( !pNext )            // im IndexArray als letzes
            pArray->pLast = this;
        else
            pNext->pPrev = this;
    }
    pArray = rIdx.pArray;

    if( pArray->pFirst == pNext )
        pArray->pFirst = this;
    if( pArray->pLast == pPrev )
        pArray->pLast = this;

    nIndex = nNewValue;

IDX_CHK_ARRAY

    return *this; }


void SwIndex::Remove()
{
    if( !pPrev )
        pArray->pFirst = pNext;
    else
        pPrev->pNext = pNext;

    if( !pNext )
        pArray->pLast = pPrev;
    else
        pNext->pPrev = pPrev;

    if( this == pArray->pMiddle )   pArray->pMiddle = pPrev;
IDX_CHK_ARRAY
}

/*************************************************************************
|*
|*    SwIndex & SwIndex::operator=( const SwIndex & aSwIndex )
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/


SwIndex& SwIndex::operator=( const SwIndex& rIdx )
{
    int bEqual;
    if( rIdx.pArray != pArray )         // im alten abmelden !!
    {
        Remove();
        pArray = rIdx.pArray;
        pNext = pPrev = 0;
        bEqual = FALSE;
    }
    else
        bEqual = rIdx.nIndex == nIndex;

    if( !bEqual )
        ChgValue( rIdx, rIdx.nIndex );
    return *this;
}

/*************************************************************************
|*
|*    SwIndex &SwIndex::Assign
|*
|*    Beschreibung
|*    Ersterstellung    VB 25.03.91
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/


SwIndex& SwIndex::Assign( SwIndexReg* pArr, xub_StrLen nIdx )
{
    if( !pArr )
    {
        pArr = SwIndexReg::pEmptyIndexArray;
        nIdx = 0;       // steht immer auf 0 !!!
    }

    if( pArr != pArray )            // im alten abmelden !!
    {
        Remove();
        pArray = pArr;
        pNext = pPrev = 0;
        if( !pArr->pFirst )         // 1. Index ??
        {
            pArr->pFirst = pArr->pLast = this;
            nIndex = nIdx;
        }
        else if( pArray->pMiddle )
        {
            if( pArray->pMiddle->nIndex <= nIdx )
            {
                if( nIdx > ((pArr->pLast->nIndex - pArr->pMiddle->nIndex) / 2) )
                    ChgValue( *pArr->pLast, nIdx );
                else
                    ChgValue( *pArr->pMiddle, nIdx );
            }
            else if( nIdx > ((pArr->pMiddle->nIndex - pArr->pFirst->nIndex) / 2) )
                ChgValue( *pArr->pMiddle, nIdx );
            else
                ChgValue( *pArr->pFirst, nIdx );
        }
        else if( nIdx > ((pArr->pLast->nIndex - pArr->pFirst->nIndex) / 2) )
            ChgValue( *pArr->pLast, nIdx );
        else
            ChgValue( *pArr->pFirst, nIdx );
    }
    else if( nIndex != nIdx )
        ChgValue( *this, nIdx );
IDX_CHK_ARRAY
    return *this;
}


SwIndexReg::SwIndexReg()
    : pFirst( 0 ), pLast( 0 ), pMiddle( 0 )
{
}

#ifndef PRODUCT


SwIndexReg::~SwIndexReg()
{
    ASSERT( !pFirst || !pLast, "Es sind noch Indizies angemeldet" );
}

#endif


void SwIndexReg::Update( const SwIndex& rIdx, xub_StrLen nDiff, BOOL bNeg )
{
    register SwIndex* pStt = (SwIndex*)&rIdx;
    register xub_StrLen nNewVal = rIdx.nIndex;
    if( bNeg )
    {
        register xub_StrLen nLast = rIdx.GetIndex() + nDiff;
        while( pStt && pStt->nIndex == nNewVal )
        {
            pStt->nIndex = nNewVal;
            pStt = pStt->pPrev;
        }
        pStt = rIdx.pNext;
        while( pStt && pStt->nIndex >= nNewVal &&
                pStt->nIndex <= nLast )
        {
            pStt->nIndex = nNewVal;
            pStt = pStt->pNext;
        }
        while( pStt )
        {
            pStt->nIndex -= nDiff;
            pStt = pStt->pNext;
        }
    }
    else
    {
        while( pStt && pStt->nIndex == nNewVal )
        {
            pStt->nIndex += nDiff;
            pStt = pStt->pPrev;
        }
        pStt = rIdx.pNext;
        while( pStt )
        {
            pStt->nIndex += nDiff;
            pStt = pStt->pNext;
        }
    }
ARR_CHK_ARRAY
}


/*************************************************************************
|*
|*    SwIndex::operator++()
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/

#ifndef PRODUCT

#ifndef CFRONT


xub_StrLen SwIndex::operator++(int)
{
    ASSERT_ID( nIndex < INVALID_INDEX, ERR_OUTOFSCOPE );

    xub_StrLen nOldIndex = nIndex;
    ChgValue( *this, nIndex+1 );
    return nOldIndex;
}

#endif


xub_StrLen SwIndex::operator++()
{
    ASSERT_ID( nIndex < INVALID_INDEX, ERR_OUTOFSCOPE );

    ChgValue( *this, nIndex+1 );
    return nIndex;
}

/*************************************************************************
|*
|*    SwIndex::operator--()
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/

#ifndef CFRONT


xub_StrLen SwIndex::operator--(int)
{
    ASSERT_ID( nIndex, ERR_OUTOFSCOPE );

    xub_StrLen nOldIndex = nIndex;
    ChgValue( *this, nIndex-1 );
    return nOldIndex;
}

#endif


xub_StrLen SwIndex::operator--()
{
    ASSERT_ID( nIndex, ERR_OUTOFSCOPE );
    return ChgValue( *this, nIndex-1 ).nIndex;
}

/*************************************************************************
|*
|*    SwIndex::operator+=( xub_StrLen )
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/


xub_StrLen SwIndex::operator+=( xub_StrLen nWert )
{
    ASSERT_ID( nIndex < INVALID_INDEX - nWert, ERR_OUTOFSCOPE);
    return ChgValue( *this, nIndex + nWert ).nIndex;
}

/*************************************************************************
|*
|*    SwIndex::operator-=( xub_StrLen )
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/


xub_StrLen SwIndex::operator-=( xub_StrLen nWert )
{
    ASSERT_ID( nIndex >= nWert, ERR_OUTOFSCOPE );
    return ChgValue( *this, nIndex - nWert ).nIndex;
}

/*************************************************************************
|*
|*    SwIndex::operator+=( const SwIndex & )
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/


xub_StrLen SwIndex::operator+=( const SwIndex & rIndex )
{
    ASSERT_ID( nIndex < INVALID_INDEX - rIndex.nIndex, ERR_OUTOFSCOPE );
    return ChgValue( *this, nIndex + rIndex.nIndex ).nIndex;
}


/*************************************************************************
|*
|*    SwIndex::operator-=( const SwIndex & )
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/


xub_StrLen SwIndex::operator-=( const SwIndex & rIndex )
{
    ASSERT_ID( nIndex >= rIndex.nIndex, ERR_OUTOFSCOPE );
    return ChgValue( *this, nIndex - rIndex.nIndex ).nIndex;
}


/*************************************************************************
|*
|*    SwIndex::operator<( const SwIndex & )
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/


BOOL SwIndex::operator<( const SwIndex & rIndex ) const
{
    ASSERT( pArray == rIndex.pArray, "Attempt to compare indices into different arrays.");
    return nIndex < rIndex.nIndex;
}

/*************************************************************************
|*
|*    SwIndex::operator<=( const SwIndex & )
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 04.06.92
|*
*************************************************************************/


BOOL SwIndex::operator<=( const SwIndex & rIndex ) const
{
    ASSERT( pArray == rIndex.pArray, "Attempt to compare indices into different arrays.");
    return nIndex <= rIndex.nIndex;
}

/*************************************************************************
|*
|*    SwIndex::operator>( const SwIndex & )
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 04.06.92
|*
*************************************************************************/


BOOL SwIndex::operator>( const SwIndex & rIndex ) const
{
    ASSERT( pArray == rIndex.pArray, "Attempt to compare indices into different arrays.");
    return nIndex > rIndex.nIndex;
}

/*************************************************************************
|*
|*    SwIndex::operator>=( const SwIndex & )
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 04.06.92
|*
*************************************************************************/


BOOL SwIndex::operator>=( const SwIndex & rIndex ) const
{
    ASSERT( pArray == rIndex.pArray, "Attempt to compare indices into different arrays.");
    return nIndex >= rIndex.nIndex;
}

#endif

/*************************************************************************
|*
|*    SwIndex & SwIndex::operator=( xub_StrLen )
|*
|*    Beschreibung
|*    Ersterstellung    JP 10.12.90
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/

#ifndef PRODUCT


SwIndex& SwIndex::operator=( xub_StrLen nWert )
{
    // Werte kopieren und im neuen Array anmelden
    if( nIndex != nWert )
        ChgValue( *this, nWert );

    return *this;
}

#endif



void SwIndexReg::MoveIdx( const SwIndex& rOldPos, const SwIndex& rNewPos )
{
    ASSERT( rOldPos.pArray == rNewPos.pArray,
            "stehen in unterschiedlichen Arrays" );

    SwIndex* pStt = (SwIndex*)&rOldPos, *pEnd = pStt;
    SwIndex* pInsPos = (SwIndex*)&rNewPos;
    xub_StrLen nOldIndex = rOldPos.nIndex,
            nNewIndex = rNewPos.nIndex;

    if( nOldIndex == nNewIndex )
        return;

    while( pInsPos->pPrev && pInsPos->pPrev->nIndex == nNewIndex )
        pInsPos = pInsPos->pPrev;

    if( nNewIndex > nOldIndex )
        --nNewIndex;

    while( pStt->pPrev && pStt->pPrev->nIndex == nOldIndex )
    {
        pStt = pStt->pPrev;
        pStt->nIndex = nNewIndex;
    }
    while( pEnd->pNext && pEnd->pNext->nIndex == nOldIndex )
    {
        pEnd = pEnd->pNext;
        pEnd->nIndex = nNewIndex;
    }

    {
        for( SwIndex* pTmp = pStt; pTmp != pEnd; pTmp = pTmp->pNext )
            pTmp->nIndex = nNewIndex;
        pTmp->nIndex = nNewIndex;
    }

    // Ausketten
    if( !pStt->pPrev )
    {
        pFirst = pEnd->pNext;
        pEnd->pNext->pPrev = 0;
    }
    else
        pStt->pPrev->pNext = pEnd->pNext;

    if( !pEnd->pNext )
    {
        pLast = pStt->pPrev;
        pStt->pPrev->pNext = 0;
    }
    else
        pEnd->pNext->pPrev = pStt->pPrev;

    // wieder einketten
    pStt->pPrev = pInsPos->pPrev;
    pEnd->pNext = pInsPos;

    if( pInsPos->pPrev )
        pInsPos->pPrev->pNext = pStt;
    pInsPos->pPrev = pEnd;

    if( pInsPos == pFirst )
        pFirst = pStt;

    if( nNewIndex < nOldIndex )     // es wurde nach vorne verschoben
    {
        while( pInsPos && pInsPos->nIndex <= nOldIndex )
        {
            ++pInsPos->nIndex;
            pInsPos = pInsPos->pNext;
        }
    }
    else                            // es wurde nach hinten verschoben
    {
        while( pStt->pPrev && pStt->pPrev->nIndex > nOldIndex )
        {
            pStt = pStt->pPrev;
            --pStt->nIndex;
        }
    }
}

void SwIndexReg::MoveTo( SwIndexReg& rArr )
{
    if( this != &rArr && pFirst )
    {
        SwIndex* pIdx = (SwIndex*)pFirst, *pNext;
        while( pIdx )
        {
            pNext = pIdx->pNext;
            pIdx->Assign( &rArr, pIdx->GetIndex() );
            pIdx = pNext;
        }
        pFirst = 0, pLast = 0, pMiddle = 0;
    }
}




