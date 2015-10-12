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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_CROWSETDATACOLUMN_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_CROWSETDATACOLUMN_HXX

#include "datacolumn.hxx"
#include "RowSetRow.hxx"
#include "columnsettings.hxx"

#include <connectivity/CommonTools.hxx>
#include <comphelper/proparrhlp.hxx>

#include <functional>

namespace dbaccess
{
    class ORowSetDataColumn;
    typedef ::comphelper::OPropertyArrayUsageHelper<ORowSetDataColumn> ORowSetDataColumn_PROP;

    class ORowSetDataColumn :   public ODataColumn,
                                public OColumnSettings,
                                public ORowSetDataColumn_PROP
    {
    protected:
        const std::function<const ::connectivity::ORowSetValue& (sal_Int32)> m_pGetValue;
        css::uno::Any        m_aOldValue;
        OUString             m_sLabel;
        OUString             m_aDescription;     // description

        virtual ~ORowSetDataColumn();
    public:
        ORowSetDataColumn(const css::uno::Reference < css::sdbc::XResultSetMetaData >& _xMetaData,
                          const css::uno::Reference < css::sdbc::XRow >& _xRow,
                          const css::uno::Reference < css::sdbc::XRowUpdate >& _xRowUpdate,
                          sal_Int32 _nPos,
                          const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _rxDBMeta,
                          const OUString& _rDescription,
                          const OUString& i_sLabel,
                          const std::function<const ::connectivity::ORowSetValue& (sal_Int32)> &_getValue);


        // css::lang::XTypeProvider
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (css::uno::RuntimeException, std::exception) override;
        // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any & rConvertedValue,
                                                            css::uno::Any & rOldValue,
                                                            sal_Int32 nHandle,
                                                            const css::uno::Any& rValue ) throw (css::lang::IllegalArgumentException) override;
        virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const css::uno::Any& rValue )throw (css::uno::Exception, std::exception) override;

        virtual void fireValueChange(const ::connectivity::ORowSetValue& _rOldValue) override;
    protected:
        using ODataColumn::getFastPropertyValue;
    };

    typedef connectivity::sdbcx::OCollection ORowSetDataColumns_BASE;
    class ORowSetDataColumns : public ORowSetDataColumns_BASE
    {
        ::rtl::Reference< ::connectivity::OSQLColumns> m_aColumns;
    protected:
        virtual connectivity::sdbcx::ObjectType createObject(const OUString& _rName) override;
        virtual void impl_refresh() throw(css::uno::RuntimeException) override;
    public:
        ORowSetDataColumns(
                        bool _bCase,
                        const ::rtl::Reference< ::connectivity::OSQLColumns>& _rColumns,
                        ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< OUString> &_rVector
                        );
        virtual ~ORowSetDataColumns();
        // only the name is identical to ::cppu::OComponentHelper
        virtual void SAL_CALL disposing() override;
        void assign(const ::rtl::Reference< ::connectivity::OSQLColumns>& _rColumns,const ::std::vector< OUString> &_rVector);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
