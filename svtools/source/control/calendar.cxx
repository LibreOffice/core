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
#include <comphelper/processfactory.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/i18n/Weekdays.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>

#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/calendar.hxx>
#include <memory>

#define DAY_OFFX                        4
#define DAY_OFFY                        2
#define MONTH_BORDERX                   4
#define MONTH_OFFY                      3
#define WEEKNUMBER_OFFX                 4
#define WEEKDAY_OFFY                    3
#define TITLE_OFFY                      3
#define TITLE_BORDERY                   2
#define SPIN_OFFX                       4
#define SPIN_OFFY                       TITLE_BORDERY

#define WEEKNUMBER_HEIGHT               85

#define CALENDAR_HITTEST_DAY            ((sal_uInt16)0x0001)
#define CALENDAR_HITTEST_MONTHTITLE     ((sal_uInt16)0x0004)
#define CALENDAR_HITTEST_PREV           ((sal_uInt16)0x0008)
#define CALENDAR_HITTEST_NEXT           ((sal_uInt16)0x0010)

#define MENU_YEAR_COUNT                 3

using namespace ::com::sun::star;

static void ImplCalendarSelectDate( IntDateSet* pTable, const Date& rDate, bool bSelect )
{
    if ( bSelect )
        pTable->insert( rDate.GetDate() );
    else
        pTable->erase( rDate.GetDate() );
}

static void ImplCalendarSelectDateRange( IntDateSet* pTable,
                                         const Date& rStartDate,
                                         const Date& rEndDate,
                                         bool bSelect )
{
    Date aStartDate = rStartDate;
    Date aEndDate = rEndDate;
    if ( aStartDate > aEndDate )
    {
        Date aTempDate = aStartDate;
        aStartDate = aEndDate;
        aEndDate = aTempDate;
    }

    if ( bSelect )
    {
        while ( aStartDate <= aEndDate )
        {
            pTable->insert( aStartDate.GetDate() );
            ++aStartDate;
        }
    }
    else
    {
        for ( IntDateSet::const_iterator it = pTable->begin(); it != pTable->end(); )
        {
            Date aDate( *it );
            if ( aDate > aEndDate )
                break;

            if ( aDate >= aStartDate )
                it = pTable->erase(it);
            else
                ++it;
        }
    }
}

static void ImplCalendarUnSelectDateRange( IntDateSet* pTable,
                                           IntDateSet* pOldTable,
                                           const Date& rStartDate,
                                           const Date& rEndDate )
{
    Date aStartDate = rStartDate;
    Date aEndDate = rEndDate;
    if ( aStartDate > aEndDate )
    {
        Date aTempDate = aStartDate;
        aStartDate = aEndDate;
        aEndDate = aTempDate;
    }

    for ( IntDateSet::const_iterator it = pTable->begin(); it != pTable->end(); )
    {
        Date aDate( *it );
        if ( aDate > aEndDate )
            break;

        if ( aDate >= aStartDate )
            it = pTable->erase(it);
        else
            ++it;
    }

    for ( IntDateSet::const_iterator it = pOldTable->begin(); it != pOldTable->end(); ++it )
    {
        Date aDate( *it );
        if ( aDate > aEndDate )
            break;
        if ( aDate >= aStartDate )
            pTable->insert( aDate.GetDate() );
    }
}

inline void ImplCalendarClearSelectDate( IntDateSet* pTable )
{
    pTable->clear();
}

void Calendar::ImplInit( WinBits nWinStyle )
{
    mpSelectTable           = new IntDateSet;
    mpOldSelectTable        = nullptr;
    mpRestoreSelectTable    = nullptr;
    mpStandardColor         = nullptr;
    mpSaturdayColor         = nullptr;
    mpSundayColor           = nullptr;
    mnDayCount              = 0;
    mnWinStyle              = nWinStyle;
    mnFirstYear             = 0;
    mnLastYear              = 0;
    mbCalc                  = true;
    mbFormat                = true;
    mbDrag                  = false;
    mbSelection             = false;
    mbMultiSelection        = false;
    mbUnSel                 = false;
    mbMenuDown              = false;
    mbSpinDown              = false;
    mbPrevIn                = false;
    mbNextIn                = false;
    mbTravelSelect          = false;
    mbSelLeft               = false;
    mbAllSel                = false;
    mbDropPos               = false;

    OUString aGregorian( "gregorian");
    maCalendarWrapper.loadCalendar( aGregorian,
            Application::GetAppLocaleDataWrapper().getLanguageTag().getLocale());
    if (maCalendarWrapper.getUniqueID() != aGregorian)
    {
        SAL_WARN( "svtools.control", "Calendar::ImplInit: No ``gregorian'' calendar available for locale ``"
            << Application::GetAppLocaleDataWrapper().getLanguageTag().getBcp47()
            << "'' and other calendars aren't supported. Using en-US fallback." );

        /* If we ever wanted to support other calendars than Gregorian a lot of
         * rewrite would be necessary to internally replace use of class Date
         * with proper class CalendarWrapper methods, get rid of fixed 12
         * months, fixed 7 days, ... */
        maCalendarWrapper.loadCalendar( aGregorian, lang::Locale( "en", "US", ""));
    }

    SetFirstDate( maCurDate );
    ImplCalendarSelectDate( mpSelectTable, maCurDate, true );

    // Sonstige Strings erzeugen
    maDayText = SvtResId(STR_SVT_CALENDAR_DAY);
    maWeekText = SvtResId(STR_SVT_CALENDAR_WEEK);

    // Tagestexte anlegen
    for (sal_Int32 i = 0; i < 31; ++i)
        maDayTexts[i] = OUString::number(i+1);

    maDragScrollTimer.SetInvokeHandler( LINK( this, Calendar, ScrollHdl ) );
    maDragScrollTimer.SetTimeout( GetSettings().GetMouseSettings().GetScrollRepeat() );
    mnDragScrollHitTest = 0;

    ImplInitSettings();
}

void Calendar::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    maSelColor = rStyleSettings.GetHighlightTextColor();
    SetPointFont(rRenderContext, rStyleSettings.GetToolFont());
    rRenderContext.SetTextColor(rStyleSettings.GetFieldTextColor());
    rRenderContext.SetBackground(Wallpaper(rStyleSettings.GetFieldColor()));
}

void Calendar::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    maSelColor = rStyleSettings.GetHighlightTextColor();
    SetPointFont(*this, rStyleSettings.GetToolFont());
    SetTextColor(rStyleSettings.GetFieldTextColor());
    SetBackground(Wallpaper(rStyleSettings.GetFieldColor()));
}

Calendar::Calendar( vcl::Window* pParent, WinBits nWinStyle ) :
    Control( pParent, nWinStyle & (WB_TABSTOP | WB_GROUP | WB_BORDER | WB_3DLOOK | WB_RANGESELECT | WB_MULTISELECT) ),
    maCalendarWrapper( Application::GetAppLocaleDataWrapper().getComponentContext() ),
    maOldFormatFirstDate( 0, 0, 1900 ),
    maOldFormatLastDate( 0, 0, 1900 ),
    maFirstDate( 0, 0, 1900 ),
    maOldFirstDate( 0, 0, 1900 ),
    maCurDate( Date::SYSTEM ),
    maOldCurDate( 0, 0, 1900 ),
    maAnchorDate( maCurDate ),
    maDropDate( 0, 0, 1900 )
{
    ImplInit( nWinStyle );
}

Calendar::~Calendar()
{
    disposeOnce();
}

void Calendar::dispose()
{
    delete mpStandardColor;
    delete mpSaturdayColor;
    delete mpSundayColor;

    delete mpSelectTable;
    delete mpOldSelectTable;
    delete mpRestoreSelectTable;
    Control::dispose();
}

DayOfWeek Calendar::ImplGetWeekStart() const
{
    // Map i18n::Weekdays to Date DayOfWeek
    DayOfWeek eDay;
    sal_Int16 nDay = maCalendarWrapper.getFirstDayOfWeek();
    switch (nDay)
    {
        case i18n::Weekdays::SUNDAY :
            eDay = SUNDAY;
            break;
        case i18n::Weekdays::MONDAY :
            eDay = MONDAY;
            break;
        case i18n::Weekdays::TUESDAY :
            eDay = TUESDAY;
            break;
        case i18n::Weekdays::WEDNESDAY :
            eDay = WEDNESDAY;
            break;
        case i18n::Weekdays::THURSDAY :
            eDay = THURSDAY;
            break;
        case i18n::Weekdays::FRIDAY :
            eDay = FRIDAY;
            break;
        case i18n::Weekdays::SATURDAY :
            eDay = SATURDAY;
            break;
        default:
            SAL_WARN( "svtools.control", "Calendar::ImplGetWeekStart: broken i18n Gregorian calendar (getFirstDayOfWeek())");
            eDay = SUNDAY;
    }
    return eDay;
}

