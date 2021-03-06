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

#pragma once

#include <cstddef>
#include <memory>

#include "scdllapi.h"

/** Compressed array of row (or column) entries, e.g. heights, flags, ...

    The array stores ranges of values such that equal consecutive values occupy only
    one entry. Initially it consists of one DataEntry with an implied start
    row/column of 0 and an end row/column of access type maximum value.

    typename A := access type, e.g. SCROW or SCCOL, must be a POD.

    typename D := data type, e.g. sal_uInt16 or sal_uInt8 or whatever, may also be a
    struct or class.

    D::operator==() and D::operator=() must be implemented. Force template
    instantiation for a specific type in source/core/data/compressedarray.cxx

    TODO: Currently the allocated memory never shrinks, must manually invoke
    Resize() if needed.
 */

template< typename A, typename D > class ScCompressedArray
{
public:
    class Iterator
    {
        friend ScCompressedArray;
        const ScCompressedArray& mrArray;
        size_t             mnIndex = 0;
        A                  mnRegion = 0;
        Iterator(const ScCompressedArray& rArray) : mrArray(rArray) {}
        Iterator(const ScCompressedArray& rArray, size_t nIndex, A nRegion) : mrArray(rArray), mnIndex(nIndex), mnRegion(nRegion) {}
    public:
        void operator++();
        Iterator operator+(size_t) const;
        const D & operator*() const { return mrArray.pData[mnIndex].aValue; }
    };
    struct DataEntry
    {
        A   nEnd;           // start is end of previous entry + 1
        D   aValue;
            DataEntry() {}  //! uninitialized
    };
    struct RangeData
    {
        A mnRow1, mnRow2;
        D maValue;
    };

    /** Construct with nMaxAccess=MAXROW, for example. */
                                ScCompressedArray( A nMaxAccess,
                                        const D& rValue );
    virtual                     ~ScCompressedArray();
    void                        Reset( const D& rValue );
    void                        SetValue( A nPos, const D& rValue );
    void                        SetValue( A nStart, A nEnd, const D& rValue );
    [[nodiscard]]
    const D&                    GetValue( A nPos ) const;
    [[nodiscard]]
    A                           GetLastPos() const { return pData[nCount-1].nEnd; }

    /** Get value for a row, and it's region end row */
    [[nodiscard]]
    const D&                    GetValue( A nPos, size_t& nIndex, A& nEnd ) const;

    /** Get range data for a row, i.e. value and start and end rows with that value */
    [[nodiscard]]
    RangeData                   GetRangeData( A nPos ) const;

    /** Get next value and it's region end row. If nIndex<nCount, nIndex is
        incremented first. If the resulting nIndex>=nCount, the value of the
        last entry is returned again. */
    [[nodiscard]]
    const D&                    GetNextValue( size_t& nIndex, A& nEnd ) const;

    /** Insert rows before nStart and copy value for inserted rows from
        nStart-1, return that value. */
    const D&                    Insert( A nStart, size_t nCount );
    void                        InsertPreservingSize( A nStart, size_t nCount, const D& rFillValue );

    void                        Remove( A nStart, size_t nCount );
    void                        RemovePreservingSize( A nStart, size_t nCount, const D& rFillValue );

    /** Copy rArray.nStart+nSourceDy to this.nStart */
    void                        CopyFrom( const ScCompressedArray& rArray,
                                    A nStart, A nEnd )
                                { CopyFrom(rArray, nStart, nEnd, nStart); }
    void                        CopyFrom( const ScCompressedArray& rArray,
                                    A nDestStart, A nDestEnd, A nSrcStart );

    // methods public for the coupled array sum methods
    /** Obtain index into entries for nPos */
    SC_DLLPUBLIC size_t                      Search( A nPos ) const;

    Iterator                    begin() const { return Iterator(*this); }

protected:
    size_t                      nCount;
    size_t                      nLimit;
    std::unique_ptr<DataEntry[]> pData;
    A                           nMaxAccess;
};

template< typename A, typename D >
void ScCompressedArray<A,D>::Reset( const D& rValue )
{
    // Create a temporary copy in case we got a reference passed that points to
    // a part of the array to be reallocated.
    D aTmpVal( rValue);
    nCount = nLimit = 1;
    pData.reset(new DataEntry[1]);
    pData[0].aValue = aTmpVal;
    pData[0].nEnd = nMaxAccess;
}

template< typename A, typename D >
void ScCompressedArray<A,D>::SetValue( A nPos, const D& rValue )
{
    SetValue( nPos, nPos, rValue);
}

template< typename A, typename D >
const D& ScCompressedArray<A,D>::GetValue( A nPos ) const
{
    size_t nIndex = Search( nPos);
    return pData[nIndex].aValue;
}

template< typename A, typename D >
const D& ScCompressedArray<A,D>::GetValue( A nPos, size_t& nIndex, A& nEnd ) const
{
    nIndex = Search( nPos);
    nEnd = pData[nIndex].nEnd;
    return pData[nIndex].aValue;
}

template< typename A, typename D >
typename ScCompressedArray<A,D>::RangeData ScCompressedArray<A,D>::GetRangeData( A nPos ) const
{
    size_t nIndex = Search( nPos);
    RangeData aData;
    aData.mnRow1 = nIndex == 0 ? 0 : pData[nIndex - 1].nEnd + 1;
    aData.mnRow2 = pData[nIndex].nEnd;
    aData.maValue = pData[nIndex].aValue;
    return aData;
}

template< typename A, typename D >
const D& ScCompressedArray<A,D>::GetNextValue( size_t& nIndex, A& nEnd ) const
{
    if (nIndex < nCount)
        ++nIndex;
    size_t nEntry = (nIndex < nCount ? nIndex : nCount-1);
    nEnd = pData[nEntry].nEnd;
    return pData[nEntry].aValue;
}

//  ScBitMaskCompressedArray
/** The data type represents bits, manageable by bitwise operations.
 */

template< typename A, typename D > class ScBitMaskCompressedArray final : public ScCompressedArray<A,D>
{
public:
                                ScBitMaskCompressedArray( A nMaxAccessP,
                                        const D& rValue )
                                    : ScCompressedArray<A,D>( nMaxAccessP, rValue )
                                    {}
    void                        AndValue( A nPos, const D& rValueToAnd );
    void                        OrValue( A nPos, const D& rValueToOr );
    void                        AndValue( A nStart, A nEnd, const D& rValueToAnd );
    void                        OrValue( A nStart, A nEnd, const D& rValueToOr );

    /** Copy values from rArray and bitwise AND them with rValueToAnd. */
    void                        CopyFromAnded(
                                    const ScBitMaskCompressedArray& rArray,
                                    A nStart, A nEnd, const D& rValueToAnd );

    /** Return the last row where an entry meets the condition:
        ((aValue & rBitMask) != 0), start searching at 0. If no entry
        meets this condition, ::std::numeric_limits<A>::max() is returned. */
    A                           GetLastAnyBitAccess( const D& rBitMask ) const;
};

template< typename A, typename D >
void ScBitMaskCompressedArray<A,D>::AndValue( A nPos, const D& rValueToAnd )
{
    const D& rValue = this->GetValue( nPos);
    if ((rValue & rValueToAnd) != rValue)
        this->SetValue( nPos, rValue & rValueToAnd);
}

template< typename A, typename D >
void ScBitMaskCompressedArray<A,D>::OrValue( A nPos, const D& rValueToOr )
{
    const D& rValue = this->GetValue( nPos);
    if ((rValue | rValueToOr) != rValue)
        this->SetValue( nPos, rValue | rValueToOr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
