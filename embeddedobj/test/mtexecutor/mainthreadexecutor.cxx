/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_embeddedobj.hxx"

#include "mainthreadexecutor.hxx"

#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL MainThreadExecutor::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.thread.MainThreadExecutor");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.thread.MainThreadExecutor");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL MainThreadExecutor::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.thread.MainThreadExecutor");
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
        if ( aArguments[nInd].Name.equalsAscii( "JobToExecute" ) )
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

