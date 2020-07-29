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

#include <compressedarray.hxx>
#include <global.hxx>

template< typename A, typename D >
ScCompressedArray<A,D>::ScCompressedArray( A nMaxAccessP, const D& rValue )
    : nCount(1)
    , nLimit(1)
    , pData( new DataEntry[1])
    , nMaxAccess( nMaxAccessP)
{
    pData[0].aValue = rValue;
    pData[0].nEnd = nMaxAccess;
}

template< typename A, typename D >
ScCompressedArray<A,D>::~ScCompressedArray()
{
}

template< typename A, typename D >
size_t ScCompressedArray<A,D>::Search( A nAccess ) const
{
    if (nAccess == 0)
        return 0;

    long nLo    = 0;
    long nHi    = static_cast<long>(nCount) - 1;
    long nStart = 0;
    long i      = 0;
    bool bFound = (nCount == 1);
    while (!bFound && nLo <= nHi)
    {
        i = (nLo + nHi) / 2;
        if (i > 0)
            nStart = static_cast<long>(pData[i - 1].nEnd);
        else
            nStart = -1;
        long nEnd = static_cast<long>(pData[i].nEnd);
        if (nEnd < static_cast<long>(nAccess))
            nLo = ++i;
        else
            if (nStart >= static_cast<long>(nAccess))
                nHi = --i;
            else
                bFound = true;
    }
    return (bFound ? static_cast<size_t>(i) : (nAccess < 0 ? 0 : nCount-1));
}

template< typename A, typename D >
void ScCompressedArray<A,D>::SetValue( A nStart, A nEnd, const D& rValue )
{
    if (!(0 <= nStart && nStart <= nMaxAccess && 0 <= nEnd && nEnd <= nMaxAccess
            && nStart <= nEnd))
        return;

    if ((nStart == 0) && (nEnd == nMaxAccess))
        Reset( rValue);
    else
    {
        // Create a temporary copy in case we got a reference passed that
        // points to a part of the array to be reallocated.
        D aNewVal( rValue);
        size_t nNeeded = nCount + 2;
        if (nLimit < nNeeded)
        {
            nLimit *= 1.5;
            if (nLimit < nNeeded)
                nLimit = nNeeded;
            std::unique_ptr<DataEntry[]> pNewData(new DataEntry[nLimit]);
            memcpy( pNewData.get(), pData.get(), nCount*sizeof(DataEntry));
            pData = std::move(pNewData);
        }

        size_t ni;          // number of leading entries
        size_t nInsert;     // insert position (nMaxAccess+1 := no insert)
        bool bCombined = false;
        bool bSplit = false;
        if (nStart > 0)
        {
            // skip leading
            ni = this->Search( nStart);

            nInsert = nMaxAccess+1;
            if (!(pData[ni].aValue == aNewVal))
            {
                if (ni == 0 || (pData[ni-1].nEnd < nStart - 1))
                {   // may be a split or a simple insert or just a shrink,
                    // row adjustment is done further down
                    if (pData[ni].nEnd > nEnd)
                        bSplit = true;
                    ni++;
                    nInsert = ni;
                }
                else if (ni > 0 && pData[ni-1].nEnd == nStart - 1)
                    nInsert = ni;
            }
            if (ni > 0 && pData[ni-1].aValue == aNewVal)
            {   // combine
                pData[ni-1].nEnd = nEnd;
                nInsert = nMaxAccess+1;
                bCombined = true;
            }
        }
        else
        {
            nInsert = 0;
            ni = 0;
        }

        size_t nj = ni;     // stop position of range to replace
        while (nj < nCount && pData[nj].nEnd <= nEnd)
            nj++;
        if (!bSplit)
        {
            if (nj < nCount && pData[nj].aValue == aNewVal)
            {   // combine
                if (ni > 0)
                {
                    if (pData[ni-1].aValue == aNewVal)
                    {   // adjacent entries
                        pData[ni-1].nEnd = pData[nj].nEnd;
                        nj++;
                    }
                    else if (ni == nInsert)
                        pData[ni-1].nEnd = nStart - 1;   // shrink
                }
                nInsert = nMaxAccess+1;
                bCombined = true;
            }
            else if (ni > 0 && ni == nInsert)
                pData[ni-1].nEnd = nStart - 1;   // shrink
        }
        if (ni < nj)
        {   // remove middle entries
            if (!bCombined)
            {   // replace one entry
                pData[ni].nEnd = nEnd;
                pData[ni].aValue = aNewVal;
                ni++;
                nInsert = nMaxAccess+1;
            }
            if (ni < nj)
            {   // remove entries
                memmove( pData.get() + ni, pData.get() + nj,
                        (nCount - nj) * sizeof(DataEntry));
                nCount -= nj - ni;
            }
        }

        if (nInsert < static_cast<size_t>(nMaxAccess+1))
        {   // insert or append new entry
            if (nInsert <= nCount)
            {
                if (!bSplit)
                    memmove( pData.get() + nInsert + 1, pData.get() + nInsert,
                            (nCount - nInsert) * sizeof(DataEntry));
                else
                {
                    memmove( pData.get() + nInsert + 2, pData.get() + nInsert,
                            (nCount - nInsert) * sizeof(DataEntry));
                    pData[nInsert+1] = pData[nInsert-1];
                    nCount++;
                }
            }
            if (nInsert)
                pData[nInsert-1].nEnd = nStart - 1;
            pData[nInsert].nEnd = nEnd;
            pData[nInsert].aValue = aNewVal;
            nCount++;
        }
    }
}

template< typename A, typename D >
void ScCompressedArray<A,D>::CopyFrom( const ScCompressedArray<A,D>& rArray, A nDestStart,
        A nDestEnd, A nSrcStart )
{
    assert( this != &rArray && "cannot copy self->self" );
    size_t nIndex = 0;
    A nRegionEnd;
    for (A j=nDestStart; j<=nDestEnd; ++j)
    {
        const D& rValue = (j==nDestStart ?
                rArray.GetValue( j - nDestStart + nSrcStart, nIndex, nRegionEnd) :
                rArray.GetNextValue( nIndex, nRegionEnd));
        nRegionEnd = nRegionEnd - nSrcStart + nDestStart;
        if (nRegionEnd > nDestEnd)
            nRegionEnd = nDestEnd;
        this->SetValue( j, nRegionEnd, rValue);
        j = nRegionEnd;
    }
}

template< typename A, typename D >
const D& ScCompressedArray<A,D>::Insert( A nStart, size_t nAccessCount )
{
    size_t nIndex = this->Search( nStart);
    // No real insertion is needed, simply extend the one entry and adapt all
    // following. In case nStart points to the start row of an entry, extend
    // the previous entry (inserting before nStart).
    if (nIndex > 0 && pData[nIndex-1].nEnd+1 == nStart)
        --nIndex;
    const D& rValue = pData[nIndex].aValue; // the value "copied"
    do
    {
        pData[nIndex].nEnd += nAccessCount;
        if (pData[nIndex].nEnd >= nMaxAccess)
        {
            pData[nIndex].nEnd = nMaxAccess;
            nCount = nIndex + 1;    // discard trailing entries
        }
    } while (++nIndex < nCount);
    return rValue;
}

template< typename A, typename D >
void ScCompressedArray<A,D>::InsertPreservingSize( A nStart, size_t nAccessCount, const D& rFillValue )
{
    const A nPrevLastPos = GetLastPos();

    Insert(nStart, nAccessCount);
    for (A i = nStart; i < A(nStart + nAccessCount); ++i)
        SetValue(i, rFillValue);

    const A nNewLastPos = GetLastPos();
    Remove(nPrevLastPos, nNewLastPos - nPrevLastPos);
}

template< typename A, typename D >
void ScCompressedArray<A,D>::Remove( A nStart, size_t nAccessCount )
{
    A nEnd = nStart + nAccessCount - 1;
    size_t nIndex = this->Search( nStart);
    // equalize/combine/remove all entries in between
    if (nEnd > pData[nIndex].nEnd)
        this->SetValue( nStart, nEnd, pData[nIndex].aValue);
    // remove an exactly matching entry by shifting up all following by one
    if ((nStart == 0 || (nIndex > 0 && nStart == pData[nIndex-1].nEnd+1)) &&
            pData[nIndex].nEnd == nEnd && nIndex < nCount-1)
    {
        // In case removing an entry results in two adjacent entries with
        // identical data, combine them into one. This is also necessary to
        // make the algorithm used in SetValue() work correctly, it relies on
        // the fact that consecutive values actually differ.
        size_t nRemove;
        if (nIndex > 0 && pData[nIndex-1].aValue == pData[nIndex+1].aValue)
        {
            nRemove = 2;
            --nIndex;
        }
        else
            nRemove = 1;
        memmove( pData.get() + nIndex, pData.get() + nIndex + nRemove, (nCount - (nIndex +
                        nRemove)) * sizeof(DataEntry));
        nCount -= nRemove;
    }
    // adjust end rows, nIndex still being valid
    do
    {
        pData[nIndex].nEnd -= nAccessCount;
    } while (++nIndex < nCount);
    pData[nCount-1].nEnd = nMaxAccess;
}

