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

#ifdef WIN32
# include <io.h>
#else
# include <unistd.h>
#endif

#include <osl/diagnose.h>

#include "hwplib.h"
#include "hgzip.h"
#include "hiodev.h"
#include "hwpfile.h"
#include "hstream.h"

const int BUFSIZE = 1024;
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


int HIODev::read1b(void *ptr, int nmemb)
{
    uchar *p = (uchar *) ptr;
    int ii;

    if (state())
        return -1;
    for (ii = 0; ii < nmemb; ii++)
    {
        p[ii] = sal::static_int_cast<uchar>(read1b());
        if (state())
            break;
    }
    return ii;
}


int HIODev::read2b(void *ptr, int nmemb)
{
    ushort *p = (ushort *) ptr;
    int ii;

    if (state())
        return -1;
    for (ii = 0; ii < nmemb; ii++)
    {
        p[ii] = sal::static_int_cast<uchar>(read2b());
        if (state())
            break;
    }
    return ii;
}


int HIODev::read4b(void *ptr, int nmemb)
{
    ulong *p = (ulong *) ptr;
    int ii;

    if (state())
        return -1;
    for (ii = 0; ii < nmemb; ii++)
    {
        p[ii] = read4b();
        if (state())
            break;
    }
    return ii;
}


// hfileiodev class
HStreamIODev::HStreamIODev(HStream & stream):_stream(stream)
{
    init();
}


HStreamIODev::~HStreamIODev()
{
    close();
}


void HStreamIODev::init()
{
    _gzfp = NULL;
    compressed = false;
}


bool HStreamIODev::open()
{
    if (!(_stream.available()))
        return false;
    return true;
}


void HStreamIODev::flush(void)
{
    if (_gzfp)
        gz_flush(_gzfp, Z_FINISH);
}


void HStreamIODev::close(void)
{
/* 플러시한 후 닫는다. */
    this->flush();
    if (_gzfp)
        gz_close(_gzfp);                          /* gz_close() calls stream_closeInput() */
    else
        _stream.closeInput();
    _gzfp = NULL;
}


int HStreamIODev::state(void) const
{
    return 0;
}


/* zlib 관련 부분 */
bool HStreamIODev::setCompressed(bool flag)
{
    compressed = flag;
    if (flag == true)
        return 0 != (_gzfp = gz_open(_stream));
    else if (_gzfp)
    {
        gz_flush(_gzfp, Z_FINISH);
        gz_close(_gzfp);
        _gzfp = 0;
    }
    return true;
}


// IO routines

#define GZREAD(ptr,len) (_gzfp?gz_read(_gzfp,ptr,len):0)

int HStreamIODev::read1b()
{
    int res = (compressed) ? GZREAD(rBuf, 1) : _stream.readBytes(rBuf, 1);

    if (res <= 0)
        return -1;
    else
        return (unsigned char) rBuf[0];
}


int HStreamIODev::read2b()
{
    int res = (compressed) ? GZREAD(rBuf, 2) : _stream.readBytes(rBuf, 2);

    if (res <= 0)
        return -1;
    else
        return ((unsigned char) rBuf[1] << 8 | (unsigned char) rBuf[0]);
}


long HStreamIODev::read4b()
{
    int res = (compressed) ? GZREAD(rBuf, 4) : _stream.readBytes(rBuf, 4);

    if (res <= 0)
        return -1;
    else
        return ((unsigned char) rBuf[3] << 24 | (unsigned char) rBuf[2] << 16 |
            (unsigned char) rBuf[1] << 8 | (unsigned char) rBuf[0]);
}


int HStreamIODev::readBlock(void *ptr, int size)
{
    int count =
        (compressed) ? GZREAD(ptr, size) : _stream.readBytes((byte *) ptr,

        size);

    return count;
}


int HStreamIODev::skipBlock(int size)
{
    if (compressed){
          if( size <= BUFSIZE )
                return GZREAD(rBuf, size);
          else{
                int remain = size;
                while(remain){
                     if( remain > BUFSIZE )
                          remain -= GZREAD(rBuf, BUFSIZE);
                     else{
                          remain -= GZREAD(rBuf, remain);
                          break;
                     }
                }
                return size - remain;
          }
     }
    return _stream.skipBytes(size);
}


HMemIODev::HMemIODev(char *s, int len)
{
    init();
    ptr = (uchar *) s;
    length = len;
}


HMemIODev::~HMemIODev()
{
    close();
}


void HMemIODev::init()
{
    ptr = 0;
    length = 0;
    pos = 0;
}


bool HMemIODev::open()
{
    return true;
}


void HMemIODev::flush(void)
{
}


void HMemIODev::close(void)
{
}


int HMemIODev::state(void) const
{
    if (pos <= length)
        return 0;
    else
        return -1;
}


bool HMemIODev::setCompressed(bool )
{
    return false;
}


int HMemIODev::read1b()
{
    if (pos <= length)
         return ptr[pos++];
     else
         return 0;
}


int HMemIODev::read2b()
{
    pos += 2;
    if (pos <= length)
         return ptr[pos - 1] << 8 | ptr[pos - 2];
     else
         return 0;
}


long HMemIODev::read4b()
{
    pos += 4;
    if (pos <= length)
         return DWORD(ptr[pos - 1] << 24 | ptr[pos - 2] << 16 |
        ptr[pos - 3] << 8 | ptr[pos - 4]);
     else
         return 0;
}


int HMemIODev::readBlock(void *p, int size)
{
    if (length < pos + size)
        size = length - pos;
    memcpy(p, ptr + pos, size);
    pos += size;
    return size;
}


int HMemIODev::skipBlock(int size)
{
    if (length < pos + size)
        return 0;
    pos += size;
    return size;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
