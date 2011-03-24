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
#include <boost/shared_ptr.hpp>
#include "oox/helper/helper.hxx"

namespace com { namespace sun { namespace star {
    namespace io { class XSeekable; }
} } }

namespace oox {

typedef ::com::sun::star::uno::Sequence< sal_Int8 > StreamDataSequence;

// ============================================================================

/** Base class for binary stream classes.
 */
class BinaryStreamBase
{
public:
    virtual             ~BinaryStreamBase();

    /** Implementations return the size of the stream, if possible.

        This function may be implemented for some types of unseekable streams,
        and MUST be implemented for all seekable streams.

        @return
            The size of the stream in bytes, or -1, if not implemented.
     */
    virtual sal_Int64   size() const = 0;

    /** Implementations return the current stream position, if possible.

        This function may be implemented for some types of unseekable streams,
        and MUST be implemented for all seekable streams.

        @return
            The current position in the stream, or -1, if not implemented.
     */
    virtual sal_Int64   tell() const = 0;

    /** Implementations seek the stream to the passed position, if
        the stream is seekable.
     */
    virtual void        seek( sal_Int64 nPos ) = 0;

    /** Implementations close the stream.
     */
    virtual void        close() = 0;

    /** Returns true, if the implementation supports the seek() operation.

        Implementations may still implement size() and tell() even if the
        stream is not seekable.
     */
    inline bool         isSeekable() const { return mbSeekable; }

    /** Returns true, if the stream position is invalid (EOF). This flag turns
        true *after* the first attempt to seek/read beyond the stream end.
     */
    inline bool         isEof() const { return mbEof; }

    /** Returns the size of the remaining data available in the stream, if
        stream supports size() and tell(), otherwise -1.
     */
    sal_Int64           getRemaining() const;

    /** Seeks the stream to the beginning, if stream is seekable.
     */
    inline void         seekToStart() { seek( 0 ); }

    /** Seeks the stream to the end, if stream is seekable.
     */
    inline void         seekToEnd() { seek( size() ); }

    /** Seeks the stream forward to a position that is a multiple of the passed
        block size, if stream is seekable.

        @param nBlockSize
            The size of the data blocks the streams needs to be aligned to.

        @param nAnchorPos
            Position in the stream the data blocks are aligned to.
     */
    void                alignToBlock( sal_Int32 nBlockSize, sal_Int64 nAnchorPos = 0 );

protected:
    inline explicit     BinaryStreamBase( bool bSeekable ) : mbEof( false ), mbSeekable( bSeekable ) {}

private:
                        BinaryStreamBase( const BinaryStreamBase& );
    BinaryStreamBase&   operator=( const BinaryStreamBase& );

protected:
    bool                mbEof;          /// End of stream flag.

private:
    const bool          mbSeekable;     /// True = implementation supports seeking.
};

// ============================================================================

/** Base class for binary input and output streams wrapping a UNO stream,
    seekable via the com.sun.star.io.XSeekable interface.
 */
class BinaryXSeekableStream : public virtual BinaryStreamBase
{
public:
    virtual             ~BinaryXSeekableStream();

    /** Returns the size of the stream, if wrapped stream is seekable, otherwise -1. */
    virtual sal_Int64   size() const;
    /** Returns the current stream position, if wrapped stream is seekable, otherwise -1. */
    virtual sal_Int64   tell() const;
    /** Seeks the stream to the passed position, if wrapped stream is seekable. */
    virtual void        seek( sal_Int64 nPos );
    /** Releases the reference to the UNO XSeekable interface. */
    virtual void        close();

protected:
    explicit            BinaryXSeekableStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable >& rxSeekable );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable >
                        mxSeekable;     /// Stream seeking interface.
};

// ============================================================================

/** Base class for binary input and output streams wrapping a
    StreamDataSequence, which is always seekable.

    The wrapped data sequence MUST live at least as long as this stream
    wrapper. The data sequence MUST NOT be changed from outside as long as this
    stream wrapper is used to modify it.
 */
class SequenceSeekableStream : public virtual BinaryStreamBase
{
public:
    /** Returns the size of the wrapped data sequence. */
    virtual sal_Int64   size() const;
    /** Returns the current stream position. */
    virtual sal_Int64   tell() const;
    /** Seeks the stream to the passed position. */
    virtual void        seek( sal_Int64 nPos );
    /** Releases the reference to the data sequence. */
    virtual void        close();

protected:
    explicit            SequenceSeekableStream( const StreamDataSequence& rData );

protected:
    const StreamDataSequence* mpData;   /// Wrapped data sequence.
    sal_Int32           mnPos;          /// Current position in the sequence.
};

// ============================================================================

} // namespace oox

#endif
