/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/types.h>

#include <scrptrun.h>

#include <hb-icu.h>

#include <vector>

namespace vcl
{
struct Run
{
    int32_t nStart;
    int32_t nEnd;
    UScriptCode nCode;
    Run(int32_t nStart_, int32_t nEnd_, UScriptCode nCode_)
        : nStart(nStart_)
        , nEnd(nEnd_)
        , nCode(nCode_)
    {
    }
};

class TextLayoutCache
{
public:
    std::vector<vcl::Run> runs;
    TextLayoutCache(sal_Unicode const* pStr, sal_Int32 const nEnd)
    {
        vcl::ScriptRun aScriptRun(reinterpret_cast<const UChar*>(pStr), nEnd);
        while (aScriptRun.next())
        {
            runs.emplace_back(aScriptRun.getScriptStart(), aScriptRun.getScriptEnd(),
                              aScriptRun.getScriptCode());
        }
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
