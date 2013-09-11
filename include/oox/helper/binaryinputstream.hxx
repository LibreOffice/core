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

#ifndef OOX_HELPER_BINARYINPUTSTREAM_HXX
#define OOX_HELPER_BINARYINPUTSTREAM_HXX

#include <vector>
#include <com/sun/star/io/XInputStream.hpp>
#include "oox/helper/binarystreambase.hxx"

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
} } }

namespace oox {

class BinaryOutputStream;

// ============================================================================

/** Interface for binary input stream classes.

    The binary data in the stream is assumed to be in little-endian format.
 */
class OOX_DLLPUBLIC BinaryInputStream : public virtual BinaryStreamBase
{
public:
    /** Derived classes implement reading nBytes bytes to the passed sequence.
        The sequence will be reallocated internally.

        @param nAtomSize
            The size of the elements in the memory block, if available. Derived
            classes may be interested in this information.

        @return
            Number of bytes really read.
     */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize = 1 ) = 0;

    /** Derived classes implement reading nBytes bytes to the (preallocated!)
        memory buffer opMem.

        @param nAtomSize
            The size of the elements in the memory block, if available. Derived
            classes may be interested in this information.

        @return
            Number of bytes really read.
     */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize = 1 ) = 0;

    /** Derived classes implement seeking the stream forward by the passed
        number of bytes. This should work for non-seekable streams too.

        @param nAtomSize
            The size of the elements in the memory block, if available. Derived
            classes may be interested in this information.
     */
    virtual void        skip( sal_Int32 nBytes, size_t nAtomSize = 1 ) = 0;

    /** Reads a value from the stream and converts it to platform byte order.
        All data types supported by the ByteOrderConverter class can be used.
     */
    template< typename Type >
    void                readValue( Type& ornValue );

    /** Reads a value from the stream and converts it to platform byte order.
        All data types supported by the ByteOrderConverter class can be used.
     */
    template< typename Type >
    Type         readValue() { Type nValue; readValue( nValue ); return nValue; }

    /** Stream operator for all data types supported by the readValue() function. */
    template< typename Type >
    BinaryInputStream& operator>>( Type& ornValue ) { readValue( ornValue ); return *this; }

    sal_Int8     readInt8() { return readValue< sal_Int8 >(); }
    sal_uInt8    readuInt8() { return readValue< sal_uInt8 >(); }
    sal_Int16    readInt16() { return readValue< sal_Int16 >(); }
    sal_uInt16   readuInt16() { return readValue< sal_uInt16 >(); }
    sal_Int32    readInt32() { return readValue< sal_Int32 >(); }
    sal_uInt32   readuInt32() { return readValue< sal_uInt32 >(); }
    sal_Int64    readInt64() { return readValue< sal_Int64 >(); }
    sal_uInt64   readuInt64() { return readValue< sal_uInt64 >(); }
    float        readFloat() { return readValue< float >(); }
    double       readDouble() { return readValue< double >(); }

    /** Reads a (preallocated!) C array of values from the stream.

        Converts all values in the array to platform byte order. All data types
        supported by the ByteOrderConverter class can be used.

        @param nElemCount
            Number of array elements to read (NOT byte count).

        @return
            Number of array elements really read (NOT byte count).
     */
    template< typename Type >
    sal_Int32           readArray( Type* opnArray, sal_Int32 nElemCount );

    /** Reads a sequence of values from the stream.

        The sequence will be reallocated internally. Converts all values in the
        array to platform byte order. All data types supported by the
        ByteOrderConverter class can be used.

        @param nElemCount
            Number of elements to put into the sequence (NOT byte count).

        @return
            Number of sequence elements really read (NOT byte count).
     */
    template< typename Type >
    sal_Int32           readArray( ::com::sun::star::uno::Sequence< Type >& orSequence, sal_Int32 nElemCount );

    /** Reads a vector of values from the stream.

        The vector will be resized internally. Converts all values in the
        vector to platform byte order. All data types supported by the
        ByteOrderConverter class can be used.

        @param nElemCount
            Number of elements to put into the vector (NOT byte count).

        @return
            Number of vector elements really read (NOT byte count).
     */
    template< typename Type >
    sal_Int32           readArray( ::std::vector< Type >& orVector, sal_Int32 nElemCount );

    /** Skips an array of values of a certain type in the stream.

        All data types supported by the ByteOrderConverter class can be used.

        @param nElemCount
            Number of array elements to skip (NOT byte count).
     */
    template< typename Type >
    void                skipArray( sal_Int32 nElemCount );

    /** Reads a NUL-terminated Unicode character array and returns the string.
     */
    OUString     readNulUnicodeArray();

    /** Reads a byte character array and returns the string.

        @param nChars
            Number of characters (bytes) to read from the stream.

        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default).
     */
    OString      readCharArray( sal_Int32 nChars, bool bAllowNulChars = false );

    /** Reads a byte character array and returns a Unicode string.

        @param nChars
            Number of characters (bytes) to read from the stream.

        @param eTextEnc
            The text encoding used to create the Unicode string.

        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default).
     */
    OUString     readCharArrayUC( sal_Int32 nChars, rtl_TextEncoding eTextEnc, bool bAllowNulChars = false );

    /** Reads a Unicode character array and returns the string.

        @param nChars
            Number of 16-bit characters to read from the stream.

        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default).
     */
    OUString     readUnicodeArray( sal_Int32 nChars, bool bAllowNulChars = false );

    /** Reads a Unicode character array (may be compressed) and returns the
        string.

        @param nChars
            Number of 8-bit or 16-bit characters to read from the stream.

        @param bCompressed
            True = Character array is compressed (stored as 8-bit characters).
            False = Character array is not compressed (stored as 16-bit characters).

        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default).
     */
    OUString     readCompressedUnicodeArray( sal_Int32 nChars, bool bCompressed, bool bAllowNulChars = false );

    /** Copies nBytes bytes from the current position to the passed output stream.
     */
    void                copyToStream( BinaryOutputStream& rOutStrm, sal_Int64 nBytes = SAL_MAX_INT64, sal_Int32 nAtomSize = 1 );

protected:
    /** This dummy default c'tor will never call the c'tor of the virtual base
        class BinaryStreamBase as this class cannot be instanciated directly. */
    BinaryInputStream() : BinaryStreamBase( false ) {}
};

typedef ::boost::shared_ptr< BinaryInputStream > BinaryInputStreamRef;

// ----------------------------------------------------------------------------

template< typename Type >
void BinaryInputStream::readValue( Type& ornValue )
{
    readMemory( &ornValue, static_cast< sal_Int32 >( sizeof( Type ) ), sizeof( Type ) );
    ByteOrderConverter::convertLittleEndian( ornValue );
}

template< typename Type >
sal_Int32 BinaryInputStream::readArray( Type* opnArray, sal_Int32 nElemCount )
{
    sal_Int32 nRet = 0;
    if( !mbEof )
    {
        sal_Int32 nReadSize = getLimitedValue< sal_Int32, sal_Int32 >( nElemCount, 0, SAL_MAX_INT32 / sizeof( Type ) ) * sizeof( Type );
        nRet = readMemory( opnArray, nReadSize, sizeof( Type ) ) / sizeof( Type );
        ByteOrderConverter::convertLittleEndianArray( opnArray, static_cast< size_t >( nRet ) );
    }
    return nRet;
}

template< typename Type >
sal_Int32 BinaryInputStream::readArray( ::com::sun::star::uno::Sequence< Type >& orSequence, sal_Int32 nElemCount )
{
    orSequence.reallocate( nElemCount );
    return orSequence.hasElements() ? readArray( orSequence.getArray(), nElemCount ) : 0;
}

template< typename Type >
sal_Int32 BinaryInputStream::readArray( ::std::vector< Type >& orVector, sal_Int32 nElemCount )
{
    orVector.resize( static_cast< size_t >( nElemCount ) );
    return orVector.empty() ? 0 : readArray( &orVector.front(), nElemCount );
}

template< typename Type >
void BinaryInputStream::skipArray( sal_Int32 nElemCount )
{
    sal_Int32 nSkipSize = getLimitedValue< sal_Int32, sal_Int32 >( nElemCount, 0, SAL_MAX_INT32 / sizeof( Type ) ) * sizeof( Type );
    skip( nSkipSize, sizeof( Type ) );
}

// ============================================================================

/** Wraps a UNO input stream and provides convenient access functions.

    The binary data in the stream is assumed to be in little-endian format.
 */
