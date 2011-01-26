/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// INCLUDE -------------------------------------------------------------------

#include <svx/simptabl.hxx>
#include <vcl/svapp.hxx>

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <unotools/intlwrapper.hxx>

// SvxSimpTblContainer  ------------------------------------------------------

SvxSimpTblContainer::SvxSimpTblContainer( Window* pParent, WinBits nWinStyle):
        Control(pParent,nWinStyle)
{
    //Do Nothing;
}

SvxSimpTblContainer::SvxSimpTblContainer( Window* pParent, const ResId& rResId):
        Control(pParent,rResId)
{
    //Do Nothing;
}

long SvxSimpTblContainer::PreNotify( NotifyEvent& rNEvt )
{
    long nResult = TRUE;
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyCode& aKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        USHORT nKey = aKeyCode.GetCode();
        if ( nKey == KEY_TAB )
            GetParent()->Notify( rNEvt );
        else if ( m_pTable->IsFocusOnCellEnabled() && ( nKey == KEY_LEFT || nKey == KEY_RIGHT ) )
            return 0;
        else
            nResult = Control::PreNotify( rNEvt );
    }
    else
        nResult = Control::PreNotify( rNEvt );

    return nResult;
}


// SvxSimpleTable ------------------------------------------------------------

SvxSimpleTable::SvxSimpleTable( Window* pParent,WinBits nBits ):
        SvHeaderTabListBox(pParent,WB_CLIPCHILDREN | WB_HSCROLL | WB_TABSTOP),
        aPrivContainer(pParent,nBits|WB_DIALOGCONTROL),
        aHeaderBar(pParent,WB_BUTTONSTYLE | WB_BORDER | WB_TABSTOP),
        nHeaderItemId(1),
        bResizeFlag(FALSE),
        bPaintFlag(TRUE)
{
    bSortDirection=TRUE;
    nSortCol=0xFFFF;
    nOldPos=0;

    SetParent(&aPrivContainer);
    aHeaderBar.SetParent(&aPrivContainer);
    aPrivContainer.SetTable( this );

    aHeaderBar.SetStartDragHdl(LINK( this, SvxSimpleTable, StartDragHdl));
    aHeaderBar.SetDragHdl(LINK( this, SvxSimpleTable, DragHdl));
    aHeaderBar.SetEndDragHdl(LINK( this, SvxSimpleTable, EndDragHdl));
    aHeaderBar.SetSelectHdl(LINK( this, SvxSimpleTable, HeaderBarClick));
    aHeaderBar.SetDoubleClickHdl(LINK( this, SvxSimpleTable, HeaderBarDblClick));

    EnableCellFocus();
    DisableTransientChildren();
    InitHeaderBar( &aHeaderBar );

    aHeaderBar.Show();
    SvHeaderTabListBox::Show();
}


SvxSimpleTable::SvxSimpleTable( Window* pParent,const ResId& rResId):
        SvHeaderTabListBox(pParent,WB_CLIPCHILDREN | WB_HSCROLL | WB_TABSTOP),
        aPrivContainer(pParent,rResId),
        aHeaderBar(pParent,WB_BUTTONSTYLE | WB_BORDER  | WB_TABSTOP),
        nHeaderItemId(1),
        bResizeFlag(TRUE),
        bPaintFlag(TRUE)
{

    bSortDirection=TRUE;
    nOldPos=0;
    nSortCol=0xFFFF;

    pMyParentWin=pParent;
    SetParent(&aPrivContainer);
    aHeaderBar.SetParent(&aPrivContainer);
    aPrivContainer.SetTable( this );

    WinBits nBits=aPrivContainer.GetStyle()|WB_DIALOGCONTROL;
    aPrivContainer.SetStyle(nBits);

    aHeaderBar.SetStartDragHdl(LINK( this, SvxSimpleTable, StartDragHdl));
    aHeaderBar.SetDragHdl(LINK( this, SvxSimpleTable, DragHdl));
    aHeaderBar.SetEndDragHdl(LINK( this, SvxSimpleTable, EndDragHdl));
    aHeaderBar.SetSelectHdl(LINK( this, SvxSimpleTable, HeaderBarClick));
    aHeaderBar.SetDoubleClickHdl(LINK( this, SvxSimpleTable, HeaderBarDblClick));

    Size theWinSize=aPrivContainer.GetOutputSizePixel();
    Size HbSize=aHeaderBar.GetSizePixel();

    HbSize.Width()=theWinSize.Width();
    theWinSize.Height()-=HbSize.Height();
    Point   thePos(0,0);

    aHeaderBar.SetPosPixel(thePos);
    aHeaderBar.SetSizePixel(HbSize);

    thePos.Y()+=HbSize.Height();
    SvHeaderTabListBox::SetPosPixel(thePos);
    SvHeaderTabListBox::SetSizePixel(theWinSize);

    EnableCellFocus();
    DisableTransientChildren();
    InitHeaderBar( &aHeaderBar );

    aHeaderBar.Show();
    SvHeaderTabListBox::Show();
}

