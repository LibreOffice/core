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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_RESULTCOLUMN_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_RESULTCOLUMN_HXX

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <column.hxx>
#include <o3tl/optional.hxx>
#include <comphelper/proparrhlp.hxx>
namespace dbaccess
{

    //  OResultColumn

    class OResultColumn : public OColumn,
                          public ::comphelper::OPropertyArrayUsageHelper < OResultColumn >
    {
    protected:
        css::uno::Reference < css::sdbc::XResultSetMetaData > m_xMetaData;
        css::uno::Reference< css::sdbc::XDatabaseMetaData >   m_xDBMetaData;
        sal_Int32                             m_nPos;
        css::uno::Any                         m_aIsRowVersion;
        mutable ::o3tl::optional< sal_Bool > m_isSigned;
        mutable ::o3tl::optional< sal_Bool > m_isCurrency;
        mutable ::o3tl::optional< sal_Bool > m_bSearchable;
        mutable ::o3tl::optional< sal_Bool > m_isCaseSensitive;
        mutable ::o3tl::optional< sal_Bool > m_isReadOnly;
        mutable ::o3tl::optional< sal_Bool > m_isWritable;
        mutable ::o3tl::optional< sal_Bool > m_isDefinitelyWritable;
        mutable ::o3tl::optional< sal_Bool > m_isAutoIncrement;
        mutable ::o3tl::optional< sal_Int32 > m_isNullable;
        mutable ::o3tl::optional< OUString > m_sColumnLabel;
        mutable ::o3tl::optional< sal_Int32 > m_nColumnDisplaySize;
        mutable ::o3tl::optional< sal_Int32 > m_nColumnType;
        mutable ::o3tl::optional< sal_Int32 > m_nPrecision;
        mutable ::o3tl::optional< sal_Int32 > m_nScale;

        virtual ~OResultColumn() override;
    public:
        OResultColumn(
            const css::uno::Reference < css::sdbc::XResultSetMetaData >& _xMetaData,
            sal_Int32 _nPos,
            const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _rxDBMeta );

    // css::lang::XTypeProvider
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // css::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // cppu::OComponentHelper
        virtual void SAL_CALL disposing() override;

    // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
        virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;

    private:
        void    impl_determineIsRowVersion_nothrow();

    protected:
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_RESULTCOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
