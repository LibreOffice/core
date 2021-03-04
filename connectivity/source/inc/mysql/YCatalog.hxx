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
#pragma once

#include <sdbcx/VCatalog.hxx>

namespace connectivity::mysql
    {
        // please don't name the class the same name as in another namespaces
        // some compilers have problems with this task as I noticed on windows
        class OMySQLCatalog : public connectivity::sdbcx::OCatalog
        {
            css::uno::Reference< css::sdbc::XConnection > m_xConnection;

            /** calls XDatabaseMetaData::getTables.
                @param  _sKindOfObject
                    The type of tables to be fetched.
                @param  _rNames
                    The container for the names to be filled. <OUT/>
            */
            void refreshObjects(const css::uno::Sequence< OUString >& _sKindOfObject,::std::vector< OUString>& _rNames);

        public:
            // implementation of the pure virtual methods
            virtual void refreshTables() override;
            virtual void refreshViews() override ;
            virtual void refreshGroups() override;
            virtual void refreshUsers() override ;

        public:
            OMySQLCatalog(const css::uno::Reference< css::sdbc::XConnection >& _xConnection);

            sdbcx::OCollection*      getPrivateTables()  const { return m_pTables.get();}
            sdbcx::OCollection*      getPrivateViews()   const { return m_pViews.get(); }
            const css::uno::Reference< css::sdbc::XConnection >& getConnection() const { return m_xConnection; }

            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            // ::cppu::OComponentHelper
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
