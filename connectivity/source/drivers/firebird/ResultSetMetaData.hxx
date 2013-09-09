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

#ifndef CONNECTIVITY_FIREBIRD_RESULTSETMETADATA_HXX
#define CONNECTIVITY_FIREBIRD_RESULTSETMETADATA_HXX

#include "Connection.hxx"

#include <ibase.h>

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>

namespace connectivity
{
    namespace firebird
    {
        typedef ::cppu::WeakImplHelper1< ::com::sun::star::sdbc::XResultSetMetaData>
                OResultSetMetaData_BASE;

        class OResultSetMetaData :  public  OResultSetMetaData_BASE
        {
        protected:
            Connection*     m_pConnection;
            XSQLDA*         m_pSqlda;

            virtual ~OResultSetMetaData();

            void verifyValidColumn(sal_Int32 column) throw(::com::sun::star::sdbc::SQLException);
        public:
            // a constructor, which is required for returning objects:
            OResultSetMetaData(Connection* pConnection,
                               XSQLDA* pSqlda)
                : m_pConnection(pConnection)
                , m_pSqlda(pSqlda)
            {}

            /// Avoid ambigous cast error from the compiler.
            inline operator ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > () throw()
            { return this; }

            virtual sal_Int32 SAL_CALL getColumnCount()
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isAutoIncrement(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isCaseSensitive(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isSearchable(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isCurrency(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL isNullable(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isSigned(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getColumnDisplaySize(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnLabel(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnName(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getSchemaName(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getPrecision(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getScale(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getTableName(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getCatalogName(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getColumnType(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnTypeName(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isReadOnly(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isWritable(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isDefinitelyWritable(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnServiceName(sal_Int32 column)
                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // CONNECTIVITY_FIREBIRD_RESULTSETMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
