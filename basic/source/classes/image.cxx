/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tools/stream.hxx>
#include <tools/tenccvt.hxx>
#include <osl/thread.h>
#include <basic/sbx.hxx>
#include "sb.hxx"
#include <string.h>
#include "image.hxx"
#include <codegen.hxx>
SbiImage::SbiImage()
{
    pStringOff = NULL;
    pStrings   = NULL;
    pCode      = NULL;
    pLegacyPCode = NULL;
    nFlags     = 0;
    nStrings   = 0;
    nStringSize= 0;
    nCodeSize  = 0;
    nLegacyCodeSize  =
    nDimBase   = 0;
    bInit      =
    bError     = false;
    bFirstInit = true;
    eCharSet   = osl_getThreadTextEncoding();
    nStringIdx = 0;
    nStringOff = 0;
}

SbiImage::~SbiImage()
{
    Clear();
}

void SbiImage::Clear()
{
    delete[] pStringOff;
    delete[] pStrings;
    delete[] pCode;
    ReleaseLegacyBuffer();
    pStringOff = NULL;
    pStrings   = NULL;
    pCode      = NULL;
    nFlags     = 0;
    nStrings   = 0;
    nStringSize= 0;
    nLegacyCodeSize  = 0;
    nCodeSize  = 0;
    eCharSet   = osl_getThreadTextEncoding();
    nDimBase   = 0;
    bError     = false;
}

/**************************************************************************
*
*    Service-Routines for Load/Store
*
**************************************************************************/

bool SbiGood( SvStream& r )
{
    return !r.IsEof() && r.GetError() == SVSTREAM_OK;
}


sal_uIntPtr SbiOpenRecord( SvStream& r, sal_uInt16 nSignature, sal_uInt16 nElem )
{
    sal_uIntPtr nPos = r.Tell();
    r.WriteUInt16( nSignature ).WriteInt32( (sal_Int32) 0 ).WriteUInt16( nElem );
    return nPos;
}


void SbiCloseRecord( SvStream& r, sal_uIntPtr nOff )
{
    sal_uIntPtr nPos = r.Tell();
    r.Seek( nOff + 2 );
    r.WriteInt32( (sal_Int32) ( nPos - nOff - 8 ) );
    r.Seek( nPos );
}

/**************************************************************************
*
*    Load/Store
*
**************************************************************************/

bool SbiImage::Load( SvStream& r, sal_uInt32& nVersion )
{

    sal_uInt16 nSign, nCount;
    sal_uInt32 nLen, nOff;

    Clear();
    
    r.ReadUInt16( nSign ).ReadUInt32( nLen ).ReadUInt16( nCount );
    sal_uIntPtr nLast = r.Tell() + nLen;
    sal_uInt32 nCharSet;               
    sal_uInt32 lDimBase;
    sal_uInt16 nReserved1;
    sal_uInt32 nReserved2;
    sal_uInt32 nReserved3;
    bool bBadVer = false;
    if( nSign == B_MODULE )
    {
        r.ReadUInt32( nVersion ).ReadUInt32( nCharSet ).ReadUInt32( lDimBase )
         .ReadUInt16( nFlags ).ReadUInt16( nReserved1 ).ReadUInt32( nReserved2 ).ReadUInt32( nReserved3 );
        eCharSet = nCharSet;
        eCharSet = GetSOLoadTextEncoding( eCharSet );
        bBadVer  = ( nVersion > B_CURVERSION );
        nDimBase = (sal_uInt16) lDimBase;
    }

    bool bLegacy = ( nVersion < B_EXT_IMG_VERSION );

    sal_uIntPtr nNext;
    while( ( nNext = r.Tell() ) < nLast )
    {

        r.ReadUInt16( nSign ).ReadUInt32( nLen ).ReadUInt16( nCount );
        nNext += nLen + 8;
        if( r.GetError() == SVSTREAM_OK )
        {
            switch( nSign )
            {
            case B_NAME:
                aName = r.ReadUniOrByteString(eCharSet);
                break;
            case B_COMMENT:
                aComment = r.ReadUniOrByteString(eCharSet );
                break;
            case B_SOURCE:
            {
                aOUSource = r.ReadUniOrByteString(eCharSet);
                break;
            }
            case B_EXTSOURCE:
            {
                for( sal_uInt16 j = 0 ; j < nCount ; j++ )
                {
                    aOUSource += r.ReadUniOrByteString(eCharSet);
                }
                break;
            }
            case B_PCODE:
                if( bBadVer ) break;
                pCode = new char[ nLen ];
                nCodeSize = nLen;
                r.Read( pCode, nCodeSize );
                if ( bLegacy )
                {
                    ReleaseLegacyBuffer(); 
                    nLegacyCodeSize = (sal_uInt16) nCodeSize;
                    pLegacyPCode = pCode;

                    PCodeBuffConvertor< sal_uInt16, sal_uInt32 > aLegacyToNew( (sal_uInt8*)pLegacyPCode, nLegacyCodeSize );
                    aLegacyToNew.convert();
                    pCode = (char*)aLegacyToNew.GetBuffer();
                    nCodeSize = aLegacyToNew.GetSize();
                    
                    
                    
                    
                    
                    
                    
                    
                }
                break;
            case B_PUBLICS:
            case B_POOLDIR:
            case B_SYMPOOL:
            case B_LINERANGES:
                break;
            case B_STRINGPOOL:
                if( bBadVer ) break;
                MakeStrings( nCount );
                short i;
                for( i = 0; i < nStrings && SbiGood( r ); i++ )
                {
                    r.ReadUInt32( nOff );
                    pStringOff[ i ] = (sal_uInt16) nOff;
                }
                r.ReadUInt32( nLen );
                if( SbiGood( r ) )
                {
                    delete [] pStrings;
                    pStrings = new sal_Unicode[ nLen ];
                    nStringSize = (sal_uInt16) nLen;

                    char* pByteStrings = new char[ nLen ];
                    r.Read( pByteStrings, nStringSize );
                    for( short j = 0; j < nStrings; j++ )
                    {
                        sal_uInt16 nOff2 = (sal_uInt16) pStringOff[ j ];
                        OUString aStr( pByteStrings + nOff2, strlen(pByteStrings + nOff2), eCharSet );
                        memcpy( pStrings + nOff2, aStr.getStr(), (aStr.getLength() + 1) * sizeof( sal_Unicode ) );
                    }
                    delete[] pByteStrings;
                }
                break;
            case B_MODEND:
                goto done;
            default:
                break;
            }
        }
        else
        {
            break;
        }
        r.Seek( nNext );
    }
done:
    r.Seek( nLast );
    if( !SbiGood( r ) )
    {
        bError = true;
    }
    return !bError;
}

