/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 by Sun Microsystems, Inc.
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
#include "precompiled_shell.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <stdio.h>
#include <objidl.h>

/*#include <string.h>
#include <filter.h>
#include <filterr.h>
#include <ntquery.h>
#include "assert.h"
#include "propspec.hxx"
#ifdef __MINGW32__
#include <algorithm>
using ::std::min;
#endif
*/

#include "internal/stream_helper.hxx"

extern "C" {
    voidpf ZCALLBACK cb_sopen OF((voidpf opaque, const char * filename, int mode));
    uLong ZCALLBACK cb_sread OF((voidpf opaque, voidpf stream, void* vuf, uLong size));
    uLong ZCALLBACK cb_swrite OF((voidpf opaque, voidpf stream, const void* buf, uLong size));
    long ZCALLBACK cb_stell OF((voidpf opaque, voidpf stream));
    long ZCALLBACK cb_sseek OF((voidpf opaque, voidpf stream, uLong offset, int origin));
    int ZCALLBACK cb_sclose OF((voidpf opaque, voidpf stream));
    int ZCALLBACK cb_serror OF((voidpf opaque, voidpf stream));

    void fill_stream_filefunc (zlib_filefunc_def* pzlib_filefunc_def);
}

//-----------------------------
IStream* PrepareIStream( IStream* pStream, zlib_filefunc_def &zFileFunc )
{
    // These next few lines work around the "Seek pointer" bug found on Vista.
    char cBuf[20];
    unsigned long nCount;
    HRESULT hr;
    ULARGE_INTEGER nNewPosition;
    LARGE_INTEGER nMove;
    nMove.QuadPart = 0;
    hr = pStream->Seek( nMove, STREAM_SEEK_SET, &nNewPosition );
    hr = pStream->Read( cBuf, 20, &nCount );

    fill_stream_filefunc( &zFileFunc );
    zFileFunc.opaque = (void*)pStream;

    return pStream;
}

extern "C" {

    // IStream callback
    voidpf ZCALLBACK cb_sopen (voidpf opaque, const char* /*filename*/, int /*mode*/) {
        return opaque;
    }

    uLong ZCALLBACK cb_sread (voidpf /*opaque*/, voidpf stream, void* buf, uLong size) {
        unsigned long newsize;
        HRESULT hr;

        hr = ((IStream *)stream)->Read (buf, size, &newsize);
        if (hr == S_OK){
            return (unsigned long)newsize;
        }
        else {
            return (uLong)0;
        }
    }

    long ZCALLBACK cb_sseek (voidpf /*opaque*/, voidpf stream, uLong offset, int origin) {
        // IStream::Seek parameters
        HRESULT hr;
        LARGE_INTEGER Move;
        DWORD dwOrigin;
        Move.QuadPart = (__int64)offset;

        switch (origin) {
            case SEEK_CUR:
                dwOrigin = STREAM_SEEK_CUR;
                break;
            case SEEK_END:
                dwOrigin = STREAM_SEEK_END;
                break;
            case SEEK_SET:
                dwOrigin = STREAM_SEEK_SET;
                break;
            default:
                return -1;
        }

        hr = ((IStream*)stream)->Seek (Move, dwOrigin, NULL);
        if (hr == S_OK){
            return 0;
        }
        else {
            return -1;
        }
    }

    long ZCALLBACK cb_stell (voidpf /*opaque*/, voidpf stream) {
        // IStream::Seek parameters
        HRESULT hr;
        LARGE_INTEGER Move;
        ULARGE_INTEGER NewPosition;
        Move.QuadPart = 0;
        NewPosition.QuadPart = 0;

        hr = ((IStream*)stream)->Seek (Move, STREAM_SEEK_CUR, &NewPosition);
        if (hr == S_OK){
            return (long) NewPosition.QuadPart;
        }
        else {
            return -1;
        }
    }

    int ZCALLBACK cb_sclose (voidpf /*opaque*/, voidpf /*stream*/) {
        return 0;
    }

    int ZCALLBACK cb_serror (voidpf /*opaque*/, voidpf /*stream*/) {
        return 0;  //RJK - for now
    }

    uLong ZCALLBACK cb_swrite (voidpf /*opaque*/, voidpf stream, const void* buf, uLong size) {
        HRESULT hr;
        unsigned long writecount;
        hr = ((IStream*)stream)->Write (buf, size, &writecount);
        if (hr == S_OK)
            return (unsigned int)writecount;
        else
            return (uLong)0;
    }

    void fill_stream_filefunc (zlib_filefunc_def* pzlib_filefunc_def) {
        pzlib_filefunc_def->zopen_file = cb_sopen;
        pzlib_filefunc_def->zread_file = cb_sread;
        pzlib_filefunc_def->zwrite_file = cb_swrite;
        pzlib_filefunc_def->ztell_file = cb_stell;
        pzlib_filefunc_def->zseek_file = cb_sseek;
        pzlib_filefunc_def->zclose_file = cb_sclose;
        pzlib_filefunc_def->zerror_file = cb_serror;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
