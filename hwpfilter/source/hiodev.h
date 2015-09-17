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

/**
 * hwpio.h
 * (C) 1999 Mizi Research, All rights are reserved
 */

#ifndef INCLUDED_HWPFILTER_SOURCE_HIODEV_H
#define INCLUDED_HWPFILTER_SOURCE_HIODEV_H

#include <sal/config.h>

#include <stdio.h>

#include <memory>
#include <sal/types.h>

#include "hwplib.h"
/**
 * @short Abstract IO class
 */
class DLLEXPORT HIODev
{
    protected:
        bool compressed;
        virtual void init();
    public:
        HIODev();
        virtual ~HIODev();

        virtual bool open() = 0;
        virtual void close() = 0;
        virtual void flush() = 0;
        virtual int  state() const = 0;
/* gzip routine wrapper */
        virtual bool setCompressed( bool ) = 0;

        virtual bool read1b(unsigned char &out) = 0;
        virtual bool read1b(char &out) = 0;
        virtual bool read2b(unsigned short &out) = 0;
        virtual bool read4b(unsigned int &out) = 0;
        virtual bool read4b(int &out) = 0;
        virtual int readBlock( void *ptr, int size ) = 0;
        virtual int skipBlock( int size ) = 0;

        int read1b( void *ptr, int nmemb );
        int read2b( void *ptr, int nmemb );
        int read4b( void *ptr, int nmemb );
};

struct gz_stream;

/* File input and output devices */

/**
 * This controls the HStream given by constructor
 * @short Stream IO device
 */
class HStreamIODev : public HIODev
{
    private:
/* zlib으로 압축을 풀기 위한 자료 구조 */
        std::unique_ptr<HStream> _stream;
        gz_stream *_gzfp;
    public:
        HStreamIODev(HStream* stream);
        virtual ~HStreamIODev();
/**
 * Check whether the stream is available
 */
        virtual bool open() SAL_OVERRIDE;
/**
 * Free stream object
 */
        virtual void close() SAL_OVERRIDE;
/**
 * If the stream is gzipped, flush the stream.
 */
        virtual void flush() SAL_OVERRIDE;
/**
 * Not implemented.
 */
        virtual int  state() const SAL_OVERRIDE;
/**
 * Set whether the stream is compressed or not
 */
        virtual bool setCompressed( bool ) SAL_OVERRIDE;
/**
 * Read one byte from stream
 */
        using HIODev::read1b;
        virtual bool read1b(unsigned char &out) SAL_OVERRIDE;
        virtual bool read1b(char &out) SAL_OVERRIDE;
/**
 * Read 2 bytes from stream
 */
        using HIODev::read2b;
        virtual bool read2b(unsigned short &out) SAL_OVERRIDE;
/**
 * Read 4 bytes from stream
 */
        using HIODev::read4b;
        virtual bool read4b(unsigned int &out) SAL_OVERRIDE;
        virtual bool read4b(int &out) SAL_OVERRIDE;
/**
 * Read some bytes from stream to given pointer as amount of size
 */
        virtual int readBlock( void *ptr, int size ) SAL_OVERRIDE;
/**
 * Move current pointer of stream as amount of size
 */
        virtual int skipBlock( int size ) SAL_OVERRIDE;
    protected:
/**
 * Initialize this object
 */
        virtual void init() SAL_OVERRIDE;
};

/* Memory, input and output devices */
/**
 * The HMemIODev class controls the Input/Output device.
 * @short Memory IO device
 */
class HMemIODev : public HIODev
{
    uchar *ptr;
    int pos, length;
    public:
        HMemIODev(char *s, int len);
        virtual ~HMemIODev();

        virtual bool open() SAL_OVERRIDE;
        virtual void close() SAL_OVERRIDE;
        virtual void flush() SAL_OVERRIDE;
        virtual int  state() const SAL_OVERRIDE;
/* gzip routine wrapper */
        virtual bool setCompressed( bool ) SAL_OVERRIDE;
        using HIODev::read1b;
        virtual bool read1b(unsigned char &out) SAL_OVERRIDE;
        virtual bool read1b(char &out) SAL_OVERRIDE;
        using HIODev::read2b;
        virtual bool read2b(unsigned short &out) SAL_OVERRIDE;
        using HIODev::read4b;
        virtual bool read4b(unsigned int &out) SAL_OVERRIDE;
        virtual bool read4b(int &out) SAL_OVERRIDE;
        virtual int readBlock( void *ptr, int size ) SAL_OVERRIDE;
        virtual int skipBlock( int size ) SAL_OVERRIDE;
    protected:
        virtual void init() SAL_OVERRIDE;
};
#endif // INCLUDED_HWPFILTER_SOURCE_HIODEV_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
