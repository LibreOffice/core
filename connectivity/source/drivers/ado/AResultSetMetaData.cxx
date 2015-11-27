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

#include "ado/AResultSetMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include "ado/Awrapado.hxx"
#include <connectivity/dbexception.hxx>

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

OResultSetMetaData::~OResultSetMetaData()
{
    if ( m_pRecordSet )
        m_pRecordSet->Release();
}

sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid() && aField.GetActualSize() != -1)
        return aField.GetActualSize();
    return 0;
}


sal_Int32 SAL_CALL OResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    return ADOS::MapADOType2Jdbc(aField.GetADOType());
}


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


sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    sal_Bool bRet = sal_False;
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if ( aField.IsValid() )
    {
        WpADOProperties aProps( aField.get_Properties() );
        if ( aProps.IsValid() )
            bRet = OTools::getValue( aProps, OUString("ISCASESENSITIVE") );
    }
    return bRet;
}


OUString SAL_CALL OResultSetMetaData::getSchemaName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return OUString();
}


OUString SAL_CALL OResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
        return aField.GetName();

    return OUString();
}

OUString SAL_CALL OResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    OUString sTableName;

    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if ( aField.IsValid() )
    {
        WpADOProperties aProps( aField.get_Properties() );
        if ( aProps.IsValid() )
            sTableName = OTools::getValue( aProps, OUString("BASETABLENAME") );
    }
    return sTableName;
}

OUString SAL_CALL OResultSetMetaData::getCatalogName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return OUString();
}

OUString SAL_CALL OResultSetMetaData::getColumnTypeName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return OUString();
}

OUString SAL_CALL OResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getColumnName(column);
}

OUString SAL_CALL OResultSetMetaData::getColumnServiceName( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return OUString();
}


sal_Bool SAL_CALL OResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        return ((aField.GetAttributes() & adFldFixed) == adFldFixed) && (aField.GetADOType() == adCurrency);
    }
    return sal_False;
}


sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    sal_Bool bRet = sal_False;
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if ( aField.IsValid() )
    {
        WpADOProperties aProps( aField.get_Properties() );
        if ( aProps.IsValid() )
        {
            bRet = OTools::getValue( aProps, OUString("ISAUTOINCREMENT") );
        }
    }
    return bRet;
}



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

sal_Int32 SAL_CALL OResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
        return aField.GetPrecision();
    return 0;
}

sal_Int32 SAL_CALL OResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
        return aField.GetNumericScale();
    return 0;
}


sal_Int32 SAL_CALL OResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldIsNullable) == adFldIsNullable;
    }
    return sal_False;
}


sal_Bool SAL_CALL OResultSetMetaData::isSearchable( sal_Int32 /*column*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}


sal_Bool SAL_CALL OResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        //  return (aField.GetStatus() & adFieldReadOnly) == adFieldReadOnly;
    }
    return sal_False;
}


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

sal_Bool SAL_CALL OResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return isDefinitelyWritable(column);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
