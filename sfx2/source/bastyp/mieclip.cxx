/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mieclip.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:56:50 $
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

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif

#include <sot/storage.hxx>
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif

#include <mieclip.hxx>
#include "sfxuno.hxx"

MSE40HTMLClipFormatObj::~MSE40HTMLClipFormatObj()
{
    delete pStrm;
}

SvStream* MSE40HTMLClipFormatObj::IsValid( SvStream& rStream )
{
    BOOL bRet = FALSE;
    if( pStrm )
        delete pStrm, pStrm = 0;

    ByteString sLine, sVersion;
    ULONG nStt = 0, nEnd = 0;
    USHORT nIndex = 0;

    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    rStream.ResetError();

    if( rStream.ReadLine( sLine ) &&
        sLine.GetToken( 0, ':', nIndex ) == "Version" )
    {
        sVersion = sLine.Copy( nIndex );
        while( rStream.ReadLine( sLine ) )
        {
            nIndex = 0;
            ByteString sTmp( sLine.GetToken( 0, ':', nIndex ) );
            if( sTmp == "StartHTML" )
                nStt = (ULONG)(sLine.Erase( 0, nIndex ).ToInt32());
            else if( sTmp == "EndHTML" )
                nEnd = (ULONG)(sLine.Erase( 0, nIndex ).ToInt32());
            else if( sTmp == "SourceURL" )
                sBaseURL = String(S2U(sLine.Erase( 0, nIndex )));

            if( nEnd && nStt &&
                ( sBaseURL.Len() || rStream.Tell() >= nStt ))
            {
                bRet = TRUE;
                break;
            }
        }
    }

    if( bRet )
    {
        rStream.Seek( nStt );

        pStrm = new SvCacheStream( ( nEnd - nStt < 0x10000l
                                        ? nEnd - nStt + 32
                                        : 0 ));
        *pStrm << rStream;
        pStrm->SetStreamSize( nEnd - nStt + 1L );
        pStrm->Seek( STREAM_SEEK_TO_BEGIN );
    }

    return pStrm;
}

