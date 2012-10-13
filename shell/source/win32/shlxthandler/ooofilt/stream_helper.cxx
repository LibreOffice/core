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
        return static_cast<unsigned long>(fread(buf, 1, size, file));
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
