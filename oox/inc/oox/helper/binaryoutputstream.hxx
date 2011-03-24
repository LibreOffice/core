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

#include "oox/helper/binarystreambase.hxx"

namespace com { namespace sun { namespace star {
    namespace io { class XOutputStream; }
} } }

namespace oox {

// ============================================================================

/** Interface for binary output stream classes.

    The binary data in the stream is written in little-endian format.
 */
class BinaryOutputStream : public virtual BinaryStreamBase
{
public:
    /** Derived classes implement writing the contents of the passed data
        sequence.

        @param nAtomSize
            The size of the elements in the memory block, if available. Derived
            classes may be interested in this information.
     */
    virtual void        writeData( const StreamDataSequence& rData, size_t nAtomSize = 1 ) = 0;

    /** Derived classes implement writing the contents of the (preallocated!)
        memory buffer pMem.

        @param nAtomSize
            The size of the elements in the memory block, if available. Derived
            classes may be interested in this information.
     */
    virtual void        writeMemory( const void* pMem, sal_Int32 nBytes, size_t nAtomSize = 1 ) = 0;

    /** Writes a value to the stream and converts it to platform byte order.
        All data types supported by the ByteOrderConverter class can be used.
     */
    template< typename Type >
    void                writeValue( Type nValue );

    /** Stream operator for all data types supported by the writeValue() function. */
    template< typename Type >
    inline BinaryOutputStream& operator<<( Type nValue ) { writeValue( nValue ); return *this; }

protected:
    /** This dummy default c'tor will never call the c'tor of the virtual base
        class BinaryStreamBase as this class cannot be instanciated directly. */
    inline explicit     BinaryOutputStream() : BinaryStreamBase( false ) {}
};

typedef ::boost::shared_ptr< BinaryOutputStream > BinaryOutputStreamRef;

// ----------------------------------------------------------------------------

template< typename Type >
void BinaryOutputStream::writeValue( Type nValue )
{
    ByteOrderConverter::convertLittleEndian( nValue );
    writeMemory( &nValue, static_cast< sal_Int32 >( sizeof( Type ) ), sizeof( Type ) );
}

// ============================================================================

/** Wraps a UNO output stream and provides convenient access functions.

    The binary data in the stream is written in little-endian format.
 */
class BinaryXOutputStream : public BinaryXSeekableStream, public BinaryOutputStream
{
public:
    /** Constructs the wrapper object for the passed output stream.

        @param rxOutStream
            The com.sun.star.io.XOutputStream interface of the output stream to
            be wrapped.

        @param bAutoClose
            True = automatically close the wrapped output stream on destruction
            of this wrapper or when close() is called.
     */
    explicit            BinaryXOutputStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rxOutStrm,
                            bool bAutoClose );

    virtual             ~BinaryXOutputStream();

    /** Flushes and closes the output stream. Does also close the wrapped UNO
        output stream if bAutoClose has been set to true in the constructor. */
    void                close();

    /** Writes the passed data sequence. */
    virtual void        writeData( const StreamDataSequence& rData, size_t nAtomSize = 1 );

    /** Write nBytes bytes from the (preallocated!) buffer pMem. */
    virtual void        writeMemory( const void* pMem, sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Stream operator for all data types supported by the writeValue() function. */
    template< typename Type >
    inline BinaryXOutputStream& operator<<( Type nValue ) { writeValue( nValue ); return *this; }

    /** Returns the XOutputStream interface of the wrapped output stream. */
    inline ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        getXOutputStream() const { return mxOutStrm; }

private:
    StreamDataSequence  maBuffer;       /// Data buffer used in writeMemory() function.
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        mxOutStrm;      /// Reference to the output stream.
    bool                mbAutoClose;    /// True = automatically close stream on destruction.
};

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
    virtual void        writeData( const StreamDataSequence& rData, size_t nAtomSize = 1 );

    /** Write nBytes bytes from the (preallocated!) buffer pMem. */
    virtual void        writeMemory( const void* pMem, sal_Int32 nBytes, size_t nAtomSize = 1 );

    /** Stream operator for all data types supported by the writeValue() function. */
    template< typename Type >
    inline SequenceOutputStream& operator<<( Type nValue ) { writeValue( nValue ); return *this; }
};

// ============================================================================

} // namespace oox

#endif
