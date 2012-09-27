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

#include "mainthreadexecutor.hxx"

#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL MainThreadExecutor::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = "com.sun.star.thread.MainThreadExecutor";
    aRet[1] = "com.sun.star.comp.thread.MainThreadExecutor";
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL MainThreadExecutor::impl_staticGetImplementationName()
{
    return ::rtl::OUString("com.sun.star.comp.thread.MainThreadExecutor");
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL MainThreadExecutor::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new MainThreadExecutor( xServiceManager ) );
}

//-------------------------------------------------------------------------
uno::Any SAL_CALL MainThreadExecutor::execute( const uno::Sequence< beans::NamedValue >& aArguments )
    throw ( lang::IllegalArgumentException,
            uno::Exception,
            uno::RuntimeException )
{
    uno::Reference< task::XJob > xJob;
    uno::Sequence< beans::NamedValue > aValues;
    sal_Int32 nValuesSize = 0;

    for ( sal_Int32 nInd = 0; nInd < aArguments.getLength(); nInd++ )
        if ( aArguments[nInd].Name == "JobToExecute" )
            aArguments[nInd].Value >>= xJob;
        else
        {
            aValues.realloc( ++nValuesSize );
            aValues[nValuesSize-1].Name = aArguments[nInd].Name;
            aValues[nValuesSize-1].Value = aArguments[nInd].Value;
        }

    if ( xJob.is() )
    {
        MainThreadExecutorRequest* pMainThreadExecutorRequest = new MainThreadExecutorRequest( xJob, aValues );
        Application::PostUserEvent( STATIC_LINK( NULL, MainThreadExecutor, worker ), pMainThreadExecutorRequest );
    }

    // TODO: wait for result
    return uno::Any();
}

//-------------------------------------------------------------------------
IMPL_STATIC_LINK( MainThreadExecutor, worker, MainThreadExecutorRequest*, pThreadExecutorRequest )
{
    pThreadExecutorRequest->doIt();

    delete pThreadExecutorRequest;
    return 0;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL MainThreadExecutor::getImplementationName()
        throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL MainThreadExecutor::supportsService( const ::rtl::OUString& ServiceName )
        throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL MainThreadExecutor::getSupportedServiceNames()
        throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

//-------------------------------------------------------------------------
MainThreadExecutorRequest::MainThreadExecutorRequest( const uno::Reference< task::XJob >& xJob,
                                                    const uno::Sequence< beans::NamedValue >& aValues )
: m_xJob( xJob )
, m_aValues( aValues )
{
}

//-------------------------------------------------------------------------
void MainThreadExecutorRequest::doIt()
{
    if ( m_xJob.is() )
        m_xJob->execute( m_aValues );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
