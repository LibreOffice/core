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

using namespace ::connectivity::firebird;

using namespace ::rtl;

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
    uno::Reference< XResultSet > xTables = m_xMetaData->getTables(Any(),
                                                            "%",
                                                            "%",
                                                            Sequence< OUString >());

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
    // TODO: implement me
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */