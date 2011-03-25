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

#include "oox/helper/binarystreambase.hxx"

#include <com/sun/star/io/XSeekable.hpp>
#include <osl/diagnose.h>

namespace oox {

// ============================================================================

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

// ============================================================================

BinaryStreamBase::~BinaryStreamBase()
{
}

sal_Int64 BinaryStreamBase::getRemaining() const
{
    // do not use isSeekable(), implementations may provide stream position and size even if not seekable
    sal_Int64 nPos = tell();
    sal_Int64 nLen = size();
    return ((nPos >= 0) && (nLen >= 0)) ? ::std::max< sal_Int64 >( nLen - nPos, 0 ) : -1;
}

void BinaryStreamBase::alignToBlock( sal_Int32 nBlockSize, sal_Int64 nAnchorPos )
{
    sal_Int64 nStrmPos = tell();
    // nothing to do, if stream is at anchor position
    if( mbSeekable && (0 <= nAnchorPos) && (nAnchorPos != nStrmPos) && (nBlockSize > 1) )
    {
        // prevent modulo with negative arguments...
        sal_Int64 nSkipSize = (nAnchorPos < nStrmPos) ?
            (nBlockSize - ((nStrmPos - nAnchorPos - 1) % nBlockSize) - 1) :
            ((nAnchorPos - nStrmPos) % nBlockSize);
        seek( nStrmPos + nSkipSize );
    }
}

// ============================================================================

BinaryXSeekableStream::BinaryXSeekableStream( const Reference< XSeekable >& rxSeekable ) :
    BinaryStreamBase( mxSeekable.is() ),
    mxSeekable( rxSeekable )
{
}

BinaryXSeekableStream::~BinaryXSeekableStream()
{
}

sal_Int64 BinaryXSeekableStream::size() const
{
    if( mxSeekable.is() ) try
    {
        return mxSeekable->getLength();
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryXSeekableStream::size - exception caught" );
    }
    return -1;
}

sal_Int64 BinaryXSeekableStream::tell() const
{
    if( mxSeekable.is() ) try
    {
        return mxSeekable->getPosition();
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryXSeekableStream::tell - exception caught" );
    }
    return -1;
}

void BinaryXSeekableStream::seek( sal_Int64 nPos )
{
    if( mxSeekable.is() ) try
    {
        mbEof = false;
        mxSeekable->seek( nPos );
    }
    catch( Exception& )
    {
        mbEof = true;
    }
}

void BinaryXSeekableStream::close()
{
    mxSeekable.clear();
    mbEof = true;
}

// ============================================================================

SequenceSeekableStream::SequenceSeekableStream( const StreamDataSequence& rData ) :
    BinaryStreamBase( true ),
    mpData( &rData ),
    mnPos( 0 )
{
}

sal_Int64 SequenceSeekableStream::size() const
{
    return mpData ? mpData->getLength() : -1;
}

sal_Int64 SequenceSeekableStream::tell() const
{
    return mpData ? mnPos : -1;
}

void SequenceSeekableStream::seek( sal_Int64 nPos )
{
    if( mpData )
    {
        mnPos = getLimitedValue< sal_Int32, sal_Int64 >( nPos, 0, mpData->getLength() );
        mbEof = mnPos != nPos;
    }
}

void SequenceSeekableStream::close()
{
    mpData = 0;
    mbEof = true;
}

// ============================================================================

} // namespace oox
