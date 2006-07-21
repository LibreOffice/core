/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xestring.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:02:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef SC_XESTRING_HXX
#include "xestring.hxx"
#endif

#include <algorithm>

#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif
#ifndef SC_XESTREAM_HXX
#include "xestream.hxx"
#endif

using ::rtl::OUString;

// ============================================================================

namespace {

// compare vectors

/** Compares two vectors.
    @return  A negative value, if rLeft<rRight; or a positive value, if rLeft>rRight;
    or 0, if rLeft==rRight. */
template< typename Type >
int lclCompareVectors( const ::std::vector< Type >& rLeft, const ::std::vector< Type >& rRight )
{
    int nResult = 0;

    // 1st: compare all elements of the vectors
    typedef typename ::std::vector< Type >::const_iterator CIT;
    CIT aEndL = rLeft.end(), aEndR = rRight.end();
    for( CIT aItL = rLeft.begin(), aItR = rRight.begin(); !nResult && (aItL != aEndL) && (aItR != aEndR); ++aItL, ++aItR )
        nResult = static_cast< int >( *aItL ) - static_cast< int >( *aItR );

    // 2nd: no differences found so far -> compare the vector sizes. Shorter vector is less
    if( !nResult )
        nResult = static_cast< int >( rLeft.size() ) - static_cast< int >( rRight.size() );

    return nResult;
}

// hashing helpers

/** Base class for value hashers.
    @descr  These function objects are used to hash any value to a sal_uInt32 value. */
template< typename Type >
struct XclHasher : public ::std::unary_function< Type, sal_uInt32 > {};

template< typename Type >
struct XclDirectHasher : public XclHasher< Type >
{
    inline sal_uInt32   operator()( Type nVal ) const { return nVal; }
};

struct XclFormatRunHasher : public XclHasher< const XclFormatRun& >
{
    inline sal_uInt32   operator()( const XclFormatRun& rRun ) const
                            { return (rRun.mnChar << 8) ^ rRun.mnFontIdx; }
};

/** Calculates a hash value from a vector.
    @descr Uses the passed hasher function object to calculate hash values from
    all vector elements. */
template< typename Type, typename ValueHasher >
sal_uInt16 lclHashVector( const ::std::vector< Type >& rVec, const ValueHasher& rHasher )
{
    sal_uInt32 nHash = rVec.size();
    typedef typename ::std::vector< Type >::const_iterator CIT;
    for( CIT aIt = rVec.begin(), aEnd = rVec.end(); aIt != aEnd; ++aIt )
        (nHash *= 31) += rHasher( *aIt );
    return static_cast< sal_uInt16 >( nHash ^ (nHash >> 16) );
}

/** Calculates a hash value from a vector. Uses XclDirectHasher to hash the vector elements. */
template< typename Type >
inline sal_uInt16 lclHashVector( const ::std::vector< Type >& rVec )
{
    return lclHashVector( rVec, XclDirectHasher< Type >() );
}

} // namespace

// constructors ---------------------------------------------------------------

XclExpString::XclExpString( XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Init( 0, nFlags, nMaxLen, true );
}

XclExpString::XclExpString( const String& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, nFlags, nMaxLen );
}

XclExpString::XclExpString(
    const String& rString, const XclFormatRunVec& rFormats,
    XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, rFormats, nFlags, nMaxLen );
}

XclExpString::XclExpString( const OUString& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, nFlags, nMaxLen );
}

XclExpString::XclExpString(
        const OUString& rString, const XclFormatRunVec& rFormats,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, rFormats, nFlags, nMaxLen );
}

// assign ---------------------------------------------------------------------

void XclExpString::Assign( const String& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Build( rString.GetBuffer(), rString.Len(), nFlags, nMaxLen );
}

void XclExpString::Assign(
        const String& rString, const XclFormatRunVec& rFormats,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, nFlags, nMaxLen );
    SetFormats( rFormats );
}

void XclExpString::Assign( const OUString& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Build( rString.getStr(), rString.getLength(), nFlags, nMaxLen );
}

