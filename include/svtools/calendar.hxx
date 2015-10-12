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

#ifndef INCLUDED_SVTOOLS_CALENDAR_HXX
#define INCLUDED_SVTOOLS_CALENDAR_HXX

#include <svtools/svtdllapi.h>
#include <unotools/calendarwrapper.hxx>
#include <com/sun/star/i18n/Weekdays.hpp>

#include <vcl/ctrl.hxx>
#include <vcl/timer.hxx>
#include <vcl/field.hxx>
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
WB_QUICKHELPSHOWSDATEINFO   Show DateInfo as BallonHelp even if QuickInfo is enabled
WB_BOLDTEXT                 We format by bold texts and DIB_BOLD is evaluated by
                            AddDateInfo()
WB_FRAMEINFO                We format in a way, so that FrameInfo can be displayed
                            and the FrameColor is evaluated by AddDateInfo()
WB_RANGESELECT              The user can select multiple days, which need to be
                            consecutive
WB_MULTISELECT              The user can select multiple days
WB_WEEKNUMBER               We also display the weekdays

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

For multiple selection (WB_RANGESELECT or WB_MULTISELECT) SelectDate(),
SelectDateRange() can select date ranges. SelectDateRange() selects
including the end date.
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
should only do this is if IsScrollDateRangeChanged() retruns sal_True.
This method returns sal_True if the area change was triggered by using the
ScrollButtons and sal_False if it was triggered by Resize(), other method
calls or by ending a selection.

*************************************************************************/


// - Calendar types -


#define WB_QUICKHELPSHOWSDATEINFO   ((WinBits)0x00004000)
#define WB_BOLDTEXT                 ((WinBits)0x00008000)
#define WB_FRAMEINFO                ((WinBits)0x00010000)
#define WB_WEEKNUMBER               ((WinBits)0x00020000)
// Needs to in agreement with the WinBits in the TabBar or
// we move it to \vcl\inc\wintypes.hxx
#ifndef WB_RANGESELECT
#define WB_RANGESELECT              ((WinBits)0x00200000)
#endif
#ifndef WB_MULTISELECT
#define WB_MULTISELECT              ((WinBits)0x00400000)
#endif

#define DIB_BOLD                    ((sal_uInt16)0x0001)


// - Calendar -


typedef std::set<sal_uInt32> IntDateSet;


class SVT_DLLPUBLIC Calendar : public Control
{
private:
    IntDateSet*     mpSelectTable;
    IntDateSet*     mpOldSelectTable;
    IntDateSet*     mpRestoreSelectTable;
    OUString        maDayTexts[31];
    OUString        maDayText;
    OUString        maWeekText;
    CalendarWrapper maCalendarWrapper;
    Rectangle       maPrevRect;
    Rectangle       maNextRect;
    OUString        maDayOfWeekText;
    long            mnDayOfWeekAry[7];
    Date            maOldFormatFirstDate;
    Date            maOldFormatLastDate;
    Date            maFirstDate;
    Date            maOldFirstDate;
    Date            maCurDate;
    Date            maOldCurDate;
    Date            maAnchorDate;
    Date            maDropDate;
    Color           maSelColor;
    Color           maOtherColor;
    Color*          mpStandardColor;
    Color*          mpSaturdayColor;
    Color*          mpSundayColor;
    sal_uLong       mnDayCount;
    long            mnDaysOffX;
    long            mnWeekDayOffY;
    long            mnDaysOffY;
    long            mnMonthHeight;
    long            mnMonthWidth;
    long            mnMonthPerLine;
    long            mnLines;
    long            mnDayWidth;
    long            mnDayHeight;
    long            mnWeekWidth;
    WinBits         mnWinStyle;
    sal_uInt16      mnFirstYear;
    sal_uInt16      mnLastYear;
    sal_uInt16      mnRequestYear;
    bool            mbCalc:1,
                    mbFormat:1,
                    mbDrag:1,
                    mbSelection:1,
                    mbMultiSelection:1,
                    mbWeekSel:1,
                    mbUnSel:1,
                    mbMenuDown:1,
                    mbSpinDown:1,
                    mbPrevIn:1,
                    mbNextIn:1,
                    mbDirect:1,
                    mbInSelChange:1,
                    mbTravelSelect:1,
                    mbScrollDateRange:1,
                    mbSelLeft:1,
                    mbAllSel:1,
                    mbDropPos:1;
    Link<Calendar*,void>   maSelectHdl;
    Timer                  maDragScrollTimer;
    sal_uInt16             mnDragScrollHitTest;

    using Control::ImplInitSettings;
    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit( WinBits nWinStyle );
    SVT_DLLPRIVATE void         ImplInitSettings();

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

