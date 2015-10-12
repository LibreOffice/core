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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABTABLE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABTABLE_HXX

#include "MacabConnection.hxx"
#include <connectivity/sdbcx/VTable.hxx>

namespace connectivity
{
    namespace macab
    {
        typedef connectivity::sdbcx::OTable MacabTable_TYPEDEF;

        OUString getTypeString(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xColProp);

        class MacabTable : public MacabTable_TYPEDEF
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > m_xMetaData;
            MacabConnection* m_pConnection;

        public:
            MacabTable( sdbcx::OCollection* _pTables, MacabConnection* _pConnection);
            MacabTable( sdbcx::OCollection* _pTables,
                    MacabConnection* _pConnection,
                    const OUString& _Name,
                    const OUString& _Type,
                    const OUString& _Description = OUString(),
                    const OUString& _SchemaName = OUString(),
                    const OUString& _CatalogName = OUString()
                );

            MacabConnection* getConnection() { return m_pConnection;}

            virtual void refreshColumns() override;

            OUString getTableName() const { return m_Name; }
            OUString getSchema() const { return m_SchemaName; }
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
