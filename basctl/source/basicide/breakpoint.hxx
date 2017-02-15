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

#ifndef INCLUDED_BASCTL_SOURCE_BASICIDE_BREAKPOINT_HXX
#define INCLUDED_BASCTL_SOURCE_BASICIDE_BREAKPOINT_HXX

#include <cstddef>
#include <vector>

#include <sal/types.h>

class SbModule;

namespace basctl
{

struct BreakPoint
{
    bool bEnabled;
    size_t nLine;
    size_t nStopAfter;
    size_t nHitCount;

    explicit BreakPoint(size_t nL)
        : bEnabled(true)
        , nLine(nL)
        , nStopAfter(0)
        , nHitCount(0)
    {
    }
};

class BreakPointList
{
private:
    void operator =(BreakPointList) = delete;
    std::vector<BreakPoint*> maBreakPoints;

public:
    BreakPointList();

    BreakPointList(BreakPointList const & rList);

    ~BreakPointList();

    void reset();

    void transfer(BreakPointList & rList);

    void InsertSorted(BreakPoint* pBrk);
    BreakPoint* FindBreakPoint(size_t nLine);
    void AdjustBreakPoints(size_t nLine, bool bInserted);
    void SetBreakPointsInBasic(SbModule* pModule);
    void ResetHitCount();

    size_t size() const;
    BreakPoint* at(size_t i);
    const BreakPoint* at(size_t i) const;
    BreakPoint* remove(BreakPoint* ptr);
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_BREAKPOINT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
