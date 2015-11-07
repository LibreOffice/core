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
#ifndef INCLUDED_SVX_SDTADITM_HXX
#define INCLUDED_SVX_SDTADITM_HXX

#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdllapi.h>


// class SdrTextAniDirectionItem


enum SdrTextAniDirection {SDRTEXTANI_LEFT,
                          SDRTEXTANI_RIGHT,
                          SDRTEXTANI_UP,
                          SDRTEXTANI_DOWN};

class SVX_DLLPUBLIC SdrTextAniDirectionItem: public SfxEnumItem {
public:
    SdrTextAniDirectionItem(SdrTextAniDirection eDir=SDRTEXTANI_LEFT): SfxEnumItem(SDRATTR_TEXT_ANIDIRECTION,(sal_uInt16)eDir) {}
    SdrTextAniDirectionItem(SvStream& rIn)                           : SfxEnumItem(SDRATTR_TEXT_ANIDIRECTION,rIn)  {}
    virtual SfxPoolItem*   Clone(SfxItemPool* pPool=nullptr) const override;
    virtual SfxPoolItem*   Create(SvStream& rIn, sal_uInt16 nVer) const override;
    virtual sal_uInt16         GetValueCount() const override;
    SdrTextAniDirection GetValue() const      { return (SdrTextAniDirection)SfxEnumItem::GetValue(); }

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual OUString GetValueTextByPos(sal_uInt16 nPos) const override;

    virtual bool GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString& rText, const IntlWrapper * = nullptr) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
