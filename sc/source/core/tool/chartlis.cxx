/*************************************************************************
 *
 *  $RCSfile: chartlis.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-20 11:33:20 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <vcl/svapp.hxx>

#include "chartlis.hxx"
#include "brdcst.hxx"
#include "document.hxx"

using namespace com::sun::star;


//2do: DocOption TimeOut?
#define SC_CHARTTIMEOUT 1000        // eine Sekunde keine Aenderung/KeyEvent


// ====================================================================

class ScChartUnoData
{
    uno::Reference< chart::XChartDataChangeEventListener >  xListener;
    uno::Reference< chart::XChartData >                     xSource;

public:
            ScChartUnoData( const uno::Reference< chart::XChartDataChangeEventListener >& rL,
                            const uno::Reference< chart::XChartData >& rS ) :
                    xListener( rL ), xSource( rS ) {}
            ~ScChartUnoData() {}

    const uno::Reference< chart::XChartDataChangeEventListener >& GetListener() const   { return xListener; }
    const uno::Reference< chart::XChartData >& GetSource() const                        { return xSource; }
};


// === ScChartListener ================================================

ScChartListener::ScChartListener( const String& rName, ScDocument* pDocP,
        const ScRange& rRange ) :
    StrData( rName ),
    pDoc( pDocP ),
    pUnoData( NULL ),
    bUsed( FALSE ),
    bDirty( FALSE ),
    bSeriesRangesScheduled( FALSE )
{
    SetRangeList( rRange );
}

ScChartListener::ScChartListener( const String& rName, ScDocument* pDocP,
        const ScRangeListRef& rRangeList ) :
    StrData( rName ),
    aRangeListRef( rRangeList ),
    pDoc( pDocP ),
    pUnoData( NULL ),
    bUsed( FALSE ),
    bDirty( FALSE ),
    bSeriesRangesScheduled( FALSE )
{
}

ScChartListener::ScChartListener( const ScChartListener& r ) :
        StrData( r ),
        pDoc( r.pDoc ),
        pUnoData( NULL ),
        bUsed( FALSE ),
        bDirty( r.bDirty ),
        bSeriesRangesScheduled( r.bSeriesRangesScheduled )
{
    if ( r.pUnoData )
        pUnoData = new ScChartUnoData( *r.pUnoData );
    if ( r.aRangeListRef.Is() )
        aRangeListRef = new ScRangeList( *r.aRangeListRef );
}

ScChartListener::~ScChartListener()
{
    if ( GetBroadcasterCount() )
        EndListeningTo();
    delete pUnoData;
}

DataObject* ScChartListener::Clone() const
{
    return new ScChartListener( *this );
}

void ScChartListener::SetUno(
        const uno::Reference< chart::XChartDataChangeEventListener >& rListener,
        const uno::Reference< chart::XChartData >& rSource )
{
//  DBG_ASSERT( rListener.is() && rSource.is(), "Nullpointer bei SetUno" );
    delete pUnoData;
    pUnoData = new ScChartUnoData( rListener, rSource );
}

uno::Reference< chart::XChartDataChangeEventListener > ScChartListener::GetUnoListener() const
{
    if ( pUnoData )
        return pUnoData->GetListener();
    return uno::Reference< chart::XChartDataChangeEventListener >();
}

uno::Reference< chart::XChartData > ScChartListener::GetUnoSource() const
{
    if ( pUnoData )
        return pUnoData->GetSource();
    return uno::Reference< chart::XChartData >();
}

void __EXPORT ScChartListener::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const ScHint* p = PTR_CAST( ScHint, &rHint );
    if( p && (p->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING)) )
    {
        bDirty = TRUE;
        pDoc->GetChartListenerCollection()->StartTimer();
    }
}

void ScChartListener::Update()
{
    if ( pDoc->IsInInterpreter() )
    {   // #73482# If interpreting do nothing and restart timer so we don't
        // interfere with interpreter and don't produce an Err522 or similar.
        // This may happen if we are rescheduled via Basic function.
        pDoc->GetChartListenerCollection()->StartTimer();
        return ;
    }
    if ( pUnoData )
    {
        bDirty = FALSE;
        //! irgendwann mal erkennen, was sich innerhalb des Charts geaendert hat
        chart::ChartDataChangeEvent aEvent( pUnoData->GetSource(),
#ifdef TF_ONE51
                                        chart::ChartDataChangeType_ALL,
#else
                                        CHARTDATACHANGETYPE_ALL,
#endif
                                        0, 0, 0, 0 );
        pUnoData->GetListener()->chartDataChanged( aEvent );
    }
    else if ( pDoc->GetAutoCalc() )
    {
        bDirty = FALSE;
        pDoc->UpdateChart( GetString(), NULL );
    }
}

void ScChartListener::StartListeningTo()
{
    if ( aRangeListRef.Is() )
        for ( ScRangePtr pR = aRangeListRef->First(); pR;
                         pR = aRangeListRef->Next() )
        {
            if ( pR->aStart == pR->aEnd )
                pDoc->StartListeningCell( pR->aStart, this );
            else
                pDoc->StartListeningArea( *pR, this );
        }
}

void ScChartListener::EndListeningTo()
{
    if ( aRangeListRef.Is() )
        for ( ScRangePtr pR = aRangeListRef->First(); pR;
                         pR = aRangeListRef->Next() )
        {
            if ( pR->aStart == pR->aEnd )
                pDoc->EndListeningCell( pR->aStart, this );
            else
                pDoc->EndListeningArea( *pR, this );
        }
}


void ScChartListener::ChangeListening( const ScRangeListRef& rRangeListRef,
            BOOL bDirtyP  )
{
    EndListeningTo();
    SetRangeList( rRangeListRef );
    StartListeningTo();
    if ( bDirtyP )
        SetDirty( TRUE );
}


void ScChartListener::SetRangeList( const ScRange& rRange )
{
    aRangeListRef = new ScRangeList;
    aRangeListRef->Append( rRange );
}


void ScChartListener::UpdateScheduledSeriesRanges()
{
    if ( bSeriesRangesScheduled )
    {
        bSeriesRangesScheduled = FALSE;
        UpdateSeriesRanges();
    }
}


void ScChartListener::UpdateSeriesRangesIntersecting( const ScRange& rRange )
{
    if ( aRangeListRef->Intersects( rRange ) )
        UpdateSeriesRanges();
}


void ScChartListener::UpdateSeriesRanges()
{
    pDoc->SetChartRangeList( GetString(), aRangeListRef );
}


BOOL ScChartListener::operator==( const ScChartListener& r )
{
    BOOL b1 = aRangeListRef.Is();
    BOOL b2 = r.aRangeListRef.Is();
    return
        pDoc == r.pDoc &&
        bUsed == r.bUsed &&
        bDirty == r.bDirty &&
        bSeriesRangesScheduled == r.bSeriesRangesScheduled &&
        GetString() == r.GetString() &&
        b1 == b2 &&
        ((!b1 && !b2) || (*aRangeListRef == *r.aRangeListRef))
        ;
}


// === ScChartListenerCollection ======================================

ScChartListenerCollection::ScChartListenerCollection( ScDocument* pDocP ) :
    StrCollection( 4, 4, FALSE ),
    pDoc( pDocP )
{
    aTimer.SetTimeoutHdl( LINK( this, ScChartListenerCollection, TimerHdl ) );
}

ScChartListenerCollection::ScChartListenerCollection(
        const ScChartListenerCollection& rColl ) :
    StrCollection( rColl ),
    pDoc( rColl.pDoc )
{
    aTimer.SetTimeoutHdl( LINK( this, ScChartListenerCollection, TimerHdl ) );
}

DataObject* ScChartListenerCollection::Clone() const
{
    return new ScChartListenerCollection( *this );
}

void ScChartListenerCollection::StartAllListeners()
{
    for ( USHORT nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ((ScChartListener*) pItems[ nIndex ])->StartListeningTo();
    }
}

void ScChartListenerCollection::ChangeListening( const String& rName,
        const ScRangeListRef& rRangeListRef, BOOL bDirty )
{
    ScChartListener aCLSearcher( rName, pDoc, rRangeListRef );
    ScChartListener* pCL;
    USHORT nIndex;
    if ( Search( &aCLSearcher, nIndex ) )
    {
        pCL = (ScChartListener*) pItems[ nIndex ];
        pCL->EndListeningTo();
        pCL->SetRangeList( rRangeListRef );
    }
    else
    {
        pCL = new ScChartListener( aCLSearcher );
        Insert( pCL );
    }
    pCL->StartListeningTo();
    if ( bDirty )
        pCL->SetDirty( TRUE );
}

void ScChartListenerCollection::FreeUnused()
{
    // rueckwaerts wg. Pointer-Aufrueckerei im Array
    for ( USHORT nIndex = nCount; nIndex-- >0; )
    {
        ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
        //  Uno-Charts nicht rauskicken
        //  (werden per FreeUno von aussen geloescht)
        if ( !pCL->IsUno() )
        {
            if ( pCL->IsUsed() )
                pCL->SetUsed( FALSE );
            else
                Free( pCL );
        }
    }
}

void ScChartListenerCollection::FreeUno( const uno::Reference< chart::XChartDataChangeEventListener >& rListener,
                                         const uno::Reference< chart::XChartData >& rSource )
{
    // rueckwaerts wg. Pointer-Aufrueckerei im Array
    for ( USHORT nIndex = nCount; nIndex-- >0; )
    {
        ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
        if ( pCL->IsUno() &&
             pCL->GetUnoListener() == rListener &&
             pCL->GetUnoSource() == rSource )
        {
            Free( pCL );
        }
        //! sollte nur einmal vorkommen?
    }
}

void ScChartListenerCollection::StartTimer()
{
    aTimer.SetTimeout( SC_CHARTTIMEOUT );
    aTimer.Start();
}

IMPL_LINK( ScChartListenerCollection, TimerHdl, Timer*, pTimer )
{
    if ( Application::AnyInput( INPUT_KEYBOARD ) )
    {
        aTimer.Start();
        return 0;
    }
    UpdateDirtyCharts();
    return 0;
}

void ScChartListenerCollection::UpdateDirtyCharts()
{
    for ( USHORT nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
        if ( pCL->IsDirty() )
            pCL->Update();
        if ( aTimer.IsActive() && !pDoc->IsImportingXML())
            break;                      // da kam einer dazwischen
    }
}


void ScChartListenerCollection::SetDirty()
{
    for ( USHORT nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
        pCL->SetDirty( TRUE );
    }
    StartTimer();
}


void ScChartListenerCollection::SetDiffDirty(
            const ScChartListenerCollection& rCmp, BOOL bSetChartRangeLists )
{
    BOOL bDirty = FALSE;
    for ( USHORT nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
        USHORT nFound;
        BOOL bFound = rCmp.Search( pCL, nFound );
        if ( !bFound || (*pCL != *((const ScChartListener*) rCmp.pItems[ nFound ])) )
        {
            if ( bSetChartRangeLists )
            {
                if ( bFound )
                {
                    const ScRangeListRef& rList1 = pCL->GetRangeList();
                    const ScRangeListRef& rList2 =
                        ((const ScChartListener*) rCmp.pItems[ nFound ])->GetRangeList();
                    BOOL b1 = rList1.Is();
                    BOOL b2 = rList2.Is();
                    if ( b1 != b2 || (b1 && b2 && (*rList1 != *rList2)) )
                        pDoc->SetChartRangeList( pCL->GetString(), rList1 );
                }
                else
                    pDoc->SetChartRangeList( pCL->GetString(), pCL->GetRangeList() );
            }
            bDirty = TRUE;
            pCL->SetDirty( TRUE );
        }
    }
    if ( bDirty )
        StartTimer();
}


void ScChartListenerCollection::SetRangeDirty( const ScRange& rRange )
{
    BOOL bDirty = FALSE;
    for ( USHORT nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
        const ScRangeListRef& rList = pCL->GetRangeList();
        if ( rList.Is() && rList->Intersects( rRange ) )
        {
            bDirty = TRUE;
            pCL->SetDirty( TRUE );
        }
    }
    if ( bDirty )
        StartTimer();
}


void ScChartListenerCollection::UpdateScheduledSeriesRanges()
{
    for ( USHORT nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
        pCL->UpdateScheduledSeriesRanges();
    }
}


void ScChartListenerCollection::UpdateSeriesRangesContainingTab( USHORT nTab )
{
    ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
    for ( USHORT nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
        pCL->UpdateSeriesRangesIntersecting( aRange );
    }
}


BOOL ScChartListenerCollection::operator==( const ScChartListenerCollection& r )
{
    // hier nicht StrCollection::operator==() verwenden, der umstaendlich via
    // IsEqual und Compare laeuft, stattdessen ScChartListener::operator==()
    if ( pDoc != r.pDoc || nCount != r.nCount )
        return FALSE;
    for ( USHORT nIndex = 0; nIndex < nCount; nIndex++ )
    {
        if ( *((ScChartListener*) pItems[ nIndex ]) !=
                *((ScChartListener*) r.pItems[ nIndex ]) )
            return FALSE;
    }
    return TRUE;
}




