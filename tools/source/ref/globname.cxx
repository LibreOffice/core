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

#include <rtl/strbuf.hxx>
#include <rtl/character.hxx>

#include <tools/stream.hxx>
#include <tools/globname.hxx>

// ImpSvGlobalName ------------------------------------------------------------
ImpSvGlobalName::ImpSvGlobalName( const ImpSvGlobalName & rObj )
    : szData(rObj.szData)
{
}

ImpSvGlobalName::ImpSvGlobalName(sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                          sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                          sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15)
{
    szData.Data1 = n1;
    szData.Data2 = n2;
    szData.Data3 = n3;
    szData.Data4[0] = b8;
    szData.Data4[1] = b9;
    szData.Data4[2] = b10;
    szData.Data4[3] = b11;
    szData.Data4[4] = b12;
    szData.Data4[5] = b13;
    szData.Data4[6] = b14;
    szData.Data4[7] = b15;
}

bool ImpSvGlobalName::operator == ( const ImpSvGlobalName & rObj ) const
{
    return !memcmp( &szData, &rObj.szData, sizeof( szData ) );
}

// SvGlobalName ----------------------------------------------------------------
SvGlobalName::SvGlobalName()
{
}

SvGlobalName::SvGlobalName( const SvGUID & rId ) :
    pImp( ImpSvGlobalName( rId ) )
{
}

SvGlobalName::SvGlobalName( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                            sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                            sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 ) :
    pImp( ImpSvGlobalName(n1, n2, n3, b8, b9, b10, b11, b12, b13, b14, b15) )
{
}

SvGlobalName::SvGlobalName( const css::uno::Sequence < sal_Int8 >& aSeq )
{
    // create SvGlobalName from a platform independent representation
    SvGUID aResult = {};
    if ( aSeq.getLength() == 16 )
    {
        aResult.Data1 = ( ( ( ( ( static_cast<sal_uInt8>(aSeq[0]) << 8 ) + static_cast<sal_uInt8>(aSeq[1]) ) << 8 ) + static_cast<sal_uInt8>(aSeq[2]) ) << 8 ) + static_cast<sal_uInt8>(aSeq[3]);
        aResult.Data2 = ( static_cast<sal_uInt8>(aSeq[4]) << 8 ) + static_cast<sal_uInt8>(aSeq[5]);
        aResult.Data3 = ( static_cast<sal_uInt8>(aSeq[6]) << 8 ) + static_cast<sal_uInt8>(aSeq[7]);
        for( int nInd = 0; nInd < 8; nInd++ )
            aResult.Data4[nInd] = static_cast<sal_uInt8>(aSeq[nInd+8]);
    }

    pImp = ::o3tl::cow_wrapper< ImpSvGlobalName >(aResult);
}

SvGlobalName::~SvGlobalName()
{
}

SvGlobalName & SvGlobalName::operator = ( const SvGlobalName & rObj )
{
    pImp = rObj.pImp;

    return *this;
}

SvGlobalName & SvGlobalName::operator = ( SvGlobalName && rObj ) noexcept
{
    pImp = std::move(rObj.pImp);
    return *this;
}

SvStream& WriteSvGlobalName( SvStream& rOStr, const SvGlobalName & rObj )
{
    rOStr.WriteUInt32( rObj.pImp->szData.Data1 );
    rOStr.WriteUInt16( rObj.pImp->szData.Data2 );
    rOStr.WriteUInt16( rObj.pImp->szData.Data3 );
    rOStr.WriteBytes( &rObj.pImp->szData.Data4, 8 );
    return rOStr;
}

SvStream& operator >> ( SvStream& rStr, SvGlobalName & rObj )
{
    // the non-const dereferencing operator
    // ensures pImp is unique
    rStr.ReadUInt32( rObj.pImp->szData.Data1 );
    rStr.ReadUInt16( rObj.pImp->szData.Data2 );
    rStr.ReadUInt16( rObj.pImp->szData.Data3 );
    rStr.ReadBytes( &rObj.pImp->szData.Data4, 8 );
    return rStr;
}


bool SvGlobalName::operator < ( const SvGlobalName & rObj ) const
{
    if( pImp->szData.Data3 < rObj.pImp->szData.Data3 )
        return true;
    else if( pImp->szData.Data3 > rObj.pImp->szData.Data3 )
        return false;

    if( pImp->szData.Data2 < rObj.pImp->szData.Data2 )
        return true;
    else if( pImp->szData.Data2 > rObj.pImp->szData.Data2 )
        return false;

    return pImp->szData.Data1 < rObj.pImp->szData.Data1;
}

bool SvGlobalName::operator == ( const SvGlobalName & rObj ) const
{
    return pImp == rObj.pImp;
}

void SvGlobalName::MakeFromMemory( void const * pData )
{
    memcpy( &pImp->szData, pData, sizeof( pImp->szData ) );
}

