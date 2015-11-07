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
#ifndef INCLUDED_SVX_SDPRCITM_HXX
#define INCLUDED_SVX_SDPRCITM_HXX

#include <svl/intitem.hxx>
#include <svx/svxdllapi.h>



// class SdrPercentItem
// Integer percents of 0


class SVX_DLLPUBLIC SdrPercentItem : public SfxUInt16Item
{
public:
    SdrPercentItem(): SfxUInt16Item() {}
    SdrPercentItem(sal_uInt16 nId, sal_uInt16 nVal=0): SfxUInt16Item(nId,nVal) {}
    SdrPercentItem(sal_uInt16 nId, SvStream& rIn):  SfxUInt16Item(nId,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=nullptr) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    virtual bool GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString& rText, const IntlWrapper * = nullptr) const override;
};


// class SdrSignedPercentItem
// Integer percents of +/-


class SVX_DLLPUBLIC SdrSignedPercentItem : public SfxInt16Item
{
public:
    SdrSignedPercentItem(): SfxInt16Item() {}
    SdrSignedPercentItem( sal_uInt16 nId, sal_Int16 nVal = 0 ) : SfxInt16Item( nId,nVal ) {}
    SdrSignedPercentItem( sal_uInt16 nId, SvStream& rIn ) : SfxInt16Item( nId,rIn ) {}
    virtual SfxPoolItem* Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual SfxPoolItem* Create( SvStream& rIn, sal_uInt16 nVer ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString& rText, const IntlWrapper * = nullptr ) const override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
