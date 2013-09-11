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

    template< typename Type >
    void writeArray( Type* opnArray, sal_Int32 nElemCount );

    /** Stream operator for all data types supported by the writeValue() function. */
    template< typename Type >
    BinaryOutputStream& operator<<( Type nValue ) { writeValue( nValue ); return *this; }

    void writeCompressedUnicodeArray( const OUString& rString, bool bCompressed, bool bAllowNulChars = false );

    void writeCharArrayUC( const OUString& rString, rtl_TextEncoding eTextEnc, bool bAllowNulChars = false );

    void writeUnicodeArray( const OUString& rString, bool bAllowNulChars = false );

protected:
    /** This dummy default c'tor will never call the c'tor of the virtual base
        class BinaryStreamBase as this class cannot be instanciated directly. */
    BinaryOutputStream() : BinaryStreamBase( false ) {}
};

template< typename Type >
void BinaryOutputStream::writeArray( Type* opnArray, sal_Int32 nElemCount )
{
    sal_Int32 nWriteSize = getLimitedValue< sal_Int32, sal_Int32 >( nElemCount, 0, SAL_MAX_INT32 / sizeof( Type ) ) * sizeof( Type );
    ByteOrderConverter::convertLittleEndianArray( opnArray, static_cast< size_t >( nElemCount ) );
    writeMemory( opnArray, nWriteSize, sizeof( Type ) );
}

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
    BinaryXOutputStream& operator<<( Type nValue ) { writeValue( nValue ); return *this; }

    /** Returns the XOutputStream interface of the wrapped output stream. */
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        getXOutputStream() const { return mxOutStrm; }

private:
    StreamDataSequence  maBuffer;       ///< Data buffer used in writeMemory() function.
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        mxOutStrm;      ///< Reference to the output stream.
    bool                mbAutoClose;    ///< True = automatically close stream on destruction.
};

// ============================================================================

/** Wraps a StreamDataSequence and provides convenient access functions.

    The binary data in the stream is written in little-endian format. After
    construction, the stream points to the beginning of the passed data
    sequence. The data sequence is expanded automatically while writing to it.
 */
class OOX_DLLPUBLIC SequenceOutputStream : public SequenceSeekableStream, public BinaryOutputStream
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
    SequenceOutputStream& operator<<( Type nValue ) { writeValue( nValue ); return *this; }
};

// ============================================================================

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
