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

#include <time.h>

#include <address.hxx>
#include <autostyl.hxx>
#include <docsh.hxx>

static sal_uLong TimeNow()          // seconds
{
    return static_cast<sal_uLong>(time(nullptr));
}

namespace {

class FindByRange
{
    ScRange const maRange;
public:
    explicit FindByRange(const ScRange& r) : maRange(r) {}
    bool operator() (const ScAutoStyleData& rData) const { return rData.aRange == maRange; }
};

class FindByTimeout
{
    sal_uLong const mnTimeout;
public:
    explicit FindByTimeout(sal_uLong n) : mnTimeout(n) {}
    bool operator() (const ScAutoStyleData& rData) const { return rData.nTimeout >= mnTimeout; }
};

struct FindNonZeroTimeout
{
    bool operator() (const ScAutoStyleData& rData) const
    {
        return rData.nTimeout != 0;
    }
};

}

ScAutoStyleList::ScAutoStyleList(ScDocShell* pShell)
    : pDocSh(pShell)
    , aTimer("ScAutoStyleList Timer")
    , aInitIdle("ScAutoStyleList InitIdle")
    , nTimerStart(0)
{
    aTimer.SetInvokeHandler( LINK( this, ScAutoStyleList, TimerHdl ) );
    aInitIdle.SetInvokeHandler( LINK( this, ScAutoStyleList, InitHdl ) );
    aInitIdle.SetPriority( TaskPriority::HIGHEST );
}

ScAutoStyleList::~ScAutoStyleList()
{
}

//  initial short delay (asynchronous call)

void ScAutoStyleList::AddInitial( const ScRange& rRange, const OUString& rStyle1,
                                    sal_uLong nTimeout, const OUString& rStyle2 )
{
    aInitials.emplace_back( rRange, rStyle1, nTimeout, rStyle2 );
    aInitIdle.Start();
}

IMPL_LINK_NOARG(ScAutoStyleList, InitHdl, Timer *, void)
{
    for (const auto& rInitial : aInitials)
    {
        //  apply first style immediately
        pDocSh->DoAutoStyle(rInitial.aRange, rInitial.aStyle1);

        //  add second style to list
        if (rInitial.nTimeout)
            AddEntry(rInitial.nTimeout, rInitial.aRange, rInitial.aStyle2 );
    }

    aInitials.clear();
}

void ScAutoStyleList::AddEntry( sal_uLong nTimeout, const ScRange& rRange, const OUString& rStyle )
{
    aTimer.Stop();
    sal_uLong nNow = TimeNow();

    // Remove the first item with the same range.
    std::vector<ScAutoStyleData>::iterator itr =
        ::std::find_if(aEntries.begin(), aEntries.end(), FindByRange(rRange));

    if (itr != aEntries.end())
        aEntries.erase(itr);

    //  adjust timeouts of all entries

    if (!aEntries.empty() && nNow != nTimerStart)
    {
        OSL_ENSURE(nNow>nTimerStart, "Time is running backwards?");
        AdjustEntries((nNow-nTimerStart)*1000);
    }

    //  find insert position
    std::vector<ScAutoStyleData>::iterator iter =
        ::std::find_if(aEntries.begin(), aEntries.end(), FindByTimeout(nTimeout));

    aEntries.insert(iter, ScAutoStyleData(nTimeout,rRange,rStyle));

    //  execute expired, restart timer

    ExecuteEntries();
    StartTimer(nNow);
}

void ScAutoStyleList::AdjustEntries( sal_uLong nDiff )  // milliseconds
{
    for (auto& rEntry : aEntries)
    {
        if (rEntry.nTimeout <= nDiff)
            rEntry.nTimeout = 0;                 // expired
        else
            rEntry.nTimeout -= nDiff;                // continue counting
    }
}

void ScAutoStyleList::ExecuteEntries()
{
    // Execute and remove all items with timeout == 0 from the begin position
    // until the first item with non-zero timeout value.
    std::vector<ScAutoStyleData>::iterator itr = aEntries.begin(), itrEnd = aEntries.end();
    for (; itr != itrEnd; ++itr)
    {
        if (itr->nTimeout)
            break;

        pDocSh->DoAutoStyle(itr->aRange, itr->aStyle);
    }
    // At this point itr should be on the first item with non-zero timeout, or
    // the end position in case all items have timeout == 0.
    aEntries.erase(aEntries.begin(), itr);
}

void ScAutoStyleList::ExecuteAllNow()
{
    aTimer.Stop();

    for (const auto& rEntry : aEntries)
        pDocSh->DoAutoStyle(rEntry.aRange, rEntry.aStyle);

    aEntries.clear();
}

void ScAutoStyleList::StartTimer( sal_uLong nNow )      // seconds
{
    // find first entry with Timeout != 0
    std::vector<ScAutoStyleData>::iterator iter =
        ::std::find_if(aEntries.begin(),aEntries.end(), FindNonZeroTimeout());

    if (iter != aEntries.end())
    {
        aTimer.SetTimeout(iter->nTimeout);
        aTimer.Start();
    }

    nTimerStart = nNow;
}

IMPL_LINK_NOARG(ScAutoStyleList, TimerHdl, Timer *, void)
{
    sal_uLong nNow = TimeNow();
    AdjustEntries(aTimer.GetTimeout());             // the set waiting time
    ExecuteEntries();
    StartTimer(nNow);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
