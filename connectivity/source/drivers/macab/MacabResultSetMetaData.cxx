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


#include "MacabResultSetMetaData.hxx"
#include "MacabHeader.hxx"
#include "MacabRecords.hxx"
#include "MacabAddressBook.hxx"
#include "macabutilities.hxx"
#include <strings.hrc>

using namespace connectivity::macab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

MacabResultSetMetaData::MacabResultSetMetaData(MacabConnection* _pConnection, OUString const & _sTableName)
    : m_pConnection(_pConnection),
        m_sTableName(_sTableName),
      m_aMacabFields()
{
}

MacabResultSetMetaData::~MacabResultSetMetaData()
{
}

void MacabResultSetMetaData::setMacabFields(const ::rtl::Reference<connectivity::OSQLColumns> &xColumns)
{
    static const char aName[] = "Name";
    MacabRecords *aRecords;
    MacabHeader *aHeader;

    aRecords = m_pConnection->getAddressBook()->getMacabRecords(m_sTableName);

    // In case, somehow, we don't have anything with the name m_sTableName
    if(aRecords == nullptr)
    {
        impl_throwError(STR_NO_TABLE);
    }

    aHeader = aRecords->getHeader();

    for (const auto& rxColumn : xColumns->get())
    {
        OUString aFieldName;
        sal_uInt32 nFieldNumber;

        rxColumn->getPropertyValue(aName) >>= aFieldName;
        nFieldNumber = aHeader->getColumnNumber(aFieldName);
        m_aMacabFields.push_back(nFieldNumber);
    }

}

sal_Int32 SAL_CALL MacabResultSetMetaData::getColumnDisplaySize(sal_Int32 /* column */)
{
    // For now, all columns are the same size.
    return 50;
}

sal_Int32 SAL_CALL MacabResultSetMetaData::getColumnType(sal_Int32 column)
{
    MacabRecords *aRecords;
    MacabHeader *aHeader;
    macabfield *aField;

    aRecords = m_pConnection->getAddressBook()->getMacabRecords(m_sTableName);

    // In case, somehow, we don't have anything with the name m_sTableName
    if(aRecords == nullptr)
    {
        impl_throwError(STR_NO_TABLE);
    }

    aHeader = aRecords->getHeader();
    aField = aHeader->get(column-1);

    if(aField == nullptr)
    {
        ::dbtools::throwInvalidIndexException(*this);
        return -1;
    }

    return ABTypeToDataType(aField->type);
}

sal_Int32 SAL_CALL MacabResultSetMetaData::getColumnCount()
{
    return m_aMacabFields.size();
}

sal_Bool SAL_CALL MacabResultSetMetaData::isCaseSensitive(sal_Int32)
{
    return true;
}

OUString SAL_CALL MacabResultSetMetaData::getSchemaName(sal_Int32)
{
    return OUString();
}

OUString SAL_CALL MacabResultSetMetaData::getColumnName(sal_Int32 column)
{
    sal_uInt32 nFieldNumber = m_aMacabFields[column - 1];
    MacabRecords *aRecords;
    MacabHeader *aHeader;

    aRecords = m_pConnection->getAddressBook()->getMacabRecords(m_sTableName);

    // In case, somehow, we don't have anything with the name m_sTableName
    if(aRecords == nullptr)
    {
        impl_throwError(STR_NO_TABLE);
    }

    aHeader = aRecords->getHeader();
    OUString aName = aHeader->getString(nFieldNumber);

    return aName;
}

OUString SAL_CALL MacabResultSetMetaData::getTableName(sal_Int32)
{
    return m_sTableName;
}

OUString SAL_CALL MacabResultSetMetaData::getCatalogName(sal_Int32)
{
    return OUString();
}

OUString SAL_CALL MacabResultSetMetaData::getColumnTypeName(sal_Int32)
{
    return OUString();
}

OUString SAL_CALL MacabResultSetMetaData::getColumnLabel(sal_Int32)
{
    return OUString();
}

OUString SAL_CALL MacabResultSetMetaData::getColumnServiceName(sal_Int32)
{
    return OUString();
}

sal_Bool SAL_CALL MacabResultSetMetaData::isCurrency(sal_Int32)
{
    return false;
}

sal_Bool SAL_CALL MacabResultSetMetaData::isAutoIncrement(sal_Int32)
{
    return false;
}

sal_Bool SAL_CALL MacabResultSetMetaData::isSigned(sal_Int32)
{
    return false;
}

sal_Int32 SAL_CALL MacabResultSetMetaData::getPrecision(sal_Int32)
{
    return 0;
}

sal_Int32 SAL_CALL MacabResultSetMetaData::getScale(sal_Int32)
{
    return 0;
}

sal_Int32 SAL_CALL MacabResultSetMetaData::isNullable(sal_Int32)
{
    return sal_Int32(true);
}

sal_Bool SAL_CALL MacabResultSetMetaData::isSearchable(sal_Int32)
{
    return true;
}

sal_Bool SAL_CALL MacabResultSetMetaData::isReadOnly(sal_Int32)
{
    return true;
}

sal_Bool SAL_CALL MacabResultSetMetaData::isDefinitelyWritable(sal_Int32)
{
    return false;
}

sal_Bool SAL_CALL MacabResultSetMetaData::isWritable(sal_Int32)
{
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
