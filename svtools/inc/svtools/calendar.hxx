/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CALENDAR_HXX
#define _CALENDAR_HXX

#include "svtools/svtdllapi.h"
#include <unotools/calendarwrapper.hxx>
#include <com/sun/star/i18n/Weekdays.hpp>

#include <vcl/ctrl.hxx>
#include <vcl/timer.hxx>
#include <vcl/field.hxx>

class Table;
class MouseEvent;
class TrackingEvent;
class KeyEvent;
class HelpEvent;
class DataChangedEvent;
class FloatingWindow;
class PushButton;
struct ImplDateInfo;
class ImplDateTable;
class ImplCFieldFloatWin;

/*************************************************************************

Beschreibung
============

class Calendar

Diese Klasse erlaubt die Auswahl eines Datum. Der Datumsbereich der
angezeigt wird, ist der, der durch die Klasse Date vorgegeben ist.
Es werden soviele Monate angezeigt, wie die Ausgabeflaeche des
Controls vorgibt. Der Anwender kann zwischen den Monaten ueber ein
ContextMenu (Bei Click auf den Monatstitel) oder durch 2 ScrollButtons
zwischen den Monaten wechseln.

--------------------------------------------------------------------------

WinBits

WB_BORDER                   Um das Fenster wird ein Border gezeichnet.
WB_TABSTOP                  Tastatursteuerung ist moeglich. Der Focus wird
                            sich geholt, wenn mit der Maus in das
                            Control geklickt wird.
WB_QUICKHELPSHOWSDATEINFO   DateInfo auch bei QuickInfo als BalloonHelp zeigen
WB_BOLDTEXT                 Formatiert wird nach fetten Texten und
                            DIB_BOLD wird bei AddDateInfo() ausgewertet
WB_FRAMEINFO                Formatiert wird so, das Frame-Info angezeigt
                            werden kann und die FrameColor bei AddDateInfo()
                            ausgewertet wird
WB_RANGESELECT              Es koennen mehrere Tage selektiert werden, die
                            jedoch alle zusammenhaengend sein muessen
WB_MULTISELECT              Es koennen mehrere Tage selektiert werden
WB_WEEKNUMBER               Es werden die Wochentage mit angezeigt

--------------------------------------------------------------------------

Mit SetCurDate() / GetCurDate() wird das ausgewaehlte Datum gesetzt und
abgefragt. Wenn der Anwnder ein Datum selektiert hat, wird Select()
gerufen. Bei einem Doppelklick auf ein Datum wird DoubleClick() gerufen.

--------------------------------------------------------------------------

Mit CalcWindowSizePixel() kann die Groesse des Fensters in Pixel fuer
die Darstellung einer bestimmte Anzahl von Monaten berechnet werden.

--------------------------------------------------------------------------

Mit SetSaturdayColor() kann eine spezielle Farbe fuer Sonnabende gesetzt
werden und mit SetSundayColor() eine fuer Sonntage. Mit AddDateInfo()
koennen Tage speziell gekennzeichnet werden. Dabei kann man einem
einzelnen Datum eine andere Farbe geben (zum Beispiel fuer Feiertage)
oder diese Umranden (zum Beispiel fuer Termine). Wenn beim Datum
kein Jahr angegeben wird, wird der Tag in jedem Jahr benutzt. Mit
AddDateInfo() kann auch jedem Datum ein Text mitgegeben werden, der
dann angezeigt wird, wenn Balloon-Hilfe an ist. Um nicht alle Jahre
mit entsprechenden Daten zu versorgen, wird der RequestDateInfo()-
Handler gerufen, wenn ein neues Jahr angezeigt wird. Es kann dann
im Handler mit GetRequestYear() das Jahr abgefragt werden.

--------------------------------------------------------------------------

Um ein ContextMenu zu einem Datum anzuzeigen, muss man den Command-Handler
ueberlagern. Mit GetDate() kann zur Mouse-Position das Datum ermittelt
werden. Bei Tastaturausloesung sollte das aktuelle Datum genommen werden.
Wenn ein ContextMenu angezeigt wird, darf der Handler der Basisklasse nicht
gerufen werden.

--------------------------------------------------------------------------

Bei Mehrfachselektion WB_RANGESELECT oder WB_MULTISELECT kann mit
SelectDate()/SelectDateRange() Datumsbereiche selektiert/deselektiert
werden. SelectDateRange() gilt inkl. EndDatum. Mit SetNoSelection() kann
alles deselektiert werden. SetCurDate() selektiert bei Mehrfachselektion
jedoch nicht das Datum mit, sondern gibt nur das Focus-Rechteck vor.

Den selektierten Bereich kann man mit GetSelectDateCount()/GetSelectDate()
abgefragt werden oder der Status von einem Datum kann mit IsDateSelected()
abgefragt werden.

Waehrend der Anwender am selektieren ist, wird der SelectionChanging()-
Handler gerufen. In diesem kann der selektierte Bereich angepasst werden,
wenn man beispielsweise den Bereich eingrenzen oder erweitern will. Der
Bereich wird mit SelectDate()/SelectDateRange() umgesetzt und mit
GetSelectDateCount()/GetSelectDate() abgefragt. Wenn man wissen moechte,
in welche Richtung selektiert wird, kann dies ueber IsSelectLeft()
abgefragt werden. sal_True bedeutet eine Selektion nach links oder oben,
sal_False eine Selektion nach rechts oder unten.

--------------------------------------------------------------------------

Wenn sich der Date-Range-Bereich anpasst und man dort die Selektion
uebernehmen will, sollte dies nur gemacht werden, wenn
IsScrollDateRangeChanged() sal_True zurueckliefert. Denn diese Methode liefert
sal_True zurueck, wenn der Bereich durch Betaetigung von den Scroll-Buttons
ausgeloest wurde. Bei sal_False wurde dies durch Resize(), Methoden-Aufrufen
oder durch Beendigung einer Selektion ausgeloest.

*************************************************************************/

