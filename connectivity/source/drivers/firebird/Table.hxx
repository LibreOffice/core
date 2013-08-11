/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITY_FIREBIRD_TABLE_HXX
#define CONNECTIVITY_FIREBIRD_TABLE_HXX

#include "Tables.hxx"

#include <connectivity/TTableHelper.hxx>

namespace connectivity
{
    namespace firebird
    {

        class Table: public OTableHelper
        {
        private:
            ::osl::Mutex& m_rMutex;

        public:
            Table(Tables* pTables,
                  ::osl::Mutex& rMutex,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                  const ::rtl::OUString& rName,
                  const ::rtl::OUString& rType,
                  const ::rtl::OUString& rDescription,
                  sal_Int32 _nPrivileges);

            // OTableHelper
            virtual ::connectivity::sdbcx::OCollection* createColumns(
                const ::connectivity::TStringVector& rNames);
            virtual ::connectivity::sdbcx::OCollection* createKeys(
                const ::connectivity::TStringVector& rNames);
            virtual ::connectivity::sdbcx::OCollection* createIndexes(
                const ::connectivity::TStringVector& rNames);

        };

    } // namespace firebird
} // namespace connectivity

#endif // CONNECTIVITY_FIREBIRD_TABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */