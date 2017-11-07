/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "SerfInputStream.hxx"

#include <cppuhelper/queryinterface.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <string.h>

using namespace cppu;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace http_dav_ucp;

// Constructor

SerfInputStream::SerfInputStream()
: mLen( 0 ),
  mPos( 0 )
{
}


// Destructor

SerfInputStream::~SerfInputStream()
{
}


// AddToStream
// Allows the caller to add some data to the "end" of the stream

void SerfInputStream::AddToStream( const char * inBuf, sal_Int32 inLen )
{
    mInputBuffer.realloc( sal::static_int_cast<sal_Int32>(mLen) + inLen );
    memcpy( mInputBuffer.getArray() + mLen, inBuf, inLen );
    mLen += inLen;
}


// queryInterface

Any SerfInputStream::queryInterface( const Type &type )
{
    Any aRet = ::cppu::queryInterface( type,
                                       static_cast< XInputStream * >( this ),
                                       static_cast< XSeekable * >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( type );
}


// readBytes
// "Reads" the specified number of bytes from the stream

sal_Int32 SAL_CALL SerfInputStream::readBytes(
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


// readSomeBytes

sal_Int32 SAL_CALL SerfInputStream::readSomeBytes(
 css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    // Warning: What should this be doing ?
    return readBytes( aData, nMaxBytesToRead );
}


// skipBytes
// Moves the current stream position forward

void SAL_CALL SerfInputStream::skipBytes( sal_Int32 nBytesToSkip )
{
    mPos += nBytesToSkip;
    if ( mPos >= mLen )
        mPos = mLen;
}


// available
// Returns the number of unread bytes currently remaining on the stream

sal_Int32 SAL_CALL SerfInputStream::available(  )
{
    return sal::static_int_cast<sal_Int32>(mLen - mPos);
}


// closeInput

void SAL_CALL SerfInputStream::closeInput()
{
}


// seek

void SAL_CALL SerfInputStream::seek( sal_Int64 location )
{
    if ( location < 0 || location > mLen )
        throw css::lang::IllegalArgumentException();
    mPos = location;
}


// getPosition

sal_Int64 SAL_CALL SerfInputStream::getPosition()
{
    return mPos;
}


// getLength

sal_Int64 SAL_CALL SerfInputStream::getLength()
{
    return mLen;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
