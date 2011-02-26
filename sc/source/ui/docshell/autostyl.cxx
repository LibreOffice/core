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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <time.h>

#include <boost/bind.hpp>

#include "attrib.hxx"
#include "autostyl.hxx"
#include "docsh.hxx"
#include "sc.hrc"

struct ScAutoStyleInitData
{
    ScRange aRange;
    String  aStyle1;
    ULONG   nTimeout;
    String  aStyle2;

    ScAutoStyleInitData( const ScRange& rR, const String& rSt1, ULONG nT, const String& rSt2 ) :
        aRange(rR), aStyle1(rSt1), nTimeout(nT), aStyle2(rSt2) {}
};

struct ScAutoStyleData
{
    ULONG   nTimeout;
    ScRange aRange;
    String  aStyle;

    ScAutoStyleData( ULONG nT, const ScRange& rR, const String& rT ) :
        nTimeout(nT), aRange(rR), aStyle(rT) {}
};

inline ULONG TimeNow()          // Sekunden
{
    return (ULONG) time(0);
}

ScAutoStyleList::ScAutoStyleList(ScDocShell* pShell) :
    pDocSh( pShell )
{
    aTimer.SetTimeoutHdl( LINK( this, ScAutoStyleList, TimerHdl ) );
    aInitTimer.SetTimeoutHdl( LINK( this, ScAutoStyleList, InitHdl ) );
    aInitTimer.SetTimeout( 0 );
}

ScAutoStyleList::~ScAutoStyleList()
{
}

//  initial short delay (asynchronous call)

void ScAutoStyleList::AddInitial( const ScRange& rRange, const String& rStyle1,
                                    ULONG nTimeout, const String& rStyle2 )
{
    aInitials.push_back(new ScAutoStyleInitData( rRange, rStyle1, nTimeout, rStyle2 ));
    aInitTimer.Start();
}

IMPL_LINK( ScAutoStyleList, InitHdl, Timer*, EMPTYARG )
{
    boost::ptr_vector<ScAutoStyleInitData>::iterator iter;
    for (iter = aInitials.begin(); iter != aInitials.end(); ++iter)
    {
        //  apply first style immediately
        pDocSh->DoAutoStyle(iter->aRange,iter->aStyle1);

        //  add second style to list
        if (iter->nTimeout)
            AddEntry(iter->nTimeout,iter->aRange,iter->aStyle2 );
    }

    aInitials.clear();

    return 0;
}

void ScAutoStyleList::AddEntry( ULONG nTimeout, const ScRange& rRange, const String& rStyle )
{
    aTimer.Stop();
    ULONG nNow = TimeNow();

    aEntries.erase(std::remove_if(aEntries.begin(),aEntries.end(),
                                  boost::bind(&ScAutoStyleData::aRange,_1) == rRange));

    //  Timeouts von allen Eintraegen anpassen

    if (!aEntries.empty() && nNow != nTimerStart)
    {
        DBG_ASSERT(nNow>nTimerStart, "Zeit laeuft rueckwaerts?");
        AdjustEntries((nNow-nTimerStart)*1000);
    }

    //  Einfuege-Position suchen
    boost::ptr_vector<ScAutoStyleData>::iterator iter = std::find_if(aEntries.begin(),aEntries.end(),
                                                                     boost::bind(&ScAutoStyleData::nTimeout,_1) >= nTimeout);

    aEntries.insert(iter,new ScAutoStyleData(nTimeout,rRange,rStyle));

    //  abgelaufene ausfuehren, Timer neu starten

    ExecuteEntries();
    StartTimer(nNow);
}

void ScAutoStyleList::AdjustEntries( ULONG nDiff )  // Millisekunden
{
    boost::ptr_vector<ScAutoStyleData>::iterator iter;
    for (iter = aEntries.begin(); iter != aEntries.end(); ++iter)
    {
        if (iter->nTimeout <= nDiff)
            iter->nTimeout = 0;                 // abgelaufen
        else
            iter->nTimeout -= nDiff;                // weiterzaehlen
    }
}

void ScAutoStyleList::ExecuteEntries()
{
    boost::ptr_vector<ScAutoStyleData>::iterator iter;
    for (iter = aEntries.begin(); iter != aEntries.end();)
    {
        if (!iter->nTimeout)
        {
            pDocSh->DoAutoStyle(iter->aRange,iter->aStyle);
            iter = aEntries.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

void ScAutoStyleList::ExecuteAllNow()
{
    aTimer.Stop();

    boost::ptr_vector<ScAutoStyleData>::iterator iter;
    for (iter = aEntries.begin(); iter != aEntries.end(); ++iter)
        pDocSh->DoAutoStyle(iter->aRange,iter->aStyle);

    aEntries.clear();
}

void ScAutoStyleList::StartTimer( ULONG nNow )      // Sekunden
{
    // ersten Eintrag mit Timeout != 0 suchen
    boost::ptr_vector<ScAutoStyleData>::iterator iter = std::find_if(aEntries.begin(),aEntries.end(),
                                                                     boost::bind(&ScAutoStyleData::nTimeout,_1) != static_cast<unsigned>(0));

    if (iter != aEntries.end())
    {
        aTimer.SetTimeout(iter->nTimeout);
        aTimer.Start();
    }

    nTimerStart = nNow;
}

IMPL_LINK( ScAutoStyleList, TimerHdl, Timer*, EMPTYARG )
{
    ULONG nNow = TimeNow();
    AdjustEntries(aTimer.GetTimeout());             // eingestellte Wartezeit
    ExecuteEntries();
    StartTimer(nNow);

    return 0;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
