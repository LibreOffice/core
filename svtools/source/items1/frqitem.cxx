/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: frqitem.cxx,v $
 * $Revision: 1.8.136.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#ifndef GCC
#endif

#include <unotools/intlwrapper.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>

#include <svtools/frqitem.hxx>

DBG_NAME( SfxFrequencyItem )

TYPEINIT1( SfxFrequencyItem, SfxPoolItem );

#define MAX_GOTO 32000

#define DECL_SAVE_GOTO()                \
    ULONG nSafetyMeasures = 0;

#define SAVE_GOTO(tag)                  \
    if(nSafetyMeasures < MAX_GOTO)      \
    { nSafetyMeasures++; goto tag; }

// -----------------------------------------------------------------------

SfxFrequencyItem::SfxFrequencyItem( USHORT which ) :
    SfxPoolItem     ( which ),
    eFrqMode        ( FRQ_DAILY ),
    eFrqTimeMode    ( FRQ_TIME_AT ),
    nDInterval1     ( 1 ),
    nDInterval2     ( 0 ),
    nDInterval3     ( 0 ),
    nTInterval1     ( 1 ),
    aTime1          ( Time( 12, 0, 0 ) ),
    aTime2          ( Time( 12, 0, 0 ) ),
    bMissingDate    ( FALSE ),
    aMissingDate    ( DateTime(0, 0) )
{
    DBG_CTOR( SfxFrequencyItem, 0 );
}

// -----------------------------------------------------------------------

SfxFrequencyItem::SfxFrequencyItem( USHORT which, FrequencyMode eMode, FrequencyTimeMode eTMode,
                                    USHORT nDI1, USHORT nDI2, USHORT nDI3, USHORT nTI1,
                                    const Time& rT1, const Time& rT2 ) :
    SfxPoolItem     ( which ),
    eFrqMode        ( eMode ),
    eFrqTimeMode    ( eTMode ),
    nDInterval1     ( nDI1 ),
    nDInterval2     ( nDI2 ),
    nDInterval3     ( nDI3 ),
    nTInterval1     ( nTI1 ),
    aTime1          ( rT1 ),
    aTime2          ( rT2 ),
    bMissingDate    ( FALSE )
{
    DBG_CTOR( SfxFrequencyItem, 0 );
}

// -----------------------------------------------------------------------

SfxFrequencyItem::SfxFrequencyItem( const SfxFrequencyItem& rItem ) :
    SfxPoolItem     ( rItem ),
    eFrqMode        ( rItem.eFrqMode ),
    eFrqTimeMode    ( rItem.eFrqTimeMode ),
    nDInterval1     ( rItem.nDInterval1 ),
    nDInterval2     ( rItem.nDInterval2 ),
    nDInterval3     ( rItem.nDInterval3 ),
    nTInterval1     ( rItem.nTInterval1 ),
    aTime1          ( rItem.aTime1 ),
    aTime2          ( rItem.aTime2 ),
    bMissingDate    ( rItem.bMissingDate )
{
    DBG_CTOR( SfxFrequencyItem, 0 );
}

// -----------------------------------------------------------------------

int SfxFrequencyItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS( SfxFrequencyItem, 0 );
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    return  ((SfxFrequencyItem&)rItem ).eFrqMode == eFrqMode &&
            ((SfxFrequencyItem&)rItem ).eFrqTimeMode == eFrqTimeMode &&
            ((SfxFrequencyItem&)rItem ).nDInterval1 == nDInterval1 &&
            ((SfxFrequencyItem&)rItem ).nDInterval2 == nDInterval2 &&
            ((SfxFrequencyItem&)rItem ).nDInterval3 == nDInterval3 &&
            ((SfxFrequencyItem&)rItem ).nTInterval1 == nTInterval1 &&
            ((SfxFrequencyItem&)rItem ).aTime1 == aTime1 &&
            ((SfxFrequencyItem&)rItem ).aTime2 == aTime2;
}

// -----------------------------------------------------------------------

