/*************************************************************************
 *
 *  $RCSfile: storinfo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:56:52 $
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

#include <stg.hxx>
#include <storinfo.hxx>
#include <exchange.hxx>
#pragma hdrstop


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
    ULONG nFormat = 0;
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
            nFormat = SotExchange::RegisterFormatName( String::CreateFromAscii( p, nLen -1 ) );
        }
        else
            rStm.SetError( SVSTREAM_GENERALERROR );
        delete p;
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


