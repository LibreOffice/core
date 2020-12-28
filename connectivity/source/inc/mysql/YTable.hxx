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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_MYSQL_YTABLE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_MYSQL_YTABLE_HXX

#include <sal/config.h>

#include <string_view>

#include <connectivity/TTableHelper.hxx>
#include <comphelper/IdPropArrayHelper.hxx>

namespace connectivity::mysql
    {

        class OMySQLTable;
        typedef ::comphelper::OIdPropertyArrayUsageHelper< OMySQLTable >    OMySQLTable_PROP;
        class OMySQLTable : public OTableHelper
                            ,public OMySQLTable_PROP
        {
            sal_Int32 m_nPrivileges; // we have to set our privileges by our own

            /** executes the statement.
                @param  _rStatement
                    The statement to execute.
                */
            void executeStatement(const OUString& _rStatement );
        protected:

            /** creates the column collection for the table
                @param  _rNames
                    The column names.
            */
            virtual sdbcx::OCollection* createColumns(const ::std::vector< OUString>& _rNames) override;

            /** creates the key collection for the table
                @param  _rNames
                    The key names.
            */
            virtual sdbcx::OCollection* createKeys(const ::std::vector< OUString>& _rNames) override;

            /** creates the index collection for the table
                @param  _rNames
                    The index names.
            */
            virtual sdbcx::OCollection* createIndexes(const ::std::vector< OUString>& _rNames) override;

            /** Returns always "RENAME TABLE " even for views.
            *
            * \return The start of the rename statement.
            * @see http://dev.mysql.com/doc/refman/5.1/de/rename-table.html
            */
            virtual OUString getRenameStart() const override;

            /** used to implement the creation of the array helper which is shared amongst all instances of the class.
                This method needs to be implemented in derived classes.
                <BR>
                The method gets called with s_aMutex acquired.
                @return                         a pointer to the newly created array helper. Must not be NULL.
            */
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const override;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

        public:
            OMySQLTable(    sdbcx::OCollection* _pTables,
                            const css::uno::Reference< css::sdbc::XConnection >& _xConnection);
            OMySQLTable(    sdbcx::OCollection* _pTables,
                            const css::uno::Reference< css::sdbc::XConnection >& _xConnection,
                            const OUString& Name,
                            const OUString& Type,
                            const OUString& Description,
                            const OUString& SchemaName,
                            const OUString& CatalogName,
                            sal_Int32 _nPrivileges
                );

            // ODescriptor
            virtual void construct() override;
            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelId();

            // XAlterTable
            virtual void SAL_CALL alterColumnByName( const OUString& colName, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;
            /** returns the ALTER TABLE XXX statement
            */
            OUString getAlterTableColumnPart() const;

            // some methods to alter table structures
            void alterColumnType(sal_Int32 nNewType,const OUString& _rColName,const css::uno::Reference< css::beans::XPropertySet >& _xDescriptor);
            void alterDefaultValue(std::u16string_view _sNewDefault,const OUString& _rColName);
            void dropDefaultValue(const OUString& _sNewDefault);

            virtual OUString getTypeCreatePattern() const override;
        };

}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_MYSQL_YTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
