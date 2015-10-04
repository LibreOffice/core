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
#ifndef INCLUDED_CONNECTIVITY_SDBCX_VCOLUMN_HXX
#define INCLUDED_CONNECTIVITY_SDBCX_VCOLUMN_HXX

#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <comphelper/IdPropArrayHelper.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <connectivity/CommonTools.hxx>
#include <cppuhelper/basemutex.hxx>
#include <connectivity/sdbcx/VDescriptor.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity
{
    namespace sdbcx
    {
        class OColumn;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OColumn> OColumn_PROP;

        typedef ::cppu::WeakComponentImplHelper< css::container::XNamed,
                                                 css::lang::XServiceInfo> OColumnDescriptor_BASE;
        typedef ::cppu::ImplHelper< css::sdbcx::XDataDescriptorFactory > OColumn_BASE;


        class OOO_DLLPUBLIC_DBTOOLS OColumn :
                                    public cppu::BaseMutex,
                                    public OColumn_BASE,
                                    public OColumnDescriptor_BASE,
                                    public OColumn_PROP,
                                    public ODescriptor
        {
        protected:
            OUString        m_TypeName;
            OUString        m_Description;
            OUString        m_DefaultValue;

            sal_Int32       m_IsNullable;
            sal_Int32       m_Precision;
            sal_Int32       m_Scale;
            sal_Int32       m_Type;

            bool            m_IsAutoIncrement;
            bool            m_IsRowVersion;
            bool            m_IsCurrency;

            OUString       m_CatalogName;
            OUString       m_SchemaName;
            OUString       m_TableName;

            using OColumnDescriptor_BASE::rBHelper;
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const override;
            virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

            virtual ~OColumn() override;
        public:
            virtual void    SAL_CALL acquire() throw() override;
            virtual void    SAL_CALL release() throw() override;

            OColumn(    bool _bCase);
            OColumn(    const OUString& Name,
                        const OUString& TypeName,
                        const OUString& DefaultValue,
                        const OUString& Description,
                        sal_Int32       IsNullable,
                        sal_Int32       Precision,
                        sal_Int32       Scale,
                        sal_Int32       Type,
                        bool            IsAutoIncrement,
                        bool            IsRowVersion,
                        bool            IsCurrency,
                        bool            _bCase,
                        const OUString& CatalogName,
                        const OUString& SchemaName,
                        const OUString& TableName);

            DECLARE_SERVICE_INFO();
            //XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
            // ODescriptor
            virtual void construct() override;
            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
            // XNamed
            virtual OUString SAL_CALL getName(  ) throw(css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;
            // XDataDescriptorFactory
            virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(css::uno::RuntimeException, std::exception) override;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SDBCX_VCOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
