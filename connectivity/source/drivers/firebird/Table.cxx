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

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;

using namespace ::osl;
using namespace ::rtl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbc;

Table::Table(Tables* pTables,
             Mutex& rMutex,
             const uno::Reference< XConnection >& rConnection,
             const OUString& rName,
             const OUString& rType,
             const OUString& rDescription,
             sal_Int32 nPrivileges):
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
    (void) nPrivileges;
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
    (void) rNames;
    // TODO: IMPLEMENT ME
    return 0;
}

OCollection* Table::createIndexes(const TStringVector& rNames)
{
    (void) rNames;
    // TODO: IMPLEMENT ME
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */