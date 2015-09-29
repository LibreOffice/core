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

#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/event.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/settings.hxx>

#include <tools/rc.h>
#include <tools/debug.hxx>

class FloatingWindow::ImplData
{
public:
    ImplData();
    ~ImplData();

    VclPtr<ToolBox> mpBox;
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

void FloatingWindow::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    mpImplData = new ImplData;

    mpWindowImpl->mbFloatWin = true;
    mbInCleanUp = false;
    mbGrabFocus = false;

    DBG_ASSERT(pParent, "FloatWindow::FloatingWindow(): - pParent == NULL!");

    if (!pParent)
        pParent = ImplGetSVData()->maWinData.mpAppWin;

    DBG_ASSERT(pParent, "FloatWindow::FloatingWindow(): - pParent == NULL and no AppWindow exists");

    // no Border, then we don't need a border window
    if (!nStyle)
    {
        mpWindowImpl->mbOverlapWin = true;
        nStyle |= WB_DIALOGCONTROL;
        SystemWindow::ImplInit(pParent, nStyle, NULL);
    }
    else
    {
        if (!(nStyle & WB_NODIALOGCONTROL))
            nStyle |= WB_DIALOGCONTROL;

        if (nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_CLOSEABLE | WB_STANDALONE)
            && !(nStyle & WB_OWNERDRAWDECORATION))
        {
            WinBits nFloatWinStyle = nStyle;
            // #99154# floaters are not closeable by default anymore, eg fullscreen floater
            // nFloatWinStyle |= WB_CLOSEABLE;
            mpWindowImpl->mbFrame = true;
            mpWindowImpl->mbOverlapWin = true;
            SystemWindow::ImplInit(pParent, nFloatWinStyle & ~WB_BORDER, NULL);
        }
        else
        {
            ImplBorderWindow* pBorderWin;
            sal_uInt16 nBorderStyle = BORDERWINDOW_STYLE_BORDER | BORDERWINDOW_STYLE_FLOAT;

            if (nStyle & WB_OWNERDRAWDECORATION)
                nBorderStyle |= BORDERWINDOW_STYLE_FRAME;
            else
                nBorderStyle |= BORDERWINDOW_STYLE_OVERLAP;

            if ((nStyle & WB_SYSTEMWINDOW) && !(nStyle & (WB_MOVEABLE | WB_SIZEABLE)))
            {
                nBorderStyle |= BORDERWINDOW_STYLE_FRAME;
                nStyle |= WB_CLOSEABLE; // make undecorated floaters closeable
            }
            pBorderWin  = VclPtr<ImplBorderWindow>::Create(pParent, nStyle, nBorderStyle);
            SystemWindow::ImplInit(pBorderWin, nStyle & ~WB_BORDER, NULL);
            pBorderWin->mpWindowImpl->mpClientWindow = this;
            pBorderWin->GetBorder(mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder,
                                  mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder);
            pBorderWin->SetDisplayActive(true);
            mpWindowImpl->mpBorderWindow = pBorderWin;
            mpWindowImpl->mpRealParent = pParent;
        }
    }
    SetActivateMode( ActivateModeFlags::NONE );

    mpNextFloat             = NULL;
    mpFirstPopupModeWin     = NULL;
    mnPostId                = 0;
    mnTitle                 = (nStyle & (WB_MOVEABLE | WB_POPUP)) ? FloatWinTitleType::Normal : FloatWinTitleType::NONE;
    mnOldTitle              = mnTitle;
    mnPopupModeFlags        = FloatWinPopupFlags::NONE;
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
    if (IsControlBackground())
        aColor = GetControlBackground();
    else if (Window::GetStyle() & WB_3DLOOK)
        aColor = rStyleSettings.GetFaceColor();
    else
        aColor = rStyleSettings.GetWindowColor();
    SetBackground(aColor);
}

FloatingWindow::FloatingWindow(vcl::Window* pParent, WinBits nStyle) :
    SystemWindow(WINDOW_FLOATINGWINDOW)
{
    ImplInit(pParent, nStyle);
}

