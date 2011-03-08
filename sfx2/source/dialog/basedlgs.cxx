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
#include "precompiled_sfx2.hxx"

// include ---------------------------------------------------------------

#include <stdlib.h>
#include <vcl/fixed.hxx>
#include <vcl/help.hxx>
#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <unotools/viewoptions.hxx>
#include <svtools/fixedhyper.hxx>
#include <svtools/controldims.hrc>

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
#include "sfxresid.hxx"
#include "dialog.hrc"

using namespace ::com::sun::star::uno;
using namespace ::rtl;

#define USERITEM_NAME OUString(RTL_CONSTASCII_USTRINGPARAM("UserItem"))

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

SfxModalDialog::SfxModalDialog(Window* pParent, const ResId &rResId )

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur modale Dialoge;
    ResId wird als ID im ini-file verwendet.
    Die dort gespeicherte Position wird gesetzt.
*/

:   ModalDialog(pParent, rResId),
    nUniqId(rResId.GetId()),
    pInputSet(0),
    pOutputSet(0)
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
    nUniqId(nUniqueId),
    pInputSet(0),
    pOutputSet(0)
{
    init();
}

// -----------------------------------------------------------------------

SfxModalDialog::~SfxModalDialog()

/*      [Beschreibung]

    Dtor; schreibt Dialogposition in das ini-file
*/

{
    SetDialogData_Impl();
    aTimer.Stop();
    delete pOutputSet;
}

void SfxModalDialog::CreateOutputItemSet( SfxItemPool& rPool )
{
    DBG_ASSERT( !pOutputSet, "Double creation of OutputSet!" );
    if (!pOutputSet)
        pOutputSet = new SfxAllItemSet( rPool );
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxModalDialog, TimerHdl_Impl, Timer*, EMPTYARG )
{
    return 0L;
}

void SfxModalDialog::CreateOutputItemSet( const SfxItemSet& rSet )
{
    DBG_ASSERT( !pOutputSet, "Double creation of OutputSet!" );
    if (!pOutputSet)
    {
        pOutputSet = new SfxItemSet( rSet );
        pOutputSet->ClearItem();
    }
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
            SfxHelp::OpenHelpAgent( &pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame(), nHelpId );
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
            SfxHelp::OpenHelpAgent( &pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame(), nHelpId );
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

// SfxSingleTabDialog ----------------------------------------------------

IMPL_LINK( SfxSingleTabDialog, OKHdl_Impl, Button *, EMPTYARG )

/*      [Beschreibung]

    Ok_Handler; f"ur die gesetzte Page wird FillItemSet() gerufen.
*/

{
    if ( !GetInputItemSet() )
    {
        // TabPage without ItemSet
        EndDialog( RET_OK );
        return 1;
    }

    if ( !GetOutputItemSet() )
    {
        CreateOutputItemSet( *GetInputItemSet() );
    }
    sal_Bool bModified = sal_False;

    if ( pImpl->m_pSfxPage->HasExchangeSupport() )
    {
        int nRet = pImpl->m_pSfxPage->DeactivatePage( GetOutputSetImpl() );
        if ( nRet != SfxTabPage::LEAVE_PAGE )
            return 0;
        else
            bModified = ( GetOutputItemSet()->Count() > 0 );
    }
    else
        bModified = pImpl->m_pSfxPage->FillItemSet( *GetOutputSetImpl() );

    if ( bModified )
    {
        // auch noch schnell User-Daten im IniManager abspeichern
        pImpl->m_pSfxPage->FillUserData();
        String sData( pImpl->m_pSfxPage->GetUserData() );
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
    pImpl           ( new SingleTabDlgImpl )
{
    DBG_WARNING( "please use the ctor with ViewFrame" );
    SetInputSet( &rSet );
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
    pImpl           ( new SingleTabDlgImpl )
{
    DBG_WARNING( "bitte den Ctor mit ViewFrame verwenden" );
    SetInputSet( pInSet );
}

// -----------------------------------------------------------------------

SfxSingleTabDialog::SfxSingleTabDialog
(
    Window* pParent,
    sal_uInt16 nUniqueId,
    const String& rInfoURL
)

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur SingleTab-Dialoge;
    ID f"ur das ini-file wird "ubergeben.
 */

:   SfxModalDialog( pParent, nUniqueId, WinBits( WB_STDMODAL | WB_3DLOOK ) ),

    pOKBtn          ( NULL ),
    pCancelBtn      ( NULL ),
    pHelpBtn        ( NULL ),
    pImpl           ( new SingleTabDlgImpl )
{
    pImpl->m_sInfoURL = rInfoURL;
}

// -----------------------------------------------------------------------

SfxSingleTabDialog::~SfxSingleTabDialog()
{
    delete pOKBtn;
    delete pCancelBtn;
    delete pHelpBtn;
    delete pImpl->m_pTabPage;
    delete pImpl->m_pSfxPage;
    delete pImpl->m_pLine;
    delete pImpl->m_pInfoImage;
    delete pImpl;
}

// -----------------------------------------------------------------------

void SfxSingleTabDialog::SetPage( TabPage* pNewPage )
{
    if ( !pImpl->m_pLine )
        pImpl->m_pLine = new FixedLine( this );

    if ( !pOKBtn )
    {
        pOKBtn = new OKButton( this, WB_DEFBUTTON );
        pOKBtn->SetClickHdl( LINK( this, SfxSingleTabDialog, OKHdl_Impl ) );
    }

    if ( pImpl->m_sInfoURL.Len() > 0 && !pImpl->m_pInfoImage )
    {
        pImpl->m_pInfoImage = new ::svt::FixedHyperlinkImage( this );
        Image aInfoImage = Image( SfxResId( IMG_INFO ) );
        Size aImageSize = aInfoImage.GetSizePixel();
        aImageSize.Width() += 4;
        aImageSize.Height() += 4;
        pImpl->m_pInfoImage->SetSizePixel( aImageSize );
        pImpl->m_pInfoImage->SetImage( aInfoImage );
        pImpl->m_pInfoImage->SetURL( pImpl->m_sInfoURL );
        pImpl->m_pInfoImage->SetClickHdl( pImpl->m_aInfoLink );
    }

    if ( pImpl->m_pTabPage )
        delete pImpl->m_pTabPage;
    if ( pImpl->m_pSfxPage )
        delete pImpl->m_pSfxPage;
    pImpl->m_pTabPage = pNewPage;

    if ( pImpl->m_pTabPage )
    {
        // Gr"ossen und Positionen anpassen
        pImpl->m_pTabPage->SetPosPixel( Point() );
        Size aOutSz( pImpl->m_pTabPage->GetSizePixel() );
        Size aOffSz = LogicToPixel( Size( RSC_SP_CTRL_X, RSC_SP_CTRL_Y ), MAP_APPFONT );
        Size aFLSz = LogicToPixel( Size( aOutSz.Width(), RSC_CD_FIXEDLINE_HEIGHT ) );
        Size aBtnSz = LogicToPixel( Size( RSC_CD_PUSHBUTTON_WIDTH, RSC_CD_PUSHBUTTON_HEIGHT ), MAP_APPFONT );

        Point aPnt( 0, aOutSz.Height() );
        pImpl->m_pLine->SetPosSizePixel( aPnt, aFLSz );
        aPnt.X() = aOutSz.Width() - aOffSz.Width() - aBtnSz.Width();
        aPnt.Y() +=  aFLSz.Height() + ( aOffSz.Height() / 2 );
        pOKBtn->SetPosSizePixel( aPnt, aBtnSz );

        if ( pImpl->m_pInfoImage )
        {
            aPnt.X() = aOffSz.Width();
            long nDelta = ( pImpl->m_pInfoImage->GetSizePixel().Height() - aBtnSz.Height() ) / 2;
            aPnt.Y() -= nDelta;
            pImpl->m_pInfoImage->SetPosPixel( aPnt );
            pImpl->m_pInfoImage->Show();
        }

        aOutSz.Height() += aFLSz.Height() + ( aOffSz.Height() / 2 ) + aBtnSz.Height() + aOffSz.Height();
        SetOutputSizePixel( aOutSz );

        pImpl->m_pLine->Show();
        pOKBtn->Show();
        pImpl->m_pTabPage->Show();

        // Text der TabPage in den Dialog setzen
        SetText( pImpl->m_pTabPage->GetText() );

        // Dialog bekommt HelpId der TabPage
        SetHelpId( pImpl->m_pTabPage->GetHelpId() );
        SetUniqueId( pImpl->m_pTabPage->GetUniqueId() );
    }
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

    if ( pImpl->m_pTabPage )
        delete pImpl->m_pTabPage;
    if ( pImpl->m_pSfxPage )
        delete pImpl->m_pSfxPage;
    pImpl->m_pSfxPage = pTabPage;
    fnGetRanges = pRangesFunc;

    if ( pImpl->m_pSfxPage )
    {
        // erstmal die User-Daten besorgen, dann erst Reset()
        SvtViewOptions aPageOpt( E_TABPAGE, String::CreateFromInt32( GetUniqId() ) );
        String sUserData;
        Any aUserItem = aPageOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            sUserData = String( aTemp );
        pImpl->m_pSfxPage->SetUserData( sUserData );
        pImpl->m_pSfxPage->Reset( *GetInputItemSet() );
        pImpl->m_pSfxPage->Show();

        // Gr"ossen und Positionen anpassen
        pImpl->m_pSfxPage->SetPosPixel( Point() );
        Size aOutSz( pImpl->m_pSfxPage->GetSizePixel() );
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
        SetText( pImpl->m_pSfxPage->GetText() );

        // Dialog bekommt HelpId der TabPage
        SetHelpId( pImpl->m_pSfxPage->GetHelpId() );
        SetUniqueId( pImpl->m_pSfxPage->GetUniqueId() );
    }
}

// -----------------------------------------------------------------------

void SfxSingleTabDialog::SetInfoLink( const Link& rLink )
{
    pImpl->m_aInfoLink = rLink;
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
    if ( GetInputItemSet() )
    {
        OSL_FAIL( "Set bereits vorhanden!" );
        return GetInputItemSet()->GetRanges();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
