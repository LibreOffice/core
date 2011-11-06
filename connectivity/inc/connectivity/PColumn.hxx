/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _CONNECTIVITY_PCOLUMN_HXX_
#define _CONNECTIVITY_PCOLUMN_HXX_

#include "connectivity/dbtoolsdllapi.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include "connectivity/CommonTools.hxx"
#include <vos/ref.hxx>
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
            ::rtl::OUString m_aRealName;
            ::rtl::OUString m_aTableName;
            ::rtl::OUString m_sLabel;
            sal_Bool        m_bFunction;
            sal_Bool        m_bDbasePrecisionChanged;
            sal_Bool        m_bAggregateFunction;
            sal_Bool        m_bIsSearchable;

        protected:
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

            virtual ~OParseColumn();
        public:
            OParseColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,sal_Bool _bCase);
            OParseColumn(const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _TypeName,
                    const ::rtl::OUString& _DefaultValue,
                    const ::rtl::OUString& _Description,
                    sal_Int32       _IsNullable,
                    sal_Int32       _Precision,
                    sal_Int32       _Scale,
                    sal_Int32       _Type,
                    sal_Bool        _IsAutoIncrement,
                    sal_Bool        _IsCurrency,
                    sal_Bool        _bCase);

            virtual void construct();

            void setRealName(const ::rtl::OUString& _rName)  { m_aRealName  = _rName; }
            void setLabel(const ::rtl::OUString& i_sLabel)   { m_sLabel  = i_sLabel; }
            void setTableName(const ::rtl::OUString& _rName) { m_aTableName = _rName; }
            void setFunction(sal_Bool _bFunction)            { m_bFunction  = _bFunction; }
            void setAggregateFunction(sal_Bool _bFunction)   { m_bAggregateFunction = _bFunction; }
            void setIsSearchable( sal_Bool _bIsSearchable )  { m_bIsSearchable = _bIsSearchable; }
            void setDbasePrecisionChanged(sal_Bool _bDbasePrecisionChanged) { m_bDbasePrecisionChanged = _bDbasePrecisionChanged; }

            ::rtl::OUString getRealName()   const { return  m_aRealName; }
            ::rtl::OUString getLabel()      const { return  m_sLabel; }
            ::rtl::OUString getTableName()  const { return  m_aTableName; }
            sal_Bool        getFunction()   const { return  m_bFunction; }
            sal_Bool        getDbasePrecisionChanged()  const { return  m_bDbasePrecisionChanged; }

        public:
            /** creates a collection of OParseColumn, as described by a result set meta data instance.
            */
            static ::vos::ORef< OSQLColumns >
                createColumnsForResultSet(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >& _rxResMetaData,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMetaData,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& i_xQueryColumns
                );

            DECLARE_STL_USTRINGACCESS_MAP(int,StringMap);
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
            const   sal_Bool        m_bAscending;
            const   ::rtl::OUString m_sTableName;

        protected:
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

            virtual ~OOrderColumn();
        public:
            OOrderColumn(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,
                const ::rtl::OUString& i_rOriginatingTableName,
                sal_Bool _bCase,
                sal_Bool _bAscending
            );

            OOrderColumn(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,
                sal_Bool _bCase,
                sal_Bool _bAscending
            );

            virtual void construct();

            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
        private:
            using OOrderColumn_BASE::createArrayHelper;
        };
    }
}

#endif //_CONNECTIVITY_PCOLUMN_HXX_

