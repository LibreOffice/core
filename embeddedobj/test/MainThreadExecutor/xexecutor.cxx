/*************************************************************************
 *
 *  $RCSfile: xexecutor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-20 15:55:19 $
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

    if ( aArguments.getLength() > 0 && aArguments[0].Name.equalsAscii( "JobToExecute" ) )
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
    aRet[0] = ::rtl::OUString::createFromAscii( "com.sun.star.comp.thread.MainThreadExecutor" );
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL UNOMainThreadExecutor::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.thread.MainThreadExecutor" );
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

