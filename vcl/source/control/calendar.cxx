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

#include <vcl/calendar.hxx>
#include <svdata.hxx>
#include <strings.hrc>
#include <memory>

#define DAY_OFFX                        4
#define DAY_OFFY                        2
#define MONTH_BORDERX                   4
#define MONTH_OFFY                      3
#define WEEKDAY_OFFY                    3
#define TITLE_OFFY                      3
#define TITLE_BORDERY                   2
#define SPIN_OFFX                       4
#define SPIN_OFFY                       TITLE_BORDERY

#define CALENDAR_HITTEST_DAY            (sal_uInt16(0x0001))
#define CALENDAR_HITTEST_MONTHTITLE     (sal_uInt16(0x0004))
#define CALENDAR_HITTEST_PREV           (sal_uInt16(0x0008))
#define CALENDAR_HITTEST_NEXT           (sal_uInt16(0x0010))

#define MENU_YEAR_COUNT                 3

using namespace ::com::sun::star;

static void ImplCalendarSelectDate( IntDateSet* pTable, const Date& rDate, bool bSelect )
{
    if ( bSelect )
        pTable->insert( rDate.GetDate() );
    else
        pTable->erase( rDate.GetDate() );
}



void Calendar::ImplInit( WinBits nWinStyle )
{
    mpSelectTable.reset(new IntDateSet);
    mnDayCount              = 0;
    mnWinStyle              = nWinStyle;
    mnFirstYear             = 0;
    mnLastYear              = 0;
    mbCalc                  = true;
    mbFormat                = true;
    mbDrag                  = false;
    mbSelection             = false;
    mbMenuDown              = false;
    mbSpinDown              = false;
    mbPrevIn                = false;
    mbNextIn                = false;
    mbTravelSelect          = false;
    mbAllSel                = false;

    OUString aGregorian( "gregorian");
    maCalendarWrapper.loadCalendar( aGregorian,
            Application::GetAppLocaleDataWrapper().getLanguageTag().getLocale());
    if (maCalendarWrapper.getUniqueID() != aGregorian)
    {
        SAL_WARN( "vcl.control", "Calendar::ImplInit: No ``gregorian'' calendar available for locale ``"
            << Application::GetAppLocaleDataWrapper().getLanguageTag().getBcp47()
            << "'' and other calendars aren't supported. Using en-US fallback." );

        /* If we ever wanted to support other calendars than Gregorian a lot of
         * rewrite would be necessary to internally replace use of class Date
         * with proper class CalendarWrapper methods, get rid of fixed 12
         * months, fixed 7 days, ... */
        maCalendarWrapper.loadCalendar( aGregorian, lang::Locale( "en", "US", ""));
    }

    SetFirstDate( maCurDate );
    ImplCalendarSelectDate( mpSelectTable.get(), maCurDate, true );

    // Sonstige Strings erzeugen
    maDayText = VclResId(STR_SVT_CALENDAR_DAY);
    maWeekText = VclResId(STR_SVT_CALENDAR_WEEK);

    // Tagestexte anlegen
    for (sal_Int32 i = 0; i < 31; ++i)
        maDayTexts[i] = OUString::number(i+1);

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
    Control( pParent, nWinStyle & (WB_TABSTOP | WB_GROUP | WB_BORDER | WB_3DLOOK) ),
    maCalendarWrapper( Application::GetAppLocaleDataWrapper().getComponentContext() ),
    maOldFormatFirstDate( 0, 0, 1900 ),
    maOldFormatLastDate( 0, 0, 1900 ),
    maFirstDate( 0, 0, 1900 ),
    maOldFirstDate( 0, 0, 1900 ),
    maCurDate( Date::SYSTEM ),
    maOldCurDate( 0, 0, 1900 )
{
    ImplInit( nWinStyle );
}

Calendar::~Calendar()
{
    disposeOnce();
}

void Calendar::dispose()
{
    mpSelectTable.reset();
    mpOldSelectTable.reset();
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
            SAL_WARN( "vcl.control", "Calendar::ImplGetWeekStart: broken i18n Gregorian calendar (getFirstDayOfWeek())");
            eDay = SUNDAY;
    }
    return eDay;
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

        OUString const a99Text("99");

        vcl::Font aOldFont = GetFont();

        long n99TextWidth = GetTextWidth( a99Text );
        long nTextHeight = GetTextHeight();

        // calculate width and x-position
        mnDayWidth      = n99TextWidth+DAY_OFFX;
        mnMonthWidth    = mnDayWidth*7;
        mnMonthWidth   += MONTH_BORDERX*2;
        mnMonthPerLine  = aOutSize.Width() / mnMonthWidth;
        if ( !mnMonthPerLine )
            mnMonthPerLine = 1;
        long nOver      = (aOutSize.Width()-(mnMonthPerLine*mnMonthWidth)) / mnMonthPerLine;
        mnMonthWidth   += nOver;
        mnDaysOffX      = MONTH_BORDERX;
        mnDaysOffX     += nOver/2;

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
        maPrevRect.SetLeft( SPIN_OFFX );
        maPrevRect.SetTop( SPIN_OFFY );
        maPrevRect.SetRight( maPrevRect.Left()+nSpinSize );
        maPrevRect.SetBottom( maPrevRect.Top()+nSpinSize );
        maNextRect.SetLeft( aOutSize.Width()-SPIN_OFFX-nSpinSize-1 );
        maNextRect.SetTop( SPIN_OFFY );
        maNextRect.SetRight( maNextRect.Left()+nSpinSize );
        maNextRect.SetBottom( maNextRect.Top()+nSpinSize );

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
    nWeekDay = static_cast<sal_uInt16>(aTempDate.GetDayOfWeek());
    nWeekDay = (nWeekDay+(7-static_cast<sal_uInt16>(eStartDay))) % 7;
    maFirstDate.AddDays( -nWeekDay );
    mnDayCount = nWeekDay;
    sal_uInt16 nDaysInMonth;
    sal_uInt16 nMonthCount = static_cast<sal_uInt16>(mnMonthPerLine*mnLines);
    for ( sal_uInt16 i = 0; i < nMonthCount; i++ )
    {
        nDaysInMonth = aTempDate.GetDaysInMonth();
        mnDayCount += nDaysInMonth;
        aTempDate.AddDays( nDaysInMonth );
    }
    Date aTempDate2 = aTempDate;
    --aTempDate2;
    nDaysInMonth = aTempDate2.GetDaysInMonth();
    aTempDate2.AddDays( -(nDaysInMonth-1) );
    nWeekDay = static_cast<sal_uInt16>(aTempDate2.GetDayOfWeek());
    nWeekDay = (nWeekDay+(7-static_cast<sal_uInt16>(eStartDay))) % 7;
    mnDayCount += 42-nDaysInMonth-nWeekDay;

    // determine colours
    maOtherColor = COL_LIGHTGRAY;
    if ( maOtherColor.IsRGBEqual( GetBackground().GetColor() ) )
        maOtherColor = COL_GRAY;

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
    sal_Int32   nDay;
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
                    sal_Int32 nDayIndex = static_cast<sal_Int32>(rDate.GetDayOfWeek());
                    nDayIndex = (nDayIndex+(7-static_cast<sal_Int32>(eStartDay))) % 7;
                    if ( (i == 0) && (j == 0) )
                    {
                        Date aTempDate = rDate;
                        aTempDate.AddDays( -nDayIndex );
                        for ( nDay = 0; nDay < nDayIndex; nDay++ )
                        {
                            nOffX = nDayX + (nDay*mnDayWidth);
                            if ( (rPos.Y() >= nDayY) && (rPos.Y() < nDayY+mnDayHeight) &&
                                 (rPos.X() >= nOffX) && (rPos.X() < nOffX+mnDayWidth) )
                            {
                                rDate = aTempDate;
                                rDate.AddDays( nDay );
                                return CALENDAR_HITTEST_DAY;
                            }
                        }
                    }
                    for ( nDay = 1; nDay <= nDaysInMonth; nDay++ )
                    {
                        if ( rPos.Y() < nDayY )
                        {
                            rDate.AddDays( nDayIndex );
                            return 0;
                        }
                        nOffX = nDayX + (nDayIndex*mnDayWidth);
                        if ( (rPos.Y() >= nDayY) && (rPos.Y() < nDayY+mnDayHeight) &&
                             (rPos.X() >= nOffX) && (rPos.X() < nOffX+mnDayWidth) )
                        {
                            rDate.AddDays( nDay-1 );
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
                        sal_uInt16 nWeekDay = static_cast<sal_uInt16>(rDate.GetDayOfWeek());
                        nWeekDay = (nWeekDay+(7-static_cast<sal_uInt16>(eStartDay))) % 7;
                        sal_Int32 nDayCount = 42-nDaysInMonth-nWeekDay;
                        Date aTempDate = rDate;
                        aTempDate.AddDays( nDaysInMonth );
                        for ( nDay = 1; nDay <= nDayCount; nDay++ )
                        {
                            if ( rPos.Y() < nDayY )
                            {
                                rDate.AddDays( nDayIndex );
                                return 0;
                            }
                            nOffX = nDayX + (nDayIndex*mnDayWidth);
                            if ( (rPos.Y() >= nDayY) && (rPos.Y() < nDayY+mnDayHeight) &&
                                 (rPos.X() >= nOffX) && (rPos.X() < nOffX+mnDayWidth) )
                            {
                                rDate = aTempDate;
                                rDate.AddDays( nDay-1 );
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

            rDate.AddDays( nDaysInMonth );
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
        aRect.AdjustLeft(nLines );
        aRect.AdjustRight(nLines );
    }

    rRenderContext.DrawRect(aRect);
    for (i = 0; i < nLines; i++)
    {
        if (bPrev)
        {
            aRect.AdjustLeft( 1 );
            aRect.AdjustRight( 1 );
        }
        else
        {
            aRect.AdjustLeft( -1 );
            aRect.AdjustRight( -1 );
        }
        aRect.AdjustTop( -1 );
        aRect.AdjustBottom( 1 );
        rRenderContext.DrawRect(aRect);
    }
}

} //end anonymous namespace

void Calendar::ImplDrawSpin(vcl::RenderContext& rRenderContext )
{
    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetButtonTextColor());
    tools::Rectangle aOutRect = maPrevRect;
    aOutRect.AdjustLeft(3 );
    aOutRect.AdjustTop(3 );
    aOutRect.AdjustRight( -3 );
    aOutRect.AdjustBottom( -3 );
    ImplDrawSpinArrow(rRenderContext, aOutRect, true);
    aOutRect = maNextRect;
    aOutRect.AdjustLeft(3 );
    aOutRect.AdjustTop(3 );
    aOutRect.AdjustRight( -3 );
    aOutRect.AdjustBottom( -3 );
    ImplDrawSpinArrow(rRenderContext, aOutRect, false);
}

void Calendar::ImplDrawDate(vcl::RenderContext& rRenderContext,
                            long nX, long nY,
                            sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear,
                            bool bOther, sal_Int32 nToday )
{
    Color const * pTextColor = nullptr;
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
            aSepPos1.AdjustX(mnMonthWidth-1 );
            aSepPos2.setX( aSepPos1.X() );
            rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
            rRenderContext.DrawLine(aSepPos1, aSepPos2);
            aSepPos1.AdjustX( 1 );
            aSepPos2.setX( aSepPos1.X() );
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
            rRenderContext.DrawLine(aStartPos, Point(nDayX + (7 * mnDayWidth), nDeltaY));
            rRenderContext.DrawTextArray(Point(nDayX + mnDayOfWeekAry[0], nDayY), maDayOfWeekText, &(mnDayOfWeekAry[1]));

            // display days
            sal_uInt16 nDaysInMonth = aDate.GetDaysInMonth();
            nDayX = nX + mnDaysOffX;
            nDayY = nY + mnDaysOffY;
            sal_uInt16 nDayIndex = static_cast<sal_uInt16>(aDate.GetDayOfWeek());
            nDayIndex = (nDayIndex + (7 - static_cast<sal_uInt16>(eStartDay))) % 7;
            if (i == 0 && j == 0)
            {
                Date aTempDate = aDate;
                aTempDate.AddDays( -nDayIndex );
                for (nDay = 0; nDay < nDayIndex; ++nDay)
                {
                    nDeltaX = nDayX + (nDay * mnDayWidth);
                    ImplDrawDate(rRenderContext, nDeltaX, nDayY, nDay + aTempDate.GetDay(),
                                 aTempDate.GetMonth(), aTempDate.GetYear(),
                                 true, nToday);
                }
            }
            for (nDay = 1; nDay <= nDaysInMonth; nDay++)
            {
                nDeltaX = nDayX + (nDayIndex * mnDayWidth);
                ImplDrawDate(rRenderContext, nDeltaX, nDayY, nDay, nMonth, nYear,
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
                sal_uInt16 nWeekDay = static_cast<sal_uInt16>(aDate.GetDayOfWeek());
                nWeekDay = (nWeekDay + (7 - static_cast<sal_uInt16>(eStartDay))) % 7;
                sal_uInt16 nDayCount = 42 - nDaysInMonth - nWeekDay;
                Date aTempDate = aDate;
                aTempDate.AddDays( nDaysInMonth );
                for (nDay = 1; nDay <= nDayCount; ++nDay)
                {
                    nDeltaX = nDayX + (nDayIndex * mnDayWidth);
                    ImplDrawDate(rRenderContext, nDeltaX, nDayY, nDay,
                                 aTempDate.GetMonth(), aTempDate.GetYear(),
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

            aDate.AddDays( nDaysInMonth );
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
    IntDateSet*  pNew = mpSelectTable.get();

    for (auto const& nKey : *pOld)
    {
        if ( pNew->find(nKey) == pNew->end() )
        {
            Date aTempDate(nKey);
            ImplUpdateDate(aTempDate);
        }
    }

    for (auto const& nKey : *pNew)
    {
        if ( pOld->find(nKey) == pOld->end() )
        {
            Date aTempDate(nKey);
            ImplUpdateDate(aTempDate);
        }
    }
}

void Calendar::ImplMouseSelect( const Date& rDate, sal_uInt16 nHitTest )
{
    std::unique_ptr<IntDateSet> pOldSel(new IntDateSet( *mpSelectTable ));
    Date    aOldDate = maCurDate;
    Date    aTempDate = rDate;

    if ( !(nHitTest & CALENDAR_HITTEST_DAY) )
        --aTempDate;

    if ( !(nHitTest & CALENDAR_HITTEST_DAY) )
        aTempDate = maOldCurDate;
    if ( aTempDate != maCurDate )
    {
        maCurDate = aTempDate;
        ImplCalendarSelectDate( mpSelectTable.get(), aOldDate, false );
        ImplCalendarSelectDate( mpSelectTable.get(), maCurDate, true );
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

void Calendar::ImplScroll( bool bPrev )
{
    Date aNewFirstMonth = GetFirstMonth();
    if ( bPrev )
    {
        --aNewFirstMonth;
        aNewFirstMonth.AddDays( -(aNewFirstMonth.GetDaysInMonth()-1));
    }
    else
        aNewFirstMonth.AddDays( aNewFirstMonth.GetDaysInMonth());
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

    if ( !nCurItemId )
        return;

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
        ImplMouseSelect( aTempDate, nHitTest );
}

void Calendar::ImplEndTracking( bool bCancel )
{
    bool bSelection = mbSelection;
    bool bSpinDown = mbSpinDown;

    mbDrag              = false;
    mbSelection         = false;
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

    if ( bSpinDown )
        return;

    if ( !bCancel )
    {
        // determine if we should scroll the visible area
        if ( !mpSelectTable->empty() )
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

    mpOldSelectTable.reset();
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
                        maOldCurDate = maCurDate;
                        mpOldSelectTable.reset(new IntDateSet( *mpSelectTable ));

                        if ( !mbSelection )
                        {
                            mbDrag = true;
                            StartTracking();
                        }

                        ImplMouseSelect( aTempDate, nHitTest );
                    }
                    if (rMEvt.GetClicks() == 2)
                        maActivateHdl.Call(this);
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
            aNewDate.AddDays( -7 );
            break;

        case KEY_DOWN:
            aNewDate.AddDays( 7 );
            break;

        case KEY_PAGEUP:
            {
            Date aTempDate = aNewDate;
            aTempDate.AddDays( -(aNewDate.GetDay()+1) );
            aNewDate.AddDays( -aTempDate.GetDaysInMonth() );
            }
            break;

        case KEY_PAGEDOWN:
            aNewDate.AddDays( aNewDate.GetDaysInMonth() );
            break;

        case KEY_RETURN:
            break;

        default:
            Control::KeyInput( rKEvt );
            break;
    }

    if ( aNewDate != maCurDate )
    {
        SetCurDate( aNewDate );
        mbTravelSelect = true;
        Select();
        mbTravelSelect = false;
    }

    if (rKEvt.GetKeyCode().GetCode() == KEY_RETURN)
    {
        if (maActivateHdl.IsSet())
            maActivateHdl.Call(this);
        else
            Control::KeyInput(rKEvt);
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
            aDateRect.SetLeft( aPt.X() );
            aDateRect.SetTop( aPt.Y() );
            aPt = OutputToScreenPixel( aDateRect.BottomRight() );
            aDateRect.SetRight( aPt.X() );
            aDateRect.SetBottom( aPt.Y() );

            if ( rHEvt.GetMode() & HelpEventMode::QUICK )
            {
                maCalendarWrapper.setGregorianDateTime( aDate);
                sal_uInt16      nWeek = static_cast<sal_uInt16>(maCalendarWrapper.getValue( i18n::CalendarFieldIndex::WEEK_OF_YEAR));
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

    if ( maCurDate == rNewDate )
        return;

    bool bUpdate    = IsVisible() && IsUpdateMode();
    Date aOldDate   = maCurDate;
    maCurDate       = rNewDate;

    ImplCalendarSelectDate( mpSelectTable.get(), aOldDate, false );
    ImplCalendarSelectDate( mpSelectTable.get(), maCurDate, true );

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
            aFirstDate.AddDays( aFirstDate.GetDaysInMonth() );
            ++aTempDate;
            while ( nDateOff > aTempDate.GetDaysInMonth() )
            {
                aFirstDate.AddDays( aFirstDate.GetDaysInMonth() );
                sal_Int32 nDaysInMonth = aTempDate.GetDaysInMonth();
                aTempDate.AddDays( nDaysInMonth );
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

void Calendar::SetFirstDate( const Date& rNewFirstDate )
{
    if ( maFirstDate != rNewFirstDate )
    {
        maFirstDate = Date( 1, rNewFirstDate.GetMonth(), rNewFirstDate.GetYear() );
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
        aDate.AddDays( aDate.GetDaysInMonth() );
    --aDate;
    return aDate;
}

sal_uInt16 Calendar::GetMonthCount() const
{
    if ( mbFormat )
        return 1;
    else
        return static_cast<sal_uInt16>(mnMonthPerLine*mnLines);
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
    sal_Int32   nDaysOff;
    sal_uInt16  nDayIndex;
    Date    aDate = GetFirstMonth();

    if ( rDate < aDate )
    {
        aRect = GetDateRect( aDate );
        nDaysOff = aDate-rDate;
        nX = nDaysOff*mnDayWidth;
        aRect.AdjustLeft( -nX );
        aRect.AdjustRight( -nX );
        return aRect;
    }
    else
    {
        Date aLastDate = GetLastMonth();
        if ( rDate > aLastDate )
        {
            sal_Int32 nWeekDay = static_cast<sal_Int32>(aLastDate.GetDayOfWeek());
            nWeekDay = (nWeekDay+(7-ImplGetWeekStart())) % 7;
            aLastDate.AddDays( -nWeekDay );
            aRect = GetDateRect( aLastDate );
            nDaysOff = rDate-aLastDate;
            nDayIndex = 0;
            for ( sal_Int32 i = 0; i <= nDaysOff; i++ )
            {
                if ( aLastDate == rDate )
                {
                    aRect.AdjustLeft(nDayIndex*mnDayWidth );
                    aRect.SetRight( aRect.Left()+mnDayWidth );
                    return aRect;
                }
                if ( nDayIndex == 6 )
                {
                    nDayIndex = 0;
                    aRect.AdjustTop(mnDayHeight );
                    aRect.AdjustBottom(mnDayHeight );
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
                nDayIndex = static_cast<sal_uInt16>(aDate.GetDayOfWeek());
                nDayIndex = (nDayIndex+(7-static_cast<sal_uInt16>(ImplGetWeekStart()))) % 7;
                for ( sal_uInt16 nDay = 1; nDay <= nDaysInMonth; nDay++ )
                {
                    if ( nDay == rDate.GetDay() )
                    {
                        aRect.SetLeft( nDayX + (nDayIndex*mnDayWidth) );
                        aRect.SetTop( nDayY );
                        aRect.SetRight( aRect.Left()+mnDayWidth );
                        aRect.SetBottom( aRect.Top()+mnDayHeight );
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

            aDate.AddDays( nDaysInMonth );
            nX += mnMonthWidth;
        }

        nY += mnMonthHeight;
    }

    return aRect;
}

void Calendar::StartSelection()
{
    maOldCurDate = maCurDate;
    mpOldSelectTable.reset(new IntDateSet( *mpSelectTable ));

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
        mbSpinDown          = false;
        mbPrevIn            = false;
        mbNextIn            = false;
    }
}

Size Calendar::CalcWindowSizePixel() const
{
    OUString  const a99Text("99");
    vcl::Font aOldFont = GetFont();

    Size    aSize;
    long    n99TextWidth = GetTextWidth( a99Text );
    long    nTextHeight = GetTextHeight();

    aSize.AdjustWidth((n99TextWidth+DAY_OFFX)*7);
    aSize.AdjustWidth(MONTH_BORDERX*2 );

    aSize.setHeight( nTextHeight + TITLE_OFFY + (TITLE_BORDERY*2) );
    aSize.AdjustHeight(nTextHeight + WEEKDAY_OFFY );
    aSize.AdjustHeight((nTextHeight+DAY_OFFY)*6);
    aSize.AdjustHeight(MONTH_OFFY );

    return aSize;
}

Size Calendar::GetOptimalSize() const
{
    return CalcWindowSizePixel();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
