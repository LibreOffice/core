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

#include "NConnection.hxx"
#include <connectivity/sdbcx/VTable.hxx>

namespace connectivity::evoab
{
        typedef connectivity::sdbcx::OTable OEvoabTable_TYPEDEF;

        class OEvoabTable : public OEvoabTable_TYPEDEF
        {
            OEvoabConnection* m_pConnection;

        public:
            OEvoabTable(    sdbcx::OCollection* _pTables,
                    OEvoabConnection* _pConnection,
                    const OUString& Name,
                    const OUString& Type,
                    const OUString& Description,
                    const OUString& SchemaName,
                    const OUString& CatalogName
                );

            OEvoabConnection* getConnection() { return m_pConnection;}

            virtual void refreshColumns() override;

            OUString const & getTableName() const { return m_Name; }
            OUString const & getSchema() const { return m_SchemaName; }
        };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
