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

#include "NeonInputStream.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/queryinterface.hxx>

#include <string.h>

using namespace cppu;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace webdav_ucp;

NeonInputStream::NeonInputStream()
: mLen( 0 ),
  mPos( 0 )
{
}

NeonInputStream::~NeonInputStream()
{
}

// Allows the caller to add some data to the "end" of the stream
void NeonInputStream::AddToStream( const char * inBuf, sal_Int32 inLen )
{
    mInputBuffer.realloc( sal::static_int_cast<sal_Int32>(mLen) + inLen );
    memcpy( mInputBuffer.getArray() + mLen, inBuf, inLen );
    mLen += inLen;
}

Any NeonInputStream::queryInterface( const Type &type )
{
    Any aRet = ::cppu::queryInterface( type,
                                       static_cast< XInputStream * >( this ),
                                       static_cast< XSeekable * >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( type );
}

// "Reads" the specified number of bytes from the stream
sal_Int32 SAL_CALL NeonInputStream::readBytes(
  css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
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

sal_Int32 SAL_CALL NeonInputStream::readSomeBytes(
 css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    // Warning: What should this be doing ?
    return readBytes( aData, nMaxBytesToRead );
}

// Moves the current stream position forward
void SAL_CALL NeonInputStream::skipBytes( sal_Int32 nBytesToSkip )
{
    mPos += nBytesToSkip;
    if ( mPos >= mLen )
        mPos = mLen;
}

// Returns the number of unread bytes currently remaining on the stream
sal_Int32 SAL_CALL NeonInputStream::available(  )
{
    return sal::static_int_cast<sal_Int32>(mLen - mPos);
}

void SAL_CALL NeonInputStream::closeInput()
{
}

void SAL_CALL NeonInputStream::seek( sal_Int64 location )
{
    if ( location < 0 )
        throw css::lang::IllegalArgumentException();

    if ( location <= mLen )
        mPos = location;
    else
        throw css::lang::IllegalArgumentException();
}

sal_Int64 SAL_CALL NeonInputStream::getPosition()
{
    return mPos;
}

sal_Int64 SAL_CALL NeonInputStream::getLength()
{
    return mLen;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
