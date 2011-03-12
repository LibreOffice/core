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
#ifndef CONNECTIVITY_ADABAS_RESULTSETMETADATA_HXX
#define CONNECTIVITY_ADABAS_RESULTSETMETADATA_HXX

#include "odbc/OResultSetMetaData.hxx"
#include <rtl/ref.hxx>

namespace connectivity
{
    namespace adabas
    {
        //**************************************************************
        //************ Class: ResultSetMetaData
        //**************************************************************

        typedef odbc::OResultSetMetaData OAdabasResultSetMetaData_BASE;
        class OAdabasResultSetMetaData :    public  OAdabasResultSetMetaData_BASE
        {
            ::rtl::Reference<OSQLColumns>   m_aSelectColumns;
        public:
            // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
            OAdabasResultSetMetaData(odbc::OConnection* _pConnection, SQLHANDLE _pStmt ,const ::rtl::Reference<OSQLColumns>& _rSelectColumns);
            virtual ~OAdabasResultSetMetaData();

            virtual sal_Int32 SAL_CALL getColumnType( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL isNullable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isAutoIncrement( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // CONNECTIVITY_ADABAS_RESULTSETMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
