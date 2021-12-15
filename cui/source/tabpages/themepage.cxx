/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/beans/PropertyValues.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <editeng/editids.hrc>
#include <sal/log.hxx>
#include <svl/grabbagitem.hxx>

#include <themepage.hxx>

using namespace com::sun::star;

const WhichRangesContainer
    SvxThemePage::m_pRanges(svl::Items<SID_ATTR_CHAR_GRABBAG, SID_ATTR_CHAR_GRABBAG>);

SvxThemePage::SvxThemePage(weld::Container* pPage, weld::DialogController* pController,
                           const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "cui/ui/themetabpage.ui", "ThemePage", &rInAttrs)
    , m_xThemeName(m_xBuilder->weld_entry("themeName"))
    , m_xColorSetName(m_xBuilder->weld_entry("colorSetName"))
{
}

SvxThemePage::~SvxThemePage() = default;

void SvxThemePage::Reset(const SfxItemSet* pAttrs)
{
    const SfxPoolItem* pItem = nullptr;
    if (!pAttrs->HasItem(SID_ATTR_CHAR_GRABBAG, &pItem))
    {
        SAL_WARN("cui.tabpages", "SvxThemePage::Reset: no SfxGrabBagItem");
        return;
    }

    const auto& rGrabBagItem = static_cast<const SfxGrabBagItem&>(*pItem);
    auto itTheme = rGrabBagItem.GetGrabBag().find("Theme");
    if (itTheme == rGrabBagItem.GetGrabBag().end())
    {
        SAL_WARN("cui.tabpages", "SvxThemePage::Reset: no Theme");
        return;
    }

    comphelper::SequenceAsHashMap aMap(itTheme->second);
    auto it = aMap.find("Name");
    if (it != aMap.end())
    {
        OUString aName;
        it->second >>= aName;
        m_xThemeName->set_text(aName);
    }

    it = aMap.find("ColorSchemeName");
    if (it != aMap.end())
    {
        OUString aName;
        it->second >>= aName;
        m_xColorSetName->set_text(aName);
    }
}

bool SvxThemePage::FillItemSet(SfxItemSet* pAttrs)
{
    const SfxItemSet& rOldSet = GetItemSet();

    if (rOldSet.HasItem(SID_ATTR_CHAR_GRABBAG))
    {
        SfxGrabBagItem aGrabBagItem(
            static_cast<const SfxGrabBagItem&>(rOldSet.Get(SID_ATTR_CHAR_GRABBAG)));

        comphelper::SequenceAsHashMap aMap;
        auto it = aGrabBagItem.GetGrabBag().find("Theme");
        if (it != aGrabBagItem.GetGrabBag().end())
        {
            aMap << it->second;
        }

        aMap["Name"] <<= m_xThemeName->get_text();
        aMap["ColorSchemeName"] <<= m_xColorSetName->get_text();

        beans::PropertyValues aTheme = aMap.getAsConstPropertyValueList();
        aGrabBagItem.GetGrabBag()["Theme"] <<= aTheme;
        pAttrs->Put(aGrabBagItem);
    }

    return true;
}

std::unique_ptr<SfxTabPage> SvxThemePage::Create(weld::Container* pPage,
                                                 weld::DialogController* pController,
                                                 const SfxItemSet* rAttrs)
{
    return std::make_unique<SvxThemePage>(pPage, pController, *rAttrs);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
