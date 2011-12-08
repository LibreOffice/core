/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _BREAKPOINT_HXX
#define _BREAKPOINT_HXX

#include <cstddef>
#include <vector>

class SbModule;

struct BreakPoint
{
    bool bEnabled;
    bool bTemp;
    size_t nLine;
    size_t nStopAfter;
    size_t nHitCount;

    BreakPoint(size_t nL) { nLine = nL; nStopAfter = 0; nHitCount = 0; bEnabled = true; bTemp = false; }
};

class BreakPointList
{
private:
    void operator =(BreakPointList); // not implemented
    ::std::vector<BreakPoint*> maBreakPoints;

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
    void clear();
    BreakPoint* remove(BreakPoint* ptr);
};

#endif  // _BREAKPOINT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
