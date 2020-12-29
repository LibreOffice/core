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


#include <svl/imageitm.hxx>
#include <com/sun/star/uno/Sequence.hxx>


SfxPoolItem* SfxImageItem::CreateDefault() { return new SfxImageItem; }

SfxImageItem::SfxImageItem( sal_uInt16 which )
    : SfxInt16Item( which, 0 ), mnAngle(0), mbMirrored(false)
{
}

SfxImageItem::SfxImageItem( const SfxImageItem& rItem )
    : SfxInt16Item( rItem ),
      mnAngle(rItem.mnAngle), mbMirrored(rItem.mbMirrored)
{
}

SfxImageItem::~SfxImageItem()
{
}

SfxImageItem* SfxImageItem::Clone( SfxItemPool* ) const
{
    return new SfxImageItem( *this );
}

bool SfxImageItem::operator==( const SfxPoolItem& rItem ) const
{
    if (!SfxInt16Item::operator==(rItem))
        return false;
    const SfxImageItem& rOther = static_cast<const SfxImageItem&>(rItem);
    return mnAngle == rOther.mnAngle && mbMirrored == rOther.mbMirrored;
}

bool SfxImageItem::QueryValue( css::uno::Any& rVal, sal_uInt8 ) const
{
    css::uno::Sequence< css::uno::Any > aSeq( 4 );
    aSeq[0] <<= GetValue();
    aSeq[1] <<= sal_Int16(mnAngle);
    aSeq[2] <<= mbMirrored;
    aSeq[3] <<= maURL;

    rVal <<= aSeq;
    return true;
}

bool SfxImageItem::PutValue( const css::uno::Any& rVal, sal_uInt8 )
{
    css::uno::Sequence< css::uno::Any > aSeq;
    if (( rVal >>= aSeq ) && ( aSeq.getLength() == 4 ))
    {
        sal_Int16     nVal = sal_Int16();
        if ( aSeq[0] >>= nVal )
            SetValue( nVal );
        sal_Int16 nTmp = 0;
        aSeq[1] >>= nTmp;
        mnAngle = Degree10(nTmp);
        aSeq[2] >>= mbMirrored;
        aSeq[3] >>= maURL;
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
