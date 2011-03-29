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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define _SVX_NUMINF_CXX

#include <svx/numinf.hxx>

// -----------------------------------------------------------------------

TYPEINIT1(SvxNumberInfoItem, SfxPoolItem);

// class SvxNumberInfoItem -----------------------------------------------

#define INIT(pNum,eVal,nDouble,rStr)    \
    SfxPoolItem     ( nId ),            \
                                        \
    pFormatter      ( pNum ),           \
    eValueType      ( eVal ),           \
    aStringVal      ( rStr ),           \
    nDoubleVal      ( nDouble ),        \
    pDelFormatArr   ( NULL ),           \
    nDelCount       ( 0 )

SvxNumberInfoItem::SvxNumberInfoItem( const sal_uInt16 nId ) :

    INIT( NULL, SVX_VALUE_TYPE_UNDEFINED, 0, String() )

{
}

// -----------------------------------------------------------------------

SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const sal_uInt16 nId ) :

    INIT( pNumFormatter, SVX_VALUE_TYPE_UNDEFINED, 0, String() )

{
}

// -----------------------------------------------------------------------

SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const String& rVal, const sal_uInt16 nId ) :

    INIT( pNumFormatter, SVX_VALUE_TYPE_STRING, 0, rVal )

{
}

// -----------------------------------------------------------------------

SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const double& rVal, const sal_uInt16 nId ) :

    INIT( pNumFormatter, SVX_VALUE_TYPE_NUMBER, rVal, String() )

{
}

// -----------------------------------------------------------------------

SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const double& rVal, const String& rValueStr,
                                      const sal_uInt16 nId ) :

    INIT( pNumFormatter, SVX_VALUE_TYPE_NUMBER, rVal, rValueStr )

{
}

#undef INIT

// -----------------------------------------------------------------------

SvxNumberInfoItem::SvxNumberInfoItem( const SvxNumberInfoItem& rItem ) :

    SfxPoolItem( rItem.Which() ),

    pFormatter   ( rItem.pFormatter ),
    eValueType   ( rItem.eValueType ),
    aStringVal   ( rItem.aStringVal ),
    nDoubleVal   ( rItem.nDoubleVal ),
    pDelFormatArr( NULL ),
    nDelCount    ( rItem.nDelCount )

{
    if ( rItem.nDelCount > 0 )
    {
        pDelFormatArr = new sal_uInt32[ rItem.nDelCount ];

        for ( sal_uInt16 i = 0; i < rItem.nDelCount; ++i )
            pDelFormatArr[i] = rItem.pDelFormatArr[i];
    }
}

// -----------------------------------------------------------------------

SvxNumberInfoItem::~SvxNumberInfoItem()
{
    if ( pDelFormatArr )
        delete []pDelFormatArr;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxNumberInfoItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    String&             rText, const IntlWrapper *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------

int SvxNumberInfoItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal which or type" );

    SvxNumberInfoItem& rOther = (SvxNumberInfoItem&)rItem;

    sal_Bool bEqual = sal_False;

    if ( nDelCount == rOther.nDelCount )
    {
        if ( nDelCount > 0 )
        {
            if ( pDelFormatArr != NULL && rOther.pDelFormatArr != NULL )
            {
                bEqual = sal_True;

                for ( sal_uInt16 i = 0; i < nDelCount && bEqual; ++i )
                    bEqual = ( pDelFormatArr[i] == rOther.pDelFormatArr[i] );
            }
        }
        else if ( nDelCount == 0 )
            bEqual = ( pDelFormatArr == NULL && rOther.pDelFormatArr == NULL );

        bEqual = bEqual &&
                 pFormatter == rOther.pFormatter &&
                 eValueType == rOther.eValueType &&
                 nDoubleVal == rOther.nDoubleVal &&
                 aStringVal == rOther.aStringVal;
    }
    return bEqual;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxNumberInfoItem::Clone( SfxItemPool * ) const
{
    return new SvxNumberInfoItem( *this );
}

// Laden/Speichern wird nicht gebraucht!
// -----------------------------------------------------------------------

SfxPoolItem* SvxNumberInfoItem::Create( SvStream& /*rStream*/, sal_uInt16 ) const
{
    return new SvxNumberInfoItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxNumberInfoItem::Store( SvStream &rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    return rStream;
}

// -----------------------------------------------------------------------

void SvxNumberInfoItem::SetNumberFormatter( SvNumberFormatter* pNumFormatter )
{
    pFormatter = pNumFormatter;
}

// -----------------------------------------------------------------------

void SvxNumberInfoItem::SetStringValue( const String& rNewVal )
{
    aStringVal = rNewVal;
    eValueType = SVX_VALUE_TYPE_STRING;
}

// -----------------------------------------------------------------------

void SvxNumberInfoItem::SetDoubleValue( const double& rNewVal )
{
    nDoubleVal = rNewVal;
    eValueType = SVX_VALUE_TYPE_NUMBER;
}

// -----------------------------------------------------------------------

void SvxNumberInfoItem::SetDelFormatArray( const sal_uInt32* pData,
                                           const sal_uInt32 nCount )
{
    if ( pDelFormatArr )
    {
        delete []pDelFormatArr;
        pDelFormatArr = NULL;
    }

    nDelCount = nCount;

    if ( nCount > 0 )
    {
        pDelFormatArr = new sal_uInt32[ nCount ];

        if ( pData != NULL )
        {
            for ( sal_uInt16 i = 0; i < nCount; ++i )
                pDelFormatArr[i] = pData[i];
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
