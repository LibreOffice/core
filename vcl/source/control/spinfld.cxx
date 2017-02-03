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

#include "tools/rc.h"

#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/settings.hxx>
#include <vcl/uitest/uiobject.hxx>

#include "controldata.hxx"
#include "spin.hxx"
#include "svdata.hxx"

namespace {

void ImplGetSpinbuttonValue(vcl::Window* pWin,
                            const Rectangle& rUpperRect, const Rectangle& rLowerRect,
                            bool bUpperIn, bool bLowerIn, bool bUpperEnabled, bool bLowerEnabled,
                            bool bHorz, SpinbuttonValue& rValue )
{
    // convert spinbutton data to a SpinbuttonValue structure for native painting

    rValue.maUpperRect = rUpperRect;
    rValue.maLowerRect = rLowerRect;

    Point aPointerPos = pWin->GetPointerPosPixel();

    ControlState nState = ControlState::ENABLED;
    if (bUpperIn)
        nState |= ControlState::PRESSED;
    if (!pWin->IsEnabled() || !bUpperEnabled)
        nState &= ~ControlState::ENABLED;
    if (pWin->HasFocus())
        nState |= ControlState::FOCUSED;
    if (pWin->IsMouseOver() && rUpperRect.IsInside(aPointerPos))
        nState |= ControlState::ROLLOVER;
    rValue.mnUpperState = nState;

    nState = ControlState::ENABLED;
    if (bLowerIn)
        nState |= ControlState::PRESSED;
    if (!pWin->IsEnabled() || !bLowerEnabled)
        nState &= ~ControlState::ENABLED;
    if (pWin->HasFocus())
        nState |= ControlState::FOCUSED;
    // for overlapping spins: highlight only one
    if (pWin->IsMouseOver() && rLowerRect.IsInside(aPointerPos) && !rUpperRect.IsInside(aPointerPos))
        nState |= ControlState::ROLLOVER;
    rValue.mnLowerState = nState;

    rValue.mnUpperPart = bHorz ? ControlPart::ButtonLeft : ControlPart::ButtonUp;
    rValue.mnLowerPart = bHorz ? ControlPart::ButtonRight : ControlPart::ButtonDown;
}

bool ImplDrawNativeSpinfield(vcl::RenderContext& rRenderContext, vcl::Window* pWin, const SpinbuttonValue& rSpinbuttonValue)
{
    bool bNativeOK = false;

    if (rRenderContext.IsNativeControlSupported(ControlType::Spinbox, ControlPart::Entire) &&
        // there is just no useful native support for spinfields with dropdown
        !(pWin->GetStyle() & WB_DROPDOWN))
    {
        if (rRenderContext.IsNativeControlSupported(ControlType::Spinbox, rSpinbuttonValue.mnUpperPart) &&
            rRenderContext.IsNativeControlSupported(ControlType::Spinbox, rSpinbuttonValue.mnLowerPart))
        {
            // only paint the embedded spin buttons, all buttons are painted at once
            Rectangle aUpperAndLowerButtons( rSpinbuttonValue.maUpperRect.GetUnion( rSpinbuttonValue.maLowerRect ) );
            bNativeOK = rRenderContext.DrawNativeControl(ControlType::Spinbox, ControlPart::AllButtons, aUpperAndLowerButtons,
                                                         ControlState::ENABLED, rSpinbuttonValue, OUString());
        }
        else
        {
            // paint the spinbox as a whole, use borderwindow to have proper clipping
            vcl::Window* pBorder = pWin->GetWindow(GetWindowType::Border);

            // to not overwrite everything, set the button region as clipregion to the border window
            Rectangle aClipRect(rSpinbuttonValue.maLowerRect);
            aClipRect.Union(rSpinbuttonValue.maUpperRect);

            vcl::RenderContext* pContext = &rRenderContext;
            vcl::Region oldRgn;
            Point aPt;
            Size aSize(pBorder->GetOutputSizePixel());    // the size of the border window, i.e., the whole control
            Rectangle aNatRgn(aPt, aSize);

            if (!pWin->SupportsDoubleBuffering())
            {
                // convert from screen space to borderwin space
                aClipRect.SetPos(pBorder->ScreenToOutputPixel(pWin->OutputToScreenPixel(aClipRect.TopLeft())));

                oldRgn = pBorder->GetClipRegion();
                pBorder->SetClipRegion(vcl::Region(aClipRect));

                pContext = pBorder;
            }

            Rectangle aBound, aContent;
            if (!ImplGetSVData()->maNWFData.mbCanDrawWidgetAnySize &&
                pContext->GetNativeControlRegion(ControlType::Spinbox, ControlPart::Entire,
                                                aNatRgn, ControlState::NONE, rSpinbuttonValue,
                                                OUString(), aBound, aContent))
            {
                aSize = aContent.GetSize();
            }

            Rectangle aRgn(aPt, aSize);
            if (pWin->SupportsDoubleBuffering())
            {
                // convert from borderwin space, to the pWin's space
                aRgn.SetPos(pWin->ScreenToOutputPixel(pBorder->OutputToScreenPixel(aRgn.TopLeft())));
            }

            bNativeOK = pContext->DrawNativeControl(ControlType::Spinbox, ControlPart::Entire, aRgn,
                                                   ControlState::ENABLED, rSpinbuttonValue, OUString());

            if (!pWin->SupportsDoubleBuffering())
                pBorder->SetClipRegion(vcl::Region(oldRgn));
        }
    }
    return bNativeOK;
}

bool ImplDrawNativeSpinbuttons(vcl::RenderContext& rRenderContext, const SpinbuttonValue& rSpinbuttonValue)
{
    bool bNativeOK = false;

    if (rRenderContext.IsNativeControlSupported(ControlType::SpinButtons, ControlPart::Entire))
    {
        Rectangle aArea = rSpinbuttonValue.maUpperRect.GetUnion(rSpinbuttonValue.maLowerRect);
        // only paint the standalone spin buttons, all buttons are painted at once
        bNativeOK = rRenderContext.DrawNativeControl(ControlType::SpinButtons, ControlPart::AllButtons, aArea,
                                                     ControlState::ENABLED, rSpinbuttonValue, OUString());
    }
    return bNativeOK;
}

}

