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

#include <algorithm>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekableInputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>

#include <string.h>
#include <vector>

using ::cppu::OWeakObject;
using ::cppu::WeakImplHelper;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::osl;

namespace comphelper
{

class UNOMemoryStream : public WeakImplHelper<XServiceInfo, XStream, XSeekableInputStream, XOutputStream, XTruncate>
{
public:
    UNOMemoryStream();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XStream
    virtual Reference< XInputStream > SAL_CALL getInputStream(  ) override;
    virtual Reference< XOutputStream > SAL_CALL getOutputStream(  ) override;

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 SAL_CALL available() override;
    virtual void SAL_CALL closeInput() override;

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) override;
    virtual sal_Int64 SAL_CALL getPosition() override;
    virtual sal_Int64 SAL_CALL getLength() override;

    // XOutputStream
    virtual void SAL_CALL writeBytes( const Sequence< sal_Int8 >& aData ) override;
    virtual void SAL_CALL flush() override;
    virtual void SAL_CALL closeOutput() override;

    // XTruncate
    virtual void SAL_CALL truncate() override;

private:
    std::vector< sal_Int8 > maData;
    sal_Int32 mnCursor;
};

UNOMemoryStream::UNOMemoryStream()
: mnCursor(0)
{
}

// XServiceInfo
OUString SAL_CALL UNOMemoryStream::getImplementationName()
{
    return OUString("com.sun.star.comp.MemoryStream");
}

sal_Bool SAL_CALL UNOMemoryStream::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL UNOMemoryStream::getSupportedServiceNames()
{
    return { "com.sun.star.comp.MemoryStream" };
}

// XStream
Reference< XInputStream > SAL_CALL UNOMemoryStream::getInputStream(  )
{
    return this;
}

Reference< XOutputStream > SAL_CALL UNOMemoryStream::getOutputStream(  )
{
    return this;
}

// XInputStream
sal_Int32 SAL_CALL UNOMemoryStream::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    if( nBytesToRead < 0 )
        throw IOException();

    nBytesToRead = std::min( nBytesToRead, available() );
    aData.realloc( nBytesToRead );

    if( nBytesToRead )
    {
        sal_Int8* pData = &(*maData.begin());
        sal_Int8* pCursor = &((pData)[mnCursor]);
        memcpy( static_cast<void*>(aData.getArray()), static_cast<void*>(pCursor), nBytesToRead );

        mnCursor += nBytesToRead;
    }

    return nBytesToRead;
}

sal_Int32 SAL_CALL UNOMemoryStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    return readBytes( aData, nMaxBytesToRead );
}

void SAL_CALL UNOMemoryStream::skipBytes( sal_Int32 nBytesToSkip )
{
    if( nBytesToSkip < 0 )
        throw IOException();

    mnCursor += std::min( nBytesToSkip, available() );
}

sal_Int32 SAL_CALL UNOMemoryStream::available()
{
    return static_cast< sal_Int32 >( maData.size() ) - mnCursor;
}

void SAL_CALL UNOMemoryStream::closeInput()
{
    mnCursor = 0;
}

// XSeekable
void SAL_CALL UNOMemoryStream::seek( sal_Int64 location )
{
    if( (location < 0) || (location > SAL_MAX_INT32) )
        throw IllegalArgumentException("this implementation does not support more than 2GB!", static_cast<OWeakObject*>(this), 0 );

    // seek operation should be able to resize the stream
    if ( location > static_cast< sal_Int64 >( maData.size() ) )
        maData.resize( static_cast< sal_Int32 >( location ) );

    if ( location > static_cast< sal_Int64 >( maData.size() ) )
        maData.resize( static_cast< sal_Int32 >( location ) );

    mnCursor = static_cast< sal_Int32 >( location );
}

sal_Int64 SAL_CALL UNOMemoryStream::getPosition()
{
    return static_cast< sal_Int64 >( mnCursor );
}

sal_Int64 SAL_CALL UNOMemoryStream::getLength()
{
    return static_cast< sal_Int64 >( maData.size() );
}

// XOutputStream
void SAL_CALL UNOMemoryStream::writeBytes( const Sequence< sal_Int8 >& aData )
{
    const sal_Int32 nBytesToWrite( aData.getLength() );
    if( nBytesToWrite )
    {
        sal_Int64 nNewSize = static_cast< sal_Int64 >( mnCursor + nBytesToWrite );
        if( nNewSize > SAL_MAX_INT32 )
        {
            OSL_ASSERT(false);
            throw IOException("this implementation does not support more than 2GB!", static_cast<OWeakObject*>(this) );
        }

        if( static_cast< sal_Int32 >( nNewSize ) > static_cast< sal_Int32 >( maData.size() ) )
            maData.resize( static_cast< sal_Int32 >( nNewSize ) );

        sal_Int8* pData = &(*maData.begin());
        sal_Int8* pCursor = &(pData[mnCursor]);
        memcpy( pCursor, aData.getConstArray(), nBytesToWrite );

        mnCursor += nBytesToWrite;
    }
}

void SAL_CALL UNOMemoryStream::flush()
{
}

void SAL_CALL UNOMemoryStream::closeOutput()
{
    mnCursor = 0;
}

//XTruncate
void SAL_CALL UNOMemoryStream::truncate()
{
    maData.resize( 0 );
    mnCursor = 0;
}

} // namespace comphelper

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_MemoryStream(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::comphelper::UNOMemoryStream());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
