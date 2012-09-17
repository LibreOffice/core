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

#ifndef _CONNECTIVITY_MORK_TABLE_HXX_
#define _CONNECTIVITY_MORK_TABLE_HXX_

#include "connectivity/TTableHelper.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "MConnection.hxx"

namespace connectivity
{
    namespace mork
    {
        typedef ::connectivity::OTableHelper OTable_Base;

        class OTable :  public OTable_Base
        {
            OConnection*    m_pConnection;

        public:
            OTable( sdbcx::OCollection* _pTables,
                    OConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description );

            OConnection* getConnection() { return m_pConnection;}

            sal_Bool isReadOnly() const { return sal_False; }

            ::rtl::OUString getTableName() const { return m_Name; }
            ::rtl::OUString getSchema() const { return m_SchemaName; }

            // OTableHelper overridables
            virtual sdbcx::OCollection* createColumns( const TStringVector& _rNames );
            virtual sdbcx::OCollection* createKeys(const TStringVector& _rNames);
            virtual sdbcx::OCollection* createIndexes(const TStringVector& _rNames);
        private:
            using OTable_Base::getConnection;
        };
    }
}
#endif // _CONNECTIVITY_MORK_TABLE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