void XclExpString::Assign(
        const OUString& rString, const XclFormatRunVec& rFormats,
        XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, nFlags, nMaxLen );
    SetFormats( rFormats );
}

void XclExpString::Assign( sal_Unicode cChar, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Build( &cChar, 1, nFlags, nMaxLen );
}

void XclExpString::AssignByte(
        const String& rString, CharSet eCharSet, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    ByteString aByteStr( rString, eCharSet );   // length may differ from length of rString
    Build( aByteStr.GetBuffer(), aByteStr.Len(), nFlags, nMaxLen );
}

void XclExpString::AssignByte( sal_Unicode cChar, CharSet eCharSet, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    if( !cChar )
    {
        sal_Char cByteChar = 0;
        Build( &cByteChar, 1, nFlags, nMaxLen );
    }
    else
    {
        ByteString aByteStr( &cChar, 1, eCharSet );     // length may be >1
        Build( aByteStr.GetBuffer(), aByteStr.Len(), nFlags, nMaxLen );
    }
}

// append ---------------------------------------------------------------------

void XclExpString::Append( const String& rString )
{
    BuildAppend( rString.GetBuffer(), rString.Len() );
}

void XclExpString::Append( const ::rtl::OUString& rString )
{
    BuildAppend( rString.getStr(), rString.getLength() );
}

void XclExpString::Append( sal_Unicode cChar )
{
    BuildAppend( &cChar, 1 );
}

void XclExpString::AppendByte( const String& rString, CharSet eCharSet )
{
    ByteString aByteStr( rString, eCharSet );   // length may differ from length of rString
    BuildAppend( aByteStr.GetBuffer(), aByteStr.Len() );
}

void XclExpString::AppendByte( sal_Unicode cChar, CharSet eCharSet )
{
    if( !cChar )
    {
        sal_Char cByteChar = 0;
        BuildAppend( &cByteChar, 1 );
    }
    else
    {
        ByteString aByteStr( &cChar, 1, eCharSet );     // length may be >1
        BuildAppend( aByteStr.GetBuffer(), aByteStr.Len() );
    }
}

// formatting runs ------------------------------------------------------------

void XclExpString::SetFormats( const XclFormatRunVec& rFormats )
{
    maFormats = rFormats;
#ifdef DBG_UTIL
    if( IsRich() )
    {
        XclFormatRunVec::const_iterator aCurr = maFormats.begin();
        XclFormatRunVec::const_iterator aPrev = aCurr;
        XclFormatRunVec::const_iterator aEnd = maFormats.end();
        for( ++aCurr; aCurr != aEnd; ++aCurr, ++aPrev )
            DBG_ASSERT( aPrev->mnChar < aCurr->mnChar, "XclExpString::SetFormats - invalid char order" );
        DBG_ASSERT( aPrev->mnChar <= mnLen, "XclExpString::SetFormats - invalid char index" );
    }
#endif
    LimitFormatCount( mbIsBiff8 ? EXC_STR_MAXLEN : EXC_STR_MAXLEN_8BIT );
}

void XclExpString::AppendFormat( sal_uInt16 nChar, sal_uInt16 nFontIdx )
{
    DBG_ASSERT( maFormats.empty() || (maFormats.back().mnChar < nChar), "XclExpString::AppendFormat - invalid char index" );
    if( maFormats.size() < static_cast< size_t >( mbIsBiff8 ? EXC_STR_MAXLEN : EXC_STR_MAXLEN_8BIT ) )
        maFormats.push_back( XclFormatRun( nChar, nFontIdx ) );
}

void XclExpString::LimitFormatCount( sal_uInt16 nMaxCount )
{
    if( maFormats.size() > nMaxCount )
        maFormats.erase( maFormats.begin() + nMaxCount, maFormats.end() );
}

sal_uInt16 XclExpString::RemoveLeadingFont()
{
    sal_uInt16 nFontIdx = EXC_FONT_NOTFOUND;
    if( !maFormats.empty() && (maFormats.front().mnChar == 0) )
    {
        nFontIdx = maFormats.front().mnFontIdx;
        maFormats.erase( maFormats.begin() );
    }
    return nFontIdx;
}

