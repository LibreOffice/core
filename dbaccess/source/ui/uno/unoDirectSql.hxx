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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_UNO_UNODIRECTSQL_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_UNO_UNODIRECTSQL_HXX

#include <svtools/genericunodialog.hxx>
#include "apitools.hxx"
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include "core_resource.hxx"
#include <com/sun/star/sdbc/XConnection.hpp>

namespace dbaui
{

    // ODirectSQLDialog
    class ODirectSQLDialog;
    typedef ::svt::OGenericUnoDialog                                        ODirectSQLDialog_BASE;
    typedef ::comphelper::OPropertyArrayUsageHelper< ODirectSQLDialog >     ODirectSQLDialog_PBASE;

    class ODirectSQLDialog
            :public ODirectSQLDialog_BASE
            ,public ODirectSQLDialog_PBASE
    {
        dbaccess::OModuleClient m_aModuleClient;
        OUString      m_sInitialSelection;
        css::uno::Reference< css::sdbc::XConnection > m_xActiveConnection;
    protected:
        explicit ODirectSQLDialog(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);
        virtual ~ODirectSQLDialog() override;

    public:
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override;

        DECLARE_SERVICE_INFO();
        /// @throws css::uno::RuntimeException
        static OUString SAL_CALL getImplementationName_Static(  );
        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );
        static css::uno::Reference< css::uno::XInterface >
        SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

        DECLARE_PROPERTYCONTAINER_DEFAULTS( );

    protected:
        // OGenericUnoDialog overridables
        virtual VclPtr<Dialog> createDialog(vcl::Window* _pParent) override;
        virtual void implInitialize(const css::uno::Any& _rValue) override;
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_UNO_UNODIRECTSQL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