    SVT_DLLPRIVATE void         ImplGetWeekFont( vcl::Font& rFont ) const;
    SVT_DLLPRIVATE void         ImplFormat();
    using Window::ImplHitTest;
    SVT_DLLPRIVATE sal_uInt16           ImplHitTest( const Point& rPos, Date& rDate ) const;
    SVT_DLLPRIVATE void         ImplDrawSpin(vcl::RenderContext& rRenderContext, bool bDrawPrev = true, bool bDrawNext = true);
    SVT_DLLPRIVATE void         ImplDrawDate(vcl::RenderContext& rRenderContext, long nX, long nY,
                                             sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear,
                                             DayOfWeek eDayOfWeek, bool bBack = true,
                                             bool bOther = false, sal_uLong nToday = 0);
    SVT_DLLPRIVATE void         ImplDraw(vcl::RenderContext& rRenderContext, bool bPaint = false);
    SVT_DLLPRIVATE void         ImplUpdateDate( const Date& rDate );
    SVT_DLLPRIVATE void         ImplUpdateSelection( IntDateSet* pOld );
    SVT_DLLPRIVATE void         ImplMouseSelect( const Date& rDate, sal_uInt16 nHitTest,
                                     bool bMove, bool bExpand, bool bExtended );
    SVT_DLLPRIVATE void         ImplUpdate( bool bCalcNew = false );
    using Window::ImplScroll;
    SVT_DLLPRIVATE void         ImplScroll( bool bPrev );
    SVT_DLLPRIVATE void         ImplInvertDropPos();
    SVT_DLLPRIVATE void         ImplShowMenu( const Point& rPos, const Date& rDate );
    SVT_DLLPRIVATE void         ImplTracking( const Point& rPos, bool bRepeat );
    SVT_DLLPRIVATE void         ImplEndTracking( bool bCancel );
    SVT_DLLPRIVATE DayOfWeek    ImplGetWeekStart() const;

protected:

    DECL_LINK_TYPED( ScrollHdl, Timer *, void );

public:
                    Calendar( vcl::Window* pParent, WinBits nWinStyle = 0 );
    virtual         ~Calendar();
    virtual void    dispose() override;

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    Tracking( const TrackingEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void    Resize() override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void            Select();

    void            SelectDate( const Date& rDate, bool bSelect = true );
    void            SetNoSelection();
    bool            IsDateSelected( const Date& rDate ) const;
    Date            GetFirstSelectedDate() const;
    void            EnableCallEverySelect( bool bEvery = true ) { mbAllSel = bEvery; }

    void            SetCurDate( const Date& rNewDate );
    void            SetFirstDate( const Date& rNewFirstDate );
    Date            GetFirstDate() const { return maFirstDate; }
    Date            GetLastDate() const { return GetFirstDate() + mnDayCount; }
    Date            GetFirstMonth() const;
    Date            GetLastMonth() const;
    sal_uInt16      GetMonthCount() const;
    bool            GetDate( const Point& rPos, Date& rDate ) const;
    Rectangle       GetDateRect( const Date& rDate ) const;

    void            StartSelection();
    void            EndSelection();

    bool            IsTravelSelect() const { return mbTravelSelect; }

    Size            CalcWindowSizePixel( long nCalcMonthPerLine = 1,
                                         long nCalcLines = 1 ) const;

    void            SetSelectHdl( const Link<Calendar*,void>& rLink ) { maSelectHdl = rLink; }
};


/*************************************************************************

Description
============

class CalendarField

This class is a DateField with which one can select a date via a DropDownButton
and the CalendarControl.

--------------------------------------------------------------------------

WinBits

See DateField

The preferences for the CalendarControl can be set via SetCalendarStyle().

--------------------------------------------------------------------------

With EnableToday()/EnableNone() we can enable a TodayButton and a NoneButton.

--------------------------------------------------------------------------

If we set WB_RANGESELECT with SetCalendarStyle(), we can select multiple days
in the Calendar.

Because we only take over the start date into the field, we should query
with GetCalendar() in the SelectHandler and with GetSelectDateCount()/GetSelectDate()
the selected range. We then can e.g. take over that value to another field.

--------------------------------------------------------------------------

If a derived Calendar should be used, we can override the CreateCalendar()
method in CalendarField and create an own calendar there ourselves.

*************************************************************************/


// - CalendarField -


class SVT_DLLPUBLIC CalendarField : public DateField
{
private:
    VclPtr<ImplCFieldFloatWin> mpFloatWin;
    VclPtr<Calendar>    mpCalendar;
    WinBits             mnCalendarStyle;
    VclPtr<PushButton>  mpTodayBtn;
    VclPtr<PushButton>  mpNoneBtn;
    Date                maDefaultDate;
    bool                mbToday;
    bool                mbNone;

                        DECL_DLLPRIVATE_LINK_TYPED( ImplSelectHdl, Calendar*, void );
                        DECL_DLLPRIVATE_LINK_TYPED( ImplClickHdl, Button*, void );
                        DECL_DLLPRIVATE_LINK_TYPED( ImplPopupModeEndHdl, FloatingWindow*, void );

public:
                        CalendarField( vcl::Window* pParent, WinBits nWinStyle );
    virtual             ~CalendarField();
    virtual void        dispose() override;

    virtual bool        ShowDropDown( bool bShow ) override;
    VclPtr<Calendar>    CreateCalendar( vcl::Window* pParent );
    Calendar*           GetCalendar();

    void                EnableToday( bool bToday = true ) { mbToday = bToday; }
    void                EnableNone( bool bNone = true ) { mbNone = bNone; }

protected:
    virtual void    StateChanged( StateChangedType nStateChange ) override;
};

#endif // INCLUDED_SVTOOLS_CALENDAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
