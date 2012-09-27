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

#include "xexecutor.hxx"
#include <vcl/svapp.hxx>
#include <osl/thread.hxx>

using namespace ::com::sun::star;

void MainThreadExecutor_Impl::execute()
{
    Application::PostUserEvent( LINK( this, MainThreadExecutor_Impl, executor ), NULL );
}

IMPL_LINK( MainThreadExecutor_Impl, executor, void*, pDummyParam )
{
    if ( m_xJob.is() )
    {
        try {
            m_xJob->execute( m_aArgs );
        } catch( uno::Exception& ) {}
    }

    m_bExecuted = sal_True;
    delete this;

    return 0;
}

MainThreadExecutor_Impl::MainThreadExecutor_Impl( const uno::Reference< task::XJob >& xJob,
                                                  const uno::Sequence< beans::NamedValue >& aArguments )
: m_xJob( xJob )
, m_aArgs( aArguments )
, m_bExecuted( sal_False )
{
}

//-------------------------------------------------------------------------
uno::Any SAL_CALL UNOMainThreadExecutor::execute( const uno::Sequence< beans::NamedValue >& aArguments )
    throw ( lang::IllegalArgumentException,
            uno::Exception,
            uno::RuntimeException )
{
    uno::Reference< task::XJob > xJob;

    if ( aArguments.getLength() > 0 && aArguments[0].Name == "JobToExecute" )
        aArguments[0].Value >>= xJob;

    if ( !xJob.is() )
        throw lang::IllegalArgumentException();

    uno::Sequence< beans::NamedValue > aArgsForJob;
    if ( aArguments.getLength() > 1 )
        aArgsForJob = uno::Sequence< beans::NamedValue >( aArguments.getConstArray() + 1, aArguments.getLength() - 1 );

    MainThreadExecutor_Impl* pExecutor = new MainThreadExecutor_Impl( xJob, aArgsForJob );
    pExecutor->execute();

    // it is not a main thread, so it can be blocked
    // while( !pExecutor->isExecuted() )
    //  ::osl::Thread::yield();

    // TODO: implement transfering of the return values and exceptions

    return uno::Any();
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL UNOMainThreadExecutor::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = "com.sun.star.comp.thread.MainThreadExecutor";
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL UNOMainThreadExecutor::impl_staticGetImplementationName()
{
    return ::rtl::OUString("com.sun.star.comp.thread.MainThreadExecutor");
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL UNOMainThreadExecutor::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new UNOMainThreadExecutor( xServiceManager ) );
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL UNOMainThreadExecutor::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL UNOMainThreadExecutor::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL UNOMainThreadExecutor::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
