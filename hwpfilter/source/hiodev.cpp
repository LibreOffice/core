/*************************************************************************
 *
 *  $RCSfile: hiodev.cpp,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:40:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2001 by Mizi Research Inc.
 *  Copyright 2003 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 2001 by Mizi Research Inc.
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* $Id: hiodev.cpp,v 1.1 2003-10-15 14:40:14 dvo Exp $ */

#ifdef __GNUG__
#pragma implementation "hiodev.h"
#endif

#include <stdio.h>
#include <errno.h>
// DVO: add zlib/ prefix
#include <zlib/zlib.h>
#ifdef WIN32
# include <io.h>
#else
# include <unistd.h>
#endif

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
        p[ii] = read1b();
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
        p[ii] = read2b();
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


// zlib 관련 부분
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


bool HMemIODev::setCompressed(bool flag)
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
