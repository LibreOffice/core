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

#include <comphelper/processfactory.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

SvSimpleTableContainer::SvSimpleTableContainer(vcl::Window* pParent, WinBits nBits)
    : Control(pParent, nBits)
    , m_pTable(NULL)
{
}

SvSimpleTableContainer::~SvSimpleTableContainer()
{
    disposeOnce();
}

void SvSimpleTableContainer::dispose()
{
    m_pTable.clear();
    Control::dispose();
}

VCL_BUILDER_FACTORY_ARGS(SvSimpleTableContainer,
                         WB_TABSTOP | WB_DIALOGCONTROL | WB_BORDER)

void SvSimpleTableContainer::SetTable(SvSimpleTable* pTable)
{
    m_pTable = pTable;
}

bool SvSimpleTableContainer::PreNotify( NotifyEvent& rNEvt )
{
    bool bResult = true;
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const vcl::KeyCode& aKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey = aKeyCode.GetCode();
        if (nKey == KEY_TAB)
            GetParent()->Notify( rNEvt );
        else if (m_pTable && m_pTable->IsFocusOnCellEnabled() && ( nKey == KEY_LEFT || nKey == KEY_RIGHT))
            return false;
        else
            bResult = Control::PreNotify( rNEvt );
    }
    else
        bResult = Control::PreNotify( rNEvt );

    return bResult;
}

void SvSimpleTableContainer::Resize()
{
    Control::Resize();
    if (m_pTable)
        m_pTable->UpdateViewSize();
}

void SvSimpleTableContainer::GetFocus()
{
    Control::GetFocus();
    if (m_pTable)
        m_pTable->GrabFocus();
}

// SvSimpleTable ------------------------------------------------------------

SvSimpleTable::SvSimpleTable(SvSimpleTableContainer& rParent, WinBits nBits):
        SvHeaderTabListBox(&rParent, nBits | WB_CLIPCHILDREN | WB_HSCROLL | WB_TABSTOP),
        m_rParentTableContainer(rParent),
        aHeaderBar(VclPtr<HeaderBar>::Create(&rParent,WB_BUTTONSTYLE | WB_BORDER | WB_TABSTOP)),
        nHeaderItemId(1),
        bPaintFlag(true),
        aCollator(*(IntlWrapper( Application::GetSettings().GetLanguageTag() ).getCaseCollator()))
{
    m_rParentTableContainer.SetTable(this);

    bSortDirection = true;
    nSortCol = 0xFFFF;
    nOldPos = 0;

    aHeaderBar->SetStartDragHdl(LINK( this, SvSimpleTable, StartDragHdl));
    aHeaderBar->SetDragHdl(LINK( this, SvSimpleTable, DragHdl));
    aHeaderBar->SetEndDragHdl(LINK( this, SvSimpleTable, EndDragHdl));
    aHeaderBar->SetSelectHdl(LINK( this, SvSimpleTable, HeaderBarClick));


    EnableCellFocus();
    DisableTransientChildren();
    InitHeaderBar( aHeaderBar );

    UpdateViewSize();

    aHeaderBar->Show();
    SvHeaderTabListBox::Show();
}

SvSimpleTable::~SvSimpleTable()
{
    disposeOnce();
}

void SvSimpleTable::dispose()
{
    m_rParentTableContainer.SetTable(NULL);
    aHeaderBar.disposeAndClear();
    SvHeaderTabListBox::dispose();
}

void SvSimpleTable::UpdateViewSize()
{
    Size theWinSize=m_rParentTableContainer.GetOutputSizePixel();
    Size HbSize=aHeaderBar->GetSizePixel();

    HbSize.Width()=theWinSize.Width();
    theWinSize.Height()-=HbSize.Height();
    Point thePos(0,0);

    aHeaderBar->SetPosPixel(thePos);
    aHeaderBar->SetSizePixel(HbSize);

    thePos.Y()+=HbSize.Height();
    SvHeaderTabListBox::SetPosPixel(thePos);
    SvHeaderTabListBox::SetSizePixel(theWinSize);
    Invalidate();
}

void SvSimpleTable::NotifyScrolled()
{
    long nOffset=-GetXOffset();
    if(nOldPos!=nOffset)
    {
        aHeaderBar->SetOffset(nOffset);
        aHeaderBar->Invalidate();
        aHeaderBar->Update();
        nOldPos=nOffset;
    }
    SvHeaderTabListBox::NotifyScrolled();
}

