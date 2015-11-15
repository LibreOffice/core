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

#include <boost/noncopyable.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/seqstream.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XSequenceOutputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star;


namespace {

class SequenceOutputStreamService:
    public cppu::WeakImplHelper<lang::XServiceInfo, io::XSequenceOutputStream>,
    private boost::noncopyable
{
public:
    explicit SequenceOutputStreamService();

    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() throw ( uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString & ServiceName ) throw ( uno::RuntimeException, std::exception ) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw ( uno::RuntimeException, std::exception ) override;

    // XServiceInfo - static versions (used for component registration)
    static OUString SAL_CALL getImplementationName_static();
    static uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static();
    static uno::Reference< uno::XInterface > SAL_CALL Create( const uno::Reference< uno::XComponentContext >& );

    // css::io::XOutputStream:
    virtual void SAL_CALL writeBytes( const uno::Sequence< ::sal_Int8 > & aData ) throw ( io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL flush() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException, std::exception ) override;
    virtual void SAL_CALL closeOutput() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException, std::exception ) override;

    // css::io::XSequenceOutputStream:
    virtual uno::Sequence< ::sal_Int8 > SAL_CALL getWrittenBytes(  ) throw ( io::NotConnectedException, io::IOException, uno::RuntimeException, std::exception) override;

private:
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
OUString SAL_CALL SequenceOutputStreamService::getImplementationName() throw ( uno::RuntimeException, std::exception )
{
    return getImplementationName_static();
}

OUString SAL_CALL SequenceOutputStreamService::getImplementationName_static()
{
    return OUString("com.sun.star.comp.SequenceOutputStreamService");
}

sal_Bool SAL_CALL SequenceOutputStreamService::supportsService( OUString const & serviceName ) throw ( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, serviceName);
}

uno::Sequence< OUString > SAL_CALL SequenceOutputStreamService::getSupportedServiceNames() throw ( uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_static();
}

uno::Sequence< OUString > SAL_CALL SequenceOutputStreamService::getSupportedServiceNames_static()
{
    uno::Sequence<OUString> s { "com.sun.star.io.SequenceOutputStream" };
    return s;
}

uno::Reference< uno::XInterface > SAL_CALL SequenceOutputStreamService::Create(
    SAL_UNUSED_PARAMETER const uno::Reference< uno::XComponentContext >& )
{
    return static_cast< ::cppu::OWeakObject * >( new SequenceOutputStreamService());
}

// css::io::XOutputStream:
void SAL_CALL SequenceOutputStreamService::writeBytes( const uno::Sequence< ::sal_Int8 > & aData ) throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->writeBytes( aData );
    m_aSequence = aData;
}

void SAL_CALL SequenceOutputStreamService::flush() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->flush();
};

void SAL_CALL SequenceOutputStreamService::closeOutput() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->closeOutput();
    m_xOutputStream.clear();
}

// css::io::XSequenceOutputStream:
uno::Sequence< ::sal_Int8 > SAL_CALL SequenceOutputStreamService::getWrittenBytes() throw ( io::NotConnectedException, io::IOException, uno::RuntimeException, std::exception)
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
