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


#include "KResultSetMetaData.hxx"
#include "kfields.hxx"
#include "KDatabaseMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>

using namespace connectivity::kab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

KabResultSetMetaData::KabResultSetMetaData()
{
}
// -------------------------------------------------------------------------
KabResultSetMetaData::~KabResultSetMetaData()
{
}
// -------------------------------------------------------------------------
void KabResultSetMetaData::setKabFields(const ::rtl::Reference<connectivity::OSQLColumns> &xColumns) throw(SQLException)
{
    OSQLColumns::Vector::const_iterator aIter;
    static const ::rtl::OUString aName("Name");

    for (aIter = xColumns->get().begin(); aIter != xColumns->get().end(); ++aIter)
    {
        ::rtl::OUString aFieldName;
        sal_uInt32 nFieldNumber;

        (*aIter)->getPropertyValue(aName) >>= aFieldName;
        nFieldNumber = findKabField(aFieldName);
        m_aKabFields.push_back(nFieldNumber);
    }
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabResultSetMetaData::getColumnDisplaySize(sal_Int32 column) throw(SQLException, RuntimeException)
{
    return m_aKabFields[column - 1] < KAB_DATA_FIELDS? 20: 50;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabResultSetMetaData::getColumnType(sal_Int32 column) throw(SQLException, RuntimeException)
{
    return m_aKabFields[column - 1] == KAB_FIELD_REVISION? DataType::TIMESTAMP: DataType::CHAR;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabResultSetMetaData::getColumnCount() throw(SQLException, RuntimeException)
{
    return m_aKabFields.size();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSetMetaData::isCaseSensitive(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabResultSetMetaData::getSchemaName(sal_Int32) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabResultSetMetaData::getColumnName(sal_Int32 column) throw(SQLException, RuntimeException)
{
    sal_uInt32 nFieldNumber = m_aKabFields[column - 1];
    ::KABC::Field::List aFields = ::KABC::Field::allFields();
    QString aQtName;

    switch (nFieldNumber)
    {
        case KAB_FIELD_REVISION:
            aQtName = KABC::Addressee::revisionLabel();
            break;
        default:
            aQtName = aFields[nFieldNumber - KAB_DATA_FIELDS]->label();
    }
    ::rtl::OUString aName((const sal_Unicode *) aQtName.ucs2());

    return aName;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabResultSetMetaData::getTableName(sal_Int32) throw(SQLException, RuntimeException)
{
    return KabDatabaseMetaData::getAddressBookTableName();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabResultSetMetaData::getCatalogName(sal_Int32) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabResultSetMetaData::getColumnTypeName(sal_Int32) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabResultSetMetaData::getColumnLabel(sal_Int32) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabResultSetMetaData::getColumnServiceName(sal_Int32) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSetMetaData::isCurrency(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSetMetaData::isAutoIncrement(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSetMetaData::isSigned(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabResultSetMetaData::getPrecision(sal_Int32) throw(SQLException, RuntimeException)
{
    return 0;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL KabResultSetMetaData::getScale(sal_Int32) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabResultSetMetaData::isNullable(sal_Int32) throw(SQLException, RuntimeException)
{
    return (sal_Int32) sal_True;
// KDE address book currently does not use NULL values.
// But it might do it someday
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSetMetaData::isSearchable(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSetMetaData::isReadOnly(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSetMetaData::isDefinitelyWritable(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSetMetaData::isWritable(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
