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
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/proparrhlp.hxx>

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
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const SAL_OVERRIDE;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() SAL_OVERRIDE;

            virtual ~OParseColumn();
        public:
            OParseColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn, bool _bCase);
            OParseColumn(const OUString& _Name,
                    const OUString& _TypeName,
                    const OUString& _DefaultValue,
                    const OUString& _Description,
                    sal_Int32       _IsNullable,
                    sal_Int32       _Precision,
                    sal_Int32       _Scale,
                    sal_Int32       _Type,
                    bool            _IsAutoIncrement,
                    bool            _IsCurrency,
                    bool            _bCase,
                    const OUString& _CatalogName,
                    const OUString& _SchemaName,
                    const OUString& _TableName);

            virtual void construct() SAL_OVERRIDE;

            void setRealName(const OUString& _rName)  { m_aRealName  = _rName; }
            void setLabel(const OUString& i_sLabel)   { m_sLabel  = i_sLabel; }
            void setTableName(const OUString& _rName) { m_TableName = _rName; }
            void setFunction(bool _bFunction)            { m_bFunction  = _bFunction; }
            void setAggregateFunction(bool _bFunction)   { m_bAggregateFunction = _bFunction; }
            void setIsSearchable( bool _bIsSearchable )  { m_bIsSearchable = _bIsSearchable; }
            void setDbasePrecisionChanged(bool _bDbasePrecisionChanged) { m_bDbasePrecisionChanged = _bDbasePrecisionChanged; }

            const OUString& getRealName()   const { return  m_aRealName; }
            const OUString& getLabel()      const { return  m_sLabel; }
            const OUString& getTableName()  const { return  m_TableName; }
            bool            getFunction()   const { return  m_bFunction; }
            bool            getDbasePrecisionChanged()  const { return  m_bDbasePrecisionChanged; }

        public:
            /** creates a collection of OParseColumn, as described by a result set meta data instance.
            */
            static ::rtl::Reference< OSQLColumns >
                createColumnsForResultSet(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >& _rxResMetaData,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMetaData,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& i_xQueryColumns
                );

            typedef std::map<OUString, int> StringMap;
            /** creates a single OParseColumn, as described by a result set meta data instance.
                The column names are unique.
            */
            static OParseColumn*
                createColumnForResultSet(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >& _rxResMetaData,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMetaData,
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
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const SAL_OVERRIDE;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() SAL_OVERRIDE;

            virtual ~OOrderColumn();
        public:
            OOrderColumn(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,
                const OUString& i_rOriginatingTableName,
                bool _bCase,
                bool _bAscending
            );

            OOrderColumn(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,
                bool _bCase,
                bool _bAscending
            );

            virtual void construct() SAL_OVERRIDE;

            virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        private:
            using OOrderColumn_BASE::createArrayHelper;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_PCOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
