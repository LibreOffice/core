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

            size_t ni;          
            size_t nInsert;     
            bool bCombined = false;
            bool bSplit = false;
            if (nStart > 0)
            {
                
                ni = this->Search( nStart);

                nInsert = nMaxAccess+1;
                if (!(pData[ni].aValue == aNewVal))
                {
                    if (ni == 0 || (pData[ni-1].nEnd < nStart - 1))
                    {   
                        
                        if (pData[ni].nEnd > nEnd)
                            bSplit = true;
                        ni++;
                        nInsert = ni;
                    }
                    else if (ni > 0 && pData[ni-1].nEnd == nStart - 1)
                        nInsert = ni;
                }
                if (ni > 0 && pData[ni-1].aValue == aNewVal)
                {   
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

            size_t nj = ni;     
            while (nj < nCount && pData[nj].nEnd <= nEnd)
                nj++;
            if (!bSplit)
            {
                if (nj < nCount && pData[nj].aValue == aNewVal)
                {   
                    if (ni > 0)
                    {
                        if (pData[ni-1].aValue == aNewVal)
                        {   
                            pData[ni-1].nEnd = pData[nj].nEnd;
                            nj++;
                        }
                        else if (ni == nInsert)
                            pData[ni-1].nEnd = nStart - 1;   
                    }
                    nInsert = nMaxAccess+1;
                    bCombined = true;
                }
                else if (ni > 0 && ni == nInsert)
                    pData[ni-1].nEnd = nStart - 1;   
            }
            if (ni < nj)
            {   
                if (!bCombined)
                {   
                    pData[ni].nEnd = nEnd;
                    pData[ni].aValue = aNewVal;
                    ni++;
                    nInsert = nMaxAccess+1;
                }
                if (ni < nj)
                {   
                    memmove( pData + ni, pData + nj,
                            (nCount - nj) * sizeof(DataEntry));
                    nCount -= nj - ni;
                }
            }

            if (nInsert < static_cast<size_t>(nMaxAccess+1))
            {   
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
        this->SetValue( j, nRegionEnd, rValue);
        j = nRegionEnd;
    }
}


template< typename A, typename D >
const D& ScCompressedArray<A,D>::Insert( A nStart, size_t nAccessCount )
{
    size_t nIndex = this->Search( nStart);
    
    
    
    if (nIndex > 0 && pData[nIndex-1].nEnd+1 == nStart)
        --nIndex;
    const D& rValue = pData[nIndex].aValue; 
    do
    {
        pData[nIndex].nEnd += nAccessCount;
        if (pData[nIndex].nEnd >= nMaxAccess)
        {
            pData[nIndex].nEnd = nMaxAccess;
            nCount = nIndex + 1;    
        }
    } while (++nIndex < nCount);
    return rValue;
}


template< typename A, typename D >
void ScCompressedArray<A,D>::Remove( A nStart, size_t nAccessCount )
{
    A nEnd = nStart + nAccessCount - 1;
    size_t nIndex = this->Search( nStart);
    
    if (nEnd > pData[nIndex].nEnd)
        this->SetValue( nStart, nEnd, pData[nIndex].aValue);
    
    if ((nStart == 0 || (nIndex > 0 && nStart == pData[nIndex-1].nEnd+1)) &&
            pData[nIndex].nEnd == nEnd && nIndex < nCount-1)
    {
        
        
        
        
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
    
    do
    {
        pData[nIndex].nEnd -= nAccessCount;
    } while (++nIndex < nCount);
    pData[nCount-1].nEnd = nMaxAccess;
}




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
            A nS = ::std::max( (nIndex>0 ? this->pData[nIndex-1].nEnd+1 : 0), nStart);
            A nE = ::std::min( this->pData[nIndex].nEnd, nEnd);
            this->SetValue( nS, nE, this->pData[nIndex].aValue & rValueToAnd);
            if (nE >= nEnd)
                break;  
            nIndex = this->Search( nE + 1);
        }
        else if (this->pData[nIndex].nEnd >= nEnd)
            break;  
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
            A nS = ::std::max( (nIndex>0 ? this->pData[nIndex-1].nEnd+1 : 0), nStart);
            A nE = ::std::min( this->pData[nIndex].nEnd, nEnd);
            this->SetValue( nS, nE, this->pData[nIndex].aValue | rValueToOr);
            if (nE >= nEnd)
                break;  
            nIndex = this->Search( nE + 1);
        }
        else if (this->pData[nIndex].nEnd >= nEnd)
            break;  
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
        this->SetValue( j, nRegionEnd, rValue & rValueToAnd);
        j = nRegionEnd;
    }
}

template< typename A, typename D >
A ScBitMaskCompressedArray<A,D>::GetFirstForCondition( A nStart, A nEnd,
        const D& rBitMask, const D& rMaskedCompare ) const
{
    size_t nIndex = this->Search( nStart);
    do
    {
        if ((this->pData[nIndex].aValue & rBitMask) == rMaskedCompare)
        {
            A nFound = nIndex > 0 ? this->pData[nIndex-1].nEnd + 1 : 0;
            return ::std::max( nFound, nStart);
        }
        if (this->pData[nIndex].nEnd >= nEnd)
            break;  
        ++nIndex;
    } while (nIndex < this->nCount);
    return ::std::numeric_limits<A>::max();
}

template< typename A, typename D >
A ScBitMaskCompressedArray<A,D>::GetLastAnyBitAccess( A nStart,
        const D& rBitMask ) const
{
    A nEnd = ::std::numeric_limits<A>::max();
    size_t nIndex = this->nCount-1;
    while (true)
    {
        if ((this->pData[nIndex].aValue & rBitMask) != 0)
        {
            nEnd = this->pData[nIndex].nEnd;
            break;  
        }
        else
        {
            if (nIndex > 0)
            {
                --nIndex;
                if (this->pData[nIndex].nEnd < nStart)
                    break;  
            }
            else
                break;  
        }
    }
    return nEnd;
}



template class ScCompressedArray< SCROW, sal_uInt8>;             
template class ScBitMaskCompressedArray< SCROW, sal_uInt8>;      

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
