/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Tables.hxx"

using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;
using namespace ::cppu;
using namespace ::rtl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

Tables::Tables(::cppu::OWeakObject& rParent,
               ::osl::Mutex& rMutex,
               const TStringVector& rVector) :
    OCollection(rParent, sal_True, rMutex, rVector)
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
    (void) rName;
    // TODO: IMPLEMENT ME
    return ObjectType();
}

//----- XDrop ----------------------------------------------------------------
void SAL_CALL Tables::dropByName(const OUString& rName)
    throw (SQLException, NoSuchElementException, RuntimeException)
{
    (void) rName;
    // TODO: IMPLEMENT ME
}

void SAL_CALL Tables::dropByIndex(const sal_Int32 nIndex)
    throw (SQLException, IndexOutOfBoundsException, RuntimeException)
{
    (void) nIndex;
    // TODO: IMPLEMENT ME
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */