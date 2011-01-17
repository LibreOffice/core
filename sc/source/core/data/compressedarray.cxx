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
#include "precompiled_sc.hxx"

#include "compressedarray.hxx"
#include "address.hxx"

#include <algorithm>

template< typename A, typename D >
ScCompressedArray<A,D>::ScCompressedArray( A nMaxAccessP, const D& rValue,
        size_t nDeltaP )
    : nCount(1)
    , nLimit(1)
    , nDelta( nDeltaP > 0 ? nDeltaP : 1)
    , pData( new DataEntry[1])
    , nMaxAccess( nMaxAccessP)
{
    pData[0].aValue = rValue;
    pData[0].nEnd = nMaxAccess;
}


template< typename A, typename D >
ScCompressedArray<A,D>::ScCompressedArray( A nMaxAccessP, const D* pDataArray,
        size_t nDataCount )
    : nCount(0)
    , nLimit( nDataCount)
    , nDelta( nScCompressedArrayDelta)
    , pData( new DataEntry[nDataCount])
    , nMaxAccess( nMaxAccessP)
{
    D aValue = pDataArray[0];
    for (size_t j=0; j<nDataCount; ++j)
    {
        if (!(aValue == pDataArray[j]))
        {
            pData[nCount].aValue = aValue;
            pData[nCount].nEnd = j-1;
            ++nCount;
            aValue = pDataArray[j];
        }
    }
    pData[nCount].aValue = aValue;
    pData[nCount].nEnd = nMaxAccess;
    ++nCount;
    Resize( nCount);
}


template< typename A, typename D >
ScCompressedArray<A,D>::~ScCompressedArray()
{
    delete[] pData;
}


template< typename A, typename D >
void ScCompressedArray<A,D>::Resize( size_t nNewLimit)
{
    if ((nCount <= nNewLimit && nNewLimit < nLimit) || nLimit < nNewLimit)
    {
        nLimit = nNewLimit;
        DataEntry* pNewData = new DataEntry[nLimit];
        memcpy( pNewData, pData, nCount*sizeof(DataEntry));
        delete[] pData;
        pData = pNewData;
    }
}


template< typename A, typename D >
size_t ScCompressedArray<A,D>::Search( A nAccess ) const
{
    if (nAccess == 0)
        return 0;

    long nLo    = 0;
    long nHi    = static_cast<long>(nCount) - 1;
    long nStart = 0;
    long nEnd   = 0;
    long i      = 0;
    bool bFound = (nCount == 1);
    while (!bFound && nLo <= nHi)
    {
        i = (nLo + nHi) / 2;
        if (i > 0)
            nStart = (long) pData[i - 1].nEnd;
        else
            nStart = -1;
        nEnd = (long) pData[i].nEnd;
        if (nEnd < (long) nAccess)
            nLo = ++i;
        else
            if (nStart >= (long) nAccess)
                nHi = --i;
            else
                bFound = true;
    }
    return (bFound ? static_cast<size_t>(i) : (nAccess < 0 ? 0 : nCount-1));
}


