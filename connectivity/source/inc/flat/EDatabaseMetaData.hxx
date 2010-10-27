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

#ifndef _CONNECTIVITY_FLAT_EDATABASEMETADATA_HXX_
#define _CONNECTIVITY_FLAT_EDATABASEMETADATA_HXX_

#include "file/FDatabaseMetaData.hxx"

namespace connectivity
{
    namespace flat
    {
        //**************************************************************
        //************ Class: java.sql.DatabaseMetaDataDate
        //**************************************************************

        class OFlatDatabaseMetaData :   public file::ODatabaseMetaData
        {
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > impl_getTypeInfo_throw();
        protected:
            virtual ~OFlatDatabaseMetaData();
        public:
            OFlatDatabaseMetaData(file::OConnection* _pCon);

            virtual ::rtl::OUString SAL_CALL getURL(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getColumns( const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern, const ::rtl::OUString& columnNamePattern ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // _CONNECTIVITY_FLAT_ODATABASEMETADATA_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
