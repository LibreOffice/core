/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Columns.hxx"

#include <connectivity/sdbcx/VColumn.hxx>

#include <com/sun/star/sdbc/XRow.hpp>

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;

using namespace ::cppu;
using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

Columns::Columns(Table& rTable,
                 Mutex& rMutex,
                 const TStringVector& rVector):
    OColumnsHelper(rTable,
                   sal_True, // TODO: is this case sensitivity?
                   rMutex,
                   rVector)
{
    OColumnsHelper::setParent(&rTable);
}

ObjectType Columns::createObject(const OUString& rColumnName)
{
    uno::Reference< XResultSet > xColumns = m_pTable->getConnection()->getMetaData()->getColumns(
                                    Any(),
                                    "",
                                    m_pTable->getName(),
                                    rColumnName);

    uno::Reference< XRow > xRow(xColumns, UNO_QUERY);

    if(!xColumns.is() || !xRow.is() || !xColumns->next())
        throw RuntimeException();

    ObjectType xColumn(new OColumn(rColumnName,          // Name
                                   xRow->getString(6),   // Type Name
                                   xRow->getString(13),  // Default Value
                                   xRow->getString(12),  // Description
                                   xRow->getInt(11),     // Nullable
                                   xRow->getInt(7),      // Precision
                                   xRow->getInt(9),      // Scale
                                   xRow->getInt(5),      // Type
                                   sal_False,            // TODO: AutoIncrement
                                   // Might need a call to getTypes to determine autoincrement
                                   sal_False,            // TODO: IsRowVersion?
                                   sal_False,            // IsCurrency -- same as autoincrement
                                   sal_True,             // Case sensitivity: yes
                                   "",
                                   "",
                                   m_pTable->getName()));

    if (xColumns->next())
        throw RuntimeException(); // There should only be one column retrieved

    return xColumn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */