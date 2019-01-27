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

#include <algorithm>
#include <cassert>

#include <osl/diagnose.h>
#include <tools/solar.h>
#include <xlstyle.hxx>
#include <xestyle.hxx>
#include <xestream.hxx>
#include <xestring.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox;

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
    auto [aItL, aItR] = std::mismatch(rLeft.begin(), rLeft.end(), rRight.begin(), rRight.end());
    if ((aItL != rLeft.end()) && (aItR != rRight.end()))
        nResult = static_cast< int >( *aItL ) - static_cast< int >( *aItR );
    else
        // 2nd: compare the vector sizes. Shorter vector is less
        nResult = static_cast< int >( rLeft.size() ) - static_cast< int >( rRight.size() );

    return nResult;
}

// hashing helpers

/** Base class for value hashers.
    @descr  These function objects are used to hash any value to a sal_uInt32 value. */
template< typename Type >
struct XclHasher {};

template< typename Type >
struct XclDirectHasher : public XclHasher< Type >
{
    sal_uInt32   operator()( Type nVal ) const { return nVal; }
};

struct XclFormatRunHasher : public XclHasher< const XclFormatRun& >
{
    sal_uInt32   operator()( const XclFormatRun& rRun ) const
                            { return (rRun.mnChar << 8) ^ rRun.mnFontIdx; }
};

/** Calculates a hash value from a vector.
    @descr Uses the passed hasher function object to calculate hash values from
    all vector elements. */
template< typename Type, typename ValueHasher >
sal_uInt16 lclHashVector( const ::std::vector< Type >& rVec, const ValueHasher& rHasher )
{
    sal_uInt32 nHash = rVec.size();
    for( const auto& rItem : rVec )
        nHash = (nHash * 31) + rHasher( rItem );
    return static_cast< sal_uInt16 >( nHash ^ (nHash >> 16) );
}

/** Calculates a hash value from a vector. Uses XclDirectHasher to hash the vector elements. */
template< typename Type >
sal_uInt16 lclHashVector( const ::std::vector< Type >& rVec )
{
    return lclHashVector( rVec, XclDirectHasher< Type >() );
}

} // namespace

// constructors ---------------------------------------------------------------

XclExpString::XclExpString( XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Init( 0, nFlags, nMaxLen, true );
}

XclExpString::XclExpString( const OUString& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Assign( rString, nFlags, nMaxLen );
}

// assign ---------------------------------------------------------------------

void XclExpString::Assign( const OUString& rString, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    Build( rString.getStr(), rString.getLength(), nFlags, nMaxLen );
}

void XclExpString::Assign( sal_Unicode cChar )
{
    Build( &cChar, 1, XclStrFlags::NONE, EXC_STR_MAXLEN );
}

void XclExpString::AssignByte(
        const OUString& rString, rtl_TextEncoding eTextEnc, XclStrFlags nFlags, sal_uInt16 nMaxLen )
{
    // length may differ from length of rString
    OString aByteStr(OUStringToOString(rString, eTextEnc));
    Build(aByteStr.getStr(), aByteStr.getLength(), nFlags, nMaxLen);
}

// append ---------------------------------------------------------------------

void XclExpString::Append( const OUString& rString )
{
    BuildAppend( rString.getStr(), rString.getLength() );
}

void XclExpString::AppendByte( const OUString& rString, rtl_TextEncoding eTextEnc )
{
    if (!rString.isEmpty())
    {
        // length may differ from length of rString
        OString aByteStr(OUStringToOString(rString, eTextEnc));
        BuildAppend(aByteStr.getStr(), aByteStr.getLength());
    }
}

void XclExpString::AppendByte( sal_Unicode cChar, rtl_TextEncoding eTextEnc )
{
    if( !cChar )
    {
        sal_Char cByteChar = 0;
        BuildAppend( &cByteChar, 1 );
    }
    else
    {
        OString aByteStr( &cChar, 1, eTextEnc );     // length may be >1
        BuildAppend( aByteStr.getStr(), aByteStr.getLength() );
    }
}

// formatting runs ------------------------------------------------------------

