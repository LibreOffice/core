/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <sal/config.h>

#include <map>
#include <vector>

#include <com/sun/star/sdbc/XDriverManager2.hpp>
#include <cpo/uno/XNamingService.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/logging.hxx>
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
                                  ,   cpo::uno::XNamingService
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
        virtual css::uno::Reference< css::sdbc::XConnection > getConnection( const OUString& url ) override;
        virtual css::uno::Reference< css::sdbc::XConnection > getConnectionWithInfo( const OUString& url, const cpo::uno::Sequence< css::beans::PropertyValue >& info ) override;
        virtual void setLoginTimeout( sal_Int32 seconds ) override;
        virtual sal_Int32 getLoginTimeout(  ) override;

    // XDriverAccess
        virtual css::uno::Reference< css::sdbc::XDriver > getDriverByURL( const OUString& url ) override;

    // XEnumerationAccess
        virtual css::uno::Reference< css::container::XEnumeration > createEnumeration(  ) override;

    // XElementAccess
        virtual cpo::uno::Type getElementType(  ) override;
        virtual bool hasElements(  ) override;

    // XServiceInfo
        virtual OUString getImplementationName(  ) override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // XNamingService
        virtual css::uno::Reference< css::uno::XInterface > getRegisteredObject( const OUString& Name ) override;
        virtual void registerObject( const OUString& Name, const css::uno::Reference< css::uno::XInterface >& Object ) override;
        virtual void revokeObject( const OUString& Name ) override;

    private:
        css::uno::Reference< css::sdbc::XDriver > implGetDriverForURL(const OUString& _rURL);

        /** retrieve the driver order preferences from the configuration and
            sort m_aDriversBS accordingly.
        */
        void initializeDriverPrecedence();

        void bootstrapDrivers();
    };

}   // namespace drivermanager


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
