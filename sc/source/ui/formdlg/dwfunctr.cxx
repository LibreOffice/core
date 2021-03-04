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

#include <global.hxx>
#include <scmod.hxx>
#include <inputhdl.hxx>
#include <tabvwsh.hxx>
#include <funcdesc.hxx>

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
    : PanelLayout(pParent, "FunctionPanel", "modules/scalc/ui/functionpanel.ui")
    , xCatBox(m_xBuilder->weld_combo_box("category"))
    , xFuncList(m_xBuilder->weld_tree_view("funclist"))
    , xInsertButton(m_xBuilder->weld_button("insert"))
    , xFiFuncDesc(m_xBuilder->weld_label("funcdesc"))
    , xConfigListener(new comphelper::ConfigurationListener("/org.openoffice.Office.Calc/Formula/Syntax"))
    , xConfigChange(std::make_unique<EnglishFunctionNameChange>(xConfigListener, this))
    , pFuncDesc(nullptr)
{
    xFuncList->set_size_request(-1, xFuncList->get_height_rows(10));

    InitLRUList();

    nArgs=0;
    xFiFuncDesc->set_size_request(-1, 5 * xFiFuncDesc->get_text_height());

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
#*  Member:     UpdateFunctionList
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
        UpdateFunctionList();
}

/*************************************************************************
#*  Member:     UpdateFunctionList
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
    xFiFuncDesc->set_label(EMPTY_OUSTRING);
    const ScFuncDesc* pDesc =
             reinterpret_cast<const ScFuncDesc*>(xFuncList->get_selected_id().toInt64());
    if (pDesc)
    {
        pDesc->initArgumentInfo();      // full argument info is needed

        OUStringBuffer aBuf(xFuncList->get_selected_text());
        aBuf.append(":\n\n");
        aBuf.append(pDesc->GetParamList());
        aBuf.append("\n\n");
        aBuf.append(*pDesc->mxFuncDesc);

        xFiFuncDesc->set_label(aBuf.makeStringAndClear());
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
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::UpdateFunctionList()
{
    sal_Int32  nSelPos   = xCatBox->get_active();
    sal_Int32  nCategory = ( -1 != nSelPos )
                            ? (nSelPos-1) : 0;

    xFuncList->clear();
    xFuncList->freeze();

    if ( nSelPos > 0 )
    {
        ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();

        const ScFuncDesc* pDesc = pFuncMgr->First( nCategory );
        while ( pDesc )
        {
            xFuncList->append(OUString::number(reinterpret_cast<sal_Int64>(pDesc)), *(pDesc->mxFuncName));
            pDesc = pFuncMgr->Next();
        }
    }
    else // LRU list
    {
        for (const formula::IFunctionDescription* pDesc : aLRUList)
        {
            if (pDesc)
            {
                xFuncList->append(OUString::number(reinterpret_cast<sal_Int64>(pDesc)), pDesc->getFunctionName());
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
             reinterpret_cast<const ScFuncDesc*>(xFuncList->get_selected_id().toInt64());
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
#*  Handle:     SelHdl
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
    UpdateFunctionList();
    SetDescription();
}

IMPL_LINK_NOARG(ScFunctionWin, SelTreeHdl, weld::TreeView&, void)
{
    SetDescription();
}

/*************************************************************************
#*  Handle:     SelHdl
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
    m_pFunctionWin->UpdateFunctionList();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
