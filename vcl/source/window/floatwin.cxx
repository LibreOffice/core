/*************************************************************************
 *
 *  $RCSfile: floatwin.cxx,v $
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

#define _SV_FLOATWIN_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <toolbox.hxx>
#endif
#ifndef _SV_FLOATWIN_HXX
#include <floatwin.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif

#pragma hdrstop

// =======================================================================

void FloatingWindow::ImplInit( Window* pParent, WinBits nStyle )
{
    mbFloatWin      = TRUE;

    if ( !pParent )
        pParent = Application::GetAppWindow();

    DBG_ASSERT( pParent, "FloatWindow::FloatingWindow(): - pParent == NULL and no AppWindow exists" );

    // no Border, then we dont need a border window
    if ( !nStyle )
    {
        mbOverlapWin = TRUE;
        if ( !(nStyle & WB_NODIALOGCONTROL) )
            nStyle |= WB_DIALOGCONTROL;
        SystemWindow::ImplInit( pParent, nStyle, NULL );
    }
    else
    {
        ImplBorderWindow* pBorderWin  = new ImplBorderWindow( pParent, nStyle, BORDERWINDOW_STYLE_OVERLAP | BORDERWINDOW_STYLE_BORDER | BORDERWINDOW_STYLE_FLOAT );
        if ( !(nStyle & WB_NODIALOGCONTROL) )
            nStyle |= WB_DIALOGCONTROL;
        SystemWindow::ImplInit( pBorderWin, nStyle & ~WB_BORDER, NULL );
        pBorderWin->mpClientWindow = this;
        pBorderWin->GetBorder( mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );
        pBorderWin->SetDisplayActive( TRUE );
        mpBorderWindow  = pBorderWin;
        mpRealParent    = pParent;
    }
    SetActivateMode( 0 );

    mpNextFloat             = NULL;
    mpFirstPopupModeWin     = NULL;
    mpBox                   = NULL;
    mnPostId                = 0;
    mnTitle                 = (nStyle & WB_MOVEABLE) ? FLOATWIN_TITLE_NORMAL : FLOATWIN_TITLE_NONE;
    mnOldTitle              = mnTitle;
    mnPopupModeFlags        = 0;
    mbInPopupMode           = FALSE;
    mbPopupMode             = FALSE;
    mbPopupModeCanceled     = FALSE;
    mbPopupModeTearOff      = FALSE;
    mbMouseDown             = FALSE;

    ImplInitSettings();
}

// -----------------------------------------------------------------------

void FloatingWindow::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    Color aColor;
    if ( IsControlBackground() )
        aColor = GetControlBackground();
    else if ( Window::GetStyle() & WB_3DLOOK )
        aColor = rStyleSettings.GetFaceColor();
    else
        aColor = rStyleSettings.GetWindowColor();
    SetBackground( aColor );
}

// =======================================================================

FloatingWindow::FloatingWindow( Window* pParent, WinBits nStyle ) :
    SystemWindow( WINDOW_FLOATINGWINDOW )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

FloatingWindow::FloatingWindow( Window* pParent, const ResId& rResId ) :
    SystemWindow( WINDOW_FLOATINGWINDOW )
{
    rResId.SetRT( RSC_FLOATINGWINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void FloatingWindow::ImplLoadRes( const ResId& rResId )
{
    SystemWindow::ImplLoadRes( rResId );

    USHORT nObjMask = ReadShortRes();

    if ( (RSC_FLOATINGWINDOW_WHMAPMODE | RSC_FLOATINGWINDOW_WIDTH |
          RSC_FLOATINGWINDOW_HEIGHT) & nObjMask )
    {
        // Groessenangabe aus der Resource verwenden
        Size    aSize;
        MapUnit eSizeMap = MAP_PIXEL;

        if ( RSC_FLOATINGWINDOW_WHMAPMODE & nObjMask )
            eSizeMap = (MapUnit) ReadShortRes();
        if ( RSC_FLOATINGWINDOW_WIDTH & nObjMask )
            aSize.Width() = ReadShortRes();
        if ( RSC_FLOATINGWINDOW_HEIGHT & nObjMask )
            aSize.Height() = ReadShortRes();

        SetRollUpOutputSizePixel( LogicToPixel( aSize, eSizeMap ) );
    }

    if (nObjMask & RSC_FLOATINGWINDOW_ZOOMIN )
    {
        if ( ReadShortRes() )
            RollUp();
    }
}


// -----------------------------------------------------------------------

FloatingWindow::~FloatingWindow()
{
    if ( IsInPopupMode() )
        EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL | FLOATWIN_POPUPMODEEND_DONTCALLHDL );

    if ( mnPostId )
        Application::RemoveUserEvent( mnPostId );
}

// -----------------------------------------------------------------------

Point FloatingWindow::ImplCalcPos( Window* pWindow,
                                   const Rectangle& rRect, ULONG nFlags,
                                   USHORT& rArrangeIndex )
{
    // Fenster-Position ermitteln
    Point       aPos;
    Size        aSize = pWindow->GetSizePixel();
    Rectangle   aScreenRect = pWindow->ImplGetFrameWindow()->GetDesktopRectPixel();
    USHORT      nArrangeAry[5];
    USHORT      nArrangeIndex;
    BOOL        bLeft;
    BOOL        bTop;
    BOOL        bBreak;
    if ( nFlags & FLOATWIN_POPUPMODE_LEFT )
    {
        nArrangeAry[0]  = FLOATWIN_POPUPMODE_LEFT;
        nArrangeAry[1]  = FLOATWIN_POPUPMODE_RIGHT;
        nArrangeAry[2]  = FLOATWIN_POPUPMODE_UP;
        nArrangeAry[3]  = FLOATWIN_POPUPMODE_DOWN;
        nArrangeAry[4]  = FLOATWIN_POPUPMODE_LEFT;
    }
    else if ( nFlags & FLOATWIN_POPUPMODE_RIGHT )
    {
        nArrangeAry[0]  = FLOATWIN_POPUPMODE_RIGHT;
        nArrangeAry[1]  = FLOATWIN_POPUPMODE_LEFT;
        nArrangeAry[2]  = FLOATWIN_POPUPMODE_UP;
        nArrangeAry[3]  = FLOATWIN_POPUPMODE_DOWN;
        nArrangeAry[4]  = FLOATWIN_POPUPMODE_RIGHT;
    }
    else if ( nFlags & FLOATWIN_POPUPMODE_UP )
    {
        nArrangeAry[0]  = FLOATWIN_POPUPMODE_UP;
        nArrangeAry[1]  = FLOATWIN_POPUPMODE_DOWN;
        nArrangeAry[2]  = FLOATWIN_POPUPMODE_RIGHT;
        nArrangeAry[3]  = FLOATWIN_POPUPMODE_LEFT;
        nArrangeAry[4]  = FLOATWIN_POPUPMODE_UP;
    }
    else
    {
        nArrangeAry[0]  = FLOATWIN_POPUPMODE_DOWN;
        nArrangeAry[1]  = FLOATWIN_POPUPMODE_UP;
        nArrangeAry[2]  = FLOATWIN_POPUPMODE_RIGHT;
        nArrangeAry[3]  = FLOATWIN_POPUPMODE_LEFT;
        nArrangeAry[4]  = FLOATWIN_POPUPMODE_DOWN;
    }
    if ( nFlags & FLOATWIN_POPUPMODE_NOAUTOARRANGE )
        nArrangeIndex = 4;
    else
        nArrangeIndex = 0;
    for ( ; nArrangeIndex < 5; nArrangeIndex++ )
    {
        bLeft = FALSE;
        bTop = FALSE;
        bBreak = TRUE;
        switch ( nArrangeAry[nArrangeIndex] )
        {
            case FLOATWIN_POPUPMODE_LEFT:
                aPos.X() = rRect.Left()-aSize.Width();
                aPos.Y() = rRect.Top();
                aPos.Y() -= pWindow->mnTopBorder;
                if ( aPos.X() < aScreenRect.Left() )
                    bBreak = FALSE;
                break;
            case FLOATWIN_POPUPMODE_RIGHT:
                aPos     = rRect.TopRight();
                aPos.Y() -= pWindow->mnTopBorder;
                if ( aPos.X()+aSize.Width() > aScreenRect.Right() )
                    bBreak = FALSE;
                break;
            case FLOATWIN_POPUPMODE_UP:
                aPos.X() = rRect.Left();
                aPos.Y() = rRect.Top()-aSize.Height();
                if ( aPos.Y() < aScreenRect.Top() )
                    bBreak = FALSE;
                break;
            case FLOATWIN_POPUPMODE_DOWN:
                aPos = rRect.BottomLeft();
                if ( aPos.Y()+aSize.Height() > aScreenRect.Bottom() )
                    bBreak = FALSE;
                break;
        }

        // Evt. noch anpassen
        if ( bBreak && !(nFlags & FLOATWIN_POPUPMODE_NOAUTOARRANGE) )
        {
            if ( (nArrangeAry[nArrangeIndex] == FLOATWIN_POPUPMODE_LEFT)  ||
                 (nArrangeAry[nArrangeIndex] == FLOATWIN_POPUPMODE_RIGHT) )
            {
                if ( aPos.Y()+aSize.Height() > aScreenRect.Bottom() )
                {
                    bTop = TRUE;
                    aPos.Y() = rRect.Bottom()-aSize.Height();
                    if ( aPos.Y() < aScreenRect.Top() )
                        aPos.Y() = aScreenRect.Top();
                }
            }
            else
            {
                 if ( aPos.X()+aSize.Width() > aScreenRect.Right() )
                 {
                    bLeft = TRUE;
                    aPos.X() = rRect.Right()-aSize.Width();
                    if ( aPos.X() < aScreenRect.Left() )
                        aPos.X() = aScreenRect.Left();
                 }
            }
        }

        if ( bBreak )
            break;
    }
    if ( nArrangeIndex > 4 )
        nArrangeIndex = 4;

    // Ansonsten soweit wie moeglich in den Bildschirm einpassen
    if ( aPos.X()+aSize.Width() > aScreenRect.Right() )
        aPos.X() = aScreenRect.Right()-aSize.Width();
    if ( aPos.X() < aScreenRect.Left() )
        aPos.X() = aScreenRect.Left();
    if ( aPos.Y()+aSize.Height() > aScreenRect.Bottom() )
        aPos.Y() = aScreenRect.Bottom()-aSize.Height();
    if ( aPos.Y() < aScreenRect.Top() )
        aPos.Y() = aScreenRect.Top();

    rArrangeIndex = nArrangeIndex;
    return aPos;
}

// -----------------------------------------------------------------------

FloatingWindow* FloatingWindow::ImplFloatHitTest( const Point& rPos, USHORT& rHitTest )
{
    FloatingWindow* pWin = this;

    do
    {
        Rectangle aRect( pWin->GetPosPixel(), pWin->GetSizePixel() );
        if ( aRect.IsInside( rPos ) )
        {
            rHitTest = IMPL_FLOATWIN_HITTEST_WINDOW;
            return pWin;
        }

        // Testen, ob Maus im Rechteck
        if ( pWin->maFloatRect.IsInside( rPos ) )
        {
            rHitTest = IMPL_FLOATWIN_HITTEST_RECT;
            return pWin;
        }

        pWin = pWin->mpNextFloat;
    }
    while ( pWin );

    rHitTest = IMPL_FLOATWIN_HITTEST_OUTSIDE;
    return NULL;
}

// -----------------------------------------------------------------------

FloatingWindow* FloatingWindow::ImplFindLastLevelFloat()
{
    FloatingWindow* pWin = this;
    FloatingWindow* pLastFoundWin = pWin;

    do
    {
        if ( pWin->GetPopupModeFlags() & FLOATWIN_POPUPMODE_NEWLEVEL )
            pLastFoundWin = pWin;

        pWin = pWin->mpNextFloat;
    }
    while ( pWin );

    return pLastFoundWin;
}

// -----------------------------------------------------------------------

BOOL FloatingWindow::ImplIsFloatPopupModeWindow( const Window* pWindow )
{
    FloatingWindow* pWin = this;

    do
    {
        if ( pWin->mpFirstPopupModeWin == pWindow )
            return TRUE;

        pWin = pWin->mpNextFloat;
    }
    while ( pWin );

    return FALSE;
}

// -----------------------------------------------------------------------

IMPL_LINK( FloatingWindow, ImplEndPopupModeHdl, void*, EMPTYARG )
{
    mnPostId            = 0;
    mnPopupModeFlags    = 0;
    mbPopupMode         = FALSE;
    PopupModeEnd();
    return 0;
}

// -----------------------------------------------------------------------

long FloatingWindow::Notify( NotifyEvent& rNEvt )
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

            if ( (nKeyCode == KEY_ESCAPE) && (GetStyle() & WB_CLOSEABLE) )
            {
                Close();
                return TRUE;
            }
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------

void FloatingWindow::StateChanged( StateChangedType nType )
{
    SystemWindow::StateChanged( nType );

    if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FloatingWindow::DataChanged( const DataChangedEvent& rDCEvt )
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

void FloatingWindow::ImplCallPopupModeEnd()
{
    // PopupMode wurde beendet
    mbInPopupMode = FALSE;

    // Handler asyncron rufen
    if ( !mnPostId )
        Application::PostUserEvent( mnPostId, LINK( this, FloatingWindow, ImplEndPopupModeHdl ) );
}

// -----------------------------------------------------------------------

void FloatingWindow::PopupModeEnd()
{
    maPopupModeEndHdl.Call( this );
}

// -----------------------------------------------------------------------

void FloatingWindow::SetTitleType( USHORT nTitle )
{
    if ( (mnTitle != nTitle) && mpBorderWindow )
    {
        mnTitle = nTitle;
        Size aOutSize = GetOutputSizePixel();
        USHORT nTitleStyle;
        if ( nTitle == FLOATWIN_TITLE_NORMAL )
            nTitleStyle = BORDERWINDOW_TITLE_SMALL;
        else if ( nTitle == FLOATWIN_TITLE_TEAROFF )
            nTitleStyle = BORDERWINDOW_TITLE_TEAROFF;
        else // nTitle == FLOATWIN_TITLE_NONE
            nTitleStyle = BORDERWINDOW_TITLE_NONE;
        ((ImplBorderWindow*)mpBorderWindow)->SetTitleType( nTitleStyle, aOutSize );
        ((ImplBorderWindow*)mpBorderWindow)->GetBorder( mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );
    }
}

// -----------------------------------------------------------------------

void FloatingWindow::StartPopupMode( const Rectangle& rRect, ULONG nFlags )
{
    DBG_ASSERT( (GetStyle() & WB_MOVEABLE) || !(nFlags & FLOATWIN_POPUPMODE_ALLOWTEAROFF),
                "TearOff only allowed, when FloatingWindow moveable" );

    // Wenn Fenster sichtbar, dann vorher hiden, da es sonst flackert
    if ( IsVisible() )
        Show( FALSE, SHOW_NOFOCUSCHANGE );

    // Wenn Fenster klein, dann vorher aufklappen
    if ( IsRollUp() )
        RollDown();

    // Title wegnehmen
    mnOldTitle = mnTitle;
    if ( nFlags & FLOATWIN_POPUPMODE_ALLOWTEAROFF )
        SetTitleType( FLOATWIN_TITLE_TEAROFF );
    else
        SetTitleType( FLOATWIN_TITLE_NONE );

    // Fenster-Position ermitteln und setzen
    USHORT nArrangeIndex;
    SetPosPixel( ImplCalcPos( this, rRect, nFlags, nArrangeIndex ) );

    // Daten seten und Fenster anzeigen
    maFloatRect             = rRect;
    maFloatRect.Left()     -= 2;
    maFloatRect.Top()      -= 2;
    maFloatRect.Right()    += 2;
    maFloatRect.Bottom()   += 2;
    mnPopupModeFlags        = nFlags;
    mbInPopupMode           = TRUE;
    mbPopupMode             = TRUE;
    mbPopupModeCanceled     = FALSE;
    mbPopupModeTearOff      = FALSE;
    mbMouseDown             = FALSE;

    mbOldSaveBackMode       = IsSaveBackgroundEnabled();
    EnableSaveBackground();

/*
    // Abfragen, ob Animation eingeschaltet ist
    if ( (Application::GetSettings().GetAnimationOptions() & ANIMATION_OPTION_POPUP) &&
         !(nFlags & FLOATWIN_POPUPMODE_NOANIMATION) )
    {
        USHORT nAniFlags;
        switch ( nArrangeAry[nArrangeIndex] )
        {
            case FLOATWIN_POPUPMODE_LEFT:
                nAniFlags = STARTANIMATION_LEFT;
                break;
            case FLOATWIN_POPUPMODE_RIGHT:
                nAniFlags = STARTANIMATION_RIGHT;
                break;
            case FLOATWIN_POPUPMODE_UP:
                nAniFlags = STARTANIMATION_UP;
                break;
            case FLOATWIN_POPUPMODE_DOWN:
                nAniFlags = STARTANIMATION_DOWN;
                break;
        }
        if ( !(nFlags & FLOATWIN_POPUPMODE_ANIMATIONSLIDE) )
        {
            if ( bLeft )
                nAniFlags |= STARTANIMATION_LEFT;
            else
                nAniFlags |= STARTANIMATION_RIGHT;
            if ( bTop )
                nAniFlags |= STARTANIMATION_UP;
            else
                nAniFlags |= STARTANIMATION_DOWN;
        }

        ImpStartAnimation( this, nAniFlags );
    }
    else
*/
        Show();

    // FloatingWindow in Liste der Fenster aufnehmen, die sich im PopupModus
    // befinden
    ImplSVData* pSVData = ImplGetSVData();
    mpNextFloat = pSVData->maWinData.mpFirstFloat;
    pSVData->maWinData.mpFirstFloat = this;
}

