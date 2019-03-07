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
#include <vcl/fixed.hxx>

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

ScFunctionWin::ScFunctionWin(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame> &rFrame)
    : PanelLayout(pParent, "FunctionPanel", "modules/scalc/ui/functionpanel.ui", rFrame)
    , pFuncDesc(nullptr)
{
    get(aCatBox, "category");
    get(aFuncList, "funclist");
    aFuncList->set_height_request(10 * aFuncList->GetTextHeight());
    get(aInsertButton, "insert");
    get(aFiFuncDesc, "funcdesc");

    InitLRUList();

    aFiFuncDesc->SetUpdateMode(true);
    nArgs=0;
    aCatBox->SetDropDownLineCount(9);
    vcl::Font aFont=aFiFuncDesc->GetFont();
    aFont.SetColor(COL_BLACK);
    aFiFuncDesc->SetFont(aFont);
    aFiFuncDesc->SetBackground( GetBackground() );       //! never transparent?
    aFiFuncDesc->set_height_request(5 * aFiFuncDesc->GetTextHeight());

    Link<ListBox&,void> aLink=LINK( this, ScFunctionWin, SelHdl);
    aCatBox->SetSelectHdl(aLink);
    aFuncList->SetSelectHdl(aLink);

    aFuncList->SetDoubleClickHdl(LINK( this, ScFunctionWin, SetSelectionHdl));
    aInsertButton->SetClickHdl(LINK( this, ScFunctionWin, SetSelectionClickHdl));

    aCatBox->SelectEntryPos(0);

    SelHdl(*aCatBox);
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
    disposeOnce();
}

void ScFunctionWin::dispose()
{
    aCatBox.clear();
    aFuncList.clear();
    aInsertButton.clear();
    aFiFuncDesc.clear();
    PanelLayout::dispose();
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

    sal_Int32  nSelPos   = aCatBox->GetSelectedEntryPos();

    if(nSelPos == 0)
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
    aFiFuncDesc->SetText( EMPTY_OUSTRING );
    const ScFuncDesc* pDesc =
             static_cast<const ScFuncDesc*>(aFuncList->GetEntryData(
                    aFuncList->GetSelectedEntryPos() ));
    if (pDesc)
    {
        pDesc->initArgumentInfo();      // full argument info is needed

        OUStringBuffer aBuf(aFuncList->GetSelectedEntry());
        aBuf.append(":\n\n");
        aBuf.append(pDesc->GetParamList());
        aBuf.append("\n\n");
        aBuf.append(*pDesc->mxFuncDesc);

        aFiFuncDesc->SetText(aBuf.makeStringAndClear());
        aFiFuncDesc->StateChanged(StateChangedType::Text);
        aFiFuncDesc->Invalidate();
        aFiFuncDesc->Update();

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
    sal_Int32  nSelPos   = aCatBox->GetSelectedEntryPos();
    sal_Int32  nCategory = ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                            ? (nSelPos-1) : 0;

    aFuncList->Clear();
    aFuncList->SetUpdateMode( false );

    if ( nSelPos > 0 )
    {
        ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();

        const ScFuncDesc* pDesc = pFuncMgr->First( nCategory );
        while ( pDesc )
        {
            aFuncList->SetEntryData(
                aFuncList->InsertEntry( *(pDesc->mxFuncName) ),
                const_cast<ScFuncDesc *>(pDesc) );
            pDesc = pFuncMgr->Next();
        }
    }
    else // LRU list
    {
        for (const formula::IFunctionDescription* pDesc : aLRUList)
        {
            if (pDesc)
                aFuncList->SetEntryData( aFuncList->InsertEntry( pDesc->getFunctionName()), const_cast<formula::IFunctionDescription *>(pDesc));
        }
    }

    aFuncList->SetUpdateMode( true );

    if ( aFuncList->GetEntryCount() > 0 )
    {
        aFuncList->Enable();
        aFuncList->SelectEntryPos( 0 );
    }
    else
    {
        aFuncList->Disable();
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
    OUString aFirstArgStr;
    OUStringBuffer aArgStr;
    OUString aString=aFuncList->GetSelectedEntry();
    SfxViewShell* pCurSh = SfxViewShell::Current();
    nArgs=0;

    if(!aString.isEmpty())
    {

        ScModule* pScMod = SC_MOD();
        ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( pCurSh );
        ScInputHandler* pHdl = pScMod->GetInputHdl( pViewSh );
        if(!pScMod->IsEditMode())
        {
            pScMod->SetInputMode(SC_INPUT_TABLE);
            // the above call can result in us being disposed
            if (OutputDevice::isDisposed())
                return;
            aString = "=";
            aString += aFuncList->GetSelectedEntry();
            if (pHdl)
                pHdl->ClearText();
        }
        const ScFuncDesc* pDesc =
             static_cast<const ScFuncDesc*>(aFuncList->GetEntryData(
                    aFuncList->GetSelectedEntryPos() ));
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
                aString = "=";
                aString += aFuncList->GetSelectedEntry();
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

IMPL_LINK( ScFunctionWin, SelHdl, ListBox&, rLb, void )
{
    if (&rLb == aCatBox.get())
    {
        UpdateFunctionList();
        SetDescription();
    }

    if (&rLb == aFuncList.get())
    {
        SetDescription();
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

IMPL_LINK_NOARG( ScFunctionWin, SetSelectionClickHdl, Button*, void )
{
    DoEnter();          // saves the input
}
IMPL_LINK_NOARG( ScFunctionWin, SetSelectionHdl, ListBox&, void )
{
    DoEnter();          // saves the input
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
