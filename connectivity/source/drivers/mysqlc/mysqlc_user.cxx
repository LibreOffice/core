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

using namespace ::connectivity;
using namespace ::connectivity::mysqlc;
using namespace ::connectivity::sdbcx;

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbc;

User::User(css::uno::Reference<css::sdbc::XConnection> xConnection)
    : OUser(true) // Case Sensitive
    , m_xConnection(std::move(xConnection))
{
}

User::User(css::uno::Reference<css::sdbc::XConnection> xConnection, const OUString& rName)
    : OUser(rName,
            true) // Case Sensitive
    , m_xConnection(std::move(xConnection))
{
}

void User::changePassword(const OUString&, const OUString& /* newPassword */)
{
    // TODO: implement
}

sal_Int32 User::getPrivileges(const OUString&, sal_Int32)
{
    // TODO: implement.
    return 0;
}

sal_Int32 User::getGrantablePrivileges(const OUString&, sal_Int32)
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
