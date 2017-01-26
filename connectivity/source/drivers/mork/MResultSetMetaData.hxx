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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MRESULTSETMETADATA_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MRESULTSETMETADATA_HXX

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <cppuhelper/implbase.hxx>
#include <vector>
#include <rtl/ref.hxx>
#include "MConnection.hxx"
#include "MTable.hxx"

namespace connectivity
{
    namespace mork
    {

        //************ Class: ResultSetMetaData

        typedef ::cppu::WeakImplHelper<css::sdbc::XResultSetMetaData>   OResultSetMetaData_BASE;

        class OResultSetMetaData :  public  OResultSetMetaData_BASE
        {
            OUString                         m_aTableName;
            ::rtl::Reference<connectivity::OSQLColumns>  m_xColumns;
            OTable*                                 m_pTable;
            bool                                  m_bReadOnly;

        protected:
            virtual ~OResultSetMetaData() override;
        public:
            // a constructor that is needed to return the object:
            // OResultSetMetaData(OConnection*  _pConnection) : m_pConnection(_pConnection){}
            OResultSetMetaData(const ::rtl::Reference<connectivity::OSQLColumns>& _rxColumns,
                               const OUString& _aTableName,OTable* _pTable,bool aReadOnly
                               )
                 :m_aTableName(_aTableName)
                 ,m_xColumns(_rxColumns)
                 ,m_pTable(_pTable)
                 ,m_bReadOnly(aReadOnly)
                 {}

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void checkColumnIndex(sal_Int32 column);
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
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MRESULTSETMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
