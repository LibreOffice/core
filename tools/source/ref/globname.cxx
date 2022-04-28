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

#include <stdio.h>
#include <string.h>

#include <comphelper/mimeconfighelper.hxx>
#include <rtl/character.hxx>

#include <tools/stream.hxx>
#include <tools/globname.hxx>

// SvGlobalName ----------------------------------------------------------------
SvGlobalName::SvGlobalName( const SvGUID & rId ) :
    m_aData( rId )
{
}

SvGlobalName::SvGlobalName( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                            sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                            sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 ) :
    m_aData{ n1, n2, n3, { b8, b9, b10, b11, b12, b13, b14, b15 } }
{
}

SvGlobalName::SvGlobalName( const css::uno::Sequence < sal_Int8 >& aSeq )
{
    // create SvGlobalName from a platform independent representation
    if ( aSeq.getLength() == 16 )
    {
        m_aData.Data1 = ( ( ( ( ( static_cast<sal_uInt8>(aSeq[0]) << 8 ) + static_cast<sal_uInt8>(aSeq[1]) ) << 8 ) + static_cast<sal_uInt8>(aSeq[2]) ) << 8 ) + static_cast<sal_uInt8>(aSeq[3]);
        m_aData.Data2 = ( static_cast<sal_uInt8>(aSeq[4]) << 8 ) + static_cast<sal_uInt8>(aSeq[5]);
        m_aData.Data3 = ( static_cast<sal_uInt8>(aSeq[6]) << 8 ) + static_cast<sal_uInt8>(aSeq[7]);
        for( int nInd = 0; nInd < 8; nInd++ )
            m_aData.Data4[nInd] = static_cast<sal_uInt8>(aSeq[nInd+8]);
    }
}

SvStream& WriteSvGlobalName( SvStream& rOStr, const SvGlobalName & rObj )
{
    rOStr.WriteUInt32( rObj.m_aData.Data1 );
    rOStr.WriteUInt16( rObj.m_aData.Data2 );
    rOStr.WriteUInt16( rObj.m_aData.Data3 );
    rOStr.WriteBytes( &rObj.m_aData.Data4, 8 );
    return rOStr;
}

SvStream& operator >> ( SvStream& rStr, SvGlobalName & rObj )
{
    rStr.ReadUInt32( rObj.m_aData.Data1 );
    rStr.ReadUInt16( rObj.m_aData.Data2 );
    rStr.ReadUInt16( rObj.m_aData.Data3 );
    rStr.ReadBytes( &rObj.m_aData.Data4, 8 );
    return rStr;
}


bool SvGlobalName::operator < ( const SvGlobalName & rObj ) const
{
    if( m_aData.Data3 < rObj.m_aData.Data3 )
        return true;
    else if( m_aData.Data3 > rObj.m_aData.Data3 )
        return false;

    if( m_aData.Data2 < rObj.m_aData.Data2 )
        return true;
    else if( m_aData.Data2 > rObj.m_aData.Data2 )
        return false;

    return m_aData.Data1 < rObj.m_aData.Data1;
}

bool SvGlobalName::operator == ( const SvGlobalName & rObj ) const
{
    return memcmp(&m_aData, &rObj.m_aData, sizeof(m_aData)) == 0;
}

void SvGlobalName::MakeFromMemory( void const * pData )
{
    memcpy( &m_aData, pData, sizeof( m_aData ) );
}

bool SvGlobalName::MakeId( std::u16string_view rIdStr )
{
    const sal_Unicode *pStr = rIdStr.data();
    if( rIdStr.size() != 36
      || '-' != pStr[ 8 ]  || '-' != pStr[ 13 ]
      || '-' != pStr[ 18 ] || '-' != pStr[ 23 ] )
        return false;

    SvGUID aGuid = {};
    auto asciiHexDigitToNumber = [](sal_Unicode c) -> sal_uInt8
    {
        if (rtl::isAsciiDigit(c))
            return c - '0';
        else
            return rtl::toAsciiUpperCase(c) - 'A' + 10;
    };

    for( int i = 0; i < 8; i++ )
    {
        if( !rtl::isAsciiHexDigit( *pStr ) )
            return false;
        aGuid.Data1 = aGuid.Data1 * 16 + asciiHexDigitToNumber( *pStr++ );
    }

    pStr++;
    for( int i = 0; i < 4; i++ )
    {
        if( !rtl::isAsciiHexDigit( *pStr ) )
            return false;
        aGuid.Data2 = aGuid.Data2 * 16 + asciiHexDigitToNumber( *pStr++ );
    }

    pStr++;
    for( int i = 0; i < 4; i++ )
    {
        if( !rtl::isAsciiHexDigit( *pStr ) )
            return false;
        aGuid.Data3 = aGuid.Data3 * 16 + asciiHexDigitToNumber( *pStr++ );
    }

    pStr++;
    for( int i = 0; i < 16; i++ )
    {
        if( !rtl::isAsciiHexDigit( *pStr ) )
            return false;
        aGuid.Data4[i/2] = aGuid.Data4[i/2] * 16 + asciiHexDigitToNumber( *pStr++ );
        if( i == 3 )
            pStr++;
    }

    m_aData = aGuid;
    return true;
}

OUString SvGlobalName::GetHexName() const
{
    char buf[ 37 ];
    int n = sprintf(buf,
                    "%8.8" SAL_PRIXUINT32 "-%4.4X-%4.4X-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",
                    m_aData.Data1, m_aData.Data2, m_aData.Data3,
                    m_aData.Data4[0], m_aData.Data4[1], m_aData.Data4[2], m_aData.Data4[3],
                    m_aData.Data4[4], m_aData.Data4[5], m_aData.Data4[6], m_aData.Data4[7]);
    assert(n == 36);
    return OUString::createFromAscii(std::string_view(buf, n));
}

css::uno::Sequence < sal_Int8 > SvGlobalName::GetByteSequence() const
{
    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    return comphelper::MimeConfigurationHelper::GetSequenceClassID(
        m_aData.Data1, m_aData.Data2, m_aData.Data3,
        m_aData.Data4[0], m_aData.Data4[1], m_aData.Data4[2], m_aData.Data4[3],
        m_aData.Data4[4], m_aData.Data4[5], m_aData.Data4[6], m_aData.Data4[7]);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
