/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basedlgs.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 16:27:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

// include ---------------------------------------------------------------

#include <stdlib.h>

#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif
#ifndef GCC
#endif

#include <sfx2/basedlgs.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/viewsh.hxx>
#include "sfxhelp.hxx"
#include "workwin.hxx"

using namespace ::com::sun::star::uno;
using namespace ::rtl;

#define USERITEM_NAME OUString::createFromAscii( "UserItem" )

class SfxModelessDialog_Impl : public SfxListener
{
public:
    ByteString      aWinState;
    SfxChildWindow* pMgr;
    BOOL            bConstructed;
    void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    Timer           aMoveTimer;
};

void SfxModelessDialog_Impl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        switch( ( (SfxSimpleHint&) rHint ).GetId() )
        {
            case SFX_HINT_DYING:
                pMgr->Destroy();
                break;
        }
    }
}

class SfxFloatingWindow_Impl : public SfxListener
{
public:
    ByteString      aWinState;
    SfxChildWindow* pMgr;
    BOOL            bConstructed;
    Timer           aMoveTimer;

    void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

void SfxFloatingWindow_Impl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        switch( ( (SfxSimpleHint&) rHint ).GetId() )
        {
            case SFX_HINT_DYING:
                pMgr->Destroy();
                break;
        }
    }
}

// class SfxModalDefParentHelper -----------------------------------------

SfxModalDefParentHelper::SfxModalDefParentHelper( Window *pWindow)
{
    pOld = Application::GetDefDialogParent();
    Application::SetDefDialogParent( pWindow );
}

// -----------------------------------------------------------------------

SfxModalDefParentHelper::~SfxModalDefParentHelper()
{
    Application::SetDefDialogParent( pOld );
}

// -----------------------------------------------------------------------

void SfxModalDialog::SetDialogData_Impl()
{
    // save settings (position and user data)
    SvtViewOptions aDlgOpt( E_DIALOG, String::CreateFromInt32( nUniqId ) );
    aDlgOpt.SetWindowState(
        OUString::createFromAscii( GetWindowState( WINDOWSTATE_MASK_POS ).GetBuffer() ) );
    if ( aExtraData.Len() )
        aDlgOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( aExtraData ) ) );
}

// -----------------------------------------------------------------------

void SfxModalDialog::GetDialogData_Impl()

/*      [Beschreibung]

    Hilfsfunktion; liest die Dialogposition aus der Ini-Datei
    und setzt diese am "ubergebenen Window.
*/

{
    SvtViewOptions aDlgOpt( E_DIALOG, String::CreateFromInt32( nUniqId ) );
    if ( aDlgOpt.Exists() )
    {
        // load settings
        SetWindowState( ByteString( aDlgOpt.GetWindowState().getStr(), RTL_TEXTENCODING_ASCII_US ) );
        Any aUserItem = aDlgOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            aExtraData = String( aTemp );
    }
}

// -----------------------------------------------------------------------

void SfxModalDialog::init()
{
    GetDialogData_Impl();
    aTimer.SetTimeout( 100 );
    aTimer.SetTimeoutHdl( LINK( this, SfxModalDialog, TimerHdl_Impl ) );
    aTimer.Start();
}

// -----------------------------------------------------------------------

SfxModalDialog::SfxModalDialog(Window* pParent, const ResId &rResId)

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur modale Dialoge;
    ResId wird als ID im ini-file verwendet.
    Die dort gespeicherte Position wird gesetzt.
*/

:       ModalDialog(pParent, rResId),
    nUniqId(rResId.GetId())
{
    init();
}

// -----------------------------------------------------------------------

SfxModalDialog::SfxModalDialog(Window* pParent,
                               sal_uInt32 nUniqueId,
                               WinBits nWinStyle) :
/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur modale Dialoge;
    ID f"ur das ini-file wird explizit "ubergeben.
    Die dort gespeicherte Position wird gesetzt.
*/

    ModalDialog(pParent, nWinStyle),
    nUniqId(nUniqueId)
{
    init();
}

// -----------------------------------------------------------------------

SfxModalDialog::~SfxModalDialog()