int SfxFrequencyItem::Compare( const SfxPoolItem&
#ifdef DBG_UTIL
rItem
#endif
) const
{
    DBG_CHKTHIS( SfxFrequencyItem, 0 );
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

/*  DateTime aThisRange( aEndDateTime - aStartDateTime );
    DateTime aRange(((const SfxFrequencyItem&)rItem).aEndDateTime -
                    ((const SfxFrequencyItem&)rItem).aStartDateTime );
    if( aRange < aThisRange )
        return -1;
    else if( aRange == aThisRange )
        return 0;
    else
*/
    return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxFrequencyItem::Create( SvStream& rStream, USHORT ) const
{
    DBG_CHKTHIS(SfxFrequencyItem, 0);

    USHORT          _eFrqMode;
    USHORT          _eFrqTimeMode;
    USHORT          _nDInterval1;
    USHORT          _nDInterval2;
    USHORT          _nDInterval3;
    USHORT          _nTInterval1;
    long            _nTime1;
    long            _nTime2;

    rStream >> _eFrqMode;
    rStream >> _eFrqTimeMode;
    rStream >> _nDInterval1;
    rStream >> _nDInterval2;
    rStream >> _nDInterval3;
    rStream >> _nTInterval1;
    rStream >> _nTime1;
    rStream >> _nTime2;

    return new SfxFrequencyItem( Which(), (FrequencyMode)_eFrqMode,
        (FrequencyTimeMode) _eFrqTimeMode, _nDInterval1, _nDInterval2, _nDInterval3,
        _nTInterval1, Time(_nTime1), Time(_nTime2) );
}

// -----------------------------------------------------------------------

SvStream& SfxFrequencyItem::Store( SvStream& rStream, USHORT ) const
{
    DBG_CHKTHIS( SfxFrequencyItem, 0 );

    USHORT nEMode = (USHORT)eFrqMode;
    USHORT nETimeMode = (USHORT)eFrqTimeMode;

    rStream << (USHORT) nEMode;
    rStream << (USHORT) nETimeMode;

    rStream << nDInterval1;
    rStream << nDInterval2;
    rStream << nDInterval3;

    rStream << nTInterval1;
    rStream << aTime1.GetTime();
    rStream << aTime2.GetTime();

    return rStream;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxFrequencyItem::Clone( SfxItemPool* ) const
{
    DBG_CHKTHIS( SfxFrequencyItem, 0 );
    return new SfxFrequencyItem( *this );
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxFrequencyItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    XubString&              rText,
    const IntlWrapper *
)   const
{
    DBG_CHKTHIS(SfxFrequencyItem, 0);
    rText.AssignAscii(RTL_CONSTASCII_STRINGPARAM("SNIY"));
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

Time SfxFrequencyItem::_CalcTime( BOOL bForToday ) const
{
    Time aNow;
    Time aTime;

    DECL_SAVE_GOTO();

    switch( eFrqTimeMode )
    {
        //////////////////////////////////////////////////////////
        //  FRQ_TIME_AT
        //
        //  Update um Uhrzeit
        //  nTime1 = 00:00:00 - 24:00:00
        //
        case FRQ_TIME_AT :
            aTime = aTime1;
            break;

        //////////////////////////////////////////////////////////
        //  FRQ_TIME_REPEAT
        //
        //  Wiederhole alle X Stunden
        //  nTInterval1 = 1 .. 8
        //
        case FRQ_TIME_REPEAT :
            aTime = Time( 0, 0 );
            if( bForToday )
            {

            RECALC_TIME_REPEAT:
                if( aNow > aTime )
                {
                    aTime += Time( nTInterval1, 0 );
                    SAVE_GOTO( RECALC_TIME_REPEAT );
                }
                break;
            }
            break;

        //////////////////////////////////////////////////////////
        //  FRQ_TIME_REPEAT_RANGE
        //
        //  FRQ_TIME_REPEAT zwischen Uhrzeit 1 und 2
        //  nTime1 = 00:00:00 - 24:00:00
        //  nTime2 = 00:00:00 - 24:00:00
        //
        case FRQ_TIME_REPEAT_RANGE :
            aTime = aTime1;
            if( bForToday )
            {
                if( aNow > aTime2 )
                    return aTime1;

            RECALC_TIME_REPEAT_RANGE:
                if( aNow > aTime )
                {
                    aTime += Time( nTInterval1, 0 );
                    if( aTime > aTime2 )
                        return aTime1;
                    SAVE_GOTO( RECALC_TIME_REPEAT_RANGE );
                }
                break;
            }
            break;
    }
    return aTime;
}

DateTime SfxFrequencyItem::CalcNextTick( const DateTime& rBase, BOOL bFirst )
{
    Date aDateToday;
    Time aTimeToday;
    Date aDateBase;
    Time aTimeBase;

    if( bFirst )
    {
        aDateBase = Date( 17, 2, 1969 );
        aTimeBase = Time( 8, 0, 0 );
    }
    else
    {
        aDateBase = rBase.GetDate();
        aTimeBase = rBase.GetTime();
    }

    Time aNextTime( _CalcTime(FALSE) );
    Date aNextDate( aDateBase );
    bMissingDate = FALSE;

    DECL_SAVE_GOTO();

    switch( eFrqMode )
    {
        //////////////////////////////////////////////////////////
        //  FRQ_DAILY
        //
        //  jeden X'ten Tag
        //  nInterval1 = 1 .. 999
        //
        //  jeden Wochentag
        // nInterval1 = USHRT_MAX
        //
        case FRQ_DAILY :
        {
            if( bFirst )
            {
                aNextTime = _CalcTime( TRUE );
                if( aNextTime < aTimeToday )
                {
                    aNextTime = _CalcTime( FALSE );
                    aNextDate = aDateToday + (USHORT)
                                ((nDInterval1 == USHRT_MAX)? 1 : nDInterval1);
                }
                else
                    aNextDate = aDateToday;
                break;
            }

            RECALC_FRQ_DAILY:
            if( aNextDate < aDateToday )
            {
                bMissingDate = TRUE;
                aMissingDate.SetDate(aNextDate.GetDate());
                aMissingDate.SetTime(_CalcTime(FALSE).GetTime());

                aNextDate += (USHORT)((nDInterval1 == USHRT_MAX)? 1 : nDInterval1);
                SAVE_GOTO( RECALC_FRQ_DAILY );
            }

            if( aNextDate == aDateToday )
            {
                aNextTime = _CalcTime( TRUE );
                if( aNextTime < aTimeToday )
                {
                    bMissingDate = TRUE;
                    aMissingDate.SetDate(aNextDate.GetDate());
                    aMissingDate.SetTime(_CalcTime(FALSE).GetTime());

                    aNextDate += (USHORT)((nDInterval1 == USHRT_MAX)? 1 : nDInterval1);
                    aNextTime = _CalcTime( FALSE );
                }
            }
            else
                aNextTime = _CalcTime( FALSE );
        } break;

        //////////////////////////////////////////////////////////
        //  FRQ_WEEKLY
        //  wiederhole jede X'te Woche
        //  nInterval1 = 1 .. 99
        //
        //  an SU, MO, TU, WE, TH, FR, SA
        //  nInterval2 = WD_SUNDAY | WD_MONDAY | WD_TUESDAY |
        //               WD_WEDNESDAY | WD_THURSDAY | WD_FRIDAY |
        //               WD_SATURDAY
        //
        case FRQ_WEEKLY :
        {
            BOOL bInRecalc = FALSE;

            RECALC_FRQ_WEEKLY:
            if( !bFirst || bInRecalc )
                aNextDate += (nDInterval1 - 1) * 7;

            aNextDate -= (USHORT) ((aNextDate.GetDayOfWeek() != SUNDAY) ?
                         aNextDate.GetDayOfWeek() + 1 : 0);

            if( nDInterval2 & WD_SUNDAY && (aNextDate >= aDateToday) )
                aNextDate += 0;
            else if( nDInterval2 & WD_MONDAY && (aNextDate + 1 >= aDateToday) )
                aNextDate += 1;
            else if( nDInterval2 & WD_TUESDAY && (aNextDate + 2 >= aDateToday) )
                aNextDate += 2;
            else if( nDInterval2 & WD_WEDNESDAY && (aNextDate + 3 >= aDateToday) )
                aNextDate += 3;
            else if( nDInterval2 & WD_THURSDAY && (aNextDate + 4 >= aDateToday) )
                aNextDate += 4;
            else if( nDInterval2 & WD_FRIDAY && (aNextDate + 5 >= aDateToday) )
                aNextDate += 5;
            else if( nDInterval2 & WD_SATURDAY && (aNextDate + 6 >= aDateToday) )
                aNextDate += 6;

            if( aNextDate < aDateToday )
            {
                bMissingDate = TRUE;
                aMissingDate.SetDate(aNextDate.GetDate());
                aMissingDate.SetTime(_CalcTime(FALSE).GetTime());

                bInRecalc = TRUE;
                aNextDate += 7;
                SAVE_GOTO( RECALC_FRQ_WEEKLY );
            }
            if( aNextDate == aDateToday )
            {
                aNextTime = _CalcTime( TRUE );
                if( aNextTime < aTimeToday )
                {
                    bInRecalc = TRUE;
                    aNextDate += 7;
                    SAVE_GOTO( RECALC_FRQ_WEEKLY );
                }
            }
            else
                aNextTime = _CalcTime( FALSE );
        } break;

        //////////////////////////////////////////////////////////
        //  FRQ_MONTHLY_DAILY
        //
        //  jeden X'ten Tag von jedem X'ten Monat
        //  nInterval1 = 1 .. 31
        //  nInterval2 = 1 .. 6
        //
        case FRQ_MONTHLY_DAILY :
        {
            BOOL bInRecalc = FALSE;
            aNextDate.SetDay( nDInterval1 );

            RECALC_FRQ_MONTHLY_DAILY:
            if( nDInterval2 > 1 || bInRecalc )
            {
                long nMonth = aNextDate.GetMonth() - 1;
                nMonth += nDInterval2;
                aNextDate.SetYear(
                    sal::static_int_cast< USHORT >(
                        aNextDate.GetYear() + nMonth / 12 ) );
                aNextDate.SetMonth(
                    sal::static_int_cast< USHORT >( ( nMonth % 12 ) + 1 ) );
            }

            if( aNextDate < aDateToday )
            {
                bMissingDate = TRUE;
                aMissingDate.SetDate(aNextDate.GetDate());
                aMissingDate.SetTime(_CalcTime(FALSE).GetTime());

                bInRecalc = TRUE;
                SAVE_GOTO( RECALC_FRQ_MONTHLY_DAILY );
            }

            if( aNextDate == aDateToday )
            {
                aNextTime = _CalcTime( TRUE );
                if( aNextTime < aTimeToday )
                {
                    bMissingDate = TRUE;
                    aMissingDate.SetDate(aNextDate.GetDate());
                    aMissingDate.SetTime(_CalcTime(FALSE).GetTime());

                    bInRecalc = TRUE;
                    SAVE_GOTO( RECALC_FRQ_MONTHLY_DAILY );
                }
            }
            else
                aNextTime = _CalcTime( FALSE );
        } break;

        //////////////////////////////////////////////////////////
        //  FRQ_MONTHLY_LOGIC
        //
        //  jeden ersten, zweiten, dritten, vierten oder letzten
        //  Wochentag jeden X'ten Monats
        //  nInterval1 = 0 .. 4
        //  nInterval2 = WD_SUNDAY | WD_MONDAY | WD_TUESDAY |
        //               WD_WEDNESDAY | WD_THURSDAY | WD_FRIDAY |
        //               WD_SATURDAY
        //  nInterval3 = 1 .. 6
        //
        case FRQ_MONTHLY_LOGIC :
        {
            BOOL bInRecalc = FALSE;

            RECALC_FRQ_MONTHLY_LOGIC:
            if( nDInterval3 > 1 || bInRecalc )
            {
                long nMonth = aNextDate.GetMonth() - 1;
                nMonth += nDInterval3;
                aNextDate.SetYear(
                    sal::static_int_cast< USHORT >(
                        aNextDate.GetYear() + nMonth / 12 ) );
                aNextDate.SetMonth(
                    sal::static_int_cast< USHORT >( ( nMonth % 12 ) + 1 ) );
            }

            USHORT nDay;
            if( nDInterval2 & WD_SUNDAY )
                nDay = 6;
            else if( nDInterval2 & WD_MONDAY )
                nDay = 0;
            else if( nDInterval2 & WD_TUESDAY )
                nDay = 1;
            else if( nDInterval2 & WD_WEDNESDAY )
                nDay = 2;
            else if( nDInterval2 & WD_THURSDAY )
                nDay = 3;
            else if( nDInterval2 & WD_FRIDAY )
                nDay = 4;
            else nDay = 5;

            if( nDInterval1 == 4 )
            {
                DateTime aDT = aNextDate;
                aDT.SetDay( 1 );
                aDT += (long)(aNextDate.GetDaysInMonth() - 1);
                if( aDT.GetDayOfWeek() != nDay )
                    for( aDT--; aDT.GetDayOfWeek() != nDay; aDT-- ) ;
                aNextDate = aDT;
            }
            else
            {
                DateTime aDT = aNextDate;
                aDT.SetDay( 1 );
                aDT += (long)(nDay - USHORT(aDT.GetDayOfWeek()));
                if( aDT.GetMonth() != aNextDate.GetMonth() )
                    aDT += 7L;
                aDT += (long)(nDInterval1 * 7);
                aNextDate = aDT;
            }

            if( aNextDate < aDateToday )
            {
                bMissingDate = TRUE;
                aMissingDate.SetDate(aNextDate.GetDate());
                aMissingDate.SetTime(_CalcTime(FALSE).GetTime());

                bInRecalc = TRUE;
                SAVE_GOTO( RECALC_FRQ_MONTHLY_LOGIC );
            }

            if( aNextDate == aDateToday )
            {
                aNextTime = _CalcTime( TRUE );
                if( aNextTime < aTimeToday )
                {
                    bMissingDate = TRUE;
                    aMissingDate.SetDate(aNextDate.GetDate());
                    aMissingDate.SetTime(_CalcTime(FALSE).GetTime());

                    bInRecalc = TRUE;
                    SAVE_GOTO( RECALC_FRQ_MONTHLY_LOGIC );
                }
            }
            else
                aNextTime = _CalcTime( FALSE );
        } break;
    }

    return DateTime( aNextDate, aNextTime );
}

