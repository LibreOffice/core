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
#ifndef CONNECTIVITY_MYSQL_CATALOG_HXX
#define CONNECTIVITY_MYSQL_CATALOG_HXX

#include "connectivity/sdbcx/VCatalog.hxx"
#include "connectivity/StdTypeDefs.hxx"

namespace connectivity
{
    namespace mysql
    {
        // please don't name the class the same name as in an other namespaces
        // some compilers have problems with this task as I noticed on windows
        class OAdabasConnection;
        class OMySQLCatalog : public connectivity::sdbcx::OCatalog
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;

            /** calls XDatabaseMetaData::getTables.
                @param  _sKindOfObject
                    The type of tables to be fetched.
                @param  _rNames
                    The container for the names to be filled. <OUT/>
            */
            void refreshObjects(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _sKindOfObject,TStringVector& _rNames);

        public:
            // implementation of the pure virtual methods
            virtual void refreshTables();
            virtual void refreshViews() ;
            virtual void refreshGroups();
            virtual void refreshUsers() ;

        public:
            OMySQLCatalog(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);

            inline sdbcx::OCollection*      getPrivateTables()  const { return m_pTables;}
            inline sdbcx::OCollection*      getPrivateViews()   const { return m_pViews; }
            inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection() const { return m_xConnection; }

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            // ::cppu::OComponentHelper
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // CONNECTIVITY_MYSQL_CATALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
