/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stgole.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:55:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "rtl/string.h"
#include "rtl/string.h"
#include "stgole.hxx"
#include "storinfo.hxx"     // Read/WriteClipboardFormat()

#include <tools/debug.hxx>

///////////////////////// class StgInternalStream ////////////////////////

StgInternalStream::StgInternalStream
    ( BaseStorage& rStg, const String& rName, BOOL bWr )
{
    bIsWritable = TRUE;
    USHORT nMode = bWr
                 ? STREAM_WRITE | STREAM_SHARE_DENYALL
                 : STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE;
    pStrm = rStg.OpenStream( rName, nMode );

    // set the error code right here in the stream
    SetError( rStg.GetError() );
    SetBufferSize( 1024 );
}

StgInternalStream::~StgInternalStream()
{
    delete pStrm;
}

ULONG StgInternalStream::GetData( void* pData, ULONG nSize )
{
    if( pStrm )
    {
        nSize = pStrm->Read( pData, nSize );
        SetError( pStrm->GetError() );
        return nSize;
    }
    else
        return 0;
}

ULONG StgInternalStream::PutData( const void* pData, ULONG nSize )
{
    if( pStrm )
    {
        nSize = pStrm->Write( pData, nSize );
        SetError( pStrm->GetError() );
        return nSize;
    }
    else
        return 0;
}

ULONG StgInternalStream::SeekPos( ULONG nPos )
{
    return pStrm ? pStrm->Seek( nPos ) : 0;
}

void StgInternalStream::FlushData()
{
    if( pStrm )
    {
        pStrm->Flush();
        SetError( pStrm->GetError() );
    }
}

void StgInternalStream::Commit()
{
    Flush();
    pStrm->Commit();
}

///////////////////////// class StgCompObjStream /////////////////////////

StgCompObjStream::StgCompObjStream( BaseStorage& rStg, BOOL bWr )
            : StgInternalStream( rStg, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1CompObj" ) ), bWr )
{
    memset( &aClsId, 0, sizeof( ClsId ) );
    nCbFormat = 0;
}

BOOL StgCompObjStream::Load()
{
    memset( &aClsId, 0, sizeof( ClsId ) );
    nCbFormat = 0;
    aUserName.Erase();
    if( GetError() != SVSTREAM_OK )
        return FALSE;
    Seek( 8L );     // skip the first part
    INT32 nMarker = 0;
    *this >> nMarker;
    if( nMarker == -1L )
    {
        *this >> aClsId;
        INT32 nLen1 = 0;
        *this >> nLen1;
        // higher bits are ignored
        nLen1 &= 0xFFFF;
        sal_Char* p = new sal_Char[ (USHORT) nLen1 ];
        if( Read( p, nLen1 ) == (ULONG) nLen1 )
        {
            aUserName = nLen1 ? String( p, gsl_getSystemTextEncoding() ) : String();
/*          // Now we can read the CB format
            INT32 nLen2 = 0;
            *this >> nLen2;
            if( nLen2 > 0 )
            {
                // get a string name
                if( nLen2 > nLen1 )
                    delete p, p = new char[ nLen2 ];
                if( Read( p, nLen2 ) == (ULONG) nLen2 && nLen2 )
                    nCbFormat = Exchange::RegisterFormatName( String( p ) );
                else
                    SetError( SVSTREAM_GENERALERROR );
            }
            else if( nLen2 == -1L )
                // Windows clipboard format
                *this >> nCbFormat;
            else
                // unknown identifier
                SetError( SVSTREAM_GENERALERROR );
*/
            nCbFormat = ReadClipboardFormat( *this );
        }
        else
            SetError( SVSTREAM_GENERALERROR );
        delete [] p;
    }
    return BOOL( GetError() == SVSTREAM_OK );
}

BOOL StgCompObjStream::Store()
{
    if( GetError() != SVSTREAM_OK )
        return FALSE;
    Seek( 0L );
    ByteString aAsciiUserName( aUserName, RTL_TEXTENCODING_ASCII_US );
    *this << (INT16) 1          // Version?
              << (INT16) -2                     // 0xFFFE = Byte Order Indicator
              << (INT32) 0x0A03         // Windows 3.10
              << (INT32) -1L
              << aClsId             // Class ID
              << (INT32) (aAsciiUserName.Len() + 1)
              << (const char *)aAsciiUserName.GetBuffer()
              << (UINT8) 0;             // string terminator
/*  // determine the clipboard format string
    String aCbFmt;
    if( nCbFormat > FORMAT_GDIMETAFILE )
    aCbFmt = Exchange::GetFormatName( nCbFormat );
    if( aCbFmt.Len() )
        *this << (INT32) aCbFmt.Len() + 1
               << (const char*) aCbFmt
               << (UINT8) 0;
    else if( nCbFormat )
         *this << (INT32) -1            // for Windows
                << (INT32) nCbFormat;
    else
        *this << (INT32) 0;         // no clipboard format
*/
    WriteClipboardFormat( *this, nCbFormat );
    *this << (INT32) 0;             // terminator
    Commit();
    return BOOL( GetError() == SVSTREAM_OK );
}

/////////////////////////// class StgOleStream ///////////////////////////

StgOleStream::StgOleStream( BaseStorage& rStg, BOOL bWr )
            : StgInternalStream( rStg, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\1Ole" ) ), bWr )
{
    nFlags = 0;
}

BOOL StgOleStream::Load()
{
    nFlags = 0;
    if( GetError() != SVSTREAM_OK )
        return FALSE;
    INT32 version = 0;
    Seek( 0L );
    *this >> version >> nFlags;
    return BOOL( GetError() == SVSTREAM_OK );
}

BOOL StgOleStream::Store()
{
    if( GetError() != SVSTREAM_OK )
        return FALSE;
    Seek( 0L );
    *this << (INT32) 0x02000001         // OLE version, format
          << (INT32) nFlags             // Object flags
          << (INT32) 0                  // Update Options
          << (INT32) 0                  // reserved
          << (INT32) 0;                 // Moniker 1
    Commit();
    return BOOL( GetError() == SVSTREAM_OK );
}

