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

#ifndef INCLUDED_CONNECTIVITY_SDBCX_VTABLE_HXX
#define INCLUDED_CONNECTIVITY_SDBCX_VTABLE_HXX

#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <comphelper/IdPropArrayHelper.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase4.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <connectivity/sdbcx/VDescriptor.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>

namespace connectivity
{
    namespace sdbcx
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
                                 public comphelper::OBaseMutex,
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

            OCollection*    m_pKeys;
            OCollection*    m_pColumns;
            OCollection*    m_pIndexes;
            OCollection*    m_pTables;  // must hold his own container to notify him when renaming

            using OTableDescriptor_BASE::rBHelper;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 _nId ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
        public:
            OTable( OCollection*    _pTables,
                    bool _bCase);
            OTable( OCollection*    _pTables,
                    bool _bCase,
                    const OUString& _Name,
                    const OUString& _Type,
                    const OUString& _Description = OUString(),
                    const OUString& _SchemaName  = OUString(),
                    const OUString& _CatalogName = OUString());

            virtual ~OTable();

            DECLARE_SERVICE_INFO();
            //XInterface
            virtual void    SAL_CALL acquire() throw() override;
            virtual void    SAL_CALL release() throw() override;
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;

            // ODescriptor
            virtual void construct() override;
            virtual void refreshColumns() override;
            virtual void refreshKeys();
            virtual void refreshIndexes();
            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
            // XColumnsSupplier
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getColumns(  ) throw(css::uno::RuntimeException, std::exception) override;
            // XKeysSupplier
            virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getKeys(  ) throw(css::uno::RuntimeException, std::exception) override;
            // XNamed
            virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;
            // XDataDescriptorFactory
            virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL createDataDescriptor() throw(css::uno::RuntimeException, std::exception) override;
            // XIndexesSupplier
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getIndexes(  ) throw(css::uno::RuntimeException, std::exception) override;
            // XRename
            virtual void SAL_CALL rename( const OUString& newName ) throw(css::sdbc::SQLException, css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;
            // XAlterTable
            virtual void SAL_CALL alterColumnByName( const OUString& colName, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) throw(css::sdbc::SQLException, css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL alterColumnByIndex( sal_Int32 index, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) throw(css::sdbc::SQLException, css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

            // helper method
            virtual css::uno::Reference< css::sdbc::XDatabaseMetaData> getMetaData() const;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SDBCX_VTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