void ImplDrawSpinButton(vcl::RenderContext& rRenderContext, vcl::Window* pWindow,
                        const Rectangle& rUpperRect, const Rectangle& rLowerRect,
                        bool bUpperIn, bool bLowerIn, bool bUpperEnabled, bool bLowerEnabled,
                        bool bHorz, bool bMirrorHorz)
{
    bool bNativeOK = false;

    if (pWindow)
    {
        // are we drawing standalone spin buttons or members of a spinfield ?
        ControlType aControl = ControlType::SpinButtons;
        switch (pWindow->GetType())
        {
            case WINDOW_EDIT:
            case WINDOW_MULTILINEEDIT:
            case WINDOW_PATTERNFIELD:
            case WINDOW_METRICFIELD:
            case WINDOW_CURRENCYFIELD:
            case WINDOW_DATEFIELD:
            case WINDOW_TIMEFIELD:
            case WINDOW_LONGCURRENCYFIELD:
            case WINDOW_NUMERICFIELD:
            case WINDOW_SPINFIELD:
                aControl = ControlType::Spinbox;
                break;
            default:
                aControl = ControlType::SpinButtons;
                break;
        }

        SpinbuttonValue aValue;
        ImplGetSpinbuttonValue(pWindow, rUpperRect, rLowerRect,
                               bUpperIn, bLowerIn, bUpperEnabled, bLowerEnabled,
                               bHorz, aValue);

        if( aControl == ControlType::Spinbox )
            bNativeOK = ImplDrawNativeSpinfield(rRenderContext, pWindow, aValue);
        else if( aControl == ControlType::SpinButtons )
            bNativeOK = ImplDrawNativeSpinbuttons(rRenderContext, aValue);
    }

    if (bNativeOK)
        return;

    ImplDrawUpDownButtons(rRenderContext,
                          rUpperRect, rLowerRect,
                          bUpperIn, bLowerIn, bUpperEnabled, bLowerEnabled,
                          bHorz, bMirrorHorz);
}

