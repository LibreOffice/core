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

#include <patterncache.hxx>

ScPatternCache::Entry::Entry()
    : nXfId(-1)
    , nNumFmtId(-1)
    , pPattern(nullptr)
{
}

ScPatternCache::ScPatternCache()
    : nNextPos(0)
{
}

ScPatternAttr* ScPatternCache::query(sal_Int32 nXfId, sal_Int32 nNumFmtId) const
{
    for (const auto& entry : maEntries)
    {
        if (entry.nXfId == nXfId && entry.nNumFmtId == nNumFmtId)
            return entry.pPattern;
    }

    return nullptr;
}

void ScPatternCache::add(sal_Int32 nXfId, sal_Int32 nNumFmtId, ScPatternAttr* pPattern)
{
    Entry& rEntry = maEntries[nNextPos];
    nNextPos = ((nNextPos + 1) % nPatternCacheSize);
    rEntry.nXfId = nXfId;
    rEntry.nNumFmtId = nNumFmtId;
    rEntry.pPattern = pPattern;
}