/*      [Beschreibung]

    Dtor; schreibt Dialogposition in das ini-file
*/

{
/*
    SfxHelpPI *pHelpPI = SFX_APP()->GetHelpPI();
    if ( pHelpPI )
        pHelpPI->ResetTopic();
*/
    SetDialogData_Impl();
    aTimer.Stop();
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxModalDialog, TimerHdl_Impl, Timer*, EMPTYARG )
{
/*
    SfxHelpPI *pHelpPI = SFX_APP()->GetHelpPI();
    if ( pHelpPI )
        pHelpPI->LoadTopic( GetHelpId() );
 */
    return 0L;
}

//-------------------------------------------------------------------------
void SfxModelessDialog::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        if ( pImp->aWinState.Len() )
        {
            SetWindowState( pImp->aWinState );
        }
        else
        {
            Point aPos = GetPosPixel();
            if ( !aPos.X() )
            {
                aSize = GetSizePixel();

                Size aParentSize = GetParent()->GetOutputSizePixel();
                Size aDlgSize = GetSizePixel();
                aPos.X() += ( aParentSize.Width() - aDlgSize.Width() ) / 2;
                aPos.Y() += ( aParentSize.Height() - aDlgSize.Height() ) / 2;

                Point aPoint;
                Rectangle aRect = GetDesktopRectPixel();
                aPoint.X() = aRect.Right() - aDlgSize.Width();
                aPoint.Y() = aRect.Bottom() - aDlgSize.Height();

                aPoint = OutputToScreenPixel( aPoint );

                if ( aPos.X() > aPoint.X() )
                    aPos.X() = aPoint.X() ;
                if ( aPos.Y() > aPoint.Y() )
                    aPos.Y() = aPoint.Y();

                if ( aPos.X() < 0 ) aPos.X() = 0;
                if ( aPos.Y() < 0 ) aPos.Y() = 0;

                SetPosPixel( aPos );
            }
        }

        pImp->bConstructed = TRUE;
    }

    ModelessDialog::StateChanged( nStateChange );
}

void SfxModelessDialog::Initialize(SfxChildWinInfo *pInfo)

/*  [Beschreibung]

    Initialisierung der Klasse SfxModelessDialog "uber ein SfxChildWinInfo.
    Die Initialisierung erfolgt erst in einem 2.Schritt
    nach dem ctor und sollte vom ctor der abgeleiteten Klasse
    oder von dem des SfxChildWindows aufgerufen werden.
*/

{
    pImp->aWinState = pInfo->aWinState;
}

void SfxModelessDialog::Resize()

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse FloatingWindow merkt sich ggf. eine
    ver"anderte Gr"o\se.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    auch SfxFloatingWindow::Resize() gerufen werden.
*/

{
    ModelessDialog::Resize();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        // start timer for saving window status information
        pImp->aMoveTimer.Start();
    }
}

void SfxModelessDialog::Move()
{
    ModelessDialog::Move();
    if ( pImp->bConstructed && pImp->pMgr && IsReallyVisible() )
    {
        // start timer for saving window status information
        pImp->aMoveTimer.Start();
    }
}

/*
    Implements a timer event that is triggered by a move or resize of the window
    This will save config information to Views.xcu with a small delay
*/
IMPL_LINK( SfxModelessDialog, TimerHdl, Timer*, EMPTYARG)
{
    pImp->aMoveTimer.Stop();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        if ( !IsRollUp() )
            aSize = GetSizePixel();
        ULONG nMask = WINDOWSTATE_MASK_POS | WINDOWSTATE_MASK_STATE;
        if ( GetStyle() & WB_SIZEABLE )
            nMask |= ( WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT );
        pImp->aWinState = GetWindowState( nMask );
        GetBindings().GetWorkWindow_Impl()->ConfigChild_Impl( SFX_CHILDWIN_DOCKINGWINDOW, SFX_ALIGNDOCKINGWINDOW, pImp->pMgr->GetType() );
    }
    return 0;
}

// -----------------------------------------------------------------------

SfxModelessDialog::SfxModelessDialog( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        Window* pParent, WinBits nWinBits ) :
    ModelessDialog (pParent, nWinBits),
    pBindings(pBindinx),
    pImp( new SfxModelessDialog_Impl )
{
    pImp->pMgr = pCW;
    pImp->bConstructed = FALSE;
    sal_uInt32 nId = GetHelpId();
    SetHelpId(0);
    SetUniqueId( nId );
    if ( pBindinx )
        pImp->StartListening( *pBindinx );
    pImp->aMoveTimer.SetTimeout(50);
    pImp->aMoveTimer.SetTimeoutHdl(LINK(this,SfxModelessDialog,TimerHdl));
}

// -----------------------------------------------------------------------

SfxModelessDialog::SfxModelessDialog( SfxBindings *pBindinx,
                        SfxChildWindow *pCW, Window *pParent,
                        const ResId& rResId ) :
    ModelessDialog(pParent, rResId),
    pBindings(pBindinx),
    pImp( new SfxModelessDialog_Impl )
{
    pImp->pMgr = pCW;
    pImp->bConstructed = FALSE;
    sal_uInt32 nId = GetHelpId();
    SetHelpId(0);
    SetUniqueId( nId );
    if ( pBindinx )
        pImp->StartListening( *pBindinx );
    pImp->aMoveTimer.SetTimeout(50);
    pImp->aMoveTimer.SetTimeoutHdl(LINK(this,SfxModelessDialog,TimerHdl));
}

// -----------------------------------------------------------------------

long SfxModelessDialog::Notify( NotifyEvent& rEvt )

/*      [Beschreibung]

    Wenn ein ModelessDialog aktiviert wird, wird sein ViewFrame aktiviert.
    Notwendig ist das bei PlugInFrames.
*/

{
    if ( rEvt.GetType() == EVENT_GETFOCUS )
    {
        pBindings->SetActiveFrame( pImp->pMgr->GetFrame() );
        pImp->pMgr->Activate_Impl();
        Window* pWindow = rEvt.GetWindow();
        ULONG nHelpId  = 0;
        while ( !nHelpId && pWindow )
        {
            nHelpId = pWindow->GetHelpId();
            pWindow = pWindow->GetParent();
        }

        if ( nHelpId )
            SfxHelp::OpenHelpAgent( pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame(), nHelpId );
    }
    else if ( rEvt.GetType() == EVENT_LOSEFOCUS && !HasChildPathFocus() )
    {
        pBindings->SetActiveFrame( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > () );
        pImp->pMgr->Deactivate_Impl();
    }
    else if( rEvt.GetType() == EVENT_KEYINPUT )
    {
        // KeyInput zuerst f"ur Dialogfunktionen zulassen ( TAB etc. )
        if ( !ModelessDialog::Notify( rEvt ) && SfxViewShell::Current() )
            // dann auch global g"ultige Acceleratoren verwenden
            return SfxViewShell::Current()->GlobalKeyInput_Impl( *rEvt.GetKeyEvent() );
        return sal_True;
    }

    return ModelessDialog::Notify( rEvt );
}

// -----------------------------------------------------------------------

SfxModelessDialog::~SfxModelessDialog()

/*      [Beschreibung]

    Dtor
*/

{
    if ( pImp->pMgr->GetFrame().is() && pImp->pMgr->GetFrame() == pBindings->GetActiveFrame() )
        pBindings->SetActiveFrame( NULL );
    delete pImp;
}

//-------------------------------------------------------------------------

sal_Bool SfxModelessDialog::Close()

/*      [Beschreibung]

    Das Fenster wird geschlossen, indem das ChildWindow durch Ausf"uhren des
    ChildWindow-Slots zerst"ort wird.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    danach SfxModelessDialogWindow::Close() gerufen werden, wenn nicht das
    Close() mit "return sal_False" abgebrochen wird.

*/

{
    // Execute mit Parametern, da Toggle von einigen ChildWindows ignoriert
    // werden kann
    SfxBoolItem aValue( pImp->pMgr->GetType(), sal_False);
    pBindings->GetDispatcher_Impl()->Execute(
        pImp->pMgr->GetType(),
        SFX_CALLMODE_RECORD|SFX_CALLMODE_SYNCHRON, &aValue, 0L );
    return sal_True;
}

//-------------------------------------------------------------------------

void SfxModelessDialog::FillInfo(SfxChildWinInfo& rInfo) const

/*  [Beschreibung]

    F"ullt ein SfxChildWinInfo mit f"ur SfxModelessDialof spezifischen Daten,
    damit sie in die INI-Datei geschrieben werden koennen.
    Es wird angenommen, da\s rInfo alle anderen evt. relevanten Daten in
    der ChildWindow-Klasse erh"alt.
    ModelessDialogs haben keine spezifischen Informationen, so dass die
    Basisimplementierung nichts tut und daher nicht gerufen werden mu\s.
*/

{
    rInfo.aSize  = aSize;
    if ( IsRollUp() )
        rInfo.nFlags |= SFX_CHILDWIN_ZOOMIN;
}

// -----------------------------------------------------------------------

long SfxFloatingWindow::Notify( NotifyEvent& rEvt )

/*      [Beschreibung]

    Wenn ein ModelessDialog aktiviert wird, wird sein ViewFrame aktiviert.
    Notwendig ist das bei PlugInFrames.
*/

{
    if ( rEvt.GetType() == EVENT_GETFOCUS )
    {
        pBindings->SetActiveFrame( pImp->pMgr->GetFrame() );
        pImp->pMgr->Activate_Impl();
        Window* pWindow = rEvt.GetWindow();
        ULONG nHelpId  = 0;
        while ( !nHelpId && pWindow )
        {
            nHelpId = pWindow->GetHelpId();
            pWindow = pWindow->GetParent();
        }

        if ( nHelpId )
            SfxHelp::OpenHelpAgent( pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame(), nHelpId );
    }
    else if ( rEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( !HasChildPathFocus() )
        {
            pBindings->SetActiveFrame( NULL );
            pImp->pMgr->Deactivate_Impl();
        }
    }
    else if( rEvt.GetType() == EVENT_KEYINPUT )
    {
        // KeyInput zuerst f"ur Dialogfunktionen zulassen
        if ( !FloatingWindow::Notify( rEvt ) && SfxViewShell::Current() )
            // dann auch global g"ultige Acceleratoren verwenden
            return SfxViewShell::Current()->GlobalKeyInput_Impl( *rEvt.GetKeyEvent() );
        return sal_True;
    }

    return FloatingWindow::Notify( rEvt );
}

// -----------------------------------------------------------------------

SfxFloatingWindow::SfxFloatingWindow( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        Window* pParent, WinBits nWinBits) :
    FloatingWindow (pParent, nWinBits),
    pBindings(pBindinx),
    pImp( new SfxFloatingWindow_Impl )
{
    pImp->pMgr = pCW;
    pImp->bConstructed = FALSE;
    sal_uInt32 nId = GetHelpId();
    SetHelpId(0);
    SetUniqueId( nId );
    if ( pBindinx )
        pImp->StartListening( *pBindinx );
    pImp->aMoveTimer.SetTimeout(50);
    pImp->aMoveTimer.SetTimeoutHdl(LINK(this,SfxFloatingWindow,TimerHdl));
}

// -----------------------------------------------------------------------

SfxFloatingWindow::SfxFloatingWindow( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        Window* pParent,
                        const ResId& rResId) :
    FloatingWindow(pParent, rResId),
    pBindings(pBindinx),
    pImp( new SfxFloatingWindow_Impl )
{
    pImp->pMgr = pCW;
    pImp->bConstructed = FALSE;
    sal_uInt32 nId = GetHelpId();
    SetHelpId(0);
    SetUniqueId( nId );
    if ( pBindinx )
        pImp->StartListening( *pBindinx );
    pImp->aMoveTimer.SetTimeout(50);
    pImp->aMoveTimer.SetTimeoutHdl(LINK(this,SfxFloatingWindow,TimerHdl));
}

//-------------------------------------------------------------------------

sal_Bool SfxFloatingWindow::Close()

