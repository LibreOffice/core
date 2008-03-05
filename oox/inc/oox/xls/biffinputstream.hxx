/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: biffinputstream.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:01:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_XLS_BIFFINPUTSTREAM_HXX
#define OOX_XLS_BIFFINPUTSTREAM_HXX

#include <vector>
#include "oox/xls/biffhelper.hxx"
#include "oox/xls/biffcodec.hxx"

namespace rtl { class OUStringBuffer; }
namespace oox { class BinaryInputStream; }

namespace oox {
namespace xls {

// ============================================================================

const sal_uInt32 BIFF_REC_SEEK_TO_BEGIN     = 0;
const sal_uInt32 BIFF_REC_SEEK_TO_END       = SAL_MAX_UINT32;

// ============================================================================

namespace prv {

/** Buffers the contents of a raw record and encapsulates stream decoding. */
class BiffInputRecordBuffer
{
public:
    explicit            BiffInputRecordBuffer( BinaryInputStream& rInStrm );

    /** Returns the core stream object. */
    inline const BinaryInputStream&
                        getCoreStream() const { return mrInStrm; }

    /** Sets a decoder object and decrypts buffered record data. */
    void                setDecoder( BiffDecoderRef xDecoder );
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
    /** Reads a value. Must NOT overread the buffer. */
    template< typename Type >
    inline void         readValue( Type& ornValue );
    /** Ignores nBytes bytes. Must NOT overread the buffer. */
    inline void         skip( sal_uInt16 nBytes );

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

// ----------------------------------------------------------------------------

template< typename Type >
inline void BiffInputRecordBuffer::readValue( Type& ornValue )
{
    read( &ornValue, static_cast< sal_uInt16 >( sizeof( Type ) ) );
    ByteOrderConverter::convertLittleEndian( ornValue );
}

} // namespace prv

// ============================================================================

/** This class is used to import BIFF streams.

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
    setDecoder() sets´a new decryption algorithm and initially enables it.
    enableDecoder( false ) may be used to stop the usage of the decryption
    temporarily (sometimes record contents are never encrypted, e.g. all BOF
    records or the stream position in SHEET records). Decryption will be
    reenabled automatically, if a new record is started with the function
    startNextRecord().

    Be careful with used data types:
    sal_uInt16: Record identifiers, raw size of single records.
    sal_uInt32: Record position and size (including CONTINUE records).
    sal_Int64: Core stream position and size.
*/
class BiffInputStream
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

                        ~BiffInputStream();

    // record control ---------------------------------------------------------

    /** Sets stream pointer to the start of the next record content.

        Ignores all CONTINUE records of the current record, if automatic
        CONTINUE usage is switched on.

        @return  False = no record found (end of stream).
     */
    bool                startNextRecord();

    /** Sets stream pointer to the start of the content of the specified record.

        The handle of the current record can be received and stored using the
        function getRecHandle() for later usage with this function.

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
    void                setDecoder( BiffDecoderRef xDecoder );

    /** Returns the current decoder object. */
    BiffDecoderRef      getDecoder() const;

    /** Enables/disables usage of current decoder.

        Decryption is reenabled automatically, if a new record is started using
        the function startNextRecord().
     */
    void                enableDecoder( bool bEnable = true );

    // stream/record state and info -------------------------------------------

    /** Returns record reading state: false = record overread. */
    inline bool         isValid() const { return mbValid; }
    /** Returns the current record identifier. */
    inline sal_uInt16   getRecId() const { return mnRecId; }
    /** Returns the position inside of the whole record content. */
    sal_uInt32          getRecPos() const;
    /** Returns the data size of the whole record without record headers. */
    sal_uInt32          getRecSize();
    /** Returns remaining data size of the whole record without record headers. */
    sal_uInt32          getRecLeft();
    /** Returns a unique handle for the current record that can be used with
        the function startRecordByHandle(). */
    inline sal_Int64    getRecHandle() const { return mnRecHandle; }
    /** Returns the record identifier of the following record. */
    sal_uInt16          getNextRecId();

    /** Returns the absolute core stream position. */
    sal_Int64           getCoreStreamPos() const;
    /** Returns the stream size. */
    sal_Int64           getCoreStreamSize() const;

    // stream read access -----------------------------------------------------

    /** Reads nBytes bytes and copies them to the passed buffer opData.

        @return  Number of bytes really read.
     */
    sal_uInt32          read( void* opData, sal_uInt32 nBytes );

    /** Reads a value from the stream and converts it to platform byte order. */
    template< typename Type >
    void                readValue( Type& ornValue );
    /** Reads a value from the stream and converts it to platform byte order. */
    template< typename Type >
    inline Type         readValue() { Type nValue; readValue( nValue ); return nValue; }

    inline sal_Int8     readInt8() { return readValue< sal_Int8 >(); }
    inline sal_uInt8    readuInt8() { return readValue< sal_uInt8 >(); }
    inline sal_Int16    readInt16() { return readValue< sal_Int16 >(); }
    inline sal_uInt16   readuInt16() { return readValue< sal_uInt16 >(); }
    inline sal_Int32    readInt32() { return readValue< sal_Int32 >(); }
    inline sal_uInt32   readuInt32() { return readValue< sal_uInt32 >(); }
    inline sal_Int64    readInt64() { return readValue< sal_Int64 >(); }
    inline sal_uInt64   readuInt64() { return readValue< sal_uInt64 >(); }
    inline float        readFloat() { return readValue< float >(); }
    inline double       readDouble() { return readValue< double >(); }

    // seeking ----------------------------------------------------------------

