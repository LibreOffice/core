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

#include <basctl/basctldllpublic.hxx>
#include <comphelper/string.hxx>
#include <editeng/editview.hxx>
#include <sfx2/viewsh.hxx>
#include <formula/funcvarargs.h>
#include <unotools/charclass.hxx>
#include <unotools/textsearch.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>

#include <global.hxx>
#include <scmod.hxx>
#include <sc.hrc>
#include <svl/stritem.hxx>
#include <inputhdl.hxx>
#include <tabvwsh.hxx>
#include <funcdesc.hxx>
#include <compiler.hxx>

#include <dwfunctr.hxx>

/*************************************************************************
#*  Member:     ScFunctionWin
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   Constructor of ScFunctionWin Class
#*
#*  Input:      Sfx - links, window, resource
#*
#*  Output:     ---
#*
#************************************************************************/

ScFunctionWin::ScFunctionWin(weld::Widget* pParent, SfxBindings* pBindings)
    : PanelLayout(pParent, u"FunctionPanel"_ustr, u"modules/scalc/ui/functionpanel.ui"_ustr)
    , xCatBox(m_xBuilder->weld_combo_box(u"category"_ustr))
    , xFuncList(m_xBuilder->weld_tree_view(u"funclist"_ustr))
    , xScratchIter(xFuncList->make_iterator())
    , xInsertButton(m_xBuilder->weld_button(u"insert"_ustr))
    , xHelpButton(m_xBuilder->weld_button(u"help"_ustr))
    , xSimilaritySearch(m_xBuilder->weld_check_button(u"similaritysearch"_ustr))
    , xFiFuncDesc(m_xBuilder->weld_text_view(u"funcdesc"_ustr))
    , m_xSearchString(m_xBuilder->weld_entry(u"search"_ustr))
    , m_pBindings(pBindings)
    , xConfigListener(new comphelper::ConfigurationListener(u"/org.openoffice.Office.Calc/Formula/Syntax"_ustr))
    , xConfigChange(std::make_unique<EnglishFunctionNameChange>(xConfigListener, this))
    , pFuncDesc(nullptr)
{
    InitLRUList();

    nArgs=0;
    m_aListHelpId = xFuncList->get_help_id();
    m_aSearchHelpId = m_xSearchString->get_help_id();

    // Description box has a height of 8 lines of text
    xFiFuncDesc->set_size_request(-1, 8 * xFiFuncDesc->get_text_height());

    m_xSearchString->connect_changed(LINK(this, ScFunctionWin, ModifyHdl));
    m_xSearchString->connect_key_press(LINK(this, ScFunctionWin, KeyInputHdl));

    xCatBox->connect_changed(LINK( this, ScFunctionWin, SelComboHdl));
    xFuncList->connect_changed(LINK( this, ScFunctionWin, SelTreeHdl));

    xFuncList->connect_row_activated(LINK( this, ScFunctionWin, SetRowActivatedHdl));
    xInsertButton->connect_clicked(LINK( this, ScFunctionWin, SetSelectionClickHdl));
    xHelpButton->connect_clicked(LINK( this, ScFunctionWin, SetHelpClickHdl));
    xSimilaritySearch->connect_toggled(LINK(this, ScFunctionWin, SetSimilarityToggleHdl));

    xCatBox->set_active(0);

    SelComboHdl(*xCatBox);
}

