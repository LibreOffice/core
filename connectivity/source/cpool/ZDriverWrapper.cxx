/*************************************************************************
 *
 *  $RCSfile: ZDriverWrapper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:38:16 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_CPOOL_ZDRIVERWRAPPER_HXX_
#include "ZDriverWrapper.hxx"
#endif
#ifndef _CONNECTIVITY_ZCONNECTIONPOOL_HXX_
#include "ZConnectionPool.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//........................................................................
namespace connectivity
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= ODriverWrapper
    //====================================================================
    //--------------------------------------------------------------------
    ODriverWrapper::ODriverWrapper( Reference< XAggregation >& _rxAggregateDriver, OConnectionPool* _pPool )
        :m_pConnectionPool(_pPool)
    {
        OSL_ENSURE(_rxAggregateDriver.is(), "ODriverWrapper::ODriverWrapper: invalid aggregate!");
        OSL_ENSURE(m_pConnectionPool, "ODriverWrapper::ODriverWrapper: invalid connection pool!");

        if (m_pConnectionPool)
            m_pConnectionPool->acquire();

        osl_incrementInterlockedCount( &m_refCount );
        if (_rxAggregateDriver.is())
        {
            // transfer the (one and only) real ref to the aggregate to our member
            m_xDriverAggregate = _rxAggregateDriver;
            _rxAggregateDriver = NULL;

            // a second "real" reference
            m_xDriver = Reference< XDriver >(m_xDriverAggregate, UNO_QUERY);
            OSL_ENSURE(m_xDriver.is(), "ODriverWrapper::ODriverWrapper: invalid aggregate (no XDriver)!");

            // set ourself as delegator
            m_xDriverAggregate->setDelegator( static_cast< XWeak* >( this ) );
        }
        osl_decrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    ODriverWrapper::~ODriverWrapper()
    {
        if (m_xDriverAggregate.is())
            m_xDriverAggregate->setDelegator(NULL);

        if (m_pConnectionPool)
            m_pConnectionPool->release();
        m_pConnectionPool = NULL;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL ODriverWrapper::queryInterface( const Type& _rType ) throw (RuntimeException)
    {
        Any aReturn = ODriverWrapper_BASE::queryInterface(_rType);
        return aReturn.hasValue() ? aReturn : (m_xDriverAggregate.is() ? m_xDriverAggregate->queryAggregation(_rType) : aReturn);
    }

    //--------------------------------------------------------------------
    Reference< XConnection > SAL_CALL ODriverWrapper::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw (SQLException, RuntimeException)
    {
        Reference< XConnection > xConnection;
        if (m_pConnectionPool)
            // route this through the pool
            xConnection = m_pConnectionPool->getConnectionWithInfo( url, info );
        else if (m_xDriver.is())
            xConnection = m_xDriver->connect( url, info );

        return xConnection;
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL ODriverWrapper::acceptsURL( const ::rtl::OUString& url ) throw (SQLException, RuntimeException)
    {
        return m_xDriver.is() && m_xDriver->acceptsURL(url);
    }

    //--------------------------------------------------------------------
    Sequence< DriverPropertyInfo > SAL_CALL ODriverWrapper::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw (SQLException, RuntimeException)
    {
        Sequence< DriverPropertyInfo > aInfo;
        if (m_xDriver.is())
            aInfo = m_xDriver->getPropertyInfo(url, info);
        return aInfo;
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL ODriverWrapper::getMajorVersion(  ) throw (RuntimeException)
    {
        return m_xDriver.is() ? m_xDriver->getMajorVersion() : 0;
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL ODriverWrapper::getMinorVersion(  ) throw (RuntimeException)
    {
        return m_xDriver.is() ? m_xDriver->getMinorVersion() : 0;
    }

//........................................................................
}   // namespace connectivity
//........................................................................


