/*************************************************************************
 *
 *  $RCSfile: autostyl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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

//==================================================================

inline ULONG TimeNow()          // Sekunden
{
    return (ULONG) time(0);
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
    ULONG i;
    ULONG nCount = aEntries.Count();
    for (i=0; i<nCount; i++)
        delete (ScAutoStyleData*) aEntries.GetObject(i);
    nCount = aInitials.Count();
    for (i=0; i<nCount; i++)
        delete (ScAutoStyleInitData*) aInitials.GetObject(i);
}

//==================================================================

//  initial short delay (asynchronous call)

void ScAutoStyleList::AddInitial( const ScRange& rRange, const String& rStyle1,
                                    ULONG nTimeout, const String& rStyle2 )
{
    ScAutoStyleInitData* pNew =
        new ScAutoStyleInitData( rRange, rStyle1, nTimeout, rStyle2 );
    aInitials.Insert( pNew, aInitials.Count() );
    aInitTimer.Start();
}

IMPL_LINK( ScAutoStyleList, InitHdl, Timer*, pTimer )
{
    DBG_ASSERT( pTimer == &aInitTimer, "wrong timer" );

    ULONG nCount = aInitials.Count();
    for (ULONG i=0; i<nCount; i++)
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

void ScAutoStyleList::AddEntry( ULONG nTimeout, const ScRange& rRange, const String& rStyle )
{
    aTimer.Stop();
    ULONG nNow = TimeNow();

    //  alten Eintrag loeschen

    ULONG nCount = aEntries.Count();
    ULONG i;
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

    ULONG nPos = LIST_APPEND;
    BOOL bFound = FALSE;
    for (i=0; i<nCount && nPos == LIST_APPEND; i++)
        if (nTimeout <= ((ScAutoStyleData*) aEntries.GetObject(i))->nTimeout)
            nPos = i;

    ScAutoStyleData* pNew = new ScAutoStyleData( nTimeout, rRange, rStyle );
    aEntries.Insert( pNew, nPos );

    //  abgelaufene ausfuehren, Timer neu starten

    ExecuteEntries();
    StartTimer(nNow);
}

void ScAutoStyleList::AdjustEntries( ULONG nDiff )  // Millisekunden
{
    ULONG nCount = aEntries.Count();
    for (ULONG i=0; i<nCount; i++)
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
    while ((pData = (ScAutoStyleData*) aEntries.GetObject(0)) && pData->nTimeout == 0)
    {
        pDocSh->DoAutoStyle( pData->aRange, pData->aStyle );    //! oder Request ???

        delete pData;
        aEntries.Remove((ULONG)0);
    }
}

void ScAutoStyleList::ExecuteAllNow()
{
    aTimer.Stop();

    ULONG nCount = aEntries.Count();
    for (ULONG i=0; i<nCount; i++)
    {
        ScAutoStyleData* pData = (ScAutoStyleData*) aEntries.GetObject(i);

        pDocSh->DoAutoStyle( pData->aRange, pData->aStyle );    //! oder Request ???

        delete pData;
    }
    aEntries.Clear();
}

void ScAutoStyleList::StartTimer( ULONG nNow )      // Sekunden
{
    // ersten Eintrag mit Timeout != 0 suchen

    ULONG nPos = 0;
    ScAutoStyleData* pData;
    while ( (pData = (ScAutoStyleData*) aEntries.GetObject(nPos)) && pData->nTimeout == 0 )
        ++nPos;

    if (pData)
    {
        aTimer.SetTimeout( pData->nTimeout );
        aTimer.Start();
    }
    nTimerStart = nNow;
}

IMPL_LINK( ScAutoStyleList, TimerHdl, Timer*, pTimer )
{
    DBG_ASSERT( pTimer == &aTimer, "falscher Timer" );

    ULONG nNow = TimeNow();
    AdjustEntries(aTimer.GetTimeout());             // eingestellte Wartezeit
    ExecuteEntries();
    StartTimer(nNow);

    return 0;
}