void ImplDrawUpDownButtons(vcl::RenderContext& rRenderContext,
                           const Rectangle& rUpperRect, const Rectangle& rLowerRect,
                           bool bUpperIn, bool bLowerIn, bool bUpperEnabled, bool bLowerEnabled,
                           bool bHorz, bool bMirrorHorz)
{
    DecorationView aDecoView(&rRenderContext);

    SymbolType eType1, eType2;

    if ( bHorz )
    {
        eType1 = bMirrorHorz ? SymbolType::SPIN_RIGHT : SymbolType::SPIN_LEFT;
        eType2 = bMirrorHorz ? SymbolType::SPIN_LEFT : SymbolType::SPIN_RIGHT;
    }
    else
    {
        eType1 = SymbolType::SPIN_UP;
        eType2 = SymbolType::SPIN_DOWN;
    }

    DrawButtonFlags nStyle = DrawButtonFlags::NoLeftLightBorder;
    // draw upper/left Button
    if (bUpperIn)
        nStyle |= DrawButtonFlags::Pressed;

    Rectangle aUpRect = aDecoView.DrawButton(rUpperRect, nStyle);

    nStyle = DrawButtonFlags::NoLeftLightBorder;
    // draw lower/right Button
    if (bLowerIn)
        nStyle |= DrawButtonFlags::Pressed;

    Rectangle aLowRect = aDecoView.DrawButton(rLowerRect, nStyle);

     // make use of additional default edge
    aUpRect.Left()--;
    aUpRect.Top()--;
    aUpRect.Right()++;
    aUpRect.Bottom()++;
    aLowRect.Left()--;
    aLowRect.Top()--;
    aLowRect.Right()++;
    aLowRect.Bottom()++;

    // draw into the edge, so that something is visible if the rectangle is too small
    if (aUpRect.GetHeight() < 4)
    {
        aUpRect.Right()++;
        aUpRect.Bottom()++;
        aLowRect.Right()++;
        aLowRect.Bottom()++;
    }

    // calculate Symbol size
    long nTempSize1 = aUpRect.GetWidth();
    long nTempSize2 = aLowRect.GetWidth();
    if (std::abs( nTempSize1-nTempSize2 ) == 1)
    {
        if (nTempSize1 > nTempSize2)
            aUpRect.Left()++;
        else
            aLowRect.Left()++;
    }
    nTempSize1 = aUpRect.GetHeight();
    nTempSize2 = aLowRect.GetHeight();
    if (std::abs(nTempSize1 - nTempSize2) == 1)
    {
        if (nTempSize1 > nTempSize2)
            aUpRect.Top()++;
        else
            aLowRect.Top()++;
    }

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    DrawSymbolFlags nSymStyle = DrawSymbolFlags::NONE;
    if (!bUpperEnabled)
        nSymStyle |= DrawSymbolFlags::Disable;
    aDecoView.DrawSymbol(aUpRect, eType1, rStyleSettings.GetButtonTextColor(), nSymStyle);

    nSymStyle = DrawSymbolFlags::NONE;
    if (!bLowerEnabled)
        nSymStyle |= DrawSymbolFlags::Disable;
    aDecoView.DrawSymbol(aLowRect, eType2, rStyleSettings.GetButtonTextColor(), nSymStyle);
}

void SpinField::ImplInitSpinFieldData()
{
    mpEdit.disposeAndClear();
    mbSpin          = false;
    mbRepeat        = false;
    mbUpperIn       = false;
    mbLowerIn       = false;
    mbInitialUp     = false;
    mbInitialDown   = false;
    mbNoSelect      = false;
    mbInDropDown    = false;
}

void SpinField::ImplInit(vcl::Window* pParent, WinBits nWinStyle)
{
    Edit::ImplInit( pParent, nWinStyle );

    if (nWinStyle & (WB_SPIN | WB_DROPDOWN))
    {
        mbSpin = true;

        // Some themes want external spin buttons, therefore the main
        // spinfield should not overdraw the border between its encapsulated
        // edit field and the spin buttons
        if ((nWinStyle & WB_SPIN) && ImplUseNativeBorder(*this, nWinStyle))
        {
            SetBackground();
            mpEdit.set(VclPtr<Edit>::Create(this, WB_NOBORDER));
            mpEdit->SetBackground();
        }
        else
            mpEdit.set(VclPtr<Edit>::Create(this, WB_NOBORDER));

        mpEdit->EnableRTL(false);
        mpEdit->SetPosPixel(Point());
        mpEdit->Show();

        SetSubEdit(mpEdit);

        maRepeatTimer.SetInvokeHandler(LINK( this, SpinField, ImplTimeout));
        maRepeatTimer.SetTimeout(GetSettings().GetMouseSettings().GetButtonStartRepeat());
        if (nWinStyle & WB_REPEAT)
            mbRepeat = true;

        SetCompoundControl(true);
    }
}

SpinField::SpinField(vcl::Window* pParent, WinBits nWinStyle) :
    Edit(WINDOW_SPINFIELD)
{
    ImplInitSpinFieldData();
    ImplInit(pParent, nWinStyle);
}

SpinField::~SpinField()
{
    disposeOnce();
}

void SpinField::dispose()
{
    mpEdit.disposeAndClear();

    Edit::dispose();
}

void SpinField::Up()
{
    ImplCallEventListenersAndHandler( VclEventId::SpinfieldUp, [this] () { maUpHdlLink.Call(*this); } );
}

void SpinField::Down()
{
    ImplCallEventListenersAndHandler( VclEventId::SpinfieldDown, [this] () { maDownHdlLink.Call(*this); } );
}

void SpinField::First()
{
    ImplCallEventListenersAndHandler( VclEventId::SpinfieldFirst, [this] () { maFirstHdlLink.Call(*this); } );
}

void SpinField::Last()
{
    ImplCallEventListenersAndHandler( VclEventId::SpinfieldLast, [this] () { maLastHdlLink.Call(*this); } );
}

void SpinField::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!HasFocus() && (!mpEdit || !mpEdit->HasFocus()))
    {
        mbNoSelect = true;
        GrabFocus();
    }

    if (!IsReadOnly())
    {
        if (maUpperRect.IsInside(rMEvt.GetPosPixel()))
        {
            mbUpperIn   = true;
            mbInitialUp = true;
            Invalidate(maUpperRect);
        }
        else if (maLowerRect.IsInside(rMEvt.GetPosPixel()))
        {
            mbLowerIn    = true;
            mbInitialDown = true;
            Invalidate(maLowerRect);
        }
        else if (maDropDownRect.IsInside(rMEvt.GetPosPixel()))
        {
            // put DropDownButton to the right
            mbInDropDown = ShowDropDown( !mbInDropDown );
            Invalidate(Rectangle(Point(), GetOutputSizePixel()));
        }

        if (mbUpperIn || mbLowerIn)
        {
            Update();
            CaptureMouse();
            if (mbRepeat)
                maRepeatTimer.Start();
            return;
        }
    }

    Edit::MouseButtonDown(rMEvt);
}

void SpinField::MouseButtonUp(const MouseEvent& rMEvt)
{
    ReleaseMouse();
    mbInitialUp = mbInitialDown = false;
    maRepeatTimer.Stop();
    maRepeatTimer.SetTimeout(GetSettings().GetMouseSettings().GetButtonStartRepeat());

    if (mbUpperIn)
    {
        mbUpperIn = false;
        Invalidate(maUpperRect);
        Update();
        Up();
    }
    else if (mbLowerIn)
    {
        mbLowerIn = false;
        Invalidate(maLowerRect);
        Update();
        Down();
    }

    Edit::MouseButtonUp(rMEvt);
}

void SpinField::MouseMove(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft())
    {
        if (mbInitialUp)
        {
            bool bNewUpperIn = maUpperRect.IsInside(rMEvt.GetPosPixel());
            if (bNewUpperIn != mbUpperIn)
            {
                if (bNewUpperIn)
                {
                    if (mbRepeat)
                        maRepeatTimer.Start();
                }
                else
                    maRepeatTimer.Stop();

                mbUpperIn = bNewUpperIn;
                Invalidate(maUpperRect);
                Update();
            }
        }
        else if (mbInitialDown)
        {
            bool bNewLowerIn = maLowerRect.IsInside(rMEvt.GetPosPixel());
            if (bNewLowerIn != mbLowerIn)
            {
                if (bNewLowerIn)
                {
                    if (mbRepeat)
                        maRepeatTimer.Start();
                }
                else
                    maRepeatTimer.Stop();

                mbLowerIn = bNewLowerIn;
                Invalidate(maLowerRect);
                Update();
            }
        }
    }

    Edit::MouseMove(rMEvt);
}

