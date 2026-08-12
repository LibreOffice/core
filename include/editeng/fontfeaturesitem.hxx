/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>
#include <vcl/font/Feature.hxx>

class EDITENG_DLLPUBLIC SvxFontFeaturesItem final : public SfxPoolItem
{
    std::vector<vcl::font::FeatureSetting> maFeatures;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SvxFontFeaturesItem)
    static SfxPoolItem* CreateDefault();

    SvxFontFeaturesItem(sal_uInt16 nId);
    SvxFontFeaturesItem(std::vector<vcl::font::FeatureSetting> aFeatures, sal_uInt16 nId);

    const std::vector<vcl::font::FeatureSetting>& GetFeatures() const { return maFeatures; }

    virtual bool operator==(const SfxPoolItem& rItem) const override;
    virtual bool supportsHashCode() const override { return true; }
    virtual size_t hashCode() const override;
    virtual SvxFontFeaturesItem* Clone(SfxItemPool* pPool = nullptr) const override;

    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric,
                                 MapUnit ePresMetric, OUString& rText,
                                 const IntlWrapper&) const override;

    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
