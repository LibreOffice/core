/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mysqlc_table.hxx"
#include "mysqlc_tables.hxx"
#include "mysqlc_catalog.hxx"

#include <TConnection.hxx>

#include <connectivity/dbtools.hxx>

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <comphelper/types.hxx>

using namespace ::connectivity;
using namespace ::connectivity::mysqlc;
using namespace ::connectivity::sdbcx;
using namespace ::cppu;
using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;

//----- OCollection -----------------------------------------------------------
void Tables::impl_refresh() { static_cast<Catalog&>(m_rParent).refreshTables(); }

static void lcl_unescape(OUString& rName)
{
    // Remove ending ` if there's one
    sal_Int32 nLastIndexBacktick = rName.lastIndexOf("`");
    if ((nLastIndexBacktick > 0) && (nLastIndexBacktick == (rName.getLength() - 1)))
    {
        rName = rName.copy(0, nLastIndexBacktick);
    }
    // Remove beginning ` and replace double ` by simple `
    rName = rName.replaceFirst("`", "").replaceAll("``", "`");
}

ObjectType Tables::createObject(const OUString& rName)
{
    OUString sCatalog, sSchema, sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData, rName, sCatalog, sSchema, sTable,
                                       ::dbtools::EComposeRule::InDataManipulation);

    Any aCatalog;
    if (!sCatalog.isEmpty())
    {
        aCatalog <<= sCatalog;
        lcl_unescape(sCatalog);
    }

    lcl_unescape(sSchema);
    lcl_unescape(sTable);

    // Only retrieving a single table, so table type is irrelevant (param 4)
    uno::Reference<XResultSet> xTables
        = m_xMetaData->getTables(aCatalog, sSchema, sTable, uno::Sequence<OUString>());

    if (!xTables.is())
        throw RuntimeException("Could not acquire table.");

    uno::Reference<XRow> xRow(xTables, UNO_QUERY_THROW);

    if (!xTables->next())
        throw RuntimeException();

    ObjectType xRet(new Table(this, m_rMutex, m_xMetaData->getConnection(),
                              xRow->getString(1), // Catalog
                              xRow->getString(2), // Schema
                              xRow->getString(3), // Name
                              xRow->getString(4), // Type
                              xRow->getString(5))); // Description / Remarks / Comments

    if (xTables->next())
        throw RuntimeException("Found more tables than expected.");

    return xRet;
}

OUString Tables::createStandardColumnPart(const Reference<XPropertySet>& xColProp,
                                          const Reference<XConnection>& _xConnection)
{
    // TODO test
    Reference<XDatabaseMetaData> xMetaData = _xConnection->getMetaData();

    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();

    bool bIsAutoIncrement = false;
    xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT))
        >>= bIsAutoIncrement;

    const OUString sQuoteString = xMetaData->getIdentifierQuoteString();
    OUStringBuffer aSql(
        ::dbtools::quoteName(sQuoteString, ::comphelper::getString(xColProp->getPropertyValue(
                                               rPropMap.getNameByIndex(PROPERTY_ID_NAME)))));

    // check if the user enter a specific string to create autoincrement values
    OUString sAutoIncrementValue;
    Reference<XPropertySetInfo> xPropInfo = xColProp->getPropertySetInfo();

    if (xPropInfo.is()
        && xPropInfo->hasPropertyByName(rPropMap.getNameByIndex(PROPERTY_ID_AUTOINCREMENTCREATION)))
        xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_AUTOINCREMENTCREATION))
            >>= sAutoIncrementValue;

    aSql.append(" ");

    aSql.append(dbtools::createStandardTypePart(xColProp, _xConnection));
    // Add character set for (VAR)BINARY (fix) types:
    // (VAR) BINARY is distinguished from other CHAR types by its character set.
    // Octets is a special character set for binary data.
    if (xPropInfo.is() && xPropInfo->hasPropertyByName(rPropMap.getNameByIndex(PROPERTY_ID_TYPE)))
    {
        sal_Int32 aType = 0;
        xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_TYPE)) >>= aType;
        if (aType == DataType::BINARY || aType == DataType::VARBINARY)
        {
            aSql.append(" ");
            aSql.append("CHARACTER SET OCTETS");
        }
    }

    if (bIsAutoIncrement && !sAutoIncrementValue.isEmpty())
    {
        aSql.append(" ");
        aSql.append(sAutoIncrementValue);
    }
    // AutoIncrement "IDENTITY" is implicitly "NOT NULL"
    else if (::comphelper::getINT32(
                 xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISNULLABLE)))
             == ColumnValue::NO_NULLS)
        aSql.append(" NOT NULL");

    return aSql.makeStringAndClear();
}

uno::Reference<XPropertySet> Tables::createDescriptor()
{
    // There is some internal magic so that the same class can be used as either
    // a descriptor or as a normal table. See VTable.cxx for the details. In our
    // case we just need to ensure we use the correct constructor.
    return new Table(this, m_rMutex, m_xMetaData->getConnection());
}

//----- XAppend ---------------------------------------------------------------
ObjectType Tables::appendObject(const OUString& /* rName */,
                                const uno::Reference<XPropertySet>& rDescriptor)
{
    OUString sSql(
        ::dbtools::createSqlCreateTableStatement(rDescriptor, m_xMetaData->getConnection()));
    OUString sCatalog, sSchema, sComposedName, sTable;
    const Reference<XConnection>& xConnection = m_xMetaData->getConnection();

    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();

    rDescriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)) >>= sCatalog;
    rDescriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= sSchema;
    rDescriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME)) >>= sTable;

    sComposedName = ::dbtools::composeTableName(m_xMetaData, sCatalog, sSchema, sTable, true,
                                                ::dbtools::EComposeRule::InTableDefinitions);
    if (sComposedName.isEmpty())
        ::dbtools::throwFunctionSequenceException(xConnection);

    m_xMetaData->getConnection()->createStatement()->execute(sSql);

    return createObject(sComposedName);
}

//----- XDrop -----------------------------------------------------------------
void Tables::dropObject(sal_Int32 nPosition, const OUString& sName)
{
    uno::Reference<XPropertySet> xTable(getObject(nPosition));

    if (ODescriptor::isNew(xTable))
        return;

    OUString sType;
    xTable->getPropertyValue("Type") >>= sType;

    m_xMetaData->getConnection()->createStatement()->execute("DROP " + sType + " " + sName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
