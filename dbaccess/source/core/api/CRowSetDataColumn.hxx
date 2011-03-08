/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef DBACCESS_CORE_API_CROWSETDATACOLUMN_HXX
#define DBACCESS_CORE_API_CROWSETDATACOLUMN_HXX

#include "datacolumn.hxx"
#include "RowSetRow.hxx"
#include "RowSetCacheIterator.hxx"
#include "columnsettings.hxx"

#include <connectivity/CommonTools.hxx>
#include <comphelper/proparrhlp.hxx>

namespace dbaccess
{
    class ORowSetDataColumn;
    typedef ::comphelper::OPropertyArrayUsageHelper<ORowSetDataColumn> ORowSetDataColumn_PROP;

    class ORowSetDataColumn :   public ODataColumn,
                                public OColumnSettings,
                                public ORowSetDataColumn_PROP
    {
    protected:
        ORowSetCacheIterator        m_aColumnValue;
        ::com::sun::star::uno::Any  m_aOldValue;

        ::rtl::OUString             m_sLabel;
        ::rtl::OUString             m_aDescription;     // description
        ORowSetBase*                m_pRowSet;

        virtual ~ORowSetDataColumn();
    public:
        ORowSetDataColumn(const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XResultSetMetaData >& _xMetaData,
                          const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XRow >& _xRow,
                          const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XRowUpdate >& _xRowUpdate,
                          sal_Int32 _nPos,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMeta,
                          const ::rtl::OUString& _rDescription,
                          const ::rtl::OUString& i_sLabel,
                          const ORowSetCacheIterator& _rColumnValue);


        // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);
        // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        virtual sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue,
                                                            ::com::sun::star::uno::Any & rOldValue,
                                                            sal_Int32 nHandle,
                                                            const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue )throw (::com::sun::star::uno::Exception);

        virtual void fireValueChange(const ::connectivity::ORowSetValue& _rOldValue);
    protected:
        using ODataColumn::getFastPropertyValue;
    };

    typedef connectivity::sdbcx::OCollection ORowSetDataColumns_BASE;
    class ORowSetDataColumns : public ORowSetDataColumns_BASE
    {
        ::rtl::Reference< ::connectivity::OSQLColumns> m_aColumns;
    protected:
        virtual connectivity::sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
    public:
        ORowSetDataColumns(
                        sal_Bool _bCase,
                        const ::rtl::Reference< ::connectivity::OSQLColumns>& _rColumns,
                        ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector
                        );
        virtual ~ORowSetDataColumns();
        // only the name is identical to ::cppu::OComponentHelper
        virtual void SAL_CALL disposing(void);
        void assign(const ::rtl::Reference< ::connectivity::OSQLColumns>& _rColumns,const ::std::vector< ::rtl::OUString> &_rVector);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
