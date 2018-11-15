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
#ifndef INCLUDED_SVX_SDTFSITM_HXX
#define INCLUDED_SVX_SDTFSITM_HXX

#include <com/sun/star/drawing/TextFitToSizeType.hpp>

#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdllapi.h>


// No automatic line breaks for AutoGrowingWidth as well (only if
// TextMaxFrameWidth is reached).


class SVX_DLLPUBLIC SdrTextFitToSizeTypeItem
    : public SfxEnumItem<css::drawing::TextFitToSizeType>
{
public:
    static SfxPoolItem* CreateDefault();
    SdrTextFitToSizeTypeItem(
            css::drawing::TextFitToSizeType const eFit = css::drawing::TextFitToSizeType_NONE)
        : SfxEnumItem(SDRATTR_TEXT_FITTOSIZE, eFit) {}
    virtual SfxPoolItem*     Clone(SfxItemPool* pPool=nullptr) const override;
    virtual sal_uInt16       GetValueCount() const override;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    static OUString          GetValueTextByPos(sal_uInt16 nPos);
    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric, MapUnit ePresMetric, OUString& rText, const IntlWrapper&) const override;
    virtual bool             HasBoolValue() const override;
    virtual bool             GetBoolValue() const override;
    virtual void             SetBoolValue(bool bVal) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
