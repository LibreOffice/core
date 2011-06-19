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
#include "ado/AResultSetMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include "ado/Awrapado.hxx"
#include "connectivity/dbexception.hxx"

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

OResultSetMetaData::OResultSetMetaData( ADORecordset* _pRecordSet)
                    :   m_pRecordSet(_pRecordSet),
                        m_nColCount(-1)
{
    if ( m_pRecordSet )
        m_pRecordSet->AddRef();
}
// -------------------------------------------------------------------------
OResultSetMetaData::~OResultSetMetaData()
{
    if ( m_pRecordSet )
        m_pRecordSet->Release();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid() && aField.GetActualSize() != -1)
        return aField.GetActualSize();
    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    return ADOS::MapADOType2Jdbc(aField.GetADOType());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    if(m_nColCount != -1 )
        return m_nColCount;

    if ( !m_pRecordSet )
        return 0;

    ADOFields* pFields  = NULL;
    m_pRecordSet->get_Fields(&pFields);
    WpOLEAppendCollection<ADOFields, ADOField, WpADOField>  aFields(pFields);
    m_nColCount = aFields.GetItemCount();
    return m_nColCount;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    sal_Bool bRet = sal_False;
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if ( aField.IsValid() )
    {
        WpADOProperties aProps( aField.get_Properties() );
        if ( aProps.IsValid() )
            bRet = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ISCASESENSITIVE")) );
    }
    return bRet;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getSchemaName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
        return aField.GetName();

    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString sTableName;

    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if ( aField.IsValid() )
    {
        WpADOProperties aProps( aField.get_Properties() );
        if ( aProps.IsValid() )
            sTableName = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BASETABLENAME")) );
    }
    return sTableName;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getCatalogName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnTypeName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
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
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        return ((aField.GetAttributes() & adFldFixed) == adFldFixed) && (aField.GetADOType() == adCurrency);
    }
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    sal_Bool bRet = sal_False;
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if ( aField.IsValid() )
    {
        WpADOProperties aProps( aField.get_Properties() );
        if ( aProps.IsValid() )
        {
            bRet = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ISAUTOINCREMENT")) );
#if OSL_DEBUG_LEVEL > 0
            sal_Int32 nCount = aProps.GetItemCount();
            for (sal_Int32 i = 0; i<nCount; ++i)
            {
                WpADOProperty aProp = aProps.GetItem(i);
                ::rtl::OUString sName = aProp.GetName();
                ::rtl::OUString sVal = aProp.GetValue();
            }
#endif
        }
    }
    return bRet;
}
// -------------------------------------------------------------------------


sal_Bool SAL_CALL OResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        DataTypeEnum eType = aField.GetADOType();
        return !(eType == adUnsignedBigInt || eType == adUnsignedInt || eType == adUnsignedSmallInt || eType == adUnsignedTinyInt);
    }
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
        return aField.GetPrecision();
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
        return aField.GetNumericScale();
    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldIsNullable) == adFldIsNullable;
    }
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isSearchable( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        //  return (aField.GetStatus() & adFieldReadOnly) == adFieldReadOnly;
    }
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldUpdatable) == adFldUpdatable;
    }
    return sal_False;
;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return isDefinitelyWritable(column);
}
// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
