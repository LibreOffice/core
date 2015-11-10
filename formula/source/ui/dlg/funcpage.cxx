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
#include <vcl/builderfactory.hxx>
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

    bool bResult = ListBox::PreNotify(rNEvt);

    MouseNotifyEvent nSwitch=aNotifyEvt.GetType();
    if(nSwitch==MouseNotifyEvent::KEYINPUT)
    {
        KeyInput(*aNotifyEvt.GetKeyEvent());
    }
    return bResult;
}

VCL_BUILDER_FACTORY_ARGS(FormulaListBox, WB_BORDER | WB_SORT)

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
        m_pLbCategory->SetEntryData(m_pLbCategory->InsertEntry(pCategory->getName()),const_cast<IFunctionCategory *>(pCategory));
    }

    m_pLbCategory->SetDropDownLineCount(m_pLbCategory->GetEntryCount());
    m_pLbCategory->SelectEntryPos(1);
    UpdateFunctionList();
    m_pLbCategory->SetSelectHdl( LINK( this, FuncPage, SelHdl ) );
    m_pLbFunction->SetSelectHdl( LINK( this, FuncPage, SelHdl ) );
    m_pLbFunction->SetDoubleClickHdl( LINK( this, FuncPage, DblClkHdl ) );
}

FuncPage::~FuncPage()
{
    disposeOnce();
}

void FuncPage::dispose()
{
    m_pLbCategory.clear();
    m_pLbFunction.clear();
    TabPage::dispose();
}

void FuncPage::impl_addFunctions(const IFunctionCategory* _pCategory)
{
    const sal_uInt32 nCount = _pCategory->getCount();
    for(sal_uInt32 i = 0 ; i < nCount; ++i)
    {
        TFunctionDesc pDesc(_pCategory->getFunction(i));
        m_pLbFunction->SetEntryData(
            m_pLbFunction->InsertEntry(pDesc->getFunctionName() ),const_cast<IFunctionDescription *>(pDesc) );
    }
}

void FuncPage::UpdateFunctionList()
{
    sal_Int32  nSelPos   = m_pLbCategory->GetSelectEntryPos();
    const IFunctionCategory* pCategory = static_cast<const IFunctionCategory*>(m_pLbCategory->GetEntryData(nSelPos));

    m_pLbFunction->Clear();
    m_pLbFunction->SetUpdateMode( false );


    if ( nSelPos > 0 )
    {
        if ( pCategory == nullptr )
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
                    m_pLbFunction->InsertEntry( pDesc->getFunctionName() ), const_cast<IFunctionDescription *>(pDesc) );
            }
        }
    }


    m_pLbFunction->SetUpdateMode( true );
    m_pLbFunction->SelectEntryPos(0);

    if(IsVisible()) SelHdl(*m_pLbFunction);
}

IMPL_LINK_TYPED( FuncPage, SelHdl, ListBox&, rLb, void )
{
    if(&rLb==m_pLbFunction)
    {
        const IFunctionDescription* pDesc = GetFuncDesc( GetFunction() );
        if ( pDesc )
        {
            const OString sHelpId = pDesc->getHelpId();
            if ( !sHelpId.isEmpty() )
                m_pLbFunction->SetHelpId(sHelpId);
        }
        aSelectionLink.Call(*this);
    }
    else
    {
        m_pLbFunction->SetHelpId(m_aHelpId);
        UpdateFunctionList();
    }
}

IMPL_LINK_NOARG_TYPED(FuncPage, DblClkHdl, ListBox&, void)
{
    aDoubleClickLink.Call(*this);
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
    return static_cast<const IFunctionDescription*>(m_pLbFunction->GetEntryData(nPos));
}

void FuncPage::InitLRUList()
{
    m_pFunctionManager->fillLastRecentlyUsedFunctions(aLRUList);
}


} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
