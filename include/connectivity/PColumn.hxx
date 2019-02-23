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
#ifndef INCLUDED_CONNECTIVITY_PCOLUMN_HXX
#define INCLUDED_CONNECTIVITY_PCOLUMN_HXX

#include <sal/config.h>

#include <map>

#include <connectivity/dbtoolsdllapi.hxx>
#include <connectivity/sdbcx/VColumn.hxx>
#include <connectivity/CommonTools.hxx>
#include <rtl/ref.hxx>
#include <comphelper/proparrhlp.hxx>

namespace com::sun::star::container { class XNameAccess; }
namespace com::sun::star::sdbc { class XDatabaseMetaData; }
namespace com::sun::star::sdbc { class XResultSetMetaData; }

namespace connectivity
{
    namespace parse
    {
        class OParseColumn;

        typedef sdbcx::OColumn OParseColumn_BASE;
        typedef ::comphelper::OPropertyArrayUsageHelper<OParseColumn> OParseColumn_PROP;

        class OOO_DLLPUBLIC_DBTOOLS OParseColumn :
            public OParseColumn_BASE, public OParseColumn_PROP
        {
            OUString    m_aRealName;
            OUString    m_sLabel;
            bool        m_bFunction;
            bool        m_bDbasePrecisionChanged;
            bool        m_bAggregateFunction;
            bool        m_bIsSearchable;

        protected:
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

            virtual ~OParseColumn() override;
        public:
            OParseColumn(const css::uno::Reference< css::beans::XPropertySet>& _xColumn, bool _bCase);
            OParseColumn(const OUString& Name,
                    const OUString& TypeName,
                    const OUString& DefaultValue,
                    const OUString& Description,
                    sal_Int32       IsNullable,
                    sal_Int32       Precision,
                    sal_Int32       Scale,
                    sal_Int32       Type,
                    bool            IsAutoIncrement,
                    bool            IsCurrency,
                    bool            _bCase,
                    const OUString& CatalogName,
                    const OUString& SchemaName,
                    const OUString& TableName);

            virtual void construct() override;

            void setRealName(const OUString& _rName)  { m_aRealName  = _rName; }
            void setLabel(const OUString& i_sLabel)   { m_sLabel  = i_sLabel; }
            void setTableName(const OUString& _rName) { m_TableName = _rName; }
            void setFunction(bool _bFunction)            { m_bFunction  = _bFunction; }
            void setAggregateFunction(bool _bFunction)   { m_bAggregateFunction = _bFunction; }
            void setIsSearchable( bool _bIsSearchable )  { m_bIsSearchable = _bIsSearchable; }
            const OUString& getRealName()   const { return  m_aRealName; }

            /** creates a collection of OParseColumn, as described by a result set meta data instance.
            */
            static ::rtl::Reference< OSQLColumns >
                createColumnsForResultSet(
                    const css::uno::Reference< css::sdbc::XResultSetMetaData >& _rxResMetaData,
                    const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _rxDBMetaData,
                    const css::uno::Reference< css::container::XNameAccess>& i_xQueryColumns
                );

            typedef std::map<OUString, int> StringMap;
            /** creates a single OParseColumn, as described by a result set meta data instance.
                The column names are unique.
            */
            static OParseColumn*
                createColumnForResultSet(
                    const css::uno::Reference< css::sdbc::XResultSetMetaData >& _rxResMetaData,
                    const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _rxDBMetaData,
                    sal_Int32 _nColumnPos,
                    StringMap& _rColumns
                );

        private:
            using OParseColumn_BASE::createArrayHelper;
        };

        class OOrderColumn;

        typedef sdbcx::OColumn OOrderColumn_BASE;
        typedef ::comphelper::OPropertyArrayUsageHelper<OOrderColumn> OOrderColumn_PROP;

        class OOO_DLLPUBLIC_DBTOOLS OOrderColumn :
            public OOrderColumn_BASE, public OOrderColumn_PROP
        {
            const   bool        m_bAscending;

        protected:
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

            virtual ~OOrderColumn() override;
        public:
            OOrderColumn(
                const css::uno::Reference< css::beans::XPropertySet>& _xColumn,
                const OUString& i_rOriginatingTableName,
                bool _bCase,
                bool _bAscending
            );

            OOrderColumn(
                const css::uno::Reference< css::beans::XPropertySet>& _xColumn,
                bool _bCase,
                bool _bAscending
            );

            virtual void construct() override;

            virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
        private:
            using OOrderColumn_BASE::createArrayHelper;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_PCOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
