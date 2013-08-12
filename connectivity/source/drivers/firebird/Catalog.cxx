/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Catalog.hxx"

using namespace ::connectivity::firebird;

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbc;

Catalog::Catalog(const uno::Reference< XConnection >& rConnection):
    OCatalog(rConnection),
    m_xConnection(rConnection)
{
}

//----- OCatalog -------------------------------------------------------------
void Catalog::refreshTables()
{
    // TODO: implement me.
    // Sets m_pTables (OCatalog)
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