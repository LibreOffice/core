/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <tools/stream.hxx>
#include <tools/globname.hxx>

/************** class ImpSvGlobalName ************************************/
ImpSvGlobalName::ImpSvGlobalName( const ImpSvGlobalName & rObj )
{
    nRefCount = 0;
    memcpy( szData, rObj.szData, sizeof( szData ) );
}

/************** class ImpSvGlobalName ************************************/
ImpSvGlobalName::ImpSvGlobalName( int )
{
    nRefCount = 1;
    memset( szData, 0, sizeof( szData ) );
}

/*************************************************************************
|*    ImpSvGlobalName::operator ==()
*************************************************************************/
BOOL ImpSvGlobalName::operator == ( const ImpSvGlobalName & rObj ) const
{
    return !memcmp( szData, rObj.szData, sizeof( szData ) );
}

/*************************************************************************
|*    SvGlobalName::SvGlobalName()
*************************************************************************/
SvGlobalName::SvGlobalName()
{
    static ImpSvGlobalName aNoName( 0 );

    pImp = &aNoName;
    pImp->nRefCount++;
}

// locker die Struktur von Windows kopiert
#ifdef WNT
struct _GUID
#else
struct GUID
#endif
{
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    BYTE   Data4[8];
};
SvGlobalName::SvGlobalName( const CLSID & rId )
{
    pImp = new ImpSvGlobalName();
    pImp->nRefCount++;
    memcpy( pImp->szData, &rId, sizeof( pImp->szData ) );
}

SvGlobalName::SvGlobalName( UINT32 n1, USHORT n2, USHORT n3,
                            BYTE b8, BYTE b9, BYTE b10, BYTE b11,
                            BYTE b12, BYTE b13, BYTE b14, BYTE b15 )
{
    pImp = new ImpSvGlobalName();
    pImp->nRefCount++;

    *(UINT32 *)pImp->szData       = n1;
    *(USHORT *)&pImp->szData[ 4 ] = n2;
    *(USHORT *)&pImp->szData[ 6 ] = n3;
    pImp->szData[ 8  ] = b8;
    pImp->szData[ 9  ] = b9;
    pImp->szData[ 10 ] = b10;
    pImp->szData[ 11 ] = b11;
    pImp->szData[ 12 ] = b12;
    pImp->szData[ 13 ] = b13;
    pImp->szData[ 14 ] = b14;
    pImp->szData[ 15 ] = b15;
}

/*************************************************************************
|*    SvGlobalName::~SvGlobalName()
*************************************************************************/
SvGlobalName::~SvGlobalName()
{
    pImp->nRefCount--;
    if( !pImp->nRefCount )
        delete pImp;
}

/*************************************************************************
|*    SvGlobalName::operator = ()
*************************************************************************/
SvGlobalName & SvGlobalName::operator = ( const SvGlobalName & rObj )
{
    rObj.pImp->nRefCount++;
    pImp->nRefCount--;
    if( !pImp->nRefCount )
        delete pImp;
    pImp = rObj.pImp;
    return *this;
}

/*************************************************************************
|*    SvGlobalName::NewImp()
*************************************************************************/
void SvGlobalName::NewImp()
{
    if( pImp->nRefCount > 1 )
    {
        pImp->nRefCount--;
        pImp = new ImpSvGlobalName( *pImp );
        pImp->nRefCount++;
    }
}

/*************************************************************************
|*    SvGlobalName::operator << ()
|*    SvGlobalName::operator >> ()
*************************************************************************/
SvStream& operator << ( SvStream& rOStr, const SvGlobalName & rObj )
{
    rOStr << *(UINT32 *)rObj.pImp->szData;
    rOStr << *(USHORT *)&rObj.pImp->szData[ 4 ];
    rOStr << *(USHORT *)&rObj.pImp->szData[ 6 ];
    rOStr.Write( (sal_Char *)&rObj.pImp->szData[ 8 ], 8 );
    return rOStr;
}

SvStream& operator >> ( SvStream& rStr, SvGlobalName & rObj )
{
    rObj.NewImp(); // kopieren, falls noetig
    rStr >> *(UINT32 *)rObj.pImp->szData;
    rStr >> *(USHORT *)&rObj.pImp->szData[ 4 ];
    rStr >> *(USHORT *)&rObj.pImp->szData[ 6 ];
    rStr.Read( (sal_Char *)&rObj.pImp->szData[ 8 ], 8 );
    return rStr;
}


