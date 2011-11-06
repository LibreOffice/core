/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    /** Sets size of data portion in bytes. 0 or 1 means no portions are used. */
    void                setPortionSize( sal_uInt8 nSize );

    // BinaryStreamBase interface (seeking) -----------------------------------

    /** Returns the absolute position in the wrapped binary stream. */
    sal_Int64           tellBase() const;
    /** Returns the total size of the wrapped binary stream. */
    sal_Int64           sizeBase() const;

    // BinaryOutputStream interface (stream write access) ---------------------

    /** Writes the passed data sequence. */
    virtual void        writeData( const StreamDataSequence& rData, size_t nAtomSize = 1 );
    /** Writes nBytes bytes from the passed buffer pMem. */
    virtual void        writeMemory( const void* pMem, sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Writes a sequence of nBytes bytes with the passed value. */
    void                fill( sal_uInt8 nValue, sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Stream operator for all data types supported by the writeValue() function. */
    template< typename Type >
    inline BiffOutputStream& operator<<( Type nValue ) { writeValue( nValue ); return *this; }

    // ------------------------------------------------------------------------
private:
    /** Checks the remaining size in the current record, creates CONTINUE record if needed. */
    void                ensureRawBlock( sal_uInt16 nSize );

    /** Checks the remaining size in the current record and creates a CONTINUE
        record if needed.
        @return  Maximum size left for writing to current record. */
    sal_uInt16          prepareWriteBlock( sal_Int32 nTotalSize, size_t nAtomSize );

private:
    prv::BiffOutputRecordBuffer maRecBuffer;    /// Raw record data buffer.
    sal_uInt8           mnPortionSize;          /// Size of data portions.
    sal_uInt8           mnPortionPos;           /// Position in current portion.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
