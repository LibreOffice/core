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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_UNO_COMPOSERDIALOGS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_UNO_COMPOSERDIALOGS_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>

#include <svtools/genericunodialog.hxx>
#include "core_resource.hxx"
#include "apitools.hxx"

namespace dbaui
{

    // ComposerDialog
    class ComposerDialog;
    typedef ::comphelper::OPropertyArrayUsageHelper< ComposerDialog >  ComposerDialog_PBASE;

    class ComposerDialog
            :public svt::OGenericUnoDialog
            ,public ComposerDialog_PBASE
    {
    protected:
        // <properties>
        css::uno::Reference< css::sdb::XSingleSelectQueryComposer >
                        m_xComposer;
        css::uno::Reference< css::sdbc::XRowSet >
                        m_xRowSet;
        // </properties>

    protected:
        explicit ComposerDialog(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);
        virtual ~ComposerDialog() override;

    public:
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override;

        DECLARE_PROPERTYCONTAINER_DEFAULTS( );

    protected:
        // own overridables
        virtual VclPtr<Dialog> createComposerDialog(
            vcl::Window* _pParent,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const css::uno::Reference< css::container::XNameAccess >& _rxColumns
        ) = 0;

    private:
        // OGenericUnoDialog overridables
        virtual VclPtr<Dialog> createDialog(vcl::Window* _pParent) override;
    };

    // RowsetFilterDialog
    class RowsetFilterDialog : public ComposerDialog
    {
    public:
        explicit RowsetFilterDialog(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );

        DECLARE_SERVICE_INFO();
        /// @throws css::uno::RuntimeException
        static OUString SAL_CALL getImplementationName_Static(  );
        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );
        static css::uno::Reference< css::uno::XInterface >
        SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

    protected:
        // own overridables
        virtual VclPtr<Dialog> createComposerDialog(
            vcl::Window* _pParent,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const css::uno::Reference< css::container::XNameAccess >& _rxColumns
        ) override;

        // OGenericUnoDialog overridables
        virtual void executedDialog( sal_Int16 _nExecutionResult ) override;
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    };

    // RowsetOrderDialog
    class RowsetOrderDialog : public ComposerDialog
    {
    public:
        explicit RowsetOrderDialog(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );

        DECLARE_SERVICE_INFO();
        /// @throws css::uno::RuntimeException
        static OUString SAL_CALL getImplementationName_Static(  );
        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );
        static css::uno::Reference< css::uno::XInterface >
        SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

    protected:
        // own overridables
        virtual VclPtr<Dialog> createComposerDialog(
            vcl::Window* _pParent,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const css::uno::Reference< css::container::XNameAccess >& _rxColumns
        ) override;

        // OGenericUnoDialog overridables
        virtual void executedDialog( sal_Int16 _nExecutionResult ) override;
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_UNO_COMPOSERDIALOGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