FloatingWindow::FloatingWindow(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame)
    : SystemWindow(WINDOW_FLOATINGWINDOW)
    , mpNextFloat(0)
    , mpFirstPopupModeWin(0)
    , mpImplData(0)
    , mnPostId(0)
    , mnPopupModeFlags(FloatWinPopupFlags::NONE)
    , mnTitle(FloatWinTitleType::Unknown)
    , mnOldTitle(FloatWinTitleType::Unknown)
    , mbInPopupMode(false)
    , mbPopupMode(false)
    , mbPopupModeCanceled(false)
    , mbPopupModeTearOff(false)
    , mbMouseDown(false)
    , mbOldSaveBackMode(false)
    , mbGrabFocus(false)
    , mbInCleanUp(false)
{
    loadUI(pParent, rID, rUIXMLDescription, rFrame);
}

//Find the real parent stashed in mpDialogParent.
void FloatingWindow::doDeferredInit(WinBits nBits)
{
    vcl::Window *pParent = mpDialogParent;
    mpDialogParent = NULL;
    ImplInit(pParent, nBits);
    mbIsDefferedInit = false;
}

void FloatingWindow::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    Color aColor;
    if (Window::GetStyle() & WB_3DLOOK)
        aColor = rStyleSettings.GetFaceColor();
    else
        aColor = rStyleSettings.GetWindowColor();

    ApplyControlBackground(rRenderContext, aColor);
}

FloatingWindow::~FloatingWindow()
{
    disposeOnce();

    // Unfortunately the vclwidgets clang plug-in says: "OutputDevice
    // subclass should have nothing in its destructor but a call to
    // disposeOnce()."
    // assert (!mnPostId);
}

void FloatingWindow::dispose()
{
    if (mpImplData)
    {
        if( mbPopupModeCanceled )
            // indicates that ESC key was pressed
            // will be handled in Window::ImplGrabFocus()
            SetDialogControlFlags( GetDialogControlFlags() | DialogControlFlags::FloatWinPopupModeEndCancel );

        if ( IsInPopupMode() )
            EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll | FloatWinPopupEndFlags::DontCallHdl );

        if ( mnPostId )
            Application::RemoveUserEvent( mnPostId );
        mnPostId = 0;
    }

    delete mpImplData;
    mpImplData = NULL;

    mpNextFloat.clear();
    mpFirstPopupModeWin.clear();
    SystemWindow::dispose();
}

Point FloatingWindow::CalcFloatingPosition( vcl::Window* pWindow, const Rectangle& rRect, FloatWinPopupFlags nFlags, sal_uInt16& rArrangeIndex )
{
    return ImplCalcPos( pWindow, rRect, nFlags, rArrangeIndex );
}

Point FloatingWindow::ImplCalcPos( vcl::Window* pWindow,
                                   const Rectangle& rRect, FloatWinPopupFlags nFlags,
                                   sal_uInt16& rArrangeIndex )
{
    // get window position
    Point       aPos;
    Size        aSize = pWindow->GetSizePixel();
    Rectangle   aScreenRect = pWindow->ImplGetFrameWindow()->GetDesktopRectPixel();
    FloatingWindow *pFloatingWindow = dynamic_cast<FloatingWindow*>( pWindow );

    // convert....
    vcl::Window* pW = pWindow;
    if ( pW->mpWindowImpl->mpRealParent )
        pW = pW->mpWindowImpl->mpRealParent;

    Rectangle normRect( rRect );  // rRect is already relative to top-level window
    normRect.SetPos( pW->ScreenToOutputPixel( normRect.TopLeft() ) );

    bool bRTL = AllSettings::GetLayoutRTL();

    Rectangle devRect(  pW->OutputToAbsoluteScreenPixel( normRect.TopLeft() ),
                        pW->OutputToAbsoluteScreenPixel( normRect.BottomRight() ) );

    Rectangle devRectRTL( devRect );
    if( bRTL )
        // create a rect that can be compared to desktop coordinates
        devRectRTL = pW->ImplOutputToUnmirroredAbsoluteScreenPixel( normRect );
    if( Application::GetScreenCount() > 1 && Application::IsUnifiedDisplay() )
        aScreenRect = Application::GetScreenPosSizePixel(
            Application::GetBestScreen( bRTL ? devRectRTL : devRect ) );

    FloatWinPopupFlags nArrangeAry[5];
    sal_uInt16        nArrangeIndex;
    Point             e1,e2;  // the common edge between the item rect and the floating window

    if ( nFlags & FloatWinPopupFlags::Left )
    {
        nArrangeAry[0]  = FloatWinPopupFlags::Left;
        nArrangeAry[1]  = FloatWinPopupFlags::Right;
        nArrangeAry[2]  = FloatWinPopupFlags::Up;
        nArrangeAry[3]  = FloatWinPopupFlags::Down;
        nArrangeAry[4]  = FloatWinPopupFlags::Left;
    }
    else if ( nFlags & FloatWinPopupFlags::Right )
    {
        nArrangeAry[0]  = FloatWinPopupFlags::Right;
        nArrangeAry[1]  = FloatWinPopupFlags::Left;
        nArrangeAry[2]  = FloatWinPopupFlags::Up;
        nArrangeAry[3]  = FloatWinPopupFlags::Down;
        nArrangeAry[4]  = FloatWinPopupFlags::Right;
    }
    else if ( nFlags & FloatWinPopupFlags::Up )
    {
        nArrangeAry[0]  = FloatWinPopupFlags::Up;
        nArrangeAry[1]  = FloatWinPopupFlags::Down;
        nArrangeAry[2]  = FloatWinPopupFlags::Right;
        nArrangeAry[3]  = FloatWinPopupFlags::Left;
        nArrangeAry[4]  = FloatWinPopupFlags::Up;
    }
    else
    {
        nArrangeAry[0]  = FloatWinPopupFlags::Down;
        nArrangeAry[1]  = FloatWinPopupFlags::Up;
        nArrangeAry[2]  = FloatWinPopupFlags::Right;
        nArrangeAry[3]  = FloatWinPopupFlags::Left;
        nArrangeAry[4]  = FloatWinPopupFlags::Down;
    }
    if ( nFlags & FloatWinPopupFlags::NoAutoArrange )
        nArrangeIndex = 4;
    else
        nArrangeIndex = 0;

    for ( ; nArrangeIndex < 5; nArrangeIndex++ )
    {
        bool bBreak = true;
        switch ( nArrangeAry[nArrangeIndex] )
        {

            case FloatWinPopupFlags::Left:
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
            case FloatWinPopupFlags::Right:
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
            case FloatWinPopupFlags::Up:
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
            case FloatWinPopupFlags::Down:
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
            default: break;
        }

        // adjust if necessary
        if ( bBreak && !(nFlags & FloatWinPopupFlags::NoAutoArrange) )
        {
            if ( (nArrangeAry[nArrangeIndex] == FloatWinPopupFlags::Left)  ||
                 (nArrangeAry[nArrangeIndex] == FloatWinPopupFlags::Right) )
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

    // caller expects coordinates relative to top-level win
    return pW->OutputToScreenPixel( aPos );
}

FloatingWindow* FloatingWindow::ImplFloatHitTest( vcl::Window* pReference, const Point& rPos, HitTest& rHitTest )
{
    FloatingWindow* pWin = this;

    Point aAbsolute( rPos );

    const OutputDevice *pWindowOutDev = pReference->GetOutDev();

    // compare coordinates in absolute screen coordinates
    if( pReference->HasMirroredGraphics()  )
    {
        if(!pReference->IsRTLEnabled() )
            // --- RTL --- re-mirror back to get device coordinates
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
        vcl::Window *pBorderWin = pWin->GetWindow( GetWindowType::Border );

        Point aPt;  // the top-left corner in output coordinates ie (0,0)
        Rectangle devRect( pBorderWin->ImplOutputToUnmirroredAbsoluteScreenPixel( Rectangle( aPt, pBorderWin->GetSizePixel()) ) ) ;
        if ( devRect.IsInside( aAbsolute ) )
        {
            rHitTest = HITTEST_WINDOW;
            return pWin;
        }

        // test, if mouse is in rectangle, (this is typically the rect of the active
        // toolbox item or similar)
        // note: maFloatRect is set in FloatingWindow::StartPopupMode() and
        //       is already in absolute device coordinates
        if ( pWin->maFloatRect.IsInside( aAbsolute ) )
        {
            rHitTest = HITTEST_RECT;
            return pWin;
        }

        pWin = pWin->mpNextFloat;
    }
    while ( pWin );

    rHitTest = HITTEST_OUTSIDE;
    return NULL;
}

FloatingWindow* FloatingWindow::ImplFindLastLevelFloat()
{
    FloatingWindow* pWin = this;
    FloatingWindow* pLastFoundWin = pWin;

    do
    {
        if ( pWin->GetPopupModeFlags() & FloatWinPopupFlags::NewLevel )
            pLastFoundWin = pWin;

        pWin = pWin->mpNextFloat;
    }
    while ( pWin );

    return pLastFoundWin;
}

bool FloatingWindow::ImplIsFloatPopupModeWindow( const vcl::Window* pWindow )
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

IMPL_LINK_NOARG_TYPED(FloatingWindow, ImplEndPopupModeHdl, void*, void)
{
    VclPtr<FloatingWindow> pThis(this);
    mnPostId            = 0;
    mnPopupModeFlags    = FloatWinPopupFlags::NONE;
    mbPopupMode         = false;
    PopupModeEnd();
}

bool FloatingWindow::Notify( NotifyEvent& rNEvt )
{
    // call Base Class first for tab control
    bool bRet = SystemWindow::Notify( rNEvt );
    if ( !bRet )
    {
        if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            vcl::KeyCode    aKeyCode = pKEvt->GetKeyCode();
            sal_uInt16      nKeyCode = aKeyCode.GetCode();

            if ( (nKeyCode == KEY_ESCAPE) && (GetStyle() & WB_CLOSEABLE) )
            {
                Close();
                return true;
            }
        }
    }

    return bRet;
}

void FloatingWindow::StateChanged( StateChangedType nType )
{
    if (nType == StateChangedType::InitShow)
    {
        DoInitialLayout();
    }

    SystemWindow::StateChanged( nType );

    if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void FloatingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    SystemWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void FloatingWindow::ImplCallPopupModeEnd()
{
    // PopupMode is finished
    mbInPopupMode = false;

    // call Handler asynchronously.
    if ( mpImplData && !mnPostId )
        mnPostId = Application::PostUserEvent( LINK( this, FloatingWindow, ImplEndPopupModeHdl ), NULL, true );
}

void FloatingWindow::PopupModeEnd()
{
    maPopupModeEndHdl.Call( this );
}

void FloatingWindow::SetTitleType( FloatWinTitleType nTitle )
{
    if ( (mnTitle != nTitle) && mpWindowImpl->mpBorderWindow )
    {
        mnTitle = nTitle;
        Size aOutSize = GetOutputSizePixel();
        sal_uInt16 nTitleStyle;
        if ( nTitle == FloatWinTitleType::Normal )
            nTitleStyle = BORDERWINDOW_TITLE_SMALL;
        else if ( nTitle == FloatWinTitleType::TearOff )
            nTitleStyle = BORDERWINDOW_TITLE_TEAROFF;
        else if ( nTitle == FloatWinTitleType::Popup )
            nTitleStyle = BORDERWINDOW_TITLE_POPUP;
        else // nTitle == FloatWinTitleType::NONE
            nTitleStyle = BORDERWINDOW_TITLE_NONE;
        static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetTitleType( nTitleStyle, aOutSize );
        static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
    }
}

void FloatingWindow::StartPopupMode( const Rectangle& rRect, FloatWinPopupFlags nFlags )
{
    if ( IsRollUp() )
        RollDown();

    // remove title
    mnOldTitle = mnTitle;
    if ( ( mpWindowImpl->mnStyle & WB_POPUP ) && !GetText().isEmpty() )
        SetTitleType( FloatWinTitleType::Popup );
    else if ( nFlags & FloatWinPopupFlags::AllowTearOff )
        SetTitleType( FloatWinTitleType::TearOff );
    else
        SetTitleType( FloatWinTitleType::NONE );

    // avoid close on focus change for decorated floating windows only
    if( mpWindowImpl->mbFrame && (GetStyle() & WB_MOVEABLE) )
        nFlags |= FloatWinPopupFlags::NoAppFocusClose;

    // #102010# For debugging Accessibility
    static const char* pEnv = getenv("SAL_FLOATWIN_NOAPPFOCUSCLOSE" );
    if( pEnv && *pEnv )
        nFlags |= FloatWinPopupFlags::NoAppFocusClose;

    // compute window position according to flags and arrangement
    sal_uInt16 nArrangeIndex;
    Point aPos = ImplCalcPos( this, rRect, nFlags, nArrangeIndex );
    SetPosPixel( aPos );

    // set data and display window
    // convert maFloatRect to absolute device coordinates
    // so they can be compared across different frames
    // !!! rRect is expected to be in screen coordinates of the parent frame window !!!
    maFloatRect             = rRect;

    vcl::Window *pReference =  GetParent();
    const OutputDevice *pParentWinOutDev = pReference->GetOutDev();

    // compare coordinates in absolute screen coordinates
    // Keep in sync with FloatingWindow::ImplFloatHitTest, e.g. fdo#33509
    if( pReference->HasMirroredGraphics()  )
    {
        if(!pReference->IsRTLEnabled() )
            // --- RTL --- re-mirror back to get device coordinates
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
    if( nFlags & FloatWinPopupFlags::GrabFocus )
    {
        // force key input even without focus (useful for menus)
        mbGrabFocus = true;
    }
    Show( true, ShowFlags::NoActivate );
}

void FloatingWindow::StartPopupMode( ToolBox* pBox, FloatWinPopupFlags nFlags )
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
        FloatWinPopupFlags::NoFocusClose     |
//        FloatWinPopupFlags::NoMouseClose       |
        FloatWinPopupFlags::AllMouseButtonClose |
//        FloatWinPopupFlags::NoMouseRectClose   |   // #105968# floating toolboxes should close when clicked in (parent's) float rect
        FloatWinPopupFlags::NoMouseUpClose;
//          |      FloatWinPopupFlags::NoAppFocusClose;

/*
 *  FloatWinPopupFlags::NoKeyClose       |
 *  don't set since it disables closing floaters with escape
 */

    // set Flags for positioning
    if ( !(nFlags & (FloatWinPopupFlags::Down | FloatWinPopupFlags::Up |
                     FloatWinPopupFlags::Left | FloatWinPopupFlags::Right |
                     FloatWinPopupFlags::NoAutoArrange)) )
    {
         if ( pBox->IsHorizontal() )
             nFlags |= FloatWinPopupFlags::Down;
         else
             nFlags |= FloatWinPopupFlags::Right;
    }

    // start FloatingMode
    StartPopupMode( aRect, nFlags );
}

void FloatingWindow::ImplEndPopupMode( FloatWinPopupEndFlags nFlags, sal_uLong nFocusId )
{
    if ( !mbInPopupMode )
        return;

    ImplSVData* pSVData = ImplGetSVData();

    mbInCleanUp = true; // prevent killing this window due to focus change while working with it

    // stop the PopupMode also for all following PopupMode windows
    while ( pSVData->maWinData.mpFirstFloat && pSVData->maWinData.mpFirstFloat.get() != this )
        pSVData->maWinData.mpFirstFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel );

    // delete window from the list
    pSVData->maWinData.mpFirstFloat = mpNextFloat;
    mpNextFloat = NULL;

    FloatWinPopupFlags nPopupModeFlags = mnPopupModeFlags;

    // hide window again if it was not deleted
    if ( !(nFlags & FloatWinPopupEndFlags::TearOff) ||
         !(nPopupModeFlags & FloatWinPopupFlags::AllowTearOff) )
    {
        Show( false, ShowFlags::NoFocusChange );

        // maybe pass focus on to a suitable FloatingWindow
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

    mbPopupModeCanceled = bool(nFlags & FloatWinPopupEndFlags::Cancel);

    // redo title
    SetTitleType( mnOldTitle );

    // set ToolBox again to normal
    if ( mpImplData->mpBox )
    {
        mpImplData->mpBox->ImplFloatControl( false, this );
        mpImplData->mpBox = NULL;
    }

    // call PopupModeEnd-Handler depending on parameter
    if ( !(nFlags & FloatWinPopupEndFlags::DontCallHdl) )
        ImplCallPopupModeEnd();

    // close all other windows depending on parameter
    if ( nFlags & FloatWinPopupEndFlags::CloseAll )
    {
        if ( !(nPopupModeFlags & FloatWinPopupFlags::NewLevel) )
        {
            if ( pSVData->maWinData.mpFirstFloat )
            {
                FloatingWindow* pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
                pLastLevelFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll );
            }
        }
    }

    mbInCleanUp = false;
}

void FloatingWindow::EndPopupMode( FloatWinPopupEndFlags nFlags )
{
    ImplEndPopupMode( nFlags );
}

void FloatingWindow::AddPopupModeWindow( vcl::Window* pWindow )
{
    // !!! up-to-now only 1 window and not yet a list
    mpFirstPopupModeWin = pWindow;
}

void FloatingWindow::setPosSizeOnContainee(Size aSize, Window &rBox)
{
    sal_Int32 nBorderWidth = get_border_width();

    aSize.Width() -= mpWindowImpl->mnLeftBorder + mpWindowImpl->mnRightBorder + 2 * nBorderWidth;
    aSize.Height() -= nBorderWidth + mpWindowImpl->mnTopBorder + mpWindowImpl->mnBottomBorder + 2 * nBorderWidth;

    Point aPos(nBorderWidth, nBorderWidth);
    VclContainer::setLayoutAllocation(rBox, aPos, aSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
