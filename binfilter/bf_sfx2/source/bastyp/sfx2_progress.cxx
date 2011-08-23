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

#include "progress.hxx"

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <bf_svtools/eitem.hxx>

#include "appdata.hxx"
#include "app.hxx"
#include "request.hxx"
#include "sfxtypes.hxx"
#include "docfile.hxx"
#include "sfxsids.hrc"
#include "objsh.hxx"
#include "appuno.hxx"

#include <time.h>

namespace binfilter {


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;




struct SfxProgress_Impl : public SfxCancellable
{
    Reference < XStatusIndicator > xStatusInd;
    String                  aText, aStateText;
    ULONG					nMax;
    clock_t 				nCreate;
    clock_t					nNextReschedule;
    BOOL					bLocked, bAllDocs;
    BOOL					bWaitMode;
    BOOL					bAllowRescheduling;
    BOOL					bRunning;
    BOOL                    bIsStatusText;

    SfxProgress*            pActiveProgress;
    SfxObjectShellRef		xObjSh;

                            SfxProgress_Impl( const String& );
    void                    Enable_Impl( BOOL );

};

//========================================================================

#define TIMEOUT_PROGRESS	 	 5L	/* 10th s */
#define MAXPERCENT_PROGRESS		33

#define TIMEOUT_RESCHEDULE 		10L	/* 10th s */
#define MAXPERCENT_RESCHEDULE 	50

#define Progress

#define aTypeLibInfo aProgressTypeLibImpl

//========================================================================
//#if (_MSC_VER < 1300)
inline ULONG Get10ThSec()
{
#if defined (MSC) && defined (WIN)
    ULONG n10Ticks = 10 * (ULONG)GetTickCount();
#else
    ULONG n10Ticks = 10 * (ULONG)clock();
#endif

    return n10Ticks / CLOCKS_PER_SEC;
}
//#else
//extern ULONG Get10ThSec();
//#endif
// -----------------------------------------------------------------------

/*N*/ void SfxProgress_Impl::Enable_Impl( BOOL bEnable )
/*N*/ {
/*N*/   SfxObjectShell* pDoc = bAllDocs ? NULL : (SfxObjectShell*) xObjSh;
/*N*/
/*N*/ 	if ( pDoc )
/*N*/ 	{
/*N*/ 	}
/*N*/ 	else
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 SFX_APP()->LockDispatcher( !bEnable );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxProgress_Impl::SfxProgress_Impl( const String &rTitle )
/*N*/     :   SfxCancellable( SFX_APP()->GetCancelManager(), rTitle ),
/*N*/ 		pActiveProgress( 0 )
/*N*/ {
/*N*/     SFX_APP()->GetCancelManager()->RemoveCancellable(this);
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxProgress::SfxProgress
/*N*/ (
/*N*/ 	SfxObjectShell*	pObjSh, /* SfxObjectShell, an der die Aktion ausgef"uhrt
                               wird. Kann NULL sein, dann wird die Applikation
                                verwendet */
/*N*/
/*N*/ 	const String&	rText,	/* Text, der in der Statuszeile vor den Statusmonitor
                               erscheint */
/*N*/
/*N*/ 	ULONG			nRange, /* Maximalwert des Bereiches */
/*N*/
/*N*/ 	BOOL			bAll    /* alle Dokumente oder nur das Dokument des ViewFrames
                               disablen (FALSE) */
/*N*/ 	,BOOL			bWait   /* initial den Wait-Pointer aktivieren (TRUE) */
/*N*/ )

/*	[Beschreibung]

    Der Konstruktor der Klasse SfxProgress schaltet den als Parameter
    "ubergebenen SfxObjectShell und SfxViewFrames, welche dieses Dokument
    anzeigen in einen Progress-Mode. D.h. solange eine dieser SfxViewFrame
    Instanzen aktiv ist, ist der dazugeh"orige SfxDispatcher und das
    dazugeh"orige Window disabled. In der Statuszeile wird ein Balken zur
    Fortschritts-Anzeige angezeigt.
*/

/*N*/ :	nVal(0),
/*N*/ 	bSuspended(TRUE),
/*N*/ 	pImp( new SfxProgress_Impl( rText ) )
/*N*/ {
/*N*/ 	pImp->bRunning = TRUE;
/*N*/ 	pImp->bAllowRescheduling = Application::IsInExecute();;
/*N*/
/*N*/
/*N*/ 	pImp->xObjSh = pObjSh;
/*N*/ 	pImp->aText = rText;
/*N*/ 	pImp->nMax = nRange;
/*N*/ 	pImp->bLocked = FALSE;
/*N*/ 	pImp->bWaitMode = bWait;
/*N*/ 	pImp->bIsStatusText = FALSE;
/*N*/ 	pImp->nCreate = Get10ThSec();
/*N*/ 	pImp->nNextReschedule = pImp->nCreate;
/*N*/ 	DBG( DbgOutf( "SfxProgress: created for '%s' at %luds",
/*N*/ 				  rText.GetBuffer(), pImp->nCreate ) );
/*N*/ 	pImp->bAllDocs = bAll;
/*N*/
/*N*/ 	pImp->pActiveProgress = GetActiveProgress( pObjSh );
/*N*/ 	if ( pObjSh )
/*N*/ 		pObjSh->SetProgress_Impl(this);
/*N*/ 	else if( !pImp->pActiveProgress )
/*?*/         {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 SFX_APP()->SetProgress_Impl(this);
/*N*/ 	Resume();
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxProgress::~SfxProgress()

/*	[Beschreibung]

    Der Destruktor der Klasse SfxProgress restauriert den alten Zustand;
    die Dokumente werden wieder freigeschaltet und die Statuszeile zeigt
    wieder Items an.
*/

/*N*/ {
/*N*/ 	Stop();
/*N*/     if ( pImp->xStatusInd.is() )
/*?*/         pImp->xStatusInd->end();
/*N*/
/*N*/ 	if( pImp->bIsStatusText == TRUE )
/*?*/ 		GetpApp()->HideStatusText( );
/*N*/     SfxObjectShell* pDoc = pImp->xObjSh;
/*N*/ 	delete pImp;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SfxProgress::Stop()

/*	[Beschreibung]

    Vorzeitiges Beenden des <SfxProgress>.
*/

/*N*/ {
/*N*/     if( pImp->pActiveProgress )
/*N*/     {
/*?*/         if ( pImp->xObjSh.Is() && pImp->xObjSh->GetProgress() == this )
/*?*/             pImp->xObjSh->SetProgress_Impl(0);
/*?*/         return;
/*N*/     }
/*N*/
/*N*/ 	if ( !pImp->bRunning )
/*N*/ 		return;
/*N*/ 	pImp->bRunning = FALSE;
/*N*/ 	DBG( DbgOutf( "SfxProgress: destroyed at %luds", Get10ThSec() ) );
/*N*/
/*N*/ 	Suspend();
/*N*/ 	if ( pImp->xObjSh.Is() )
/*N*/ 		pImp->xObjSh->SetProgress_Impl(0);
/*N*/ 	else
/*?*/         {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 SFX_APP()->SetProgress_Impl(0);
/*N*/ 	if ( pImp->bLocked )
/*N*/         pImp->Enable_Impl(TRUE);
/*N*/ }

// -----------------------------------------------------------------------

/*?*/ void SfxProgress::SetText
/*?*/ (
/*?*/ 	const String&	rText	/*	neuer Text */
/*?*/ )

/*	[Beschreibung]

    "Andert den Text, der links neben dem Fortschritts-Balken
    angezeigt wird.
*/

/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*?*/ }

// -----------------------------------------------------------------------



// -----------------------------------------------------------------------

// muss in AppDaten
static ULONG nLastTime = 0;


// -----------------------------------------------------------------------

/*N*/ IMPL_STATIC_LINK( SfxProgress, DefaultBindingProgress, SvProgressArg*, pArg )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	return 0;
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ BOOL SfxProgress::SetState
/*N*/ (
/*N*/ 	ULONG	nNewVal,	/* neuer Wert f"ur die Fortschritts-Anzeige */
/*N*/
/*N*/ 	ULONG	nNewRange	/* neuer Maximalwert, 0 f"ur Beibehaltung des alten */
/*N*/ )
/*	[Beschreibung]

    Setzen des aktuellen Status; nach einem zeitlichen Versatz
    wird Reschedule aufgerufen.


    [R"uckgabewert]

    BOOL				TRUE
                        Fortfahren mit der Aktion

                        FALSE
                        Abbrechen der Aktion
*/

/*N*/ {
/*N*/ 	// wurde via Stop-Button angehalten?
/*N*/ 	if ( pImp->IsCancelled() )
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	if( pImp->pActiveProgress ) return TRUE;
/*N*/
/*N*/ 	// neuen Wert "ubernehmen
/*N*/ 	BOOL bOver=FALSE;
/*N*/ 	nVal = nNewVal;
/*N*/
/*N*/ 	// neuer Range?
/*N*/ 	if ( nNewRange && nNewRange != pImp->nMax )
/*N*/ 	{
/*?*/ 		DBG( DbgOutf( "SfxProgress: range changed from %lu to %lu",
/*?*/ 					  pImp->nMax, nNewRange ) );
/*?*/ 		pImp->nMax = nNewRange;
/*?*/ 		bOver = TRUE;
/*N*/ 	}
/*N*/
/*N*/     if ( !pImp->xStatusInd.is() )
/*N*/ 	{
/*N*/         // get the active ViewFrame of the document this progress is working on
/*N*/         // if it doesn't work on a document, take the current ViewFrame
/*N*/         SfxObjectShell* pObjSh = pImp->xObjSh;
/*N*/             {
/*N*/                 // don't show status indicator for hidden documents
/*N*/                 SfxMedium* pMedium = pObjSh->GetMedium();
/*N*/                 SFX_ITEMSET_ARG( pMedium->GetItemSet(), pHiddenItem, SfxBoolItem, SID_HIDDEN, FALSE );
/*N*/                 if ( !pHiddenItem || !pHiddenItem->GetValue() )
/*N*/                 {
/*N*/                     // not in a view, perhaps it's just loading
/*N*/                     {
/*N*/                         SFX_ITEMSET_ARG( pMedium->GetItemSet(), pIndicatorItem, SfxUnoAnyItem, SID_PROGRESS_STATUSBAR_CONTROL, FALSE );
/*N*/                         Reference< XStatusIndicator > xInd;
/*N*/                         if ( pIndicatorItem && (pIndicatorItem->GetValue()>>=xInd) )
/*?*/                             pImp->xStatusInd = xInd;
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/
/*N*/         if ( pImp->xStatusInd.is() )
/*N*/         {
/*?*/             pImp->xStatusInd->start( pImp->aText, pImp->nMax );
/*N*/         }
/*N*/ 	}
/*N*/
/*N*/ 	// schon ein StbManager?
/*N*/     if ( pImp->xStatusInd.is() )
/*N*/     {
/*?*/         pImp->xStatusInd->setValue( nNewVal );
/*N*/     }
/*N*/
/*N*/ 	return TRUE;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SfxProgress::Resume()

/*	[Beschreibung]

    Nimmt die Anzeige des Status nach einer Unterbrechung wieder auf.

    [Querverweise]
    <SfxProgress::Suspend()>
*/

/*N*/ {
/*N*/ 	if( pImp->pActiveProgress ) return;
/*N*/ 	if ( bSuspended )
/*N*/ 	{
/*N*/ 		DBG( DbgOutf( "SfxProgress: resumed" ) );
/*N*/         if ( pImp->xStatusInd.is() )
/*N*/         {
/*?*/             pImp->xStatusInd->start( pImp->aText, pImp->nMax );
/*?*/             pImp->xStatusInd->setValue( nVal );
/*N*/         }
/*N*/ 		bSuspended = FALSE;
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SfxProgress::Suspend()

/*	[Beschreibung]

    Unterbricht die Anzeige des Status

    [Querverweise]
    <SfxProgress::Resume()>
*/

/*N*/ {
/*N*/ 	if( pImp->pActiveProgress ) return;
/*N*/ 	if ( !bSuspended )
/*N*/ 	{
/*N*/ 		DBG( DbgOutf( "SfxProgress: suspended" ) );
/*N*/ 		bSuspended = TRUE;
/*N*/         if ( pImp->xStatusInd.is() )
/*N*/         {
/*?*/             pImp->xStatusInd->reset();
/*N*/         }
/*N*/
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SfxProgress::Reschedule()

/*	[Beschreibung]

    Reschedule von au"sen rufbar

*/

/*N*/ {
/*N*/ 	SFX_STACK(SfxProgress::Reschedule);
/*N*/
/*N*/ 	if( pImp->pActiveProgress ) return;
/*N*/     SfxApplication* pApp = SFX_APP();
/*N*/     if ( pImp->bLocked && 0 == pApp->Get_Impl()->nRescheduleLocks )
/*N*/ 	{
/*N*/         SfxAppData_Impl *pAppData = pApp->Get_Impl();
/*N*/ 		++pAppData->nInReschedule;
/*N*/ 		Application::Reschedule();
/*N*/ 		--pAppData->nInReschedule;
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ SfxProgress* SfxProgress::GetActiveProgress
/*N*/ (
/*N*/ 	SfxObjectShell* pDocSh	  /*  <SfxObjectShell>, die nach einem laufenden
                                    <SfxProgress> gefragt werden soll, oder
                                    0, wenn ein f"ur die gesamte Applikation
                                    laufender SfxProgress erfragt werden soll.
                                    Der Pointer braucht nur zum Zeitpunkt des
                                    Aufrufs g"ultig zu sein. */
/*N*/ )
/*N*/
/*N*/ /*	[Beschreibung]

    Mit dieser Methode kann erfragt werden, ob und welcher <SfxProgress>-
    f"ur eine bestimmte Instanz von SfxObjectShell oder gar die gesamte
    Applikation zur Zeit aktiv ist. Dies kann z.B. zum Abfangen von
    Time-Out-Events etc. verwendet werden.

    Anstelle eines Pointer auf den SfxProgress der SfxObjectShell wird
    ggf. der auf den SfxProgress der Applikation geliefert, mit der
    Abfrage 'SfxProgress::GetActiveProgress(pMyDocSh)' wird also
    insofern vorhanden der SfxProgress von 'pMyDocSh' geliefert,
    sonst der SfxProgress der Applikation bzw. ein 0-Pointer.


    [Anmerkung]

    "auft kein SfxProgress an der Applikation und ebenfalls keiner an
    der angegebenen SfxObjectShell, dann wird immer 0 zur"uckgeliefert,
    auch wenn an einer anderen SfxObjectShell ein SfxProgress l"uft.


    [Querverweise]

    <SfxApplication::GetProgress()const>
    <SfxObjectShell::GetProgress()const>
*/
/*N*/
/*N*/ {
/*N*/ 	SfxProgress *pProgress = 0;
/*N*/ 	if ( pDocSh )
/*N*/ 		pProgress = pDocSh->GetProgress();
/*N*/ 	if ( !pProgress )
/*N*/ 		pProgress = SFX_APP()->GetProgress();
/*N*/ 	return pProgress;
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


}
