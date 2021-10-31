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

#include <memory>
#include <vector>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sfx2/dllapi.h>
#include <sfx2/sfxsids.hrc>
#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <tools/color.hxx>

class SfxColorSetListItem;

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

class VirtualThemeColorSet
{
public:
    VirtualThemeColorSet() = default;
    VirtualThemeColorSet(const std::shared_ptr<ColorSet>& pColorSet)
        : mpThemeColorSet(pColorSet)
    {
    }
    const std::weak_ptr<ColorSet>& getColorSetPtr() { return mpThemeColorSet; }

private:
    std::weak_ptr<ColorSet> mpThemeColorSet = std::weak_ptr<ColorSet>();
};

class SFX2_DLLPUBLIC ColorSets
{
    std::vector<std::shared_ptr<ColorSet>> maColorSets;
    std::vector<std::shared_ptr<VirtualThemeColorSet>> maVirtualThemeColorSets;
    // TEMP: should remove
    sal_Int32 mnThemeColorSetIndex;

public:
    ColorSets();
    ~ColorSets();

    // TEMP: should remove
    const ColorSet& getThemeColorSet() const { return getColorSet(mnThemeColorSetIndex); }

    // TEMP: should remove
    void setThemeColorSet(sal_Int32 nIndex);
    void setThemeColorSet(std::u16string_view rName);

    const std::vector<std::shared_ptr<ColorSet>>& getColorSets() const { return maColorSets; }

    const ColorSet& getColorSet(sal_Int32 nIndex) const { return *maColorSets[nIndex]; }
    const std::shared_ptr<ColorSet>& getColorSetPtr(sal_Int32 nIndex) const
    {
        return maColorSets[nIndex];
    }
    const ColorSet& getColorSet(std::u16string_view rName) const;
    const std::shared_ptr<ColorSet>& getColorSetPtr(std::u16string_view rName) const;

    int addColorSet(const ColorSet& rColorSet)
    {
        maColorSets.emplace_back(std::make_shared<ColorSet>(rColorSet));
        return maColorSets.size() - 1;
    }

    const std::shared_ptr<VirtualThemeColorSet>& getVirtualColorSet(int nVirtualColorSetIndex) const
    {
        // HACK: dont forget to change this... :)
        return maVirtualThemeColorSets[nVirtualColorSetIndex];
    }

    sal_Int32
    getVirtualColorSetIndex(const std::weak_ptr<VirtualThemeColorSet>& pVirtualColorSet) const
    {
        if (auto aIt = std::find_if(
                maVirtualThemeColorSets.begin(), maVirtualThemeColorSets.end(),
                [pVirtualColorSet](const std::shared_ptr<VirtualThemeColorSet>& pVirtualColorSet_) {
                    return pVirtualColorSet_.get() == pVirtualColorSet.lock().get();
                });
            aIt != maVirtualThemeColorSets.end())
        {
            return aIt - maVirtualThemeColorSets.begin();
        }
        return -1;
    }

    int addVirtualThemeColorSet(int nIndexColorSetToReference)
    {
        maVirtualThemeColorSets.emplace_back(
            std::make_shared<VirtualThemeColorSet>(getColorSetPtr(nIndexColorSetToReference)));
        return maVirtualThemeColorSets.size() - 1;
    }

    static ColorSets* getColorSetsFromItemSet(const SfxItemSet& rSet);
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

    ColorSets& GetSfxColorSetList() const { return *mpColorSets; }
    std::shared_ptr<ColorSets> GetSfxColorSetListPtr() const { return mpColorSets; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
