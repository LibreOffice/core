/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_TABLES_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_TABLES_HXX

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
        protected:
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >
                m_xMetaData;

            // OCollection
            virtual void impl_refresh()
                throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
            virtual ::connectivity::sdbcx::ObjectType createObject(
                                                const ::rtl::OUString& rName) SAL_OVERRIDE;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    createDescriptor() SAL_OVERRIDE;
            virtual ::connectivity::sdbcx::ObjectType appendObject(
                        const OUString& rName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rDescriptor) SAL_OVERRIDE;

        public:
            Tables(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& rMetaData,
                   ::cppu::OWeakObject& rParent,
                   ::osl::Mutex& rMutex,
                   ::connectivity::TStringVector& rNames) : sdbcx::OCollection(rParent, true, rMutex, rNames), m_xMetaData(rMetaData) {}

            // TODO: we should also implement XDataDescriptorFactory, XRefreshable,
            // XAppend,  etc., but all are optional.

            // XDrop
            virtual void dropObject(sal_Int32 nPosition, const ::rtl::OUString& rName) SAL_OVERRIDE;

        };

    } // namespace firebird
} // namespace connectivity


#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_TABLES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
