/*************************************************************************
 *
 *  $RCSfile: svdibrow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-02 16:33:59 $
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

#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif

#include "svdibrow.hxx"

#ifndef _STDLIB_H
#include <stdlib.h>
#define _STDLIB_H
#endif

#include "svditext.hxx"
#include "flditem.hxx"
#include "editdata.hxx"
#include "svdpool.hxx"
#include "svdattr.hxx"
#include "svdattrx.hxx"
#include "svdview.hxx"

#ifndef _XENUM_HXX //autogen
#include "xenum.hxx"
#endif

#ifndef _SVX_XLINEIT0_HXX //autogen
#include "xlineit0.hxx"
#endif

#ifndef _SVX_XLNSTWIT_HXX //autogen
#include "xlnstwit.hxx"
#endif

#ifndef _SVX_XLNEDWIT_HXX //autogen
#include "xlnedwit.hxx"
#endif

#ifndef SVX_XFILLIT0_HXX //autogen
#include "xfillit0.hxx"
#endif

#ifndef _SVX_XFLBMTIT_HXX //autogen
#include "xflbmtit.hxx"
#endif

#ifndef _SVX_TEXTIT0_HXX //autogen
#include "xtextit0.hxx"
#endif

#ifndef _SVX_XFLBSTIT_HXX //autogen
#include "xflbstit.hxx"
#endif

#ifndef _SVX_XFLBTOXY_HXX //autogen
#include "xflbtoxy.hxx"
#endif

#ifndef _SVX_XFTSHIT_HXX //autogen
#include "xftshit.hxx"
#endif

#ifndef _SVX_COLRITEM_HXX //autogen
#include "colritem.hxx"
#endif

#ifndef _EEITEMID_HXX
#include <eeitemid.hxx>
#endif

#ifndef _SVX_FONTITEM_HXX //autogen
#include "fontitem.hxx"
#endif

#ifndef _SVX_FHGTITEM_HXX //autogen
#include "fhgtitem.hxx"
#endif

#include <charscaleitem.hxx>

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _SFXFLAGITEM_HXX //autogen
#include <svtools/flagitem.hxx>
#endif

#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif

#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif

#ifndef _SFXRNGITEM_HXX
#include <svtools/rngitem.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#define ITEMBROWSER_WHICHCOL_ID 1
#define ITEMBROWSER_STATECOL_ID 2
#define ITEMBROWSER_TYPECOL_ID  3
#define ITEMBROWSER_NAMECOL_ID  4
#define ITEMBROWSER_VALUECOL_ID 5

enum ItemType {
    ITEM_DONTKNOW, ITEM_BYTE, ITEM_INT16, ITEM_UINT16, ITEM_INT32, ITEM_UINT32,
    ITEM_ENUM, ITEM_BOOL, ITEM_FLAG, ITEM_STRING, ITEM_POINT, ITEM_RECT, ITEM_RANGE, ITEM_LRANGE,
    ITEM_FRACTION,
    ITEM_XCOLOR,
    ITEM_COLOR,
    ITEM_FONT, ITEM_FONTHEIGHT, ITEM_FONTWIDTH, ITEM_FIELD
};

class ImpItemListRow
{
public:
    XubString                   aName;
    XubString                   aValue;
    SfxItemState                eState;
    UINT16                      nWhichId;

    TypeId                      pType;
    ItemType                    eItemType;

    INT32                       nVal;
    INT32                       nMin;
    INT32                       nMax;

    BOOL                        bComment;
    BOOL                        bIsNum;
    BOOL                        bCanNum;

public:
    ImpItemListRow()
    :   eState(SFX_ITEM_UNKNOWN),
        pType(NULL),
        eItemType(ITEM_DONTKNOW),
        nWhichId(0),
        bComment(FALSE),
        bIsNum(FALSE),
        bCanNum(FALSE),
        nVal(0),
        nMin(0),
        nMax(0)
    {}

    XubString GetItemTypeStr() const;
    BOOL operator==(const ImpItemListRow& rEntry) const;
    BOOL operator!=(const ImpItemListRow& rEntry) const { return !operator==(rEntry); }
};

XubString ImpItemListRow::GetItemTypeStr() const
{
    XubString aStr;

    switch(eItemType)
    {
        case ITEM_BYTE      : aStr.AppendAscii("Byte");     break;
        case ITEM_INT16     : aStr.AppendAscii("Int16");    break;
        case ITEM_UINT16    : aStr.AppendAscii("UInt16");   break;
        case ITEM_INT32     : aStr.AppendAscii("Int32");    break;
        case ITEM_UINT32    : aStr.AppendAscii("UInt32");   break;
        case ITEM_ENUM      : aStr.AppendAscii("Enum");     break;
        case ITEM_BOOL      : aStr.AppendAscii("Bool");     break;
        case ITEM_FLAG      : aStr.AppendAscii("Flag");     break;
        case ITEM_STRING    : aStr.AppendAscii("String");   break;
        case ITEM_POINT     : aStr.AppendAscii("Point");    break;
        case ITEM_RECT      : aStr.AppendAscii("Rectangle");break;
        case ITEM_RANGE     : aStr.AppendAscii("Range");    break;
        case ITEM_LRANGE    : aStr.AppendAscii("LRange");   break;
        case ITEM_FRACTION  : aStr.AppendAscii("Fraction"); break;
        case ITEM_XCOLOR    : aStr.AppendAscii("XColor");   break;
        case ITEM_COLOR     : aStr.AppendAscii("Color");    break;
        case ITEM_FONT      : aStr.AppendAscii("Font");     break;
        case ITEM_FONTHEIGHT:aStr.AppendAscii("FontHeight");break;
        case ITEM_FONTWIDTH :aStr.AppendAscii("FontWidth"); break;
        case ITEM_FIELD     :aStr.AppendAscii("Field");     break;
    }

    return aStr;
}

BOOL ImpItemListRow::operator==(const ImpItemListRow& rEntry) const
{
    return (aName.Equals(rEntry.aName)
        && aValue.Equals(rEntry.aValue)
        && eState==rEntry.eState
        && nWhichId==rEntry.nWhichId
        && bComment==rEntry.bComment
        && bIsNum==rEntry.bIsNum
        && bCanNum==rEntry.bCanNum
        && pType==rEntry.pType
        && eItemType==rEntry.eItemType
        && nVal==rEntry.nVal
        && nMin==rEntry.nMin
        && nMax==rEntry.nMax);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpItemEdit: public Edit
{
    _SdrItemBrowserControl*     pBrowse;

public:
    ImpItemEdit(Window* pParent, _SdrItemBrowserControl* pBrowse_, WinBits nBits=0)
    :   Edit(pParent, nBits),
        pBrowse(pBrowse_)
    {}

    virtual ~ImpItemEdit();
    virtual void KeyInput(const KeyEvent& rEvt);
};

__EXPORT ImpItemEdit::~ImpItemEdit()
{
}

void __EXPORT ImpItemEdit::KeyInput(const KeyEvent& rKEvt)
{
    _SdrItemBrowserControl* pBrowseMerk = pBrowse;

    UINT16 nKeyCode(rKEvt.GetKeyCode().GetCode() + rKEvt.GetKeyCode().GetModifier());

    if(nKeyCode == KEY_RETURN)
    {
        pBrowseMerk->EndChangeEntry();
        pBrowseMerk->GrabFocus();
    }
    else if(nKeyCode == KEY_ESCAPE)
    {
        pBrowseMerk->BrkChangeEntry();
        pBrowseMerk->GrabFocus();
    }
    else if(nKeyCode == KEY_UP || nKeyCode == KEY_DOWN)
    {
        pBrowseMerk->EndChangeEntry();
        pBrowseMerk->GrabFocus();
        pBrowseMerk->KeyInput(rKEvt);
    }
    else
        Edit::KeyInput(rKEvt);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#define MYBROWSEMODE (BROWSER_THUMBDRAGGING|BROWSER_KEEPHIGHLIGHT|BROWSER_NO_HSCROLL|BROWSER_HIDECURSOR)

_SdrItemBrowserControl::_SdrItemBrowserControl(Window* pParent, WinBits nBits):
    BrowseBox(pParent,nBits,MYBROWSEMODE),
    aList(1024,16,16)
{
    ImpCtor();
}

__EXPORT _SdrItemBrowserControl::~_SdrItemBrowserControl()
{
    if(pEditControl)
        delete pEditControl;

    if(pAktChangeEntry)
        delete pAktChangeEntry;

    Clear();
}

void _SdrItemBrowserControl::ImpCtor()
{
    pEditControl = NULL;
    pAktChangeEntry = NULL;
    nLastWhichOfs = 0;
    nLastWhich = 0;
    nLastWhichOben = 0;  // not implemented yet
    nLastWhichUnten = 0; // not implemented yet
    bWhichesButNames = FALSE;
    bDontHideIneffectiveItems = FALSE;
    bDontSortItems = FALSE;
    bShowWhichIds = FALSE;
    bShowRealValues = FALSE;
    bShowWhichIds = TRUE;   // not implemented yet
    bShowRealValues = TRUE; // not implemented yet

    rtl_TextEncoding aTextEncoding = gsl_getSystemTextEncoding();

    InsertDataColumn(
        ITEMBROWSER_WHICHCOL_ID,
        String("Which", aTextEncoding),
        GetTextWidth(String(" Which ", aTextEncoding)) + 2);
    InsertDataColumn(
        ITEMBROWSER_STATECOL_ID,
        String("State", aTextEncoding),
        Max(GetTextWidth(String(" State ", aTextEncoding)) + 2 ,
            GetTextWidth(String("DontCare", aTextEncoding)) + 2));
    InsertDataColumn(
        ITEMBROWSER_TYPECOL_ID ,
        String("Type", aTextEncoding),
        GetTextWidth(String(" Type_ ", aTextEncoding)) + 2);
    InsertDataColumn(
        ITEMBROWSER_NAMECOL_ID ,
        String("Name", aTextEncoding),
        150);
    InsertDataColumn(
        ITEMBROWSER_VALUECOL_ID,
        String("Value", aTextEncoding),
        GetTextWidth(String("12345678901234567890", aTextEncoding)));
    SetDataRowHeight(
        GetTextHeight());

    long nWdt=GetColumnWidth(ITEMBROWSER_WHICHCOL_ID)+
              GetColumnWidth(ITEMBROWSER_STATECOL_ID)+
              GetColumnWidth(ITEMBROWSER_TYPECOL_ID )+
              GetColumnWidth(ITEMBROWSER_NAMECOL_ID )+
              GetColumnWidth(ITEMBROWSER_VALUECOL_ID);

    long nHgt=GetTitleHeight()+16*GetDataRowHeight();

    SetOutputSizePixel(Size(nWdt,nHgt));
}

void _SdrItemBrowserControl::Clear()
{
    ULONG nAnz=aList.Count();
    for (ULONG nNum=0; nNum<nAnz; nNum++) {
        delete ImpGetEntry(nNum);
    }
    aList.Clear();
    BrowseBox::Clear();
}

long __EXPORT _SdrItemBrowserControl::GetRowCount() const
{
    return aList.Count();
}

BOOL __EXPORT _SdrItemBrowserControl::SeekRow(long nRow)
{
    nAktPaintRow=nRow;
    return TRUE;
}

void __EXPORT _SdrItemBrowserControl::PaintField(OutputDevice& rDev, const Rectangle& rRect, USHORT nColumnId) const
{
    if (nAktPaintRow<0 || (ULONG)nAktPaintRow>=aList.Count()) {
        return;
    }
    Rectangle aR(rRect);
    aR.Bottom()++;
    ImpItemListRow* pEntry=ImpGetEntry(nAktPaintRow);
    if (pEntry->bComment)
    {
        if (nColumnId==ITEMBROWSER_NAMECOL_ID)
        {
            rDev.SetLineColor();
            rDev.SetFillColor( Color( COL_LIGHTGRAY ) );
            aR.Left()=0;
            aR.Right()=rDev.GetOutputSize().Width();
            rDev.DrawRect(aR);
            rDev.DrawText(rRect.TopLeft(),pEntry->aName);
        }
    } else {
        rtl_TextEncoding aTextEncoding = gsl_getSystemTextEncoding();

        XubString aStr("???", aTextEncoding);
        switch (nColumnId) {
            case ITEMBROWSER_WHICHCOL_ID:
                aStr = UniString::CreateFromInt32(pEntry->nWhichId); break;
            case ITEMBROWSER_STATECOL_ID: {
                switch (pEntry->eState) {
                    case SFX_ITEM_UNKNOWN : aStr=String("Uknown", aTextEncoding);   break;
                    case SFX_ITEM_DISABLED: aStr=String("Disabled", aTextEncoding); break;
                    case SFX_ITEM_DONTCARE: aStr=String("DontCare", aTextEncoding); break;
                    case SFX_ITEM_SET     : aStr=String("Set", aTextEncoding);      break;
                    case SFX_ITEM_DEFAULT : aStr=String("Default", aTextEncoding);  break;
                } // switch
            } break;
            case ITEMBROWSER_TYPECOL_ID: aStr = pEntry->GetItemTypeStr(); break;
            case ITEMBROWSER_NAMECOL_ID: aStr = pEntry->aName; break;
            case ITEMBROWSER_VALUECOL_ID: aStr = pEntry->aValue; break;
        } // switch
        rDev.SetClipRegion(aR);
        rDev.DrawText(aR.TopLeft(),aStr);
        rDev.SetClipRegion();
    }
}

ULONG _SdrItemBrowserControl::GetCurrentPos() const
{
    ULONG nRet=CONTAINER_ENTRY_NOTFOUND;
    if (GetSelectRowCount()==1) {
        long nPos=((BrowseBox*)this)->FirstSelectedRow();
        if (nPos>=0 && (ULONG)nPos<aList.Count()) {
            nRet=(ULONG)nPos;
        }
    }
    return nRet;
}

USHORT _SdrItemBrowserControl::GetCurrentWhich() const
{
    USHORT nRet=0;
    ULONG nPos=GetCurrentPos();
    if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
        nRet=ImpGetEntry(nPos)->nWhichId;
    }
    return nRet;
}

void __EXPORT _SdrItemBrowserControl::DoubleClick(const BrowserMouseEvent&)
{
    ULONG nPos=GetCurrentPos();
    if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
        BegChangeEntry(nPos);
    }
}

void __EXPORT _SdrItemBrowserControl::KeyInput(const KeyEvent& rKEvt)
{
    USHORT nKeyCode=rKEvt.GetKeyCode().GetCode()+rKEvt.GetKeyCode().GetModifier();
    FASTBOOL bAusgewertet=FALSE;
    ULONG nPos=GetCurrentPos();
    if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
        if (nKeyCode==KEY_RETURN) {
            if (BegChangeEntry(nPos)) bAusgewertet=TRUE;
        } else if (nKeyCode==KEY_ESCAPE) {
            // ...
        } else if (rKEvt.GetKeyCode().GetModifier()==KEY_SHIFT+KEY_MOD1+KEY_MOD2) { // Strg
            if (nKeyCode==KEY_SHIFT+KEY_MOD1+KEY_MOD2+KEY_W) {
                bWhichesButNames=!bWhichesButNames;
                SetDirty();
            }
            if (nKeyCode==KEY_SHIFT+KEY_MOD1+KEY_MOD2+KEY_I) {
                bDontHideIneffectiveItems=!bDontHideIneffectiveItems;
                SetDirty();
            }
            if (nKeyCode==KEY_SHIFT+KEY_MOD1+KEY_MOD2+KEY_S) {
                bDontSortItems=!bDontSortItems;
                SetDirty();
            }
        }
/*      else if (XubString::IsPrintable(rKEvt.GetCharCode())) {
            if (BegChangeEntry(nPos)) {
                bAusgewertet=TRUE;
                // folgende 3 Statements, weil
                // pEditControl->KeyInput(rKEvt) nicht geht
                pEditControl->SetText(rKEvt.GetCharCode());
                pEditControl->SetModifyFlag();
                pEditControl->SetSelection(1);
            }
        } */
    }
    if (!bAusgewertet) BrowseBox::KeyInput(rKEvt);
}

void _SdrItemBrowserControl::SetDirty()
{
    aSetDirtyHdl.Call(this);
}

void __EXPORT _SdrItemBrowserControl::Select()
{
    EndChangeEntry();
    BrowseBox::Select();
    ImpSaveWhich();
}

void _SdrItemBrowserControl::ImpSaveWhich()
{
    USHORT nWh=GetCurrentWhich();
    if (nWh!=0) {
        long nPos=GetCurrentPos();
        long nTop=GetTopRow();
        long nBtm=GetTopRow()+GetVisibleRows()+1;
        nLastWhich=nWh;
        nLastWhichOfs=nPos-nTop;
        if (nTop<0) nTop=0;
        if (nBtm>=(long)aList.Count()) nBtm=aList.Count()-1;
        nLastWhichOben=ImpGetEntry(nTop)->nWhichId;
        nLastWhichUnten=ImpGetEntry(nBtm)->nWhichId;
    }
}

void _SdrItemBrowserControl::ImpRestoreWhich()
{
    if (nLastWhich!=0) {
        FASTBOOL bFnd=FALSE;
        USHORT nBestMinWh=0,nBestMaxWh=0xFFFF;       // not implemented yet
        ULONG nBestMinPos=0,nBestMaxPos=0xFFFFFFFF;  // not implemented yet
        ULONG nAnz=aList.Count();
        ULONG nNum;
        for (nNum=0; nNum<nAnz && !bFnd; nNum++) {
            ImpItemListRow* pEntry=ImpGetEntry(nNum);
            if (!pEntry->bComment) {
                USHORT nWh=pEntry->nWhichId;
                if (nWh==nLastWhich) bFnd=TRUE;
                else if (nWh<nLastWhich && nWh>nBestMinWh) nBestMinPos=nNum;
                else if (nWh>nLastWhich && nWh<nBestMaxWh) nBestMaxPos=nNum;
            }
        }
        if (bFnd) {
            long nPos=nNum-1;
            long nWhichOfs=nPos-GetTopRow();
            if (nWhichOfs!=nLastWhichOfs) {
                ScrollRows(nWhichOfs-nLastWhichOfs);
            }
            GoToRow(nPos);
        }
    }
}

FASTBOOL _SdrItemBrowserControl::BegChangeEntry(ULONG nPos)
{
    BrkChangeEntry();
    FASTBOOL bRet=FALSE;
    ImpItemListRow* pEntry=ImpGetEntry(nPos);
    if (pEntry!=NULL && !pEntry->bComment) {
        SetMode(MYBROWSEMODE & ~BROWSER_KEEPHIGHLIGHT);
        pEditControl=new ImpItemEdit(&GetDataWindow(),this,0/*|WB_BORDER|WB_SVLOOK*/);
        Rectangle aRect(GetFieldRectPixel(nPos,ITEMBROWSER_VALUECOL_ID,FALSE));
        aRect.Left()+=2; // Kleiner Offset fuer's Edit, damit's pixelgenau stimmt
        aRect.Right()--;
        pEditControl->SetPosSizePixel(aRect.TopLeft(),aRect.GetSize());
        pEditControl->SetText(pEntry->aValue);
        pEditControl->SetBackground( Wallpaper(Color(COL_LIGHTGRAY)));
        Font aFont(pEditControl->GetFont());
        aFont.SetFillColor(Color(COL_LIGHTGRAY));
        pEditControl->SetFont(aFont);
        pEditControl->Show();
        pEditControl->GrabFocus();
        pEditControl->SetSelection(Selection(SELECTION_MIN,SELECTION_MAX));
        Window* pParent=GetParent();
        aWNamMerk=pParent->GetText();
        XubString aNeuNam(aWNamMerk);
        aNeuNam += sal_Unicode(' ');
        aNeuNam += pEntry->GetItemTypeStr();
        if (pEntry->bCanNum) {
            aNeuNam.AppendAscii(": ");
            aNeuNam += UniString::CreateFromInt32(pEntry->nMin);
            aNeuNam.AppendAscii("..");
            aNeuNam += UniString::CreateFromInt32(pEntry->nMax);
        }
        aNeuNam.AppendAscii(" - Type 'del' to reset to default.");
        pParent->SetText(aNeuNam);
        pAktChangeEntry=new ImpItemListRow(*pEntry);
        bRet=TRUE;
    }
    return bRet;
}

FASTBOOL _SdrItemBrowserControl::EndChangeEntry()
{
    FASTBOOL bRet=FALSE;
    if (pEditControl!=NULL) {
        aEntryChangedHdl.Call(this);
        delete pEditControl;
        pEditControl=NULL;
        delete pAktChangeEntry;
        pAktChangeEntry=NULL;
        Window* pParent=GetParent();
        pParent->SetText(aWNamMerk);
        SetMode(MYBROWSEMODE);
        bRet=TRUE;
    }
    return bRet;
}

void _SdrItemBrowserControl::BrkChangeEntry()
{
    if (pEditControl!=NULL) {
        delete pEditControl;
        pEditControl=NULL;
        delete pAktChangeEntry;
        pAktChangeEntry=NULL;
        Window* pParent=GetParent();
        pParent->SetText(aWNamMerk);
        SetMode(MYBROWSEMODE);
    }
}

void _SdrItemBrowserControl::ImpSetEntry(const ImpItemListRow& rEntry, ULONG nEntryNum)
{
    ImpItemListRow* pAktEntry=ImpGetEntry(nEntryNum);
    if (pAktEntry==NULL) {
        aList.Insert(new ImpItemListRow(rEntry),CONTAINER_APPEND);
        RowInserted(nEntryNum);
    } else if (*pAktEntry!=rEntry) {
        FASTBOOL bStateDiff=rEntry.eState!=pAktEntry->eState;
        FASTBOOL bValueDiff=!rEntry.aValue.Equals(pAktEntry->aValue);
        FASTBOOL bAllDiff=TRUE;
        if (bStateDiff || bValueDiff) {
            // Checken, ob nur State und/oder Value geaendert
            ImpItemListRow aTest(rEntry);
            aTest.eState=pAktEntry->eState;
            aTest.aValue=pAktEntry->aValue;
            if (aTest==*pAktEntry) bAllDiff=FALSE;
        }
        *pAktEntry=rEntry;
        if (bAllDiff) {
            RowModified(nEntryNum);
        } else {
            if (bStateDiff) RowModified(nEntryNum,ITEMBROWSER_STATECOL_ID);
            if (bValueDiff) RowModified(nEntryNum,ITEMBROWSER_VALUECOL_ID);
        }
    }
}

FASTBOOL ImpGetItem(const SfxItemSet& rSet, USHORT nWhich, const SfxPoolItem*& rpItem)
{
    SfxItemState eState=rSet.GetItemState(nWhich,TRUE,&rpItem);
    if (eState==SFX_ITEM_DEFAULT) {
        rpItem=&rSet.Get(nWhich);
    }
    return (eState==SFX_ITEM_DEFAULT || eState==SFX_ITEM_SET) && rpItem!=NULL;
}

FASTBOOL IsItemIneffective(USHORT nWhich, const SfxItemSet* pSet, USHORT& rIndent)
{
    rIndent=0;
    if (pSet==NULL) return FALSE;
    const SfxPoolItem* pItem=NULL;
    FASTBOOL bRet=FALSE;
    switch (nWhich) {
        case XATTR_LINEDASH         :
        case XATTR_LINEWIDTH        :
        case XATTR_LINECOLOR        :
        case XATTR_LINESTART        :
        case XATTR_LINEEND          :
        case XATTR_LINESTARTWIDTH   :
        case XATTR_LINEENDWIDTH     :
        case XATTR_LINESTARTCENTER  :
        case XATTR_LINEENDCENTER    :
        case XATTR_LINETRANSPARENCE : {
            rIndent=1;
            if (ImpGetItem(*pSet,XATTR_LINESTYLE,pItem)) {
                XLineStyle eLineStyle=((const XLineStyleItem*)pItem)->GetValue();
                if (eLineStyle==XLINE_NONE) return TRUE;
                if (eLineStyle!=XLINE_DASH && nWhich==XATTR_LINEDASH) return TRUE;
            }
            if (nWhich==XATTR_LINESTART || nWhich==XATTR_LINESTARTCENTER) {
                rIndent=2;
                if (ImpGetItem(*pSet,XATTR_LINESTARTWIDTH,pItem)) {
                    INT32 nWdt=((const XLineStartWidthItem*)pItem)->GetValue();
                    if (nWdt==0) return TRUE;
                }
            }
            if (nWhich==XATTR_LINEEND || nWhich==XATTR_LINEENDCENTER) {
                rIndent=2;
                if (ImpGetItem(*pSet,XATTR_LINEENDWIDTH,pItem)) {
                    INT32 nWdt=((const XLineEndWidthItem*)pItem)->GetValue();
                    if (nWdt==0) return TRUE;
                }
            }
        } break;
        case XATTR_FILLCOLOR           : /*nur bei Style=Color*/
        case XATTR_FILLGRADIENT        : /*nur bei Style=Gradient*/
        case XATTR_FILLHATCH           : /*nur bei Style=Hatch*/
        case XATTR_FILLTRANSPARENCE    : /*nur bei Style=Color*/
        case XATTR_GRADIENTSTEPCOUNT   : /*nur bei Style=Gradient*/
        case XATTR_FILLBACKGROUND      : /*nur bei Style=Hatch*/
        {
            rIndent=1;
            if (ImpGetItem(*pSet,XATTR_FILLSTYLE,pItem)) {
                XFillStyle eFillStyle=((const XFillStyleItem*)pItem)->GetValue();
                if (eFillStyle==XFILL_NONE) return TRUE;
                // Transparenz z.Zt. nur fuer SolidFill
                if (eFillStyle!=XFILL_SOLID && (nWhich==XATTR_FILLCOLOR || nWhich==XATTR_FILLTRANSPARENCE)) return TRUE;
                if (eFillStyle!=XFILL_GRADIENT && (nWhich==XATTR_FILLGRADIENT || nWhich==XATTR_GRADIENTSTEPCOUNT)) return TRUE;
                if (eFillStyle!=XFILL_HATCH && (nWhich==XATTR_FILLHATCH || nWhich==XATTR_FILLBACKGROUND)) return TRUE;
            }
        } break;
        case XATTR_FILLBITMAP          :
        case XATTR_FILLBMP_TILE        :
        case XATTR_FILLBMP_POS         : /* z.Zt. nur wenn TILE=TRUE */
        case XATTR_FILLBMP_SIZEX       : /* nur wenn nicht Stretch */
        case XATTR_FILLBMP_SIZEY       : /* nur wenn nicht Stretch */
        case XATTR_FILLBMP_SIZELOG     : /* nur wenn SIZELOG=FALSE zum ruecksetzen auf TRUE (alt) -> doch noch in Gebrauch */
        case XATTR_FILLBMP_TILEOFFSETX : /* nur wenn TILE=TRUE */
        case XATTR_FILLBMP_TILEOFFSETY : /* nur wenn TILE=TRUE */
        case XATTR_FILLBMP_STRETCH     : /* nur wenn TILE=FALSE */
        case XATTR_FILLBMP_POSOFFSETX  : /* nur wenn TILE=TRUE*/
        case XATTR_FILLBMP_POSOFFSETY  : { /* nur wenn TILE=TRUE*/
            rIndent=1;
            if (ImpGetItem(*pSet,XATTR_FILLSTYLE,pItem)) {
                XFillStyle eFillStyle=((const XFillStyleItem*)pItem)->GetValue();
                if (eFillStyle!=XFILL_BITMAP) return TRUE;
            }
            if (nWhich==XATTR_FILLBITMAP || nWhich==XATTR_FILLBMP_TILE) {
                return FALSE; // immer anwaehlbar
            }
            FASTBOOL bTileTRUE=FALSE;
            FASTBOOL bTileFALSE=FALSE;
            FASTBOOL bStretchTRUE=FALSE;
            FASTBOOL bStretchFALSE=FALSE;
            if (ImpGetItem(*pSet,XATTR_FILLBMP_TILE,pItem)) {
                bTileTRUE=((const XFillBmpTileItem*)pItem)->GetValue();
                bTileFALSE=!bTileTRUE;
            }
            if (ImpGetItem(*pSet,XATTR_FILLBMP_STRETCH,pItem)) {
                bStretchTRUE=((const XFillBmpStretchItem*)pItem)->GetValue();
                bStretchFALSE=!bStretchTRUE;
            }
            // Stretch nicht anwaehlbar, wenn Tile=TRUE
            if (nWhich==XATTR_FILLBMP_STRETCH) return bTileTRUE;
            // und uebrig bleiben 7+1 Item (Unterattribute)
            rIndent=2;
            // Pos (enum) nicht anwaehlbar, wenn Tile=FALSE
            if (nWhich==XATTR_FILLBMP_POS) return bTileFALSE;
            // SizeXY nicht anwaehlbar bei Stretch=TRUE
            if (nWhich==XATTR_FILLBMP_SIZEX || nWhich==XATTR_FILLBMP_SIZEY) {
                return bTileFALSE && bStretchTRUE;
            }
            // 2 Items speziell fuer Tile
            if (nWhich==XATTR_FILLBMP_POSOFFSETX  || nWhich==XATTR_FILLBMP_POSOFFSETY) {
                return bTileFALSE;
            }
            // Noch 2 Items speziell fuer Tile die sich jedoch gegenseitig ausschliessen
            if (nWhich==XATTR_FILLBMP_TILEOFFSETX || nWhich==XATTR_FILLBMP_TILEOFFSETY) {
                if (bTileFALSE) return TRUE;
                USHORT nX=0,nY=0;
                FASTBOOL bX=FALSE,bY=FALSE;
                if (ImpGetItem(*pSet,XATTR_FILLBMP_TILEOFFSETX,pItem)) {
                    nX=((const XFillBmpTileOffsetXItem*)pItem)->GetValue();
                    bX=TRUE;
                }
                if (ImpGetItem(*pSet,XATTR_FILLBMP_TILEOFFSETY,pItem)) {
                    nY=((const XFillBmpTileOffsetYItem*)pItem)->GetValue();
                    bY=TRUE;
                }
                if (nWhich==XATTR_FILLBMP_TILEOFFSETX) {
                    if (nX!=0 || !bX) return FALSE;
                    if (nY!=0) return TRUE;
                } else {
                    if (nY!=0 || !bY) return FALSE;
                    if (nX!=0) return TRUE;
                }
            }
            // SizeLog nicht anwaehlbar bei Stretch=TRUE
            // und sonst auch nur wenn es auf SizeLog=FALSE gesetzt ist.
            // -> wohl doch noch in Gebrauch
            // (TRUE ist der statische PoolDefault)
            if (nWhich==XATTR_FILLBMP_SIZELOG) {
                if (bTileFALSE && bStretchTRUE) return TRUE;
            }
        } break;

        case XATTR_FORMTXTADJUST    :
        case XATTR_FORMTXTDISTANCE  :
        case XATTR_FORMTXTSTART     :
        case XATTR_FORMTXTMIRROR    :
        case XATTR_FORMTXTOUTLINE   :
        case XATTR_FORMTXTSHADOW    :
        case XATTR_FORMTXTSHDWCOLOR :
        case XATTR_FORMTXTSHDWXVAL  :
        case XATTR_FORMTXTSHDWYVAL  :
        case XATTR_FORMTXTSTDFORM   :
        case XATTR_FORMTXTHIDEFORM  :
        case XATTR_FORMTXTSHDWTRANSP: {
            rIndent=1;
            if (ImpGetItem(*pSet,XATTR_FORMTXTSTYLE,pItem)) {
                XFormTextStyle eStyle=((const XFormTextStyleItem*)pItem)->GetValue();
                if (eStyle==XFT_NONE) return TRUE;
            }
            if ((nWhich>=XATTR_FORMTXTSHDWCOLOR && nWhich<=XATTR_FORMTXTSHDWYVAL) || nWhich>=XATTR_FORMTXTSHDWTRANSP) {
                rIndent=2;
                if (ImpGetItem(*pSet,XATTR_FORMTXTSHADOW,pItem)) {
                    XFormTextShadow eShadow=((const XFormTextShadowItem*)pItem)->GetValue();
                    if (eShadow==XFTSHADOW_NONE) return TRUE;
                }
            }
        } break;

        case SDRATTR_SHADOWCOLOR       :
        case SDRATTR_SHADOWXDIST       :
        case SDRATTR_SHADOWYDIST       :
        case SDRATTR_SHADOWTRANSPARENCE:
        case SDRATTR_SHADOW3D          :
        case SDRATTR_SHADOWPERSP       : {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_SHADOW,pItem)) {
                FASTBOOL bShadow=((const SdrShadowItem*)pItem)->GetValue();
                if (!bShadow) return TRUE;
            }
        } break;

        case SDRATTR_CAPTIONANGLE: {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_CAPTIONFIXEDANGLE,pItem)) {
                FASTBOOL bFixed=((const SdrCaptionFixedAngleItem*)pItem)->GetValue();
                if (!bFixed) return TRUE;
            }
        } break;
        case SDRATTR_CAPTIONESCREL:
        case SDRATTR_CAPTIONESCABS: {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_CAPTIONESCISREL,pItem)) {
                FASTBOOL bRel=((const SdrCaptionEscIsRelItem*)pItem)->GetValue();
                if (bRel && nWhich==SDRATTR_CAPTIONESCABS) return TRUE;
                if (!bRel && nWhich==SDRATTR_CAPTIONESCREL) return TRUE;
            }
        } break;
        case SDRATTR_CAPTIONLINELEN: {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_CAPTIONFITLINELEN,pItem)) {
                FASTBOOL bFit=((const SdrCaptionFitLineLenItem*)pItem)->GetValue();
                if (bFit) return TRUE;
            }
        } break;

        case SDRATTR_TEXT_MINFRAMEHEIGHT:
        case SDRATTR_TEXT_MAXFRAMEHEIGHT: {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_TEXT_AUTOGROWHEIGHT,pItem)) {
                FASTBOOL bAutoGrow=((const SdrTextAutoGrowHeightItem*)pItem)->GetValue();
                if (!bAutoGrow) return TRUE;
            }
        } break;
        case SDRATTR_TEXT_MINFRAMEWIDTH:
        case SDRATTR_TEXT_MAXFRAMEWIDTH: {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_TEXT_AUTOGROWWIDTH,pItem)) {
                FASTBOOL bAutoGrow=((const SdrTextAutoGrowWidthItem*)pItem)->GetValue();
                if (!bAutoGrow) return TRUE;
            }
        } break;
        case SDRATTR_TEXT_VERTADJUST:
        case SDRATTR_TEXT_HORZADJUST: {
            if (ImpGetItem(*pSet,SDRATTR_TEXT_FITTOSIZE,pItem)) {
                SdrFitToSizeType eFit=((const SdrTextFitToSizeTypeItem*)pItem)->GetValue();
                if (eFit!=SDRTEXTFIT_NONE) return TRUE;
            }
        } break;

        case SDRATTR_TEXT_ANIDIRECTION  :
        case SDRATTR_TEXT_ANISTARTINSIDE:
        case SDRATTR_TEXT_ANISTOPINSIDE :
        case SDRATTR_TEXT_ANICOUNT      :
        case SDRATTR_TEXT_ANIDELAY      :
        case SDRATTR_TEXT_ANIAMOUNT     : {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_TEXT_ANIKIND,pItem)) {
                SdrTextAniKind eAniKind=((const SdrTextAniKindItem*)pItem)->GetValue();
                if (eAniKind==SDRTEXTANI_NONE) return TRUE;
                if (eAniKind==SDRTEXTANI_BLINK && (nWhich==SDRATTR_TEXT_ANIDIRECTION || nWhich==SDRATTR_TEXT_ANIAMOUNT)) return TRUE;
                if (eAniKind==SDRTEXTANI_SLIDE && (nWhich==SDRATTR_TEXT_ANISTARTINSIDE || nWhich==SDRATTR_TEXT_ANISTOPINSIDE)) return TRUE;
            }
        } break;

        case SDRATTR_EDGELINEDELTAANZ: return TRUE;
        case SDRATTR_EDGELINE1DELTA:
        case SDRATTR_EDGELINE2DELTA:
        case SDRATTR_EDGELINE3DELTA: {
            if (ImpGetItem(*pSet,SDRATTR_EDGEKIND,pItem)) {
                SdrEdgeKind eKind=((const SdrEdgeKindItem*)pItem)->GetValue();
                if (eKind==SDREDGE_THREELINES) {
                    if (nWhich>SDRATTR_EDGELINE2DELTA) return TRUE;
                    else return FALSE;
                }
                if (eKind!=SDREDGE_ORTHOLINES && eKind!=SDREDGE_BEZIER) return TRUE;
            }
            if (ImpGetItem(*pSet,SDRATTR_EDGELINEDELTAANZ,pItem)) {
                UINT16 nAnz=((const SdrEdgeLineDeltaAnzItem*)pItem)->GetValue();
                if (nAnz==0) return TRUE;
                if (nAnz==1 && nWhich>SDRATTR_EDGELINE1DELTA) return TRUE;
                if (nAnz==2 && nWhich>SDRATTR_EDGELINE2DELTA) return TRUE;
                if (nAnz==3 && nWhich>SDRATTR_EDGELINE3DELTA) return TRUE;
            }
        } break;

        case SDRATTR_CIRCSTARTANGLE:
        case SDRATTR_CIRCENDANGLE  : {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_CIRCKIND,pItem)) {
                SdrCircKind eKind=((const SdrCircKindItem*)pItem)->GetValue();
                if (eKind==SDRCIRC_FULL) return TRUE;
            }
        } break;
    } // switch
    return bRet;
}

USHORT ImpSortWhich(USHORT nWhich)
{
    switch (nWhich) {
        // Line
        case XATTR_LINESTART            : nWhich=XATTR_LINETRANSPARENCE     ; break;
        case XATTR_LINEEND              : nWhich=XATTR_LINESTARTWIDTH       ; break;
        case XATTR_LINESTARTWIDTH       : nWhich=XATTR_LINESTART            ; break;
        case XATTR_LINEENDWIDTH         : nWhich=XATTR_LINESTARTCENTER      ; break;
        case XATTR_LINESTARTCENTER      : nWhich=XATTR_LINEENDWIDTH         ; break;
        case XATTR_LINEENDCENTER        : nWhich=XATTR_LINEEND              ; break;
        case XATTR_LINETRANSPARENCE     : nWhich=XATTR_LINEENDCENTER        ; break;

        // Fill
        case XATTR_FILLBMP_POS          : nWhich=XATTR_FILLBMP_STRETCH      ; break;
        case XATTR_FILLBMP_SIZEX        : nWhich=XATTR_FILLBMP_POS          ; break;
        case XATTR_FILLBMP_SIZEY        : nWhich=XATTR_FILLBMP_SIZEX        ; break;
        case XATTR_FILLBMP_SIZELOG      : nWhich=XATTR_FILLBMP_SIZEY        ; break;
        case XATTR_FILLBMP_TILEOFFSETX  : nWhich=XATTR_FILLBMP_SIZELOG      ; break;
        case XATTR_FILLBMP_TILEOFFSETY  : nWhich=XATTR_FILLBMP_TILEOFFSETX  ; break;
        case XATTR_FILLBMP_STRETCH      : nWhich=XATTR_FILLBMP_TILEOFFSETY  ; break;

        // Fontwork
        case XATTR_FORMTXTSHADOW        : nWhich=XATTR_FORMTXTSTDFORM       ; break;
        case XATTR_FORMTXTSHDWCOLOR     : nWhich=XATTR_FORMTXTHIDEFORM      ; break;
        case XATTR_FORMTXTSHDWXVAL      : nWhich=XATTR_FORMTXTSHADOW        ; break;
        case XATTR_FORMTXTSHDWYVAL      : nWhich=XATTR_FORMTXTSHDWCOLOR     ; break;
        case XATTR_FORMTXTSTDFORM       : nWhich=XATTR_FORMTXTSHDWXVAL      ; break;
        case XATTR_FORMTXTHIDEFORM      : nWhich=XATTR_FORMTXTSHDWYVAL      ; break;

        // Misc
        case SDRATTR_TEXT_MINFRAMEHEIGHT: nWhich=SDRATTR_TEXT_FITTOSIZE     ; break;
        case SDRATTR_TEXT_AUTOGROWHEIGHT: nWhich=SDRATTR_TEXT_LEFTDIST      ; break;
        case SDRATTR_TEXT_FITTOSIZE     : nWhich=SDRATTR_TEXT_RIGHTDIST     ; break;
        case SDRATTR_TEXT_LEFTDIST      : nWhich=SDRATTR_TEXT_UPPERDIST     ; break;
        case SDRATTR_TEXT_RIGHTDIST     : nWhich=SDRATTR_TEXT_LOWERDIST     ; break;
        case SDRATTR_TEXT_UPPERDIST     : nWhich=SDRATTR_TEXT_AUTOGROWWIDTH ; break;
        case SDRATTR_TEXT_LOWERDIST     : nWhich=SDRATTR_TEXT_MINFRAMEWIDTH ; break;
        case SDRATTR_TEXT_VERTADJUST    : nWhich=SDRATTR_TEXT_MAXFRAMEWIDTH ; break;
        case SDRATTR_TEXT_MAXFRAMEHEIGHT: nWhich=SDRATTR_TEXT_AUTOGROWHEIGHT; break;
        case SDRATTR_TEXT_MINFRAMEWIDTH : nWhich=SDRATTR_TEXT_MINFRAMEHEIGHT; break;
        case SDRATTR_TEXT_MAXFRAMEWIDTH : nWhich=SDRATTR_TEXT_MAXFRAMEHEIGHT; break;
        case SDRATTR_TEXT_AUTOGROWWIDTH : nWhich=SDRATTR_TEXT_HORZADJUST    ; break;
        case SDRATTR_TEXT_HORZADJUST    : nWhich=SDRATTR_TEXT_VERTADJUST    ; break;
    } // switch
    return nWhich;
}

#define INSERTCOMMENT(nStartId,nEndId,aStr) \
    { if (nWhich0<nStartId && nWhich>=nStartId && nWhich<=nEndId) aCommentStr=aStr; }

void _SdrItemBrowserControl::SetAttributes(const SfxItemSet* pSet, const SfxItemSet* p2ndSet)
{
    SetMode(MYBROWSEMODE & ~BROWSER_KEEPHIGHLIGHT);
    if (pSet!=NULL) {
        rtl_TextEncoding aTextEncoding = gsl_getSystemTextEncoding();
        USHORT nEntryNum=0;
        SfxWhichIter aIter(*pSet);
        const SfxItemPool* pPool=pSet->GetPool();
        USHORT nWhich0=0;
        USHORT nWhich=aIter.FirstWhich();
        while (nWhich!=0) {
            // Nun erstmal etwas umsortieren
            // Geht nur, solange keine InvalidItems, d.h. keine Luecken
            // an dieser Stelle im Set sind
            if (!bDontSortItems) nWhich=ImpSortWhich(nWhich);
            SfxItemState eState=pSet->GetItemState(nWhich);
            if (p2ndSet!=NULL) {
                SfxItemState e2ndState=p2ndSet->GetItemState(nWhich);
                if (eState==SFX_ITEM_DEFAULT) eState=SFX_ITEM_DISABLED;
                else if (e2ndState==SFX_ITEM_DEFAULT) eState=SFX_ITEM_DEFAULT;
            }
            if (eState!=SFX_ITEM_DISABLED) {
                const SfxPoolItem& rItem=pSet->Get(nWhich);
                USHORT nIndent=0;
                if (!HAS_BASE(SfxVoidItem,&rItem) && !HAS_BASE(SfxSetItem,&rItem) && (!IsItemIneffective(nWhich,pSet,nIndent) || bDontHideIneffectiveItems)) {
                    XubString aCommentStr;

                    INSERTCOMMENT(XATTR_LINE_FIRST,XATTR_LINE_LAST,String("L I N I E", aTextEncoding));
                    INSERTCOMMENT(XATTR_FILL_FIRST,XATTR_FILL_LAST,String("F L Ä C H E", aTextEncoding));
                    INSERTCOMMENT(XATTR_TEXT_FIRST,XATTR_TEXT_LAST,String("F O N T W O R K", aTextEncoding));
                    INSERTCOMMENT(SDRATTR_SHADOW_FIRST,SDRATTR_SHADOW_LAST,String("S C H A T T E N", aTextEncoding));
                    INSERTCOMMENT(SDRATTR_CAPTION_FIRST,SDRATTR_CAPTION_LAST,String("L E G E N D E", aTextEncoding));
                    INSERTCOMMENT(SDRATTR_MISC_FIRST,SDRATTR_MISC_LAST,String("V E R S C H I E D E N E S", aTextEncoding));
                    INSERTCOMMENT(SDRATTR_EDGE_FIRST,SDRATTR_EDGE_LAST,String("V E R B I N D E R", aTextEncoding));
                    INSERTCOMMENT(SDRATTR_MEASURE_FIRST,SDRATTR_MEASURE_LAST,String("B E M A S S U N G", aTextEncoding));
                    INSERTCOMMENT(SDRATTR_CIRC_FIRST,SDRATTR_CIRC_LAST,String("K R E I S", aTextEncoding));
                    INSERTCOMMENT(SDRATTR_NOTPERSIST_FIRST,SDRATTR_NOTPERSIST_LAST,String("N O T P E R S I S T", aTextEncoding));
                    INSERTCOMMENT(SDRATTR_MOVEX,SDRATTR_VERTSHEARONE,String("Transformationen auf alle Objekte einzeln", aTextEncoding));
                    INSERTCOMMENT(SDRATTR_RESIZEXALL,SDRATTR_VERTSHEARALL,String("Transformationen auf alle Objekte gemeinsam", aTextEncoding));
                    INSERTCOMMENT(SDRATTR_TRANSFORMREF1X,SDRATTR_TRANSFORMREF2Y,String("View-Referenzpunkte", aTextEncoding));
                    INSERTCOMMENT(SDRATTR_GRAF_FIRST,SDRATTR_GRAF_LAST,String("G R A F I K", aTextEncoding));
                    INSERTCOMMENT(EE_ITEMS_START,EE_ITEMS_END,String("E D I T  E N G I N E", aTextEncoding));
                    INSERTCOMMENT(EE_ITEMS_END+1,EE_ITEMS_END+1,String("... by Joe Merten, JME Engineering Berlin ...", aTextEncoding));

                    if(aCommentStr.Len())
                    {
                        ImpItemListRow aEntry;
                        aEntry.bComment=TRUE;
                        aEntry.aName=aCommentStr;
                        ImpSetEntry(aEntry,nEntryNum);
                        nEntryNum++;
                    }
                    nWhich0=nWhich;
                    ImpItemListRow aEntry;
                    SdrItemPool::TakeItemName(nWhich, aEntry.aName);
                    nIndent*=2;

                    while(nIndent > 0)
                    {
                        aEntry.aName.Insert(sal_Unicode(' '), 0);
                        nIndent--;
                    }

                    aEntry.eState=eState;
                    aEntry.nWhichId=nWhich;
                    if (!IsInvalidItem(&rItem)) {
                        aEntry.pType=rItem.Type();
                        aEntry.nMax=0x7FFFFFFF;
                        aEntry.nMin=-aEntry.nMax;
                        aEntry.nVal=-4711;
                        if      (HAS_BASE(SfxByteItem     ,&rItem)) aEntry.eItemType=ITEM_BYTE;
                        else if (HAS_BASE(SfxInt16Item    ,&rItem)) aEntry.eItemType=ITEM_INT16;
                        else if (HAS_BASE(SfxUInt16Item   ,&rItem)) aEntry.eItemType=ITEM_UINT16;
                        else if (HAS_BASE(SfxInt32Item    ,&rItem)) aEntry.eItemType=ITEM_INT32;
                        else if (HAS_BASE(SfxUInt32Item   ,&rItem)) aEntry.eItemType=ITEM_UINT32;
                        else if (HAS_BASE(SfxEnumItemInterface,&rItem)) aEntry.eItemType=ITEM_ENUM;
                        else if (HAS_BASE(SfxBoolItem     ,&rItem)) aEntry.eItemType=ITEM_BOOL;
                        else if (HAS_BASE(SfxFlagItem     ,&rItem)) aEntry.eItemType=ITEM_FLAG;
                        else if (HAS_BASE(XColorItem      ,&rItem)) aEntry.eItemType=ITEM_XCOLOR;
                        else if (HAS_BASE(SfxStringItem   ,&rItem)) aEntry.eItemType=ITEM_STRING;
                        else if (HAS_BASE(SfxPointItem    ,&rItem)) aEntry.eItemType=ITEM_POINT;
                        else if (HAS_BASE(SfxRectangleItem,&rItem)) aEntry.eItemType=ITEM_RECT;
                        else if (HAS_BASE(SfxRangeItem    ,&rItem)) aEntry.eItemType=ITEM_RANGE;
                        else if (HAS_BASE(SdrFractionItem ,&rItem)) aEntry.eItemType=ITEM_FRACTION;
                        else if (HAS_BASE(SvxColorItem    ,&rItem)) aEntry.eItemType=ITEM_COLOR;
                        else if (HAS_BASE(SvxFontItem     ,&rItem)) aEntry.eItemType=ITEM_FONT;
                        else if (HAS_BASE(SvxFontHeightItem,&rItem))aEntry.eItemType=ITEM_FONTHEIGHT;
                        else if (HAS_BASE(SvxCharScaleWidthItem,&rItem)) aEntry.eItemType=ITEM_FONTWIDTH;
                        else if (HAS_BASE(SvxFieldItem    ,&rItem)) aEntry.eItemType=ITEM_FIELD;
                        switch (aEntry.eItemType) {
                            case ITEM_BYTE      : aEntry.bIsNum=TRUE;  aEntry.nVal=((SfxByteItem  &)rItem).GetValue(); aEntry.nMin=0;      aEntry.nMax=255;   break;
                            case ITEM_INT16     : aEntry.bIsNum=TRUE;  aEntry.nVal=((SfxInt16Item &)rItem).GetValue(); aEntry.nMin=-32767; aEntry.nMax=32767; break;
                            case ITEM_UINT16    : aEntry.bIsNum=TRUE;  aEntry.nVal=((SfxUInt16Item&)rItem).GetValue(); aEntry.nMin=0;      aEntry.nMax=65535; break;
                            case ITEM_INT32     : aEntry.bIsNum=TRUE;  aEntry.nVal=((SfxInt32Item &)rItem).GetValue();                                        break;
                            case ITEM_UINT32    : aEntry.bIsNum=TRUE;  aEntry.nVal=((SfxUInt32Item&)rItem).GetValue(); aEntry.nMin=0; /*aEntry.nMax=0xFF...*/;break;
                            case ITEM_ENUM      : aEntry.bCanNum=TRUE; aEntry.nVal=((SfxEnumItemInterface&)rItem).GetEnumValue(); aEntry.nMin=0; aEntry.nMax=((SfxEnumItemInterface&)rItem).GetValueCount()-1; break;
                            case ITEM_BOOL      : aEntry.bCanNum=TRUE; aEntry.nVal=((SfxBoolItem  &)rItem).GetValue(); aEntry.nMin=0; aEntry.nMax=1;          break;
                            case ITEM_FLAG      : aEntry.bCanNum=TRUE; aEntry.nVal=((SfxFlagItem  &)rItem).GetValue(); aEntry.nMin=0; aEntry.nMax=0xFFFF;     break;
                            case ITEM_FONTHEIGHT: aEntry.bCanNum=TRUE; aEntry.nVal=((SvxFontHeightItem&)rItem).GetHeight(); aEntry.nMin=0;                    break;
                            case ITEM_FONTWIDTH : aEntry.bCanNum=TRUE; aEntry.nVal=((SvxCharScaleWidthItem&)rItem).GetValue();    aEntry.nMin=0; aEntry.nMax=0xFFFF;break;
                        } // switch
                        if (aEntry.bIsNum) aEntry.bCanNum=TRUE;
                        FASTBOOL bGetPres=TRUE;
                        if (bGetPres) {
                            rItem.GetPresentation(SFX_ITEM_PRESENTATION_NAMELESS,
                                                  pPool->GetMetric(nWhich),
                                                  SFX_MAPUNIT_MM,aEntry.aValue);
                            if (aEntry.bCanNum) {
                                aEntry.aValue.InsertAscii(": ",0);
                                aEntry.aValue.Insert(UniString::CreateFromInt32(aEntry.nVal),0);
                            }
                        } else {
                            if (aEntry.eItemType==ITEM_BOOL) aEntry.aValue.AppendAscii(aEntry.nVal!=0 ? "True" : "False");
                            else if (aEntry.bCanNum) aEntry.aValue = UniString::CreateFromInt32(aEntry.nVal);
                            else if (aEntry.eItemType==ITEM_STRING) aEntry.aValue=((SfxStringItem&)rItem).GetValue();
                            else if (aEntry.eItemType==ITEM_ENUM && nWhich!=EE_CHAR_WEIGHT) aEntry.aValue=((SfxEnumItemInterface&)rItem).GetValueTextByPos((BOOL)aEntry.nVal);
                            else aEntry.aValue = String("GPF", aTextEncoding);
                        }
                    } else {
                        aEntry.aValue = String("InvalidItem", aTextEncoding);
                    }
                    ImpSetEntry(aEntry,nEntryNum);
                    nEntryNum++;
                }
            }
            nWhich=aIter.NextWhich();
        } // while

        if (aList.Count()>nEntryNum) { // evtl. noch zuviele Eintraege
            ULONG nTooMuch=aList.Count()-nEntryNum;
            for (ULONG nNum=0; nNum<nTooMuch; nNum++) {
                delete ImpGetEntry(nEntryNum);
                aList.Remove(nEntryNum);
            }
            RowRemoved(nEntryNum,nTooMuch);
        }
    } else {
        Clear(); // wenn pSet==NULL
    }
    ImpRestoreWhich();
    SetMode(MYBROWSEMODE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

_SdrItemBrowserWindow::_SdrItemBrowserWindow(Window* pParent, WinBits nBits):
    FloatingWindow(pParent,nBits),
    aBrowse(this)
{
    SetOutputSizePixel(aBrowse.GetSizePixel());
    SetText(String("Joe's ItemBrowser", gsl_getSystemTextEncoding()));
    aBrowse.Show();
}

__EXPORT _SdrItemBrowserWindow::~_SdrItemBrowserWindow()
{
}

void __EXPORT _SdrItemBrowserWindow::Resize()
{
    aBrowse.SetSizePixel(GetOutputSizePixel());
}

void __EXPORT _SdrItemBrowserWindow::GetFocus()
{
    aBrowse.GrabFocus();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrItemBrowser::SdrItemBrowser(SdrView& rView):
    _SdrItemBrowserWindow(ImpGetViewWin(rView)),
    pView(&rView),
    bDirty(FALSE)
{
    aIdleTimer.SetTimeoutHdl(LINK(this,SdrItemBrowser,IdleHdl));
    GetBrowserControl().SetEntryChangedHdl(LINK(this,SdrItemBrowser,ChangedHdl));
    GetBrowserControl().SetSetDirtyHdl(LINK(this,SdrItemBrowser,SetDirtyHdl));
    SetDirty();
}

Window* SdrItemBrowser::ImpGetViewWin(SdrView& rView)
{
    USHORT nAnz=rView.GetWinCount();
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        OutputDevice* pOut=rView.GetWin(nNum);
        if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
            return (Window*)pOut;
        }
    }
    return 0L;
}

void SdrItemBrowser::ForceParent()
{
    Window* pWin=ImpGetViewWin(*pView);
    if (pWin!=NULL) SetParent(pWin);
}

void SdrItemBrowser::SetDirty()
{
    if (!bDirty) {
        bDirty=TRUE;
        aIdleTimer.SetTimeout(1);
        aIdleTimer.Start();
    }
}

void SdrItemBrowser::Undirty()
{
    aIdleTimer.Stop();
    bDirty = FALSE;

//  SfxItemSet aSet(pView->GetAttributes());
    SfxItemSet aSet(pView->GetModel()->GetItemPool());
    pView->GetAttributes(aSet);

    if(pView->HasMarkedObj())
    {
        // SfxItemSet a2ndSet(pView->GetAttributes(TRUE));
        SfxItemSet a2ndSet(pView->GetModel()->GetItemPool());
        pView->GetAttributes(a2ndSet, TRUE);

        SetAttributes(&aSet,&a2ndSet);
    }
    else
    {
        SetAttributes(&aSet);
    }
}

IMPL_LINK(SdrItemBrowser,IdleHdl,Timer*,aTimer)
{
    Undirty();
    return 0;
}

IMPL_LINK(SdrItemBrowser,ChangedHdl,_SdrItemBrowserControl*,pBrowse)
{
    const ImpItemListRow* pEntry=pBrowse->GetAktChangeEntry();
    if (pEntry!=NULL)
    {
//      SfxItemSet aSet(pView->GetAttributes());
        SfxItemSet aSet(pView->GetModel()->GetItemPool());
        pView->GetAttributes(aSet);

        SfxItemSet aNewSet(*aSet.GetPool(),pEntry->nWhichId,pEntry->nWhichId);
        XubString aNewText(pBrowse->GetNewEntryValue());
        BOOL bDel( aNewText.EqualsAscii("del")
            || aNewText.EqualsAscii("Del")
            || aNewText.EqualsAscii("DEL")
            || aNewText.EqualsAscii("default")
            || aNewText.EqualsAscii("Default")
            || aNewText.EqualsAscii("DEFAULT"));

        if (!bDel) {
            SfxPoolItem* pNewItem=aSet.Get(pEntry->nWhichId).Clone();
            long nLongVal=0;
            nLongVal = aNewText.ToInt32();
            if (pEntry->bCanNum) {
                if (nLongVal>pEntry->nMax) nLongVal=pEntry->nMax;
                if (nLongVal<pEntry->nMin) nLongVal=pEntry->nMin;
            }
            FASTBOOL bPair=FALSE;
            FASTBOOL bPairX=TRUE;
            FASTBOOL bPairY=FALSE;
            USHORT nSepLen=1;
            long nLongX = aNewText.ToInt32();
            long nLongY=0;
            xub_StrLen nPos = aNewText.Search(sal_Unicode('/'));
            if (nPos==STRING_NOTFOUND) nPos=aNewText.Search(sal_Unicode(':'));
            if (nPos==STRING_NOTFOUND) nPos=aNewText.Search(sal_Unicode(' '));
            if (nPos==STRING_NOTFOUND) { nPos=aNewText.SearchAscii(".."); if (nPos!=STRING_NOTFOUND) nSepLen=2; }
            if (nPos!=STRING_NOTFOUND) {
                bPair=TRUE;
                bPairX=nPos>0;
                XubString s(aNewText);
                s.Erase(0,nPos+nSepLen);
                bPairY = (BOOL)aNewText.Len();
                nLongY = s.ToInt32();
            }
            switch (pEntry->eItemType) {
                case ITEM_BYTE  : ((SfxByteItem  *)pNewItem)->SetValue((BYTE  )nLongVal); break;
                case ITEM_INT16 : ((SfxInt16Item *)pNewItem)->SetValue((INT16 )nLongVal); break;
                case ITEM_UINT16: ((SfxUInt16Item*)pNewItem)->SetValue((UINT16)nLongVal); break;
                case ITEM_INT32: {
                    if(HAS_BASE(SdrAngleItem, pNewItem))
                    {
                        aNewText.SearchAndReplace(sal_Unicode(','), sal_Unicode('.'));
                        double nVal = aNewText.ToFloat();
                        nLongVal = (long)(nVal * 100 + 0.5);
                    }
                    ((SfxInt32Item *)pNewItem)->SetValue((INT32)nLongVal);
                } break;
                case ITEM_UINT32: ((SfxUInt32Item*)pNewItem)->SetValue(aNewText.ToInt32()); break;
                case ITEM_ENUM  : ((SfxEnumItemInterface*)pNewItem)->SetEnumValue((USHORT)nLongVal); break;
                case ITEM_BOOL: {
                    aNewText.ToUpperAscii();
                    if (aNewText.EqualsAscii("TRUE")) nLongVal=1;
                    if (aNewText.EqualsAscii("JA")) nLongVal=1;
                    if (aNewText.EqualsAscii("AN")) nLongVal=1;
                    if (aNewText.EqualsAscii("EIN")) nLongVal=1;
                    if (aNewText.EqualsAscii("ON")) nLongVal=1;
                    if (aNewText.EqualsAscii("YES")) nLongVal=1;
                    ((SfxBoolItem*)pNewItem)->SetValue((BOOL)nLongVal);
                } break;
                case ITEM_FLAG  : ((SfxFlagItem  *)pNewItem)->SetValue((USHORT)nLongVal); break;
                case ITEM_STRING: ((SfxStringItem*)pNewItem)->SetValue(aNewText); break;
                case ITEM_POINT : ((SfxPointItem*)pNewItem)->SetValue(Point(nLongX,nLongY)); break;
                case ITEM_RECT  : break;
                case ITEM_RANGE : {
                    ((SfxRangeItem*)pNewItem)->From()=(USHORT)nLongX;
                    ((SfxRangeItem*)pNewItem)->From()=(USHORT)nLongY;
                } break;
                case ITEM_LRANGE : {
                } break;
                case ITEM_FRACTION: {
                    if (!bPairX) nLongX=1;
                    if (!bPairY) nLongY=1;
                    ((SdrFractionItem*)pNewItem)->SetValue(Fraction(nLongX,nLongY));
                } break;
                case ITEM_XCOLOR: break;
                case ITEM_COLOR: break;
                case ITEM_FONT: {
                    ((SvxFontItem*)pNewItem)->GetFamily()=FAMILY_DONTKNOW;
                    ((SvxFontItem*)pNewItem)->GetFamilyName()=aNewText;
                    ((SvxFontItem*)pNewItem)->GetStyleName().Erase();
                } break;
                case ITEM_FONTHEIGHT: {
                    ULONG nHgt=0;
                    USHORT nProp=100;
                    if (aNewText.Search(sal_Unicode('%'))!=STRING_NOTFOUND) {
                        nProp=(USHORT)nLongVal;
                    } else {
                        nHgt=nLongVal;
                    }
                    ((SvxFontHeightItem*)pNewItem)->SetHeight(nHgt,nProp);
                } break;
                case ITEM_FONTWIDTH: {
                    USHORT nProp=100;
                    if (aNewText.Search(sal_Unicode('%'))!=STRING_NOTFOUND) {
                        nProp=(USHORT)nLongVal;
                    }
                    ((SvxCharScaleWidthItem*)pNewItem)->SetValue(nProp);
                } break;
                case ITEM_FIELD: break;
            } // switch
            aNewSet.Put(*pNewItem);
            delete pNewItem;
        }
        pView->SetAttributes(aNewSet,bDel);
    }
    return 0;
}

IMPL_LINK(SdrItemBrowser,SetDirtyHdl,_SdrItemBrowserControl*,pBrowse)
{
    SetDirty();
    return 0;
}


