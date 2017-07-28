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
            css::uno::Reference< css::sdbc::XDatabaseMetaData >
                m_xMetaData;

            static OUString createStandardColumnPart(const css::uno::Reference< css::beans::XPropertySet >& xColProp,const css::uno::Reference< com::sun::star::sdbc::XConnection>& _xConnection);

            // OCollection
            virtual void impl_refresh() override;
            virtual ::connectivity::sdbcx::ObjectType createObject(
                                                const ::rtl::OUString& rName) override;
            virtual css::uno::Reference< css::beans::XPropertySet >
                    createDescriptor() override;
            virtual ::connectivity::sdbcx::ObjectType appendObject(
                        const OUString& rName,
                        const css::uno::Reference< css::beans::XPropertySet >& rDescriptor) override;

        public:
            Tables(const css::uno::Reference< css::sdbc::XDatabaseMetaData >& rMetaData,
                   ::cppu::OWeakObject& rParent,
                   ::osl::Mutex& rMutex,
                   ::connectivity::TStringVector const & rNames) : sdbcx::OCollection(rParent, true, rMutex, rNames), m_xMetaData(rMetaData) {}

            // TODO: we should also implement XDataDescriptorFactory, XRefreshable,
            // XAppend,  etc., but all are optional.

            // XDrop
            virtual void dropObject(sal_Int32 nPosition, const ::rtl::OUString& rName) override;

        };

    } // namespace firebird
} // namespace connectivity


#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_TABLES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
