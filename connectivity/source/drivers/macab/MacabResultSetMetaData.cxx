/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MacabResultSetMetaData.cxx,v $
 * $Revision: 1.3 $
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

#include "MacabResultSetMetaData.hxx"
#include "MacabHeader.hxx"
#include "MacabRecords.hxx"
#include "MacabAddressBook.hxx"
#include "macabutilities.hxx"

using namespace connectivity::macab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

MacabResultSetMetaData::MacabResultSetMetaData(MacabConnection* _pConnection, ::rtl::OUString _sTableName)
    : m_pConnection(_pConnection),
        m_sTableName(_sTableName),
      m_aMacabFields()
{
}
// -------------------------------------------------------------------------
MacabResultSetMetaData::~MacabResultSetMetaData()
{
}
// -------------------------------------------------------------------------
void MacabResultSetMetaData::setMacabFields(const ::vos::ORef<connectivity::OSQLColumns> &xColumns) throw(SQLException)
{
    OSQLColumns::const_iterator aIter;
    static const ::rtl::OUString aName(::rtl::OUString::createFromAscii("Name"));
    MacabRecords *aRecords;
    MacabHeader *aHeader;

    aRecords = m_pConnection->getAddressBook()->getMacabRecords(m_sTableName);

    // In case, somehow, we don't have anything with the name m_sTableName
    if(aRecords == NULL)
    {
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii("No Such Table!"),
            NULL);
        return;
    }

    aHeader = aRecords->getHeader();

    for (aIter = xColumns->begin(); aIter != xColumns->end(); ++aIter)
    {
        ::rtl::OUString aFieldName;
        sal_uInt32 nFieldNumber;

        (*aIter)->getPropertyValue(aName) >>= aFieldName;
        nFieldNumber = aHeader->getColumnNumber(aFieldName);
        m_aMacabFields.push_back(nFieldNumber);
    }

}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL MacabResultSetMetaData::getColumnDisplaySize(sal_Int32 column) throw(SQLException, RuntimeException)
{
    // For now, all columns are the same size.
    return 50;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL MacabResultSetMetaData::getColumnType(sal_Int32 column) throw(SQLException, RuntimeException)
{
    MacabRecords *aRecords;
    MacabHeader *aHeader;
    macabfield *aField;

    aRecords = m_pConnection->getAddressBook()->getMacabRecords(m_sTableName);

    // In case, somehow, we don't have anything with the name m_sTableName
    if(aRecords == NULL)
    {
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii("No Such Table!"),
            NULL);
        return -1;
    }

    aHeader = aRecords->getHeader();
    aField = aHeader->get(column-1);

    if(aField == NULL)
    {
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii("No column at location: ") +
            ::rtl::OUString::valueOf(column),
            NULL);

        return -1;
    }

    return ABTypeToDataType(aField->type);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL MacabResultSetMetaData::getColumnCount() throw(SQLException, RuntimeException)
{
    return m_aMacabFields.size();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL MacabResultSetMetaData::isCaseSensitive(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL MacabResultSetMetaData::getSchemaName(sal_Int32) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL MacabResultSetMetaData::getColumnName(sal_Int32 column) throw(SQLException, RuntimeException)
{
    sal_uInt32 nFieldNumber = m_aMacabFields[column - 1];
    MacabRecords *aRecords;
    MacabHeader *aHeader;

    aRecords = m_pConnection->getAddressBook()->getMacabRecords(m_sTableName);

    // In case, somehow, we don't have anything with the name m_sTableName
    if(aRecords == NULL)
    {
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii("No Such Table!"),
            NULL);
        return ::rtl::OUString();
    }

    aHeader = aRecords->getHeader();
    ::rtl::OUString aName = aHeader->getString(nFieldNumber);

    return aName;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL MacabResultSetMetaData::getTableName(sal_Int32) throw(SQLException, RuntimeException)
{
    return m_sTableName;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL MacabResultSetMetaData::getCatalogName(sal_Int32) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL MacabResultSetMetaData::getColumnTypeName(sal_Int32) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL MacabResultSetMetaData::getColumnLabel(sal_Int32) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL MacabResultSetMetaData::getColumnServiceName(sal_Int32) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL MacabResultSetMetaData::isCurrency(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL MacabResultSetMetaData::isAutoIncrement(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL MacabResultSetMetaData::isSigned(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL MacabResultSetMetaData::getPrecision(sal_Int32) throw(SQLException, RuntimeException)
{
    return 0;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL MacabResultSetMetaData::getScale(sal_Int32) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL MacabResultSetMetaData::isNullable(sal_Int32) throw(SQLException, RuntimeException)
{
    return (sal_Int32) sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL MacabResultSetMetaData::isSearchable(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL MacabResultSetMetaData::isReadOnly(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL MacabResultSetMetaData::isDefinitelyWritable(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL MacabResultSetMetaData::isWritable(sal_Int32) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
