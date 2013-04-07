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

#ifndef _CONNECTIVITY_COLUMN_HXX_
#define _CONNECTIVITY_COLUMN_HXX_

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS OColumn
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

        sal_Bool        m_AutoIncrement;
        sal_Bool        m_CaseSensitive;
        sal_Bool        m_Searchable;
        sal_Bool        m_Currency;
        sal_Bool        m_Signed;
        sal_Bool        m_ReadOnly;
        sal_Bool        m_Writable;
        sal_Bool        m_DefinitelyWritable;

    public:
        OColumn() {}
        OColumn(const OUString &_aTableName,
                const OUString &_aColumnName,

                sal_Int32       _aNullable=0,
                sal_Int32       _aColumnDisplaySize=0,
                sal_Int32       _aPrecision=0,
                sal_Int32       _aScale=0,
                sal_Int32       _aColumnType=0,

                sal_Bool        _aAutoIncrement=sal_False,
                sal_Bool        _aCaseSensitive=sal_False,
                sal_Bool        _aSearchable=sal_True,
                sal_Bool        _aCurrency=sal_False,
                sal_Bool        _aSigned=sal_False,
                sal_Bool        _aReadOnly=sal_True,
                sal_Bool        _aWritable=sal_False,
                sal_Bool        _aDefinitelyWritable=sal_False,

                const OUString &_aColumnLabel = OUString(),
                const OUString &_aColumnTypeName = OUString(),
                const OUString &_aColumnServiceName = OUString())
        :   m_TableName(_aTableName),
            m_ColumnName(_aColumnName),
            m_ColumnLabel(_aColumnLabel),
            m_ColumnTypeName(_aColumnTypeName),
            m_ColumnServiceName(_aColumnServiceName),

            m_Nullable(_aNullable),
            m_ColumnDisplaySize(_aColumnDisplaySize),
            m_Precision(_aPrecision),
            m_Scale(_aScale),
            m_ColumnType(_aColumnType),

            m_AutoIncrement(_aAutoIncrement),
            m_CaseSensitive(_aCaseSensitive),
            m_Searchable(_aSearchable),
            m_Currency(_aCurrency),
            m_Signed(_aSigned),
            m_ReadOnly(_aReadOnly),
            m_Writable(_aWritable),
            m_DefinitelyWritable(_aDefinitelyWritable)
        {
            if(m_ColumnLabel.isEmpty())
                m_ColumnLabel = _aColumnName;
        }

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t ,void* _pHint ) SAL_THROW(())
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void *,void* ) SAL_THROW(())
            {  }

        sal_Bool isAutoIncrement()              const { return m_AutoIncrement; }
        sal_Bool isCaseSensitive()              const { return m_CaseSensitive; }
        sal_Bool isSearchable()                 const { return m_Searchable; }
        sal_Bool isCurrency()                   const { return m_Currency; }
        sal_Bool isSigned()                     const { return m_Signed; }
        sal_Bool isReadOnly()                   const { return m_ReadOnly; }
        sal_Bool isWritable()                   const { return m_Writable; }
        sal_Bool isDefinitelyWritable()         const { return m_DefinitelyWritable; }

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

#endif //_CONNECTIVITY_COLUMN_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