bool SvGlobalName::MakeId( const OUString & rIdStr )
{
    const sal_Unicode *pStr = rIdStr.getStr();
    if( rIdStr.getLength() == 36
      && '-' == pStr[ 8 ]  && '-' == pStr[ 13 ]
      && '-' == pStr[ 18 ] && '-' == pStr[ 23 ] )
    {
        sal_uInt32 nFirst = 0;
        int i = 0;
        for( i = 0; i < 8; i++ )
        {
            if( rtl::isAsciiHexDigit( *pStr ) )
                if( rtl::isAsciiDigit( *pStr ) )
                    nFirst = nFirst * 16 + (*pStr - '0');
                else
                    nFirst = nFirst * 16 + (rtl::toAsciiUpperCase( *pStr ) - 'A' + 10 );
            else
                return false;
            pStr++;
        }

        sal_uInt16 nSec = 0;
        pStr++;
        for( i = 0; i < 4; i++ )
        {
            if( rtl::isAsciiHexDigit( *pStr ) )
                if( rtl::isAsciiDigit( *pStr ) )
                    nSec = nSec * 16 + (*pStr - '0');
                else
                    nSec = nSec * 16 + static_cast<sal_uInt16>(rtl::toAsciiUpperCase( *pStr ) - 'A' + 10 );
            else
                return false;
            pStr++;
        }

        sal_uInt16 nThird = 0;
        pStr++;
        for( i = 0; i < 4; i++ )
        {
            if( rtl::isAsciiHexDigit( *pStr ) )
                if( rtl::isAsciiDigit( *pStr ) )
                    nThird = nThird * 16 + (*pStr - '0');
                else
                    nThird = nThird * 16 + static_cast<sal_uInt16>(rtl::toAsciiUpperCase( *pStr ) - 'A' + 10 );
            else
                return false;
            pStr++;
        }

        sal_Int8 szRemain[ 8 ] = {};
        pStr++;
        for( i = 0; i < 16; i++ )
        {
            if( rtl::isAsciiHexDigit( *pStr ) )
                if( rtl::isAsciiDigit( *pStr ) )
                    szRemain[i/2] = szRemain[i/2] * 16 + (*pStr - '0');
                else
                    szRemain[i/2] = szRemain[i/2] * 16 + static_cast<sal_Int8>(rtl::toAsciiUpperCase( *pStr ) - 'A' + 10 );
            else
                return false;
            pStr++;
            if( i == 3 )
                pStr++;
        }

        memcpy(&pImp->szData.Data1, &nFirst, sizeof(nFirst));
        memcpy(&pImp->szData.Data2, &nSec, sizeof(nSec));
        memcpy(&pImp->szData.Data3, &nThird, sizeof(nThird));
        memcpy(&pImp->szData.Data4, szRemain, 8);
        return true;
    }
    return false;
}

OUString SvGlobalName::GetHexName() const
{
    OStringBuffer aHexBuffer(36);

    char buf[ 10 ];
    sprintf( buf, "%8.8" SAL_PRIXUINT32, pImp->szData.Data1 );
    aHexBuffer.append(buf);
    aHexBuffer.append('-');
    sprintf( buf, "%4.4X", pImp->szData.Data2 );
    aHexBuffer.append(buf);
    aHexBuffer.append('-');
    sprintf( buf, "%4.4X", pImp->szData.Data3 );
    aHexBuffer.append(buf);
    aHexBuffer.append('-');
    for( int i = 0; i < 2; i++ )
    {
        sprintf( buf, "%2.2x", pImp->szData.Data4[ i ] );
        aHexBuffer.append(buf);
    }
    aHexBuffer.append('-');
    for( int i = 2; i < 8; i++ )
    {
        sprintf( buf, "%2.2x", pImp->szData.Data4[ i ] );
        aHexBuffer.append(buf);
    }
    return OStringToOUString(aHexBuffer.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
}

css::uno::Sequence < sal_Int8 > SvGlobalName::GetByteSequence() const
{
    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    css::uno::Sequence< sal_Int8 > aResult(
        { /* [ 0] */ static_cast<sal_Int8>(pImp->szData.Data1 >> 24),
          /* [ 1] */ static_cast<sal_Int8>((pImp->szData.Data1 << 8 ) >> 24),
          /* [ 2] */ static_cast<sal_Int8>((pImp->szData.Data1 << 16 ) >> 24),
          /* [ 3] */ static_cast<sal_Int8>((pImp->szData.Data1 << 24 ) >> 24),
          /* [ 4] */ static_cast<sal_Int8>(pImp->szData.Data2 >> 8),
          /* [ 5] */ static_cast<sal_Int8>((pImp->szData.Data2 << 8 ) >> 8),
          /* [ 6] */ static_cast<sal_Int8>(pImp->szData.Data3 >> 8),
          /* [ 7] */ static_cast<sal_Int8>((pImp->szData.Data3 << 8 ) >> 8),
          /* [ 8] */ static_cast<sal_Int8>(pImp->szData.Data4[ 0 ]),
          /* [ 9] */ static_cast<sal_Int8>(pImp->szData.Data4[ 1 ]),
          /* [10] */ static_cast<sal_Int8>(pImp->szData.Data4[ 2 ]),
          /* [11] */ static_cast<sal_Int8>(pImp->szData.Data4[ 3 ]),
          /* [12] */ static_cast<sal_Int8>(pImp->szData.Data4[ 4 ]),
          /* [13] */ static_cast<sal_Int8>(pImp->szData.Data4[ 5 ]),
          /* [14] */ static_cast<sal_Int8>(pImp->szData.Data4[ 6 ]),
          /* [15] */ static_cast<sal_Int8>(pImp->szData.Data4[ 7 ]) });

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
