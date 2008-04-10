/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binarystreambase.hxx,v $
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