class OOX_DLLPUBLIC BinaryXInputStream : public BinaryXSeekableStream, public BinaryInputStream
{
public:
    /** Constructs the wrapper object for the passed input stream.

        @param rxInStream
            The com.sun.star.io.XInputStream interface of the UNO input stream
            to be wrapped.

        @param bAutoClose
            True = automatically close the wrapped input stream on destruction
            of this wrapper or when close() is called.
     */
    explicit            BinaryXInputStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm,
                            bool bAutoClose );

    virtual             ~BinaryXInputStream();

    /** Closes the input stream. Does also close the wrapped UNO input stream
        if bAutoClose has been set to true in the constructor. */
    virtual void        close();

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Reads nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Seeks the stream forward by the passed number of bytes. This works for
        non-seekable streams too. */
    virtual void        skip( sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Stream operator for all data types supported by the readValue() function. */
    template< typename Type >
    BinaryXInputStream& operator>>( Type& ornValue ) { readValue( ornValue ); return *this; }

private:
    StreamDataSequence  maBuffer;       ///< Data buffer used in readMemory() function.
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        mxInStrm;       ///< Reference to the input stream.
    bool                mbAutoClose;    ///< True = automatically close stream on destruction.
};

// ============================================================================

/** Wraps a StreamDataSequence and provides convenient access functions.

    The binary data in the stream is assumed to be in little-endian format.
 */
class OOX_DLLPUBLIC SequenceInputStream : public SequenceSeekableStream, public BinaryInputStream
{
public:
    /** Constructs the wrapper object for the passed data sequence.

        @attention
            The passed data sequence MUST live at least as long as this stream
            wrapper. The data sequence MUST NOT be changed from outside as long
            as this stream wrapper is used to read from it.
     */
    explicit            SequenceInputStream( const StreamDataSequence& rData );

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Reads nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Seeks the stream forward by the passed number of bytes. This works for
        non-seekable streams too. */
    virtual void        skip( sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Stream operator for all data types supported by the readValue() function. */
    template< typename Type >
    SequenceInputStream& operator>>( Type& ornValue ) { readValue( ornValue ); return *this; }

private:
    /** Returns the number of bytes available in the sequence for the passed byte count. */
    sal_Int32    getMaxBytes( sal_Int32 nBytes ) const
                            { return getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, mpData->getLength() - mnPos ); }
};

// ============================================================================

/** Wraps a BinaryInputStream and provides access to a specific part of the
    stream data.

    Provides access to the stream data block starting at the current position
    of the stream, and with a specific length. If the wrapped stream is
    seekable, this wrapper will treat the position of the wrapped stream at
    construction time as position "0" (therefore the class name).

    The passed input stream MUST live at least as long as this stream wrapper.
    The stream MUST NOT be changed from outside as long as this stream wrapper
    is used to read from it.
 */
class RelativeInputStream : public BinaryInputStream
{
public:
    /** Constructs the wrapper object for the passed stream.

        @param nSize
            If specified, restricts the amount of data that can be read from
            the passed input stream.
     */
    explicit            RelativeInputStream(
                            BinaryInputStream& rInStrm,
                            sal_Int64 nSize = SAL_MAX_INT64 );

    /** Returns the size of the data block in the wrapped stream offered by
        this wrapper. */
    virtual sal_Int64   size() const;

    /** Returns the current relative stream position. */
    virtual sal_Int64   tell() const;

    /** Seeks the stream to the passed relative position, if the wrapped stream
        is seekable. */
    virtual void        seek( sal_Int64 nPos );

    /** Closes the input stream but not the wrapped stream. */
    virtual void        close();

    /** Reads nBytes bytes to the passed sequence. Does not read out of the
        data block whose size has been specified on construction.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Reads nBytes bytes to the (existing) buffer opMem. Does not read out of
        the data block whose size has been specified on construction.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Seeks the stream forward by the passed number of bytes. This works for
        non-seekable streams too. Does not seek out of the data block. */
    virtual void        skip( sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Stream operator for all data types supported by the readValue() function. */
    template< typename Type >
    RelativeInputStream& operator>>( Type& ornValue ) { readValue( ornValue ); return *this; }

private:
    /** Returns the number of bytes available in the sequence for the passed byte count. */
    sal_Int32    getMaxBytes( sal_Int32 nBytes ) const
                            { return getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, mnSize - mnRelPos ); }

private:
    BinaryInputStream*  mpInStrm;
    sal_Int64           mnStartPos;
    sal_Int64           mnRelPos;
    sal_Int64           mnSize;
};

// ============================================================================

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
