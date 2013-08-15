/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Columns.hxx"
#include "Table.hxx"

#include <comphelper/sequence.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/TIndexes.hxx>
#include <connectivity/TKeys.hxx>

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;

using namespace ::osl;
using namespace ::rtl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

Table::Table(Tables* pTables,
             Mutex& rMutex,
             const uno::Reference< XConnection >& rConnection):
    OTableHelper(pTables,
                 rConnection,
                 sal_True),
    m_rMutex(rMutex)
{
    OTableHelper::construct();
}

Table::Table(Tables* pTables,
             Mutex& rMutex,
             const uno::Reference< XConnection >& rConnection,
             const OUString& rName,
             const OUString& rType,
             const OUString& rDescription):
    OTableHelper(pTables,
                 rConnection,
                 sal_True,
                 rName,
                 rType,
                 rDescription,
                 "",
                 ""),
    m_rMutex(rMutex)
{
    OTableHelper::construct();
}

//----- OTableHelper ---------------------------------------------------------
OCollection* Table::createColumns(const TStringVector& rNames)
{
    return new Columns(*this,
                       m_rMutex,
                       rNames);
}

OCollection* Table::createKeys(const TStringVector& rNames)
{
    // TODO: maybe add a wrapper here in case we the OKeysHelper isn't
    // fully FB compatible.
    return new OKeysHelper(this,
                           m_rMutex,
                           rNames);
}

OCollection* Table::createIndexes(const TStringVector& rNames)
{
    return new OIndexesHelper(this,
                              m_rMutex,
                              rNames);
}

//----- XAlterTable -----------------------------------------------------------
void SAL_CALL Table::alterColumnByName(const OUString& rColName,
                                       const uno::Reference< XPropertySet >& rDescriptor)
    throw(SQLException, NoSuchElementException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(WeakComponentImplHelperBase::rBHelper.bDisposed);

    uno::Reference< XPropertySet > xColumn(m_pColumns->getByName(rColName), UNO_QUERY);

    // sdbcx::Descriptor
    bool bNameChanged = xColumn->getPropertyValue("Name") != rDescriptor->getPropertyValue("Name");
    // sdbcx::ColumnDescriptor
    bool bTypeChanged = xColumn->getPropertyValue("Type") != rDescriptor->getPropertyValue("Type");
    bool bTypeNameChanged = xColumn->getPropertyValue("TypeName") != rDescriptor->getPropertyValue("TypeName");
    bool bPrecisionChanged = xColumn->getPropertyValue("Precision") != rDescriptor->getPropertyValue("Precision");
    bool bScaleChanged = xColumn->getPropertyValue("Scale") != rDescriptor->getPropertyValue("Scale");
    bool bIsNullableChanged = xColumn->getPropertyValue("IsNullable") != rDescriptor->getPropertyValue("IsNullable");
    bool bIsAutoIncrementChanged = xColumn->getPropertyValue("IsAutoIncrement") != rDescriptor->getPropertyValue("IsAutoIncrement");
    // TODO: remainder -- these are all "optional" so have to detect presence and change.

    bool bDefaultChanged = xColumn->getPropertyValue("DefaultValue")
                                     != rDescriptor->getPropertyValue("DefaultValue");

    if (bNameChanged)
    {
        OUString sNewTableName;
        rDescriptor->getPropertyValue("Name") >>= sNewTableName;
        OUString sSql(getAlterTableColumn(rColName)
                                            + " TO \"" + sNewTableName + "\"");

        getConnection()->createStatement()->execute(sSql);
    }

    if (bTypeChanged || bTypeNameChanged)
    {
        OUString sSql(getAlterTableColumn(rColName) + "TYPE " +
                ::dbtools::createStandardTypePart(rDescriptor, getConnection()));
        getConnection()->createStatement()->execute(sSql);
        // TODO: could cause errors e.g. if incompatible types, deal with them here as appropriate.
        // possibly we have to wrap things in Util::evaluateStatusVector.
    }

    if (bPrecisionChanged || bScaleChanged
        || bIsNullableChanged || bIsAutoIncrementChanged)
    {
        // TODO: changeType
    }

    if (bDefaultChanged)
    {
        OUString sOldDefault, sNewDefault;
        xColumn->getPropertyValue("DefaultValue") >>= sOldDefault;
        rDescriptor->getPropertyValue("DefaultValue") >>= sNewDefault;

        OUString sSql;
        if (sNewDefault.isEmpty())
            sSql = getAlterTableColumn(rColName) + "DROP DEFAULT";
        else
            sSql = getAlterTableColumn(rColName) + "SET DEFAULT " + sNewDefault;

        getConnection()->createStatement()->execute(sSql);
    }

    m_pColumns->refresh();
    // TODO: implement me
}

// ----- XRename --------------------------------------------------------------
void SAL_CALL Table::rename(const OUString& rName)
    throw(SQLException, ElementExistException, RuntimeException)
{
    (void) rName;
    throw RuntimeException(); // Firebird doesn't support this.
}

// ----- XInterface -----------------------------------------------------------
Any SAL_CALL Table::queryInterface(const Type& rType)
    throw(RuntimeException)
{
    if (rType.getTypeName() == "com.sun.star.sdbcx.XRename")
        return Any();

    return OTableHelper::queryInterface(rType);
}

// ----- XTypeProvider --------------------------------------------------------
uno::Sequence< Type > SAL_CALL Table::getTypes()
    throw(RuntimeException)
{
    uno::Sequence< Type > aTypes = OTableHelper::getTypes();

    for (int i = 0; i < aTypes.getLength(); i++)
    {
        if (aTypes[i].getTypeName() == "com.sun.star.sdbcx.XRename")
        {
            ::comphelper::removeElementAt(aTypes, i);
            break;
        }
    }

    return OTableHelper::getTypes();
}

OUString Table::getAlterTableColumn(const OUString& rColumn)
{
    return ("ALTER TABLE \"" + getName() + "\" ALTER COLUMN \"" + rColumn + "\" ");
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */