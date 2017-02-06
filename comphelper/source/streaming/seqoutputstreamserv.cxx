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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/XSequenceOutputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star;


namespace {

class SequenceOutputStreamService:
    public cppu::WeakImplHelper<lang::XServiceInfo, io::XSequenceOutputStream>
{
public:
    explicit SequenceOutputStreamService();

    // noncopyable
    SequenceOutputStreamService(const SequenceOutputStreamService&) = delete;
    const SequenceOutputStreamService& operator=(const SequenceOutputStreamService&) = delete;

    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString & ServiceName ) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::io::XOutputStream:
    virtual void SAL_CALL writeBytes( const uno::Sequence< ::sal_Int8 > & aData ) override;
    virtual void SAL_CALL flush() override;
    virtual void SAL_CALL closeOutput() override;

    // css::io::XSequenceOutputStream:
    virtual uno::Sequence< ::sal_Int8 > SAL_CALL getWrittenBytes(  ) override;

private:
    virtual ~SequenceOutputStreamService() override {};


    ::osl::Mutex m_aMutex;
    uno::Reference< io::XOutputStream > m_xOutputStream;
    uno::Sequence< ::sal_Int8 > m_aSequence;
};
SequenceOutputStreamService::SequenceOutputStreamService()
{
    m_xOutputStream.set( static_cast < ::cppu::OWeakObject* >( new ::comphelper::OSequenceOutputStream( m_aSequence ) ), uno::UNO_QUERY_THROW );
}

// com.sun.star.uno.XServiceInfo:
OUString SAL_CALL SequenceOutputStreamService::getImplementationName()
{
    return OUString("com.sun.star.comp.SequenceOutputStreamService");
}

sal_Bool SAL_CALL SequenceOutputStreamService::supportsService( OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

uno::Sequence< OUString > SAL_CALL SequenceOutputStreamService::getSupportedServiceNames()
{
    uno::Sequence<OUString> s { "com.sun.star.io.SequenceOutputStream" };
    return s;
}

// css::io::XOutputStream:
void SAL_CALL SequenceOutputStreamService::writeBytes( const uno::Sequence< ::sal_Int8 > & aData )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->writeBytes( aData );
    m_aSequence = aData;
}

void SAL_CALL SequenceOutputStreamService::flush()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->flush();
};

void SAL_CALL SequenceOutputStreamService::closeOutput()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->closeOutput();
    m_xOutputStream.clear();
}

// css::io::XSequenceOutputStream:
uno::Sequence< ::sal_Int8 > SAL_CALL SequenceOutputStreamService::getWrittenBytes()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->flush();
    return m_aSequence;
}

} // anonymous namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_SequenceOutputStreamService(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SequenceOutputStreamService());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
