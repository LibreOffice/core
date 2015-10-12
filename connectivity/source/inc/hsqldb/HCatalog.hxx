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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_HSQLDB_HCATALOG_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_HSQLDB_HCATALOG_HXX

#include <connectivity/sdbcx/VCatalog.hxx>
#include <connectivity/StdTypeDefs.hxx>

namespace connectivity
{
    namespace hsqldb
    {
        // please don't name the class the same name as in an other namespaces
        // some compilers have problems with this task as I noticed on windows
        class OHCatalog : public connectivity::sdbcx::OCatalog
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;

            /** calls XDatabaseMetaData::getTables.
                @param  _sKindOfObject
                    The type of tables to be fetched.
                @param  _rNames
                    The container for the names to be filled.
            */
            void refreshObjects(const ::com::sun::star::uno::Sequence< OUString >& _sKindOfObject,TStringVector& _rNames);

        public:
            // implementation of the pure virtual methods
            virtual void refreshTables() override;
            virtual void refreshViews() override ;
            virtual void refreshGroups() override;
            virtual void refreshUsers() override ;

        public:
            OHCatalog(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);

            inline sdbcx::OCollection*      getPrivateTables()  const { return m_pTables;}
            inline sdbcx::OCollection*      getPrivateViews()   const { return m_pViews; }
            inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection() const { return m_xConnection; }

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
            // ::cppu::OComponentHelper
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_HSQLDB_HCATALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