bool SbiImage::Save( SvStream& r, sal_uInt32 nVer )
{
    bool bLegacy = ( nVer < B_EXT_IMG_VERSION );

    
    
    if ( bLegacy && ExceedsLegacyLimits() )
    {
        SbiImage aEmptyImg;
        aEmptyImg.aName = aName;
        aEmptyImg.Save( r, B_LEGACYVERSION );
        return true;
    }
    
    sal_uIntPtr nStart = SbiOpenRecord( r, B_MODULE, 1 );
    sal_uIntPtr nPos;

    eCharSet = GetSOStoreTextEncoding( eCharSet );
    if ( bLegacy )
    {
        r.WriteInt32( (sal_Int32) B_LEGACYVERSION );
    }
    else
    {
        r.WriteInt32( (sal_Int32) B_CURVERSION );
    }
    r .WriteInt32( (sal_Int32) eCharSet )
      .WriteInt32( (sal_Int32) nDimBase )
      .WriteInt16( (sal_Int16) nFlags )
      .WriteInt16( (sal_Int16) 0 )
      .WriteInt32( (sal_Int32) 0 )
      .WriteInt32( (sal_Int32) 0 );

    
    if( !aName.isEmpty() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_NAME, 1 );
        r.WriteUniOrByteString( aName, eCharSet );
        SbiCloseRecord( r, nPos );
    }
    
    if( !aComment.isEmpty() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_COMMENT, 1 );
        r.WriteUniOrByteString( aComment, eCharSet );
        SbiCloseRecord( r, nPos );
    }
    
    if( !aOUSource.isEmpty() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_SOURCE, 1 );
        r.WriteUniOrByteString( aOUSource, eCharSet );
        SbiCloseRecord( r, nPos );
    }
    
    if( pCode && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_PCODE, 1 );
        if ( bLegacy )
        {
            ReleaseLegacyBuffer(); 
            PCodeBuffConvertor< sal_uInt32, sal_uInt16 > aNewToLegacy( (sal_uInt8*)pCode, nCodeSize );
            aNewToLegacy.convert();
            pLegacyPCode = (char*)aNewToLegacy.GetBuffer();
            nLegacyCodeSize = aNewToLegacy.GetSize();
            r.Write( pLegacyPCode, nLegacyCodeSize );
        }
        else
        {
            r.Write( pCode, nCodeSize );
        }
        SbiCloseRecord( r, nPos );
    }
    
    if( nStrings )
    {
        nPos = SbiOpenRecord( r, B_STRINGPOOL, nStrings );
        
        
        short i;

        for( i = 0; i < nStrings && SbiGood( r ); i++ )
        {
            r.WriteUInt32( (sal_uInt32) pStringOff[ i ] );
        }
        
        char* pByteStrings = new char[ nStringSize ];
        for( i = 0; i < nStrings; i++ )
        {
            sal_uInt16 nOff = (sal_uInt16) pStringOff[ i ];
            OString aStr(OUStringToOString(OUString(pStrings + nOff), eCharSet));
            memcpy( pByteStrings + nOff, aStr.getStr(), (aStr.getLength() + 1) * sizeof( char ) );
        }
        r.WriteUInt32( (sal_uInt32) nStringSize );
        r.Write( pByteStrings, nStringSize );

        delete[] pByteStrings;
        SbiCloseRecord( r, nPos );
    }
    
    SbiCloseRecord( r, nStart );
    if( !SbiGood( r ) )
    {
        bError = true;
    }
    return !bError;
}

