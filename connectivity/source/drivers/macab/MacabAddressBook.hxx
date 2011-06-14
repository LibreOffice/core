/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_MACAB_ADDRESSBOOK_HXX_
#define _CONNECTIVITY_MACAB_ADDRESSBOOK_HXX_

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
