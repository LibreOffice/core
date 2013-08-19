/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITY_FIREBIRD_USER_HXX
#define CONNECTIVITY_FIREBIRD_USER_HXX

#include <connectivity/sdbcx/VUser.hxx>

#include <com/sun/star/sdbc/XConnection.hpp>

namespace connectivity
{
    namespace firebird
    {

        /**
         * This implements com.sun.star.sdbcx.Container.
         */
        class User: public ::connectivity::sdbcx::OUser
        {

        public:
            /**
             * Create a "new" descriptor, which isn't yet in the database.
             */
            User(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& rConnection);
            /**
             * For a user that already exists in the db.
             */
            User(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& rConnection,
                 const ::rtl::OUString& rName);

            // IRefreshableGroups::
            virtual void refreshGroups();
        };

    } // namespace firebird
} // namespace connectivity


#endif // CONNECTIVITY_FIREBIRD_USER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */