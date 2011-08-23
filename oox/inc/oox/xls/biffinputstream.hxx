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

#ifndef OOX_XLS_BIFFINPUTSTREAM_HXX
#define OOX_XLS_BIFFINPUTSTREAM_HXX

#include <vector>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/xls/biffhelper.hxx"
#include "oox/xls/biffcodec.hxx"

namespace rtl { class OUStringBuffer; }

namespace oox {
namespace xls {

// ============================================================================

namespace prv {

/** Buffers the contents of a raw record and encapsulates stream decoding. */
class BiffInputRecordBuffer
{
public:
    explicit            BiffInputRecordBuffer( BinaryInputStream& rInStrm );

    /** Returns the wrapped binary base stream. */
    inline const BinaryInputStream& getBaseStream() const { return mrInStrm; }

    /** Sets a decoder object and decrypts buffered record data. */
    void                setDecoder( const BiffDecoderRef& rxDecoder );
    /** Returns the current decoder object. */
    inline BiffDecoderRef getDecoder() const { return mxDecoder; }
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
    /** Returns the current read position in the current record body. */
    inline sal_uInt16   getRecPos() const { return mnRecPos; }
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

// ============================================================================

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
    if the record is reset (with the function resetRecord()), or if the next
    record is started.

    To switch off the automatic lookup of CONTINUE records, use resetRecord()
    with false parameter. This is useful e.g. on import of drawing layer data,
    where sometimes solely CONTINUE records will occur. The automatic lookup
    keeps switched off until the method resetRecord() is called with parameter
    true. All other settings done on the stream (e.g. alternative CONTINUE
    record identifier, enabled decryption, NUL substitution character) will be
    reset to default values, if a new record is started.

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
            The base input stream. Must be seekable. Will be seeked to its
            start position.

        @param bContLookup  Automatic CONTINUE lookup on/off.
     */
    explicit            BiffInputStream(
                            BinaryInputStream& rInStream,
                            bool bContLookup = true );

    // record control ---------------------------------------------------------

    /** Sets stream pointer to the start of the next record content.

        Ignores all CONTINUE records of the current record, if automatic
        CONTINUE usage is switched on.

        @return  False = no record found (end of stream).
     */
    bool                startNextRecord();

    /** Sets stream pointer to the start of the content of the specified record.

        The handle of the current record can be received and stored using the
        function getRecHandle() for later usage with this function. The record
        handle is equivalent to the position of the underlying binary stream,
        thus the function can be used to perform a hard seek to a specific
        position, if it is sure that a record starts exactly at this position.

        @return  False = no record found (invalid handle passed).
     */
    bool                startRecordByHandle( sal_Int64 nRecHandle );

    /** Sets stream pointer to begin of record content.

        @param bContLookup
            Automatic CONTINUE lookup on/off. In difference to other stream
            settings, this setting is persistent until next call of this
            function (because it is wanted to receive the next CONTINUE records
            separately).
        @param nAltContId
            Sets an alternative record identifier for content continuation.
            This value is reset automatically when a new record is started with
            startNextRecord().
     */
    void                resetRecord(
                            bool bContLookup,
                            sal_uInt16 nAltContId = BIFF_ID_UNKNOWN );

    /** Sets stream pointer before current record and invalidates stream.

        The next call to startNextRecord() will start again the current record.
        This can be used in situations where a loop or a function leaves on a
        specific record, but the parent context expects to start this record by
        itself. The stream is invalid as long as the first record has not been
        started (it is not allowed to call any other stream operation then).
     */
    void                rewindRecord();

    // decoder ----------------------------------------------------------------

    /** Sets a new decoder object.

        Enables decryption of record contents for the rest of the stream.
     */
    void                setDecoder( const BiffDecoderRef& rxDecoder );

    /** Enables/disables usage of current decoder.

        Decryption is reenabled automatically, if a new record is started using
        the function startNextRecord().
     */
    void                enableDecoder( bool bEnable = true );

    // stream/record state and info -------------------------------------------

    /** Returns the current record identifier. */
    inline sal_uInt16   getRecId() const { return mnRecId; }
    /** Returns the record identifier of the following record. */
    sal_uInt16          getNextRecId();

    /** Returns a unique handle for the current record that can be used with
        the function startRecordByHandle(). */
    inline sal_Int64    getRecHandle() const { return mnRecHandle; }

    // BinaryStreamBase interface (seeking) -----------------------------------

    /** Returns true, as the BIFF input stream is required to be seekable. */
    virtual bool        isSeekable() const;
    /** Returns the position inside of the whole record content. */
    virtual sal_Int64   tell() const;
    /** Returns the data size of the whole record without record headers. */
    virtual sal_Int64   getLength() const;
    /** Seeks in record content to the specified position. */
    virtual void        seek( sal_Int64 nRecPos );

    /** Returns the absolute position in the wrapped binary stream. */
    sal_Int64           tellBase() const;
    /** Returns the total size of the wrapped binary stream. */
    sal_Int64           getBaseLength() const;

    // BinaryInputStream interface (stream read access) -----------------------

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes );
    /** Reads nBytes bytes and copies them to the passed buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes );
    /** Seeks forward inside the current record. */
    virtual void        skip( sal_Int32 nBytes );

    /** Stream operator for integral and floating-point types. */
    template< typename Type >
    inline BiffInputStream& operator>>( Type& ornValue ) { readValue( ornValue ); return *this; }

    // byte strings -----------------------------------------------------------

    /** Reads 8/16 bit string length and character array, and returns the string.
        @param b16BitLen
            True = Read 16-bit string length field before the character array.
            False = Read 8-bit string length field before the character array.
        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default).
     */
    ::rtl::OString      readByteString( bool b16BitLen, bool bAllowNulChars = false );

