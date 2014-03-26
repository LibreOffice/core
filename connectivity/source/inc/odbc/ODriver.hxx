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

#ifndef _CONNECTIVITY_ODBC_ODRIVER_HXX_
#define _CONNECTIVITY_ODBC_ODRIVER_HXX_

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase2.hxx>
#include "odbc/OFunctiondefs.hxx"
#include "odbc/odbcbasedllapi.hxx"
#include "connectivity/CommonTools.hxx"
#include <osl/module.h>

namespace connectivity
{
    namespace odbc
    {
        typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::sdbc::XDriver, ::com::sun::star::lang::XServiceInfo > ODriver_BASE;

        class OOO_DLLPUBLIC_ODBCBASE SAL_NO_VTABLE ODBCDriver : public ODriver_BASE
        {
        protected:
            ::osl::Mutex                            m_aMutex;

            connectivity::OWeakRefArray             m_xConnections; //  vector containing a list
                                                        //  of all the Connection objects
                                                        //  for this Driver

            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xORB;
            SQLHANDLE   m_pDriverHandle;

            virtual SQLHANDLE   EnvironmentHandle(OUString &_rPath) = 0;

        public:

            ODBCDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            // only possibility to get the odbc functions
            virtual oslGenericFunction getOdbcFunction(sal_Int32 _nIndex)  const = 0;
            // OComponentHelper
            virtual void SAL_CALL disposing(void) SAL_OVERRIDE;
            // XInterface
            static OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

            // XDriver
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() const { return m_xORB; }
        };
    }

}
#endif //_CONNECTIVITY_ODBC_ODRIVER_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
