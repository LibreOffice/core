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


#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <stdio.h>
#include <objidl.h>
#include "internal/stream_helper.hxx"

BufferStream::BufferStream(IStream *str) :
    stream(str)
{
    // These next few lines work around the "Seek pointer" bug found on Vista.
    char cBuf[20];
    unsigned long nCount;
    ULARGE_INTEGER nNewPosition;
    LARGE_INTEGER nMove;
    nMove.QuadPart = 0;
    stream->Seek( nMove, STREAM_SEEK_SET, &nNewPosition );
    stream->Read( cBuf, 20, &nCount );
}

BufferStream::~BufferStream()
{
}

unsigned long BufferStream::sread (unsigned char *buf, unsigned long size)
{
    unsigned long newsize;
    HRESULT hr;

    hr = stream->Read (buf, size, &newsize);
    if (hr == S_OK)
        return (unsigned long)newsize;
    else
        return (unsigned long)0;
}

long BufferStream::stell ()
{
    HRESULT hr;
    LARGE_INTEGER Move;
    ULARGE_INTEGER NewPosition;
    Move.QuadPart = 0;
    NewPosition.QuadPart = 0;

    hr = ((IStream *)stream)->Seek (Move, STREAM_SEEK_CUR, &NewPosition);
    if (hr == S_OK)
        return (long) NewPosition.QuadPart;
    else
        return -1;
}

long BufferStream::sseek (unsigned long offset, int origin)
{
    HRESULT hr;
    LARGE_INTEGER Move;
    DWORD dwOrigin;
    Move.QuadPart = (__int64)offset;

    switch (origin)
    {
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

    hr = stream->Seek (Move, dwOrigin, NULL);
    if (hr == S_OK)
        return 0;
    else
        return -1;
}

FileStream::FileStream(const char *filename) :
    file(0)
{
    file = fopen(filename, "rb");
}

FileStream::~FileStream()
{
    if (file)
        fclose(file);
}

unsigned long FileStream::sread (unsigned char *buf, unsigned long size)
{
    if (file)
        return fread(buf, 1, size, file);
    return 0;
}

long FileStream::stell ()
{
    if (file)
        return ftell(file);
    return -1L;
}

long FileStream::sseek (unsigned long offset, int origin)
{
    if (file)
        return fseek(file, offset, origin);
    return -1L;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
