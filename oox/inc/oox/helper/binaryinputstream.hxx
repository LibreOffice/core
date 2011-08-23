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

#ifndef OOX_HELPER_BINARYINPUTSTREAM_HXX
#define OOX_HELPER_BINARYINPUTSTREAM_HXX

#include <boost/shared_ptr.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include "oox/helper/binarystreambase.hxx"

namespace oox {

class BinaryOutputStream;

// ============================================================================

/** Interface for binary input stream classes.

    The binary data in the stream is assumed to be in little-endian format.
 */
class BinaryInputStream : public virtual BinaryStreamBase
{
public:
    /** Derived classes implement reading nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes ) = 0;
    /** Derived classes implement reading nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes ) = 0;
    /** Derived classes implement seeking the stream forward by the passed
        number of bytes. This should work for non-seekable streams too. */
    virtual void        skip( sal_Int32 nBytes ) = 0;

    /** Reads a value from the stream and converts it to platform byte order.
        Supported types: SAL integers (8 to 64 bit), float, double. */
    template< typename Type >
    void                readValue( Type& ornValue );
    /** Reads a value from the stream and converts it to platform byte order.
        Supported types: SAL integers (8 to 64 bit), float, double. */
    template< typename Type >
    inline Type         readValue() { Type nValue; readValue( nValue ); return nValue; }
    /** Stream operator for integral and floating-point types. */
    template< typename Type >
    inline BinaryInputStream& operator>>( Type& ornValue ) { readValue( ornValue ); return *this; }

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

    /** Reads a NUL-terminated byte character array and returns the string. */
    ::rtl::OString      readNulCharArray();

    /** Reads a NUL-terminated byte character array and returns a Unicode string.
        @param eTextEnc  The text encoding used to create the Unicode string. */
    ::rtl::OUString     readNulCharArrayUC( rtl_TextEncoding eTextEnc );

    /** Reads a NUL-terminated Unicode character array and returns the string. */
    ::rtl::OUString     readNulUnicodeArray();

    /** Reads nChar byte characters and returns the string.
        @param nChars  Number of characters (bytes) to read from the stream.
        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default). */
    ::rtl::OString      readCharArray( sal_Int32 nChars, bool bAllowNulChars = false );

    /** Reads nChar byte characters and returns a Unicode string.
        @param nChars  Number of characters (bytes) to read from the stream.
        @param eTextEnc  The text encoding used to create the Unicode string.
        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default). */
    ::rtl::OUString     readCharArrayUC( sal_Int32 nChars, rtl_TextEncoding eTextEnc, bool bAllowNulChars = false );

    /** Reads nChars Unicode characters and returns the string.
        @param nChars  Number of 16-bit characters to read from the stream.
        @param bAllowNulChars
            True = NUL characters are inserted into the imported string.
            False = NUL characters are replaced by question marks (default). */
    ::rtl::OUString     readUnicodeArray( sal_Int32 nChars, bool bAllowNulChars = false );

    /** Copies nBytes bytes from the current position to the passed output stream. */
    void                copyToStream( BinaryOutputStream& rOutStrm, sal_Int64 nBytes = SAL_MAX_INT64 );

private:
    /** Used by the readValue() template functions to read built-in types.
        @descr  Derived classes may overwrite this default implementation which
            simply calls readMemory() with something own. */
    virtual void        readAtom( void* opMem, sal_uInt8 nSize );
};

typedef ::boost::shared_ptr< BinaryInputStream > BinaryInputStreamRef;

// ----------------------------------------------------------------------------

template< typename Type >
void BinaryInputStream::readValue( Type& ornValue )
{
    // can be instanciated for all types supported in class ByteOrderConverter
    readAtom( &ornValue, static_cast< sal_Int32 >( sizeof( Type ) ) );
    ByteOrderConverter::convertLittleEndian( ornValue );
}

// ============================================================================

/** Wraps a com.sun.star.io.XInputStream and provides convenient access functions.

    The binary data in the stream is assumed to be in little-endian format.
 */
class BinaryXInputStream : public BinaryXSeekableStream, public BinaryInputStream
{
public:
    /** Constructs the wrapper object for the passed input stream.

        @param rxInStream  The com.sun.star.io.XInputStream interface of the
            input stream to be wrapped.
        @param bAutoClose  True = automatically close the wrapped input stream
            on destruction of this wrapper.
     */
    explicit            BinaryXInputStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm,
                            bool bAutoClose );

