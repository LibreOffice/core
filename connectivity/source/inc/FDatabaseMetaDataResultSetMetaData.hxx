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
#include <map>
#include <vector>
#include "OColumn.hxx"

namespace connectivity
{

    //************ Class: ODatabaseMetaDataResultSetMetaData

    typedef ::cppu::WeakImplHelper< css::sdbc::XResultSetMetaData>   ODatabaseMetaResultSetMetaData_BASE;

    class ODatabaseMetaDataResultSetMetaData :    public  ODatabaseMetaResultSetMetaData_BASE
    {
        std::map<sal_Int32,connectivity::OColumn> m_mColumns;
        std::map<sal_Int32,connectivity::OColumn>::const_iterator m_mColumnsIter;

    protected:
        virtual ~ODatabaseMetaDataResultSetMetaData() override;
    public:
        // a Constructor, that is needed for when returning the object is needed:
        ODatabaseMetaDataResultSetMetaData( )
        {
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

        // methods to set the right column mapping
        void setColumnPrivilegesMap();
        void setColumnMap();
        void setColumnsMap();
        void setTableNameMap();
        void setTablesMap();
        void setProcedureColumnsMap();
        void setPrimaryKeysMap();
        void setIndexInfoMap();
        void setTablePrivilegesMap();
        void setCrossReferenceMap();
        void setTypeInfoMap();
        void setProcedureNameMap();
        void setProceduresMap();
        void setTableTypes();
        void setBestRowIdentifierMap() { setVersionColumnsMap();}
        void setVersionColumnsMap();
        void setExportedKeysMap() { setCrossReferenceMap(); }
        void setImportedKeysMap() { setCrossReferenceMap(); }
        void setCatalogsMap();
        void setSchemasMap();
        void setUDTsMap();
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
