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

#pragma once


#include <comphelper/IdPropArrayHelper.hxx>
#include <connectivity/CommonTools.hxx>
#include <sdbcx/VTypeDef.hxx>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <connectivity/sdbcx/VDescriptor.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <memory>
#include <utility>

namespace connectivity::sdbcx
{

    struct OOO_DLLPUBLIC_DBTOOLS KeyProperties
    {
        ::std::vector< OUString> m_aKeyColumnNames;
        OUString m_ReferencedTable;
        sal_Int32       m_Type;
        sal_Int32       m_UpdateRule;
        sal_Int32       m_DeleteRule;
        KeyProperties(OUString ReferencedTable,
                      sal_Int32     Type,
                      sal_Int32     UpdateRule,
                      sal_Int32     DeleteRule)
                      :m_ReferencedTable(std::move(ReferencedTable)),
                      m_Type(Type),
                      m_UpdateRule(UpdateRule),
                      m_DeleteRule(DeleteRule)
        {}
        KeyProperties():m_Type(0),m_UpdateRule(0),m_DeleteRule(0){}
    };
    typedef ::cppu::ImplHelper1< css::sdbcx::XDataDescriptorFactory > OKey_BASE;
    class OCollection;

    class OOO_DLLPUBLIC_DBTOOLS OKey :
                            public cppu::BaseMutex,
                            public ODescriptor_BASE,
                            public IRefreshableColumns,
                            public ::comphelper::OIdPropertyArrayUsageHelper<OKey>,
                            public ODescriptor,
                            public OKey_BASE
    {
    protected:
        std::shared_ptr<KeyProperties>   m_aProps;
        // no Reference! see OCollection::acquire
        std::unique_ptr<OCollection> m_pColumns;

        using ODescriptor_BASE::rBHelper;
        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const override;
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
    public:
        OKey(bool _bCase);
        OKey(const OUString& Name,std::shared_ptr<KeyProperties> _xProps,bool _bCase);

        virtual ~OKey( ) override;

        DECLARE_SERVICE_INFO();
        //XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
        virtual void SAL_CALL acquire() noexcept override;
        virtual void SAL_CALL release() noexcept override;
        //XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
        // ODescriptor
        virtual void construct() override;

        // ::cppu::OComponentHelper
        virtual void SAL_CALL disposing() override;
        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        // XColumnsSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getColumns(  ) override;

        // XNamed
        virtual OUString SAL_CALL getName(  ) override;
        virtual void SAL_CALL setName( const OUString& aName ) override;
        // XDataDescriptorFactory
        virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) override;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
