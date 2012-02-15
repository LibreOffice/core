/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