SvxSimpleTable::~SvxSimpleTable()
{
    SetParent(pMyParentWin);
    aPrivContainer.SetParent(this);
    aHeaderBar.SetParent(this);
}


void SvxSimpleTable::UpdateViewSize()
{
    Size theWinSize=aPrivContainer.GetOutputSizePixel();
    Size HbSize=aHeaderBar.GetSizePixel();

    HbSize.Width()=theWinSize.Width();
    theWinSize.Height()-=HbSize.Height();
    Point   thePos(0,0);

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

    USHORT nPrivTabCount = TabCount();
    if ( nPrivTabCount )
    {
        if ( nPrivTabCount > aHeaderBar.GetItemCount() )
            nPrivTabCount = aHeaderBar.GetItemCount();

        USHORT i, nNewSize = static_cast< USHORT >( GetTab(0) ), nPos = 0;
        for ( i = 1; i < nPrivTabCount; ++i )
        {
            nNewSize = static_cast< USHORT >( GetTab(i) ) - nPos;
            aHeaderBar.SetItemSize( i, nNewSize );
            nPos = (USHORT)GetTab(i);
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

    USHORT nPrivTabCount = TabCount();
    USHORT nPos = 0;
    USHORT nNewSize = ( nPrivTabCount > 0 ) ? (USHORT)GetTab(0) : 0;

    long nOffset=-GetXOffset();
    nOldPos=nOffset;

    aHeaderBar.SetOffset(nOffset);
    aHeaderBar.Invalidate();

    if(nPrivTabCount && bPaintFlag)
    {
        if(nPrivTabCount>aHeaderBar.GetItemCount())
                nPrivTabCount=aHeaderBar.GetItemCount();

        for(USHORT i=1;i<nPrivTabCount;i++)
        {
            nNewSize = static_cast< USHORT >( GetTab(i) ) - nPos;
            aHeaderBar.SetItemSize( i, nNewSize );
            nPos= static_cast< USHORT >( GetTab(i) );
        }
    }
    bPaintFlag=TRUE;
}
void SvxSimpleTable::InsertHeaderEntry(const XubString& rText,USHORT nCol,
                                       HeaderBarItemBits nBits)
{
    xub_StrLen nEnd = rText.Search( sal_Unicode( '\t' ) );
    if( nEnd == STRING_NOTFOUND )
    {
        aHeaderBar.InsertItem(nHeaderItemId++, rText, 0, nBits, nCol);
    }
    else
    {
        xub_StrLen nCount = rText.GetTokenCount( sal_Unicode( '\t' ) );

        for( xub_StrLen i=0; i<nCount; i++ )
        {
            String  aString=rText.GetToken(i, sal_Unicode( '\t' ) );
            aHeaderBar.InsertItem(nHeaderItemId++, aString, 0, nBits, nCol);
        }
    }
    SetTabs();
}

void SvxSimpleTable::ClearAll()
{
    aHeaderBar.Clear();
    Clear();
}
void SvxSimpleTable::ClearHeader()
{
    aHeaderBar.Clear();
}

void SvxSimpleTable::ShowTable()
{
    aPrivContainer.Show();
}

void SvxSimpleTable::HideTable()
{
    aPrivContainer.Hide();
}

BOOL SvxSimpleTable::IsVisible() const
{
    return aPrivContainer.IsVisible();
}

void SvxSimpleTable::EnableTable()
{
    aPrivContainer.Enable();
}

void SvxSimpleTable::DisableTable()
{
    aPrivContainer.Disable();
}

BOOL SvxSimpleTable::IsEnabled() const
{
    return aPrivContainer.IsEnabled();
}

void SvxSimpleTable::TableToTop()
{
    aPrivContainer.ToTop();
}

void SvxSimpleTable::SetPosPixel( const Point& rNewPos )
{
    aPrivContainer.SetPosPixel(rNewPos);
}

Point SvxSimpleTable::GetPosPixel() const
{
    return aPrivContainer.GetPosPixel();
}

void SvxSimpleTable::SetPosSizePixel( const Point& rNewPos, Size& rNewSize )
{
    aPrivContainer.SetPosPixel(rNewPos);
    aPrivContainer.SetSizePixel(rNewSize);
}

void SvxSimpleTable::SetPosSize( const Point& rNewPos, const Size& rNewSize )
{
    aPrivContainer.SetPosPixel(rNewPos);
    SvHeaderTabListBox::SetPosSizePixel(rNewPos,rNewSize);
}

Size SvxSimpleTable::GetSizePixel() const
{
    return aPrivContainer.GetSizePixel();
}

Size SvxSimpleTable::GetOutputSizePixel() const
{
    return aPrivContainer.GetOutputSizePixel();
}

void SvxSimpleTable::SetSizePixel(const Size& rNewSize )
{
    aPrivContainer.SetSizePixel(rNewSize);
    UpdateViewSize();
}

void SvxSimpleTable::SetOutputSizePixel(const Size& rNewSize )
{
    aPrivContainer.SetOutputSizePixel(rNewSize);
    UpdateViewSize();
}

USHORT SvxSimpleTable::GetSelectedCol()
{
    return (aHeaderBar.GetCurItemId()-1);
}

void SvxSimpleTable::SortByCol(USHORT nCol,BOOL bDir)
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
    USHORT nId=aHeaderBar.GetCurItemId();

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
    USHORT nPrivTabCount=TabCount();
    USHORT nPos=0;
    USHORT nNewSize=0;

    if(nPrivTabCount)
    {
        if(nPrivTabCount>aHeaderBar.GetItemCount())
                nPrivTabCount=aHeaderBar.GetItemCount();

        //for(USHORT i=1;i<=nPrivTabCount;i++)
        for(USHORT i=1;i<nPrivTabCount;i++)
        {
            nNewSize = static_cast< USHORT >( aHeaderBar.GetItemSize(i) ) + nPos;
            SetTab( i, nNewSize, MAP_PIXEL );
            nPos = nNewSize;
        }
    }
    bPaintFlag=FALSE;
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

SvLBoxItem* SvxSimpleTable::GetEntryAtPos( SvLBoxEntry* pEntry, USHORT nPos ) const
{
    DBG_ASSERT(pEntry,"GetEntryText:Invalid Entry");
    SvLBoxItem* pItem = NULL;

    if( pEntry )
    {
        USHORT nCount = pEntry->ItemCount();

        nPos++;

        if( nTreeFlags & TREEFLAG_CHKBTN ) nPos++;

        if( nPos < nCount )
        {
            pItem = pEntry->GetItem( nPos);
        }
    }
    return pItem;
}

StringCompare SvxSimpleTable::ColCompare(SvLBoxEntry* pLeft,SvLBoxEntry* pRight)
{
    StringCompare eCompare=COMPARE_EQUAL;

    SvLBoxItem* pLeftItem = GetEntryAtPos( pLeft, nSortCol);
    SvLBoxItem* pRightItem = GetEntryAtPos( pRight, nSortCol);


    if(pLeftItem != NULL && pRightItem != NULL)
    {
        USHORT nLeftKind=pLeftItem->IsA();
        USHORT nRightKind=pRightItem->IsA();

        if(nRightKind == SV_ITEM_ID_LBOXSTRING &&
            nLeftKind == SV_ITEM_ID_LBOXSTRING )
        {
            IntlWrapper aIntlWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
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
    SvLBoxEntry* pLeft = (SvLBoxEntry*)(pData->pLeft );
    SvLBoxEntry* pRight = (SvLBoxEntry*)(pData->pRight );
    return (long) ColCompare(pLeft,pRight);
}


