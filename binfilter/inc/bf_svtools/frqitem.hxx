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

#ifndef _FRQITEM_HXX
#define _FRQITEM_HXX

#include <tools/rtti.hxx>

#include <tools/time.hxx>

#include <tools/datetime.hxx>

#include <tools/string.hxx>

#include <bf_svtools/poolitem.hxx>

class SvStream;

namespace binfilter
{

DBG_NAMEEX(SfxFrequencyItem)

// class SfxFrequencyItem -------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/*
FRQ_DAILY
    * jeden X'ten Tag
    nInterval1 = 1 .. 999

    * jeden Wochentag
    nInterval1 = USHRT_MAX

FRQ_WEEKLY
    + wiederhole jede X'te Woche
    nInterval1 = 1 .. 99

    + an SU, MO, TU, WE, TH, FR, SA
    nInterval2 = WD_SUNDAY | WD_MONDAY | WD_TUESDAY | WD_WEDNESDAY |
                 WD_THURSDAY | WD_FRIDAY | WD_SATURDAY

FRQ_MONTHLY_DAILY
    * jeden X'ten Tag von jedem X'ten Monat
    nInterval1 = 1 .. 31
    nInterval2 = 1 .. 6

FRQ_MONTHLY_LOGIC
    * jeden ersten, zweiten, dritten, vierten oder letzten Wochentag jeden X'ten Monats
    nInterval1 = 0 .. 4
    nInterval2 = WD_SUNDAY | WD_MONDAY | WD_TUESDAY | WD_WEDNESDAY |
                 WD_THURSDAY | WD_FRIDAY | WD_SATURDAY
    nInterval3 = 1 .. 6

===============================================================================

FRQ_TIME_AT
    * Update um Uhrzeit
    nTime1 = 00:00:00 - 24:00:00

FRQ_TIME_REPEAT
    * Wiederhole alle X Stunden
    nTInterval1 = 1 .. 8
        + zwischen Uhrzeit 1 und 2
        nTime1 = 00:00:00 - 24:00:00
        nTime2 = 00:00:00 - 24:00:00

*/
///////////////////////////////////////////////////////////////////////////////

enum FrequencyMode
{
    FRQ_DAILY				= 1,
    FRQ_WEEKLY				= 2,
    FRQ_MONTHLY_DAILY		= 3,
    FRQ_MONTHLY_LOGIC		= 4
};

enum FrequencyTimeMode
{
    FRQ_TIME_AT				= 1,
    FRQ_TIME_REPEAT			= 2,
    FRQ_TIME_REPEAT_RANGE	= 3
};

#define WD_SUNDAY		0x0001
#define WD_MONDAY		0x0002
#define WD_TUESDAY		0x0004
#define WD_WEDNESDAY	0x0008
#define WD_THURSDAY		0x0010
#define WD_FRIDAY		0x0020
#define WD_SATURDAY		0x0040

class SfxFrequencyItem : public SfxPoolItem
{
private:
            FrequencyMode		eFrqMode;
            FrequencyTimeMode	eFrqTimeMode;

            USHORT				nDInterval1;
            USHORT				nDInterval2;
            USHORT				nDInterval3;

            USHORT				nTInterval1;
            Time				aTime1;
            Time				aTime2;

            BOOL				bMissingDate;
            DateTime			aMissingDate;

public:
            TYPEINFO();

            SfxFrequencyItem( const SfxFrequencyItem& rCpy );
            SfxFrequencyItem( USHORT nWhich, FrequencyMode eMode, FrequencyTimeMode eTMode,
                              USHORT nDI1, USHORT nDI2, USHORT nDI3, USHORT nTI1,
                              const Time& rT1, const Time& rT2 );
            ~SfxFrequencyItem() { DBG_DTOR(SfxFrequencyItem, 0); }

    virtual	int				operator==( const SfxPoolItem& )			const;
    using SfxPoolItem::Compare;
    virtual int				Compare( const SfxPoolItem &rWith )			const;
    virtual SfxPoolItem*	Create( SvStream&, USHORT nItemVersion )	const;
    virtual SvStream&		Store( SvStream&, USHORT nItemVersion )		const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 )				const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString &rText,
                                const ::IntlWrapper * = 0 ) const;

    BOOL					HasMissingDate() const 				{ return bMissingDate; }
    DateTime				GetMissingDate() const 				{ return aMissingDate; }

    FrequencyMode			GetFrequencyMode() const 			{ return eFrqMode; }
    FrequencyTimeMode		GetFrequencyTimeMode() const 		{ return eFrqTimeMode; }
    USHORT					GetDInterval_1() const				{ return nDInterval1; }
    USHORT					GetDInterval_2() const				{ return nDInterval2; }
    USHORT					GetDInterval_3() const				{ return nDInterval3; }
    USHORT					GetTInterval_1() const				{ return nTInterval1; }
    const Time&				GetTime_1() const 					{ return aTime1; }
    const Time&				GetTime_2() const 					{ return aTime2; }

    void					SetFrequencyMode(FrequencyMode eNew)	{ eFrqMode = eNew; }
    void					SetFrequencyTimeMode(FrequencyTimeMode eNew){ eFrqTimeMode = eNew; }
    void					SetDInterval_1(USHORT nNew)			 	{ nDInterval1 = nNew; }
    void					SetDInterval_2(USHORT nNew)			 	{ nDInterval2 = nNew; }
    void					SetDInterval_3(USHORT nNew)			 	{ nDInterval3 = nNew; }
    void					SetTInterval_1(USHORT nNew)			 	{ nTInterval1 = nNew; }
    void					SetTime_1(const Time& rNew)				{ aTime1 = rNew; }
    void					SetTime_2(const Time& rNew)				{ aTime2 = rNew; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
