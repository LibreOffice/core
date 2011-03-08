/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_SDBCX_CATALOG_HXX_
#define _CONNECTIVITY_SDBCX_CATALOG_HXX_

#include <osl/diagnose.h>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#include <com/sun/star/sdbcx/XGroupsSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <cppuhelper/compbase5.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "connectivity/CommonTools.hxx"
#include "OSubComponent.hxx"
#include "connectivity/sdbcx/IRefreshable.hxx"
#include "connectivity/StdTypeDefs.hxx"
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    namespace sdbcx
    {

        class OCollection;
        // OCatalog is a general catalog class
        // other drivers can be derived their catalog from this class when they want to support sdbcx
        // it holds already tables, views, groups and users

        typedef ::cppu::WeakComponentImplHelper5< ::com::sun::star::sdbcx::XTablesSupplier,
                                                  ::com::sun::star::sdbcx::XViewsSupplier,
                                                  ::com::sun::star::sdbcx::XUsersSupplier,
                                                  ::com::sun::star::sdbcx::XGroupsSupplier,
                                                  ::com::sun::star::lang::XServiceInfo> OCatalog_BASE;


        class OOO_DLLPUBLIC_DBTOOLS SAL_NO_VTABLE OCatalog :
                            public OCatalog_BASE,
                            public IRefreshableGroups,
                            public IRefreshableUsers,
                            public connectivity::OSubComponent<OCatalog, OCatalog_BASE>
        {
            friend class connectivity::OSubComponent<OCatalog, OCatalog_BASE>;
        protected:

            ::osl::Mutex        m_aMutex;

            // this members are deleted when the dtor is called
            // they are hold weak
            OCollection*        m_pTables;
            OCollection*        m_pViews;
            OCollection*        m_pGroups;
            OCollection*        m_pUsers;

            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > m_xMetaData; // just to make things easier

            /** builds the name which should be used to access the object later on in the collection.
                Will only be called in fillNames.
                @param  _xRow
                    The current row from the resultset given to fillNames.
            */
            virtual ::rtl::OUString buildName(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >& _xRow);

            /** fills a vector with the nescessary names which can be used in combination with the collections.
                For each row buildName will be called.
                @param  _xResult
                    The resultset which should be used to fill the names. Will be disposed after return and set to NULL.
                @param  _rNames
                    The vector who will be filled.
            */
            void fillNames(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& _xResult,TStringVector& _rNames);

        public:
            OCatalog(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> &_xConnection);
            virtual ~OCatalog();

            DECLARE_SERVICE_INFO();

            // refreshTables is called when the method getTables had been called
            // the member m_pTables has to be created
            virtual void refreshTables()    = 0;
            // refreshViews is called when the method getViews had been called
            virtual void refreshViews()     = 0;

            // the other refresh methods come from base classes IRefreshableGroups and IRefreshableUsers

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            void SAL_CALL acquire() throw();
            void SAL_CALL release() throw();
            // XTablesSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw(::com::sun::star::uno::RuntimeException);
            // XViewsSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getViews(  ) throw(::com::sun::star::uno::RuntimeException);
            // XUsersSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getUsers(  ) throw(::com::sun::star::uno::RuntimeException);
            // XGroupsSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getGroups(  ) throw(::com::sun::star::uno::RuntimeException);

        };
    }
}

#endif // _CONNECTIVITY_SDBCX_CATALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
