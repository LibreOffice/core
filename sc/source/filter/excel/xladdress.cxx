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
#include "precompiled_sc.hxx"
#include "xladdress.hxx"
#include "xestream.hxx"
#include "xltracer.hxx"
#include "xistream.hxx"

// ============================================================================

void XclAddress::Read( XclImpStream& rStrm, bool bCol16Bit )
{
    rStrm >> mnRow;
    if( bCol16Bit )
        rStrm >> mnCol;
    else
        mnCol = rStrm.ReaduInt8();
}

void XclAddress::Write( XclExpStream& rStrm, bool bCol16Bit ) const
{
    rStrm << mnRow;
    if( bCol16Bit )
        rStrm << mnCol;
    else
        rStrm << static_cast< sal_uInt8 >( mnCol );
}

// ----------------------------------------------------------------------------

bool XclRange::Contains( const XclAddress& rPos ) const
{
    return  (maFirst.mnCol <= rPos.mnCol) && (rPos.mnCol <= maLast.mnCol) &&
            (maFirst.mnRow <= rPos.mnRow) && (rPos.mnRow <= maLast.mnRow);
}

void XclRange::Read( XclImpStream& rStrm, bool bCol16Bit )
{
    rStrm >> maFirst.mnRow >> maLast.mnRow;
    if( bCol16Bit )
        rStrm >> maFirst.mnCol >> maLast.mnCol;
    else
    {
        maFirst.mnCol = rStrm.ReaduInt8();
        maLast.mnCol = rStrm.ReaduInt8();
    }
}

void XclRange::Write( XclExpStream& rStrm, bool bCol16Bit ) const
{
    rStrm << maFirst.mnRow << maLast.mnRow;
    if( bCol16Bit )
        rStrm << maFirst.mnCol << maLast.mnCol;
    else
        rStrm << static_cast< sal_uInt8 >( maFirst.mnCol ) << static_cast< sal_uInt8 >( maLast.mnCol );
}

// ----------------------------------------------------------------------------

XclRange XclRangeList::GetEnclosingRange() const
{
    XclRange aXclRange;
    if( !empty() )
    {
        const_iterator aIt = begin(), aEnd = end();
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

void XclRangeList::Read( XclImpStream& rStrm, bool bCol16Bit )
{
    sal_uInt16 nCount;
    rStrm >> nCount;
    size_t nOldSize = size();
    resize( nOldSize + nCount );
    for( iterator aIt = begin() + nOldSize; rStrm.IsValid() && (nCount > 0); --nCount, ++aIt )
        aIt->Read( rStrm, bCol16Bit );
}

void XclRangeList::Write( XclExpStream& rStrm, bool bCol16Bit ) const
{
    WriteSubList( rStrm, 0, size(), bCol16Bit );
}

void XclRangeList::WriteSubList( XclExpStream& rStrm, size_t nBegin, size_t nCount, bool bCol16Bit ) const
{
    DBG_ASSERT( nBegin <= size(), "XclRangeList::WriteSubList - invalid start position" );
    size_t nEnd = ::std::min< size_t >( nBegin + nCount, size() );
    sal_uInt16 nXclCount = ulimit_cast< sal_uInt16 >( nEnd - nBegin );
    rStrm << nXclCount;
    rStrm.SetSliceSize( bCol16Bit ? 8 : 6 );
    for( const_iterator aIt = begin() + nBegin, aEnd = begin() + nEnd; aIt != aEnd; ++aIt )
        aIt->Write( rStrm, bCol16Bit );
}

// ============================================================================

XclAddressConverterBase::XclAddressConverterBase( XclTracer& rTracer, const ScAddress& rMaxPos ) :
    mrTracer( rTracer ),
    maMaxPos( rMaxPos ),
    mnMaxCol( static_cast< sal_uInt16 >( rMaxPos.Col() ) ),
    mnMaxRow( static_cast< sal_uInt16 >( rMaxPos.Row() ) ),
    mbColTrunc( false ),
    mbRowTrunc( false ),
    mbTabTrunc( false )
{
}

XclAddressConverterBase::~XclAddressConverterBase()
{
}

bool XclAddressConverterBase::CheckScTab( SCTAB nScTab, bool bWarn )
{
    bool bValid = (0 <= nScTab) && (nScTab <= maMaxPos.Tab());
    if( !bValid && bWarn )
    {
        mbTabTrunc |= (nScTab > maMaxPos.Tab());  // do not warn for deleted refs
        mrTracer.TraceInvalidTab( nScTab, maMaxPos.Tab() );
    }
    return bValid;
}

// ============================================================================

