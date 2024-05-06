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

#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>

#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

connectivity::mysqlc::Table::Table(
    Tables* pTables, osl::Mutex& rMutex,
    const css::uno::Reference<OMetaConnection::XConnection>& rConnection)
    : OTableHelper(pTables, rConnection, true)
    , m_rMutex(rMutex)
    , m_nPrivileges(0)
{
    construct();
}

connectivity::mysqlc::Table::Table(
    Tables* pTables, osl::Mutex& rMutex,
    const css::uno::Reference<OMetaConnection::XConnection>& rConnection, const OUString& rCatalog,
    const OUString& rSchema, const OUString& rName, const OUString& rType,
    const OUString& rDescription)
    : OTableHelper(pTables, rConnection, true, rName, rType, rDescription, rSchema, rCatalog)
    , m_rMutex(rMutex)
    , m_nPrivileges(0)
{
    construct();
}

void connectivity::mysqlc::Table::construct()
{
    OTableHelper::construct();
    if (isNew())
        return;

    // TODO: get privileges when in non-embedded mode.
    m_nPrivileges = css::sdbcx::Privilege::DROP | css::sdbcx::Privilege::REFERENCE
                    | css::sdbcx::Privilege::ALTER | css::sdbcx::Privilege::CREATE
                    | css::sdbcx::Privilege::READ | css::sdbcx::Privilege::DELETE
                    | css::sdbcx::Privilege::UPDATE | css::sdbcx::Privilege::INSERT
                    | css::sdbcx::Privilege::SELECT;
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRIVILEGES),
                     PROPERTY_ID_PRIVILEGES, css::beans::PropertyAttribute::READONLY,
                     &m_nPrivileges, cppu::UnoType<decltype(m_nPrivileges)>::get());
}
//----- OTableHelper ---------------------------------------------------------
connectivity::sdbcx::OCollection*
connectivity::mysqlc::Table::createColumns(const ::std::vector<OUString>& rNames)
{
    return new Columns(*this, m_rMutex, rNames);
}

connectivity::sdbcx::OCollection*
connectivity::mysqlc::Table::createKeys(const ::std::vector<OUString>& rNames)
{
    return new Keys(this, m_rMutex, rNames);
}

connectivity::sdbcx::OCollection*
connectivity::mysqlc::Table::createIndexes(const ::std::vector<OUString>& rNames)
{
    return new Indexes(this, m_rMutex, rNames);
}

//----- XAlterTable -----------------------------------------------------------
void SAL_CALL connectivity::mysqlc::Table::alterColumnByName(
    const OUString& rColName, const css::uno::Reference<XPropertySet>& rDescriptor)
{
    osl::MutexGuard aGuard(m_rMutex);
    checkDisposed(WeakComponentImplHelperBase::rBHelper.bDisposed);

    css::uno::Reference<XPropertySet> xColumn(m_xColumns->getByName(rColName), css::uno::UNO_QUERY);

    // sdbcx::Descriptor
    const bool bNameChanged
        = xColumn->getPropertyValue(u"Name"_ustr) != rDescriptor->getPropertyValue(u"Name"_ustr);
    // sdbcx::ColumnDescriptor
    const bool bTypeChanged
        = xColumn->getPropertyValue(u"Type"_ustr) != rDescriptor->getPropertyValue(u"Type"_ustr);
    const bool bTypeNameChanged
        = !comphelper::getString(xColumn->getPropertyValue(u"TypeName"_ustr))
               .equalsIgnoreAsciiCase(
                   comphelper::getString(rDescriptor->getPropertyValue(u"TypeName"_ustr)));
    const bool bPrecisionChanged = xColumn->getPropertyValue(u"Precision"_ustr)
                                   != rDescriptor->getPropertyValue(u"Precision"_ustr);
    const bool bScaleChanged
        = xColumn->getPropertyValue(u"Scale"_ustr) != rDescriptor->getPropertyValue(u"Scale"_ustr);

    const bool bIsNullableChanged = xColumn->getPropertyValue(u"IsNullable"_ustr)
                                    != rDescriptor->getPropertyValue(u"IsNullable"_ustr);

    const bool bIsAutoIncrementChanged = xColumn->getPropertyValue(u"IsAutoIncrement"_ustr)
                                         != rDescriptor->getPropertyValue(u"IsAutoIncrement"_ustr);

    // there's also DefaultValue but not related to database directly, it seems completely internal to LO
    // so no need to test it
    // TODO: remainder -- these are all "optional" so have to detect presence and change.
    if (bTypeChanged || bTypeNameChanged || bPrecisionChanged || bScaleChanged || bIsNullableChanged
        || bIsAutoIncrementChanged)
    {
        // If bPrecisionChanged this will only succeed if we have increased the
        // precision, otherwise an exception is thrown -- however the base
        // gui then offers to delete and recreate the column.
        OUStringBuffer sSql(300);
        sSql.append(getAlterTableColumnPart() + " MODIFY COLUMN `" + rColName + "` "
                    + ::dbtools::createStandardTypePart(rDescriptor, getConnection()));

        if (comphelper::getBOOL(rDescriptor->getPropertyValue(u"IsAutoIncrement"_ustr)))
            sSql.append(" auto_increment");

        // see ColumnValue: NO_NULLS = 0, NULLABLE = 1, NULLABLE_UNKNOWN
        // so entry required = yes corresponds to NO_NULLS = 0 and only in this case
        // NOT NULL
        if (comphelper::getINT32(rDescriptor->getPropertyValue(u"IsNullable"_ustr)) == 0)
            sSql.append(" NOT NULL");

        getConnection()->createStatement()->execute(sSql.makeStringAndClear());
    }

    if (bNameChanged)
    {
        OUString sNewColName;
        rDescriptor->getPropertyValue(u"Name"_ustr) >>= sNewColName;
        OUString sSql(getAlterTableColumnPart() + " RENAME COLUMN `" + rColName + "` TO `"
                      + sNewColName + "`");

        getConnection()->createStatement()->execute(sSql);
    }

    m_xColumns->refresh();
}

void SAL_CALL connectivity::mysqlc::Table::alterColumnByIndex(
    sal_Int32 index, const css::uno::Reference<css::beans::XPropertySet>& descriptor)
{
    osl::MutexGuard aGuard(m_rMutex);
    css::uno::Reference<XPropertySet> xColumn(m_xColumns->getByIndex(index),
                                              css::uno::UNO_QUERY_THROW);
    alterColumnByName(comphelper::getString(xColumn->getPropertyValue(
                          OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))),
                      descriptor);
}

OUString connectivity::mysqlc::Table::getAlterTableColumnPart() const
{
    return "ALTER TABLE "
           + ::dbtools::composeTableName(getMetaData(), m_CatalogName, m_SchemaName, m_Name, true,
                                         ::dbtools::EComposeRule::InTableDefinitions);
}

OUString connectivity::mysqlc::Table::getRenameStart() const { return u"RENAME TABLE "_ustr; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
