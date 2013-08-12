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

#include <com/sun/star/sdbc/XRow.hpp>

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;
using namespace ::cppu;
using namespace ::osl;
using namespace ::rtl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;


Tables::Tables(const uno::Reference< XDatabaseMetaData >& rMetaData,
               OWeakObject& rParent,
               Mutex& rMutex,
               TStringVector& rNames) :
    OCollection(rParent,
                sal_True,
                rMutex,
                rNames),
    m_rMutex(rMutex),
    m_xMetaData(rMetaData)
{
}

//----- OCollection -----------------------------------------------------------
void Tables::impl_refresh()
    throw(RuntimeException)
{
    // TODO: IMPLEMENT ME
}

ObjectType Tables::createObject(const OUString& rName)
{
    // TODO: parse the name.
    // TODO: use table types
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

// //----- XDrop ----------------------------------------------------------------
// void SAL_CALL Tables::dropByName(const OUString& rName)
//     throw (SQLException, NoSuchElementException, RuntimeException)
// {
//     (void) rName;
//     // TODO: IMPLEMENT ME
// }
//
// void SAL_CALL Tables::dropByIndex(const sal_Int32 nIndex)
//     throw (SQLException, IndexOutOfBoundsException, RuntimeException)
// {
//     (void) nIndex;
//     // TODO: IMPLEMENT ME
// }
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */