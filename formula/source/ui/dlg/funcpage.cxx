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
#include <formula/IFunctionDescription.hxx>

#include "funcpage.hxx"
#include <unotools/syslocale.hxx>
#include <unotools/charclass.hxx>

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
sal_Int32 FuncPage::m_nRememberedFunctionCategory = 1;

FuncPage::FuncPage(weld::Container* pParent, const IFunctionManager* _pFunctionManager)
    : m_xBuilder(Application::CreateBuilder(pParent, u"formula/ui/functionpage.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_container(u"FunctionPage"_ustr))
    , m_xLbCategory(m_xBuilder->weld_combo_box(u"category"_ustr))
    , m_xLbFunction(m_xBuilder->weld_tree_view(u"function"_ustr))
    , m_xLbFunctionSearchString(m_xBuilder->weld_entry(u"search"_ustr))
    , m_pFunctionManager(_pFunctionManager)
{
    m_xLbFunction->make_sorted();
    m_aHelpId = m_xLbFunction->get_help_id();

    m_pFunctionManager->fillLastRecentlyUsedFunctions(aLRUList);

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
    m_xLbFunction->connect_changed(LINK(this, FuncPage, SelTreeViewHdl));
    m_xLbFunction->connect_row_activated(LINK(this, FuncPage, DblClkHdl));
    m_xLbFunction->connect_key_press(LINK(this, FuncPage, KeyInputHdl));
    m_xLbFunctionSearchString->connect_changed(LINK(this, FuncPage, ModifyHdl));

    m_xLbFunctionSearchString->grab_focus();
}

FuncPage::~FuncPage() {}

void FuncPage::impl_addFunctions(const IFunctionCategory* _pCategory)
{
    const sal_uInt32 nCount = _pCategory->getCount();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        TFunctionDesc pDesc(_pCategory->getFunction(i));
        if (!pDesc->isHidden())
        {
            OUString sId(weld::toId(pDesc));
            m_xLbFunction->append(sId, pDesc->getFunctionName());
        }
    }
}

//aStr is non-empty when user types in the search box to search some function
void FuncPage::UpdateFunctionList(const OUString& aStr)
{
    m_xLbFunction->clear();
    m_xLbFunction->freeze();

    const sal_Int32 nSelPos = m_xLbCategory->get_active();
    // tdf#104487 - remember last used function category
    m_nRememberedFunctionCategory = nSelPos;

    if (aStr.isEmpty() || nSelPos == 0)
    {
        const IFunctionCategory* pCategory
            = weld::fromId<const IFunctionCategory*>(m_xLbCategory->get_id(nSelPos));

        if (nSelPos > 0)
        {
            if (pCategory == nullptr)
            {
                const sal_uInt32 nCount = m_pFunctionManager->getCount();
                for (sal_uInt32 i = 0; i < nCount; ++i)
                {
                    impl_addFunctions(m_pFunctionManager->getCategory(i));
                }
            }
            else
            {
                impl_addFunctions(pCategory);
            }
        }
        else // LRU-List
        {
            for (auto const& elem : aLRUList)
            {
                if (elem) // may be null if a function is no longer available
                {
                    OUString sId(weld::toId(elem));
                    m_xLbFunction->append(sId, elem->getFunctionName());
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
                // tdf#146781 - search for the desired function also in the description
                if (rCharClass.uppercase(pDesc->getFunctionName()).indexOf(aSearchStr) >= 0
                    || rCharClass.uppercase(pDesc->getDescription()).indexOf(aSearchStr) >= 0)
                {
                    if (!pDesc->isHidden())
                    {
                        OUString sId(weld::toId(pDesc));
                        m_xLbFunction->append(sId, pDesc->getFunctionName());
                    }
                }
            }
        }
    }

    m_xLbFunction->thaw();
    // Ensure no function is selected so the Next button doesn't overwrite a
    // function that is not in the list with an arbitrary selected one.
    m_xLbFunction->unselect_all();

    if (IsVisible())
        SelTreeViewHdl(*m_xLbFunction);
}

IMPL_LINK_NOARG(FuncPage, SelComboBoxHdl, weld::ComboBox&, void)
{
    OUString searchStr = m_xLbFunctionSearchString->get_text();
    m_xLbFunction->set_help_id(m_aHelpId);
    UpdateFunctionList(searchStr);
}

IMPL_LINK_NOARG(FuncPage, SelTreeViewHdl, weld::TreeView&, void)
{
    const IFunctionDescription* pDesc = GetFuncDesc(GetFunction());
    if (pDesc)
    {
        const OUString sHelpId = pDesc->getHelpId();
        if (!sHelpId.isEmpty())
            m_xLbFunction->set_help_id(sHelpId);
    }
    aSelectionLink.Call(*this);
}

IMPL_LINK_NOARG(FuncPage, DblClkHdl, weld::TreeView&, bool)
{
    aDoubleClickLink.Call(*this);
    return true;
}

IMPL_LINK_NOARG(FuncPage, ModifyHdl, weld::Entry&, void)
{
    // While typing select All category.
    m_xLbCategory->set_active(1);
    OUString searchStr = m_xLbFunctionSearchString->get_text();
    UpdateFunctionList(searchStr);
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

const IFunctionDescription* FuncPage::GetFuncDesc(sal_Int32 nPos) const
{
    if (nPos == -1)
        return nullptr;
    // not pretty, but hopefully rare
    return weld::fromId<const IFunctionDescription*>(m_xLbFunction->get_id(nPos));
}

} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