// ------------------
// - Calendar-Types -
// ------------------

#define WB_QUICKHELPSHOWSDATEINFO   ((WinBits)0x00004000)
#define WB_BOLDTEXT                 ((WinBits)0x00008000)
#define WB_FRAMEINFO                ((WinBits)0x00010000)
#define WB_WEEKNUMBER               ((WinBits)0x00020000)
// Muss mit den WinBits beim TabBar uebereinstimmen oder mal
// nach \vcl\inc\wintypes.hxx verlagert werden
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

class SVT_DLLPUBLIC Calendar : public Control
{
private:
    ImplDateTable*  mpDateTable;
    Table*          mpSelectTable;
    Table*          mpOldSelectTable;
    Table*          mpRestoreSelectTable;
    XubString*      mpDayText[31];
    XubString       maDayText;
    XubString       maWeekText;
    CalendarWrapper maCalendarWrapper;
    Rectangle       maPrevRect;
    Rectangle       maNextRect;
    String          maDayOfWeekText;
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
    long            mnDummy2;
    long            mnDummy3;
    long            mnDummy4;
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

#ifdef _SV_CALENDAR_CXX
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
    SVT_DLLPRIVATE void         ImplUpdateSelection( Table* pOld );
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
#endif

protected:
    sal_Bool            ShowDropPos( const Point& rPos, Date& rDate );
    void            HideDropPos();

    DECL_STATIC_LINK( Calendar, ScrollHdl, Timer *);

public:
                    Calendar( Window* pParent, WinBits nWinStyle = 0 );
                    Calendar( Window* pParent, const ResId& rResId );
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

    /// Set one of ::com::sun::star::i18n::Weekdays.
    void            SetWeekStart( sal_Int16 nDay );

    /// Set how many days of a week must reside in the first week of a year.
    void            SetMinimumNumberOfDaysInWeek( sal_Int16 nDays );

    void            SelectDate( const Date& rDate, sal_Bool bSelect = sal_True );
    void            SelectDateRange( const Date& rStartDate, const Date& rEndDate,
                                     sal_Bool bSelect = sal_True );
    void            SetNoSelection();
    sal_Bool            IsDateSelected( const Date& rDate ) const;
    sal_uLong           GetSelectDateCount() const;
    Date            GetSelectDate( sal_uLong nIndex = 0 ) const;
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
    sal_Bool            GetDropDate( Date& rDate ) const;

    long            GetCurMonthPerLine() const { return mnMonthPerLine; }
    long            GetCurLines() const { return mnLines; }

    void            SetStandardColor( const Color& rColor );
    const Color&    GetStandardColor() const;
    void            SetSaturdayColor( const Color& rColor );
    const Color&    GetSaturdayColor() const;
    void            SetSundayColor( const Color& rColor );
    const Color&    GetSundayColor() const;

    void            AddDateInfo( const Date& rDate, const XubString& rText,
                                 const Color* pTextColor = NULL,
                                 const Color* pFrameColor = NULL,
                                 sal_uInt16 nFlags = 0 );
    void            RemoveDateInfo( const Date& rDate );
    void            ClearDateInfo();
    XubString       GetDateInfoText( const Date& rDate );

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

Beschreibung
============

class CalendarField

Bei dieser Klasse handelt es sich um ein DateField, wo ueber einen
DropDown-Button ueber das Calendar-Control ein Datum ausgewaehlt werden
kann.

--------------------------------------------------------------------------

WinBits

Siehe DateField

Die Vorgaben fuer das CalendarControl koennen ueber SetCalendarStyle()
gesetzt werden.

--------------------------------------------------------------------------

Mit EnableToday()/EnableNone() kann ein Today-Button und ein None-Button
enabled werden.

--------------------------------------------------------------------------

Wenn mit SetCalendarStyle() WB_RANGESELECT gesetzt wird, koennen im
Calendar auch mehrere Tage selektiert werden. Da immer nur das Start-Datum
in das Feld uebernommen wird, sollte dann im Select-Handler mit
GetCalendar() der Calendar abgefragt werden und an dem mit
GetSelectDateCount()/GetSelectDate() der selektierte Bereich abgefragt
werden, um beispielsweise diese dann in ein weiteres Feld zu uebernehmen.

--------------------------------------------------------------------------

Wenn ein abgeleiteter Calendar verwendet werden soll, kann am
CalendarField die Methode CreateCalendar() ueberlagert werden und
dort ein eigener Calendar erzeugt werden.

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

#ifdef _SV_CALENDAR_CXX
                        DECL_DLLPRIVATE_LINK( ImplSelectHdl, Calendar* );
                        DECL_DLLPRIVATE_LINK( ImplClickHdl, PushButton* );
                        DECL_DLLPRIVATE_LINK( ImplPopupModeEndHdl, FloatingWindow* );
#endif

public:
                        CalendarField( Window* pParent, WinBits nWinStyle );
                        CalendarField( Window* pParent, const ResId& rResId );
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