void Calendar::ImplGetWeekFont( vcl::Font& rFont )
{
    // weeknumber is displayed in WEEKNUMBER_HEIGHT%-Fontheight
    Size aFontSize = rFont.GetFontSize();
    aFontSize.Height() *= WEEKNUMBER_HEIGHT;
    aFontSize.Height() /= 100;
    rFont.SetFontSize( aFontSize );
    rFont.SetWeight( WEIGHT_NORMAL );
}

void Calendar::ImplFormat()
{
    if ( !mbFormat )
        return;

    if ( mbCalc )
    {
        Size aOutSize = GetOutputSizePixel();

        if ( (aOutSize.Width() <= 1) || (aOutSize.Height() <= 1) )
            return;

        OUString a99Text("99");

        vcl::Font aOldFont = GetFont();

        // Wochenanzeige beruecksichtigen
        if ( mnWinStyle & WB_WEEKNUMBER )
        {
            vcl::Font aTempFont = aOldFont;
            ImplGetWeekFont( aTempFont );
            SetFont( aTempFont );
            mnWeekWidth = GetTextWidth( a99Text )+WEEKNUMBER_OFFX;
            SetFont( aOldFont );
        }
        else
            mnWeekWidth = 0;

        if ( mnWinStyle & WB_BOLDTEXT )
        {
            vcl::Font aFont = aOldFont;
            if ( aFont.GetWeight() < WEIGHT_BOLD )
                aFont.SetWeight( WEIGHT_BOLD );
            else
                aFont.SetWeight( WEIGHT_NORMAL );
            SetFont( aFont );
        }

        long n99TextWidth = GetTextWidth( a99Text );
        long nTextHeight = GetTextHeight();

        // calculate width and x-position
        mnDayWidth      = n99TextWidth+DAY_OFFX;
        mnMonthWidth    = mnDayWidth*7;
        mnMonthWidth   += mnWeekWidth;
        mnMonthWidth   += MONTH_BORDERX*2;
        mnMonthPerLine  = aOutSize.Width() / mnMonthWidth;
        if ( !mnMonthPerLine )
            mnMonthPerLine = 1;
        long nOver      = ((aOutSize.Width()-(mnMonthPerLine*mnMonthWidth)) / mnMonthPerLine);
        mnMonthWidth   += nOver;
        mnDaysOffX      = MONTH_BORDERX;
        mnDaysOffX     += nOver/2;
        mnDaysOffX     += mnWeekWidth;

        // calculate height and y-position
        mnDayHeight     = nTextHeight + DAY_OFFY;
        mnWeekDayOffY   = nTextHeight + TITLE_OFFY + (TITLE_BORDERY*2);
        mnDaysOffY      = mnWeekDayOffY + nTextHeight + WEEKDAY_OFFY;
        mnMonthHeight   = (mnDayHeight*6) + mnDaysOffY;
        mnMonthHeight  += MONTH_OFFY;
        mnLines         = aOutSize.Height() / mnMonthHeight;
        if ( !mnLines )
            mnLines = 1;
        mnMonthHeight  += (aOutSize.Height()-(mnLines*mnMonthHeight)) / mnLines;

        // calculate spinfields
        long nSpinSize      = nTextHeight+TITLE_BORDERY-SPIN_OFFY;
        maPrevRect.Left()   = SPIN_OFFX;
        maPrevRect.Top()    = SPIN_OFFY;
        maPrevRect.Right()  = maPrevRect.Left()+nSpinSize;
        maPrevRect.Bottom() = maPrevRect.Top()+nSpinSize;
        maNextRect.Left()   = aOutSize.Width()-SPIN_OFFX-nSpinSize-1;
        maNextRect.Top()    = SPIN_OFFY;
        maNextRect.Right()  = maNextRect.Left()+nSpinSize;
        maNextRect.Bottom() = maNextRect.Top()+nSpinSize;

        if ( mnWinStyle & WB_BOLDTEXT )
            SetFont( aOldFont );

        // Calculate DayOfWeekText (gets displayed in a narrow font)
        maDayOfWeekText.clear();
        long nStartOffX = 0;
        sal_Int16 nDay = maCalendarWrapper.getFirstDayOfWeek();
        for ( sal_Int16 nDayOfWeek = 0; nDayOfWeek < 7; nDayOfWeek++ )
        {
            // Use narrow name.
            OUString aDayOfWeek( maCalendarWrapper.getDisplayName(
                        i18n::CalendarDisplayIndex::DAY, nDay, 2));
            long nOffX = (mnDayWidth-GetTextWidth( aDayOfWeek ))/2;
            if ( mnWinStyle & WB_BOLDTEXT )
                nOffX++;
            if ( !nDayOfWeek )
                nStartOffX = nOffX;
            else
                nOffX -= nStartOffX;
            nOffX += nDayOfWeek * mnDayWidth;
            mnDayOfWeekAry[nDayOfWeek] = nOffX;
            maDayOfWeekText += aDayOfWeek;
            nDay++;
            nDay %= 7;
        }

        mbCalc = false;
    }

    // calculate number of days

    DayOfWeek eStartDay = ImplGetWeekStart();

    sal_uInt16 nWeekDay;
    Date aTempDate = GetFirstMonth();
    maFirstDate = aTempDate;
    nWeekDay = (sal_uInt16)aTempDate.GetDayOfWeek();
    nWeekDay = (nWeekDay+(7-(sal_uInt16)eStartDay)) % 7;
    maFirstDate -= (sal_uLong)nWeekDay;
    mnDayCount = nWeekDay;
    sal_uInt16 nDaysInMonth;
    sal_uInt16 nMonthCount = (sal_uInt16)(mnMonthPerLine*mnLines);
    for ( sal_uInt16 i = 0; i < nMonthCount; i++ )
    {
        nDaysInMonth = aTempDate.GetDaysInMonth();
        mnDayCount += nDaysInMonth;
        aTempDate += nDaysInMonth;
    }
    Date aTempDate2 = aTempDate;
    --aTempDate2;
    nDaysInMonth = aTempDate2.GetDaysInMonth();
    aTempDate2 -= nDaysInMonth-1;
    nWeekDay = (sal_uInt16)aTempDate2.GetDayOfWeek();
    nWeekDay = (nWeekDay+(7-(sal_uInt16)eStartDay)) % 7;
    mnDayCount += 42-nDaysInMonth-nWeekDay;

    // determine colours
    maOtherColor = Color( COL_LIGHTGRAY );
    if ( maOtherColor.IsRGBEqual( GetBackground().GetColor() ) )
        maOtherColor.SetColor( COL_GRAY );

    Date aLastDate = GetLastDate();
    if ( (maOldFormatLastDate != aLastDate) ||
         (maOldFormatFirstDate != maFirstDate) )
    {
        maOldFormatFirstDate = maFirstDate;
        maOldFormatLastDate  = aLastDate;
    }

    // get DateInfo
    sal_Int16 nNewFirstYear = maFirstDate.GetYear();
    sal_Int16 nNewLastYear = GetLastDate().GetYear();
    if ( mnFirstYear )
    {
        if ( nNewFirstYear < mnFirstYear )
        {
            mnFirstYear = nNewFirstYear;
        }
        if ( nNewLastYear > mnLastYear )
        {
            mnLastYear = nNewLastYear;
        }
    }
    else
    {
        mnFirstYear = nNewFirstYear;
        mnLastYear = nNewLastYear;
    }

    mbFormat = false;
}

