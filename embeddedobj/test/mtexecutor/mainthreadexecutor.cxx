/*************************************************************************
 *
 *  $RCSfile: mainthreadexecutor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mav $ $Date: 2003-12-17 11:32:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

