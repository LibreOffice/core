/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vector>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sfx2/dllapi.h>
#include <svl/poolitem.hxx>
#include <tools/color.hxx>

class ColorSet
{
    OUString maColorSetName;
    std::vector<Color> maColors;

public:
    ColorSet(OUString const& aName);

    void add(sal_uInt32 nIndex, ::Color aColorData) { maColors[nIndex] = aColorData; }

    const OUString& getName() const { return maColorSetName; }
    const Color& getColor(sal_uInt32 nIndex) const { return maColors[nIndex]; }

    const std::vector<Color>& getColors() const { return maColors; }
};

class SFX2_DLLPUBLIC ColorSets
{
    std::vector<ColorSet> maColorSets;
    sal_Int32 mnThemeColorSetIndex;

public:
    ColorSets();
    ~ColorSets();

    static ColorSets& get()
    {
        static ColorSets aColorSetsInstance;
        return aColorSetsInstance;
    }

    const ColorSet& getThemeColorSet() const { return getColorSet(mnThemeColorSetIndex); }

    void setThemeColorSet(sal_Int32 nIndex);

    void setThemeColorSet(std::u16string_view rName);

    const std::vector<ColorSet>& getColorSets() const { return maColorSets; }

    const ColorSet& getColorSet(sal_uInt32 nIndex) const { return maColorSets[nIndex]; }

    const ColorSet& getColorSet(std::u16string_view rName) const;
};

class SFX2_DLLPUBLIC SfxColorSetListItem final : public SfxPoolItem
{
    std::shared_ptr<ColorSets> mpColorSets;

public:
    static SfxPoolItem* CreateDefault();
    SfxColorSetListItem();
    SfxColorSetListItem(const ColorSets& rColorSets, sal_uInt16 nWhich);
    SfxColorSetListItem(const SfxColorSetListItem&);

    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric,
                                 MapUnit ePresMetric, OUString& rText,
                                 const IntlWrapper&) const override;

    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SfxColorSetListItem* Clone(SfxItemPool* pPool = nullptr) const override;
    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    const ColorSets& GetSfxColorSetList() const { return *mpColorSets; }
    std::shared_ptr<ColorSets> GetSfxColorSetListPtr() const { return mpColorSets; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
