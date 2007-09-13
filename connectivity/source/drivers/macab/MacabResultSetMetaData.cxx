/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MacabResultSetMetaData.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:54:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "MacabResultSetMetaData.hxx"

#ifndef _CONNECTIVITY_MACAB_HEADER_HXX_
#include "MacabHeader.hxx"
#endif

#ifndef _CONNECTIVITY_MACAB_RECORDS_HXX_
#include "MacabRecords.hxx"
#endif

#ifndef _CONNECTIVITY_MACAB_ADDRESSBOOK_HXX_
#include "MacabAddressBook.hxx"
#endif

#ifndef _CONNECTIVITY_MACAB_UTILITIES_HXX_
#include "macabutilities.hxx"
#endif

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