bool SpinField::EventNotify(NotifyEvent& rNEvt)
{
    bool bDone = false;
    if (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
    {
        const KeyEvent& rKEvt = *rNEvt.GetKeyEvent();
        if (!IsReadOnly())
        {
            sal_uInt16 nMod = rKEvt.GetKeyCode().GetModifier();
            switch (rKEvt.GetKeyCode().GetCode())
            {
                case KEY_UP:
                {
                    if (!nMod)
                    {
                        Up();
                        bDone = true;
                    }
                }
                break;
                case KEY_DOWN:
                {
                    if (!nMod)
                    {
                        Down();
                        bDone = true;
                    }
                    else if ((nMod == KEY_MOD2) && !mbInDropDown && (GetStyle() & WB_DROPDOWN))
                    {
                        mbInDropDown = ShowDropDown(true);
                        Invalidate(Rectangle(Point(), GetOutputSizePixel()));
                        bDone = true;
                    }
                }
                break;
                case KEY_PAGEUP:
                {
                    if (!nMod)
                    {
                        Last();
                        bDone = true;
                    }
                }
                break;
                case KEY_PAGEDOWN:
                {
                    if (!nMod)
                    {
                        First();
                        bDone = true;
                    }
                }
                break;
            }
        }
    }

    if (rNEvt.GetType() == MouseNotifyEvent::COMMAND)
    {
        if ((rNEvt.GetCommandEvent()->GetCommand() == CommandEventId::Wheel) && !IsReadOnly())
        {
            MouseWheelBehaviour nWheelBehavior(GetSettings().GetMouseSettings().GetWheelBehavior());
            if (nWheelBehavior == MouseWheelBehaviour::ALWAYS
               || (nWheelBehavior == MouseWheelBehaviour::FocusOnly && HasChildPathFocus()))
            {
                const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
                if (pData->GetMode() == CommandWheelMode::SCROLL)
                {
                    if (pData->GetDelta() < 0L)
                        Down();
                    else
                        Up();
                    bDone = true;
                }
            }
            else
                bDone = false;  // don't eat this event, let the default handling happen (i.e. scroll the context)
        }
    }

    return bDone || Edit::EventNotify(rNEvt);
}

void SpinField::FillLayoutData() const
{
    if (mbSpin)
    {
        mpControlData->mpLayoutData.reset( new vcl::ControlLayoutData );
        AppendLayoutData(*GetSubEdit());
        GetSubEdit()->SetLayoutDataParent(this);
    }
    else
        Edit::FillLayoutData();
}

void SpinField::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    if (mbSpin)
    {
        bool bEnable = IsEnabled();
        ImplDrawSpinButton(rRenderContext, this, maUpperRect, maLowerRect,
                           mbUpperIn, mbLowerIn, bEnable, bEnable);
    }

    if (GetStyle() & WB_DROPDOWN)
    {
        DecorationView aView(&rRenderContext);

        DrawButtonFlags nStyle = DrawButtonFlags::NoLightBorder;
        if (mbInDropDown)
            nStyle |= DrawButtonFlags::Pressed;
        Rectangle aInnerRect = aView.DrawButton(maDropDownRect, nStyle);

        SymbolType eSymbol = SymbolType::SPIN_DOWN;
        DrawSymbolFlags nSymbolStyle = IsEnabled() ? DrawSymbolFlags::NONE : DrawSymbolFlags::Disable;
        aView.DrawSymbol(aInnerRect, eSymbol, rRenderContext.GetSettings().GetStyleSettings().GetButtonTextColor(), nSymbolStyle);
    }

    Edit::Paint(rRenderContext, rRect);
}

