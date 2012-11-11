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
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#define _CONNECTIVITY_SDBCX_COLUMN_HXX_

#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <comphelper/IdPropArrayHelper.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/implbase1.hxx>
#include "connectivity/CommonTools.hxx"
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/sdbcx/VDescriptor.hxx"
#include "connectivity/dbtoolsdllapi.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity
{
    namespace sdbcx
    {
        class OColumn;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OColumn> OColumn_PROP;

        typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::container::XNamed,
                                                  ::com::sun::star::lang::XServiceInfo> OColumnDescriptor_BASE;
        typedef ::cppu::ImplHelper1< ::com::sun::star::sdbcx::XDataDescriptorFactory > OColumn_BASE;


        class OOO_DLLPUBLIC_DBTOOLS OColumn :
                                    public comphelper::OBaseMutex,
                                    public OColumn_BASE,
                                    public OColumnDescriptor_BASE,
                                    public OColumn_PROP,
                                    public ODescriptor
        {
        protected:
            ::rtl::OUString m_TypeName;
            ::rtl::OUString m_Description;
            ::rtl::OUString m_DefaultValue;

            sal_Int32       m_IsNullable;
            sal_Int32       m_Precision;
            sal_Int32       m_Scale;
            sal_Int32       m_Type;

            sal_Bool        m_IsAutoIncrement;
            sal_Bool        m_IsRowVersion;
            sal_Bool        m_IsCurrency;

            ::rtl::OUString m_CatalogName;
            ::rtl::OUString m_SchemaName;
            ::rtl::OUString m_TableName;

            using OColumnDescriptor_BASE::rBHelper;
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const;
            virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

            virtual ~OColumn();
        public:
            virtual void    SAL_CALL acquire() throw();
            virtual void    SAL_CALL release() throw();

            OColumn(    sal_Bool _bCase);
            OColumn(    const ::rtl::OUString& _Name,
                        const ::rtl::OUString& _TypeName,
                        const ::rtl::OUString& _DefaultValue,
                        const ::rtl::OUString& _Description,
                        sal_Int32       _IsNullable,
                        sal_Int32       _Precision,
                        sal_Int32       _Scale,
                        sal_Int32       _Type,
                        sal_Bool        _IsAutoIncrement,
                        sal_Bool        _IsRowVersion,
                        sal_Bool        _IsCurrency,
                        sal_Bool        _bCase,
                        const ::rtl::OUString& _CatalogName,
                        const ::rtl::OUString& _SchemaName,
                        const ::rtl::OUString& _TableName);

            DECLARE_SERVICE_INFO();
            //XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            // ODescriptor
            virtual void construct();
            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XNamed
            virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
            // XDataDescriptorFactory
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // _CONNECTIVITY_SDBCX_COLUMN_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
