/*************************************************************************
 *
 *  $RCSfile: image.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#pragma hdrstop
#include <svtools/sbx.hxx>
#include "sb.hxx"
#include <string.h>     // memset() etc
#include "image.hxx"
#include "filefmt.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBASIC, SBASIC_CODE )

SbiImage::SbiImage()
{
    rTypes     = new SbxArray;
    pStringOff = NULL;
    pStrings   = NULL;
    pCode      = NULL;
    nFlags     =
    nStrings   =
    nStringSize=
    nCodeSize  =
    nDimBase   = 0;
    bInit      =
    bError     = FALSE;
    eCharSet   = gsl_getSystemTextEncoding();
}

SbiImage::~SbiImage()
{
    Clear();
}

void SbiImage::Clear()
{
    delete pStringOff;
    delete pStrings;
    delete pCode;
    pStringOff = NULL;
    pStrings   = NULL;
    pCode      = NULL;
    nFlags     =
    nStrings   =
    nStringSize=
    nCodeSize  = 0;
    eCharSet   = gsl_getSystemTextEncoding();
    nDimBase   = 0;
    bError     = FALSE;
}

/**************************************************************************
*
*   Service-Routinen fuer das Laden und Speichern
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
*   Laden und Speichern
*
**************************************************************************/

// Falls die Versionsnummer nicht passt, werden die binaeren Teile
// nicht geladen, wohl aber Source, Kommentar und Name.

BOOL SbiImage::Load( SvStream& r )
{
    UINT16 nSign, nCount;
    UINT32 nLen, nOff;

    Clear();
    ULONG nStart = r.Tell();
    // Master-Record einlesen
    r >> nSign >> nLen >> nCount;
    ULONG nLast = r.Tell() + nLen;
    UINT32 nVersion = 0;            // Versionsnummer
    UINT32 nCharSet;                // System-Zeichensatz
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
        bBadVer  = BOOL( nVersion != B_CURVERSION );
        nDimBase = (USHORT) lDimBase;
    }

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
                r.ReadByteString( aSource, eCharSet );
                //r >> aSource;
                break;
            case B_PCODE:
                if( bBadVer ) break;
                pCode = new char[ nLen ];
                nCodeSize = (USHORT) nLen;
                r.Read( pCode, nCodeSize );
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
                    delete pStrings;
                    pStrings = new sal_Unicode[ nLen ];
                    nStringSize = (USHORT) nLen;

                    char* pByteStrings = new char[ nLen ];
                    r.Read( pByteStrings, nStringSize );
                    for( short i = 0; i < nStrings; i++ )
                    {
                        USHORT nOff = pStringOff[ i ];
                        String aStr( pByteStrings + nOff, eCharSet );
                        memcpy( pStrings + nOff, aStr.GetBuffer(), (aStr.Len() + 1) * sizeof( sal_Unicode ) );
                    }
                    delete pByteStrings;
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

BOOL SbiImage::Save( SvStream& r )
{
    // Erst mal der Header:
    ULONG nStart = SbiOpenRecord( r, B_MODULE, 1 );
    ULONG nPos;
    r << (INT32) B_CURVERSION
      << (INT32) eCharSet
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
    if( aSource.Len() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_SOURCE, 1 );
        r.WriteByteString( aSource, eCharSet );
        //r << aSource;
        SbiCloseRecord( r, nPos );
    }
    // Binaere Daten?
    if( pCode && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, B_PCODE, 1 );
        r.Write( pCode, nCodeSize );
        SbiCloseRecord( r, nPos );
    }
    // String-Pool?
    if( nStrings )
    {
        nPos = SbiOpenRecord( r, B_STRINGPOOL, nStrings );
        // Fuer jeden String:
        //  UINT32 Offset des Strings im Stringblock
        for( short i = 0; i < nStrings && SbiGood( r ); i++ )
            r << (UINT32) pStringOff[ i ];

        // Danach der String-Block
        char* pByteStrings = new char[ nStringSize ];
        for( i = 0; i < nStrings; i++ )
        {
            USHORT nOff = pStringOff[ i ];
            ByteString aStr( pStrings + nOff, eCharSet );
            memcpy( pByteStrings + nOff, aStr.GetBuffer(), (aStr.Len() + 1) * sizeof( char ) );
        }
        r << (UINT32) nStringSize;
        r.Write( pByteStrings, nStringSize );
        delete pByteStrings;
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
*   Routinen, die auch vom Compiler gerufen werden
*
**************************************************************************/

void SbiImage::MakeStrings( short nSize )
{
    nStrings = nStringIdx = nStringOff = 0;
    nStringSize = 1024;
    pStrings = new sal_Unicode[ nStringSize ];
    pStringOff = new UINT16[ nSize ];
    if( pStrings && pStringOff )
    {
        nStrings = nSize;
        memset( pStringOff, 0, nSize * sizeof( UINT16 ) );
        memset( pStrings, 0, nStringSize * sizeof( sal_Unicode ) );
    }
    else
        bError = TRUE;
}

// Hinzufuegen eines Strings an den StringPool. Der String-Puffer
// waechst dynamisch in 1K-Schritten

// AB 12.5.2000 Aus Zeitgruenden vorerst weiter auf char-Basis
// TODO: Auch hier auf Unicode umstellen, d.h. sal_Unicode-Array
void SbiImage::AddString( const String& r )
{
    if( nStringIdx >= nStrings )
        bError = TRUE;
    if( !bError )
    {
        UINT16 len = r.Len() + 1;
        long needed = (long) nStringOff + len;
        if( needed > 0xFF00L )
            bError = TRUE;  // out of mem!
        else if( (USHORT) needed > nStringSize )
        {
            sal_Unicode* p = new sal_Unicode[ nStringSize + 1024 ];
            if( p )
            {
                memcpy( p, pStrings, nStringSize * sizeof( sal_Unicode ) );
                delete pStrings;
                pStrings = p;
                nStringSize += 1024;
            }
            else
                bError = TRUE;
        }
        if( !bError )
        {
            pStringOff[ nStringIdx++ ] = nStringOff;
            //ByteString aByteStr( r, eCharSet );
            memcpy( pStrings + nStringOff, r.GetBuffer(), len * sizeof( sal_Unicode ) );
            nStringOff += len;
            // war das der letzte String? Dann die Groesse
            // des Puffers aktualisieren
            if( nStringIdx >= nStrings )
                nStringSize = nStringOff;
        }
    }
}

// Codeblock hinzufuegen
// Der Block wurde vom Compiler aus der Klasse SbBuffer herausgeholt
// und ist bereits per new angelegt. Ausserdem enthaelt er alle Integers
// im Big Endian-Format, kann also direkt gelesen/geschrieben werden.

void SbiImage::AddCode( char* p, USHORT s )
{
    pCode = p;
    nCodeSize = s;
}

void SbiImage::AddType(SbxObject* pObject) // User-Type mit aufnehmen
{
    SbxObject *pCopyObject = new SbxObject(*pObject);
    rTypes->Insert (pCopyObject,rTypes->Count());
}

/**************************************************************************
*
*   Zugriffe auf das Image
*
**************************************************************************/

// IDs zaehlen ab 1!!

String SbiImage::GetString( short nId ) const
{
    if( nId && nId <= nStrings )
    {
        USHORT nOff = pStringOff[ --nId ];
        String aStr( pStrings + nOff );
        return aStr;
    }
    return String();
}

const SbxObject* SbiImage::FindType (String aTypeName) const
{
        return (SbxObject *)rTypes->Find(aTypeName,SbxCLASS_OBJECT);
}