void XclExpString::AppendFormat( sal_uInt16 nChar, sal_uInt16 nFontIdx, bool bDropDuplicate )
{
    OSL_ENSURE( maFormats.empty() || (maFormats.back().mnChar < nChar), "XclExpString::AppendFormat - invalid char index" );
    size_t nMaxSize = static_cast< size_t >( mbIsBiff8 ? EXC_STR_MAXLEN : EXC_STR_MAXLEN_8BIT );
    if( maFormats.empty() || ((maFormats.size() < nMaxSize) && (!bDropDuplicate || (maFormats.back().mnFontIdx != nFontIdx))) )
        maFormats.emplace_back( nChar, nFontIdx );
}

void XclExpString::AppendTrailingFormat( sal_uInt16 nFontIdx )
{
    AppendFormat( mnLen, nFontIdx, false );
}

void XclExpString::LimitFormatCount( sal_uInt16 nMaxCount )
{
    if( maFormats.size() > nMaxCount )
        maFormats.erase( maFormats.begin() + nMaxCount, maFormats.end() );
}

sal_uInt16 XclExpString::GetLeadingFont()
{
    sal_uInt16 nFontIdx = EXC_FONT_NOTFOUND;
    if( !maFormats.empty() && (maFormats.front().mnChar == 0) )
    {
        nFontIdx = maFormats.front().mnFontIdx;
    }
    return nFontIdx;
}

