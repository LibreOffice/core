/*************************************************************************
 *
 *  $RCSfile: basedlgs.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:46 $
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

// include ---------------------------------------------------------------

#include <stdlib.h>

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#if SUPD<613//MUSTINI
    #ifndef _SFXINIMGR_HXX //autogen
    #include <svtools/iniman.hxx>
    #endif
#endif

#pragma hdrstop

#include "basedlgs.hxx"
#include "viewfrm.hxx"
#include "tabdlg.hxx"
#include "app.hxx"
#if SUPD<613//MUSTINI
#include "inimgr.hxx"
#endif
#include "bindings.hxx"
#include "dispatch.hxx"
#include "sfxhelp.hxx"
#include "accmgr.hxx"

static String aEmptyString;

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

void SetDialogData_Impl(SfxViewFrame *pFrame, Window *pDlg,
                        sal_uInt16 nId, const String &rExtraData = aEmptyString)
{
    // Konfiguration in Ini-Manager abspeichern
#if SUPD<613//MUSTINI
    SfxIniManager *pIniMgr =
        pFrame ? pFrame->GetIniManager() : SFX_APP()->GetAppIniManager();
    String aDlgData( pIniMgr->GetString( pDlg->GetPosPixel(), Size() ) );
    if( rExtraData.Len() )
    {
        aDlgData += pIniMgr->GetToken();
        aDlgData += rExtraData;
    }
    pIniMgr->Set( aDlgData, SFX_KEY_DIALOG, nId );
#endif
}

// -----------------------------------------------------------------------

String GetDialogData_Impl( SfxViewFrame *pFrame, Window *pDlg, sal_uInt16 nId)

/*      [Beschreibung]

    Hilfsfunktion; liest die Dialogposition aus der Ini-Datei
    und setzt diese am "ubergebenen Window.
*/

{
    String aRetString;
    // Konfiguration vorhanden?
#if SUPD<613//MUSTINI
    SfxIniManager *pIniMgr =
        pFrame ? pFrame->GetIniManager() : SFX_APP()->GetAppIniManager();
    String aDlgData( pIniMgr->Get( SFX_KEY_DIALOG, nId ) );
    if ( aDlgData.Len() > 0 )
    {
        // Konfiguration verwenden
        Point aPos;
        Size aDummySize;
        if ( pIniMgr->GetPosSize(
                aDlgData.GetToken(
                    0, pIniMgr->GetToken()), aPos, aDummySize ) )
        {
            pDlg->SetPosPixel(aPos);
            sal_uInt16 nPos = aDlgData.Search( pIniMgr->GetToken() );
            if( nPos != STRING_NOTFOUND )
            {
                aRetString = aDlgData.Copy( nPos + 1 );
                aDlgData.Erase(nPos+1);
            }
        }
    }
#endif
    return aRetString;
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
    DBG_WARNING( "bitte den Ctor mit ViewFrame verwenden" );

    aExtraData = GetDialogData_Impl(0, this, nUniqId);
    aTimer.SetTimeout( 100 );
    aTimer.SetTimeoutHdl( LINK( this, SfxModalDialog, TimerHdl_Impl ) );
    aTimer.Start();
}

// -----------------------------------------------------------------------

SfxModalDialog::SfxModalDialog(Window* pParent,
                               sal_uInt16 nUniqueId,
                               WinBits nWinStyle) :
/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur modale Dialoge;
    ID f"ur das ini-file wird explizit "ubergeben.
    Die dort gespeicherte Position wird gesetzt.
*/

    ModalDialog(pParent, nWinStyle),
    nUniqId(nUniqueId)
{
    DBG_WARNING( "bitte den Ctor mit ViewFrame verwenden" );
    aExtraData = GetDialogData_Impl(0, this, nUniqId);
    aTimer.SetTimeout( 100 );
    aTimer.SetTimeoutHdl( LINK( this, SfxModalDialog, TimerHdl_Impl ) );
    aTimer.Start();
}

