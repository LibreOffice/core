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

#include "compressedarray.hxx"
#include "address.hxx"
#include "global.hxx"

#include <algorithm>

template< typename A, typename D >
ScCompressedArray<A,D>::ScCompressedArray( A nMaxAccessP, const D& rValue )
    : m_data(1)
    , nMaxAccess(nMaxAccessP)
{
    m_data[0].aValue = rValue;
    m_data[0].nEnd = nMaxAccess;
}

template< typename A, typename D >
ScCompressedArray<A,D>::ScCompressedArray( A nMaxAccessP, const D* pDataArray, size_t nDataCount )
    : m_data(0)
    , nMaxAccess(nMaxAccessP)
{
    D aValue = pDataArray[0];
    for ( size_t j = 0; j < nDataCount; ++j )
    {
        if ( aValue != pDataArray[j] )
            m_data.emplace_back( j-1, aValue );
        aValue = pDataArray[j];
    }
    m_data.emplace_back( nMaxAccess, aValue );
    m_data.shrink_to_fit();
}

template< typename A, typename D >
void ScCompressedArray<A,D>::Resize( size_t nNewLimit )
{
    if ( nNewLimit == m_data.size() )
        m_data.shrink_to_fit();
    else
        m_data.reserve( nNewLimit );
}

template< typename A, typename D >
size_t ScCompressedArray<A,D>::Search( A nAccess ) const
{
    if (nAccess == 0)
        return 0;

    long nLo    = 0;
    long nHi    = static_cast<long>(m_data.size()) - 1;
    long nStart = 0;
    long i      = 0;
    bool bFound = (m_data.size() == 1);
    while (!bFound && nLo <= nHi)
    {
        i = (nLo + nHi) / 2;
        if (i > 0)
            nStart = static_cast<long>(m_data[i - 1].nEnd);
        else
            nStart = -1;
        long nEnd = static_cast<long>(m_data[i].nEnd);
        if (nEnd < static_cast<long>(nAccess))
            nLo = ++i;
        else
            if (nStart >= static_cast<long>(nAccess))
                nHi = --i;
            else
                bFound = true;
    }
    return (bFound ? static_cast<size_t>(i) : (nAccess < 0 ? 0 : m_data.size()-1));
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

            size_t ni = 0;          // number of leading entries
            size_t nInsert = 0;     // insert position (nMaxAccess+1 := no insert)
            bool bCombined = false;
            bool bSplit = false;
            if (nStart > 0)
            {
                // skip leading
                ni = this->Search( nStart);

                nInsert = nMaxAccess+1;
                if ( m_data[ni].aValue != aNewVal )
                {
                    if ( ni == 0 || (m_data[ni-1].nEnd < nStart - 1) )
                    {   // may be a split or a simple insert or just a shrink,
                        // row adjustment is done further down
                        if ( m_data[ni].nEnd > nEnd )
                            bSplit = true;
                        ni++;
                        nInsert = ni;
                    }
                    else if ( ni > 0 && m_data[ni-1].nEnd == nStart - 1 )
                        nInsert = ni;
                }
                if ( ni > 0 && m_data[ni-1].aValue == aNewVal )
                {   // combine
                    m_data[ni-1].nEnd = nEnd;
                    nInsert = nMaxAccess+1;
                    bCombined = true;
                }
            }

            size_t nj = ni;     // stop position of range to replace
            while ( nj < m_data.size() && m_data[nj].nEnd <= nEnd )
                nj++;
            if ( !bSplit )
            {
                if ( nj < m_data.size() && m_data[nj].aValue == aNewVal )
                {   // combine
                    if (ni > 0)
                    {
                        if ( m_data[ni-1].aValue == aNewVal )
                        {   // adjacent entries
                            m_data[ni-1].nEnd = m_data[nj].nEnd;
                            nj++;
                        }
                        else if ( ni == nInsert )
                            m_data[ni-1].nEnd = nStart - 1;   // shrink
                    }
                    nInsert = nMaxAccess+1;
                    bCombined = true;
                }
                else if (ni > 0 && ni == nInsert)
                    m_data[ni-1].nEnd = nStart - 1;   // shrink
            }
            if ( ni < nj )
            {   // remove middle entries
                if ( !bCombined )
                {   // replace one entry
                    m_data[ni].nEnd = nEnd;
                    m_data[ni].aValue = aNewVal;
                    ni++;
                    nInsert = nMaxAccess+1;
                }
                m_data.erase( m_data.begin()+ni, m_data.begin()+nj );
            }

            if ( nInsert < static_cast<size_t>(nMaxAccess+1) )
            {   // insert or append new entry
                if ( nInsert <= m_data.size() && bSplit )
                {
                    m_data.insert( m_data.begin()+nInsert, m_data[nInsert-1] );
                }
                if (nInsert)
                    m_data[nInsert-1].nEnd = nStart - 1;
                m_data.insert( m_data.begin()+nInsert, { nEnd, aNewVal } );
            }
        }
    }
}

