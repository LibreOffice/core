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

#include <tools/stream.hxx>
#include <tools/tenccvt.hxx>
#include <basic/sbx.hxx>
#include "sb.hxx"
#include <string.h>     // memset() etc
#include "image.hxx"
#include <codegen.hxx>
SbiImage::SbiImage()
{
    pStringOff = NULL;
    pStrings   = NULL;
    pCode      = NULL;
    pLegacyPCode       = NULL;
    nFlags     = 0;
    nStrings   = 0;
    nStringSize= 0;
    nCodeSize  = 0;
    nLegacyCodeSize  =
    nDimBase   = 0;
    bInit      =
    bError     = false;
    bFirstInit = sal_True;
    eCharSet   = osl_getThreadTextEncoding();
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

sal_Bool SbiGood( SvStream& r )
{
    return sal_Bool( !r.IsEof() && r.GetError() == SVSTREAM_OK );
}

// Open Record
sal_uIntPtr SbiOpenRecord( SvStream& r, sal_uInt16 nSignature, sal_uInt16 nElem )
{
    sal_uIntPtr nPos = r.Tell();
    r << nSignature << (sal_Int32) 0 << nElem;
    return nPos;
}

// Close Record
void SbiCloseRecord( SvStream& r, sal_uIntPtr nOff )
{
    sal_uIntPtr nPos = r.Tell();
    r.Seek( nOff + 2 );
    r << (sal_Int32) ( nPos - nOff - 8 );
    r.Seek( nPos );
}

/**************************************************************************
*
*    Load/Store
*
**************************************************************************/

sal_Bool SbiImage::Load( SvStream& r, sal_uInt32& nVersion )
{

    sal_uInt16 nSign, nCount;
    sal_uInt32 nLen, nOff;

    Clear();
    // Read Master-Record
    r >> nSign >> nLen >> nCount;
    sal_uIntPtr nLast = r.Tell() + nLen;
    sal_uInt32 nCharSet;               // System charset
    sal_uInt32 lDimBase;
    sal_uInt16 nReserved1;
    sal_uInt32 nReserved2;
    sal_uInt32 nReserved3;
    sal_Bool bBadVer = sal_False;
    if( nSign == B_MODULE )
    {
        r >> nVersion >> nCharSet >> lDimBase
          >> nFlags >> nReserved1 >> nReserved2 >> nReserved3;
        eCharSet = (CharSet) nCharSet;
        eCharSet = GetSOLoadTextEncoding( eCharSet );
        bBadVer  = sal_Bool( nVersion > B_CURVERSION );
        nDimBase = (sal_uInt16) lDimBase;
    }

    bool bLegacy = ( nVersion < B_EXT_IMG_VERSION );

    sal_uIntPtr nNext;
    while( ( nNext = r.Tell() ) < nLast )
    {

        r >> nSign >> nLen >> nCount;
        nNext += nLen + 8;
        if( r.GetError() == SVSTREAM_OK )
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
                    String aTmp = r.ReadUniOrByteString(eCharSet);
                    aOUSource += aTmp;
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
                    ReleaseLegacyBuffer(); // release any previously held buffer
                    nLegacyCodeSize = (sal_uInt16) nCodeSize;
                    pLegacyPCode = pCode;

                    PCodeBuffConvertor< sal_uInt16, sal_uInt32 > aLegacyToNew( (sal_uInt8*)pLegacyPCode, nLegacyCodeSize );
                    aLegacyToNew.convert();
                    pCode = (char*)aLegacyToNew.GetBuffer();
                    nCodeSize = aLegacyToNew.GetSize();
                    // we don't release the legacy buffer
                    // right now, thats because the module
                    // needs it to fix up the method
                    // nStart members. When that is done
                    // the module can release the buffer
                    // or it can wait until this routine
                    // is called again or when this class                       // destructs all of which will trigger
                    // release of the buffer.
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
                    r >> nOff;
                    pStringOff[ i ] = (sal_uInt16) nOff;
                }
                r >> nLen;
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
                        String aStr( pByteStrings + nOff2, eCharSet );
                        memcpy( pStrings + nOff2, aStr.GetBuffer(), (aStr.Len() + 1) * sizeof( sal_Unicode ) );
                    }
                    delete[] pByteStrings;
                } break;
            case B_MODEND:
                goto done;
            default:
                break;
        }
        else
            break;
        r.Seek( nNext );
    }