bool XclExpString::IsEqual( const XclExpString& rCmp ) const
{
    return
        (mnLen          == rCmp.mnLen)          &&
        (mbIsBiff8      == rCmp.mbIsBiff8)      &&
        (mbIsUnicode    == rCmp.mbIsUnicode)    &&
        (mbWrapped      == rCmp.mbWrapped)      &&
        (
            ( mbIsBiff8 && (maUniBuffer  == rCmp.maUniBuffer)) ||
            (!mbIsBiff8 && (maCharBuffer == rCmp.maCharBuffer))
        ) &&
        (maFormats      == rCmp.maFormats);
}

bool XclExpString::IsLessThan( const XclExpString& rCmp ) const
{
    int nResult = mbIsBiff8 ?
        lclCompareVectors( maUniBuffer, rCmp.maUniBuffer ) :
        lclCompareVectors( maCharBuffer, rCmp.maCharBuffer );
    return (nResult != 0) ? (nResult < 0) : (maFormats < rCmp.maFormats);
}

// get data -------------------------------------------------------------------

sal_uInt16 XclExpString::GetFormatsCount() const
{
    return static_cast< sal_uInt16 >( maFormats.size() );
}

sal_uInt8 XclExpString::GetFlagField() const
{
    return (mbIsUnicode ? EXC_STRF_16BIT : 0) | (IsRich() ? EXC_STRF_RICH : 0);
}

sal_uInt16 XclExpString::GetHeaderSize() const
{
    return
        (mb8BitLen ? 1 : 2) +           // length field
        (IsWriteFlags() ? 1 : 0) +      // flag field
        (IsWriteFormats() ? 2 : 0);     // richtext formattting count
}

sal_Size XclExpString::GetBufferSize() const
{
    return mnLen * (mbIsUnicode ? 2 : 1);
}

sal_Size XclExpString::GetSize() const
{
    return
        GetHeaderSize() +                                   // header
        GetBufferSize() +                                   // character buffer
        (IsWriteFormats() ? (4 * GetFormatsCount()) : 0);   // richtext formattting
}

sal_uInt16 XclExpString::GetChar( sal_uInt16 nCharIdx ) const
{
    DBG_ASSERT( nCharIdx < Len(), "XclExpString::GetChar - invalid character index" );
    return static_cast< sal_uInt16 >( mbIsBiff8 ? maUniBuffer[ nCharIdx ] : maCharBuffer[ nCharIdx ] );
}

sal_uInt16 XclExpString::GetHash() const
{
    return
        (mbIsBiff8 ? lclHashVector( maUniBuffer ) : lclHashVector( maCharBuffer )) ^
        lclHashVector( maFormats, XclFormatRunHasher() );
}

// streaming ------------------------------------------------------------------

void XclExpString::WriteLenField( XclExpStream& rStrm ) const
{
    if( mb8BitLen )
        rStrm << static_cast< sal_uInt8 >( mnLen );
    else
        rStrm << mnLen;
}

void XclExpString::WriteFlagField( XclExpStream& rStrm ) const
{
    if( mbIsBiff8 )
    {
        PrepareWrite( rStrm, 1 );
        rStrm << GetFlagField();
        rStrm.SetSliceSize( 0 );
    }
}

void XclExpString::WriteHeader( XclExpStream& rStrm ) const
{
    DBG_ASSERT( !mb8BitLen || (mnLen < 256), "XclExpString::WriteHeader - string too long" );
    PrepareWrite( rStrm, GetHeaderSize() );
    // length
    WriteLenField( rStrm );
    // flag field
    if( IsWriteFlags() )
        rStrm << GetFlagField();
    // format run count
    if( IsWriteFormats() )
        rStrm << GetFormatsCount();
    rStrm.SetSliceSize( 0 );
}

void XclExpString::WriteBuffer( XclExpStream& rStrm ) const
{
    if( mbIsBiff8 )
        rStrm.WriteUnicodeBuffer( maUniBuffer, GetFlagField() );
    else
        rStrm.WriteCharBuffer( maCharBuffer );
}