    /** Reads 8/16 bit string length and character array, and returns a Unicode string.
        @param b16BitLen
            True = Read 16-bit string length field before the character array.
            False = Read 8-bit string length field before the character array.
        @param eTextEnc  The text encoding used to create the Unicode string.
        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default).
     */
    ::rtl::OUString     readByteStringUC( bool b16BitLen, rtl_TextEncoding eTextEnc, bool bAllowNulChars = false );

    /** Ignores 8/16 bit string length and character array.
        @param b16BitLen
            True = Read 16-bit string length field before the character array.
            False = Read 8-bit string length field before the character array.
     */
    void                skipByteString( bool b16BitLen );

    // Unicode strings --------------------------------------------------------

    /** Reads nChars characters of a BIFF8 string, and returns the string.
        @param nChars  Number of characters to read from the stream.
        @param b16BitChars
            True = The character array contains 16-bit characters.
            False = The character array contains truncated 8-bit characters.
        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default).
     */
    ::rtl::OUString     readUniStringChars( sal_uInt16 nChars, bool b16BitChars, bool bAllowNulChars = false );

    /** Reads 8-bit flags, extended header, nChar characters, extended data of
        a BIFF8 string, and returns the string.
        @param nChars  Number of characters to read from the stream.
        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default).
     */
    ::rtl::OUString     readUniStringBody( sal_uInt16 nChars, bool bAllowNulChars = false );

    /** Reads 16-bit character count, 8-bit flags, extended header, character
        array, extended data of a BIFF8 string, and returns the string.
        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default).
     */
    ::rtl::OUString     readUniString( bool bAllowNulChars = false );

    /** Ignores nChars characters of a BIFF8 string.
        @param nChars  Number of characters to skip in the stream.
        @param b16BitChars
            True = The character array contains 16-bit characters.
            False = The character array contains truncated 8-bit characters.
     */
    void                skipUniStringChars( sal_uInt16 nChars, bool b16BitChars );

    /** Ignores 8-bit flags, extended header, nChar characters, extended data
        of a BIFF8 string.
        @param nChars  Number of characters to skip in the stream.
     */
    void                skipUniStringBody( sal_uInt16 nChars );

    /** Ignores 16-bit character count, 8-bit flags, extended header, character
        array, extended data of a BIFF8 string.
     */
    void                skipUniString();

    // ------------------------------------------------------------------------
private:
    /** Forwards calls of readValue() template functions to the record buffer. */
    virtual void        readAtom( void* opMem, sal_uInt8 nSize );

    /** Initializes all members after base stream has been seeked to new record. */
    void                setupRecord();
    /** Restarts the current record from the beginning. */
    void                restartRecord( bool bInvalidateRecSize );
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
    /** Goes to start of the next CONTINUE record while reading strings.
        @descr  Stream must be located at the end of a raw record. If reading
        has been started in a CONTINUE record, jumps to an existing following
        CONTINUE record, even if handling of CONTINUE records is disabled (this
        is a special handling for TXO string data). Reads additional Unicode
        flag byte at start of the new raw record and sets or resets rb16BitChars.
        @return  True if next CONTINUE record has been found and initialized. */
    bool                jumpToNextStringContinue( bool& rb16BitChars );
    /** Calculates the complete length of the current record including CONTINUE
        records, stores the length in mnComplRecSize. */
    void                calcRecordLength();

    /** Ensures that reading nBytes bytes is possible with next stream access.
        @descr  Stream must be located at the end of a raw record, and handling
        of CONTINUE records must be enabled.
        @return  True if nBytes can be read from stream. */
    bool                ensureRawReadSize( sal_uInt16 nBytes );
    /** Returns the maximum size of raw data possible to read in one block. */
    sal_uInt16          getMaxRawReadSize( sal_Int32 nBytes ) const;

    /** Reads an array of Unicode characters and appends them to the passed buffer. */
    void                appendUnicodeArray( ::rtl::OUStringBuffer& orBuffer, sal_uInt16 nChars, bool b16BitChars, bool bAllowNulChars );
    /** Reads the BIFF8 Unicode string header fields. */
    void                readUniStringHeader( bool& orb16BitChars, sal_Int32& ornAddSize );

private:
    prv::BiffInputRecordBuffer maRecBuffer; /// Raw record data buffer.

    sal_Int64           mnRecHandle;        /// Handle of current record.
    sal_uInt16          mnRecId;            /// Identifier of current record (not the CONTINUE ID).
    sal_uInt16          mnAltContId;        /// Alternative identifier for content continuation records.

    sal_Int64           mnCurrRecSize;      /// Helper for record size and position.
    sal_Int64           mnComplRecSize;     /// Size of complete record data (with CONTINUEs).
    bool                mbHasComplRec;      /// True = mnComplRecSize is valid.

    bool                mbCont;             /// True = automatic CONTINUE lookup enabled.
};

// ============================================================================

class BiffInputStreamPos
{
public:
    explicit            BiffInputStreamPos( BiffInputStream& rStrm );

    bool                restorePosition();

    inline BiffInputStream& getStream() { return mrStrm; }

private:
    BiffInputStream&    mrStrm;
    sal_Int64           mnRecHandle;
    sal_Int64           mnRecPos;
};

// ============================================================================

/** Stores the current position of the passed stream on construction and
    restores it automatically on destruction. */
class BiffInputStreamPosGuard : private BiffInputStreamPos
{
public:
    explicit            BiffInputStreamPosGuard( BiffInputStream& rStrm );
                        ~BiffInputStreamPosGuard();
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
