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

#ifndef INCLUDED_OOX_OLE_VBAINPUTSTREAM_HXX
#define INCLUDED_OOX_OLE_VBAINPUTSTREAM_HXX

#include <vector>
#include <oox/helper/binaryinputstream.hxx>

namespace oox {
namespace ole {



/** A non-seekable input stream that implements run-length decompression. */
class VbaInputStream : public BinaryInputStream
{
public:
    explicit            VbaInputStream( BinaryInputStream& rInStrm );

    /** Returns -1, stream size is not determinable. */
    virtual sal_Int64   size() const override;
    /** Returns -1, stream position is not tracked. */
    virtual sal_Int64   tell() const override;
    /** Does nothing, stream is not seekable. */
    virtual void        seek( sal_Int64 nPos ) override;
    /** Closes the input stream but not the wrapped stream. */
    virtual void        close() override;

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize = 1 ) override;
    /** Reads nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize = 1 ) override;
    /** Seeks the stream forward by the passed number of bytes. */
    virtual void        skip( sal_Int32 nBytes, size_t nAtomSize = 1 ) override;

private:
    /** If no data left in chunk buffer, reads the next chunk from stream. */
    bool                updateChunk();

private:
    typedef ::std::vector< sal_uInt8 > ChunkBuffer;

    BinaryInputStream*  mpInStrm;
    ChunkBuffer         maChunk;
    size_t              mnChunkPos;
};



} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
