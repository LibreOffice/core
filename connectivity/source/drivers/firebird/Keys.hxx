/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITY_FIREBIRD_KEYS_HXX
#define CONNECTIVITY_FIREBIRD_KEYS_HXX

#include <connectivity/TKeys.hxx>

namespace connectivity
{

    namespace firebird
    {

        class Table;

        class Keys: public ::connectivity::OKeysHelper
        {
        private:
            Table* m_pTable;

        public:
            Keys(Table* pTable,
                 ::osl::Mutex& rMutex,
                 const ::connectivity::TStringVector& rNames);

        // OKeysHelper / XDrop
        void dropObject(sal_Int32 nPosition, const ::rtl::OUString sName);

        };
    }
}
#endif // CONNECTIVITY_FIREBIRD_KEYS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */