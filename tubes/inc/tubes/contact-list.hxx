/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Collabora Ltd.
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef INCLUDED_TUBES_CONTACT_LIST_HXX
#define INCLUDED_TUBES_CONTACT_LIST_HXX

#include <set>
#include <utility>
#include <vector>

#include "tubes/tubesdllapi.h"

typedef struct _TpAccount TpAccount;
typedef struct _TpContact TpContact;
typedef struct _TpAccountManager TpAccountManager;

typedef ::std::pair< TpAccount *, TpContact * > AccountContactPair;
typedef ::std::vector< AccountContactPair > AccountContactPairV;

class TUBES_DLLPUBLIC ContactList
{
public:
    ContactList(TpAccountManager *pAccountManager);
    ~ContactList();

    AccountContactPairV     getContacts();

private:
    TpAccountManager*       mpAccountManager;
    std::set< TpContact* >  maRegistered;

};

#endif // INCLUDED_TUBES_CONTACT_LIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
