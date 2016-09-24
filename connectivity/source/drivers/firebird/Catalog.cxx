/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Catalog.hxx"
#include "Tables.hxx"
#include "Users.hxx"

using namespace ::connectivity::firebird;
using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

Catalog::Catalog(const uno::Reference< XConnection >& rConnection):
    OCatalog(rConnection),
    m_xConnection(rConnection)
{
}

//----- OCatalog -------------------------------------------------------------
void Catalog::refreshTables()
{
    // TODO: set type -- currenty we also get system tables...
    Sequence< OUString > aTypes(2);
    aTypes[0] = "TABLE";
    aTypes[1] = "VIEW";

    uno::Reference< XResultSet > xTables = m_xMetaData->getTables(Any(),
                                                            "%",
                                                            "%",
                                                            aTypes);

    if (!xTables.is())
        return;

    TStringVector aTableNames;

    fillNames(xTables, aTableNames);

    if (!m_pTables)
        m_pTables = new Tables(m_xConnection->getMetaData(),
                               *this,
                               m_aMutex,
                               aTableNames);
    else
        m_pTables->reFill(aTableNames);

}

void Catalog::refreshViews()
{
    // TODO: implement me.
    // Sets m_pViews (OCatalog)
}

//----- IRefreshableGroups ---------------------------------------------------
void Catalog::refreshGroups()
{
    // TODO: implement me
}

//----- IRefreshableUsers ----------------------------------------------------
void Catalog::refreshUsers()
{
    OUString sSql("SELECT DISTINCT RDB$USER FROM RDB$USER_PRIVILEGES");

    uno::Reference< XResultSet > xUsers = m_xMetaData->getConnection()
                                            ->createStatement()->executeQuery(sSql);

    if (!xUsers.is())
        return;

    TStringVector aUserNames;

    uno::Reference< XRow > xRow(xUsers,UNO_QUERY);
    while (xUsers->next())
    {
        aUserNames.push_back(xRow->getString(1));
    }

    if (!m_pUsers)
        m_pUsers = new Users(m_xConnection->getMetaData(),
                             *this,
                             m_aMutex,
                             aUserNames);
    else
        m_pUsers->reFill(aUserNames);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
