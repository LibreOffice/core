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

#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <svl/zforlist.hxx>
#include <svl/stritem.hxx>
#include "formula/IFunctionDescription.hxx"

#include "funcpage.hxx"
#include "formdlgs.hrc"
#include "ForResId.hrc"
#include "ModuleHelper.hxx"

namespace formula
{

FormulaListBox::FormulaListBox( vcl::Window* pParent, WinBits nBits ):
    ListBox(pParent, nBits)
{}

void FormulaListBox::KeyInput( const KeyEvent& rKEvt )
{
    KeyEvent aKEvt=rKEvt;

    if(aKEvt.GetCharCode()==' ')
        DoubleClick();
}

bool FormulaListBox::PreNotify( NotifyEvent& rNEvt )
{
    NotifyEvent aNotifyEvt=rNEvt;

    bool nResult = ListBox::PreNotify(rNEvt);

    sal_uInt16 nSwitch=aNotifyEvt.GetType();
    if(nSwitch==EVENT_KEYINPUT)
    {
        KeyInput(*aNotifyEvt.GetKeyEvent());
    }
    return nResult;
}

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeFormulaListBox(vcl::Window *pParent, VclBuilder::stringmap &)
{
    return new FormulaListBox(pParent, WB_BORDER | WB_SORT);
}

inline sal_uInt16 Lb2Cat( sal_uInt16 nLbPos )
{
    // Category 0 == LRU, otherwise Categories == LbPos-1
    if ( nLbPos > 0 )
        nLbPos -= 1;

    return nLbPos;
}

FuncPage::FuncPage(vcl::Window* pParent,const IFunctionManager* _pFunctionManager):
    TabPage(pParent, "FunctionPage", "formula/ui/functionpage.ui"),
    m_pFunctionManager(_pFunctionManager)
{
    get(m_pLbCategory, "category");
    get(m_pLbFunction, "function");
    m_pLbFunction->SetStyle(m_pLbFunction->GetStyle() | WB_SORT);
    Size aSize(LogicToPixel(Size(86 , 162), MAP_APPFONT));
    m_pLbFunction->set_height_request(aSize.Height());
    m_pLbFunction->set_width_request(aSize.Width());
    m_aHelpId = m_pLbFunction->GetHelpId();
    m_pLbFunction->SetUniqueId(m_aHelpId);

    InitLRUList();

    const sal_uInt32 nCategoryCount = m_pFunctionManager->getCount();
    for(sal_uInt32 j= 0; j < nCategoryCount; ++j)
    {
        const IFunctionCategory* pCategory = m_pFunctionManager->getCategory(j);
        m_pLbCategory->SetEntryData(m_pLbCategory->InsertEntry(pCategory->getName()),(void*)pCategory);
    }

    m_pLbCategory->SelectEntryPos(1);
    UpdateFunctionList();
    m_pLbCategory->SetSelectHdl( LINK( this, FuncPage, SelHdl ) );
    m_pLbFunction->SetSelectHdl( LINK( this, FuncPage, SelHdl ) );
    m_pLbFunction->SetDoubleClickHdl( LINK( this, FuncPage, DblClkHdl ) );
}

void FuncPage::impl_addFunctions(const IFunctionCategory* _pCategory)
{
    const sal_uInt32 nCount = _pCategory->getCount();
    for(sal_uInt32 i = 0 ; i < nCount; ++i)
    {
        TFunctionDesc pDesc(_pCategory->getFunction(i));
        m_pLbFunction->SetEntryData(
            m_pLbFunction->InsertEntry(pDesc->getFunctionName() ),(void*)pDesc );
    }
}

void FuncPage::UpdateFunctionList()
{
    sal_Int32  nSelPos   = m_pLbCategory->GetSelectEntryPos();
    const IFunctionCategory* pCategory = static_cast<const IFunctionCategory*>(m_pLbCategory->GetEntryData(nSelPos));
    sal_Int32  nCategory = ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                            ? Lb2Cat( nSelPos ) : 0;

    (void)nCategory;

    m_pLbFunction->Clear();
    m_pLbFunction->SetUpdateMode( false );


    if ( nSelPos > 0 )
    {
        if ( pCategory == NULL )
        {
            const sal_uInt32 nCount = m_pFunctionManager->getCount();
            for(sal_uInt32 i = 0 ; i < nCount; ++i)
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
        ::std::vector< TFunctionDesc >::iterator aIter = aLRUList.begin();
        ::std::vector< TFunctionDesc >::iterator aEnd = aLRUList.end();

        for ( ; aIter != aEnd; ++aIter )
        {
            const IFunctionDescription* pDesc = *aIter;
            if (pDesc)  // may be null if a function is no longer available
            {
                m_pLbFunction->SetEntryData(
                    m_pLbFunction->InsertEntry( pDesc->getFunctionName() ), (void*)pDesc );
            }
        }
    }


    m_pLbFunction->SetUpdateMode( true );
    m_pLbFunction->SelectEntryPos(0);

    if(IsVisible()) SelHdl(m_pLbFunction);
}

IMPL_LINK( FuncPage, SelHdl, ListBox*, pLb )
{
    if(pLb==m_pLbFunction)
    {
        const IFunctionDescription* pDesc = GetFuncDesc( GetFunction() );
        if ( pDesc )
        {
            const OString sHelpId = pDesc->getHelpId();
            if ( !sHelpId.isEmpty() )
                m_pLbFunction->SetHelpId(sHelpId);
        }
        aSelectionLink.Call(this);
    }
    else
    {
        m_pLbFunction->SetHelpId(m_aHelpId);
        UpdateFunctionList();
    }
    return 0;
}

IMPL_LINK_NOARG(FuncPage, DblClkHdl)
{
    aDoubleClickLink.Call(this);
    return 0;
}

void FuncPage::SetCategory(sal_Int32 nCat)
{
    m_pLbCategory->SelectEntryPos(nCat);
    UpdateFunctionList();
}

sal_Int32 FuncPage::GetFuncPos(const IFunctionDescription* _pDesc)
{
    return m_pLbFunction->GetEntryPos(_pDesc);
}

void FuncPage::SetFunction(sal_Int32 nFunc)
{
    m_pLbFunction->SelectEntryPos(nFunc);
}

void FuncPage::SetFocus()
{
    m_pLbFunction->GrabFocus();
}

sal_Int32 FuncPage::GetCategory()
{
    return m_pLbCategory->GetSelectEntryPos();
}

sal_Int32 FuncPage::GetFunction()
{
    return m_pLbFunction->GetSelectEntryPos();
}

sal_Int32 FuncPage::GetFunctionEntryCount()
{
    return m_pLbFunction->GetSelectEntryCount();
}

OUString FuncPage::GetSelFunctionName() const
{
    return m_pLbFunction->GetSelectEntry();
}

const IFunctionDescription* FuncPage::GetFuncDesc( sal_Int32 nPos ) const
{
    // not pretty, but hopefully rare
    return (const IFunctionDescription*) m_pLbFunction->GetEntryData(nPos);
}

void FuncPage::InitLRUList()
{
    m_pFunctionManager->fillLastRecentlyUsedFunctions(aLRUList);
}


} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
