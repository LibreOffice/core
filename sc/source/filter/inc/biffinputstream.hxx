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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_BIFFINPUTSTREAM_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_BIFFINPUTSTREAM_HXX

#include <vector>
#include <oox/helper/binaryinputstream.hxx>
#include "biffhelper.hxx"
#include "biffcodec.hxx"

namespace oox {
namespace xls {

namespace prv {

/** Buffers the contents of a raw record and encapsulates stream decoding. */
class BiffInputRecordBuffer
{
public:
    explicit            BiffInputRecordBuffer( BinaryInputStream& rInStrm );

    /** Sets a decoder object and decrypts buffered record data. */
    void                setDecoder( const BiffDecoderRef& rxDecoder );
    /** Enables/disables usage of current decoder. */
    void                enableDecoder( bool bEnable );

    /** Restarts the stream at the passed position. Buffer is invalid until the
        next call of startRecord() or startNextRecord(). */
    void                restartAt( sal_Int64 nPos );

    /** Reads the record header at the passed position. */
    bool                startRecord( sal_Int64 nHeaderPos );
    /** Reads the next record header from the stream. */
    bool                startNextRecord();
    /** Returns the start position of the record header in the core stream. */
    sal_uInt16          getNextRecId();

    /** Returns the start position of the record header in the core stream. */
    inline sal_Int64    getRecHeaderPos() const { return mnHeaderPos; }
    /** Returns the current record identifier. */
    inline sal_uInt16   getRecId() const { return mnRecId; }
    /** Returns the current record size. */
    inline sal_uInt16   getRecSize() const { return mnRecSize; }
    /** Returns the number of remaining bytes in the current record body. */
    inline sal_uInt16   getRecLeft() const { return mnRecSize - mnRecPos; }

    /** Reads nBytes bytes to the existing buffer opData. Must NOT overread the source buffer. */
    void                read( void* opData, sal_uInt16 nBytes );
    /** Ignores nBytes bytes. Must NOT overread the buffer. */
    void                skip( sal_uInt16 nBytes );

private:
    /** Updates data buffer from stream, if needed. */
    void                updateBuffer();
    /** Updates decoded data from original data. */
    void                updateDecoded();

private:
    typedef ::std::vector< sal_uInt8 > DataBuffer;

    BinaryInputStream&  mrInStrm;               /// Core input stream.
    DataBuffer          maOriginalData;         /// Original data read from stream.
    DataBuffer          maDecodedData;          /// Decoded data.
    DataBuffer*         mpCurrentData;          /// Points to data buffer currently in use.
    BiffDecoderRef      mxDecoder;              /// Decoder object.
    sal_Int64           mnHeaderPos;            /// Stream start position of current record header.
    sal_Int64           mnBodyPos;              /// Stream start position of current record body.
    sal_Int64           mnBufferBodyPos;        /// Stream start position of buffered data.
    sal_Int64           mnNextHeaderPos;        /// Stream start position of next record header.
    sal_uInt16          mnRecId;                /// Current record identifier.
    sal_uInt16          mnRecSize;              /// Current record size.
    sal_uInt16          mnRecPos;               /// Current position in record body.
    bool                mbValidHeader;          /// True = valid record header.
};

} // namespace prv

/** This class is used to read BIFF record streams.

    An instance is constructed with a BinaryInputStream object. The passed
    stream is reset to its start while constructing this stream.

    To start reading a record call startNextRecord(). Now it is possible to
    read all contents of the record using operator>>() or any of the read***()
    functions. If some data exceeds the record size limit, the stream looks for
    a following CONTINUE record and jumps automatically to it. It is NOT
    allowed that an atomic data type is split into two records (e.g. 4 bytes of
    a double in one record and the other 4 bytes in a following CONTINUE).

    Trying to read over the record limits results in a stream error. The
    isValid() function indicates that by returning false. From now on the data
    returned by the read functions is undefined. The error state will be reset,
    if the next record is started.

    The import stream supports decrypting the stream data. The contents of a
    record (not the record header) will be encrypted by Excel if the file has
    been stored with password protection. The functions setDecoder() and
    enableDecoder() control the usage of the decryption algorithms.
    setDecoder() sets a new decryption algorithm and initially enables it.
    enableDecoder( false ) may be used to stop the usage of the decryption
    temporarily (sometimes record contents are never encrypted, e.g. all BOF
    records or the stream position in SHEET records). Decryption will be
    reenabled automatically, if a new record is started with the function
    startNextRecord().
*/
class BiffInputStream : public BinaryInputStream
{
public:
    /** Constructs the BIFF record stream using the passed binary stream.

        @param rInStream
            The base input stream. Must be seekable. Will be sought to its
            start position.
     */
    explicit            BiffInputStream(
                            BinaryInputStream& rInStream );

    // record control ---------------------------------------------------------

    /** Sets stream pointer to the start of the next record content.

        Ignores all CONTINUE records of the current record, if automatic
        CONTINUE usage is switched on.

        @return  False = no record found (end of stream).
     */
    bool                startNextRecord();

    /** Sets stream pointer before current record and invalidates stream.

        The next call to startNextRecord() will start again the current record.
        This can be used in situations where a loop or a function leaves on a
        specific record, but the parent context expects to start this record by
        itself. The stream is invalid as long as the first record has not been
        started (it is not allowed to call any other stream operation then).
     */
    void                rewindRecord();

    // decoder ----------------------------------------------------------------

    /** Enables/disables usage of current decoder.

        Decryption is reenabled automatically, if a new record is started using
        the function startNextRecord().
     */
    void                enableDecoder();

    // stream/record state and info -------------------------------------------

    /** Returns the current record identifier. */
    inline sal_uInt16   getRecId() const { return mnRecId; }
    /** Returns the record identifier of the following record. */
    sal_uInt16          getNextRecId();

    /** Returns a unique handle for the current record that can be used with
        the function startRecordByHandle(). */
    inline sal_Int64    getRecHandle() const { return mnRecHandle; }

    // BinaryStreamBase interface (seeking) -----------------------------------

    /** Returns the data size of the whole record without record headers. */
    virtual sal_Int64   size() const override;
    /** Returns the position inside of the whole record content. */
    virtual sal_Int64   tell() const override;
    /** Seeks in record content to the specified position. */
    virtual void        seek( sal_Int64 nRecPos ) override;
    /** Closes the input stream but not the wrapped stream. */
    virtual void        close() override;

    // BinaryInputStream interface (stream read access) -----------------------

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize = 1 ) override;
    /** Reads nBytes bytes and copies them to the passed buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize = 1 ) override;
    /** Seeks forward inside the current record. */
    virtual void        skip( sal_Int32 nBytes, size_t nAtomSize = 1 ) override;

    /** Stream operator for integral and floating-point types. */
    template< typename Type >
    inline BiffInputStream& operator>>( Type& ornValue ) { ornValue = readValue<Type>(); return *this; }

    // byte strings -----------------------------------------------------------

    /** Reads 16 bit string length and character array, and returns a Unicode string.
        NUL characters are replaced by question marks (default).
        @param eTextEnc  The text encoding used to create the Unicode string.
     */
    OUString     readByteStringUC( rtl_TextEncoding eTextEnc );

private:
    /** Initializes all members after base stream has been sought to new record. */
    void                setupRecord();
    /** Restarts the current record from the beginning. */
    void                restartRecord();
    /** Sets stream pointer before specified record and invalidates stream. */
    void                rewindToRecord( sal_Int64 nRecHandle );
    /** Returns true, if stream was able to start a valid record. */
    inline bool         isInRecord() const { return mnRecHandle >= 0; }

    /** Returns true, if the passed ID is real or alternative continuation record ID. */
    bool                isContinueId( sal_uInt16 nRecId ) const;
    /** Goes to start of the next CONTINUE record.
        @descr  Stream must be located at the end of a raw record, and handling
        of CONTINUE records must be enabled.
        @return  True if next CONTINUE record has been found and initialized. */
    bool                jumpToNextContinue();
    /** Calculates the complete length of the current record including CONTINUE
        records, stores the length in mnComplRecSize. */
    void                calcRecordLength();

    /** Returns the maximum size of raw data possible to read in one block. */
    sal_uInt16          getMaxRawReadSize( sal_Int32 nBytes, size_t nAtomSize ) const;

private:
    prv::BiffInputRecordBuffer maRecBuffer; /// Raw record data buffer.

    sal_Int64           mnRecHandle;        /// Handle of current record.
    sal_uInt16          mnRecId;            /// Identifier of current record (not the CONTINUE ID).
    sal_uInt16          mnAltContId;        /// Alternative identifier for content continuation records.

    sal_Int64           mnCurrRecSize;      /// Helper for record size and position.
    sal_Int64           mnComplRecSize;     /// Size of complete record data (with CONTINUEs).
    bool                mbHasComplRec;      /// True = mnComplRecSize is valid.
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
