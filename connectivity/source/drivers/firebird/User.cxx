/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "User.hxx"

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbc;

User::User(const css::uno::Reference< css::sdbc::XConnection >& rConnection):
    OUser(true) // Case Sensitive
    , m_xConnection(rConnection)
{}

User::User(const css::uno::Reference< css::sdbc::XConnection >& rConnection, const OUString& rName):
    OUser(rName,
          true) // Case Sensitive
    , m_xConnection(rConnection)
{}

void User::changePassword(const OUString&, const OUString& newPassword)
{
    m_xConnection->createStatement()->execute("ALTER USER " + m_Name + " PASSWORD '" + newPassword + "'");
}

sal_Int32 User::getPrivileges(const OUString& , sal_Int32 )
{
    // TODO: implement.
    return 0;
}

sal_Int32 User::getGrantablePrivileges(const OUString& , sal_Int32 )
{
    // TODO: implement.
    return 0;
}

//----- IRefreshableGroups ----------------------------------------------------
void User::refreshGroups()
{
    // TODO: implement.
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
