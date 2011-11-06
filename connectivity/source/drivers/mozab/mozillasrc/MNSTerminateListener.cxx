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
#include "precompiled_connectivity.hxx"
#include <MNSTerminateListener.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/XDesktop.hpp>
#include <MNSInit.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
::com::sun::star::uno::Reference< ::com::sun::star::frame::XTerminateListener>          MNSTerminateListener::mxTerminateListener = new MNSTerminateListener();

// -----------------------------------------
// - MNSTerminateListener -
// -----------------------------------------

MNSTerminateListener::MNSTerminateListener(  )
{
}

// -----------------------------------------------------------------------------

MNSTerminateListener::~MNSTerminateListener()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL MNSTerminateListener::disposing( const EventObject& /*Source*/ ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL MNSTerminateListener::queryTermination( const EventObject& /*aEvent*/ ) throw( TerminationVetoException, RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL MNSTerminateListener::notifyTermination( const EventObject& /*aEvent*/ ) throw( RuntimeException )
{
    MNS_Term(sal_True); //Force XPCOM to shutdown
}

void MNSTerminateListener::addTerminateListener()
{
    Reference< XMultiServiceFactory >   xFact( ::comphelper::getProcessServiceFactory() );

    if( xFact.is() )
    {
        Reference< XDesktop > xDesktop( xFact->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );

        if( xDesktop.is() )
            xDesktop->addTerminateListener(mxTerminateListener);
    }
}

