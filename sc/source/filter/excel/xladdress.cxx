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

#include <xladdress.hxx>
#include <xestream.hxx>
#include <xltracer.hxx>
#include <xistream.hxx>

#include <osl/diagnose.h>

void XclAddress::Read( XclImpStream& rStrm )
{
    mnRow = rStrm.ReaduInt16();
    mnCol = rStrm.ReaduInt16();
}

void XclAddress::Write( XclExpStream& rStrm ) const
{
    rStrm << static_cast<sal_uInt16> (mnRow);
    rStrm << mnCol;
}

bool XclRange::Contains( const XclAddress& rPos ) const
{
    return  (maFirst.mnCol <= rPos.mnCol) && (rPos.mnCol <= maLast.mnCol) &&
            (maFirst.mnRow <= rPos.mnRow) && (rPos.mnRow <= maLast.mnRow);
}

void XclRange::Read( XclImpStream& rStrm, bool bCol16Bit )
{
    maFirst.mnRow = rStrm.ReaduInt16();
    maLast.mnRow = rStrm.ReaduInt16();

    if( bCol16Bit )
    {
        maFirst.mnCol = rStrm.ReaduInt16();
        maLast.mnCol = rStrm.ReaduInt16();
    }
    else
    {
        maFirst.mnCol = rStrm.ReaduInt8();
        maLast.mnCol = rStrm.ReaduInt8();
    }
}

void XclRange::Write( XclExpStream& rStrm, bool bCol16Bit ) const
{
    rStrm << static_cast<sal_uInt16>(maFirst.mnRow) << static_cast<sal_uInt16>(maLast.mnRow);
    if( bCol16Bit )
        rStrm << maFirst.mnCol << maLast.mnCol;
    else
        rStrm << static_cast< sal_uInt8 >( maFirst.mnCol ) << static_cast< sal_uInt8 >( maLast.mnCol );
}

XclRange XclRangeList::GetEnclosingRange() const
{
    XclRange aXclRange;
    if( !mRanges.empty() )
    {
        XclRangeVector::const_iterator aIt = mRanges.begin(), aEnd = mRanges.end();
        aXclRange = *aIt;
        for( ++aIt; aIt != aEnd; ++aIt )
        {
            aXclRange.maFirst.mnCol = ::std::min( aXclRange.maFirst.mnCol, aIt->maFirst.mnCol );
            aXclRange.maFirst.mnRow = ::std::min( aXclRange.maFirst.mnRow, aIt->maFirst.mnRow );
            aXclRange.maLast.mnCol = ::std::max( aXclRange.maLast.mnCol, aIt->maLast.mnCol );
            aXclRange.maLast.mnRow = ::std::max( aXclRange.maLast.mnRow, aIt->maLast.mnRow );
        }
    }
    return aXclRange;
}

void XclRangeList::Read( XclImpStream& rStrm, bool bCol16Bit, sal_uInt16 nCountInStream )
{
    sal_uInt16 nCount;
    if (nCountInStream)
        nCount = nCountInStream;
    else
        nCount = rStrm.ReaduInt16();
    size_t nOldSize = mRanges.size();
    mRanges.resize( nOldSize + nCount );
    for( XclRangeVector::iterator aIt = mRanges.begin() + nOldSize; rStrm.IsValid() && (nCount > 0); --nCount, ++aIt )
        aIt->Read( rStrm, bCol16Bit );
}

void XclRangeList::Write( XclExpStream& rStrm, bool bCol16Bit, sal_uInt16 nCountInStream ) const
{
    WriteSubList( rStrm, 0, mRanges.size(), bCol16Bit, nCountInStream );
}

void XclRangeList::WriteSubList( XclExpStream& rStrm, size_t nBegin, size_t nCount, bool bCol16Bit,
        sal_uInt16 nCountInStream ) const
{
    OSL_ENSURE( nBegin <= mRanges.size(), "XclRangeList::WriteSubList - invalid start position" );
    size_t nEnd = ::std::min< size_t >( nBegin + nCount, mRanges.size() );
    if (!nCountInStream)
    {
        sal_uInt16 nXclCount = ulimit_cast< sal_uInt16 >( nEnd - nBegin );
        rStrm << nXclCount;
    }
    rStrm.SetSliceSize( bCol16Bit ? 8 : 6 );
    std::for_each(mRanges.begin() + nBegin, mRanges.begin() + nEnd,
        [&rStrm, &bCol16Bit](const XclRange& rRange) { rRange.Write(rStrm, bCol16Bit); });
}

XclAddressConverterBase::XclAddressConverterBase( XclTracer& rTracer, const ScAddress& rMaxPos ) :
    mrTracer( rTracer ),
    maMaxPos( rMaxPos ),
    mnMaxCol( static_cast< sal_uInt16 >( rMaxPos.Col() ) ),
    mnMaxRow( static_cast< sal_uInt16 >( rMaxPos.Row() ) ),
    mbColTrunc( false ),
    mbRowTrunc( false ),
    mbTabTrunc( false )
{
    OSL_ENSURE( static_cast< size_t >( rMaxPos.Col() ) <= SAL_MAX_UINT16, "XclAddressConverterBase::XclAddressConverterBase - invalid max column" );
    OSL_ENSURE( static_cast< size_t >( rMaxPos.Row() ) <= SAL_MAX_UINT32, "XclAddressConverterBase::XclAddressConverterBase - invalid max row" );
}

XclAddressConverterBase::~XclAddressConverterBase()
{
}

void XclAddressConverterBase::CheckScTab( SCTAB nScTab )
{
    bool bValid = (0 <= nScTab) && (nScTab <= maMaxPos.Tab());
    if( !bValid )
    {
        mbTabTrunc |= (nScTab > maMaxPos.Tab());  // do not warn for deleted refs
        mrTracer.TraceInvalidTab( nScTab, maMaxPos.Tab() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