    /** Seeks absolute in record content to the specified position.

        The value 0 means start of record, independent from physical stream
        position.
     */
    BiffInputStream&    seek( sal_uInt32 nRecPos );

    /** Seeks forward inside the current record. */
    BiffInputStream&    skip( sal_uInt32 nBytes );

    // strings ----------------------------------------------------------------

    /** Enables/disables reading NUL characters into strings.

        Default behaviour is to replace NUL characters with question marks.
        This default behaviour is restored automatically, if a new record is
        started using the function startNextRecord().

        @param bEnable
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default).
     */
    inline void         enableNulChars( bool bEnable ) { mbNulChars = bEnable; }

    // byte strings -----------------------------------------------------------

    /** Reads nChar byte characters and returns the string. */
    ::rtl::OString      readCharArray( sal_uInt16 nChars );
    /** Reads nChar byte characters and returns the string. */
    ::rtl::OUString     readCharArray( sal_uInt16 nChars, rtl_TextEncoding eTextEnc );

    /** Reads 8/16 bit string length and character array, and returns the string. */
    ::rtl::OString      readByteString( bool b16BitLen );
    /** Reads 8/16 bit string length and character array, and returns the string. */
    ::rtl::OUString     readByteString( bool b16BitLen, rtl_TextEncoding eTextEnc );

    /** Ignores 8/16 bit string length and character array. */
    void                skipByteString( bool b16BitLen );

    // Unicode strings --------------------------------------------------------

    /** Reads nChars Unicode characters and returns the string. */
    ::rtl::OUString     readUnicodeArray( sal_uInt16 nChars );

    /** Reads nChars characters of a BIFF8 string, and returns the string. */
    ::rtl::OUString     readUniStringChars( sal_uInt16 nChars, bool b16Bit );
    /** Reads 8 bit flags, extended header, nChar characters, extended data of
        a BIFF8 string, and returns the string. */
    ::rtl::OUString     readUniString( sal_uInt16 nChars );
    /** Reads 16 bit character count, 8 bit flags, extended header, character
        array, extended data of a BIFF8 string, and returns the string. */
    ::rtl::OUString     readUniString();

    /** Ignores nChars characters of a BIFF8 string. */
    void                skipUniStringChars( sal_uInt16 nChars, bool b16Bit );
    /** Ignores 8 bit flags, extended header, nChar characters, extended data
        of a BIFF8 string. */
    void                skipUniString( sal_uInt16 nChars );
    /** Ignores 16 bit character count, 8 bit flags, extended header, character
        array, extended data of a BIFF8 string. */
    void                skipUniString();

    // ------------------------------------------------------------------------
private:
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
        @return  Copy of mbValid. */
    bool                jumpToNextContinue();
    /** Goes to start of the next CONTINUE record while reading strings.
        @descr  Stream must be located at the end of a raw record. If reading
        has been started in a CONTINUE record, jumps to an existing following
        CONTINUE record, even if handling of CONTINUE records is disabled (this
        is a special handling for TXO string data). Reads additional Unicode
        flag byte at start of the new raw record and sets or resets rb16Bit.
        @return  Copy of mbValid. */
    bool                jumpToNextStringContinue( bool& rb16Bit );

    /** Ensures that reading nBytes bytes is possible with next stream access.
        @descr  Stream must be located at the end of a raw record, and handling
        of CONTINUE records must be enabled.
        @return  Copy of mbValid. */
    bool                ensureRawReadSize( sal_uInt16 nBytes );
    /** Returns the maximum size of raw data possible to read in one block. */
    sal_uInt16          getMaxRawReadSize( sal_uInt32 nBytes ) const;

    /** Reads an array of Unicode characters and appends them to the passed buffer. */
    void                appendUnicodeArray( ::rtl::OUStringBuffer& orBuffer, sal_uInt16 nChars, bool b16Bit );
    /** Reads the BIFF8 Unicode string header fields. */
    void                readUniStringHeader( bool& orb16Bit, sal_uInt32& ornAddSize );

private:
    prv::BiffInputRecordBuffer maRecBuffer; /// Raw record data buffer.

    sal_Int64           mnRecHandle;        /// Handle of current record.
    sal_uInt16          mnRecId;            /// Identifier of current record (not the CONTINUE ID).
    sal_uInt16          mnAltContId;        /// Alternative identifier for content continuation records.

    sal_uInt32          mnCurrRecSize;      /// Helper for record size and position.
    sal_uInt32          mnComplRecSize;     /// Size of complete record data (with CONTINUEs).
    bool                mbHasComplRec;      /// True = mnComplRecSize is valid.

    bool                mbCont;             /// True = automatic CONTINUE lookup enabled.
    bool                mbNulChars;         /// True = import NUL characters.
    bool                mbValid;            /// True = last stream operation successful (no overread).
};

// ----------------------------------------------------------------------------

template< typename Type >
inline void BiffInputStream::readValue( Type& ornValue )
{
    if( ensureRawReadSize( static_cast< sal_uInt16 >( sizeof( Type ) ) ) )
        maRecBuffer.readValue( ornValue );
}

template< typename Type >
inline BiffInputStream& operator>>( BiffInputStream& rStrm, Type& ornValue )
{
    rStrm.readValue( ornValue );
    return rStrm;
}

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
    sal_uInt32          mnRecPos;
};

// ============================================================================

class BiffInputStreamGuard : private BiffInputStreamPos
{
public:
    explicit            BiffInputStreamGuard( BiffInputStream& rStrm );
                        ~BiffInputStreamGuard();
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

