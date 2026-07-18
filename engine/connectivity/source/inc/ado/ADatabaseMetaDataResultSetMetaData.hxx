/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <vector>
#include <ado/Awrapado.hxx>
#include <ado/ADatabaseMetaDataResultSet.hxx>
#include <OColumn.hxx>

namespace connectivity::ado
{

    //************ Class: ResultSetMetaData

    typedef ::cppu::WeakImplHelper< css::sdbc::XResultSetMetaData>   ODatabaseMetaResultSetMetaData_BASE;

    class ODatabaseMetaDataResultSetMetaData :  public  ODatabaseMetaResultSetMetaData_BASE
    {
        friend class ODatabaseMetaDataResultSet;

        const std::vector<sal_Int32> &m_vMapping; // when not every column is needed
        std::map<sal_Int32,connectivity::OColumn> m_mColumns;
        std::map<sal_Int32,connectivity::OColumn>::const_iterator m_mColumnsIter;

        ADORecordset*   m_pRecordSet;
        sal_Int32       m_nColCount;

    private:
        ODatabaseMetaDataResultSetMetaData( const ODatabaseMetaDataResultSetMetaData& );            // never implemented
        ODatabaseMetaDataResultSetMetaData& operator=( const ODatabaseMetaDataResultSetMetaData& ); // never implemented

    protected:
        void setColumnPrivilegesMap();
        void setColumnsMap();
        void setTablesMap();
        void setProcedureColumnsMap();
        void setPrimaryKeysMap();
        void setIndexInfoMap();
        void setTablePrivilegesMap();
        void setCrossReferenceMap();
        void setTypeInfoMap();
        void setProceduresMap();
    public:
        // a Constructor, that is needed for when Returning the Object is needed:
        ODatabaseMetaDataResultSetMetaData( ADORecordset* _pRecordSet ,ODatabaseMetaDataResultSet* _pRes)
                :   m_vMapping(_pRes->getColumnMapping()),
                    m_pRecordSet(_pRecordSet),
                    m_nColCount(m_vMapping.size()-1)
        {
            if(m_pRecordSet)
                m_pRecordSet->AddRef();
        }
        ~ODatabaseMetaDataResultSetMetaData() override;

        /// Avoid ambiguous cast error from the compiler.
        operator css::uno::Reference< css::sdbc::XResultSetMetaData > () noexcept
        { return this; }

        virtual sal_Int32 getColumnCount(  ) override;
        virtual bool isAutoIncrement( sal_Int32 column ) override;
        virtual bool isCaseSensitive( sal_Int32 column ) override;
        virtual bool isSearchable( sal_Int32 column ) override;
        virtual bool isCurrency( sal_Int32 column ) override;
        virtual sal_Int32 isNullable( sal_Int32 column ) override;
        virtual bool isSigned( sal_Int32 column ) override;
        virtual sal_Int32 getColumnDisplaySize( sal_Int32 column ) override;
        virtual OUString getColumnLabel( sal_Int32 column ) override;
        virtual OUString getColumnName( sal_Int32 column ) override;
        virtual OUString getSchemaName( sal_Int32 column ) override;
        virtual sal_Int32 getPrecision( sal_Int32 column ) override;
        virtual sal_Int32 getScale( sal_Int32 column ) override;
        virtual OUString getTableName( sal_Int32 column ) override;
        virtual OUString getCatalogName( sal_Int32 column ) override;
        virtual sal_Int32 getColumnType( sal_Int32 column ) override;
        virtual OUString getColumnTypeName( sal_Int32 column ) override;
        virtual bool isReadOnly( sal_Int32 column ) override;
        virtual bool isWritable( sal_Int32 column ) override;
        virtual bool isDefinitelyWritable( sal_Int32 column ) override;
        virtual OUString getColumnServiceName( sal_Int32 column ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
