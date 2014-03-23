/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <stdlib.h>             // fuer qsort
#include <tools/solar.h>

#include "errhdl.hxx"           // fuers ASSERT
#include "index.hxx"
#include "error.h"              // fuers ASSERT

#ifdef DBG_UTIL
int SwIndex::nSerial = 0;
#endif


#ifdef CHK

#define IDX_CHK_ARRAY       pArray->ChkArr();
#define AR R_CHK_ARRAY       ChkArr();

void SwIndexReg::ChkArr()
{
    if ( ! ((pFirst && pLast) || (!pFirst && !pLast)))
    {
        ASSERT(false, "array not correctly indexed");
    }

    if( !pFirst )
        return;

    xub_StrLen nVal = 0;
    const SwIndex* pIdx = pFirst, *pPrev = 0;
    if ( ! (!pIdx->pPrev))
    {
        ASSERT(false, "array-pFirst not at beginning");
    }

    while( pIdx != pLast )
    {
        if ( ! (pIdx->pPrev != pIdx && pIdx->pNext != pIdx))
        {
            ASSERT(false, "index points to itself");
        }

        if ( ! (pIdx->nIndex >= nVal))
        {
            ASSERT(false, "wrong order");
        }
        if ( ! (pPrev == pIdx->pPrev))
        {
            ASSERT(false, "wrong array pointers");
        }
        if ( ! (this == pIdx->pArray))
        {
            ASSERT(false, "wrong array/child relationship");
        }
        pPrev = pIdx;
        pIdx = pIdx->pNext;
        nVal = pPrev->nIndex;
    }
}

#else                                   // CHK

#define IDX_CHK_ARRAY
#define ARR_CHK_ARRAY

#endif                                  // CHK



SwIndex::SwIndex(SwIndexReg *const pArr, xub_StrLen const nIdx)
    : nIndex( nIdx ), pArray( pArr ), pNext( 0 ), pPrev( 0 )
{
    if( !pArray )
    {
        pArray = SwIndexReg::pEmptyIndexArray;
        nIndex = 0;     // steht immer auf 0 !!!
    }

    if( !pArray->pFirst )         // 1. Index ??
        pArray->pFirst = pArray->pLast = this;
    else if( nIdx > ((pArray->pLast->nIndex - pArray->pFirst->nIndex) / 2) )
        ChgValue( *pArray->pLast, nIdx );
    else
        ChgValue( *pArray->pFirst, nIdx );

#ifdef DBG_UTIL
    MySerial = ++nSerial;       // nur in der nicht PRODUCT-Version
#endif
IDX_CHK_ARRAY
}


SwIndex::SwIndex( const SwIndex& rIdx, short nIdx )
    : pArray( rIdx.pArray ), pNext( 0 ), pPrev( 0 )
{
    ChgValue( rIdx, rIdx.nIndex + nIdx );

#ifdef DBG_UTIL
    MySerial = ++nSerial;       // nur in der nicht PRODUCT-Version
#endif
IDX_CHK_ARRAY
}


SwIndex::SwIndex( const SwIndex& rIdx )
    : nIndex( rIdx.nIndex ), pArray( rIdx.pArray ), pNext( 0 ), pPrev( 0 )
{
    ChgValue( rIdx, rIdx.nIndex );
#ifdef DBG_UTIL
    MySerial = ++nSerial;       // nur in der nicht PRODUCT-Version
#endif
IDX_CHK_ARRAY
}


SwIndex& SwIndex::ChgValue( const SwIndex& rIdx, xub_StrLen nNewValue )
{
    SwIndex* pFnd = (SwIndex*)&rIdx;
    if( rIdx.nIndex > nNewValue )               // nach vorne versuchen
    {
        SwIndex* pPrv;
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
        SwIndex* pNxt;
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
    if (pArray->pFirst==NULL && pArray->pLast==NULL)
    {
        // The index object is not a member of its list and therefore
        // can not be removed.
        return;
    }

    if (pPrev==NULL && pNext==NULL)
    {
        // Removing last element in list.
        pArray->pFirst = NULL;
        pArray->pLast = NULL;
    }
    else
    {
        if( !pPrev )
        {
            OSL_ASSERT(pNext!=NULL);
            pArray->pFirst = pNext;
        }
        else
            pPrev->pNext = pNext;

        if( !pNext )
        {
            OSL_ASSERT(pPrev!=NULL);
            pArray->pLast = pPrev;
        }
        else
            pNext->pPrev = pPrev;
    }
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
        bEqual = sal_False;
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
        else if( pArr->pLast && (nIdx > ((pArr->pLast->nIndex - pArr->pFirst->nIndex) / 2)) )
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
    : pFirst( 0 ), pLast( 0 )
{
}



SwIndexReg::~SwIndexReg()
{
    ASSERT( !pFirst || !pLast, "Es sind noch Indizies angemeldet" );
}


void SwIndexReg::Update(
    SwIndex const & rIdx,
    const xub_StrLen nDiff,
    const bool bNeg,
    const bool /* argument is only used in derived class*/ )
{
    SwIndex* pStt = const_cast<SwIndex*>(&rIdx);
    const xub_StrLen nNewVal = rIdx.nIndex;
    if( bNeg )
    {
        const xub_StrLen nLast = rIdx.GetIndex() + nDiff;
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
            pStt->nIndex = pStt->nIndex - nDiff;
            pStt = pStt->pNext;
        }
    }
    else
    {
        while( pStt && pStt->nIndex == nNewVal )
        {
            pStt->nIndex = pStt->nIndex + nDiff;
            pStt = pStt->pPrev;
        }
        pStt = rIdx.pNext;
        while( pStt )
        {
            pStt->nIndex = pStt->nIndex + nDiff;
            pStt = pStt->pNext;
        }
    }
ARR_CHK_ARRAY
}

#ifdef DBG_UTIL
#ifndef CFRONT

/*************************************************************************
|*
|*    SwIndex::operator++()
|*
|*    Beschreibung
|*    Ersterstellung    JP 07.11.90
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/
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

sal_Bool SwIndex::operator<( const SwIndex & rIndex ) const
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

sal_Bool SwIndex::operator<=( const SwIndex & rIndex ) const
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

sal_Bool SwIndex::operator>( const SwIndex & rIndex ) const
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

sal_Bool SwIndex::operator>=( const SwIndex & rIndex ) const
{
    ASSERT( pArray == rIndex.pArray, "Attempt to compare indices into different arrays.");
    return nIndex >= rIndex.nIndex;
}

/*************************************************************************
|*
|*    SwIndex & SwIndex::operator=( xub_StrLen )
|*
|*    Beschreibung
|*    Ersterstellung    JP 10.12.90
|*    Letzte Aenderung  JP 07.03.94
|*
*************************************************************************/

SwIndex& SwIndex::operator=( xub_StrLen nWert )
{
    // Werte kopieren und im neuen Array anmelden
    if( nIndex != nWert )
        ChgValue( *this, nWert );

    return *this;
}

#endif // ifndef PRODUCT

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
        pFirst = 0, pLast = 0;
    }
}
