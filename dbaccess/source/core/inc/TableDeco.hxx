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

#include <memory>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <cppuhelper/compbase.hxx>
#include "datasettings.hxx"
#include "column.hxx"

#include <connectivity/CommonTools.hxx>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <comphelper/IdPropArrayHelper.hxx>

namespace dbaccess
{
    typedef ::cppu::WeakComponentImplHelper<   css::sdbcx::XColumnsSupplier,
                                               css::sdbcx::XKeysSupplier,
                                               css::container::XNamed,
                                               css::lang::XServiceInfo,
                                               css::sdbcx::XDataDescriptorFactory,
                                               css::sdbcx::XIndexesSupplier,
                                               css::sdbcx::XRename,
                                               css::lang::XUnoTunnel,
                                               css::sdbcx::XAlterTable> OTableDescriptor_BASE;
    // OTables
    class ODBTableDecorator;
    typedef ::comphelper::OIdPropertyArrayUsageHelper< ODBTableDecorator >  ODBTableDecorator_PROP;

    class ODBTableDecorator :public cppu::BaseMutex
                            ,public OTableDescriptor_BASE
                            ,public ODataSettings //ODataSettings_Base
                            ,public IColumnFactory
                            ,public ::connectivity::sdbcx::IRefreshableColumns
                            ,public ODBTableDecorator_PROP
    {
        void fillPrivileges() const;
    protected:
        css::uno::Reference< css::container::XContainerListener > m_xColumnMediator;
        css::uno::Reference< css::sdbcx::XColumnsSupplier >       m_xTable;
        css::uno::Reference< css::container::XNameAccess >        m_xColumnDefinitions;
        css::uno::Reference< css::sdbc::XConnection >             m_xConnection;
        css::uno::Reference< css::sdbc::XDatabaseMetaData >       m_xMetaData;
        css::uno::Reference< css::util::XNumberFormatsSupplier >  m_xNumberFormats;

        // <properties>
        mutable sal_Int32                                         m_nPrivileges;
        // </properties>
        // note: this thing uses the ref-count of "this", see OCollection::acquire()!
        std::unique_ptr<::connectivity::sdbcx::OCollection>       m_pColumns;

        // IColumnFactory
        virtual rtl::Reference<OColumn> createColumn(const OUString& _rName) const override;
        virtual css::uno::Reference< css::beans::XPropertySet > createColumnDescriptor() override;
        virtual void columnAppended( const css::uno::Reference< css::beans::XPropertySet >& _rxSourceDescriptor ) override;
        virtual void columnDropped(const OUString& _sName) override;

        virtual void refreshColumns() override;

        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 _nId) const override;
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

        // OPropertySetHelper
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                            css::uno::Any & rConvertedValue,
                            css::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const css::uno::Any& rValue ) override;
        virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle) const override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const css::uno::Any& rValue
                                                 ) override;

        virtual ~ODBTableDecorator() override;
    public:
        /** constructs a wrapper supporting the com.sun.star.sdb.Table service.

            @param _rxConn
                the connection the table belongs to. Must not be <NULL/>
            @param _rxTable
                the table from the driver can be <NULL/>
            @throws css::sdbc::SQLException
        */
        ODBTableDecorator(
            const css::uno::Reference< css::sdbc::XConnection >& _rxConn,
            const css::uno::Reference< css::sdbcx::XColumnsSupplier >& _rxTable,
            const css::uno::Reference< css::util::XNumberFormatsSupplier >& _rxNumberFormats,
            const css::uno::Reference< css::container::XNameAccess >& _rxColumnDefinitions
        );

        // ODescriptor
        void construct();

        //XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
        virtual void SAL_CALL acquire() throw() override;
        virtual void SAL_CALL release() throw() override;
        //XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // OComponentHelper
        virtual void SAL_CALL disposing() override;

    // css::lang::XServiceInfo
        DECLARE_SERVICE_INFO();
        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    // css::sdbcx::XRename,
        virtual void SAL_CALL rename( const OUString& _rNewName ) override;

    // css::sdbcx::XAlterTable,
        virtual void SAL_CALL alterColumnByName( const OUString& _rName, const css::uno::Reference< css::beans::XPropertySet >& _rxDescriptor ) override;
        virtual void SAL_CALL alterColumnByIndex( sal_Int32 _nIndex, const css::uno::Reference< css::beans::XPropertySet >& _rxDescriptor ) override;

        // XNamed
        virtual OUString SAL_CALL getName() override;
        virtual void SAL_CALL setName( const OUString& aName ) override;
        // css::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
        static css::uno::Sequence< sal_Int8 > getUnoTunnelId();

        // XColumnsSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getColumns(  ) override;
        // XKeysSupplier
        virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getKeys(  ) override;
        // XIndexesSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getIndexes(  ) override;
        // XDataDescriptorFactory
        virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) override;

    protected:
        using ODataSettings::getFastPropertyValue;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