sal_uInt16 Calendar::ImplHitTest( const Point& rPos, Date& rDate ) const
{
    if ( mbFormat )
        return 0;

    if ( maPrevRect.IsInside( rPos ) )
        return CALENDAR_HITTEST_PREV;
    else if ( maNextRect.IsInside( rPos ) )
        return CALENDAR_HITTEST_NEXT;

    long        nY;
    long        nOffX;
    sal_uInt16      nDay;
    DayOfWeek   eStartDay = ImplGetWeekStart();

    rDate = GetFirstMonth();
    nY = 0;
    for ( long i = 0; i < mnLines; i++ )
    {
        if ( rPos.Y() < nY )
            return 0;

        long nX = 0;
        long nYMonth = nY+mnMonthHeight;
        for ( long j = 0; j < mnMonthPerLine; j++ )
        {
            if ( (rPos.X() < nX) && (rPos.Y() < nYMonth) )
                return 0;

            sal_uInt16 nDaysInMonth = rDate.GetDaysInMonth();

            // matching month was found
            if ( (rPos.X() > nX) && (rPos.Y() < nYMonth) &&
                 (rPos.X() < nX+mnMonthWidth) )
            {
                if ( rPos.Y() < (nY+(TITLE_BORDERY*2)+mnDayHeight))
                    return CALENDAR_HITTEST_MONTHTITLE;
                else
                {
                    long nDayX = nX+mnDaysOffX;
                    long nDayY = nY+mnDaysOffY;
                    if ( rPos.Y() < nDayY )
                        return 0;
                    sal_uInt16 nDayIndex = (sal_uInt16)rDate.GetDayOfWeek();
                    nDayIndex = (nDayIndex+(7-(sal_uInt16)eStartDay)) % 7;
                    if ( (i == 0) && (j == 0) )
                    {
                        Date aTempDate = rDate;
                        aTempDate -= nDayIndex;
                        for ( nDay = 0; nDay < nDayIndex; nDay++ )
                        {
                            nOffX = nDayX + (nDay*mnDayWidth);
                            if ( (rPos.Y() >= nDayY) && (rPos.Y() < nDayY+mnDayHeight) &&
                                 (rPos.X() >= nOffX) && (rPos.X() < nOffX+mnDayWidth) )
                            {
                                rDate = aTempDate;
                                rDate += nDay;
                                return CALENDAR_HITTEST_DAY;
                            }
                        }
                    }
                    for ( nDay = 1; nDay <= nDaysInMonth; nDay++ )
                    {
                        if ( rPos.Y() < nDayY )
                        {
                            rDate += nDayIndex;
                            return 0;
                        }
                        nOffX = nDayX + (nDayIndex*mnDayWidth);
                        if ( (rPos.Y() >= nDayY) && (rPos.Y() < nDayY+mnDayHeight) &&
                             (rPos.X() >= nOffX) && (rPos.X() < nOffX+mnDayWidth) )
                        {
                            rDate += nDay-1;
                            return CALENDAR_HITTEST_DAY;
                        }
                        if ( nDayIndex == 6 )
                        {
                            nDayIndex = 0;
                            nDayY += mnDayHeight;
                        }
                        else
                            nDayIndex++;
                    }
                    if ( (i == mnLines-1) && (j == mnMonthPerLine-1) )
                    {
                        sal_uInt16 nWeekDay = (sal_uInt16)rDate.GetDayOfWeek();
                        nWeekDay = (nWeekDay+(7-(sal_uInt16)eStartDay)) % 7;
                        sal_uInt16 nDayCount = 42-nDaysInMonth-nWeekDay;
                        Date aTempDate = rDate;
                        aTempDate += nDaysInMonth;
                        for ( nDay = 1; nDay <= nDayCount; nDay++ )
                        {
                            if ( rPos.Y() < nDayY )
                            {
                                rDate += nDayIndex;
                                return 0;
                            }
                            nOffX = nDayX + (nDayIndex*mnDayWidth);
                            if ( (rPos.Y() >= nDayY) && (rPos.Y() < nDayY+mnDayHeight) &&
                                 (rPos.X() >= nOffX) && (rPos.X() < nOffX+mnDayWidth) )
                            {
                                rDate = aTempDate;
                                rDate += nDay-1;
                                return CALENDAR_HITTEST_DAY;
                            }
                            if ( nDayIndex == 6 )
                            {
                                nDayIndex = 0;
                                nDayY += mnDayHeight;
                            }
                            else
                                nDayIndex++;
                        }
                    }
                }
            }

            rDate += nDaysInMonth;
            nX += mnMonthWidth;
        }

        nY += mnMonthHeight;
    }

    return 0;
}

namespace
{

void ImplDrawSpinArrow(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect, bool bPrev)
{
    long i;
    long n;
    long nLines;
    long nHeight = rRect.GetHeight();
    long nWidth = rRect.GetWidth();
    if (nWidth < nHeight)
        n = nWidth;
    else
        n = nHeight;
    if (!(n & 0x01))
        n--;
    nLines = n/2;

    tools::Rectangle aRect(Point( rRect.Left() + (nWidth / 2) - (nLines / 2),
                            rRect.Top() + (nHeight / 2) ),
                     Size(1, 1));
    if (!bPrev)
    {
        aRect.Left()  += nLines;
        aRect.Right() += nLines;
    }

    rRenderContext.DrawRect(aRect);
    for (i = 0; i < nLines; i++)
    {
        if (bPrev)
        {
            aRect.Left()++;
            aRect.Right()++;
        }
        else
        {
            aRect.Left()--;
            aRect.Right()--;
        }
        aRect.Top()--;
        aRect.Bottom()++;
        rRenderContext.DrawRect(aRect);
    }
}

} //end anonymous namespace

void Calendar::ImplDrawSpin(vcl::RenderContext& rRenderContext )
{
    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetButtonTextColor());
    tools::Rectangle aOutRect = maPrevRect;
    aOutRect.Left()   += 3;
    aOutRect.Top()    += 3;
    aOutRect.Right()  -= 3;
    aOutRect.Bottom() -= 3;
    ImplDrawSpinArrow(rRenderContext, aOutRect, true);
    aOutRect = maNextRect;
    aOutRect.Left()   += 3;
    aOutRect.Top()    += 3;
    aOutRect.Right()  -= 3;
    aOutRect.Bottom() -= 3;
    ImplDrawSpinArrow(rRenderContext, aOutRect, false);
}

void Calendar::ImplDrawDate(vcl::RenderContext& rRenderContext,
                            long nX, long nY,
                            sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear,
                            DayOfWeek eDayOfWeek,
                            bool bOther, sal_Int32 nToday )
{
    Color* pTextColor = nullptr;
    const OUString& rDay = maDayTexts[nDay - 1];
    tools::Rectangle aDateRect(nX, nY, nX + mnDayWidth - 1, nY + mnDayHeight - 1);

    bool bSel = false;
    bool bFocus = false;
    // actual day
    if ((nDay   == maCurDate.GetDay()) &&
        (nMonth == maCurDate.GetMonth()) &&
        (nYear  == maCurDate.GetYear()))
    {
        bFocus = true;
    }
    if (mpSelectTable)
    {
        if (mpSelectTable->find(Date(nDay, nMonth, nYear).GetDate()) != mpSelectTable->end())
            bSel = true;
    }

    // get textcolour
    if (bSel)
        pTextColor = &maSelColor;
    else if (bOther)
        pTextColor = &maOtherColor;
    else
    {
        if (eDayOfWeek == SATURDAY)
            pTextColor = mpSaturdayColor;
        else if (eDayOfWeek == SUNDAY)
            pTextColor = mpSundayColor;
        if (!pTextColor)
            pTextColor = mpStandardColor;
    }

    if (bFocus)
        HideFocus();

    // display background
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    if (bSel)
    {
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rStyleSettings.GetHighlightColor());
        rRenderContext.DrawRect(aDateRect);
    }

    // display text
    long nTextX = nX + (mnDayWidth - GetTextWidth(rDay)) - (DAY_OFFX / 2);
    long nTextY = nY + (mnDayHeight - GetTextHeight()) / 2;
    if (pTextColor)
    {
        Color aOldColor = rRenderContext.GetTextColor();
        rRenderContext.SetTextColor(*pTextColor);
        rRenderContext.DrawText(Point(nTextX, nTextY), rDay);
        rRenderContext.SetTextColor(aOldColor);
    }
    else
        rRenderContext.DrawText(Point(nTextX, nTextY), rDay);

    // today
    Date aTodayDate(maCurDate);
    if (nToday)
        aTodayDate.SetDate(nToday);
    else
        aTodayDate = Date(Date::SYSTEM);
    if ((nDay   == aTodayDate.GetDay()) &&
        (nMonth == aTodayDate.GetMonth()) &&
        (nYear  == aTodayDate.GetYear()))
    {
        rRenderContext.SetLineColor(rStyleSettings.GetWindowTextColor());
        rRenderContext.SetFillColor();
        rRenderContext.DrawRect(aDateRect);
    }

    // if needed do FocusRect
    if (bFocus && HasFocus())
        ShowFocus(aDateRect);

    if (mbDropPos && maDropDate == Date(nDay, nMonth, nYear))
        ImplInvertDropPos();
}

void Calendar::ImplDraw(vcl::RenderContext& rRenderContext)
{
    ImplFormat();

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    Size aOutSize(GetOutputSizePixel());
    long i;
    long j;
    long nY;
    long nDeltaX;
    long nDeltaY;
    long nDayX;
    long nDayY;
    sal_Int32 nToday = Date(Date::SYSTEM).GetDate();
    sal_uInt16 nDay;
    sal_uInt16 nMonth;
    sal_Int16 nYear;
    Date aDate = GetFirstMonth();
    DayOfWeek eStartDay = ImplGetWeekStart();

    HideFocus();

    nY = 0;
    for (i = 0; i < mnLines; i++)
    {
        // display title bar
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
        tools::Rectangle aTitleRect(0, nY, aOutSize.Width() - 1, nY + mnDayHeight - DAY_OFFY + TITLE_BORDERY * 2);
        rRenderContext.DrawRect(aTitleRect);
        Point aTopLeft1(aTitleRect.Left(), aTitleRect.Top());
        Point aTopLeft2(aTitleRect.Left(), aTitleRect.Top() + 1);
        Point aBottomRight1(aTitleRect.Right(), aTitleRect.Bottom());
        Point aBottomRight2(aTitleRect.Right(), aTitleRect.Bottom() - 1);
        rRenderContext.SetLineColor(rStyleSettings.GetDarkShadowColor());
        rRenderContext.DrawLine(aTopLeft1, Point(aBottomRight1.X(), aTopLeft1.Y()));
        rRenderContext.SetLineColor(rStyleSettings.GetLightColor() );
        rRenderContext.DrawLine(aTopLeft2, Point(aBottomRight2.X(), aTopLeft2.Y()));
        rRenderContext.DrawLine(aTopLeft2, Point(aTopLeft2.X(), aBottomRight2.Y()));
        rRenderContext.SetLineColor(rStyleSettings.GetShadowColor() );
        rRenderContext.DrawLine(Point(aTopLeft2.X(), aBottomRight2.Y()), aBottomRight2);
        rRenderContext.DrawLine(Point(aBottomRight2.X(), aTopLeft2.Y()), aBottomRight2);
        rRenderContext.SetLineColor(rStyleSettings.GetDarkShadowColor());
        rRenderContext.DrawLine(Point(aTopLeft1.X(), aBottomRight1.Y()), aBottomRight1);
        Point aSepPos1(0, aTitleRect.Top() + TITLE_BORDERY);
        Point aSepPos2(0, aTitleRect.Bottom() - TITLE_BORDERY);
        for (j = 0; j < mnMonthPerLine-1; j++)
        {
            aSepPos1.X() += mnMonthWidth-1;
            aSepPos2.X() = aSepPos1.X();
            rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
            rRenderContext.DrawLine(aSepPos1, aSepPos2);
            aSepPos1.X()++;
            aSepPos2.X() = aSepPos1.X();
            rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
            rRenderContext.DrawLine(aSepPos1, aSepPos2);
        }

        long nX = 0;
        for (j = 0; j < mnMonthPerLine; j++)
        {
            nMonth  = aDate.GetMonth();
            nYear   = aDate.GetYear();

            // display month in title bar
            nDeltaX = nX;
            nDeltaY = nY + TITLE_BORDERY;
            OUString aMonthText = maCalendarWrapper.getDisplayName(i18n::CalendarDisplayIndex::MONTH, nMonth - 1, 1)
                    + " "
                    + OUString::number(nYear);
            long nMonthTextWidth = rRenderContext.GetTextWidth(aMonthText);
            long nMonthOffX1 = 0;
            long nMonthOffX2 = 0;
            if (i == 0)
            {
                if (j == 0)
                    nMonthOffX1 = maPrevRect.Right() + 1;
                if (j == mnMonthPerLine - 1)
                    nMonthOffX2 = aOutSize.Width() - maNextRect.Left() + 1;
            }
            long nMaxMonthWidth = mnMonthWidth - nMonthOffX1 - nMonthOffX2 - 4;
            if (nMonthTextWidth > nMaxMonthWidth)
            {
                // Abbreviated month name.
                aMonthText  = maCalendarWrapper.getDisplayName(i18n::CalendarDisplayIndex::MONTH, nMonth - 1, 0)
                            + " "
                            + OUString::number(nYear);
                nMonthTextWidth = rRenderContext.GetTextWidth(aMonthText);
            }
            long nTempOff = (mnMonthWidth - nMonthTextWidth + 1) / 2;
            if (nTempOff < nMonthOffX1)
                nDeltaX += nMonthOffX1 + 1;
            else
            {
                if (nTempOff + nMonthTextWidth > mnMonthWidth - nMonthOffX2)
                    nDeltaX += mnMonthWidth - nMonthOffX2 - nMonthTextWidth;
                else
                    nDeltaX += nTempOff;
            }
            rRenderContext.SetTextColor(rStyleSettings.GetButtonTextColor());
            rRenderContext.DrawText(Point(nDeltaX, nDeltaY), aMonthText);
            rRenderContext.SetTextColor(rStyleSettings.GetWindowTextColor());

            // display week bar
            nDayX = nX + mnDaysOffX;
            nDayY = nY + mnWeekDayOffY;
            nDeltaY = nDayY + mnDayHeight;
            rRenderContext.SetLineColor(rStyleSettings.GetWindowTextColor());
            Point aStartPos(nDayX, nDeltaY);
            if (mnWinStyle & WB_WEEKNUMBER)
                aStartPos.X() -= WEEKNUMBER_OFFX - 2;
            rRenderContext.DrawLine(aStartPos, Point(nDayX + (7 * mnDayWidth), nDeltaY));
            rRenderContext.DrawTextArray(Point(nDayX + mnDayOfWeekAry[0], nDayY), maDayOfWeekText, &(mnDayOfWeekAry[1]));

            // display weeknumbers
            if (mnWinStyle & WB_WEEKNUMBER)
            {
                nDayX = nX + mnDaysOffX;
                nDayY = nY + mnWeekDayOffY;
                nDeltaY = nDayY + mnDayHeight;
                long nMonthHeight = mnDayHeight * 6;
                rRenderContext.DrawLine(Point(nDayX - WEEKNUMBER_OFFX + 2, nDeltaY),
                                        Point(nDayX - WEEKNUMBER_OFFX + 2, nDeltaY + nMonthHeight));
                vcl::Font aOldFont = rRenderContext.GetFont();
                vcl::Font aTempFont = aOldFont;
                ImplGetWeekFont(aTempFont);
                rRenderContext.SetFont(aTempFont);
                nDayX -= mnWeekWidth;
                nDayY = nY + mnDaysOffY;
                maCalendarWrapper.setGregorianDateTime(aDate);
                for (sal_uInt16 nWeekCount = 0; nWeekCount < 6; ++nWeekCount)
                {
                    sal_Int32 nWeek = maCalendarWrapper.getValue(i18n::CalendarFieldIndex::WEEK_OF_YEAR);
                    OUString aWeekText(OUString::number(nWeek));
                    long nOffX = (mnWeekWidth - WEEKNUMBER_OFFX) - rRenderContext.GetTextWidth(aWeekText);
                    long nOffY = (mnDayHeight - GetTextHeight()) / 2;
                    rRenderContext.DrawText(Point(nDayX + nOffX, nDayY + nOffY), aWeekText);
                    nDayY += mnDayHeight;
                    maCalendarWrapper.addValue(i18n::CalendarFieldIndex::DAY_OF_MONTH, 7);
                }
                rRenderContext.SetFont(aOldFont);
            }

            // display days
            sal_uInt16 nDaysInMonth = aDate.GetDaysInMonth();
            nDayX = nX + mnDaysOffX;
            nDayY = nY + mnDaysOffY;
            sal_uInt16 nDayIndex = (sal_uInt16) aDate.GetDayOfWeek();
            nDayIndex = (nDayIndex + (7 - (sal_uInt16)eStartDay)) % 7;
            if (i == 0 && j == 0)
            {
                Date aTempDate = aDate;
                aTempDate -= nDayIndex;
                for (nDay = 0; nDay < nDayIndex; ++nDay)
                {
                    nDeltaX = nDayX + (nDay * mnDayWidth);
                    ImplDrawDate(rRenderContext, nDeltaX, nDayY, nDay + aTempDate.GetDay(),
                                 aTempDate.GetMonth(), aTempDate.GetYear(),
                                 (DayOfWeek)((nDay + (sal_uInt16)eStartDay) % 7), true, nToday);
                }
            }
            for (nDay = 1; nDay <= nDaysInMonth; nDay++)
            {
                nDeltaX = nDayX + (nDayIndex * mnDayWidth);
                ImplDrawDate(rRenderContext, nDeltaX, nDayY, nDay, nMonth, nYear,
                             (DayOfWeek)((nDayIndex + (sal_uInt16)eStartDay) % 7),
                             false, nToday);
                if (nDayIndex == 6)
                {
                    nDayIndex = 0;
                    nDayY += mnDayHeight;
                }
                else
                    nDayIndex++;
            }
            if ((i == mnLines - 1) && (j == mnMonthPerLine - 1))
            {
                sal_uInt16 nWeekDay = (sal_uInt16)aDate.GetDayOfWeek();
                nWeekDay = (nWeekDay + (7 - (sal_uInt16)eStartDay)) % 7;
                sal_uInt16 nDayCount = 42 - nDaysInMonth - nWeekDay;
                Date aTempDate = aDate;
                aTempDate += nDaysInMonth;
                for (nDay = 1; nDay <= nDayCount; ++nDay)
                {
                    nDeltaX = nDayX + (nDayIndex * mnDayWidth);
                    ImplDrawDate(rRenderContext, nDeltaX, nDayY, nDay,
                                 aTempDate.GetMonth(), aTempDate.GetYear(),
                                 (DayOfWeek)((nDayIndex + (sal_uInt16)eStartDay) % 7),
                                 true, nToday);
                    if (nDayIndex == 6)
                    {
                        nDayIndex = 0;
                        nDayY += mnDayHeight;
                    }
                    else
                        nDayIndex++;
                }
            }

            aDate += nDaysInMonth;
            nX += mnMonthWidth;
        }

        nY += mnMonthHeight;
    }

    // draw spin buttons
    ImplDrawSpin(rRenderContext);
}

