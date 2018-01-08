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

#ifndef INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_COLUMNDEF_HXX
#define INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_COLUMNDEF_HXX

#include <rtl/ustring.hxx>
#include <vector>

namespace dbahsql
{
/// nAutoIncrement: column is auto incremented started with value nAutoIncrement
class ColumnDefinition
{
private:
    OUString m_sName;
    sal_Int32 m_eType; // css::sdbc::DataType
    std::vector<sal_Int32> m_aParams;
    bool m_bPrimaryKey;
    sal_Int32 m_nAutoIncrement;
    bool m_bNullable;

public:
    ColumnDefinition(const OUString& sName, sal_Int32 eType, const std::vector<sal_Int32> aParams,
                     bool bPrimary = false, sal_Int32 nAutoIncr = -1, bool bNullable = true);

    OUString getName() const { return m_sName; }
    sal_Int32 getDataType() const { return m_eType; }
    bool isPrimaryKey() const { return m_bPrimaryKey; }
    bool isNullable() const { return m_bNullable; }
    sal_Int32 getAutoIncrementDefault() const
    {
        assert(m_nAutoIncrement > 0);
        return m_nAutoIncrement;
    }
    bool isAutoIncremental() const { return m_nAutoIncrement >= 0; }
    const std::vector<sal_Int32> getParams() const { return m_aParams; }
};
}

#endif // INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_COLUMNDEF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
