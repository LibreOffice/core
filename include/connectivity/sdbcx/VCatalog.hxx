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

#ifndef INCLUDED_CONNECTIVITY_SDBCX_VCATALOG_HXX
#define INCLUDED_CONNECTIVITY_SDBCX_VCATALOG_HXX

#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#include <com/sun/star/sdbcx/XGroupsSupplier.hpp>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <connectivity/CommonTools.hxx>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <memory>

namespace com::sun::star::sdbc { class XConnection; }
namespace com::sun::star::sdbc { class XDatabaseMetaData; }
namespace com::sun::star::sdbc { class XResultSet; }
namespace com::sun::star::sdbc { class XRow; }

namespace connectivity
{
    namespace sdbcx
    {

        class OCollection;
        // OCatalog is a general catalog class
        // other drivers can be derived their catalog from this class when they want to support sdbcx
        // it holds already tables, views, groups and users

        typedef ::cppu::WeakComponentImplHelper< css::sdbcx::XTablesSupplier,
                                                 css::sdbcx::XViewsSupplier,
                                                 css::sdbcx::XUsersSupplier,
                                                 css::sdbcx::XGroupsSupplier,
                                                 css::lang::XServiceInfo> OCatalog_BASE;


        class OOO_DLLPUBLIC_DBTOOLS SAL_NO_VTABLE OCatalog :
                            public OCatalog_BASE,
                            public IRefreshableGroups,
                            public IRefreshableUsers
        {
        protected:

            ::osl::Mutex        m_aMutex;

            // this members are deleted when the dtor is called
            // they are hold weak
            std::unique_ptr<OCollection> m_pTables;
            std::unique_ptr<OCollection> m_pViews;
            std::unique_ptr<OCollection> m_pGroups;
            std::unique_ptr<OCollection> m_pUsers;

            css::uno::Reference< css::sdbc::XDatabaseMetaData > m_xMetaData; // just to make things easier

            /** builds the name which should be used to access the object later on in the collection.
                Will only be called in fillNames.
                @param  _xRow
                    The current row from the resultset given to fillNames.
            */
            virtual OUString buildName(  const css::uno::Reference< css::sdbc::XRow >& _xRow);

            /** fills a vector with the necessary names which can be used in combination with the collections.
                For each row buildName will be called.
                @param  _xResult
                    The resultset which should be used to fill the names. Will be disposed after return and set to NULL.
                @param  _rNames
                    The vector who will be filled.
            */
            void fillNames(css::uno::Reference< css::sdbc::XResultSet >& _xResult,::std::vector< OUString>& _rNames);

        public:
            OCatalog(const css::uno::Reference< css::sdbc::XConnection> &_xConnection);
            virtual ~OCatalog() override;

            DECLARE_SERVICE_INFO();

            // refreshTables is called when the method getTables had been called
            // the member m_pTables has to be created
            virtual void refreshTables()    = 0;
            // refreshViews is called when the method getViews had been called
            virtual void refreshViews()     = 0;

            // the other refresh methods come from base classes IRefreshableGroups and IRefreshableUsers

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XTablesSupplier
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTables(  ) override;
            // XViewsSupplier
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getViews(  ) override;
            // XUsersSupplier
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getUsers(  ) override;
            // XGroupsSupplier
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getGroups(  ) override;

        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SDBCX_VCATALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
