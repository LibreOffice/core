/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Table.hxx"
#include "Tables.hxx"
#include "Catalog.hxx"

#include <connectivity/dbtools.hxx>

#include <com/sun/star/sdbc/XRow.hpp>

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;
using namespace ::cppu;
using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;


//----- OCollection -----------------------------------------------------------
void Tables::impl_refresh()
    throw(RuntimeException)
{
    static_cast<Catalog&>(m_rParent).refreshTables();
}

ObjectType Tables::createObject(const OUString& rName)
{
    // Only retrieving a single table, so table type is irrelevant (param 4)
    uno::Reference< XResultSet > xTables = m_xMetaData->getTables(Any(),
                                                                  OUString(),
                                                                  rName,
                                                                  uno::Sequence< OUString >());

    if (!xTables.is())
        throw RuntimeException();

    uno::Reference< XRow > xRow(xTables,UNO_QUERY);

    if (!xRow.is() || !xTables->next())
        throw RuntimeException();

    ObjectType xRet(new Table(this,
                              m_rMutex,
                              m_xMetaData->getConnection(),
                              xRow->getString(3), // Name
                              xRow->getString(4), // Type
                              xRow->getString(5))); // Description / Remarks / Comments

    if (xTables->next())
        throw RuntimeException(); // Only one table should be returned

    return xRet;
}

uno::Reference< XPropertySet > Tables::createDescriptor()
{
    // There is some internal magic so that the same class can be used as either
    // a descriptor or as a normal table. See VTable.cxx for the details. In our
    // case we just need to ensure we use the correct constructor.
    return new Table(this, m_rMutex, m_xMetaData->getConnection());
}

//----- XAppend ---------------------------------------------------------------
ObjectType Tables::appendObject(const OUString& rName,
                                const uno::Reference< XPropertySet >& rDescriptor)
{
    OUString sSql(::dbtools::createSqlCreateTableStatement(rDescriptor,
                                                            m_xMetaData->getConnection()));
    m_xMetaData->getConnection()->createStatement()->execute(sSql);

    return createObject(rName);
}

//----- XDrop -----------------------------------------------------------------
void Tables::dropObject(sal_Int32 nPosition, const OUString& sName)
{
    uno::Reference< XPropertySet > xTable(getObject(nPosition));

    if (!ODescriptor::isNew(xTable))
    {
        OUStringBuffer sSql("DROP ");

        OUString sType;
        xTable->getPropertyValue("Type") >>= sType;
        sSql.append(sType);

        const OUString sQuoteString = m_xMetaData->getIdentifierQuoteString();
        sSql.append(::dbtools::quoteName(sQuoteString,sName));

        m_xMetaData->getConnection()->createStatement()->execute(sSql.makeStringAndClear());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
