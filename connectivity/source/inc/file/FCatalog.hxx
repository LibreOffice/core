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
#if 1

#include "connectivity/sdbcx/VCatalog.hxx"

#include "file/filedllapi.hxx"

namespace connectivity
{
    namespace file
    {
        class OConnection;
        class OOO_DLLPUBLIC_FILE SAL_NO_VTABLE OFileCatalog :
            public connectivity::sdbcx::OCatalog
        {
        protected:
            OConnection*                                        m_pConnection;

            /** builds the name which should be used to access the object later on in the collection.
                Will only be called in fillNames.
                @param  _xRow
                    The current row from the resultset given to fillNames.
            */
            virtual ::rtl::OUString buildName(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >& _xRow);

        public:
            virtual void refreshTables();
            virtual void refreshViews();
            virtual void refreshGroups();
            virtual void refreshUsers();

        public:
            OFileCatalog(OConnection* _pCon);
            OConnection*    getConnection() { return m_pConnection; }

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            // ::cppu::OComponentHelper
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL disposing(void);
        };
    }
}
#endif // _CONNECTIVITY_FILE_CATALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
