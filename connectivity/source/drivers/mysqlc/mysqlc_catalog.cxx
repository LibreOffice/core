/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mysqlc_catalog.hxx"
#include "mysqlc_tables.hxx"
#include "mysqlc_views.hxx"

connectivity::mysqlc::Catalog::Catalog(
    const css::uno::Reference<css::sdbc::XConnection>& rConnection)
    : OCatalog(rConnection)
    , m_xConnection(rConnection)
{
}

//----- OCatalog -------------------------------------------------------------
void connectivity::mysqlc::Catalog::refreshTables()
{
    css::uno::Reference<css::sdbc::XResultSet> xTables
        = m_xMetaData->getTables(css::uno::Any(), "%", "%", {});

    if (!xTables.is())
        return;

    ::std::vector<OUString> aTableNames;

    fillNames(xTables, aTableNames);

    if (!m_pTables)
        m_pTables.reset(new Tables(m_xConnection->getMetaData(), *this, m_aMutex, aTableNames));
    else
        m_pTables->reFill(aTableNames);
}

void connectivity::mysqlc::Catalog::refreshViews()
{
    css::uno::Reference<css::sdbc::XResultSet> xViews
        = m_xMetaData->getTables(css::uno::Any(), "%", "%", { "VIEW" });

    if (!xViews.is())
        return;

    ::std::vector<OUString> aViewNames;

    fillNames(xViews, aViewNames);

    if (!m_pViews)
        m_pViews.reset(new Views(m_xConnection, *this, m_aMutex, aViewNames));
    else
        m_pViews->reFill(aViewNames);
}

//----- IRefreshableGroups ---------------------------------------------------
void connectivity::mysqlc::Catalog::refreshGroups()
{
    // TODO: implement me
}

//----- IRefreshableUsers ----------------------------------------------------
void connectivity::mysqlc::Catalog::refreshUsers()
{
    // TODO: implement me
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
