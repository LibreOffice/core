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

#include <comphelper/string.hxx>
#include <editeng/editview.hxx>
#include <sfx2/viewsh.hxx>
#include <formula/funcvarargs.h>
#include <unotools/charclass.hxx>

#include <global.hxx>
#include <scmod.hxx>
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

ScFunctionWin::ScFunctionWin(weld::Widget* pParent)
    : PanelLayout(pParent, u"FunctionPanel"_ustr, u"modules/scalc/ui/functionpanel.ui"_ustr)
    , xCatBox(m_xBuilder->weld_combo_box(u"category"_ustr))
    , xFuncList(m_xBuilder->weld_tree_view(u"funclist"_ustr))
    , xInsertButton(m_xBuilder->weld_button(u"insert"_ustr))
    , xFiFuncDesc(m_xBuilder->weld_text_view(u"funcdesc"_ustr))
    , m_xSearchString(m_xBuilder->weld_entry(u"search"_ustr))
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
        ScModule* pScMod = SC_MOD();
        pScMod->InsertEntryToLRUList(pFuncDesc->nFIndex);
    }
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

    if ( nSelPos > 0 )
    {
        ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();

        // Use the corresponding CharClass for uppercase() depending on whether
        // English function names are used, or localized names.
        const CharClass* pCharClass = (ScGlobal::GetStarCalcFunctionList()->IsEnglishFunctionNames()
                ? ScCompiler::GetCharClassEnglish()
                : ScCompiler::GetCharClassLocalized());

        const OUString aSearchStr(pCharClass->uppercase(rSearchString));

        // First add the functions that start with the search string
        const ScFuncDesc* pDesc = pFuncMgr->First( nCategory );
        while ( pDesc )
        {
            if (rSearchString.isEmpty()
                || (pCharClass->uppercase(pDesc->getFunctionName()).startsWith(aSearchStr)))
                xFuncList->append(weld::toId(pDesc), *(pDesc->mxFuncName));
            pDesc = pFuncMgr->Next();
        }

        // Now add the functions that have the search string in the middle of the function name
        // Note that this will only be necessary if the search string is not empty
        if (!rSearchString.isEmpty())
        {
            pDesc = pFuncMgr->First( nCategory );
            while ( pDesc )
            {
                if (pCharClass->uppercase(pDesc->getFunctionName()).indexOf(aSearchStr) > 0)
                    xFuncList->append(weld::toId(pDesc), *(pDesc->mxFuncName));
                pDesc = pFuncMgr->Next();
            }
        }
    }
    else // LRU list
    {
        for (const formula::IFunctionDescription* pDesc : aLRUList)
        {
            if (pDesc)
            {
                xFuncList->append(weld::toId(pDesc), pDesc->getFunctionName());
            }
        }
    }

    xFuncList->thaw();

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
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::DoEnter()
{
    OUStringBuffer aArgStr;
    OUString aString=xFuncList->get_selected_text();
    SfxViewShell* pCurSh = SfxViewShell::Current();
    nArgs=0;

    if(!aString.isEmpty())
    {
        OUString aFirstArgStr;
        ScModule* pScMod = SC_MOD();
        ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( pCurSh );
        ScInputHandler* pHdl = pScMod->GetInputHdl( pViewSh );
        if(!pScMod->IsEditMode())
        {
            rtl::Reference<comphelper::ConfigurationListener> xDetectDisposed(xConfigListener);
            pScMod->SetInputMode(SC_INPUT_TABLE);
            // the above call can result in us being disposed
            if (xDetectDisposed->isDisposed())
                return;
            aString = "=" + xFuncList->get_selected_text();
            if (pHdl)
                pHdl->ClearText();
        }
        const ScFuncDesc* pDesc =
             weld::fromId<const ScFuncDesc*>(xFuncList->get_selected_id());
        if (pDesc)
        {
            pFuncDesc=pDesc;
            UpdateLRUList();
            nArgs = pDesc->nArgCount;
            if(nArgs>0)
            {
                // NOTE: Theoretically the first parameter could have the
                // suppress flag as well, but practically it doesn't.
                aFirstArgStr = pDesc->maDefArgNames[0];
                aFirstArgStr = comphelper::string::strip(aFirstArgStr, ' ');
                aFirstArgStr = aFirstArgStr.replaceAll(" ", "_");
                aArgStr = aFirstArgStr;
                if ( nArgs != VAR_ARGS && nArgs != PAIRED_VAR_ARGS )
                {   // no VarArgs or Fix plus VarArgs, but not VarArgs only
                    sal_uInt16 nFix;
                    if (nArgs >= PAIRED_VAR_ARGS)
                        nFix = nArgs - PAIRED_VAR_ARGS + 2;
                    else if (nArgs >= VAR_ARGS)
                        nFix = nArgs - VAR_ARGS + 1;
                    else
                        nFix = nArgs;
                    for ( sal_uInt16 nArg = 1;
                            nArg < nFix && !pDesc->pDefArgFlags[nArg].bOptional; nArg++ )
                    {
                        aArgStr.append("; ");
                        OUString sTmp = pDesc->maDefArgNames[nArg];
                        sTmp = comphelper::string::strip(sTmp, ' ');
                        sTmp = sTmp.replaceAll(" ", "_");
                        aArgStr.append(sTmp);
                    }
                }
            }
        }
        if (pHdl)
        {
            if (pHdl->GetEditString().isEmpty())
            {
                aString = "=" + xFuncList->get_selected_text();
            }
            EditView *pEdView=pHdl->GetActiveView();
            if(pEdView!=nullptr) // @ needed because of crash during setting a name
            {
                if(nArgs>0)
                {
                    pHdl->InsertFunction(aString);
                    pEdView->InsertText(aArgStr.makeStringAndClear(),true);
                    ESelection  aESel=pEdView->GetSelection();
                    aESel.nEndPos = aESel.nStartPos + aFirstArgStr.getLength();
                    pEdView->SetSelection(aESel);
                    pHdl->DataChanged();
                }
                else
                {
                    aString += "()";
                    pEdView->InsertText(aString);
                    pHdl->DataChanged();
                }
            }
        }
        InitLRUList();
    }
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
    // Switch to the "All" category when searching a function
    xCatBox->set_active(1);
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
#*  Function:   Processes key inputs when the serch entry has focus
#*
#************************************************************************/

IMPL_LINK(ScFunctionWin, KeyInputHdl, const KeyEvent&, rEvent, bool)
{
    bool bHandled = false;

    switch (rEvent.GetKeyCode().GetCode())
    {
    case KEY_RETURN:
        {
            DoEnter();
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
    UpdateFunctionList(u""_ustr);
    SetDescription();
    m_xSearchString->set_text(u""_ustr);
    m_xSearchString->grab_focus();
}

IMPL_LINK_NOARG(ScFunctionWin, SelTreeHdl, weld::TreeView&, void)
{
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

IMPL_LINK_NOARG( ScFunctionWin, SetRowActivatedHdl, weld::TreeView&, bool )
{
    DoEnter();          // saves the input
    return true;
}

void EnglishFunctionNameChange::setProperty(const css::uno::Any &rProperty)
{
    ConfigurationListenerProperty::setProperty(rProperty);
    m_pFunctionWin->InitLRUList();
    m_pFunctionWin->UpdateFunctionList(u""_ustr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