// -----------------------------------------------------------------------

SfxModalDialog::SfxModalDialog
(
    SfxViewFrame*   pViewFrame,
    Window*                 pParent,
    const ResId&    rResId
)

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur modale Dialoge;
    ResId wird als ID im ini-file verwendet.
    Die dort gespeicherte Position wird gesetzt.
*/

:       ModalDialog(pParent, rResId),
    nUniqId(rResId.GetId())
{
    aExtraData = GetDialogData_Impl(pViewFrame, this, nUniqId);
    aTimer.SetTimeout( 100 );
    aTimer.SetTimeoutHdl( LINK( this, SfxModalDialog, TimerHdl_Impl ) );
    aTimer.Start();
}

// -----------------------------------------------------------------------

SfxModalDialog::SfxModalDialog
(
    SfxViewFrame*   pViewFrame,
    Window*                 pParent,
    sal_uInt16                  nUniqueId,
    WinBits                 nWinStyle
)

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur modale Dialoge;
    ID f"ur das ini-file wird explizit "ubergeben.
    Die dort gespeicherte Position wird gesetzt.
*/

:       ModalDialog(pParent, nWinStyle),
    nUniqId(nUniqueId)
{
    aExtraData = GetDialogData_Impl(pViewFrame, this, nUniqId);
    aTimer.SetTimeout( 100 );
    aTimer.SetTimeoutHdl( LINK( this, SfxModalDialog, TimerHdl_Impl ) );
    aTimer.Start();
}

// -----------------------------------------------------------------------

SfxModalDialog::~SfxModalDialog()

/*      [Beschreibung]

    Dtor; schreibt Dialogposition in das ini-file
*/

{
    SfxHelpPI *pHelpPI = SFX_APP()->GetHelpPI();
    if ( pHelpPI )
        pHelpPI->ResetTopic();

    SetDialogData_Impl(0, this, nUniqId, aExtraData);
    aTimer.Stop();
}

// -----------------------------------------------------------------------

IMPL_LINK( SfxModalDialog, TimerHdl_Impl, Timer*, EMPTYARG )
{
    SfxHelpPI *pHelpPI = SFX_APP()->GetHelpPI();
    if ( pHelpPI )
        pHelpPI->LoadTopic( GetHelpId() );
    return 0L;
}

//-------------------------------------------------------------------------

void SfxModelessDialog::Initialize(SfxChildWinInfo *pInfo)

/*  [Beschreibung]

    Initialisierung der Klasse SfxModelessDialog "uber ein SfxChildWinInfo.
    Die Initialisierung erfolgt erst in einem 2.Schritt
    nach dem ctor und sollte vom ctor der abgeleiteten Klasse
    oder von dem des SfxChildWindows aufgerufen werden.
*/

{
    Point aPos;
    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 )
    {
        aPos = pInfo->aPos;
        if ( GetStyle() & WB_SIZEABLE )
            SetSizePixel( pInfo->aSize );

        // Initiale Gr"o\se aus pInfo merken
        aSize = GetSizePixel();

        // Soll das FloatingWindow eingezoomt werden ?
        if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
            RollUp();
    }
    else
    {
        // Initiale Gr"o\se aus Resource oder ctor merken
        aSize = GetSizePixel();

        Size aParentSize = GetParent()->GetOutputSizePixel();
        Size aSize = GetSizePixel();
        aPos.X() += ( aParentSize.Width() - aSize.Width() ) / 2;
        aPos.Y() += ( aParentSize.Height() - aSize.Height() ) / 2;
    }

    Point aPoint;
    Rectangle aRect = GetDesktopRectPixel();
    Size aSize( GetSizePixel() );
    aPoint.X() = aRect.Right() - aSize.Width();
    aPoint.Y() = aRect.Bottom() - aSize.Height();

    aPoint = OutputToScreenPixel( aPoint );

    if ( aPos.X() > aPoint.X() )
        aPos.X() = aPoint.X() ;
    if ( aPos.Y() > aPoint.Y() )
        aPos.Y() = aPoint.Y();

    if ( aPos.X() < 0 ) aPos.X() = 0;
    if ( aPos.Y() < 0 ) aPos.Y() = 0;

    SetPosPixel( aPos );
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
    if ( !IsRollUp() )
        aSize = GetSizePixel();
}

