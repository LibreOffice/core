/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITY_FIREBIRD_CATALOG_HXX
#define CONNECTIVITY_FIREBIRD_CATALOG_HXX

#include <connectivity/sdbcx/VCatalog.hxx>

namespace connectivity
{
    namespace firebird
    {
        class Catalog: public ::connectivity::sdbcx::OCatalog
        {
        protected:
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                m_xConnection;

        public:
            Catalog(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& rConnection);

            // OCatalog
            virtual void refreshTables();
            virtual void refreshViews();

            // IRefreshableGroups
            virtual void refreshGroups();

            // IRefreshableUsers
            virtual void refreshUsers();
        };
    } // namespace firebird
} // namespace connectivity

#endif //CONNECTIVITY_FIREBIRD_CATALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */