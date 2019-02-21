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

#ifndef INCLUDED_VCL_CALENDAR_HXX
#define INCLUDED_VCL_CALENDAR_HXX

#include <vcl/dllapi.h>
#include <unotools/calendarwrapper.hxx>

#include <vcl/ctrl.hxx>
#include <vcl/field.hxx>
#include <memory>
#include <set>

class MouseEvent;
class TrackingEvent;
class KeyEvent;
class HelpEvent;
class DataChangedEvent;
class FloatingWindow;
class PushButton;
class ImplCFieldFloatWin;
class Button;

/*************************************************************************

Description
============

class Calendar

This class allows for the selection of a date. The displayed date range is
the one specified by the Date class. We display as many months as we have
space in the control. The user can switch between months using a ContextMenu
(clicking on the month's name) or via two ScrollButtons in-between the months.

--------------------------------------------------------------------------

WinBits

WB_BORDER                   We draw a border around the window.
WB_TABSTOP                  Keyboard control is possible. We get the focus, when
                            the user clicks in the Control.

--------------------------------------------------------------------------

We set and get the selected date by SetCurDate()/GetCurDate().
If the user selects a date Select() is called. If the user double clicks
DoubleClick() is called.

--------------------------------------------------------------------------

CalcWindowSizePixel() calculates the window size in pixel that is needed
to display a certain number of months.

--------------------------------------------------------------------------

SetSaturdayColor() and SetSundayColor() set a special color for Saturdays
and Sundays.
AddDateInfo() marks special days. With that we can set e.g. public holidays
to another color or encircle them (for e.g. appointments).
If we do not supply a year in the date, the day is used in EVERY year.

AddDateInfo() can also add text for every date, which is displayed if the
BalloonHelp is enabled.
In order to not have to supply all years with the relevant data, we call
the RequestDateInfo() handler if a new year is displayed. We can then query
the year in the handler with GetRequestYear().

--------------------------------------------------------------------------

In order to display a ContextMenu for a date, we need to override the
Command handler. GetDate() can infer the date from the mouse's position.
If we use the keyboard, the current date should be use.

If a ContextMenu is displayed, the baseclass' handler must not be called.

--------------------------------------------------------------------------

SetNoSelection() deselects everything.
SetCurDate() does not select the current date, but only defines the focus
rectangle.
GetSelectDateCount()/GetSelectDate() query the selected range.
IsDateSelected() queries for the status of a date.

The SelectionChanging() handler is being called while a user selects a
date. In it, we can change the selected range. E.g. if we want to limit
or extend the selected range. The selected range is realised via SelectDate()
and SelectDateRange() and queried with GetSelectDateCount()/GetSelectDate().

IsSelectLeft() returns the direction of the selection:
sal_True is a selection to the left or up
sal_False is a selection to the right or down

--------------------------------------------------------------------------

If the DateRange area changes and we want to take over the selection, we
should only do this is if IsScrollDateRangeChanged() returns sal_True.
This method returns sal_True if the area change was triggered by using the
ScrollButtons and sal_False if it was triggered by Resize(), other method
calls or by ending a selection.

*************************************************************************/

typedef std::set<sal_Int32> IntDateSet;

