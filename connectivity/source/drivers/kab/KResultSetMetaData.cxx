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

#include "KResultSetMetaData.hxx"
#include "kfields.hxx"
#include "KDatabaseMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>

using namespace connectivity::kab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

KabResultSetMetaData::KabResultSetMetaData(KabConnection* _pConnection)
    : m_pConnection(_pConnection),
      m_aKabFields()
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
    static const ::rtl::OUString aName(RTL_CONSTASCII_USTRINGPARAM("Name"));

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
