/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/ColorSets.hxx>

#include <optional>
#include <unordered_set>
#include <docmodel/theme/ColorSet.hxx>

using namespace com::sun::star;

namespace svx
{

ColorSets::ColorSets()
{
    init();
}

ColorSets& ColorSets::get()
{
    static std::optional<ColorSets> sColorSet;
    if (!sColorSet)
        sColorSet = ColorSets();
    return *sColorSet;
}


void ColorSets::init()
{
    {
        model::ColorSet aColorSet(u"LibreOffice"_ustr);
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0x000000);
        aColorSet.add(model::ThemeColorType::Light2, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Accent1, 0x18A303);
        aColorSet.add(model::ThemeColorType::Accent2, 0x0369A3);
        aColorSet.add(model::ThemeColorType::Accent3, 0xA33E03);
        aColorSet.add(model::ThemeColorType::Accent4, 0x8E03A3);
        aColorSet.add(model::ThemeColorType::Accent5, 0xC99C00);
        aColorSet.add(model::ThemeColorType::Accent6, 0xC9211E);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x0000EE);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x551A8B);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet(u"Rainbow"_ustr);
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0x1C1C1C);
        aColorSet.add(model::ThemeColorType::Light2, 0xDDDDDD);
        aColorSet.add(model::ThemeColorType::Accent1, 0xFF0000);
        aColorSet.add(model::ThemeColorType::Accent2, 0xFF8000);
        aColorSet.add(model::ThemeColorType::Accent3, 0xFFFF00);
        aColorSet.add(model::ThemeColorType::Accent4, 0x00A933);
        aColorSet.add(model::ThemeColorType::Accent5, 0x2A6099);
        aColorSet.add(model::ThemeColorType::Accent6, 0x800080);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x0000EE);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x551A8B);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet(u"Beach"_ustr);
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0xFFBF00);
        aColorSet.add(model::ThemeColorType::Light2, 0x333333);
        aColorSet.add(model::ThemeColorType::Accent1, 0xFFF5CE);
        aColorSet.add(model::ThemeColorType::Accent2, 0xDEE6EF);
        aColorSet.add(model::ThemeColorType::Accent3, 0xE8F2A1);
        aColorSet.add(model::ThemeColorType::Accent4, 0xFFD7D7);
        aColorSet.add(model::ThemeColorType::Accent5, 0xDEE7E5);
        aColorSet.add(model::ThemeColorType::Accent6, 0xDDDBB6);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x7777EE);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0xEE77D7);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet(u"Sunset"_ustr);
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0x492300);
        aColorSet.add(model::ThemeColorType::Light2, 0xF6F9D4);
        aColorSet.add(model::ThemeColorType::Accent1, 0xFFFF00);
        aColorSet.add(model::ThemeColorType::Accent2, 0xFFBF00);
        aColorSet.add(model::ThemeColorType::Accent3, 0xFF8000);
        aColorSet.add(model::ThemeColorType::Accent4, 0xFF4000);
        aColorSet.add(model::ThemeColorType::Accent5, 0xBF0041);
        aColorSet.add(model::ThemeColorType::Accent6, 0x800080);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x0000EE);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x551A8B);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet(u"Ocean"_ustr);
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0x2A6099);
        aColorSet.add(model::ThemeColorType::Light2, 0xCCCCCC);
        aColorSet.add(model::ThemeColorType::Accent1, 0x800080);
        aColorSet.add(model::ThemeColorType::Accent2, 0x55308D);
        aColorSet.add(model::ThemeColorType::Accent3, 0x2A6099);
        aColorSet.add(model::ThemeColorType::Accent4, 0x158466);
        aColorSet.add(model::ThemeColorType::Accent5, 0x00A933);
        aColorSet.add(model::ThemeColorType::Accent6, 0x81D41A);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x0000EE);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x551A8B);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet(u"Forest"_ustr);
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0x000000);
        aColorSet.add(model::ThemeColorType::Light2, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Accent1, 0x813709);
        aColorSet.add(model::ThemeColorType::Accent2, 0x224B12);
        aColorSet.add(model::ThemeColorType::Accent3, 0x706E0C);
        aColorSet.add(model::ThemeColorType::Accent4, 0x355269);
        aColorSet.add(model::ThemeColorType::Accent5, 0xBE480A);
        aColorSet.add(model::ThemeColorType::Accent6, 0xBE480A);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x2A6099);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x800080);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet(u"Breeze"_ustr);
        aColorSet.add(model::ThemeColorType::Dark1, 0x232629);
        aColorSet.add(model::ThemeColorType::Light1, 0xFCFCFC);
        aColorSet.add(model::ThemeColorType::Dark2, 0x31363B);
        aColorSet.add(model::ThemeColorType::Light2, 0xEFF0F1);
        aColorSet.add(model::ThemeColorType::Accent1, 0xDA4453);
        aColorSet.add(model::ThemeColorType::Accent2, 0xF47750);
        aColorSet.add(model::ThemeColorType::Accent3, 0xFDBC4B);
        aColorSet.add(model::ThemeColorType::Accent4, 0xC9CE3B);
        aColorSet.add(model::ThemeColorType::Accent5, 0x1CDC9A);
        aColorSet.add(model::ThemeColorType::Accent6, 0x2ECC71);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x1D99F3);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x3DAEE9);
        maColorSets.push_back(aColorSet);
    }
}

model::ColorSet const* ColorSets::getColorSet(std::u16string_view rName) const
{
    for (const model::ColorSet & rColorSet : maColorSets)
    {
        if (rColorSet.getName() == rName)
            return &rColorSet;
    }
    return nullptr;
}
namespace
{

OUString findUniqueName(std::unordered_set<OUString> const& rNames, OUString const& rNewName)
{
    auto iterator = rNames.find(rNewName);
    if (iterator == rNames.cend())
        return rNewName;

    int i = 1;
    OUString aName;
    do
    {
        aName = rNewName + "_" + OUString::number(i);
        i++;
        iterator = rNames.find(aName);
    } while (iterator != rNames.cend());

    return aName;
}

} // end anonymous namespace

void ColorSets::insert(model::ColorSet const& rNewColorSet, IdenticalNameAction eAction)
{
    if (eAction == IdenticalNameAction::Overwrite)
    {
        for (model::ColorSet& rColorSet : maColorSets)
        {
            if (rColorSet.getName() == rNewColorSet.getName())
            {
                rColorSet = rNewColorSet;
                return;
            }
        }
        // color set not found, so insert it
        maColorSets.push_back(rNewColorSet);
    }
    else if (eAction == IdenticalNameAction::AutoRename)
    {
        std::unordered_set<OUString> aNames;
        for (model::ColorSet& rColorSet : maColorSets)
            aNames.insert(rColorSet.getName());

        OUString aName = findUniqueName(aNames, rNewColorSet.getName());

        model::ColorSet aNewColorSet = rNewColorSet;
        aNewColorSet.setName(aName);
        maColorSets.push_back(aNewColorSet);
    }
}

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
