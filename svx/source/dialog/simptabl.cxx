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

#include <svx/simptabl.hxx>
#include <vcl/builder.hxx>
#include <vcl/svapp.hxx>

#include <comphelper/processfactory.hxx>
#include <unotools/intlwrapper.hxx>
#include <svtools/svlbitm.hxx>
#include "svtools/treelistentry.hxx"

// SvxSimpleTableContainer  ------------------------------------------------------

SvxSimpleTableContainer::SvxSimpleTableContainer(Window* pParent, const ResId& rResId)
    : Control(pParent, rResId)
    , m_pTable(NULL)
{
    SetBorderStyle(WINDOW_BORDER_NOBORDER);
}

SvxSimpleTableContainer::SvxSimpleTableContainer(Window* pParent, WinBits nBits)
    : Control(pParent, nBits)
    , m_pTable(NULL)
{
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxSimpleTableContainer(Window *pParent,
    VclBuilder::stringmap &)
{
    return new SvxSimpleTableContainer(pParent, WB_TABSTOP | WB_DIALOGCONTROL | WB_BORDER);
}

void SvxSimpleTableContainer::SetTable(SvxSimpleTable* pTable)
{
    m_pTable = pTable;
}

long SvxSimpleTableContainer::PreNotify( NotifyEvent& rNEvt )
{
    long nResult = sal_True;
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyCode& aKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey = aKeyCode.GetCode();
        if (nKey == KEY_TAB)
            GetParent()->Notify( rNEvt );
        else if (m_pTable && m_pTable->IsFocusOnCellEnabled() && ( nKey == KEY_LEFT || nKey == KEY_RIGHT))
            return 0;
        else
            nResult = Control::PreNotify( rNEvt );
    }
    else
        nResult = Control::PreNotify( rNEvt );

    return nResult;
}

void SvxSimpleTableContainer::Resize()
{
    Control::Resize();
    if (m_pTable)
        m_pTable->UpdateViewSize();
}

void SvxSimpleTableContainer::GetFocus()
{
    Control::GetFocus();
    if (m_pTable)
        m_pTable->GrabFocus();
}

// SvxSimpleTable ------------------------------------------------------------

SvxSimpleTable::SvxSimpleTable(SvxSimpleTableContainer& rParent, WinBits nBits):
        SvHeaderTabListBox(&rParent, nBits | WB_CLIPCHILDREN | WB_HSCROLL | WB_TABSTOP),
        m_rParentTableContainer(rParent),
        aHeaderBar(&rParent,WB_BUTTONSTYLE | WB_BORDER | WB_TABSTOP),
        nHeaderItemId(1),
        bResizeFlag(sal_False),
        bPaintFlag(sal_True)
{
    m_rParentTableContainer.SetTable(this);

    bSortDirection=sal_True;
    nSortCol=0xFFFF;
    nOldPos=0;

    aHeaderBar.SetStartDragHdl(LINK( this, SvxSimpleTable, StartDragHdl));
    aHeaderBar.SetDragHdl(LINK( this, SvxSimpleTable, DragHdl));
    aHeaderBar.SetEndDragHdl(LINK( this, SvxSimpleTable, EndDragHdl));
    aHeaderBar.SetSelectHdl(LINK( this, SvxSimpleTable, HeaderBarClick));
    aHeaderBar.SetDoubleClickHdl(LINK( this, SvxSimpleTable, HeaderBarDblClick));

    EnableCellFocus();
    DisableTransientChildren();
    InitHeaderBar( &aHeaderBar );

    UpdateViewSize();

    aHeaderBar.Show();
    SvHeaderTabListBox::Show();
}

SvxSimpleTable::~SvxSimpleTable()
{
    m_rParentTableContainer.SetTable(NULL);
}

void SvxSimpleTable::UpdateViewSize()
{
    Size theWinSize=m_rParentTableContainer.GetOutputSizePixel();
    Size HbSize=aHeaderBar.GetSizePixel();

    HbSize.Width()=theWinSize.Width();
    theWinSize.Height()-=HbSize.Height();
    Point thePos(0,0);

    aHeaderBar.SetPosPixel(thePos);
    aHeaderBar.SetSizePixel(HbSize);

    thePos.Y()+=HbSize.Height();
    SvHeaderTabListBox::SetPosPixel(thePos);
    SvHeaderTabListBox::SetSizePixel(theWinSize);
    Invalidate();
}

void SvxSimpleTable::NotifyScrolled()
{
    long nOffset=-GetXOffset();
    if(nOldPos!=nOffset)
    {
        aHeaderBar.SetOffset(nOffset);
        aHeaderBar.Invalidate();
        aHeaderBar.Update();
        nOldPos=nOffset;
    }
    SvHeaderTabListBox::NotifyScrolled();
}

void SvxSimpleTable::SetTabs()
{
    SvHeaderTabListBox::SetTabs();

    sal_uInt16 nPrivTabCount = TabCount();
    if ( nPrivTabCount )
    {
        if ( nPrivTabCount > aHeaderBar.GetItemCount() )
            nPrivTabCount = aHeaderBar.GetItemCount();

        sal_uInt16 i, nNewSize = static_cast< sal_uInt16 >( GetTab(0) ), nPos = 0;
        for ( i = 1; i < nPrivTabCount; ++i )
        {
            nNewSize = static_cast< sal_uInt16 >( GetTab(i) ) - nPos;
            aHeaderBar.SetItemSize( i, nNewSize );
            nPos = (sal_uInt16)GetTab(i);
        }

        aHeaderBar.SetItemSize( i, HEADERBAR_FULLSIZE ); // because no tab for last entry
    }
}

void SvxSimpleTable::SetTabs( long* pTabs, MapUnit eMapUnit)
{
    SvHeaderTabListBox::SetTabs(pTabs,eMapUnit);
}

void SvxSimpleTable::Paint( const Rectangle& rRect )
{
    SvHeaderTabListBox::Paint(rRect );

    sal_uInt16 nPrivTabCount = TabCount();
    sal_uInt16 nNewSize = ( nPrivTabCount > 0 ) ? (sal_uInt16)GetTab(0) : 0;

    long nOffset=-GetXOffset();
    nOldPos=nOffset;

    aHeaderBar.SetOffset(nOffset);
    aHeaderBar.Invalidate();

    if(nPrivTabCount && bPaintFlag)
    {
        if(nPrivTabCount>aHeaderBar.GetItemCount())
                nPrivTabCount=aHeaderBar.GetItemCount();

        sal_uInt16 nPos = 0;
        for(sal_uInt16 i=1;i<nPrivTabCount;i++)
        {
            nNewSize = static_cast< sal_uInt16 >( GetTab(i) ) - nPos;
            aHeaderBar.SetItemSize( i, nNewSize );
            nPos= static_cast< sal_uInt16 >( GetTab(i) );
        }
    }
    bPaintFlag=sal_True;
}
void SvxSimpleTable::InsertHeaderEntry(const OUString& rText,
    sal_uInt16 nCol, HeaderBarItemBits nBits)
{
    sal_Int32 nEnd = rText.indexOf( sal_Unicode( '\t' ) );
    if( nEnd == -1 )
    {
        aHeaderBar.InsertItem(nHeaderItemId++, rText, 0, nBits, nCol);
    }
    else
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aString = rText.getToken(0, '\t', nIndex);
            aHeaderBar.InsertItem(nHeaderItemId++, aString, 0, nBits, nCol);
        }
        while ( nIndex >= 0 );
    }
    SetTabs();
}

void SvxSimpleTable::ClearHeader()
{
    aHeaderBar.Clear();
}

void SvxSimpleTable::ShowTable()
{
    m_rParentTableContainer.Show();
}

void SvxSimpleTable::HideTable()
{
    m_rParentTableContainer.Hide();
}

sal_Bool SvxSimpleTable::IsVisible() const
{
    return m_rParentTableContainer.IsVisible();
}

void SvxSimpleTable::EnableTable()
{
    m_rParentTableContainer.Enable();
}

void SvxSimpleTable::DisableTable()
{
    m_rParentTableContainer.Disable();
}

sal_Bool SvxSimpleTable::IsEnabled() const
{
    return m_rParentTableContainer.IsEnabled();
}

sal_uInt16 SvxSimpleTable::GetSelectedCol()
{
    return (aHeaderBar.GetCurItemId()-1);
}

void SvxSimpleTable::SortByCol(sal_uInt16 nCol,sal_Bool bDir)
{
    bSortDirection=bDir;
    if(nSortCol!=0xFFFF)
        aHeaderBar.SetItemBits(nSortCol+1,HIB_STDSTYLE);

    if (nCol != 0xFFFF)
    {
        if(bDir)
        {
            aHeaderBar.SetItemBits( nCol+1, HIB_STDSTYLE | HIB_DOWNARROW);
            GetModel()->SetSortMode(SortAscending);
        }
        else
        {
            aHeaderBar.SetItemBits( nCol+1, HIB_STDSTYLE | HIB_UPARROW);
            GetModel()->SetSortMode(SortDescending);
        }
        nSortCol=nCol;
        GetModel()->SetCompareHdl( LINK( this, SvxSimpleTable, CompareHdl));
        GetModel()->Resort();
    }
    else
        GetModel()->SetSortMode(SortNone);
    nSortCol=nCol;
}

