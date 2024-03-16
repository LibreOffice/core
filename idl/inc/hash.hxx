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

#include <rtl/string.hxx>
#include <tools/solar.h>
#include <unordered_map>
#include <memory>
#include <utility>

class SvStringHashEntry
{
    OString     aName;
    sal_uInt32   nValue;
public:
    SvStringHashEntry( OString aName_ )
        : aName(std::move(aName_))
        , nValue(0)
    {
    }

    const OString&  GetName() const { return aName; }

    void            SetValue( sal_uInt32 n ) { nValue = n; }
    sal_uInt32       GetValue() const { return nValue; }
};

class SvStringHashTable
{
    std::unordered_map<sal_uInt32, std::unique_ptr<SvStringHashEntry>> maInt2EntryMap;
    std::unordered_map<OString, sal_uInt32> maString2IntMap;
    sal_uInt32 mnNextId = 0;

public:
    SvStringHashEntry * Insert( OString const & rElement, sal_uInt32 * pInsertPos );
    bool Test( OString const & rElement, sal_uInt32 * pInsertPos );
    SvStringHashEntry * Get( sal_uInt32 nInsertPos ) const;
    OString GetNearString( std::string_view rName ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
