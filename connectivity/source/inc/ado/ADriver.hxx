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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_ADRIVER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_ADRIVER_HXX

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase.hxx>
#include <connectivity/CommonTools.hxx>

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
}}}}

namespace connectivity
{
    namespace ado
    {

        css::uno::Reference< css::uno::XInterface >  SAL_CALL ODriver_CreateInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxFactory) throw( css::uno::Exception );

        typedef ::cppu::WeakComponentImplHelper< css::sdbc::XDriver,
                                                 css::sdbcx::XDataDefinitionSupplier,
                                                 css::lang::XServiceInfo
                                               > ODriver_BASE;
        class ODriver : public ODriver_BASE
        {
            ::osl::Mutex                            m_aMutex;

            connectivity::OWeakRefArray             m_xConnections; //  vector containing a list
                                                        //  of all the Connection objects
                                                        //  for this Driver
            css::uno::Reference< css::lang::XMultiServiceFactory > m_xORB;


        public:
            ODriver(const css::uno::Reference< css::lang::XMultiServiceFactory >& _xORB);
            ~ODriver();

            // OComponentHelper
            virtual void SAL_CALL disposing();
            // XInterface
            static OUString getImplementationName_Static(  ) throw(css::uno::RuntimeException);
            static css::uno::Sequence< OUString > getSupportedServiceNames_Static(  ) throw (css::uno::RuntimeException);
            css::uno::Reference< css::lang::XMultiServiceFactory > getORB() const { return m_xORB; }

        private:
            void impl_checkURL_throw(const OUString& _sUrl);

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException);
            virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException);

            // XDriver
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
            virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
            virtual css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw(css::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw(css::uno::RuntimeException);

            // XDataDefinitionSupplier
            virtual css::uno::Reference< css::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByConnection( const css::uno::Reference< css::sdbc::XConnection >& connection ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
            virtual css::uno::Reference< css::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByURL( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) throw(css::sdbc::SQLException, css::uno::RuntimeException);
        };
    }

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_ADRIVER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