void SvxSimpleTable::HBarClick()
{
    sal_uInt16 nId=aHeaderBar.GetCurItemId();

    if (aHeaderBar.GetItemBits(nId) & HIB_CLICKABLE)
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

void SvxSimpleTable::HBarDblClick()
{
    aHeaderBarDblClickLink.Call(this);
}

void SvxSimpleTable::HBarStartDrag()
{
    if(!aHeaderBar.IsItemMode())
    {
        Rectangle aSizeRect(Point(0,0),
            SvHeaderTabListBox::GetOutputSizePixel());
        aSizeRect.Left()=-GetXOffset()+aHeaderBar.GetDragPos();
        aSizeRect.Right()=-GetXOffset()+aHeaderBar.GetDragPos();
        ShowTracking( aSizeRect, SHOWTRACK_SPLIT );
    }
}
void SvxSimpleTable::HBarDrag()
{
    HideTracking();
    if(!aHeaderBar.IsItemMode())
    {
        Rectangle aSizeRect(Point(0,0),
            SvHeaderTabListBox::GetOutputSizePixel());
        aSizeRect.Left()=-GetXOffset()+aHeaderBar.GetDragPos();
        aSizeRect.Right()=-GetXOffset()+aHeaderBar.GetDragPos();
        ShowTracking( aSizeRect, SHOWTRACK_SPLIT );
    }
}
void SvxSimpleTable::HBarEndDrag()
{
    HideTracking();
    sal_uInt16 nPrivTabCount=TabCount();

    if(nPrivTabCount)
    {
        if(nPrivTabCount>aHeaderBar.GetItemCount())
                nPrivTabCount=aHeaderBar.GetItemCount();

        sal_uInt16 nPos=0;
        sal_uInt16 nNewSize=0;
        for(sal_uInt16 i=1;i<nPrivTabCount;i++)
        {
            nNewSize = static_cast< sal_uInt16 >( aHeaderBar.GetItemSize(i) ) + nPos;
            SetTab( i, nNewSize, MAP_PIXEL );
            nPos = nNewSize;
        }
    }
    bPaintFlag=sal_False;
    Invalidate();
    Update();
}

CommandEvent SvxSimpleTable::GetCommandEvent() const
{
    return aCEvt;
}

void SvxSimpleTable::Command( const CommandEvent& rCEvt )
{
    aCEvt=rCEvt;
    aCommandLink.Call(this);
    SvHeaderTabListBox::Command(rCEvt);
}

IMPL_LINK( SvxSimpleTable, StartDragHdl, HeaderBar*, pCtr)
{
    if(pCtr==&aHeaderBar)
    {
        HBarStartDrag();
    }
    return 0;
}

IMPL_LINK( SvxSimpleTable, DragHdl, HeaderBar*, pCtr)
{
    if(pCtr==&aHeaderBar)
    {
        HBarDrag();
    }
    return 0;
}

IMPL_LINK( SvxSimpleTable, EndDragHdl, HeaderBar*, pCtr)
{
    if(pCtr==&aHeaderBar)
    {
        HBarEndDrag();
    }
    return 0;
}

IMPL_LINK( SvxSimpleTable, HeaderBarClick, HeaderBar*, pCtr)
{
    if(pCtr==&aHeaderBar)
    {
        HBarClick();
    }
    return 0;
}

IMPL_LINK( SvxSimpleTable, HeaderBarDblClick, HeaderBar*, pCtr)
{
    if(pCtr==&aHeaderBar)
    {
        HBarDblClick();
    }
    return 0;
}

SvLBoxItem* SvxSimpleTable::GetEntryAtPos( SvTreeListEntry* pEntry, sal_uInt16 nPos ) const
{
    DBG_ASSERT(pEntry,"GetEntryText:Invalid Entry");
    SvLBoxItem* pItem = NULL;

    if( pEntry )
    {
        sal_uInt16 nCount = pEntry->ItemCount();

        nPos++;

        if( nTreeFlags & TREEFLAG_CHKBTN ) nPos++;

        if( nPos < nCount )
        {
            pItem = pEntry->GetItem( nPos);
        }
    }
    return pItem;
}

StringCompare SvxSimpleTable::ColCompare(SvTreeListEntry* pLeft,SvTreeListEntry* pRight)
{
    StringCompare eCompare=COMPARE_EQUAL;

    SvLBoxItem* pLeftItem = GetEntryAtPos( pLeft, nSortCol);
    SvLBoxItem* pRightItem = GetEntryAtPos( pRight, nSortCol);


    if(pLeftItem != NULL && pRightItem != NULL)
    {
        sal_uInt16 nLeftKind = pLeftItem->GetType();
        sal_uInt16 nRightKind = pRightItem->GetType();

        if(nRightKind == SV_ITEM_ID_LBOXSTRING &&
            nLeftKind == SV_ITEM_ID_LBOXSTRING )
        {
            IntlWrapper aIntlWrapper( Application::GetSettings().GetLanguageTag() );
            const CollatorWrapper* pCollator = aIntlWrapper.getCaseCollator();

            eCompare=(StringCompare)pCollator->compareString( ((SvLBoxString*)pLeftItem)->GetText(),
                                    ((SvLBoxString*)pRightItem)->GetText());

            if(eCompare==COMPARE_EQUAL) eCompare=COMPARE_LESS;
        }
    }
    return eCompare;
}

IMPL_LINK( SvxSimpleTable, CompareHdl, SvSortData*, pData)
{
    SvTreeListEntry* pLeft = (SvTreeListEntry*)(pData->pLeft );
    SvTreeListEntry* pRight = (SvTreeListEntry*)(pData->pRight );
    return (long) ColCompare(pLeft,pRight);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
