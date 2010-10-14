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

// MARKER( update_precomp.py ): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include "comphelper_module.hxx"

#include <sal/config.h>
#include <osl/mutex.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase3.hxx>
#include <comphelper/seqstream.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XSeekableInputStream.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>


using namespace ::com::sun::star;

namespace {

class SequenceInputStreamService:
    public ::cppu::WeakImplHelper3<
        lang::XServiceInfo,
        io::XSeekableInputStream,
        lang::XInitialization>
{
public:
    explicit SequenceInputStreamService();

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw ( uno::RuntimeException );
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString & ServiceName ) throw ( uno::RuntimeException );
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw ( uno::RuntimeException );

    // XServiceInfo - static versions (used for component registration)
    static ::rtl::OUString SAL_CALL getImplementationName_static();
    static uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static();
    static uno::Reference< uno::XInterface > SAL_CALL Create( const uno::Reference< uno::XComponentContext >& );

    // ::com::sun::star::io::XInputStream:
    virtual ::sal_Int32 SAL_CALL readBytes( uno::Sequence< ::sal_Int8 > & aData, ::sal_Int32 nBytesToRead ) throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException );
    virtual ::sal_Int32 SAL_CALL readSomeBytes( uno::Sequence< ::sal_Int8 > & aData, ::sal_Int32 nMaxBytesToRead ) throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException );
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException );
    virtual ::sal_Int32 SAL_CALL available() throw ( uno::RuntimeException, io::NotConnectedException, io::IOException );
    virtual void SAL_CALL closeInput() throw ( uno::RuntimeException, io::NotConnectedException, io::IOException );

    // ::com::sun::star::io::XSeekable:
    virtual void SAL_CALL seek( ::sal_Int64 location ) throw ( uno::RuntimeException, lang::IllegalArgumentException, io::IOException );
    virtual ::sal_Int64 SAL_CALL getPosition() throw ( uno::RuntimeException, io::IOException );
    virtual ::sal_Int64 SAL_CALL getLength() throw ( uno::RuntimeException, io::IOException );

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize( const uno::Sequence< ::com::sun::star::uno::Any > & aArguments ) throw ( uno::RuntimeException, uno::Exception );

private:
    SequenceInputStreamService( SequenceInputStreamService & ); // not defined
    void operator =( SequenceInputStreamService & ); // not defined

    virtual ~SequenceInputStreamService() {}


    ::osl::Mutex m_aMutex;
    sal_Bool m_bInitialized;
    uno::Reference< io::XInputStream > m_xInputStream;
    uno::Reference< io::XSeekable > m_xSeekable;
};

SequenceInputStreamService::SequenceInputStreamService()
: m_bInitialized( sal_False )
{}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL SequenceInputStreamService::getImplementationName() throw ( uno::RuntimeException )
{
    return getImplementationName_static();
}

::rtl::OUString SAL_CALL SequenceInputStreamService::getImplementationName_static()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.SequenceInputStreamService" ) );
}

::sal_Bool SAL_CALL SequenceInputStreamService::supportsService( ::rtl::OUString const & serviceName ) throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > serviceNames = getSupportedServiceNames();
    for ( ::sal_Int32 i = 0; i < serviceNames.getLength(); ++i ) {
        if ( serviceNames[i] == serviceName )
            return sal_True;
    }
    return sal_False;
}

uno::Sequence< ::rtl::OUString > SAL_CALL SequenceInputStreamService::getSupportedServiceNames() throw ( uno::RuntimeException )
{
    return getSupportedServiceNames_static();
}

uno::Sequence< ::rtl::OUString > SAL_CALL SequenceInputStreamService::getSupportedServiceNames_static()
{
    uno::Sequence< ::rtl::OUString > s( 1 );
    s[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.io.SequenceInputStream" ) );
    return s;
}

uno::Reference< uno::XInterface > SAL_CALL SequenceInputStreamService::Create(
    const uno::Reference< uno::XComponentContext >& )
{
    return static_cast< ::cppu::OWeakObject * >( new SequenceInputStreamService() );
}

// ::com::sun::star::io::XInputStream:
::sal_Int32 SAL_CALL SequenceInputStreamService::readBytes( uno::Sequence< ::sal_Int8 > & aData, ::sal_Int32 nBytesToRead ) throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xInputStream.is() )
        throw io::NotConnectedException();

    return m_xInputStream->readBytes( aData, nBytesToRead );
}

::sal_Int32 SAL_CALL SequenceInputStreamService::readSomeBytes( uno::Sequence< ::sal_Int8 > & aData, ::sal_Int32 nMaxBytesToRead ) throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xInputStream.is() )
        throw io::NotConnectedException();

    return m_xInputStream->readSomeBytes( aData, nMaxBytesToRead );
}

void SAL_CALL SequenceInputStreamService::skipBytes( ::sal_Int32 nBytesToSkip ) throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xInputStream.is() )
        throw io::NotConnectedException();

    return m_xInputStream->skipBytes( nBytesToSkip );
}

::sal_Int32 SAL_CALL SequenceInputStreamService::available() throw ( uno::RuntimeException, io::NotConnectedException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xInputStream.is() )
        throw io::NotConnectedException();

    return m_xInputStream->available();
}

void SAL_CALL SequenceInputStreamService::closeInput() throw ( uno::RuntimeException, io::NotConnectedException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xInputStream.is() )
        throw io::NotConnectedException();

    m_xInputStream->closeInput();
    m_xInputStream = uno::Reference< io::XInputStream >();
    m_xSeekable = uno::Reference< io::XSeekable >();
}

// ::com::sun::star::io::XSeekable:
void SAL_CALL SequenceInputStreamService::seek( ::sal_Int64 location ) throw ( uno::RuntimeException, lang::IllegalArgumentException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xSeekable.is() )
        throw io::NotConnectedException();

    m_xSeekable->seek( location );
}

::sal_Int64 SAL_CALL SequenceInputStreamService::getPosition() throw ( uno::RuntimeException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xSeekable.is() )
        throw io::NotConnectedException();

    return m_xSeekable->getPosition();
}

::sal_Int64 SAL_CALL SequenceInputStreamService::getLength() throw ( uno::RuntimeException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xSeekable.is() )
        throw io::NotConnectedException();

    return m_xSeekable->getLength();
}

// ::com::sun::star::lang::XInitialization:
void SAL_CALL SequenceInputStreamService::initialize( const uno::Sequence< ::com::sun::star::uno::Any > & aArguments ) throw ( uno::RuntimeException, uno::Exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bInitialized )
        throw frame::DoubleInitializationException();

    if ( aArguments.getLength() != 1 )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Wrong number of arguments!\n" ),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    uno::Sequence< sal_Int8 > aSeq;
    if ( aArguments[0] >>= aSeq )
    {
        uno::Reference< io::XInputStream > xInputStream(
                        static_cast< ::cppu::OWeakObject* >( new ::comphelper::SequenceInputStream( aSeq ) ),
                        uno::UNO_QUERY_THROW );
        uno::Reference< io::XSeekable > xSeekable( xInputStream, uno::UNO_QUERY_THROW );
        m_xInputStream = xInputStream;
        m_xSeekable = xSeekable;
        m_bInitialized = sal_True;
    }
    else
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Unexpected type of argument!\n" ),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );
}

} // anonymous namespace

void createRegistryInfo_SequenceInputStream()
{
    static ::comphelper::module::OAutoRegistration< SequenceInputStreamService > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
