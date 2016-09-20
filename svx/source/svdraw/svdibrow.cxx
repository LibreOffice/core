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

#include <stdlib.h>
#include <limits>

#include "editeng/fontitem.hxx"
#include "svdibrow.hxx"
#include <editeng/charscaleitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/flditem.hxx>
#include <svl/flagitem.hxx>
#include <svl/ptitem.hxx>
#include <svl/rectitem.hxx>
#include <svl/rngitem.hxx>
#include <svl/whiter.hxx>

#include <svx/sdrpaintwindow.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdview.hxx>
#include <svx/sxcaitm.hxx>
#include <svx/sxcecitm.hxx>
#include <sxcikitm.hxx>
#include <svx/sxcllitm.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sxelditm.hxx>
#include <svx/sxfiitm.hxx>
#include <svx/xenum.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbtoxy.hxx>
#include <svx/xftshit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xtextit0.hxx>

using namespace com::sun::star;

#define ITEMBROWSER_WHICHCOL_ID 1
#define ITEMBROWSER_STATECOL_ID 2
#define ITEMBROWSER_TYPECOL_ID  3
#define ITEMBROWSER_NAMECOL_ID  4
#define ITEMBROWSER_VALUECOL_ID 5

#define ITEM_NOT_FOUND std::numeric_limits<std::size_t>::max()

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
    OUString                        aName;
    OUString                        aValue;
    SfxItemState                    eState;
    sal_uInt16                      nWhichId;

    const std::type_info*           pType;
    ItemType                        eItemType;

    sal_Int32                       nVal;
    sal_Int32                       nMin;
    sal_Int32                       nMax;

    bool                            bComment;
    bool                            bIsNum;
    bool                            bCanNum;

public:
    ImpItemListRow()
    :   eState(SfxItemState::UNKNOWN),
        nWhichId(0),
        pType(nullptr),
        eItemType(ITEM_DONTKNOW),
        nVal(0),
        nMin(0),
        nMax(0),
        bComment(false),
        bIsNum(false),
        bCanNum(false)
    {}

    OUString GetItemTypeStr() const;
    bool operator==(const ImpItemListRow& rEntry) const;
    bool operator!=(const ImpItemListRow& rEntry) const { return !operator==(rEntry); }
};

OUString ImpItemListRow::GetItemTypeStr() const
{
    switch(eItemType)
    {
        case ITEM_BYTE      : return OUString("Byte");     break;
        case ITEM_INT16     : return OUString("Int16");    break;
        case ITEM_UINT16    : return OUString("UInt16");   break;
        case ITEM_INT32     : return OUString("Int32");    break;
        case ITEM_UINT32    : return OUString("UInt32");   break;
        case ITEM_ENUM      : return OUString("Enum");     break;
        case ITEM_BOOL      : return OUString("Bool");     break;
        case ITEM_FLAG      : return OUString("Flag");     break;
        case ITEM_STRING    : return OUString("String");   break;
        case ITEM_POINT     : return OUString("Point");    break;
        case ITEM_RECT      : return OUString("Rectangle");break;
        case ITEM_RANGE     : return OUString("Range");    break;
        case ITEM_LRANGE    : return OUString("LRange");   break;
        case ITEM_FRACTION  : return OUString("Fraction"); break;
        case ITEM_XCOLOR    : return OUString("XColor");   break;
        case ITEM_COLOR     : return OUString("Color");    break;
        case ITEM_FONT      : return OUString("Font");     break;
        case ITEM_FONTHEIGHT: return OUString("FontHeight");break;
        case ITEM_FONTWIDTH : return OUString("FontWidth"); break;
        case ITEM_FIELD     : return OUString("Field");     break;
        default: break;
    }

    return OUString();
}

