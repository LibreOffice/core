/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITY_FIREBIRD_COLUMNS_HXX
#define CONNECTIVITY_FIREBIRD_COLUMNS_HXX

#include "Table.hxx"

#include <connectivity/TColumnsHelper.hxx>

namespace connectivity
{
    namespace firebird
    {
        class Columns: public ::connectivity::OColumnsHelper
        {
        public:
            Columns(Table& rTable,
                    ::osl::Mutex& rMutex,
                    const ::connectivity::TStringVector &_rVector);

            // OColumnsHelper
            virtual ::connectivity::sdbcx::ObjectType createObject(
                                                        const ::rtl::OUString& rName);
        };

    } // namespace firebird
} // namespace connectivity


#endif // CONNECTIVITY_FIREBIRD_COLUMNS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */