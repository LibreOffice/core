/*************************************************************************
 *
 *  $RCSfile: floatwin.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:58:22 $
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
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <toolbox.hxx>
#endif
#ifndef _SV_FLOATWIN_HXX
#include <floatwin.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#pragma hdrstop

// =======================================================================

void FloatingWindow::ImplInit( Window* pParent, WinBits nStyle )
{
    mbFloatWin = TRUE;
    mbInCleanUp = FALSE;
    mbGrabFocus = FALSE;


    if ( !pParent )
        pParent = Application::GetAppWindow();

    DBG_ASSERT( pParent, "FloatWindow::FloatingWindow(): - pParent == NULL and no AppWindow exists" );

    // no Border, then we dont need a border window
    if ( !nStyle )
    {
        mbOverlapWin = TRUE;
        nStyle |= WB_DIALOGCONTROL;
        SystemWindow::ImplInit( pParent, nStyle, NULL );
    }
    else
    {
        if ( !(nStyle & WB_NODIALOGCONTROL) )
            nStyle |= WB_DIALOGCONTROL;

        if( nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_CLOSEABLE | WB_STANDALONE) )
        {
            WinBits nFloatWinStyle = nStyle;
            // #99154# floaters are not closeable by default anymore, eg fullscreen floater
            // nFloatWinStyle |= WB_CLOSEABLE;
            mbFrame = TRUE;
            mbOverlapWin = TRUE;
            SystemWindow::ImplInit( pParent, nFloatWinStyle & ~WB_BORDER, NULL );
        }
        else
        {
            ImplBorderWindow*   pBorderWin;
            USHORT              nBorderStyle = BORDERWINDOW_STYLE_OVERLAP | BORDERWINDOW_STYLE_BORDER | BORDERWINDOW_STYLE_FLOAT;
            if ( (nStyle & WB_SYSTEMWINDOW) && !(nStyle & (WB_MOVEABLE | WB_SIZEABLE)) )
            {
                nBorderStyle |= BORDERWINDOW_STYLE_FRAME;
                nStyle |= WB_CLOSEABLE; // make undecorated floaters closeable
            }
            pBorderWin  = new ImplBorderWindow( pParent, nStyle, nBorderStyle );
            SystemWindow::ImplInit( pBorderWin, nStyle & ~WB_BORDER, NULL );
            pBorderWin->mpClientWindow = this;
            pBorderWin->GetBorder( mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );
            pBorderWin->SetDisplayActive( TRUE );
            mpBorderWindow  = pBorderWin;
            mpRealParent    = pParent;
        }
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
    if( mbPopupModeCanceled )
        // indicates that ESC key was pressed
        // will be handled in Window::ImplGrabFocus()
        SetDialogControlFlags( GetDialogControlFlags() | WINDOW_DLGCTRL_FLOATWIN_POPUPMODEEND_CANCEL );

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

    // convert....
    Window* pW = pWindow;
    if ( pW->mpRealParent )
        pW = pW->mpRealParent;

    Rectangle normRect( rRect );  // rRect is already relative to top-level window
    normRect.SetPos( pW->ScreenToOutputPixel( normRect.TopLeft() ) );

    BOOL bRTL = Application::GetSettings().GetLayoutRTL();

    Rectangle devRect(  pW->OutputToAbsoluteScreenPixel( normRect.TopLeft() ),
                        pW->OutputToAbsoluteScreenPixel( normRect.BottomRight() ) );

    Rectangle devRectRTL( devRect );
    if( bRTL )
        // create a rect that can be compared to desktop coordinates
        devRectRTL = pW->ImplOutputToUnmirroredAbsoluteScreenPixel( normRect );


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
                aPos.X() = devRect.Left()-aSize.Width();
                aPos.Y() = devRect.Top();
                aPos.Y() -= pWindow->mnTopBorder;
                if( bRTL ) // --- RTL --- we're comparing screen coordinates here
                {
                    if( (devRectRTL.Right()+aSize.Width()) > aScreenRect.Right() )
                        bBreak = FALSE;
                }
                else
                {
                    if ( aPos.X() < aScreenRect.Left() )
                        bBreak = FALSE;
                }
                break;
            case FLOATWIN_POPUPMODE_RIGHT:
                aPos     = devRect.TopRight();
                aPos.Y() -= pWindow->mnTopBorder;
                if( bRTL ) // --- RTL --- we're comparing screen coordinates here
                {
                    if( (devRectRTL.Left() - aSize.Width()) < aScreenRect.Left() )
                        bBreak = FALSE;
                }
                else
                {
                    if ( aPos.X()+aSize.Width() > aScreenRect.Right() )
                        bBreak = FALSE;
                }
                break;
            case FLOATWIN_POPUPMODE_UP:
                aPos.X() = devRect.Left();
                aPos.Y() = devRect.Top()-aSize.Height();
                if ( aPos.Y() < aScreenRect.Top() )
                    bBreak = FALSE;
                break;
            case FLOATWIN_POPUPMODE_DOWN:
                aPos = devRect.BottomLeft();
                if ( aPos.Y()+aSize.Height() > aScreenRect.Bottom() )
                    bBreak = FALSE;
                break;
        }

        /*
         *  #95901# avoid mouse pointer for popup menus because
         *  of sawfish window manager. This cannot be done in
         *  the Unix dependant part since that cannot decide between
         *  popup menus and other menus/floatwins.
         */
        if( ( (nArrangeAry[nArrangeIndex] == FLOATWIN_POPUPMODE_DOWN)  ||
              (nArrangeAry[nArrangeIndex] == FLOATWIN_POPUPMODE_RIGHT) )
            && ( nFlags & FLOATWIN_POPUPMODE_ALLMOUSEBUTTONCLOSE ) )
            aPos.X() += 2;

        // Evt. noch anpassen
        if ( bBreak && !(nFlags & FLOATWIN_POPUPMODE_NOAUTOARRANGE) )
        {
            if ( (nArrangeAry[nArrangeIndex] == FLOATWIN_POPUPMODE_LEFT)  ||
                 (nArrangeAry[nArrangeIndex] == FLOATWIN_POPUPMODE_RIGHT) )
            {
                if ( aPos.Y()+aSize.Height() > aScreenRect.Bottom() )
                {
                    bTop = TRUE;
                    aPos.Y() = devRect.Bottom()-aSize.Height();
                    if ( aPos.Y() < aScreenRect.Top() )
                        aPos.Y() = aScreenRect.Top();
                }
            }
            else
            {
                if ( !bRTL && aPos.X()+aSize.Width() > aScreenRect.Right() )
                {
                    bLeft = TRUE;
                    aPos.X() = devRect.Right()-aSize.Width();
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
    /*
    // should not be required anymore: moving windows
    // into the screen is done in the sal layer anyway
    if ( aPos.X()+aSize.Width() > aScreenRect.Right() )
        aPos.X() = aScreenRect.Right()-aSize.Width();
    if ( aPos.X() < aScreenRect.Left() )
        aPos.X() = aScreenRect.Left();
    if ( aPos.Y()+aSize.Height() > aScreenRect.Bottom() )
        aPos.Y() = aScreenRect.Bottom()-aSize.Height();
    if ( aPos.Y() < aScreenRect.Top() )
        aPos.Y() = aScreenRect.Top();
*/
    rArrangeIndex = nArrangeIndex;

    aPos = pW->AbsoluteScreenToOutputPixel( aPos );


    // caller expects cordinates relative to top-level win
    return pW->OutputToScreenPixel( aPos );
}

// -----------------------------------------------------------------------

FloatingWindow* FloatingWindow::ImplFloatHitTest( Window* pReference, const Point& rPos, USHORT& rHitTest )
{
    FloatingWindow* pWin = this;

    Point aAbsolute( rPos );

    // compare coordinates in absolute screen coordinates
    if( pReference->ImplHasMirroredGraphics()  )
    {
        if(!pReference->IsRTLEnabled() )
            // --- RTL --- re-mirror back to get device coordiantes
            pReference->ImplReMirror( aAbsolute );

        Rectangle aRect( pReference->ScreenToOutputPixel(aAbsolute), Size(1,1) ) ;
        aRect = pReference->ImplOutputToUnmirroredAbsoluteScreenPixel( aRect );
        aAbsolute = aRect.TopLeft();
    }
    else
        aAbsolute = Point( pReference->OutputToAbsoluteScreenPixel(
            pReference->ScreenToOutputPixel(rPos) ) );

    do
    {
        Rectangle devRect( pWin->ImplOutputToUnmirroredAbsoluteScreenPixel( Rectangle( pWin->ScreenToOutputPixel(pWin->GetPosPixel()), pWin->GetSizePixel()) ) ) ;
        if ( devRect.IsInside( aAbsolute ) )
        {
            rHitTest = IMPL_FLOATWIN_HITTEST_WINDOW;
            return pWin;
        }

        // test, if mouse in rectangle
        /*
         *  maFloatRect is set in startpopup mode and
         *  already is in parent coordinates.
         */
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

    // avoid close on focus change for decorated floating windows only
    if( mbFrame && (GetStyle() & WB_MOVEABLE) )
        nFlags |= FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE;
    else
        nFlags &= ~FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE;

    // #102010# For debugging Accessibility
    // MT->SSA: I wanted to set that flag in menu.cxx, why do you remove it above???
    static const char* pEnv = getenv("SAL_FLOATWIN_NOAPPFOCUSCLOSE" );
    if( pEnv && *pEnv )
        nFlags |= FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE;

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
    // FloatingWindow in Liste der Fenster aufnehmen, die sich im PopupModus
    // befinden
    ImplSVData* pSVData = ImplGetSVData();
    mpNextFloat = pSVData->maWinData.mpFirstFloat;
    pSVData->maWinData.mpFirstFloat = this;
    if( nFlags & FLOATWIN_POPUPMODE_GRABFOCUS )
        mbGrabFocus = TRUE;     // force key input even without focus (useful for menues)
    Show( TRUE, SHOW_NOACTIVATE );
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
    Point aPos;
    aPos = GetParent()->OutputToScreenPixel( GetParent()->AbsoluteScreenToOutputPixel( pBox->OutputToAbsoluteScreenPixel( aRect.TopLeft() ) ) );
    aRect.SetPos( aPos );

    nFlags |=
        FLOATWIN_POPUPMODE_NOFOCUSCLOSE     |
//        FLOATWIN_POPUPMODE_NOMOUSECLOSE       |
        FLOATWIN_POPUPMODE_ALLMOUSEBUTTONCLOSE |
//        FLOATWIN_POPUPMODE_NOMOUSERECTCLOSE   |   // #105968# floating toolboxes should close when clicked in (parent's) float rect
        FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE   |
        FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE;
/*
 *  FLOATWIN_POPUPMODE_NOKEYCLOSE       |
 *  don't set since it diables closing floaters with escape
 */

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

    mbInCleanUp = TRUE; // prevent killing this window due to focus change while working with it

    // Bei allen nachfolgenden PopupMode-Fenster den Modus auch beenden
    while ( pSVData->maWinData.mpFirstFloat && pSVData->maWinData.mpFirstFloat != this )
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

    mbInCleanUp = FALSE;
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

