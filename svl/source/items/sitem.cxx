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

#include <rtl/ustring.hxx>
#include <svl/itemset.hxx>
#include <svl/setitem.hxx>
#include <svl/poolitem.hxx>

SfxSetItem::SfxSetItem( sal_uInt16 which, const SfxItemSet &rSet) :
    SfxPoolItem(which),
    maSet(rSet)
{
    assert(!dynamic_cast<const SfxAllItemSet*>(&rSet) && "cannot handle SfxAllItemSet here");
    setIsSetItem();
}


SfxSetItem::SfxSetItem( sal_uInt16 which, SfxItemSet &&pS) :
    SfxPoolItem(which),
    maSet(pS)
{
    assert(!dynamic_cast<SfxAllItemSet*>(&pS) && "cannot handle SfxAllItemSet here");
    setIsSetItem();
}


SfxSetItem::SfxSetItem( const SfxSetItem& rCopy, SfxItemPool *pPool ) :
    SfxPoolItem(rCopy),
    maSet(rCopy.maSet.CloneAsValue(true, pPool))
{
    assert(!dynamic_cast<const SfxAllItemSet*>(&rCopy.maSet) && "cannot handle SfxAllItemSet here");
    setIsSetItem();
}


bool SfxSetItem::operator==( const SfxPoolItem& rCmp) const
{
    assert(SfxPoolItem::operator==(rCmp));
    return maSet == static_cast<const SfxSetItem &>(rCmp).maSet;
}


bool SfxSetItem::GetPresentation
(
    SfxItemPresentation    /*ePresentation*/,
    MapUnit                /*eCoreMetric*/,
    MapUnit                /*ePresentationMetric*/,
    OUString&              /*rText*/,
    const IntlWrapper&
)   const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
