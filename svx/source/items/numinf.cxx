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

#include <svx/numinf.hxx>


#define INIT(pNum,eVal,nDouble,rStr)    \
    SfxPoolItem     ( nId ),            \
                                        \
    pFormatter      ( pNum ),           \
    eValueType      ( eVal ),           \
    aStringVal      ( rStr ),           \
    nDoubleVal      ( nDouble ),        \
    pDelFormatArr   ( nullptr ),           \
    nDelCount       ( 0 )

SvxNumberInfoItem::SvxNumberInfoItem( const sal_uInt16 nId ) :

    INIT( nullptr, SvxNumberValueType::Undefined, 0, "" )

{
}


SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const sal_uInt16 nId ) :

    INIT( pNumFormatter, SvxNumberValueType::Undefined, 0, "" )

{
}


SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const OUString& rVal, const sal_uInt16 nId ) :

    INIT( pNumFormatter, SvxNumberValueType::String, 0, rVal )

{
}


SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const double& rVal, const sal_uInt16 nId ) :

    INIT( pNumFormatter, SvxNumberValueType::Number, rVal, "" )

{
}


SvxNumberInfoItem::SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                                      const double& rVal, const OUString& rValueStr,
                                      const sal_uInt16 nId ) :

    INIT( pNumFormatter, SvxNumberValueType::Number, rVal, rValueStr )

{
}

#undef INIT


SvxNumberInfoItem::SvxNumberInfoItem( const SvxNumberInfoItem& rItem ) :

    SfxPoolItem( rItem.Which() ),

    pFormatter   ( rItem.pFormatter ),
    eValueType   ( rItem.eValueType ),
    aStringVal   ( rItem.aStringVal ),
    nDoubleVal   ( rItem.nDoubleVal ),
    pDelFormatArr( nullptr ),
    nDelCount    ( rItem.nDelCount )

{
    if ( rItem.nDelCount > 0 )
    {
        pDelFormatArr.reset( new sal_uInt32[ rItem.nDelCount ] );

        for ( sal_uInt32 i = 0; i < rItem.nDelCount; ++i )
            pDelFormatArr[i] = rItem.pDelFormatArr[i];
    }
}


SvxNumberInfoItem::~SvxNumberInfoItem()
{
}


bool SvxNumberInfoItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    return false;
}


bool SvxNumberInfoItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const SvxNumberInfoItem& rOther = static_cast<const SvxNumberInfoItem&>(rItem);

    bool bEqual = false;

    if ( nDelCount == rOther.nDelCount )
    {
        if ( nDelCount > 0 )
        {
            if ( pDelFormatArr != nullptr && rOther.pDelFormatArr != nullptr )
            {
                bEqual = true;

                for ( sal_uInt32 i = 0; i < nDelCount && bEqual; ++i )
                    bEqual = ( pDelFormatArr[i] == rOther.pDelFormatArr[i] );
            }
        }
        else if ( nDelCount == 0 )
            bEqual = ( pDelFormatArr == nullptr && rOther.pDelFormatArr == nullptr );

        bEqual = bEqual &&
                 pFormatter == rOther.pFormatter &&
                 eValueType == rOther.eValueType &&
                 nDoubleVal == rOther.nDoubleVal &&
                 aStringVal == rOther.aStringVal;
    }
    return bEqual;
}


SfxPoolItem* SvxNumberInfoItem::Clone( SfxItemPool * ) const
{
    return new SvxNumberInfoItem( *this );
}


void SvxNumberInfoItem::SetDelFormatArray( const sal_uInt32* pData,
                                           const sal_uInt32 nCount )
{
    pDelFormatArr.reset();

    nDelCount = nCount;

    if ( nCount > 0 )
    {
        pDelFormatArr.reset( new sal_uInt32[ nCount ] );

        if ( pData != nullptr )
        {
            for ( sal_uInt32 i = 0; i < nCount; ++i )
                pDelFormatArr[i] = pData[i];
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