done:
    r.Seek( nLast );
    if( !SbiGood( r ) )
        bError = true;
    return sal_Bool( !bError );
}

sal_Bool SbiImage::Save( SvStream& r, sal_uInt32 nVer )
{
    bool bLegacy = ( nVer < B_EXT_IMG_VERSION );

    // detect if old code exceeds legacy limits
    // if so, then disallow save
    if ( bLegacy && ExceedsLegacyLimits() )
    {
        SbiImage aEmptyImg;
        aEmptyImg.aName = aName;
        aEmptyImg.Save( r, B_LEGACYVERSION );
        return sal_True;
    }
    // First of all the header
    sal_uIntPtr nStart = SbiOpenRecord( r, B_MODULE, 1 );
    sal_uIntPtr nPos;

    eCharSet = GetSOStoreTextEncoding( eCharSet );
    if ( bLegacy )
        r << (sal_Int32) B_LEGACYVERSION;
    else
        r << (sal_Int32) B_CURVERSION;
    r  << (sal_Int32) eCharSet
      << (sal_Int32) nDimBase
      << (sal_Int16) nFlags
      << (sal_Int16) 0
      << (sal_Int32) 0
      << (sal_Int32) 0;

    // Name?
    if( aName.Len() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_NAME, 1 );
        r.WriteUniOrByteString( aName, eCharSet );
        SbiCloseRecord( r, nPos );
    }
    // Comment?
    if( aComment.Len() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_COMMENT, 1 );
        r.WriteUniOrByteString( aComment, eCharSet );
        SbiCloseRecord( r, nPos );
    }
    // Source?
    if( !aOUSource.isEmpty() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_SOURCE, 1 );
        String aTmp;
        sal_Int32 nLen = aOUSource.getLength();
        const sal_Int32 nMaxUnitSize = STRING_MAXLEN - 1;
        if( nLen > STRING_MAXLEN )
            aTmp = aOUSource.copy( 0, nMaxUnitSize );
        else
            aTmp = aOUSource;
        r.WriteUniOrByteString( aTmp, eCharSet );
        SbiCloseRecord( r, nPos );

        if( nLen > STRING_MAXLEN )
        {
            sal_Int32 nRemainingLen = nLen - nMaxUnitSize;
            sal_uInt16 nUnitCount = sal_uInt16( (nRemainingLen + nMaxUnitSize - 1) / nMaxUnitSize );
            nPos = SbiOpenRecord( r, B_EXTSOURCE, nUnitCount );
            for( sal_uInt16 i = 0 ; i < nUnitCount ; i++ )
            {
                sal_Int32 nCopyLen =
                    (nRemainingLen > nMaxUnitSize) ? nMaxUnitSize : nRemainingLen;
                String aTmp2 = aOUSource.copy( (i+1) * nMaxUnitSize, nCopyLen );
                nRemainingLen -= nCopyLen;
                r.WriteUniOrByteString( aTmp2, eCharSet );
            }
            SbiCloseRecord( r, nPos );
        }
    }
    // Binary data?
    if( pCode && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_PCODE, 1 );
        if ( bLegacy )
        {
            ReleaseLegacyBuffer(); // release any previously held buffer
            PCodeBuffConvertor< sal_uInt32, sal_uInt16 > aNewToLegacy( (sal_uInt8*)pCode, nCodeSize );
            aNewToLegacy.convert();
            pLegacyPCode = (char*)aNewToLegacy.GetBuffer();
            nLegacyCodeSize = aNewToLegacy.GetSize();
                r.Write( pLegacyPCode, nLegacyCodeSize );
        }
        else
            r.Write( pCode, nCodeSize );
        SbiCloseRecord( r, nPos );
    }
    // String-Pool?
    if( nStrings )
    {
        nPos = SbiOpenRecord( r, B_STRINGPOOL, nStrings );
        // For every String:
        //  sal_uInt32 Offset of the Strings in the Stringblock
        short i;

        for( i = 0; i < nStrings && SbiGood( r ); i++ )
            r << (sal_uInt32) pStringOff[ i ];

        // Then the String-Block
        char* pByteStrings = new char[ nStringSize ];
        for( i = 0; i < nStrings; i++ )
        {
            sal_uInt16 nOff = (sal_uInt16) pStringOff[ i ];
            rtl::OString aStr(rtl::OUStringToOString(rtl::OUString(pStrings + nOff), eCharSet));
            memcpy( pByteStrings + nOff, aStr.getStr(), (aStr.getLength() + 1) * sizeof( char ) );
        }
        r << (sal_uInt32) nStringSize;
        r.Write( pByteStrings, nStringSize );

        delete[] pByteStrings;
        SbiCloseRecord( r, nPos );
    }
    // Set overall length
    SbiCloseRecord( r, nStart );
    if( !SbiGood( r ) )
        bError = true;
    return sal_Bool( !bError );
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
        bError = true;
}