sal_uInt16 XclExpString::RemoveLeadingFont()
{
    sal_uInt16 nFontIdx = GetLeadingFont();
    if( nFontIdx != EXC_FONT_NOTFOUND )
    {
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
    return (mbIsUnicode ? EXC_STRF_16BIT : 0) | (IsWriteFormats() ? EXC_STRF_RICH : 0);
}

sal_uInt16 XclExpString::GetHeaderSize() const
{
    return
        (mb8BitLen ? 1 : 2) +           // length field
        (IsWriteFlags() ? 1 : 0) +      // flag field
        (IsWriteFormats() ? 2 : 0);     // richtext formatting count
}

std::size_t XclExpString::GetBufferSize() const
{
    return static_cast<std::size_t>(mnLen) * (mbIsUnicode ? 2 : 1);
}

std::size_t XclExpString::GetSize() const
{
    return
        GetHeaderSize() +                                   // header
        GetBufferSize() +                                   // character buffer
        (IsWriteFormats() ? (4 * GetFormatsCount()) : 0);   // richtext formatting
}

sal_uInt16 XclExpString::GetChar( sal_uInt16 nCharIdx ) const
{
    OSL_ENSURE( nCharIdx < Len(), "XclExpString::GetChar - invalid character index" );
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
    OSL_ENSURE( !mb8BitLen || (mnLen < 256), "XclExpString::WriteHeader - string too long" );
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

void XclExpString::WriteFormats( XclExpStream& rStrm, bool bWriteSize ) const
{
    if( IsRich() )
    {
        if( mbIsBiff8 )
        {
            if( bWriteSize )
                rStrm << GetFormatsCount();
            rStrm.SetSliceSize( 4 );
            for( const auto& rFormat : maFormats )
                rStrm << rFormat.mnChar << rFormat.mnFontIdx;
        }
        else
        {
            if( bWriteSize )
                rStrm << static_cast< sal_uInt8 >( GetFormatsCount() );
            rStrm.SetSliceSize( 2 );
            for( const auto& rFormat : maFormats )
                rStrm << static_cast< sal_uInt8 >( rFormat.mnChar ) << static_cast< sal_uInt8 >( rFormat.mnFontIdx );
        }
        rStrm.SetSliceSize( 0 );
    }
}

void XclExpString::Write( XclExpStream& rStrm ) const
{
    if (!mbSkipHeader)
        WriteHeader( rStrm );
    WriteBuffer( rStrm );
    if( IsWriteFormats() )      // only in BIFF8 included in string
        WriteFormats( rStrm );
}

void XclExpString::WriteHeaderToMem( sal_uInt8* pnMem ) const
{
    assert(pnMem);
    OSL_ENSURE( !mb8BitLen || (mnLen < 256), "XclExpString::WriteHeaderToMem - string too long" );
    OSL_ENSURE( !IsWriteFormats(), "XclExpString::WriteHeaderToMem - formatted strings not supported" );
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
    assert(pnMem);
    if( !IsEmpty() )
    {
        if( mbIsBiff8 )
        {
            for( const sal_uInt16 nChar : maUniBuffer )
            {
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

static sal_uInt16 lcl_WriteRun( XclExpXmlStream& rStrm, const ScfUInt16Vec& rBuffer, sal_uInt16 nStart, sal_Int32 nLength, const XclExpFont* pFont )
{
    if( nLength == 0 )
        return nStart;

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();

    rWorksheet->startElement( XML_r, FSEND );
    if( pFont )
    {
        const XclFontData& rFontData = pFont->GetFontData();
        rWorksheet->startElement( XML_rPr, FSEND );
        XclXmlUtils::WriteFontData( rWorksheet, rFontData, XML_rFont );
        rWorksheet->endElement( XML_rPr );
    }
    rWorksheet->startElement( XML_t,
                FSNS(XML_xml, XML_space), "preserve", FSEND );
    rWorksheet->writeEscaped( XclXmlUtils::ToOUString( rBuffer, nStart, nLength ) );
    rWorksheet->endElement( XML_t );
    rWorksheet->endElement( XML_r );
    return nStart + nLength;
}

void XclExpString::WriteXml( XclExpXmlStream& rStrm ) const
{
    sax_fastparser::FSHelperPtr rWorksheet = rStrm.GetCurrentStream();

    if( !IsWriteFormats() )
    {
        rWorksheet->startElement( XML_t,
                FSNS(XML_xml, XML_space), "preserve", FSEND );
        rWorksheet->writeEscaped( XclXmlUtils::ToOUString( *this ) );
        rWorksheet->endElement( XML_t );
    }
    else
    {
        XclExpFontBuffer& rFonts = rStrm.GetRoot().GetFontBuffer();

        sal_uInt16  nStart = 0;
        const XclExpFont* pFont = nullptr;
        for ( const auto& rFormat : maFormats )
        {
            nStart = lcl_WriteRun( rStrm, GetUnicodeBuffer(),
                    nStart, rFormat.mnChar-nStart, pFont );
            pFont = rFonts.GetFont( rFormat.mnFontIdx );
        }
        lcl_WriteRun( rStrm, GetUnicodeBuffer(),
                nStart, GetUnicodeBuffer().size() - nStart, pFont );
    }
}

bool XclExpString::IsWriteFlags() const
{
    return mbIsBiff8 && (!IsEmpty() || !mbSmartFlags);
}

bool XclExpString::IsWriteFormats() const
{
    return mbIsBiff8 && !mbSkipFormats && IsRich();
}

void XclExpString::SetStrLen( sal_Int32 nNewLen )
{
    sal_uInt16 nAllowedLen = (mb8BitLen && (mnMaxLen > 255)) ? 255 : mnMaxLen;
    mnLen = limit_cast< sal_uInt16 >( nNewLen, 0, nAllowedLen );
}

void XclExpString::CharsToBuffer( const sal_Unicode* pcSource, sal_Int32 nBegin, sal_Int32 nLen )
{
    OSL_ENSURE( maUniBuffer.size() >= static_cast< size_t >( nBegin + nLen ),
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
    OSL_ENSURE( maCharBuffer.size() >= static_cast< size_t >( nBegin + nLen ),
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
    mbIsUnicode = bBiff8 && ( nFlags & XclStrFlags::ForceUnicode );
    mb8BitLen = bool( nFlags & XclStrFlags::EightBitLength );
    mbSmartFlags = bBiff8 && ( nFlags & XclStrFlags::SmartFlags );
    mbSkipFormats = bool( nFlags & XclStrFlags::SeparateFormats );
    mbWrapped = false;
    mbSkipHeader = bool( nFlags & XclStrFlags::NoHeader );
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
    OSL_ENSURE( mbIsBiff8, "XclExpString::BuildAppend - must not be called at byte strings" );
    if( mbIsBiff8 )
    {
        sal_uInt16 nOldLen = mnLen;
        InitAppend( nAddLen );
        CharsToBuffer( pcSource, nOldLen, mnLen - nOldLen );
    }
}

void XclExpString::BuildAppend( const sal_Char* pcSource, sal_Int32 nAddLen )
{
    OSL_ENSURE( !mbIsBiff8, "XclExpString::BuildAppend - must not be called at unicode strings" );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
