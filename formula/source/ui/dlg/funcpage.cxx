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

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <formula/IFunctionDescription.hxx>

#include "funcpage.hxx"
#include <unotools/syslocale.hxx>
#include <unotools/charclass.hxx>
#include <unotools/textsearch.hxx>

namespace formula
{
IMPL_LINK(FuncPage, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    if (rKEvt.GetCharCode() == ' ')
    {
        aDoubleClickLink.Call(*this);
        return true;
    }
    return false;
}

// tdf#104487 - remember last used function category - set default to All category
sal_Int32 FuncPage::m_nRememberedFunctionCategory = ALL_CATEGORY;

FuncPage::FuncPage(weld::Container* pParent, const IFunctionManager* _pFunctionManager)
    : m_xBuilder(Application::CreateBuilder(pParent, u"formula/ui/functionpage.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_container(u"FunctionPage"_ustr))
    , m_xLbCategory(m_xBuilder->weld_combo_box(u"category"_ustr))
    , m_xLbFunction(m_xBuilder->weld_tree_view(u"function"_ustr))
    , m_xScratchIter(m_xLbFunction->make_iterator())
    , m_xLbFunctionSearchString(m_xBuilder->weld_entry(u"search"_ustr))
    , m_xSimilaritySearch(m_xBuilder->weld_check_button(u"similaritysearch"_ustr))
    , m_xHelpButton(m_xBuilder->weld_button(u"help"_ustr))
    , m_pFunctionManager(_pFunctionManager)
{
    m_aHelpId = m_xLbFunction->get_help_id();

    m_pFunctionManager->fillLastRecentlyUsedFunctions(aLRUList);
    m_pFunctionManager->fillFavouriteFunctions(aFavouritesList);

    const sal_uInt32 nCategoryCount = m_pFunctionManager->getCount();
    for (sal_uInt32 j = 0; j < nCategoryCount; ++j)
    {
        const IFunctionCategory* pCategory = m_pFunctionManager->getCategory(j);
        OUString sId(weld::toId(pCategory));
        m_xLbCategory->append(sId, pCategory->getName());
    }

    // tdf#104487 - remember last used function category
    m_xLbCategory->set_active(m_nRememberedFunctionCategory);
    OUString searchStr = m_xLbFunctionSearchString->get_text();
    UpdateFunctionList(searchStr);
    // lock to its initial size
    m_xLbFunction->set_size_request(m_xLbFunction->get_preferred_size().Width(),
                                    m_xLbFunction->get_height_rows(15));
    m_xLbCategory->connect_changed(LINK(this, FuncPage, SelComboBoxHdl));
    m_xLbFunction->connect_selection_changed(LINK(this, FuncPage, SelTreeViewHdl));
    m_xLbFunction->connect_row_activated(LINK(this, FuncPage, DblClkHdl));
    m_xLbFunction->connect_key_press(LINK(this, FuncPage, KeyInputHdl));
    m_xLbFunctionSearchString->connect_changed(LINK(this, FuncPage, ModifyHdl));
    m_xSimilaritySearch->connect_toggled(LINK(this, FuncPage, SimilarityToggleHdl));
    m_xHelpButton->connect_clicked(LINK(this, FuncPage, SelHelpClickHdl));

    m_xHelpButton->set_sensitive(false);
    m_xLbFunctionSearchString->grab_focus();
}

FuncPage::~FuncPage() {}

weld::TreeIter* FuncPage::FillCategoriesMap(const OUString& aCategory, bool bFill)
{
    if (!bFill)
        return nullptr;

    if (mCategories.find(aCategory) == mCategories.end())
    {
        mCategories[aCategory] = m_xLbFunction->make_iterator();
        m_xLbFunction->insert(nullptr, -1, &aCategory, nullptr, nullptr, nullptr, false,
                              mCategories[aCategory].get());
    }
    return mCategories[aCategory].get();
}

bool FuncPage::IsFavourite(sal_uInt16 nFIndex) const
{
    return aFavouritesList.find(nFIndex) != aFavouritesList.end();
}

bool FuncPage::UpdateFavouritesList()
{
    sal_uInt16 nFIndex = GetFuncIndex();
    if (nFIndex == 0)
        return false;

    if (IsFavourite(nFIndex))
        aFavouritesList.erase(nFIndex);
    else
        aFavouritesList.insert(nFIndex);
    return true;
}

void FuncPage::impl_addFunctions(const IFunctionCategory* _pCategory, bool bFillCategories)
{
    weld::TreeIter* pCategoryIter = FillCategoriesMap(_pCategory->getName(), bFillCategories);

    const sal_uInt32 nCount = _pCategory->getCount();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        TFunctionDesc pDesc(_pCategory->getFunction(i));
        if (!pDesc->isHidden())
        {
            OUString aFunction(pDesc->getFunctionName());
            OUString sId(weld::toId(pDesc));

            m_xLbFunction->insert(pCategoryIter, -1, &aFunction, &sId, nullptr, nullptr, false,
                                  m_xScratchIter.get());
        }
    }
}

void FuncPage::SearchFunction(const OUString& rFuncName, const OUString& rSearchString,
                              TFunctionDesc pDesc, const bool bSimilaritySearch)
{
    std::pair<sal_Int32, sal_Int32> score = std::make_pair(0, 0);
    if (bSimilaritySearch && !utl::TextSearch::SimilaritySearch(rFuncName, rSearchString, score))
        return;
    if (!bSimilaritySearch && rFuncName.indexOf(rSearchString) < 0
        && rSearchString.indexOf(rFuncName) < 0)
        return;

    sFuncScores.insert(std::make_pair(score, std::make_pair(rFuncName, pDesc)));
}

//aStr is non-empty when user types in the search box to search some function
void FuncPage::UpdateFunctionList(const OUString& aStr)
{
    m_xLbFunction->clear();
    m_xLbFunction->freeze();
    mCategories.clear();
    sFuncScores.clear();

    const sal_Int32 nSelPos = m_xLbCategory->get_active();
    bool bCollapse = nSelPos == ALL_CATEGORY;
    bool bFilter = !aStr.isEmpty();
    // tdf#104487 - remember last used function category
    m_nRememberedFunctionCategory = nSelPos;

    if (!bFilter || nSelPos == LRU_CATEGORY || nSelPos == FAVOURITES_CATEGORY)
    {
        const IFunctionCategory* pCategory
            = weld::fromId<const IFunctionCategory*>(m_xLbCategory->get_id(nSelPos));

        if (nSelPos >= ALL_CATEGORY)
        {
            if (pCategory == nullptr)
            {
                const sal_uInt32 nCount = m_pFunctionManager->getCount();
                for (sal_uInt32 i = 0; i < nCount; ++i)
                {
                    impl_addFunctions(m_pFunctionManager->getCategory(i), bCollapse);
                }
            }
            else
            {
                impl_addFunctions(pCategory, false);
            }
        }
        else if (nSelPos == LRU_CATEGORY) // LRU-List
        {
            for (auto const& elem : aLRUList)
            {
                if (elem) // may be null if a function is no longer available
                {
                    OUString aFunction(elem->getFunctionName());
                    OUString sId(weld::toId(elem));

                    m_xLbFunction->insert(nullptr, -1, &aFunction, &sId, nullptr, nullptr, false,
                                          m_xScratchIter.get());
                }
            }
        }
        else // Favourites-List
        {
            for (const auto& elem : aFavouritesList)
            {
                if (m_pFunctionManager->Get(elem))
                {
                    TFunctionDesc pDesc(m_pFunctionManager->Get(elem));
                    if (pDesc && !pDesc->isHidden())
                    {
                        OUString aFunction(pDesc->getFunctionName());
                        OUString sId(weld::toId(pDesc));

                        m_xLbFunction->insert(nullptr, -1, &aFunction, &sId, nullptr, nullptr,
                                              false, m_xScratchIter.get());
                    }
                }
            }
        }
    }
    else
    {
        SvtSysLocale aSysLocale;
        const CharClass& rCharClass = aSysLocale.GetCharClass();
        const OUString aSearchStr(rCharClass.uppercase(aStr));

        const sal_uInt32 nCategoryCount = m_pFunctionManager->getCount();
        // Category listbox holds additional entries for Last Used and All, so
        // the offset should be two but hard coded numbers are ugly...
        const sal_Int32 nCategoryOffset = m_xLbCategory->get_count() - nCategoryCount;
        // If a real category (not Last Used or All) is selected, list only
        // functions of that category. Else list all, LRU is handled above.
        sal_Int32 nCatBeg = (nSelPos == -1 ? -1 : nSelPos - nCategoryOffset);
        sal_uInt32 nCatEnd;
        if (nCatBeg < 0)
        {
            nCatBeg = 0;
            nCatEnd = nCategoryCount;
        }
        else
        {
            nCatEnd = nCatBeg + 1;
        }
        for (sal_uInt32 i = nCatBeg; i < nCatEnd; ++i)
        {
            const IFunctionCategory* pCategory = m_pFunctionManager->getCategory(i);
            const sal_uInt32 nFunctionCount = pCategory->getCount();
            for (sal_uInt32 j = 0; j < nFunctionCount; ++j)
            {
                TFunctionDesc pDesc(pCategory->getFunction(j));
                const OUString aFunction(rCharClass.uppercase(pDesc->getFunctionName()));
                SearchFunction(aFunction, aSearchStr, pDesc, m_xSimilaritySearch->get_active());
            }
        }

        for (const auto& func : sFuncScores)
        {
            TFunctionDesc pDesc(func.second.second);
            if (!pDesc->isHidden())
            {
                const OUString aCategory(pDesc->getCategory()->getName());
                const OUString aFunction(func.second.first);
                const OUString aFuncDescId(weld::toId(pDesc));
                weld::TreeIter* pCategory = FillCategoriesMap(aCategory, bCollapse);

                m_xLbFunction->insert(pCategory, -1, &aFunction, &aFuncDescId, nullptr, nullptr,
                                      false, m_xScratchIter.get());
            }
        }
    }

    m_xLbFunction->thaw();
    // Ensure no function is selected so the Next button doesn't overwrite a
    // function that is not in the list with an arbitrary selected one.
    m_xLbFunction->unselect_all();

    if (bCollapse && bFilter)
    {
        for (const auto& category : mCategories)
            m_xLbFunction->expand_row(*category.second);
    }

    if (IsVisible())
        SelTreeViewHdl(*m_xLbFunction);
}

IMPL_LINK_NOARG(FuncPage, SelComboBoxHdl, weld::ComboBox&, void)
{
    if (m_xLbCategory->get_active() == LRU_CATEGORY
        || m_xLbCategory->get_active() == FAVOURITES_CATEGORY)
        m_xLbFunctionSearchString->set_text(u""_ustr);
    m_xHelpButton->set_sensitive(false);
    m_xLbFunction->set_help_id(m_aHelpId);
    OUString searchStr = m_xLbFunctionSearchString->get_text();
    UpdateFunctionList(searchStr);
}

IMPL_LINK_NOARG(FuncPage, SelTreeViewHdl, weld::TreeView&, void)
{
    const IFunctionDescription* pDesc = GetFuncDesc();
    if (pDesc)
    {
        const OUString sHelpId = pDesc->getHelpId();
        if (!sHelpId.isEmpty())
            m_xLbFunction->set_help_id(sHelpId);
    }
    bool bSensitivity = weld::fromId<const IFunctionDescription*>(m_xLbFunction->get_selected_id());
    m_xHelpButton->set_sensitive(bSensitivity);
    aSelectionLink.Call(*this);
}

IMPL_LINK_NOARG(FuncPage, DblClkHdl, weld::TreeView&, bool)
{
    const OUString aString = m_xLbFunction->get_selected_text();
    if (mCategories.find(aString) != mCategories.end())
    {
        const auto& categoryRow = *(mCategories[aString]);
        if (m_xLbFunction->get_row_expanded(categoryRow))
            m_xLbFunction->collapse_row(categoryRow);
        else
            m_xLbFunction->expand_row(categoryRow);
        return true;
    }
    m_xLbFunctionSearchString->set_text(OUString());
    aDoubleClickLink.Call(*this);
    return true;
}

IMPL_LINK_NOARG(FuncPage, ModifyHdl, weld::Entry&, void)
{
    if (m_xLbCategory->get_active() == LRU_CATEGORY
        || m_xLbCategory->get_active() == FAVOURITES_CATEGORY)
    {
        m_xLbCategory->set_active(ALL_CATEGORY);
        m_xHelpButton->set_sensitive(false);
    }
    OUString searchStr = m_xLbFunctionSearchString->get_text();
    UpdateFunctionList(searchStr);
}

IMPL_LINK_NOARG(FuncPage, SimilarityToggleHdl, weld::Toggleable&, void)
{
    OUString searchStr = m_xLbFunctionSearchString->get_text();
    UpdateFunctionList(searchStr);
}

IMPL_LINK_NOARG(FuncPage, SelHelpClickHdl, weld::Button&, void)
{
    if (const auto pDesc
        = weld::fromId<const IFunctionDescription*>(m_xLbFunction->get_selected_id()))
    {
        if (Help* pHelp = Application::GetHelp())
        {
            const OUString sHelpId = pDesc->getHelpId();
            if (!sHelpId.isEmpty())
            {
                pHelp->Start(sHelpId);
            }
        }
    }
}

void FuncPage::SetCategory(sal_Int32 nCat)
{
    // tdf#104487 - remember last used function category
    m_nRememberedFunctionCategory = nCat;
    m_xLbCategory->set_active(nCat);
    UpdateFunctionList(OUString());
}

sal_Int32 FuncPage::GetFuncPos(const IFunctionDescription* _pDesc)
{
    return m_xLbFunction->find_id(weld::toId(_pDesc));
}

void FuncPage::SetFunction(sal_Int32 nFunc)
{
    if (nFunc == -1)
        m_xLbFunction->unselect_all();
    else
        m_xLbFunction->select(nFunc);
}

void FuncPage::SetFocus() { m_xLbFunction->grab_focus(); }

sal_Int32 FuncPage::GetCategory() const { return m_xLbCategory->get_active(); }

sal_Int32 FuncPage::GetCategoryEntryCount() const { return m_xLbCategory->get_count(); }

sal_Int32 FuncPage::GetFunction() const { return m_xLbFunction->get_selected_index(); }

sal_Int32 FuncPage::GetFunctionEntryCount() const { return m_xLbFunction->n_children(); }

OUString FuncPage::GetSelFunctionName() const { return m_xLbFunction->get_selected_text(); }

sal_uInt16 FuncPage::GetFuncIndex() const
{
    return m_pFunctionManager->getFunctionIndex(GetFuncDesc());
}

const IFunctionDescription* FuncPage::GetFuncDesc() const
{
    if (GetFunction() == -1)
        return nullptr;
    // not pretty, but hopefully rare
    return weld::fromId<const IFunctionDescription*>(m_xLbFunction->get_selected_id());
}

} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
