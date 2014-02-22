/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svdata.hxx>
#include <brdwin.hxx>
#include <window.h>
#include <salframe.hxx>

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/event.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/settings.hxx>

#include <tools/rc.h>
#include <tools/debug.hxx>


// =======================================================================

class FloatingWindow::ImplData
{
public:
    ImplData();
    ~ImplData();

    ToolBox*        mpBox;
    Rectangle       maItemEdgeClipRect; // used to clip the common edge between a toolbar item and the border of this window
};

FloatingWindow::ImplData::ImplData()
{
    mpBox = NULL;
}

FloatingWindow::ImplData::~ImplData()
{
}

Rectangle& FloatingWindow::ImplGetItemEdgeClipRect()
{
    return mpImplData->maItemEdgeClipRect;
}

// =======================================================================

void FloatingWindow::ImplInit( Window* pParent, WinBits nStyle )
{
    mpImplData = new ImplData;

    mpWindowImpl->mbFloatWin = true;
    mbInCleanUp = false;
    mbGrabFocus = false;

    DBG_ASSERT( pParent, "FloatWindow::FloatingWindow(): - pParent == NULL!" );

    if ( !pParent )
        pParent = ImplGetSVData()->maWinData.mpAppWin;

    DBG_ASSERT( pParent, "FloatWindow::FloatingWindow(): - pParent == NULL and no AppWindow exists" );

    // no Border, then we dont need a border window
    if ( !nStyle )
    {
        mpWindowImpl->mbOverlapWin = true;
        nStyle |= WB_DIALOGCONTROL;
        SystemWindow::ImplInit( pParent, nStyle, NULL );
    }
    else
    {
        if ( !(nStyle & WB_NODIALOGCONTROL) )
            nStyle |= WB_DIALOGCONTROL;

        if( nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_CLOSEABLE | WB_STANDALONE)
            && !(nStyle & WB_OWNERDRAWDECORATION) )
        {
            WinBits nFloatWinStyle = nStyle;
            // #99154# floaters are not closeable by default anymore, eg fullscreen floater
            // nFloatWinStyle |= WB_CLOSEABLE;
            mpWindowImpl->mbFrame = true;
            mpWindowImpl->mbOverlapWin = true;
            SystemWindow::ImplInit( pParent, nFloatWinStyle & ~WB_BORDER, NULL );
        }
        else
        {
            ImplBorderWindow*   pBorderWin;
            sal_uInt16              nBorderStyle = BORDERWINDOW_STYLE_BORDER | BORDERWINDOW_STYLE_FLOAT;

            if( nStyle & WB_OWNERDRAWDECORATION ) nBorderStyle |= BORDERWINDOW_STYLE_FRAME;
            else                                  nBorderStyle |= BORDERWINDOW_STYLE_OVERLAP;

            if ( (nStyle & WB_SYSTEMWINDOW) && !(nStyle & (WB_MOVEABLE | WB_SIZEABLE)) )
            {
                nBorderStyle |= BORDERWINDOW_STYLE_FRAME;
                nStyle |= WB_CLOSEABLE; // make undecorated floaters closeable
            }
            pBorderWin  = new ImplBorderWindow( pParent, nStyle, nBorderStyle );
            SystemWindow::ImplInit( pBorderWin, nStyle & ~WB_BORDER, NULL );
            pBorderWin->mpWindowImpl->mpClientWindow = this;
            pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
            pBorderWin->SetDisplayActive( true );
            mpWindowImpl->mpBorderWindow  = pBorderWin;
            mpWindowImpl->mpRealParent    = pParent;
        }
    }
    SetActivateMode( 0 );

    mpNextFloat             = NULL;
    mpFirstPopupModeWin     = NULL;
    mnPostId                = 0;
    mnTitle                 = (nStyle & (WB_MOVEABLE | WB_POPUP)) ? FLOATWIN_TITLE_NORMAL : FLOATWIN_TITLE_NONE;
    mnOldTitle              = mnTitle;
    mnPopupModeFlags        = 0;
    mbInPopupMode           = false;
    mbPopupMode             = false;
    mbPopupModeCanceled     = false;
    mbPopupModeTearOff      = false;
    mbMouseDown             = false;

    ImplInitSettings();
}



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



void FloatingWindow::ImplLoadRes( const ResId& rResId )
{
    SystemWindow::ImplLoadRes( rResId );

    sal_uLong nObjMask = ReadLongRes();

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

    delete mpImplData;
}



Point FloatingWindow::CalcFloatingPosition( Window* pWindow, const Rectangle& rRect, sal_uLong nFlags, sal_uInt16& rArrangeIndex )
{
    return ImplCalcPos( pWindow, rRect, nFlags, rArrangeIndex );
}



