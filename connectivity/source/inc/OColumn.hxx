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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_OCOLUMN_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_OCOLUMN_HXX

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{
    class OColumn
    {
        OUString m_CatalogName;
        OUString m_SchemaName;
        OUString m_TableName;
        OUString m_ColumnName;
        OUString m_ColumnLabel;
        OUString m_ColumnTypeName;
        OUString m_ColumnServiceName;

        sal_Int32       m_Nullable;
        sal_Int32       m_ColumnDisplaySize;
        sal_Int32       m_Precision;
        sal_Int32       m_Scale;
        sal_Int32       m_ColumnType;

        bool        m_AutoIncrement;
        bool        m_CaseSensitive;
        bool        m_Searchable;
        bool        m_Currency;
        bool        m_Signed;
        bool        m_ReadOnly;
        bool        m_Writable;
        bool        m_DefinitelyWritable;

    public:
        OColumn()
            : m_Nullable(0)
            , m_ColumnDisplaySize(0)
            , m_Precision(0)
            , m_Scale(0)
            , m_ColumnType(0)

            , m_AutoIncrement(false)
            , m_CaseSensitive(false)
            , m_Searchable(true)
            , m_Currency(false)
            , m_Signed(false)
            , m_ReadOnly(true)
            , m_Writable(false)
            , m_DefinitelyWritable(false)
            {}

        OColumn(const OUString &_aTableName,
                const OUString &_aColumnName,

                sal_Int32       _aNullable=0,
                sal_Int32       _aColumnDisplaySize=0,
                sal_Int32       _aPrecision=0,
                sal_Int32       _aScale=0,
                sal_Int32       _aColumnType=0)
        :   m_TableName(_aTableName),
            m_ColumnName(_aColumnName),
            m_ColumnLabel(),
            m_ColumnTypeName(),
            m_ColumnServiceName(),

            m_Nullable(_aNullable),
            m_ColumnDisplaySize(_aColumnDisplaySize),
            m_Precision(_aPrecision),
            m_Scale(_aScale),
            m_ColumnType(_aColumnType),

            m_AutoIncrement(false),
            m_CaseSensitive(false),
            m_Searchable(true),
            m_Currency(false),
            m_Signed(false),
            m_ReadOnly(true),
            m_Writable(false),
            m_DefinitelyWritable(false)
        {
            if(m_ColumnLabel.isEmpty())
                m_ColumnLabel = _aColumnName;
        }

        static void * SAL_CALL operator new( size_t nSize )
            { return ::rtl_allocateMemory( nSize ); }
        static void * SAL_CALL operator new( size_t ,void* _pHint )
            { return _pHint; }
        static void SAL_CALL operator delete( void * pMem )
            { ::rtl_freeMemory( pMem ); }
        static void SAL_CALL operator delete( void *,void* )
            {  }

        bool isAutoIncrement()              const { return m_AutoIncrement; }
        bool isCaseSensitive()              const { return m_CaseSensitive; }
        bool isSearchable()                 const { return m_Searchable; }
        bool isCurrency()                   const { return m_Currency; }
        bool isSigned()                     const { return m_Signed; }
        bool isReadOnly()                   const { return m_ReadOnly; }
        bool isWritable()                   const { return m_Writable; }
        bool isDefinitelyWritable()         const { return m_DefinitelyWritable; }

        sal_Int32 isNullable()                  const { return m_Nullable; }
        sal_Int32 getColumnDisplaySize()        const { return m_ColumnDisplaySize; }
        sal_Int32 getPrecision()                const { return m_Precision; }
        sal_Int32 getScale()                    const { return m_Scale; }
        sal_Int32 getColumnType()               const { return m_ColumnType; }

        const OUString& getColumnLabel()         const { return m_ColumnLabel; }
        const OUString& getColumnName()          const { return m_ColumnName; }
        const OUString& getSchemaName()          const { return m_SchemaName; }
        const OUString& getTableName()           const { return m_TableName; }
        const OUString& getCatalogName()         const { return m_CatalogName; }
        const OUString& getColumnTypeName()      const { return m_ColumnTypeName; }
        const OUString& getColumnServiceName()   const { return m_ColumnServiceName; }

    };
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_OCOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
