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

#include "connectivity/dbexception.hxx"
#include "connectivity/dbtools.hxx"
#include <comphelper/types.hxx>
#include <comphelper/extract.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/diagnose_ex.h>
#include "MResultSetMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>

using namespace connectivity::mork;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::beans;
using namespace ::dbtools;
using namespace ::comphelper;

// -------------------------------------------------------------------------
OResultSetMetaData::~OResultSetMetaData()
{
   m_xColumns = NULL;
}

// -----------------------------------------------------------------------------
void OResultSetMetaData::checkColumnIndex(sal_Int32 column)  throw(SQLException, RuntimeException)
{
    if(column <= 0 || column > (sal_Int32)(sal_Int32)m_xColumns->get().size())
        throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getPrecision(column);
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnType( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return DataType::VARCHAR; // at the moment there exists only this type
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    return static_cast<sal_Int32>((m_xColumns->get()).size());
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getSchemaName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);

    ::rtl::OUString sColumnName;
    try
    {
        Reference< XPropertySet > xColumnProps( (m_xColumns->get())[column-1], UNO_QUERY_THROW );
        OSL_VERIFY( xColumnProps->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_NAME ) ) >>= sColumnName );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return sColumnName;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getTableName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return m_aTableName;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getCatalogName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);
    return getString((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME)));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getColumnName(column);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnServiceName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);
    return getBOOL((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY)));
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSetMetaData::isSigned( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);
    return getINT32((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)));
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    checkColumnIndex(column);
    return getINT32((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)));
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);
    return getINT32((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)));
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString sColumnName( getColumnName( column ) );

    if ( !m_pTable || !m_pTable->getConnection() )
    {
        OSL_FAIL( "OResultSetMetaData::isSearchable: suspicious: called without table or connection!" );
        return sal_False;
    }

    return sal_True;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    checkColumnIndex(column);
    sal_Bool bReadOnly = (m_xColumns->get())[column-1]->getPropertySetInfo()->hasPropertyByName(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FUNCTION)) &&
        ::cppu::any2bool((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FUNCTION)));

    return m_bReadOnly || bReadOnly || m_pTable->isReadOnly();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return !isReadOnly(column);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return !isReadOnly(column);
}
// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
