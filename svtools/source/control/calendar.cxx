/*************************************************************************
 *
 *  $RCSfile: calendar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nf $ $Date: 2000-10-18 11:23:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _APP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TABLE_HXX
#include <tools/table.hxx>
#endif
#ifndef _HELP_HXX
#include <vcl/help.hxx>
#endif
#ifndef _MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _DECOVIEW_HXX
#include <vcl/decoview.hxx>
#endif
#ifndef _FLOATWIN_HXX
#include <vcl/floatwin.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#define _SV_CALENDAR_CXX
#include <svtools.hrc>
#include <svtdata.hxx>
#include <calendar.hxx>

// =======================================================================

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

#define CALENDAR_HITTEST_DAY            ((USHORT)0x0001)
#define CALENDAR_HITTEST_WEEK           ((USHORT)0x0002)
#define CALENDAR_HITTEST_MONTHTITLE     ((USHORT)0x0004)
#define CALENDAR_HITTEST_PREV           ((USHORT)0x0008)
#define CALENDAR_HITTEST_NEXT           ((USHORT)0x0010)
#define CALENDAR_HITTEST_OUTSIDE        ((USHORT)0x1000)

#define MENU_YEAR_COUNT                 3

#define TABLE_DATE_SELECTED             ((void*)0x00000001)

// =======================================================================

struct ImplDateInfo
{
    XubString   maText;
    Color*      mpTextColor;
    Color*      mpFrameColor;
    USHORT      mnFlags;

                ImplDateInfo( const XubString& rText ) :
                    maText( rText )
                { mpTextColor = mpFrameColor = NULL; mnFlags = 0; }
                ~ImplDateInfo() { delete mpTextColor; delete mpFrameColor; }
};

DECLARE_TABLE( ImplDateTable, ImplDateInfo* );

// =======================================================================

static void ImplCalendarSelectDate( Table* pTable, const Date& rDate, BOOL bSelect )
{
    if ( bSelect )
        pTable->Insert( rDate.GetDate(), TABLE_DATE_SELECTED );
    else
        pTable->Remove( rDate.GetDate() );
}

// -----------------------------------------------------------------------

static void ImplCalendarSelectDateRange( Table* pTable,
                                         const Date& rStartDate,
                                         const Date& rEndDate,
                                         BOOL bSelect )
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
            pTable->Insert( aStartDate.GetDate(), TABLE_DATE_SELECTED );
            aStartDate++;
        }
    }
    else
    {
        void* p = pTable->First();
        while ( p )
        {
            Date aDate( pTable->GetCurKey() );
            if ( aDate > aEndDate )
                break;

            if ( aDate >= aStartDate )
                pTable->Remove( aDate.GetDate() );
            else
                p = pTable->Next();
        }
    }
}

// -----------------------------------------------------------------------

static void ImplCalendarUnSelectDateRange( Table* pTable,
                                           Table* pOldTable,
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

    void* p = pTable->First();
    while ( p )
    {
        Date aDate( pTable->GetCurKey() );
        if ( aDate > aEndDate )
            break;

        if ( aDate >= aStartDate )
            pTable->Remove( aDate.GetDate() );
        else
            p = pTable->Next();
    }

    p = pOldTable->First();
    while ( p )
    {
        Date aDate( pOldTable->GetCurKey() );
        if ( aDate > aEndDate )
            break;
        if ( aDate >= aStartDate )
            pTable->Insert( aDate.GetDate(), TABLE_DATE_SELECTED );

        p = pOldTable->Next();
    }
}

// -----------------------------------------------------------------------

inline void ImplCalendarClearSelectDate( Table* pTable )
{
    pTable->Clear();
}

// =======================================================================

void Calendar::ImplInit( WinBits nWinStyle )
{
    mpDateTable             = NULL;
    mpSelectTable           = new Table;
    mpOldSelectTable        = NULL;
    mpRestoreSelectTable    = NULL;
    mpStandardColor         = NULL;
    mpSaturdayColor         = NULL;
    mpSundayColor           = NULL;
    mnDayCount              = 0;
    mnWinStyle              = nWinStyle;
    mnFirstYear             = 0;
    mnLastYear              = 0;
    mnRequestYear           = 0;
    mbCalc                  = TRUE;
    mbFormat                = TRUE;
    mbDrag                  = FALSE;
    mbSelection             = FALSE;
    mbMultiSelection        = FALSE;
    mbWeekSel               = FALSE;
    mbUnSel                 = FALSE;
    mbMenuDown              = FALSE;
    mbSpinDown              = FALSE;
    mbPrevIn                = FALSE;
    mbNextIn                = FALSE;
    mbDirect                = FALSE;
    mbInSelChange           = FALSE;
    mbTravelSelect          = FALSE;
    mbScrollDateRange       = FALSE;
    mbSelLeft               = FALSE;
    mbAllSel                = FALSE;
    mbDropPos               = FALSE;
    SetFirstDate( maCurDate );
    ImplCalendarSelectDate( mpSelectTable, maCurDate, TRUE );

    // Sonstige Strings erzeugen
    maDayText = XubString( SvtResId( STR_SVT_CALENDAR_DAY ) );
    maWeekText = XubString( SvtResId( STR_SVT_CALENDAR_WEEK ) );

    // Tagestexte anlegen
    for ( USHORT i = 0; i < 31; i++ )
        mpDayText[i] = new UniString( UniString::CreateFromInt32( i+1 ) );

    maDragScrollTimer.SetTimeoutHdl( STATIC_LINK( this, Calendar, ScrollHdl ) );
    maDragScrollTimer.SetTimeout( GetSettings().GetMouseSettings().GetScrollRepeat() );
    mnDragScrollHitTest = 0;

    ImplInitSettings();
}

// -----------------------------------------------------------------------

void Calendar::ImplInitSettings()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    maSelColor = rStyleSettings.GetHighlightTextColor();
    SetPointFont( rStyleSettings.GetToolFont() );
    SetTextColor( rStyleSettings.GetFieldTextColor() );
    SetBackground( Wallpaper( rStyleSettings.GetFieldColor() ) );
}

// -----------------------------------------------------------------------

Calendar::Calendar( Window* pParent, WinBits nWinStyle ) :
    Control( pParent, nWinStyle & (WB_TABSTOP | WB_GROUP | WB_BORDER | WB_3DLOOK | WB_RANGESELECT | WB_MULTISELECT) ),
    maIntn( Application::GetAppInternational() ),
    maOldFormatFirstDate( 0, 0, 1900 ),
    maOldFormatLastDate( 0, 0, 1900 ),
    maFirstDate( 0, 0, 1900 ),
    maOldFirstDate( 0, 0, 1900 ),
    maOldCurDate( 0, 0, 1900 ),
    maAnchorDate( maCurDate ),
    maDropDate( 0, 0, 1900 )
{
    ImplInit( nWinStyle );
}

// -----------------------------------------------------------------------

Calendar::Calendar( Window* pParent, const ResId& rResId ) :
    Control( pParent, rResId ),
    maIntn( Application::GetAppInternational() ),
    maOldFormatFirstDate( 0, 0, 1900 ),
    maOldFormatLastDate( 0, 0, 1900 ),
    maFirstDate( 0, 0, 1900 ),
    maOldFirstDate( 0, 0, 1900 ),
    maOldCurDate( 0, 0, 1900 ),
    maAnchorDate( maCurDate ),
    maDropDate( 0, 0, 1900 )
{
    ImplInit( rResId.aWinBits );
}

// -----------------------------------------------------------------------

Calendar::~Calendar()
{
    delete mpStandardColor;
    delete mpSaturdayColor;
    delete mpSundayColor;

    if ( mpDateTable )
    {
        ImplDateInfo* pDateInfo = mpDateTable->First();
        while ( pDateInfo )
        {
            delete pDateInfo;
            pDateInfo = mpDateTable->Next();
        }

        delete mpDateTable;
    }

    delete mpSelectTable;
    if ( mpOldSelectTable )
        delete mpOldSelectTable;
    if ( mpRestoreSelectTable )
        delete mpRestoreSelectTable;

    for ( USHORT i = 0; i < 31; i++ )
        delete mpDayText[i];
}

// -----------------------------------------------------------------------

void Calendar::ImplGetWeekFont( Font& rFont ) const
{
    // Wochennummer geben wir in WEEKNUMBER_HEIGHT%-Fonthoehe aus
    Size aFontSize = rFont.GetSize();
    aFontSize.Height() *= WEEKNUMBER_HEIGHT;
    aFontSize.Height() /= 100;
    rFont.SetSize( aFontSize );
    rFont.SetWeight( WEIGHT_NORMAL );
}

// -----------------------------------------------------------------------

void Calendar::ImplFormat()
{
    if ( !mbFormat )
        return;

    DayOfWeek eStartDay = maIntn.GetWeekStart();

    if ( mbCalc )
    {
        Size aOutSize = GetOutputSizePixel();

        if ( (aOutSize.Width() <= 1) || (aOutSize.Height() <= 1) )
            return;

        XubString a99Text( XubString( RTL_CONSTASCII_USTRINGPARAM( "99" ) ) );

        Font aOldFont = GetFont();

        // Wochenanzeige beruecksichtigen
        if ( mnWinStyle & WB_WEEKNUMBER )
        {
            Font aTempFont = aOldFont;
            ImplGetWeekFont( aTempFont );
            SetFont( aTempFont );
            mnWeekWidth = GetTextWidth( a99Text )+WEEKNUMBER_OFFX;
            SetFont( aOldFont );
        }
        else
            mnWeekWidth = 0;

        if ( mnWinStyle & WB_BOLDTEXT )
        {
            Font aFont = aOldFont;
            if ( aFont.GetWeight() < WEIGHT_BOLD )
                aFont.SetWeight( WEIGHT_BOLD );
            else
                aFont.SetWeight( WEIGHT_NORMAL );
            SetFont( aFont );
        }

        long n99TextWidth = GetTextWidth( a99Text );
        long nTextHeight = GetTextHeight();

        // Breiten und X-Positionen berechnen
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

        // Hoehen und Y-Positionen berechnen
        mnDayHeight     = nTextHeight + DAY_OFFY;
        mnWeekDayOffY   = nTextHeight + TITLE_OFFY + (TITLE_BORDERY*2);
        mnDaysOffY      = mnWeekDayOffY + nTextHeight + WEEKDAY_OFFY;
        mnMonthHeight   = (mnDayHeight*6) + mnDaysOffY;
        mnMonthHeight  += MONTH_OFFY;
        mnLines         = aOutSize.Height() / mnMonthHeight;
        if ( !mnLines )
            mnLines = 1;
        mnMonthHeight  += (aOutSize.Height()-(mnLines*mnMonthHeight)) / mnLines;

        // Spinfelder berechnen
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

        // DayOffWeekText berechnen (werden im schmalen Font ausgegeben)
        maDayOfWeekText.Erase();
        long nStartOffX = 0;
        USHORT eDay = (USHORT)eStartDay;
        for ( USHORT nDayOfWeek = 0; nDayOfWeek < 7; nDayOfWeek++ )
        {
            String aDayOfWeek( maIntn.GetAbbrevDayText( (DayOfWeek)eDay ).GetChar( 0 ) );
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
            eDay++;
            eDay %= 7;
        }

        mbCalc = FALSE;
    }

    // Anzahl Tage berechnen
    USHORT nWeekDay;
    Date aTempDate = GetFirstMonth();
    maFirstDate = aTempDate;
    nWeekDay = (USHORT)aTempDate.GetDayOfWeek();
    nWeekDay = (nWeekDay+(7-(USHORT)eStartDay)) % 7;
    maFirstDate -= (ULONG)nWeekDay;
    mnDayCount = nWeekDay;
    USHORT nDaysInMonth;
    USHORT nMonthCount = (USHORT)(mnMonthPerLine*mnLines);
    for ( USHORT i = 0; i < nMonthCount; i++ )
    {
        nDaysInMonth = aTempDate.GetDaysInMonth();
        mnDayCount += nDaysInMonth;
        aTempDate += nDaysInMonth;
    }
    Date aTempDate2 = aTempDate;
    aTempDate2--;
    nDaysInMonth = aTempDate2.GetDaysInMonth();
    aTempDate2 -= nDaysInMonth-1;
    nWeekDay = (USHORT)aTempDate2.GetDayOfWeek();
    nWeekDay = (nWeekDay+(7-(USHORT)eStartDay)) % 7;
    mnDayCount += 42-nDaysInMonth-nWeekDay;

    // Farben festlegen
    maOtherColor = Color( COL_LIGHTGRAY );
    if ( maOtherColor.IsRGBEqual( GetBackground().GetColor() ) )
        maOtherColor.SetColor( COL_GRAY );

    Date aLastDate = GetLastDate();
    if ( (maOldFormatLastDate != aLastDate) ||
         (maOldFormatFirstDate != maFirstDate) )
    {
        maOldFormatFirstDate = maFirstDate;
        maOldFormatLastDate  = aLastDate;
        DateRangeChanged();
    }

    // DateInfo besorgen
    USHORT nNewFirstYear = maFirstDate.GetYear();
    USHORT nNewLastYear = GetLastDate().GetYear();
    if ( mnFirstYear )
    {
        if ( nNewFirstYear < mnFirstYear )
        {
            for ( mnRequestYear = nNewFirstYear; mnRequestYear < mnFirstYear; mnRequestYear++ )
                RequestDateInfo();
            mnFirstYear = nNewFirstYear;
        }
        if ( nNewLastYear > mnLastYear )
        {
            for ( mnRequestYear = mnLastYear; mnRequestYear < nNewLastYear; mnRequestYear++ )
                RequestDateInfo();
            mnLastYear = nNewLastYear;
        }
    }
    else
    {
        for ( mnRequestYear = nNewFirstYear; mnRequestYear < nNewLastYear; mnRequestYear++ )
            RequestDateInfo();
        mnFirstYear = nNewFirstYear;
        mnLastYear = nNewLastYear;
    }
    mnRequestYear = 0;

    mbFormat = FALSE;
}

// -----------------------------------------------------------------------

USHORT Calendar::ImplHitTest( const Point& rPos, Date& rDate ) const
{
    if ( mbFormat )
        return 0;

    if ( maPrevRect.IsInside( rPos ) )
        return CALENDAR_HITTEST_PREV;
    else if ( maNextRect.IsInside( rPos ) )
        return CALENDAR_HITTEST_NEXT;

    long        nX;
    long        nY;
    long        nOffX;
    long        nYMonth;
    USHORT      nDay;
    DayOfWeek   eStartDay = maIntn.GetWeekStart();

    rDate = GetFirstMonth();
    nY = 0;
    for ( long i = 0; i < mnLines; i++ )
    {
        if ( rPos.Y() < nY )
            return 0;

        nX = 0;
        nYMonth = nY+mnMonthHeight;
        for ( long j = 0; j < mnMonthPerLine; j++ )
        {
            if ( (rPos.X() < nX) && (rPos.Y() < nYMonth) )
                return 0;

            USHORT nDaysInMonth = rDate.GetDaysInMonth();

            // Entsprechender Monat gefunden
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
                    USHORT nDayIndex = (USHORT)rDate.GetDayOfWeek();
                    nDayIndex = (nDayIndex+(7-(USHORT)eStartDay)) % 7;
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
                        USHORT nWeekDay = (USHORT)rDate.GetDayOfWeek();
                        nWeekDay = (nWeekDay+(7-(USHORT)eStartDay)) % 7;
                        USHORT nDayCount = 42-nDaysInMonth-nWeekDay;
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

// -----------------------------------------------------------------------

static void ImplDrawSpinArrow( OutputDevice* pDev, const Rectangle& rRect,
                               BOOL bPrev )
{
    long    i;
    long    n;
    long    nLines;
    long    nHeight = rRect.GetHeight();
    long    nWidth = rRect.GetWidth();
    if ( nWidth < nHeight )
        n = nWidth;
    else
        n = nHeight;
    if ( !(n & 0x01) )
        n--;
    nLines = n/2;

    Rectangle aRect( Point( rRect.Left()+(nWidth/2)-(nLines/2),
                            rRect.Top()+(nHeight/2) ),
                     Size( 1, 1 ) );
    if ( !bPrev )
    {
        aRect.Left()  += nLines;
        aRect.Right() += nLines;
    }

    pDev->DrawRect( aRect );
    for ( i = 0; i < nLines; i++ )
    {
        if ( bPrev )
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
        pDev->DrawRect( aRect );
    }
}

// -----------------------------------------------------------------------

void Calendar::ImplDrawSpin( BOOL bDrawPrev, BOOL bDrawNext )
{
    if ( !bDrawPrev && !bDrawNext )
        return;

    SetLineColor();
    SetFillColor( GetSettings().GetStyleSettings().GetButtonTextColor() );
    if ( bDrawPrev )
    {
        Rectangle aOutRect = maPrevRect;
        aOutRect.Left()   += 3;
        aOutRect.Top()    += 3;
        aOutRect.Right()  -= 3;
        aOutRect.Bottom() -= 3;
        ImplDrawSpinArrow( this, aOutRect, TRUE );
    }
    if ( bDrawNext )
    {
        Rectangle aOutRect = maNextRect;
        aOutRect.Left()   += 3;
        aOutRect.Top()    += 3;
        aOutRect.Right()  -= 3;
        aOutRect.Bottom() -= 3;
        ImplDrawSpinArrow( this, aOutRect, FALSE );
    }
}

// -----------------------------------------------------------------------

void Calendar::ImplDrawDate( long nX, long nY,
                             USHORT nDay, USHORT nMonth, USHORT nYear,
                             DayOfWeek eDayOfWeek,
                             BOOL bBack, BOOL bOther, ULONG nToday )
{
    ImplDateInfo*   pDateInfo;
    Color*          pTextColor = NULL;
    const String&   rDay = *(mpDayText[nDay-1]);
    Rectangle       aDateRect( nX, nY, nX+mnDayWidth-1, nY+mnDayHeight-1 );

    BOOL bSel = FALSE;
    BOOL bFocus = FALSE;
    // Aktueller Tag
    if ( (nDay   == maCurDate.GetDay()) &&
         (nMonth == maCurDate.GetMonth()) &&
         (nYear  == maCurDate.GetYear()) )
        bFocus = TRUE;
    if ( mpSelectTable )
    {
        if ( mpSelectTable->IsKeyValid( Date( nDay, nMonth, nYear ).GetDate() ) )
            bSel = TRUE;
    }

    // Dateinfo ermitteln
    if ( mpDateTable )
    {
        pDateInfo = mpDateTable->Get( Date( nDay, nMonth, nYear ).GetDate() );
        if ( !pDateInfo )
            pDateInfo = mpDateTable->Get( Date( nDay, nMonth, 0 ).GetDate() );
    }
    else
        pDateInfo = NULL;

    // Textfarbe ermitteln
    if ( bSel )
        pTextColor = &maSelColor;
    else if ( bOther )
        pTextColor = &maOtherColor;
    else
    {
        if ( pDateInfo && pDateInfo->mpTextColor )
            pTextColor = pDateInfo->mpTextColor;
        else
        {
            if ( eDayOfWeek == SATURDAY )
                pTextColor = mpSaturdayColor;
            else if ( eDayOfWeek == SUNDAY )
                pTextColor = mpSundayColor;
            if ( !pTextColor )
                pTextColor = mpStandardColor;
        }
    }

    if ( bFocus )
        HideFocus();

    // Font ermitteln
    Font aOldFont = GetFont();
    BOOL bBoldFont = FALSE;
    if ( (mnWinStyle & WB_BOLDTEXT) &&
         pDateInfo && (pDateInfo->mnFlags & DIB_BOLD) )
    {
        bBoldFont = TRUE;
        Font aFont = aOldFont;
        if ( aFont.GetWeight() < WEIGHT_BOLD )
            aFont.SetWeight( WEIGHT_BOLD );
        else
            aFont.SetWeight( WEIGHT_NORMAL );
        SetFont( aFont );
    }

    // Hintergrund ausgeben
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if ( bSel || bBack )
    {
        if ( bSel )
        {
            SetLineColor();
            SetFillColor( rStyleSettings.GetHighlightColor() );
            DrawRect( aDateRect );
        }
        else
            Erase( aDateRect );
    }

    // Text ausgeben
    long nTextX = nX+(mnDayWidth-GetTextWidth( rDay ))-(DAY_OFFX/2);
    long nTextY = nY+(mnDayHeight-GetTextHeight())/2;
    if ( pTextColor )
    {
        Color aOldColor = GetTextColor();
        SetTextColor( *pTextColor );
        DrawText( Point( nTextX, nTextY ), rDay );
        SetTextColor( aOldColor );
    }
    else
        DrawText( Point( nTextX, nTextY ), rDay );

    // Heute
    Date aTodayDate( maCurDate );
    if ( nToday )
        aTodayDate.SetDate( nToday );
    else
        aTodayDate = Date();
    if ( (nDay   == aTodayDate.GetDay()) &&
         (nMonth == aTodayDate.GetMonth()) &&
         (nYear  == aTodayDate.GetYear()) )
    {
        SetLineColor( rStyleSettings.GetWindowTextColor() );
        SetFillColor();
        DrawRect( aDateRect );
    }

    // Evt. DateInfo ausgeben
    if ( (mnWinStyle & WB_FRAMEINFO) && pDateInfo && pDateInfo->mpFrameColor )
    {
        SetLineColor( *(pDateInfo->mpFrameColor) );
        SetFillColor();
        Rectangle aFrameRect( aDateRect );
        aFrameRect.Left()++;
        aFrameRect.Top()++;
        long nFrameWidth = aFrameRect.GetWidth();
        long nFrameHeight = aFrameRect.GetHeight();
        long nFrameOff;
        if ( nFrameWidth < nFrameHeight )
        {
            nFrameOff = nFrameHeight-nFrameWidth;
            aFrameRect.Top() += nFrameOff/2;
            nFrameOff %= 2;
            aFrameRect.Bottom() -= nFrameOff;
        }
        else if ( nFrameWidth > nFrameHeight )
        {
            nFrameOff = nFrameWidth-nFrameHeight;
            aFrameRect.Left() += nFrameOff/2;
            nFrameOff %= 2;
            aFrameRect.Right() -= nFrameOff;
        }
        DrawEllipse( aFrameRect );
    }

    // Evt. noch FocusRect
    if ( bFocus && HasFocus() )
        ShowFocus( aDateRect );

    if( mbDropPos && maDropDate == Date( nDay, nMonth, nYear ) )
        ImplInvertDropPos();

    if ( bBoldFont )
        SetFont( aOldFont );
}

// -----------------------------------------------------------------------

void Calendar::ImplDraw( BOOL bPaint )
{
    ImplFormat();

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    Size        aOutSize = GetOutputSizePixel();
    long        i;
    long        j;
    long        nX;
    long        nY;
    long        nOffX;
    long        nOffY;
    long        nDayX;
    long        nDayY;
    ULONG       nToday = Date().GetDate();
    USHORT      nDay;
    USHORT      nMonth;
    USHORT      nYear;
    Date        aDate = GetFirstMonth();
    DayOfWeek   eStartDay = maIntn.GetWeekStart();

    HideFocus();

    nY = 0;
    for ( i = 0; i < mnLines; i++ )
    {
        // Titleleiste ausgeben
        SetLineColor();
        SetFillColor( rStyleSettings.GetFaceColor() );
        Rectangle aTitleRect( 0, nY, aOutSize.Width()-1, nY+mnDayHeight-DAY_OFFY+TITLE_BORDERY*2 );
        if ( !bPaint )
        {
            Rectangle aTempRect( 1, aTitleRect.Top()+TITLE_BORDERY,
                                 aOutSize.Width()-2,
                                 aTitleRect.Bottom()-TITLE_BORDERY );
            if ( !i )
            {
                aTempRect.Left()  = maPrevRect.Right()+1;
                aTempRect.Right() = maNextRect.Left()-1;
            }
            DrawRect( aTempRect );
        }
        else
        {
            DrawRect( aTitleRect );
            Point aTopLeft1( aTitleRect.Left(), aTitleRect.Top() );
            Point aTopLeft2( aTitleRect.Left(), aTitleRect.Top()+1 );
            Point aBottomRight1( aTitleRect.Right(), aTitleRect.Bottom() );
            Point aBottomRight2( aTitleRect.Right(), aTitleRect.Bottom()-1 );
            SetLineColor( rStyleSettings.GetDarkShadowColor() );
            DrawLine( aTopLeft1, Point( aBottomRight1.X(), aTopLeft1.Y() ) );
            SetLineColor( rStyleSettings.GetLightColor() );
            DrawLine( aTopLeft2, Point( aBottomRight2.X(), aTopLeft2.Y() ) );
            DrawLine( aTopLeft2, Point( aTopLeft2.X(), aBottomRight2.Y() ) );
            SetLineColor( rStyleSettings.GetShadowColor() );
            DrawLine( Point( aTopLeft2.X(), aBottomRight2.Y() ), aBottomRight2 );
            DrawLine( Point( aBottomRight2.X(), aTopLeft2.Y() ), aBottomRight2 );
            SetLineColor( rStyleSettings.GetDarkShadowColor() );
            DrawLine( Point( aTopLeft1.X(), aBottomRight1.Y() ), aBottomRight1 );
        }
        Point aSepPos1( 0, aTitleRect.Top()+TITLE_BORDERY );
        Point aSepPos2( 0, aTitleRect.Bottom()-TITLE_BORDERY );
        for ( j = 0; j < mnMonthPerLine-1; j++ )
        {
            aSepPos1.X() += mnMonthWidth-1;
            aSepPos2.X() = aSepPos1.X();
            SetLineColor( rStyleSettings.GetShadowColor() );
            DrawLine( aSepPos1, aSepPos2 );
            aSepPos1.X()++;
            aSepPos2.X() = aSepPos1.X();
            SetLineColor( rStyleSettings.GetLightColor() );
            DrawLine( aSepPos1, aSepPos2 );
        }

        nX = 0;
        for ( j = 0; j < mnMonthPerLine; j++ )
        {
            nMonth  = aDate.GetMonth();
            nYear   = aDate.GetYear();

            // Monat in der Titleleiste ausgeben
            nOffX = nX;
            nOffY = nY+TITLE_BORDERY;
            String aMonthText( maIntn.GetMonthText( nMonth ) );
            aMonthText += ' ';
            aMonthText += String::CreateFromInt64( nYear );
            long nMonthTextWidth = GetTextWidth( aMonthText );
            long nMonthOffX1 = 0;
            long nMonthOffX2 = 0;
            if ( i == 0 )
            {
                if ( j == 0 )
                    nMonthOffX1 = maPrevRect.Right()+1;
                if ( j == mnMonthPerLine-1 )
                    nMonthOffX2 = aOutSize.Width()-maNextRect.Left()+1;
            }
            long nMaxMonthWidth = mnMonthWidth-nMonthOffX1-nMonthOffX2-4;
            if ( nMonthTextWidth > nMaxMonthWidth )
            {
                aMonthText  = maIntn.GetAbbrevMonthText( nMonth );
                aMonthText += ' ';
                aMonthText += String::CreateFromInt64( nYear );
                nMonthTextWidth = GetTextWidth( aMonthText );
            }
            long nTempOff = (mnMonthWidth-nMonthTextWidth+1)/2;
            if ( nTempOff < nMonthOffX1 )
                nOffX += nMonthOffX1+1;
            else
            {
                if ( nTempOff+nMonthTextWidth > mnMonthWidth-nMonthOffX2 )
                    nOffX += mnMonthWidth-nMonthOffX2-nMonthTextWidth;
                else
                    nOffX += nTempOff;
            }
            SetTextColor( rStyleSettings.GetButtonTextColor() );
            DrawText( Point( nOffX, nOffY ), aMonthText );
            SetTextColor( rStyleSettings.GetWindowTextColor() );

            // Weekleiste ausgeben
            if ( bPaint )
            {
                nDayX = nX+mnDaysOffX;
                nDayY = nY+mnWeekDayOffY;
                nOffY = nDayY + mnDayHeight;
                SetLineColor( rStyleSettings.GetWindowTextColor() );
                Point aStartPos( nDayX, nOffY );
                if ( mnWinStyle & WB_WEEKNUMBER )
                    aStartPos.X() -= WEEKNUMBER_OFFX-2;
                DrawLine( aStartPos, Point( nDayX+(7*mnDayWidth), nOffY ) );
                DrawTextArray( Point( nDayX+mnDayOfWeekAry[0], nDayY ), maDayOfWeekText, &(mnDayOfWeekAry[1]) );
            }

            // Week-Numbers ausgeben
            if ( mnWinStyle & WB_WEEKNUMBER )
            {
                nDayX = nX+mnDaysOffX;
                nDayY = nY+mnWeekDayOffY;
                nOffY = nDayY + mnDayHeight;
                long nMonthHeight = mnDayHeight*6;
                if ( bPaint )
                    DrawLine( Point( nDayX-WEEKNUMBER_OFFX+2, nOffY ), Point( nDayX-WEEKNUMBER_OFFX+2, nOffY+nMonthHeight ) );
                else
                    Erase( Rectangle( nDayX-mnWeekWidth-WEEKNUMBER_OFFX, nOffY, nDayX-WEEKNUMBER_OFFX-1, nOffY+nMonthHeight ) );

                Font aOldFont = GetFont();
                Font aTempFont = aOldFont;
                ImplGetWeekFont( aTempFont );
                SetFont( aTempFont );
                nDayX -= mnWeekWidth;
                nDayY = nY+mnDaysOffY;
                Date aTempDate = aDate;
                for ( USHORT nWeekCount = 0; nWeekCount < 6; nWeekCount++ )
                {
                    String  aWeekText( aTempDate.GetWeekOfYear( eStartDay, maIntn.GetWeekCountStart() ) );
                    long    nOffX = (mnWeekWidth-WEEKNUMBER_OFFX)-GetTextWidth( aWeekText );
                    long    nOffY = (mnDayHeight-GetTextHeight())/2;
                    DrawText( Point( nDayX+nOffX, nDayY+nOffY ), aWeekText );
                    nDayY += mnDayHeight;
                    aTempDate += 7;
                }
                SetFont( aOldFont );
            }

            // Tage ausgeben
            USHORT nDaysInMonth = aDate.GetDaysInMonth();
            nDayX = nX+mnDaysOffX;
            nDayY = nY+mnDaysOffY;
            if ( !bPaint )
            {
                Rectangle aClearRect( nDayX, nDayY,
                                      nDayX+(7*mnDayWidth)-1, nDayY+(6*mnDayHeight)-1 );
                Erase( aClearRect );
            }
            USHORT nDayIndex = (USHORT)aDate.GetDayOfWeek();
            nDayIndex = (nDayIndex+(7-(USHORT)eStartDay)) % 7;
            if ( (i == 0) && (j == 0) )
            {
                Date aTempDate = aDate;
                aTempDate -= nDayIndex;
                for ( nDay = 0; nDay < nDayIndex; nDay++ )
                {
                    nOffX = nDayX + (nDay*mnDayWidth);
                    ImplDrawDate( nOffX, nDayY, nDay+aTempDate.GetDay(),
                                  aTempDate.GetMonth(), aTempDate.GetYear(),
                                  (DayOfWeek)((nDay+(USHORT)eStartDay)%7), FALSE, TRUE, nToday );
                }
            }
            for ( nDay = 1; nDay <= nDaysInMonth; nDay++ )
            {
                nOffX = nDayX + (nDayIndex*mnDayWidth);
                ImplDrawDate( nOffX, nDayY, nDay, nMonth, nYear,
                              (DayOfWeek)((nDayIndex+(USHORT)eStartDay)%7),
                              FALSE, FALSE, nToday );
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
                USHORT nWeekDay = (USHORT)aDate.GetDayOfWeek();
                nWeekDay = (nWeekDay+(7-(USHORT)eStartDay)) % 7;
                USHORT nDayCount = 42-nDaysInMonth-nWeekDay;
                Date aTempDate = aDate;
                aTempDate += nDaysInMonth;
                for ( nDay = 1; nDay <= nDayCount; nDay++ )
                {
                    nOffX = nDayX + (nDayIndex*mnDayWidth);
                    ImplDrawDate( nOffX, nDayY, nDay,
                                  aTempDate.GetMonth(), aTempDate.GetYear(),
                                  (DayOfWeek)((nDayIndex+(USHORT)eStartDay)%7),
                                  FALSE, TRUE, nToday );
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

    // Spin-Buttons zeichnen
    if ( bPaint )
        ImplDrawSpin();
}

// -----------------------------------------------------------------------

void Calendar::ImplUpdateDate( const Date& rDate )
{
    if ( IsReallyVisible() && IsUpdateMode() )
    {
        Rectangle aDateRect( GetDateRect( rDate ) );
        if ( !aDateRect.IsEmpty() )
        {
            BOOL bOther = (rDate < GetFirstMonth()) || (rDate > GetLastMonth());
            ImplDrawDate( aDateRect.Left(), aDateRect.Top(),
                          rDate.GetDay(), rDate.GetMonth(), rDate.GetYear(),
                          rDate.GetDayOfWeek(), TRUE, bOther );
        }
    }
}

// -----------------------------------------------------------------------

void Calendar::ImplUpdateSelection( Table* pOld )
{
    Table*  pNew = mpSelectTable;
    void*   p;
    ULONG   nKey;

    p = pOld->First();
    while ( p )
    {
        nKey = pOld->GetCurKey();
        if ( !pNew->Get( nKey ) )
        {
            Date aTempDate( nKey );
            ImplUpdateDate( aTempDate );
        }

        p = pOld->Next();
    }

    p = pNew->First();
    while ( p )
    {
        nKey = pNew->GetCurKey();
        if ( !pOld->Get( nKey ) )
        {
            Date aTempDate( nKey );
            ImplUpdateDate( aTempDate );
        }

        p = pNew->Next();
    }
}

// -----------------------------------------------------------------------

void Calendar::ImplMouseSelect( const Date& rDate, USHORT nHitTest,
                                BOOL bMove, BOOL bExpand, BOOL bExtended )
{
    Table*  pOldSel = new Table( *mpSelectTable );
    Date    aOldDate = maCurDate;
    Date    aTempDate = rDate;

    if ( !(nHitTest & CALENDAR_HITTEST_DAY) )
        aTempDate--;

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
                        ImplCalendarSelectDateRange( mpSelectTable, Date( 1, 1, 0 ), aTempDate, FALSE );
                        ImplCalendarSelectDateRange( mpSelectTable, maAnchorDate, Date( 31, 12, 9999 ), FALSE );
                    }
                    else
                    {
                        ImplCalendarSelectDateRange( mpSelectTable, Date( 1, 1, 0 ), maAnchorDate, FALSE );
                        ImplCalendarSelectDateRange( mpSelectTable, aTempDate, Date( 31, 12, 9999 ), FALSE );
                    }
                }
                ImplCalendarSelectDateRange( mpSelectTable, aTempDate, maAnchorDate, TRUE );
            }
            else if ( bExtended && !(mnWinStyle & WB_RANGESELECT) )
            {
                maAnchorDate = aTempDate;
                if ( IsDateSelected( aTempDate ) )
                {
                    mbUnSel = TRUE;
                    ImplCalendarSelectDate( mpSelectTable, aTempDate, FALSE );
                }
                else
                {
                    ImplCalendarSelectDate( mpSelectTable, aTempDate, TRUE );
                }
            }
            else
            {
                maAnchorDate = aTempDate;
                ImplCalendarClearSelectDate( mpSelectTable );
                ImplCalendarSelectDate( mpSelectTable, aTempDate, TRUE );
            }

            mpRestoreSelectTable = new Table( *mpSelectTable );
        }
    }
    else
    {
        if ( aTempDate < maCurDate )
            mbSelLeft = TRUE;
        else
            mbSelLeft = FALSE;
        if ( !(nHitTest & CALENDAR_HITTEST_DAY) )
            aTempDate = maOldCurDate;
        if ( !bMove )
            maAnchorDate = aTempDate;
        if ( aTempDate != maCurDate )
        {
            maCurDate = aTempDate;
            ImplCalendarSelectDate( mpSelectTable, aOldDate, FALSE );
            ImplCalendarSelectDate( mpSelectTable, maCurDate, TRUE );
        }
    }

    BOOL bNewSel = *pOldSel != *mpSelectTable;
    if ( (maCurDate != aOldDate) || bNewSel )
    {
        if ( bNewSel )
        {
            mbInSelChange = TRUE;
            SelectionChanging();
            mbInSelChange = FALSE;
        }
        HideFocus();
        if ( bNewSel )
            ImplUpdateSelection( pOldSel );
        if ( !bNewSel || !pOldSel->Get( aOldDate.GetDate() ) )
            ImplUpdateDate( aOldDate );
        // Damit Focus-Rechteck auch wieder neu ausgegeben wird
        if ( HasFocus() || !bNewSel || !mpSelectTable->Get( maCurDate.GetDate() ) )
            ImplUpdateDate( maCurDate );
    }
    delete pOldSel;
}

// -----------------------------------------------------------------------

void Calendar::ImplUpdate( BOOL bCalcNew )
{
    if ( IsReallyVisible() && IsUpdateMode() )
    {
        if ( bCalcNew && !mbCalc )
            Invalidate();
        else if ( !mbFormat && !mbCalc )
        {
            if ( mbDirect )
            {
                mbFormat = TRUE;
                ImplDraw( FALSE );
                return;
            }
            else
                Invalidate();
        }
    }

    if ( bCalcNew )
        mbCalc = TRUE;
    mbFormat = TRUE;
}

// -----------------------------------------------------------------------

void Calendar::ImplInvertDropPos()
{
    Rectangle aRect = GetDateRect( maDropDate );//this is one Pixel to width and one to heigh
    aRect.Bottom() = aRect.Top()+mnDayHeight-1;
    aRect.Right() = aRect.Left()+mnDayWidth-1;
    Invert( aRect );
}

// -----------------------------------------------------------------------

void Calendar::ImplScroll( BOOL bPrev )
{
    Date aNewFirstMonth = GetFirstMonth();
    if ( bPrev )
    {
        aNewFirstMonth--;
        aNewFirstMonth -= aNewFirstMonth.GetDaysInMonth()-1;
    }
    else
        aNewFirstMonth += aNewFirstMonth.GetDaysInMonth();
    mbDirect = TRUE;
    SetFirstDate( aNewFirstMonth );
    mbDirect = FALSE;
}

// -----------------------------------------------------------------------

void Calendar::ImplShowMenu( const Point& rPos, const Date& rDate )
{
    EndSelection();

    Date        aOldFirstDate = GetFirstMonth();
    PopupMenu   aPopupMenu;
    PopupMenu*  pYearPopupMenus[MENU_YEAR_COUNT];
    USHORT      nMonthOff;
    USHORT      nCurItemId;
    USHORT      nYear = rDate.GetYear()-1;
    USHORT      i;
    USHORT      j;
    USHORT      nYearIdCount = 1000;

    nMonthOff = (rDate.GetYear()-aOldFirstDate.GetYear())*12;
    if ( aOldFirstDate.GetMonth() < rDate.GetMonth() )
        nMonthOff += rDate.GetMonth()-aOldFirstDate.GetMonth();
    else
        nMonthOff -= aOldFirstDate.GetMonth()-rDate.GetMonth();

    // Menu aufbauen (Jahre mit verschiedenen Monaten aufnehmen)
    for ( i = 0; i < MENU_YEAR_COUNT; i++ )
    {
        pYearPopupMenus[i] = new PopupMenu;
        for ( j = 1; j <= 12; j++ )
            pYearPopupMenus[i]->InsertItem( nYearIdCount+j, maIntn.GetMonthText( j ) );
        aPopupMenu.InsertItem( 10+i, UniString::CreateFromInt32( nYear+i ) );
        aPopupMenu.SetPopupMenu( 10+i, pYearPopupMenus[i] );
        nYearIdCount += 1000;
    }

    mbMenuDown = TRUE;
    nCurItemId = aPopupMenu.Execute( this, rPos );
    mbMenuDown = FALSE;

    // Menu zerstoeren
    aPopupMenu.SetPopupMenu( 2, NULL );
    for ( i = 0; i < MENU_YEAR_COUNT; i++ )
    {
        aPopupMenu.SetPopupMenu( 10+i, NULL );
        delete pYearPopupMenus[i];
    }

    if ( nCurItemId )
    {
        USHORT nTempMonthOff = nMonthOff % 12;
        USHORT nTempYearOff = nMonthOff / 12;
        USHORT nNewMonth = nCurItemId % 1000;
        USHORT nNewYear = nYear+((nCurItemId-1000)/1000);
        if ( nTempMonthOff < nNewMonth )
            nNewMonth -= nTempMonthOff;
        else
        {
            nNewYear--;
            nNewMonth = 12-(nTempMonthOff-nNewMonth);
        }
        nNewYear -= nTempYearOff;
        SetFirstDate( Date( 1, nNewMonth, nNewYear ) );
    }
}

// -----------------------------------------------------------------------

void Calendar::ImplTracking( const Point& rPos, BOOL bRepeat )
{
    Date    aTempDate = maCurDate;
    USHORT  nHitTest = ImplHitTest( rPos, aTempDate );

    if ( mbSpinDown )
    {
        mbPrevIn = (nHitTest & CALENDAR_HITTEST_PREV) != 0;
        mbNextIn = (nHitTest & CALENDAR_HITTEST_NEXT) != 0;

        if ( bRepeat && (mbPrevIn || mbNextIn) )
        {
            mbScrollDateRange = TRUE;
            ImplScroll( mbPrevIn );
            mbScrollDateRange = FALSE;
        }
    }
    else
        ImplMouseSelect( aTempDate, nHitTest, TRUE, FALSE, FALSE );
}

// -----------------------------------------------------------------------

void Calendar::ImplEndTracking( const Point& rPos, BOOL bCancel )
{
    BOOL bSelection = mbSelection;
    BOOL bSpinDown = mbSpinDown;

    mbDrag              = FALSE;
    mbSelection         = FALSE;
    mbMultiSelection    = FALSE;
    mbUnSel             = FALSE;
    mbSpinDown          = FALSE;
    mbPrevIn            = FALSE;
    mbNextIn            = FALSE;

    if ( bCancel )
    {
        if ( maOldFirstDate != maFirstDate )
            SetFirstDate( maOldFirstDate );

        if ( !bSpinDown )
        {
            Table*  pOldSel = new Table( *mpSelectTable );
            Date    aOldDate = maCurDate;
            maCurDate       = maOldCurDate;
            *mpSelectTable  = *mpOldSelectTable;
            HideFocus();
            ImplUpdateSelection( pOldSel );
            if ( !pOldSel->Get( aOldDate.GetDate() ) )
                ImplUpdateDate( aOldDate );
            // Damit Focus-Rechteck auch wieder neu ausgegeben wird
            if ( HasFocus() || !mpSelectTable->Get( maCurDate.GetDate() ) )
                ImplUpdateDate( maCurDate );
            delete pOldSel;
        }
    }

    if ( !bSpinDown )
    {
        if ( !bCancel )
        {
            // Feststellen, ob wir sichtbaren Bereich scrollen sollen
            ULONG nSelCount = mpSelectTable->Count();
            if ( nSelCount )
            {
                Date aFirstSelDate( mpSelectTable->GetObjectKey( 0 ) );
                Date aLastSelDate( mpSelectTable->GetObjectKey( nSelCount-1 ) );
                if ( aLastSelDate < GetFirstMonth() )
                    ImplScroll( TRUE );
                else if ( GetLastMonth() < aFirstSelDate )
                    ImplScroll( FALSE );
            }
        }

        if ( mbAllSel ||
             (!bCancel && ((maCurDate != maOldCurDate) || (*mpOldSelectTable != *mpSelectTable))) )
            Select();

        if ( !bSelection && (mnWinStyle & WB_TABSTOP) && !bCancel )
            GrabFocus();

        delete mpOldSelectTable;
        mpOldSelectTable = NULL;
        delete mpRestoreSelectTable;
        mpRestoreSelectTable = NULL;
    }
}

// -----------------------------------------------------------------------

IMPL_STATIC_LINK( Calendar, ScrollHdl, Timer*, pTimer )
{
    BOOL bPrevIn = (pThis->mnDragScrollHitTest & CALENDAR_HITTEST_PREV) != 0;
    BOOL bNextIn = (pThis->mnDragScrollHitTest & CALENDAR_HITTEST_NEXT) != 0;
    if( bNextIn || bPrevIn )
    {
        pThis->mbScrollDateRange = TRUE;
        pThis->ImplScroll( bPrevIn );
        pThis->mbScrollDateRange = FALSE;
    }
    return 0;
}

// -----------------------------------------------------------------------

void Calendar::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && !mbMenuDown )
    {
        Date    aTempDate = maCurDate;
        USHORT  nHitTest = ImplHitTest( rMEvt.GetPosPixel(), aTempDate );
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
                    mbSpinDown = TRUE;
                    mbScrollDateRange = TRUE;
                    ImplScroll( mbPrevIn );
                    mbScrollDateRange = FALSE;
                    // Hier muss BUTTONREPEAT stehen, also nicht wieder
                    // auf SCROLLREPEAT aendern, sondern mit TH abklaeren,
                    // warum es evtl. anders sein sollte (71775)
                    StartTracking( STARTTRACK_BUTTONREPEAT );
                }
                else
                {
                    if ( (rMEvt.GetClicks() == 2) && (nHitTest & CALENDAR_HITTEST_DAY) )
                        DoubleClick();
                    else
                    {
                        if ( mpOldSelectTable )
                            delete mpOldSelectTable;
                        maOldCurDate = maCurDate;
                        mpOldSelectTable = new Table( *mpSelectTable );

                        if ( !mbSelection )
                        {
                            mbDrag = TRUE;
                            StartTracking();
                        }

                        mbMultiSelection = (mnWinStyle & (WB_MULTISELECT | WB_RANGESELECT)) != 0;
                        if ( (nHitTest & CALENDAR_HITTEST_DAY) && mbMultiSelection )
                            mbWeekSel = TRUE;
                        else
                            mbWeekSel = FALSE;
                        ImplMouseSelect( aTempDate, nHitTest, FALSE, rMEvt.IsShift(), rMEvt.IsMod1() );
                    }
                }
            }
        }

        return;
    }

    Control::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void Calendar::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && mbSelection )
        ImplEndTracking( rMEvt.GetPosPixel(), FALSE );
    else
        Control::MouseButtonUp( rMEvt );
}

// -----------------------------------------------------------------------

void Calendar::MouseMove( const MouseEvent& rMEvt )
{
    if ( mbSelection && rMEvt.GetButtons() )
        ImplTracking( rMEvt.GetPosPixel(), FALSE );
    else
        Control::MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void Calendar::Tracking( const TrackingEvent& rTEvt )
{
    Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();

    if ( rTEvt.IsTrackingEnded() )
        ImplEndTracking( aMousePos, rTEvt.IsTrackingCanceled() );
    else
        ImplTracking( aMousePos, rTEvt.IsTrackingRepeat() );
}

// -----------------------------------------------------------------------

void Calendar::KeyInput( const KeyEvent& rKEvt )
{
    Date    aNewDate = maCurDate;
    BOOL    bMultiSel = (mnWinStyle & (WB_RANGESELECT | WB_MULTISELECT)) != 0;
    BOOL    bExpand = rKEvt.GetKeyCode().IsShift();
    BOOL    bExtended = rKEvt.GetKeyCode().IsMod1();

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_HOME:
            aNewDate.SetDay( 1 );
            break;

        case KEY_END:
            aNewDate.SetDay( aNewDate.GetDaysInMonth() );
            break;

        case KEY_LEFT:
            aNewDate--;
            break;

        case KEY_RIGHT:
            aNewDate++;
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
                    BOOL bDateSel = IsDateSelected( maCurDate );
                    SelectDate( maCurDate, !bDateSel );
                    mbSelLeft = FALSE;
                    SelectionChanging();
                    mbTravelSelect = TRUE;
                    Select();
                    mbTravelSelect = FALSE;
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
            Table* pOldSel = new Table( *mpSelectTable );
            Date aOldAnchorDate = maAnchorDate;
            mbSelLeft = aNewDate < maAnchorDate;
            if ( !bExtended )
            {
                if ( mbSelLeft )
                {
                    ImplCalendarSelectDateRange( mpSelectTable, Date( 1, 1, 0 ), aNewDate, FALSE );
                    ImplCalendarSelectDateRange( mpSelectTable, maAnchorDate, Date( 31, 12, 9999 ), FALSE );
                }
                else
                {
                    ImplCalendarSelectDateRange( mpSelectTable, Date( 1, 1, 0 ), maAnchorDate, FALSE );
                    ImplCalendarSelectDateRange( mpSelectTable, aNewDate, Date( 31, 12, 9999 ), FALSE );
                }
            }
            ImplCalendarSelectDateRange( mpSelectTable, aNewDate, maAnchorDate, TRUE );
            mbDirect = TRUE;
            SetCurDate( aNewDate );
            mbDirect = FALSE;
            maAnchorDate = aOldAnchorDate;
            mbInSelChange = TRUE;
            SelectionChanging();
            mbInSelChange = FALSE;
            ImplUpdateSelection( pOldSel );
        }
        else
        {
            if ( mnWinStyle & WB_RANGESELECT )
            {
                SetNoSelection();
                SelectDate( aNewDate, TRUE );
            }
            mbDirect = TRUE;
            SetCurDate( aNewDate );
            mbDirect = FALSE;
        }
        mbTravelSelect = TRUE;
        Select();
        mbTravelSelect = FALSE;
    }
}

// -----------------------------------------------------------------------

void Calendar::Paint( const Rectangle& rRect )
{
    ImplDraw( TRUE );
}

// -----------------------------------------------------------------------

void Calendar::GetFocus()
{
    ImplUpdateDate( maCurDate );
    Control::GetFocus();
}

// -----------------------------------------------------------------------

void Calendar::LoseFocus()
{
    HideFocus();
    Control::LoseFocus();
}

// -----------------------------------------------------------------------

void Calendar::Resize()
{
    ImplUpdate( TRUE );
    Control::Resize();
}

// -----------------------------------------------------------------------

void Calendar::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & (HELPMODE_QUICK | HELPMODE_BALLOON) )
    {
        Date aDate = maCurDate;
        if ( GetDate( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ), aDate ) )
        {
            Rectangle aDateRect = GetDateRect( aDate );
            Point aPt = OutputToScreenPixel( aDateRect.TopLeft() );
            aDateRect.Left()   = aPt.X();
            aDateRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aDateRect.BottomRight() );
            aDateRect.Right()  = aPt.X();
            aDateRect.Bottom() = aPt.Y();

            if ( (rHEvt.GetMode() & HELPMODE_BALLOON) || (mnWinStyle & WB_QUICKHELPSHOWSDATEINFO) )
            {
                ImplDateInfo* pInfo;
                if ( mpDateTable )
                {
                    pInfo = mpDateTable->Get( aDate.GetDate() );
                    if ( !pInfo )
                        pInfo = mpDateTable->Get( Date( aDate.GetDay(), aDate.GetMonth(), 0 ).GetDate() );
                }
                else
                    pInfo = NULL;
                if ( pInfo )
                {
                    XubString aStr = pInfo->maText;
                    if ( aStr.Len() )
                    {
                        Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), aDateRect, aStr );
                        return;
                    }
                }
            }

            if ( rHEvt.GetMode() & HELPMODE_QUICK )
            {
                USHORT      nWeek = aDate.GetWeekOfYear( maIntn.GetWeekStart(), maIntn.GetWeekCountStart() );
                USHORT      nMonth = aDate.GetMonth();
                XubString   aStr( maDayText );
                aStr.AppendAscii( ": " );
                aStr.Append( XubString::CreateFromInt32( aDate.GetDayOfYear() ) );
                aStr.AppendAscii( " / " );
                aStr.Append( maWeekText );
                aStr.AppendAscii( ": " );
                aStr.Append( XubString::CreateFromInt32( nWeek ) );
                // Evt. noch Jahr hinzufuegen, wenn es nicht das gleiche ist
                if ( (nMonth == 12) && (nWeek == 1) )
                {
                    aStr.AppendAscii( ",  " );
                    aStr.Append( XubString::CreateFromInt32( aDate.GetYear()+1 ) );
                }
                else if ( (nMonth == 1) && (nWeek > 50) )
                {
                    aStr.AppendAscii( ", " );
                    aStr.Append( XubString::CreateFromInt32( aDate.GetYear()-1 ) );
                }
                Help::ShowQuickHelp( this, aDateRect, aStr );
                return;
            }
        }
    }

    Control::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void Calendar::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        if ( !mbSelection && rCEvt.IsMouseEvent() )
        {
            Date    aTempDate = maCurDate;
            USHORT  nHitTest = ImplHitTest( rCEvt.GetMousePosPixel(), aTempDate );
            if ( nHitTest & CALENDAR_HITTEST_MONTHTITLE )
            {
                ImplShowMenu( rCEvt.GetMousePosPixel(), aTempDate );
                return;
            }
        }
    }
    else if ( rCEvt.GetCommand() == COMMAND_WHEEL )
    {
        const CommandWheelData* pData = rCEvt.GetWheelData();
        if ( pData->GetMode() == COMMAND_WHEEL_SCROLL )
        {
            long nNotchDelta = pData->GetNotchDelta();
            if ( nNotchDelta < 0 )
            {
                while ( nNotchDelta < 0 )
                {
                    ImplScroll( TRUE );
                    nNotchDelta++;
                }
            }
            else
            {
                while ( nNotchDelta > 0 )
                {
                    ImplScroll( FALSE );
                    nNotchDelta--;
                }
            }

            return;
        }
    }

    Control::Command( rCEvt );
}

// -----------------------------------------------------------------------

void Calendar::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
        ImplFormat();
}

// -----------------------------------------------------------------------

void Calendar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void Calendar::SelectionChanging()
{
    maSelectionChangingHdl.Call( this );
}

// -----------------------------------------------------------------------

void Calendar::DateRangeChanged()
{
    maDateRangeChangedHdl.Call( this );
}

// -----------------------------------------------------------------------

void Calendar::RequestDateInfo()
{
    maRequestDateInfoHdl.Call( this );
}

// -----------------------------------------------------------------------

void Calendar::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void Calendar::Select()
{
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void Calendar::SetInternational( const International& rIntn )
{
    ImplUpdate( TRUE );

    maIntn = rIntn;
}

// -----------------------------------------------------------------------

void Calendar::SelectDate( const Date& rDate, BOOL bSelect )
{
    if ( !rDate.IsValid() )
        return;

    Table* pOldSel;

    if ( !mbInSelChange )
        pOldSel = new Table( *mpSelectTable );
    else
        pOldSel = NULL;

    ImplCalendarSelectDate( mpSelectTable, rDate, bSelect );

    if ( pOldSel )
    {
        ImplUpdateSelection( pOldSel );
        delete pOldSel;
    }
}

// -----------------------------------------------------------------------

void Calendar::SelectDateRange( const Date& rStartDate, const Date& rEndDate,
                                BOOL bSelect )
{
    if ( !rStartDate.IsValid() || !rEndDate.IsValid() )
        return;

    Table* pOldSel;

    if ( !mbInSelChange )
        pOldSel = new Table( *mpSelectTable );
    else
        pOldSel = NULL;

    ImplCalendarSelectDateRange( mpSelectTable, rStartDate, rEndDate, bSelect );

    if ( pOldSel )
    {
        ImplUpdateSelection( pOldSel );
        delete pOldSel;
    }
}

// -----------------------------------------------------------------------

void Calendar::SetNoSelection()
{
    Table* pOldSel;

    if ( !mbInSelChange )
        pOldSel = new Table( *mpSelectTable );
    else
        pOldSel = NULL;

    ImplCalendarClearSelectDate( mpSelectTable );

    if ( pOldSel )
    {
        ImplUpdateSelection( pOldSel );
        delete pOldSel;
    }
}

// -----------------------------------------------------------------------

BOOL Calendar::IsDateSelected( const Date& rDate ) const
{
    return mpSelectTable->IsKeyValid( rDate.GetDate() );
}

// -----------------------------------------------------------------------

ULONG Calendar::GetSelectDateCount() const
{
    return mpSelectTable->Count();
}

// -----------------------------------------------------------------------

Date Calendar::GetSelectDate( ULONG nIndex ) const
{
    if ( nIndex < mpSelectTable->Count() )
        return Date( mpSelectTable->GetObjectKey( nIndex ) );
    else
    {
        Date aDate( 0, 0, 0 );
        return aDate;
    }
}

// -----------------------------------------------------------------------

void Calendar::SetCurDate( const Date& rNewDate )
{
    if ( !rNewDate.IsValid() )
        return;

    if ( maCurDate != rNewDate )
    {
        BOOL bUpdate    = IsVisible() && IsUpdateMode();
        Date aOldDate   = maCurDate;
        maCurDate       = rNewDate;
        maAnchorDate    = maCurDate;

        if ( !(mnWinStyle & (WB_RANGESELECT | WB_MULTISELECT)) )
        {
            ImplCalendarSelectDate( mpSelectTable, aOldDate, FALSE );
            ImplCalendarSelectDate( mpSelectTable, maCurDate, TRUE );
        }
        else if ( !HasFocus() )
            bUpdate = FALSE;

        // Aktuelles Datum noch in den sichtbaren Bereich verschieben
        if ( mbFormat || (maCurDate < GetFirstMonth()) )
            SetFirstDate( maCurDate );
        else if ( maCurDate > GetLastMonth() )
        {
            Date aTempDate = GetLastMonth();
            long nDateOff = maCurDate-aTempDate;
            if ( nDateOff < 365 )
            {
                Date maFirstDate = GetFirstMonth();
                maFirstDate += maFirstDate.GetDaysInMonth();
                aTempDate++;
                while ( nDateOff > aTempDate.GetDaysInMonth() )
                {
                    maFirstDate += maFirstDate.GetDaysInMonth();
                    long nDaysInMonth = aTempDate.GetDaysInMonth();
                    aTempDate += nDaysInMonth;
                    nDateOff -= nDaysInMonth;
                }
                SetFirstDate( maFirstDate );
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

// -----------------------------------------------------------------------

void Calendar::SetFirstDate( const Date& rNewFirstDate )
{
    if ( maFirstDate != rNewFirstDate )
    {
        maFirstDate = Date( 1, rNewFirstDate.GetMonth(), rNewFirstDate.GetYear() );
        mbDropPos = FALSE;
        ImplUpdate();
    }
}

// -----------------------------------------------------------------------

Date Calendar::GetFirstMonth() const
{
    if ( maFirstDate.GetDay() > 1 )
    {
        if ( maFirstDate.GetMonth() == 12 )
            return Date( 1, 1, maFirstDate.GetYear()+1 );
        else
            return Date( 1, maFirstDate.GetMonth()+1, maFirstDate.GetYear() );
    }
    else
        return maFirstDate;
}

// -----------------------------------------------------------------------

Date Calendar::GetLastMonth() const
{
    Date aDate = GetFirstMonth();
    USHORT nMonthCount = GetMonthCount();
    for ( USHORT i = 0; i < nMonthCount; i++ )
        aDate += aDate.GetDaysInMonth();
    aDate--;
    return aDate;
}

// -----------------------------------------------------------------------

USHORT Calendar::GetMonthCount() const
{
    if ( mbFormat )
        return 1;
    else
        return (USHORT)(mnMonthPerLine*mnLines);
}

// -----------------------------------------------------------------------

BOOL Calendar::GetDropDate( Date& rDate ) const
{
    if( mbDropPos )
    {
        rDate = maDropDate;
        return TRUE;
    }
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Calendar::GetDate( const Point& rPos, Date& rDate ) const
{
    Date    aDate = maCurDate;
    USHORT  nHitTest = ImplHitTest( rPos, aDate );
    if ( nHitTest & CALENDAR_HITTEST_DAY )
    {
        rDate = aDate;
        return TRUE;
    }
    else
        return FALSE;
}

// -----------------------------------------------------------------------

Rectangle Calendar::GetDateRect( const Date& rDate ) const
{
    Rectangle aRect;

    if ( mbFormat || (rDate < maFirstDate) || (rDate > (maFirstDate+mnDayCount)) )
        return aRect;

    long    nX;
    long    nY;
    ULONG   nDaysOff;
    USHORT  nDayIndex;
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
            USHORT nWeekDay = (USHORT)aLastDate.GetDayOfWeek();
            nWeekDay = (nWeekDay+(7-(USHORT)maIntn.GetWeekStart())) % 7;
            aLastDate -= nWeekDay;
            aRect = GetDateRect( aLastDate );
            nDaysOff = rDate-aLastDate;
            nDayIndex = 0;
            for ( USHORT i = 0; i <= nDaysOff; i++ )
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
                aLastDate++;
            }
        }
    }

    nY = 0;
    for ( long i = 0; i < mnLines; i++ )
    {
        nX = 0;
        for ( long j = 0; j < mnMonthPerLine; j++ )
        {
            USHORT nDaysInMonth = aDate.GetDaysInMonth();

            // Monat gerufen
            if ( (aDate.GetMonth() == rDate.GetMonth()) &&
                 (aDate.GetYear() == rDate.GetYear()) )
            {
                long nDayX = nX+mnDaysOffX;
                long nDayY = nY+mnDaysOffY;
                nDayIndex = (USHORT)aDate.GetDayOfWeek();
                nDayIndex = (nDayIndex+(7-(USHORT)maIntn.GetWeekStart())) % 7;
                for ( USHORT nDay = 1; nDay <= nDaysInMonth; nDay++ )
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

// -----------------------------------------------------------------------

void Calendar::SetStandardColor( const Color& rColor )
{
    if ( mpStandardColor )
        *mpStandardColor = rColor;
    else
        mpStandardColor = new Color( rColor );
    ImplUpdate();
}

// -----------------------------------------------------------------------

void Calendar::SetSaturdayColor( const Color& rColor )
{
    if ( mpSaturdayColor )
        *mpSaturdayColor = rColor;
    else
        mpSaturdayColor = new Color( rColor );
    ImplUpdate();
}

// -----------------------------------------------------------------------

void Calendar::SetSundayColor( const Color& rColor )
{
    if ( mpSundayColor )
        *mpSundayColor = rColor;
    else
        mpSundayColor = new Color( rColor );
    ImplUpdate();
}

// -----------------------------------------------------------------------

void Calendar::AddDateInfo( const Date& rDate, const String& rText,
                            const Color* pTextColor, const Color* pFrameColor,
                            USHORT nFlags )
{
    if ( !mpDateTable )
        mpDateTable = new ImplDateTable( 256, 256 );

    BOOL            bChanged = FALSE;
    ULONG           nKey = rDate.GetDate();
    ImplDateInfo*   pDateInfo = mpDateTable->Get( nKey );
    if ( pDateInfo )
        pDateInfo->maText = rText;
    else
    {
        pDateInfo = new ImplDateInfo( rText );
        mpDateTable->Insert( nKey, pDateInfo );
    }
    if ( pTextColor )
    {
        if ( pDateInfo->mpTextColor )
        {
            if ( *(pDateInfo->mpTextColor) != *pTextColor )
            {
                *(pDateInfo->mpTextColor) = *pTextColor;
                bChanged = TRUE;
            }
        }
        else
        {
            pDateInfo->mpTextColor = new Color( *pTextColor );
            bChanged = TRUE;
        }
    }
    else
    {
        if ( pDateInfo->mpTextColor )
        {
            delete pDateInfo->mpTextColor;
            pDateInfo->mpTextColor = NULL;
            bChanged = TRUE;
        }
    }
    if ( pFrameColor )
    {
        if ( pDateInfo->mpFrameColor )
        {
            if ( *(pDateInfo->mpFrameColor) != *pFrameColor )
            {
                *(pDateInfo->mpFrameColor) = *pFrameColor;
                bChanged = TRUE;
            }
        }
        else
        {
            pDateInfo->mpFrameColor = new Color( *pFrameColor );
            bChanged = TRUE;
        }
    }
    else
    {
        if ( pDateInfo->mpFrameColor )
        {
            delete pDateInfo->mpFrameColor;
            pDateInfo->mpFrameColor = NULL;
            bChanged = TRUE;
        }
    }
    if ( pDateInfo->mnFlags != nFlags )
    {
        pDateInfo->mnFlags = nFlags;
        bChanged = TRUE;
    }

    if ( bChanged )
        ImplUpdateDate( rDate );
}

// -----------------------------------------------------------------------

void Calendar::RemoveDateInfo( const Date& rDate )
{
    if ( mpDateTable )
    {
        ImplDateInfo* pDateInfo = mpDateTable->Remove( rDate.GetDate() );
        if ( pDateInfo )
        {
            delete pDateInfo;
            ImplUpdateDate( rDate );
        }
    }
}

// -----------------------------------------------------------------------

void Calendar::ClearDateInfo()
{
    if ( mpDateTable )
    {
        ImplDateInfo* pDateInfo = mpDateTable->First();
        while ( pDateInfo )
        {
            ULONG nKey = mpDateTable->GetCurKey();
            mpDateTable->Remove( nKey );
            Date aDate( nKey );
            ImplUpdateDate( aDate );
            delete pDateInfo;
            pDateInfo = mpDateTable->First();
        }
        delete mpDateTable;
        mpDateTable = NULL;
    }
}

// -----------------------------------------------------------------------

XubString Calendar::GetDateInfoText( const Date& rDate )
{
    XubString aRet;
    if ( mpDateTable )
    {
        ULONG           nKey = rDate.GetDate();
        ImplDateInfo*   pDateInfo = mpDateTable->Get( nKey );
        if ( pDateInfo )
            aRet = pDateInfo->maText;
    }
    return aRet;
}

// -----------------------------------------------------------------------

BOOL Calendar::ShowDropPos( const Point& rPos, Date& rDate )
{
    Date    aTempDate = maCurDate;
    mnDragScrollHitTest = ImplHitTest( rPos, aTempDate );

    if ( mnDragScrollHitTest )
    {
        if ( mnDragScrollHitTest & (CALENDAR_HITTEST_PREV | CALENDAR_HITTEST_NEXT) )
        {
            if ( !maDragScrollTimer.IsActive() )
                maDragScrollTimer.Start();
        }
        else
        {
            maDragScrollTimer.Stop();
            if ( mnDragScrollHitTest & CALENDAR_HITTEST_DAY )
            {
                if ( !mbDropPos || (aTempDate != maDropDate) )
                {
                    if( mbDropPos )
                        ImplInvertDropPos();
                    maDropDate = aTempDate;
                    mbDropPos = TRUE;
                    ImplInvertDropPos();
                }

                rDate = maDropDate;
                return TRUE;
            }
        }
    }
    else
        maDragScrollTimer.Stop();

    HideDropPos();
    return FALSE;
}

// -----------------------------------------------------------------------

void Calendar::HideDropPos()
{
    if ( mbDropPos )
    {
        ImplInvertDropPos();
        mbDropPos = FALSE;
    }
}

// -----------------------------------------------------------------------

void Calendar::StartSelection()
{
    if ( mpOldSelectTable )
        delete mpOldSelectTable;
    maOldCurDate = maCurDate;
    mpOldSelectTable = new Table( *mpSelectTable );

    mbSelection = TRUE;
}

// -----------------------------------------------------------------------

void Calendar::EndSelection()
{
    if ( mbDrag || mbSpinDown || mbSelection )
    {
        if ( !mbSelection )
            ReleaseMouse();

        mbDrag              = FALSE;
        mbSelection         = FALSE;
        mbMultiSelection    = FALSE;
        mbSpinDown          = FALSE;
        mbPrevIn            = FALSE;
        mbNextIn            = FALSE;
    }
}

// -----------------------------------------------------------------------

Size Calendar::CalcWindowSizePixel( long nCalcMonthPerLine,
                                    long nCalcLines ) const
{
    XubString   a99Text( XubString( RTL_CONSTASCII_USTRINGPARAM( "99" ) ) );
    Font        aOldFont = GetFont();

    // Wochenanzeige beruecksichtigen
    long nWeekWidth;
    if ( mnWinStyle & WB_WEEKNUMBER )
    {
        Font aTempFont = aOldFont;
        ImplGetWeekFont( aTempFont );
        ((Calendar*)this)->SetFont( aTempFont );
        nWeekWidth = GetTextWidth( a99Text )+WEEKNUMBER_OFFX;
        ((Calendar*)this)->SetFont( aOldFont );
    }
    else
        nWeekWidth = 0;

    if ( mnWinStyle & WB_BOLDTEXT )
    {
        Font aFont = aOldFont;
        if ( aFont.GetWeight() < WEIGHT_BOLD )
            aFont.SetWeight( WEIGHT_BOLD );
        else
            aFont.SetWeight( WEIGHT_NORMAL );
        ((Calendar*)this)->SetFont( aFont );
    }

    Size    aSize;
    long    n99TextWidth = GetTextWidth( a99Text );
    long    nTextHeight = GetTextHeight();

    if ( mnWinStyle & WB_BOLDTEXT )
        ((Calendar*)this)->SetFont( aOldFont );

    aSize.Width()  += ((n99TextWidth+DAY_OFFX)*7) + nWeekWidth;
    aSize.Width()  += MONTH_BORDERX*2;
    aSize.Width()  *= nCalcMonthPerLine;

    aSize.Height()  = nTextHeight + TITLE_OFFY + (TITLE_BORDERY*2);
    aSize.Height() += nTextHeight + WEEKDAY_OFFY;
    aSize.Height() += ((nTextHeight+DAY_OFFY)*6);
    aSize.Height() += MONTH_OFFY;
    aSize.Height() *= nCalcLines;

    return aSize;
}

// =======================================================================

#define CALFIELD_EXTRA_BUTTON_WIDTH         14
#define CALFIELD_EXTRA_BUTTON_HEIGHT        8
#define CALFIELD_SEP_X                      6
#define CALFIELD_BORDERLINE_X               5
#define CALFIELD_BORDER_YTOP                4
#define CALFIELD_BORDER_Y                   5

// =======================================================================

class ImplCFieldFloatWin : public FloatingWindow
{
private:
    Calendar*       mpCalendar;
    PushButton*     mpTodayBtn;
    PushButton*     mpNoneBtn;
    FixedLine*      mpFixedLine;

public:
                    ImplCFieldFloatWin( Window* pParent );
                    ~ImplCFieldFloatWin();

    void            SetCalendar( Calendar* pCalendar )
                        { mpCalendar = pCalendar; }

    PushButton*     EnableTodayBtn( BOOL bEnable );
    PushButton*     EnableNoneBtn( BOOL bEnable );
    void            ArrangeButtons();

    long            Notify( NotifyEvent& rNEvt );
};

// -----------------------------------------------------------------------

ImplCFieldFloatWin::ImplCFieldFloatWin( Window* pParent ) :
    FloatingWindow( pParent )
{
    mpCalendar  = NULL;
    mpTodayBtn  = NULL;
    mpNoneBtn   = NULL;
    mpFixedLine = NULL;
}

// -----------------------------------------------------------------------

ImplCFieldFloatWin::~ImplCFieldFloatWin()
{
    delete mpTodayBtn;
    delete mpNoneBtn;
    delete mpFixedLine;
}

// -----------------------------------------------------------------------

PushButton* ImplCFieldFloatWin::EnableTodayBtn( BOOL bEnable )
{
    if ( bEnable )
    {
        if ( !mpTodayBtn )
        {
            mpTodayBtn = new PushButton( this, WB_NOPOINTERFOCUS );
            XubString aTodayText( SvtResId( STR_SVT_CALENDAR_TODAY ) );
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
        if ( mpTodayBtn )
        {
            delete mpTodayBtn;
            mpTodayBtn = NULL;
        }
    }

    return mpTodayBtn;
}

// -----------------------------------------------------------------------

PushButton* ImplCFieldFloatWin::EnableNoneBtn( BOOL bEnable )
{
    if ( bEnable )
    {
        if ( !mpNoneBtn )
        {
            mpNoneBtn = new PushButton( this, WB_NOPOINTERFOCUS );
            XubString aNoneText( SvtResId( STR_SVT_CALENDAR_NONE ) );
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
        if ( mpNoneBtn )
        {
            delete mpNoneBtn;
            mpNoneBtn = NULL;
        }
    }

    return mpNoneBtn;
}

// -----------------------------------------------------------------------

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
            mpFixedLine = new FixedLine( this );
            mpFixedLine->Show();
        }
        long nLineWidth = aOutSize.Width()-(CALFIELD_BORDERLINE_X*2);
        mpFixedLine->SetPosSizePixel( (aOutSize.Width()-nLineWidth)/2, aOutSize.Height()+((CALFIELD_BORDER_YTOP-2)/2),
                                      nLineWidth, 2, WINDOW_POSSIZE_POSSIZE );
        aOutSize.Height() += nBtnHeight + (CALFIELD_BORDER_Y*2) + CALFIELD_BORDER_YTOP;
        SetOutputSizePixel( aOutSize );
    }
    else
    {
        if ( mpFixedLine )
        {
            delete mpFixedLine;
            mpFixedLine = NULL;
        }
    }
}

// -----------------------------------------------------------------------

long ImplCFieldFloatWin::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        if ( pKEvt->GetKeyCode().GetCode() == KEY_RETURN )
            mpCalendar->Select();
    }

    return FloatingWindow::Notify( rNEvt );
}

// =======================================================================

CalendarField::CalendarField( Window* pParent, WinBits nWinStyle ) :
    DateField( pParent, nWinStyle ),
    maDefaultDate( 0, 0, 0 )
{
    mpFloatWin      = NULL;
    mpCalendar      = NULL;
    mnCalendarStyle = 0;
    mbToday         = FALSE;
    mbNone          = FALSE;
}

// -----------------------------------------------------------------------

CalendarField::CalendarField( Window* pParent, const ResId& rResId ) :
    DateField( pParent, rResId ),
    maDefaultDate( 0, 0, 0 )
{
    mpFloatWin      = NULL;
    mpCalendar      = NULL;
    mnCalendarStyle = 0;
    mbToday         = FALSE;
    mbNone          = FALSE;
}

// -----------------------------------------------------------------------

CalendarField::~CalendarField()
{
    if ( mpFloatWin )
    {
        delete mpCalendar;
        delete mpFloatWin;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( CalendarField, ImplSelectHdl, Calendar*, pCalendar )
{
    if ( !pCalendar->IsTravelSelect() )
    {
        mpFloatWin->EndPopupMode();
        EndDropDown();
        GrabFocus();
        Date aNewDate = mpCalendar->GetSelectDate( 0 );
        if ( IsEmptyDate() || ( aNewDate != GetDate() ) )
        {
            SetDate( aNewDate );
            SetModifyFlag();
            Modify();
        }
        Select();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( CalendarField, ImplClickHdl, PushButton*, pBtn )
{
    mpFloatWin->EndPopupMode();
    EndDropDown();
    GrabFocus();

    if ( pBtn == mpTodayBtn )
    {
        Date aToday;
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
    Select();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( CalendarField, ImplPopupModeEndHdl, FloatingWindow*, EMPTYARG )
{
    EndDropDown();
    GrabFocus();
    mpCalendar->EndSelection();
    return 0;
}

// -----------------------------------------------------------------------

void CalendarField::Select()
{
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

BOOL CalendarField::ShowDropDown( BOOL bShow )
{
    if ( bShow )
    {
        Calendar* pCalendar = GetCalendar();

        Date aDate = GetDate();
        if ( IsEmptyDate() || !aDate.IsValid() )
        {
            if ( maDefaultDate.IsValid() )
                aDate = maDefaultDate;
            else
                aDate = Date();
        }
        if ( pCalendar->GetStyle() & (WB_RANGESELECT | WB_MULTISELECT) )
        {
            pCalendar->SetNoSelection();
            pCalendar->SelectDate( aDate );
        }
        pCalendar->SetCurDate( aDate );
        Point       aPos( GetParent()->OutputToScreenPixel( GetPosPixel() ) );
        Rectangle   aRect( aPos, GetSizePixel() );
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
        mpFloatWin->StartPopupMode( aRect, FLOATWIN_POPUPMODE_DOWN );
    }
    else
    {
        mpFloatWin->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL );
        mpCalendar->EndSelection();
        EndDropDown();
    }
    return TRUE;
}

// -----------------------------------------------------------------------

Calendar* CalendarField::CreateCalendar( Window* pParent )
{
    return new Calendar( pParent, mnCalendarStyle | WB_TABSTOP );
}

// -----------------------------------------------------------------------

Calendar* CalendarField::GetCalendar()
{
    if ( !mpFloatWin )
    {
        mpFloatWin = new ImplCFieldFloatWin( this );
        mpFloatWin->SetPopupModeEndHdl( LINK( this, CalendarField, ImplPopupModeEndHdl ) );
        mpCalendar = CreateCalendar( mpFloatWin );
        mpCalendar->SetPosPixel( Point() );
        mpCalendar->SetSelectHdl( LINK( this, CalendarField, ImplSelectHdl ) );
    }

    return mpCalendar;
}
