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

#pragma once

#include "Connection.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <cppuhelper/compbase.hxx>
#include <unotools/tempfile.hxx>

namespace connectivity::firebird
    {
        // The SQL dialect in use
        // Has to be used in various isc_* calls.
        // 3: Is IB6 -- minimum required for delimited identifiers.
        const int FIREBIRD_SQL_DIALECT = 3;

        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XDriver,
                                                   css::sdbcx::XDataDefinitionSupplier,
                                                   css::lang::XServiceInfo > ODriver_BASE;

        class FirebirdDriver : public ODriver_BASE
        {
        private:
            css::uno::Reference<css::uno::XComponentContext> m_aContext;
            ::utl::TempFile m_firebirdTMPDirectory;
            ::utl::TempFile m_firebirdLockDirectory;

        protected:
            ::osl::Mutex                m_aMutex;       // mutex is need to control member access
            OWeakRefArray               m_xConnections; //  vector containing a list
                                                        //  of all the Connection objects
                                                        //  for this Driver

        public:

            explicit FirebirdDriver(const css::uno::Reference< css::uno::XComponentContext >& _rxContext);
            virtual ~FirebirdDriver() override;
            const css::uno::Reference<css::uno::XComponentContext>& getContext() const { return m_aContext; }

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName(  ) override;
            virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
            virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

            // XDriver
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) override;
            virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) override;
            virtual css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) override;
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) override;
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) override;

            // XDataDefinitionSupplier
            virtual css::uno::Reference< css::sdbcx::XTablesSupplier >
                SAL_CALL getDataDefinitionByConnection(
                    const css::uno::Reference< css::sdbc::XConnection >& rxConnection) override;
            virtual css::uno::Reference< css::sdbcx::XTablesSupplier >
                SAL_CALL getDataDefinitionByURL(
                    const OUString& rsURL,
                    const css::uno::Sequence< css::beans::PropertyValue >& rInfo) override;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
