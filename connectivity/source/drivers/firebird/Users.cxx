/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "User.hxx"
#include "Users.hxx"

#include <connectivity/dbtools.hxx>

#include <com/sun/star/sdbc/XRow.hpp>

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;
using namespace ::cppu;
using namespace ::osl;
using namespace ::rtl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;


Users::Users(const uno::Reference< XDatabaseMetaData >& rMetaData,
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
void Users::impl_refresh()
    throw(RuntimeException)
{
    // TODO: IMPLEMENT ME
}

ObjectType Users::createObject(const OUString& rName)
{
    (void) rName;
    // TODO: set query string
    OUString sSql;
    uno::Reference< XResultSet > xUsers = m_xMetaData->getConnection()
                                            ->createStatement()->executeQuery(sSql);

    if (!xUsers.is())
        throw RuntimeException();

    uno::Reference< XRow > xRow(xUsers,UNO_QUERY);

    if (!xRow.is() || !xUsers->next())
        throw RuntimeException();

    ObjectType xRet(new User(m_xMetaData->getConnection(),
                              xRow->getString(1))); // Name

    if (xUsers->next())
        throw RuntimeException(); // Only one user should be returned

    return xRet;
}

uno::Reference< XPropertySet > Users::createDescriptor()
{
    // There is some internal magic so that the same class can be used as either
    // a descriptor or as a normal user. See VUser.cxx for the details. In our
    // case we just need to ensure we use the correct constructor.
    return new User(m_xMetaData->getConnection());
}

//----- XAppend ---------------------------------------------------------------
ObjectType Users::appendObject(const OUString& rName,
                                const uno::Reference< XPropertySet >& rDescriptor)
{
    // TODO: set sSql as appropriate
    (void) rName;
    (void) rDescriptor;
    OUString sSql;
    m_xMetaData->getConnection()->createStatement()->execute(sSql);

    return createObject(rName);
}

//----- XDrop -----------------------------------------------------------------
void Users::dropObject(sal_Int32 nPosition, const OUString sName)
{
    uno::Reference< XPropertySet > xUser(getObject(nPosition));

    if (!ODescriptor::isNew(xUser))
    {
        (void) sName;
        // TODO: drop me
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */