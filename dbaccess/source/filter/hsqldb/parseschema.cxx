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

#include "parseschema.hxx"
#include "fbcreateparser.hxx"

#include <com/sun/star/io/TextInputStream.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <comphelper/processfactory.hxx>

namespace dbahsql
{
using namespace css::io;
using namespace css::uno;
using namespace css::embed;

SchemaParser::SchemaParser(Reference<XStorage>& rStorage)
    : m_rStorage(rStorage)
{
}

SqlStatementVector SchemaParser::parseSchema()
{
    assert(m_rStorage);

    constexpr char SCHEMA_FILENAME[] = "script";
    if (!m_rStorage->hasByName(SCHEMA_FILENAME))
    {
        SAL_WARN("dbaccess", "script file does not exist in storage during hsqldb import");
        assert(false); // TODO throw error
    }

    Reference<XStream> xStream(m_rStorage->openStreamElement(SCHEMA_FILENAME, ElementModes::READ));

    Reference<XComponentContext> rContext = comphelper::getProcessComponentContext();
    Reference<XTextInputStream2> xTextInput = TextInputStream::create(rContext);
    xTextInput->setEncoding("UTF-8");
    xTextInput->setInputStream(xStream->getInputStream());

    SqlStatementVector parsedStatements;
    while (!xTextInput->isEOF())
    {
        // every line contains exactly one DDL statement
        OUString sSql = xTextInput->readLine();

        if (sSql.startsWith("SET") || sSql.startsWith("CREATE USER")
            || sSql.startsWith("CREATE SCHEMA") || sSql.startsWith("GRANT"))
            continue;

        if (sSql.startsWith("CREATE CACHED TABLE") || sSql.startsWith("CREATE TABLE"))
        {
            FbCreateStmtParser aCreateParser;
            aCreateParser.parse(sSql);

            sSql = aCreateParser.compose();
        }

        parsedStatements.push_back(sSql);
    }

    return parsedStatements;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
