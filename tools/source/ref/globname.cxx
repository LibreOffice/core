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

ImpSvGlobalName::ImpSvGlobalName( const ImpSvGlobalName & rObj )
{
    nRefCount = 0;
    memcpy( szData, rObj.szData, sizeof( szData ) );
}

ImpSvGlobalName::ImpSvGlobalName( Empty )
{
    nRefCount = 1;
    memset( szData, 0, sizeof( szData ) );
}

sal_Bool ImpSvGlobalName::operator == ( const ImpSvGlobalName & rObj ) const
{
    return !memcmp( szData, rObj.szData, sizeof( szData ) );
}

// SvGlobalName ----------------------------------------------------------------

SvGlobalName::SvGlobalName()
{
    static ImpSvGlobalName aNoName( ImpSvGlobalName::EMPTY );

    pImp = &aNoName;
    pImp->nRefCount++;
}

#ifdef WNT
struct _GUID
#else
struct GUID
#endif
{
    sal_uInt32 Data1;
    sal_uInt16 Data2;
    sal_uInt16 Data3;
    sal_uInt8  Data4[8];
};
SvGlobalName::SvGlobalName( const CLSID & rId )
{
    pImp = new ImpSvGlobalName();
    pImp->nRefCount++;
    memcpy( pImp->szData, &rId, sizeof( pImp->szData ) );
}

SvGlobalName::SvGlobalName( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                            sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                            sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 )
{
    pImp = new ImpSvGlobalName();
    pImp->nRefCount++;

    memcpy(pImp->szData, &n1, sizeof(n1));
    memcpy(pImp->szData+4, &n2, sizeof(n2));
    memcpy(pImp->szData+6, &n3, sizeof(n3));
    pImp->szData[ 8  ] = b8;
    pImp->szData[ 9  ] = b9;
    pImp->szData[ 10 ] = b10;
    pImp->szData[ 11 ] = b11;
    pImp->szData[ 12 ] = b12;
    pImp->szData[ 13 ] = b13;
    pImp->szData[ 14 ] = b14;
    pImp->szData[ 15 ] = b15;
}

SvGlobalName::~SvGlobalName()
{
    pImp->nRefCount--;
    if( !pImp->nRefCount )
        delete pImp;
}

SvGlobalName & SvGlobalName::operator = ( const SvGlobalName & rObj )
{
    rObj.pImp->nRefCount++;
    pImp->nRefCount--;
    if( !pImp->nRefCount )
        delete pImp;
    pImp = rObj.pImp;
    return *this;
}

void SvGlobalName::NewImp()
{
    if( pImp->nRefCount > 1 )
    {
        pImp->nRefCount--;
        pImp = new ImpSvGlobalName( *pImp );
        pImp->nRefCount++;
    }
}

SvStream& operator << ( SvStream& rOStr, const SvGlobalName & rObj )
{
    sal_uInt32 a;
    memcpy(&a, rObj.pImp->szData, sizeof(sal_uInt32));
    rOStr << a;

    sal_uInt16 b;
    memcpy(&b, rObj.pImp->szData+4, sizeof(sal_uInt16));
    rOStr << b;

    memcpy(&b, rObj.pImp->szData+6, sizeof(sal_uInt16));
    rOStr << b;

    rOStr.Write( (sal_Char *)&rObj.pImp->szData[ 8 ], 8 );
    return rOStr;
}

SvStream& operator >> ( SvStream& rStr, SvGlobalName & rObj )
{
    rObj.NewImp(); // copy if necessary

    sal_uInt32 a;
    rStr >> a;
    memcpy(rObj.pImp->szData, &a, sizeof(sal_uInt32));

    sal_uInt16 b;
    rStr >> b;
    memcpy(rObj.pImp->szData+4, &b, sizeof(sal_uInt16));

    rStr >> b;
    memcpy(rObj.pImp->szData+6, &b, sizeof(sal_uInt16));

    rStr.Read( (sal_Char *)&rObj.pImp->szData[ 8 ], 8 );
    return rStr;
}


sal_Bool SvGlobalName::operator < ( const SvGlobalName & rObj ) const
{
    int n = memcmp( pImp->szData +6, rObj.pImp->szData +6,
                    sizeof( pImp->szData ) -6);
    if( n < 0 )
        return sal_True;
    else if( n > 0 )
        return sal_False;

    sal_uInt16 Data2_a;
    memcpy(&Data2_a, pImp->szData+4, sizeof(sal_uInt16));

    sal_uInt16 Data2_b;
    memcpy(&Data2_b, rObj.pImp->szData+4, sizeof(sal_uInt16));

    if( Data2_a < Data2_b )
        return sal_True;
    else if( Data2_a == Data2_b )
    {
        sal_uInt32 Data1_a;
        memcpy(&Data1_a, pImp->szData, sizeof(sal_uInt32));

        sal_uInt32 Data1_b;
        memcpy(&Data1_b, rObj.pImp->szData, sizeof(sal_uInt32));

        return Data1_a  < Data1_b;
    }
    else
        return sal_False;

}

SvGlobalName & SvGlobalName::operator += ( sal_uInt32 n )
{
    NewImp();

    sal_uInt32 nOld;
    memcpy(&nOld, pImp->szData, sizeof(sal_uInt32));
    sal_uInt32 nNew = nOld + n;
    memcpy(pImp->szData, &nNew, sizeof(sal_uInt32));

    if( nOld > nNew )
    {
        // overflow
        sal_uInt16 Data2;
        memcpy(&Data2, pImp->szData + 4, sizeof(sal_uInt16));
        ++Data2;
        memcpy(pImp->szData + 4, &Data2, sizeof(sal_uInt16));
    }
    return *this;
}