/*      [Beschreibung]

    Das Fenster wird geschlossen, indem das ChildWindow durch Ausf"uhren des
    ChildWindow-Slots zerst"ort wird.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    danach SfxFloatingWindow::Close() gerufen werden, wenn nicht das Close()
    mit "return sal_False" abgebrochen wird.

*/

{
    // Execute mit Parametern, da Toggle von einigen ChildWindows ignoriert
    // werden kann
    SfxBoolItem aValue( pImp->pMgr->GetType(), sal_False);
    pBindings->GetDispatcher_Impl()->Execute(
            pImp->pMgr->GetType(),
            SFX_CALLMODE_RECORD|SFX_CALLMODE_SYNCHRON, &aValue, 0L );
    return sal_True;
}

// -----------------------------------------------------------------------

SfxFloatingWindow::~SfxFloatingWindow()

/*      [Beschreibung]

    Dtor
*/

{
    if ( pImp->pMgr->GetFrame() == pBindings->GetActiveFrame() )
        pBindings->SetActiveFrame( NULL );
    delete pImp;
}

//-------------------------------------------------------------------------

void SfxFloatingWindow::Resize()

/*  [Beschreibung]

    Diese virtuelle Methode der Klasse FloatingWindow merkt sich ggf. eine
    ver"anderte Gr"o\se.
    Wird diese Methode von einer abgeleiteten Klasse "uberschrieben, mu\s
    auch SfxFloatingWindow::Resize() gerufen werden.
*/

{
    FloatingWindow::Resize();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        // start timer for saving window status information
        pImp->aMoveTimer.Start();
    }
}

void SfxFloatingWindow::Move()
{
    FloatingWindow::Move();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        // start timer for saving window status information
        pImp->aMoveTimer.Start();
    }
}

/*
    Implements a timer event that is triggered by a move or resize of the window
    This will save config information to Views.xcu with a small delay
*/
IMPL_LINK( SfxFloatingWindow, TimerHdl, Timer*, EMPTYARG)
{
    pImp->aMoveTimer.Stop();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        if ( !IsRollUp() )
            aSize = GetSizePixel();
        ULONG nMask = WINDOWSTATE_MASK_POS | WINDOWSTATE_MASK_STATE;
        if ( GetStyle() & WB_SIZEABLE )
            nMask |= ( WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT );
        pImp->aWinState = GetWindowState( nMask );
        GetBindings().GetWorkWindow_Impl()->ConfigChild_Impl( SFX_CHILDWIN_DOCKINGWINDOW, SFX_ALIGNDOCKINGWINDOW, pImp->pMgr->GetType() );
    }
    return 0;
}

//-------------------------------------------------------------------------
void SfxFloatingWindow::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        // FloatingWindows are not centered by default
        if ( pImp->aWinState.Len() )
            SetWindowState( pImp->aWinState );
        pImp->bConstructed = TRUE;
    }

    FloatingWindow::StateChanged( nStateChange );
}


void SfxFloatingWindow::Initialize(SfxChildWinInfo *pInfo)

/*  [Beschreibung]

    Initialisierung der Klasse SfxFloatingWindow "uber ein SfxChildWinInfo.
    Die Initialisierung erfolgt erst in einem 2.Schritt
    nach dem ctor und sollte vom ctor der abgeleiteten Klasse
    oder von dem des SfxChildWindows aufgerufen werden.
*/
{
    pImp->aWinState = pInfo->aWinState;
}

//-------------------------------------------------------------------------

void SfxFloatingWindow::FillInfo(SfxChildWinInfo& rInfo) const

/*  [Beschreibung]

    F"ullt ein SfxChildWinInfo mit f"ur SfxFloatingWindow spezifischen Daten,
    damit sie in die INI-Datei geschrieben werden koennen.
    Es wird angenommen, da\s rInfo alle anderen evt. relevanten Daten in
    der ChildWindow-Klasse erh"alt.
    Eingetragen werden hier gemerkte Gr"o\se und das ZoomIn-Flag.
    Wird diese Methode "uberschrieben, mu\s zuerst die Basisimplementierung
    gerufen werden.
*/

{
    rInfo.aSize  = aSize;
    if ( IsRollUp() )
        rInfo.nFlags |= SFX_CHILDWIN_ZOOMIN;
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxSingleTabDialog, OKHdl_Impl, Button *, pButton )

/*      [Beschreibung]

    Ok_Handler; f"ur die gesetzte Page wird FillItemSet() gerufen.
*/

{
    (void)pButton; //unused
    if ( !pOutSet )
    {
        pOutSet = new SfxItemSet( *pOptions );
        pOutSet->ClearItem();
    }
    sal_Bool bModified = sal_False;

    if ( pPage->HasExchangeSupport() )
    {
        int nRet = pPage->DeactivatePage( pOutSet );

        if ( nRet != SfxTabPage::LEAVE_PAGE )
            return 0;
        else
            bModified = ( pOutSet->Count() > 0 );
    }
    else
        bModified = pPage->FillItemSet( *pOutSet );

    if ( bModified )
    {
        // auch noch schnell User-Daten im IniManager abspeichern
        pPage->FillUserData();
        String sData( pPage->GetUserData() );
        SvtViewOptions aPageOpt( E_TABPAGE, String::CreateFromInt32( GetUniqId() ) );
        aPageOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( sData ) ) );
        EndDialog( RET_OK );
    }
    else
        EndDialog( RET_CANCEL );
    return 0;
}

// -----------------------------------------------------------------------

SfxSingleTabDialog::SfxSingleTabDialog
(
    Window *pParent,
    const SfxItemSet& rSet,
    sal_uInt16 nUniqueId
) :

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur SingleTab-Dialoge;
    ID f"ur das ini-file wird "ubergeben.
*/

    SfxModalDialog( pParent, nUniqueId, WinBits( WB_STDMODAL | WB_3DLOOK ) ),

    pOKBtn          ( 0 ),
    pCancelBtn      ( 0 ),
    pHelpBtn        ( 0 ),
    pPage           ( 0 ),
    pOptions        ( &rSet ),
    pOutSet         ( 0 )

{
    DBG_WARNING( "please use the ctor with ViewFrame" );
}

// -----------------------------------------------------------------------

SfxSingleTabDialog::SfxSingleTabDialog
(
    Window* pParent,
    sal_uInt16 nUniqueId,
    const SfxItemSet* pInSet
)

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur SingleTab-Dialoge;
    ID f"ur das ini-file wird "ubergeben.
    Sollte nicht mehr benutzt werden.
 */

:   SfxModalDialog( pParent, nUniqueId, WinBits( WB_STDMODAL | WB_3DLOOK ) ),

    pOKBtn          ( 0 ),
    pCancelBtn      ( 0 ),
    pHelpBtn        ( 0 ),
    pPage           ( 0 ),
    pOptions        ( pInSet ),
    pOutSet         ( 0 )

{
    DBG_WARNING( "bitte den Ctor mit ViewFrame verwenden" );
}

// -----------------------------------------------------------------------

SfxSingleTabDialog::~SfxSingleTabDialog()

/*      [Beschreibung]

    Dtor; l"oscht ggf. die TabPage.
 */

{
    delete pOKBtn;
    delete pCancelBtn;
    delete pHelpBtn;
    delete pPage;
    delete pOutSet;
}

// -----------------------------------------------------------------------

void SfxSingleTabDialog::SetTabPage( SfxTabPage* pTabPage,
                                     GetTabPageRanges pRangesFunc )
/*      [Beschreibung]

    Setzen einer (neuen) TabPage; eine bereits vorhandene Page
    wird gel"oscht.
    Die "ubergebene Page wird durch Aufruf von Reset() mit dem
    initial "ubergebenen Itemset initialisiert.
*/

{
    if ( !pOKBtn )
    {
        pOKBtn = new OKButton( this, WB_DEFBUTTON );
        pOKBtn->SetClickHdl( LINK( this, SfxSingleTabDialog, OKHdl_Impl ) );
    }
    if ( !pCancelBtn )
        pCancelBtn = new CancelButton( this );
    if ( !pHelpBtn )
        pHelpBtn = new HelpButton( this );

    if ( pPage )
        delete pPage;
    pPage = pTabPage;
    fnGetRanges = pRangesFunc;

    if ( pPage )
    {
        // erstmal die User-Daten besorgen, dann erst Reset()
        SvtViewOptions aPageOpt( E_TABPAGE, String::CreateFromInt32( GetUniqId() ) );
        String sUserData;
        Any aUserItem = aPageOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            sUserData = String( aTemp );
        pPage->SetUserData( sUserData );
        pPage->Reset( *pOptions );
        pPage->Show();

        // Gr"ossen und Positionen anpassen
        pPage->SetPosPixel( Point() );
        Size aOutSz( pPage->GetSizePixel() );
        Size aBtnSiz = LogicToPixel( Size( 50, 14 ), MAP_APPFONT );
        Point aPnt( aOutSz.Width(), LogicToPixel( Point( 0, 6 ), MAP_APPFONT ).Y() );
        aOutSz.Width() += aBtnSiz.Width() + LogicToPixel( Size( 6, 0 ), MAP_APPFONT ).Width();
        SetOutputSizePixel( aOutSz );
        pOKBtn->SetPosSizePixel( aPnt, aBtnSiz );
        pOKBtn->Show();
        aPnt.Y() = LogicToPixel( Point( 0, 23 ), MAP_APPFONT ).Y();
        pCancelBtn->SetPosSizePixel( aPnt, aBtnSiz );
        pCancelBtn->Show();
        aPnt.Y() = LogicToPixel( Point( 0, 43 ), MAP_APPFONT ).Y();
        pHelpBtn->SetPosSizePixel( aPnt, aBtnSiz );

        if ( Help::IsContextHelpEnabled() )
            pHelpBtn->Show();

        // Text der TabPage in den Dialog setzen
        SetText( pPage->GetText() );

        // Dialog bekommt HelpId der TabPage
        SetHelpId( pPage->GetHelpId() );
        SetUniqueId( pPage->GetUniqueId() );
    }
}

//--------------------------------------------------------------------
// Vergleichsfunktion fuer qsort

#ifdef WNT
int __cdecl BaseDlgsCmpUS_Impl( const void* p1, const void* p2 )
#else
#if defined(OS2) && defined(ICC)
int _Optlink BaseDlgsCmpUS_Impl(        const void* p1, const void* p2 )
#else
extern "C" int BaseDlgsCmpUS_Impl( const void* p1, const void* p2 )
#endif
#endif
{
    return *(sal_uInt16*)p1 - *(sal_uInt16*)p2;
}

// -----------------------------------------------------------------------

/*
    Bildet das Set "uber die Ranges der Page. Die Page muss die statische
    Methode f"ur das Erfragen ihrer Ranges bei SetTabPage angegeben haben,
    liefert also ihr Set onDemand.
 */
const sal_uInt16* SfxSingleTabDialog::GetInputRanges( const SfxItemPool& rPool )
{
    if ( pOptions )
    {
        DBG_ERROR( "Set bereits vorhanden!" );
        return pOptions->GetRanges();
    }

    if ( pRanges )
        return pRanges;
    SvUShorts aUS(16, 16);

    if ( fnGetRanges)
    {
        const sal_uInt16 *pTmpRanges = (fnGetRanges)();
        const sal_uInt16 *pIter = pTmpRanges;
        sal_uInt16 nLen;
        for ( nLen = 0; *pIter; ++nLen, ++pIter )
            ;
        aUS.Insert( pTmpRanges, nLen, aUS.Count() );
    }

    //! Doppelte Ids entfernen?
    sal_uInt16 nCount = aUS.Count();

    for ( sal_uInt16 i = 0; i < nCount; ++i )
        aUS[i] = rPool.GetWhich( aUS[i]) ;

    // sortieren
    if ( aUS.Count() > 1 )
        qsort( (void*)aUS.GetData(), aUS.Count(), sizeof(sal_uInt16), BaseDlgsCmpUS_Impl );

    pRanges = new sal_uInt16[aUS.Count() + 1];
    memcpy( pRanges, aUS.GetData(), sizeof(sal_uInt16) * aUS.Count() );
    pRanges[aUS.Count()] = 0;
    return pRanges;
}

