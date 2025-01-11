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

#pragma once

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
        virtual void flush() = 0;
        virtual bool state() const = 0;
/* gzip routine wrapper */
        virtual bool setCompressed( bool ) = 0;

        virtual bool read1b(unsigned char &out) = 0;
        virtual bool read1b(char &out) = 0;
        virtual bool read2b(unsigned short &out) = 0;
        virtual bool read4b(unsigned int &out) = 0;
        virtual bool read4b(int &out) = 0;
        virtual size_t readBlock( void *ptr, size_t size ) = 0;
        virtual size_t skipBlock( size_t size ) = 0;

        size_t read2b( void *ptr, size_t nmemb );
        size_t read4b( void *ptr, size_t nmemb );
};

struct gz_stream;

/* File input and output devices */

/**
 * This controls the HStream given by constructor
 * @short Stream IO device
 */
class HStreamIODev final: public HIODev
{
    private:
/* zlib으로 압축을 풀기 위한 자료 구조 */
        std::unique_ptr<HStream> _stream;
        gz_stream *_gzfp;
    public:
        explicit HStreamIODev(std::unique_ptr<HStream> stream);
        virtual ~HStreamIODev() override;
/**
 * Check whether the stream is available
 */
        virtual bool open() override;
/**
 * If the stream is gzipped, flush the stream.
 */
        virtual void flush() override;
/**
 * Not implemented.
 */
        virtual bool state() const override;
/**
 * Set whether the stream is compressed or not
 */
        virtual bool setCompressed( bool ) override;
/**
 * Read one byte from stream
 */
        virtual bool read1b(unsigned char &out) override;
        virtual bool read1b(char &out) override;
/**
 * Read 2 bytes from stream
 */
        using HIODev::read2b;
        virtual bool read2b(unsigned short &out) override;
/**
 * Read 4 bytes from stream
 */
        using HIODev::read4b;
        virtual bool read4b(unsigned int &out) override;
        virtual bool read4b(int &out) override;
/**
 * Read some bytes from stream to given pointer as amount of size
 */
        virtual size_t readBlock( void *ptr, size_t size ) override;
/**
 * Move current pointer of stream as amount of size
 */
        virtual size_t skipBlock( size_t size ) override;
    private:
/**
 * Initialize this object
 */
        virtual void init() override;
};

/* Memory, input and output devices */
/**
 * The HMemIODev class controls the Input/Output device.
 * @short Memory IO device
 */
class HMemIODev final: public HIODev
{
    uchar *ptr;
    size_t pos, length;
    public:
        HMemIODev(char *s, size_t len);
        virtual ~HMemIODev() override;

        virtual bool open() override;
        virtual void flush() override;
        virtual bool state() const override;
/* gzip routine wrapper */
        virtual bool setCompressed( bool ) override;
        virtual bool read1b(unsigned char &out) override;
        virtual bool read1b(char &out) override;
        using HIODev::read2b;
        virtual bool read2b(unsigned short &out) override;
        using HIODev::read4b;
        virtual bool read4b(unsigned int &out) override;
        virtual bool read4b(int &out) override;
        virtual size_t readBlock( void *ptr, size_t size ) override;
        virtual size_t skipBlock( size_t size ) override;
    private:
        virtual void init() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