sal_Bool SvGlobalName::operator == ( const SvGlobalName & rObj ) const
{
    return *pImp == *rObj.pImp;
}

void SvGlobalName::MakeFromMemory( void * pData )
{
    NewImp();
    memcpy( pImp->szData, pData, sizeof( pImp->szData ) );
}

sal_Bool SvGlobalName::MakeId( const String & rIdStr )
{
    rtl::OString aStr(rtl::OUStringToOString(rIdStr,
        RTL_TEXTENCODING_ASCII_US));
    const sal_Char *pStr = aStr.getStr();
    if( rIdStr.Len() == 36
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
                return sal_False;
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
                return sal_False;
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
                return sal_False;
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
                return sal_False;
            pStr++;
            if( i == 3 )
                pStr++;
        }

        NewImp();
        memcpy(&pImp->szData[0], &nFirst, sizeof(nFirst));
        memcpy(&pImp->szData[4], &nSec, sizeof(nSec));
        memcpy(&pImp->szData[6], &nThird, sizeof(nThird));
        memcpy(&pImp->szData[ 8 ], szRemain, 8);
        return sal_True;
    }
    return sal_False;
}

String SvGlobalName::GetHexName() const
{
    rtl::OStringBuffer aHexBuffer;

    sal_Char buf[ 10 ];
    sal_uInt32 Data1;
    memcpy(&Data1, pImp->szData, sizeof(sal_uInt32));
    sprintf( buf, "%8.8" SAL_PRIXUINT32, Data1 );
    aHexBuffer.append(buf);
    aHexBuffer.append('-');
    sal_uInt16 i ;
    for( i = 4; i < 8; i += 2 )
    {
        sal_uInt16 Data2;
        memcpy(&Data2, pImp->szData+i, sizeof(sal_uInt16));
        sprintf( buf, "%4.4X", Data2 );
        aHexBuffer.append(buf);
        aHexBuffer.append('-');
    }
    for( i = 8; i < 10; i++ )
    {
        sprintf( buf, "%2.2x", pImp->szData[ i ] );
        aHexBuffer.append(buf);
    }
    aHexBuffer.append('-');
    for( i = 10; i < 16; i++ )
    {
        sprintf( buf, "%2.2x", pImp->szData[ i ] );
        aHexBuffer.append(buf);
    }
    return rtl::OStringToOUString(aHexBuffer.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
}

com::sun::star::uno::Sequence < sal_Int8 > SvGlobalName::GetByteSequence() const
{
    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    com::sun::star::uno::Sequence< sal_Int8 > aResult( 16 );

    sal_uInt32 Data1;
    memcpy(&Data1, pImp->szData, sizeof(sal_uInt32));
    aResult[0] = (sal_Int8) (Data1 >> 24);
    aResult[1] = (sal_Int8) ((Data1 << 8 ) >> 24);
    aResult[2] = (sal_Int8) ((Data1 << 16 ) >> 24);
    aResult[3] = (sal_Int8) ((Data1 << 24 ) >> 24);
    sal_uInt16 Data2;
    memcpy(&Data2, pImp->szData+4, sizeof(sal_uInt16));
    aResult[4] = (sal_Int8) (Data2 >> 8);
    aResult[5] = (sal_Int8) ((Data2 << 8 ) >> 8);
    sal_uInt16 Data3;
    memcpy(&Data3, pImp->szData+6, sizeof(sal_uInt16));
    aResult[6] = (sal_Int8) (Data3 >> 8);
    aResult[7] = (sal_Int8) ((Data3 << 8 ) >> 8);
    aResult[8] = pImp->szData[ 8 ];
    aResult[9] = pImp->szData[ 9 ];
    aResult[10] = pImp->szData[ 10 ];
    aResult[11] = pImp->szData[ 11 ];
    aResult[12] = pImp->szData[ 12 ];
    aResult[13] = pImp->szData[ 13 ];
    aResult[14] = pImp->szData[ 14 ];
    aResult[15] = pImp->szData[ 15 ];

    return aResult;
}

SvGlobalName::SvGlobalName( const com::sun::star::uno::Sequence < sal_Int8 >& aSeq )
{
    // create SvGlobalName from a platform independent representation
    GUID aResult;
    memset( &aResult, 0, sizeof( aResult ) );
    if ( aSeq.getLength() == 16 )
    {
        aResult.Data1 = ( ( ( ( ( ( sal_uInt8 )aSeq[0] << 8 ) + ( sal_uInt8 )aSeq[1] ) << 8 ) + ( sal_uInt8 )aSeq[2] ) << 8 ) + ( sal_uInt8 )aSeq[3];
        aResult.Data2 = ( ( sal_uInt8 )aSeq[4] << 8 ) + ( sal_uInt8 )aSeq[5];
        aResult.Data3 = ( ( sal_uInt8 )aSeq[6] << 8 ) + ( sal_uInt8 )aSeq[7];
        for( int nInd = 0; nInd < 8; nInd++ )
            aResult.Data4[nInd] = ( sal_uInt8 )aSeq[nInd+8];
    }

    pImp = new ImpSvGlobalName();
    pImp->nRefCount++;
    memcpy( pImp->szData, &aResult, sizeof( pImp->szData ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
