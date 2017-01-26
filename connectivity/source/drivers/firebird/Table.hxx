/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_TABLE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_TABLE_HXX

#include "Tables.hxx"

#include <connectivity/TTableHelper.hxx>

namespace connectivity
{
    namespace firebird
    {

        /**
         * Implements sdbcx.Table. We don't support table renaming (XRename)
         * hence the appropriate methods are overridden.
         */
        class Table: public OTableHelper
        {
        private:
            ::osl::Mutex& m_rMutex;
            sal_Int32 m_nPrivileges;

            /**
             * Get the ALTER TABLE [TABLE] ALTER [COLUMN] String.
             * Includes a trailing space.
             */
            ::rtl::OUString getAlterTableColumn(const ::rtl::OUString& rColumn);

        protected:
            void construct() override;

        public:
            Table(Tables* pTables,
                  ::osl::Mutex& rMutex,
                  const css::uno::Reference< css::sdbc::XConnection >& _xConnection);
            Table(Tables* pTables,
                  ::osl::Mutex& rMutex,
                  const css::uno::Reference< css::sdbc::XConnection >& _xConnection,
                  const ::rtl::OUString& rName,
                  const ::rtl::OUString& rType,
                  const ::rtl::OUString& rDescription);

            // OTableHelper
            virtual ::connectivity::sdbcx::OCollection* createColumns(
                const ::connectivity::TStringVector& rNames) override;
            virtual ::connectivity::sdbcx::OCollection* createKeys(
                const ::connectivity::TStringVector& rNames) override;
            virtual ::connectivity::sdbcx::OCollection* createIndexes(
                const ::connectivity::TStringVector& rNames) override;

            // XAlterTable
            /**
             * See css::sdbcx::ColumnDescriptor for details of
             * rDescriptor.
             */
            virtual void SAL_CALL alterColumnByName(
                    const ::rtl::OUString& rColName,
                    const css::uno::Reference< css::beans::XPropertySet >& rDescriptor) override;

            // XRename -- UNSUPPORTED
            virtual void SAL_CALL rename(const ::rtl::OUString& sName) override;

            //XInterface
            virtual css::uno::Any
                    SAL_CALL queryInterface(const css::uno::Type & rType) override;

            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type >
                    SAL_CALL getTypes() override;

        };

    } // namespace firebird
} // namespace connectivity

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_TABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
