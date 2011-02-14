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
#include "precompiled_sfx2.hxx"
#include <tools/stream.hxx>
#include <tools/cachestr.hxx>

#include <sot/storage.hxx>
#include <sot/formats.hxx>

#include <sfx2/mieclip.hxx>
#include <sfx2/sfxuno.hxx>

MSE40HTMLClipFormatObj::~MSE40HTMLClipFormatObj()
{
    delete pStrm;
}

SvStream* MSE40HTMLClipFormatObj::IsValid( SvStream& rStream )
{
    sal_Bool bRet = sal_False;
    if( pStrm )
        delete pStrm, pStrm = 0;

    ByteString sLine, sVersion;
    sal_uIntPtr nStt = 0, nEnd = 0;
    sal_uInt16 nIndex = 0;

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
                nStt = (sal_uIntPtr)(sLine.Erase( 0, nIndex ).ToInt32());
            else if( sTmp == "EndHTML" )
                nEnd = (sal_uIntPtr)(sLine.Erase( 0, nIndex ).ToInt32());
            else if( sTmp == "SourceURL" )
                sBaseURL = String(S2U(sLine.Erase( 0, nIndex )));

            if( nEnd && nStt &&
                ( sBaseURL.Len() || rStream.Tell() >= nStt ))
            {
                bRet = sal_True;
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

