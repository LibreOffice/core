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
#ifndef _CONNECTIVITY_MOZAB_CATALOG_HXX_
#define _CONNECTIVITY_MOZAB_CATALOG_HXX_

#include "connectivity/sdbcx/VCatalog.hxx"

namespace connectivity
{
    namespace mozab
    {
        // please don't name the class the same name as in an other namespaces
        // some compilers have problems with this task as I noticed on windows
        class OConnection;
        class OCatalog : public connectivity::sdbcx::OCatalog
        {
            OConnection*    m_pConnection;      // used to get the metadata
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > m_xMetaData; // just to make things easier

        public:
            // implementation of the pure virtual methods
            virtual void refreshTables();
            virtual void refreshViews() ;
            virtual void refreshGroups();
            virtual void refreshUsers() ;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw(::com::sun::star::uno::RuntimeException);
        public:
            OCatalog(OConnection* _pCon);

            OConnection*            getConnection()     const { return m_pConnection; }
            sdbcx::OCollection*     getPrivateTables()  const { return m_pTables;}
            sdbcx::OCollection*     getPrivateViews()   const { return m_pViews; }

            static const ::rtl::OUString& getDot();
        };
    }
}
#endif // _CONNECTIVITY_MOZAB_CATALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
