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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_MANAGER_MDRIVERMANAGER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_MANAGER_MDRIVERMANAGER_HXX

#include <sal/config.h>

#include <map>
#include <vector>

#include <com/sun/star/sdbc/XDriverManager2.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/logging.hxx>
#include <osl/mutex.hxx>
#include <connectivity/DriversConfig.hxx>

namespace drivermanager
{

    typedef std::map< OUString, css::uno::Reference< css::sdbc::XDriver > > DriverCollection;

    struct DriverAccess
    {
        OUString           sImplementationName;        /// the implementation name of the driver
        css::uno::Reference< css::sdbc::XDriver >                  xDriver;                    /// the driver itself
        css::uno::Reference< css::lang::XSingleComponentFactory >  xComponentFactory;          /// the factory to create the driver component (if not already done so)
    };


    // OSDBCDriverManager - the one-instance service for managing SDBC drivers

    typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XDriverManager2
                                  ,   css::lang::XServiceInfo
                                  ,   css::uno::XNamingService
                                  >   OSDBCDriverManager_Base;

    class OSDBCDriverManager final : public cppu::BaseMutex, public OSDBCDriverManager_Base
    {
        friend class ODriverEnumeration;

        css::uno::Reference<css::uno::XComponentContext>  m_xContext;
        ::comphelper::EventLogger       m_aEventLogger;

        typedef std::vector<DriverAccess> DriverAccessArray;
        DriverAccessArray               m_aDriversBS;

        // for drivers registered at runtime (not bootstrapped) we don't require an XServiceInfo interface,
        // so we have to remember their impl-name in another way
        typedef std::map< OUString, css::uno::Reference< css::sdbc::XDriver > > DriverCollection;
        DriverCollection                m_aDriversRT;

        ::connectivity::DriversConfig   m_aDriverConfig;
        sal_Int32                       m_nLoginTimeout;

    public:

        explicit OSDBCDriverManager(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext );
        virtual ~OSDBCDriverManager() override;

    // XDriverManager
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection( const OUString& url ) override;
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnectionWithInfo( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) override;
        virtual void SAL_CALL setLoginTimeout( sal_Int32 seconds ) override;
        virtual sal_Int32 SAL_CALL getLoginTimeout(  ) override;

    // XDriverAccess
        virtual css::uno::Reference< css::sdbc::XDriver > SAL_CALL getDriverByURL( const OUString& url ) override;

    // XEnumerationAccess
        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) override;

    // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType(  ) override;
        virtual sal_Bool SAL_CALL hasElements(  ) override;

    // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XNamingService
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getRegisteredObject( const OUString& Name ) override;
        virtual void SAL_CALL registerObject( const OUString& Name, const css::uno::Reference< css::uno::XInterface >& Object ) override;
        virtual void SAL_CALL revokeObject( const OUString& Name ) override;

    private:
        css::uno::Reference< css::sdbc::XDriver > implGetDriverForURL(const OUString& _rURL);

        /** retrieve the driver order preferences from the configuration and
            sort m_aDriversBS accordingly.
        */
        void initializeDriverPrecedence();

        void bootstrapDrivers();
    };

}   // namespace drivermanager

#endif // INCLUDED_CONNECTIVITY_SOURCE_MANAGER_MDRIVERMANAGER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
