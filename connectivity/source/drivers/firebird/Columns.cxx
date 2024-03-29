/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Columns.hxx"
#include "Column.hxx"

using namespace ::connectivity;
using namespace ::connectivity::firebird;

using namespace ::cppu;
using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

Columns::Columns(Table& rTable,
                 Mutex& rMutex,
                 const ::std::vector< OUString>& rVector):
    OColumnsHelper(rTable,
                   true, // TODO: is this case sensitivity?
                   rMutex,
                   rVector,
                   /*bUseHardRef*/true)
{
    OColumnsHelper::setParent(&rTable);
}

Reference< css::beans::XPropertySet > Columns::createDescriptor()
{
    return new Column;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
