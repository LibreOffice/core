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
//============================================================================
namespace formula
{

FormulaListBox::FormulaListBox( Window* pParent, const ResId& rResId ):
    ListBox(pParent,rResId)
{}

void FormulaListBox::KeyInput( const KeyEvent& rKEvt )
{
    KeyEvent aKEvt=rKEvt;

    if(aKEvt.GetCharCode()==' ')
        DoubleClick();
}

long FormulaListBox::PreNotify( NotifyEvent& rNEvt )
{
    NotifyEvent aNotifyEvt=rNEvt;

    long nResult=ListBox::PreNotify(rNEvt);

    sal_uInt16 nSwitch=aNotifyEvt.GetType();
    if(nSwitch==EVENT_KEYINPUT)
    {
        KeyInput(*aNotifyEvt.GetKeyEvent());
    }
    return nResult;
}



//============================================================================

inline sal_uInt16 Lb2Cat( sal_uInt16 nLbPos )
{
    // Category 0 == LRU, otherwise Categories == LbPos-1
    if ( nLbPos > 0 )
        nLbPos -= 1;

    return nLbPos;
}

//============================================================================

FuncPage::FuncPage(Window* pParent,const IFunctionManager* _pFunctionManager):
    TabPage(pParent,ModuleRes(RID_FORMULATAB_FUNCTION)),
    aFtCategory     ( this, ModuleRes( FT_CATEGORY ) ),
    aLbCategory     ( this, ModuleRes( LB_CATEGORY ) ),
    aFtFunction     ( this, ModuleRes( FT_FUNCTION ) ),
    aLbFunction     ( this, ModuleRes( LB_FUNCTION ) ),
    m_pFunctionManager(_pFunctionManager)
{
    FreeResource();
    m_aHelpId = aLbFunction.GetHelpId();
    aLbFunction.SetUniqueId(m_aHelpId);

    InitLRUList();

    const sal_uInt32 nCategoryCount = m_pFunctionManager->getCount();
    for(sal_uInt32 j= 0; j < nCategoryCount; ++j)
    {
        const IFunctionCategory* pCategory = m_pFunctionManager->getCategory(j);
        aLbCategory.SetEntryData(aLbCategory.InsertEntry(pCategory->getName()),(void*)pCategory);
    }

    aLbCategory.SelectEntryPos(1);
    UpdateFunctionList();
    aLbCategory.SetSelectHdl( LINK( this, FuncPage, SelHdl ) );
    aLbFunction.SetSelectHdl( LINK( this, FuncPage, SelHdl ) );
    aLbFunction.SetDoubleClickHdl( LINK( this, FuncPage, DblClkHdl ) );
}
// -----------------------------------------------------------------------------
void FuncPage::impl_addFunctions(const IFunctionCategory* _pCategory)
{
    const sal_uInt32 nCount = _pCategory->getCount();
    for(sal_uInt32 i = 0 ; i < nCount; ++i)
    {
        TFunctionDesc pDesc(_pCategory->getFunction(i));
        aLbFunction.SetEntryData(
            aLbFunction.InsertEntry(pDesc->getFunctionName() ),(void*)pDesc );
    } // for(sal_uInt32 i = 0 ; i < nCount; ++i)
}

void FuncPage::UpdateFunctionList()
{
    sal_uInt16  nSelPos   = aLbCategory.GetSelectEntryPos();
    const IFunctionCategory* pCategory = static_cast<const IFunctionCategory*>(aLbCategory.GetEntryData(nSelPos));
    sal_uInt16  nCategory = ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                            ? Lb2Cat( nSelPos ) : 0;

    (void)nCategory;

    aLbFunction.Clear();
    aLbFunction.SetUpdateMode( sal_False );
    //------------------------------------------------------

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
                aLbFunction.SetEntryData(
                    aLbFunction.InsertEntry( pDesc->getFunctionName() ), (void*)pDesc );
            }
        }
    }

    //------------------------------------------------------
    aLbFunction.SetUpdateMode( sal_True );
    aLbFunction.SelectEntryPos(0);

    if(IsVisible()) SelHdl(&aLbFunction);
}

IMPL_LINK( FuncPage, SelHdl, ListBox*, pLb )
{
    if(pLb==&aLbFunction)
    {
        const IFunctionDescription* pDesc = GetFuncDesc( GetFunction() );
        if ( pDesc )
        {
            const OString sHelpId = pDesc->getHelpId();
            if ( !sHelpId.isEmpty() )
                aLbFunction.SetHelpId(sHelpId);
        }
        aSelectionLink.Call(this);
    }
    else
    {
        aLbFunction.SetHelpId(m_aHelpId);
        UpdateFunctionList();
    }
    return 0;
}

IMPL_LINK_NOARG(FuncPage, DblClkHdl)
{
    aDoubleClickLink.Call(this);
    return 0;
}

void FuncPage::SetCategory(sal_uInt16 nCat)
{
    aLbCategory.SelectEntryPos(nCat);
    UpdateFunctionList();
}
sal_uInt16 FuncPage::GetFuncPos(const IFunctionDescription* _pDesc)
{
    return aLbFunction.GetEntryPos(_pDesc);
}
void FuncPage::SetFunction(sal_uInt16 nFunc)
{
    aLbFunction.SelectEntryPos(nFunc);
}

void FuncPage::SetFocus()
{
    aLbFunction.GrabFocus();
}

sal_uInt16 FuncPage::GetCategory()
{
    return aLbCategory.GetSelectEntryPos();
}

sal_uInt16 FuncPage::GetFunction()
{
    return aLbFunction.GetSelectEntryPos();
}

sal_uInt16 FuncPage::GetFunctionEntryCount()
{
    return aLbFunction.GetSelectEntryCount();
}

OUString FuncPage::GetSelFunctionName() const
{
    return aLbFunction.GetSelectEntry();
}
const IFunctionDescription* FuncPage::GetFuncDesc( sal_uInt16 nPos ) const
{
    // not pretty, but hopefully rare
    return (const IFunctionDescription*) aLbFunction.GetEntryData(nPos);
}

void FuncPage::InitLRUList()
{
    m_pFunctionManager->fillLastRecentlyUsedFunctions(aLRUList);
}


} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
