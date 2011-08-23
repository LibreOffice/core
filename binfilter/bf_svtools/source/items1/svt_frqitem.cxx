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

// MARKER(update_precomp.py): autogen include statement, do not remove

#ifndef GCC
#endif

#include <unotools/intlwrapper.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>

#include <bf_svtools/frqitem.hxx>

namespace binfilter
{

DBG_NAME( SfxFrequencyItem )

TYPEINIT1( SfxFrequencyItem, SfxPoolItem );

#define MAX_GOTO 32000

#define DECL_SAVE_GOTO()				\
    ULONG nSafetyMeasures = 0;

#define SAVE_GOTO(tag)					\
    if(nSafetyMeasures < MAX_GOTO)	 	\
    { nSafetyMeasures++; goto tag; }

// -----------------------------------------------------------------------

SfxFrequencyItem::SfxFrequencyItem( USHORT which, FrequencyMode eMode, FrequencyTimeMode eTMode,
                                    USHORT nDI1, USHORT nDI2, USHORT nDI3, USHORT nTI1,
                                    const Time& rT1, const Time& rT2 ) :
    SfxPoolItem		( which ),
    eFrqMode   		( eMode ),
    eFrqTimeMode	( eTMode ),
    nDInterval1		( nDI1 ),
    nDInterval2		( nDI2 ),
    nDInterval3		( nDI3 ),
    nTInterval1		( nTI1 ),
    aTime1			( rT1 ),
    aTime2 			( rT2 ),
    bMissingDate	( FALSE )
{
    DBG_CTOR( SfxFrequencyItem, 0 );
}

// -----------------------------------------------------------------------

SfxFrequencyItem::SfxFrequencyItem( const SfxFrequencyItem& rItem ) :
    SfxPoolItem		( rItem ),
    eFrqMode   		( rItem.eFrqMode ),
    eFrqTimeMode	( rItem.eFrqTimeMode ),
    nDInterval1		( rItem.nDInterval1 ),
    nDInterval2		( rItem.nDInterval2 ),
    nDInterval3		( rItem.nDInterval3 ),
    nTInterval1		( rItem.nTInterval1 ),
    aTime1			( rItem.aTime1 ),
    aTime2 			( rItem.aTime2 ),
    bMissingDate	( rItem.bMissingDate )
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

/*	DateTime aThisRange( aEndDateTime - aStartDateTime );
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

    USHORT			_eFrqMode;
    USHORT			_eFrqTimeMode;
    USHORT		 	_nDInterval1;
    USHORT		 	_nDInterval2;
    USHORT		 	_nDInterval3;
    USHORT		 	_nTInterval1;
    long		 	_nTime1;
    long		 	_nTime2;

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
    SfxItemPresentation 	/*ePresentation*/,
    SfxMapUnit				/*eCoreMetric*/,
    SfxMapUnit				/*ePresentationMetric*/,
    XubString& 				rText,
    const ::IntlWrapper *
)	const
{
    DBG_CHKTHIS(SfxFrequencyItem, 0);
    rText.AssignAscii(RTL_CONSTASCII_STRINGPARAM("SNIY"));
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

}
