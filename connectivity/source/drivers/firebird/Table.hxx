/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <string_view>

#include "Tables.hxx"

#include <connectivity/TTableHelper.hxx>

namespace connectivity::firebird
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
            OUString getAlterTableColumn(std::u16string_view rColumn);

        protected:
            void construct() override;

        public:
            Table(Tables* pTables,
                  ::osl::Mutex& rMutex,
                  const css::uno::Reference< css::sdbc::XConnection >& _xConnection);
            Table(Tables* pTables,
                  ::osl::Mutex& rMutex,
                  const css::uno::Reference< css::sdbc::XConnection >& _xConnection,
                  const OUString& rName,
                  const OUString& rType,
                  const OUString& rDescription);

            // OTableHelper
            virtual ::connectivity::sdbcx::OCollection* createColumns(
                const ::std::vector< OUString>& rNames) override;
            virtual ::connectivity::sdbcx::OCollection* createKeys(
                const ::std::vector< OUString>& rNames) override;
            virtual ::connectivity::sdbcx::OCollection* createIndexes(
                const ::std::vector< OUString>& rNames) override;

            // XAlterTable
            /**
             * See css::sdbcx::ColumnDescriptor for details of
             * rDescriptor.
             */
            virtual void SAL_CALL alterColumnByName(
                    const OUString& rColName,
                    const css::uno::Reference< css::beans::XPropertySet >& rDescriptor) override;

            // XRename -- UNSUPPORTED
            virtual void SAL_CALL rename(const OUString& sName) override;

            //XInterface
            virtual css::uno::Any
                    SAL_CALL queryInterface(const css::uno::Type & rType) override;

            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type >
                    SAL_CALL getTypes() override;

        };

} // namespace connectivity::firebird

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
