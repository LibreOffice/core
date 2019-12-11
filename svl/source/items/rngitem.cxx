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

#include <sal/config.h>

#include <svl/rngitem.hxx>


SfxRangeItem::SfxRangeItem( sal_uInt16 which, sal_uInt16 from, sal_uInt16 to ):
    SfxPoolItem( which ),
    nFrom( from ),
    nTo( to )
{
}


bool SfxRangeItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    MapUnit                 /*eCoreMetric*/,
    MapUnit                 /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper&
)   const
{
    rText = OUString::number(nFrom) + ":" + OUString::number(nTo);
    return true;
}


bool SfxRangeItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    const SfxRangeItem& rT = static_cast<const SfxRangeItem&>(rItem);
    return nFrom==rT.nFrom && nTo==rT.nTo;
}

SfxRangeItem* SfxRangeItem::Clone(SfxItemPool *) const
{
    return new SfxRangeItem( Which(), nFrom, nTo );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
