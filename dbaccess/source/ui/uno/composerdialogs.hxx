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

#ifndef DBACCESS_SOURCE_UI_UNO_COMPOSERDIALOGS_HXX
#define DBACCESS_SOURCE_UI_UNO_COMPOSERDIALOGS_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>

#include <svtools/genericunodialog.hxx>
#include "moduledbu.hxx"
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
        OModuleClient m_aModuleClient;
    protected:
        // <properties>
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >
                        m_xComposer;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >
                        m_xRowSet;
        // </properties>

    protected:
        ComposerDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB);
        ~ComposerDialog();

    public:
        DECLARE_IMPLEMENTATION_ID( );

        DECLARE_PROPERTYCONTAINER_DEFAULTS( );

    protected:
        // own overridables
        virtual Dialog* createComposerDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxColumns
        ) = 0;

    private:
        // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);
    };

    // RowsetFilterDialog
    class RowsetFilterDialog : public ComposerDialog
    {
    public:
        RowsetFilterDialog(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB
        );

        DECLARE_SERVICE_INFO_STATIC( );

    protected:
        // own overridables
        virtual Dialog* createComposerDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxColumns
        );

        // OGenericUnoDialog overridables
        virtual void executedDialog( sal_Int16 _nExecutionResult );
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    };

    // RowsetOrderDialog
    class RowsetOrderDialog : public ComposerDialog
    {
    public:
        RowsetOrderDialog(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB
        );

        DECLARE_SERVICE_INFO_STATIC( );

    protected:
        // own overridables
        virtual Dialog* createComposerDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxColumns
        );

        // OGenericUnoDialog overridables
        virtual void executedDialog( sal_Int16 _nExecutionResult );
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments )
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);
    };

}   // namespace dbaui

#endif // DBACCESS_SOURCE_UI_UNO_COMPOSERDIALOGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
