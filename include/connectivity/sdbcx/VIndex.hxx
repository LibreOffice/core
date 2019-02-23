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

#ifndef INCLUDED_CONNECTIVITY_SDBCX_VINDEX_HXX
#define INCLUDED_CONNECTIVITY_SDBCX_VINDEX_HXX

#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <comphelper/IdPropArrayHelper.hxx>
#include <cppuhelper/basemutex.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/sdbcx/VTypeDef.hxx>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <connectivity/sdbcx/VDescriptor.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <cppuhelper/implbase1.hxx>

namespace connectivity
{
    namespace sdbcx
    {
        class OCollection;
        class OIndex;
        typedef ::cppu::ImplHelper1< css::sdbcx::XDataDescriptorFactory > OIndex_BASE;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OIndex> OIndex_PROP;

        class OOO_DLLPUBLIC_DBTOOLS OIndex :
                                    public cppu::BaseMutex,
                                    public ODescriptor_BASE,
                                    public IRefreshableColumns,
                                    public OIndex_PROP,
                                    public ODescriptor,
                                    public OIndex_BASE
        {
        protected:
            OUString        m_Catalog;
            bool            m_IsUnique;
            bool            m_IsPrimaryKeyIndex;
            bool            m_IsClustered;

            rtl::Reference<OCollection>    m_pColumns;

            using ODescriptor_BASE::rBHelper;
            virtual void refreshColumns() override;
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const override;
            virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
        public:
            OIndex(bool _bCase);
            OIndex( const OUString& Name,
                    const OUString& Catalog,
                    bool _isUnique,
                    bool _isPrimaryKeyIndex,
                    bool _isClustered,
                    bool _bCase);

            virtual ~OIndex( ) override;

            DECLARE_SERVICE_INFO();

            //XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
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
}

#endif // INCLUDED_CONNECTIVITY_SDBCX_VINDEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
