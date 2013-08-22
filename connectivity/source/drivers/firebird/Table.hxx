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

        /**
         * Implements sdbcx.Table. We don't support table renaming (XRename)
         * hence the appropriate methods are overriden.
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
            void construct();

        public:
            Table(Tables* pTables,
                  ::osl::Mutex& rMutex,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);
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

            // XRename -- UNSUPPORTED
            virtual void SAL_CALL rename(const ::rtl::OUString& sName)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::container::ElementExistException,
                      ::com::sun::star::uno::RuntimeException);

            //XInterface
            virtual ::com::sun::star::uno::Any
                    SAL_CALL queryInterface(const ::com::sun::star::uno::Type & rType)
                throw(::com::sun::star::uno::RuntimeException);

            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
                    SAL_CALL getTypes()
                throw(::com::sun::star::uno::RuntimeException);

        };

    } // namespace firebird
} // namespace connectivity

#endif // CONNECTIVITY_FIREBIRD_TABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */