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

#ifndef INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_CREATEPARSER_HXX
#define INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_CREATEPARSER_HXX

#include <vector>
#include "columndef.hxx"

namespace dbahsql
{
class CreateStmtParser
{
private:
    std::vector<ColumnDefinition> m_aColumns;
    std::vector<OUString> m_aForeignParts;
    OUString m_sTableName;

protected:
    void parseColumnPart(const OUString& sColumnPart);

public:
    CreateStmtParser();
    virtual ~CreateStmtParser() {}
    OUString getTableName() const { return m_sTableName; }
    const std::vector<ColumnDefinition>& getColumnDef() const { return m_aColumns; }
    const std::vector<OUString>& getForeignParts() const { return m_aForeignParts; }
    void parse(const OUString& sSql);
    virtual OUString compose() const = 0;
};
}

#endif // INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_CREATEPARSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
