/*************************************************************************
 *
 *  $RCSfile: property.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

//#include "Svitems.hxx"

#include "property.hxx"
/*
#include "property.hrc"
#include "Scresid.hxx"
*/
#define FRAME_OFFSET 4


SvXPropertyControl::SvXPropertyControl( Window* pParent, WinBits nWinStyle)
            :   Control(pParent,nWinStyle)
{
}

SvXPropertyControl::SvXPropertyControl( Window* pParent, const ResId& rResId )
            :   Control(pParent,rResId )
{
}

//------------------------------------------------------------------

SvXPropertyEdit::SvXPropertyEdit( Window* pParent, WinBits nWinStyle)
        :       SvXPropertyControl(pParent,nWinStyle),
                aEdit(this,WB_BORDER | WB_TABSTOP)
{
    pListener=NULL;
    aEdit.SetModifyHdl(
        LINK( this, SvXPropertyEdit, ModifiedHdl ));
    aEdit.SetGetFocusHdl(
        LINK( this, SvXPropertyEdit, GetFocusHdl));
    aEdit.SetLoseFocusHdl(
        LINK( this, SvXPropertyEdit, LoseFocusHdl));

    aEdit.Show();
}

SvXPropertyEdit::SvXPropertyEdit( Window* pParent, const ResId& rResId)
        :       SvXPropertyControl(pParent,rResId),
                aEdit(this,WB_BORDER | WB_TABSTOP)
{
    pListener=NULL;
    aEdit.SetModifyHdl(
        LINK( this, SvXPropertyEdit, ModifiedHdl ));
    aEdit.SetGetFocusHdl(
        LINK( this, SvXPropertyEdit, GetFocusHdl));
    aEdit.SetLoseFocusHdl(
        LINK( this, SvXPropertyEdit, LoseFocusHdl));

    Size aSize=GetSizePixel();
    SetCtrSize(aSize);
    aEdit.Show();
}

void SvXPropertyEdit::SetSvXPropertyCtrListener(SvXPropertyCtrListener* pCtrListener)
{
    pListener=pCtrListener;
}

SvXPropertyCtrListener* SvXPropertyEdit::GetSvXPropertyCtrListener()
{
    return pListener;
}


void SvXPropertyEdit::SetCtrSize(const Size& rSize)
{
    SetSizePixel(rSize);
    Size aSize=GetOutputSizePixel();
    Point aPos(0,0);
    aEdit.SetPosPixel(aPos);
    aEdit.SetSizePixel(aSize);
}

void SvXPropertyEdit::SetLocked(BOOL bLocked)
{
    if(bLocked)
        Disable();
    else
        Enable();
}

void SvXPropertyEdit::SetProperty(const String &rString)
{
    aEdit.SetText(rString);
}

String SvXPropertyEdit::GetProperty() const
{
    return aEdit.GetText();
}

BOOL SvXPropertyEdit::HasList()
{
    return FALSE;
};


void SvXPropertyEdit::ClearList()
{
    return;
}
void SvXPropertyEdit::InsertEntry( const String& rString,USHORT nPos)
{
    return;
}

void SvXPropertyEdit::SetMyName(const String &rString)
{
    aName=rString;
}

String SvXPropertyEdit::GetMyName()const
{
    return aName;
}

void SvXPropertyEdit::SetMyData(void* pDat)
{
    pData=pDat;
}

void* SvXPropertyEdit::GetMyData()
{
    return pData;
}


IMPL_LINK( SvXPropertyEdit, ModifiedHdl, Edit*, pEd )
{
    if(pListener!=NULL)
        pListener->Modified(this);
    return 0;
}

IMPL_LINK( SvXPropertyEdit, GetFocusHdl, Edit*, pEd )
{
    if(pListener!=NULL)
        pListener->GetFocus(this);
    return 0;
}

IMPL_LINK( SvXPropertyEdit, LoseFocusHdl, Edit*, pEd )
{
    if(pListener!=NULL)
        pListener->LoseFocus(this);
    return 0;
}

//------------------------------------------------------------------

SvXPropertyListBox::SvXPropertyListBox( Window* pParent, WinBits nWinStyle)
        :       SvXPropertyControl(pParent,nWinStyle),
                aListBox(this,WB_BORDER | WB_DROPDOWN | WB_TABSTOP)
{
    pListener=NULL;
    aListBox.SetSelectHdl(
        LINK( this, SvXPropertyListBox, ModifiedHdl ));
    aListBox.SetGetFocusHdl(
        LINK( this, SvXPropertyListBox, GetFocusHdl));
    aListBox.SetLoseFocusHdl(
        LINK( this, SvXPropertyListBox, LoseFocusHdl));
    aListBox.Show();
}

SvXPropertyListBox::SvXPropertyListBox( Window* pParent, const ResId& rResId)
        :       SvXPropertyControl(pParent,rResId),
                aListBox(this,WB_BORDER | WB_DROPDOWN | WB_TABSTOP)
{
    pListener=NULL;
    aListBox.SetSelectHdl(
        LINK( this, SvXPropertyListBox, ModifiedHdl ));
    aListBox.SetGetFocusHdl(
        LINK( this, SvXPropertyListBox, GetFocusHdl));
    aListBox.SetLoseFocusHdl(
        LINK( this, SvXPropertyListBox, LoseFocusHdl));

    Size aSize=GetSizePixel();
    SetCtrSize(aSize);
    aListBox.Show();
}

void SvXPropertyListBox::SetSvXPropertyCtrListener(SvXPropertyCtrListener* pCtrListener)
{
    pListener=pCtrListener;
}

SvXPropertyCtrListener* SvXPropertyListBox::GetSvXPropertyCtrListener()
{
    return pListener;
}