void SpinField::ImplCalcButtonAreas(OutputDevice* pDev, const Size& rOutSz, Rectangle& rDDArea,
                                    Rectangle& rSpinUpArea, Rectangle& rSpinDownArea)
{
    const StyleSettings& rStyleSettings = pDev->GetSettings().GetStyleSettings();

    Size aSize = rOutSz;
    Size aDropDownSize;

    if (GetStyle() & WB_DROPDOWN)
    {
        long nW = rStyleSettings.GetScrollBarSize();
        nW = GetDrawPixel( pDev, nW );
        aDropDownSize = Size( CalcZoom( nW ), aSize.Height() );
        aSize.Width() -= aDropDownSize.Width();
        rDDArea = Rectangle( Point( aSize.Width(), 0 ), aDropDownSize );
        rDDArea.Top()--;
    }
    else
        rDDArea.SetEmpty();

    // calculate sizes according to the height
    if (GetStyle() & WB_SPIN)
    {
        long nBottom1 = aSize.Height()/2;
        long nBottom2 = aSize.Height()-1;
        long nTop2 = nBottom1;
        if ( !(aSize.Height() & 0x01) )
            nBottom1--;

        bool bNativeRegionOK = false;
        Rectangle aContentUp, aContentDown;

        if ((pDev->GetOutDevType() == OUTDEV_WINDOW) &&
            // there is just no useful native support for spinfields with dropdown
            ! (GetStyle() & WB_DROPDOWN) &&
            IsNativeControlSupported(ControlType::Spinbox, ControlPart::Entire))
        {
            vcl::Window *pWin = static_cast<vcl::Window*>(pDev);
            vcl::Window *pBorder = pWin->GetWindow( GetWindowType::Border );

            // get the system's spin button size
            ImplControlValue aControlValue;
            Rectangle aBound;
            Point aPoint;

            // use the full extent of the control
            Rectangle aArea( aPoint, pBorder->GetOutputSizePixel() );

            bNativeRegionOK =
                pWin->GetNativeControlRegion(ControlType::Spinbox, ControlPart::ButtonUp,
                    aArea, ControlState::NONE, aControlValue, OUString(), aBound, aContentUp) &&
                pWin->GetNativeControlRegion(ControlType::Spinbox, ControlPart::ButtonDown,
                    aArea, ControlState::NONE, aControlValue, OUString(), aBound, aContentDown);

            if (bNativeRegionOK)
            {
                // convert back from border space to local coordinates
                aPoint = pBorder->ScreenToOutputPixel( pWin->OutputToScreenPixel( aPoint ) );
                aContentUp.Move(-aPoint.X(), -aPoint.Y());
                aContentDown.Move(-aPoint.X(), -aPoint.Y());
            }
        }

        if (bNativeRegionOK)
        {
            rSpinUpArea = aContentUp;
            rSpinDownArea = aContentDown;
        }
        else
        {
            aSize.Width() -= CalcZoom( GetDrawPixel( pDev, rStyleSettings.GetSpinSize() ) );

            rSpinUpArea = Rectangle( aSize.Width(), 0, rOutSz.Width()-aDropDownSize.Width()-1, nBottom1 );
            rSpinDownArea = Rectangle( rSpinUpArea.Left(), nTop2, rSpinUpArea.Right(), nBottom2 );
        }
    }
    else
    {
        rSpinUpArea.SetEmpty();
        rSpinDownArea.SetEmpty();
    }
}

void SpinField::Resize()
{
    if (mbSpin)
    {
        Control::Resize();
        Size aSize = GetOutputSizePixel();
        bool bSubEditPositioned = false;

        if (GetStyle() & (WB_SPIN | WB_DROPDOWN))
        {
            ImplCalcButtonAreas( this, aSize, maDropDownRect, maUpperRect, maLowerRect );

            ImplControlValue aControlValue;
            Point aPoint;
            Rectangle aContent, aBound;

            // use the full extent of the control
            vcl::Window *pBorder = GetWindow( GetWindowType::Border );
            Rectangle aArea( aPoint, pBorder->GetOutputSizePixel() );

            // adjust position and size of the edit field
            if (GetNativeControlRegion(ControlType::Spinbox, ControlPart::SubEdit, aArea, ControlState::NONE,
                                       aControlValue, OUString(), aBound, aContent) &&
                // there is just no useful native support for spinfields with dropdown
                !(GetStyle() & WB_DROPDOWN))
            {
                // convert back from border space to local coordinates
                aPoint = pBorder->ScreenToOutputPixel(OutputToScreenPixel(aPoint));
                aContent.Move(-aPoint.X(), -aPoint.Y());

                // use the themes drop down size
                mpEdit->SetPosPixel( aContent.TopLeft() );
                bSubEditPositioned = true;
                aSize = aContent.GetSize();
            }
            else
            {
                if (maUpperRect.IsEmpty())
                {
                    SAL_WARN_IF( maDropDownRect.IsEmpty(), "vcl", "SpinField::Resize: SPIN && DROPDOWN, but all empty rects?" );
                    aSize.Width() = maDropDownRect.Left();
                }
                else
                    aSize.Width() = maUpperRect.Left();
            }
        }

        if (!bSubEditPositioned)
        {
            // this moves our sub edit if RTL gets switched
            mpEdit->SetPosPixel(Point());
        }
        mpEdit->SetSizePixel(aSize);

        if (GetStyle() & WB_SPIN)
            Invalidate(Rectangle(maUpperRect.TopLeft(), maLowerRect.BottomRight()));
        if (GetStyle() & WB_DROPDOWN)
            Invalidate(maDropDownRect);
    }
}

