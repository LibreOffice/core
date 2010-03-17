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
#include <osl/diagnose.h>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::io::XSeekable;

namespace oox {

// ============================================================================

BinaryStreamBase::~BinaryStreamBase()
{
}

bool BinaryStreamBase::isSeekable() const
{
    return false;
}

sal_Int64 BinaryStreamBase::getLength() const
{
    return -1;
}

sal_Int64 BinaryStreamBase::tell() const
{
    return -1;
}

void BinaryStreamBase::seek( sal_Int64 )
{
}

sal_Int64 BinaryStreamBase::getRemaining() const
{
    return isSeekable() ? ::std::max< sal_Int64 >( getLength() - tell(), 0 ) : -1;
}

// ============================================================================

BinaryXSeekableStream::BinaryXSeekableStream( const Reference< XSeekable >& rxSeekable ) :
    mxSeekable( rxSeekable )
{
}

bool BinaryXSeekableStream::isSeekable() const
{
    return mxSeekable.is();
}

sal_Int64 BinaryXSeekableStream::getLength() const
{
    if( mxSeekable.is() ) try
    {
        return mxSeekable->getLength();
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryXSeekableStream::getLength - exception caught" );
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

// ============================================================================

bool SequenceSeekableStream::isSeekable() const
{
    return true;
}

sal_Int64 SequenceSeekableStream::getLength() const
{
    return mrData.getLength();
}

sal_Int64 SequenceSeekableStream::tell() const
{
    return mnPos;
}

void SequenceSeekableStream::seek( sal_Int64 nPos )
{
    mnPos = getLimitedValue< sal_Int32, sal_Int64 >( nPos, 0, mrData.getLength() );
    mbEof = mnPos < nPos;
}

// ============================================================================

} // namespace oox

