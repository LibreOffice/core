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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_DRIVER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_DRIVER_HXX

#include <com/sun/star/sdbc/XDriver.hpp>
#include <java/lang/Object.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/logging.hxx>

namespace connectivity
{
    /// @throws css::uno::Exception
    css::uno::Reference< css::uno::XInterface > java_sql_Driver_CreateInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxFactory);

    class java_sql_Driver : public ::cppu::WeakImplHelper< css::sdbc::XDriver,css::lang::XServiceInfo>
    {
        css::uno::Reference<css::uno::XComponentContext> m_aContext;
        ::comphelper::EventLogger m_aLogger;

    protected:
        virtual ~java_sql_Driver() override;

    public:
        java_sql_Driver(const css::uno::Reference< css::uno::XComponentContext >& _rxContext);

        /// @throws css::uno::RuntimeException
        static OUString getImplementationName_Static(  );
        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static(  );

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XDriver
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) override ;
        virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) override ;
        virtual css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) override ;
        virtual sal_Int32 SAL_CALL getMajorVersion(  ) override ;
        virtual sal_Int32 SAL_CALL getMinorVersion(  ) override;

        const css::uno::Reference<css::uno::XComponentContext>& getContext() const { return m_aContext; }
        const ::comphelper::EventLogger&   getLogger() const { return m_aLogger; }
    };

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_SQL_DRIVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
