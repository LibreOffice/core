/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "User.hxx"
#include "Users.hxx"

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;


Users::Users(const uno::Reference< XDatabaseMetaData >& rMetaData,
             OWeakObject& rParent,
             Mutex& rMutex,
             ::std::vector< OUString> const & rNames) :
    OCollection(rParent,
                true,
                rMutex,
                rNames),
    m_xMetaData(rMetaData)
{
}

//----- OCollection -----------------------------------------------------------
void Users::impl_refresh()
{
    // TODO: IMPLEMENT ME
}

ObjectType Users::createObject(const OUString& rName)
{
    return new User(m_xMetaData->getConnection(), rName);
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
                                const uno::Reference< XPropertySet >&)
{
    // TODO: set sSql as appropriate
    m_xMetaData->getConnection()->createStatement()->execute(OUString());

    return createObject(rName);
}

//----- XDrop -----------------------------------------------------------------
void Users::dropObject(sal_Int32 nPosition, const OUString&)
{
    uno::Reference< XPropertySet > xUser(getObject(nPosition));

    if (!ODescriptor::isNew(xUser))
    {
        // TODO: drop me
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
