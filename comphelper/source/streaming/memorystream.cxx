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


#include "comphelper_module.hxx"

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekableInputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase4.hxx>

#include <string.h>
#include <vector>

using ::cppu::OWeakObject;
using ::cppu::WeakImplHelper4;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::osl;

namespace comphelper
{

class UNOMemoryStream : public WeakImplHelper4 < XStream, XSeekableInputStream, XOutputStream, XTruncate >
{
public:
    UNOMemoryStream();
    virtual ~UNOMemoryStream();

    // XStream
    virtual Reference< XInputStream > SAL_CALL getInputStream(  ) throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Reference< XOutputStream > SAL_CALL getOutputStream(  ) throw (RuntimeException, std::exception) SAL_OVERRIDE;

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL available() throw (NotConnectedException, IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL closeInput() throw (NotConnectedException, IOException, RuntimeException, std::exception) SAL_OVERRIDE;

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) throw (IllegalArgumentException, IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int64 SAL_CALL getPosition() throw (IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int64 SAL_CALL getLength() throw (IOException, RuntimeException, std::exception) SAL_OVERRIDE;

    // XOutputStream
    virtual void SAL_CALL writeBytes( const Sequence< sal_Int8 >& aData ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL flush() throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL closeOutput() throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception) SAL_OVERRIDE;

    // XTruncate
    virtual void SAL_CALL truncate() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo - static versions (used for component registration)
    static OUString SAL_CALL getImplementationName_static();
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_static();
    static Reference< XInterface > SAL_CALL Create( const Reference< ::com::sun::star::uno::XComponentContext >& );

private:
    std::vector< sal_Int8 > maData;
    sal_Int32 mnCursor;
};

UNOMemoryStream::UNOMemoryStream()
: mnCursor(0)
{
}

UNOMemoryStream::~UNOMemoryStream()
{
}

// XStream
Reference< XInputStream > SAL_CALL UNOMemoryStream::getInputStream(  ) throw (RuntimeException, std::exception)
{
    return this;
}

Reference< XOutputStream > SAL_CALL UNOMemoryStream::getOutputStream(  ) throw (RuntimeException, std::exception)
{
    return this;
}

// XInputStream
sal_Int32 SAL_CALL UNOMemoryStream::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception)
{
    if( nBytesToRead < 0 )
        throw IOException();

    nBytesToRead = std::min( nBytesToRead, available() );
    aData.realloc( nBytesToRead );

    if( nBytesToRead )
    {
        sal_Int8* pData = static_cast<sal_Int8*>(&(*maData.begin()));
        sal_Int8* pCursor = &((pData)[mnCursor]);
        memcpy( (void*)aData.getArray(), (void*)pCursor, nBytesToRead );

        mnCursor += nBytesToRead;
    }

    return nBytesToRead;
}

sal_Int32 SAL_CALL UNOMemoryStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception)
{
    return readBytes( aData, nMaxBytesToRead );
}

void SAL_CALL UNOMemoryStream::skipBytes( sal_Int32 nBytesToSkip ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception)
{
    if( nBytesToSkip < 0 )
        throw IOException();

    mnCursor += std::min( nBytesToSkip, available() );
}

sal_Int32 SAL_CALL UNOMemoryStream::available() throw (NotConnectedException, IOException, RuntimeException, std::exception)
{
    return static_cast< sal_Int32 >( maData.size() ) - mnCursor;
}

void SAL_CALL UNOMemoryStream::closeInput() throw (NotConnectedException, IOException, RuntimeException, std::exception)
{
    mnCursor = 0;
}

// XSeekable
void SAL_CALL UNOMemoryStream::seek( sal_Int64 location ) throw (IllegalArgumentException, IOException, RuntimeException, std::exception)
{
    if( (location < 0) || (location > SAL_MAX_INT32) )
        throw IllegalArgumentException("this implementation does not support more than 2GB!", Reference< XInterface >(static_cast<OWeakObject*>(this)), 0 );

    // seek operation should be able to resize the stream
    if ( location > static_cast< sal_Int64 >( maData.size() ) )
        maData.resize( static_cast< sal_Int32 >( location ) );

    if ( location > static_cast< sal_Int64 >( maData.size() ) )
        maData.resize( static_cast< sal_Int32 >( location ) );

    mnCursor = static_cast< sal_Int32 >( location );
}

sal_Int64 SAL_CALL UNOMemoryStream::getPosition() throw (IOException, RuntimeException, std::exception)
{
    return static_cast< sal_Int64 >( mnCursor );
}

sal_Int64 SAL_CALL UNOMemoryStream::getLength() throw (IOException, RuntimeException, std::exception)
{
    return static_cast< sal_Int64 >( maData.size() );
}

// XOutputStream
void SAL_CALL UNOMemoryStream::writeBytes( const Sequence< sal_Int8 >& aData ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception)
{
    const sal_Int32 nBytesToWrite( aData.getLength() );
    if( nBytesToWrite )
    {
        sal_Int64 nNewSize = static_cast< sal_Int64 >( mnCursor + nBytesToWrite );
        if( nNewSize > SAL_MAX_INT32 )
        {
            OSL_ASSERT(false);
            throw IOException("this implementation does not support more than 2GB!", Reference< XInterface >(static_cast<OWeakObject*>(this)) );
        }

        if( static_cast< sal_Int32 >( nNewSize ) > static_cast< sal_Int32 >( maData.size() ) )
            maData.resize( static_cast< sal_Int32 >( nNewSize ) );

        sal_Int8* pData = static_cast<sal_Int8*>(&(*maData.begin()));
        sal_Int8* pCursor = &(pData[mnCursor]);
        memcpy( (void*)pCursor, (void*)aData.getConstArray(), nBytesToWrite );

        mnCursor += nBytesToWrite;
    }
}

void SAL_CALL UNOMemoryStream::flush() throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception)
{
}

void SAL_CALL UNOMemoryStream::closeOutput() throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception)
{
    mnCursor = 0;
}

//XTruncate
void SAL_CALL UNOMemoryStream::truncate() throw (IOException, RuntimeException, std::exception)
{
    maData.resize( 0 );
    mnCursor = 0;
}

OUString SAL_CALL UNOMemoryStream::getImplementationName_static()
{
    return OUString("com.sun.star.comp.MemoryStream");
}

Sequence< OUString > SAL_CALL UNOMemoryStream::getSupportedServiceNames_static()
{
    Sequence< OUString > aSeq(1);
    aSeq[0] = getImplementationName_static();
    return aSeq;
}

Reference< XInterface > SAL_CALL UNOMemoryStream::Create(
    SAL_UNUSED_PARAMETER const Reference< XComponentContext >& )
{
    return static_cast<OWeakObject*>(new UNOMemoryStream());
}

} // namespace comphelper

void createRegistryInfo_UNOMemoryStream()
{
    static ::comphelper::module::OAutoRegistration< ::comphelper::UNOMemoryStream > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