void XclExpString::WriteFormats( XclExpStream& rStrm ) const
{
    if( IsRich() )
    {
        XclFormatRunVec::const_iterator aIt = maFormats.begin(), aEnd = maFormats.end();
        if( mbIsBiff8 )
        {
            rStrm.SetSliceSize( 4 );
            for( ; aIt != aEnd; ++aIt )
                rStrm << aIt->mnChar << aIt->mnFontIdx;
        }
        else
        {
            rStrm.SetSliceSize( 2 );
            for( ; aIt != aEnd; ++aIt )
                rStrm << static_cast< sal_uInt8 >( aIt->mnChar ) << static_cast< sal_uInt8 >( aIt->mnFontIdx );
        }
        rStrm.SetSliceSize( 0 );
    }
}

void XclExpString::Write( XclExpStream& rStrm ) const
{
    WriteHeader( rStrm );
    WriteBuffer( rStrm );
    if( IsWriteFormats() )      // only in BIFF8 included in string
        WriteFormats( rStrm );
}

void XclExpString::WriteHeaderToMem( sal_uInt8* pnMem ) const
{
    DBG_ASSERT( pnMem, "XclExpString::WriteHeaderToMem - no memory to write to" );
    DBG_ASSERT( !mb8BitLen || (mnLen < 256), "XclExpString::WriteHeaderToMem - string too long" );
    DBG_ASSERT( !IsWriteFormats(), "XclExpString::WriteHeaderToMem - formatted strings not supported" );
    // length
    if( mb8BitLen )
    {
        *pnMem = static_cast< sal_uInt8 >( mnLen );
        ++pnMem;
    }
    else
    {
        ShortToSVBT16( mnLen, pnMem );
        pnMem += 2;
    }
    // flag field
    if( IsWriteFlags() )
        *pnMem = GetFlagField();
}

void XclExpString::WriteBufferToMem( sal_uInt8* pnMem ) const
{
    DBG_ASSERT( pnMem, "XclExpString::WriteBufferToMem - no memory to write to" );
    if( !IsEmpty() )
    {
        if( mbIsBiff8 )
        {
            for( ScfUInt16Vec::const_iterator aIt = maUniBuffer.begin(), aEnd = maUniBuffer.end(); aIt != aEnd; ++aIt )
            {
                sal_uInt16 nChar = *aIt;
                *pnMem = static_cast< sal_uInt8 >( nChar );
                ++pnMem;
                if( mbIsUnicode )
                {
                    *pnMem = static_cast< sal_uInt8 >( nChar >> 8 );
                    ++pnMem;
                }
            }
        }
        else
            memcpy( pnMem, &maCharBuffer[ 0 ], mnLen );
    }
}

void XclExpString::WriteToMem( sal_uInt8* pnMem ) const
{
    WriteHeaderToMem( pnMem );
    WriteBufferToMem( pnMem + GetHeaderSize() );
}

// ----------------------------------------------------------------------------

bool XclExpString::IsWriteFlags() const
{
    return mbIsBiff8 && (!IsEmpty() || !mbSmartFlags);
}

bool XclExpString::IsWriteFormats() const
{
    return mbIsBiff8 && IsRich();
}

void XclExpString::SetStrLen( sal_Int32 nNewLen )
{
    sal_uInt16 nAllowedLen = (mb8BitLen && (mnMaxLen > 255)) ? 255 : mnMaxLen;
    mnLen = limit_cast< sal_uInt16 >( nNewLen, 0, nAllowedLen );
}

void XclExpString::CharsToBuffer( const sal_Unicode* pcSource, sal_Int32 nBegin, sal_Int32 nLen )
{
    DBG_ASSERT( maUniBuffer.size() >= static_cast< size_t >( nBegin + nLen ),
        "XclExpString::CharsToBuffer - char buffer invalid" );
    ScfUInt16Vec::iterator aBeg = maUniBuffer.begin() + nBegin;
    ScfUInt16Vec::iterator aEnd = aBeg + nLen;
    const sal_Unicode* pcSrcChar = pcSource;
    for( ScfUInt16Vec::iterator aIt = aBeg; aIt != aEnd; ++aIt, ++pcSrcChar )
    {
        *aIt = static_cast< sal_uInt16 >( *pcSrcChar );
        if( *aIt & 0xFF00 )
            mbIsUnicode = true;
    }
    if( !mbWrapped )
        mbWrapped = ::std::find( aBeg, aEnd, EXC_LF ) != aEnd;
}

