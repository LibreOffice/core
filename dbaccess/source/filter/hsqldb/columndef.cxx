
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

#include "columndef.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <utility>

namespace dbahsql
{
ColumnDefinition::ColumnDefinition(OUString sName, sal_Int32 eType,
                                   std::vector<sal_Int32>&& aParams, bool bPrimary,
                                   sal_Int32 nAutoIncr, bool bNullable, bool bCaseInsensitive,
                                   OUString sDefault)
    : m_sName(std::move(sName))
    , m_eType(eType)
    , m_aParams(std::move(aParams))
    , m_bPrimaryKey(bPrimary)
    , m_nAutoIncrement(nAutoIncr)
    , m_bNullable(bNullable)
    , m_bCaseInsensitive(bCaseInsensitive)
    , m_sDefaultValue(std::move(sDefault))
{
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
