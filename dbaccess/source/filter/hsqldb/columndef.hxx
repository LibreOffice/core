/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

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
    bool m_bCaseInsensitive;
    OUString m_sDefaultValue;

public:
    ColumnDefinition(const OUString& sName, sal_Int32 eType, const std::vector<sal_Int32>& aParams,
                     bool bPrimary = false, sal_Int32 nAutoIncr = -1, bool bNullable = true,
                     bool bCaseInsensitive = false, const OUString& sDefault = OUString{});

    OUString const& getName() const { return m_sName; }
    sal_Int32 getDataType() const { return m_eType; }
    bool isPrimaryKey() const { return m_bPrimaryKey; }
    bool isNullable() const { return m_bNullable; }
    bool isAutoIncremental() const { return m_nAutoIncrement >= 0; }
    bool isCaseInsensitive() const { return m_bCaseInsensitive; }
    sal_Int32 getStartValue() const { return m_nAutoIncrement; }
    const std::vector<sal_Int32>& getParams() const { return m_aParams; }
    OUString const& getDefault() const { return m_sDefaultValue; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
