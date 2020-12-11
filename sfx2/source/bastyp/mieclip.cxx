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

#include <sal/config.h>

#include <o3tl/safeint.hxx>
#include <tools/stream.hxx>

#include <sfx2/mieclip.hxx>

MSE40HTMLClipFormatObj::~MSE40HTMLClipFormatObj()
{
}

SvStream* MSE40HTMLClipFormatObj::IsValid( SvStream& rStream )
{
    bool bRet = false;
    pStrm.reset();

    OString sLine;
    sal_Int32 nStt = -1, nEnd = -1, nFragStart = -1, nFragEnd = -1;
    sal_Int32 nIndex = 0;

    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    rStream.ResetError();

    if( rStream.ReadLine( sLine ) &&
        sLine.getToken( 0, ':', nIndex ) == "Version" )
    {
        while( rStream.ReadLine( sLine ) )
        {
            nIndex = 0;
            OString sTmp(sLine.getToken(0, ':', nIndex));
            if (sTmp == "StartHTML")
                nStt = sLine.copy(nIndex).toInt32();
            else if (sTmp == "EndHTML")
                nEnd = sLine.copy(nIndex).toInt32();
            else if (sTmp == "StartFragment")
                nFragStart = sLine.copy(nIndex).toInt32();
            else if (sTmp == "EndFragment")
                nFragEnd = sLine.copy(nIndex).toInt32();
            else if (sTmp == "SourceURL")
                sBaseURL = OStringToOUString( sLine.subView(nIndex), RTL_TEXTENCODING_UTF8 );

            if (nEnd >= 0 && nStt >= 0 &&
                (!sBaseURL.isEmpty() || rStream.Tell() >= o3tl::make_unsigned(nStt)))
            {
                bRet = true;
                break;
            }
        }
    }

    if( bRet )
    {
        rStream.Seek( nStt );

        pStrm.reset( new SvMemoryStream( ( nEnd - nStt < 0x10000l
                                        ? nEnd - nStt + 32
                                        : 0 )) );
        pStrm->WriteStream( rStream );
        pStrm->SetStreamSize( nEnd - nStt + 1 );
        pStrm->Seek( STREAM_SEEK_TO_BEGIN );
        return pStrm.get();
    }

    if (nFragStart > 0 && nFragEnd > 0 && nFragEnd > nFragStart)
    {
        size_t nSize = nFragEnd - nFragStart + 1;
        if (nSize < 0x10000L)
        {
            rStream.Seek(nFragStart);
            pStrm.reset( new SvMemoryStream(nSize) );
            pStrm->WriteStream( rStream );
            pStrm->SetStreamSize(nSize);
            pStrm->Seek(STREAM_SEEK_TO_BEGIN);
            return pStrm.get();
        }
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
