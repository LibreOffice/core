/*************************************************************************
 *
 *  $RCSfile: dialog.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
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

#define _SV_DIALOG_CXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <button.hxx>
#endif
#ifndef _SV_MNEMONIC_HXX
#include <mnemonic.hxx>
#endif
#ifndef _SV_ACCESS_HXX
#include <access.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <dialog.hxx>
#endif

#ifdef DBG_UTIL
#ifndef _SV_MSGBOX_HXX
#include <msgbox.hxx>
#endif
#endif

#include <unowrap.hxx>

#pragma hdrstop

// =======================================================================

#ifdef DBG_UTIL

static ByteString ImplGetDialogText( Dialog* pDialog )
{
    ByteString aErrorStr( pDialog->GetText(), RTL_TEXTENCODING_UTF8 );
    if ( (pDialog->GetType() == WINDOW_MESSBOX) ||
         (pDialog->GetType() == WINDOW_INFOBOX) ||
         (pDialog->GetType() == WINDOW_WARNINGBOX) ||
         (pDialog->GetType() == WINDOW_ERRORBOX) ||
         (pDialog->GetType() == WINDOW_QUERYBOX) )
    {
        aErrorStr += ", ";
        aErrorStr += ByteString( ((MessBox*)pDialog)->GetMessText(), RTL_TEXTENCODING_UTF8 );
    }
    return aErrorStr;
}

#endif

// =======================================================================

static BOOL ImplIsMnemonicCtrl( Window* pWindow )
{
    if ( (pWindow->GetType() == WINDOW_RADIOBUTTON) ||
         (pWindow->GetType() == WINDOW_CHECKBOX) ||
         (pWindow->GetType() == WINDOW_TRISTATEBOX) ||
         (pWindow->GetType() == WINDOW_PUSHBUTTON) )
        return TRUE;

    if ( pWindow->GetType() == WINDOW_FIXEDTEXT )
    {
        if ( pWindow->GetStyle() & (WB_INFO | WB_NOLABEL) )
            return FALSE;
        Window* pNextWindow = pWindow->GetWindow( WINDOW_NEXT );
        if ( !pNextWindow )
            return FALSE;
        pNextWindow = pNextWindow->GetWindow( WINDOW_CLIENT );
        if ( !(pNextWindow->GetStyle() & WB_TABSTOP) ||
             (pNextWindow->GetType() == WINDOW_FIXEDTEXT) ||
             (pNextWindow->GetType() == WINDOW_GROUPBOX) ||
             (pNextWindow->GetType() == WINDOW_RADIOBUTTON) ||
             (pNextWindow->GetType() == WINDOW_CHECKBOX) ||
             (pNextWindow->GetType() == WINDOW_TRISTATEBOX) ||
             (pNextWindow->GetType() == WINDOW_PUSHBUTTON) )
            return FALSE;

        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void ImplWindowAutoMnemonic( Window* pWindow )
{
    ImplMnemonicGenerator   aMnemonicGenerator;
    Window*                 pGetChild;
    Window*                 pChild;

    // Die schon vergebenen Mnemonics registieren
    pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while ( pGetChild )
    {
        pChild = pGetChild->ImplGetWindow();
        aMnemonicGenerator.RegisterMnemonic( pChild->GetText() );
        pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
    }

    // Bei TabPages auch noch die Controls vom Dialog beruecksichtigen
    if ( pWindow->GetType() == WINDOW_TABPAGE )
    {
        Window* pParent = pWindow->GetParent();
        if ( pParent->GetType() == WINDOW_TABCONTROL )
            pParent = pParent->GetParent();

        if ( (pParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL )
        {
            pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
            while ( pGetChild )
            {
                pChild = pGetChild->ImplGetWindow();
                aMnemonicGenerator.RegisterMnemonic( pChild->GetText() );
                pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
            }
        }
    }

    // Die Mnemonics an die Controls vergeben, die noch keinen haben
    pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while ( pGetChild )
    {
        pChild = pGetChild->ImplGetWindow();
        if ( ImplIsMnemonicCtrl( pChild ) )
        {
            XubString aText = pChild->GetText();
            if ( aMnemonicGenerator.CreateMnemonic( aText ) )
                pChild->SetText( aText );
        }

        pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
    }
}

// =======================================================================

static PushButton* ImplGetDefaultButton( Dialog* pDialog )
{
    Window* pChild = pDialog->GetWindow( WINDOW_FIRSTCHILD );
    while ( pChild )
    {
        if ( pChild->ImplIsPushButton() )
        {
            PushButton* pPushButton = (PushButton*)pChild;
            if ( pPushButton->ImplIsDefButton() )
                return pPushButton;
        }

        pChild = pChild->GetWindow( WINDOW_NEXT );
    }

    return NULL;
}

// -----------------------------------------------------------------------

static PushButton* ImplGetOKButton( Dialog* pDialog )
{
    Window* pChild = pDialog->GetWindow( WINDOW_FIRSTCHILD );
    while ( pChild )
    {
        if ( pChild->GetType() == WINDOW_OKBUTTON )
            return (PushButton*)pChild;

        pChild = pChild->GetWindow( WINDOW_NEXT );
    }

    return NULL;
}

// -----------------------------------------------------------------------

static PushButton* ImplGetCancelButton( Dialog* pDialog )
{
    Window* pChild = pDialog->GetWindow( WINDOW_FIRSTCHILD );
    while ( pChild )
    {
        if ( pChild->GetType() == WINDOW_CANCELBUTTON )
            return (PushButton*)pChild;

        pChild = pChild->GetWindow( WINDOW_NEXT );
    }

    return NULL;
}

// -----------------------------------------------------------------------

static void ImplMouseAutoPos( Dialog* pDialog )
{
    ULONG nMouseOptions = pDialog->GetSettings().GetMouseSettings().GetOptions();
    if ( nMouseOptions & MOUSE_OPTION_AUTOCENTERPOS )
    {
        Size aSize = pDialog->GetOutputSizePixel();
        pDialog->SetPointerPosPixel( Point( aSize.Width()/2, aSize.Height()/2 ) );
    }
    else if ( nMouseOptions & MOUSE_OPTION_AUTODEFBTNPOS )
    {
        Window* pWindow = ImplGetDefaultButton( pDialog );
        if ( !pWindow )
            pWindow = ImplGetOKButton( pDialog );
        if ( !pWindow )
            pWindow = ImplGetCancelButton( pDialog );
        if ( !pWindow )
            pWindow = pDialog;
        Size aSize = pWindow->GetOutputSizePixel();
        pWindow->SetPointerPosPixel( Point( aSize.Width()/2, aSize.Height()/2 ) );
    }
}

// =======================================================================

void Dialog::ImplInitData()
{
    mbDialog            = TRUE;

    mpDialogParent      = NULL;
    mpResult            = NULL;
    mpPrevExecuteDlg    = NULL;
    mbInExecute         = FALSE;
    mbOldSaveBack       = FALSE;
    mbInClose           = FALSE;
    mbModalMode         = FALSE;
}

// -----------------------------------------------------------------------

void Dialog::ImplInit( Window* pParent, WinBits nStyle )
{
    USHORT nSysWinMode = Application::GetSystemWindowMode();

    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;
    nStyle |= WB_ROLLABLE;

    if ( !pParent )
    {
        pParent = Application::GetDefDialogParent();
        if ( !pParent && !(nStyle & WB_SYSTEMWINDOW) )
            pParent = Application::GetAppWindow();

        // If Parent is disabled, then we search for a modal dialog
        // in this frame
        if ( pParent && !pParent->IsInputEnabled() )
        {
            ImplSVData* pSVData = ImplGetSVData();
            Dialog*     pExeDlg = pSVData->maWinData.mpLastExecuteDlg;
            while ( pExeDlg )
            {
                // Nur wenn er sichtbar und enabled ist
                if ( pParent->ImplGetFirstOverlapWindow()->IsWindowOrChild( pExeDlg, TRUE ) &&
                     pExeDlg->IsReallyVisible() &&
                     pExeDlg->IsEnabled() && pExeDlg->IsInputEnabled() )
                {
                    pParent = pExeDlg;
                    break;
                }

                pExeDlg = pExeDlg->mpPrevExecuteDlg;
            }
        }
    }

    if ( pParent && !(nSysWinMode & SYSTEMWINDOW_MODE_NOAUTOMODE) )
    {
        if ( !pParent->mpFrameWindow->IsVisible() )
            pParent = NULL;
        else
        {
            if ( pParent->mpFrameWindow->IsDialog() )
            {
                Size aOutSize = pParent->mpFrameWindow->GetOutputSizePixel();
                if ( (aOutSize.Width() < 210) ||(aOutSize.Height() < 160) )
                    nStyle |= WB_SYSTEMWINDOW;
            }
        }
    }

    if ( !pParent || (nStyle & WB_SYSTEMWINDOW) ||
         (pParent->mpFrameData->mbNeedSysWindow && !(nSysWinMode & SYSTEMWINDOW_MODE_NOAUTOMODE)) ||
         (nSysWinMode & SYSTEMWINDOW_MODE_DIALOG) )
    {
        // Fenster mit einem schmallen Border anlegen?
        if ( (nStyle & (WB_BORDER | WB_NOBORDER | WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE)) == WB_BORDER )
        {
            ImplBorderWindow* pBorderWin  = new ImplBorderWindow( pParent, nStyle, BORDERWINDOW_STYLE_FRAME );
            SystemWindow::ImplInit( pBorderWin, nStyle & ~WB_BORDER, NULL );
            pBorderWin->mpClientWindow = this;
            pBorderWin->GetBorder( mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );
            mpBorderWindow  = pBorderWin;
            mpRealParent    = pParent;
        }
        else
        {
            mbFrame         = TRUE;
            mbOverlapWin    = TRUE;
            SystemWindow::ImplInit( pParent, nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_CLOSEABLE | WB_STANDALONE), NULL );
            // Jetzt alle StyleBits setzen
            mnStyle = nStyle;
        }
    }
    else
    {
        ImplBorderWindow* pBorderWin  = new ImplBorderWindow( pParent, nStyle, BORDERWINDOW_STYLE_OVERLAP | BORDERWINDOW_STYLE_BORDER );
        SystemWindow::ImplInit( pBorderWin, nStyle & ~WB_BORDER, NULL );
        pBorderWin->mpClientWindow = this;
        pBorderWin->GetBorder( mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );
        mpBorderWindow  = pBorderWin;
        mpRealParent    = pParent;
    }

    SetActivateMode( ACTIVATE_MODE_GRABFOCUS );

    ImplInitSettings();
}

// -----------------------------------------------------------------------

void Dialog::ImplInitSettings()
{
    // Wir haben graue Dialoge
    if ( IsControlBackground() )
        SetBackground( GetControlBackground() );
    else
        SetBackground( GetSettings().GetStyleSettings().GetDialogColor() );
}

// -----------------------------------------------------------------------

void Dialog::ImplCenterDialog()
{
    Rectangle   aDeskRect = ImplGetFrameWindow()->GetDesktopRectPixel();
    Point       aDeskPos = aDeskRect.TopLeft();
    Size        aDeskSize = aDeskRect.GetSize();
    Size        aWinSize = GetSizePixel();
    Point       aWinPos( ((aDeskSize.Width() - aWinSize.Width()) / 2) + aDeskPos.X(),
                         ((aDeskSize.Height() - aWinSize.Height()) / 2) + aDeskPos.Y() );

    // Pruefen, ob Dialogbox ausserhalb des Desks liegt
    if ( (aWinPos.X() + aWinSize.Width()) > (aDeskPos.X()+aDeskSize.Width()) )
        aWinPos.X() = aDeskPos.X()+aDeskSize.Width() - aWinSize.Width();
    if ( (aWinPos.Y()+aWinSize.Height()) > (aDeskPos.Y()+aDeskSize.Height()) )
        aWinPos.Y() = aDeskPos.Y()+aDeskSize.Height() - aWinSize.Height();
    // Linke Ecke bevorzugen, da Titelbar oben ist
    if ( aWinPos.X() < aDeskPos.X() )
        aWinPos.X() = aDeskPos.X();
    if ( aWinPos.Y() < aDeskPos.Y() )
        aWinPos.Y() = aDeskPos.Y();

    SetPosPixel( aWinPos );
}

// -----------------------------------------------------------------------

Dialog::Dialog( WindowType nType ) :
    SystemWindow( nType )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

Dialog::Dialog( Window* pParent, WinBits nStyle ) :
    SystemWindow( WINDOW_DIALOG )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

Dialog::Dialog( Window* pParent, const ResId& rResId ) :
    SystemWindow( WINDOW_DIALOG )
{
    ImplInitData();
    rResId.SetRT( RSC_DIALOG );
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
}

// -----------------------------------------------------------------------

long Dialog::Notify( NotifyEvent& rNEvt )
{
    // Zuerst Basisklasse rufen wegen TabSteuerung
    long nRet = SystemWindow::Notify( rNEvt );
    if ( !nRet )
    {
        if ( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            KeyCode         aKeyCode = pKEvt->GetKeyCode();
            USHORT          nKeyCode = aKeyCode.GetCode();

            if ( (nKeyCode == KEY_ESCAPE) &&
                 ((GetStyle() & WB_CLOSEABLE) || ImplGetCancelButton( this ) || ImplGetOKButton( this )) )
            {
                Close();
                return TRUE;
            }
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------

void Dialog::StateChanged( StateChangedType nType )
{
    SystemWindow::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( Application::IsAutoMnemonicEnabled() )
            ImplWindowAutoMnemonic( this );

        if ( IsDefaultPos() && !mbFrame )
            ImplCenterDialog();
        if ( !HasChildPathFocus() || HasFocus() )
            GrabFocusToFirstControl();
        if ( !(GetStyle() & WB_CLOSEABLE) )
        {
            if ( ImplGetCancelButton( this ) || ImplGetOKButton( this ) )
            {
                if ( ImplGetBorderWindow() )
                    ((ImplBorderWindow*)ImplGetBorderWindow())->SetCloser();
                else
                {
                    // ... missing implementation, if we are a frame window,
                    // because it is currently impossible to implement.
                    SetStyle( GetStyle() | WB_CLOSEABLE );
                }
            }
        }

        ImplMouseAutoPos( this );
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void Dialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    SystemWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

BOOL Dialog::Close()
{
    if ( mxWindowPeer.is() )
    {
        Application::GetUnoWrapper()->WindowEvent_Close( this );
        if ( IsCreatedWithToolkit() )
            return FALSE;
    }

    mbInClose = TRUE;

    if ( !(GetStyle() & WB_CLOSEABLE) )
    {
        ImplDelData aDelData;
        ImplAddDel( &aDelData );
        PushButton* pButton = ImplGetCancelButton( this );
        if ( pButton )
            pButton->Click();
        else
        {
            pButton = ImplGetOKButton( this );
            if ( pButton )
                pButton->Click();
        }
        if ( aDelData.IsDelete() )
            return TRUE;
        ImplRemoveDel( &aDelData );
    }

    if ( IsInExecute() )
    {
        EndDialog( FALSE );
        mbInClose = FALSE;
        return TRUE;
    }
    else
    {
        mbInClose = FALSE;
        return SystemWindow::Close();
    }
}

// -----------------------------------------------------------------------

short Dialog::Execute()
{
    if ( mbInExecute )
    {
#ifdef DBG_UTIL
        ByteString aErrorStr( "Dialog::Execute() is called in Dialoh::Execute(): " );
        aErrorStr += ImplGetDialogText( this );
        DBG_ERROR( aErrorStr.GetBuffer() );
#endif
        return 0;
    }

    if ( Application::IsDialogCancelEnabled() )
    {
#ifdef DBG_UTIL
        ByteString aErrorStr( "Dialog::Execute() is called in a none UI application: " );
        aErrorStr += ImplGetDialogText( this );
        DBG_ERROR( aErrorStr.GetBuffer() );
#endif
        return 0;
    }

#ifdef DBG_UTIL
    Window* pParent = GetParent();
    if ( pParent )
    {
        pParent = pParent->ImplGetFirstOverlapWindow();
        DBG_ASSERT( pParent->IsReallyVisible(),
                    "Dialog::Execute() - Parent not visible" );
    }
#endif

    ImplSVData* pSVData = ImplGetSVData();

    // Dialoge, die sich in Execute befinden, miteinander verketten
    mpPrevExecuteDlg = pSVData->maWinData.mpLastExecuteDlg;
    pSVData->maWinData.mpLastExecuteDlg = this;

    // Capture beenden, damit der Dialog bedient werden kann
    if ( pSVData->maWinData.mpTrackWin )
        pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    if ( pSVData->maWinData.mpCaptureWin )
        pSVData->maWinData.mpCaptureWin->ReleaseMouse();
    EnableInput( TRUE, TRUE );

    if ( GetParent() )
    {
        NotifyEvent aNEvt( EVENT_EXECUTEDIALOG, this );
        GetParent()->Notify( aNEvt );
    }
    long nRet;
    mpResult = &nRet;
    mbInExecute = TRUE;
    SetModalInputMode( TRUE );
    mbOldSaveBack = IsSaveBackgroundEnabled();
    EnableSaveBackground();
    Show();

    if ( Application::GetAccessHdlCount() )
    {
        Application::AccessNotify( AccessNotification( ACCESS_EVENT_MODALDIALOG_START, this ) );
        Application::AccessNotify( AccessNotification( ACCESS_EVENT_DLGCONTROLS, this ) );
    }

    // Solange Yielden, bis EndDialog aufgerufen wird, oder der Dialog
    // zerstoert wird (sollte nicht sein und ist nur vorsichtsmassnahme)
    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    pSVData->maAppData.mnModalMode++;
    while ( !aDelData.IsDelete() && mbInExecute )
        Application::Yield();
    pSVData->maAppData.mnModalMode--;
    if ( !aDelData.IsDelete() )
        ImplRemoveDel( &aDelData );
#ifdef DBG_UTIL
    else
    {
        DBG_ERROR( "Dialog::Execute() - Dialog destroyed in Execute()" );
    }
#endif

    return (short)nRet;
}

// -----------------------------------------------------------------------

void Dialog::EndDialog( long nResult )
{
    if ( mbInExecute )
    {
        SetModalInputMode( FALSE );

        // Dialog aus der Kette der Dialoge die in Execute stehen entfernen
        ImplSVData* pSVData = ImplGetSVData();
        Dialog* pPrevDlg = NULL;
        Dialog* pExeDlg = pSVData->maWinData.mpLastExecuteDlg;
        while ( pExeDlg )
        {
            if ( pExeDlg == this )
            {
                if ( pPrevDlg )
                    pPrevDlg->mpPrevExecuteDlg = mpPrevExecuteDlg;
                else
                    pSVData->maWinData.mpLastExecuteDlg = mpPrevExecuteDlg;
                break;
            }
            pExeDlg = pExeDlg->mpPrevExecuteDlg;
        }
        mpPrevExecuteDlg = NULL;

        Hide();
        EnableSaveBackground( mbOldSaveBack );
        if ( GetParent() )
        {
            NotifyEvent aNEvt( EVENT_ENDEXECUTEDIALOG, this );
            GetParent()->Notify( aNEvt );
        }
        if ( Application::GetAccessHdlCount() )
            Application::AccessNotify( AccessNotification( ACCESS_EVENT_MODALDIALOG_END, this ) );
        if ( mpResult )
            *mpResult = nResult;
        mpResult    = NULL;
        mbInExecute = FALSE;
    }
}

// -----------------------------------------------------------------------

void Dialog::SetModalInputMode( BOOL bModal )
{
    if ( bModal == mbModalMode )
        return;

    ImplSVData* pSVData = ImplGetSVData();
    mbModalMode = bModal;
    if ( bModal )
    {
        pSVData->maAppData.mnModalDialog++;

        // Diable the prev Modal Dialog, because our dialog must close at first,
        // before the other dialog can be closed (because the other dialog
        // is on stack since our dialog returns)
        if ( mpPrevExecuteDlg && !mpPrevExecuteDlg->IsWindowOrChild( this, TRUE ) )
            mpPrevExecuteDlg->EnableInput( FALSE, TRUE, TRUE, this );

        // determine next overlap dialog parent
        Window* pParent = GetParent();
        if ( pParent )
        {
            mpDialogParent = pParent->ImplGetFirstOverlapWindow();
            if ( mpDialogParent )
                mpDialogParent->EnableInput( FALSE, TRUE, TRUE, this );
        }
    }
    else
    {
        pSVData->maAppData.mnModalDialog--;

        if ( mpDialogParent )
            mpDialogParent->EnableInput( TRUE, TRUE, TRUE, this );

        // Enable the prev Modal Dialog
        if ( mpPrevExecuteDlg && !mpPrevExecuteDlg->IsWindowOrChild( this, TRUE ) )
            mpPrevExecuteDlg->EnableInput( TRUE, TRUE, TRUE, this );
    }
}

// -----------------------------------------------------------------------

void Dialog::SetModalInputMode( BOOL bModal, BOOL bSubModalDialogs )
{
    if ( bSubModalDialogs )
    {
        Window* pOverlap = ImplGetFirstOverlapWindow();
        pOverlap = pOverlap->mpFirstOverlap;
        while ( pOverlap )
        {
            if ( pOverlap->IsDialog() )
                ((Dialog*)pOverlap)->SetModalInputMode( bModal, TRUE );
            pOverlap = pOverlap->mpNext;
        }
    }

    SetModalInputMode( bModal );
}

// -----------------------------------------------------------------------

void Dialog::GrabFocusToFirstControl()
{
    Window* pFocusControl;

    // Wenn Dialog den Focus hat, versuchen wr trotzdem
    // ein Focus-Control zu finden
    if ( HasFocus() )
        pFocusControl = NULL;
    else
    {
        // Wenn schon ein Child-Fenster mal den Focus hatte,
        // dann dieses bevorzugen
        pFocusControl = ImplGetFirstOverlapWindow()->mpLastFocusWindow;
        // Control aus der Dialog-Steuerung suchen
        if ( pFocusControl )
            pFocusControl = ImplFindDlgCtrlWindow( pFocusControl );
    }
    // Kein Control hatte vorher den Focus, oder das Control
    // befindet sich nicht in der Tab-Steuerung, dann das erste
    // Control in der TabSteuerung den Focus geben
    if ( !pFocusControl ||
         !(pFocusControl->GetStyle() & WB_TABSTOP) ||
         !pFocusControl->IsVisible() ||
         !pFocusControl->IsEnabled() || !pFocusControl->IsInputEnabled() )
    {
        USHORT n = 0;
        pFocusControl = ImplGetDlgWindow( n, DLGWINDOW_FIRST );
    }
    if ( pFocusControl )
        pFocusControl->ImplControlFocus( GETFOCUS_INIT );
}

// =======================================================================

ModelessDialog::ModelessDialog( Window* pParent, WinBits nStyle ) :
    Dialog( WINDOW_MODELESSDIALOG )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ModelessDialog::ModelessDialog( Window* pParent, const ResId& rResId ) :
    Dialog( WINDOW_MODELESSDIALOG )
{
    rResId.SetRT( RSC_MODELESSDIALOG );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// =======================================================================

ModalDialog::ModalDialog( Window* pParent, WinBits nStyle ) :
    Dialog( WINDOW_MODALDIALOG )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ModalDialog::ModalDialog( Window* pParent, const ResId& rResId ) :
    Dialog( WINDOW_MODALDIALOG )
{
    rResId.SetRT( RSC_MODALDIALOG );
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
}
