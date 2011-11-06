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
#include "ZPooledConnection.hxx"
#include "ZConnectionWrapper.hxx"
#ifndef _CONNECTIVITY_CONNECTIONWRAPPER_HXX_
#include "connectivity/ConnectionWrapper.hxx"
#endif
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <comphelper/types.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/compbase1.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::reflection;
using namespace connectivity;
using namespace ::osl;

OPooledConnection::OPooledConnection(const Reference< XConnection >& _xConnection,
                                    const Reference< ::com::sun::star::reflection::XProxyFactory >& _rxProxyFactory)
    : OPooledConnection_Base(m_aMutex)
    ,m_xRealConnection(_xConnection)
    ,m_xProxyFactory(_rxProxyFactory)
{

}
// -----------------------------------------------------------------------------
// OComponentHelper
void SAL_CALL OPooledConnection::disposing(void)
{
    MutexGuard aGuard(m_aMutex);
    if (m_xComponent.is())
        m_xComponent->removeEventListener(this);
m_xComponent.clear();
    ::comphelper::disposeComponent(m_xRealConnection);
}
// -----------------------------------------------------------------------------
// XEventListener
void SAL_CALL OPooledConnection::disposing( const EventObject& /*Source*/ ) throw (RuntimeException)
{
m_xComponent.clear();
}
// -----------------------------------------------------------------------------
//XPooledConnection
Reference< XConnection > OPooledConnection::getConnection()  throw(SQLException, RuntimeException)
{
    if(!m_xComponent.is() && m_xRealConnection.is())
    {
        Reference< XAggregation > xConProxy = m_xProxyFactory->createProxy(m_xRealConnection.get());
        m_xComponent = new OConnectionWeakWrapper(xConProxy);
        // register as event listener for the new connection
        if (m_xComponent.is())
            m_xComponent->addEventListener(this);
    }
    return Reference< XConnection >(m_xComponent,UNO_QUERY);
}
// -----------------------------------------------------------------------------

