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

        OUString             m_sLabel;
        OUString             m_aDescription;     // description
        ORowSetBase*                m_pRowSet;

        virtual ~ORowSetDataColumn();
    public:
        ORowSetDataColumn(const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XResultSetMetaData >& _xMetaData,
                          const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XRow >& _xRow,
                          const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XRowUpdate >& _xRowUpdate,
                          sal_Int32 _nPos,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMeta,
                          const OUString& _rDescription,
                          const OUString& i_sLabel,
                          const ORowSetCacheIterator& _rColumnValue);


        // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException, std::exception);
        // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        virtual sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue,
                                                            ::com::sun::star::uno::Any & rOldValue,
                                                            sal_Int32 nHandle,
                                                            const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue )throw (::com::sun::star::uno::Exception, std::exception);

        virtual void fireValueChange(const ::connectivity::ORowSetValue& _rOldValue);
    protected:
        using ODataColumn::getFastPropertyValue;
    };

    typedef connectivity::sdbcx::OCollection ORowSetDataColumns_BASE;
    class ORowSetDataColumns : public ORowSetDataColumns_BASE
    {
        ::rtl::Reference< ::connectivity::OSQLColumns> m_aColumns;
    protected:
        virtual connectivity::sdbcx::ObjectType createObject(const OUString& _rName);
        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
    public:
        ORowSetDataColumns(
                        sal_Bool _bCase,
                        const ::rtl::Reference< ::connectivity::OSQLColumns>& _rColumns,
                        ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< OUString> &_rVector
                        );
        virtual ~ORowSetDataColumns();
        // only the name is identical to ::cppu::OComponentHelper
        virtual void SAL_CALL disposing(void);
        void assign(const ::rtl::Reference< ::connectivity::OSQLColumns>& _rColumns,const ::std::vector< OUString> &_rVector);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