    virtual             ~BinaryXInputStream();

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes );
    /** Reads nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes );
    /** Seeks the stream forward by the passed number of bytes. This works for
        non-seekable streams too. */
    virtual void        skip( sal_Int32 nBytes );

    /** Stream operator for integral and floating-point types. */
    template< typename Type >
    inline BinaryXInputStream& operator>>( Type& ornValue ) { readValue( ornValue ); return *this; }

    /** Returns the XInputStream interface of the wrapped input stream. */
    inline ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        getXInputStream() const { return mxInStrm; }
    /** Closes the wrapped XInputStream. */
    void                close();

private:
    StreamDataSequence  maBuffer;       /// Data buffer used in readMemory() function.
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        mxInStrm;       /// Reference to the input stream.
    bool                mbAutoClose;    /// True = automatically close stream on destruction.
};

typedef ::boost::shared_ptr< BinaryXInputStream > BinaryXInputStreamRef;

// ============================================================================

/** Wraps a StreamDataSequence and provides convenient access functions.

    The binary data in the stream is assumed to be in little-endian format.
 */
class SequenceInputStream : public SequenceSeekableStream, public BinaryInputStream
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
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes );
    /** Reads nBytes bytes to the (existing) buffer opMem.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes );
    /** Seeks the stream forward by the passed number of bytes. This works for
        non-seekable streams too. */
    virtual void        skip( sal_Int32 nBytes );

    /** Stream operator for integral and floating-point types. */
    template< typename Type >
    inline SequenceInputStream& operator>>( Type& ornValue ) { readValue( ornValue ); return *this; }
};

typedef ::boost::shared_ptr< SequenceInputStream > SequenceInputStreamRef;

// ============================================================================

/** Wraps a BinaryInputStream and provides access to a specific part of the
    stream data.

    @descr
        Provides access to the stream data block starting at the current
        position of the stream, and with a specific length. If the wrapped
        stream is seekable, this wrapper will treat the position the wrapped
        has at construction time as position "0" (therefore the class name).
 */
class RelativeInputStream : public BinaryInputStream
{
public:
    /** Constructs the wrapper object for the passed stream.

        @attention
            The passed input stream MUST live at least as long as this stream
            wrapper. The stream MUST NOT be changed from outside as long as
            this stream wrapper is used to read from it.
            
        @param nLength
            If specified, restricts the amount of data that can be read from
            the passed input stream.
     */
    explicit            RelativeInputStream(
                            BinaryInputStream& rInStrm,
                            sal_Int64 nLength = SAL_MAX_INT64 );

    /** Returns whether the wrapped stream is seekable. */
    virtual bool        isSeekable() const;
    /** Returns the size of the data block in the wrapped stream offered by
        this wrapper. */
    virtual sal_Int64   getLength() const;
    /** Returns the current relative stream position. */
    virtual sal_Int64   tell() const;
    /** Seeks the stream to the passed relative position, if the wrapped stream
        is seekable. */
    virtual void        seek( sal_Int64 nPos );

    /** Reads nBytes bytes to the passed sequence. Does not read out of the
        data block whose size has been specified on construction.
        @return  Number of bytes really read. */
    virtual sal_Int32   readData( StreamDataSequence& orData, sal_Int32 nBytes );
    /** Reads nBytes bytes to the (existing) buffer opMem. Does not read out of
        the data block whose size has been specified on construction.
        @return  Number of bytes really read. */
    virtual sal_Int32   readMemory( void* opMem, sal_Int32 nBytes );
    /** Seeks the stream forward by the passed number of bytes. This works for
        non-seekable streams too. Does not seek out of the data block. */
    virtual void        skip( sal_Int32 nBytes );

    /** Stream operator for integral and floating-point types. */
    template< typename Type >
    inline RelativeInputStream& operator>>( Type& ornValue ) { readValue( ornValue ); return *this; }

private:
    BinaryInputStream&  mrInStrm;
    sal_Int64           mnStartPos;
    sal_Int64           mnRelPos;
    sal_Int64           mnLength;
};

typedef ::boost::shared_ptr< RelativeInputStream > RelativeInputStreamRef;

// ============================================================================

} // namespace oox

#endif

