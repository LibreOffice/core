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

#include <rtl/strbuf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/menu.hxx>
#include <vcl/decoview.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>
#include <vcl/commandevent.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/i18n/Weekdays.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <sal/log.hxx>

#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/calendar.hxx>
#include <memory>

#define CALFIELD_EXTRA_BUTTON_WIDTH         14
#define CALFIELD_EXTRA_BUTTON_HEIGHT        8
#define CALFIELD_SEP_X                      6
#define CALFIELD_BORDERLINE_X               5
#define CALFIELD_BORDER_YTOP                4
#define CALFIELD_BORDER_Y                   5

class ImplCFieldFloatWin : public FloatingWindow
{
private:
    VclPtr<Calendar>    mpCalendar;
    VclPtr<PushButton>  mpTodayBtn;
    VclPtr<PushButton>  mpNoneBtn;
    VclPtr<FixedLine>   mpFixedLine;

public:
                    explicit ImplCFieldFloatWin( vcl::Window* pParent );
    virtual         ~ImplCFieldFloatWin() override;
    virtual void    dispose() override;

    void            SetCalendar( Calendar* pCalendar )
                        { mpCalendar = pCalendar; }

    PushButton*     EnableTodayBtn( bool bEnable );
    PushButton*     EnableNoneBtn( bool bEnable );
    void            ArrangeButtons();

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
};

ImplCFieldFloatWin::ImplCFieldFloatWin( vcl::Window* pParent ) :
    FloatingWindow( pParent, WB_BORDER | WB_SYSTEMWINDOW | WB_NOSHADOW  )
{
    mpCalendar  = nullptr;
    mpTodayBtn  = nullptr;
    mpNoneBtn   = nullptr;
    mpFixedLine = nullptr;
}

ImplCFieldFloatWin::~ImplCFieldFloatWin()
{
    disposeOnce();
}

void ImplCFieldFloatWin::dispose()
{
    mpTodayBtn.disposeAndClear();
    mpNoneBtn.disposeAndClear();
    mpFixedLine.disposeAndClear();
    mpCalendar.clear();
    FloatingWindow::dispose();
}

PushButton* ImplCFieldFloatWin::EnableTodayBtn( bool bEnable )
{
    if ( bEnable )
    {
        if ( !mpTodayBtn )
        {
            mpTodayBtn = VclPtr<PushButton>::Create( this, WB_NOPOINTERFOCUS );
            OUString aTodayText(SvtResId(STR_SVT_CALENDAR_TODAY));
            mpTodayBtn->SetText( aTodayText );
            Size aSize;
            aSize.setWidth( mpTodayBtn->GetCtrlTextWidth( mpTodayBtn->GetText() ) );
            aSize.setHeight( mpTodayBtn->GetTextHeight() );
            aSize.AdjustWidth(CALFIELD_EXTRA_BUTTON_WIDTH );
            aSize.AdjustHeight(CALFIELD_EXTRA_BUTTON_HEIGHT );
            mpTodayBtn->SetSizePixel( aSize );
            mpTodayBtn->Show();
        }
    }
    else
    {
        mpTodayBtn.disposeAndClear();
    }

    return mpTodayBtn;
}

PushButton* ImplCFieldFloatWin::EnableNoneBtn( bool bEnable )
{
    if ( bEnable )
    {
        if ( !mpNoneBtn )
        {
            mpNoneBtn = VclPtr<PushButton>::Create( this, WB_NOPOINTERFOCUS );
            OUString aNoneText(SvtResId(STR_SVT_CALENDAR_NONE));
            mpNoneBtn->SetText( aNoneText );
            Size aSize;
            aSize.setWidth( mpNoneBtn->GetCtrlTextWidth( mpNoneBtn->GetText() ) );
            aSize.setHeight( mpNoneBtn->GetTextHeight() );
            aSize.AdjustWidth(CALFIELD_EXTRA_BUTTON_WIDTH );
            aSize.AdjustHeight(CALFIELD_EXTRA_BUTTON_HEIGHT );
            mpNoneBtn->SetSizePixel( aSize );
            mpNoneBtn->Show();
        }
    }
    else
    {
        mpNoneBtn.disposeAndClear();
    }

    return mpNoneBtn;
}

void ImplCFieldFloatWin::ArrangeButtons()
{
    long nBtnHeight = 0;
    long nBtnWidth  = 0;
    Size aOutSize   = GetOutputSizePixel();
    if ( mpTodayBtn && mpNoneBtn )
    {
        Size aTodayBtnSize = mpTodayBtn->GetSizePixel();
        Size aNoneBtnSize  = mpNoneBtn->GetSizePixel();
        if ( aTodayBtnSize.Width() < aNoneBtnSize.Width() )
            aTodayBtnSize.setWidth( aNoneBtnSize.Width() );
        else
            aNoneBtnSize.setWidth( aTodayBtnSize.Width() );
        if ( aTodayBtnSize.Height() < aNoneBtnSize.Height() )
            aTodayBtnSize.setHeight( aNoneBtnSize.Height() );
        else
            aNoneBtnSize.setHeight( aTodayBtnSize.Height() );

        nBtnWidth  = aTodayBtnSize.Width() + aNoneBtnSize.Width() + CALFIELD_SEP_X;
        nBtnHeight = aTodayBtnSize.Height();
        long nX = (aOutSize.Width()-nBtnWidth)/2;
        long nY = aOutSize.Height()+CALFIELD_BORDER_Y+CALFIELD_BORDER_YTOP;
        mpTodayBtn->SetPosSizePixel( Point( nX, nY ), aTodayBtnSize );
        nX += aTodayBtnSize.Width() + CALFIELD_SEP_X;
        mpNoneBtn->SetPosSizePixel( Point( nX, nY ), aNoneBtnSize );
    }
    else if ( mpTodayBtn )
    {
        Size aTodayBtnSize = mpTodayBtn->GetSizePixel();
        nBtnWidth  = aTodayBtnSize.Width();
        nBtnHeight = aTodayBtnSize.Height();
        mpTodayBtn->SetPosPixel( Point( (aOutSize.Width()-nBtnWidth)/2, aOutSize.Height()+CALFIELD_BORDER_Y+CALFIELD_BORDER_YTOP ) );
    }
    else if ( mpNoneBtn )
    {
        Size aNoneBtnSize  = mpNoneBtn->GetSizePixel();
        nBtnWidth  = aNoneBtnSize.Width();
        nBtnHeight = aNoneBtnSize.Height();
        mpNoneBtn->SetPosPixel( Point( (aOutSize.Width()-nBtnWidth)/2, aOutSize.Height()+CALFIELD_BORDER_Y+CALFIELD_BORDER_YTOP ) );
    }

    if ( nBtnHeight )
    {
        if ( !mpFixedLine )
        {
            mpFixedLine = VclPtr<FixedLine>::Create( this );
            mpFixedLine->Show();
        }
        long nLineWidth = aOutSize.Width()-(CALFIELD_BORDERLINE_X*2);
        mpFixedLine->setPosSizePixel( (aOutSize.Width()-nLineWidth)/2, aOutSize.Height()+((CALFIELD_BORDER_YTOP-2)/2),
                                      nLineWidth, 2 );
        aOutSize.AdjustHeight(nBtnHeight + (CALFIELD_BORDER_Y*2) + CALFIELD_BORDER_YTOP );
        SetOutputSizePixel( aOutSize );
    }
    else
    {
        mpFixedLine.disposeAndClear();
    }
}

bool ImplCFieldFloatWin::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        if ( pKEvt->GetKeyCode().GetCode() == KEY_RETURN )
            mpCalendar->Select();
    }

    return FloatingWindow::EventNotify( rNEvt );
}

CalendarField::CalendarField(vcl::Window* pParent, WinBits nWinStyle)
    : DateField(pParent, nWinStyle)
    , mpFloatWin(nullptr)
    , mpCalendar(nullptr)
    , mpTodayBtn(nullptr)
    , mpNoneBtn(nullptr)
    , mbToday(false)
    , mbNone(false)
{
}

CalendarField::~CalendarField()
{
    disposeOnce();
}

void CalendarField::dispose()
{
    mpCalendar.disposeAndClear();
    mpFloatWin.disposeAndClear();
    mpTodayBtn.clear();
    mpNoneBtn.clear();
    DateField::dispose();
}

