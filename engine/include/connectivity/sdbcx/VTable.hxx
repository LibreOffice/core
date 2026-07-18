/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#ifndef INCLUDED_CONNECTIVITY_SDBCX_VTABLE_HXX
#define INCLUDED_CONNECTIVITY_SDBCX_VTABLE_HXX

#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/IdPropArrayHelper.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <connectivity/sdbcx/VDescriptor.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/dbtoolsdllapi.hxx>

namespace com::sun::star::sdbc { class XDatabaseMetaData; }

namespace connectivity::sdbcx
    {

        class OTable;
        class OCollection;

        typedef ::cppu::WeakComponentImplHelper<   css::sdbcx::XColumnsSupplier,
                                                   css::sdbcx::XKeysSupplier,
                                                   css::container::XNamed,
                                                   css::lang::XServiceInfo> OTableDescriptor_BASE;

        typedef ::cppu::ImplHelper4<               css::sdbcx::XDataDescriptorFactory,
                                                   css::sdbcx::XIndexesSupplier,
                                                   css::sdbcx::XRename,
                                                   css::sdbcx::XAlterTable > OTable_BASE;

        typedef ::comphelper::OIdPropertyArrayUsageHelper<OTable> OTable_PROP;


        class OOO_DLLPUBLIC_DBTOOLS OTable :
                                 public cppu::BaseMutex,
                                 public OTable_BASE,
                                 public OTableDescriptor_BASE,
                                 public IRefreshableColumns,
                                 public OTable_PROP,
                                 public ODescriptor
        {
        protected:
            OUString m_CatalogName;
            OUString m_SchemaName;
            OUString m_Description;
            OUString m_Type;

            // no Reference! see OCollection::acquire
            std::unique_ptr<OCollection> m_xKeys;
            std::unique_ptr<OCollection> m_xColumns;
            std::unique_ptr<OCollection> m_xIndexes;
            OCollection*    m_pTables;  // must hold his own container to notify him when renaming

            using OTableDescriptor_BASE::rBHelper;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 _nId ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & getInfoHelper() override;
        public:
            OTable( OCollection*    _pTables,
                    bool _bCase);
            OTable( OCollection*    _pTables,
                    bool _bCase,
                    const OUString& Name,
                    OUString Type,
                    OUString Description = OUString(),
                    OUString SchemaName  = OUString(),
                    OUString CatalogName = OUString());

            virtual ~OTable() override;

            DECLARE_SERVICE_INFO();
            //XInterface
            virtual void    acquire() noexcept override;
            virtual void    release() noexcept override;
            virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
            //XTypeProvider
            virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;

            // ODescriptor
            virtual void construct() override;
            virtual void refreshColumns() override;
            virtual void refreshKeys();
            virtual void refreshIndexes();
            // ::cppu::OComponentHelper
            virtual void disposing() override;
            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
            // XColumnsSupplier
            virtual css::uno::Reference< css::container::XNameAccess > getColumns(  ) override;
            // XKeysSupplier
            virtual css::uno::Reference< css::container::XIndexAccess > getKeys(  ) override;
            // XNamed
            virtual OUString getName() override;
            virtual void setName( const OUString& aName ) override;
            // XDataDescriptorFactory
            virtual css::uno::Reference< css::beans::XPropertySet > createDataDescriptor() override;
            // XIndexesSupplier
            virtual css::uno::Reference< css::container::XNameAccess > getIndexes(  ) override;
            // XRename
            virtual void rename( const OUString& newName ) override;
            // XAlterTable
            virtual void alterColumnByName( const OUString& colName, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;
            virtual void alterColumnByIndex( sal_Int32 index, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;

            // helper method
            virtual css::uno::Reference< css::sdbc::XDatabaseMetaData> getMetaData() const;
        };

}

#endif // INCLUDED_CONNECTIVITY_SDBCX_VTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
