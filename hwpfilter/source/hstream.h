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

#ifndef INCLUDED_HWPFILTER_SOURCE_HSTREAM_H
#define INCLUDED_HWPFILTER_SOURCE_HSTREAM_H

typedef unsigned char byte;
/**
 * Stream class
 */
class HStream
{
    public:
        HStream();
        virtual ~HStream();

/**
 *
 */
        void addData( const byte *buf, int aToAdd);
/**
 * Read some byte to buf as given size
 */
        int readBytes( byte *buf, int aToRead);
/**
 * Skip some byte from stream as given size
 */
        int skipBytes( int aToSkip );
/**
 * @returns Size of remained stream
 */
        int available() const;
/**
 * remove the stream from this object.
 */
        void closeInput();
    private:
        int size;
        byte *seq;
        int pos;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
