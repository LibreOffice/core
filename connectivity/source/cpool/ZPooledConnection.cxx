/*************************************************************************
 *
 *  $RCSfile: ZPooledConnection.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2002-08-12 09:04:22 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef CONNECTIVITY_POOLEDCONNECTION_HXX
#include "ZPooledConnection.hxx"
#endif
#ifndef _CONNECTIVITY_CONNECTIONWRAPPER_HXX_
#include "connectivity/ConnectionWrapper.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif

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
    m_xComponent = NULL;
    ::comphelper::disposeComponent(m_xRealConnection);
}
// -----------------------------------------------------------------------------
// XEventListener
void SAL_CALL OPooledConnection::disposing( const EventObject& Source ) throw (RuntimeException)
{
    m_xComponent = NULL;
}
// -----------------------------------------------------------------------------
namespace connectivity
{
    using namespace ::cppu;
    typedef ::cppu::WeakComponentImplHelper1< XCloseable > OConnectionWeakWrapper_BASE;
    class OConnectionWeakWrapper :  public ::comphelper::OBaseMutex
                                  , public OConnectionWeakWrapper_BASE
                                  , public OConnectionWrapper
    {
    public:
        OConnectionWeakWrapper(Reference< XAggregation >& _xConnection)
            : OConnectionWeakWrapper_BASE(m_aMutex)
            ,OConnectionWrapper()
        {
            setDelegation(_xConnection,m_refCount);
        }
        DECLARE_XINTERFACE();
        DECLARE_XTYPEPROVIDER( );
        // --------------------------------------------------------------------------------
        // XCloseable
        void SAL_CALL close(  ) throw(SQLException, RuntimeException)
        {
            {
                ::osl::MutexGuard aGuard( m_aMutex );
                checkDisposed(rBHelper.bDisposed);

            }
            dispose();
        }

    };

    IMPLEMENT_FORWARD_XINTERFACE2(OConnectionWeakWrapper,OConnectionWeakWrapper_BASE,OConnectionWrapper)
    IMPLEMENT_FORWARD_XTYPEPROVIDER2(OConnectionWeakWrapper,OConnectionWeakWrapper_BASE,OConnectionWrapper)
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