void SvSimpleTable::SetTabs()
{
    SvHeaderTabListBox::SetTabs();

    sal_uInt16 nPrivTabCount = TabCount();
    if ( nPrivTabCount )
    {
        if ( nPrivTabCount > aHeaderBar->GetItemCount() )
            nPrivTabCount = aHeaderBar->GetItemCount();

        sal_uInt16 i, nPos = 0;
        for ( i = 1; i < nPrivTabCount; ++i )
        {
            sal_uInt16 nNewSize = static_cast< sal_uInt16 >( GetTab(i) ) - nPos;
            aHeaderBar->SetItemSize( i, nNewSize );
            nPos = (sal_uInt16)GetTab(i);
        }

        aHeaderBar->SetItemSize( i, HEADERBAR_FULLSIZE ); // because no tab for last entry
    }
}

void SvSimpleTable::SetTabs(const long* pTabs, MapUnit eMapUnit)
{
    SvHeaderTabListBox::SetTabs(pTabs,eMapUnit);
}

void SvSimpleTable::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    SvHeaderTabListBox::Paint(rRenderContext, rRect);

    sal_uInt16 nPrivTabCount = TabCount();

    long nOffset =- GetXOffset();
    nOldPos = nOffset;

    aHeaderBar->SetOffset(nOffset);
    aHeaderBar->Invalidate();

    if (nPrivTabCount && bPaintFlag)
    {
        if (nPrivTabCount>aHeaderBar->GetItemCount())
            nPrivTabCount=aHeaderBar->GetItemCount();

        sal_uInt16 nPos = 0;
        for (sal_uInt16 i = 1; i < nPrivTabCount; i++)
        {
            sal_uInt16 nNewSize = static_cast<sal_uInt16>(GetTab(i)) - nPos;
            aHeaderBar->SetItemSize(i, nNewSize);
            nPos = static_cast<sal_uInt16>(GetTab(i));
        }
    }
    bPaintFlag = true;
}

void SvSimpleTable::InsertHeaderEntry(const OUString& rText,
    sal_uInt16 nCol, HeaderBarItemBits nBits)
{
    sal_Int32 nEnd = rText.indexOf( '\t' );
    if( nEnd == -1 )
    {
        aHeaderBar->InsertItem(nHeaderItemId++, rText, 0, nBits, nCol);
    }
    else
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aString = rText.getToken(0, '\t', nIndex);
            aHeaderBar->InsertItem(nHeaderItemId++, aString, 0, nBits, nCol);
        }
        while ( nIndex >= 0 );
    }
    SetTabs();
}

void SvSimpleTable::ClearHeader()
{
    aHeaderBar->Clear();
}

void SvSimpleTable::ShowTable()
{
    m_rParentTableContainer.Show();
}

void SvSimpleTable::HideTable()
{
    m_rParentTableContainer.Hide();
}

bool SvSimpleTable::IsVisible() const
{
    return m_rParentTableContainer.IsVisible();
}

void SvSimpleTable::EnableTable()
{
    m_rParentTableContainer.Enable();
}

void SvSimpleTable::DisableTable()
{
    m_rParentTableContainer.Disable();
}

bool SvSimpleTable::IsEnabled() const
{
    return m_rParentTableContainer.IsEnabled();
}

sal_uInt16 SvSimpleTable::GetSelectedCol()
{
    return (aHeaderBar->GetCurItemId()-1);
}

void SvSimpleTable::SortByCol(sal_uInt16 nCol, bool bDir)
{
    if(nSortCol!=0xFFFF)
        aHeaderBar->SetItemBits(nSortCol+1,HeaderBarItemBits::STDSTYLE);

    if (nCol != 0xFFFF)
    {
        if(bDir || nSortCol != nCol)
        {
            aHeaderBar->SetItemBits( nCol+1, HeaderBarItemBits::STDSTYLE | HeaderBarItemBits::DOWNARROW);
            GetModel()->SetSortMode(SortAscending);
            bDir = true;
        }
        else
        {
            aHeaderBar->SetItemBits( nCol+1, HeaderBarItemBits::STDSTYLE | HeaderBarItemBits::UPARROW);
            GetModel()->SetSortMode(SortDescending);
        }

        GetModel()->SetCompareHdl( LINK( this, SvSimpleTable, CompareHdl));

        if(nSortCol == nCol)
        {
            GetModel()->Reverse();
            Resize();   //update rows
        }
        else
        {
            nSortCol=nCol;
            GetModel()->Resort();
        }
    }
    else
        GetModel()->SetSortMode(SortNone);
    nSortCol=nCol;
    bSortDirection=bDir;
    SetAlternatingRowColors( true );
}

void SvSimpleTable::HBarClick()
{
    sal_uInt16 nId=aHeaderBar->GetCurItemId();

    if (aHeaderBar->GetItemBits(nId) & HeaderBarItemBits::CLICKABLE)
    {
        if(nId==nSortCol+1)
        {
            SortByCol(nId-1,!bSortDirection);
        }
        else
        {
            SortByCol(nId-1,bSortDirection);
        }

        aHeaderBarClickLink.Call(this);
    }
}

void SvSimpleTable::HBarStartDrag()
{
    if(!aHeaderBar->IsItemMode())
    {
        Rectangle aSizeRect(Point(0,0),
            SvHeaderTabListBox::GetOutputSizePixel());
        aSizeRect.Left()=-GetXOffset()+aHeaderBar->GetDragPos();
        aSizeRect.Right()=-GetXOffset()+aHeaderBar->GetDragPos();
        ShowTracking( aSizeRect, SHOWTRACK_SPLIT );
    }
}
void SvSimpleTable::HBarDrag()
{
    HideTracking();
    if(!aHeaderBar->IsItemMode())
    {
        Rectangle aSizeRect(Point(0,0),
            SvHeaderTabListBox::GetOutputSizePixel());
        aSizeRect.Left()=-GetXOffset()+aHeaderBar->GetDragPos();
        aSizeRect.Right()=-GetXOffset()+aHeaderBar->GetDragPos();
        ShowTracking( aSizeRect, SHOWTRACK_SPLIT );
    }
}
void SvSimpleTable::HBarEndDrag()
{
    HideTracking();
    sal_uInt16 nPrivTabCount=TabCount();

    if(nPrivTabCount)
    {
        if(nPrivTabCount>aHeaderBar->GetItemCount())
                nPrivTabCount=aHeaderBar->GetItemCount();

        sal_uInt16 nPos=0;
        sal_uInt16 nNewSize=0;
        for(sal_uInt16 i=1;i<nPrivTabCount;i++)
        {
            nNewSize = static_cast< sal_uInt16 >( aHeaderBar->GetItemSize(i) ) + nPos;
            SetTab( i, nNewSize, MAP_PIXEL );
            nPos = nNewSize;
        }
    }
    bPaintFlag = false;
    Invalidate();
    Update();
}


void SvSimpleTable::Command( const CommandEvent& rCEvt )
{
    aCEvt=rCEvt;
    aCommandLink.Call(this);
    SvHeaderTabListBox::Command(rCEvt);
}

IMPL_LINK_TYPED( SvSimpleTable, StartDragHdl, HeaderBar*, pCtr, void)
{
    if(pCtr==aHeaderBar.get())
    {
        HBarStartDrag();
    }
}

IMPL_LINK_TYPED( SvSimpleTable, DragHdl, HeaderBar*, pCtr, void)
{
    if(pCtr==aHeaderBar.get())
    {
        HBarDrag();
    }
}

IMPL_LINK_TYPED( SvSimpleTable, EndDragHdl, HeaderBar*, pCtr, void)
{
    if(pCtr==aHeaderBar.get())
    {
        HBarEndDrag();
    }
}

IMPL_LINK_TYPED( SvSimpleTable, HeaderBarClick, HeaderBar*, pCtr, void)
{
    if(pCtr==aHeaderBar.get())
    {
        HBarClick();
    }
}

SvLBoxItem* SvSimpleTable::GetEntryAtPos( SvTreeListEntry* pEntry, sal_uInt16 nPos ) const
{
    DBG_ASSERT(pEntry,"GetEntryText:Invalid Entry");
    SvLBoxItem* pItem = NULL;

    if( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();

        nPos++;

        if( nTreeFlags & SvTreeFlags::CHKBTN ) nPos++;

        if( nPos < nCount )
        {
            pItem = &pEntry->GetItem( nPos);
        }
    }
    return pItem;
}

sal_Int32 SvSimpleTable::ColCompare(SvTreeListEntry* pLeft,SvTreeListEntry* pRight)
{
    sal_Int32 nCompare = 0;

    SvLBoxItem* pLeftItem = GetEntryAtPos( pLeft, nSortCol);
    SvLBoxItem* pRightItem = GetEntryAtPos( pRight, nSortCol);


    if(pLeftItem != NULL && pRightItem != NULL)
    {
        sal_uInt16 nLeftKind = pLeftItem->GetType();
        sal_uInt16 nRightKind = pRightItem->GetType();

        if(nRightKind == SV_ITEM_ID_LBOXSTRING &&
            nLeftKind == SV_ITEM_ID_LBOXSTRING )
            nCompare = aCollator.compareString( static_cast<SvLBoxString*>(pLeftItem)->GetText(),
                                    static_cast<SvLBoxString*>(pRightItem)->GetText());
    }
    return nCompare;
}

IMPL_LINK_TYPED( SvSimpleTable, CompareHdl, const SvSortData&, rData, sal_Int32)
{
    SvTreeListEntry* pLeft = const_cast<SvTreeListEntry*>(rData.pLeft);
    SvTreeListEntry* pRight = const_cast<SvTreeListEntry*>(rData.pRight);
    return ColCompare(pLeft,pRight);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
