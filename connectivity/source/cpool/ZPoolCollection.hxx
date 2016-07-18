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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZPOOLCOLLECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZPOOLCOLLECTION_HXX

#include <sal/config.h>

#include <map>

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbc/XDriverManager2.hpp>
#include <com/sun/star/sdbc/XConnectionPool.hpp>
#include <com/sun/star/sdbc/XPooledConnection.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <comphelper/stl_types.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>

namespace connectivity
{
    class OConnectionPool;

    // OPoolCollection - the one-instance service for PooledConnections
    // manages the active connections and the connections in the pool

    typedef ::cppu::WeakImplHelper<    css::sdbc::XConnectionPool,
                                       css::lang::XServiceInfo,
                                       css::frame::XTerminateListener,
                                       css::beans::XPropertyChangeListener
                                       >   OPoolCollection_Base;

    /// OPoolCollection: control the whole connection pooling for oo
    class OPoolCollection : public OPoolCollection_Base
    {


        typedef ::comphelper::OInterfaceCompare< css::sdbc::XDriver >  ODriverCompare;
        typedef std::map<OUString, rtl::Reference<OConnectionPool>> OConnectionPools;

        typedef std::map<
                css::uno::Reference< css::sdbc::XDriver >,
                css::uno::WeakReference< css::sdbc::XDriver >,
                ODriverCompare>
                MapDriver2DriverRef;

        MapDriver2DriverRef                                       m_aDriverProxies;
        ::osl::Mutex                                              m_aMutex;
        OConnectionPools                                          m_aPools;          // the driver pools
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        css::uno::Reference< css::sdbc::XDriverManager2 >         m_xManager;
        css::uno::Reference< css::reflection::XProxyFactory >     m_xProxyFactory;
        css::uno::Reference< css::uno::XInterface >               m_xConfigNode;      // config node for general connection pooling
        css::uno::Reference< css::frame::XDesktop2>               m_xDesktop;

    private:
        OPoolCollection(const OPoolCollection&) = delete;
        int operator= (const OPoolCollection&) = delete;

        explicit OPoolCollection(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext);

        // some configuration helper methods
        css::uno::Reference< css::uno::XInterface > createWithServiceFactory(const OUString& _rPath) const;
        css::uno::Reference< css::uno::XInterface > const & getConfigPoolRoot();
        static css::uno::Reference< css::uno::XInterface > createWithProvider(   const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxConfProvider,
                                                                                                    const OUString& _rPath);
        static css::uno::Reference< css::uno::XInterface > openNode( const OUString& _rPath,
                                                                                        const css::uno::Reference< css::uno::XInterface >& _xTreeNode) throw();
        bool isPoolingEnabled();
        bool isDriverPoolingEnabled(const OUString& _sDriverImplName,
                                        css::uno::Reference< css::uno::XInterface >& _rxDriverNode);
        bool isPoolingEnabledByUrl( const OUString& _sUrl,
                                        css::uno::Reference< css::sdbc::XDriver >& _rxDriver,
                                        OUString& _rsImplName,
                                        css::uno::Reference< css::uno::XInterface >& _rxDriverNode);

        OConnectionPool* getConnectionPool( const OUString& _sImplName,
                                            const css::uno::Reference< css::sdbc::XDriver >& _xDriver,
                                            const css::uno::Reference< css::uno::XInterface >& _rxDriverNode);
        void clearConnectionPools(bool _bDispose);
        void clearDesktop();
    protected:
        virtual ~OPoolCollection();
    public:

        static css::uno::Any getNodeValue( const OUString& _rPath,
                                                        const css::uno::Reference< css::uno::XInterface>& _xTreeNode)throw();

    // XDriverManager
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection( const OUString& url ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnectionWithInfo( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setLoginTimeout( sal_Int32 seconds ) throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getLoginTimeout(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XDriverAccess
        virtual css::uno::Reference< css::sdbc::XDriver > SAL_CALL getDriverByURL( const OUString& url ) throw (css::uno::RuntimeException, std::exception) override;
    // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo - static methods
        static css::uno::Reference< css::uno::XInterface > SAL_CALL CreateInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >&);
        static OUString SAL_CALL getImplementationName_Static(  ) throw(css::uno::RuntimeException);
        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw(css::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) throw (css::uno::RuntimeException, std::exception) override;

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const css::lang::EventObject& Event ) throw (css::frame::TerminationVetoException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL notifyTermination( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;
    };
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZPOOLCOLLECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
