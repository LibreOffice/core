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


#include <svl/flagitem.hxx>
#include <svl/poolitem.hxx>
#include <tools/stream.hxx>




SfxFlagItem::SfxFlagItem( sal_uInt16 nW, sal_uInt16 nV ) :
    SfxPoolItem( nW ),
    nVal(nV)
{
}



SfxFlagItem::SfxFlagItem( const SfxFlagItem& rItem ) :
    SfxPoolItem( rItem ),
    nVal( rItem.nVal )
{
}



SvStream& SfxFlagItem::Store(SvStream &rStream, sal_uInt16) const
{
    rStream.WriteUInt16( nVal );
    return rStream;
}



bool SfxFlagItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper *
)   const
{
    rText.clear();
    for ( sal_uInt8 nFlag = 0; nFlag < GetFlagCount(); ++nFlag )
        rText += GetFlag(nFlag) ? OUString("true") : OUString("false");
    return true;
}



sal_uInt8 SfxFlagItem::GetFlagCount() const
{
    SAL_INFO("svl", "calling GetValueText(sal_uInt16) on SfxFlagItem -- override!");
    return 0;
}



SfxPoolItem* SfxFlagItem::Create(SvStream &, sal_uInt16) const
{
    SAL_INFO("svl", "calling Create() on SfxFlagItem -- override!");
    return 0;
}



bool SfxFlagItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return static_cast<const SfxFlagItem&>(rItem).nVal == nVal;
}




SfxPoolItem* SfxFlagItem::Clone(SfxItemPool *) const
{
    return new SfxFlagItem( *this );
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
