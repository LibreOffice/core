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
#include "precompiled_connectivity.hxx"
#include "ZDriverWrapper.hxx"
#include "ZConnectionPool.hxx"
#include <osl/diagnose.h>

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
