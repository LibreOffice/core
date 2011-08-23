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

#ifndef OOX_OLE_VBAINPUTSTREAM_HXX
#define OOX_OLE_VBAINPUTSTREAM_HXX

#include <vector>
#include "oox/helper/binaryinputstream.hxx"

namespace oox {
namespace ole {

// ============================================================================

/** A non-seekable input stream that implements run-length decompression. */
class VbaInputStream : public BinaryInputStream
{
public:
    explicit            VbaInputStream( BinaryInputStream& rInStrm );

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes );
    /** Reads nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes );
    /** Seeks the stream forward by the passed number of bytes. */
    virtual void        skip( sal_Int32 nBytes );

private:
    /** If no data left in chunk buffer, reads the next chunk from stream. */
    bool                updateChunk();

private:
    typedef ::std::vector< sal_uInt8 > ChunkBuffer;

    BinaryInputStream&  mrInStrm;
    ChunkBuffer         maChunk;
    size_t              mnChunkPos;
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
