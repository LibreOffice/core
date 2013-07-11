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
    bool bRet = false;
    if( pStrm )
        delete pStrm, pStrm = 0;

    OString sLine, sVersion;
    sal_Int32 nStt = -1, nEnd = -1, nFragStart = -1, nFragEnd = -1;
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
            OString sTmp(sLine.getToken(0, ':', nIndex));
            if (sTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("StartHTML")))
                nStt = sLine.copy(nIndex).toInt32();
            else if (sTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("EndHTML")))
                nEnd = sLine.copy(nIndex).toInt32();
            else if (sTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("StartFragment")))
                nFragStart = sLine.copy(nIndex).toInt32();
            else if (sTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("EndFragment")))
                nFragEnd = sLine.copy(nIndex).toInt32();
            else if (sTmp.equalsL(RTL_CONSTASCII_STRINGPARAM("SourceURL")))
                sBaseURL = S2U(sLine.copy(nIndex));

            if (nEnd >= 0 && nStt >= 0 &&
                (sBaseURL.Len() || rStream.Tell() >= static_cast<sal_Size>(nStt)))
            {
                bRet = true;
                break;
            }
        }
    }

    if( bRet )
    {
        rStream.Seek( nStt );

        pStrm = new SvMemoryStream( ( nEnd - nStt < 0x10000l
                                        ? nEnd - nStt + 32
                                        : 0 ));
        *pStrm << rStream;
        pStrm->SetStreamSize( nEnd - nStt + 1L );
        pStrm->Seek( STREAM_SEEK_TO_BEGIN );
        return pStrm;
    }

    if (nFragStart > 0 && nFragEnd > 0 && nFragEnd > nFragStart)
    {
        sal_uIntPtr nSize = static_cast<sal_uIntPtr>(nFragEnd - nFragStart + 1);
        if (nSize < 0x10000L)
        {
            rStream.Seek(nFragStart);
            pStrm = new SvMemoryStream(nSize);
            *pStrm << rStream;
            pStrm->SetStreamSize(nSize);
            pStrm->Seek(STREAM_SEEK_TO_BEGIN);
            return pStrm;
        }
    }

    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
