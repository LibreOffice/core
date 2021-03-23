/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sdbcx/VUser.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>

namespace connectivity::firebird
    {

        /**
         * This implements com.sun.star.sdbcx.Container.
         */
        class User: public ::connectivity::sdbcx::OUser
        {
            css::uno::Reference< css::sdbc::XConnection > m_xConnection;

        public:
            /**
             * Create a "new" descriptor, which isn't yet in the database.
             */
            User(const css::uno::Reference< css::sdbc::XConnection >& rConnection);
            /**
             * For a user that already exists in the db.
             */
            User(const css::uno::Reference< css::sdbc::XConnection >& rConnection, const OUString& rName);

            // XAuthorizable
            virtual void SAL_CALL changePassword(const OUString&, const OUString& newPassword) override;
            virtual sal_Int32 SAL_CALL getPrivileges(const OUString&, sal_Int32) override;
            virtual sal_Int32 SAL_CALL getGrantablePrivileges(const OUString&, sal_Int32) override;

            // IRefreshableGroups::
            virtual void refreshGroups() override;
        };

} // namespace connectivity::firebird

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
