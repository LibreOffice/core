/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "sal/log.hxx"
#include <memory>
#include <sfx2/ColorSets.hxx>

ColorSet::ColorSet(OUString const& aColorSetName)
    : maColorSetName(aColorSetName)
    , maColors(12)
{
}

ColorSets::ColorSets()
    : maColorSets()
    , maVirtualThemeColorSets()
    , mnThemeColorSetIndex(0)
{
    {
        ColorSet aColorSet("Breeze");
        aColorSet.add(0, 0xFCFCFC);
        aColorSet.add(1, 0x232629);
        aColorSet.add(2, 0xEFF0F1);
        aColorSet.add(3, 0x31363B);
        aColorSet.add(4, 0xDA4453);
        aColorSet.add(5, 0xF47750);
        aColorSet.add(6, 0xFDBC4B);
        aColorSet.add(7, 0xC9CE3B);
        aColorSet.add(8, 0x1CDC9A);
        aColorSet.add(9, 0x2ECC71);
        aColorSet.add(10, 0x1D99F3);
        aColorSet.add(11, 0x3DAEE9);
        addColorSet(aColorSet);
    }
    {
        ColorSet aColorSet("Material Blue");
        aColorSet.add(0, 0xFFFFFF);
        aColorSet.add(1, 0x212121);
        aColorSet.add(2, 0xECEFF1);
        aColorSet.add(3, 0x37474F);
        aColorSet.add(4, 0x7986CB);
        aColorSet.add(5, 0x303F9F);
        aColorSet.add(6, 0x64B5F6);
        aColorSet.add(7, 0x1976D2);
        aColorSet.add(8, 0x4FC3F7);
        aColorSet.add(9, 0x0277BD);
        aColorSet.add(10, 0x4DD0E1);
        aColorSet.add(11, 0x0097A7);
        addColorSet(aColorSet);
    }
    {
        ColorSet aColorSet("Material Red");
        aColorSet.add(0, 0xFFFFFF);
        aColorSet.add(1, 0x212121);
        aColorSet.add(2, 0xF5F5F5);
        aColorSet.add(3, 0x424242);
        aColorSet.add(4, 0xFF9800);
        aColorSet.add(5, 0xFF6D00);
        aColorSet.add(6, 0xFF5722);
        aColorSet.add(7, 0xDD2C00);
        aColorSet.add(8, 0xF44336);
        aColorSet.add(9, 0xD50000);
        aColorSet.add(10, 0xE91E63);
        aColorSet.add(11, 0xC51162);
        addColorSet(aColorSet);
    }
    {
        ColorSet aColorSet("Material Green");
        aColorSet.add(0, 0xFFFFFF);
        aColorSet.add(1, 0x212121);
        aColorSet.add(2, 0xF5F5F5);
        aColorSet.add(3, 0x424242);
        aColorSet.add(4, 0x009688);
        aColorSet.add(5, 0x00bfa5);
        aColorSet.add(6, 0x4caf50);
        aColorSet.add(7, 0x00c853);
        aColorSet.add(8, 0x8bc34a);
        aColorSet.add(9, 0x64dd17);
        aColorSet.add(10, 0xcddc39);
        aColorSet.add(11, 0xaeea00);
        addColorSet(aColorSet);
    }
}

ColorSets::~ColorSets() {}

const ColorSet& ColorSets::getColorSet(std::u16string_view rName) const
{
    for (const std::shared_ptr<ColorSet>& rColorSet : maColorSets)
    {
        if (rColorSet->getName() == rName)
            return *rColorSet;
    }
    return *maColorSets[0];
}

const std::shared_ptr<ColorSet>& ColorSets::getColorSetPtr(std::u16string_view rName) const
{
    for (const std::shared_ptr<ColorSet>& rColorSet : maColorSets)
    {
        if (rColorSet->getName() == rName)
            return rColorSet;
    }
    return maColorSets[0];
}

void ColorSets::setThemeColorSet(sal_Int32 nIndex) { mnThemeColorSetIndex = nIndex; }

void ColorSets::setThemeColorSet(std::u16string_view rName)
{
    for (sal_uInt32 nIndex = 0; nIndex < maColorSets.size(); ++nIndex)
    {
        if (maColorSets[nIndex]->getName() == rName)
        {
            mnThemeColorSetIndex = nIndex;
            break;
        }
    }
}

ColorSets* ColorSets::getColorSetsFromItemSet(const SfxItemSet& rSet)
{
    if (const SfxColorSetListItem* const pColorSetListItem = rSet.GetItem(SID_COLOR_SETS))
    {
        return &pColorSetListItem->GetSfxColorSetList();
    }
    else
    {
        return nullptr;
    }
}

SfxColorSetListItem::SfxColorSetListItem() {}

SfxColorSetListItem::SfxColorSetListItem(const ColorSets& rColorSets, sal_uInt16 nW)
    : SfxPoolItem(nW)
{
    mpColorSets = std::make_shared<ColorSets>(rColorSets);
}

SfxColorSetListItem::SfxColorSetListItem(const SfxColorSetListItem& rItem)
    : SfxPoolItem(rItem)
    , mpColorSets(rItem.mpColorSets)
{
}

bool SfxColorSetListItem::GetPresentation(SfxItemPresentation /*ePres*/, MapUnit /*eCoreUnit*/,
                                          MapUnit /*ePresUnit*/, OUString& rText,
                                          const IntlWrapper&) const
{
    rText.clear();
    return false;
}

bool SfxColorSetListItem::operator==(const SfxPoolItem& rItem) const
{
    assert(SfxPoolItem::operator==(rItem));
    // TODO: check these out before pushing...
    return false;
    //    return static_cast<const SfxColorSetListItem&>(rItem).mrColorSets == mrColorSets;
}

SfxColorSetListItem* SfxColorSetListItem::Clone(SfxItemPool*) const
{
    return new SfxColorSetListItem(*this);
}

/// TODO: check these out before pushing...
bool SfxColorSetListItem::QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId) const
{
    return false;
}

bool SfxColorSetListItem::PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) { return false; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
