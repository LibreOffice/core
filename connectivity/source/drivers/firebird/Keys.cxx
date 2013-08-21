/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Keys.hxx"
#include "Table.hxx"

#include <connectivity/dbtools.hxx>

using namespace ::connectivity;
using namespace ::connectivity::firebird;

using namespace ::dbtools;
using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

Keys::Keys(Table* pTable, Mutex& rMutex, const TStringVector& rNames):
    OKeysHelper(pTable,
                rMutex,
                rNames),
    m_pTable(pTable)
{
}

//----- XDrop ----------------------------------------------------------------
void Keys::dropObject(sal_Int32 nPosition, const OUString sName)
{
    Reference< XConnection> xConnection = m_pTable->getConnection();

    if (!m_pTable->isNew())
    {
        uno::Reference<XPropertySet> xKey(getObject(nPosition), UNO_QUERY);

        if (xKey.is())
        {
            const OUString sQuote = m_pTable->getConnection()->getMetaData()
                                                    ->getIdentifierQuoteString();

            OUString sSql("ALTER TABLE " + quoteName(sQuote, m_pTable->getName())
                             + " DROP CONSTRAINT " + quoteName(sQuote, sName));

            m_pTable->getConnection()->createStatement()->execute(sSql);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */