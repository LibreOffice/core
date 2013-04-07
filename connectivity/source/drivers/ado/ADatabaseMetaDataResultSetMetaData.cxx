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

#include "ado/ADatabaseMetaDataResultSetMetaData.hxx"
#include "ado/Awrapado.hxx"
#include "connectivity/dbexception.hxx"


using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSetMetaData::~ODatabaseMetaDataResultSetMetaData()
{
    if(m_pRecordSet)
        m_pRecordSet->Release();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    sal_Int32 nSize = 0;
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        nSize = (*m_mColumnsIter).second.getColumnDisplaySize();
    else if(m_pRecordSet)
    {
        WpADOField aField = ADOS::getField(m_pRecordSet,m_vMapping[column]);
        if(aField.IsValid())
            nSize = aField.GetActualSize();
    }
    return nSize;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    sal_Int32  nType = 0;
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        nType = (*m_mColumnsIter).second.getColumnType();
    else if(m_pRecordSet)
    {
        WpADOField aField = ADOS::getField(m_pRecordSet,m_vMapping[column]);
        nType = ADOS::MapADOType2Jdbc(aField.GetADOType());
    }
    return nType;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    if(!m_pRecordSet)
        return 0;
    if(m_nColCount != -1)
        return m_nColCount;

    if(m_vMapping.size())
        return m_mColumns.size();

    ADOFields* pFields  = NULL;
    m_pRecordSet->get_Fields(&pFields);
    WpOLEAppendCollection<ADOFields, ADOField, WpADOField>  aFields(pFields);
    m_nColCount = aFields.GetItemCount();
    return m_nColCount;
}
// -------------------------------------------------------------------------

OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnName();
    if(!m_pRecordSet)
        return OUString();
    WpADOField aField = ADOS::getField(m_pRecordSet,m_vMapping[column]);
    if(aField.IsValid())
        return aField.GetName();

    return OUString();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnLabel();
    return getColumnName(column);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isCurrency();
    if(!m_pRecordSet)
        return 0;
    WpADOField aField = ADOS::getField(m_pRecordSet,m_vMapping[column]);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldFixed) == adFldFixed;
    }
    return sal_False;
}
// -------------------------------------------------------------------------


sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isSigned();
    if(!m_pRecordSet)
        return 0;
    WpADOField aField = ADOS::getField(m_pRecordSet,m_vMapping[column]);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldNegativeScale) == adFldNegativeScale;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getPrecision();
    if(!m_pRecordSet)
        return 0;
    WpADOField aField = ADOS::getField(m_pRecordSet,m_vMapping[column]);
    if(aField.IsValid())
        return aField.GetPrecision();
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getScale();

    if(!m_pRecordSet)
        return 0;

    WpADOField aField = ADOS::getField(m_pRecordSet,m_vMapping[column]);
    if(aField.IsValid())
        return aField.GetNumericScale();
    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isNullable();

    if(!m_pRecordSet)
        return 0;

    WpADOField aField = ADOS::getField(m_pRecordSet,m_vMapping[column]);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldIsNullable) == adFldIsNullable;
    }
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isReadOnly();

    if(!m_pRecordSet)
        return 0;

    WpADOField aField = ADOS::getField(m_pRecordSet,m_vMapping[column]);
    if(aField.IsValid())
    {
        //  return (aField.GetStatus() & adFieldReadOnly) == adFieldReadOnly;
    }
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isDefinitelyWritable();

    if(!m_pRecordSet)
        return 0;

    WpADOField aField = ADOS::getField(m_pRecordSet,m_vMapping[column]);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldUpdatable) == adFldUpdatable;
    }
    return sal_False;
;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isWritable();
    return isDefinitelyWritable(column);
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
