/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <time.h>
#include "autostyl.hxx"

#include "docsh.hxx"
#include "attrib.hxx"
#include "sc.hrc"

//==================================================================

struct ScAutoStyleInitData
{
    ScRange aRange;
    String  aStyle1;
    sal_uLong   nTimeout;
    String  aStyle2;

    ScAutoStyleInitData( const ScRange& rR, const String& rSt1, sal_uLong nT, const String& rSt2 ) :
        aRange(rR), aStyle1(rSt1), nTimeout(nT), aStyle2(rSt2) {}
};

struct ScAutoStyleData
{
    sal_uLong   nTimeout;
    ScRange aRange;
    String  aStyle;

    ScAutoStyleData( sal_uLong nT, const ScRange& rR, const String& rT ) :
        nTimeout(nT), aRange(rR), aStyle(rT) {}
};

//==================================================================

inline sal_uLong TimeNow()          // Sekunden
{
    return (sal_uLong) time(0);
}

//==================================================================

ScAutoStyleList::ScAutoStyleList(ScDocShell* pShell) :
    pDocSh( pShell )
{
    aTimer.SetTimeoutHdl( LINK( this, ScAutoStyleList, TimerHdl ) );
    aInitTimer.SetTimeoutHdl( LINK( this, ScAutoStyleList, InitHdl ) );
    aInitTimer.SetTimeout( 0 );
}

ScAutoStyleList::~ScAutoStyleList()
{
    sal_uLong i;
    sal_uLong nCount = aEntries.Count();
    for (i=0; i<nCount; i++)
        delete (ScAutoStyleData*) aEntries.GetObject(i);
    nCount = aInitials.Count();
    for (i=0; i<nCount; i++)
        delete (ScAutoStyleInitData*) aInitials.GetObject(i);
}

//==================================================================

//  initial short delay (asynchronous call)

void ScAutoStyleList::AddInitial( const ScRange& rRange, const String& rStyle1,
                                    sal_uLong nTimeout, const String& rStyle2 )
{
    ScAutoStyleInitData* pNew =
        new ScAutoStyleInitData( rRange, rStyle1, nTimeout, rStyle2 );
    aInitials.Insert( pNew, aInitials.Count() );
    aInitTimer.Start();
}

IMPL_LINK( ScAutoStyleList, InitHdl, Timer*, EMPTYARG )
{
    sal_uLong nCount = aInitials.Count();
    for (sal_uLong i=0; i<nCount; i++)
    {
        ScAutoStyleInitData* pData = (ScAutoStyleInitData*) aInitials.GetObject(i);

        //  apply first style immediately
        pDocSh->DoAutoStyle( pData->aRange, pData->aStyle1 );

        //  add second style to list
        if ( pData->nTimeout )
            AddEntry( pData->nTimeout, pData->aRange, pData->aStyle2 );

        delete pData;
    }
    aInitials.Clear();

    return 0;
}

//==================================================================

void ScAutoStyleList::AddEntry( sal_uLong nTimeout, const ScRange& rRange, const String& rStyle )
{
    aTimer.Stop();
    sal_uLong nNow = TimeNow();

    //  alten Eintrag loeschen

    sal_uLong nCount = aEntries.Count();
    sal_uLong i;
    for (i=0; i<nCount; i++)
    {
        ScAutoStyleData* pData = (ScAutoStyleData*) aEntries.GetObject(i);
        if (pData->aRange == rRange)
        {
            delete pData;
            aEntries.Remove(i);
            --nCount;
            break;                      // nicht weitersuchen - es kann nur einen geben
        }
    }

    //  Timeouts von allen Eintraegen anpassen

    if (nCount && nNow != nTimerStart)
    {
        DBG_ASSERT(nNow>nTimerStart, "Zeit laeuft rueckwaerts?");
        AdjustEntries((nNow-nTimerStart)*1000);
    }

    //  Einfuege-Position suchen

    sal_uLong nPos = LIST_APPEND;
    for (i=0; i<nCount && nPos == LIST_APPEND; i++)
        if (nTimeout <= ((ScAutoStyleData*) aEntries.GetObject(i))->nTimeout)
            nPos = i;

    ScAutoStyleData* pNew = new ScAutoStyleData( nTimeout, rRange, rStyle );
    aEntries.Insert( pNew, nPos );

    //  abgelaufene ausfuehren, Timer neu starten

    ExecuteEntries();
    StartTimer(nNow);
}

void ScAutoStyleList::AdjustEntries( sal_uLong nDiff )  // Millisekunden
{
    sal_uLong nCount = aEntries.Count();
    for (sal_uLong i=0; i<nCount; i++)
    {
        ScAutoStyleData* pData = (ScAutoStyleData*) aEntries.GetObject(i);
        if ( pData->nTimeout <= nDiff )
            pData->nTimeout = 0;                    // abgelaufen
        else
            pData->nTimeout -= nDiff;               // weiterzaehlen
    }
}

void ScAutoStyleList::ExecuteEntries()
{
    ScAutoStyleData* pData;
    while ((pData = (ScAutoStyleData*) aEntries.GetObject(0)) != NULL && pData->nTimeout == 0)
    {
        pDocSh->DoAutoStyle( pData->aRange, pData->aStyle );    //! oder Request ???

        delete pData;
        aEntries.Remove((sal_uLong)0);
    }
}

void ScAutoStyleList::ExecuteAllNow()
{
    aTimer.Stop();

    sal_uLong nCount = aEntries.Count();
    for (sal_uLong i=0; i<nCount; i++)
    {
        ScAutoStyleData* pData = (ScAutoStyleData*) aEntries.GetObject(i);

        pDocSh->DoAutoStyle( pData->aRange, pData->aStyle );    //! oder Request ???

        delete pData;
    }
    aEntries.Clear();
}

void ScAutoStyleList::StartTimer( sal_uLong nNow )      // Sekunden
{
    // ersten Eintrag mit Timeout != 0 suchen

    sal_uLong nPos = 0;
    ScAutoStyleData* pData;
    while ( (pData = (ScAutoStyleData*) aEntries.GetObject(nPos)) != NULL && pData->nTimeout == 0 )
        ++nPos;

    if (pData)
    {
        aTimer.SetTimeout( pData->nTimeout );
        aTimer.Start();
    }
    nTimerStart = nNow;
}

IMPL_LINK( ScAutoStyleList, TimerHdl, Timer*, EMPTYARG )
{
    sal_uLong nNow = TimeNow();
    AdjustEntries(aTimer.GetTimeout());             // eingestellte Wartezeit
    ExecuteEntries();
    StartTimer(nNow);

    return 0;
}