void SpinField::StateChanged(StateChangedType nType)
{
    Edit::StateChanged(nType);

    if (nType == StateChangedType::Enable)
    {
        if (mbSpin || (GetStyle() & WB_DROPDOWN))
        {
            mpEdit->Enable(IsEnabled());

            if (mbSpin)
            {
                Invalidate(maLowerRect);
                Invalidate(maUpperRect);
            }
            if (GetStyle() & WB_DROPDOWN)
                Invalidate(maDropDownRect);
        }
    }
    else if (nType == StateChangedType::Style)
    {
        if (GetStyle() & WB_REPEAT)
            mbRepeat = true;
        else
            mbRepeat = false;
    }
    else if (nType == StateChangedType::Zoom)
    {
        Resize();
        if (mpEdit)
            mpEdit->SetZoom(GetZoom());
        Invalidate();
    }
    else if (nType == StateChangedType::ControlFont)
    {
        if (mpEdit)
            mpEdit->SetControlFont(GetControlFont());
        Invalidate();
    }
    else if (nType == StateChangedType::ControlForeground)
    {
        if (mpEdit)
            mpEdit->SetControlForeground(GetControlForeground());
        Invalidate();
    }
    else if (nType == StateChangedType::ControlBackground)
    {
        if (mpEdit)
            mpEdit->SetControlBackground(GetControlBackground());
        Invalidate();
    }
    else if( nType == StateChangedType::Mirroring )
    {
        if (mpEdit)
            mpEdit->CompatStateChanged(StateChangedType::Mirroring);
        Resize();
    }
}

void SpinField::DataChanged( const DataChangedEvent& rDCEvt )
{
    Edit::DataChanged(rDCEvt);

    if ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        Resize();
        Invalidate();
    }
}

Rectangle* SpinField::ImplFindPartRect(const Point& rPt)
{
    if (maUpperRect.IsInside(rPt))
        return &maUpperRect;
    else if (maLowerRect.IsInside(rPt))
        return &maLowerRect;
    else
        return nullptr;
}

bool SpinField::PreNotify(NotifyEvent& rNEvt)
{
    const MouseEvent* pMouseEvt = nullptr;

    if ((rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != nullptr)
    {
        if (!pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged())
        {
            // trigger redraw if mouse over state has changed
            if( IsNativeControlSupported(ControlType::Spinbox, ControlPart::Entire) ||
                IsNativeControlSupported(ControlType::Spinbox, ControlPart::AllButtons) )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                Rectangle* pLastRect = ImplFindPartRect( GetLastPointerPosPixel() );
                if( pRect != pLastRect || (pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow()) )
                {
                    // FIXME: this is currently only on OS X
                    // check for other platforms that need similar handling
                    if (ImplGetSVData()->maNWFData.mbNoFocusRects && IsNativeWidgetEnabled() &&
                        IsNativeControlSupported(ControlType::Editbox, ControlPart::Entire))
                    {
                        ImplInvalidateOutermostBorder(this);
                    }
                    else
                    {
                        // paint directly
                        vcl::Region aRgn( GetActiveClipRegion() );
                        if (pLastRect)
                        {
                            SetClipRegion(vcl::Region(*pLastRect));
                            Invalidate(*pLastRect);
                            SetClipRegion( aRgn );
                        }
                        if (pRect)
                        {
                            SetClipRegion(vcl::Region(*pRect));
                            Invalidate(*pRect);
                            SetClipRegion( aRgn );
                        }
                    }
                }
            }
        }
    }

    return Edit::PreNotify(rNEvt);
}

void SpinField::EndDropDown()
{
    mbInDropDown = false;
    Invalidate(Rectangle(Point(), GetOutputSizePixel()));
}

bool SpinField::ShowDropDown( bool )
{
    return false;
}

