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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MCATALOG_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MCATALOG_HXX

#include <connectivity/sdbcx/VCatalog.hxx>

namespace connectivity
{
    namespace mork
    {
        // please don't name the class the same name as in an other namespaces
        // some compilers have problems with this task as I noticed on windows
        class OConnection;
        class OCatalog : public connectivity::sdbcx::OCatalog
        {
            OConnection*    m_pConnection;      // used to get the metadata
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > m_xMetaData; // just to make things easier

        public:
            // implementation of the pure virtual methods
            virtual void refreshTables() override;
            virtual void refreshViews() override ;
            virtual void refreshGroups() override;
            virtual void refreshUsers() override ;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        public:
            explicit OCatalog(OConnection* _pCon);

            OConnection*            getConnection()     const { return m_pConnection; }

        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MCATALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
