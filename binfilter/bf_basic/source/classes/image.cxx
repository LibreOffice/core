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

#include <tools/stream.hxx>
#include <tools/tenccvt.hxx>
#include "sbx.hxx"
#include "sb.hxx"
#include <string.h>		// memset() etc
#include "image.hxx"
#include "codegen.hxx"

namespace binfilter {

SbiImage::SbiImage()
{
    pStringOff = NULL;
    pStrings   = NULL;
    pCode  	   = NULL;
    pLegacyPCode  	   = NULL;
    nFlags	   = 0;
    nStrings   = 0;
    nStringSize= 0;
    nCodeSize  = 0;
    nLegacyCodeSize  =
    nDimBase   = 0;
    bInit	   =
    bError	   = FALSE;
    bFirstInit = TRUE;
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
    pCode  	   = NULL;
    nFlags	   = 0;
    nStrings   = 0;
    nStringSize= 0;
    nLegacyCodeSize  = 0;
    nCodeSize  = 0;
    eCharSet   = gsl_getSystemTextEncoding();
    nDimBase   = 0;
    bError	   = FALSE;
}

/**************************************************************************
*
*	Service-Routinen fuer das Laden und Speichern
*
**************************************************************************/

BOOL SbiGood( SvStream& r )
{
    return BOOL( !r.IsEof() && r.GetError() == SVSTREAM_OK );
}

// Oeffnen eines Records

ULONG SbiOpenRecord( SvStream& r, UINT16 nSignature, UINT16 nElem )
{
    ULONG nPos = r.Tell();
    r << nSignature << (INT32) 0 << nElem;
    return nPos;
}

// Schliessen eines Records

void SbiCloseRecord( SvStream& r, ULONG nOff )
{
    ULONG nPos = r.Tell();
    r.Seek( nOff + 2 );
    r << (INT32) ( nPos - nOff - 8 );
    r.Seek( nPos );
}

/**************************************************************************
*
*	Laden und Speichern
*
**************************************************************************/

// Falls die Versionsnummer nicht passt, werden die binaeren Teile
// nicht geladen, wohl aber Source, Kommentar und Name.

BOOL SbiImage::Load( SvStream& r, UINT32& nVersion )
{

    UINT16 nSign, nCount;
    UINT32 nLen, nOff;

    Clear();
    // Master-Record einlesen
    r >> nSign >> nLen >> nCount;
    ULONG nLast = r.Tell() + nLen;
    UINT32 nCharSet;			   	// System-Zeichensatz
    UINT32 lDimBase;
    UINT16 nReserved1;
    UINT32 nReserved2;
    UINT32 nReserved3;
    BOOL bBadVer = FALSE;
    if( nSign == B_MODULE )
    {
        r >> nVersion >> nCharSet >> lDimBase
          >> nFlags >> nReserved1 >> nReserved2 >> nReserved3;
        eCharSet = (CharSet) nCharSet;
        eCharSet = GetSOLoadTextEncoding( eCharSet );
        bBadVer  = BOOL( nVersion > B_CURVERSION );
        nDimBase = (USHORT) lDimBase;
    }

    bool bLegacy = ( nVersion < B_EXT_IMG_VERSION );

    ULONG nNext;
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
                for( UINT16 j = 0 ; j < nCount ; j++ )
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
                    nLegacyCodeSize = (UINT16) nCodeSize;
                    pLegacyPCode = pCode;

                    PCodeBuffConvertor< UINT16, UINT32 > aLegacyToNew( (BYTE*)pLegacyPCode, nLegacyCodeSize );
                    aLegacyToNew.convert();
                    pCode = (char*)aLegacyToNew.GetBuffer();
                    nCodeSize = aLegacyToNew.GetSize();
                    // we don't release the legacy buffer
                    // right now, thats because the module
                    // needs it to fix up the method
                    // nStart members. When that is done
                    // the module can release the buffer
                    // or it can wait until this routine
                    // is called again or when this class						// destructs all of which will trigger
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
                    pStringOff[ i ] = (USHORT) nOff;
                }
                r >> nLen;
                if( SbiGood( r ) )
                {
                    delete [] pStrings;
                    pStrings = new sal_Unicode[ nLen ];
                    nStringSize = (USHORT) nLen;

                    char* pByteStrings = new char[ nLen ];
                    r.Read( pByteStrings, nStringSize );
                    for( short j = 0; j < nStrings; j++ )
                    {
                        USHORT nOff2 = (USHORT) pStringOff[ j ];
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
        bError = TRUE;
    return BOOL( !bError );
}

BOOL SbiImage::Save( SvStream& r, UINT32 nVer )
{
    bool bLegacy = ( nVer < B_EXT_IMG_VERSION );

    // detect if old code exceeds legacy limits
    // if so, then disallow save
    if ( bLegacy && ExceedsLegacyLimits() )
    {
        SbiImage aEmptyImg;
        aEmptyImg.aName = aName;
        aEmptyImg.Save( r, B_LEGACYVERSION );	      		
        return TRUE;
    }
    // Erst mal der Header:
    ULONG nStart = SbiOpenRecord( r, B_MODULE, 1 );
    ULONG nPos;

    eCharSet = GetSOStoreTextEncoding( eCharSet );
    if ( bLegacy )
        r << (INT32) B_LEGACYVERSION;
    else
        r << (INT32) B_CURVERSION;
    r  << (INT32) eCharSet
      << (INT32) nDimBase
      << (INT16) nFlags
      << (INT16) 0
      << (INT32) 0
      << (INT32) 0;

    // Name?
    if( aName.Len() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_NAME, 1 );
        r.WriteByteString( aName, eCharSet );
        //r << aName;
        SbiCloseRecord( r, nPos );
    }
    // Kommentar?
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
            UINT16 nUnitCount = UINT16( (nRemainingLen + nMaxUnitSize - 1) / nMaxUnitSize );
            nPos = SbiOpenRecord( r, B_EXTSOURCE, nUnitCount );
            for( UINT16 i = 0 ; i < nUnitCount ; i++ )
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
    // Binaere Daten?
    if( pCode && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_PCODE, 1 );
        if ( bLegacy )
        {
            ReleaseLegacyBuffer(); // release any previously held buffer
            PCodeBuffConvertor< UINT32, UINT16 > aNewToLegacy( (BYTE*)pCode, nCodeSize );
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
        // Fuer jeden String:
        //	UINT32 Offset des Strings im Stringblock
        short i;

        for( i = 0; i < nStrings && SbiGood( r ); i++ )
            r << (UINT32) pStringOff[ i ];

        // Danach der String-Block
        char* pByteStrings = new char[ nStringSize ];
        for( i = 0; i < nStrings; i++ )
        {
            USHORT nOff = (USHORT) pStringOff[ i ];
            ByteString aStr( pStrings + nOff, eCharSet );
            memcpy( pByteStrings + nOff, aStr.GetBuffer(), (aStr.Len() + 1) * sizeof( char ) );
        }
        r << (UINT32) nStringSize;
        r.Write( pByteStrings, nStringSize );

        delete[] pByteStrings;
        SbiCloseRecord( r, nPos );
    }
    // Und die Gesamtlaenge setzen
    SbiCloseRecord( r, nStart );
    if( !SbiGood( r ) )
        bError = TRUE;
    return BOOL( !bError );
}

/**************************************************************************
*
*	Routinen, die auch vom Compiler gerufen werden
*
**************************************************************************/

void SbiImage::MakeStrings( short nSize )
{
    nStrings = 0;
    nStringIdx = 0;
    nStringOff = 0;
    nStringSize = 1024;
    pStrings = new sal_Unicode[ nStringSize ];
    pStringOff = new UINT32[ nSize ];
    if( pStrings && pStringOff )
    {
        nStrings = nSize;
        memset( pStringOff, 0, nSize * sizeof( UINT32 ) );
        memset( pStrings, 0, nStringSize * sizeof( sal_Unicode ) );
    }
    else
        bError = TRUE;
}

/**************************************************************************
*
*	Zugriffe auf das Image
*
**************************************************************************/

const SbxObject* SbiImage::FindType (String aTypeName) const
{
    return rTypes.Is() ? (SbxObject*)rTypes->Find(aTypeName,SbxCLASS_OBJECT) : NULL;
}

UINT16
SbiImage::CalcLegacyOffset( INT32 nOffset )
{
    return SbiCodeGen::calcLegacyOffSet( (BYTE*)pCode, nOffset ) ;
}
UINT32
SbiImage::CalcNewOffset( INT16 nOffset )
{
    return SbiCodeGen::calcNewOffSet( (BYTE*)pLegacyPCode, nOffset ) ;
}

void 
SbiImage::ReleaseLegacyBuffer()
{
    delete[] pLegacyPCode;
    pLegacyPCode = NULL;
    nLegacyCodeSize = 0;
}

BOOL 
SbiImage::ExceedsLegacyLimits()
{
    if ( ( nStringSize > 0xFF00L ) || ( CalcLegacyOffset( nCodeSize ) > 0xFF00L ) )
        return TRUE;
    return FALSE;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