/*************************************************************************
#*  Member:     ScFunctionWin
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   Destructor of ScFunctionWin Class
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

ScFunctionWin::~ScFunctionWin()
{
    xConfigChange.reset();
    xConfigListener->dispose();
    xConfigListener.clear();

    xCatBox.reset();
    xFuncList.reset();
    xInsertButton.reset();
    xHelpButton.reset();
    xSimilaritySearch.reset();
    xFiFuncDesc.reset();
}

/*************************************************************************
#*  Member:     InitLRUList
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   Updates the list of functions depending on the set category
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::InitLRUList()
{
    ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
    pFuncMgr->fillLastRecentlyUsedFunctions(aLRUList);

    sal_Int32 nSelPos  = xCatBox->get_active();

    if (nSelPos == 0)
        UpdateFunctionList(u""_ustr);
}


/*************************************************************************
#*  Member:     FillCategoriesMap
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   Fills the categories map.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

weld::TreeIter* ScFunctionWin::FillCategoriesMap(const OUString& aCategory, bool bFill)
{
    if (!bFill)
        return nullptr;

    if (mCategories.find(aCategory) == mCategories.end())
    {
        mCategories[aCategory] = xFuncList->make_iterator();
        xFuncList->insert(nullptr, -1, &aCategory, nullptr, nullptr, nullptr, false,
            mCategories[aCategory].get());
    }
    return mCategories[aCategory].get();
}

/*************************************************************************
#*  Member:     UpdateLRUList
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   Updates the list of last used functions.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::UpdateLRUList()
{
    if (pFuncDesc && pFuncDesc->nFIndex!=0)
    {
        ScModule::get()->InsertEntryToLRUList(pFuncDesc->nFIndex);
    }
}

void ScFunctionWin::SearchFunction(const OUString& rFuncName, const OUString& rSearchString,
                                   const ScFuncDesc* pDesc, const bool bSimilaritySearch)
{
    std::pair<sal_Int32, sal_Int32> score = std::make_pair(0, 0);
    if (bSimilaritySearch && !utl::TextSearch::SimilaritySearch(rFuncName, rSearchString, score))
        return;
    if (!bSimilaritySearch && rFuncName.indexOf(rSearchString) < 0
        && rSearchString.indexOf(rFuncName) < 0)
        return;

    sFuncScores.insert(std::make_pair(score, std::make_pair(rFuncName, pDesc)));
}

/*************************************************************************
#*  Member:     SetDescription
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::SetDescription()
{
    xFiFuncDesc->set_text(OUString());
    const ScFuncDesc* pDesc =
             weld::fromId<const ScFuncDesc*>(xFuncList->get_selected_id());
    if (pDesc)
    {
        pDesc->initArgumentInfo();      // full argument info is needed

        OUString aBuf = xFuncList->get_selected_text() +
            ":\n\n" +
            pDesc->GetParamList() +
            "\n\n" +
            *pDesc->mxFuncDesc;

        xFiFuncDesc->set_text(aBuf);

        // Update help ID for the selected entry
        const OUString sHelpId = pDesc->getHelpId();
        if (!sHelpId.isEmpty())
            xFuncList->set_help_id(pDesc->getHelpId());
        else
            xFuncList->set_help_id(m_aListHelpId);
    }
}

/*************************************************************************
#*  Member:     UpdateFunctionList
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   Updates the list of functions depending on the set category
#*
#*  Input:      Search string used to filter the list of functions
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::UpdateFunctionList(const OUString& rSearchString)
{
    sal_Int32  nSelPos   = xCatBox->get_active();
    sal_Int32  nCategory = ( -1 != nSelPos )
                            ? (nSelPos-1) : 0;

    xFuncList->clear();
    xFuncList->freeze();
    mCategories.clear();
    sFuncScores.clear();

    bool bCollapse = nCategory == 0;
    bool bFilter = !rSearchString.isEmpty();
    if ( nSelPos > 0 )
    {
        ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();

        // Use the corresponding CharClass for uppercase() depending on whether
        // English function names are used, or localized names.
        const CharClass* pCharClass = (ScGlobal::GetStarCalcFunctionList()->IsEnglishFunctionNames()
                ? ScCompiler::GetCharClassEnglish()
                : ScCompiler::GetCharClassLocalized());

        const OUString aSearchStr(pCharClass->uppercase(rSearchString));

        const ScFuncDesc* pDesc = pFuncMgr->First(nCategory);
        while (pDesc)
        {
            const OUString aCategory(pDesc->getCategory()->getName());
            const OUString aFunction(pCharClass->uppercase(pDesc->getFunctionName()));
            const OUString aFuncDescId(weld::toId(pDesc));

            if (bFilter)
                SearchFunction(aFunction, aSearchStr, pDesc, xSimilaritySearch->get_active());
            else
            {
                weld::TreeIter* pCategory = FillCategoriesMap(aCategory, bCollapse);
                xFuncList->insert(pCategory, -1, &aFunction, &aFuncDescId, nullptr, nullptr,
                            false, xScratchIter.get());
            }
            pDesc = pFuncMgr->Next();
        }

        for (const auto& func : sFuncScores)
        {
            pDesc = func.second.second;
            const OUString aCategory(pDesc->getCategory()->getName());
            const OUString aFunction(func.second.first);
            const OUString aFuncDescId(weld::toId(pDesc));
            weld::TreeIter* pCategory = FillCategoriesMap(aCategory, bCollapse);

            xFuncList->insert(pCategory, -1, &aFunction, &aFuncDescId, nullptr, nullptr, false,
                              xScratchIter.get());
        }
    }
    else // LRU list
    {
        for (const formula::IFunctionDescription* pDesc : aLRUList)
        {
            if (pDesc)
            {
                OUString aFunction = pDesc->getFunctionName();
                OUString aFuncDescId = weld::toId(pDesc);

                xFuncList->insert(nullptr, -1, &aFunction, &aFuncDescId, nullptr, nullptr,
                        false, xScratchIter.get());
            }
        }
    }

    xFuncList->thaw();

    if (bCollapse && bFilter)
    {
        for (const auto& category : mCategories)
            xFuncList->expand_row(*category.second);
    }

    if (xFuncList->n_children() > 0)
    {
        xFuncList->set_sensitive(true);
        xFuncList->select(0);
    }
    else
    {
        xFuncList->set_sensitive(false);
    }
}

/*************************************************************************
#*  Member:     DoEnter
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   Save input into document. Is called after clicking the
#*              Apply button or a double-click on the function list.
#*
#*  Input:      Boolean to know if I double-clicked/press-enter or not
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::DoEnter(bool bDoubleOrEnter)
{
    OUString aString=xFuncList->get_selected_text();
    const bool isCategory = mCategories.find(aString) != mCategories.end();
    if (isCategory && !bDoubleOrEnter)
        return;

    if (isCategory)
    {
        const auto& categoryRow = *(mCategories[aString]);
        if (xFuncList->get_row_expanded(categoryRow))
            xFuncList->collapse_row(categoryRow);
        else
            xFuncList->expand_row(categoryRow);
        return;
    }

    SfxViewShell* pCurSh = SfxViewShell::Current();
    nArgs=0;

    if(!aString.isEmpty())
    {
        const ScFuncDesc* pDesc =
             weld::fromId<const ScFuncDesc*>(xFuncList->get_selected_id());
        if (pDesc)
        {
            pFuncDesc=pDesc;
            UpdateLRUList();
            nArgs = pDesc->nArgCount;
        }
        InitLRUList();
    }

    const SfxStringItem aFunction(FN_PARAM_1, aString);
    // -1 when function-id is available, category index otherwise
    const SfxInt16Item nCategory(FN_PARAM_2, -1);
    const SfxStringItem aFunctionId(FN_PARAM_3, xFuncList->get_selected_id());
    GetBindings().GetDispatcher()->ExecuteList(SID_INS_FUNCTION, SfxCallMode::SYNCHRON,
                                               { &aFunction, &nCategory, &aFunctionId });

    if ( pCurSh )
    {
        vcl::Window* pShellWnd = pCurSh->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }

}

/*************************************************************************
#*  Handle:     ModifyHdl
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   Handles changes in the search text
#*
#************************************************************************/

