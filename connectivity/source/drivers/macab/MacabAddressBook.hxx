/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once
#if 1

#include "MacabRecords.hxx"
#include "MacabGroup.hxx"

#include <vector>

#include <premac.h>
#include <Carbon/Carbon.h>
#include <AddressBook/ABAddressBookC.h>
#include <postmac.h>

namespace connectivity
{
    namespace macab
    {
        class MacabAddressBook
        {
            protected:
                ABAddressBookRef m_aAddressBook;
                MacabRecords *m_xMacabRecords;
                ::std::vector<MacabGroup *> m_xMacabGroups;
                sal_Bool m_bRetrievedGroups;
            private:
                void manageDuplicateGroups(::std::vector<MacabGroup *> _xGroups) const;
            public:
                MacabAddressBook();
                ~MacabAddressBook();
                static const ::rtl::OUString & getDefaultTableName();

                MacabRecords *getMacabRecords();
                ::std::vector<MacabGroup *> getMacabGroups();

                MacabGroup *getMacabGroup(::rtl::OUString _groupName);
                MacabRecords *getMacabRecords(const ::rtl::OUString _tableName);

                MacabGroup *getMacabGroupMatch(::rtl::OUString _groupName);
                MacabRecords *getMacabRecordsMatch(const ::rtl::OUString _tableName);
        };

    }
}

#endif // _CONNECTIVITY_MACAB_ADDRESSBOOK_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
