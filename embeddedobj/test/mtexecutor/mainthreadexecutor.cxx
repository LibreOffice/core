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
#include "precompiled_embeddedobj.hxx"

#include "mainthreadexecutor.hxx"

#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL MainThreadExecutor::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.thread.MainThreadExecutor"));
    aRet[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.thread.MainThreadExecutor"));
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL MainThreadExecutor::impl_staticGetImplementationName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.thread.MainThreadExecutor"));
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
        if ( aArguments[nInd].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "JobToExecute" ) ) )
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