// -----------------------------------------------------------------------

SfxModelessDialog::SfxModelessDialog( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        Window* pParent, WinBits nWinBits ) :
    ModelessDialog (pParent, nWinBits),
    pBindings(pBindinx),
    pMgr(pCW)

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur nicht-modale Dialoge.
*/

{
    sal_uInt32 nId = GetHelpId();
    SetHelpId(0);
    SetUniqueId( nId );
}

// -----------------------------------------------------------------------

SfxModelessDialog::SfxModelessDialog( SfxBindings *pBindinx,
                        SfxChildWindow *pCW, Window *pParent,
                        const ResId& rResId ) :
    ModelessDialog(pParent, rResId),
    pBindings(pBindinx),
    pMgr(pCW)

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur nicht-modale Dialoge.
*/

{
    sal_uInt32 nId = GetHelpId();
    SetHelpId(0);
    SetUniqueId( nId );
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
/*
        SfxViewFrame *pFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        if ( pFrame )
            pFrame->MakeActive_Impl();

        if ( rEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
            ModelessDialog::ParentNotify( rEvt );
        return sal_True;
 */
        pBindings->SetActiveFrame( pMgr->GetFrame() );
        pMgr->Activate_Impl();
    }
    else if ( rEvt.GetType() == EVENT_LOSEFOCUS && !HasChildPathFocus() )
    {
        pBindings->SetActiveFrame( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > () );
        pMgr->Deactivate_Impl();
    }
    else if( rEvt.GetType() == EVENT_KEYINPUT )
    {
        // KeyInput zuerst f"ur Dialogfunktionen zulassen ( TAB etc. )
        if ( !ModelessDialog::Notify( rEvt ) )
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
    SfxBoolItem aValue( pMgr->GetType(), sal_False);
    pBindings->GetDispatcher_Impl()->Execute(
        pMgr->GetType(),
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
    ModelessDialogs haben keine spezifischen Informationen, so daá die
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
/*
    if ( rEvt.GetType() == EVENT_GETFOCUS || rEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
        SfxViewFrame *pFrame = pBindings->GetDispatcher_Impl()->GetFrame();
        if ( pFrame )
            pFrame->MakeActive_Impl();

        if ( rEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
            FloatingWindow::ParentNotify( rEvt );
        return sal_True;
    }
*/
    if ( rEvt.GetType() == EVENT_GETFOCUS )
    {
        pBindings->SetActiveFrame( pMgr->GetFrame() );
        pMgr->Activate_Impl();
    }
    else if ( rEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( !HasChildPathFocus() )
            pMgr->Deactivate_Impl();
    }
    else if( rEvt.GetType() == EVENT_KEYINPUT )
    {
        // KeyInput zuerst f"ur Dialogfunktionen zulassen
        if ( !FloatingWindow::Notify( rEvt ) )
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
    pMgr(pCW)

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur FloatingWindows;
*/

{
    sal_uInt32 nId = GetHelpId();
    SetHelpId(0);
    SetUniqueId( nId );
}

// -----------------------------------------------------------------------

SfxFloatingWindow::SfxFloatingWindow( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        Window* pParent,
                        const ResId& rResId) :
    FloatingWindow(pParent, rResId),
    pBindings(pBindinx),
    pMgr(pCW)

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur FloatingWindows;
*/

{
    sal_uInt32 nId = GetHelpId();
    SetHelpId(0);
    SetUniqueId( nId );
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
    SfxBoolItem aValue( pMgr->GetType(), sal_False);
    pBindings->GetDispatcher_Impl()->Execute(
            pMgr->GetType(),
            SFX_CALLMODE_RECORD|SFX_CALLMODE_SYNCHRON, &aValue, 0L );
    return sal_True;
}

// -----------------------------------------------------------------------

SfxFloatingWindow::~SfxFloatingWindow()

/*      [Beschreibung]

    Dtor
*/

{
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
    if ( !IsRollUp() )
        aSize = GetSizePixel();
}

//-------------------------------------------------------------------------

void SfxFloatingWindow::Initialize(SfxChildWinInfo *pInfo)

/*  [Beschreibung]

    Initialisierung der Klasse SfxFloatingWindow "uber ein SfxChildWinInfo.
    Die Initialisierung erfolgt erst in einem 2.Schritt
    nach dem ctor und sollte vom ctor der abgeleiteten Klasse
    oder von dem des SfxChildWindows aufgerufen werden.
*/

{
    Point aPos;
    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 )
    {
        aPos = pInfo->aPos;
        if ( GetStyle() & WB_SIZEABLE )
        {
            Size aMinSize( GetMinOutputSizePixel() );
            if ( pInfo->aSize.Width() < aMinSize.Width() )
                pInfo->aSize.Width() = aMinSize.Width();
            if ( pInfo->aSize.Height() < aMinSize.Height() )
                pInfo->aSize.Height() = aMinSize.Height();
            SetSizePixel( pInfo->aSize );
        }

        // Initiale Gr"o\se aus pInfo merken
        aSize = GetSizePixel();

        // Soll das FloatingWindow eingezoomt werden ?
        if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
            RollUp();
    }
    else
    {
        // Initiale Gr"o\se aus Resource oder ctor merken
        aSize = GetSizePixel();
        Size aParentSize = GetParent()->GetOutputSizePixel();
        aPos.X() += (aParentSize.Width() - aSize.Width()) / 2;
        aPos.Y() += (aParentSize.Height() - aSize.Height()) / 2;
    }

    Point aPoint;
    Rectangle aRect = GetDesktopRectPixel();
    Size aSize( GetSizePixel() );
    aPoint.X() = aRect.Right() - aSize.Width();
    aPoint.Y() = aRect.Bottom() - aSize.Height();

    aPoint = OutputToScreenPixel( aPoint );

    if ( aPos.X() > aPoint.X() )
        aPos.X() = aPoint.X() ;
    if ( aPos.Y() > aPoint.Y() )
        aPos.Y() = aPoint.Y();

    if ( aPos.X() < 0 ) aPos.X() = 0;
    if ( aPos.Y() < 0 ) aPos.Y() = 0;

    SetPosPixel( aPos );
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
#if SUPD<613//MUSTINI
        SfxIniManager* pIniMgr = SFX_APP()->GetAppIniManager();
#endif
        pPage->FillUserData();
        String sData( pPage->GetUserData() );

#if SUPD<613//MUSTINI
        if ( sData.Len() )
            pIniMgr->Set( sData, SFX_KEY_PAGE, GetUniqId() );
#endif
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
    sal_uInt16 nUniqueId,
    sal_Bool bGroupBox
) :

/*      [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur SingleTab-Dialoge;
    ID f"ur das ini-file wird "ubergeben.
*/

    SfxModalDialog( pParent, nUniqueId, WinBits( WB_STDMODAL | WB_3DLOOK ) ),

    pOKBtn          ( 0 ),
    pCancelBtn      ( 0 ),
    pHelpBtn        ( 0 ),
    pGroupBox       ( 0 ),

    pPage           ( 0 ),
    pOptions        ( &rSet ),
    pOutSet         ( 0 ),
    bGrpBox         ( bGroupBox )

{
    DBG_WARNING( "bitte den Ctor mit ViewFrame verwenden" );
}

// -----------------------------------------------------------------------

SfxSingleTabDialog::SfxSingleTabDialog
(
    SfxViewFrame*           pViewFrame,
    Window*                         pParent,
    const SfxItemSet&       rSet,
    sal_uInt16                          nUniqueId,
    sal_Bool                            bGroupBox
) :

/*  [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur SingleTab-Dialoge;
    ID f"ur das ini-file wird "ubergeben.
*/

    SfxModalDialog( pParent, nUniqueId, WinBits( WB_STDMODAL | WB_3DLOOK ) ),

    pOKBtn          ( 0 ),
    pCancelBtn      ( 0 ),
    pHelpBtn        ( 0 ),
    pGroupBox       ( 0 ),

    pPage           ( 0 ),
    pOptions        ( &rSet ),
    pOutSet         ( 0 ),
    bGrpBox         ( bGroupBox )

{
}

// -----------------------------------------------------------------------

SfxSingleTabDialog::SfxSingleTabDialog
(
    Window* pParent,
    sal_uInt16 nUniqueId,
    const SfxItemSet* pInSet,
    sal_Bool bGroupBox
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
    pGroupBox       ( 0 ),

    pPage           ( 0 ),
    pOptions        ( pInSet ),
    pOutSet         ( 0 ),
    bGrpBox         ( bGroupBox )

{
    DBG_WARNING( "bitte den Ctor mit ViewFrame verwenden" );
}

// -----------------------------------------------------------------------

SfxSingleTabDialog::SfxSingleTabDialog
(
    SfxViewFrame* pViewFrame,
    Window* pParent,
    sal_uInt16 nUniqueId,
    const SfxItemSet* pInSet,
    sal_Bool bGroupBox
)

/*  [Beschreibung]

    Konstruktor der allgemeinen Basisklasse f"ur SingleTab-Dialoge;
    ID f"ur das ini-file wird "ubergeben.
*/

:   SfxModalDialog( pParent, nUniqueId, WinBits( WB_STDMODAL | WB_3DLOOK ) ),

    pOKBtn          ( 0 ),
    pCancelBtn      ( 0 ),
    pHelpBtn        ( 0 ),
    pGroupBox       ( 0 ),

    pPage           ( 0 ),
    pOptions        ( pInSet ),
    pOutSet         ( 0 ),
    bGrpBox         ( bGroupBox )

{
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
    delete pGroupBox;
    delete pPage;
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
    DBG_ASSERT( !bGrpBox, "GroupBox no more supported" );
    bGrpBox = sal_False;

    if ( !pOKBtn )
    {
        pOKBtn = new OKButton( this, WB_DEFBUTTON );
        pOKBtn->SetClickHdl( LINK( this, SfxSingleTabDialog, OKHdl_Impl ) );
    }
    if ( !pCancelBtn )
        pCancelBtn = new CancelButton( this );
    if ( !pHelpBtn )
        pHelpBtn = new HelpButton( this );
    if ( !pGroupBox )
        pGroupBox = new GroupBox( this );

    if ( pPage )
        delete pPage;
    pPage = pTabPage;
    fnGetRanges = pRangesFunc;

    if ( pPage )
    {
        // erstmal die User-Daten besorgen, dann erst Reset()
#if SUPD<613//MUSTINI
        SfxIniManager* pIniMgr = SFX_APP()->GetAppIniManager();
        pPage->SetUserData( pIniMgr->Get( SFX_KEY_PAGE, GetUniqId() ) );
#else
        pPage->SetUserData( String() );
#endif
        pPage->Reset( *pOptions );
        pPage->Show();

        // Gr"ossen und Positionen anpassen
        pPage->SetPosPixel( Point() );
        Size aOutSz( pPage->GetSizePixel() );
        Size aBtnSiz = LogicToPixel( Size( 50, 14 ), MAP_APPFONT );
        Point aPnt( aOutSz.Width(),
                    LogicToPixel( Point( 0, 6 ), MAP_APPFONT ).Y() );
        aOutSz.Width() += aBtnSiz.Width() +
                          LogicToPixel( Size( 6, 0 ), MAP_APPFONT ).Width();
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
int BaseDlgsCmpUS_Impl( const void* p1, const void* p2 )
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


