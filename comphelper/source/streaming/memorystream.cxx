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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include "comphelper_module.hxx"

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekableInputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase4.hxx>

#include <string.h>
#include <vector>

using ::rtl::OUString;
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
    virtual Reference< XInputStream > SAL_CALL getInputStream(  ) throw (RuntimeException);
    virtual Reference< XOutputStream > SAL_CALL getOutputStream(  ) throw (RuntimeException);

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL available() throw (NotConnectedException, IOException, RuntimeException);
    virtual void SAL_CALL closeInput() throw (NotConnectedException, IOException, RuntimeException);

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) throw (IllegalArgumentException, IOException, RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition() throw (IOException, RuntimeException);
    virtual sal_Int64 SAL_CALL getLength() throw (IOException, RuntimeException);

    // XOutputStream
    virtual void SAL_CALL writeBytes( const Sequence< sal_Int8 >& aData ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual void SAL_CALL flush() throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);
    virtual void SAL_CALL closeOutput() throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException);

    // XTruncate
    virtual void SAL_CALL truncate() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo - static versions (used for component registration)
    static ::rtl::OUString SAL_CALL getImplementationName_static();
    static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static();
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
Reference< XInputStream > SAL_CALL UNOMemoryStream::getInputStream(  ) throw (RuntimeException)
{
    return this;
}

Reference< XOutputStream > SAL_CALL UNOMemoryStream::getOutputStream(  ) throw (RuntimeException)
{
    return this;
}

// XInputStream
sal_Int32 SAL_CALL UNOMemoryStream::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
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

sal_Int32 SAL_CALL UNOMemoryStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    return readBytes( aData, nMaxBytesToRead );
}

void SAL_CALL UNOMemoryStream::skipBytes( sal_Int32 nBytesToSkip ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    if( nBytesToSkip < 0 )
        throw IOException();

    mnCursor += std::min( nBytesToSkip, available() );
}

sal_Int32 SAL_CALL UNOMemoryStream::available() throw (NotConnectedException, IOException, RuntimeException)
{
    return static_cast< sal_Int32 >( maData.size() ) - mnCursor;
}

void SAL_CALL UNOMemoryStream::closeInput() throw (NotConnectedException, IOException, RuntimeException)
{
    mnCursor = 0;
}

// XSeekable
void SAL_CALL UNOMemoryStream::seek( sal_Int64 location ) throw (IllegalArgumentException, IOException, RuntimeException)
{
    if( (location < 0) || (location > SAL_MAX_INT32) )
        throw IllegalArgumentException( OUString(RTL_CONSTASCII_USTRINGPARAM("this implementation does not support more than 2GB!")), Reference< XInterface >(static_cast<OWeakObject*>(this)), 0 );

    // seek operation should be able to resize the stream
    if ( location > static_cast< sal_Int64 >( maData.size() ) )
        maData.resize( static_cast< sal_Int32 >( location ) );

    if ( location > static_cast< sal_Int64 >( maData.size() ) )
        maData.resize( static_cast< sal_Int32 >( location ) );

    mnCursor = static_cast< sal_Int32 >( location );
}

sal_Int64 SAL_CALL UNOMemoryStream::getPosition() throw (IOException, RuntimeException)
{
    return static_cast< sal_Int64 >( mnCursor );
}

sal_Int64 SAL_CALL UNOMemoryStream::getLength() throw (IOException, RuntimeException)
{
    return static_cast< sal_Int64 >( maData.size() );
}

// XOutputStream
void SAL_CALL UNOMemoryStream::writeBytes( const Sequence< sal_Int8 >& aData ) throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    const sal_Int32 nBytesToWrite( aData.getLength() );
    if( nBytesToWrite )
    {
        sal_Int64 nNewSize = static_cast< sal_Int64 >( mnCursor + nBytesToWrite );
        if( nNewSize > SAL_MAX_INT32 )
        {
            OSL_ASSERT(false);
            throw IOException( OUString(RTL_CONSTASCII_USTRINGPARAM("this implementation does not support more than 2GB!")), Reference< XInterface >(static_cast<OWeakObject*>(this)) );
        }

        if( static_cast< sal_Int32 >( nNewSize ) > static_cast< sal_Int32 >( maData.size() ) )
            maData.resize( static_cast< sal_Int32 >( nNewSize ) );

        sal_Int8* pData = static_cast<sal_Int8*>(&(*maData.begin()));
        sal_Int8* pCursor = &(pData[mnCursor]);
        memcpy( (void*)pCursor, (void*)aData.getConstArray(), nBytesToWrite );

        mnCursor += nBytesToWrite;
    }
}

void SAL_CALL UNOMemoryStream::flush() throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
}

void SAL_CALL UNOMemoryStream::closeOutput() throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    mnCursor = 0;
}

//XTruncate
void SAL_CALL UNOMemoryStream::truncate() throw (IOException, RuntimeException)
{
    maData.resize( 0 );
    mnCursor = 0;
}

::rtl::OUString SAL_CALL UNOMemoryStream::getImplementationName_static()
{
    static const OUString sImplName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.MemoryStream" ) );
    return sImplName;
}

Sequence< ::rtl::OUString > SAL_CALL UNOMemoryStream::getSupportedServiceNames_static()
{
    Sequence< OUString > aSeq(1);
    aSeq[0] = getImplementationName_static();
    return aSeq;
}

Reference< XInterface > SAL_CALL UNOMemoryStream::Create(
    const Reference< XComponentContext >& )
{
    return static_cast<OWeakObject*>(new UNOMemoryStream());
}

} // namespace comphelper

void createRegistryInfo_UNOMemoryStream()
{
    static ::comphelper::module::OAutoRegistration< ::comphelper::UNOMemoryStream > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
