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

#pragma once

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <cppuhelper/implbase.hxx>
#include <odbc/OFunctions.hxx>
#include <odbc/odbcbasedllapi.hxx>
#include <vector>
#include <odbc/OConnection.hxx>

namespace connectivity::odbc
    {

        //************ Class: ResultSetMetaData

        typedef ::cppu::WeakImplHelper<        css::sdbc::XResultSetMetaData>   OResultSetMetaData_BASE;

        class OOO_DLLPUBLIC_ODBCBASE OResultSetMetaData final :
            public  OResultSetMetaData_BASE
        {
            std::vector<sal_Int32> m_vMapping; // when not every column is needed
            std::map<sal_Int32,sal_Int32> m_aColumnTypes;

            SQLHANDLE       m_aStatementHandle;
            OConnection*    m_pConnection;
            sal_Int32       m_nColCount;
            bool        m_bUseODBC2Types;

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            OUString getCharColAttrib(sal_Int32 column,sal_Int32 ident);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            sal_Int32 getNumColAttrib(sal_Int32 column,sal_Int32 ident);
        public:
            // A ctor that is needed for returning the object
            OResultSetMetaData(OConnection* _pConnection, SQLHANDLE _pStmt )
                :m_aStatementHandle( _pStmt )
                ,m_pConnection(_pConnection)
                ,m_nColCount(-1)
                ,m_bUseODBC2Types(false)
            {}
            OResultSetMetaData(OConnection* _pConnection, SQLHANDLE _pStmt ,const std::vector<sal_Int32> & _vMapping)
                    :m_vMapping(_vMapping)
                    ,m_aStatementHandle( _pStmt )
                    ,m_pConnection(_pConnection)
                    ,m_nColCount(_vMapping.size()-1)
                    ,m_bUseODBC2Types(false)
            {}
            virtual ~OResultSetMetaData() override;


            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static SQLLEN getNumColAttrib(OConnection const * _pConnection
                                              ,SQLHANDLE _aStatementHandle
                                              ,const css::uno::Reference< css::uno::XInterface >& _xInterface
                                              ,sal_Int32 _column
                                              ,sal_Int32 ident);

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static SQLSMALLINT getColumnODBCType(OConnection const * _pConnection
                                              ,SQLHANDLE _aStatementHandle
                                              ,const css::uno::Reference< css::uno::XInterface >& _xInterface
                                              ,sal_Int32 column);

            oslGenericFunction getOdbcFunction(ODBC3SQLFunctionId _nIndex)  const
            {
                return m_pConnection->getOdbcFunction(_nIndex);
            }

            virtual sal_Int32 SAL_CALL getColumnCount(  ) override;
            virtual sal_Bool SAL_CALL isAutoIncrement( sal_Int32 column ) override;
            virtual sal_Bool SAL_CALL isCaseSensitive( sal_Int32 column ) override;
            virtual sal_Bool SAL_CALL isSearchable( sal_Int32 column ) override;
            virtual sal_Bool SAL_CALL isCurrency( sal_Int32 column ) override;
            virtual sal_Int32 SAL_CALL isNullable( sal_Int32 column ) override;
            virtual sal_Bool SAL_CALL isSigned( sal_Int32 column ) override;
            virtual sal_Int32 SAL_CALL getColumnDisplaySize( sal_Int32 column ) override;
            virtual OUString SAL_CALL getColumnLabel( sal_Int32 column ) override;
            virtual OUString SAL_CALL getColumnName( sal_Int32 column ) override;
            virtual OUString SAL_CALL getSchemaName( sal_Int32 column ) override;
            virtual sal_Int32 SAL_CALL getPrecision( sal_Int32 column ) override;
            virtual sal_Int32 SAL_CALL getScale( sal_Int32 column ) override;
            virtual OUString SAL_CALL getTableName( sal_Int32 column ) override;
            virtual OUString SAL_CALL getCatalogName( sal_Int32 column ) override;
            virtual sal_Int32 SAL_CALL getColumnType( sal_Int32 column ) override;
            virtual OUString SAL_CALL getColumnTypeName( sal_Int32 column ) override;
            virtual sal_Bool SAL_CALL isReadOnly( sal_Int32 column ) override;
            virtual sal_Bool SAL_CALL isWritable( sal_Int32 column ) override;
            virtual sal_Bool SAL_CALL isDefinitelyWritable( sal_Int32 column ) override;
            virtual OUString SAL_CALL getColumnServiceName( sal_Int32 column ) override;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
