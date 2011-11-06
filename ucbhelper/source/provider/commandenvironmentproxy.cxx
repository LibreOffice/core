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
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <osl/mutex.hxx>
#include <ucbhelper/commandenvironmentproxy.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace rtl;

namespace ucbhelper
{

//=========================================================================
//=========================================================================
//
// struct CommandEnvironmentProxy_Impl.
//
//=========================================================================
//=========================================================================

struct CommandEnvironmentProxy_Impl
{
    osl::Mutex                       m_aMutex;
    Reference< XCommandEnvironment > m_xEnv;
    Reference< XInteractionHandler > m_xInteractionHandler;
    Reference< XProgressHandler >    m_xProgressHandler;
    sal_Bool m_bGotInteractionHandler;
    sal_Bool m_bGotProgressHandler;

    CommandEnvironmentProxy_Impl(
        const Reference< XCommandEnvironment >& rxEnv )
    : m_xEnv( rxEnv ), m_bGotInteractionHandler( sal_False ),
      m_bGotProgressHandler( sal_False ) {}
};

//=========================================================================
//=========================================================================
//
// CommandEnvironmentProxy Implementation.
//
//=========================================================================
//=========================================================================

CommandEnvironmentProxy::CommandEnvironmentProxy(
                        const Reference< XCommandEnvironment >& rxEnv )
{
    m_pImpl = new CommandEnvironmentProxy_Impl( rxEnv );
}

//=========================================================================
// virtual
CommandEnvironmentProxy::~CommandEnvironmentProxy()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods
//
//=========================================================================

XINTERFACE_IMPL_2( CommandEnvironmentProxy,
                   XTypeProvider,
                   XCommandEnvironment );

//=========================================================================
//
// XTypeProvider methods
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( CommandEnvironmentProxy,
                      XTypeProvider,
                      XCommandEnvironment );

//=========================================================================
//
// XCommandEnvironemnt methods.
//
//=========================================================================

// virtual
Reference< XInteractionHandler > SAL_CALL
CommandEnvironmentProxy::getInteractionHandler()
    throw ( RuntimeException )
{
    if ( m_pImpl->m_xEnv.is() )
    {
        if ( !m_pImpl->m_bGotInteractionHandler )
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            if ( !m_pImpl->m_bGotInteractionHandler )
            {
                m_pImpl->m_xInteractionHandler
                                = m_pImpl->m_xEnv->getInteractionHandler();
                m_pImpl->m_bGotInteractionHandler = sal_True;
            }
        }
    }
    return m_pImpl->m_xInteractionHandler;
}

//=========================================================================
// virtual
Reference< XProgressHandler > SAL_CALL
CommandEnvironmentProxy::getProgressHandler()
    throw ( RuntimeException )
{
    if ( m_pImpl->m_xEnv.is() )
    {
        if ( !m_pImpl->m_bGotProgressHandler )
        {
            osl::MutexGuard aGuard( m_pImpl->m_aMutex );
            if ( !m_pImpl->m_bGotProgressHandler )
            {
                m_pImpl->m_xProgressHandler
                                = m_pImpl->m_xEnv->getProgressHandler();
                m_pImpl->m_bGotProgressHandler = sal_True;
            }
        }
    }
    return m_pImpl->m_xProgressHandler;
}

} /* namespace ucbhelper */

