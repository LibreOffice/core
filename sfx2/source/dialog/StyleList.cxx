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

#include<vcl/weld.hxx>
#include <StyleList.hxx>

void weld::StyleList::setStyleFamily(SfxStyleFamily family)
{
 /*   StyleTreeArr_Impl aArr;
    SfxStyleSheetBase* pStyle = pStyleSheetPool->First(family, SfxStyleSearchBits::AllVisible);

    while (pStyle)
    {
        StyleTree_Impl* pNew = new StyleTree_Impl(pStyle->GetName(), pStyle->GetParent());
        aArr.emplace_back(pNew);
        pStyle = pStyleSheetPool->Next();
    }

    OUString aCharUIName =  getDefaultStyleName(family);
    MakeTree_Impl(aArr, aCharUIName);
    std::vector<OUString> aCharEntries;
    MakeExpanded_Impl(*mxCharTreeBox, aCharEntries);
    mxCharTreeBox->freeze();
    mxCharTreeBox->clear();
    const sal_uInt16 nCharCount = aArr.size();
    const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
    const SfxStyleFamily eFam = pItem->GetFamily();
    if (eFam == SfxStyleFamily::Para)
    {
        for (sal_uInt16 i = 0; i < nCharCount; ++i)
        {
            mxCharTreeBox->set_visible(true);
            FillBox_Impl(*mxCharTreeBox, aArr[i].get(), aCharEntries, SfxStyleFamily::Char, nullptr);
            aArr[i].reset();
        }
    }
    SfxTemplateItem* pCharState = pFamilyState[0].get();
    mxCharTreeBox->thaw();

    std::unique_ptr<weld::TreeIter> xCharEntry = mxCharTreeBox->make_iterator();
    bool bCharEntry = mxCharTreeBox->get_iter_first(*xCharEntry);
    if (bCharEntry && nCharCount)
        mxCharTreeBox->expand_row(*xCharEntry);

    while (bCharEntry)
    {
        if (IsExpanded_Impl(aCharEntries, mxCharTreeBox->get_text(*xCharEntry)))
            mxCharTreeBox->expand_row(*xCharEntry);
        bCharEntry = mxCharTreeBox->iter_next(*xCharEntry);
    }

    OUString aCharStyle;
    if(pCharState)
        aCharStyle =  pCharState->GetStyleName();
    SelectStyle(aCharStyle, false);
    EnableDelete();*/
}

void weld::StyleList::setPreview(bool bPreview)
{
}

void weld::StyleList::setFilter(weld::ComboBox& combobox)
{
}