void Calendar::ImplUpdateDate( const Date& rDate )
{
    if (IsReallyVisible() && IsUpdateMode())
    {
        tools::Rectangle aDateRect(GetDateRect(rDate));
        if (!aDateRect.IsEmpty())
        {
            Invalidate(aDateRect);
        }
    }
}

void Calendar::ImplUpdateSelection( IntDateSet* pOld )
{
    IntDateSet*  pNew = mpSelectTable;

    for ( IntDateSet::const_iterator it = pOld->begin(); it != pOld->end(); ++it )
    {
        sal_uLong nKey = *it;
        if ( pNew->find( nKey ) == pNew->end() )
        {
            Date aTempDate( nKey );
            ImplUpdateDate( aTempDate );
        }
    }

    for ( IntDateSet::const_iterator it = pNew->begin(); it != pNew->end(); ++it )
    {
        sal_uLong nKey = *it;
        if ( pOld->find( nKey ) == pOld->end() )
        {
            Date aTempDate( nKey );
            ImplUpdateDate( aTempDate );
        }
    }
}

void Calendar::ImplMouseSelect( const Date& rDate, sal_uInt16 nHitTest,
                                bool bMove, bool bExpand, bool bExtended )
{
    std::unique_ptr<IntDateSet> pOldSel(new IntDateSet( *mpSelectTable ));
    Date    aOldDate = maCurDate;
    Date    aTempDate = rDate;

    if ( !(nHitTest & CALENDAR_HITTEST_DAY) )
        --aTempDate;

    if ( mbMultiSelection )
    {
        maCurDate = aTempDate;
        mbSelLeft = aTempDate < maAnchorDate;

        if ( bMove )
        {
            if ( mbSelLeft )
            {
                ImplCalendarUnSelectDateRange( mpSelectTable, mpRestoreSelectTable, Date( 1, 1, 0 ), aTempDate );
                ImplCalendarUnSelectDateRange( mpSelectTable, mpRestoreSelectTable, maAnchorDate, Date( 31, 12, 9999 ) );
            }
            else
            {
                ImplCalendarUnSelectDateRange( mpSelectTable, mpRestoreSelectTable, Date( 1, 1, 0 ), maAnchorDate );
                ImplCalendarUnSelectDateRange( mpSelectTable, mpRestoreSelectTable, aTempDate, Date( 31, 12, 9999 ) );
            }
            ImplCalendarSelectDateRange( mpSelectTable, aTempDate, maAnchorDate, !mbUnSel );
        }
        else
        {
            if ( bExpand )
            {
                if ( !bExtended )
                {
                    if ( mbSelLeft )
                    {
                        ImplCalendarSelectDateRange( mpSelectTable, Date( 1, 1, 0 ), aTempDate, false );
                        ImplCalendarSelectDateRange( mpSelectTable, maAnchorDate, Date( 31, 12, 9999 ), false );
                    }
                    else
                    {
                        ImplCalendarSelectDateRange( mpSelectTable, Date( 1, 1, 0 ), maAnchorDate, false );
                        ImplCalendarSelectDateRange( mpSelectTable, aTempDate, Date( 31, 12, 9999 ), false );
                    }
                }
                ImplCalendarSelectDateRange( mpSelectTable, aTempDate, maAnchorDate, true );
            }
            else if ( bExtended && !(mnWinStyle & WB_RANGESELECT) )
            {
                maAnchorDate = aTempDate;
                if ( IsDateSelected( aTempDate ) )
                {
                    mbUnSel = true;
                    ImplCalendarSelectDate( mpSelectTable, aTempDate, false );
                }
                else
                {
                    ImplCalendarSelectDate( mpSelectTable, aTempDate, true );
                }
            }
            else
            {
                maAnchorDate = aTempDate;
                ImplCalendarClearSelectDate( mpSelectTable );
                ImplCalendarSelectDate( mpSelectTable, aTempDate, true );
            }

            mpRestoreSelectTable = new IntDateSet( *mpSelectTable );
        }
    }
    else
    {
        if ( aTempDate < maCurDate )
            mbSelLeft = true;
        else
            mbSelLeft = false;
        if ( !(nHitTest & CALENDAR_HITTEST_DAY) )
            aTempDate = maOldCurDate;
        if ( !bMove )
            maAnchorDate = aTempDate;
        if ( aTempDate != maCurDate )
        {
            maCurDate = aTempDate;
            ImplCalendarSelectDate( mpSelectTable, aOldDate, false );
            ImplCalendarSelectDate( mpSelectTable, maCurDate, true );
        }
    }

    bool bNewSel = *pOldSel != *mpSelectTable;
    if ( (maCurDate != aOldDate) || bNewSel )
    {
        HideFocus();
        if ( bNewSel )
            ImplUpdateSelection( pOldSel.get() );
        if ( !bNewSel || pOldSel->find( aOldDate.GetDate() ) == pOldSel->end() )
            ImplUpdateDate( aOldDate );
        // assure focus rectangle is displayed again
        if ( HasFocus() || !bNewSel
             || mpSelectTable->find( maCurDate.GetDate() ) == mpSelectTable->end() )
            ImplUpdateDate( maCurDate );
    }
}

void Calendar::ImplUpdate( bool bCalcNew )
{
    if (IsReallyVisible() && IsUpdateMode())
    {
        if (bCalcNew && !mbCalc)
        {
            Invalidate();
        }
        else if (!mbFormat && !mbCalc)
        {
            Invalidate();
        }
    }

    if (bCalcNew)
        mbCalc = true;
    mbFormat = true;
}

void Calendar::ImplInvertDropPos()
{
    tools::Rectangle aRect = GetDateRect( maDropDate );//this is one Pixel to width and one to heigh
    aRect.Bottom() = aRect.Top()+mnDayHeight-1;
    aRect.Right() = aRect.Left()+mnDayWidth-1;
    Invert( aRect );
}

void Calendar::ImplScroll( bool bPrev )
{
    Date aNewFirstMonth = GetFirstMonth();
    if ( bPrev )
    {
        --aNewFirstMonth;
        aNewFirstMonth -= aNewFirstMonth.GetDaysInMonth()-1;
    }
    else
        aNewFirstMonth += aNewFirstMonth.GetDaysInMonth();
    SetFirstDate( aNewFirstMonth );
}

