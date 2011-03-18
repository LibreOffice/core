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

#ifndef _CONNECTIVITY_MACAB_CATALOG_HXX_
#define _CONNECTIVITY_MACAB_CATALOG_HXX_

#include "connectivity/sdbcx/VCatalog.hxx"

namespace connectivity
{
    namespace macab
    {
            class MacabConnection;

        class MacabCatalog : public connectivity::sdbcx::OCatalog
        {
            MacabConnection* m_pConnection;     // used to get the metadata
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > m_xMetaData; // just to make things easier

        public:
            MacabCatalog(MacabConnection* _pCon);

            inline MacabConnection* getConnection() const { return m_pConnection; }

            static const ::rtl::OUString& getDot();

            // implementation of the pure virtual methods
            virtual void refreshTables();
            virtual void refreshViews();
            virtual void refreshGroups();
            virtual void refreshUsers();

            // XTablesSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(
                    ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // _CONNECTIVITY_MACAB_CATALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
