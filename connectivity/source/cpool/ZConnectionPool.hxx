/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZCONNECTIONPOOL_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZCONNECTIONPOOL_HXX

#include <sal/config.h>

#include <map>
#include <vector>

#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/sdbc/XPooledConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <salhelper/timer.hxx>
#include <rtl/ref.hxx>
#include <rtl/digest.h>

namespace connectivity
{
    class OConnectionPool;

    /// OPoolTimer - Invalidates the connection pool

    class OPoolTimer : public ::salhelper::Timer
    {
        OConnectionPool* m_pPool;
    public:
        OPoolTimer(OConnectionPool* _pPool,const ::salhelper::TTimeValue& Time)
            : ::salhelper::Timer(Time)
            ,m_pPool(_pPool)
        {}
    protected:
        virtual void SAL_CALL onShot() override;
    };


    // OConnectionPool - the one-instance service for PooledConnections
    // manages the active connections and the connections in the pool

    typedef ::cppu::WeakImplHelper< css::beans::XPropertyChangeListener>  OConnectionPool_Base;

    // typedef for the internal structure
    typedef std::vector< css::uno::Reference< css::sdbc::XPooledConnection> > TPooledConnections;

     // contains the currently pooled connections
    typedef struct
    {
        TPooledConnections  aConnections;
        sal_Int32           nALiveCount; // will be decremented every time a time says to, when will reach zero the pool will be deleted
    } TConnectionPool;

    struct TDigestHolder
    {
        sal_uInt8 m_pBuffer[RTL_DIGEST_LENGTH_SHA1];
        TDigestHolder()
        {
            m_pBuffer[0] = 0;
        }

    };

    //  typedef TDigestHolder

    struct TDigestLess
    {
        bool operator() (const TDigestHolder& x, const TDigestHolder& y) const
        {
            sal_uInt32 i;
            for(i=0;i < RTL_DIGEST_LENGTH_SHA1 && (x.m_pBuffer[i] >= y.m_pBuffer[i]); ++i)
                ;
            return i < RTL_DIGEST_LENGTH_SHA1;
        }
    };

    typedef std::map< TDigestHolder,TConnectionPool,TDigestLess> TConnectionMap;

    // contains additional information about a activeconnection which is needed to put it back to the pool
    typedef struct
    {
        TConnectionMap::iterator aPos;
        css::uno::Reference< css::sdbc::XPooledConnection> xPooledConnection;
    } TActiveConnectionInfo;

    typedef std::map< css::uno::Reference< css::sdbc::XConnection>,
                        TActiveConnectionInfo> TActiveConnectionMap;

    class OConnectionPool : public OConnectionPool_Base
    {
        TConnectionMap          m_aPool;                // the pooled connections
        TActiveConnectionMap    m_aActiveConnections;   // the currently active connections

        ::osl::Mutex            m_aMutex;
        ::rtl::Reference<OPoolTimer>    m_xInvalidator;         // invalidates the conntection pool when shot

        css::uno::Reference< css::sdbc::XDriver >             m_xDriver;      // the one and only driver for this connectionpool
        css::uno::Reference< css::uno::XInterface >           m_xDriverNode;  // config node entry
        css::uno::Reference< css::reflection::XProxyFactory > m_xProxyFactory;
        sal_Int32               m_nTimeOut;
        sal_Int32               m_nALiveCount;

    private:
        css::uno::Reference< css::sdbc::XConnection> createNewConnection(const OUString& _rURL,
                                const css::uno::Sequence< css::beans::PropertyValue >& _rInfo);
        css::uno::Reference< css::sdbc::XConnection> getPooledConnection(TConnectionMap::iterator& _rIter);
        // calculate the timeout and the corresponding ALiveCount
        void calculateTimeOuts();

    protected:
        // the dtor will be called from the last instance  (last release call)
        virtual ~OConnectionPool() override;
    public:
        OConnectionPool(const css::uno::Reference< css::sdbc::XDriver >& _xDriver,
                        const css::uno::Reference< css::uno::XInterface >& _xDriverNode,
                        const css::uno::Reference< css::reflection::XProxyFactory >& _rxProxyFactory);

        // delete all refs
        void clear(bool _bDispose);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnectionWithInfo( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info );
        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) override;

        void invalidatePooledConnections();
    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZCONNECTIONPOOL_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
