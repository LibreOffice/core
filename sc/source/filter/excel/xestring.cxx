/*************************************************************************
 *
 *  $RCSfile: xestring.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:34:52 $
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

// ============================================================================

#ifndef SC_XESTRING_HXX
#include "xestring.hxx"
#endif

#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif
#ifndef SC_XESTREAM_HXX
#include "xestream.hxx"
#endif


using ::rtl::OUString;


// ============================================================================

/** All allowed flags for exporting a BIFF2-BIFF7 byte string. */
const XclStrFlags nAllowedFlags27 = EXC_STR_8BITLENGTH;
/** All allowed flags for export. */
const XclStrFlags nAllowedFlags = nAllowedFlags27 | EXC_STR_FORCEUNICODE | EXC_STR_SMARTFLAGS;


// ----------------------------------------------------------------------------

namespace {

// compare vectors

/** Returns -1, if rLeft<rRight; or 1, if rLeft>rRight; or 0, if rLeft==rRight. */
template< typename Type >
inline int lclCompare( const Type& rLeft, const Type& rRight )
{
    int nResult = 0;
    if( rLeft < rRight )
        nResult = -1;
    else if( rRight < rLeft )
        nResult = 1;
    return nResult;
}

/** Compares vectors. Returns -1, if rLeft<rRight; or 1, if rLeft>rRight; or 0, if rLeft==rRight. */
template< typename Type >
int lclCompareVectors( const ::std::vector< Type >& rLeft, const ::std::vector< Type >& rRight )
{
    int nResult = 0;

    // 1st: compare all elements of the vectors
    typedef ::std::vector< Type >::const_iterator CIT;
    CIT aEndL = rLeft.end(), aEndR = rRight.end();
    for( CIT aItL = rLeft.begin(), aItR = rRight.begin(); !nResult && (aItL != aEndL) && (aItR != aEndR); ++aItL, ++aItR )
        nResult = lclCompare( *aItL, *aItR );

    // 2nd: no differences found so far -> compare the vector sizes. Shorter vector is less
    if( !nResult )
        nResult = lclCompare( rLeft.size(), rRight.size() );

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
    inline sal_uInt32           operator()( Type nVal ) const { return nVal; }
};

struct XclFormatRunHasher : public XclHasher< const XclFormatRun& >
{
    inline sal_uInt32           operator()( const XclFormatRun& rRun ) const
                                    { return (rRun.mnChar << 8) ^ rRun.mnFontIx; }
};

/** Calculates a hash value from a vector.
    @descr Uses the passed hasher function object to calculate hash values from
    all vector elements. */
template< typename Type, typename ValueHasher >
sal_uInt16 lclHashVector( const ::std::vector< Type >& rVec, const ValueHasher& rHasher )
{
    sal_uInt32 nHash = rVec.size();
    for( ::std::vector< Type >::const_iterator aIt = rVec.begin(), aEnd = rVec.end(); aIt != aEnd; ++aIt )
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
    LimitFormatCount( mbIsBiff8 ? 0xFFFF : 0xFF );
}

void XclExpString::AppendFormat( sal_uInt16 nChar, sal_uInt16 nFontIx )
{
    DBG_ASSERT( maFormats.empty() || (maFormats.back().mnChar < nChar), "XclExpString::AppendFormat - invalid char index" );
    if( maFormats.size() < static_cast< sal_uInt32 >( mbIsBiff8 ? 0xFFFF : 0xFF ) )
        maFormats.push_back( XclFormatRun( nChar, nFontIx ) );
}

void XclExpString::LimitFormatCount( sal_uInt16 nMaxCount )
{
    if( maFormats.size() > nMaxCount )
        maFormats.erase( maFormats.begin() + nMaxCount, maFormats.end() );
}

sal_uInt16 XclExpString::RemoveFontOfChar(sal_uInt16 nCharIx)
{
    XclFormatRunVec::iterator aStart = maFormats.begin();
    XclFormatRunVec::const_iterator aEnd   = maFormats.end();
    sal_uInt16 nFontIx = EXC_FONT_NOTFOUND;
    for( aStart; aStart != aEnd; ++aStart)
    {
        if( aStart->mnChar == nCharIx )
        {
            nFontIx = aStart->mnFontIx ;
            maFormats.erase(aStart);
            break;
        }
    }
    return nFontIx;
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
    return nResult ? (nResult < 0) : (maFormats < rCmp.maFormats);
}


// get data -------------------------------------------------------------------

sal_uInt16 XclExpString::GetFormatsCount() const
{
    return static_cast< sal_uInt16 >( mbIsBiff8 ? maFormats.size() : 0 );
}

sal_uInt8 XclExpString::GetFlagField() const
{
    return (mbIsUnicode ? EXC_STRF_16BIT : 0) | (IsRich() ? EXC_STRF_RICH : 0);
}

sal_uInt32 XclExpString::GetBufferSize() const
{
    return mnLen * (mbIsUnicode ? 2 : 1);
}

sal_uInt32 XclExpString::GetSize() const
{
    return
        (mb8BitLen ? 1 : 2) +                                   // length field
        (IsWriteFlags() ? 1 : 0) +                              // flag field
        GetBufferSize() +                                       // character buffer
        (IsWriteFormats() ? (4 * GetFormatsCount() + 2) : 0);   // richtext formattting
}

sal_uInt16 XclExpString::GetHash() const
{
    return
        (mbIsBiff8 ? lclHashVector( maUniBuffer ) : lclHashVector( maCharBuffer )) ^
        lclHashVector( maFormats, XclFormatRunHasher() );
}


// streaming ------------------------------------------------------------------

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