IMPL_LINK( CalendarField, ImplSelectHdl, Calendar*, pCalendar, void )
{
    if ( pCalendar->IsTravelSelect() )
        return;

    mpFloatWin->EndPopupMode();
    EndDropDown();
    GrabFocus();
    Date aNewDate = mpCalendar->GetFirstSelectedDate();
    if ( IsEmptyDate() || ( aNewDate != GetDate() ) )
    {
        SetDate( aNewDate );
        SetModifyFlag();
        Modify();
    }
}

IMPL_LINK( CalendarField, ImplClickHdl, Button*, pButton, void )
{
    PushButton* pBtn = static_cast<PushButton*>(pButton);
    mpFloatWin->EndPopupMode();
    EndDropDown();
    GrabFocus();

    if ( pBtn == mpTodayBtn )
    {
        Date aToday( Date::SYSTEM );
        if ( (aToday != GetDate()) || IsEmptyDate() )
        {
            SetDate( aToday );
            SetModifyFlag();
            Modify();
        }
    }
    else if ( pBtn == mpNoneBtn )
    {
        if ( !IsEmptyDate() )
        {
            SetEmptyDate();
            SetModifyFlag();
            Modify();
        }
    }
}

IMPL_LINK_NOARG(CalendarField, ImplPopupModeEndHdl, FloatingWindow*, void)
{
    EndDropDown();
    GrabFocus();
    mpCalendar->EndSelection();
}

bool CalendarField::ShowDropDown( bool bShow )
{
    if ( bShow )
    {
        Calendar* pCalendar = GetCalendar();

        Date aDate = GetDate();
        if ( IsEmptyDate() || !aDate.IsValidAndGregorian() )
        {
            aDate = Date( Date::SYSTEM );
        }
        pCalendar->SetCurDate( aDate );
        Point       aPos( GetParent()->OutputToScreenPixel( GetPosPixel() ) );
        tools::Rectangle   aRect( aPos, GetSizePixel() );
        aRect.AdjustBottom( -1 );
        mpCalendar->SetOutputSizePixel( mpCalendar->CalcWindowSizePixel() );
        mpFloatWin->SetOutputSizePixel( mpCalendar->GetSizePixel() );
        mpFloatWin->SetCalendar( mpCalendar );
        mpTodayBtn = mpFloatWin->EnableTodayBtn( mbToday );
        mpNoneBtn = mpFloatWin->EnableNoneBtn( mbNone );
        if ( mpTodayBtn )
            mpTodayBtn->SetClickHdl( LINK( this, CalendarField, ImplClickHdl ) );
        if ( mpNoneBtn )
            mpNoneBtn->SetClickHdl( LINK( this, CalendarField, ImplClickHdl ) );
        mpFloatWin->ArrangeButtons();
        mpCalendar->EnableCallEverySelect();
        mpCalendar->StartSelection();
        mpCalendar->GrabFocus();
        mpCalendar->Show();
        mpFloatWin->StartPopupMode( aRect, FloatWinPopupFlags::Down );
    }
    else
    {
        mpFloatWin->EndPopupMode( FloatWinPopupEndFlags::Cancel );
        mpCalendar->EndSelection();
        EndDropDown();
    }
    return true;
}

Calendar* CalendarField::GetCalendar()
{
    if ( !mpFloatWin )
    {
        mpFloatWin = VclPtr<ImplCFieldFloatWin>::Create( this );
        mpFloatWin->SetPopupModeEndHdl( LINK( this, CalendarField, ImplPopupModeEndHdl ) );
        mpCalendar = VclPtr<Calendar>::Create( mpFloatWin, WB_TABSTOP );
        mpCalendar->SetPosPixel( Point() );
        mpCalendar->SetSelectHdl( LINK( this, CalendarField, ImplSelectHdl ) );
    }

    return mpCalendar;
}

void CalendarField::StateChanged( StateChangedType nStateChange )
{
    DateField::StateChanged( nStateChange );

    if ( ( nStateChange == StateChangedType::Style ) && GetSubEdit() )
    {
        WinBits nAllAlignmentBits = ( WB_LEFT | WB_CENTER | WB_RIGHT | WB_TOP | WB_VCENTER | WB_BOTTOM );
        WinBits nMyAlignment = GetStyle() & nAllAlignmentBits;
        GetSubEdit()->SetStyle( ( GetSubEdit()->GetStyle() & ~nAllAlignmentBits ) | nMyAlignment );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
