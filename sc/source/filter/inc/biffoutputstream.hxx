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

#ifndef OOX_XLS_BIFFOUTPUTSTREAM_HXX
#define OOX_XLS_BIFFOUTPUTSTREAM_HXX

#include <vector>
#include "oox/helper/binaryoutputstream.hxx"
#include "biffhelper.hxx"

namespace oox { class BinaryOutputStream; }

namespace oox {
namespace xls {

// ============================================================================

namespace prv {

/** Buffers the contents of a raw record. */
class BiffOutputRecordBuffer
{
public:
    explicit            BiffOutputRecordBuffer(
                            BinaryOutputStream& rOutStrm,
                            sal_uInt16 nMaxRecSize );
    /** Returns the wrapped binary base stream. */
    inline const BinaryOutputStream& getBaseStream() const { return mrOutStrm; }

    /** Starts a new record. */
    void                startRecord( sal_uInt16 nRecId );
    /** Finishes the current record. Must be called for every started record. */
    void                endRecord();

    /** Returns the number of remaining bytes in the current record body. */
    inline sal_uInt16   getRecLeft() const { return static_cast< sal_uInt16 >( mnMaxRecSize - maData.size() ); }

    /** Writes nBytes bytes from the existing buffer pData. Must NOT overwrite the destination buffer. */
    void                write( const void* pData, sal_uInt16 nBytes );

private:
    typedef ::std::vector< sal_uInt8 > DataBuffer;

    BinaryOutputStream& mrOutStrm;              /// Core output stream.
    DataBuffer          maData;                 /// Record data buffer.
    sal_uInt16          mnMaxRecSize;           /// Maximum size of record contents.
    sal_uInt16          mnRecId;                /// Current record identifier.
    bool                mbInRec;                /// True = currently writing inside of a record.
};

} // namespace prv

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
