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

#include "precompiled_comphelper.hxx"

#include "comphelper_module.hxx"

#include <sal/config.h>
#include <osl/mutex.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase2.hxx>
#include <comphelper/seqstream.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XSequenceOutputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star;


namespace {

class SequenceOutputStreamService:
public ::cppu::WeakImplHelper2 < lang::XServiceInfo, io::XSequenceOutputStream >
{
public:
    explicit SequenceOutputStreamService();

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw ( uno::RuntimeException );
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString & ServiceName ) throw ( uno::RuntimeException );
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw ( uno::RuntimeException );

    // XServiceInfo - static versions (used for component registration)
    static ::rtl::OUString SAL_CALL getImplementationName_static();
    static uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static();
    static uno::Reference< uno::XInterface > SAL_CALL Create( const uno::Reference< uno::XComponentContext >& );

    // ::com::sun::star::io::XOutputStream:
    virtual void SAL_CALL writeBytes( const uno::Sequence< ::sal_Int8 > & aData ) throw ( io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException );
    virtual void SAL_CALL flush() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException );
    virtual void SAL_CALL closeOutput() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException );

    // ::com::sun::star::io::XSequenceOutputStream:
    virtual uno::Sequence< ::sal_Int8 > SAL_CALL getWrittenBytes(  ) throw ( io::NotConnectedException, io::IOException, uno::RuntimeException);

private:
    SequenceOutputStreamService( SequenceOutputStreamService & ); //not defined
    void operator =( SequenceOutputStreamService & ); //not defined

    virtual ~SequenceOutputStreamService() {};


    ::osl::Mutex m_aMutex;
    uno::Reference< io::XOutputStream > m_xOutputStream;
    uno::Sequence< ::sal_Int8 > m_aSequence;
};
SequenceOutputStreamService::SequenceOutputStreamService()
{
    m_xOutputStream.set( static_cast < ::cppu::OWeakObject* >( new ::comphelper::OSequenceOutputStream( m_aSequence ) ), uno::UNO_QUERY_THROW );
}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL SequenceOutputStreamService::getImplementationName() throw ( uno::RuntimeException )
{
    return getImplementationName_static();
}

::rtl::OUString SAL_CALL SequenceOutputStreamService::getImplementationName_static()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.SequenceOutputStreamService" ) );
}

::sal_Bool SAL_CALL SequenceOutputStreamService::supportsService( ::rtl::OUString const & serviceName ) throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > serviceNames = getSupportedServiceNames();
    for ( ::sal_Int32 i = 0; i < serviceNames.getLength(); ++i ) {
        if ( serviceNames[i] == serviceName )
            return sal_True;
    }
    return sal_False;
}

uno::Sequence< ::rtl::OUString > SAL_CALL SequenceOutputStreamService::getSupportedServiceNames() throw ( uno::RuntimeException )
{
    return getSupportedServiceNames_static();
}

uno::Sequence< ::rtl::OUString > SAL_CALL SequenceOutputStreamService::getSupportedServiceNames_static()
{
    uno::Sequence< ::rtl::OUString > s( 1 );
    s[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.SequenceOutputStream" ) );
    return s;
}

uno::Reference< uno::XInterface > SAL_CALL SequenceOutputStreamService::Create(
    const uno::Reference< uno::XComponentContext >& )
{
    return static_cast< ::cppu::OWeakObject * >( new SequenceOutputStreamService());
}

// ::com::sun::star::io::XOutputStream:
void SAL_CALL SequenceOutputStreamService::writeBytes( const uno::Sequence< ::sal_Int8 > & aData ) throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->writeBytes( aData );
    m_aSequence = aData;
}

void SAL_CALL SequenceOutputStreamService::flush() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->flush();
};

void SAL_CALL SequenceOutputStreamService::closeOutput() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->closeOutput();
    m_xOutputStream = uno::Reference< io::XOutputStream >();
}

// ::com::sun::star::io::XSequenceOutputStream:
uno::Sequence< ::sal_Int8 > SAL_CALL SequenceOutputStreamService::getWrittenBytes() throw ( io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->flush();
    return m_aSequence;
}

} // anonymous namespace

void createRegistryInfo_SequenceOutputStream()
{
    static ::comphelper::module::OAutoRegistration< SequenceOutputStreamService > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
