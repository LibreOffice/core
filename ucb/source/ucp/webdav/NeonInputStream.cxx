/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <string.h>

#include "NeonInputStream.hxx"

using namespace cppu;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace webdav_ucp;

// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------
NeonInputStream::NeonInputStream( void )
: mLen( 0 ),
  mPos( 0 )
{
}

// -------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------
NeonInputStream::~NeonInputStream( void )
{
}

// -------------------------------------------------------------------
// AddToStream
// Allows the caller to add some data to the "end" of the stream
// -------------------------------------------------------------------
void NeonInputStream::AddToStream( const char * inBuf, sal_Int32 inLen )
{
    mInputBuffer.realloc( sal::static_int_cast<sal_Int32>(mLen) + inLen );
    memcpy( mInputBuffer.getArray() + mLen, inBuf, inLen );
    mLen += inLen;
}

// -------------------------------------------------------------------
// queryInterface
// -------------------------------------------------------------------
Any NeonInputStream::queryInterface( const Type &type )
                        throw( RuntimeException )
{
    Any aRet = ::cppu::queryInterface( type,
                                       static_cast< XInputStream * >( this ),
                                       static_cast< XSeekable * >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( type );
}

// -------------------------------------------------------------------
// readBytes
// "Reads" the specified number of bytes from the stream
// -------------------------------------------------------------------
sal_Int32 SAL_CALL NeonInputStream::readBytes(
  ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    // Work out how much we're actually going to write
    sal_Int32 theBytes2Read = nBytesToRead;
    sal_Int32 theBytesLeft  = sal::static_int_cast<sal_Int32>(mLen - mPos);
    if ( theBytes2Read > theBytesLeft )
        theBytes2Read = theBytesLeft;

    // Realloc buffer.
    aData.realloc( theBytes2Read );

    // Write the data
    memcpy(
        aData.getArray(), mInputBuffer.getConstArray() + mPos, theBytes2Read );

    // Update our stream position for next time
    mPos += theBytes2Read;

    return theBytes2Read;
}

// -------------------------------------------------------------------
// readSomeBytes
// -------------------------------------------------------------------
sal_Int32 SAL_CALL NeonInputStream::readSomeBytes(
 ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    // Warning: What should this be doing ?
    return readBytes( aData, nMaxBytesToRead );
}

// -------------------------------------------------------------------
// skipBytes
// Moves the current stream position forward
// -------------------------------------------------------------------
void SAL_CALL NeonInputStream::skipBytes( sal_Int32 nBytesToSkip )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    mPos += nBytesToSkip;
    if ( mPos >= mLen )
        mPos = mLen;
}

// -------------------------------------------------------------------
// available
// Returns the number of unread bytes currently remaining on the stream
// -------------------------------------------------------------------
sal_Int32 SAL_CALL NeonInputStream::available(  )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    return sal::static_int_cast<sal_Int32>(mLen - mPos);
}

// -------------------------------------------------------------------
// closeInput
// -------------------------------------------------------------------
void SAL_CALL NeonInputStream::closeInput( void )
         throw( ::com::sun::star::io::NotConnectedException,
                  ::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException )
{
}

// -------------------------------------------------------------------
// seek
// -------------------------------------------------------------------
void SAL_CALL NeonInputStream::seek( sal_Int64 location )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    if ( location < 0 )
        throw ::com::sun::star::lang::IllegalArgumentException();

    if ( location <= mLen )
        mPos = location;
    else
        throw ::com::sun::star::lang::IllegalArgumentException();
}

// -------------------------------------------------------------------
// getPosition
// -------------------------------------------------------------------
sal_Int64 SAL_CALL NeonInputStream::getPosition()
        throw( ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    return mPos;
}

// -------------------------------------------------------------------
// getLength
// -------------------------------------------------------------------
sal_Int64 SAL_CALL NeonInputStream::getLength()
        throw( ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    return mLen;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
