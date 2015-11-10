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

#include "file/FResultSetMetaData.hxx"
#include "file/FTable.hxx"
#include <comphelper/extract.hxx>
#include <connectivity/dbexception.hxx>
#include <comphelper/types.hxx>


using namespace ::comphelper;
using namespace connectivity;
using namespace dbtools;
using namespace connectivity::file;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


OResultSetMetaData::OResultSetMetaData(const ::rtl::Reference<connectivity::OSQLColumns>& _rxColumns,const OUString& _aTableName,OFileTable* _pTable)
    :m_aTableName(_aTableName)
    ,m_xColumns(_rxColumns)
    ,m_pTable(_pTable)
{
}


OResultSetMetaData::~OResultSetMetaData()
{
    m_xColumns = nullptr;
}

void OResultSetMetaData::checkColumnIndex(sal_Int32 column)  throw(SQLException, RuntimeException)
{
    if(column <= 0 || column > (sal_Int32)(sal_Int32)m_xColumns->get().size())
        throwInvalidIndexException(*this);
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    return getPrecision(column);
}


sal_Int32 SAL_CALL OResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    checkColumnIndex(column);
    return getINT32((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));
}


sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException, std::exception)
{
    return (m_xColumns->get()).size();
}


sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return sal_False;
}


OUString SAL_CALL OResultSetMetaData::getSchemaName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return OUString();
}


OUString SAL_CALL OResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    checkColumnIndex(column);

    Any aName((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)));
    return aName.hasValue() ? getString(aName) : getString((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)));
}

OUString SAL_CALL OResultSetMetaData::getTableName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return m_aTableName;
}

OUString SAL_CALL OResultSetMetaData::getCatalogName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return OUString();
}

OUString SAL_CALL OResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    checkColumnIndex(column);
    return getString((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME)));
}

OUString SAL_CALL OResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    return getColumnName(column);
}

OUString SAL_CALL OResultSetMetaData::getColumnServiceName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return OUString();
}


sal_Bool SAL_CALL OResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    checkColumnIndex(column);
    return getBOOL((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY)));
}


sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement( sal_Int32 /*setCatalogcolumn*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return sal_False;
}

sal_Bool SAL_CALL OResultSetMetaData::isSigned( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return sal_True;
}

sal_Int32 SAL_CALL OResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    checkColumnIndex(column);
    return getINT32((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)));
}

sal_Int32 SAL_CALL OResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    checkColumnIndex(column);
    return getINT32((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)));
}


sal_Int32 SAL_CALL OResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    checkColumnIndex(column);
    return getINT32((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)));
}


sal_Bool SAL_CALL OResultSetMetaData::isSearchable( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return sal_True;
}


sal_Bool SAL_CALL OResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    checkColumnIndex(column);
    return m_pTable->isReadOnly() || (
                            (m_xColumns->get())[column-1]->getPropertySetInfo()->hasPropertyByName(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FUNCTION)) &&
                                ::cppu::any2bool((m_xColumns->get())[column-1]->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FUNCTION))));
}


sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    return !isReadOnly(column);
}

sal_Bool SAL_CALL OResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException, std::exception)
{
    return !isReadOnly(column);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