// Add a string to StringPool. The String buffer is dynamically
// growing in 1K-Steps
void SbiImage::AddString( const String& r )
{
    if( nStringIdx >= nStrings )
        bError = true;
    if( !bError )
    {
        xub_StrLen  len = r.Len() + 1;
        sal_uInt32 needed = nStringOff + len;
        if( needed > 0xFFFFFF00L )
            bError = true;  // out of mem!
        else if( needed > nStringSize )
        {
            sal_uInt32 nNewLen = needed + 1024;
            nNewLen &= 0xFFFFFC00;  // trim to 1K border
            if( nNewLen > 0xFFFFFF00L )
                nNewLen = 0xFFFFFF00L;
            sal_Unicode* p = NULL;
            if( (p = new sal_Unicode[ nNewLen ]) != NULL )
            {
                memcpy( p, pStrings, nStringSize * sizeof( sal_Unicode ) );
                delete[] pStrings;
                pStrings = p;
                nStringSize = sal::static_int_cast< sal_uInt16 >(nNewLen);
            }
            else
                bError = true;
        }
        if( !bError )
        {
            pStringOff[ nStringIdx++ ] = nStringOff;
            memcpy( pStrings + nStringOff, r.GetBuffer(), len * sizeof( sal_Unicode ) );
            nStringOff = nStringOff + len;
            // Last String? The update the size of the buffer
            if( nStringIdx >= nStrings )
                nStringSize = nStringOff;
        }
    }
}

// Add code block
// The block was fetched by the compiler from class SbBuffer and
// is already created with new. Additionally it contains all Integers
// in Big Endian format, so can be directly read/written.
void SbiImage::AddCode( char* p, sal_uInt32 s )
{
    pCode = p;
    nCodeSize = s;
}

// Add user type
void SbiImage::AddType(SbxObject* pObject)
{
    if( !rTypes.Is() )
        rTypes = new SbxArray;
    SbxObject *pCopyObject = new SbxObject(*pObject);
    rTypes->Insert (pCopyObject,rTypes->Count());
}

void SbiImage::AddEnum(SbxObject* pObject) // Register enum type
{
    if( !rEnums.Is() )
        rEnums = new SbxArray;
    rEnums->Insert( pObject, rEnums->Count() );
}


/**************************************************************************
*
*    Accessing the image
*
**************************************************************************/

// Note: IDs start with 1
String SbiImage::GetString( short nId ) const
{
    if( nId && nId <= nStrings )
    {
        sal_uInt32 nOff = pStringOff[ nId - 1 ];
        sal_Unicode* pStr = pStrings + nOff;

        // #i42467: Special treatment for vbNullChar
        if( *pStr == 0 )
        {
            sal_uInt32 nNextOff = (nId < nStrings) ? pStringOff[ nId ] : nStringOff;
            sal_uInt32 nLen = nNextOff - nOff - 1;
            if( nLen == 1 )
            {
                // Force length 1 and make char 0 afterwards
                String aNullCharStr( rtl::OUString(" ") );
                aNullCharStr.SetChar( 0, 0 );
                return aNullCharStr;
            }
        }
        else
            return rtl::OUString(pStr);
    }
    return rtl::OUString();
}

const SbxObject* SbiImage::FindType (String aTypeName) const
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

sal_Bool SbiImage::ExceedsLegacyLimits()
{
    if ( ( nStringSize > 0xFF00L ) || ( CalcLegacyOffset( nCodeSize ) > 0xFF00L ) )
        return sal_True;
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
