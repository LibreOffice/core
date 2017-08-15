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


class FloatingWindow::ImplData
{
public:
    ImplData();

    VclPtr<ToolBox> mpBox;
    tools::Rectangle       maItemEdgeClipRect; // used to clip the common edge between a toolbar item and the border of this window
    Point maPos; // position of the floating window wrt. parent
};

FloatingWindow::ImplData::ImplData()
{
    mpBox = nullptr;
}

tools::Rectangle& FloatingWindow::ImplGetItemEdgeClipRect()
{
    return mpImplData->maItemEdgeClipRect;
}

void FloatingWindow::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    mpImplData = new ImplData;

    mpWindowImpl->mbFloatWin = true;
    mbInCleanUp = false;
    mbGrabFocus = false;

    SAL_WARN_IF(!pParent, "vcl", "FloatWindow::FloatingWindow(): - pParent == NULL!");

    if (!pParent)
        pParent = ImplGetSVData()->maWinData.mpAppWin;

    SAL_WARN_IF(!pParent, "vcl", "FloatWindow::FloatingWindow(): - pParent == NULL and no AppWindow exists");

    // no Border, then we don't need a border window
    if (!nStyle)
    {
        mpWindowImpl->mbOverlapWin = true;
        nStyle |= WB_DIALOGCONTROL;
        SystemWindow::ImplInit(pParent, nStyle, nullptr);
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
            SystemWindow::ImplInit(pParent, nFloatWinStyle & ~WB_BORDER, nullptr);
        }
        else
        {
            VclPtr<ImplBorderWindow> pBorderWin;
            BorderWindowStyle nBorderStyle = BorderWindowStyle::Border | BorderWindowStyle::Float;

            if (nStyle & WB_OWNERDRAWDECORATION)
                nBorderStyle |= BorderWindowStyle::Frame;
            else
                nBorderStyle |= BorderWindowStyle::Overlap;

            if ((nStyle & WB_SYSTEMWINDOW) && !(nStyle & (WB_MOVEABLE | WB_SIZEABLE)))
            {
                nBorderStyle |= BorderWindowStyle::Frame;
                nStyle |= WB_CLOSEABLE; // make undecorated floaters closeable
            }
            pBorderWin  = VclPtr<ImplBorderWindow>::Create(pParent, nStyle, nBorderStyle);
            SystemWindow::ImplInit(pBorderWin, nStyle & ~WB_BORDER, nullptr);
            pBorderWin->mpWindowImpl->mpClientWindow = this;
            pBorderWin->GetBorder(mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder,
                                  mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder);
            pBorderWin->SetDisplayActive(true);
            mpWindowImpl->mpBorderWindow = pBorderWin;
            mpWindowImpl->mpRealParent = pParent;
        }
    }
    SetActivateMode( ActivateModeFlags::NONE );

    mpNextFloat             = nullptr;
    mpFirstPopupModeWin     = nullptr;
    mnPostId                = nullptr;
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
    SystemWindow(WindowType::FLOATINGWINDOW)
{
    ImplInit(pParent, nStyle);
}

FloatingWindow::FloatingWindow(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame)
    : SystemWindow(WindowType::FLOATINGWINDOW)
    , mpNextFloat(nullptr)
    , mpFirstPopupModeWin(nullptr)
    , mpImplData(nullptr)
    , mnPostId(nullptr)
    , mnPopupModeFlags(FloatWinPopupFlags::NONE)
    , mnTitle(FloatWinTitleType::Unknown)
    , mnOldTitle(FloatWinTitleType::Unknown)
    , mbInPopupMode(false)
    , mbPopupMode(false)
    , mbPopupModeCanceled(false)
    , mbPopupModeTearOff(false)
    , mbMouseDown(false)
    , mbGrabFocus(false)
    , mbInCleanUp(false)
{
    loadUI(pParent, rID, rUIXMLDescription, rFrame);
}

//Find the real parent stashed in mpDialogParent.
void FloatingWindow::doDeferredInit(WinBits nBits)
{
    vcl::Window *pParent = mpDialogParent;
    mpDialogParent = nullptr;
    ImplInit(pParent, nBits);
    mbIsDeferredInit = false;
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
    assert (!mnPostId);
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
        mnPostId = nullptr;
    }

    delete mpImplData;
    mpImplData = nullptr;

    mpNextFloat.clear();
    mpFirstPopupModeWin.clear();
    mxPrevFocusWin.clear();
    SystemWindow::dispose();
}

