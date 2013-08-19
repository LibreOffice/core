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

#ifndef _CALENDAR_HXX
#define _CALENDAR_HXX

#include "svtools/svtdllapi.h"
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

// ------------------
// - Calendar types -
// ------------------

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

// ------------
// - Calendar -
// ------------

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
    sal_Int32       mnDayOfWeekAry[7];
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
    sal_uLong           mnDayCount;
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
    sal_uInt16          mnFirstYear;
    sal_uInt16          mnLastYear;
    sal_uInt16          mnRequestYear;
    sal_Bool            mbCalc:1,
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
    Link            maSelectionChangingHdl;
    Link            maDateRangeChangedHdl;
    Link            maRequestDateInfoHdl;
    Link            maDoubleClickHdl;
    Link            maSelectHdl;
    Timer           maDragScrollTimer;
    sal_uInt16          mnDragScrollHitTest;

    using Control::ImplInitSettings;
    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit( WinBits nWinStyle );
    SVT_DLLPRIVATE void         ImplInitSettings();
    SVT_DLLPRIVATE void         ImplGetWeekFont( Font& rFont ) const;
    SVT_DLLPRIVATE void         ImplFormat();
    using Window::ImplHitTest;
    SVT_DLLPRIVATE sal_uInt16           ImplHitTest( const Point& rPos, Date& rDate ) const;
    SVT_DLLPRIVATE void         ImplDrawSpin( sal_Bool bDrawPrev = sal_True, sal_Bool bDrawNext = sal_True );
    SVT_DLLPRIVATE void         ImplDrawDate( long nX, long nY,
                                  sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear,
                                  DayOfWeek eDayOfWeek,
                                  sal_Bool bBack = sal_True, sal_Bool bOther = sal_False,
                                  sal_uLong nToday = 0 );
    SVT_DLLPRIVATE void         ImplDraw( sal_Bool bPaint = sal_False );
    SVT_DLLPRIVATE void         ImplUpdateDate( const Date& rDate );
    SVT_DLLPRIVATE void         ImplUpdateSelection( IntDateSet* pOld );
    SVT_DLLPRIVATE void         ImplMouseSelect( const Date& rDate, sal_uInt16 nHitTest,
                                     sal_Bool bMove, sal_Bool bExpand, sal_Bool bExtended );
    SVT_DLLPRIVATE void         ImplUpdate( sal_Bool bCalcNew = sal_False );
    using Window::ImplScroll;
    SVT_DLLPRIVATE void         ImplScroll( sal_Bool bPrev );
    SVT_DLLPRIVATE void         ImplInvertDropPos();
    SVT_DLLPRIVATE void         ImplShowMenu( const Point& rPos, const Date& rDate );
    SVT_DLLPRIVATE void         ImplTracking( const Point& rPos, sal_Bool bRepeat );
    SVT_DLLPRIVATE void         ImplEndTracking( sal_Bool bCancel );
    SVT_DLLPRIVATE DayOfWeek    ImplGetWeekStart() const;

protected:

    DECL_STATIC_LINK( Calendar, ScrollHdl, Timer *);