/*************************************************************************
|*    SvGlobalName::operator < ()
*************************************************************************/
BOOL SvGlobalName::operator < ( const SvGlobalName & rObj ) const
{
    int n = memcmp( pImp->szData +6, rObj.pImp->szData +6,
                    sizeof( pImp->szData ) -6);
    if( n < 0 )
        return TRUE;
    else if( n > 0 )
        return FALSE;
    else if( *(USHORT *)&pImp->szData[ 4 ] < *(USHORT *)&rObj.pImp->szData[ 4 ] )
        return TRUE;
    else if( *(USHORT *)&pImp->szData[ 4 ] == *(USHORT *)&rObj.pImp->szData[ 4 ] )
        return *(UINT32 *)pImp->szData  < *(UINT32 *)rObj.pImp->szData;
    else
        return FALSE;

}

/*************************************************************************
|*    SvGlobalName::operator +=()
*************************************************************************/
SvGlobalName & SvGlobalName::operator += ( UINT32 n )
{
    NewImp();
    UINT32 nOld = (*(UINT32 *)pImp->szData);
    (*(UINT32 *)pImp->szData) += n;
    if( nOld > *(UINT32 *)pImp->szData )
        // ueberlauf
        (*(USHORT *)&pImp->szData[ 4 ])++;
    return *this;
}

/*************************************************************************
|*    SvGlobalName::operator ==()
*************************************************************************/
BOOL SvGlobalName::operator == ( const SvGlobalName & rObj ) const
{
    return *pImp == *rObj.pImp;
}

void SvGlobalName::MakeFromMemory( void * pData )
{
    NewImp();
    memcpy( pImp->szData, pData, sizeof( pImp->szData ) );
}