void Calendar::ImplShowMenu( const Point& rPos, const Date& rDate )
{
    EndSelection();

    Date        aOldFirstDate = GetFirstMonth();
    ScopedVclPtrInstance<PopupMenu> aPopupMenu;
    sal_uInt16      nMonthOff;
    sal_uInt16      nCurItemId;
    sal_uInt16      nYear = rDate.GetYear()-1;
    sal_uInt16      i;
    sal_uInt16      j;
    sal_uInt16      nYearIdCount = 1000;

    nMonthOff = (rDate.GetYear()-aOldFirstDate.GetYear())*12;
    if ( aOldFirstDate.GetMonth() < rDate.GetMonth() )
        nMonthOff += rDate.GetMonth()-aOldFirstDate.GetMonth();
    else
        nMonthOff -= aOldFirstDate.GetMonth()-rDate.GetMonth();

    // construct menu (include years with different months)
    for ( i = 0; i < MENU_YEAR_COUNT; i++ )
    {
        VclPtrInstance<PopupMenu> pYearPopupMenu;
        for ( j = 1; j <= 12; j++ )
            pYearPopupMenu->InsertItem( nYearIdCount+j,
                    maCalendarWrapper.getDisplayName(
                        i18n::CalendarDisplayIndex::MONTH, j-1, 1));
        aPopupMenu->InsertItem( 10+i, OUString::number( nYear+i ) );
        aPopupMenu->SetPopupMenu( 10+i, pYearPopupMenu );
        nYearIdCount += 1000;
    }

    mbMenuDown = true;
    nCurItemId = aPopupMenu->Execute( this, rPos );
    mbMenuDown = false;

    if ( nCurItemId )
    {
        sal_uInt16 nTempMonthOff = nMonthOff % 12;
        sal_uInt16 nTempYearOff = nMonthOff / 12;
        sal_uInt16 nNewMonth = nCurItemId % 1000;
        sal_uInt16 nNewYear = nYear+((nCurItemId-1000)/1000);
        if ( nTempMonthOff < nNewMonth )
            nNewMonth = nNewMonth - nTempMonthOff;
        else
        {
            nNewYear--;
            nNewMonth = 12-(nTempMonthOff-nNewMonth);
        }
        nNewYear = nNewYear - nTempYearOff;
        SetFirstDate( Date( 1, nNewMonth, nNewYear ) );
    }
}

void Calendar::ImplTracking( const Point& rPos, bool bRepeat )
{
    Date    aTempDate = maCurDate;
    sal_uInt16  nHitTest = ImplHitTest( rPos, aTempDate );

    if ( mbSpinDown )
    {
        mbPrevIn = (nHitTest & CALENDAR_HITTEST_PREV) != 0;
        mbNextIn = (nHitTest & CALENDAR_HITTEST_NEXT) != 0;

        if ( bRepeat && (mbPrevIn || mbNextIn) )
        {
            ImplScroll( mbPrevIn );
        }
    }
    else
        ImplMouseSelect( aTempDate, nHitTest, true, false, false );
}

void Calendar::ImplEndTracking( bool bCancel )
{
    bool bSelection = mbSelection;
    bool bSpinDown = mbSpinDown;

    mbDrag              = false;
    mbSelection         = false;
    mbMultiSelection    = false;
    mbUnSel             = false;
    mbSpinDown          = false;
    mbPrevIn            = false;
    mbNextIn            = false;

    if ( bCancel )
    {
        if ( maOldFirstDate != maFirstDate )
            SetFirstDate( maOldFirstDate );

        if ( !bSpinDown )
        {
            std::unique_ptr<IntDateSet> pOldSel(new IntDateSet( *mpSelectTable ));
            Date    aOldDate = maCurDate;
            maCurDate       = maOldCurDate;
            *mpSelectTable  = *mpOldSelectTable;
            HideFocus();
            ImplUpdateSelection( pOldSel.get() );
            if ( pOldSel->find( aOldDate.GetDate() ) == pOldSel->end() )
                ImplUpdateDate( aOldDate );
            //  assure focus rectangle is displayed again
            if ( HasFocus() || mpSelectTable->find( maCurDate.GetDate() ) == mpSelectTable->end() )
                ImplUpdateDate( maCurDate );
        }
    }

    if ( !bSpinDown )
    {
        if ( !bCancel )
        {
            // determine if we should scroll the visible area
            sal_uLong nSelCount = mpSelectTable->size();
            if ( nSelCount )
            {
                Date aFirstSelDate( *mpSelectTable->begin() );
                Date aLastSelDate( *mpSelectTable->rbegin() );
                if ( aLastSelDate < GetFirstMonth() )
                    ImplScroll( true );
                else if ( GetLastMonth() < aFirstSelDate )
                    ImplScroll( false );
            }
        }

        if ( mbAllSel ||
             (!bCancel && ((maCurDate != maOldCurDate) || (*mpOldSelectTable != *mpSelectTable))) )
            Select();

        if ( !bSelection && (mnWinStyle & WB_TABSTOP) && !bCancel )
            GrabFocus();

        delete mpOldSelectTable;
        mpOldSelectTable = nullptr;
        delete mpRestoreSelectTable;
        mpRestoreSelectTable = nullptr;
    }
}

IMPL_LINK_NOARG( Calendar, ScrollHdl, Timer*, void )
{
    bool bPrevIn = (mnDragScrollHitTest & CALENDAR_HITTEST_PREV) != 0;
    bool bNextIn = (mnDragScrollHitTest & CALENDAR_HITTEST_NEXT) != 0;
    if( bNextIn || bPrevIn )
    {
        ImplScroll( bPrevIn );
    }
}

void Calendar::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && !mbMenuDown )
    {
        Date    aTempDate = maCurDate;
        sal_uInt16  nHitTest = ImplHitTest( rMEvt.GetPosPixel(), aTempDate );
        if ( nHitTest )
        {
            if ( nHitTest & CALENDAR_HITTEST_MONTHTITLE )
                ImplShowMenu( rMEvt.GetPosPixel(), aTempDate );
            else
            {
                maOldFirstDate = maFirstDate;

                mbPrevIn = (nHitTest & CALENDAR_HITTEST_PREV) != 0;
                mbNextIn = (nHitTest & CALENDAR_HITTEST_NEXT) != 0;
                if ( mbPrevIn || mbNextIn )
                {
                    mbSpinDown = true;
                    ImplScroll( mbPrevIn );
                    // it should really read BUTTONREPEAT, therefore do not
                    // change it to SCROLLREPEAT, check with TH,
                    // why it could be different (71775)
                    StartTracking( StartTrackingFlags::ButtonRepeat );
                }
                else
                {
                    if ( (rMEvt.GetClicks() != 2) || !(nHitTest & CALENDAR_HITTEST_DAY) )
                    {
                        delete mpOldSelectTable;
                        maOldCurDate = maCurDate;
                        mpOldSelectTable = new IntDateSet( *mpSelectTable );

                        if ( !mbSelection )
                        {
                            mbDrag = true;
                            StartTracking();
                        }

                        mbMultiSelection = (mnWinStyle & (WB_MULTISELECT | WB_RANGESELECT)) != 0;
                        ImplMouseSelect( aTempDate, nHitTest, false, rMEvt.IsShift(), rMEvt.IsMod1() );
                    }
                }
            }
        }

        return;
    }

    Control::MouseButtonDown( rMEvt );
}

void Calendar::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && mbSelection )
        ImplEndTracking( false );
    else
        Control::MouseButtonUp( rMEvt );
}

void Calendar::MouseMove( const MouseEvent& rMEvt )
{
    if ( mbSelection && rMEvt.GetButtons() )
        ImplTracking( rMEvt.GetPosPixel(), false );
    else
        Control::MouseMove( rMEvt );
}

void Calendar::Tracking( const TrackingEvent& rTEvt )
{
    Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();

    if ( rTEvt.IsTrackingEnded() )
        ImplEndTracking( rTEvt.IsTrackingCanceled() );
    else
        ImplTracking( aMousePos, rTEvt.IsTrackingRepeat() );
}

