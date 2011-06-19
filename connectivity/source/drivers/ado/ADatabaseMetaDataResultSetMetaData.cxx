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

::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnName();
    if(!m_pRecordSet)
        return ::rtl::OUString();
    WpADOField aField = ADOS::getField(m_pRecordSet,m_vMapping[column]);
    if(aField.IsValid())
        return aField.GetName();

    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
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