template< typename A, typename D >
void ScCompressedArray<A,D>::SetValue( A nStart, A nEnd, const D& rValue )
{
    if (0 <= nStart && nStart <= nMaxAccess && 0 <= nEnd && nEnd <= nMaxAccess
            && nStart <= nEnd)
    {
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
                nLimit += nDelta;
                if (nLimit < nNeeded)
                    nLimit = nNeeded;
                DataEntry* pNewData = new DataEntry[nLimit];
                memcpy( pNewData, pData, nCount*sizeof(DataEntry));
                delete[] pData;
                pData = pNewData;
            }

            size_t ni;          // number of leading entries
            size_t nInsert;     // insert position (nMaxAccess+1 := no insert)
            bool bCombined = false;
            bool bSplit = false;
            if (nStart > 0)
            {
                // skip leading
                ni = Search( nStart);

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
                    memmove( pData + ni, pData + nj,
                            (nCount - nj) * sizeof(DataEntry));
                    nCount -= nj - ni;
                }
            }

            if (nInsert < static_cast<size_t>(nMaxAccess+1))
            {   // insert or append new entry
                if (nInsert <= nCount)
                {
                    if (!bSplit)
                        memmove( pData + nInsert + 1, pData + nInsert,
                                (nCount - nInsert) * sizeof(DataEntry));
                    else
                    {
                        memmove( pData + nInsert + 2, pData + nInsert,
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
}


template< typename A, typename D >
void ScCompressedArray<A,D>::CopyFrom( const ScCompressedArray<A,D>& rArray, A nStart,
        A nEnd, long nSourceDy )
{
    size_t nIndex;
    A nRegionEnd;
    for (A j=nStart; j<=nEnd; ++j)
    {
        const D& rValue = (j==nStart ?
                rArray.GetValue( j+nSourceDy, nIndex, nRegionEnd) :
                rArray.GetNextValue( nIndex, nRegionEnd));
        nRegionEnd -= nSourceDy;
        if (nRegionEnd > nEnd)
            nRegionEnd = nEnd;
        SetValue( j, nRegionEnd, rValue);
        j = nRegionEnd;
    }
}


template< typename A, typename D >
const D& ScCompressedArray<A,D>::Insert( A nStart, size_t nAccessCount )
{
    size_t nIndex = Search( nStart);
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
void ScCompressedArray<A,D>::Remove( A nStart, size_t nAccessCount )
{
    A nEnd = nStart + nAccessCount - 1;
    size_t nIndex = Search( nStart);
    // equalize/combine/remove all entries in between
    if (nEnd > pData[nIndex].nEnd)
        SetValue( nStart, nEnd, pData[nIndex].aValue);
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
        memmove( pData + nIndex, pData + nIndex + nRemove, (nCount - (nIndex +
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
A ScCompressedArray<A,D>::GetLastUnequalAccess( A nStart, const D& rCompare )
{
    A nEnd = ::std::numeric_limits<A>::max();
    size_t nIndex = nCount-1;
    while (1)
    {
        if (pData[nIndex].aValue != rCompare)
        {
            nEnd = pData[nIndex].nEnd;
            break;  // while
        }
        else
        {
            if (nIndex > 0)
            {
                --nIndex;
                if (pData[nIndex].nEnd < nStart)
                    break;  // while
            }
            else
                break;  // while
        }
    }
    return nEnd;
}


// === ScSummableCompressedArray =============================================

template< typename A, typename D >
unsigned long ScSummableCompressedArray<A,D>::SumValues( A nStart, A nEnd ) const
{
    size_t nIndex = Search( nStart);
    unsigned long nSum = SumValuesContinuation( nStart, nEnd, nIndex);
    if (nEnd > this->nMaxAccess)
        nSum += this->pData[this->nCount-1].aValue * (nEnd - this->nMaxAccess);
    return nSum;
}


template< typename A, typename D >
unsigned long ScSummableCompressedArray<A,D>::SumValuesContinuation(
        A nStart, A nEnd, size_t& nIndex ) const
{
    unsigned long nSum = 0;
    A nS = nStart;
    while (nIndex < this->nCount && nS <= nEnd)
    {
        A nE = ::std::min( this->pData[nIndex].nEnd, nEnd);
        // FIXME: test for overflow in a single region?
        unsigned long nNew = (unsigned long) this->pData[nIndex].aValue * (nE - nS + 1);
        nSum += nNew;
        if (nSum < nNew)
            return ::std::numeric_limits<unsigned long>::max();
        nS = nE + 1;
        if (nS <= nEnd)
            ++nIndex;
    }
    return nSum;
}


template< typename A, typename D >
unsigned long ScSummableCompressedArray<A,D>::SumScaledValuesContinuation(
        A nStart, A nEnd, size_t& nIndex, double fScale ) const
{
    unsigned long nSum = 0;
    A nS = nStart;
    while (nIndex < this->nCount && nS <= nEnd)
    {
        A nE = ::std::min( this->pData[nIndex].nEnd, nEnd);
        unsigned long nScaledVal = (unsigned long) (this->pData[nIndex].aValue * fScale);
        // FIXME: test for overflow in a single region?
        unsigned long nNew = nScaledVal * (nE - nS + 1);
        nSum += nNew;
        if (nSum < nNew)
            return ::std::numeric_limits<unsigned long>::max();
        nS = nE + 1;
        if (nS <= nEnd)
            ++nIndex;
    }
    return nSum;
}


// === ScBitMaskCompressedArray ==============================================

template< typename A, typename D >
void ScBitMaskCompressedArray<A,D>::AndValue( A nStart, A nEnd,
        const D& rValueToAnd )
{
    if (nStart > nEnd)
        return;

    size_t nIndex = Search( nStart);
    do
    {
        if ((this->pData[nIndex].aValue & rValueToAnd) != this->pData[nIndex].aValue)
        {
            A nS = ::std::max( (nIndex>0 ? this->pData[nIndex-1].nEnd+1 : 0), nStart);
            A nE = ::std::min( this->pData[nIndex].nEnd, nEnd);
            SetValue( nS, nE, this->pData[nIndex].aValue & rValueToAnd);
            if (nE >= nEnd)
                break;  // while
            nIndex = Search( nE + 1);
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

    size_t nIndex = Search( nStart);
    do
    {
        if ((this->pData[nIndex].aValue | rValueToOr) != this->pData[nIndex].aValue)
        {
            A nS = ::std::max( (nIndex>0 ? this->pData[nIndex-1].nEnd+1 : 0), nStart);
            A nE = ::std::min( this->pData[nIndex].nEnd, nEnd);
            SetValue( nS, nE, this->pData[nIndex].aValue | rValueToOr);
            if (nE >= nEnd)
                break;  // while
            nIndex = Search( nE + 1);
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
        const D& rValueToAnd, long nSourceDy )
{
    size_t nIndex;
    A nRegionEnd;
    for (A j=nStart; j<=nEnd; ++j)
    {
        const D& rValue = (j==nStart ?
                rArray.GetValue( j+nSourceDy, nIndex, nRegionEnd) :
                rArray.GetNextValue( nIndex, nRegionEnd));
        nRegionEnd -= nSourceDy;
        if (nRegionEnd > nEnd)
            nRegionEnd = nEnd;
        SetValue( j, nRegionEnd, rValue & rValueToAnd);
        j = nRegionEnd;
    }
}


template< typename A, typename D >
void ScBitMaskCompressedArray<A,D>::CopyFromOred(
        const ScBitMaskCompressedArray<A,D>& rArray, A nStart, A nEnd,
        const D& rValueToOr, long nSourceDy )
{
    size_t nIndex;
    A nRegionEnd;
    for (A j=nStart; j<=nEnd; ++j)
    {
        const D& rValue = (j==nStart ?
                rArray.GetValue( j+nSourceDy, nIndex, nRegionEnd) :
                rArray.GetNextValue( nIndex, nRegionEnd));
        nRegionEnd -= nSourceDy;
        if (nRegionEnd > nEnd)
            nRegionEnd = nEnd;
        SetValue( j, nRegionEnd, rValue | rValueToOr);
        j = nRegionEnd;
    }
}


template< typename A, typename D >
A ScBitMaskCompressedArray<A,D>::GetBitStateStart( A nEnd,
        const D& rBitMask, const D& rMaskedCompare ) const
{
    A nStart = ::std::numeric_limits<A>::max();
    size_t nIndex = Search( nEnd);
    while ((this->pData[nIndex].aValue & rBitMask) == rMaskedCompare)
    {
        if (nIndex > 0)
        {
            --nIndex;
            nStart = this->pData[nIndex].nEnd + 1;
        }
        else
        {
            nStart = 0;
            break;  // while
        }
    }
    return nStart;
}


template< typename A, typename D >
A ScBitMaskCompressedArray<A,D>::GetBitStateEnd( A nStart,
        const D& rBitMask, const D& rMaskedCompare ) const
{
    A nEnd = ::std::numeric_limits<A>::max();
    size_t nIndex = Search( nStart);
    while (nIndex < this->nCount && (this->pData[nIndex].aValue & rBitMask) ==
            rMaskedCompare)
    {
        nEnd = this->pData[nIndex].nEnd;
        ++nIndex;
    }
    return nEnd;
}


template< typename A, typename D >
A ScBitMaskCompressedArray<A,D>::GetFirstForCondition( A nStart, A nEnd,
        const D& rBitMask, const D& rMaskedCompare ) const
{
    size_t nIndex = Search( nStart);
    do
    {
        if ((this->pData[nIndex].aValue & rBitMask) == rMaskedCompare)
        {
            A nFound = nIndex > 0 ? this->pData[nIndex-1].nEnd + 1 : 0;
            return ::std::max( nFound, nStart);
        }
        if (this->pData[nIndex].nEnd >= nEnd)
            break;  // while
        ++nIndex;
    } while (nIndex < this->nCount);
    return ::std::numeric_limits<A>::max();
}


template< typename A, typename D >
A ScBitMaskCompressedArray<A,D>::GetLastForCondition( A nStart, A nEnd,
        const D& rBitMask, const D& rMaskedCompare ) const
{
    size_t nIndex = Search( nEnd);
    while (1)
    {
        if ((this->pData[nIndex].aValue & rBitMask) == rMaskedCompare)
            return ::std::min( this->pData[nIndex].nEnd, nEnd);

        if (nIndex > 0)
        {
            --nIndex;
            if (this->pData[nIndex].nEnd < nStart)
                break;  // while
        }
        else
            break;  // while
    }
    return ::std::numeric_limits<A>::max();
}


template< typename A, typename D >
A ScBitMaskCompressedArray<A,D>::CountForCondition( A nStart, A nEnd,
        const D& rBitMask, const D& rMaskedCompare ) const
{
    A nRet = 0;
    size_t nIndex = Search( nStart);
    do
    {
        if ((this->pData[nIndex].aValue & rBitMask) == rMaskedCompare)
        {
            A nS = ::std::max( (nIndex>0 ? this->pData[nIndex-1].nEnd+1 : 0), nStart);
            A nE = ::std::min( this->pData[nIndex].nEnd, nEnd);
            nRet += nE - nS + 1;
        }
        if (this->pData[nIndex].nEnd >= nEnd)
            break;  // while
        ++nIndex;
    } while (nIndex < this->nCount);
    return nRet;
}


template< typename A, typename D >
size_t ScBitMaskCompressedArray<A,D>::FillArrayForCondition( A nStart, A nEnd,
        const D& rBitMask, const D& rMaskedCompare,
        A * pArray, size_t nArraySize ) const
{
    size_t nUsed = 0;
    size_t nIndex = Search( nStart);
    while (nIndex < this->nCount && nUsed < nArraySize)
    {
        if ((this->pData[nIndex].aValue & rBitMask) == rMaskedCompare)
        {
            A nS = ::std::max( (nIndex>0 ? this->pData[nIndex-1].nEnd+1 : 0), nStart);
            A nE = ::std::min( this->pData[nIndex].nEnd, nEnd);
            while (nS <= nE && nUsed < nArraySize)
                pArray[nUsed++] = nS++;
        }
        if (this->pData[nIndex].nEnd >= nEnd)
            break;  // while
        ++nIndex;
    }
    return nUsed;
}


template< typename A, typename D >
bool ScBitMaskCompressedArray<A,D>::HasCondition( A nStart, A nEnd,
        const D& rBitMask, const D& rMaskedCompare ) const
{
    size_t nIndex = Search( nStart);
    do
    {
        if ((this->pData[nIndex].aValue & rBitMask) == rMaskedCompare)
            return true;
        if (this->pData[nIndex].nEnd >= nEnd)
            break;  // while
        ++nIndex;
    }  while (nIndex < this->nCount);
    return false;
}


template< typename A, typename D >
A ScBitMaskCompressedArray<A,D>::CountForAnyBitCondition( A nStart, A nEnd,
        const D& rBitMask ) const
{
    A nRet = 0;
    size_t nIndex = Search( nStart);
    do
    {
        if ((this->pData[nIndex].aValue & rBitMask) != 0)
        {
            A nS = ::std::max( (nIndex>0 ? this->pData[nIndex-1].nEnd+1 : 0), nStart);
            A nE = ::std::min( this->pData[nIndex].nEnd, nEnd);
            nRet += nE - nS + 1;
        }
        if (this->pData[nIndex].nEnd >= nEnd)
            break;  // while
        ++nIndex;
    } while (nIndex < this->nCount);
    return nRet;
}


template< typename A, typename D >
A ScBitMaskCompressedArray<A,D>::GetLastAnyBitAccess( A nStart,
        const D& rBitMask ) const
{
    A nEnd = ::std::numeric_limits<A>::max();
    size_t nIndex = this->nCount-1;
    while (1)
    {
        if ((this->pData[nIndex].aValue & rBitMask) != 0)
        {
            nEnd = this->pData[nIndex].nEnd;
            break;  // while
        }
        else
        {
            if (nIndex > 0)
            {
                --nIndex;
                if (this->pData[nIndex].nEnd < nStart)
                    break;  // while
            }
            else
                break;  // while
        }
    }
    return nEnd;
}


template< typename A, typename D >
template< typename S >
unsigned long ScBitMaskCompressedArray<A,D>::SumCoupledArrayForCondition(
        A nStart, A nEnd, const D& rBitMask, const D& rMaskedCompare,
        const ScSummableCompressedArray<A,S>& rArray ) const
{
    unsigned long nSum = 0;
    A nS = nStart;
    size_t nIndex1 = Search( nStart);
    size_t nIndex2 = rArray.Search( nStart);
    do
    {
        if ((this->pData[nIndex1].aValue & rBitMask) == rMaskedCompare)
        {
            while (nIndex2 < rArray.GetEntryCount() &&
                    rArray.GetDataEntry(nIndex2).nEnd < nS)
                ++nIndex2;
            unsigned long nNew = rArray.SumValuesContinuation( nS,
                    ::std::min( this->pData[nIndex1].nEnd, nEnd), nIndex2);
            nSum += nNew;
            if (nSum < nNew)
                return ::std::numeric_limits<unsigned long>::max();
        }
        nS = this->pData[nIndex1].nEnd + 1;
        ++nIndex1;
    } while (nIndex1 < this->nCount && nS <= nEnd);
    if (nEnd > this->nMaxAccess &&
            (this->pData[this->GetEntryCount()-1].aValue & rBitMask) == rMaskedCompare)
        nSum += rArray.GetDataEntry(rArray.GetEntryCount()-1).aValue * (nEnd -
                this->nMaxAccess);
    return nSum;
}


template< typename A, typename D >
template< typename S >
unsigned long ScBitMaskCompressedArray<A,D>::SumScaledCoupledArrayForCondition(
        A nStart, A nEnd, const D& rBitMask, const D& rMaskedCompare,
        const ScSummableCompressedArray<A,S>& rArray, double fScale ) const
{
    unsigned long nSum = 0;
    A nS = nStart;
    size_t nIndex1 = Search( nStart);
    size_t nIndex2 = rArray.Search( nStart);
    do
    {
        if ((this->pData[nIndex1].aValue & rBitMask) == rMaskedCompare)
        {
            while (nIndex2 < rArray.GetEntryCount() &&
                    rArray.GetDataEntry(nIndex2).nEnd < nS)
                ++nIndex2;
            unsigned long nNew = rArray.SumScaledValuesContinuation( nS,
                    ::std::min( this->pData[nIndex1].nEnd, nEnd), nIndex2, fScale);
            nSum += nNew;
            if (nSum < nNew)
                return ::std::numeric_limits<unsigned long>::max();
        }
        nS = this->pData[nIndex1].nEnd + 1;
        ++nIndex1;
    } while (nIndex1 < this->nCount && nS <= nEnd);
    if (nEnd > this->nMaxAccess &&
            (this->pData[this->GetEntryCount()-1].aValue & rBitMask) == rMaskedCompare)
        nSum += (unsigned long)
            (rArray.GetDataEntry(rArray.GetEntryCount()-1).aValue * fScale) *
            (nEnd - this->nMaxAccess);
    return nSum;
}


// === ScCompressedArrayIterator =============================================

template< typename A, typename D >
template< typename X >
void ScCompressedArrayIterator<A,D>::Follow(
        const ScCompressedArrayIterator<A,X>& rIter )
{
    nCurrent = rIter.GetPos();
    if (GetRangeStart() <= nCurrent && nCurrent <= GetRangeEnd())
        ; // nothing
    else if (nCurrent > GetRangeEnd())
    {
        A nPos = nCurrent;  // nCurrent gets changed in NextRange()
        bool bAdv;
        do
        {
            bAdv = NextRange();
        } while (bAdv && GetRangeEnd() < nPos);
        nCurrent = nPos;
    }
    else
        nIndex = rArray.Search( nCurrent);
}


// === ScCoupledCompressedArrayIterator ======================================

template< typename A, typename D, typename S >
ScCoupledCompressedArrayIterator<A,D,S>::ScCoupledCompressedArrayIterator(
        const ScBitMaskCompressedArray<A,D> & rArray1, A nStart, A nEnd,
        const D& rBitMaskP, const D& rMaskedCompareP,
        const ScCompressedArray<A,S> & rArray2 )
    : aIter1( rArray1, nStart, nEnd )
    , aIter2( rArray2, nStart, nEnd )
    , rBitMask( rBitMaskP )
    , rMaskedCompare( rMaskedCompareP )
{
    InitLimits();
}


template< typename A, typename D, typename S >
void ScCoupledCompressedArrayIterator<A,D,S>::InitLimits()
{
    bool bFound = true;
    bool bMoved = false;
    while (bFound && ((*aIter1 & rBitMask) != rMaskedCompare))
    {
        bFound = aIter1.NextRange();
        bMoved = true;
    }
    if (bMoved && bFound)
        aIter2.Follow( aIter1);
}


template< typename A, typename D, typename S >
void ScCoupledCompressedArrayIterator<A,D,S>::NewLimits( A nStart, A nEnd )
{
    aIter1.NewLimits( nStart, nEnd);
    aIter2.NewLimits( nStart, nEnd);
    InitLimits();
}


template< typename A, typename D, typename S >
bool ScCoupledCompressedArrayIterator<A,D,S>::NextRange()
{
    bool bAdv;
    if (aIter1.GetRangeEnd() <= aIter2.GetRangeEnd())
    {
        // Advance bit mask array until condition is met, coupled array
        // follows.
        do
        {
            bAdv = aIter1.NextRange();
        } while (bAdv && ((*aIter1 & rBitMask) != rMaskedCompare));
        if (bAdv)
            aIter2.Follow( aIter1);
    }
    else
    {
        // Make coupled array catch up with bit mask array.
        do
        {
            bAdv = aIter2.NextRange();
        } while (bAdv && aIter2.GetRangeEnd() < aIter1.GetRangeStart());
        if (bAdv)
            aIter1.Follow( aIter2);     // synchronize aIter1.nCurrent
    }
    return operator bool();
}


template< typename A, typename D, typename S >
void ScCoupledCompressedArrayIterator<A,D,S>::Resync( A nPos )
{
    aIter1.Resync( nPos);
    aIter2.Resync( nPos);
    InitLimits();
}


// === Force instantiation of specializations ================================

template class ScCompressedArray< SCROW, sal_uInt16>;           // heights, base class
template class ScSummableCompressedArray< SCROW, sal_uInt16>;   // heights
template class ScCompressedArray< SCROW, sal_uInt8>;             // flags, base class
template class ScBitMaskCompressedArray< SCROW, sal_uInt8>;      // flags
template unsigned long ScBitMaskCompressedArray< SCROW,
    sal_uInt8>::SumCoupledArrayForCondition( SCROW, SCROW, const sal_uInt8&, const sal_uInt8&,
            const ScSummableCompressedArray< SCROW, sal_uInt16>&) const;
template unsigned long ScBitMaskCompressedArray< SCROW,
    sal_uInt8>::SumScaledCoupledArrayForCondition( SCROW, SCROW, const sal_uInt8&,
            const sal_uInt8&, const ScSummableCompressedArray< SCROW, sal_uInt16>&,
            double) const;
template void ScCompressedArrayIterator< SCROW, sal_uInt16>::Follow(
        const ScCompressedArrayIterator< SCROW, sal_uInt8>&);
template class ScCoupledCompressedArrayIterator< SCROW, sal_uInt8, sal_uInt16>;

// === EOF ===================================================================
