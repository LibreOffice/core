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
sal_Bool ImpSvGlobalName::operator == ( const ImpSvGlobalName & rObj ) const
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

    *(sal_uInt32 *)pImp->szData       = n1;
    *(sal_uInt16 *)&pImp->szData[ 4 ] = n2;
    *(sal_uInt16 *)&pImp->szData[ 6 ] = n3;
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
    rOStr << *(sal_uInt32 *)rObj.pImp->szData;
    rOStr << *(sal_uInt16 *)&rObj.pImp->szData[ 4 ];
    rOStr << *(sal_uInt16 *)&rObj.pImp->szData[ 6 ];
    rOStr.Write( (sal_Char *)&rObj.pImp->szData[ 8 ], 8 );
    return rOStr;
}

SvStream& operator >> ( SvStream& rStr, SvGlobalName & rObj )
{
    rObj.NewImp(); // kopieren, falls noetig
    rStr >> *(sal_uInt32 *)rObj.pImp->szData;
    rStr >> *(sal_uInt16 *)&rObj.pImp->szData[ 4 ];
    rStr >> *(sal_uInt16 *)&rObj.pImp->szData[ 6 ];
    rStr.Read( (sal_Char *)&rObj.pImp->szData[ 8 ], 8 );
    return rStr;
}


/*************************************************************************
|*    SvGlobalName::operator < ()
*************************************************************************/
sal_Bool SvGlobalName::operator < ( const SvGlobalName & rObj ) const
{
    int n = memcmp( pImp->szData +6, rObj.pImp->szData +6,
                    sizeof( pImp->szData ) -6);
    if( n < 0 )
        return sal_True;
    else if( n > 0 )
        return sal_False;
    else if( *(sal_uInt16 *)&pImp->szData[ 4 ] < *(sal_uInt16 *)&rObj.pImp->szData[ 4 ] )
        return sal_True;
    else if( *(sal_uInt16 *)&pImp->szData[ 4 ] == *(sal_uInt16 *)&rObj.pImp->szData[ 4 ] )
        return *(sal_uInt32 *)pImp->szData  < *(sal_uInt32 *)rObj.pImp->szData;
    else
        return sal_False;

}

/*************************************************************************
|*    SvGlobalName::operator +=()
*************************************************************************/
SvGlobalName & SvGlobalName::operator += ( sal_uInt32 n )
{
    NewImp();
    sal_uInt32 nOld = (*(sal_uInt32 *)pImp->szData);
    (*(sal_uInt32 *)pImp->szData) += n;
    if( nOld > *(sal_uInt32 *)pImp->szData )
        // ueberlauf
        (*(sal_uInt16 *)&pImp->szData[ 4 ])++;
    return *this;
}

/*************************************************************************
|*    SvGlobalName::operator ==()
*************************************************************************/
sal_Bool SvGlobalName::operator == ( const SvGlobalName & rObj ) const
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
sal_Bool SvGlobalName::MakeId( const String & rIdStr )
{
    ByteString  aStr( rIdStr, RTL_TEXTENCODING_ASCII_US );
    sal_Char * pStr = (sal_Char *)aStr.GetBuffer();
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
        *(sal_uInt32 *)pImp->szData       = nFirst;
        *(sal_uInt16 *)&pImp->szData[ 4 ] = nSec;
        *(sal_uInt16 *)&pImp->szData[ 6 ] = nThird;
        memcpy( &pImp->szData[ 8 ], szRemain, 8 );
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
|*    SvGlobalName::GetctorName()
*************************************************************************/
String SvGlobalName::GetctorName() const
{
    ByteString aRet;

    sal_Char buf[ 20 ];
    sprintf( buf, "0x%8.8lX", (sal_uIntPtr)*(sal_uInt32 *)pImp->szData );
    aRet += buf;
    sal_uInt16 i;
    for( i = 4; i < 8; i += 2 )
    {
        aRet += ',';
        sprintf( buf, "0x%4.4X", *(sal_uInt16 *)&pImp->szData[ i ] );
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
    sprintf( buf, "%8.8lX", (sal_uIntPtr)*(sal_uInt32 *)pImp->szData );
    aRet += buf;
    aRet += '-';
    sal_uInt16 i ;
    for( i = 4; i < 8; i += 2 )
    {
        sprintf( buf, "%4.4X", *(sal_uInt16 *)&pImp->szData[ i ] );
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
    for( sal_uIntPtr i = Count(); i > 0; i-- )
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
SvGlobalName SvGlobalNameList::GetObject( sal_uLong nPos )
{
    return SvGlobalName( (ImpSvGlobalName *)aList.GetObject( nPos ) );
}

/*************************************************************************
|*    SvGlobalNameList::IsEntry()
*************************************************************************/
sal_Bool SvGlobalNameList::IsEntry( const SvGlobalName & rName )
{
    for( sal_uIntPtr i = Count(); i > 0; i-- )
    {
        if( *rName.pImp == *(ImpSvGlobalName *)aList.GetObject( i -1 ) )
            return sal_True;
    }
    return sal_False;
}

com::sun::star::uno::Sequence < sal_Int8 > SvGlobalName::GetByteSequence() const
{
    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    com::sun::star::uno::Sequence< sal_Int8 > aResult( 16 );

    aResult[0] = (sal_Int8) (*(sal_uInt32 *)pImp->szData >> 24);
    aResult[1] = (sal_Int8) ((*(sal_uInt32 *)pImp->szData << 8 ) >> 24);
    aResult[2] = (sal_Int8) ((*(sal_uInt32 *)pImp->szData << 16 ) >> 24);
    aResult[3] = (sal_Int8) ((*(sal_uInt32 *)pImp->szData << 24 ) >> 24);
    aResult[4] = (sal_Int8) (*(sal_uInt16 *)&pImp->szData[ 4 ] >> 8);
    aResult[5] = (sal_Int8) ((*(sal_uInt16 *)&pImp->szData[ 4 ] << 8 ) >> 8);
    aResult[6] = (sal_Int8) (*(sal_uInt16 *)&pImp->szData[ 6 ] >> 8);
    aResult[7] = (sal_Int8) ((*(sal_uInt16 *)&pImp->szData[ 6 ] << 8 ) >> 8);
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