public:
                    Calendar( Window* pParent, WinBits nWinStyle = 0 );
                    ~Calendar();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    SelectionChanging();
    virtual void    DateRangeChanged();
    virtual void    RequestDateInfo();
    virtual void    DoubleClick();
    virtual void    Select();

    const CalendarWrapper& GetCalendarWrapper() const { return maCalendarWrapper; }

    void            SelectDate( const Date& rDate, sal_Bool bSelect = sal_True );
    void            SetNoSelection();
    sal_Bool            IsDateSelected( const Date& rDate ) const;
    Date            GetFirstSelectedDate() const;
    void            EnableCallEverySelect( sal_Bool bEvery = sal_True ) { mbAllSel = bEvery; }
    sal_Bool            IsCallEverySelectEnabled() const { return mbAllSel; }

    sal_uInt16          GetRequestYear() const { return mnRequestYear; }
    void            SetCurDate( const Date& rNewDate );
    Date            GetCurDate() const { return maCurDate; }
    void            SetFirstDate( const Date& rNewFirstDate );
    Date            GetFirstDate() const { return maFirstDate; }
    Date            GetLastDate() const { return GetFirstDate() + mnDayCount; }
    sal_uLong           GetDayCount() const { return mnDayCount; }
    Date            GetFirstMonth() const;
    Date            GetLastMonth() const;
    sal_uInt16          GetMonthCount() const;
    sal_Bool            GetDate( const Point& rPos, Date& rDate ) const;
    Rectangle       GetDateRect( const Date& rDate ) const;

    long            GetCurMonthPerLine() const { return mnMonthPerLine; }
    long            GetCurLines() const { return mnLines; }

    const Color&    GetStandardColor() const;
    const Color&    GetSaturdayColor() const;
    const Color&    GetSundayColor() const;

    void            StartSelection();
    void            EndSelection();

    sal_Bool            IsTravelSelect() const { return mbTravelSelect; }
    sal_Bool            IsScrollDateRangeChanged() const { return mbScrollDateRange; }
    sal_Bool            IsSelectLeft() const { return mbSelLeft; }

    Size            CalcWindowSizePixel( long nCalcMonthPerLine = 1,
                                         long nCalcLines = 1 ) const;

    void            SetSelectionChangingHdl( const Link& rLink ) { maSelectionChangingHdl = rLink; }
    const Link&     GetSelectionChangingHdl() const { return maSelectionChangingHdl; }
    void            SetDateRangeChangedHdl( const Link& rLink ) { maDateRangeChangedHdl = rLink; }
    const Link&     GetDateRangeChangedHdl() const { return maDateRangeChangedHdl; }
    void            SetRequestDateInfoHdl( const Link& rLink ) { maRequestDateInfoHdl = rLink; }
    const Link&     GetRequestDateInfoHdl() const { return maRequestDateInfoHdl; }
    void            SetDoubleClickHdl( const Link& rLink ) { maDoubleClickHdl = rLink; }
    const Link&     GetDoubleClickHdl() const { return maDoubleClickHdl; }
    void            SetSelectHdl( const Link& rLink ) { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const { return maSelectHdl; }
};

inline const Color& Calendar::GetStandardColor() const
{
    if ( mpStandardColor )
        return *mpStandardColor;
    else
        return GetFont().GetColor();
}

inline const Color& Calendar::GetSaturdayColor() const
{
    if ( mpSaturdayColor )
        return *mpSaturdayColor;
    else
        return GetFont().GetColor();
}

inline const Color& Calendar::GetSundayColor() const
{
    if ( mpSundayColor )
        return *mpSundayColor;
    else
        return GetFont().GetColor();
}

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

// -----------------
// - CalendarField -
// -----------------

class SVT_DLLPUBLIC CalendarField : public DateField
{
private:
    ImplCFieldFloatWin* mpFloatWin;
    Calendar*           mpCalendar;
    WinBits             mnCalendarStyle;
    PushButton*         mpTodayBtn;
    PushButton*         mpNoneBtn;
    Date                maDefaultDate;
    sal_Bool                mbToday;
    sal_Bool                mbNone;
    Link                maSelectHdl;

                        DECL_DLLPRIVATE_LINK( ImplSelectHdl, Calendar* );
                        DECL_DLLPRIVATE_LINK( ImplClickHdl, PushButton* );
                        DECL_DLLPRIVATE_LINK( ImplPopupModeEndHdl, void* );

public:
                        CalendarField( Window* pParent, WinBits nWinStyle );
                        ~CalendarField();

    virtual void        Select();

    virtual sal_Bool        ShowDropDown( sal_Bool bShow );
    virtual Calendar*   CreateCalendar( Window* pParent );
    Calendar*           GetCalendar();

    void                SetDefaultDate( const Date& rDate ) { maDefaultDate = rDate; }
    Date                GetDefaultDate() const { return maDefaultDate; }

    void                EnableToday( sal_Bool bToday = sal_True ) { mbToday = bToday; }
    sal_Bool                IsTodayEnabled() const { return mbToday; }
    void                EnableNone( sal_Bool bNone = sal_True ) { mbNone = bNone; }
    sal_Bool                IsNoneEnabled() const { return mbNone; }

    void                SetCalendarStyle( WinBits nStyle ) { mnCalendarStyle = nStyle; }
    WinBits             GetCalendarStyle() const { return mnCalendarStyle; }

    void                SetSelectHdl( const Link& rLink ) { maSelectHdl = rLink; }
    const Link&         GetSelectHdl() const { return maSelectHdl; }

protected:
    virtual void    StateChanged( StateChangedType nStateChange );
};

#endif  // _CALENDAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
