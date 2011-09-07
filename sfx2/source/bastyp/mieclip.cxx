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

    rtl::OString sLine, sVersion;
    sal_uIntPtr nStt = 0, nEnd = 0;
    sal_Int32 nIndex = 0;

    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    rStream.ResetError();

    if( rStream.ReadLine( sLine ) &&
        sLine.getToken( 0, ':', nIndex ) == "Version" )
    {
        sVersion = sLine.copy( nIndex );
        while( rStream.ReadLine( sLine ) )
        {
            nIndex = 0;
            rtl::OString sTmp(sLine.getToken(0, ':', nIndex));
            if (sTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("StartHTML")))
                nStt = (sal_uIntPtr)(sLine.copy(nIndex).toInt32());
            else if (sTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("EndHTML")))
                nEnd = (sal_uIntPtr)(sLine.copy(nIndex).toInt32());
            else if (sTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("SourceURL")))
                sBaseURL = S2U(sLine.copy(nIndex));

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
