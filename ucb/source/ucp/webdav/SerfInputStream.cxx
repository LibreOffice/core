/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "SerfInputStream.hxx"
#include <string.h>

using namespace cppu;
using namespace rtl;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace http_dav_ucp;





SerfInputStream::SerfInputStream( void )
: mLen( 0 ),
  mPos( 0 )
{
}




SerfInputStream::~SerfInputStream( void )
{
}





void SerfInputStream::AddToStream( const char * inBuf, sal_Int32 inLen )
{
    mInputBuffer.realloc( sal::static_int_cast<sal_Int32>(mLen) + inLen );
    memcpy( mInputBuffer.getArray() + mLen, inBuf, inLen );
    mLen += inLen;
}




Any SerfInputStream::queryInterface( const Type &type )
                        throw( RuntimeException )
{
    Any aRet = ::cppu::queryInterface( type,
                                       static_cast< XInputStream * >( this ),
                                       static_cast< XSeekable * >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( type );
}





sal_Int32 SAL_CALL SerfInputStream::readBytes(
  ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    
    sal_Int32 theBytes2Read = nBytesToRead;
    sal_Int32 theBytesLeft  = sal::static_int_cast<sal_Int32>(mLen - mPos);
    if ( theBytes2Read > theBytesLeft )
        theBytes2Read = theBytesLeft;

    
    aData.realloc( theBytes2Read );

    
    memcpy(
        aData.getArray(), mInputBuffer.getConstArray() + mPos, theBytes2Read );

    
    mPos += theBytes2Read;

    return theBytes2Read;
}




sal_Int32 SAL_CALL SerfInputStream::readSomeBytes(
 ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    
    return readBytes( aData, nMaxBytesToRead );
}





void SAL_CALL SerfInputStream::skipBytes( sal_Int32 nBytesToSkip )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    mPos += nBytesToSkip;
    if ( mPos >= mLen )
        mPos = mLen;
}





sal_Int32 SAL_CALL SerfInputStream::available(  )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    return sal::static_int_cast<sal_Int32>(mLen - mPos);
}




void SAL_CALL SerfInputStream::closeInput( void )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
}




void SAL_CALL SerfInputStream::seek( sal_Int64 location )
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




sal_Int64 SAL_CALL SerfInputStream::getPosition()
        throw( ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    return mPos;
}




sal_Int64 SAL_CALL SerfInputStream::getLength()
        throw( ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException )
{
    return mLen;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