// -----------------------------------------------------------------------

void FloatingWindow::StartPopupMode( ToolBox* pBox, ULONG nFlags )
{
    // Selektieten Button ermitteln
    USHORT nItemId = pBox->GetDownItemId();
    if ( !nItemId )
        return;

    mpBox = pBox;
    pBox->ImplFloatControl( TRUE, this );

    // Daten von der ToolBox holen
    Rectangle aRect = pBox->GetItemRect( nItemId );
    Point aPos = pBox->OutputToScreenPixel( aRect.TopLeft() );
    aRect.SetPos( aPos );

    // Flags fuer Positionierung bestimmen
    if ( !(nFlags & (FLOATWIN_POPUPMODE_DOWN | FLOATWIN_POPUPMODE_UP |
                     FLOATWIN_POPUPMODE_LEFT | FLOATWIN_POPUPMODE_RIGHT |
                     FLOATWIN_POPUPMODE_NOAUTOARRANGE)) )
    {
         WindowAlign eAlign = pBox->GetAlign();
         if ( pBox->IsHorizontal() )
         {
             if ( pBox->IsFloatingMode() || (eAlign == WINDOWALIGN_TOP) )
                 nFlags |= FLOATWIN_POPUPMODE_DOWN;
             else
                 nFlags |= FLOATWIN_POPUPMODE_UP;
         }
         else
         {
             if ( eAlign == WINDOWALIGN_LEFT )
                 nFlags |= FLOATWIN_POPUPMODE_RIGHT;
             else
                 nFlags |= FLOATWIN_POPUPMODE_LEFT;
         }
    }

    // FloatingModus starten
    StartPopupMode( aRect, nFlags );
}

