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
#include <com/sun/star/util/Color.hpp>

#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/editids.hrc>
#include <sal/log.hxx>
#include <svl/grabbagitem.hxx>
#include <svx/colorbox.hxx>

#include <themepage.hxx>

using namespace com::sun::star;

const WhichRangesContainer
    SvxThemePage::m_pRanges(svl::Items<SID_ATTR_CHAR_GRABBAG, SID_ATTR_CHAR_GRABBAG>);

SvxThemePage::SvxThemePage(weld::Container* pPage, weld::DialogController* pController,
                           const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "cui/ui/themetabpage.ui", "ThemePage", &rInAttrs)
    , m_xThemeName(m_xBuilder->weld_entry("themeName"))
    , m_xColorSetName(m_xBuilder->weld_entry("colorSetName"))
    , m_xDk1(new ColorListBox(m_xBuilder->weld_menu_button("btnDk1"),
                              [this] { return GetDialogController()->getDialog(); }))
    , m_xLt1(new ColorListBox(m_xBuilder->weld_menu_button("btnLt1"),
                              [this] { return GetDialogController()->getDialog(); }))
    , m_xDk2(new ColorListBox(m_xBuilder->weld_menu_button("btnDk2"),
                              [this] { return GetDialogController()->getDialog(); }))
    , m_xLt2(new ColorListBox(m_xBuilder->weld_menu_button("btnLt2"),
                              [this] { return GetDialogController()->getDialog(); }))
    , m_xAccent1(new ColorListBox(m_xBuilder->weld_menu_button("btnAccent1"),
                                  [this] { return GetDialogController()->getDialog(); }))
    , m_xAccent2(new ColorListBox(m_xBuilder->weld_menu_button("btnAccent2"),
                                  [this] { return GetDialogController()->getDialog(); }))
    , m_xAccent3(new ColorListBox(m_xBuilder->weld_menu_button("btnAccent3"),
                                  [this] { return GetDialogController()->getDialog(); }))
    , m_xAccent4(new ColorListBox(m_xBuilder->weld_menu_button("btnAccent4"),
                                  [this] { return GetDialogController()->getDialog(); }))
    , m_xAccent5(new ColorListBox(m_xBuilder->weld_menu_button("btnAccent5"),
                                  [this] { return GetDialogController()->getDialog(); }))
    , m_xAccent6(new ColorListBox(m_xBuilder->weld_menu_button("btnAccent6"),
                                  [this] { return GetDialogController()->getDialog(); }))
    , m_xHlink(new ColorListBox(m_xBuilder->weld_menu_button("btnHlink"),
                                [this] { return GetDialogController()->getDialog(); }))
    , m_xFolHlink(new ColorListBox(m_xBuilder->weld_menu_button("btnFolHlink"),
                                   [this] { return GetDialogController()->getDialog(); }))
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

    it = aMap.find("ColorScheme");
    if (it != aMap.end())
    {
        uno::Sequence<util::Color> aColors;
        it->second >>= aColors;
        m_xDk1->SelectEntry(Color(ColorTransparency, aColors[0]));
        m_xLt1->SelectEntry(Color(ColorTransparency, aColors[1]));
        m_xDk2->SelectEntry(Color(ColorTransparency, aColors[2]));
        m_xLt2->SelectEntry(Color(ColorTransparency, aColors[3]));
        m_xAccent1->SelectEntry(Color(ColorTransparency, aColors[4]));
        m_xAccent2->SelectEntry(Color(ColorTransparency, aColors[5]));
        m_xAccent3->SelectEntry(Color(ColorTransparency, aColors[6]));
        m_xAccent4->SelectEntry(Color(ColorTransparency, aColors[7]));
        m_xAccent5->SelectEntry(Color(ColorTransparency, aColors[8]));
        m_xAccent6->SelectEntry(Color(ColorTransparency, aColors[9]));
        m_xHlink->SelectEntry(Color(ColorTransparency, aColors[10]));
        m_xFolHlink->SelectEntry(Color(ColorTransparency, aColors[11]));
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
        std::vector<util::Color> aColorScheme = {
            static_cast<sal_Int32>(m_xDk1->GetSelectEntryColor()),
            static_cast<sal_Int32>(m_xLt1->GetSelectEntryColor()),
            static_cast<sal_Int32>(m_xDk2->GetSelectEntryColor()),
            static_cast<sal_Int32>(m_xLt2->GetSelectEntryColor()),
            static_cast<sal_Int32>(m_xAccent1->GetSelectEntryColor()),
            static_cast<sal_Int32>(m_xAccent2->GetSelectEntryColor()),
            static_cast<sal_Int32>(m_xAccent3->GetSelectEntryColor()),
            static_cast<sal_Int32>(m_xAccent4->GetSelectEntryColor()),
            static_cast<sal_Int32>(m_xAccent5->GetSelectEntryColor()),
            static_cast<sal_Int32>(m_xAccent6->GetSelectEntryColor()),
            static_cast<sal_Int32>(m_xHlink->GetSelectEntryColor()),
            static_cast<sal_Int32>(m_xFolHlink->GetSelectEntryColor()),
        };
        aMap["ColorScheme"] <<= comphelper::containerToSequence(aColorScheme);

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
