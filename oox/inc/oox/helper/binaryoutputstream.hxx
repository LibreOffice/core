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

#ifndef OOX_HELPER_BINARYOUTPUTSTREAM_HXX
#define OOX_HELPER_BINARYOUTPUTSTREAM_HXX

#include <boost/shared_ptr.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include "oox/helper/binarystreambase.hxx"

namespace oox {

// ============================================================================

/** Interface for binary output stream classes.

    The binary data in the stream is written in little-endian format.
 */
class BinaryOutputStream : public virtual BinaryStreamBase
{
public:
    /** Derived classes implement writing the passed data sequence. */
    virtual void        writeData( const StreamDataSequence& rData ) = 0;
    /** Derived classes implement writing from the (existing) buffer pMem. */
    virtual void        writeMemory( const void* pMem, sal_Int32 nBytes ) = 0;

    /** Writes a value to the stream and converts it to platform byte order.
        Supported types: SAL integers (8 to 64 bit), float, double. */
    template< typename Type >
    void                writeValue( Type nValue );
    /** Stream operator for integral and floating-point types. */
    template< typename Type >
    inline BinaryOutputStream& operator<<( Type nValue ) { writeValue( nValue ); return *this; }

private:
    /** Used by the writeValue() template function to write built-in types.
        @descr  Derived classes may overwrite this default implementation which
            simply calls writeMemory() with something own. */
    virtual void        writeAtom( const void* pMem, sal_uInt8 nSize );
};

typedef ::boost::shared_ptr< BinaryOutputStream > BinaryOutputStreamRef;

// ----------------------------------------------------------------------------

template< typename Type >
void BinaryOutputStream::writeValue( Type nValue )
{
    // can be instanciated for all types supported in class ByteOrderConverter
    ByteOrderConverter::convertLittleEndian( nValue );
    writeMemory( &nValue, static_cast< sal_Int32 >( sizeof( Type ) ) );
}

// ============================================================================

/** Wraps a com.sun.star.io.XOutputStream and provides convenient access functions.

    The binary data in the stream is written in little-endian format.
 */
class BinaryXOutputStream : public BinaryXSeekableStream, public BinaryOutputStream
{
public:
    /** Constructs the wrapper object for the passed output stream.

        @param rxOutStream  The com.sun.star.io.XOutputStream interface of the
            output stream to be wrapped.
        @param bAutoClose  True = automatically close the wrapped output stream
            on destruction of this wrapper.
     */
    explicit            BinaryXOutputStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rxOutStrm,
                            bool bAutoClose );

    virtual             ~BinaryXOutputStream();

    /** Writes the passed data sequence. */
    virtual void        writeData( const StreamDataSequence& rData );
    /** Write nBytes bytes from the (existing) buffer pMem. */
    virtual void        writeMemory( const void* pMem, sal_Int32 nBytes );

    /** Stream operator for integral and floating-point types. */
    template< typename Type >
    inline BinaryXOutputStream& operator<<( Type nValue ) { writeValue( nValue ); return *this; }

    /** Returns the XOutputStream interface of the wrapped output stream. */
    inline ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        getXOutputStream() const { return mxOutStrm; }
    /** Flushes and closes the wrapped XOutputStream. */
    void                close();

private:
    StreamDataSequence  maBuffer;       /// Data buffer used in writeMemory() function.
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        mxOutStrm;      /// Reference to the output stream.
    bool                mbAutoClose;    /// True = automatically close stream on destruction.
};

typedef ::boost::shared_ptr< BinaryXOutputStream > BinaryXOutputStreamRef;

// ============================================================================

/** Wraps a StreamDataSequence and provides convenient access functions.

    The binary data in the stream is written in little-endian format. After
    construction, the stream points to the beginning of the passed data
    sequence. The data sequence is expanded automatically while writing to it.
 */
class SequenceOutputStream : public SequenceSeekableStream, public BinaryOutputStream
{
public:
    /** Constructs the wrapper object for the passed data sequence.

        @attention
            The passed data sequence MUST live at least as long as this stream
            wrapper. The data sequence MUST NOT be changed from outside as long
            as this stream wrapper is used to write to it.
     */
    explicit            SequenceOutputStream( StreamDataSequence& rData );

    /** Writes the passed data sequence. */
    virtual void        writeData( const StreamDataSequence& rData );
    /** Write nBytes bytes from the (existing) buffer pMem. */
    virtual void        writeMemory( const void* pMem, sal_Int32 nBytes );

    /** Stream operator for integral and floating-point types. */
    template< typename Type >
    inline SequenceOutputStream& operator<<( Type nValue ) { writeValue( nValue ); return *this; }
};

typedef ::boost::shared_ptr< SequenceOutputStream > SequenceOutputStreamRef;

// ============================================================================

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
