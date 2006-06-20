/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: storinfo.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:55:45 $
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

#include <stg.hxx>
#include <storinfo.hxx>
#include <exchange.hxx>


/************** class SvStorageInfoList **********************************
*************************************************************************/
PRV_SV_IMPL_OWNER_LIST(SvStorageInfoList,SvStorageInfo)

const SvStorageInfo * SvStorageInfoList::Get( const String & rEleName )
{
    for( ULONG i = 0; i < Count(); i++ )
    {
        const SvStorageInfo & rType = GetObject( i );
        if( rType.GetName() == rEleName )
            return &rType;
    }
    return NULL;
}

/************** class SvStorageInfo **************************************
*************************************************************************/
ULONG ReadClipboardFormat( SvStream & rStm )
{
    sal_uInt32 nFormat = 0;
    INT32 nLen = 0;
    rStm >> nLen;
    if( rStm.IsEof() )
        rStm.SetError( SVSTREAM_GENERALERROR );
    if( nLen > 0 )
    {
        // get a string name
        sal_Char * p = new sal_Char[ nLen ];
        if( rStm.Read( p, nLen ) == (ULONG) nLen )
        {
            nFormat = SotExchange::RegisterFormatName( String::CreateFromAscii( p, short(nLen-1) ) );
        }
        else
            rStm.SetError( SVSTREAM_GENERALERROR );
        delete [] p;
    }
    else if( nLen == -1L )
        // Windows clipboard format
        // SV und Win stimmen ueberein (bis einschl. FORMAT_GDIMETAFILE)
        rStm >> nFormat;
    else if( nLen == -2L )
    {
        rStm >> nFormat;
        // Mac clipboard format
        // ??? not implemented
        rStm.SetError( SVSTREAM_GENERALERROR );
    }
    else if( nLen != 0 )
    {
        // unknown identifier
        rStm.SetError( SVSTREAM_GENERALERROR );
    }
    return nFormat;
}

void WriteClipboardFormat( SvStream & rStm, ULONG nFormat )
{
    // determine the clipboard format string
    String aCbFmt;
    if( nFormat > FORMAT_GDIMETAFILE )
        aCbFmt = SotExchange::GetFormatName( nFormat );
    if( aCbFmt.Len() )
    {
        ByteString aAsciiCbFmt( aCbFmt, RTL_TEXTENCODING_ASCII_US );
        rStm << (INT32) (aAsciiCbFmt.Len() + 1);
        rStm << (const char *)aAsciiCbFmt.GetBuffer();
        rStm << (UINT8) 0;
    }
    else if( nFormat )
        rStm << (INT32) -1         // for Windows
             << (INT32) nFormat;
    else
        rStm << (INT32) 0;         // no clipboard format
}


