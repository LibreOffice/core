/*************************************************************************
 *
 *  $RCSfile: simptabl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// INCLUDE -------------------------------------------------------------------

#include "simptabl.hxx"
#include <vcl/svapp.hxx>


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
    long nResult=TRUE;

    USHORT nSwitch=rNEvt.GetType();
    if(nSwitch==EVENT_KEYINPUT)
    {
        const KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
        USHORT nKey=aKeyCode.GetCode();
        if(nKey==KEY_TAB)
        {
            GetParent()->Notify(rNEvt);
        }
        else
            nResult=Control::PreNotify(rNEvt);
    }
    else
        nResult=Control::PreNotify(rNEvt);

    return nResult;
}


// SvxSimpleTable ------------------------------------------------------------

SvxSimpleTable::SvxSimpleTable( Window* pParent,WinBits nBits ):
        SvTabListBox(pParent,WB_CLIPCHILDREN | WB_HSCROLL | WB_TABSTOP),
        aPrivContainer(pParent,nBits|WB_DIALOGCONTROL),
        aHeaderBar(pParent,WB_BUTTONSTYLE | WB_BORDER | WB_TABSTOP),
        bResizeFlag(FALSE),
        bPaintFlag(TRUE),
        nHeaderItemId(1)
{
    bSortDirection=TRUE;
    nSortCol=0xFFFF;
    nOldPos=0;

    SetParent(&aPrivContainer);
    aHeaderBar.SetParent(&aPrivContainer);

    aHeaderBar.SetStartDragHdl(LINK( this, SvxSimpleTable, StartDragHdl));
    aHeaderBar.SetDragHdl(LINK( this, SvxSimpleTable, DragHdl));
    aHeaderBar.SetEndDragHdl(LINK( this, SvxSimpleTable, EndDragHdl));
    aHeaderBar.SetSelectHdl(LINK( this, SvxSimpleTable, HeaderBarClick));
    aHeaderBar.SetDoubleClickHdl(LINK( this, SvxSimpleTable, HeaderBarDblClick));

    aHeaderBar.Show();
    SvTabListBox::Show();
}


SvxSimpleTable::SvxSimpleTable( Window* pParent,const ResId& rResId):
        SvTabListBox(pParent,WB_CLIPCHILDREN | WB_HSCROLL | WB_TABSTOP),
        aPrivContainer(pParent,rResId),
        aHeaderBar(pParent,WB_BUTTONSTYLE | WB_BORDER  | WB_TABSTOP),
        bResizeFlag(TRUE),
        bPaintFlag(TRUE),
        nHeaderItemId(1)
{

    bSortDirection=TRUE;
    nOldPos=0;
    nSortCol=0xFFFF;

    pMyParentWin=pParent;
    SetParent(&aPrivContainer);
    aHeaderBar.SetParent(&aPrivContainer);

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
    SvTabListBox::SetPosPixel(thePos);
    SvTabListBox::SetSizePixel(theWinSize);
    aHeaderBar.Show();
    SetWindowBits(WB_CLIPCHILDREN | WB_HSCROLL);
    SvTabListBox::Show();
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
    SvTabListBox::SetPosPixel(thePos);
    SvTabListBox::SetSizePixel(theWinSize);
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
    SvTabListBox::NotifyScrolled();
}

void SvxSimpleTable::SetTabs()
{
    SvTabListBox::SetTabs();

    USHORT nPrivTabCount=TabCount();
    USHORT nPos=0;
    USHORT nNewSize=(USHORT)GetTab(0);

    if(nPrivTabCount)
    {
        if(nPrivTabCount>aHeaderBar.GetItemCount())
                nPrivTabCount=aHeaderBar.GetItemCount();

        USHORT i;
        for(i=1;i<nPrivTabCount;i++)
        {
            nNewSize=GetTab(i)-nPos;
            aHeaderBar.SetItemSize(i,nNewSize );
            nPos=(USHORT) GetTab(i);
        }
        aHeaderBar.SetItemSize(i,HEADERBAR_FULLSIZE); // Da fuer letzten Eintrag kein Tab

    }
}
void SvxSimpleTable::SetTabs( long* pTabs, MapUnit eMapUnit)
{
    SvTabListBox::SetTabs(pTabs,eMapUnit);
}

void SvxSimpleTable::Paint( const Rectangle& rRect )
{
    SvTabListBox::Paint(rRect );

    USHORT nPrivTabCount=TabCount();
    USHORT nPos=0;
    USHORT nNewSize=(USHORT)GetTab(0);

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
            nNewSize=GetTab(i)-nPos;
            aHeaderBar.SetItemSize(i,nNewSize );
            nPos=(USHORT) GetTab(i);
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

void SvxSimpleTable::Show()
{
    aPrivContainer.Show();
}

void SvxSimpleTable::Hide()
{
    aPrivContainer.Show();
}

BOOL SvxSimpleTable::IsVisible() const
{
    return aPrivContainer.IsVisible();
}

void SvxSimpleTable::Enable()
{
    aPrivContainer.Enable();
}

void SvxSimpleTable::Disable()
{
    aPrivContainer.Disable();
}

BOOL SvxSimpleTable::IsEnabled() const
{
    return aPrivContainer.IsEnabled();
}

void SvxSimpleTable::ToTop()
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
    SvTabListBox::SetPosSizePixel(rNewPos,rNewSize);
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
            SvTabListBox::GetOutputSizePixel());
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
            SvTabListBox::GetOutputSizePixel());
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
            nNewSize=aHeaderBar.GetItemSize(i)+nPos;
            SetTab(i,nNewSize,MAP_PIXEL);
            nPos=nNewSize;
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
    SvTabListBox::Command(rCEvt);
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
            const International& rInter = GetpApp()->GetAppInternational();

            eCompare=rInter.Compare( ((SvLBoxString*)pLeftItem)->GetText(),
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