IMPL_LINK_NOARG(ScFunctionWin, ModifyHdl, weld::Entry&, void)
{
    if (xCatBox->get_active() == 0)
    {
        xCatBox->set_active(1);
        xHelpButton->set_sensitive(false);
    }
    OUString searchStr = m_xSearchString->get_text();
    UpdateFunctionList(searchStr);
    SetDescription();
}

/*************************************************************************
#*  Handle:     KeyInputHdl
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   Processes key inputs when the search entry has focus
#*
#************************************************************************/

IMPL_LINK(ScFunctionWin, KeyInputHdl, const KeyEvent&, rEvent, bool)
{
    bool bHandled = false;

    switch (rEvent.GetKeyCode().GetCode())
    {
    case KEY_RETURN:
        {
            DoEnter(true);
            bHandled = true;
        }
        break;
    case KEY_DOWN:
        {
            int nNewIndex = std::min(xFuncList->get_selected_index() + 1, xFuncList->n_children() - 1);
            xFuncList->select(nNewIndex);
            SetDescription();
            bHandled = true;
        }
        break;
    case KEY_UP:
        {
            int nNewIndex = std::max(xFuncList->get_selected_index() - 1, 0);
            xFuncList->select(nNewIndex);
            SetDescription();
            bHandled = true;
        }
        break;
    case KEY_ESCAPE:
        {
            // Escape in an empty search field should move focus to the document,
            // adhering to Sidebar guidelines
            if (m_xSearchString->get_text().isEmpty())
            {
                if (SfxViewShell* pCurSh = SfxViewShell::Current())
                {
                    vcl::Window* pShellWnd = pCurSh->GetWindow();

                    if (pShellWnd)
                        pShellWnd->GrabFocusToDocument();
                }
                bHandled = true;
                break;
            }
            m_xSearchString->set_text(u""_ustr);
            UpdateFunctionList(u""_ustr);
            bHandled = true;
        }
        break;
    case KEY_F1:
        {
            const ScFuncDesc* pDesc = weld::fromId<const ScFuncDesc*>(xFuncList->get_selected_id());
            OUString sHelpId;
            if (pDesc)
                sHelpId = pDesc->getHelpId();

            if (!sHelpId.isEmpty())
                m_xSearchString->set_help_id(sHelpId);
            else
                m_xSearchString->set_help_id(m_aSearchHelpId);
            bHandled = false;
        }
        break;
    }

    return bHandled;
}

