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

#ifndef OOX_HELPER_BINARYSTREAMBASE_HXX
#define OOX_HELPER_BINARYSTREAMBASE_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/io/XSeekable.hpp>
#include "oox/helper/helper.hxx"

namespace oox {

typedef ::com::sun::star::uno::Sequence< sal_Int8 > StreamDataSequence;

// ============================================================================

/** Base interface for seekable binary stream classes. */
class BinaryStreamBase
{
public:
    virtual             ~BinaryStreamBase();

    /** Derived classes return whether the stream is seekable. Default: false. */
    virtual bool        isSeekable() const;
    /** Derived classes returns the size of the stream, if seekable, otherwise/default: -1. */
    virtual sal_Int64   getLength() const;
    /** Derived classes return the current stream position, if seekable, otherwise/default: -1. */
    virtual sal_Int64   tell() const;
    /** Derived classes implement seeking the stream to the passed position, if seekable. */
    virtual void        seek( sal_Int64 nPos );

    /** Returns true, if the stream position is invalid (EOF). This flag turns
        true *after* the first attempt to seek/read beyond the stream end. */
    inline bool         isEof() const { return mbEof; }

    /** Returns the size of the remaining data, if stream is seekable, otherwise -1. */
    sal_Int64           getRemaining() const;
    /** Seeks the stream to the beginning, if stream is seekable. */
    inline void         seekToStart() { seek( 0 ); }
    /** Seeks the stream to the end, if stream is seekable. */
    inline void         seekToEnd() { seek( getLength() ); }

protected:
    inline explicit     BinaryStreamBase() : mbEof( false ) {}

private:
                        BinaryStreamBase( const BinaryStreamBase& );
    BinaryStreamBase&   operator=( const BinaryStreamBase& );

protected:
    bool                mbEof;
};

// ============================================================================

/** Base class for binary input and output streams wrapping an API stream,
    seekable via the com.sun.star.io.XSeekable interface.
 */
class BinaryXSeekableStream : public virtual BinaryStreamBase
{
public:
    /** Returns true, if the wrapped stream is seekable. */
    virtual bool        isSeekable() const;
    /** Returns the size of the stream, if stream is seekable, otherwise -1. */
    virtual sal_Int64   getLength() const;
    /** Returns the current stream position, if stream is seekable, otherwise -1. */
    virtual sal_Int64   tell() const;
    /** Seeks the stream to the passed position, if stream is seekable. */
    virtual void        seek( sal_Int64 nPos );

protected:
    explicit            BinaryXSeekableStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable >& rxSeekable );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable >
                        mxSeekable;     /// Stream seeking interface.
};

// ============================================================================

/** Base class for binary input and output streams wrapping a
    StreamDataSequence, which is always seekable. */
class SequenceSeekableStream : public virtual BinaryStreamBase
{
public:
    /** Returns true (data sequence streams are always seekable). */
    virtual bool        isSeekable() const;
    /** Returns the size of the wrapped data sequence. */
    virtual sal_Int64   getLength() const;
    /** Returns the current stream position. */
    virtual sal_Int64   tell() const;
    /** Seeks the stream to the passed position. */
    virtual void        seek( sal_Int64 nPos );

protected:
    /** Constructs the wrapper object for the passed data sequence.

        @attention
            The passed data sequence MUST live at least as long as this stream
            wrapper. The data sequence MUST NOT be changed from outside as long
            as this stream wrapper is used to modify it.
     */
    inline explicit     SequenceSeekableStream( const StreamDataSequence& rData ) : mrData( rData ), mnPos( 0 ) {}

protected:
    const StreamDataSequence& mrData;   /// Wrapped data sequence.
    sal_Int32           mnPos;          /// Current position in the sequence.
};

// ============================================================================

} // namespace oox

#endif

