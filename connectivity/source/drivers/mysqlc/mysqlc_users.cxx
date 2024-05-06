/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <utility>

#include "mysqlc_user.hxx"
#include "mysqlc_users.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <TConnection.hxx>

using namespace ::connectivity;
using namespace ::connectivity::mysqlc;
using namespace ::connectivity::sdbcx;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

Users::Users(const uno::Reference<XDatabaseMetaData>& rMetaData, OWeakObject& rParent,
             Mutex& rMutex, ::std::vector<OUString> const& rNames)
    : OCollection(rParent, true, rMutex, rNames)
    , m_xMetaData(rMetaData)
{
}

//----- OCollection -----------------------------------------------------------
void Users::impl_refresh()
{
    // TODO: IMPLEMENT ME
}

ObjectType Users::createObject(const OUString& rName)
{
    return new OUserExtend(m_xMetaData->getConnection(), rName);
}

uno::Reference<XPropertySet> Users::createDescriptor()
{
    // There is some internal magic so that the same class can be used as either
    // a descriptor or as a normal user. See VUser.cxx for the details. In our
    // case we just need to ensure we use the correct constructor.
    return new OUserExtend(m_xMetaData->getConnection(), u""_ustr);
}

//----- XAppend ---------------------------------------------------------------
ObjectType Users::appendObject(const OUString& rName,
                               const uno::Reference<XPropertySet>& descriptor)
{
    OUString aSql(u"GRANT USAGE ON * TO "_ustr);
    OUString aQuote = m_xMetaData->getIdentifierQuoteString();
    aSql += ::dbtools::quoteName(aQuote, rName) + " @\"%\" ";
    OUString sPassword;
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD))
        >>= sPassword;
    if (!sPassword.isEmpty())
    {
        aSql += " IDENTIFIED BY '" + sPassword + "'";
    }

    Reference<XStatement> statement = m_xMetaData->getConnection()->createStatement();
    if (statement.is())
        statement->execute(aSql);
    ::comphelper::disposeComponent(statement);

    return createObject(rName);
}

//----- XDrop -----------------------------------------------------------------
void Users::dropObject(sal_Int32 nPosition, const OUString& rName)
{
    uno::Reference<XPropertySet> xUser(getObject(nPosition));

    if (!ODescriptor::isNew(xUser))
    {
        Reference<XStatement> statement = m_xMetaData->getConnection()->createStatement();
        if (statement.is())
        {
            statement->execute("DROP USER " + rName);
            ::comphelper::disposeComponent(statement);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
