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
                  const ::rtl::OUString& rDescription);

            // OTableHelper
            virtual ::connectivity::sdbcx::OCollection* createColumns(
                const ::connectivity::TStringVector& rNames);
            virtual ::connectivity::sdbcx::OCollection* createKeys(
                const ::connectivity::TStringVector& rNames);
            virtual ::connectivity::sdbcx::OCollection* createIndexes(
                const ::connectivity::TStringVector& rNames);

            // XAlterTable
            /**
             * See ::com::sun::star::sdbcx::ColumnDescriptor for details of
             * rDescriptor.
             */
            virtual void SAL_CALL alterColumnByName(
                    const ::rtl::OUString& rColName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rDescriptor)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::container::NoSuchElementException,
                      ::com::sun::star::uno::RuntimeException);

        };

    } // namespace firebird
} // namespace connectivity

#endif // CONNECTIVITY_FIREBIRD_TABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */