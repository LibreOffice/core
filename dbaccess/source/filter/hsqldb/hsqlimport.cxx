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

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include "hsqlimport.hxx"
#include "parseschema.hxx"

namespace dbahsql
{
using namespace css::uno;
using namespace css::sdbc;
using namespace css::embed;

HsqlImporter::HsqlImporter(Reference<XConnection>& rConnection, const Reference<XStorage>& rStorage)
    : m_rConnection(rConnection)
    , m_xStorage(nullptr)
{
    m_xStorage.set(rStorage);
}

void HsqlImporter::importSchema()
{
    assert(m_xStorage);

    SchemaParser parser(m_xStorage);
    SqlStatementVector statements = parser.parseSchema();

    for (auto& sSql : statements)
    {
        Reference<XStatement> statement = m_rConnection->createStatement();
        statement->executeQuery(sSql);
    }
}

void HsqlImporter::importHsqlDatabase() { importSchema(); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
