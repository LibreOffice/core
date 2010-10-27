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

#ifndef _CONNECTIVITY_EVOAB_RESULTSETMETADATA_HXX_
#define _CONNECTIVITY_EVOAB_RESULTSETMETADATA_HXX_

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <cppuhelper/implbase1.hxx>
#include "NConnection.hxx"
#include <rtl/ref.hxx>
#include <com/sun/star/connection/XConnection.hpp>
namespace connectivity
{
    namespace evoab
    {
        //**************************************************************
        //************ Class: ResultSetMetaData
        //**************************************************************
        typedef ::cppu::WeakImplHelper1< ::com::sun::star::sdbc::XResultSetMetaData>   OResultSetMetaData_BASE;

        class OEvoabResultSetMetaData : public  OResultSetMetaData_BASE
        {
          ::rtl::OUString       m_aTableName;
                  ::std::vector<sal_Int32>        m_aEvoabFields;

        protected:
            virtual ~OEvoabResultSetMetaData();
        public:
          OEvoabResultSetMetaData(const ::rtl::OUString& _aTableName);
          void setEvoabFields(const ::rtl::Reference<connectivity::OSQLColumns> &xColumns) throw(::com::sun::star::sdbc::SQLException);
          inline sal_uInt32 fieldAtColumn(sal_Int32 columnIndex) const
                        { return m_aEvoabFields[columnIndex - 1]; }
          inline sal_Int32 getFieldSize() const
            {return m_aEvoabFields.size();}
          /// Avoid ambigous cast error from the compiler.
          inline operator ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > () throw()
          { return this; }

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
