/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Columns.hxx"
#include "Indexes.hxx"
#include "Keys.hxx"
#include "Table.hxx"

#include "TConnection.hxx"

#include <comphelper/sequence.hxx>
#include <connectivity/dbtools.hxx>

#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;

using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;

Table::Table(Tables* pTables,
             Mutex& rMutex,
             const uno::Reference< XConnection >& rConnection):
    OTableHelper(pTables,
                 rConnection,
                 true),
    m_rMutex(rMutex),
    m_nPrivileges(0)
{
    construct();
}

Table::Table(Tables* pTables,
             Mutex& rMutex,
             const uno::Reference< XConnection >& rConnection,
             const OUString& rName,
             const OUString& rType,
             const OUString& rDescription):
    OTableHelper(pTables,
                 rConnection,
                 true,
                 rName,
                 rType,
                 rDescription,
                 "",
                 ""),
    m_rMutex(rMutex),
    m_nPrivileges(0)
{
    construct();
}

void Table::construct()
{
    OTableHelper::construct();
    if (!isNew())
    {
        // TODO: get privileges when in non-embedded mode.
        m_nPrivileges = Privilege::DROP         |
                        Privilege::REFERENCE    |
                        Privilege::ALTER        |
                        Privilege::CREATE       |
                        Privilege::READ         |
                        Privilege::DELETE       |
                        Privilege::UPDATE       |
                        Privilege::INSERT       |
                        Privilege::SELECT;
        registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRIVILEGES),
                         PROPERTY_ID_PRIVILEGES,
                         PropertyAttribute::READONLY,
                         &m_nPrivileges,
                         cppu::UnoType<decltype(m_nPrivileges)>::get());
    }
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
    return new Keys(this,
                    m_rMutex,
                    rNames);
}

OCollection* Table::createIndexes(const TStringVector& rNames)
{
    return new Indexes(this,
                       m_rMutex,
                       rNames);
}

//----- XAlterTable -----------------------------------------------------------
void SAL_CALL Table::alterColumnByName(const OUString& rColName,
                                       const uno::Reference< XPropertySet >& rDescriptor)
    throw(SQLException, NoSuchElementException, RuntimeException, std::exception)
{
    MutexGuard aGuard(m_rMutex);
    checkDisposed(WeakComponentImplHelperBase::rBHelper.bDisposed);

    uno::Reference< XPropertySet > xColumn(m_pColumns->getByName(rColName), UNO_QUERY);

    // sdbcx::Descriptor
    const bool bNameChanged = xColumn->getPropertyValue("Name") != rDescriptor->getPropertyValue("Name");
    // sdbcx::ColumnDescriptor
    const bool bTypeChanged = xColumn->getPropertyValue("Type") != rDescriptor->getPropertyValue("Type");
    const bool bTypeNameChanged = xColumn->getPropertyValue("TypeName") != rDescriptor->getPropertyValue("TypeName");
    const bool bPrecisionChanged = xColumn->getPropertyValue("Precision") != rDescriptor->getPropertyValue("Precision");
    const bool bScaleChanged = xColumn->getPropertyValue("Scale") != rDescriptor->getPropertyValue("Scale");
    const bool bIsNullableChanged = xColumn->getPropertyValue("IsNullable") != rDescriptor->getPropertyValue("IsNullable");
    const bool bIsAutoIncrementChanged = xColumn->getPropertyValue("IsAutoIncrement") != rDescriptor->getPropertyValue("IsAutoIncrement");
    // TODO: remainder -- these are all "optional" so have to detect presence and change.

    bool bDefaultChanged = xColumn->getPropertyValue("DefaultValue")
                                     != rDescriptor->getPropertyValue("DefaultValue");

    // TODO: quote identifiers as needed.
    if (bNameChanged)
    {
        OUString sNewTableName;
        rDescriptor->getPropertyValue("Name") >>= sNewTableName;
        OUString sSql(getAlterTableColumn(rColName)
                                            + " TO \"" + sNewTableName + "\"");

        getConnection()->createStatement()->execute(sSql);
    }

    if (bTypeChanged || bTypeNameChanged || bPrecisionChanged || bScaleChanged)
    {
        // If bPrecisionChanged this will only succeed if we have increased the
        // precision, otherwise an exception is thrown -- however the base
        // gui then offers to delete and recreate the column.
        OUString sSql(getAlterTableColumn(rColName) + "TYPE " +
                ::dbtools::createStandardTypePart(rDescriptor, getConnection()));
        getConnection()->createStatement()->execute(sSql);
        // TODO: could cause errors e.g. if incompatible types, deal with them here as appropriate.
        // possibly we have to wrap things in Util::evaluateStatusVector.
    }

    if (bIsNullableChanged)
    {
        sal_Int32 nNullabble = 0;
        rDescriptor->getPropertyValue("IsNullable") >>= nNullabble;

        if (nNullabble != ColumnValue::NULLABLE_UNKNOWN)
        {

            OUString sSql;
            // Dirty hack: can't change null directly in sql, we have to fiddle
            // the system tables manually.
            if (nNullabble == ColumnValue::NULLABLE)
            {
                sSql = "UPDATE RDB$RELATION_FIELDS SET RDB$NULL_FLAG = NULL "
                       "WHERE RDB$FIELD_NAME = '" + rColName + "' "
                       "AND RDB$RELATION_NAME = '" + getName() + "'";
            }
            else if (nNullabble == ColumnValue::NO_NULLS)
            {
                // And if we are making NOT NULL then we have to make sure we have
                // no nulls left in the column.
                OUString sFillNulls("UPDATE \"" + getName() + "\" SET \""
                                    + rColName + "\" = 0 "
                                    "WHERE \"" + rColName + "\" IS NULL");
                getConnection()->createStatement()->execute(sFillNulls);

                sSql = "UPDATE RDB$RELATION_FIELDS SET RDB$NULL_FLAG = 1 "
                       "WHERE RDB$FIELD_NAME = '" + rColName + "' "
                       "AND RDB$RELATION_NAME = '" + getName() + "'";
            }
            getConnection()->createStatement()->execute(sSql);
        }
        else
        {
            SAL_WARN("connectivity.firebird", "Attempting to set Nullable to NULLABLE_UNKNOWN");
        }
    }

    if (bIsAutoIncrementChanged)
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
}

// ----- XRename --------------------------------------------------------------
void SAL_CALL Table::rename(const OUString& rName)
    throw(SQLException, ElementExistException, RuntimeException, std::exception)
{
    (void) rName;
    throw RuntimeException(); // Firebird doesn't support this.
}

// ----- XInterface -----------------------------------------------------------
Any SAL_CALL Table::queryInterface(const Type& rType)
    throw(RuntimeException, std::exception)
{
    if (rType.getTypeName() == "com.sun.star.sdbcx.XRename")
        return Any();

    return OTableHelper::queryInterface(rType);
}

// ----- XTypeProvider --------------------------------------------------------
uno::Sequence< Type > SAL_CALL Table::getTypes()
    throw(RuntimeException, std::exception)
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
