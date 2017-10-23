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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_ODRIVER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_ODRIVER_HXX

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/compbase.hxx>
#include <connectivity/odbc.hxx>
#include <odbc/odbcbasedllapi.hxx>
#include <connectivity/CommonTools.hxx>
#include <osl/module.h>
#include <odbc/OTools.hxx>

namespace connectivity
{
    namespace odbc
    {
        typedef ::cppu::WeakComponentImplHelper< css::sdbc::XDriver, css::lang::XServiceInfo > ODriver_BASE;

        class OOO_DLLPUBLIC_ODBCBASE SAL_NO_VTABLE ODBCDriver : public ODriver_BASE
        {
        protected:
            ::osl::Mutex                            m_aMutex;

            connectivity::OWeakRefArray             m_xConnections; //  vector containing a list
                                                        //  of all the Connection objects
                                                        //  for this Driver

            css::uno::Reference< css::lang::XMultiServiceFactory > m_xORB;
            SQLHANDLE   m_pDriverHandle;

            virtual SQLHANDLE   EnvironmentHandle(OUString &_rPath) = 0;

        public:

            ODBCDriver(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxFactory);

            // only possibility to get the odbc functions
            virtual oslGenericFunction getOdbcFunction(ODBC3SQLFunctionId _nIndex)  const = 0;
            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            /// @throws css::uno::RuntimeException
            static OUString getImplementationName_Static(  );
            /// @throws css::uno::RuntimeException
            static css::uno::Sequence< OUString > getSupportedServiceNames_Static(  );

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

            const css::uno::Reference< css::lang::XMultiServiceFactory >& getORB() const { return m_xORB; }
        };
    }

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_ODRIVER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