void Calendar::KeyInput( const KeyEvent& rKEvt )
{
    Date    aNewDate = maCurDate;
    bool    bMultiSel = (mnWinStyle & (WB_RANGESELECT | WB_MULTISELECT)) != 0;
    bool    bExpand = rKEvt.GetKeyCode().IsShift();
    bool    bExtended = rKEvt.GetKeyCode().IsMod1();

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_HOME:
            aNewDate.SetDay( 1 );
            break;

        case KEY_END:
            aNewDate.SetDay( aNewDate.GetDaysInMonth() );
            break;

        case KEY_LEFT:
            --aNewDate;
            break;

        case KEY_RIGHT:
            ++aNewDate;
            break;

        case KEY_UP:
            aNewDate -= 7;
            break;

        case KEY_DOWN:
            aNewDate += 7;
            break;

        case KEY_PAGEUP:
            {
            Date aTempDate = aNewDate;
            aTempDate -= aNewDate.GetDay()+1;
            aNewDate -= aTempDate.GetDaysInMonth();
            }
            break;

        case KEY_PAGEDOWN:
            aNewDate += aNewDate.GetDaysInMonth();
            break;

        case KEY_SPACE:
            if ( bMultiSel && !(mnWinStyle & WB_RANGESELECT) )
            {
                if ( !bExpand )
                {
                    bool bDateSel = IsDateSelected( maCurDate );
                    SelectDate( maCurDate, !bDateSel );
                    mbSelLeft = false;
                    mbTravelSelect = true;
                    Select();
                    mbTravelSelect = false;
                }
            }
            else
                Control::KeyInput( rKEvt );
            break;

        default:
            Control::KeyInput( rKEvt );
            break;
    }

    if ( aNewDate != maCurDate )
    {
        if ( bMultiSel && bExpand )
        {
            std::unique_ptr<IntDateSet> pOldSel(new IntDateSet( *mpSelectTable ));
            Date aOldAnchorDate = maAnchorDate;
            mbSelLeft = aNewDate < maAnchorDate;
            if ( !bExtended )
            {
                if ( mbSelLeft )
                {
                    ImplCalendarSelectDateRange( mpSelectTable, Date( 1, 1, 0 ), aNewDate, false );
                    ImplCalendarSelectDateRange( mpSelectTable, maAnchorDate, Date( 31, 12, 9999 ), false );
                }
                else
                {
                    ImplCalendarSelectDateRange( mpSelectTable, Date( 1, 1, 0 ), maAnchorDate, false );
                    ImplCalendarSelectDateRange( mpSelectTable, aNewDate, Date( 31, 12, 9999 ), false );
                }
            }
            ImplCalendarSelectDateRange( mpSelectTable, aNewDate, maAnchorDate, true );
            SetCurDate( aNewDate );
            maAnchorDate = aOldAnchorDate;
            ImplUpdateSelection( pOldSel.get() );
        }
        else
        {
            if ( mnWinStyle & WB_RANGESELECT )
            {
                SetNoSelection();
                SelectDate( aNewDate );
            }
            SetCurDate( aNewDate );
        }
        mbTravelSelect = true;
        Select();
        mbTravelSelect = false;
    }
}

void Calendar::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    ImplDraw(rRenderContext);
}

void Calendar::GetFocus()
{
    ImplUpdateDate( maCurDate );
    Control::GetFocus();
}

void Calendar::LoseFocus()
{
    HideFocus();
    Control::LoseFocus();
}

void Calendar::Resize()
{
    ImplUpdate( true );
    Control::Resize();
}

void Calendar::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & (HelpEventMode::QUICK | HelpEventMode::BALLOON) )
    {
        Date aDate = maCurDate;
        if ( GetDate( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ), aDate ) )
        {
            tools::Rectangle aDateRect = GetDateRect( aDate );
            Point aPt = OutputToScreenPixel( aDateRect.TopLeft() );
            aDateRect.Left()   = aPt.X();
            aDateRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aDateRect.BottomRight() );
            aDateRect.Right()  = aPt.X();
            aDateRect.Bottom() = aPt.Y();

            if ( rHEvt.GetMode() & HelpEventMode::QUICK )
            {
                maCalendarWrapper.setGregorianDateTime( aDate);
                sal_uInt16      nWeek = (sal_uInt16) maCalendarWrapper.getValue( i18n::CalendarFieldIndex::WEEK_OF_YEAR);
                sal_uInt16      nMonth = aDate.GetMonth();
                OUString   aStr = maDayText
                                + ": "
                                + OUString::number(aDate.GetDayOfYear())
                                + " / "
                                + maWeekText
                                + ": "
                                + OUString::number(nWeek);
        // if year is not the same, add it
                if ( (nMonth == 12) && (nWeek == 1) )
                {
                    aStr += ",  " + OUString::number(aDate.GetNextYear());
                }
                else if ( (nMonth == 1) && (nWeek > 50) )
                {
                    aStr += ", " + OUString::number(aDate.GetYear()-1);
                }
                Help::ShowQuickHelp( this, aDateRect, aStr );
                return;
            }
        }
    }

    Control::RequestHelp( rHEvt );
}

void Calendar::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        if ( !mbSelection && rCEvt.IsMouseEvent() )
        {
            Date    aTempDate = maCurDate;
            sal_uInt16  nHitTest = ImplHitTest( rCEvt.GetMousePosPixel(), aTempDate );
            if ( nHitTest & CALENDAR_HITTEST_MONTHTITLE )
            {
                ImplShowMenu( rCEvt.GetMousePosPixel(), aTempDate );
                return;
            }
        }
    }
    else if ( rCEvt.GetCommand() == CommandEventId::Wheel )
    {
        const CommandWheelData* pData = rCEvt.GetWheelData();
        if ( pData->GetMode() == CommandWheelMode::SCROLL )
        {
            long nNotchDelta = pData->GetNotchDelta();
            if ( nNotchDelta < 0 )
            {
                while ( nNotchDelta < 0 )
                {
                    ImplScroll( true );
                    nNotchDelta++;
                }
            }
            else
            {
                while ( nNotchDelta > 0 )
                {
                    ImplScroll( false );
                    nNotchDelta--;
                }
            }

            return;
        }
    }

    Control::Command( rCEvt );
}

void Calendar::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == StateChangedType::InitShow )
        ImplFormat();
}

void Calendar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void Calendar::Select()
{
    maSelectHdl.Call( this );
}

void Calendar::SelectDate( const Date& rDate, bool bSelect )
{
    if ( !rDate.IsValidAndGregorian() )
        return;

    std::unique_ptr<IntDateSet> pOldSel;

    pOldSel.reset(new IntDateSet( *mpSelectTable ));

    ImplCalendarSelectDate( mpSelectTable, rDate, bSelect );

    if ( pOldSel )
        ImplUpdateSelection( pOldSel.get() );
}

void Calendar::SetNoSelection()
{
    std::unique_ptr<IntDateSet> pOldSel;

    pOldSel.reset(new IntDateSet( *mpSelectTable ));

    ImplCalendarClearSelectDate( mpSelectTable );

    if ( pOldSel )
        ImplUpdateSelection( pOldSel.get() );
}

bool Calendar::IsDateSelected( const Date& rDate ) const
{
    return mpSelectTable->find( rDate.GetDate() ) != mpSelectTable->end();
}

Date Calendar::GetFirstSelectedDate() const
{
    if ( !mpSelectTable->empty() )
        return Date( *mpSelectTable->begin() );
    else
    {
        Date aDate( 0, 0, 0 );
        return aDate;
    }
}

void Calendar::SetCurDate( const Date& rNewDate )
{
    if ( !rNewDate.IsValidAndGregorian() )
        return;

    if ( maCurDate != rNewDate )
    {
        bool bUpdate    = IsVisible() && IsUpdateMode();
        Date aOldDate   = maCurDate;
        maCurDate       = rNewDate;
        maAnchorDate    = maCurDate;

        if ( !(mnWinStyle & (WB_RANGESELECT | WB_MULTISELECT)) )
        {
            ImplCalendarSelectDate( mpSelectTable, aOldDate, false );
            ImplCalendarSelectDate( mpSelectTable, maCurDate, true );
        }
        else if ( !HasFocus() )
            bUpdate = false;

        // shift actual date in the visible area
        if ( mbFormat || (maCurDate < GetFirstMonth()) )
            SetFirstDate( maCurDate );
        else if ( maCurDate > GetLastMonth() )
        {
            Date aTempDate = GetLastMonth();
            long nDateOff = maCurDate-aTempDate;
            if ( nDateOff < 365 )
            {
                Date aFirstDate = GetFirstMonth();
                aFirstDate += aFirstDate.GetDaysInMonth();
                ++aTempDate;
                while ( nDateOff > aTempDate.GetDaysInMonth() )
                {
                    aFirstDate += aFirstDate.GetDaysInMonth();
                    long nDaysInMonth = aTempDate.GetDaysInMonth();
                    aTempDate += nDaysInMonth;
                    nDateOff -= nDaysInMonth;
                }
                SetFirstDate( aFirstDate );
            }
            else
                SetFirstDate( maCurDate );
        }
        else
        {
            if ( bUpdate )
            {
                HideFocus();
                ImplUpdateDate( aOldDate );
                ImplUpdateDate( maCurDate );
            }
        }
    }
}

void Calendar::SetFirstDate( const Date& rNewFirstDate )
{
    if ( maFirstDate != rNewFirstDate )
    {
        maFirstDate = Date( 1, rNewFirstDate.GetMonth(), rNewFirstDate.GetYear() );
        mbDropPos = false;
        ImplUpdate();
    }
}

Date Calendar::GetFirstMonth() const
{
    if ( maFirstDate.GetDay() > 1 )
    {
        if ( maFirstDate.GetMonth() == 12 )
            return Date( 1, 1, maFirstDate.GetNextYear() );
        else
            return Date( 1, maFirstDate.GetMonth()+1, maFirstDate.GetYear() );
    }
    else
        return maFirstDate;
}

