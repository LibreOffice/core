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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_MYSQL_YDRIVER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_MYSQL_YDRIVER_HXX

#include <sal/config.h>

#include <map>

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <connectivity/CommonTools.hxx>


namespace connectivity
{


    class OMetaConnection;

    namespace mysql
    {
        css::uno::Reference< css::uno::XInterface > SAL_CALL ODriverDelegator_CreateInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxMSF) throw( css::uno::Exception );

        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XDriver
                                               ,   css::sdbcx::XDataDefinitionSupplier
                                               ,   css::lang::XServiceInfo
                                               >   ODriverDelegator_BASE;

        typedef ::std::pair< css::uno::WeakReferenceHelper,OMetaConnection*> TWeakConnectionPair;
        typedef ::std::pair< css::uno::WeakReferenceHelper,TWeakConnectionPair> TWeakPair;
        typedef ::std::vector< TWeakPair > TWeakPairVector;
        typedef std::map< OUString, css::uno::Reference< css::sdbc::XDriver > > TJDBCDrivers;

        /** delegates all calls to the orignal driver and extend the existing one with the SDBCX layer.

        */
        class ODriverDelegator : public ::comphelper::OBaseMutex
                                ,public ODriverDelegator_BASE
        {
            TJDBCDrivers                                              m_aJdbcDrivers; // all jdbc drivers
            TWeakPairVector                                           m_aConnections; //  vector containing a list
                                                                                                //  of all the Connection objects
                                                                                                //  for this Driver
            css::uno::Reference< css::sdbc::XDriver >                 m_xODBCDriver;
            css::uno::Reference< css::sdbc::XDriver >                 m_xNativeDriver;
            css::uno::Reference< css::uno::XComponentContext >        m_xContext;

            /** load the driver we want to delegate.
                The <member>m_xODBCDriver</member> or <member>m_xDBCDriver</member> may be <NULL/> if the driver could not be loaded.
                @param  url
                    The URL.
                @param  info
                    The property info contains which driver we have to delegate.
                @return
                    The driver which was currently selected.
            */
            css::uno::Reference< css::sdbc::XDriver > loadDriver( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info );

        public:
            /** creates a new delegator for a mysql driver
            */
            ODriverDelegator(const css::uno::Reference< css::uno::XComponentContext >& _rxContext);

            // XServiceInfo
            DECLARE_SERVICE_INFO();
            static OUString getImplementationName_Static(  ) throw(css::uno::RuntimeException);
            static css::uno::Sequence< OUString > getSupportedServiceNames_Static(  ) throw (css::uno::RuntimeException);

            // XDriver
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw (css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw (css::uno::RuntimeException, std::exception) override;

            // XDataDefinitionSupplier
            virtual css::uno::Reference< css::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByConnection( const css::uno::Reference< css::sdbc::XConnection >& connection ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByURL( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        protected:
            /// dtor
            virtual ~ODriverDelegator() override;
            // OComponentHelper
            virtual void SAL_CALL disposing() override;
        };
    }


}   // namespace connectivity

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_MYSQL_YDRIVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
