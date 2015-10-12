/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABCATALOG_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABCATALOG_HXX

#include <connectivity/sdbcx/VCatalog.hxx>

namespace connectivity
{
    namespace macab
    {
            class MacabConnection;

        class MacabCatalog : public connectivity::sdbcx::OCatalog
        {
            MacabConnection* m_pConnection;     // used to get the metadata
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > m_xMetaData; // just to make things easier

        public:
            explicit MacabCatalog(MacabConnection* _pCon);

            inline MacabConnection* getConnection() const { return m_pConnection; }

            static const OUString& getDot();

            // implementation of the pure virtual methods
            virtual void refreshTables() override;
            virtual void refreshViews() override;
            virtual void refreshGroups() override;
            virtual void refreshUsers() override;

            // XTablesSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(
                    ) throw(::com::sun::star::uno::RuntimeException) override;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABCATALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
