/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mysqlc_columns.hxx"
#include "mysqlc_column.hxx"

connectivity::mysqlc::Columns::Columns(Table& rTable, osl::Mutex& rMutex,
                                       const ::std::vector<OUString>& rVector)
    : OColumnsHelper(rTable,
                     true, // case sensitivity
                     rMutex, rVector,
                     /*bUseHardRef*/ true)
{
    OColumnsHelper::setParent(&rTable);
}

css::uno::Reference<css::beans::XPropertySet> connectivity::mysqlc::Columns::createDescriptor()
{
    return new Column;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
