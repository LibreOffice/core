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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_TABLE_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_TABLE_HXX

#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include "apitools.hxx"
#include "datasettings.hxx"
#include <column.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/TTableHelper.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/IdPropArrayHelper.hxx>

namespace dbaccess
{

    // OTables
    class ODBTable;
    class OContainerMediator;
    typedef ::comphelper::OIdPropertyArrayUsageHelper< ODBTable >   ODBTable_PROP;
    typedef ::connectivity::OTableHelper                            OTable_Base;

    class ODBTable  :public ODataSettings_Base
                    ,public ODBTable_PROP
                    ,public OTable_Base
                    ,public IColumnFactory
    {
    private:
        ::rtl::Reference< OContainerMediator >                m_pColumnMediator;

    protected:
        css::uno::Reference< css::container::XNameAccess >    m_xColumnDefinitions;
        css::uno::Reference< css::container::XNameAccess >    m_xDriverColumns;

    // <properties>
        sal_Int32                                             m_nPrivileges;
    // </properties>

        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const override;
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

        // IColumnFactory
        virtual OColumn*    createColumn(const OUString& _rName) const override;
        virtual css::uno::Reference< css::beans::XPropertySet > createColumnDescriptor() override;
        virtual void columnAppended( const css::uno::Reference< css::beans::XPropertySet >& _rxSourceDescriptor ) override;
        virtual void columnDropped(const OUString& _sName) override;

        /** creates the column collection for the table
            @param  _rNames
                The column names.
        */
        virtual ::connectivity::sdbcx::OCollection* createColumns(const ::connectivity::TStringVector& _rNames) override;

        /** creates the key collection for the table
            @param  _rNames
                The key names.
        */
        virtual ::connectivity::sdbcx::OCollection* createKeys(const ::connectivity::TStringVector& _rNames) override;

        /** creates the index collection for the table
            @param  _rNames
                The index names.
        */
        virtual ::connectivity::sdbcx::OCollection* createIndexes(const ::connectivity::TStringVector& _rNames) override;

        // OComponentHelper
        virtual void SAL_CALL disposing() override;
    public:
        /** constructs a wrapper supporting the com.sun.star.sdb.Table service.<BR>
            @param          _rxConn         the connection the table belongs to
            @param          _rxTable        the table from the driver can be null
            @param          _rCatalog       the name of the catalog the table belongs to. May be empty.
            @param          _rSchema        the name of the schema the table belongs to. May be empty.
            @param          _rName          the name of the table
            @param          _rType          the type of the table, as supplied by the driver
            @param          _rDesc          the description of the table, as supplied by the driver
        */
        ODBTable(connectivity::sdbcx::OCollection* _pTables
                ,const css::uno::Reference< css::sdbc::XConnection >& _rxConn
                ,const OUString& _rCatalog
                , const OUString& _rSchema
                , const OUString& _rName
                ,const OUString& _rType
                , const OUString& _rDesc
                ,const css::uno::Reference< css::container::XNameAccess >& _rxColumnDefinitions)
            throw(css::sdbc::SQLException);

        ODBTable(connectivity::sdbcx::OCollection* _pTables
                ,const css::uno::Reference< css::sdbc::XConnection >& _rxConn)
                throw(css::sdbc::SQLException);
        virtual ~ODBTable();

        // ODescriptor
        virtual void construct() override;

        //XInterface
        DECLARE_XINTERFACE()
        //XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (css::uno::RuntimeException, std::exception) override;
        static css::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

    // css::lang::XServiceInfo
        DECLARE_SERVICE_INFO();

    // css::beans::XPropertySet
        virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle) const override;

    // css::sdbcx::XRename,
        virtual void SAL_CALL rename( const OUString& _rNewName ) throw(css::sdbc::SQLException, css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;

    // css::sdbcx::XAlterTable,
        virtual void SAL_CALL alterColumnByName( const OUString& _rName, const css::uno::Reference< css::beans::XPropertySet >& _rxDescriptor ) throw(css::sdbc::SQLException, css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;

        // css::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    private:
        using OTable_Base::createArrayHelper;
        using OTable_Base::getFastPropertyValue;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_TABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
