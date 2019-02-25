/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_USERS_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_USERS_HXX

#include "DatabaseMetaData.hxx"

#include <connectivity/sdbcx/VCollection.hxx>

namespace connectivity
{
    namespace firebird
    {

        /**
         * This implements com.sun.star.sdbcx.Container.
         */
        class Users: public ::connectivity::sdbcx::OCollection
        {
            css::uno::Reference< css::sdbc::XDatabaseMetaData >
                m_xMetaData;
        protected:

            // OCollection
            virtual void impl_refresh() override;
            virtual ::connectivity::sdbcx::ObjectType createObject(
                                                const OUString& rName) override;
            virtual css::uno::Reference< css::beans::XPropertySet >
                    createDescriptor() override;
            virtual ::connectivity::sdbcx::ObjectType appendObject(
                        const OUString& rName,
                        const css::uno::Reference< css::beans::XPropertySet >& rDescriptor) override;

        public:
            Users(const css::uno::Reference< css::sdbc::XDatabaseMetaData >& rMetaData,
                  ::cppu::OWeakObject& rParent,
                  ::osl::Mutex& rMutex,
                  ::std::vector< OUString> const & rNames);

            // TODO: we should also implement XDataDescriptorFactory, XRefreshable,
            // XAppend,  etc., but all are optional.

            // XDrop
            virtual void dropObject(sal_Int32 nPosition, const OUString& rName) override;

        };

    } // namespace firebird
} // namespace connectivity


#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_USERS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
