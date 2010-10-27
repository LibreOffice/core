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
#ifndef CONNECTIVITY_SRESULSETMETADATA_HXX
#define CONNECTIVITY_SRESULSETMETADATA_HXX

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <cppuhelper/implbase1.hxx>
#include <vector>
#include <rtl/ref.hxx>
#include "MConnection.hxx"
#include "MTable.hxx"

namespace connectivity
{
    namespace mozab
    {
        //**************************************************************
        //************ Class: ResultSetMetaData
        //**************************************************************
        typedef ::cppu::WeakImplHelper1<        ::com::sun::star::sdbc::XResultSetMetaData>   OResultSetMetaData_BASE;

        class OResultSetMetaData :  public  OResultSetMetaData_BASE
        {
            ::rtl::OUString                         m_aTableName;
            ::rtl::Reference<connectivity::OSQLColumns>  m_xColumns;
            OTable*                                 m_pTable;
            sal_Bool                                  m_bReadOnly;

        protected:
            virtual ~OResultSetMetaData();
        public:
            // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
            // OResultSetMetaData(OConnection*  _pConnection) : m_pConnection(_pConnection){}
            OResultSetMetaData(const ::rtl::Reference<connectivity::OSQLColumns>& _rxColumns,
                               const ::rtl::OUString& _aTableName,OTable* _pTable,sal_Bool aReadOnly
                               )
                 :m_aTableName(_aTableName)
                 ,m_xColumns(_rxColumns)
                 ,m_pTable(_pTable)
                 ,m_bReadOnly(aReadOnly)
                 {}


            /// Avoid ambigous cast error from the compiler.
            inline operator ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > () throw()
            { return this; }

            void checkColumnIndex(sal_Int32 column)  throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getColumnCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isAutoIncrement( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isCaseSensitive( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isSearchable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isCurrency( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL isNullable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isSigned( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getColumnDisplaySize( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnLabel( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getSchemaName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getPrecision( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getTableName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getCatalogName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getColumnType( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnTypeName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isReadOnly( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isWritable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isDefinitelyWritable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnServiceName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // CONNECTIVITY_SRESULSETMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
