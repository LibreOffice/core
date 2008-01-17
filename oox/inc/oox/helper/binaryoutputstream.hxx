/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binaryoutputstream.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:46 $
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

#ifndef OOX_HELPER_BINARYOUTPUTSTREAM_HXX
#define OOX_HELPER_BINARYOUTPUTSTREAM_HXX

#include <boost/shared_ptr.hpp>
#include "oox/helper/binarystreambase.hxx"

namespace com { namespace sun { namespace star {
    namespace io { class XOutputStream; }
} } }

namespace oox {

class BinaryInputStream;

// ============================================================================

/** Wraps a binary output stream and provides convenient access functions.

    The binary data in the stream is written in little-endian format.
 */
class BinaryOutputStream : public BinaryStreamBase
{
public:
    /** Constructs the wrapper object for the passed output stream.

        @param rxOutStream  The com.sun.star.io.XOutputStream interface of the
            output stream to be wrapped.
        @param bAutoClose  True = automatically close the wrapped output stream
            on destruction of this wrapper.
     */
    explicit            BinaryOutputStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rxOutStrm,
                            bool bAutoClose );

    virtual             ~BinaryOutputStream();

    /** Returns true, if the wrapped stream is valid. */
    inline bool         is() const { return mxOutStrm.is(); }
    /** Returns the XOutputStream interface of the wrapped output stream. */
    inline ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        getXOutputStream() const { return mxOutStrm; }

    /** Writes the passed sequence. */
    void                write( const ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer );
    /** Writes nBytes bytes from the (existing) buffer pBuffer. */
    void                write( const void* pBuffer, sal_Int32 nBytes );

    /** Writes a value to the stream and converts it to platform byte order. */
    template< typename Type >
    void                writeValue( Type nValue );

    /** Copies nBytes bytes from the current position of the passed input stream. */
    void                copy( BinaryInputStream& rInStrm, sal_Int64 nBytes = SAL_MAX_INT32 );

    /** Flushes and closes the wrapped XOutputStream. */
    void                close();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        mxOutStrm;      /// Reference to the output stream.
    bool                mbAutoClose;    /// True = automatically close stream on destruction.
};

typedef ::boost::shared_ptr< BinaryOutputStream > BinaryOutputStreamRef;

// ----------------------------------------------------------------------------

template< typename Type >
void BinaryOutputStream::writeValue( Type nValue )
{
    // can be instanciated for all types supported in class ByteOrderConverter
    ByteOrderConverter::convertLittleEndian( nValue );
    write( &nValue, static_cast< sal_Int32 >( sizeof( Type ) ) );
}

template< typename Type >
inline BinaryOutputStream& operator<<( BinaryOutputStream& rOutStrm, Type nValue )
{
    rOutStrm.writeValue( nValue );
    return rOutStrm;
}

// ============================================================================

} // namespace oox

#endif

