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

#include <stdio.h>
#include <errno.h>

#ifdef _WIN32
# include <io.h>
#else
# include <unistd.h>
#endif


#include "hwplib.h"
#include "hgzip.h"
#include "hiodev.h"
#include "hwpfile.h"
#include "hstream.hxx"

const size_t BUFSIZE = 1024;
static uchar rBuf[BUFSIZE];

// HIODev abstract class
HIODev::HIODev()
{
    init();
}


HIODev::~HIODev()
{
}


void HIODev::init()
{
    compressed = false;
}

size_t HIODev::read2b(void *ptr, size_t nmemb)
{
    ushort *p = static_cast<ushort *>(ptr);

    if (state())
        return 0;
    size_t ii;
    for (ii = 0; ii < nmemb; ++ii)
    {
        if (!read2b(p[ii]))
            break;
        if (state())
            break;
    }
    return ii;
}

size_t HIODev::read4b(void *ptr, size_t nmemb)
{
    uint *p = static_cast<uint *>(ptr);

    if (state())
        return 0;
    size_t ii;
    for (ii = 0; ii < nmemb; ++ii)
    {
        if (!read4b(p[ii]))
            break;
        if (state())
            break;
    }
    return ii;
}


// hfileiodev class
HStreamIODev::HStreamIODev(std::unique_ptr<HStream> stream):_stream(std::move(stream))
{
    init();
}


HStreamIODev::~HStreamIODev()
{
/* 플러시한 후 닫는다. */
    flush();
    if (_gzfp)
        gz_close(_gzfp);
    _gzfp = nullptr;
}


void HStreamIODev::init()
{
    _gzfp = nullptr;
    compressed = false;
}


bool HStreamIODev::open()
{
    return _stream->available() != 0;
}


void HStreamIODev::flush()
{
    if (_gzfp)
        gz_flush(_gzfp, Z_FINISH);
}

bool HStreamIODev::state() const
{
    return false;
}

/* zlib 관련 부분 */
bool HStreamIODev::setCompressed(bool flag)
{
    compressed = flag;
    if (flag)
        return nullptr != (_gzfp = gz_open(*_stream));
    else if (_gzfp)
    {
        gz_flush(_gzfp, Z_FINISH);
        gz_close(_gzfp);
        _gzfp = nullptr;
    }
    return true;
}


// IO routines

#define GZREAD(ptr,len) (_gzfp?gz_read(_gzfp,ptr,len):0)

bool HStreamIODev::read1b(unsigned char &out)
{
    size_t res = compressed ? GZREAD(rBuf, 1) : _stream->readBytes(rBuf, 1);

    if (res < 1)
        return false;

    out = static_cast<unsigned char>(rBuf[0]);
    return true;
}

bool HStreamIODev::read1b(char &out)
{
    unsigned char tmp8;
    if (!read1b(tmp8))
        return false;
    out = tmp8;
    return true;
}

bool HStreamIODev::read2b(unsigned short &out)
{
    size_t res = compressed ? GZREAD(rBuf, 2) : _stream->readBytes(rBuf, 2);

    if (res < 2)
        return false;

    out = (static_cast<unsigned char>(rBuf[1]) << 8 | static_cast<unsigned char>(rBuf[0]));
    return true;
}

bool HStreamIODev::read4b(unsigned int &out)
{
    size_t res = compressed ? GZREAD(rBuf, 4) : _stream->readBytes(rBuf, 4);

    if (res < 4)
        return false;

    out = (static_cast<unsigned char>(rBuf[3]) << 24 | static_cast<unsigned char>(rBuf[2]) << 16 |
        static_cast<unsigned char>(rBuf[1]) << 8 | static_cast<unsigned char>(rBuf[0]));
    return true;
}

bool HStreamIODev::read4b(int &out)
{
    unsigned int tmp32;
    if (!read4b(tmp32))
        return false;
    out = tmp32;
    return true;
}

size_t HStreamIODev::readBlock(void *ptr, size_t size)
{
    size_t count =
        compressed
        ? GZREAD(ptr, size)
        : _stream->readBytes(static_cast<byte *>(ptr), size);

    return count;
}

size_t HStreamIODev::skipBlock(size_t size)
{
    if (compressed){
          if( size <= BUFSIZE )
                return GZREAD(rBuf, size);
          else{
                size_t remain = size;
                while(remain){
                     if( remain > BUFSIZE ) {
                          size_t read = GZREAD(rBuf, BUFSIZE);
                          remain -= read;
                          if (read != BUFSIZE)
                              break;
                     }
                     else{
                          remain -= GZREAD(rBuf, remain);
                          break;
                     }
                }
                return size - remain;
          }
     }
    return _stream->skipBytes(size);
}


HMemIODev::HMemIODev(char *s, size_t len)
{
    init();
    ptr = reinterpret_cast<uchar *>(s);
    length = len;
}


HMemIODev::~HMemIODev()
{
}


void HMemIODev::init()
{
    ptr = nullptr;
    length = 0;
    pos = 0;
}


bool HMemIODev::open()
{
    return true;
}


void HMemIODev::flush()
{
}

bool HMemIODev::state() const
{
    return pos > length;
}

bool HMemIODev::setCompressed(bool )
{
    return false;
}

bool HMemIODev::read1b(unsigned char &out)
{
    ++pos;
    if (!state())
    {
        out = ptr[pos - 1];
        return true;
    }
    return false;
}

bool HMemIODev::read1b(char &out)
{
    unsigned char tmp8;
    if (!read1b(tmp8))
        return false;
    out = tmp8;
    return true;
}

bool HMemIODev::read2b(unsigned short &out)
{
    pos += 2;
    if (!state())
    {
         out = ptr[pos - 1] << 8 | ptr[pos - 2];
         return true;
    }
    return false;
}

bool HMemIODev::read4b(unsigned int &out)
{
    pos += 4;
    if (!state())
    {
        out = static_cast<unsigned int>(ptr[pos - 1] << 24 | ptr[pos - 2] << 16 |
                    ptr[pos - 3] << 8 | ptr[pos - 4]);
        return true;
    }
    return false;
}

bool HMemIODev::read4b(int &out)
{
    unsigned int tmp32;
    if (!read4b(tmp32))
        return false;
    out = tmp32;
    return true;
}

size_t HMemIODev::readBlock(void *p, size_t size)
{
    if (state())
        return 0;
    if (length < pos + size)
        size = length - pos;
    memcpy(p, ptr + pos, size);
    pos += size;
    return size;
}

size_t HMemIODev::skipBlock(size_t size)
{
    if (state() || length < pos + size)
        return 0;
    pos += size;
    return size;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