Date Calendar::GetLastMonth() const
{
    Date aDate = GetFirstMonth();
    sal_uInt16 nMonthCount = GetMonthCount();
    for ( sal_uInt16 i = 0; i < nMonthCount; i++ )
        aDate += aDate.GetDaysInMonth();
    --aDate;
    return aDate;
}

sal_uInt16 Calendar::GetMonthCount() const
{
    if ( mbFormat )
        return 1;
    else
        return (sal_uInt16)(mnMonthPerLine*mnLines);
}

bool Calendar::GetDate( const Point& rPos, Date& rDate ) const
{
    Date    aDate = maCurDate;
    sal_uInt16  nHitTest = ImplHitTest( rPos, aDate );
    if ( nHitTest & CALENDAR_HITTEST_DAY )
    {
        rDate = aDate;
        return true;
    }
    else
        return false;
}

tools::Rectangle Calendar::GetDateRect( const Date& rDate ) const
{
    tools::Rectangle aRect;

    if ( mbFormat || (rDate < maFirstDate) || (rDate > (maFirstDate+mnDayCount)) )
        return aRect;

    long    nX;
    long    nY;
    sal_uLong   nDaysOff;
    sal_uInt16  nDayIndex;
    Date    aDate = GetFirstMonth();

    if ( rDate < aDate )
    {
        aRect = GetDateRect( aDate );
        nDaysOff = aDate-rDate;
        nX = (long)(nDaysOff*mnDayWidth);
        aRect.Left() -= nX;
        aRect.Right() -= nX;
        return aRect;
    }
    else
    {
        Date aLastDate = GetLastMonth();
        if ( rDate > aLastDate )
        {
            sal_uInt16 nWeekDay = (sal_uInt16)aLastDate.GetDayOfWeek();
            nWeekDay = (nWeekDay+(7-(sal_uInt16)ImplGetWeekStart())) % 7;
            aLastDate -= nWeekDay;
            aRect = GetDateRect( aLastDate );
            nDaysOff = rDate-aLastDate;
            nDayIndex = 0;
            for ( sal_uLong i = 0; i <= nDaysOff; i++ )
            {
                if ( aLastDate == rDate )
                {
                    aRect.Left() += nDayIndex*mnDayWidth;
                    aRect.Right() = aRect.Left()+mnDayWidth;
                    return aRect;
                }
                if ( nDayIndex == 6 )
                {
                    nDayIndex = 0;
                    aRect.Top() += mnDayHeight;
                    aRect.Bottom() += mnDayHeight;
                }
                else
                    nDayIndex++;
                ++aLastDate;
            }
        }
    }

    nY = 0;
    for ( long i = 0; i < mnLines; i++ )
    {
        nX = 0;
        for ( long j = 0; j < mnMonthPerLine; j++ )
        {
            sal_uInt16 nDaysInMonth = aDate.GetDaysInMonth();

            // month is called
            if ( (aDate.GetMonth() == rDate.GetMonth()) &&
                 (aDate.GetYear() == rDate.GetYear()) )
            {
                long nDayX = nX+mnDaysOffX;
                long nDayY = nY+mnDaysOffY;
                nDayIndex = (sal_uInt16)aDate.GetDayOfWeek();
                nDayIndex = (nDayIndex+(7-(sal_uInt16)ImplGetWeekStart())) % 7;
                for ( sal_uInt16 nDay = 1; nDay <= nDaysInMonth; nDay++ )
                {
                    if ( nDay == rDate.GetDay() )
                    {
                        aRect.Left()    = nDayX + (nDayIndex*mnDayWidth);
                        aRect.Top()     = nDayY;
                        aRect.Right()   = aRect.Left()+mnDayWidth;
                        aRect.Bottom()  = aRect.Top()+mnDayHeight;
                        break;
                    }
                    if ( nDayIndex == 6 )
                    {
                        nDayIndex = 0;
                        nDayY += mnDayHeight;
                    }
                    else
                        nDayIndex++;
                }
            }

            aDate += nDaysInMonth;
            nX += mnMonthWidth;
        }

        nY += mnMonthHeight;
    }

    return aRect;
}

void Calendar::StartSelection()
{
    delete mpOldSelectTable;
    maOldCurDate = maCurDate;
    mpOldSelectTable = new IntDateSet( *mpSelectTable );

    mbSelection = true;
}

void Calendar::EndSelection()
{
    if ( mbDrag || mbSpinDown || mbSelection )
    {
        if ( !mbSelection )
            ReleaseMouse();

        mbDrag              = false;
        mbSelection         = false;
        mbMultiSelection    = false;
        mbSpinDown          = false;
        mbPrevIn            = false;
        mbNextIn            = false;
    }
}

Size Calendar::CalcWindowSizePixel() const
{
    OUString  a99Text("99");
    vcl::Font aOldFont = GetFont();

    // take display of week into account
    long nWeekWidth;
    if ( mnWinStyle & WB_WEEKNUMBER )
    {
        vcl::Font aTempFont = aOldFont;
        ImplGetWeekFont( aTempFont );
        const_cast<Calendar*>(this)->SetFont( aTempFont );
        nWeekWidth = GetTextWidth( a99Text )+WEEKNUMBER_OFFX;
        const_cast<Calendar*>(this)->SetFont( aOldFont );
    }
    else
        nWeekWidth = 0;

    if ( mnWinStyle & WB_BOLDTEXT )
    {
        vcl::Font aFont = aOldFont;
        if ( aFont.GetWeight() < WEIGHT_BOLD )
            aFont.SetWeight( WEIGHT_BOLD );
        else
            aFont.SetWeight( WEIGHT_NORMAL );
        const_cast<Calendar*>(this)->SetFont( aFont );
    }

    Size    aSize;
    long    n99TextWidth = GetTextWidth( a99Text );
    long    nTextHeight = GetTextHeight();

    if ( mnWinStyle & WB_BOLDTEXT )
        const_cast<Calendar*>(this)->SetFont( aOldFont );

    aSize.Width()  += ((n99TextWidth+DAY_OFFX)*7) + nWeekWidth;
    aSize.Width()  += MONTH_BORDERX*2;

    aSize.Height()  = nTextHeight + TITLE_OFFY + (TITLE_BORDERY*2);
    aSize.Height() += nTextHeight + WEEKDAY_OFFY;
    aSize.Height() += ((nTextHeight+DAY_OFFY)*6);
    aSize.Height() += MONTH_OFFY;

    return aSize;
}

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
            aSize.Width()   = mpTodayBtn->GetCtrlTextWidth( mpTodayBtn->GetText() );
            aSize.Height()  = mpTodayBtn->GetTextHeight();
            aSize.Width()  += CALFIELD_EXTRA_BUTTON_WIDTH;
            aSize.Height() += CALFIELD_EXTRA_BUTTON_HEIGHT;
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
            aSize.Width()   = mpNoneBtn->GetCtrlTextWidth( mpNoneBtn->GetText() );
            aSize.Height()  = mpNoneBtn->GetTextHeight();
            aSize.Width()  += CALFIELD_EXTRA_BUTTON_WIDTH;
            aSize.Height() += CALFIELD_EXTRA_BUTTON_HEIGHT;
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
            aTodayBtnSize.Width() = aNoneBtnSize.Width();
        else
            aNoneBtnSize.Width() = aTodayBtnSize.Width();
        if ( aTodayBtnSize.Height() < aNoneBtnSize.Height() )
            aTodayBtnSize.Height() = aNoneBtnSize.Height();
        else
            aNoneBtnSize.Height() = aTodayBtnSize.Height();

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
        aOutSize.Height() += nBtnHeight + (CALFIELD_BORDER_Y*2) + CALFIELD_BORDER_YTOP;
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
    , mnCalendarStyle(0)
    , mpTodayBtn(nullptr)
    , mpNoneBtn(nullptr)
    , maDefaultDate( Date::EMPTY )
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
    if ( !pCalendar->IsTravelSelect() )
    {
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
            if ( maDefaultDate.IsValidAndGregorian() )
                aDate = maDefaultDate;
            else
                aDate = Date( Date::SYSTEM );
        }
        if ( pCalendar->GetStyle() & (WB_RANGESELECT | WB_MULTISELECT) )
        {
            pCalendar->SetNoSelection();
            pCalendar->SelectDate( aDate );
        }
        pCalendar->SetCurDate( aDate );
        Point       aPos( GetParent()->OutputToScreenPixel( GetPosPixel() ) );
        tools::Rectangle   aRect( aPos, GetSizePixel() );
        aRect.Bottom() -= 1;
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
        mpCalendar = VclPtr<Calendar>::Create( mpFloatWin, mnCalendarStyle | WB_TABSTOP );
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
