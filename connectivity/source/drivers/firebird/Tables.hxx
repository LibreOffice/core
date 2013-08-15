/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITY_FIREBIRD_TABLES_HXX
#define CONNECTIVITY_FIREBIRD_TABLES_HXX

#include "DatabaseMetaData.hxx"

#include <connectivity/sdbcx/VCollection.hxx>

namespace connectivity
{
    namespace firebird
    {

        /**
         * This implements com.sun.star.sdbcx.Container, which seems to be
         * also known by the name of Tables and Collection.
         */
        class Tables: public ::connectivity::sdbcx::OCollection
        {
        private:
            ::osl::Mutex& m_rMutex;

        protected:
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >
                m_xMetaData;

            // OCollection
            virtual void impl_refresh()
                throw(::com::sun::star::uno::RuntimeException);
            virtual ::connectivity::sdbcx::ObjectType createObject(
                                                const ::rtl::OUString& rName);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    createDescriptor();
            virtual ::connectivity::sdbcx::ObjectType appendObject(
                        const OUString& rName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rDescriptor);

        public:
            Tables(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& rMetaData,
                   ::cppu::OWeakObject& rParent,
                   ::osl::Mutex& rMutex,
                   ::connectivity::TStringVector& rNames);

            // TODO: we should also implement XDataDescriptorFactory, XRefreshable,
            // XAppend,  etc., but all are optional.

            // XDrop
            virtual void dropObject(sal_Int32 nPosition, const ::rtl::OUString rName);

        };

    } // namespace firebird
} // namespace connectivity


#endif // CONNECTIVITY_FIREBIRD_TABLES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */