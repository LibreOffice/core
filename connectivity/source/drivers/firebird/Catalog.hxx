/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_CATALOG_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_CATALOG_HXX

#include <connectivity/sdbcx/VCatalog.hxx>

namespace connectivity
{
    namespace firebird
    {
        class Catalog: public ::connectivity::sdbcx::OCatalog
        {
            css::uno::Reference< css::sdbc::XConnection >
                m_xConnection;

        public:
            explicit Catalog(const css::uno::Reference< css::sdbc::XConnection >& rConnection);

            // OCatalog
            virtual void refreshTables() override;
            virtual void refreshViews() override;

            // IRefreshableGroups
            virtual void refreshGroups() override;

            // IRefreshableUsers
            virtual void refreshUsers() override;
        };
    } // namespace firebird
} // namespace connectivity

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_CATALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