void SvXPropertyListBox::SetCtrSize(const Size& rSize)
{
    SetSizePixel(rSize);
    Size aSize=GetOutputSizePixel();
    Point aPos(0,0);
    aListBox.SetPosPixel(aPos);
    aListBox.SetSizePixel(aSize);
}

void SvXPropertyListBox::SetLocked(BOOL bLocked)
{
    if(bLocked)
        Disable();
    else
        Enable();
}

void SvXPropertyListBox::SetProperty(const String &rString)
{
    aListBox.SelectEntry(rString);
}

String SvXPropertyListBox::GetProperty()const
{
    return aListBox.GetSelectEntry();
}

BOOL SvXPropertyListBox::HasList()
{
    return TRUE;
}


void SvXPropertyListBox::ClearList()
{
    aListBox.Clear();
}

void SvXPropertyListBox::InsertEntry( const String& rString,USHORT nPos)
{
    aListBox.InsertEntry(rString,nPos);
}

void SvXPropertyListBox::SetMyName(const String &rString)
{
    aName=rString;
}

String SvXPropertyListBox::GetMyName()const
{
    return aName;
}

void SvXPropertyListBox::SetMyData(void* pDat)
{
    pData=pDat;
}

void* SvXPropertyListBox::GetMyData()
{
    return pData;
}

IMPL_LINK( SvXPropertyListBox, ModifiedHdl, ListBox*, pLB )
{
    if(pListener!=NULL)
        pListener->Modified(this);
    return 0;
}

IMPL_LINK( SvXPropertyListBox, GetFocusHdl, ListBox*, pLB )
{
    if(pListener!=NULL)
        pListener->GetFocus(this);
    return 0;
}

IMPL_LINK( SvXPropertyListBox, LoseFocusHdl, ListBox*, pLB )
{
    if(pListener!=NULL)
        pListener->LoseFocus(this);
    return 0;
}

//------------------------------------------------------------------


SvXPropertyComboBox::SvXPropertyComboBox( Window* pParent, WinBits nWinStyle)
        :       SvXPropertyControl(pParent,nWinStyle),
                aComboBox(this,WB_BORDER | WB_DROPDOWN | WB_TABSTOP)
{
    pListener=NULL;
    aComboBox.SetModifyHdl(
        LINK( this, SvXPropertyComboBox, ModifiedHdl ));
    aComboBox.SetGetFocusHdl(
        LINK( this, SvXPropertyComboBox, GetFocusHdl));
    aComboBox.SetLoseFocusHdl(
        LINK( this, SvXPropertyComboBox, LoseFocusHdl));
    aComboBox.Show();
}

SvXPropertyComboBox::SvXPropertyComboBox( Window* pParent, const ResId& rResId)
        :       SvXPropertyControl(pParent,rResId),
                aComboBox(this,WB_BORDER | WB_DROPDOWN | WB_TABSTOP)
{
    pListener=NULL;
    aComboBox.SetModifyHdl(
        LINK( this, SvXPropertyComboBox, ModifiedHdl ));
    aComboBox.SetGetFocusHdl(
        LINK( this, SvXPropertyComboBox, GetFocusHdl));
    aComboBox.SetLoseFocusHdl(
        LINK( this, SvXPropertyComboBox, LoseFocusHdl));

    Size aSize=GetSizePixel();
    SetCtrSize(aSize);
    aComboBox.Show();
}

void SvXPropertyComboBox::SetLocked(BOOL bLocked)
{
    if(bLocked)
        Disable();
    else
        Enable();
}

void SvXPropertyComboBox::SetSvXPropertyCtrListener(SvXPropertyCtrListener* pCtrListener)
{
    pListener=pCtrListener;
}

SvXPropertyCtrListener* SvXPropertyComboBox::GetSvXPropertyCtrListener()
{
    return pListener;
}


void SvXPropertyComboBox::SetCtrSize(const Size& rSize)
{
    SetSizePixel(rSize);
    Size aSize=GetOutputSizePixel();
    Point aPos(0,0);
    aComboBox.SetPosPixel(aPos);
    aComboBox.SetSizePixel(aSize);
}


void SvXPropertyComboBox::SetProperty(const String &rString)
{
    aComboBox.SetText(rString);
}

String SvXPropertyComboBox::GetProperty()   const
{
    return aComboBox.GetText();
}

BOOL SvXPropertyComboBox::HasList()
{
    return TRUE;
}

void SvXPropertyComboBox::ClearList()
{
    aComboBox.Clear();
}

void SvXPropertyComboBox::InsertEntry( const String& rString,USHORT nPos)
{
    aComboBox.InsertEntry(rString,nPos);
}

void SvXPropertyComboBox::SetMyName(const String &rString)
{
    aName=rString;
}

String SvXPropertyComboBox::GetMyName()const
{
    return aName;
}

void SvXPropertyComboBox::SetMyData(void* pDat)
{
    pData=pDat;
}

void* SvXPropertyComboBox::GetMyData()
{
    return pData;
}

IMPL_LINK( SvXPropertyComboBox, ModifiedHdl, ComboBox*, pCB )
{
    if(pListener!=NULL)
        pListener->Modified(this);
    return 0;
}

IMPL_LINK( SvXPropertyComboBox, GetFocusHdl, ComboBox*, pCB )
{
    if(pListener!=NULL)
        pListener->GetFocus(this);
    return 0;
}

IMPL_LINK( SvXPropertyComboBox, LoseFocusHdl, ComboBox*, pCB )
{
    if(pListener!=NULL)
        pListener->LoseFocus(this);
    return 0;
}
//------------------------------------------------------------------

SvPropertyLine::SvPropertyLine( Window* pParent,WinBits nWinStyle)
        :   Control(pParent,nWinStyle),
            aName(this,WB_BORDER),
            aXButton(this,WB_BORDER),
            pSvXPropertyControl(NULL),
            bIsLocked(FALSE),
            bIsHyperlink(FALSE)
{
    bNeedsRepaint = TRUE;
    bHasXButton = FALSE;
    aXButton.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "..." ) ) );
    aName.Show();
    aXButton.Show();
    eKindOfCtr = KOC_UNDEFINED;
    Wallpaper aWall = GetBackground();
    aWall.SetColor( Color( COL_TRANSPARENT ) );
    SetBackground( aWall );
}

SvPropertyLine::SvPropertyLine( Window* pParent,const ResId& rResId )
        :   Control(pParent,rResId),
            aName       (this,WB_BORDER),
            aXButton    (this,WB_BORDER),
            pSvXPropertyControl(NULL),
            bIsLocked(FALSE),
            bIsHyperlink(FALSE)
{
    bNeedsRepaint = TRUE;
    bHasXButton = FALSE;
    eKindOfCtr = KOC_UNDEFINED;
    aXButton.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "..." ) ) );
    aName.Show();
    aXButton.Show();
    Wallpaper aWall = GetBackground();
    aWall.SetColor( Color( COL_TRANSPARENT ) );
    SetBackground( aWall );
    Resize();
}

void SvPropertyLine::SetSvXPropertyControl(SvXPropertyControl* pXControl)
{
    pSvXPropertyControl=pXControl;
    pSvXPropertyControl->Show();
    Resize();
}

SvXPropertyControl* SvPropertyLine::GetSvXPropertyControl()
{
    return pSvXPropertyControl;
}

void SvPropertyLine::Resize()
{
    Size aSize=GetOutputSizePixel();
    Size a2Size=aSize;

    aSize.Width()=nNameWidth;
    a2Size.Width()-=nNameWidth;

    Point aPos(0,0);
    aName.SetPosPixel(aPos);
    aName.SetSizePixel(aSize);

    USHORT nXButtonWidth=0;

    if(bHasXButton)
    {
        nXButtonWidth=(USHORT)aSize.Height();
    }
    a2Size.Width()=a2Size.Width()-nXButtonWidth;

    aPos.X()+=aSize.Width();

    if(pSvXPropertyControl!=NULL)
    {
        pSvXPropertyControl->SetPosPixel(aPos);
        pSvXPropertyControl->SetCtrSize(a2Size);
    }

    if(bHasXButton)
    {
        aPos.X()=GetOutputSizePixel().Width()
                    -nXButtonWidth;
        aSize.Width()=nXButtonWidth;
        aXButton    .SetSizePixel(aSize);
        aXButton    .SetPosPixel(aPos);
    }
}

void SvPropertyLine::SetNeedsRepaint(BOOL bFlag)
{
    bNeedsRepaint=bFlag;
}

BOOL SvPropertyLine::NeedsRepaint()
{
    return bNeedsRepaint;
}

void SvPropertyLine::SetName(const String& rString )
{
    aName.SetText(rString);
    aName.Invalidate();
}

String SvPropertyLine::GetName() const
{
    return aName.GetText();
}

void SvPropertyLine::SetKindOfControl(eKindOfControl eKOC)
{
    eKindOfCtr=eKOC;
}

eKindOfControl SvPropertyLine::GetKindOfControl()
{
    return eKindOfCtr;
}

void SvPropertyLine::ShowXButton()
{
    bHasXButton=TRUE;
    aXButton.Show();
    Resize();
}
void SvPropertyLine::HideXButton()
{
    bHasXButton=FALSE;
    aXButton.Hide();
    Resize();
}
BOOL SvPropertyLine::IsVisibleXButton()
{
    return bHasXButton;
}

void SvPropertyLine::ShowAsHyperLink(BOOL nFlag)
{
    bIsHyperlink=nFlag;
    if(nFlag)
    {
        Font aFont=GetFont();
        aFont.SetUnderline(UNDERLINE_SINGLE);
        aFont.SetColor(Color(COL_BLUE));
        aName.SetFont(aFont);
    }
    else
    {
        Font aFont=GetFont();
        aName.SetFont(aFont);
    }
}

BOOL SvPropertyLine::IsShownAsHyperlink()
{
    return bIsHyperlink;
}

void SvPropertyLine::Locked(BOOL nFlag)
{
    bIsLocked=nFlag;
    if(pSvXPropertyControl!=NULL)
        pSvXPropertyControl->SetLocked(nFlag);
}

BOOL SvPropertyLine::IsLocked()
{
    return bIsLocked;
}

void SvPropertyLine::SetNameWidth(USHORT nWidth)
{
    nNameWidth=nWidth;
    Resize();
}

void SvPropertyLine::SetClickHdl(const Link& rLink)
{
    aXButton.SetClickHdl(rLink );
}

//----------------------------------------------------------

SvXPropEvListener::SvXPropEvListener()
{
    pTheActiveControl=NULL;
}

void SvXPropEvListener::Modified (SvXPropertyControl *pSvXPCtr)
{
    pTheActiveControl=pSvXPCtr;
    aModifyLink.Call(this);
}

void SvXPropEvListener::GetFocus (SvXPropertyControl *pSvXPCtr)
{
    pTheActiveControl=pSvXPCtr;
    aGetFocusLink.Call(this);
}

void SvXPropEvListener::LoseFocus (SvXPropertyControl *pSvXPCtr)
{
    pTheActiveControl=pSvXPCtr;
    aLoseFocusLink.Call(this);
}

void SvXPropEvListener::KeyInput (SvXPropertyControl *pSvXPCtr,const KeyCode& theKeyCode)
{
    pTheActiveControl=pSvXPCtr;
    aKeyCode=theKeyCode;
    aKeyInputLink.Call(this);
}

SvXPropertyControl *    SvXPropEvListener::GetPropertyControl()
{
    return pTheActiveControl;
}

KeyCode SvXPropEvListener::GetKeyCode() const
{
    return aKeyCode;
}

//------------------------------------------------------------------

SvListBoxForProperties::SvListBoxForProperties( Window* pParent, WinBits nWinStyle)
        :   Control(pParent,nWinStyle),
            aVScroll(this,WB_VSCROLL|WB_REPEAT|WB_DRAG),
            aPlayGround(this,WB_DIALOGCONTROL),
            pPropDataControl(NULL)
{

    aListener.SetModifyHdl   (LINK( this, SvListBoxForProperties,   ModifyHdl));
    aListener.SetGetFocusHdl (LINK( this, SvListBoxForProperties, GetFocusHdl));
    aListener.SetLoseFocusHdl(LINK( this, SvListBoxForProperties,LoseFocusHdl));
    aListener.SetKeyInputHdl (LINK( this, SvListBoxForProperties, KeyInputHdl));

    nYOffset=0;
    nTheNameSize=0;
    ListBox aListBox(this,WB_DROPDOWN);
    aListBox.SetPosSizePixel(Point(0,0),Size(100,100));
    nRowHeight=(USHORT)aListBox.GetSizePixel().Height();
    Wallpaper aWall = aPlayGround.GetBackground();
    aWall.SetColor( Color( COL_TRANSPARENT ) );
    aPlayGround.SetBackground( aWall );
    aPlayGround.Show();
    aVScroll.Hide();
    aVScroll.SetScrollHdl(
        LINK( this, SvListBoxForProperties, ScrollHdl ));

}


SvListBoxForProperties::SvListBoxForProperties( Window* pParent, const ResId& rResId )
            :       Control(pParent,rResId),
                    aVScroll(this,WB_VSCROLL|WB_REPEAT|WB_DRAG),
                    aPlayGround(this,0),
                    pPropDataControl(NULL)
{
    nTheNameSize=0;
    nYOffset=0;
    ListBox aListBox(this,WB_DROPDOWN);
    aListBox.SetPosSizePixel(Point(0,0),Size(100,100));
    nRowHeight=(USHORT)aListBox.GetSizePixel().Height();
    Wallpaper aWall = aPlayGround.GetBackground();
    aWall.SetColor( Color( COL_TRANSPARENT ) );
    aPlayGround.SetBackground( aWall );
    aPlayGround.Show();
    aVScroll.Hide();
    aVScroll.SetScrollHdl( LINK( this, SvListBoxForProperties, ScrollHdl ) );
    UpdateVScroll();
    Resize();
}

SvListBoxForProperties::~SvListBoxForProperties()
{
    Clear();
}

void SvListBoxForProperties::Clear()
{
    for(USHORT i=0;i<PLineArray.Count();i++)
    {
        SvPropertyLine* pPropLine=PLineArray[i];

        switch(pPropLine->GetKindOfControl())
        {
            case    KOC_LISTBOX:
            case    KOC_COMBOBOX:
            case    KOC_EDIT:       delete pPropLine->GetSvXPropertyControl();
                                    break;
        }
        delete pPropLine;
    }
    PLineArray.Remove(0,PLineArray.Count());
}


void SvListBoxForProperties::Resize()
{
    Size aSize=GetOutputSizePixel();
    Size a2Size=aSize;
    Size aVScrollSize;

    if(aVScroll.IsVisible())
    {
        Point aPos(0,0);
        aVScrollSize=aVScroll.GetSizePixel();
        aVScrollSize.Height()=aSize.Height();
        a2Size.Width()-=aVScrollSize.Width();
        aPos.X()=a2Size.Width();
        aVScroll.SetPosPixel(aPos);
        aVScroll.SetSizePixel(aVScrollSize);
    }

    aPlayGround.SetPosPixel(Point(0,0));
    aPlayGround.SetSizePixel(a2Size);
    UpdatePosNSize();
}

void SvListBoxForProperties::SetController( SvPropertyDataControl *pPDC)
{
    pPropDataControl=pPDC;
}

USHORT SvListBoxForProperties::CalcVisibleLines()
{
    Size aSize=aPlayGround.GetOutputSizePixel();
    USHORT nResult=0;
    if(nRowHeight!=0)
        nResult=(USHORT) aSize.Height()/nRowHeight;

    return nResult;
}

void SvListBoxForProperties::UpdateVScroll()
{
    USHORT nLines=CalcVisibleLines();
    aVScroll.SetPageSize(nLines-1);
    aVScroll.SetVisibleSize(nLines-1);
    aVScroll.SetRange(Range(0,PLineArray.Count()-1));
    if(PLineArray.Count()<=nLines)
    {
        aVScroll.Hide();
    }
    else
    {
        BOOL bFlag=aVScroll.IsVisible();
        aVScroll.Show();
        if(!bFlag)Resize();
    }


}

void SvListBoxForProperties::UpdatePosNSize()
{
    Point aPos(0,nYOffset);

    for(USHORT i=0; i<PLineArray.Count();i++)
    {
        if((PLineArray[i])->NeedsRepaint())
        {
            (PLineArray[i])->SetPosPixel(aPos);
            Size aSize=aPlayGround.GetOutputSizePixel();
            aSize.Height()=nRowHeight;
            (PLineArray[i])->SetSizePixel(aSize);
            (PLineArray[i])->SetNameWidth(nTheNameSize+2*FRAME_OFFSET);
            (PLineArray[i])->Invalidate();
            (PLineArray[i])->Update();
            (PLineArray[i])->Show();
            (PLineArray[i])->SetNeedsRepaint(FALSE);
        }
        else
        {
            if((PLineArray[i])->IsVisible())
            {
                Size aSize=aPlayGround.GetOutputSizePixel();
                aSize.Height()=nRowHeight;
                (PLineArray[i])->SetSizePixel(aSize);
                (PLineArray[i])->SetNameWidth(nTheNameSize+2*FRAME_OFFSET);
                (PLineArray[i])->Invalidate();
            }
        }

        aPos.Y()+=nRowHeight;
    }
    aPlayGround.Invalidate();
    aPlayGround.Update();
}

void SvListBoxForProperties::UpdatePlayGround()
{
    Point aPos(0,0);
    long nThumbPos=aVScroll.GetThumbPos();
    long nLines=aVScroll.GetPageSize();
    long nDelta=aVScroll.GetDelta();

    USHORT nStart,nEnd;
    Size aSize=aPlayGround.GetOutputSizePixel();
    Point aPEnd;
    aPEnd.X()=aSize.Width();

    if(nDelta>0)
    {
        nStart=(USHORT)(nThumbPos+nLines+1-nDelta);
        nEnd=(USHORT)(nThumbPos+nLines);
        aPos.Y()=(nLines+1-nDelta)*nRowHeight;
    }
    else
    {
        nStart=(USHORT)nThumbPos;
        nEnd=(USHORT)(nThumbPos-nDelta);
        aPEnd.Y()=(nThumbPos-nDelta)*nRowHeight;;
    }

    aSize.Height()=nRowHeight;

    nDelta=-nDelta*nRowHeight;

    aPlayGround.Scroll(0,nDelta,SCROLL_CHILDREN);

    for(USHORT i=nStart; i<=nEnd;i++)
    {
        (PLineArray[i])->SetPosSizePixel(aPos,aSize);
        (PLineArray[i])->SetNameWidth(nTheNameSize+2*FRAME_OFFSET);
        (PLineArray[i])->Show();
        aPos.Y()+=nRowHeight;
    }
    aPlayGround.Update();
}

void SvListBoxForProperties::UpdateAll()
{
    UpdatePosNSize();
    UpdatePlayGround();
    //UpdateVScroll();
}

void SvListBoxForProperties::DisableUpdate()
{
    bUpdate=FALSE;
}

void SvListBoxForProperties::EnableUpdate()
{
    bUpdate=TRUE;
    UpdateAll();
}

void SvListBoxForProperties::SetPropertyValue( const String & rEntryName, const String & rValue )
{
    USHORT i, iEnd = PLineArray.Count();
    for( i = 0 ; i < iEnd ; i++ )
    {
        SvPropertyLine* pPropLine = PLineArray[ i ];
        SvXPropertyControl* pSvXPCtr=pPropLine->GetSvXPropertyControl();
        if( pSvXPCtr && pSvXPCtr->GetMyName() == rEntryName )
        {
            pSvXPCtr->SetProperty( rValue );
        }
    }
}

USHORT SvListBoxForProperties::AppendEntry( const SvPropertyData& aPropData)
{
    return InsertEntry(aPropData);
}

USHORT SvListBoxForProperties::InsertEntry( const SvPropertyData& aPropData, USHORT nPos)
{
    USHORT nInsPos=nPos;
    SvPropertyLine* pPropLine=new SvPropertyLine(&aPlayGround,WB_TABSTOP | WB_DIALOGCONTROL);

    long nThumbPos=aVScroll.GetThumbPos();
    long nLines=aVScroll.GetPageSize();

    if(nPos==LISTBOX_APPEND)
    {
        nInsPos=PLineArray.Count();
        PLineArray.Insert(pPropLine,nInsPos);
    }
    else
    {
        PLineArray.Insert(pPropLine,nPos);
    }
    pPropLine->SetNameWidth(nTheNameSize);
    UpdateVScroll();
    UpdatePosNSize();
    ChangeEntry(aPropData,nInsPos);
    return nInsPos;
}

void SvListBoxForProperties::ChangeEntry( const SvPropertyData& aPropData, USHORT nPos)
{
    if(nPos<PLineArray.Count())
    {
        SvPropertyLine* pPropLine=PLineArray[nPos];

        switch(pPropLine->GetKindOfControl())
        {
            case    KOC_LISTBOX:
            case    KOC_COMBOBOX:
            case    KOC_EDIT:       delete pPropLine->GetSvXPropertyControl();
                                    break;
        }

        switch(aPropData.eKind)
        {
            case    KOC_LISTBOX:
                                    pPropLine->SetSvXPropertyControl(
                                            new SvXPropertyComboBox(pPropLine,WB_TABSTOP));
                                    pPropLine->SetKindOfControl(aPropData.eKind);
                                    break;
            case    KOC_COMBOBOX:
                                    pPropLine->SetSvXPropertyControl(
                                            new SvXPropertyComboBox(pPropLine,WB_TABSTOP));
                                    pPropLine->SetKindOfControl(aPropData.eKind);
                                    break;
            case    KOC_EDIT:
                                    pPropLine->SetSvXPropertyControl(
                                            new SvXPropertyEdit(pPropLine,WB_TABSTOP));
                                    pPropLine->SetKindOfControl(aPropData.eKind);
                                    break;
            case    KOC_USERDEFINED:
                                    pPropLine->SetSvXPropertyControl(aPropData.pControl);
                                    aPropData.pControl->SetParent(pPropLine);
                                    pPropLine->SetKindOfControl(aPropData.eKind);
                                    break;
            default:
                                    pPropLine->SetSvXPropertyControl(NULL);
                                    pPropLine->SetKindOfControl(KOC_UNDEFINED);
                                    break;
        }

        SvXPropertyControl* pSvXPCtr=pPropLine->GetSvXPropertyControl();

        if(pSvXPCtr!=NULL)
        {
            pSvXPCtr->SetSvXPropertyCtrListener(&aListener);
            pSvXPCtr->SetProperty(aPropData.aValue);
            pSvXPCtr->SetMyData(aPropData.pDataPtr);
            pSvXPCtr->SetMyName(aPropData.aName);

            if(pSvXPCtr->HasList())
            {
                for(int i=0;i<aPropData.theValues.Count();i++)
                {
                    pSvXPCtr->InsertEntry(*(aPropData.theValues[i]));
                }
            }
        }

        pPropLine->SetName(aPropData.aName);

        USHORT nTextWidth=(USHORT)aPlayGround.GetTextWidth(aPropData.aName);

        if ( nTheNameSize < nTextWidth )
            nTheNameSize = nTextWidth;

        if ( aPropData.bHasVisibleXButton )
        {
            pPropLine->SetClickHdl(LINK( this, SvListBoxForProperties, ClickHdl ) );
            pPropLine->ShowXButton();
        }
        else
            pPropLine->HideXButton();

        pPropLine->Locked(aPropData.bIsLocked);

        pPropLine->ShowAsHyperLink(aPropData.bIsHyperLink);
        pPropLine->SetData(aPropData.pDataPtr);
    }
}

USHORT SvListBoxForProperties::GetFirstVisibleEntry()
{
    return 0;
}
void SvListBoxForProperties::SetFirstVisibleEntry(USHORT nPos)
{
    return;
}

void SvListBoxForProperties::SetSelectedEntry(USHORT nPos)
{
    return;
}

USHORT SvListBoxForProperties::GetSelectedEntry()
{
    return 0;
}

IMPL_LINK( SvListBoxForProperties, ScrollHdl, ScrollBar*, pSB )
{
    if(pSB!=NULL)
    {
        long nDelta=aVScroll.GetDelta();
        nYOffset=-aVScroll.GetThumbPos()*nRowHeight;
        //aPlayGround.SetUpdateMode(FALSE);

        long nThumbPos=aVScroll.GetThumbPos();
        long nLines=aVScroll.GetPageSize();

        UpdatePlayGround();

        for(int i=nThumbPos-nDelta; i<nThumbPos+nLines-nDelta;i++)
        {
            if(i>=nThumbPos && i<=nThumbPos+nLines)
            {
                (PLineArray[i])->SetNeedsRepaint(TRUE);
            }
            else
            {
                (PLineArray[i])->Hide();
                (PLineArray[i])->SetNeedsRepaint(FALSE);
            }
        }
    }
    return 0;
}

IMPL_LINK( SvListBoxForProperties, ClickHdl, PushButton*,pPB)
{
    if(pPB!=NULL)
    {
       SvPropertyLine *pPropLine=(SvPropertyLine *)pPB->GetParent();
       SvXPropertyControl* pSvXPCtr=pPropLine->GetSvXPropertyControl();
       pPropDataControl->Clicked(pSvXPCtr->GetMyName(),
                            pSvXPCtr->GetProperty(),pSvXPCtr->GetMyData());
    }
    return 0;
}

IMPL_LINK( SvListBoxForProperties, ModifyHdl,SvXPropEvListener*, pSvXPEvL)
{
    if(pSvXPEvL!=NULL && pPropDataControl!=NULL)
    {

        SvXPropertyControl* pSvXPCtr=aListener.GetPropertyControl();

        pPropDataControl->Modified(pSvXPCtr->GetMyName(),
                            pSvXPCtr->GetProperty(),pSvXPCtr->GetMyData());

    }
    return 0;
}

IMPL_LINK( SvListBoxForProperties, GetFocusHdl,SvXPropEvListener*, pSvXPEvL)
{
    if(pSvXPEvL!=NULL && pPropDataControl!=NULL)
    {
        SvXPropertyControl* pSvXPCtr=aListener.GetPropertyControl();

        pPropDataControl->Select(pSvXPCtr->GetMyName(),pSvXPCtr->GetMyData());
    }
    return 0;
}

IMPL_LINK( SvListBoxForProperties, LoseFocusHdl,SvXPropEvListener*, pSvXPEvL)
{
    if(pSvXPEvL!=NULL && pPropDataControl!=NULL)
    {
        SvXPropertyControl* pSvXPCtr=aListener.GetPropertyControl();

        pPropDataControl->Commit(pSvXPCtr->GetMyName(),
                    pSvXPCtr->GetProperty(),pSvXPCtr->GetMyData());
        /*
        {
            pSvXPCtr->SetProperty(
                pPropDataControl->GetTheCorrectProperty());
        }
        */
    }
    return 0;
}

IMPL_LINK( SvListBoxForProperties, KeyInputHdl,SvXPropEvListener*, pSvXPEvL)
{
    if(pSvXPEvL!=NULL && pPropDataControl!=NULL)
    {
        SvXPropertyControl* pSvXPCtr=aListener.GetPropertyControl();
    }
    return 0;
}




SvTabPageForProperties::SvTabPageForProperties(Window* pParent,WinBits nWinStyle)
            :   TabPage(pParent,nWinStyle),
                aLbProp(this)
{
    aLbProp.Show();
}

void SvTabPageForProperties::Resize()
{
    Point aPos(3,3);
    Size aSize=GetOutputSizePixel();
    aSize.Width()-=6;
    aSize.Height()-=6;

    aLbProp.SetPosSizePixel(aPos,aSize);
}

SvListBoxForProperties* SvTabPageForProperties::GetTheListBox()
{
    return &aLbProp;
}


SvPropertyBox::SvPropertyBox( Window* pParent, WinBits nWinStyle)
            : Control(pParent,nWinStyle),
              aTabControl(this)
{
    aTabControl.Show();
}

SvPropertyBox::SvPropertyBox( Window* pParent, const ResId& rResId )
            : Control(pParent,rResId),
              aTabControl(this)
{
    aTabControl.Show();
    Resize();
}

SvPropertyBox::~SvPropertyBox()
{
    ClearAll();
}


void SvPropertyBox::ClearAll()
{
    USHORT nCount=aTabControl.GetPageCount();

    for(USHORT i=nCount;i>=1;i--)
    {
        SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                                    aTabControl.GetTabPage(i);
        aTabControl.RemovePage(i);
        delete pPage;
    }
}


void SvPropertyBox::Resize()
{
    Point aPos(3,3);
    Size aSize=GetOutputSizePixel();
    aSize.Width()-=6;
    aSize.Height()-=6;

    aTabControl.SetPosSizePixel(aPos,aSize);

    USHORT nCount=aTabControl.GetPageCount();

    aSize=aTabControl.GetOutputSizePixel();
    for(USHORT i=1;i<=nCount;i++)
    {
        SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                                    aTabControl.GetTabPage(i);
        pPage->SetSizePixel(aSize);
    }

}


USHORT SvPropertyBox::AppendPage( const String & rText )
{
    USHORT nId=aTabControl.GetPageCount()+1;
    aTabControl.InsertPage( nId,rText);
    SvTabPageForProperties* pPage=new SvTabPageForProperties(&aTabControl);
    pPage->SetSizePixel(aTabControl.GetOutputSizePixel());
    pPage->GetTheListBox()->SetController(pThePropDataCtr);
    aTabControl.SetTabPage( nId, pPage);
    aTabControl.SetCurPageId(nId);
    return nId;
}

void SvPropertyBox::SetPage( USHORT nId)
{
    aTabControl.SetCurPageId(nId);
}

USHORT SvPropertyBox::GetCurPage()
{
    return aTabControl.GetCurPageId();
}

USHORT SvPropertyBox::CalcVisibleLines()
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    return pPage->GetTheListBox()->CalcVisibleLines();
}
void SvPropertyBox::EnableUpdate()
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    pPage->GetTheListBox()->EnableUpdate();
}

void SvPropertyBox::DisableUpdate()
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    pPage->GetTheListBox()->DisableUpdate();
}

void SvPropertyBox::SetController(SvPropertyDataControl *pDataCtr)
{
    pThePropDataCtr=pDataCtr;
    USHORT nCount=aTabControl.GetPageCount();

    for(USHORT i=1;i<=nCount;i++)
    {
        SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                                    aTabControl.GetTabPage(i);

        pPage->GetTheListBox()->SetController(pThePropDataCtr);
    }

}

USHORT SvPropertyBox::InsertEntry( const SvPropertyData& rData, USHORT nPos)
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    return pPage->GetTheListBox()->InsertEntry(rData,nPos);
}

void SvPropertyBox::ChangeEntry( const SvPropertyData& rData, USHORT nPos)
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    pPage->GetTheListBox()->ChangeEntry(rData,nPos);
}

USHORT SvPropertyBox::AppendEntry( const SvPropertyData& rData)
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    return pPage->GetTheListBox()->AppendEntry(rData);
}

void SvPropertyBox::SetPropertyValue( const String & rEntryName, const String & rValue )
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    pPage->GetTheListBox()->SetPropertyValue( rEntryName, rValue );
}

void SvPropertyBox::SetFirstVisibleEntry(USHORT nPos)
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    pPage->GetTheListBox()->SetFirstVisibleEntry(nPos);
}
USHORT SvPropertyBox::GetFirstVisibleEntry()
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    return pPage->GetTheListBox()->GetFirstVisibleEntry();
}

void SvPropertyBox::SetSelectedEntry(USHORT nPos)
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    pPage->GetTheListBox()->SetSelectedEntry(nPos);
}
USHORT SvPropertyBox::GetSelectedEntry()
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    return pPage->GetTheListBox()->GetSelectedEntry();
}

void SvPropertyBox::ClearTable()
{
    SvTabPageForProperties* pPage=(SvTabPageForProperties*)
                aTabControl.GetTabPage(aTabControl.GetCurPageId());
    pPage->GetTheListBox()->Clear();
}


void SvBasicPropertyDataControl::Modified(const String& aName,
                                const String& aVal,void* pData)
{
    aEntryName=aName;
    aEntryProperty=aVal;
    pTheData=pData;
    aModifyLink.Call(this);
}

void SvBasicPropertyDataControl::Clicked(   const String& aName,
                                    const String& aVal,
                                    void* pData)        //Xtension-Button pressed
{
    aEntryName=aName;
    aEntryProperty=aVal;
    pTheData=pData;
    aClickedLink.Call(this);
}

void SvBasicPropertyDataControl::Commit(        const String& aName,
                                    const String& aVal,
                                    void* pData)        //User accept changes
{
    aEntryName=aName;
    aEntryProperty=aVal;
    pTheData=pData;
    aCommitLink.Call(this);
}

void SvBasicPropertyDataControl::Select(        const String& aName,
                                    void* pData)        //User select new Row
{
    aEntryName=aName;
    pTheData=pData;
    aSelectLink.Call(this);
}

void SvBasicPropertyDataControl::LinkClicked(const String& aName, void* pData)
{
}


String  SvBasicPropertyDataControl::GetName() const //Tell's the name of the Property
{
    return aEntryName;
}

String  SvBasicPropertyDataControl::GetProperty() const //Tell's the content of the Property
{
    return aEntryProperty;
}

void*   SvBasicPropertyDataControl::GetData() //Tell's the storage
{
    return pTheData;
}

/*
String SvBasicPropertyDataControl::GetTheCorrectProperty() const
{
    return aCorrectProperty;
}
*/

void SvBasicPropertyDataControl::SetTheCorrectProperty(const String& aString)
{
    aCorrectProperty=aString;
}

void SvBasicPropertyDataControl::SetIsCorrect(BOOL nFlag)
{
    bCorrectness=nFlag;
}



//========================================================================
// Property-Dialog:
/* zum TESTEN im CALC*/
//========================================================================
/*
ScPropertyDlg::ScPropertyDlg( Window* pParent) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_PROPERTIES) ),
    aPropListBox    ( this, ResId( CTR_PROPLINE) ),
    aKindOfListBox  (this,ResId( LB_KIND_OF_PROP)),
    aModAnswer      (this,ResId(FT_ANSMOD   )),
    aClickAnswer    (this,ResId(FT_ANSCLICK )),
    aCommitAnswer   (this,ResId(FT_ANSCOMMIT)),
    aSelectAnswer   (this,ResId(FT_ANSSELECT)),
    anOk(this,ResId( BTN_OK)),
    aCancel(this,ResId( BTN_CANCEL))
{
    FreeResource();

    aKindOfListBox.SelectEntryPos(0);
    aKindOfListBox.SetSelectHdl(
        LINK( this, ScPropertyDlg, ModifiedHdl ));
    nCount=0;
    nClickCount=0;

    String aString("Don't know ");

    aBaProDatCtr.SetModifyHdl (LINK( this, ScPropertyDlg, RowModifiedHdl ));
    aBaProDatCtr.SetClickedHdl(LINK( this, ScPropertyDlg, ClickHdl       ));
    aBaProDatCtr.SetCommitHdl (LINK( this, ScPropertyDlg, SelectHdl      ));
    aBaProDatCtr.SetSelectHdl (LINK( this, ScPropertyDlg, CommitHdl      ));

    aPropListBox.SetController(&aBaProDatCtr);

    USHORT nPageId=aPropListBox.AppendPage("YabbaDabbaDo");
    aPropListBox.SetPage(nPageId);
    aProperty.eKind=KOC_EDIT;
    aProperty.aName=aString;
    aProperty.aName+=String((USHORT)++nCount);
    aProperty.aValue=sizeof ComboBox;
    aProperty.bHasVisibleXButton=FALSE;
    aProperty.bIsHyperLink=FALSE;
    aProperty.bIsLocked=FALSE;
    aProperty.pDataPtr=NULL;
    aProperty.pControl=NULL;
    aProperty.theValues.Insert(new String("1"),aProperty.theValues.Count());
    aProperty.theValues.Insert(new String("2"),aProperty.theValues.Count());
    aProperty.theValues.Insert(new String("3"),aProperty.theValues.Count());
    aProperty.theValues.Insert(new String("4"),aProperty.theValues.Count());
    aPropListBox.InsertEntry(aProperty);
    aProperty.bHasVisibleXButton=TRUE;
    aProperty.aName=aString;
    aProperty.aName+=String((USHORT)++nCount);
    aProperty.aValue="42";
    aProperty.eKind=KOC_LISTBOX;
    aPropListBox.InsertEntry(aProperty);
    aProperty.aName=aString;
    aProperty.aName+=String((USHORT)++nCount);
    aProperty.eKind=KOC_COMBOBOX;
    aProperty.bHasVisibleXButton=FALSE;
    aPropListBox.InsertEntry(aProperty);
}

ScPropertyDlg::~ScPropertyDlg()
{
    delete aProperty.theValues[0];
    delete aProperty.theValues[1];
    delete aProperty.theValues[2];
    delete aProperty.theValues[3];
}

IMPL_LINK( ScPropertyDlg, ModifiedHdl, ListBox*, pLB )
{
    if(pLB!=NULL)
    {
        String aString("Don't know ");
        aProperty.aName=aString;
        aProperty.aName+=String((USHORT)++nCount);
        if(nCount>20)
        {
            String aStr("Yabba ");
            aStr+=aPropListBox.GetCurPage();
            USHORT nPageId=aPropListBox.AppendPage(aStr);
            aPropListBox.SetPage(nPageId);
            nCount=0;
        }

        aProperty.eKind=(eKindOfControl)(aKindOfListBox.GetSelectEntryPos()+1);
        aProperty.bHasVisibleXButton=FALSE;
        if((nCount % 5)==0) aProperty.bHasVisibleXButton=TRUE;
        aPropListBox.InsertEntry(aProperty);
    }
    return 0;
}

IMPL_LINK( ScPropertyDlg, RowModifiedHdl, SvBasicPropertyDataControl* ,pProCtr)
{
    if(pProCtr!=NULL)
    {
        aModAnswer.SetText(aBaProDatCtr.GetProperty());
        aModAnswer.Invalidate();
        aBaProDatCtr.SetIsCorrect(TRUE);
    }
    return 0;
}

IMPL_LINK( ScPropertyDlg, CommitHdl, SvBasicPropertyDataControl*,pProCtr)
{
    if(pProCtr!=NULL)
    {
        aCommitAnswer.SetText(aBaProDatCtr.GetProperty());
        aCommitAnswer.Invalidate();
        aBaProDatCtr.SetIsCorrect(TRUE);
    }
    return 0;
}

IMPL_LINK( ScPropertyDlg, ClickHdl, SvBasicPropertyDataControl*,pProCtr)
{
    if(pProCtr!=NULL)
    {
        aClickAnswer.SetText(aBaProDatCtr.GetName());
        aClickAnswer.Invalidate();
        aBaProDatCtr.SetIsCorrect(TRUE);
    }
    return 0;
}

IMPL_LINK( ScPropertyDlg, SelectHdl, SvBasicPropertyDataControl*,pProCtr)
{
    if(pProCtr!=NULL)
    {
        aSelectAnswer.SetText(aBaProDatCtr.GetName());
        aSelectAnswer.Invalidate();
        aBaProDatCtr.SetIsCorrect(TRUE);
    }
    return 0;
}
*/