/**************************************************************************
*
*    Routines called by the compiler
*
**************************************************************************/

void SbiImage::MakeStrings( short nSize )
{
    nStrings = 0;
    nStringIdx = 0;
    nStringOff = 0;
    nStringSize = 1024;
    pStrings = new sal_Unicode[ nStringSize ];
    pStringOff = new sal_uInt32[ nSize ];
    if( pStrings && pStringOff )
    {
        nStrings = nSize;
        memset( pStringOff, 0, nSize * sizeof( sal_uInt32 ) );
        memset( pStrings, 0, nStringSize * sizeof( sal_Unicode ) );
    }
    else
    {
        bError = true;
    }
}



void SbiImage::AddString( const OUString& r )
{
    if( nStringIdx >= nStrings )
    {
        bError = true;
    }
    if( !bError )
    {
        sal_Int32  len = r.getLength() + 1;
        sal_uInt32 needed = nStringOff + len;
        if( needed > 0xFFFFFF00L )
        {
            bError = true;  
        }
        else if( needed > nStringSize )
        {
            sal_uInt32 nNewLen = needed + 1024;
            nNewLen &= 0xFFFFFC00;  
            sal_Unicode* p = NULL;
            if( (p = new sal_Unicode[ nNewLen ]) != NULL )
            {
                memcpy( p, pStrings, nStringSize * sizeof( sal_Unicode ) );
                delete[] pStrings;
                pStrings = p;
                nStringSize = sal::static_int_cast< sal_uInt16 >(nNewLen);
            }
            else
            {
                bError = true;
            }
        }
        if( !bError )
        {
            pStringOff[ nStringIdx++ ] = nStringOff;
            memcpy( pStrings + nStringOff, r.getStr(), len * sizeof( sal_Unicode ) );
            nStringOff = nStringOff + len;
            
            if( nStringIdx >= nStrings )
            {
                nStringSize = nStringOff;
            }
        }
    }
}





void SbiImage::AddCode( char* p, sal_uInt32 s )
{
    pCode = p;
    nCodeSize = s;
}


void SbiImage::AddType(SbxObject* pObject)
{
    if( !rTypes.Is() )
    {
        rTypes = new SbxArray;
    }
    SbxObject *pCopyObject = new SbxObject(*pObject);
    rTypes->Insert (pCopyObject,rTypes->Count());
}

void SbiImage::AddEnum(SbxObject* pObject) 
{
    if( !rEnums.Is() )
    {
        rEnums = new SbxArray;
    }
    rEnums->Insert( pObject, rEnums->Count() );
}


/**************************************************************************
*
*    Accessing the image
*
**************************************************************************/


OUString SbiImage::GetString( short nId ) const
{
    if( nId && nId <= nStrings )
    {
        sal_uInt32 nOff = pStringOff[ nId - 1 ];
        sal_Unicode* pStr = pStrings + nOff;

        
        if( *pStr == 0 )
        {
            sal_uInt32 nNextOff = (nId < nStrings) ? pStringOff[ nId ] : nStringOff;
            sal_uInt32 nLen = nNextOff - nOff - 1;
            if( nLen == 1 )
            {
                
                OUString aNullCharStr( (sal_Unicode)0);
                return aNullCharStr;
            }
        }
        else
        {
            return OUString(pStr);
        }
    }
    return OUString();
}

const SbxObject* SbiImage::FindType (OUString aTypeName) const
{
    return rTypes.Is() ? (SbxObject*)rTypes->Find(aTypeName,SbxCLASS_OBJECT) : NULL;
}

sal_uInt16 SbiImage::CalcLegacyOffset( sal_Int32 nOffset )
{
    return SbiCodeGen::calcLegacyOffSet( (sal_uInt8*)pCode, nOffset ) ;
}

sal_uInt32 SbiImage::CalcNewOffset( sal_Int16 nOffset )
{
    return SbiCodeGen::calcNewOffSet( (sal_uInt8*)pLegacyPCode, nOffset ) ;
}

void  SbiImage::ReleaseLegacyBuffer()
{
    delete[] pLegacyPCode;
    pLegacyPCode = NULL;
    nLegacyCodeSize = 0;
}

bool SbiImage::ExceedsLegacyLimits()
{
    return ( nStringSize > 0xFF00L ) || ( CalcLegacyOffset( nCodeSize ) > 0xFF00L );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
