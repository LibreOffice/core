/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PColumn.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:15:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_PCOLUMN_HXX_
#define _CONNECTIVITY_PCOLUMN_HXX_

#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif

#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif

#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif

namespace connectivity
{
    namespace parse
    {
        class OParseColumn;

        typedef sdbcx::OColumn OParseColumn_BASE;
        typedef ::comphelper::OPropertyArrayUsageHelper<OParseColumn> OParseColumn_PROP;

        class OParseColumn :    public OParseColumn_BASE,
                                public OParseColumn_PROP
        {
            ::rtl::OUString m_aRealName;
            ::rtl::OUString m_aTableName;
            sal_Bool        m_bFunction;
            sal_Bool        m_bDbasePrecisionChanged;
            sal_Bool        m_bAggregateFunction;
        protected:
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

            virtual ~OParseColumn();
        public:
            OParseColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,sal_Bool _bCase);
            OParseColumn(const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _TypeName,
                    const ::rtl::OUString& _DefaultValue,
                    sal_Int32       _IsNullable,
                    sal_Int32       _Precision,
                    sal_Int32       _Scale,
                    sal_Int32       _Type,
                    sal_Bool        _IsAutoIncrement,
                    sal_Bool        _IsCurrency,
                    sal_Bool        _bCase);

            virtual void construct();

            void setRealName(const ::rtl::OUString& _rName)  { m_aRealName  = _rName; }
            void setTableName(const ::rtl::OUString& _rName) { m_aTableName = _rName; }
            void setFunction(sal_Bool _bFunction)            { m_bFunction  = _bFunction; }
            void setAggregateFunction(sal_Bool _bFunction)   { m_bAggregateFunction = _bFunction; }
            void setDbasePrecisionChanged(sal_Bool _bDbasePrecisionChanged) { m_bDbasePrecisionChanged = _bDbasePrecisionChanged; }

            ::rtl::OUString getRealName() const { return  m_aRealName; }
            ::rtl::OUString getTableName() const { return  m_aTableName; }
            sal_Bool        getFunction() const { return  m_bFunction; }
            sal_Bool        getDbasePrecisionChanged()  const { return  m_bDbasePrecisionChanged; }

        public:
            /** creates a collection of OParseColumn, as described by a result set meta data instance.
            */
            static ::vos::ORef< OSQLColumns >
                createColumnsForResultSet(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >& _rxResMetaData,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMetaData
                );

            /** creates a single OParseColumn, as described by a result set meta data instance
            */
            static OParseColumn*
                createColumnForResultSet(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData >& _rxResMetaData,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMetaData,
                    sal_Int32 _nColumnPos
                );

        private:
            using OParseColumn_BASE::createArrayHelper;
        };

        class OOrderColumn;

        typedef sdbcx::OColumn OOrderColumn_BASE;
        typedef ::comphelper::OPropertyArrayUsageHelper<OOrderColumn> OOrderColumn_PROP;

        class OOrderColumn :    public OOrderColumn_BASE,
                                public OOrderColumn_PROP
        {
            sal_Bool        m_bAscending;
            sal_Bool        m_bOrder;
        protected:
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

            virtual ~OOrderColumn();
        public:
            OOrderColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,sal_Bool _bCase,sal_Bool _bAscending);
            OOrderColumn(const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _TypeName,
                    const ::rtl::OUString& _DefaultValue,
                    sal_Int32       _IsNullable,
                    sal_Int32       _Precision,
                    sal_Int32       _Scale,
                    sal_Int32       _Type,
                    sal_Bool        _IsAutoIncrement,
                    sal_Bool        _IsCurrency,
                    sal_Bool        _bCase
                    ,sal_Bool _bAscending);

            virtual void construct();

            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
        private:
            using OOrderColumn_BASE::createArrayHelper;
        };
    }
}

#endif //_CONNECTIVITY_PCOLUMN_HXX_