Point FloatingWindow::CalcFloatingPosition( vcl::Window* pWindow, const tools::Rectangle& rRect, FloatWinPopupFlags nFlags, sal_uInt16& rArrangeIndex )
{
    return ImplCalcPos( pWindow, rRect, nFlags, rArrangeIndex );
}

Point FloatingWindow::ImplCalcPos( vcl::Window* pWindow,
                                   const tools::Rectangle& rRect, FloatWinPopupFlags nFlags,
                                   sal_uInt16& rArrangeIndex )
{
    // get window position
    Point       aPos;
    Size        aSize = ::isLayoutEnabled(pWindow) ? pWindow->get_preferred_size() : pWindow->GetSizePixel();
    tools::Rectangle   aScreenRect = pWindow->ImplGetFrameWindow()->GetDesktopRectPixel();
    FloatingWindow *pFloatingWindow = dynamic_cast<FloatingWindow*>( pWindow );

    // convert....
    vcl::Window* pW = pWindow;
    if ( pW->mpWindowImpl->mpRealParent )
        pW = pW->mpWindowImpl->mpRealParent;

    tools::Rectangle normRect( rRect );  // rRect is already relative to top-level window
    normRect.SetPos( pW->ScreenToOutputPixel( normRect.TopLeft() ) );

    bool bRTL = AllSettings::GetLayoutRTL();

    tools::Rectangle devRect(  pW->OutputToAbsoluteScreenPixel( normRect.TopLeft() ),
                        pW->OutputToAbsoluteScreenPixel( normRect.BottomRight() ) );

    tools::Rectangle devRectRTL( devRect );
    if( bRTL )
        // create a rect that can be compared to desktop coordinates
        devRectRTL = pW->ImplOutputToUnmirroredAbsoluteScreenPixel( normRect );
    if( Application::GetScreenCount() > 1 && Application::IsUnifiedDisplay() )
        aScreenRect = Application::GetScreenPosSizePixel(
            Application::GetBestScreen( bRTL ? devRectRTL : devRect ) );

    FloatWinPopupFlags nArrangeAry[5];
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

    sal_uInt16 nArrangeIndex = 0;

    for ( ; nArrangeIndex < 5; nArrangeIndex++ )
    {
        bool bBreak = true;
        switch ( nArrangeAry[nArrangeIndex] )
        {

            case FloatWinPopupFlags::Left:
                aPos.X() = devRect.Left()-aSize.Width()+1;
                aPos.Y() = devRect.Top();
                aPos.Y() -= pWindow->mpWindowImpl->mnTopBorder;
                if( bRTL )
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
                if( bRTL )
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
        if (bBreak)
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
                if( bRTL )
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
            tools::Rectangle( e1, e2 );
    }

    // caller expects coordinates relative to top-level win
    return pW->OutputToScreenPixel( aPos );
}

Point FloatingWindow::ImplConvertToAbsPos(vcl::Window* pReference, const Point& rPos)
{
    Point aAbsolute( rPos );

    const OutputDevice *pWindowOutDev = pReference->GetOutDev();

    // compare coordinates in absolute screen coordinates
    if( pReference->HasMirroredGraphics()  )
    {
        if(!pReference->IsRTLEnabled() )
            pWindowOutDev->ReMirror( aAbsolute );

        tools::Rectangle aRect( pReference->ScreenToOutputPixel(aAbsolute), Size(1,1) ) ;
        aRect = pReference->ImplOutputToUnmirroredAbsoluteScreenPixel( aRect );
        aAbsolute = aRect.TopLeft();
    }
    else
        aAbsolute = pReference->OutputToAbsoluteScreenPixel(
            pReference->ScreenToOutputPixel(rPos) );

    return aAbsolute;
}

tools::Rectangle FloatingWindow::ImplConvertToAbsPos(vcl::Window* pReference, const tools::Rectangle& rRect)
{
    tools::Rectangle aFloatRect = rRect;

    const OutputDevice *pParentWinOutDev = pReference->GetOutDev();

    // compare coordinates in absolute screen coordinates
    // Keep in sync with FloatingWindow::ImplFloatHitTest, e.g. fdo#33509
    if( pReference->HasMirroredGraphics()  )
    {
        if(!pReference->IsRTLEnabled() )
            pParentWinOutDev->ReMirror(aFloatRect);

        aFloatRect.SetPos(pReference->ScreenToOutputPixel(aFloatRect.TopLeft()));
        aFloatRect = pReference->ImplOutputToUnmirroredAbsoluteScreenPixel(aFloatRect);
    }
    else
        aFloatRect.SetPos(pReference->OutputToAbsoluteScreenPixel(pReference->ScreenToOutputPixel(rRect.TopLeft())));
    return aFloatRect;
}

FloatingWindow* FloatingWindow::ImplFloatHitTest( vcl::Window* pReference, const Point& rPos, HitTest& rHitTest )
{
    FloatingWindow* pWin = this;

    Point aAbsolute(FloatingWindow::ImplConvertToAbsPos(pReference, rPos));

    do
    {
        // compute the floating window's size in absolute screen coordinates

        // use the border window to have the exact position
        vcl::Window *pBorderWin = pWin->GetWindow( GetWindowType::Border );

        Point aPt;  // the top-left corner in output coordinates ie (0,0)
        tools::Rectangle devRect( pBorderWin->ImplOutputToUnmirroredAbsoluteScreenPixel( tools::Rectangle( aPt, pBorderWin->GetSizePixel()) ) ) ;
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
    return nullptr;
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

IMPL_LINK_NOARG(FloatingWindow, ImplEndPopupModeHdl, void*, void)
{
    VclPtr<FloatingWindow> pThis(this);
    mnPostId            = nullptr;
    mnPopupModeFlags    = FloatWinPopupFlags::NONE;
    mbPopupMode         = false;
    PopupModeEnd();
}

bool FloatingWindow::EventNotify( NotifyEvent& rNEvt )
{
    // call Base Class first for tab control
    bool bRet = SystemWindow::EventNotify( rNEvt );
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

void FloatingWindow::LogicInvalidate(const tools::Rectangle* /*pRectangle*/)
{
    Dialog* pParentDlg = GetParentDialog();
    SAL_DEBUG("FloatingWindow::LogicInvalidate");
    if (pParentDlg)
    {
        SAL_DEBUG("emitiign FloatingWindow::LogicInvalidate");
        pParentDlg->InvalidateFloatingWindow(mpImplData->maPos);
    }
}

void FloatingWindow::StateChanged( StateChangedType nType )
{
    if (nType == StateChangedType::InitShow)
    {
        DoInitialLayout();
    }

    SystemWindow::StateChanged( nType );
    if (nType == StateChangedType::InitShow && IsVisible())
    {
        Dialog* pParentDlg = GetParentDialog();
        pParentDlg->InvalidateFloatingWindow(mpImplData->maPos);
    }
    else if (!IsVisible())
    {
        Dialog* pParentDlg = GetParentDialog();
        pParentDlg->CloseFloatingWindow();
    }

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
        mnPostId = Application::PostUserEvent(LINK(this, FloatingWindow, ImplEndPopupModeHdl));
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
        BorderWindowTitleType nTitleStyle;
        if ( nTitle == FloatWinTitleType::Normal )
            nTitleStyle = BorderWindowTitleType::Small;
        else if ( nTitle == FloatWinTitleType::TearOff )
            nTitleStyle = BorderWindowTitleType::Tearoff;
        else if ( nTitle == FloatWinTitleType::Popup )
            nTitleStyle = BorderWindowTitleType::Popup;
        else // nTitle == FloatWinTitleType::NONE
            nTitleStyle = BorderWindowTitleType::NONE;
        static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetTitleType( nTitleStyle, aOutSize );
        static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
    }
}

void FloatingWindow::StartPopupMode( const tools::Rectangle& rRect, FloatWinPopupFlags nFlags )
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

    // compute window position according to flags and arrangement
    sal_uInt16 nArrangeIndex;
    mpImplData->maPos = ImplCalcPos( this, rRect, nFlags, nArrangeIndex );
    SetPosPixel( mpImplData->maPos );

    // set data and display window
    // convert maFloatRect to absolute device coordinates
    // so they can be compared across different frames
    // !!! rRect is expected to be in screen coordinates of the parent frame window !!!
    maFloatRect = FloatingWindow::ImplConvertToAbsPos(GetParent(), rRect);

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

    // add FloatingWindow to list of windows that are in popup mode
    ImplSVData* pSVData = ImplGetSVData();
    mpNextFloat = pSVData->maWinData.mpFirstFloat;
    pSVData->maWinData.mpFirstFloat = this;
    if (nFlags & FloatWinPopupFlags::GrabFocus)
    {
        // force key input even without focus (useful for menus)
        mbGrabFocus = true;
        mxPrevFocusWin = Window::SaveFocus();
        mpWindowImpl->mpFrameData->mbHasFocus = true;
        GrabFocus();
    }
    Show( true, ShowFlags::NoActivate );
}

void FloatingWindow::StartPopupMode( ToolBox* pBox, FloatWinPopupFlags nFlags )
{
    mpImplData->mpBox = pBox;

    // get selected button
    sal_uInt16 nItemId = pBox->GetDownItemId();

    if ( nItemId )
        pBox->ImplFloatControl( true, this );

    // retrieve some data from the ToolBox
    tools::Rectangle aRect = nItemId ? pBox->GetItemRect( nItemId ) : pBox->GetOverflowRect();

    // convert to parent's screen coordinates
    mpImplData->maPos = GetParent()->OutputToScreenPixel( GetParent()->AbsoluteScreenToOutputPixel( pBox->OutputToAbsoluteScreenPixel( aRect.TopLeft() ) ) );
    aRect.SetPos( mpImplData->maPos );

    nFlags |=
        FloatWinPopupFlags::AllMouseButtonClose |
        FloatWinPopupFlags::NoMouseUpClose;

    // set Flags for positioning
    if ( !(nFlags & (FloatWinPopupFlags::Down | FloatWinPopupFlags::Up |
                     FloatWinPopupFlags::Left | FloatWinPopupFlags::Right)) )
    {
         if ( pBox->IsHorizontal() )
             nFlags |= FloatWinPopupFlags::Down;
         else
             nFlags |= FloatWinPopupFlags::Right;
    }

    // start FloatingMode
    StartPopupMode( aRect, nFlags );
}

void FloatingWindow::ImplEndPopupMode( FloatWinPopupEndFlags nFlags, const VclPtr<vcl::Window>& xFocusId )
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
    mpNextFloat = nullptr;

    FloatWinPopupFlags nPopupModeFlags = mnPopupModeFlags;

    // hide window again if it was not deleted
    if ( !(nFlags & FloatWinPopupEndFlags::TearOff) ||
         !(nPopupModeFlags & FloatWinPopupFlags::AllowTearOff) )
    {
        Show( false, ShowFlags::NoFocusChange );

        if (HasChildPathFocus() && xFocusId != nullptr)
        {
            // restore focus to previous focus window if we still have the focus
            Window::EndSaveFocus(xFocusId);
        }
        else if ( pSVData->maWinData.mpFocusWin && pSVData->maWinData.mpFirstFloat &&
                  ImplIsWindowOrChild( pSVData->maWinData.mpFocusWin ) )
        {
            // maybe pass focus on to a suitable FloatingWindow
            pSVData->maWinData.mpFirstFloat->GrabFocus();
        }
        mbPopupModeTearOff = false;
    }
    else
    {
        mbPopupModeTearOff = true;
    }

    mbPopupModeCanceled = bool(nFlags & FloatWinPopupEndFlags::Cancel);

    // redo title
    SetTitleType( mnOldTitle );

    // set ToolBox again to normal
    if (mpImplData && mpImplData->mpBox)
    {
        mpImplData->mpBox->ImplFloatControl( false, this );
        // if the parent ToolBox is in popup mode, it should be closed too.
        if ( GetDockingManager()->IsInPopupMode( mpImplData->mpBox ) )
            nFlags |= FloatWinPopupEndFlags::CloseAll;

        mpImplData->mpBox = nullptr;
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
    ImplEndPopupMode(nFlags, mxPrevFocusWin);
}

void FloatingWindow::AddPopupModeWindow( vcl::Window* pWindow )
{
    // !!! up-to-now only 1 window and not yet a list
    mpFirstPopupModeWin = pWindow;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
