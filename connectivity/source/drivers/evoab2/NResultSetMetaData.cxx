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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "NResultSetMetaData.hxx"
#include "NDatabaseMetaData.hxx"
#include "connectivity/dbexception.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include "NDebug.hxx"
#include "resource/evoab2_res.hrc"

using namespace connectivity::evoab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

OEvoabResultSetMetaData::OEvoabResultSetMetaData(const ::rtl::OUString& _aTableName)
    : m_aTableName(_aTableName),
      m_aEvoabFields()
{

}
// -------------------------------------------------------------------------
OEvoabResultSetMetaData::~OEvoabResultSetMetaData()
{
}
// -------------------------------------------------------------------------
void OEvoabResultSetMetaData::setEvoabFields(const ::rtl::Reference<connectivity::OSQLColumns> &xColumns) throw(SQLException)
{
        OSQLColumns::Vector::const_iterator aIter;
        static const ::rtl::OUString aName(RTL_CONSTASCII_USTRINGPARAM("Name"));

        for (aIter = xColumns->get().begin(); aIter != xColumns->get().end(); ++aIter)
        {
                ::rtl::OUString aFieldName;

                (*aIter)->getPropertyValue(aName) >>= aFieldName;
                guint nFieldNumber = findEvoabField(aFieldName);
                if (nFieldNumber == (guint)-1)
                {
                    connectivity::SharedResources aResource;
                    const ::rtl::OUString sError( aResource.getResourceStringWithSubstitution(
                            STR_INVALID_COLUMNNAME,
                            "$columnname$", aFieldName
                         ) );
                    ::dbtools::throwGenericSQLException( sError, *this );
                }
                m_aEvoabFields.push_back(nFieldNumber);
        }
}

// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnDisplaySize( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return 50;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnType( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
        sal_uInt32 nField = m_aEvoabFields[nColumnNum - 1];
    return evoab::getFieldType (nField);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    return m_aEvoabFields.size();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isCaseSensitive( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getSchemaName( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnName( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    sal_uInt32 nField = m_aEvoabFields[nColumnNum - 1];
    return evoab::getFieldName( nField );
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnTypeName( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    sal_uInt32 nField = m_aEvoabFields[nColumnNum - 1];
    return evoab::getFieldTypeName( nField );
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnLabel( sal_Int32 nColumnNum ) throw(SQLException, RuntimeException)
{
    sal_uInt32 nField = m_aEvoabFields[nColumnNum - 1];
    const ColumnProperty *pSpecs = getField(nField);
    GParamSpec *pSpec = pSpecs->pField;
    rtl::OUString aLabel;

    if( pSpec )
            aLabel = rtl::OStringToOUString( g_param_spec_get_nick( (GParamSpec *) pSpec ),
                                             RTL_TEXTENCODING_UTF8 );
    return aLabel;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getColumnServiceName( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getTableName( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return m_aTableName;//::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE"));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabResultSetMetaData::getCatalogName( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabResultSetMetaData::isCurrency( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isAutoIncrement( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isSigned( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getPrecision( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::getScale( sal_Int32 /*nColumnNum*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabResultSetMetaData::isNullable( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isSearchable( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isReadOnly( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isDefinitelyWritable( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabResultSetMetaData::isWritable( sal_Int32 /*nColumnNum*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
