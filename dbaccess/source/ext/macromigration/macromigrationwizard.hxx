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

#include "macromigrationdialog.hxx"

#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <svtools/genericunodialog.hxx>

#include "core_resource.hxx"
#include "strings.hrc"

namespace dbmm
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::sdb::XOfficeDatabaseDocument;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::frame::XStorable;

    // MacroMigrationDialogService
    class MacroMigrationDialogService;
    typedef ::svt::OGenericUnoDialog                                                MacroMigrationDialogService_Base;
    typedef ::comphelper::OPropertyArrayUsageHelper< MacroMigrationDialogService >  MacroMigrationDialogService_PBase;

    class MacroMigrationDialogService
                :public MacroMigrationDialogService_Base
                ,public MacroMigrationDialogService_PBase
    {
    public:
        explicit MacroMigrationDialogService( const Reference< XComponentContext >& _rxContext );

        // XTypeProvider
        virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XPropertySet
        virtual Reference< XPropertySetInfo >  SAL_CALL getPropertySetInfo() override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        // helper for factories
        static Reference< XInterface > SAL_CALL Create( const Reference< XComponentContext >& _rxContext );
        /// @throws RuntimeException
        static OUString SAL_CALL getImplementationName_static();
        /// @throws RuntimeException
        static Sequence< OUString > SAL_CALL getSupportedServiceNames_static();

    protected:
        virtual ~MacroMigrationDialogService() override;

    protected:
        virtual VclPtr<Dialog> createDialog( vcl::Window* _pParent ) override;

    private:
        Reference< XOfficeDatabaseDocument >    m_xDocument;
    };

} // namespace dbmm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
