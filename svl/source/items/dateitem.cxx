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

#include <svl/dateitem.hxx>

#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/Locale.hpp>


// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxDateTimeItem)


// -----------------------------------------------------------------------

TYPEINIT1(SfxDateTimeItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxDateTimeItem::SfxDateTimeItem( sal_uInt16 which, const DateTime& rDT ) :
    SfxPoolItem( which ),
    aDateTime( rDT )

{
    DBG_CTOR(SfxDateTimeItem, 0);
}

// -----------------------------------------------------------------------

SfxDateTimeItem::SfxDateTimeItem( const SfxDateTimeItem& rItem ) :
    SfxPoolItem( rItem ),
    aDateTime( rItem.aDateTime )
{
    DBG_CTOR(SfxDateTimeItem, 0);
}

// -----------------------------------------------------------------------

int SfxDateTimeItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS(SfxDateTimeItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ( ( (SfxDateTimeItem&)rItem ).aDateTime == aDateTime );
}

// -----------------------------------------------------------------------

int SfxDateTimeItem::Compare( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS(SfxDateTimeItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    // da X.Compare( Y ) am String einem Compare( Y, X ) entspricht,
    // vergleichen wir hier Y mit X
    if ( ( (const SfxDateTimeItem&)rItem ).aDateTime < aDateTime )
        return -1;
    else if ( ( (const SfxDateTimeItem&)rItem ).aDateTime == aDateTime )
        return 0;
    else
        return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxDateTimeItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    DBG_CHKTHIS(SfxDateTimeItem, 0);
    sal_uInt32 nDate = 0;
    sal_Int32 nTime = 0;
    rStream >> nDate;
    rStream >> nTime;
    DateTime aDT(nDate, nTime);
    return new SfxDateTimeItem( Which(), aDT );
}

// -----------------------------------------------------------------------

SvStream& SfxDateTimeItem::Store( SvStream& rStream, sal_uInt16 ) const
{
    DBG_CHKTHIS(SfxDateTimeItem, 0);
    rStream << aDateTime.GetDate();
    rStream << static_cast<sal_Int32>(aDateTime.GetTime());
    return rStream;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxDateTimeItem::Clone( SfxItemPool* ) const
{
    DBG_CHKTHIS(SfxDateTimeItem, 0);
    return new SfxDateTimeItem( *this );
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxDateTimeItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper *   pIntlWrapper
)   const
{
    DBG_CHKTHIS(SfxDateTimeItem, 0);
    if (aDateTime.IsValidDate())
        if (pIntlWrapper)
        {
            rText = pIntlWrapper->getLocaleData()->getDate(aDateTime) +
                    ", " +
                    pIntlWrapper->getLocaleData()->getTime(aDateTime);
        }
        else
        {
            DBG_WARNING("SfxDateTimeItem::GetPresentation():"
                         " Using default en_US IntlWrapper");
            const IntlWrapper aIntlWrapper( LanguageTag( LANGUAGE_ENGLISH_US) );
            rText = aIntlWrapper.getLocaleData()->getDate(aDateTime) +
                    ", " +
                    aIntlWrapper.getLocaleData()->getTime(aDateTime);
        }
    else
        rText = OUString();
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// virtual
bool SfxDateTimeItem::PutValue( const com::sun::star::uno::Any& rVal,
                                   sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    com::sun::star::util::DateTime aValue;
    if ( rVal >>= aValue )
    {
        aDateTime = DateTime( Date( aValue.Day,
                                      aValue.Month,
                                      aValue.Year ),
                              Time( aValue.Hours,
                                      aValue.Minutes,
                                      aValue.Seconds,
                                      aValue.NanoSeconds ) );
        return true;
    }

    OSL_FAIL( "SfxDateTimeItem::PutValue - Wrong type!" );
    return false;
}

// virtual
bool SfxDateTimeItem::QueryValue( com::sun::star::uno::Any& rVal,
                                   sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    com::sun::star::util::DateTime aValue( aDateTime.GetNanoSec(),
                                           aDateTime.GetSec(),
                                              aDateTime.GetMin(),
                                           aDateTime.GetHour(),
                                           aDateTime.GetDay(),
                                           aDateTime.GetMonth(),
                                           aDateTime.GetYear(),
                                           false);
    rVal <<= aValue;
    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