class VCL_DLLPUBLIC Calendar final : public Control
{
    std::unique_ptr<IntDateSet> mpSelectTable;
    std::unique_ptr<IntDateSet> mpOldSelectTable;
    OUString        maDayTexts[31];
    OUString        maDayText;
    OUString        maWeekText;
    CalendarWrapper maCalendarWrapper;
    tools::Rectangle       maPrevRect;
    tools::Rectangle       maNextRect;
    OUString        maDayOfWeekText;
    long            mnDayOfWeekAry[7];
    Date            maOldFormatFirstDate;
    Date            maOldFormatLastDate;
    Date            maFirstDate;
    Date            maOldFirstDate;
    Date            maCurDate;
    Date            maOldCurDate;
    Color           maSelColor;
    Color           maOtherColor;
    sal_Int32       mnDayCount;
    long            mnDaysOffX;
    long            mnWeekDayOffY;
    long            mnDaysOffY;
    long            mnMonthHeight;
    long            mnMonthWidth;
    long            mnMonthPerLine;
    long            mnLines;
    long            mnDayWidth;
    long            mnDayHeight;
    WinBits         mnWinStyle;
    sal_Int16       mnFirstYear;
    sal_Int16       mnLastYear;
    bool            mbCalc:1,
                    mbFormat:1,
                    mbDrag:1,
                    mbSelection:1,
                    mbMenuDown:1,
                    mbSpinDown:1,
                    mbPrevIn:1,
                    mbNextIn:1,
                    mbTravelSelect:1,
                    mbAllSel:1;
    Link<Calendar*,void>   maSelectHdl;
    Link<Calendar*,void>   maActivateHdl;

    using Control::ImplInitSettings;
    using Window::ImplInit;
    VCL_DLLPRIVATE void         ImplInit( WinBits nWinStyle );
    VCL_DLLPRIVATE void         ImplInitSettings();

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

    VCL_DLLPRIVATE void         ImplFormat();
    using Window::ImplHitTest;
    VCL_DLLPRIVATE sal_uInt16   ImplHitTest( const Point& rPos, Date& rDate ) const;
    VCL_DLLPRIVATE void         ImplDrawSpin(vcl::RenderContext& rRenderContext);
    VCL_DLLPRIVATE void         ImplDrawDate(vcl::RenderContext& rRenderContext, long nX, long nY,
                                             sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear,
                                             bool bOther, sal_Int32 nToday);
    VCL_DLLPRIVATE void         ImplDraw(vcl::RenderContext& rRenderContext);
    VCL_DLLPRIVATE void         ImplUpdateDate( const Date& rDate );
    VCL_DLLPRIVATE void         ImplUpdateSelection( IntDateSet* pOld );
    VCL_DLLPRIVATE void         ImplMouseSelect( const Date& rDate, sal_uInt16 nHitTest );
    VCL_DLLPRIVATE void         ImplUpdate( bool bCalcNew = false );
    using Window::ImplScroll;
    VCL_DLLPRIVATE void         ImplScroll( bool bPrev );
    VCL_DLLPRIVATE void         ImplShowMenu( const Point& rPos, const Date& rDate );
    VCL_DLLPRIVATE void         ImplTracking( const Point& rPos, bool bRepeat );
    VCL_DLLPRIVATE void         ImplEndTracking( bool bCancel );
    VCL_DLLPRIVATE DayOfWeek    ImplGetWeekStart() const;

    virtual Size GetOptimalSize() const override;
public:
                    Calendar( vcl::Window* pParent, WinBits nWinStyle );
    virtual         ~Calendar() override;
    virtual void    dispose() override;

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    Tracking( const TrackingEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    Resize() override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void            Select();

    Date            GetFirstSelectedDate() const;
    void            EnableCallEverySelect() { mbAllSel = true; }

    void            SetCurDate( const Date& rNewDate );
    void            SetFirstDate( const Date& rNewFirstDate );
    const Date&     GetFirstDate() const { return maFirstDate; }
    Date            GetLastDate() const { return GetFirstDate() + mnDayCount; }
    Date            GetFirstMonth() const;
    Date            GetLastMonth() const;
    sal_uInt16      GetMonthCount() const;
    bool            GetDate( const Point& rPos, Date& rDate ) const;
    tools::Rectangle       GetDateRect( const Date& rDate ) const;

    void            StartSelection();
    void            EndSelection();

    bool            IsTravelSelect() const { return mbTravelSelect; }

    Size            CalcWindowSizePixel() const;

    void            SetSelectHdl( const Link<Calendar*,void>& rLink ) { maSelectHdl = rLink; }
    void            SetActivateHdl( const Link<Calendar*,void>& rLink ) { maActivateHdl = rLink; }
};

#endif // INCLUDED_VCL_CALENDAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
