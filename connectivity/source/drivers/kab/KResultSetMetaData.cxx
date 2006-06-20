/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: KResultSetMetaData.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:39:36 $
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

#include "KResultSetMetaData.hxx"

#ifndef _CONNECTIVITY_KAB_FIELDS_HXX_
#include "kfields.hxx"
#endif
#ifndef _CONNECTIVITY_KAB_DATABASEMETADATA_HXX_
#include "KDatabaseMetaData.hxx"
#endif

#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif

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
void KabResultSetMetaData::setKabFields(const ::vos::ORef<connectivity::OSQLColumns> &xColumns) throw(SQLException)
{
    OSQLColumns::const_iterator aIter;
    static const ::rtl::OUString aName(::rtl::OUString::createFromAscii("Name"));

    for (aIter = xColumns->begin(); aIter != xColumns->end(); ++aIter)
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
