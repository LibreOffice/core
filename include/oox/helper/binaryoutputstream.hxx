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

#ifndef INCLUDED_OOX_HELPER_BINARYOUTPUTSTREAM_HXX
#define INCLUDED_OOX_HELPER_BINARYOUTPUTSTREAM_HXX

#include <cstddef>
#include <memory>

#include <com/sun/star/uno/Reference.hxx>
#include <oox/dllapi.h>
#include <oox/helper/binarystreambase.hxx>
#include <oox/helper/helper.hxx>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com::sun::star {
    namespace io { class XOutputStream; }
}

namespace oox {


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

    template< typename Type >
    void writeArray( Type* opnArray, sal_Int32 nElemCount );

    template< typename Type >
    void writeArray( const Type* opnArray, sal_Int32 nElemCount );

    /** Writes a value to the stream and converts it to platform byte order.
        All data types supported by the ByteOrderConverter class can be used.
     */
    template< typename Type >
    void                  writeValue( Type nValue );

    BinaryOutputStream&   WriteInt16(sal_Int16 x)   { writeValue(x); return *this; }
    BinaryOutputStream&   WriteUInt16(sal_uInt16 x) { writeValue(x); return *this; }
    BinaryOutputStream&   WriteInt32(sal_Int32 x)   { writeValue(x); return *this; }
    BinaryOutputStream&   WriteUInt32(sal_uInt32 x) { writeValue(x); return *this; }
    BinaryOutputStream&   WriteInt64(sal_Int64 x)   { writeValue(x); return *this; }

    void writeCompressedUnicodeArray( const OUString& rString, bool bCompressed );

    void writeCharArrayUC( std::u16string_view rString, rtl_TextEncoding eTextEnc );

    void writeUnicodeArray( const OUString& rString );

protected:
    BinaryOutputStream() = default;

private:
    BinaryOutputStream( BinaryOutputStream const& ) = delete;
    BinaryOutputStream& operator=( BinaryOutputStream const& ) = delete;
};

template< typename Type >
void BinaryOutputStream::writeArray( Type* opnArray, sal_Int32 nElemCount )
{
    sal_Int32 nWriteSize = getLimitedValue< sal_Int32, sal_Int32 >( nElemCount, 0, SAL_MAX_INT32 / sizeof( Type ) ) * sizeof( Type );
    ByteOrderConverter::convertLittleEndianArray( opnArray, static_cast< size_t >( nElemCount ) );
    writeMemory( opnArray, nWriteSize, sizeof( Type ) );
}

template< typename Type >
void BinaryOutputStream::writeArray( const Type* opnArray, sal_Int32 nElemCount )
{
    std::unique_ptr<Type[]> xArray(new Type[nElemCount]);
    std::uninitialized_copy(opnArray, opnArray + nElemCount, xArray.get());
    writeArray(xArray.get(), nElemCount);
}

template< typename Type >
void BinaryOutputStream::writeValue( Type nValue )
{
    ByteOrderConverter::convertLittleEndian( nValue );
    writeMemory( &nValue, static_cast< sal_Int32 >( sizeof( Type ) ), sizeof( Type ) );
}


/** Wraps a UNO output stream and provides convenient access functions.

    The binary data in the stream is written in little-endian format.
 */
class OOX_DLLPUBLIC BinaryXOutputStream final : public BinaryXSeekableStream, public BinaryOutputStream
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
                            const css::uno::Reference< css::io::XOutputStream >& rxOutStrm,
                            bool bAutoClose );

    virtual             ~BinaryXOutputStream() override;

    /** Flushes and closes the output stream. Does also close the wrapped UNO
        output stream if bAutoClose has been set to true in the constructor. */
    void                close() override;

    /** Writes the passed data sequence. */
    virtual void        writeData( const StreamDataSequence& rData, size_t nAtomSize = 1 ) override;

    /** Write nBytes bytes from the (preallocated!) buffer pMem. */
    virtual void        writeMemory( const void* pMem, sal_Int32 nBytes, size_t nAtomSize = 1 ) override;

private:
    StreamDataSequence  maBuffer;       ///< Data buffer used in writeMemory() function.
    css::uno::Reference< css::io::XOutputStream >
                        mxOutStrm;      ///< Reference to the output stream.
    bool                mbAutoClose;    ///< True = automatically close stream on destruction.
};

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
