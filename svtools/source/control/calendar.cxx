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

#include <vcl/InterimItemWindow.hxx>
#include <vcl/event.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/svapp.hxx>

#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/calendar.hxx>

namespace
{
    class ImplCFieldFloat final
    {
    private:
        std::unique_ptr<weld::Builder> mxBuilder;
        std::unique_ptr<weld::Container> mxContainer;
        std::unique_ptr<weld::Calendar> mxCalendar;
        std::unique_ptr<weld::Button> mxTodayBtn;
        std::unique_ptr<weld::Button> mxNoneBtn;

    public:
        ImplCFieldFloat(vcl::Window* pContainer)
            : mxBuilder(Application::CreateInterimBuilder(pContainer, "svt/ui/calendar.ui"))
            , mxContainer(mxBuilder->weld_container("Calendar"))
            , mxCalendar(mxBuilder->weld_calendar("date"))
            , mxTodayBtn(mxBuilder->weld_button("today"))
            , mxNoneBtn(mxBuilder->weld_button("none"))
        {
        }

        weld::Calendar* GetCalendar() { return mxCalendar.get(); }
        weld::Button*   EnableTodayBtn(bool bEnable);
        weld::Button*   EnableNoneBtn(bool bEnable);

        void GrabFocus()
        {
            mxCalendar->grab_focus();
        }
    };
}

struct ImplCFieldFloatWin : public DockingWindow
{
    explicit ImplCFieldFloatWin(vcl::Window* pParent);
    virtual void dispose() override;
    virtual ~ImplCFieldFloatWin() override;
    virtual void GetFocus() override;

    VclPtr<vcl::Window> mxBox;
    std::unique_ptr<ImplCFieldFloat> mxWidget;
};

ImplCFieldFloatWin::ImplCFieldFloatWin(vcl::Window* pParent)
    : DockingWindow(pParent, "InterimDockParent", "svx/ui/interimdockparent.ui")
    , mxBox(get("box"))
{
    setDeferredProperties();
    mxWidget.reset(new ImplCFieldFloat(mxBox.get()));
}

ImplCFieldFloatWin::~ImplCFieldFloatWin()
{
    disposeOnce();
}

void ImplCFieldFloatWin::dispose()
{
    mxWidget.reset();
    mxBox.disposeAndClear();
    DockingWindow::dispose();
}

void ImplCFieldFloatWin::GetFocus()
{
    DockingWindow::GetFocus();
    if (!mxWidget)
        return;
    mxWidget->GrabFocus();
}

weld::Button* ImplCFieldFloat::EnableTodayBtn(bool bEnable)
{
    mxTodayBtn->set_visible(bEnable);
    return bEnable ? mxTodayBtn.get() : nullptr;
}

weld::Button* ImplCFieldFloat::EnableNoneBtn(bool bEnable)
{
    mxNoneBtn->set_visible(bEnable);
    return bEnable ? mxNoneBtn.get() : nullptr;
}

CalendarField::CalendarField(vcl::Window* pParent, WinBits nWinStyle)
    : DateField(pParent, nWinStyle)
    , mpFloatWin(nullptr)
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
    mpTodayBtn = nullptr;
    mpNoneBtn = nullptr;
    mpFloatWin.disposeAndClear();
    DateField::dispose();
}

IMPL_LINK(CalendarField, ImplSelectHdl, weld::Calendar&, rCalendar, void)
{
    Date aNewDate = rCalendar.get_date();

    vcl::Window::GetDockingManager()->EndPopupMode(mpFloatWin);
    mpFloatWin->EnableDocking(false);
    EndDropDown();
    GrabFocus();
    if ( IsEmptyDate() || ( aNewDate != GetDate() ) )
    {
        SetDate( aNewDate );
        SetModifyFlag();
        Modify();
    }
}

IMPL_LINK(CalendarField, ImplClickHdl, weld::Button&, rBtn, void)
{
    vcl::Window::GetDockingManager()->EndPopupMode(mpFloatWin);
    mpFloatWin->EnableDocking(false);
    EndDropDown();
    GrabFocus();

    if (&rBtn == mpTodayBtn)
    {
        Date aToday( Date::SYSTEM );
        if ( (aToday != GetDate()) || IsEmptyDate() )
        {
            SetDate( aToday );
            SetModifyFlag();
            Modify();
        }
    }
    else if (&rBtn == mpNoneBtn)
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
}

bool CalendarField::ShowDropDown( bool bShow )
{
    if ( bShow )
    {
        if ( !mpFloatWin )
            mpFloatWin = VclPtr<ImplCFieldFloatWin>::Create( this );

        Date aDate = GetDate();
        if ( IsEmptyDate() || !aDate.IsValidAndGregorian() )
        {
            aDate = Date( Date::SYSTEM );
        }
        weld::Calendar* pCalendar = mpFloatWin->mxWidget->GetCalendar();
        pCalendar->set_date( aDate );
        pCalendar->connect_activated(LINK(this, CalendarField, ImplSelectHdl));
        mpTodayBtn = mpFloatWin->mxWidget->EnableTodayBtn(mbToday);
        mpNoneBtn = mpFloatWin->mxWidget->EnableNoneBtn(mbNone);
        if (mpTodayBtn)
            mpTodayBtn->connect_clicked( LINK( this, CalendarField, ImplClickHdl ) );
        if (mpNoneBtn)
            mpNoneBtn->connect_clicked( LINK( this, CalendarField, ImplClickHdl ) );
        Point aPos(GetParent()->OutputToScreenPixel(GetPosPixel()));
        tools::Rectangle aRect(aPos, GetSizePixel());
        aRect.AdjustBottom( -1 );
        DockingManager* pDockingManager = vcl::Window::GetDockingManager();
        mpFloatWin->EnableDocking(true);
        pDockingManager->SetPopupModeEndHdl(mpFloatWin, LINK(this, CalendarField, ImplPopupModeEndHdl));
        pDockingManager->StartPopupMode(mpFloatWin, aRect, FloatWinPopupFlags::Down | FloatWinPopupFlags::GrabFocus);
    }
    else
    {
        vcl::Window::GetDockingManager()->EndPopupMode(mpFloatWin);
        mpFloatWin->EnableDocking(false);
        EndDropDown();
    }
    return true;
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
