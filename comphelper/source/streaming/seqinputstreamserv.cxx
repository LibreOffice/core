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

#include <sal/config.h>

#include "comphelper_module.hxx"
#include "comphelper_services.hxx"

#include <osl/mutex.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/seqstream.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/XSeekableInputStream.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>


using namespace ::com::sun::star;

namespace {

class SequenceInputStreamService:
    public ::cppu::WeakImplHelper<
        lang::XServiceInfo,
        io::XSeekableInputStream,
        lang::XInitialization>
{
public:
    explicit SequenceInputStreamService();

    // noncopyable
    SequenceInputStreamService(const SequenceInputStreamService&) = delete;
    const SequenceInputStreamService& operator=(const SequenceInputStreamService&) = delete;

    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString & ServiceName ) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::io::XInputStream:
    virtual ::sal_Int32 SAL_CALL readBytes( uno::Sequence< ::sal_Int8 > & aData, ::sal_Int32 nBytesToRead ) override;
    virtual ::sal_Int32 SAL_CALL readSomeBytes( uno::Sequence< ::sal_Int8 > & aData, ::sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) override;
    virtual ::sal_Int32 SAL_CALL available() override;
    virtual void SAL_CALL closeInput() override;

    // css::io::XSeekable:
    virtual void SAL_CALL seek( ::sal_Int64 location ) override;
    virtual ::sal_Int64 SAL_CALL getPosition() override;
    virtual ::sal_Int64 SAL_CALL getLength() override;

    // css::lang::XInitialization:
    virtual void SAL_CALL initialize( const uno::Sequence< css::uno::Any > & aArguments ) override;

private:
    virtual ~SequenceInputStreamService() override {}


    ::osl::Mutex m_aMutex;
    bool m_bInitialized;
    uno::Reference< io::XInputStream > m_xInputStream;
    uno::Reference< io::XSeekable > m_xSeekable;
};

SequenceInputStreamService::SequenceInputStreamService()
: m_bInitialized( false )
{}

// com.sun.star.uno.XServiceInfo:
OUString SAL_CALL SequenceInputStreamService::getImplementationName()
{
    return OUString ( "com.sun.star.comp.SequenceInputStreamService" );
}

sal_Bool SAL_CALL SequenceInputStreamService::supportsService( OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

uno::Sequence< OUString > SAL_CALL SequenceInputStreamService::getSupportedServiceNames()
{
    uno::Sequence<OUString> s { "com.sun.star.io.SequenceInputStream" };
    return s;
}

// css::io::XInputStream:
::sal_Int32 SAL_CALL SequenceInputStreamService::readBytes( uno::Sequence< ::sal_Int8 > & aData, ::sal_Int32 nBytesToRead )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xInputStream.is() )
        throw io::NotConnectedException();

    return m_xInputStream->readBytes( aData, nBytesToRead );
}

::sal_Int32 SAL_CALL SequenceInputStreamService::readSomeBytes( uno::Sequence< ::sal_Int8 > & aData, ::sal_Int32 nMaxBytesToRead )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xInputStream.is() )
        throw io::NotConnectedException();

    return m_xInputStream->readSomeBytes( aData, nMaxBytesToRead );
}

void SAL_CALL SequenceInputStreamService::skipBytes( ::sal_Int32 nBytesToSkip )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xInputStream.is() )
        throw io::NotConnectedException();

    return m_xInputStream->skipBytes( nBytesToSkip );
}

::sal_Int32 SAL_CALL SequenceInputStreamService::available()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xInputStream.is() )
        throw io::NotConnectedException();

    return m_xInputStream->available();
}

void SAL_CALL SequenceInputStreamService::closeInput()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xInputStream.is() )
        throw io::NotConnectedException();

    m_xInputStream->closeInput();
    m_xInputStream.clear();
    m_xSeekable.clear();
}

// css::io::XSeekable:
void SAL_CALL SequenceInputStreamService::seek( ::sal_Int64 location )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xSeekable.is() )
        throw io::NotConnectedException();

    m_xSeekable->seek( location );
}

::sal_Int64 SAL_CALL SequenceInputStreamService::getPosition()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xSeekable.is() )
        throw io::NotConnectedException();

    return m_xSeekable->getPosition();
}

::sal_Int64 SAL_CALL SequenceInputStreamService::getLength()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xSeekable.is() )
        throw io::NotConnectedException();

    return m_xSeekable->getLength();
}

// css::lang::XInitialization:
void SAL_CALL SequenceInputStreamService::initialize( const uno::Sequence< css::uno::Any > & aArguments )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bInitialized )
        throw frame::DoubleInitializationException();

    if ( aArguments.getLength() != 1 )
        throw lang::IllegalArgumentException( "Wrong number of arguments!",
                                            static_cast< ::cppu::OWeakObject* >(this),
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
        m_bInitialized = true;
    }
    else
        throw lang::IllegalArgumentException( "Unexpected type of argument!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            1 );
}

} // anonymous namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_SequenceInputStreamService(
                                             css::uno::XComponentContext *,
                                             css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SequenceInputStreamService());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