bool ImpItemListRow::operator==(const ImpItemListRow& rEntry) const
{
    return (aName.equals(rEntry.aName)
        && aValue.equals(rEntry.aValue)
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


class ImpItemEdit: public Edit
{
    VclPtr<SdrItemBrowserControl>     pBrowse;

public:
    ImpItemEdit(vcl::Window* pParent, SdrItemBrowserControl* pBrowse_, WinBits nBits=0)
    :   Edit(pParent, nBits),
        pBrowse(pBrowse_)
    {}
    virtual ~ImpItemEdit() override { disposeOnce(); }
    virtual void dispose() override { pBrowse.clear(); Edit::dispose(); }
    virtual void KeyInput(const KeyEvent& rEvt) override;
};

void ImpItemEdit::KeyInput(const KeyEvent& rKEvt)
{
    SdrItemBrowserControl* pBrowseMerk = pBrowse;

    sal_uInt16 nKeyCode(rKEvt.GetKeyCode().GetCode() + rKEvt.GetKeyCode().GetModifier());

    if(nKeyCode == KEY_RETURN)
    {
        pBrowseMerk->EndChangeEntry();
        pBrowseMerk->GrabFocus();
    }
    else if(nKeyCode == KEY_ESCAPE)
    {
        pBrowseMerk->BreakChangeEntry();
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

// - SdrItemBrowserControl -

#define MYBROWSEMODE (BrowserMode::THUMBDRAGGING|BrowserMode::KEEPHIGHLIGHT|BrowserMode::NO_HSCROLL|BrowserMode::HIDECURSOR)

SdrItemBrowserControl::SdrItemBrowserControl(vcl::Window* pParent):
    BrowseBox(pParent, WB_3DLOOK | WB_BORDER | WB_TABSTOP, MYBROWSEMODE),
    aList()
{
    ImpCtor();
}

SdrItemBrowserControl::~SdrItemBrowserControl()
{
    disposeOnce();
}

void SdrItemBrowserControl::dispose()
{
    pEditControl.disposeAndClear();

    delete pAktChangeEntry;

    Clear();
    BrowseBox::dispose();
}

void SdrItemBrowserControl::ImpCtor()
{
    pEditControl = nullptr;
    pAktChangeEntry = nullptr;
    nLastWhichOfs = 0;
    nLastWhich = 0;
    nLastWhichOben = 0;  // not implemented yet
    nLastWhichUnten = 0; // not implemented yet
    bWhichesButNames = false;
    bDontHideIneffectiveItems = false;
    bDontSortItems = false;
    bShowWhichIds = false;
    bShowRealValues = false;
    bShowWhichIds = true;   // not implemented yet
    bShowRealValues = true; // not implemented yet

    InsertDataColumn(
        ITEMBROWSER_WHICHCOL_ID,
        "Which",
        GetTextWidth(" Which ") + 2);
    InsertDataColumn(
        ITEMBROWSER_STATECOL_ID,
        "State",
        std::max(GetTextWidth(" State ") + 2 ,
            GetTextWidth("DontCare") + 2));
    InsertDataColumn(
        ITEMBROWSER_TYPECOL_ID ,
        "Type",
        GetTextWidth(" Type_ ") + 2);
    InsertDataColumn(
        ITEMBROWSER_NAMECOL_ID ,
        "Name",
        150);
    InsertDataColumn(
        ITEMBROWSER_VALUECOL_ID,
        "Value",
        GetTextWidth("12345678901234567890"));
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

void SdrItemBrowserControl::Clear()
{
    const std::size_t nCount=aList.size();
    for (std::size_t nNum=0; nNum<nCount; ++nNum) {
        delete ImpGetEntry(nNum);
    }
    aList.clear();
    BrowseBox::Clear();
}

long SdrItemBrowserControl::GetRowCount() const
{
    return aList.size();
}

bool SdrItemBrowserControl::SeekRow(long nRow)
{
    nAktPaintRow = nRow;
    return true;
}

OUString SdrItemBrowserControl::GetCellText(long _nRow, sal_uInt16 _nColId) const
{
    OUString sRet;
    if ( _nRow >= 0 && _nRow < (sal_Int32)aList.size() )
    {
        ImpItemListRow* pEntry = ImpGetEntry(_nRow);
        if ( pEntry )
        {
            if ( pEntry->bComment )
            {
                if (_nColId == ITEMBROWSER_NAMECOL_ID)
                    sRet = pEntry->aName;
            }
            else
            {
                sRet = "???";
                switch (_nColId)
                {
                    case ITEMBROWSER_WHICHCOL_ID:
                        sRet = OUString::number( pEntry->nWhichId ); break;
                    case ITEMBROWSER_STATECOL_ID:
                    {
                        switch (pEntry->eState)
                        {
                            case SfxItemState::UNKNOWN : sRet = "Unknown";  break;
                            case SfxItemState::DISABLED: sRet = "Disabled"; break;
                            case SfxItemState::DONTCARE: sRet = "DontCare"; break;
                            case SfxItemState::SET     : sRet = "Set";      break;
                            case SfxItemState::DEFAULT : sRet = "Default";  break;
                            case SfxItemState::READONLY: sRet = "ReadOnly";  break;
                        } // switch
                    } break;
                    case ITEMBROWSER_TYPECOL_ID: sRet = pEntry->GetItemTypeStr(); break;
                    case ITEMBROWSER_NAMECOL_ID: sRet = pEntry->aName; break;
                    case ITEMBROWSER_VALUECOL_ID: sRet = pEntry->aValue; break;
                } // switch
            }
        }
    }
    return sRet;
}

void SdrItemBrowserControl::PaintField(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId) const
{
    if (nAktPaintRow<0 || static_cast<std::size_t>(nAktPaintRow)>=aList.size()) {
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
        rDev.SetClipRegion(vcl::Region(aR));
        rDev.DrawText(aR.TopLeft(),GetCellText(nAktPaintRow,nColumnId));
        rDev.SetClipRegion();
    }
}

std::size_t SdrItemBrowserControl::GetCurrentPos() const
{
    std::size_t nRet=ITEM_NOT_FOUND;
    if (GetSelectRowCount()==1) {
        long nPos=static_cast<BrowseBox*>(const_cast<SdrItemBrowserControl *>(this))->FirstSelectedRow();
        if (nPos>=0 && static_cast<std::size_t>(nPos)<aList.size()) {
            nRet = static_cast<std::size_t>(nPos);
        }
    }
    return nRet;
}

sal_uInt16 SdrItemBrowserControl::GetCurrentWhich() const
{
    sal_uInt16 nRet=0;
    const std::size_t nPos=GetCurrentPos();
    if (nPos!=ITEM_NOT_FOUND) {
        nRet=ImpGetEntry(nPos)->nWhichId;
    }
    return nRet;
}

void SdrItemBrowserControl::DoubleClick(const BrowserMouseEvent&)
{
    const std::size_t nPos=GetCurrentPos();
    if (nPos!=ITEM_NOT_FOUND) {
        BeginChangeEntry(nPos);
    }
}

void SdrItemBrowserControl::KeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16 nKeyCode=rKEvt.GetKeyCode().GetCode()+rKEvt.GetKeyCode().GetModifier();
    bool bAusgewertet = false;
    const std::size_t nPos=GetCurrentPos();
    if (nPos!=ITEM_NOT_FOUND) {
        if (nKeyCode==KEY_RETURN) {
            if (BeginChangeEntry(nPos)) bAusgewertet = true;
        } else if (nKeyCode==KEY_ESCAPE) {

        } else if (rKEvt.GetKeyCode().GetModifier()==KEY_SHIFT+KEY_MOD1+KEY_MOD2) { // Ctrl
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
    }
    if (!bAusgewertet) BrowseBox::KeyInput(rKEvt);
}

void SdrItemBrowserControl::SetDirty()
{
    aSetDirtyHdl.Call(*this);
}

Rectangle SdrItemBrowserControl::GetFieldCharacterBounds(sal_Int32 /*_nRow*/,sal_Int32 /*_nColumnPos*/,sal_Int32 /*nIndex*/)
{
    // no accessibility implementation required
    return Rectangle();
}

sal_Int32 SdrItemBrowserControl::GetFieldIndexAtPoint(sal_Int32 /*_nRow*/,sal_Int32 /*_nColumnPos*/,const Point& /*_rPoint*/)
{
    // no accessibility implementation required
    return -1;
}

void SdrItemBrowserControl::Select()
{
    EndChangeEntry();
    BrowseBox::Select();
    ImpSaveWhich();
}

void SdrItemBrowserControl::ImpSaveWhich()
{
    sal_uInt16 nWh=GetCurrentWhich();
    if (nWh!=0) {
        long nPos=GetCurrentPos();
        long nTop=GetTopRow();
        long nBtm=GetTopRow()+GetVisibleRows()+1;
        nLastWhich=nWh;
        nLastWhichOfs=nPos-nTop;
        if (nTop<0) nTop=0;
        if (nBtm>=(long)aList.size()) nBtm=aList.size()-1;
        nLastWhichOben=ImpGetEntry(nTop)->nWhichId;
        nLastWhichUnten=ImpGetEntry(nBtm)->nWhichId;
    }
}

void SdrItemBrowserControl::ImpRestoreWhich()
{
    if (nLastWhich!=0) {
        bool bFnd = false;
        const std::size_t nCount=aList.size();
        std::size_t nNum;
        for (nNum=0; nNum<nCount && !bFnd; nNum++) {
            ImpItemListRow* pEntry=ImpGetEntry(nNum);
            if (!pEntry->bComment) {
                sal_uInt16 nWh=pEntry->nWhichId;
                if (nWh==nLastWhich) bFnd = true;
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

bool SdrItemBrowserControl::BeginChangeEntry(std::size_t nPos)
{
    BreakChangeEntry();
    bool bRet = false;
    ImpItemListRow* pEntry=ImpGetEntry(nPos);
    if (pEntry!=nullptr && !pEntry->bComment) {
        SetMode(MYBROWSEMODE & BrowserMode(~BrowserMode::KEEPHIGHLIGHT));
        pEditControl=VclPtr<ImpItemEdit>::Create(&GetDataWindow(),this,0);
        Rectangle aRect(GetFieldRectPixel(nPos, ITEMBROWSER_VALUECOL_ID, false));
        aRect.Left()+=2; // little offset for the Edit, so it's exact to the pixel
        aRect.Right()--;
        pEditControl->SetPosSizePixel(aRect.TopLeft(),aRect.GetSize());
        pEditControl->SetText(pEntry->aValue);
        pEditControl->SetBackground( Wallpaper(Color(COL_LIGHTGRAY)));
        vcl::Font aFont(pEditControl->GetFont());
        aFont.SetFillColor(Color(COL_LIGHTGRAY));
        pEditControl->SetFont(aFont);
        pEditControl->Show();
        pEditControl->GrabFocus();
        pEditControl->SetSelection(Selection(SELECTION_MIN,SELECTION_MAX));
        vcl::Window* pParent=GetParent();
        aWNamMerk=pParent->GetText();
        OUString aNeuNam(aWNamMerk);
        aNeuNam += " ";
        aNeuNam += pEntry->GetItemTypeStr();
        if (pEntry->bCanNum) {
            aNeuNam += ": ";
            aNeuNam += OUString::number(pEntry->nMin);
            aNeuNam += "..";
            aNeuNam += OUString::number(pEntry->nMax);
        }
        aNeuNam += " - Type 'del' to reset to default.";
        pParent->SetText(aNeuNam);
        pAktChangeEntry=new ImpItemListRow(*pEntry);
        bRet = true;
    }
    return bRet;
}

void SdrItemBrowserControl::EndChangeEntry()
{
    if (!pEditControl)
        return;

    aEntryChangedHdl.Call(*this);
    BreakChangeEntry();
}

void SdrItemBrowserControl::BreakChangeEntry()
{
    if (pEditControl!=nullptr) {
        pEditControl.disposeAndClear();
        delete pAktChangeEntry;
        pAktChangeEntry=nullptr;
        vcl::Window* pParent=GetParent();
        pParent->SetText(aWNamMerk);
        SetMode(MYBROWSEMODE);
    }
}

void SdrItemBrowserControl::ImpSetEntry(const ImpItemListRow& rEntry, std::size_t nEntryNum)
{
    SAL_WARN_IF(nEntryNum > aList.size(), "svx", "trying to set item " << nEntryNum << "in a vector of size " << aList.size());
    if (nEntryNum >= aList.size()) {
        nEntryNum = aList.size();
        aList.push_back(new ImpItemListRow(rEntry));
        RowInserted(nEntryNum);
    } else {
        ImpItemListRow* pAktEntry=ImpGetEntry(nEntryNum);
        if (*pAktEntry!=rEntry) {
            bool bStateDiff=rEntry.eState!=pAktEntry->eState;
            bool bValueDiff=!rEntry.aValue.equals(pAktEntry->aValue);
            bool bAllDiff = true;
            if (bStateDiff || bValueDiff) {
                // check whether only state and/or value have changed
                ImpItemListRow aTest(rEntry);
                aTest.eState=pAktEntry->eState;
                aTest.aValue=pAktEntry->aValue;
                if (aTest==*pAktEntry) bAllDiff = false;
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
}

bool ImpGetItem(const SfxItemSet& rSet, sal_uInt16 nWhich, const SfxPoolItem*& rpItem)
{
    SfxItemState eState=rSet.GetItemState(nWhich,true,&rpItem);
    if (eState==SfxItemState::DEFAULT) {
        rpItem=&rSet.Get(nWhich);
    }
    return (eState==SfxItemState::DEFAULT || eState==SfxItemState::SET) && rpItem!=nullptr;
}

bool IsItemIneffective(sal_uInt16 nWhich, const SfxItemSet* pSet, sal_uInt16& rIndent)
{
    rIndent=0;
    if (pSet==nullptr) return false;
    const SfxPoolItem* pItem=nullptr;
    bool bRet = false;
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
                drawing::LineStyle eLineStyle=static_cast<const XLineStyleItem*>(pItem)->GetValue();
                if (eLineStyle==drawing::LineStyle_NONE) return true;
                if (eLineStyle!=drawing::LineStyle_DASH && nWhich==XATTR_LINEDASH) return true;
            }
            if (nWhich==XATTR_LINESTART || nWhich==XATTR_LINESTARTCENTER) {
                rIndent=2;
                if (ImpGetItem(*pSet,XATTR_LINESTARTWIDTH,pItem)) {
                    sal_Int32 nWdt=static_cast<const XLineStartWidthItem*>(pItem)->GetValue();
                    if (nWdt==0) return true;
                }
            }
            if (nWhich==XATTR_LINEEND || nWhich==XATTR_LINEENDCENTER) {
                rIndent=2;
                if (ImpGetItem(*pSet,XATTR_LINEENDWIDTH,pItem)) {
                    sal_Int32 nWdt=static_cast<const XLineEndWidthItem*>(pItem)->GetValue();
                    if (nWdt==0) return true;
                }
            }
        } break;
        case XATTR_FILLCOLOR           : /* only for Style=Color */
        case XATTR_FILLGRADIENT        : /* only for Style=Gradient */
        case XATTR_FILLHATCH           : /* only for Style=Hatch */
        case XATTR_FILLTRANSPARENCE    : /* only for Style=Color */
        case XATTR_GRADIENTSTEPCOUNT   : /* only for Style=Gradient */
        case XATTR_FILLBACKGROUND      : /* only for Style=Hatch */
        {
            rIndent=1;
            if (ImpGetItem(*pSet,XATTR_FILLSTYLE,pItem)) {
                drawing::FillStyle eFillStyle=static_cast<const XFillStyleItem*>(pItem)->GetValue();
                if (eFillStyle==drawing::FillStyle_NONE) return true;
                // transparency currently only for SolidFill
                if (eFillStyle!=drawing::FillStyle_SOLID && (nWhich==XATTR_FILLCOLOR || nWhich==XATTR_FILLTRANSPARENCE)) return true;
                if (eFillStyle!=drawing::FillStyle_GRADIENT && (nWhich==XATTR_FILLGRADIENT || nWhich==XATTR_GRADIENTSTEPCOUNT)) return true;
                if (eFillStyle!=drawing::FillStyle_HATCH && (nWhich==XATTR_FILLHATCH || nWhich==XATTR_FILLBACKGROUND)) return true;
            }
        } break;
        case XATTR_FILLBITMAP          :
        case XATTR_FILLBMP_TILE        :
        case XATTR_FILLBMP_POS         : /* currently only if TILE=sal_True */
        case XATTR_FILLBMP_SIZEX       : /* only if not Stretch */
        case XATTR_FILLBMP_SIZEY       : /* only if not Stretch */
        case XATTR_FILLBMP_SIZELOG     : /* only if SIZELOG=sal_False to reset to sal_True (old) -> but is still in use */
        case XATTR_FILLBMP_TILEOFFSETX : /* only if TILE=sal_True */
        case XATTR_FILLBMP_TILEOFFSETY : /* only if TILE=sal_True */
        case XATTR_FILLBMP_STRETCH     : /* only if TILE=sal_False */
        case XATTR_FILLBMP_POSOFFSETX  : /* only if TILE=sal_True*/
        case XATTR_FILLBMP_POSOFFSETY  : { /* only if TILE=sal_True*/
            rIndent=1;
            if (ImpGetItem(*pSet,XATTR_FILLSTYLE,pItem)) {
                drawing::FillStyle eFillStyle=static_cast<const XFillStyleItem*>(pItem)->GetValue();
                if (eFillStyle!=drawing::FillStyle_BITMAP) return true;
            }
            if (nWhich==XATTR_FILLBITMAP || nWhich==XATTR_FILLBMP_TILE) {
                return false; // always selectable
            }
            bool bTileTRUE = false;
            bool bTileFALSE = false;
            bool bStretchTRUE = false;
            if (ImpGetItem(*pSet,XATTR_FILLBMP_TILE,pItem)) {
                bTileTRUE=static_cast<const XFillBmpTileItem*>(pItem)->GetValue();
                bTileFALSE=!bTileTRUE;
            }
            if (ImpGetItem(*pSet,XATTR_FILLBMP_STRETCH,pItem)) {
                bStretchTRUE=static_cast<const XFillBmpStretchItem*>(pItem)->GetValue();
            }
            // Stretch not selectable if Tile=TRUE
            if (nWhich==XATTR_FILLBMP_STRETCH) return bTileTRUE;
            // and 7+1 items (sub-attributes) remain
            rIndent=2;
            // Pos (enum) not selectable if Tile=FALSE
            if (nWhich==XATTR_FILLBMP_POS) return bTileFALSE;
            // SizeXY not selectable if Stretch=TRUE
            if (nWhich==XATTR_FILLBMP_SIZEX || nWhich==XATTR_FILLBMP_SIZEY) {
                return bTileFALSE && bStretchTRUE;
            }
            // 2 items specially for Tile
            if (nWhich==XATTR_FILLBMP_POSOFFSETX  || nWhich==XATTR_FILLBMP_POSOFFSETY) {
                return bTileFALSE;
            }
            // another 2 items specially for Tile -- however, these exclude each other
            if (nWhich==XATTR_FILLBMP_TILEOFFSETX || nWhich==XATTR_FILLBMP_TILEOFFSETY) {
                if (bTileFALSE) return true;
                sal_uInt16 nX=0,nY=0;
                bool bX = false,bY = false;
                if (ImpGetItem(*pSet,XATTR_FILLBMP_TILEOFFSETX,pItem)) {
                    nX=static_cast<const XFillBmpTileOffsetXItem*>(pItem)->GetValue();
                    bX = true;
                }
                if (ImpGetItem(*pSet,XATTR_FILLBMP_TILEOFFSETY,pItem)) {
                    nY=static_cast<const XFillBmpTileOffsetYItem*>(pItem)->GetValue();
                    bY = true;
                }
                if (nWhich==XATTR_FILLBMP_TILEOFFSETX) {
                    if (nX!=0 || !bX) return false;
                    if (nY!=0) return true;
                } else {
                    if (nY!=0 || !bY) return false;
                    if (nX!=0) return true;
                }
            }
            // SizeLog not selectable if Stretch=TRUE and/or
            // if SizeLog=sal_False.
            // -> apparently still in use
            // (sal_True is the static PoolDefault)
            if (nWhich==XATTR_FILLBMP_SIZELOG) {
                if (bTileFALSE && bStretchTRUE) return true;
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
        case XATTR_FORMTXTHIDEFORM  :
        case XATTR_FORMTXTSHDWTRANSP: {
            rIndent=1;
            if (ImpGetItem(*pSet,XATTR_FORMTXTSTYLE,pItem)) {
                XFormTextStyle eStyle=static_cast<const XFormTextStyleItem*>(pItem)->GetValue();
                if (eStyle==XFormTextStyle::NONE) return true;
            }
            if ((nWhich>=XATTR_FORMTXTSHDWCOLOR && nWhich<=XATTR_FORMTXTSHDWYVAL) || nWhich>=XATTR_FORMTXTSHDWTRANSP) {
                rIndent=2;
                if (ImpGetItem(*pSet,XATTR_FORMTXTSHADOW,pItem)) {
                    XFormTextShadow eShadow=static_cast<const XFormTextShadowItem*>(pItem)->GetValue();
                    if (eShadow==XFormTextShadow::NONE) return true;
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
                bool bShadow=static_cast<const SdrOnOffItem*>(pItem)->GetValue();
                if (!bShadow) return true;
            }
        } break;

        case SDRATTR_CAPTIONANGLE: {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_CAPTIONFIXEDANGLE,pItem)) {
                bool bFixed=static_cast<const SdrOnOffItem*>(pItem)->GetValue();
                if (!bFixed) return true;
            }
        } break;
        case SDRATTR_CAPTIONESCREL:
        case SDRATTR_CAPTIONESCABS: {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_CAPTIONESCISREL,pItem)) {
                bool bRel=static_cast<const SdrCaptionEscIsRelItem*>(pItem)->GetValue();
                if (bRel && nWhich==SDRATTR_CAPTIONESCABS) return true;
                if (!bRel && nWhich==SDRATTR_CAPTIONESCREL) return true;
            }
        } break;
        case SDRATTR_CAPTIONLINELEN: {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_CAPTIONFITLINELEN,pItem)) {
                bool bFit=static_cast<const SdrCaptionFitLineLenItem*>(pItem)->GetValue();
                if (bFit) return true;
            }
        } break;

        case SDRATTR_TEXT_MINFRAMEHEIGHT:
        case SDRATTR_TEXT_MAXFRAMEHEIGHT: {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_TEXT_AUTOGROWHEIGHT,pItem)) {
                bool bAutoGrow=static_cast<const SdrOnOffItem*>(pItem)->GetValue();
                if (!bAutoGrow) return true;
            }
        } break;
        case SDRATTR_TEXT_MINFRAMEWIDTH:
        case SDRATTR_TEXT_MAXFRAMEWIDTH: {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_TEXT_AUTOGROWWIDTH,pItem)) {
                bool bAutoGrow=static_cast<const SdrOnOffItem*>(pItem)->GetValue();
                if (!bAutoGrow) return true;
            }
        } break;
        case SDRATTR_TEXT_VERTADJUST:
        case SDRATTR_TEXT_HORZADJUST: {
            if (ImpGetItem(*pSet,SDRATTR_TEXT_FITTOSIZE,pItem)) {
                SdrFitToSizeType eFit=static_cast<const SdrTextFitToSizeTypeItem*>(pItem)->GetValue();
                if (eFit!=SdrFitToSizeType::NONE) return true;
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
                SdrTextAniKind eAniKind=static_cast<const SdrTextAniKindItem*>(pItem)->GetValue();
                if (eAniKind==SDRTEXTANI_NONE) return true;
                if (eAniKind==SDRTEXTANI_BLINK && (nWhich==SDRATTR_TEXT_ANIDIRECTION || nWhich==SDRATTR_TEXT_ANIAMOUNT)) return true;
                if (eAniKind==SDRTEXTANI_SLIDE && (nWhich==SDRATTR_TEXT_ANISTARTINSIDE || nWhich==SDRATTR_TEXT_ANISTOPINSIDE)) return true;
            }
        } break;

        case SDRATTR_EDGELINEDELTAANZ: return true;
        case SDRATTR_EDGELINE1DELTA:
        case SDRATTR_EDGELINE2DELTA:
        case SDRATTR_EDGELINE3DELTA: {
            if (ImpGetItem(*pSet,SDRATTR_EDGEKIND,pItem)) {
                SdrEdgeKind eKind=static_cast<const SdrEdgeKindItem*>(pItem)->GetValue();
                if (eKind==SdrEdgeKind::ThreeLines) {
                    if (nWhich>SDRATTR_EDGELINE2DELTA) return true;
                    else return false;
                }
                if (eKind!=SdrEdgeKind::OrthoLines && eKind!=SdrEdgeKind::Bezier) return true;
            }
            if (ImpGetItem(*pSet,SDRATTR_EDGELINEDELTAANZ,pItem)) {
                sal_uInt16 nCount=static_cast<const SdrEdgeLineDeltaCountItem*>(pItem)->GetValue();
                if (nCount==0) return true;
                if (nCount==1 && nWhich>SDRATTR_EDGELINE1DELTA) return true;
                if (nCount==2 && nWhich>SDRATTR_EDGELINE2DELTA) return true;
            }
        } break;

        case SDRATTR_CIRCSTARTANGLE:
        case SDRATTR_CIRCENDANGLE  : {
            rIndent=1;
            if (ImpGetItem(*pSet,SDRATTR_CIRCKIND,pItem)) {
                SdrCircKind eKind=static_cast<const SdrCircKindItem*>(pItem)->GetValue();
                if (eKind==SDRCIRC_FULL) return true;
            }
        } break;
    } // switch
    return bRet;
}

sal_uInt16 ImpSortWhich(sal_uInt16 nWhich)
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
        case XATTR_FORMTXTSHADOW        : nWhich=XATTR_FORMTXTSHDWXVAL      ; break;
        case XATTR_FORMTXTSHDWCOLOR     : nWhich=XATTR_FORMTXTHIDEFORM      ; break;
        case XATTR_FORMTXTSHDWXVAL      : nWhich=XATTR_FORMTXTSHADOW        ; break;
        case XATTR_FORMTXTSHDWYVAL      : nWhich=XATTR_FORMTXTSHDWCOLOR     ; break;
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

void SdrItemBrowserControl::SetAttributes(const SfxItemSet* pSet, const SfxItemSet* p2ndSet)
{
    SetMode(MYBROWSEMODE & BrowserMode(~BrowserMode::KEEPHIGHLIGHT));
    if (pSet!=nullptr) {
        sal_uInt16 nEntryNum=0;
        SfxWhichIter aIter(*pSet);
        const SfxItemPool* pPool=pSet->GetPool();
        sal_uInt16 nWhich0=0;
        sal_uInt16 nWhich=aIter.FirstWhich();
        while (nWhich!=0) {
            // Now sort this a little bit differently.
            // Only works as long as there are no InvalidItems, i. e. no gaps
            // at this position in the Set.
            if (!bDontSortItems) nWhich=ImpSortWhich(nWhich);
            SfxItemState eState=pSet->GetItemState(nWhich);
            if (p2ndSet!=nullptr) {
                SfxItemState e2ndState=p2ndSet->GetItemState(nWhich);
                if (eState==SfxItemState::DEFAULT) eState=SfxItemState::DISABLED;
                else if (e2ndState==SfxItemState::DEFAULT) eState=SfxItemState::DEFAULT;
            }
            if (eState!=SfxItemState::DISABLED) {
                const SfxPoolItem& rItem=pSet->Get(nWhich);
                sal_uInt16 nIndent=0;
                if (dynamic_cast<const SfxVoidItem *>(&rItem) == nullptr && dynamic_cast<const SfxSetItem *>(&rItem) == nullptr
                        && (!IsItemIneffective(nWhich,pSet,nIndent) || bDontHideIneffectiveItems)) {
                    OUString aCommentStr;

                    INSERTCOMMENT(XATTR_LINE_FIRST,XATTR_LINE_LAST,"L I N E");
                    INSERTCOMMENT(XATTR_FILL_FIRST,XATTR_FILL_LAST,"F I L L");
                    INSERTCOMMENT(XATTR_TEXT_FIRST,XATTR_TEXT_LAST,"F O N T W O R K");
                    INSERTCOMMENT(SDRATTR_SHADOW_FIRST,SDRATTR_SHADOW_LAST,"S H A D O W");
                    INSERTCOMMENT(SDRATTR_CAPTION_FIRST,SDRATTR_CAPTION_LAST,"C A P T I O N");
                    INSERTCOMMENT(SDRATTR_MISC_FIRST,SDRATTR_MISC_LAST,"M I S C E L L A N E O U S");
                    INSERTCOMMENT(SDRATTR_EDGE_FIRST,SDRATTR_EDGE_LAST,"C O N N E C T O R");
                    INSERTCOMMENT(SDRATTR_MEASURE_FIRST,SDRATTR_MEASURE_LAST,"D I M E N S I O N");
                    INSERTCOMMENT(SDRATTR_CIRC_FIRST,SDRATTR_CIRC_LAST,"C I R C U L A R");
                    INSERTCOMMENT(SDRATTR_NOTPERSIST_FIRST,SDRATTR_NOTPERSIST_LAST,"N O T P E R S I S T");
                    INSERTCOMMENT(SDRATTR_MOVEX,SDRATTR_VERTSHEARONE,"Transformation of all object individually");
                    INSERTCOMMENT(SDRATTR_RESIZEXALL,SDRATTR_VERTSHEARALL,"Transformation of all objects together");
                    INSERTCOMMENT(SDRATTR_TRANSFORMREF1X,SDRATTR_TRANSFORMREF2Y,"V I E W R E F E R E N C E P O I N T S");
                    INSERTCOMMENT(SDRATTR_GRAF_FIRST,SDRATTR_GRAF_LAST,"G R A P H I C");
                    INSERTCOMMENT(EE_ITEMS_START,EE_ITEMS_END,"E D I T  E N G I N E");
                    INSERTCOMMENT(EE_ITEMS_END+1,EE_ITEMS_END+1,"... by Joe Merten, JME Engineering Berlin ...");

                    if (!aCommentStr.isEmpty())
                    {
                        ImpItemListRow aEntry;
                        aEntry.bComment = true;
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
                        aEntry.aName = " " + aEntry.aName;
                        nIndent--;
                    }

                    aEntry.eState=eState;
                    aEntry.nWhichId=nWhich;
                    if (!IsInvalidItem(&rItem)) {
                        aEntry.pType=&typeid(rItem);
                        aEntry.nMax=0x7FFFFFFF;
                        aEntry.nMin=-aEntry.nMax;
                        aEntry.nVal=-4711;
                        if      (dynamic_cast<const SfxByteItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_BYTE;
                        else if (dynamic_cast<const SfxInt16Item *>(&rItem) != nullptr) aEntry.eItemType=ITEM_INT16;
                        else if (dynamic_cast<const SfxUInt16Item *>(&rItem) != nullptr) aEntry.eItemType=ITEM_UINT16;
                        else if (dynamic_cast<const SfxInt32Item *>(&rItem) != nullptr) aEntry.eItemType=ITEM_INT32;
                        else if (dynamic_cast<const SfxUInt32Item *>(&rItem) != nullptr) aEntry.eItemType=ITEM_UINT32;
                        else if (dynamic_cast<const SfxEnumItemInterface *>(&rItem) != nullptr) aEntry.eItemType=ITEM_ENUM;
                        else if (dynamic_cast<const SfxBoolItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_BOOL;
                        else if (dynamic_cast<const SfxFlagItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_FLAG;
                        else if (dynamic_cast<const XColorItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_XCOLOR;
                        else if (dynamic_cast<const SfxStringItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_STRING;
                        else if (dynamic_cast<const SfxPointItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_POINT;
                        else if (dynamic_cast<const SfxRectangleItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_RECT;
                        else if (dynamic_cast<const SfxRangeItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_RANGE;
                        else if (dynamic_cast<const SdrFractionItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_FRACTION;
                        else if (dynamic_cast<const SvxColorItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_COLOR;
                        else if (dynamic_cast<const SvxFontItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_FONT;
                        else if (dynamic_cast<const SvxFontHeightItem *>(&rItem) != nullptr)aEntry.eItemType=ITEM_FONTHEIGHT;
                        else if (dynamic_cast<const SvxCharScaleWidthItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_FONTWIDTH;
                        else if (dynamic_cast<const SvxFieldItem *>(&rItem) != nullptr) aEntry.eItemType=ITEM_FIELD;
                        switch (aEntry.eItemType) {
                            case ITEM_BYTE      : aEntry.bIsNum = true;  aEntry.nVal=static_cast<const SfxByteItem  &>(rItem).GetValue(); aEntry.nMin=0;      aEntry.nMax=255;   break;
                            case ITEM_INT16     : aEntry.bIsNum = true;  aEntry.nVal=static_cast<const SfxInt16Item &>(rItem).GetValue(); aEntry.nMin=-32767; aEntry.nMax=32767; break;
                            case ITEM_UINT16    : aEntry.bIsNum = true;  aEntry.nVal=static_cast<const SfxUInt16Item&>(rItem).GetValue(); aEntry.nMin=0;      aEntry.nMax=65535; break;
                            case ITEM_INT32     : aEntry.bIsNum = true;  aEntry.nVal=static_cast<const SfxInt32Item &>(rItem).GetValue();                                        break;
                            case ITEM_UINT32    : aEntry.bIsNum = true;  aEntry.nVal=static_cast<const SfxUInt32Item&>(rItem).GetValue(); aEntry.nMin=0; /*aEntry.nMax=0xFF...*/;break;
                            case ITEM_ENUM      : aEntry.bCanNum = true; aEntry.nVal=static_cast<const SfxEnumItemInterface&>(rItem).GetEnumValue(); aEntry.nMin=0; aEntry.nMax=static_cast<const SfxEnumItemInterface&>(rItem).GetValueCount()-1; break;
                            case ITEM_BOOL      : aEntry.bCanNum = true; aEntry.nVal=sal_Int32(static_cast<const SfxBoolItem  &>(rItem).GetValue()); aEntry.nMin=0; aEntry.nMax=1;          break;
                            case ITEM_FLAG      : aEntry.bCanNum = true; aEntry.nVal=static_cast<const SfxFlagItem  &>(rItem).GetValue(); aEntry.nMin=0; aEntry.nMax=0xFFFF;     break;
                            case ITEM_FONTHEIGHT: aEntry.bCanNum = true; aEntry.nVal=static_cast<const SvxFontHeightItem&>(rItem).GetHeight(); aEntry.nMin=0;                    break;
                            case ITEM_FONTWIDTH : aEntry.bCanNum = true; aEntry.nVal=static_cast<const SvxCharScaleWidthItem&>(rItem).GetValue();    aEntry.nMin=0; aEntry.nMax=0xFFFF;break;
                            default: break;
                        } // switch
                        if (aEntry.bIsNum) aEntry.bCanNum = true;

                        rItem.GetPresentation(SFX_ITEM_PRESENTATION_NAMELESS,
                                              pPool->GetMetric(nWhich),
                                              MAP_MM, aEntry.aValue);
                        if (aEntry.bCanNum)
                        {
                            aEntry.aValue = OUString::number(aEntry.nVal) + ": " + aEntry.aValue;
                        }
                    }
                    else
                    {
                        aEntry.aValue = "InvalidItem";
                    }
                    ImpSetEntry(aEntry,nEntryNum);
                    nEntryNum++;
                }
            }
            nWhich=aIter.NextWhich();
        } // while

        if (aList.size()>nEntryNum) { // maybe still too many entries
            size_t const nTooMuch = aList.size() - nEntryNum;
            for (size_t n = nEntryNum; n < aList.size(); ++n) {
                delete aList[n];
            }
            aList.erase(aList.begin() + nEntryNum, aList.end());
            RowRemoved(nEntryNum,nTooMuch);
        }
    } else {
        Clear(); // if pSet==NULL
    }
    ImpRestoreWhich();
    SetMode(MYBROWSEMODE);
}

SdrItemBrowser::SdrItemBrowser(SdrView& rView):
    FloatingWindow(ImpGetViewWin(rView), WB_STDDOCKWIN|WB_3DLOOK|WB_CLOSEABLE|WB_SIZEMOVE),
    aBrowse(VclPtr<SdrItemBrowserControl>::Create(this)),
    aIdle("svx svdraw SdrItemBrowser"),
    pView(&rView),
    bDirty(false)
{
    SetOutputSizePixel(aBrowse->GetSizePixel());
    SetText("Joe's ItemBrowser");
    aBrowse->Show();
    aIdle.SetIdleHdl(LINK(this,SdrItemBrowser,IdleHdl));
    GetBrowserControl()->SetEntryChangedHdl(LINK(this,SdrItemBrowser,ChangedHdl));
    GetBrowserControl()->SetSetDirtyHdl(LINK(this,SdrItemBrowser,SetDirtyHdl));
    SetDirty();
}

SdrItemBrowser::~SdrItemBrowser()
{
    disposeOnce();
}

void SdrItemBrowser::dispose()
{
    aBrowse.disposeAndClear();
    FloatingWindow::dispose();
}

void SdrItemBrowser::Resize()
{
    aBrowse->SetSizePixel(GetOutputSizePixel());
}

void SdrItemBrowser::GetFocus()
{
    aBrowse->GrabFocus();
}

vcl::Window* SdrItemBrowser::ImpGetViewWin(SdrView& rView)
{
    const sal_uInt32 nWinCount(rView.PaintWindowCount());

    for(sal_uInt32 a(0L); a < nWinCount; a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);

        if(OUTDEV_WINDOW == pCandidate->GetOutputDevice().GetOutDevType())
        {
            return static_cast<vcl::Window*>(&pCandidate->GetOutputDevice());
        }
    }

    return nullptr;
}

void SdrItemBrowser::ForceParent()
{
    vcl::Window* pWin=ImpGetViewWin(*pView);
    if (pWin!=nullptr) SetParent(pWin);
}

void SdrItemBrowser::SetDirty()
{
    if (!bDirty) {
        bDirty = true;
        aIdle.SetPriority(SchedulerPriority::HIGH_IDLE);
        aIdle.Start();
    }
}

void SdrItemBrowser::Undirty()
{
    aIdle.Stop();
    bDirty = false;

    SfxItemSet aSet(pView->GetModel()->GetItemPool());
    pView->GetAttributes(aSet);

    if(pView->AreObjectsMarked())
    {
        SfxItemSet a2ndSet(pView->GetModel()->GetItemPool());
        pView->GetAttributes(a2ndSet, true);

        SetAttributes(&aSet,&a2ndSet);
    }
    else
    {
        SetAttributes(&aSet, nullptr);
    }
}

IMPL_LINK_NOARG_TYPED(SdrItemBrowser, IdleHdl, Idle *, void)
{
    Undirty();
}

IMPL_LINK_TYPED(SdrItemBrowser, ChangedHdl, SdrItemBrowserControl&, rBrowse, void)
{
    const ImpItemListRow* pEntry = rBrowse.GetAktChangeEntry();
    if (pEntry!=nullptr)
    {
        SfxItemSet aSet(pView->GetModel()->GetItemPool());
        pView->GetAttributes(aSet);

        SfxItemSet aNewSet(*aSet.GetPool(),pEntry->nWhichId,pEntry->nWhichId);
        OUString aNewText(rBrowse.GetNewEntryValue());
        bool bDel( aNewText == "del"
            || aNewText == "Del"
            || aNewText == "DEL"
            || aNewText == "default"
            || aNewText == "Default"
            || aNewText == "DEFAULT" );

        if (!bDel) {
            SfxPoolItem* pNewItem=aSet.Get(pEntry->nWhichId).Clone();
            sal_Int32 nLongVal = aNewText.toInt32();
            if (pEntry->bCanNum) {
                if (nLongVal>pEntry->nMax) nLongVal=pEntry->nMax;
                if (nLongVal<pEntry->nMin) nLongVal=pEntry->nMin;
            }
            bool bPairX = true;
            bool bPairY = false;
            sal_uInt16 nSepLen=1;
            long nLongX = aNewText.toInt32();
            long nLongY=0;
            sal_Int32 nPos = aNewText.indexOf('/');
            if (nPos==-1) nPos=aNewText.indexOf(':');
            if (nPos==-1) nPos=aNewText.indexOf(' ');
            if (nPos==-1) { nPos=aNewText.indexOf(".."); if (nPos!=-1) nSepLen=2; }
            if (nPos!=01)
            {
                bPairX=nPos>0;
                OUString s(aNewText.copy(nPos+nSepLen));
                bPairY = !aNewText.isEmpty();
                nLongY = s.toInt32();
            }
            switch (pEntry->eItemType) {
                case ITEM_BYTE  : static_cast<SfxByteItem  *>(pNewItem)->SetValue((sal_uInt8  )nLongVal); break;
                case ITEM_INT16 : static_cast<SfxInt16Item *>(pNewItem)->SetValue((sal_Int16 )nLongVal); break;
                case ITEM_UINT16: static_cast<SfxUInt16Item*>(pNewItem)->SetValue((sal_uInt16)nLongVal); break;
                case ITEM_INT32: {
                    if(dynamic_cast<const SdrAngleItem *>(pNewItem) != nullptr)
                    {
                        aNewText = aNewText.replace(',', '.');
                        double nVal = aNewText.toFloat();
                        nLongVal = static_cast<sal_Int32>(nVal * 100.0 + 0.5);
                    }
                    static_cast<SfxInt32Item *>(pNewItem)->SetValue(nLongVal);
                } break;
                case ITEM_UINT32: static_cast<SfxUInt32Item*>(pNewItem)->SetValue(aNewText.toInt32()); break;
                case ITEM_ENUM  : static_cast<SfxEnumItemInterface*>(pNewItem)->SetEnumValue((sal_uInt16)nLongVal); break;
                case ITEM_BOOL: {
                    aNewText = aNewText.toAsciiUpperCase();
                    if (aNewText == "TRUE") nLongVal=1;
                    if (aNewText == "JA") nLongVal=1;
                    if (aNewText == "AN") nLongVal=1;
                    if (aNewText == "EIN") nLongVal=1;
                    if (aNewText == "ON") nLongVal=1;
                    if (aNewText == "YES") nLongVal=1;
                    static_cast<SfxBoolItem*>(pNewItem)->SetValue(nLongVal == 1);
                } break;
                case ITEM_FLAG  : static_cast<SfxFlagItem  *>(pNewItem)->SetValue((sal_uInt16)nLongVal); break;
                case ITEM_STRING: static_cast<SfxStringItem*>(pNewItem)->SetValue(aNewText); break;
                case ITEM_POINT : static_cast<SfxPointItem*>(pNewItem)->SetValue(Point(nLongX,nLongY)); break;
                case ITEM_RECT  : break;
                case ITEM_RANGE : {
                    static_cast<SfxRangeItem*>(pNewItem)->From()=(sal_uInt16)nLongX;
                    static_cast<SfxRangeItem*>(pNewItem)->From()=(sal_uInt16)nLongY;
                } break;
                case ITEM_LRANGE : {
                } break;
                case ITEM_FRACTION: {
                    if (!bPairX) nLongX=1;
                    if (!bPairY) nLongY=1;
                    static_cast<SdrFractionItem*>(pNewItem)->SetValue(Fraction(nLongX,nLongY));
                } break;
                case ITEM_XCOLOR: break;
                case ITEM_COLOR: break;
                case ITEM_FONT: {
                    static_cast<SvxFontItem*>(pNewItem)->SetFamily( FAMILY_DONTKNOW );
                    static_cast<SvxFontItem*>(pNewItem)->SetFamilyName(aNewText);
                    static_cast<SvxFontItem*>(pNewItem)->SetStyleName(OUString());
                } break;
                case ITEM_FONTHEIGHT: {
                    sal_uInt32 nHgt=0;
                    sal_uInt16 nProp=100;
                    if (aNewText.indexOf('%') != -1) {
                        nProp=static_cast<sal_uInt16>(nLongVal);
                    } else {
                        nHgt=static_cast<sal_uInt32>(nLongVal);
                    }
                    static_cast<SvxFontHeightItem*>(pNewItem)->SetHeight(nHgt,nProp);
                } break;
                case ITEM_FONTWIDTH: {
                    sal_uInt16 nProp=100;
                    if (aNewText.indexOf('%') != -1) {
                        nProp=(sal_uInt16)nLongVal;
                    }
                    static_cast<SvxCharScaleWidthItem*>(pNewItem)->SetValue(nProp);
                } break;
                case ITEM_FIELD: break;
                default: break;
            } // switch
            aNewSet.Put(*pNewItem);
            delete pNewItem;
        }
        pView->SetAttributes(aNewSet,bDel);
    }
}

IMPL_LINK_NOARG_TYPED(SdrItemBrowser, SetDirtyHdl, SdrItemBrowserControl&, void)
{
    SetDirty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