// -----------------------------------------------------------------------

void FloatingWindow::ImplEndPopupMode( USHORT nFlags, ULONG nFocusId )
{
    if ( !mbInPopupMode )
        return;

    ImplSVData* pSVData = ImplGetSVData();

    // Bei allen nachfolgenden PopupMode-Fenster den Modus auch beenden
    while ( pSVData->maWinData.mpFirstFloat != this )
        pSVData->maWinData.mpFirstFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL );

    // Fenster aus der Liste austragen
    pSVData->maWinData.mpFirstFloat = mpNextFloat;
    mpNextFloat = NULL;

    ULONG nPopupModeFlags = mnPopupModeFlags;

    // Wenn nicht abgerissen wurde, dann Fenster wieder Hiden
    if ( !(nFlags & FLOATWIN_POPUPMODEEND_TEAROFF) ||
         !(nPopupModeFlags & FLOATWIN_POPUPMODE_ALLOWTEAROFF) )
    {
        Show( FALSE, SHOW_NOFOCUSCHANGE );

        // Focus evt. auf ein entsprechendes FloatingWindow weiterschalten
        if ( nFocusId )
            Window::EndSaveFocus( nFocusId );
        else if ( pSVData->maWinData.mpFocusWin && pSVData->maWinData.mpFirstFloat &&
                  ImplIsWindowOrChild( pSVData->maWinData.mpFocusWin ) )
            pSVData->maWinData.mpFirstFloat->GrabFocus();
        mbPopupModeTearOff = FALSE;
    }
    else
    {
        mbPopupModeTearOff = TRUE;
        if ( nFocusId )
            Window::EndSaveFocus( nFocusId, FALSE );
    }
    EnableSaveBackground( mbOldSaveBackMode );

    mbPopupModeCanceled = (nFlags & FLOATWIN_POPUPMODEEND_CANCEL) != 0;

    // Gegebenenfalls den Title wieder herstellen
    SetTitleType( mnOldTitle );

    // ToolBox wieder auf normal schalten
    if ( mpBox )
    {
        mpBox->ImplFloatControl( FALSE, this );
        mpBox = NULL;
    }

    // Je nach Parameter den PopupModeEnd-Handler rufen
    if ( !(nFlags & FLOATWIN_POPUPMODEEND_DONTCALLHDL) )
        ImplCallPopupModeEnd();

    // Je nach Parameter die restlichen Fenster auch noch schliessen
    if ( nFlags & FLOATWIN_POPUPMODEEND_CLOSEALL )
    {
        if ( !(nPopupModeFlags & FLOATWIN_POPUPMODE_NEWLEVEL) )
        {
            if ( pSVData->maWinData.mpFirstFloat )
            {
                FloatingWindow* pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
                pLastLevelFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
            }
        }
    }
}

// -----------------------------------------------------------------------

void FloatingWindow::EndPopupMode( USHORT nFlags )
{
    ImplEndPopupMode( nFlags );
}

// -----------------------------------------------------------------------

void FloatingWindow::AddPopupModeWindow( Window* pWindow )
{
    // !!! bisher erst 1 Fenster und noch keine Liste
    mpFirstPopupModeWin = pWindow;
}

// -----------------------------------------------------------------------

void FloatingWindow::RemovePopupModeWindow( Window* pWindow )
{
    // !!! bisher erst 1 Fenster und noch keine Liste
    if ( mpFirstPopupModeWin == pWindow )
        mpFirstPopupModeWin = NULL;
}

