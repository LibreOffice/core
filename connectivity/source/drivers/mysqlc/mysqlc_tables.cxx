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
#include <comphelper/types.hxx>

//----- OCollection -----------------------------------------------------------
void connectivity::mysqlc::Tables::impl_refresh()
{
    static_cast<Catalog&>(m_rParent).refreshTables();
}

static void lcl_unescape(OUString& rName)
{
    // Remove ending ` if there's one
    sal_Int32 nLastIndexBacktick = rName.lastIndexOf("`");
    if ((nLastIndexBacktick > 0) && (nLastIndexBacktick == (rName.getLength() - 1)))
    {
        rName = rName.copy(0, nLastIndexBacktick);
    }

    // Remove beginning `
    nLastIndexBacktick = rName.indexOf("`");
    if (nLastIndexBacktick == 0)
    {
        rName = rName.copy(1, rName.getLength() - 1);
    }

    // Replace double ` by simple `
    rName = rName.replaceAll("``", "`");
}

connectivity::sdbcx::ObjectType connectivity::mysqlc::Tables::createObject(const OUString& rName)
{
    OUString sCatalog, sSchema, sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData, rName, sCatalog, sSchema, sTable,
                                       ::dbtools::EComposeRule::InDataManipulation);

    css::uno::Any aCatalog;
    if (!sCatalog.isEmpty())
    {
        lcl_unescape(sCatalog);
        aCatalog <<= sCatalog;
    }

    lcl_unescape(sSchema);
    lcl_unescape(sTable);

    // Only retrieving a single table, so table type is irrelevant (param 4)
    css::uno::Reference<css::sdbc::XResultSet> xTables
        = m_xMetaData->getTables(aCatalog, sSchema, sTable, css::uno::Sequence<OUString>());

    if (!xTables.is())
        throw css::uno::RuntimeException(u"Could not acquire table."_ustr);

    css::uno::Reference<css::sdbc::XRow> xRow(xTables, css::uno::UNO_QUERY_THROW);

    if (!xTables->next())
        throw css::uno::RuntimeException();

    connectivity::sdbcx::ObjectType xRet(
        new Table(this, m_rMutex, m_xMetaData->getConnection(),
                  xRow->getString(1), // Catalog
                  xRow->getString(2), // Schema
                  xRow->getString(3), // Name
                  xRow->getString(4), // Type
                  xRow->getString(5))); // Description / Remarks / Comments

    if (xTables->next())
        throw css::uno::RuntimeException(u"Found more tables than expected."_ustr);

    return xRet;
}

css::uno::Reference<css::beans::XPropertySet> connectivity::mysqlc::Tables::createDescriptor()
{
    // There is some internal magic so that the same class can be used as either
    // a descriptor or as a normal table. See VTable.cxx for the details. In our
    // case we just need to ensure we use the correct constructor.
    return new Table(this, m_rMutex, m_xMetaData->getConnection());
}

//----- XAppend ---------------------------------------------------------------
void connectivity::mysqlc::Tables::createTable(
    const css::uno::Reference<css::beans::XPropertySet>& descriptor)
{
    const css::uno::Reference<css::sdbc::XConnection> xConnection = m_xMetaData->getConnection();
    OUString aSql = ::dbtools::createSqlCreateTableStatement(descriptor, xConnection);

    css::uno::Reference<css::sdbc::XStatement> xStmt = xConnection->createStatement();
    if (xStmt.is())
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}

// XAppend
connectivity::sdbcx::ObjectType connectivity::mysqlc::Tables::appendObject(
    const OUString& _rForName, const css::uno::Reference<css::beans::XPropertySet>& descriptor)
{
    createTable(descriptor);
    return createObject(_rForName);
}

void connectivity::mysqlc::Tables::appendNew(const OUString& _rsNewTable)
{
    insertElement(_rsNewTable, nullptr);

    // notify our container listeners
    css::container::ContainerEvent aEvent(static_cast<XContainer*>(this),
                                          css::uno::Any(_rsNewTable), css::uno::Any(),
                                          css::uno::Any());
    comphelper::OInterfaceIteratorHelper3 aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        aListenerLoop.next()->elementInserted(aEvent);
}

OUString
connectivity::mysqlc::Tables::getNameForObject(const connectivity::sdbcx::ObjectType& _xObject)
{
    OSL_ENSURE(_xObject.is(), "OTables::getNameForObject: Object is NULL!");
    return ::dbtools::composeTableName(m_xMetaData, _xObject,
                                       ::dbtools::EComposeRule::InDataManipulation, false)
        .replaceAll(u"`", u"̀ `");
}

//----- XDrop -----------------------------------------------------------------
void connectivity::mysqlc::Tables::dropObject(sal_Int32 nPosition, const OUString& sName)
{
    css::uno::Reference<css::beans::XPropertySet> xTable(getObject(nPosition));

    if (connectivity::sdbcx::ODescriptor::isNew(xTable))
        return;

    OUString sType;
    xTable->getPropertyValue(u"Type"_ustr) >>= sType;

    m_xMetaData->getConnection()->createStatement()->execute("DROP " + sType + " " + sName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