template< typename A, typename D >
void ScCompressedArray<A,D>::RemovePreservingSize( A nStart, size_t nAccessCount, const D& rFillValue )
{
    const A nPrevLastPos = GetLastPos();

    Remove(nStart, nAccessCount);

    const A nNewLastPos = GetLastPos();
    InsertPreservingSize(nNewLastPos, nNewLastPos - nPrevLastPos, rFillValue);
}

template< typename A, typename D >
void ScCompressedArray<A,D>::Iterator::operator++()
{
    ++mnRegion;
    if (mnRegion > mrArray.pData[mnIndex].nEnd)
        ++mnIndex;
}

template< typename A, typename D >
typename ScCompressedArray<A,D>::Iterator ScCompressedArray<A,D>::Iterator::operator+(size_t nAccessCount) const
{
    A nRegion = mnRegion + nAccessCount;
    auto nIndex = mnIndex;
    while (nRegion > mrArray.pData[nIndex].nEnd)
        ++nIndex;
    return Iterator(mrArray, nIndex, nRegion);
}

// === ScBitMaskCompressedArray ==============================================

template< typename A, typename D >
void ScBitMaskCompressedArray<A,D>::AndValue( A nStart, A nEnd,
        const D& rValueToAnd )
{
    if (nStart > nEnd)
        return;

    size_t nIndex = this->Search( nStart);
    do
    {
        if ((this->pData[nIndex].aValue & rValueToAnd) != this->pData[nIndex].aValue)
        {
            A nS = ::std::max<A>( (nIndex>0 ? this->pData[nIndex-1].nEnd+1 : 0), nStart);
            A nE = ::std::min( this->pData[nIndex].nEnd, nEnd);
            this->SetValue( nS, nE, this->pData[nIndex].aValue & rValueToAnd);
            if (nE >= nEnd)
                break;  // while
            nIndex = this->Search( nE + 1);
        }
        else if (this->pData[nIndex].nEnd >= nEnd)
            break;  // while
        else
            ++nIndex;
    } while (nIndex < this->nCount);
}

template< typename A, typename D >
void ScBitMaskCompressedArray<A,D>::OrValue( A nStart, A nEnd,
        const D& rValueToOr )
{
    if (nStart > nEnd)
        return;

    size_t nIndex = this->Search( nStart);
    do
    {
        if ((this->pData[nIndex].aValue | rValueToOr) != this->pData[nIndex].aValue)
        {
            A nS = ::std::max<A>( (nIndex>0 ? this->pData[nIndex-1].nEnd+1 : 0), nStart);
            A nE = ::std::min( this->pData[nIndex].nEnd, nEnd);
            this->SetValue( nS, nE, this->pData[nIndex].aValue | rValueToOr);
            if (nE >= nEnd)
                break;  // while
            nIndex = this->Search( nE + 1);
        }
        else if (this->pData[nIndex].nEnd >= nEnd)
            break;  // while
        else
            ++nIndex;
    } while (nIndex < this->nCount);
}

template< typename A, typename D >
void ScBitMaskCompressedArray<A,D>::CopyFromAnded(
        const ScBitMaskCompressedArray<A,D>& rArray, A nStart, A nEnd,
        const D& rValueToAnd )
{
    size_t nIndex = 0;
    A nRegionEnd;
    for (A j=nStart; j<=nEnd; ++j)
    {
        const D& rValue = (j==nStart ?
                rArray.GetValue( j, nIndex, nRegionEnd) :
                rArray.GetNextValue( nIndex, nRegionEnd));
        if (nRegionEnd > nEnd)
            nRegionEnd = nEnd;
        this->SetValue( j, nRegionEnd, rValue & rValueToAnd);
        j = nRegionEnd;
    }
}

template< typename A, typename D >
A ScBitMaskCompressedArray<A,D>::GetLastAnyBitAccess( const D& rBitMask ) const
{
    A nEnd = ::std::numeric_limits<A>::max();
    size_t nIndex = this->nCount-1;
    while (true)
    {
        if (this->pData[nIndex].aValue & rBitMask)
        {
            nEnd = this->pData[nIndex].nEnd;
            break;  // while
        }
        else
        {
            if (nIndex > 0)
            {
                --nIndex;
                if (this->pData[nIndex].nEnd < 0)
                    break;  // while
            }
            else
                break;  // while
        }
    }
    return nEnd;
}

// === Force instantiation of specializations ================================

template class ScCompressedArray< SCROW, CRFlags>;             // flags, base class
template class ScBitMaskCompressedArray< SCROW, CRFlags>;      // flags
template class ScCompressedArray< SCCOL, sal_uInt16>;
template class ScCompressedArray< SCCOL, CRFlags>;
template class ScCompressedArray< SCROW, sal_uInt16>;
template class ScBitMaskCompressedArray< SCCOL, CRFlags>;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