/*************************************************************************
|*    SvGlobalName::MakeId()
*************************************************************************/
BOOL SvGlobalName::MakeId( const String & rIdStr )
{
    ByteString  aStr( rIdStr, RTL_TEXTENCODING_ASCII_US );
    sal_Char * pStr = (sal_Char *)aStr.GetBuffer();
    if( rIdStr.Len() == 36
      && '-' == pStr[ 8 ]  && '-' == pStr[ 13 ]
      && '-' == pStr[ 18 ] && '-' == pStr[ 23 ] )
    {
        UINT32 nFirst = 0;
        int i = 0;
        for( i = 0; i < 8; i++ )
        {
            if( isxdigit( *pStr ) )
                if( isdigit( *pStr ) )
                    nFirst = nFirst * 16 + (*pStr - '0');
                else
                    nFirst = nFirst * 16 + (toupper( *pStr ) - 'A' + 10 );
            else
                return FALSE;
            pStr++;
        }

        UINT16 nSec = 0;
        pStr++;
        for( i = 0; i < 4; i++ )
        {
            if( isxdigit( *pStr ) )
                if( isdigit( *pStr ) )
                    nSec = nSec * 16 + (*pStr - '0');
                else
                    nSec = nSec * 16 + (UINT16)(toupper( *pStr ) - 'A' + 10 );
            else
                return FALSE;
            pStr++;
        }

        UINT16 nThird = 0;
        pStr++;
        for( i = 0; i < 4; i++ )
        {
            if( isxdigit( *pStr ) )
                if( isdigit( *pStr ) )
                    nThird = nThird * 16 + (*pStr - '0');
                else
                    nThird = nThird * 16 + (UINT16)(toupper( *pStr ) - 'A' + 10 );
            else
                return FALSE;
            pStr++;
        }

        BYTE szRemain[ 8 ];
        memset( szRemain, 0, sizeof( szRemain ) );
        pStr++;
        for( i = 0; i < 16; i++ )
        {
            if( isxdigit( *pStr ) )
                if( isdigit( *pStr ) )
                    szRemain[i/2] = szRemain[i/2] * 16 + (*pStr - '0');
                else
                    szRemain[i/2] = szRemain[i/2] * 16 + (BYTE)(toupper( *pStr ) - 'A' + 10 );
            else
                return FALSE;
            pStr++;
            if( i == 3 )
                pStr++;
        }

        NewImp();
        *(UINT32 *)pImp->szData       = nFirst;
        *(USHORT *)&pImp->szData[ 4 ] = nSec;
        *(USHORT *)&pImp->szData[ 6 ] = nThird;
        memcpy( &pImp->szData[ 8 ], szRemain, 8 );
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*    SvGlobalName::GetctorName()
*************************************************************************/
String SvGlobalName::GetctorName() const
{
    ByteString aRet;

    sal_Char buf[ 20 ];
    sprintf( buf, "0x%8.8lX", (ULONG)*(UINT32 *)pImp->szData );
    aRet += buf;
    USHORT i;
    for( i = 4; i < 8; i += 2 )
    {
        aRet += ',';
        sprintf( buf, "0x%4.4X", *(USHORT *)&pImp->szData[ i ] );
        aRet += buf;
    }
    for( i = 8; i < 16; i++ )
    {
        aRet += ',';
        sprintf( buf, "0x%2.2x", pImp->szData[ i ] );
        aRet += buf;
    }
    return String( aRet, RTL_TEXTENCODING_ASCII_US );
}

/*************************************************************************
|*    SvGlobalName::GetHexName()
*************************************************************************/
String SvGlobalName::GetHexName() const
{
    ByteString aRet;

    sal_Char buf[ 10 ];
    sprintf( buf, "%8.8lX", (ULONG)*(UINT32 *)pImp->szData );
    aRet += buf;
    aRet += '-';
    USHORT i ;
    for( i = 4; i < 8; i += 2 )
    {
        sprintf( buf, "%4.4X", *(USHORT *)&pImp->szData[ i ] );
        aRet += buf;
        aRet += '-';
    }
    for( i = 8; i < 10; i++ )
    {
        sprintf( buf, "%2.2x", pImp->szData[ i ] );
        aRet += buf;
    }
    aRet += '-';
    for( i = 10; i < 16; i++ )
    {
        sprintf( buf, "%2.2x", pImp->szData[ i ] );
        aRet += buf;
    }
    return String( aRet, RTL_TEXTENCODING_ASCII_US );
}

/************** SvGlobalNameList ****************************************/
/************************************************************************/
/*************************************************************************
|*    SvGlobalNameList::SvGlobalNameList()
*************************************************************************/
SvGlobalNameList::SvGlobalNameList()
    : aList( 1, 1 )
{
}

/*************************************************************************
|*    SvGlobalNameList::~SvGlobalNameList()
*************************************************************************/
SvGlobalNameList::~SvGlobalNameList()
{
    for( ULONG i = Count(); i > 0; i-- )
    {
        ImpSvGlobalName * pImp = (ImpSvGlobalName *)aList.GetObject( i -1 );
        pImp->nRefCount--;
        if( !pImp->nRefCount )
            delete pImp;
    }
}

/*************************************************************************
|*    SvGlobalNameList::Append()
*************************************************************************/
void SvGlobalNameList::Append( const SvGlobalName & rName )
{
    rName.pImp->nRefCount++;
    aList.Insert( rName.pImp, LIST_APPEND );
}

/*************************************************************************
|*    SvGlobalNameList::GetObject()
*************************************************************************/
SvGlobalName SvGlobalNameList::GetObject( ULONG nPos )
{
    return SvGlobalName( (ImpSvGlobalName *)aList.GetObject( nPos ) );
}

/*************************************************************************
|*    SvGlobalNameList::IsEntry()
*************************************************************************/
BOOL SvGlobalNameList::IsEntry( const SvGlobalName & rName )
{
    for( ULONG i = Count(); i > 0; i-- )
    {
        if( *rName.pImp == *(ImpSvGlobalName *)aList.GetObject( i -1 ) )
            return TRUE;
    }
    return FALSE;
}

com::sun::star::uno::Sequence < sal_Int8 > SvGlobalName::GetByteSequence() const
{
    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    com::sun::star::uno::Sequence< sal_Int8 > aResult( 16 );

    aResult[0] = (sal_Int8) (*(UINT32 *)pImp->szData >> 24);
    aResult[1] = (sal_Int8) ((*(UINT32 *)pImp->szData << 8 ) >> 24);
    aResult[2] = (sal_Int8) ((*(UINT32 *)pImp->szData << 16 ) >> 24);
    aResult[3] = (sal_Int8) ((*(UINT32 *)pImp->szData << 24 ) >> 24);
    aResult[4] = (sal_Int8) (*(USHORT *)&pImp->szData[ 4 ] >> 8);
    aResult[5] = (sal_Int8) ((*(USHORT *)&pImp->szData[ 4 ] << 8 ) >> 8);
    aResult[6] = (sal_Int8) (*(USHORT *)&pImp->szData[ 6 ] >> 8);
    aResult[7] = (sal_Int8) ((*(USHORT *)&pImp->szData[ 6 ] << 8 ) >> 8);
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
