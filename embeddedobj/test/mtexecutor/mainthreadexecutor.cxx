/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mainthreadexecutor.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:46:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

