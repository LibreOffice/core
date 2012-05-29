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

#ifndef STREAM_HELPER_HXX_INCLUDED
#define STREAM_HELPER_HXX_INCLUDED

#include "internal/types.hxx"

struct IStream;

class BufferStream : public StreamInterface
{
public:
    BufferStream(IStream *str);
    ~BufferStream();
    unsigned long sread (unsigned char *vuf, unsigned long size);
    long stell ();
    long sseek (unsigned long offset, int origin);
private:
    IStream *stream;
};

class FileStream : public StreamInterface
{
public:
    FileStream(const char *filename);
    ~FileStream();
    unsigned long sread (unsigned char *buf, unsigned long size);
    long stell ();
    long sseek (unsigned long offset, int origin);
private:
    FILE *file;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
