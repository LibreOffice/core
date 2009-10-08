/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chartlis.cxx,v $
 * $Revision: 1.11 $
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



#include <vcl/svapp.hxx>

#include "chartlis.hxx"
#include "brdcst.hxx"
#include "document.hxx"

using namespace com::sun::star;


//2do: DocOption TimeOut?
//#define SC_CHARTTIMEOUT 1000      // eine Sekunde keine Aenderung/KeyEvent

// Update chart listeners quickly, to get a similar behavior to loaded charts
// which register UNO listeners.
#define SC_CHARTTIMEOUT 10


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
    pUnoData( NULL ),
    pDoc( pDocP ),
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
    pUnoData( NULL ),
    pDoc( pDocP ),
    bUsed( FALSE ),
    bDirty( FALSE ),
    bSeriesRangesScheduled( FALSE )
{
}

ScChartListener::ScChartListener( const ScChartListener& r ) :
        StrData( r ),
        SvtListener(),
        pUnoData( NULL ),
        pDoc( r.pDoc ),
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
    if ( HasBroadcaster() )
        EndListeningTo();
    delete pUnoData;
}

ScDataObject* ScChartListener::Clone() const
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

void __EXPORT ScChartListener::Notify( SvtBroadcaster&, const SfxHint& rHint )
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
                                        chart::ChartDataChangeType_ALL,
                                        0, 0, 0, 0 );
        pUnoData->GetListener()->chartDataChanged( aEvent );
    }
    else if ( pDoc->GetAutoCalc() )
    {
        bDirty = FALSE;
        pDoc->UpdateChart( GetString());
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


void ScChartListener::UpdateChartIntersecting( const ScRange& rRange )
{
    if ( aRangeListRef->Intersects( rRange ) )
    {
        // force update (chart has to be loaded), don't use ScChartListener::Update
        pDoc->UpdateChart( GetString());
    }
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
    ScStrCollection( 4, 4, FALSE ),
    pDoc( pDocP )
{
    aTimer.SetTimeoutHdl( LINK( this, ScChartListenerCollection, TimerHdl ) );
}

ScChartListenerCollection::ScChartListenerCollection(
        const ScChartListenerCollection& rColl ) :
    ScStrCollection( rColl ),
    pDoc( rColl.pDoc )
{
    aTimer.SetTimeoutHdl( LINK( this, ScChartListenerCollection, TimerHdl ) );
}

ScChartListenerCollection::~ScChartListenerCollection()
{
    //  #96783# remove ChartListener objects before aTimer dtor is called, because
    //  ScChartListener::EndListeningTo may cause ScChartListenerCollection::StartTimer
    //  to be called if an empty ScNoteCell is deleted

    if (GetCount())
        FreeAll();
}

ScDataObject*   ScChartListenerCollection::Clone() const
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

IMPL_LINK( ScChartListenerCollection, TimerHdl, Timer*, EMPTYARG )
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


void ScChartListenerCollection::UpdateChartsContainingTab( SCTAB nTab )
{
    ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
    for ( USHORT nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
        pCL->UpdateChartIntersecting( aRange );
    }
}


BOOL ScChartListenerCollection::operator==( const ScChartListenerCollection& r )
{
    // hier nicht ScStrCollection::operator==() verwenden, der umstaendlich via
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




