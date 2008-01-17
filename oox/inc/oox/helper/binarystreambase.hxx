/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binarystreambase.hxx,v $
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

#ifndef OOX_HELPER_BINARYSTREAMBASE_HXX
#define OOX_HELPER_BINARYSTREAMBASE_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/io/XSeekable.hpp>
#include "oox/helper/helper.hxx"

namespace oox {

// ============================================================================

/** Base class for binary input and output streams. */
class BinaryStreamBase
{
public:
    template< typename Interface >
    explicit            BinaryStreamBase(
                            const ::com::sun::star::uno::Reference< Interface >& rxStrm );

    virtual             ~BinaryStreamBase();

    /** Returns true, if the wrapped stream is seekable. */
    inline bool         isSeekable() const { return mxSeekable.is(); }
    /** Returns the XSeekable interface of the wrapped stream. */
    inline ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable >
                        getXSeekable() const { return mxSeekable; }

    /** Returns the size of the stream, if stream is seekable, otherwise -1. */
    sal_Int64           getLength() const;
    /** Returns the current stream position, if stream is seekable, otherwise -1. */
    sal_Int64           tell() const;

    /** Seeks the stream to the passed position, if stream is seekable. */
    void                seek( sal_Int64 nPos );
    /** Seeks the stream to the beginning, if stream is seekable. */
    inline void         seekToStart() { seek( 0 ); }
    /** Seeks the stream to the end, if stream is seekable. */
    inline void         seekToEnd() { seek( getLength() ); }

protected:
    ::com::sun::star::uno::Sequence< sal_Int8 >
                        maBuffer;       /// Data buffer.

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable >
                        mxSeekable;     /// Stream seeking interface.
};

// ----------------------------------------------------------------------------

template< typename Interface >
BinaryStreamBase::BinaryStreamBase( const ::com::sun::star::uno::Reference< Interface >& rxStrm ) :
    maBuffer( 0x8000 ),
    mxSeekable( rxStrm, ::com::sun::star::uno::UNO_QUERY )
{
}

// ============================================================================

} // namespace oox

#endif

