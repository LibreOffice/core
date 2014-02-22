/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "User.hxx"

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace  ::connectivity::sdbcx;

using namespace ::rtl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbc;

User::User(const uno::Reference< XConnection >& rConnection):
    OUser(sal_True) 
{
    (void) rConnection;
}

User::User(const uno::Reference< XConnection >& rConnection, const OUString& rName):
    OUser(rName,
          sal_True) 
{
    (void) rConnection;
}


void User::refreshGroups()
{
    
}