template< typename A, typename D >
void ScCompressedArray<A,D>::CopyFrom( const ScCompressedArray<A,D>& rArray, A nStart,
        A nEnd )
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
    if ( nIndex > 0 && m_data[nIndex-1].nEnd+1 == nStart )
        --nIndex;
    const D& rValue = m_data[nIndex].aValue; // the value "copied"
    do
    {
        m_data[nIndex].nEnd += nAccessCount;
        if ( m_data[nIndex].nEnd >= nMaxAccess )
        {
            m_data[nIndex].nEnd = nMaxAccess;
            m_data.resize(nIndex + 1);    // discard trailing entries
        }
    } while (++nIndex < m_data.size());
    return rValue;
}

template< typename A, typename D >
void ScCompressedArray<A,D>::Remove( A nStart, size_t nAccessCount )
{
    A nEnd = nStart + nAccessCount - 1;
    size_t nIndex = this->Search( nStart);
    // equalize/combine/remove all entries in between
    if ( nEnd > m_data[nIndex].nEnd )
        this->SetValue( nStart, nEnd, m_data[nIndex].aValue);
    // remove an exactly matching entry by shifting up all following by one
    if ((nStart == 0 || (nIndex > 0 && nStart == m_data[nIndex-1].nEnd+1)) &&
            m_data[nIndex].nEnd == nEnd && nIndex < m_data.size()-1)
    {
        // In case removing an entry results in two adjacent entries with
        // identical data, combine them into one. This is also necessary to
        // make the algorithm used in SetValue() work correctly, it relies on
        // the fact that consecutive values actually differ.
        const auto last = m_data.begin() + nIndex + 1;
        if ( nIndex > 0 && m_data[nIndex-1].aValue == m_data[nIndex+1].aValue )
        {
            --nIndex;
        }
        m_data.erase( m_data.begin() + nIndex, last );
    }
    // adjust end rows, nIndex still being valid
    do
    {
        m_data[nIndex].nEnd -= nAccessCount;
    } while (++nIndex < m_data.size());
    m_data[m_data.size()-1].nEnd = nMaxAccess;
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
        if ((this->m_data[nIndex].aValue & rValueToAnd) != this->m_data[nIndex].aValue)
        {
            A nS = ::std::max( (nIndex>0 ? this->m_data[nIndex-1].nEnd+1 : 0), nStart);
            A nE = ::std::min( this->m_data[nIndex].nEnd, nEnd);
            this->SetValue( nS, nE, this->m_data[nIndex].aValue & rValueToAnd);
            if (nE >= nEnd)
                break;  // while
            nIndex = this->Search( nE + 1);
        }
        else if (this->m_data[nIndex].nEnd >= nEnd)
            break;  // while
        else
            ++nIndex;
    } while (nIndex < this->m_data.size());
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
        if ((this->m_data[nIndex].aValue | rValueToOr) != this->m_data[nIndex].aValue)
        {
            A nS = ::std::max( (nIndex>0 ? this->m_data[nIndex-1].nEnd+1 : 0), nStart);
            A nE = ::std::min( this->m_data[nIndex].nEnd, nEnd);
            this->SetValue( nS, nE, this->m_data[nIndex].aValue | rValueToOr);
            if (nE >= nEnd)
                break;  // while
            nIndex = this->Search( nE + 1);
        }
        else if (this->m_data[nIndex].nEnd >= nEnd)
            break;  // while
        else
            ++nIndex;
    } while (nIndex < this->m_data.size());
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
    size_t nIndex = this->m_data.size()-1;
    while (true)
    {
        if ( this->m_data[nIndex].aValue & rBitMask )
        {
            nEnd = this->m_data[nIndex].nEnd;
            break;  // while
        }
        else
        {
            if (nIndex > 0)
            {
                --nIndex;
                if (this->m_data[nIndex].nEnd < 0)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