Size SpinField::CalcMinimumSizeForText(const OUString &rString) const
{
    Size aSz = Edit::CalcMinimumSizeForText(rString);

    if ( GetStyle() & WB_DROPDOWN )
        aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
    if ( GetStyle() & WB_SPIN )
    {
        ImplControlValue aControlValue;
        Rectangle aArea( Point(), Size(100, aSz.Height()));
        Rectangle aEntireBound, aEntireContent, aEditBound, aEditContent;
        if (
               GetNativeControlRegion(ControlType::Spinbox, ControlPart::Entire,
                   aArea, ControlState::NONE, aControlValue, OUString(), aEntireBound, aEntireContent) &&
               GetNativeControlRegion(ControlType::Spinbox, ControlPart::SubEdit,
                   aArea, ControlState::NONE, aControlValue, OUString(), aEditBound, aEditContent)
           )
        {
            aSz.Width() += (aEntireContent.GetWidth() - aEditContent.GetWidth());
        }
        else
        {
            aSz.Width() += maUpperRect.GetWidth();
        }
    }

    return aSz;
}

Size SpinField::CalcMinimumSize() const
{
    return CalcMinimumSizeForText(GetText());
}

Size SpinField::GetOptimalSize() const
{
    return CalcMinimumSize();
}

Size SpinField::CalcSize(sal_Int32 nChars) const
{
    Size aSz = Edit::CalcSize( nChars );

    if ( GetStyle() & WB_DROPDOWN )
        aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
    if ( GetStyle() & WB_SPIN )
        aSz.Width() += GetSettings().GetStyleSettings().GetSpinSize();

    return aSz;
}

IMPL_LINK( SpinField, ImplTimeout, Timer*, pTimer, void )
{
    if ( pTimer->GetTimeout() == GetSettings().GetMouseSettings().GetButtonStartRepeat() )
    {
        pTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonRepeat() );
        pTimer->Start();
    }
    else
    {
        if ( mbInitialUp )
            Up();
        else
            Down();
    }
}

void SpinField::Draw(OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags)
{
    Edit::Draw(pDev, rPos, rSize, nFlags);

    WinBits nFieldStyle = GetStyle();
    if ( !(nFlags & DrawFlags::NoControls ) && ( nFieldStyle & (WB_SPIN|WB_DROPDOWN) ) )
    {
        Point aPos = pDev->LogicToPixel( rPos );
        Size aSize = pDev->LogicToPixel( rSize );
        OutDevType eOutDevType = pDev->GetOutDevType();
        AllSettings aOldSettings = pDev->GetSettings();

        pDev->Push();
        pDev->SetMapMode();

        if (eOutDevType == OUTDEV_PRINTER)
        {
            StyleSettings aStyleSettings = aOldSettings.GetStyleSettings();
            aStyleSettings.SetFaceColor(COL_LIGHTGRAY);
            aStyleSettings.SetButtonTextColor(COL_BLACK);
            AllSettings aSettings(aOldSettings);
            aSettings.SetStyleSettings(aStyleSettings);
            pDev->SetSettings(aSettings);
        }

        Rectangle aDD, aUp, aDown;
        ImplCalcButtonAreas(pDev, aSize, aDD, aUp, aDown);
        aDD.Move(aPos.X(), aPos.Y());
        aUp.Move(aPos.X(), aPos.Y());
        aUp.Top()++;
        aDown.Move(aPos.X(), aPos.Y());

        Color aButtonTextColor;
        if ((nFlags & DrawFlags::Mono) || (eOutDevType == OUTDEV_PRINTER))
            aButtonTextColor = Color( COL_BLACK );
        else
            aButtonTextColor = GetSettings().GetStyleSettings().GetButtonTextColor();

        if (GetStyle() & WB_DROPDOWN)
        {
            DecorationView aView( pDev );
            DrawButtonFlags nStyle = DrawButtonFlags::NoLightBorder;
            Rectangle aInnerRect = aView.DrawButton( aDD, nStyle );
            SymbolType eSymbol = SymbolType::SPIN_DOWN;
            DrawSymbolFlags nSymbolStyle = (IsEnabled() || (nFlags & DrawFlags::NoDisable)) ? DrawSymbolFlags::NONE : DrawSymbolFlags::Disable;
            aView.DrawSymbol(aInnerRect, eSymbol, aButtonTextColor, nSymbolStyle);
        }

        if (GetStyle() & WB_SPIN)
        {
            ImplDrawSpinButton(*pDev, this, aUp, aDown, false, false);
        }

        pDev->Pop();
        pDev->SetSettings(aOldSettings);
    }
}

FactoryFunction SpinField::GetUITestFactory() const
{
    return SpinFieldUIObject::create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
