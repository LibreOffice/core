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
#ifndef INCLUDED_EDITENG_COLRITEM_HXX
#define INCLUDED_EDITENG_COLRITEM_HXX

#include <svl/poolitem.hxx>
#include <tools/color.hxx>
#include <editeng/editengdllapi.h>

#define VERSION_USEAUTOCOLOR    1

/** SvxColorItem item describes a color.
*/
class EDITENG_DLLPUBLIC SvxColorItem final : public SfxPoolItem
{
private:
    Color mColor;
    sal_Int16 maThemeIndex;
    sal_Int16 maTintShade;
    /// Luminance Modulation: 100th percentage, defaults to 100%.
    sal_Int16 mnLumMod;
    /// Luminance Offset: 100th percentage, defaults to 0%.
    sal_Int16 mnLumOff;

public:
    static SfxPoolItem* CreateDefault();

    explicit SvxColorItem(const sal_uInt16 nId);
    SvxColorItem(const Color& aColor, const sal_uInt16 nId);
    virtual ~SvxColorItem() override;

    // "pure virtual Methods" from SfxPoolItem
    virtual bool operator==(const SfxPoolItem& rPoolItem) const override;
    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 MapUnit eCoreMetric, MapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper& rIntlWrapper) const override;

    virtual SvxColorItem* Clone(SfxItemPool* pPool = nullptr) const override;
    SvxColorItem(SvxColorItem const &) = default; // SfxPoolItem copy function dichotomy

    const Color& GetValue() const
    {
        return mColor;
    }
    void SetValue(const Color& rNewColor);

    sal_Int16 GetThemeIndex() const
    {
        return maThemeIndex;
    }

    void SetThemeIndex(sal_Int16 nIndex)
    {
        maThemeIndex = nIndex;
    }

    sal_Int16 GetTintOrShade() const
    {
        return maTintShade;
    }

    void SetTintOrShade(sal_Int16 nTintOrShade)
    {
        maTintShade = nTintOrShade;
    }

    sal_Int16 GetLumMod() const { return mnLumMod; }

    sal_Int16 GetLumOff() const { return mnLumOff; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
