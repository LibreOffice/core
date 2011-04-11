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

#ifndef _DBACORE_RESULTCOLUMN_HXX_
#define _DBACORE_RESULTCOLUMN_HXX_

#include <com/sun/star/sdbc/XResultSetMetaData.hdl>
#include <column.hxx>
#include <boost/optional.hpp>
namespace dbaccess
{
    //************************************************************
    //  OResultColumn
    //************************************************************
    class OResultColumn : public OColumn,
                          public ::comphelper::OPropertyArrayUsageHelper < OResultColumn >
    {
    protected:
        ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XResultSetMetaData > m_xMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >   m_xDBMetaData;
        sal_Int32                   m_nPos;
        ::com::sun::star::uno::Any  m_aIsRowVersion;
        mutable ::boost::optional< sal_Bool > m_isSigned;
        mutable ::boost::optional< sal_Bool > m_isCurrency;
        mutable ::boost::optional< sal_Bool > m_bSearchable;
        mutable ::boost::optional< sal_Bool > m_isCaseSensitive;
        mutable ::boost::optional< sal_Bool > m_isReadOnly;
        mutable ::boost::optional< sal_Bool > m_isWritable;
        mutable ::boost::optional< sal_Bool > m_isDefinitelyWritable;
        mutable ::boost::optional< sal_Bool > m_isAutoIncrement;
        mutable ::boost::optional< sal_Int32 > m_isNullable;
        mutable ::boost::optional< ::rtl::OUString > m_sColumnLabel;
        mutable ::boost::optional< sal_Int32 > m_nColumnDisplaySize;
        mutable ::boost::optional< sal_Int32 > m_nColumnType;
        mutable ::boost::optional< sal_Int32 > m_nPrecision;
        mutable ::boost::optional< sal_Int32 > m_nScale;

        virtual ~OResultColumn();
    public:
        OResultColumn(
            const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XResultSetMetaData >& _xMetaData,
            sal_Int32 _nPos,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMeta );

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // cppu::OComponentHelper
        virtual void SAL_CALL disposing(void);

    // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;

    private:
        void    impl_determineIsRowVersion_nothrow();

    protected:
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };
}
#endif // _DBACORE_RESULTCOLUMN_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
