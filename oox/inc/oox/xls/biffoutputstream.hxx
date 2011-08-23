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
#include "oox/xls/biffhelper.hxx"

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
    /** Writes a sequence of nBytes bytes with the passed value. */
    void                fill( sal_uInt8 nValue, sal_uInt16 nBytes );

private:
    typedef ::std::vector< sal_uInt8 > DataBuffer;

    BinaryOutputStream& mrOutStrm;              /// Core ouput stream.
    DataBuffer          maData;                 /// Record data buffer.
    sal_uInt16          mnMaxRecSize;           /// Maximum size of record contents.
    sal_uInt16          mnRecId;                /// Current record identifier.
    bool                mbInRec;                /// True = currently writing inside of a record.
};

} // namespace prv

// ============================================================================

/** This class is used to write BIFF record streams.

    An instance is constructed with a BinaryOutputStream object and the
    maximum size of BIFF record contents (e.g. 2080 bytes in BIFF2-BIFF5, or
    8224 bytes in BIFF8).

    To start writing a record, call startRecord() with the record identifier.
    Each record must be closed by calling endRecord().

    If some data exceeds the record size limit, a CONTINUE record will be
    started automatically and the new data will be written to this record. If
    specific data pieces must not be split into the current and a following
    CONTINUE record, use setPortionSize(). Example: To write a sequence of
    16-bit values where 4 values form a unit and cannot be split, call
    setPortionSize(8) first (4*2 bytes == 8).

    To write unicode character arrays, call writeUnicodeBuffer(). It creates
    CONTINUE records and repeats the unicode string flag byte automatically.
*/
class BiffOutputStream : public BinaryOutputStream
{
public:
    explicit            BiffOutputStream(
                            BinaryOutputStream& rOutStream,
                            sal_uInt16 nMaxRecSize );

    // record control ---------------------------------------------------------

    /** Starts a new record. */
    void                startRecord( sal_uInt16 nRecId );

    /** Finishes the current record. Must be called for every started record. */
    void                endRecord();

    /** Sets size of data portion in bytes. 0 means no portions are used. */
    void                setPortionSize( sal_uInt16 nSize );

    // BinaryStreamBase interface (seeking) -----------------------------------

    /** Returns the absolute position in the wrapped binary stream. */
    sal_Int64           tellBase() const;
    /** Returns the total size of the wrapped binary stream. */
    sal_Int64           getBaseLength() const;

    // BinaryOutputStream interface (stream write access) ---------------------

    /** Writes the passed data sequence. */
    virtual void        writeData( const StreamDataSequence& rData );
    /** Writes nBytes bytes from the passed buffer pMem. */
    virtual void        writeMemory( const void* pMem, sal_Int32 nBytes );

    /** Writes a sequence of nBytes bytes with the passed value. */
    void                fill( sal_uInt8 nValue, sal_Int32 nBytes );
    /** Writes a block of memory, ensures that it is not split to a CONTINUE record. */
    void                writeBlock( const void* pMem, sal_uInt16 nBytes );

    /** Stream operator for integral and floating-point types. */
    template< typename Type >
    inline BiffOutputStream& operator<<( Type nValue ) { writeValue( nValue ); return *this; }

    // ------------------------------------------------------------------------
private:
    /** Forwards calls of writeValue() template functions to the record buffer. */
    virtual void        writeAtom( const void* pMem, sal_uInt8 nSize );

    /** Checks the remaining size in the current record, creates CONTINUE record if needed. */
    void                ensureRawBlock( sal_uInt16 nSize );
    /** Checks the remaining size in the current record and creates CONTINUE record if needed.
        @return  Maximum size left for writing to current record. */
    sal_uInt16          prepareRawBlock( sal_Int32 nTotalSize );

private:
    prv::BiffOutputRecordBuffer maRecBuffer;    /// Raw record data buffer.
    sal_uInt16          mnPortionSize;          /// Size of data portions.
    sal_uInt16          mnPortionPos;           /// Position in current portion.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