Point FloatingWindow::ImplCalcPos( Window* pWindow,
                                   const Rectangle& rRect, sal_uLong nFlags,
                                   sal_uInt16& rArrangeIndex )
{
    // Fenster-Position ermitteln
    Point       aPos;
    Size        aSize = pWindow->GetSizePixel();
    Rectangle   aScreenRect = pWindow->ImplGetFrameWindow()->GetDesktopRectPixel();
    FloatingWindow *pFloatingWindow = dynamic_cast<FloatingWindow*>( pWindow );

    // convert....
    Window* pW = pWindow;
    if ( pW->mpWindowImpl->mpRealParent )
        pW = pW->mpWindowImpl->mpRealParent;

    Rectangle normRect( rRect );  // rRect is already relative to top-level window
    normRect.SetPos( pW->ScreenToOutputPixel( normRect.TopLeft() ) );

    bool bRTL = Application::GetSettings().GetLayoutRTL();

    Rectangle devRect(  pW->OutputToAbsoluteScreenPixel( normRect.TopLeft() ),
                        pW->OutputToAbsoluteScreenPixel( normRect.BottomRight() ) );

    Rectangle devRectRTL( devRect );
    if( bRTL )
        // create a rect that can be compared to desktop coordinates
        devRectRTL = pW->ImplOutputToUnmirroredAbsoluteScreenPixel( normRect );
    if( Application::GetScreenCount() > 1 && Application::IsUnifiedDisplay() )
        aScreenRect = Application::GetScreenPosSizePixel(
            Application::GetBestScreen( bRTL ? devRectRTL : devRect ) );


    sal_uInt16      nArrangeAry[5];
    sal_uInt16      nArrangeIndex;
    bool        bBreak;
    Point       e1,e2;  // the common edge between the item rect and the floating window

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
        bBreak = true;
        switch ( nArrangeAry[nArrangeIndex] )
        {

            case FLOATWIN_POPUPMODE_LEFT:
                aPos.X() = devRect.Left()-aSize.Width()+1;
                aPos.Y() = devRect.Top();
                aPos.Y() -= pWindow->mpWindowImpl->mnTopBorder;
                if( bRTL ) // --- RTL --- we're comparing screen coordinates here
                {
                    if( (devRectRTL.Right()+aSize.Width()) > aScreenRect.Right() )
                        bBreak = false;
                }
                else
                {
                    if ( aPos.X() < aScreenRect.Left() )
                        bBreak = false;
                }
                if( bBreak )
                {
                    e1 = devRect.TopLeft();
                    e2 = devRect.BottomLeft();
                    // set non-zero width
                    e2.X()++;
                    // don't clip corners
                    e1.Y()++;
                    e2.Y()--;
                }
                break;
            case FLOATWIN_POPUPMODE_RIGHT:
                aPos     = devRect.TopRight();
                aPos.Y() -= pWindow->mpWindowImpl->mnTopBorder;
                if( bRTL ) // --- RTL --- we're comparing screen coordinates here
                {
                    if( (devRectRTL.Left() - aSize.Width()) < aScreenRect.Left() )
                        bBreak = false;
                }
                else
                {
                    if ( aPos.X()+aSize.Width() > aScreenRect.Right() )
                        bBreak = false;
                }
                if( bBreak )
                {
                    e1 = devRect.TopRight();
                    e2 = devRect.BottomRight();
                    // set non-zero width
                    e2.X()++;
                    // don't clip corners
                    e1.Y()++;
                    e2.Y()--;
                }
                break;
            case FLOATWIN_POPUPMODE_UP:
                aPos.X() = devRect.Left();
                aPos.Y() = devRect.Top()-aSize.Height()+1;
                if ( aPos.Y() < aScreenRect.Top() )
                    bBreak = false;
                if( bBreak )
                {
                    e1 = devRect.TopLeft();
                    e2 = devRect.TopRight();
                    // set non-zero height
                    e2.Y()++;
                    // don't clip corners
                    e1.X()++;
                    e2.X()--;
                }
                break;
            case FLOATWIN_POPUPMODE_DOWN:
                aPos = devRect.BottomLeft();
                if ( aPos.Y()+aSize.Height() > aScreenRect.Bottom() )
                    bBreak = false;
                if( bBreak )
                {
                    e1 = devRect.BottomLeft();
                    e2 = devRect.BottomRight();
                    // set non-zero height
                    e2.Y()++;
                    // don't clip corners
                    e1.X()++;
                    e2.X()--;
                }
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
                    aPos.Y() = devRect.Bottom()-aSize.Height()+1;
                    if ( aPos.Y() < aScreenRect.Top() )
                        aPos.Y() = aScreenRect.Top();
                }
            }
            else
            {
                if( bRTL ) // --- RTL --- we're comparing screen coordinates here
                {
                    if( devRectRTL.Right()-aSize.Width()+1 < aScreenRect.Left() )
                        aPos.X() -= aScreenRect.Left() - devRectRTL.Right() + aSize.Width() - 1;
                }
                else if ( aPos.X()+aSize.Width() > aScreenRect.Right() )
                {
                    aPos.X() = devRect.Right()-aSize.Width()+1;
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

    rArrangeIndex = nArrangeIndex;

    aPos = pW->AbsoluteScreenToOutputPixel( aPos );

    // store a cliprect that can be used to clip the common edge of the itemrect and the floating window
    if( pFloatingWindow )
    {
        pFloatingWindow->mpImplData->maItemEdgeClipRect =
            Rectangle( e1, e2 );
    }

    // caller expects cordinates relative to top-level win
    return pW->OutputToScreenPixel( aPos );
}



FloatingWindow* FloatingWindow::ImplFloatHitTest( Window* pReference, const Point& rPos, sal_uInt16& rHitTest )
{
    FloatingWindow* pWin = this;

    Point aAbsolute( rPos );

    const OutputDevice *pWindowOutDev = pReference->GetOutDev();

    // compare coordinates in absolute screen coordinates
    if( pReference->HasMirroredGraphics()  )
    {
        if(!pReference->IsRTLEnabled() )
            // --- RTL --- re-mirror back to get device coordiantes
            pWindowOutDev->ReMirror( aAbsolute );

        Rectangle aRect( pReference->ScreenToOutputPixel(aAbsolute), Size(1,1) ) ;
        aRect = pReference->ImplOutputToUnmirroredAbsoluteScreenPixel( aRect );
        aAbsolute = aRect.TopLeft();
    }
    else
        aAbsolute = Point( pReference->OutputToAbsoluteScreenPixel(
            pReference->ScreenToOutputPixel(rPos) ) );

    do
    {
        // compute the floating window's size in absolute screen coordinates

        // use the border window to have the exact position
        Window *pBorderWin = pWin->GetWindow( WINDOW_BORDER );

        Point aPt;  // the top-left corner in output coordinates ie (0,0)
        Rectangle devRect( pBorderWin->ImplOutputToUnmirroredAbsoluteScreenPixel( Rectangle( aPt, pBorderWin->GetSizePixel()) ) ) ;
        if ( devRect.IsInside( aAbsolute ) )
        {
            rHitTest = IMPL_FLOATWIN_HITTEST_WINDOW;
            return pWin;
        }

        // test, if mouse is in rectangle, (this is typically the rect of the active
        // toolbox item or similar)
        // note: maFloatRect is set in FloatingWindow::StartPopupMode() and
        //       is already in absolute device coordinates
        if ( pWin->maFloatRect.IsInside( aAbsolute ) )
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



bool FloatingWindow::ImplIsFloatPopupModeWindow( const Window* pWindow )
{
    FloatingWindow* pWin = this;

    do
    {
        if ( pWin->mpFirstPopupModeWin == pWindow )
            return true;

        pWin = pWin->mpNextFloat;
    }
    while ( pWin );

    return false;
}



IMPL_LINK_NOARG(FloatingWindow, ImplEndPopupModeHdl)
{
    mnPostId            = 0;
    mnPopupModeFlags    = 0;
    mbPopupMode         = false;
    PopupModeEnd();
    return 0;
}



bool FloatingWindow::Notify( NotifyEvent& rNEvt )
{
    // Zuerst Basisklasse rufen wegen TabSteuerung
    bool nRet = SystemWindow::Notify( rNEvt );
    if ( !nRet )
    {
        if ( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            KeyCode         aKeyCode = pKEvt->GetKeyCode();
            sal_uInt16          nKeyCode = aKeyCode.GetCode();

            if ( (nKeyCode == KEY_ESCAPE) && (GetStyle() & WB_CLOSEABLE) )
            {
                Close();
                return true;
            }
        }
    }

    return nRet;
}



void FloatingWindow::StateChanged( StateChangedType nType )
{
    SystemWindow::StateChanged( nType );

    if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}



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



void FloatingWindow::ImplCallPopupModeEnd()
{
    // PopupMode wurde beendet
    mbInPopupMode = false;

    // Handler asyncron rufen
    if ( !mnPostId )
        Application::PostUserEvent( mnPostId, LINK( this, FloatingWindow, ImplEndPopupModeHdl ) );
}



void FloatingWindow::PopupModeEnd()
{
    maPopupModeEndHdl.Call( this );
}



void FloatingWindow::SetTitleType( sal_uInt16 nTitle )
{
    if ( (mnTitle != nTitle) && mpWindowImpl->mpBorderWindow )
    {
        mnTitle = nTitle;
        Size aOutSize = GetOutputSizePixel();
        sal_uInt16 nTitleStyle;
        if ( nTitle == FLOATWIN_TITLE_NORMAL )
            nTitleStyle = BORDERWINDOW_TITLE_SMALL;
        else if ( nTitle == FLOATWIN_TITLE_TEAROFF )
            nTitleStyle = BORDERWINDOW_TITLE_TEAROFF;
        else if ( nTitle == FLOATWIN_TITLE_POPUP )
            nTitleStyle = BORDERWINDOW_TITLE_POPUP;
        else // nTitle == FLOATWIN_TITLE_NONE
            nTitleStyle = BORDERWINDOW_TITLE_NONE;
        ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetTitleType( nTitleStyle, aOutSize );
        ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
    }
}



void FloatingWindow::StartPopupMode( const Rectangle& rRect, sal_uLong nFlags )
{
    // avoid flickering
    if ( IsVisible() )
        Show( false, SHOW_NOFOCUSCHANGE );

    if ( IsRollUp() )
        RollDown();

    // remove title
    mnOldTitle = mnTitle;
    if ( ( mpWindowImpl->mnStyle & WB_POPUP ) && !GetText().isEmpty() )
        SetTitleType( FLOATWIN_TITLE_POPUP );
    else if ( nFlags & FLOATWIN_POPUPMODE_ALLOWTEAROFF )
        SetTitleType( FLOATWIN_TITLE_TEAROFF );
    else
        SetTitleType( FLOATWIN_TITLE_NONE );

    // avoid close on focus change for decorated floating windows only
    if( mpWindowImpl->mbFrame && (GetStyle() & WB_MOVEABLE) )
        nFlags |= FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE;

    // #102010# For debugging Accessibility
    static const char* pEnv = getenv("SAL_FLOATWIN_NOAPPFOCUSCLOSE" );
    if( pEnv && *pEnv )
        nFlags |= FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE;

    // compute window position according to flags and arrangement
    sal_uInt16 nArrangeIndex;
    Point aPos = ImplCalcPos( this, rRect, nFlags, nArrangeIndex );
    SetPosPixel( aPos );

    // set data and display window
    // convert maFloatRect to absolute device coordinates
    // so they can be compared across different frames
    // !!! rRect is expected to be in screen coordinates of the parent frame window !!!
    maFloatRect             = rRect;

    Window *pReference =  GetParent();
    const OutputDevice *pParentWinOutDev = pReference->GetOutDev();

    // compare coordinates in absolute screen coordinates
    // Keep in sync with FloatingWindow::ImplFloatHitTest, e.g. fdo#33509
    if( pReference->HasMirroredGraphics()  )
    {
        if(!pReference->IsRTLEnabled() )
            // --- RTL --- re-mirror back to get device coordiantes
            pParentWinOutDev->ReMirror(maFloatRect);

        maFloatRect.SetPos(pReference->ScreenToOutputPixel(maFloatRect.TopLeft()));
        maFloatRect = pReference->ImplOutputToUnmirroredAbsoluteScreenPixel(maFloatRect);
    }
    else
        maFloatRect.SetPos(pReference->OutputToAbsoluteScreenPixel(pReference->ScreenToOutputPixel(rRect.TopLeft())));

    maFloatRect.Left()     -= 2;
    maFloatRect.Top()      -= 2;
    maFloatRect.Right()    += 2;
    maFloatRect.Bottom()   += 2;
    mnPopupModeFlags        = nFlags;
    mbInPopupMode           = true;
    mbPopupMode             = true;
    mbPopupModeCanceled     = false;
    mbPopupModeTearOff      = false;
    mbMouseDown             = false;

    mbOldSaveBackMode       = IsSaveBackgroundEnabled();
    EnableSaveBackground();

    // add FloatingWindow to list of windows that are in popup mode
    ImplSVData* pSVData = ImplGetSVData();
    mpNextFloat = pSVData->maWinData.mpFirstFloat;
    pSVData->maWinData.mpFirstFloat = this;
    if( nFlags & FLOATWIN_POPUPMODE_GRABFOCUS )
    {
        // force key input even without focus (useful for menus)
        mbGrabFocus = true;
    }
    Show( true, SHOW_NOACTIVATE );
}



void FloatingWindow::StartPopupMode( ToolBox* pBox, sal_uLong nFlags )
{
    // get selected button
    sal_uInt16 nItemId = pBox->GetDownItemId();
    if ( !nItemId )
        return;

    mpImplData->mpBox = pBox;
    pBox->ImplFloatControl( true, this );

    // retrieve some data from the ToolBox
    Rectangle aRect = pBox->GetItemRect( nItemId );
    Point aPos;
    // convert to parent's screen coordinates
    aPos = GetParent()->OutputToScreenPixel( GetParent()->AbsoluteScreenToOutputPixel( pBox->OutputToAbsoluteScreenPixel( aRect.TopLeft() ) ) );
    aRect.SetPos( aPos );

    nFlags |=
        FLOATWIN_POPUPMODE_NOFOCUSCLOSE     |
//        FLOATWIN_POPUPMODE_NOMOUSECLOSE       |
        FLOATWIN_POPUPMODE_ALLMOUSEBUTTONCLOSE |
//        FLOATWIN_POPUPMODE_NOMOUSERECTCLOSE   |   // #105968# floating toolboxes should close when clicked in (parent's) float rect
        FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE;
//          |      FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE;

/*
 *  FLOATWIN_POPUPMODE_NOKEYCLOSE       |
 *  don't set since it disables closing floaters with escape
 */

    // Flags fuer Positionierung bestimmen
    if ( !(nFlags & (FLOATWIN_POPUPMODE_DOWN | FLOATWIN_POPUPMODE_UP |
                     FLOATWIN_POPUPMODE_LEFT | FLOATWIN_POPUPMODE_RIGHT |
                     FLOATWIN_POPUPMODE_NOAUTOARRANGE)) )
    {
         if ( pBox->IsHorizontal() )
             nFlags |= FLOATWIN_POPUPMODE_DOWN;
         else
             nFlags |= FLOATWIN_POPUPMODE_RIGHT;
    }

    // FloatingModus starten
    StartPopupMode( aRect, nFlags );
}



void FloatingWindow::ImplEndPopupMode( sal_uInt16 nFlags, sal_uLong nFocusId )
{
    if ( !mbInPopupMode )
        return;

    ImplSVData* pSVData = ImplGetSVData();

    mbInCleanUp = true; // prevent killing this window due to focus change while working with it

    // Bei allen nachfolgenden PopupMode-Fenster den Modus auch beenden
    while ( pSVData->maWinData.mpFirstFloat && pSVData->maWinData.mpFirstFloat != this )
        pSVData->maWinData.mpFirstFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL );


    // Fenster aus der Liste austragen
    pSVData->maWinData.mpFirstFloat = mpNextFloat;
    mpNextFloat = NULL;

    sal_uLong nPopupModeFlags = mnPopupModeFlags;

    // Wenn nicht abgerissen wurde, dann Fenster wieder Hiden
    if ( !(nFlags & FLOATWIN_POPUPMODEEND_TEAROFF) ||
         !(nPopupModeFlags & FLOATWIN_POPUPMODE_ALLOWTEAROFF) )
    {
        Show( false, SHOW_NOFOCUSCHANGE );

        // Focus evt. auf ein entsprechendes FloatingWindow weiterschalten
        if ( nFocusId )
            Window::EndSaveFocus( nFocusId );
        else if ( pSVData->maWinData.mpFocusWin && pSVData->maWinData.mpFirstFloat &&
                  ImplIsWindowOrChild( pSVData->maWinData.mpFocusWin ) )
            pSVData->maWinData.mpFirstFloat->GrabFocus();
        mbPopupModeTearOff = false;
    }
    else
    {
        mbPopupModeTearOff = true;
        if ( nFocusId )
            Window::EndSaveFocus( nFocusId, false );
    }
    EnableSaveBackground( mbOldSaveBackMode );

    mbPopupModeCanceled = (nFlags & FLOATWIN_POPUPMODEEND_CANCEL) != 0;

    // Gegebenenfalls den Title wieder herstellen
    SetTitleType( mnOldTitle );

    // ToolBox wieder auf normal schalten
    if ( mpImplData->mpBox )
    {
        mpImplData->mpBox->ImplFloatControl( false, this );
        mpImplData->mpBox = NULL;
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

    mbInCleanUp = false;
}



void FloatingWindow::EndPopupMode( sal_uInt16 nFlags )
{
    ImplEndPopupMode( nFlags );
}



void FloatingWindow::AddPopupModeWindow( Window* pWindow )
{
    // !!! bisher erst 1 Fenster und noch keine Liste
    mpFirstPopupModeWin = pWindow;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
