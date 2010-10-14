/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#ifndef _VOS_ISTREAM_HXX_
#define _VOS_ISTREAM_HXX_

#include "sal/types.h"
#   include <vos/types.hxx>

namespace vos
{

/** Interface for a stream of data, on that you can read and write blocks of bytes.
*/
class IStream
{
public:

    IStream() { }
    virtual ~IStream() { }


    /** Retrieve n bytes from the stream and copy them into pBuffer.
        @param pBuffer receives the read data.
        @param n the number of bytes to read. pBuffer must be large enough
        to hold the n bytes!
        @return the number of read bytes
    */
    virtual sal_Int32 SAL_CALL read(void* pBuffer,
                         sal_uInt32 n) const= 0;

    /** Write n bytes from pBuffer to the stream.
        @param pBuffer contains the data to be written.
        @param n the number of bytes to write.
        @return the number of written bytes
    */
    virtual sal_Int32 SAL_CALL write(const void* pBuffer,
                          sal_uInt32 n)= 0;

    /** Checks if stream is closed for further reading.
        @return True is stream has ended (e.g. was closed).
    */
    virtual sal_Bool SAL_CALL isEof() const = 0;
};

}

#endif  // _VOS_ISTREAM_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