    bool bWriteFlags = IsWriteFlags();
    bool bWriteFormats = IsWriteFormats();
    PrepareWrite( rStrm, (mb8BitLen ? 1 : 2) + (bWriteFlags ? 1 : 0) + (bWriteFormats ? 2 : 0) );

    // length
    if( mb8BitLen )
        rStrm << static_cast< sal_uInt8 >( mnLen );
    else
        rStrm << mnLen;
    // flag field
    if( bWriteFlags )
        rStrm << GetFlagField();
    // format run count
    if( bWriteFormats )
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
                rStrm << aIt->mnChar << aIt->mnFontIx;
        }
        else
        {
            rStrm.SetSliceSize( 2 );
            for( ; aIt != aEnd; ++aIt )
                rStrm << static_cast< sal_uInt8 >( aIt->mnChar ) << static_cast< sal_uInt8 >( aIt->mnFontIx );
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

void XclExpString::WriteBuffer( void* pDest ) const
{
    if( pDest && !IsEmpty() )
    {
        if( mbIsBiff8 )
        {
            sal_uInt8* pDest8 = reinterpret_cast< sal_uInt8* >( pDest );
            for( ScfUInt16Vec::const_iterator aIter = maUniBuffer.begin(), aEnd = maUniBuffer.end(); aIter != aEnd; ++aIter )
            {
                sal_uInt16 nChar = *aIter;
                *pDest8 = static_cast< sal_uInt8 >( nChar );
                ++pDest8;
                if( mbIsUnicode )
                {
                    *pDest8 = static_cast< sal_uInt8 >( nChar >> 8 );
                    ++pDest8;
                }
            }
        }
        else
            memcpy( pDest, &maCharBuffer[ 0 ], mnLen );
    }
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
    sal_Int32 nAllowedLen = (mb8BitLen && (mnMaxLen > 255)) ? 255 : mnMaxLen;
    mnLen = static_cast< sal_uInt16 >( ::std::min( ::std::max( nNewLen, 0L ), nAllowedLen ) );
}

void XclExpString::CharsToBuffer( const sal_Unicode* pcSource, sal_Int32 nBegin, sal_Int32 nLen )
{
    DBG_ASSERT( maUniBuffer.size() >= static_cast< size_t >( nBegin + nLen ),
        "XclExpString::CharsToBuffer - char buffer invalid" );
    ScfUInt16Vec::iterator aBegin = maUniBuffer.begin() + nBegin;
    ScfUInt16Vec::iterator aEnd = aBegin + nLen;
    const sal_Unicode* pcSrcChar = pcSource;
    for( ScfUInt16Vec::iterator aIter = aBegin; aIter != aEnd; ++aIter, ++pcSrcChar )
    {
        *aIter = static_cast< sal_uInt16 >( *pcSrcChar );
        if( *aIter & 0xFF00 )
            mbIsUnicode = true;
    }
    mbWrapped = ::std::find( aBegin, aEnd, EXC_LF ) != aEnd;
}

void XclExpString::CharsToBuffer( const sal_Char* pcSource, sal_Int32 nBegin, sal_Int32 nLen )
{
    DBG_ASSERT( maCharBuffer.size() >= static_cast< size_t >( nBegin + nLen ),
        "XclExpString::CharsToBuffer - char buffer invalid" );
    ScfUInt8Vec::iterator aBegin = maCharBuffer.begin() + nBegin;
    ScfUInt8Vec::iterator aEnd = aBegin + nLen;
    const sal_Char* pcSrcChar = pcSource;
    for( ScfUInt8Vec::iterator aIter = aBegin; aIter != aEnd; ++aIter, ++pcSrcChar )
        *aIter = static_cast< sal_uInt8 >( *pcSrcChar );
    mbIsUnicode = false;
    mbWrapped = ::std::find( aBegin, aEnd, EXC_LF_C ) != aEnd;
}

void XclExpString::Init( sal_Int32 nCurrLen, XclStrFlags nFlags, sal_uInt16 nMaxLen, bool bBiff8 )
{
    DBG_ASSERT( (nFlags & ~(bBiff8 ? nAllowedFlags : nAllowedFlags27)) == 0, "XclExpString::Init - unknown flag" );
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

void XclExpString::PrepareWrite( XclExpStream& rStrm, sal_uInt32 nBytes ) const
{
    rStrm.SetSliceSize( nBytes + (mbIsUnicode ? 2 : 1) );
}


// ============================================================================

