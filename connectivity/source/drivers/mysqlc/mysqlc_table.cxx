/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mysqlc_columns.hxx"
#include "mysqlc_indexes.hxx"
#include "mysqlc_keys.hxx"
#include "mysqlc_table.hxx"

#include <TConnection.hxx>

#include <sal/log.hxx>
#include <comphelper/sequence.hxx>
#include <connectivity/dbtools.hxx>

#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::connectivity;
using namespace ::connectivity::mysqlc;
using namespace ::connectivity::sdbcx;

using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;

Table::Table(Tables* pTables, Mutex& rMutex, const uno::Reference<XConnection>& rConnection)
    : OTableHelper(pTables, rConnection, true)
    , m_rMutex(rMutex)
    , m_nPrivileges(0)
{
    construct();
}

Table::Table(Tables* pTables, Mutex& rMutex, const uno::Reference<XConnection>& rConnection,
             const OUString& rName, const OUString& rType, const OUString& rDescription)
    : OTableHelper(pTables, rConnection, true, rName, rType, rDescription, "", "")
    , m_rMutex(rMutex)
    , m_nPrivileges(0)
{
    construct();
}

void Table::construct()
{
    OTableHelper::construct();
    if (isNew())
        return;

    // TODO: get privileges when in non-embedded mode.
    m_nPrivileges = Privilege::DROP | Privilege::REFERENCE | Privilege::ALTER | Privilege::CREATE
                    | Privilege::READ | Privilege::DELETE | Privilege::UPDATE | Privilege::INSERT
                    | Privilege::SELECT;
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRIVILEGES),
                     PROPERTY_ID_PRIVILEGES, PropertyAttribute::READONLY, &m_nPrivileges,
                     cppu::UnoType<decltype(m_nPrivileges)>::get());
}
//----- OTableHelper ---------------------------------------------------------
OCollection* Table::createColumns(const ::std::vector<OUString>& rNames)
{
    return new Columns(*this, m_rMutex, rNames);
}

OCollection* Table::createKeys(const ::std::vector<OUString>& rNames)
{
    return new Keys(this, m_rMutex, rNames);
}

OCollection* Table::createIndexes(const ::std::vector<OUString>& rNames)
{
    return new Indexes(this, m_rMutex, rNames);
}

//----- XAlterTable -----------------------------------------------------------
void SAL_CALL Table::alterColumnByName(const OUString& /* rColName */,
                                       const uno::Reference<XPropertySet>& /* rDescriptor */)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(WeakComponentImplHelperBase::rBHelper.bDisposed);

    uno::Reference<XPropertySet> xColumn(m_xColumns->getByName(rColName), UNO_QUERY);

    // sdbcx::Descriptor
    const bool bNameChanged
        = xColumn->getPropertyValue("Name") != rDescriptor->getPropertyValue("Name");
    // sdbcx::ColumnDescriptor
    const bool bTypeChanged
        = xColumn->getPropertyValue("Type") != rDescriptor->getPropertyValue("Type");
    const bool bTypeNameChanged
        = xColumn->getPropertyValue("TypeName") != rDescriptor->getPropertyValue("TypeName");
    const bool bPrecisionChanged
        = xColumn->getPropertyValue("Precision") != rDescriptor->getPropertyValue("Precision");
    const bool bScaleChanged
        = xColumn->getPropertyValue("Scale") != rDescriptor->getPropertyValue("Scale");
    const bool bIsNullableChanged
        = xColumn->getPropertyValue("IsNullable") != rDescriptor->getPropertyValue("IsNullable");
    const bool bIsAutoIncrementChanged = xColumn->getPropertyValue("IsAutoIncrement")
                                         != rDescriptor->getPropertyValue("IsAutoIncrement");

    // TODO: remainder -- these are all "optional" so have to detect presence and change.

    bool bDefaultChanged = xColumn->getPropertyValue("DefaultValue")
                           != rDescriptor->getPropertyValue("DefaultValue");

    // TODO: tests to do
    if (bTypeChanged || bTypeNameChanged || bPrecisionChanged || bScaleChanged || bIsNullableChanged
        || bIsAutoIncrementChanged || bDefaultChanged)
    {
        // If bPrecisionChanged this will only succeed if we have increased the
        // precision, otherwise an exception is thrown -- however the base
        // gui then offers to delete and recreate the column.
        OUString sSql(getAlterTableColumn(rColName) + "TYPE "
                      + ::dbtools::createStandardTypePart(rDescriptor, getConnection()));
        getConnection()->createStatement()->execute(sSql);
        // TODO: could cause errors e.g. if incompatible types, deal with them here as appropriate.
        // possibly we have to wrap things in Util::evaluateStatusVector.
    }

    // TODO: tests to do
    // TODO: quote identifiers as needed.
    if (bNameChanged)
    {
        OUString sNewColName;
        rDescriptor->getPropertyValue("Name") >>= sNewColName;
        OUString sSql(getAlterTableColumn(rColName) + " TO \"" + sNewColName + "\"");

        getConnection()->createStatement()->execute(sSql);
    }

    m_xColumns->refresh();
}

void SAL_CALL Table::alterColumnByIndex(
    sal_Int32 /* index */, const css::uno::Reference<css::beans::XPropertySet>& /* descriptor */)
{
    MutexGuard aGuard(m_rMutex);
    // TODO: implement
}

// ----- XRename --------------------------------------------------------------
void SAL_CALL Table::rename(const OUString&)
{
    MutexGuard aGuard(m_rMutex);
    // TODO: implement
}

// ----- XInterface -----------------------------------------------------------
Any SAL_CALL Table::queryInterface(const Type& rType)
{
    if (rType.getTypeName() == "com.sun.star.sdbcx.XRename")
        return Any();
    return OTableHelper::queryInterface(rType);
}

// ----- XTypeProvider --------------------------------------------------------
uno::Sequence<Type> SAL_CALL Table::getTypes() { return OTableHelper::getTypes(); }

OUString Table::getAlterTableColumn(std::u16string_view rColumn)
{
    // TODO: test
    return ("ALTER TABLE \"" + getName() + "\" ALTER COLUMN \"" + rColumn + "\" ");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
