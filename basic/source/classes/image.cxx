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
#include "precompiled_basic.hxx"
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
    bError     = sal_False;
    bFirstInit = sal_True;
    eCharSet   = gsl_getSystemTextEncoding();
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
    eCharSet   = gsl_getSystemTextEncoding();
    nDimBase   = 0;
    bError     = sal_False;
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

// If the version number does not find, binary parts are omitted, but not
// source, comments and name
sal_Bool SbiImage::Load( SvStream& r )
{
    sal_uInt32 nVersion = 0;        // Versionsnumber
    return Load( r, nVersion );
}
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
        short i;

        r >> nSign >> nLen >> nCount;
        nNext += nLen + 8;
        if( r.GetError() == SVSTREAM_OK )
          switch( nSign )
        {
            case B_NAME:
                r.ReadByteString( aName, eCharSet );
                //r >> aName;
                break;
            case B_COMMENT:
                r.ReadByteString( aComment, eCharSet );
                //r >> aComment;
                break;
            case B_SOURCE:
            {
                String aTmp;
                r.ReadByteString( aTmp, eCharSet );
                aOUSource = aTmp;
                //r >> aSource;
                break;
            }
#ifdef EXTENDED_BINARY_MODULES
            case B_EXTSOURCE:
            {
                for( sal_uInt16 j = 0 ; j < nCount ; j++ )
                {
                    String aTmp;
                    r.ReadByteString( aTmp, eCharSet );
                    aOUSource += aTmp;
                }
                break;
            }
#endif
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
    //if( eCharSet != ::GetSystemCharSet() )
        //ConvertStrings();
    if( !SbiGood( r ) )
        bError = sal_True;
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
        r.WriteByteString( aName, eCharSet );
        //r << aName;
        SbiCloseRecord( r, nPos );
    }
    // Comment?
    if( aComment.Len() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_COMMENT, 1 );
        r.WriteByteString( aComment, eCharSet );
        //r << aComment;
        SbiCloseRecord( r, nPos );
    }
    // Source?
    if( aOUSource.getLength() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_SOURCE, 1 );
        String aTmp;
        sal_Int32 nLen = aOUSource.getLength();
        const sal_Int32 nMaxUnitSize = STRING_MAXLEN - 1;
        if( nLen > STRING_MAXLEN )
            aTmp = aOUSource.copy( 0, nMaxUnitSize );
        else
            aTmp = aOUSource;
        r.WriteByteString( aTmp, eCharSet );
        //r << aSource;
        SbiCloseRecord( r, nPos );

#ifdef EXTENDED_BINARY_MODULES
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
                r.WriteByteString( aTmp2, eCharSet );
            }
            SbiCloseRecord( r, nPos );
        }
#endif
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
            ByteString aStr( pStrings + nOff, eCharSet );
            memcpy( pByteStrings + nOff, aStr.GetBuffer(), (aStr.Len() + 1) * sizeof( char ) );
        }
        r << (sal_uInt32) nStringSize;
        r.Write( pByteStrings, nStringSize );

        delete[] pByteStrings;
        SbiCloseRecord( r, nPos );
    }
    // Set overall length
    SbiCloseRecord( r, nStart );
    if( !SbiGood( r ) )
        bError = sal_True;
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
        bError = sal_True;
}

// Hinzufuegen eines Strings an den StringPool. Der String-Puffer
// waechst dynamisch in 1K-Schritten
// Add a string to StringPool. The String buffer is dynamically
// growing in 1K-Steps
void SbiImage::AddString( const String& r )
{
    if( nStringIdx >= nStrings )
        bError = sal_True;
    if( !bError )
    {
        xub_StrLen  len = r.Len() + 1;
        sal_uInt32 needed = nStringOff + len;
        if( needed > 0xFFFFFF00L )
            bError = sal_True;  // out of mem!
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
                bError = sal_True;
        }
        if( !bError )
        {
            pStringOff[ nStringIdx++ ] = nStringOff;
            //ByteString aByteStr( r, eCharSet );
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
                String aNullCharStr( String::CreateFromAscii( " " ) );
                aNullCharStr.SetChar( 0, 0 );
                return aNullCharStr;
            }
        }
        else
        {
            String aStr( pStr );
            return aStr;
        }
    }
    return String();
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
