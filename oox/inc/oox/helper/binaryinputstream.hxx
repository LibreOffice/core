/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binaryinputstream.hxx,v $
 * $Revision: 1.3 $
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
#include "oox/helper/binarystreambase.hxx"

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
} } }

namespace oox {

// ============================================================================

/** Wraps a binary input stream and provides convenient access functions.

    The binary data in the stream is assumed to be in little-endian format.
 */
class BinaryInputStream : public BinaryStreamBase
{
public:
    /** Constructs the wrapper object for the passed input stream.

        @param rxInStream  The com.sun.star.io.XInputStream interface of the
            input stream to be wrapped.
        @param bAutoClose  True = automatically close the wrapped input stream
            on destruction of this wrapper.
     */
    explicit            BinaryInputStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm,
                            bool bAutoClose );

    virtual             ~BinaryInputStream();

    /** Returns true, if the wrapped stream is valid. */
    inline bool         is() const { return mxInStrm.is(); }
    /** Returns the XInputStream interface of the wrapped input stream. */
    inline ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        getXInputStream() const { return mxInStrm; }

    /** Seeks the stream forward by the passed number of bytes. This works for
        non-seekable streams too. */
    void                skip( sal_Int32 nBytes );

    /** Reads nBytes bytes to the passed sequence.
        @return  Number of bytes really read. */
    sal_Int32           read( ::com::sun::star::uno::Sequence< sal_Int8 >& orBuffer, sal_Int32 nBytes );
    /** Reads nBytes bytes to the (existing) buffer pBuffer.
        @return  Number of bytes really read. */
    sal_Int32           read( void* opBuffer, sal_Int32 nBytes );

    /** Reads a value from the stream and converts it to platform byte order. */
    template< typename Type >
    void                readValue( Type& ornValue );
    /** Reads a value from the stream and converts it to platform byte order. */
    template< typename Type >
    inline Type         readValue() { Type nValue; readValue( nValue ); return nValue; }

    /** Closes the wrapped XInputStream. */
    void                close();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        mxInStrm;       /// Reference to the input stream.
    bool                mbAutoClose;    /// True = automatically close stream on destruction.
};

typedef ::boost::shared_ptr< BinaryInputStream > BinaryInputStreamRef;

// ----------------------------------------------------------------------------

template< typename Type >
void BinaryInputStream::readValue( Type& ornValue )
{
    // can be instanciated for all types supported in class ByteOrderConverter
    read( &ornValue, static_cast< sal_Int32 >( sizeof( Type ) ) );
    ByteOrderConverter::convertLittleEndian( ornValue );
}

template< typename Type >
inline BinaryInputStream& operator>>( BinaryInputStream& rInStrm, Type& ornValue )
{
    rInStrm.readValue( ornValue );
    return rInStrm;
}

// ============================================================================

} // namespace oox

#endif