void XclExpString::CharsToBuffer( const sal_Char* pcSource, sal_Int32 nBegin, sal_Int32 nLen )
{
    DBG_ASSERT( maCharBuffer.size() >= static_cast< size_t >( nBegin + nLen ),
        "XclExpString::CharsToBuffer - char buffer invalid" );
    ScfUInt8Vec::iterator aBeg = maCharBuffer.begin() + nBegin;
    ScfUInt8Vec::iterator aEnd = aBeg + nLen;
    const sal_Char* pcSrcChar = pcSource;
    for( ScfUInt8Vec::iterator aIt = aBeg; aIt != aEnd; ++aIt, ++pcSrcChar )
        *aIt = static_cast< sal_uInt8 >( *pcSrcChar );
    mbIsUnicode = false;
    if( !mbWrapped )
        mbWrapped = ::std::find( aBeg, aEnd, EXC_LF_C ) != aEnd;
}

void XclExpString::Init( sal_Int32 nCurrLen, XclStrFlags nFlags, sal_uInt16 nMaxLen, bool bBiff8 )
{
    mbIsBiff8 = bBiff8;
    mbIsUnicode = bBiff8 && ::get_flag( nFlags, EXC_STR_FORCEUNICODE );
    mb8BitLen = ::get_flag( nFlags, EXC_STR_8BITLENGTH );
    mbSmartFlags = bBiff8 && ::get_flag( nFlags, EXC_STR_SMARTFLAGS );
    mbWrapped = false;
    mnMaxLen = nMaxLen;
    SetStrLen( nCurrLen );

    maFormats.clear();
    if( mbIsBiff8 )
    {
        maCharBuffer.clear();
        maUniBuffer.resize( mnLen );
    }
    else
    {
        maUniBuffer.clear();
        maCharBuffer.resize( mnLen );
    }
}

void XclExpString::Build( const sal_Unicode* pcSource, sal_Int32 nCurrLen, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Init( nCurrLen, nFlags, nMaxLen, true );
    CharsToBuffer( pcSource, 0, mnLen );
}

void XclExpString::Build( const sal_Char* pcSource, sal_Int32 nCurrLen, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Init( nCurrLen, nFlags, nMaxLen, false );
    CharsToBuffer( pcSource, 0, mnLen );
}

void XclExpString::InitAppend( sal_Int32 nAddLen )
{
    SetStrLen( static_cast< sal_Int32 >( mnLen ) + nAddLen );
    if( mbIsBiff8 )
        maUniBuffer.resize( mnLen );
    else
        maCharBuffer.resize( mnLen );
}

void XclExpString::BuildAppend( const sal_Unicode* pcSource, sal_Int32 nAddLen )
{
    DBG_ASSERT( mbIsBiff8, "XclExpString::BuildAppend - must not be called at byte strings" );
    if( mbIsBiff8 )
    {
        sal_uInt16 nOldLen = mnLen;
        InitAppend( nAddLen );
        CharsToBuffer( pcSource, nOldLen, mnLen - nOldLen );
    }
}

void XclExpString::BuildAppend( const sal_Char* pcSource, sal_Int32 nAddLen )
{
    DBG_ASSERT( !mbIsBiff8, "XclExpString::BuildAppend - must not be called at unicode strings" );
    if( !mbIsBiff8 )
    {
        sal_uInt16 nOldLen = mnLen;
        InitAppend( nAddLen );
        CharsToBuffer( pcSource, nOldLen, mnLen - nOldLen );
    }
}

void XclExpString::PrepareWrite( XclExpStream& rStrm, sal_uInt16 nBytes ) const
{
    rStrm.SetSliceSize( nBytes + (mbIsUnicode ? 2 : 1) );
}

// ============================================================================

