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

#include <interpretercontext.hxx>
#include <formula/token.hxx>
#include <lookupcache.hxx>
#include <algorithm>

ScInterpreterContext::ContextPoolType ScInterpreterContext::aContextPool;

ScInterpreterContext::~ScInterpreterContext()
{
    ResetTokens();
    delete mScLookupCache;
}

void ScInterpreterContext::ResetTokens()
{
    for (auto p : maTokens)
        if (p)
            p->DecRef();

    mnTokenCachePos = 0;
    std::fill(maTokens.begin(), maTokens.end(), nullptr);
}

void ScInterpreterContext::Reset(const ScDocument& rDoc, SvNumberFormatter* pFormatter)
{
    mpDoc = &rDoc;
    mpFormatter = pFormatter;
    maDelayedSetNumberFormat.clear();
    delete mScLookupCache;
    mScLookupCache = nullptr;
    ResetTokens();
}

ScInterpreterContext::ContextPoolType&
ScInterpreterContext::GetContexts(size_t nSize, const ScDocument& rDoc,
                                  SvNumberFormatter* pFormatter)
{
    size_t nOldSize = aContextPool.size();
    aContextPool.resize(nSize);
    for (size_t nIdx = 0; nIdx < nSize; ++nIdx)
    {
        if (nIdx >= nOldSize)
            aContextPool[nIdx].reset(new ScInterpreterContext(rDoc, pFormatter, nIdx));
        else
            aContextPool[nIdx]->Reset(rDoc, pFormatter);
    }

    return aContextPool;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
