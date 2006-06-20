/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OColumn.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:57:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_COLUMN_HXX_
#define _CONNECTIVITY_COLUMN_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace connectivity
{
    class OColumn
    {
        ::rtl::OUString m_CatalogName;
        ::rtl::OUString m_SchemaName;
        ::rtl::OUString m_TableName;
        ::rtl::OUString m_ColumnName;
        ::rtl::OUString m_ColumnLabel;
        ::rtl::OUString m_ColumnTypeName;
        ::rtl::OUString m_ColumnServiceName;

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
        OColumn(const ::rtl::OUString &_aTableName,
                const ::rtl::OUString &_aColumnName,

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

                const ::rtl::OUString &_aColumnLabel = ::rtl::OUString(),
                const ::rtl::OUString &_aColumnTypeName = ::rtl::OUString(),
                const ::rtl::OUString &_aColumnServiceName = ::rtl::OUString())
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
            if(!m_ColumnLabel.getLength())
                m_ColumnLabel = _aColumnName;
        }

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t ,void* _pHint ) SAL_THROW( () )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void *,void* ) SAL_THROW( () )
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

        const ::rtl::OUString& getColumnLabel()         const { return m_ColumnLabel; }
        const ::rtl::OUString& getColumnName()          const { return m_ColumnName; }
        const ::rtl::OUString& getSchemaName()          const { return m_SchemaName; }
        const ::rtl::OUString& getTableName()           const { return m_TableName; }
        const ::rtl::OUString& getCatalogName()         const { return m_CatalogName; }
        const ::rtl::OUString& getColumnTypeName()      const { return m_ColumnTypeName; }
        const ::rtl::OUString& getColumnServiceName()   const { return m_ColumnServiceName; }

    };
}

#endif //_CONNECTIVITY_COLUMN_HXX_