/*************************************************************************
#*  Handle:     SelComboHdl
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   A change of the category will update the list of functions.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_NOARG(ScFunctionWin, SelComboHdl, weld::ComboBox&, void)
{
    if (xCatBox->get_active() == 0)
        m_xSearchString->set_text(u""_ustr);
    xHelpButton->set_sensitive(xCatBox->get_active() != 1);
    OUString searchStr = m_xSearchString->get_text();
    UpdateFunctionList(searchStr);
    SetDescription();
}

IMPL_LINK_NOARG(ScFunctionWin, SelTreeHdl, weld::TreeView&, void)
{
    bool bSensitivity = weld::fromId<const ScFuncDesc*>(xFuncList->get_selected_id());
    xHelpButton->set_sensitive(bSensitivity);
    SetDescription();
}

/*************************************************************************
#*  Handle:     SetSelectionClickHdl
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   A change of the category will update the list of functions.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_NOARG( ScFunctionWin, SetSelectionClickHdl, weld::Button&, void )
{
    DoEnter();          // saves the input
}

/*************************************************************************
#*  Handle:     SetHelpClickHdl
#*------------------------------------------------------------------------
#*
#*  Class:      ScFunctionWin
#*
#*  Function:   Get selected function's official help.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_NOARG( ScFunctionWin, SetHelpClickHdl, weld::Button&, void )
{
    if (const auto pDesc = weld::fromId<const ScFuncDesc*>(xFuncList->get_selected_id()))
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

IMPL_LINK_NOARG(ScFunctionWin, SetSimilarityToggleHdl, weld::Toggleable&, void)
{
    OUString searchStr = m_xSearchString->get_text();
    UpdateFunctionList(searchStr);
    SetDescription();
}

IMPL_LINK_NOARG( ScFunctionWin, SetRowActivatedHdl, weld::TreeView&, bool )
{
    DoEnter(true);      // saves the input
    return true;
}

void EnglishFunctionNameChange::setProperty(const css::uno::Any &rProperty)
{
    ConfigurationListenerProperty::setProperty(rProperty);
    m_pFunctionWin->InitLRUList();
    m_pFunctionWin->UpdateFunctionList(u""_ustr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
