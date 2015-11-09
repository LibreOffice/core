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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <rtl/strbuf.hxx>

#include <tools/stream.hxx>
#include <tools/globname.hxx>

// ImpSvGlobalName ------------------------------------------------------------
ImpSvGlobalName::ImpSvGlobalName()
{
    memset( &szData, 0, sizeof( szData ) );
}

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
SvGlobalName::SvGlobalName() :
    pImp()
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
    SvGUID aResult;
    memset( &aResult, 0, sizeof( aResult ) );
    if ( aSeq.getLength() == 16 )
    {
        aResult.Data1 = ( ( ( ( ( ( sal_uInt8 )aSeq[0] << 8 ) + ( sal_uInt8 )aSeq[1] ) << 8 ) + ( sal_uInt8 )aSeq[2] ) << 8 ) + ( sal_uInt8 )aSeq[3];
        aResult.Data2 = ( ( sal_uInt8 )aSeq[4] << 8 ) + ( sal_uInt8 )aSeq[5];
        aResult.Data3 = ( ( sal_uInt8 )aSeq[6] << 8 ) + ( sal_uInt8 )aSeq[7];
        for( int nInd = 0; nInd < 8; nInd++ )
            aResult.Data4[nInd] = ( sal_uInt8 )aSeq[nInd+8];
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

SvStream& WriteSvGlobalName( SvStream& rOStr, const SvGlobalName & rObj )
{
    rOStr.WriteUInt32( rObj.pImp->szData.Data1 );
    rOStr.WriteUInt16( rObj.pImp->szData.Data2 );
    rOStr.WriteUInt16( rObj.pImp->szData.Data3 );
    rOStr.Write( &rObj.pImp->szData.Data4, 8 );
    return rOStr;
}

SvStream& operator >> ( SvStream& rStr, SvGlobalName & rObj )
{
    // the non-const dereferencing operator
    // ensures pImp is unique
    rStr.ReadUInt32( rObj.pImp->szData.Data1 );
    rStr.ReadUInt16( rObj.pImp->szData.Data2 );
    rStr.ReadUInt16( rObj.pImp->szData.Data3 );
    rStr.Read( &rObj.pImp->szData.Data4, 8 );
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

SvGlobalName & SvGlobalName::operator += ( sal_uInt32 n )
{
    sal_uInt32 nOld = pImp->szData.Data1;
    pImp->szData.Data1 += n;

    if( nOld > pImp->szData.Data1 )
    {
        // overflow
        pImp->szData.Data2++;
    }
    return *this;
}

bool SvGlobalName::operator == ( const SvGlobalName & rObj ) const
{
    return pImp == rObj.pImp;
}

void SvGlobalName::MakeFromMemory( void * pData )
{
    memcpy( &pImp->szData, pData, sizeof( pImp->szData ) );
}

bool SvGlobalName::MakeId( const OUString & rIdStr )
{
    OString aStr(OUStringToOString(rIdStr,
        RTL_TEXTENCODING_ASCII_US));
    const sal_Char *pStr = aStr.getStr();
    if( rIdStr.getLength() == 36
      && '-' == pStr[ 8 ]  && '-' == pStr[ 13 ]
      && '-' == pStr[ 18 ] && '-' == pStr[ 23 ] )
    {
        sal_uInt32 nFirst = 0;
        int i = 0;
        for( i = 0; i < 8; i++ )
        {
            if( isxdigit( *pStr ) )
                if( isdigit( *pStr ) )
                    nFirst = nFirst * 16 + (*pStr - '0');
                else
                    nFirst = nFirst * 16 + (toupper( *pStr ) - 'A' + 10 );
            else
                return false;
            pStr++;
        }

        sal_uInt16 nSec = 0;
        pStr++;
        for( i = 0; i < 4; i++ )
        {
            if( isxdigit( *pStr ) )
                if( isdigit( *pStr ) )
                    nSec = nSec * 16 + (*pStr - '0');
                else
                    nSec = nSec * 16 + (sal_uInt16)(toupper( *pStr ) - 'A' + 10 );
            else
                return false;
            pStr++;
        }

        sal_uInt16 nThird = 0;
        pStr++;
        for( i = 0; i < 4; i++ )
        {
            if( isxdigit( *pStr ) )
                if( isdigit( *pStr ) )
                    nThird = nThird * 16 + (*pStr - '0');
                else
                    nThird = nThird * 16 + (sal_uInt16)(toupper( *pStr ) - 'A' + 10 );
            else
                return false;
            pStr++;
        }

        sal_Int8 szRemain[ 8 ];
        memset( szRemain, 0, sizeof( szRemain ) );
        pStr++;
        for( i = 0; i < 16; i++ )
        {
            if( isxdigit( *pStr ) )
                if( isdigit( *pStr ) )
                    szRemain[i/2] = szRemain[i/2] * 16 + (*pStr - '0');
                else
                    szRemain[i/2] = szRemain[i/2] * 16 + (sal_Int8)(toupper( *pStr ) - 'A' + 10 );
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
    OStringBuffer aHexBuffer;

    sal_Char buf[ 10 ];
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
    css::uno::Sequence< sal_Int8 > aResult( 16 );

    aResult[ 0] = (sal_Int8) (pImp->szData.Data1 >> 24);
    aResult[ 1] = (sal_Int8) ((pImp->szData.Data1 << 8 ) >> 24);
    aResult[ 2] = (sal_Int8) ((pImp->szData.Data1 << 16 ) >> 24);
    aResult[ 3] = (sal_Int8) ((pImp->szData.Data1 << 24 ) >> 24);
    aResult[ 4] = (sal_Int8) (pImp->szData.Data2 >> 8);
    aResult[ 5] = (sal_Int8) ((pImp->szData.Data2 << 8 ) >> 8);
    aResult[ 6] = (sal_Int8) (pImp->szData.Data3 >> 8);
    aResult[ 7] = (sal_Int8) ((pImp->szData.Data3 << 8 ) >> 8);
    aResult[ 8] = pImp->szData.Data4[ 0 ];
    aResult[ 9] = pImp->szData.Data4[ 1 ];
    aResult[10] = pImp->szData.Data4[ 2 ];
    aResult[11] = pImp->szData.Data4[ 3 ];
    aResult[12] = pImp->szData.Data4[ 4 ];
    aResult[13] = pImp->szData.Data4[ 5 ];
    aResult[14] = pImp->szData.Data4[ 6 ];
    aResult[15] = pImp->szData.Data4[ 7 ];

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
