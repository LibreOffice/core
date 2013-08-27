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

#include <cstring>
#include <climits>

#include <tools/shl.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <svl/eitem.hxx>
#include <svl/rectitem.hxx>
#include <svl/smplhint.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/ruler.hxx>
#include <svx/rulritem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/protitem.hxx>

#include "rlrcitem.hxx"

#ifndef RULER_TAB_RTL
#define RULER_TAB_RTL           ((sal_uInt16)0x0010)
#endif

// STATIC DATA -----------------------------------------------------------

#define CTRL_ITEM_COUNT 14
#define GAP 10
#define OBJECT_BORDER_COUNT 4
#define TAB_GAP 1
#define INDENT_GAP 2
#define INDENT_FIRST_LINE   2
#define INDENT_LEFT_MARGIN  3
#define INDENT_RIGHT_MARGIN 4
#define INDENT_COUNT        3 //without the first two old values

#ifdef DEBUG_RULER
#include <vcl/lstbox.hxx>
class RulerDebugWindow : public Window
{
    ListBox aBox;
public:
        RulerDebugWindow(Window* pParent) :
            Window(pParent, WB_BORDER|WB_SIZEMOVE|WB_DIALOGCONTROL|WB_CLIPCHILDREN|WB_SYSTEMWINDOW),
            aBox(this, WB_BORDER)
            {
                Size aOutput(200, 400);
                SetOutputSizePixel(aOutput);
                aBox.SetSizePixel(aOutput);
                aBox.Show();
                Show();
                Size aParentSize(pParent->GetOutputSizePixel());
                Size aOwnSize(GetSizePixel());
                aParentSize.Width() -= aOwnSize.Width();
                aParentSize.Height() -= aOwnSize.Height();
                SetPosPixel(Point(aParentSize.Width(), aParentSize.Height()));
            }
        ~RulerDebugWindow();

        ListBox& GetLBox() {return aBox;}
        static void     AddDebugText(const sal_Char* pDescription, const String& rText );
};
static RulerDebugWindow* pDebugWindow = 0;

RulerDebugWindow::~RulerDebugWindow()
{
    pDebugWindow = 0;
}
void     RulerDebugWindow::AddDebugText(const sal_Char* pDescription, const String& rText )
{
    if(!pDebugWindow)
    {
        Window* pParent = Application::GetFocusWindow();
        while(pParent->GetParent())
            pParent = pParent->GetParent();
        pDebugWindow = new RulerDebugWindow(pParent);
    }
    OUString sContent( OUString::createFromAscii(pDescription) );
    sContent += rText;
    sal_uInt16 nPos = pDebugWindow->GetLBox().InsertEntry(sContent);
    pDebugWindow->GetLBox().SelectEntryPos(nPos);
    pDebugWindow->GrabFocus();
}

#define ADD_DEBUG_TEXT(cDescription, sValue) \
    RulerDebugWindow::AddDebugText(cDescription, sValue);

#define REMOVE_DEBUG_WINDOW \
    delete pDebugWindow;    \
    pDebugWindow = 0;

#else
#define ADD_DEBUG_TEXT(cDescription, sValue)
#define REMOVE_DEBUG_WINDOW
#endif

struct SvxRuler_Impl {
    sal_uInt16 *pPercBuf;
    sal_uInt16 *pBlockBuf;
    sal_uInt16 nPercSize;
    long   nTotalDist;
    long   lOldWinPos;
    long   lMaxLeftLogic;
    long   lMaxRightLogic;
    long   lLastLMargin;
    long   lLastRMargin;
    SvxProtectItem aProtectItem;
    SfxBoolItem* pTextRTLItem;
    sal_uInt16 nControlerItems;
    sal_uInt16 nIdx;
    sal_uInt16 nColLeftPix;
    sal_uInt16 nColRightPix;    // Pixel values for left / right edge
                                // For columns; buffered to prevent
                                // recalculation errors
                                // May be has to be widen for future values
    sal_Bool bIsTableRows : 1;  // mpColumnItem contains table rows instead of columns
    //#i24363# tab stops relative to indent
    sal_Bool bIsTabsRelativeToIndent : 1; // Tab stops relative to paragraph indent?

    SvxRuler_Impl() :
        pPercBuf(0), pBlockBuf(0), nPercSize(0), nTotalDist(0),
        lOldWinPos(0), lMaxLeftLogic(0), lMaxRightLogic(0),
        lLastLMargin(0), lLastRMargin(0), aProtectItem(SID_RULER_PROTECT),
        pTextRTLItem(0), nControlerItems(0), nIdx(0),
        nColLeftPix(0), nColRightPix(0),
        bIsTableRows(sal_False),
        bIsTabsRelativeToIndent(sal_True)
    {
    }

    ~SvxRuler_Impl()
    {
        nPercSize = 0; nTotalDist = 0;
        delete[] pPercBuf; delete[] pBlockBuf; pPercBuf = 0;
        delete pTextRTLItem;
    }
    void SetPercSize(sal_uInt16 nSize);

};

void SvxRuler_Impl::SetPercSize(sal_uInt16 nSize)
{
    if(nSize > nPercSize)
    {
        delete[] pPercBuf;
        delete[] pBlockBuf;
        pPercBuf = new sal_uInt16[nPercSize = nSize];
        pBlockBuf = new sal_uInt16[nPercSize = nSize];
    }
    size_t nSize2 = sizeof(sal_uInt16) * nPercSize;
    memset(pPercBuf, 0, nSize2);
    memset(pBlockBuf, 0, nSize2);
}

// Constructor of the ruler

// SID_ATTR_ULSPACE, SID_ATTR_LRSPACE
// expects as parameter SvxULSpaceItem for page edge
// (either left/right or top/bottom)
// Ruler: SetMargin1, SetMargin2

// SID_RULER_PAGE_POS
// expectes as parameter the initial value of the page and page width
// Ruler: SetPagePos

// SID_ATTR_TABSTOP
// expects: SvxTabStopItem
// Ruler: SetTabs

// SID_ATTR_PARA_LRSPACE
// left, right paragraph edge in H-ruler
// Ruler: SetIndents

// SID_RULER_BORDERS
// Table borders, columns
// expects: something like SwTabCols
// Ruler: SetBorders

SvxRuler::SvxRuler(
            Window* pParent,        // StarView Parent
            Window* pWin,           // Output window: is used for conversion
                                    // logical units <-> pixels
            sal_uInt16 flags,       // Display flags, see ruler.hxx
            SfxBindings &rBindings, // associated Bindings
            WinBits nWinStyle) :    // StarView WinBits
    Ruler(pParent, nWinStyle),
    pCtrlItem(new SvxRulerItem *[CTRL_ITEM_COUNT]),
    pEditWin(pWin),
    pRuler_Imp(new SvxRuler_Impl),
    bAppSetNullOffset(sal_False),  // Is the 0-offset of the ruler set by the application?
    lLogicNullOffset(0),
    lAppNullOffset(LONG_MAX),
    lMinFrame(5),
    lInitialDragPos(0),
    nFlags(flags),
    nDragType(NONE),
    nDefTabType(RULER_TAB_LEFT),
    nTabCount(0),
    nTabBufSize(0),
    lDefTabDist(50),
    lTabPos(-1),
    pTabs(0),
    pIndents(0),
    pBorders(new RulerBorder[1]), // due to one column tables
    nBorderCount(0),
    pObjectBorders(0),
    pBindings(&rBindings),
    nDragOffset(0),
    nMaxLeft(0),
    nMaxRight(0),
    bValid(sal_False),
    bListening(sal_False),
    bActive(sal_True)
{
    /* Constructor; Initialize data buffer; controller items are created */

    memset(pCtrlItem, 0, sizeof(SvxRulerItem *) * CTRL_ITEM_COUNT);

    rBindings.EnterRegistrations();

    // Create Supported Items
    sal_uInt16 i = 0;

    // Page edges
    pCtrlItem[i++] = new SvxRulerItem(SID_RULER_LR_MIN_MAX, *this, rBindings);
    if((nWinStyle & WB_VSCROLL) == WB_VSCROLL)
    {
        bHorz = sal_False;
        pCtrlItem[i++] = new SvxRulerItem(SID_ATTR_LONG_ULSPACE, *this, rBindings);
    }
    else
    {
        bHorz = sal_True;
        pCtrlItem[i++] = new SvxRulerItem(SID_ATTR_LONG_LRSPACE, *this, rBindings);
    }

    // Page Position
    pCtrlItem[i++] = new SvxRulerItem(SID_RULER_PAGE_POS, *this, rBindings);

    if((nFlags & SVXRULER_SUPPORT_TABS) == SVXRULER_SUPPORT_TABS)
    {
        sal_uInt16 nTabStopId = bHorz ? SID_ATTR_TABSTOP : SID_ATTR_TABSTOP_VERTICAL;
        pCtrlItem[i++] = new SvxRulerItem(nTabStopId, *this, rBindings);
        SetExtraType(RULER_EXTRA_TAB, nDefTabType);
    }

    if(0 != (nFlags & (SVXRULER_SUPPORT_PARAGRAPH_MARGINS |SVXRULER_SUPPORT_PARAGRAPH_MARGINS_VERTICAL)))
    {
        if(bHorz)
            pCtrlItem[i++] = new SvxRulerItem(SID_ATTR_PARA_LRSPACE, *this, rBindings);
        else
            pCtrlItem[i++] = new SvxRulerItem(SID_ATTR_PARA_LRSPACE_VERTICAL, *this, rBindings);
        pIndents = new RulerIndent[5 + INDENT_GAP];
        memset(pIndents, 0, sizeof(RulerIndent) * (3 + INDENT_GAP));
        pIndents[0].nStyle = RULER_STYLE_DONTKNOW;
        pIndents[1].nStyle = RULER_STYLE_DONTKNOW;
        pIndents[INDENT_FIRST_LINE].nStyle = RULER_INDENT_TOP;
        pIndents[INDENT_LEFT_MARGIN].nStyle = RULER_INDENT_BOTTOM;
        pIndents[INDENT_RIGHT_MARGIN].nStyle = RULER_INDENT_BOTTOM;

        for(sal_uInt16 nIn = 0; nIn < 7; nIn++)
            pIndents[nIn].nPos = 0;
    }

    if( (nFlags & SVXRULER_SUPPORT_BORDERS) ==  SVXRULER_SUPPORT_BORDERS )
    {
        pCtrlItem[i++] = new SvxRulerItem(bHorz ? SID_RULER_BORDERS : SID_RULER_BORDERS_VERTICAL, *this, rBindings);
        pCtrlItem[i++] = new SvxRulerItem(bHorz ? SID_RULER_ROWS : SID_RULER_ROWS_VERTICAL, *this, rBindings);
    }

    pCtrlItem[i++] = new SvxRulerItem(SID_RULER_TEXT_RIGHT_TO_LEFT, *this, rBindings);

    if( (nFlags & SVXRULER_SUPPORT_OBJECT) == SVXRULER_SUPPORT_OBJECT )
    {
        pCtrlItem[i++] = new SvxRulerItem(SID_RULER_OBJECT, *this, rBindings );
        pObjectBorders = new RulerBorder[OBJECT_BORDER_COUNT];
        size_t nSize = sizeof( RulerBorder ) * OBJECT_BORDER_COUNT;
        memset(pObjectBorders, 0, nSize);
        for(sal_uInt16 nBorder = 0; nBorder < OBJECT_BORDER_COUNT; ++nBorder)
        {
            pObjectBorders[nBorder].nPos   = 0;
            pObjectBorders[nBorder].nWidth = 0;
            pObjectBorders[nBorder].nStyle = RULER_BORDER_MOVEABLE;
        }
    }

    pCtrlItem[i++] = new SvxRulerItem( SID_RULER_PROTECT, *this, rBindings );
    pCtrlItem[i++] = new SvxRulerItem(SID_RULER_BORDER_DISTANCE, *this, rBindings);
    pRuler_Imp->nControlerItems=i;

    if( (nFlags & SVXRULER_SUPPORT_SET_NULLOFFSET) == SVXRULER_SUPPORT_SET_NULLOFFSET )
        SetExtraType(RULER_EXTRA_NULLOFFSET, 0);

    rBindings.LeaveRegistrations();
}


SvxRuler::~SvxRuler()
{
    /* Destructor ruler; release internal buffer */
    REMOVE_DEBUG_WINDOW
    if(bListening)
        EndListening(*pBindings);

    pBindings->EnterRegistrations();

    for(sal_uInt16 i = 0; i < CTRL_ITEM_COUNT  && pCtrlItem[i]; ++i)
        delete pCtrlItem[i];
    delete[] pCtrlItem;

    delete[] pIndents;
    delete[] pBorders;
    delete[] pObjectBorders;
    delete[] pTabs;
    delete pRuler_Imp;

    pBindings->LeaveRegistrations();
}

long SvxRuler::MakePositionSticky(long aPosition, bool aSnapToFrameMargin) const
{
    long aLeftFramePosition  = ConvertHPosPixel(GetLeftFrameMargin());
    long aRightFramePosition = ConvertHPosPixel(GetRightFrameMargin());

    long aTick = GetCurrentRulerUnit().nTick1;
    long aTickPixel = pEditWin->LogicToPixel(Size(0, aTick), GetCurrentMapMode()).Height();
    long aHalfTick = aTick / 2;
    long aHalfTickPixel = aTickPixel / 2;

    if (aSnapToFrameMargin)
    {
        if (aPosition > aLeftFramePosition - aHalfTickPixel && aPosition < aLeftFramePosition + aHalfTickPixel)
        {
            return aLeftFramePosition;
        }
        else if (aPosition > aRightFramePosition - aHalfTickPixel && aPosition < aRightFramePosition + aHalfTickPixel)
        {
            return aRightFramePosition;
        }
    }

    // Move "coordinate system" to frame position so ticks are calculated correctly
    long aTranslatedPosition = aPosition - aLeftFramePosition;
    // Convert position to current selected map mode
    long aPositionLogic = pEditWin->PixelToLogic(Size(0, aTranslatedPosition), GetCurrentMapMode()).Height();
    aPositionLogic = ((aPositionLogic + aHalfTick) / aTick) * aTick;
    // Convert back to pixels
    aPosition = pEditWin->LogicToPixel(Size(0, aPositionLogic), GetCurrentMapMode()).Height();
    // Move "coordinate system" back to original position
    return aPosition + aLeftFramePosition;
}

long SvxRuler::ConvertHPosPixel(long nVal) const
{
    return pEditWin->LogicToPixel(Size(nVal, 0)).Width();
}

long SvxRuler::ConvertVPosPixel(long nVal) const
{
    return pEditWin->LogicToPixel(Size(0, nVal)).Height();
}

long SvxRuler::ConvertHSizePixel(long nVal) const
{
    return pEditWin->LogicToPixel(Size(nVal, 0)).Width();
}

long SvxRuler::ConvertVSizePixel(long nVal) const
{
    return pEditWin->LogicToPixel(Size(0, nVal)).Height();
}

long SvxRuler::ConvertPosPixel(long nVal) const
{
    return bHorz ? ConvertHPosPixel(nVal): ConvertVPosPixel(nVal);
}

long SvxRuler::ConvertSizePixel(long nVal) const
{
    return bHorz? ConvertHSizePixel(nVal): ConvertVSizePixel(nVal);
}

inline long SvxRuler::ConvertHPosLogic(long nVal) const
{
    return pEditWin->PixelToLogic(Size(nVal, 0)).Width();
}

inline long SvxRuler::ConvertVPosLogic(long nVal) const
{
    return pEditWin->PixelToLogic(Size(0, nVal)).Height();
}

inline long SvxRuler::ConvertHSizeLogic(long nVal) const
{
    return pEditWin->PixelToLogic(Size(nVal, 0)).Width();
}

inline long SvxRuler::ConvertVSizeLogic(long nVal) const
{
    return pEditWin->PixelToLogic(Size(0, nVal)).Height();
}

inline long SvxRuler::ConvertPosLogic(long nVal) const
{
    return bHorz? ConvertHPosLogic(nVal): ConvertVPosLogic(nVal);
}

inline long SvxRuler::ConvertSizeLogic(long nVal) const
{
    return bHorz? ConvertHSizeLogic(nVal): ConvertVSizeLogic(nVal);
}

long SvxRuler::PixelHAdjust(long nVal, long nValOld) const
{
    if(ConvertHSizePixel(nVal) != ConvertHSizePixel(nValOld))
        return  nVal;
    else
        return  nValOld;
}

long SvxRuler::PixelVAdjust(long nVal, long nValOld) const
{
    if(ConvertVSizePixel(nVal) != ConvertVSizePixel(nValOld))
        return  nVal;
    else
        return  nValOld;
}

long SvxRuler::PixelAdjust(long nVal, long nValOld) const
{
    if(ConvertSizePixel(nVal) != ConvertSizePixel(nValOld))
        return  nVal;
    else
        return  nValOld;
}

inline sal_uInt16 SvxRuler::GetObjectBordersOff(sal_uInt16 nIdx) const
{
    return bHorz ? nIdx : nIdx + 2;
}

/*
    Update Upper Left edge.
    Items are translated into the representation of the ruler.
*/
void SvxRuler::UpdateFrame()
{
    const sal_uInt16 nMarginStyle =
        ( pRuler_Imp->aProtectItem.IsSizeProtected() ||
          pRuler_Imp->aProtectItem.IsPosProtected() ) ?
        0 : RULER_MARGIN_SIZEABLE;

    if(mpLRSpaceItem.get() && mpPagePosItem.get())
    {
        // if no initialization by default app behavior
        const long nOld = lLogicNullOffset;
        lLogicNullOffset = mpColumnItem.get() ? mpColumnItem->GetLeft(): mpLRSpaceItem->GetLeft();
        if(bAppSetNullOffset)
            lAppNullOffset += lLogicNullOffset - nOld;
        if(!bAppSetNullOffset || lAppNullOffset == LONG_MAX)
        {
            Ruler::SetNullOffset(ConvertHPosPixel(lLogicNullOffset));
            SetMargin1( 0, nMarginStyle );
            lAppNullOffset = 0;
        }
        else
            SetMargin1( ConvertHPosPixel( lAppNullOffset ), nMarginStyle );
        long lRight = 0;
            // evaluate the table right edge of the table
        if(mpColumnItem.get() && mpColumnItem->IsTable())
            lRight = mpColumnItem->GetRight();
        else
            lRight = mpLRSpaceItem->GetRight();

        sal_uIntPtr aWidth=
            ConvertHPosPixel(mpPagePosItem->GetWidth() - lRight -
                                    lLogicNullOffset + lAppNullOffset);
        SetMargin2( aWidth, nMarginStyle );
    }
    else if(mpULSpaceItem.get() && mpPagePosItem.get())
    {
        // relative the upper edge of the surrounding frame
        const long nOld = lLogicNullOffset;
        lLogicNullOffset = mpColumnItem.get() ? mpColumnItem->GetLeft() : mpULSpaceItem->GetUpper();
        if(bAppSetNullOffset)
            lAppNullOffset += lLogicNullOffset - nOld;
        if(!bAppSetNullOffset || lAppNullOffset == LONG_MAX)
        {
            Ruler::SetNullOffset(ConvertVPosPixel(lLogicNullOffset));
            lAppNullOffset = 0;
            SetMargin1( 0, nMarginStyle );
        }
        else
            SetMargin1( ConvertVPosPixel( lAppNullOffset ),nMarginStyle );

        long lLower = mpColumnItem.get() ? mpColumnItem->GetRight() : mpULSpaceItem->GetLower();

        SetMargin2(
            ConvertVPosPixel(mpPagePosItem->GetHeight() - lLower - lLogicNullOffset + lAppNullOffset),
            nMarginStyle);
    }
    else
    {
        // turns off the view
        SetMargin1();
        SetMargin2();
    }

    if(mpColumnItem.get())
    {
        pRuler_Imp->nColLeftPix = (sal_uInt16) ConvertSizePixel(mpColumnItem->GetLeft());
        pRuler_Imp->nColRightPix = (sal_uInt16) ConvertSizePixel(mpColumnItem->GetRight());
    }
}

void SvxRuler::MouseMove( const MouseEvent& rMEvt )
{
    if( bActive )
    {
        pBindings->Update( SID_RULER_LR_MIN_MAX );
        pBindings->Update( SID_ATTR_LONG_ULSPACE );
        pBindings->Update( SID_ATTR_LONG_LRSPACE );
        pBindings->Update( SID_RULER_PAGE_POS );
        pBindings->Update( bHorz ? SID_ATTR_TABSTOP : SID_ATTR_TABSTOP_VERTICAL);
        pBindings->Update( bHorz ? SID_ATTR_PARA_LRSPACE : SID_ATTR_PARA_LRSPACE_VERTICAL);
        pBindings->Update( bHorz ? SID_RULER_BORDERS : SID_RULER_BORDERS_VERTICAL);
        pBindings->Update( bHorz ? SID_RULER_ROWS : SID_RULER_ROWS_VERTICAL);
        pBindings->Update( SID_RULER_OBJECT );
        pBindings->Update( SID_RULER_PROTECT );
    }
    Ruler::MouseMove( rMEvt );
}

void SvxRuler::StartListening_Impl()
{
    if(!bListening)
    {
        bValid = sal_False;
        StartListening(*pBindings);
        bListening = sal_True;
    }
}

void SvxRuler::UpdateFrame(const SvxLongLRSpaceItem *pItem) // new value LRSpace
{
    /* Store new value LRSpace; delete old ones if possible */
    if(bActive)
    {
        if(pItem)
            mpLRSpaceItem.reset(new SvxLongLRSpaceItem(*pItem));
        else
            mpLRSpaceItem.reset(NULL);
        StartListening_Impl();
    }
}

void SvxRuler::UpdateFrameMinMax(const SfxRectangleItem *pItem) // value for MinMax
{
    /* Set new value for MinMax; delete old ones if possible */
    if(bActive)
    {
        if(pItem)
            mpMinMaxItem.reset(new SfxRectangleItem(*pItem));
        else
            mpMinMaxItem.reset(NULL);
    }
}


void SvxRuler::UpdateFrame(const SvxLongULSpaceItem *pItem) // new value
{
    /* Update Right/bottom margin */
    if(bActive && !bHorz)
    {
        if(pItem)
            mpULSpaceItem.reset(new SvxLongULSpaceItem(*pItem));
        else
            mpULSpaceItem.reset(NULL);
        StartListening_Impl();
    }
}

void SvxRuler::Update( const SvxProtectItem* pItem )
{
    if( pItem )
        pRuler_Imp->aProtectItem = *pItem;
}

void SvxRuler::UpdateTextRTL(const SfxBoolItem* pItem)
{
    if(bActive && bHorz)
    {
        delete pRuler_Imp->pTextRTLItem;
        pRuler_Imp->pTextRTLItem = 0;
        if(pItem)
            pRuler_Imp->pTextRTLItem = new SfxBoolItem(*pItem);
        SetTextRTL(pRuler_Imp->pTextRTLItem && pRuler_Imp->pTextRTLItem->GetValue());
        StartListening_Impl();
    }
}

void SvxRuler::Update(
                const SvxColumnItem *pItem,  // new value
                sal_uInt16 nSID) //Slot Id to identify NULL items
{
    /* Set new value for column view */
    if(bActive)
    {
        if(pItem)
        {
            mpColumnItem.reset(new SvxColumnItem(*pItem));
            pRuler_Imp->bIsTableRows = (pItem->Which() == SID_RULER_ROWS || pItem->Which() == SID_RULER_ROWS_VERTICAL);
            if(!bHorz && !pRuler_Imp->bIsTableRows)
                mpColumnItem->SetWhich(SID_RULER_BORDERS_VERTICAL);
        }
        else if(mpColumnItem.get() && mpColumnItem->Which() == nSID)
        //there are two groups of column items table/frame columns and table rows
        //both can occur in vertical or horizontal mode
        //the horizontal ruler handles the SID_RULER_BORDERS and SID_RULER_ROWS_VERTICAL
        //and the vertical handles SID_RULER_BORDERS_VERTICAL and SID_RULER_ROWS
        //if mpColumnItem is already set with one of the ids then a NULL pItem argument
        //must not delete it
        {
            mpColumnItem.reset(NULL);
            pRuler_Imp->bIsTableRows = sal_False;
        }
        StartListening_Impl();
    }
}


void SvxRuler::UpdateColumns()
{
    /* Update column view */
    if(mpColumnItem.get() && mpColumnItem->Count() > 1)
    {
        if( nBorderCount < mpColumnItem->Count())
        {
            delete[] pBorders;
            nBorderCount = mpColumnItem->Count();
            pBorders = new RulerBorder[nBorderCount];
        }
        sal_uInt16 nStyleFlags = RULER_BORDER_VARIABLE;
        sal_Bool bProtectColumns =
            pRuler_Imp->aProtectItem.IsSizeProtected() ||
            pRuler_Imp->aProtectItem.IsPosProtected();
        if( !bProtectColumns )
            nStyleFlags |= RULER_BORDER_MOVEABLE;

        if( mpColumnItem->IsTable() )
            nStyleFlags |= RULER_BORDER_TABLE;
        else if ( !bProtectColumns )
            nStyleFlags |= RULER_BORDER_SIZEABLE;

        sal_uInt16 nBorders = mpColumnItem->Count();
        if(!pRuler_Imp->bIsTableRows)
            --nBorders;
        for(sal_uInt16 i = 0; i < nBorders; ++i)
        {
            pBorders[i].nStyle = nStyleFlags;
            if(!(*mpColumnItem.get())[i].bVisible)
                pBorders[i].nStyle |= RULER_STYLE_INVISIBLE;
            pBorders[i].nPos =
                ConvertPosPixel((*mpColumnItem.get())[i].nEnd + lAppNullOffset);
            if(mpColumnItem->Count() == i + 1)
            {
                //with table rows the end of the table is contained in the
                //column item but it has no width!
                pBorders[i].nWidth = 0;
            }
            else
            {
                pBorders[i].nWidth =
                    ConvertSizePixel((*mpColumnItem.get())[i+1].nStart -
                                 (*mpColumnItem.get())[i].nEnd);
            }
            pBorders[i].nMinPos =
                ConvertPosPixel((*mpColumnItem.get())[i].nEndMin + lAppNullOffset);
            pBorders[i].nMaxPos =
                ConvertPosPixel((*mpColumnItem.get())[i].nEndMax + lAppNullOffset);
        }
        SetBorders(mpColumnItem->Count() - 1, pBorders);
    }
    else
    {
        SetBorders();
    }
}

void SvxRuler::UpdateObject()
{
    /* Update view of object representation */
    if(mpObjectItem.get())
    {
        DBG_ASSERT(pObjectBorders, "no Buffer");
        // !! to the page margin
        long nMargin = mpLRSpaceItem.get() ? mpLRSpaceItem->GetLeft() : 0;
        pObjectBorders[0].nPos =
            ConvertPosPixel(mpObjectItem->GetStartX() -
                            nMargin + lAppNullOffset);
        pObjectBorders[1].nPos =
            ConvertPosPixel(mpObjectItem->GetEndX() - nMargin + lAppNullOffset);
        nMargin = mpULSpaceItem.get() ? mpULSpaceItem->GetUpper() : 0;
        pObjectBorders[2].nPos =
            ConvertPosPixel(mpObjectItem->GetStartY() -
                            nMargin + lAppNullOffset);
        pObjectBorders[3].nPos =
            ConvertPosPixel(mpObjectItem->GetEndY() - nMargin + lAppNullOffset);

        const sal_uInt16 nOff = GetObjectBordersOff(0);
        SetBorders(2, pObjectBorders + nOff);
    }
    else
    {
        SetBorders();
    }
}

void SvxRuler::UpdatePara()
{

    /*  Update the view for paragraph indents:
        Left margin, first line indent, right margin paragraph update
        pIndents[0] = Buffer for old intent
        pIndents[1] = Buffer for old intent
        pIndents[INDENT_FIRST_LINE] = First line indent
        pIndents[3] = left margin
        pIndents[4] = right margin
        pIndents[5] = left border distance
        pIndents[6] = right border distance
    */

    // Dependence on PagePosItem
    if(mpParaItem.get() && mpPagePosItem.get() && !mpObjectItem.get())
    {
        sal_Bool bRTLText = pRuler_Imp->pTextRTLItem && pRuler_Imp->pTextRTLItem->GetValue();
        // First-line indent is negative to the left paragraph margin
        long nLeftFrameMargin = GetLeftFrameMargin();
        long nRightFrameMargin = GetRightFrameMargin();
        SetLeftFrameMargin(ConvertHPosPixel(nLeftFrameMargin));
        SetRightFrameMargin(ConvertHPosPixel(nRightFrameMargin));

        if(bRTLText)
        {
            pIndents[INDENT_FIRST_LINE].nPos = ConvertHPosPixel(
                nRightFrameMargin - mpParaItem->GetTxtLeft() - mpParaItem->GetTxtFirstLineOfst() + lAppNullOffset );
        }
        else
        {
            pIndents[INDENT_FIRST_LINE].nPos =
                ConvertHPosPixel(
                    nLeftFrameMargin + mpParaItem->GetTxtLeft() + mpParaItem->GetTxtFirstLineOfst() + lAppNullOffset);
        }
        if( mpParaItem->IsAutoFirst() )
            pIndents[INDENT_FIRST_LINE].nStyle |= RULER_STYLE_INVISIBLE;
        else
            pIndents[INDENT_FIRST_LINE].nStyle &= ~RULER_STYLE_INVISIBLE;

        if(bRTLText)
        {
            // left margin
            pIndents[INDENT_LEFT_MARGIN].nPos =
                ConvertHPosPixel(nRightFrameMargin - mpParaItem->GetTxtLeft() + lAppNullOffset);

            // right margin
            pIndents[INDENT_RIGHT_MARGIN].nPos =
                ConvertHPosPixel(nLeftFrameMargin + mpParaItem->GetRight() + lAppNullOffset);

        }
        else
        {
            // left margin
            sal_Int32 leftMargin =
                        ConvertHPosPixel(nLeftFrameMargin + mpParaItem->GetTxtLeft() + lAppNullOffset);
            pIndents[INDENT_LEFT_MARGIN].nPos = leftMargin;

            // right margin, always negative to the right edge of the surrounding frames
            sal_Int32 rightMargin =
                        ConvertHPosPixel(nRightFrameMargin - mpParaItem->GetRight() + lAppNullOffset);
            pIndents[INDENT_RIGHT_MARGIN].nPos = rightMargin;
        }
        SetIndents(INDENT_COUNT, pIndents+INDENT_GAP);
    }
    else
    {
        if(pIndents)
        {
            pIndents[INDENT_FIRST_LINE].nPos = 0;
            pIndents[INDENT_LEFT_MARGIN].nPos = 0;
            pIndents[INDENT_RIGHT_MARGIN].nPos = 0;
        }
        SetIndents();        // turn off
    }
}

void SvxRuler::UpdatePara(const SvxLRSpaceItem *pItem) // new value of paragraph indents
{
    /* Store new value of paragraph indents */
    if(bActive)
    {
        if(pItem)
            mpParaItem.reset(new SvxLRSpaceItem(*pItem));
        else
            mpParaItem.reset(NULL);
        StartListening_Impl();
    }
}

void SvxRuler::UpdateParaBorder(const SvxLRSpaceItem * pItem )
{
    /* Border distance */
    if(bActive)
    {
        if(pItem)
            mpParaBorderItem.reset(new SvxLRSpaceItem(*pItem));
        else
            mpParaBorderItem.reset(NULL);
        StartListening_Impl();
    }
}

void SvxRuler::UpdatePage()
{
    /* Update view of position and width of page */
    if(mpPagePosItem.get())
    {
        // all objects are automatically adjusted
        if(bHorz)
        {
            SetPagePos(
                pEditWin->LogicToPixel(mpPagePosItem->GetPos()).X(),
                pEditWin->LogicToPixel(Size(mpPagePosItem->GetWidth(), 0)).
                Width());
        }
        else
        {
            SetPagePos(
                pEditWin->LogicToPixel(mpPagePosItem->GetPos()).Y(),
                pEditWin->LogicToPixel(Size(0, mpPagePosItem->GetHeight())).
                Height());
        }
        if(bAppSetNullOffset)
            SetNullOffset(ConvertSizePixel(-lAppNullOffset + lLogicNullOffset));
    }
    else
    {
        SetPagePos();
    }

    long lPos = 0;
    Point aOwnPos = GetPosPixel();
    Point aEdtWinPos = pEditWin->GetPosPixel();
    if( Application::GetSettings().GetLayoutRTL() && bHorz )
    {
        //#i73321# in RTL the window and the ruler is not mirrored but the
        // influence of the vertical ruler is inverted
        Size aOwnSize = GetSizePixel();
        Size aEdtWinSize = pEditWin->GetSizePixel();
        lPos = aOwnSize.Width() - aEdtWinSize.Width();
        lPos -= (aEdtWinPos - aOwnPos).X();
    }
    else
    {
        Point aPos(aEdtWinPos - aOwnPos);
        lPos = bHorz ? aPos.X() : aPos.Y();
    }

    // Unfortunately, we get the offset of the edit window to the ruler never
    // through a status message. So we set it ourselves if necessary.
    if(lPos != pRuler_Imp->lOldWinPos)
    {
        pRuler_Imp->lOldWinPos=lPos;
        SetWinPos(lPos);
    }
}

void SvxRuler::Update(const SvxPagePosSizeItem *pItem) // new value of page attributes
{
    /* Store new value of page attributes */
    if(bActive)
    {
        if(pItem)
            mpPagePosItem.reset(new SvxPagePosSizeItem(*pItem));
        else
            mpPagePosItem.reset(NULL);
        StartListening_Impl();
    }
}

void SvxRuler::SetDefTabDist(long inDefTabDist)  // New distance for DefaultTabs in App-Metrics
{
    /* New distance is set for DefaultTabs */
    lDefTabDist = inDefTabDist;
    UpdateTabs();
}

sal_uInt16 ToSvTab_Impl(SvxTabAdjust eAdj)
{
    /* Internal convertion routine between SV-Tab.-Enum and Svx */
    switch(eAdj) {
        case SVX_TAB_ADJUST_LEFT:    return RULER_TAB_LEFT;
        case SVX_TAB_ADJUST_RIGHT:   return RULER_TAB_RIGHT;
        case SVX_TAB_ADJUST_DECIMAL: return RULER_TAB_DECIMAL;
        case SVX_TAB_ADJUST_CENTER:  return RULER_TAB_CENTER;
        case SVX_TAB_ADJUST_DEFAULT: return RULER_TAB_DEFAULT;
        default: ; //prevent warning
    }
    return 0;
}

SvxTabAdjust ToAttrTab_Impl(sal_uInt16 eAdj)
{
    switch(eAdj) {
        case RULER_TAB_LEFT:    return SVX_TAB_ADJUST_LEFT    ;
        case RULER_TAB_RIGHT:   return SVX_TAB_ADJUST_RIGHT   ;
        case RULER_TAB_DECIMAL: return SVX_TAB_ADJUST_DECIMAL ;
        case RULER_TAB_CENTER:  return SVX_TAB_ADJUST_CENTER  ;
        case RULER_TAB_DEFAULT: return SVX_TAB_ADJUST_DEFAULT ;
    }
    return SVX_TAB_ADJUST_LEFT;
}

void SvxRuler::UpdateTabs()
{
    if(IsDrag())
        return;
    if(mpPagePosItem.get() && mpParaItem.get() && mpTabStopItem.get() && !mpObjectItem.get())
    {
        // buffer for DefaultTabStop
        // Distance last Tab <-> Right paragraph margin / DefaultTabDist
        sal_Bool bRTL = pRuler_Imp->pTextRTLItem && pRuler_Imp->pTextRTLItem->GetValue();
        long nLeftFrameMargin = GetLeftFrameMargin();
        long nRightFrameMargin = GetRightFrameMargin();

        //#i24363# tab stops relative to indent
        const long nParaItemTxtLeft = mpParaItem->GetTxtLeft();

        const long lParaIndent = nLeftFrameMargin + nParaItemTxtLeft;

        const long lLastTab =
             mpTabStopItem->Count()?
              ConvertHPosPixel((*mpTabStopItem.get())[mpTabStopItem->Count()-1].GetTabPos()): 0;
        const long lPosPixel =
            ConvertHPosPixel(lParaIndent) + lLastTab;
        const long lRightIndent =
            ConvertHPosPixel(nRightFrameMargin - mpParaItem->GetRight());
        long nDefTabDist = ConvertHPosPixel(lDefTabDist);
        if( !nDefTabDist )
            nDefTabDist = 1;
        const sal_uInt16 nDefTabBuf = lPosPixel > lRightIndent ||
            lLastTab > lRightIndent
                ? 0
                : (sal_uInt16)( (lRightIndent - lPosPixel) / nDefTabDist );

        if(mpTabStopItem->Count() + TAB_GAP + nDefTabBuf > nTabBufSize)
        {
            delete[] pTabs;
            // 10 (GAP) in stock
            nTabBufSize = mpTabStopItem->Count() + TAB_GAP + nDefTabBuf + GAP;
            pTabs = new RulerTab[nTabBufSize];
        }

        nTabCount = 0;
        sal_uInt16 j;
        //#i24363# tab stops relative to indent
        const long lRightPixMargin = ConvertSizePixel(nRightFrameMargin - nParaItemTxtLeft );
        const long lParaIndentPix = ConvertSizePixel(lParaIndent);
        for(j = 0; j < mpTabStopItem->Count(); ++j)
        {
            const SvxTabStop *pTab = &(*mpTabStopItem.get())[j];
            pTabs[nTabCount+TAB_GAP].nPos =
                ConvertHPosPixel(
                (pRuler_Imp->bIsTabsRelativeToIndent ? lParaIndent : 0 ) + pTab->GetTabPos() + lAppNullOffset);
            if(bRTL)
            {
                pTabs[nTabCount+TAB_GAP].nPos = lParaIndentPix + lRightPixMargin - pTabs[nTabCount+TAB_GAP].nPos;
            }
            pTabs[nTabCount+TAB_GAP].nStyle = ToSvTab_Impl(pTab->GetAdjustment());
            ++nTabCount;
        }
        if(!mpTabStopItem->Count())
            pTabs[0].nPos = bRTL ? lRightPixMargin : lParaIndentPix;

        // fill the rest with default Tabs
        if(bRTL)
        {
            for(j = 0; j < nDefTabBuf; ++j)
            {
                pTabs[nTabCount + TAB_GAP].nPos =
                    pTabs[nTabCount].nPos - nDefTabDist;

                if(j == 0 )
                    pTabs[nTabCount + TAB_GAP].nPos -=
                        ((pTabs[nTabCount + TAB_GAP].nPos - lRightPixMargin)
                         % nDefTabDist );
                if(pTabs[nTabCount+TAB_GAP].nPos <= lParaIndentPix)
                    break;
                pTabs[nTabCount + TAB_GAP].nStyle = RULER_TAB_DEFAULT;
                ++nTabCount;
            }
        }
        else
        {
            for(j = 0; j < nDefTabBuf; ++j)
            {
                if( j == 0 )
                {
                    //set the first default tab stop
                    if(pRuler_Imp->bIsTabsRelativeToIndent)
                    {
                        pTabs[nTabCount + TAB_GAP].nPos =
                            (pTabs[nTabCount].nPos + nDefTabDist);
                        pTabs[nTabCount + TAB_GAP].nPos -=
                            ((pTabs[nTabCount + TAB_GAP].nPos - lParaIndentPix)
                                % nDefTabDist );
                    }
                    else
                    {
                        if( pTabs[nTabCount].nPos < 0 )
                        {
                            pTabs[nTabCount + TAB_GAP].nPos = ( pTabs[nTabCount].nPos / nDefTabDist ) * nDefTabDist;
                        }
                        else
                        {
                            pTabs[nTabCount + TAB_GAP].nPos = ( pTabs[nTabCount].nPos / nDefTabDist + 1 ) * nDefTabDist;
                        }
                    }

                }
                else
                {
                    //simply add the default distance to the last position
                    pTabs[nTabCount + TAB_GAP].nPos = pTabs[nTabCount].nPos + nDefTabDist;
                }

                if(pTabs[nTabCount + TAB_GAP].nPos >= lRightIndent)
                    break;
                pTabs[nTabCount + TAB_GAP].nStyle = RULER_TAB_DEFAULT;
                ++nTabCount;
            }
        }
        SetTabs(nTabCount, pTabs+TAB_GAP);
        DBG_ASSERT(nTabCount + TAB_GAP <= nTabBufSize, "BufferSize too small");
    }
    else
    {
        SetTabs();
    }
}

void SvxRuler::Update(const SvxTabStopItem *pItem) // new value for tabs
{
    /* Store new value for tabs; delete old ones if possible */
    if(bActive)
    {
        if(pItem)
        {
            mpTabStopItem.reset(new SvxTabStopItem(*pItem));
            if(!bHorz)
                mpTabStopItem->SetWhich(SID_ATTR_TABSTOP_VERTICAL);
        }
        else
        {
            mpTabStopItem.reset(NULL);
        }
        StartListening_Impl();
    }
}

void SvxRuler::Update(const SvxObjectItem *pItem) // new value for objects
{
    /* Store new value for objects */
    if(bActive)
    {
        if(pItem)
            mpObjectItem.reset(new SvxObjectItem(*pItem));
        else
            mpObjectItem.reset(NULL);
        StartListening_Impl();
    }
}

void SvxRuler::SetNullOffsetLogic(long lVal) // Setting of the logic NullOffsets
{
    lAppNullOffset = lLogicNullOffset - lVal;
    bAppSetNullOffset = sal_True;
    Ruler::SetNullOffset(ConvertSizePixel(lVal));
    Update();
}

void SvxRuler::Update()
{
    /* Perform update of view */
    if(IsDrag())
        return;

    UpdatePage();
    UpdateFrame();
    if((nFlags & SVXRULER_SUPPORT_OBJECT) == SVXRULER_SUPPORT_OBJECT)
        UpdateObject();
    else
        UpdateColumns();

    if(0 != (nFlags & (SVXRULER_SUPPORT_PARAGRAPH_MARGINS | SVXRULER_SUPPORT_PARAGRAPH_MARGINS_VERTICAL)))
      UpdatePara();

    if(0 != (nFlags & SVXRULER_SUPPORT_TABS))
      UpdateTabs();
}

long SvxRuler::GetPageWidth() const
{
    if (!mpPagePosItem.get())
        return 0;
    return bHorz ? mpPagePosItem->GetWidth() : mpPagePosItem->GetHeight();
}

inline long SvxRuler::GetFrameLeft() const
{
    /* Get Left margin in Pixels */
    return  bAppSetNullOffset ?
            GetMargin1() + ConvertSizePixel(lLogicNullOffset) :
            Ruler::GetNullOffset();
}

inline void SvxRuler::SetFrameLeft(long lFrameLeft)
{
    /* Set Left margin in Pixels */
    sal_Bool bProtectColumns =
                pRuler_Imp->aProtectItem.IsSizeProtected() ||
                pRuler_Imp->aProtectItem.IsPosProtected();
    if(bAppSetNullOffset)
    {
        SetMargin1(lFrameLeft - ConvertSizePixel(lLogicNullOffset),
                   bProtectColumns ? 0 : RULER_MARGIN_SIZEABLE);
    }
    else
    {
        Ruler::SetNullOffset(lFrameLeft);
    }
}

long SvxRuler::GetFirstLineIndent() const
{
    /* Get First-line indent in pixels */
    return mpParaItem.get() ? pIndents[INDENT_FIRST_LINE].nPos : GetMargin1();
}

long SvxRuler::GetLeftIndent() const
{
    /* Get Left paragraph margin in Pixels */
    return mpParaItem.get() ? pIndents[INDENT_LEFT_MARGIN].nPos : GetMargin1();
}

long SvxRuler::GetRightIndent() const
{
    /* Get Right paragraph margin in Pixels */
    return mpParaItem.get() ? pIndents[INDENT_RIGHT_MARGIN].nPos : GetMargin2();
}


long SvxRuler::GetLogicRightIndent() const
{
    /* Get Right paragraph margin in Logic */
    return mpParaItem.get() ? GetRightFrameMargin() - mpParaItem->GetRight() : GetRightFrameMargin();
}

// Left margin in App values, is either the margin (= 0)  or the left edge of
// the column that is set in the column attribute as current column.
long SvxRuler::GetLeftFrameMargin() const
{
    // #126721# for some unknown reason the current column is set to 0xffff
    DBG_ASSERT(!mpColumnItem.get() || mpColumnItem->GetActColumn() < mpColumnItem->Count(),
                    "issue #126721# - invalid current column!");
    long nLeft = 0;
    if (mpColumnItem.get() &&
        mpColumnItem->Count() &&
        mpColumnItem->GetActColumn() < mpColumnItem->Count())
    {
        nLeft = (*mpColumnItem.get())[mpColumnItem->GetActColumn()].nStart;
    }
    if(mpParaBorderItem.get() && (!mpColumnItem || mpColumnItem->IsTable()))
        nLeft += mpParaBorderItem->GetLeft();
    return nLeft;
}

inline long SvxRuler::GetLeftMin() const
{
    DBG_ASSERT(mpMinMaxItem.get(), "no MinMax value set");
    if (mpMinMaxItem.get())
    {
        if (bHorz)
            return mpMinMaxItem->GetValue().Left();
        else
            return mpMinMaxItem->GetValue().Top();
    }
    return 0;
}

inline long SvxRuler::GetRightMax() const
{
    DBG_ASSERT(mpMinMaxItem.get(), "no MinMax value set");
    if (mpMinMaxItem.get())
    {
        if (bHorz)
            return mpMinMaxItem->GetValue().Right();
        else
            return mpMinMaxItem->GetValue().Bottom();
    }
    return 0;
}


long SvxRuler::GetRightFrameMargin() const
{
    /* Get right frame margin (in logical units) */
    if(mpColumnItem.get())
    {
        if(!IsActLastColumn( sal_True ))
        {
            return (*mpColumnItem.get())[GetActRightColumn( sal_True )].nEnd;
        }
    }

    long lResult = lLogicNullOffset;

    // If possible deduct right table entry
    if(mpColumnItem.get() && mpColumnItem->IsTable())
        lResult += mpColumnItem->GetRight();
    else if(bHorz && mpLRSpaceItem.get())
        lResult += mpLRSpaceItem->GetRight();
    else if(!bHorz && mpULSpaceItem.get())
        lResult += mpULSpaceItem->GetLower();

    if(bHorz)
        lResult = mpPagePosItem->GetWidth() - lResult;
    else
        lResult = mpPagePosItem->GetHeight() - lResult;

    return lResult;
}

#define NEG_FLAG ( (nFlags & SVXRULER_SUPPORT_NEGATIVE_MARGINS) == \
                   SVXRULER_SUPPORT_NEGATIVE_MARGINS )
#define TAB_FLAG ( mpColumnItem.get() && mpColumnItem->IsTable() )

long SvxRuler::GetCorrectedDragPos( sal_Bool bLeft, sal_Bool bRight )
{
    /*
        Corrects the position within the calculated limits. The limit values are in
        pixels relative to the page edge.
    */

    const long lNullPix = Ruler::GetNullOffset();
    long lDragPos = GetDragPos() + lNullPix;
ADD_DEBUG_TEXT("lDragPos: ", OUString::number(lDragPos))
     sal_Bool bHoriRows = bHorz && pRuler_Imp->bIsTableRows;
    if((bLeft || (bHoriRows)) && lDragPos < nMaxLeft)
        lDragPos = nMaxLeft;
    else if((bRight||bHoriRows) && lDragPos > nMaxRight)
        lDragPos = nMaxRight;
    return lDragPos - lNullPix;
}

void ModifyTabs_Impl( sal_uInt16 nCount, // Number of Tabs
                      RulerTab *pTabs,   // Tab buffer
                      long lDiff)        // difference to be added
{
    /* Helper function, move all the tabs by a fixed value */
    if( pTabs )
        for(sal_uInt16 i = 0; i < nCount; ++i)
            pTabs[i].nPos += lDiff;
}

void SvxRuler::DragMargin1()
{
    /* Dragging the left edge of frame */
    long aDragPosition = GetCorrectedDragPos( !TAB_FLAG || !NEG_FLAG, sal_True );
    aDragPosition = MakePositionSticky(aDragPosition, false);

    // Check if position changed
    if (aDragPosition == 0)
        return;

    DrawLine_Impl(lTabPos, ( TAB_FLAG && NEG_FLAG ) ? 3 : 7, bHorz);
    if (mpColumnItem.get() && (nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL))
        DragBorders();
    AdjustMargin1(aDragPosition);
}

void SvxRuler::AdjustMargin1(long lDiff)
{
    const long nOld = bAppSetNullOffset? GetMargin1(): GetNullOffset();
    const long lDragPos = lDiff;
    sal_Bool bProtectColumns =
        pRuler_Imp->aProtectItem.IsSizeProtected() ||
        pRuler_Imp->aProtectItem.IsPosProtected();

    const sal_uInt16 nMarginStyle =
        bProtectColumns ? 0 : RULER_MARGIN_SIZEABLE;

    if(!bAppSetNullOffset)
    {
        long _lDiff = lDragPos;
        SetNullOffset(nOld + _lDiff);
        if (!mpColumnItem.get() || !(nDragType & DRAG_OBJECT_SIZE_LINEAR))
        {
            SetMargin2( GetMargin2() - _lDiff, nMarginStyle );

            if (!mpColumnItem.get() && !mpObjectItem.get() && mpParaItem.get())
            {
                // Right indent of the old position
                pIndents[INDENT_RIGHT_MARGIN].nPos -= _lDiff;
                SetIndents(INDENT_COUNT, pIndents+INDENT_GAP);
            }
            if(mpObjectItem.get())
            {
                pObjectBorders[GetObjectBordersOff(0)].nPos -= _lDiff;
                pObjectBorders[GetObjectBordersOff(1)].nPos -= _lDiff;
                SetBorders(2, pObjectBorders + GetObjectBordersOff(0));
            }
            if(mpColumnItem.get())
            {
                for(sal_uInt16 i = 0; i < mpColumnItem->Count()-1; ++i)
                    pBorders[i].nPos -= _lDiff;
                SetBorders(mpColumnItem->Count()-1, pBorders);
                if(mpColumnItem->IsFirstAct())
                {
                    // Right indent of the old position
                    if(mpParaItem.get())
                    {
                        pIndents[INDENT_RIGHT_MARGIN].nPos -= _lDiff;
                        SetIndents(INDENT_COUNT, pIndents+INDENT_GAP);
                    }
                }
                else
                {
                    if(mpParaItem.get())
                    {
                        pIndents[INDENT_FIRST_LINE].nPos -= _lDiff;
                        pIndents[INDENT_LEFT_MARGIN].nPos -= _lDiff;
                        pIndents[INDENT_RIGHT_MARGIN].nPos -= _lDiff;
                        SetIndents(INDENT_COUNT, pIndents+INDENT_GAP);
                    }
                }
                if(mpTabStopItem.get() && (nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
                   &&!IsActFirstColumn())
                {
                    ModifyTabs_Impl(nTabCount+TAB_GAP, pTabs, -_lDiff);
                    SetTabs(nTabCount, pTabs+TAB_GAP);
                }
            }
        }
    }
    else
    {
        long _lDiff = lDragPos - nOld;
        SetMargin1(nOld + _lDiff, nMarginStyle );

        if (!mpColumnItem.get() || !(nDragType & (DRAG_OBJECT_SIZE_LINEAR | DRAG_OBJECT_SIZE_PROPORTIONAL)))
        {
            if (!mpColumnItem.get() && !mpObjectItem.get() && mpParaItem.get())
            {
                // Left indent of the old position
                pIndents[INDENT_FIRST_LINE].nPos += _lDiff;
                pIndents[INDENT_LEFT_MARGIN].nPos += _lDiff;
                SetIndents(INDENT_COUNT, pIndents+INDENT_GAP);
            }

            if (mpColumnItem.get())
            {
                for(sal_uInt16 i = 0; i < mpColumnItem->Count()-1; ++i)
                    pBorders[i].nPos += _lDiff;
                SetBorders(mpColumnItem->Count()-1, pBorders);
                if (mpColumnItem->IsFirstAct())
                {
                    // Left indent of the old position
                    if(mpParaItem.get())
                    {
                        pIndents[INDENT_FIRST_LINE].nPos += _lDiff;
                        pIndents[INDENT_LEFT_MARGIN].nPos += _lDiff;
                        SetIndents(INDENT_COUNT, pIndents+INDENT_GAP);
                    }
                }
                else
                {
                    if(mpParaItem.get())
                    {
                        pIndents[INDENT_FIRST_LINE].nPos += _lDiff;
                        pIndents[INDENT_LEFT_MARGIN].nPos += _lDiff;
                        pIndents[INDENT_RIGHT_MARGIN].nPos += _lDiff;
                        SetIndents(INDENT_COUNT, pIndents+INDENT_GAP);
                    }
                }
            }
            if(mpTabStopItem.get())
            {
                ModifyTabs_Impl(nTabCount+TAB_GAP, pTabs, _lDiff);
                SetTabs(nTabCount, pTabs+TAB_GAP);
            }
        }
    }
}

void SvxRuler::DragMargin2()
{
    /* Dragging the right edge of frame */
    long aDragPosition = GetCorrectedDragPos( sal_True, !TAB_FLAG || !NEG_FLAG);
    aDragPosition = MakePositionSticky(aDragPosition, false);
    long lDiff = aDragPosition - GetMargin2();

    // Check if position changed
    if (lDiff == 0)
        return;

    if( pRuler_Imp->bIsTableRows &&
        !bHorz &&
        mpColumnItem.get() &&
        (nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL))
    {
        DragBorders();
    }

    sal_Bool bProtectColumns =
        pRuler_Imp->aProtectItem.IsSizeProtected() ||
        pRuler_Imp->aProtectItem.IsPosProtected();

    const sal_uInt16 nMarginStyle = bProtectColumns ? 0 : RULER_MARGIN_SIZEABLE;

    SetMargin2( aDragPosition, nMarginStyle );

    // Right indent of the old position
    if((!mpColumnItem.get() || IsActLastColumn()) && mpParaItem.get())
    {
        pIndents[INDENT_FIRST_LINE].nPos += lDiff;
        SetIndents(INDENT_COUNT, pIndents + INDENT_GAP);
    }

    DrawLine_Impl(lTabPos, ( TAB_FLAG && NEG_FLAG ) ? 5 : 7, bHorz);
}

void SvxRuler::DragIndents()
{
    /* Dragging the paragraph indents */
    long aDragPosition = NEG_FLAG ? GetDragPos() : GetCorrectedDragPos();
    const sal_uInt16 nIndex = GetDragAryPos() + INDENT_GAP;

    aDragPosition = MakePositionSticky(aDragPosition);

    const long lDiff = pIndents[nIndex].nPos - aDragPosition;

    // Check if position changed
    if (lDiff == 0)
        return;

    if((nIndex == INDENT_FIRST_LINE || nIndex == INDENT_LEFT_MARGIN )  &&
        (nDragType & DRAG_OBJECT_LEFT_INDENT_ONLY) != DRAG_OBJECT_LEFT_INDENT_ONLY)
    {
        pIndents[INDENT_FIRST_LINE].nPos -= lDiff;
    }

    pIndents[nIndex].nPos = aDragPosition;

    SetIndents(INDENT_COUNT, pIndents + INDENT_GAP);
    DrawLine_Impl(lTabPos, 1, bHorz);
}

void SvxRuler::DrawLine_Impl(long& lTabPosition, int nNew, sal_Bool bHorizontal)
{
    /*
       Output routine for the ledger line when moving tabs, tables and other
       columns
    */
    if(bHorizontal)
    {
        const long nHeight = pEditWin->GetOutputSize().Height();
        Point aZero = pEditWin->GetMapMode().GetOrigin();
        if(lTabPosition != -1)
        {
            pEditWin->InvertTracking(
                Rectangle( Point(lTabPosition, -aZero.Y()),
                           Point(lTabPosition, -aZero.Y() + nHeight)),
                SHOWTRACK_SPLIT | SHOWTRACK_CLIP );
        }
        if( nNew & 1 )
        {
            long nDrapPosition = GetCorrectedDragPos( ( nNew & 4 ) != 0, ( nNew & 2 ) != 0 );
            nDrapPosition = MakePositionSticky(nDrapPosition);
            lTabPosition = ConvertHSizeLogic( nDrapPosition + GetNullOffset() );
            if(mpPagePosItem.get())
                lTabPosition += mpPagePosItem->GetPos().X();
            pEditWin->InvertTracking(
                Rectangle( Point(lTabPosition, -aZero.Y()),
                           Point(lTabPosition, -aZero.Y() + nHeight) ),
                SHOWTRACK_CLIP | SHOWTRACK_SPLIT );
        }
    }
    else
    {
        const long nWidth = pEditWin->GetOutputSize().Width();
        Point aZero = pEditWin->GetMapMode().GetOrigin();
        if(lTabPosition != -1)
        {
            pEditWin->InvertTracking(
                Rectangle( Point(-aZero.X(),          lTabPosition),
                           Point(-aZero.X() + nWidth, lTabPosition)),
                SHOWTRACK_SPLIT | SHOWTRACK_CLIP );
        }

        if(nNew & 1)
        {
            long nDrapPosition = GetCorrectedDragPos();
            nDrapPosition = MakePositionSticky(nDrapPosition);
            lTabPosition = ConvertVSizeLogic(nDrapPosition + GetNullOffset());
            if(mpPagePosItem.get())
                lTabPosition += mpPagePosItem->GetPos().Y();
            pEditWin->InvertTracking(
                Rectangle( Point(-aZero.X(),        lTabPosition),
                           Point(-aZero.X()+nWidth, lTabPosition)),
                SHOWTRACK_CLIP | SHOWTRACK_SPLIT );
        }
    }
}

void SvxRuler::DragTabs()
{
    /* Dragging of Tabs */
    long aDragPosition = GetCorrectedDragPos(sal_True, sal_False);
    aDragPosition = MakePositionSticky(aDragPosition);

    sal_uInt16 nIdx = GetDragAryPos() + TAB_GAP;
    long nDiff = aDragPosition - pTabs[nIdx].nPos;
    if (nDiff == 0)
        return;

    DrawLine_Impl(lTabPos, 7, bHorz);

    if(nDragType & DRAG_OBJECT_SIZE_LINEAR)
    {

        for(sal_uInt16 i = nIdx; i < nTabCount; ++i)
        {
            pTabs[i].nPos += nDiff;
            // limit on maximum
            if(pTabs[i].nPos > GetMargin2())
                pTabs[nIdx].nStyle |= RULER_STYLE_INVISIBLE;
            else
                pTabs[nIdx].nStyle &= ~RULER_STYLE_INVISIBLE;
        }
    }
    else if(nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
    {
        pRuler_Imp->nTotalDist -= nDiff;
        pTabs[nIdx].nPos = aDragPosition;
        for(sal_uInt16 i = nIdx+1; i < nTabCount; ++i)
        {
            if(pTabs[i].nStyle & RULER_TAB_DEFAULT)
                // can be canceled at the DefaultTabs
                break;
            long nDelta = pRuler_Imp->nTotalDist * pRuler_Imp->pPercBuf[i];
            nDelta /= 1000;
            pTabs[i].nPos = pTabs[nIdx].nPos + nDelta;
            if(pTabs[i].nPos+GetNullOffset() > nMaxRight)
                pTabs[i].nStyle |= RULER_STYLE_INVISIBLE;
            else
                pTabs[i].nStyle &= ~RULER_STYLE_INVISIBLE;
        }
    }
    else
    {
        pTabs[nIdx].nPos = aDragPosition;
    }

    if(IsDragDelete())
        pTabs[nIdx].nStyle |= RULER_STYLE_INVISIBLE;
    else
        pTabs[nIdx].nStyle &= ~RULER_STYLE_INVISIBLE;
    SetTabs(nTabCount, pTabs+TAB_GAP);
}

void SvxRuler::SetActive(sal_Bool bOn)
{
    if(bOn)
    {
        Activate();
    }
    else
        Deactivate();
    if(bActive!=bOn)
    {
        pBindings->EnterRegistrations();
        if(bOn)
            for(sal_uInt16 i=0;i<pRuler_Imp->nControlerItems;i++)
                pCtrlItem[i]->ReBind();
        else
            for(sal_uInt16 j=0;j<pRuler_Imp->nControlerItems;j++)
                pCtrlItem[j]->UnBind();
        pBindings->LeaveRegistrations();
    }
    bActive = bOn;
}

void SvxRuler::UpdateParaContents_Impl(
                            long l,            // Difference
                            UpdateType eType)  // Art (all, left or right)
{
    /* Helper function; carry Tabs and Paragraph Margins */
    switch(eType) {
    case MOVE_RIGHT:
        pIndents[INDENT_RIGHT_MARGIN].nPos += l;
        break;
    case MOVE_ALL:
        pIndents[INDENT_RIGHT_MARGIN].nPos += l;
        // no break
    case MOVE_LEFT:
        {
            pIndents[INDENT_FIRST_LINE].nPos += l;
            pIndents[INDENT_LEFT_MARGIN].nPos += l;
            if ( pTabs )
            {
                for(sal_uInt16 i = 0; i < nTabCount+TAB_GAP;++i)
                    pTabs[i].nPos += l;
                SetTabs(nTabCount, pTabs+TAB_GAP);
            }
            break;
        }
    }
    SetIndents(INDENT_COUNT, pIndents+INDENT_GAP);
}

void SvxRuler::DragBorders()
{
    /* Dragging of Borders (Tables and other columns) */
    sal_Bool bLeftIndentsCorrected  = sal_False;
    sal_Bool bRightIndentsCorrected = sal_False;
    int nIndex;

    if(GetDragType() == RULER_TYPE_BORDER)
    {
        DrawLine_Impl(lTabPos, 7, bHorz);
        nIndex = GetDragAryPos();
    }
    else
    {
        nIndex = 0;
    }

    sal_uInt16 nDragSize = GetDragSize();
    long lDiff = 0;

    // the drag position has to be corrected to be able to prevent borders from passing each other
    long lPos = MakePositionSticky(GetCorrectedDragPos());

    switch(nDragSize)
    {
        case RULER_DRAGSIZE_MOVE:
        {
ADD_DEBUG_TEXT("lLastLMargin: ", OUString::number(pRuler_Imp->lLastLMargin))
            if(GetDragType() == RULER_TYPE_BORDER)
                lDiff = lPos - nDragOffset - pBorders[nIndex].nPos;
            else
                lDiff = GetDragType() == RULER_TYPE_MARGIN1 ? lPos - pRuler_Imp->lLastLMargin : lPos - pRuler_Imp->lLastRMargin;

            if(nDragType & DRAG_OBJECT_SIZE_LINEAR)
            {
                long nRight = GetMargin2() - lMinFrame; // Right limiters
                for(int i = nBorderCount - 2; i >= nIndex; --i)
                {
                    long l = pBorders[i].nPos;
                    pBorders[i].nPos += lDiff;
                    pBorders[i].nPos = std::min(pBorders[i].nPos, nRight - pBorders[i].nWidth);
                    nRight = pBorders[i].nPos - lMinFrame;
                    // RR update the column
                    if(i == GetActRightColumn())
                    {
                        UpdateParaContents_Impl(pBorders[i].nPos - l, MOVE_RIGHT);
                        bRightIndentsCorrected = sal_True;
                    }
                    // LAR, EZE update the column
                    else if(i == GetActLeftColumn())
                    {
                        UpdateParaContents_Impl(pBorders[i].nPos - l, MOVE_LEFT);
                        bLeftIndentsCorrected = sal_True;
                    }
                }
            }
            else if(nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
            {
                int nLimit;
                long lLeft;
                int nStartLimit = nBorderCount-2;
                switch(GetDragType())
                {
                default: ;//prevent warning
                    OSL_FAIL("svx::SvxRuler::DragBorders(), unknown drag type!" );
                case RULER_TYPE_BORDER:
                    if(pRuler_Imp->bIsTableRows)
                    {
                        pBorders[nIndex].nPos += lDiff;
                        if(bHorz)
                        {
                            lLeft = pBorders[nIndex].nPos;
                            pRuler_Imp->nTotalDist -= lDiff;
                            nLimit = nIndex + 1;
                        }
                        else
                        {
                            lLeft = 0;
                            nStartLimit = nIndex - 1;
                            pRuler_Imp->nTotalDist += lDiff;
                            nLimit = 0;
                        }
                    }
                    else
                    {
                        nLimit = nIndex + 1;
                        pBorders[nIndex].nPos += lDiff;
                        lLeft = pBorders[nIndex].nPos;
                        pRuler_Imp->nTotalDist -= lDiff;
                    }
                break;
                case RULER_TYPE_MARGIN1:
                    nLimit = 0;
                    lLeft = pRuler_Imp->lLastLMargin + lDiff;
                    pRuler_Imp->nTotalDist -= lDiff;
                break;
                case RULER_TYPE_MARGIN2:
                    nLimit = 0;
                    lLeft= 0;
                    nStartLimit = nBorderCount - 2;
                    pRuler_Imp->nTotalDist += lDiff;
                break;
                }

                for(int i  = nStartLimit; i >= nLimit; --i)
                {

                    long l = pBorders[i].nPos;
                    pBorders[i].nPos=lLeft+
                        (pRuler_Imp->nTotalDist*pRuler_Imp->pPercBuf[i])/1000+
                            pRuler_Imp->pBlockBuf[i];

                    // RR update the column
                    if(!pRuler_Imp->bIsTableRows)
                    {
                        if(i == GetActRightColumn())
                        {
                            UpdateParaContents_Impl(pBorders[i].nPos - l, MOVE_RIGHT);
                            bRightIndentsCorrected = sal_True;
                        }
                        // LAR, EZE update the column
                        else if(i == GetActLeftColumn())
                        {
                            UpdateParaContents_Impl(pBorders[i].nPos - l, MOVE_LEFT);
                            bLeftIndentsCorrected = sal_True;
                        }
                    }
                }
                if(pRuler_Imp->bIsTableRows)
                {
                    //in vertical tables the left borders have to be moved
                    if(bHorz)
                    {
                        for(int i  = 0; i < nIndex; ++i)
                            pBorders[i].nPos += lDiff;
                        AdjustMargin1(lDiff);
                    }
                    else
                    {
                        //otherwise the right borders are moved
                        for(int i  = mpColumnItem->Count() - 1; i > nIndex; --i)
                            pBorders[i].nPos += lDiff;
                        SetMargin2( GetMargin2() + lDiff, 0 );
                    }
                }
            }
            else if(pRuler_Imp->bIsTableRows)
            {
                //moving rows: if a row is resized all following rows
                //have to be moved by the same amount.
                //This includes the left border when the table is not limited
                //to a lower frame border.
                int nLimit;
                if(GetDragType()==RULER_TYPE_BORDER)
                {
                    nLimit = nIndex + 1;
                    pBorders[nIndex].nPos += lDiff;
                }
                else
                {
                    nLimit=0;
                }
                //in vertical tables the left borders have to be moved
                if(bHorz)
                {
                    for(int i  = 0; i < nIndex; ++i)
                    {
                        pBorders[i].nPos += lDiff;
                    }
                    AdjustMargin1(lDiff);
                }
                else
                {
                    //otherwise the right borders are moved
                    for(int i  = nBorderCount-2; i >= nLimit; --i)
                    {
                        pBorders[i].nPos += lDiff;
                    }
                    SetMargin2( GetMargin2() + lDiff, 0 );
                }
            }
            else
                pBorders[nIndex].nPos += lDiff;
            break;
        }
      case RULER_DRAGSIZE_1:
        {
            lDiff = lPos - pBorders[nIndex].nPos;
            pBorders[nIndex].nWidth += pBorders[nIndex].nPos - lPos;
            pBorders[nIndex].nPos = lPos;
            break;
        }
      case RULER_DRAGSIZE_2:
        {
            const long nOld = pBorders[nIndex].nWidth;
            pBorders[nIndex].nWidth = lPos - pBorders[nIndex].nPos;
            lDiff = pBorders[nIndex].nWidth - nOld;
            break;
        }
    }
    if(!bRightIndentsCorrected &&
       GetActRightColumn() == nIndex &&
       nDragSize != RULER_DRAGSIZE_2 &&
       pIndents &&
       !pRuler_Imp->bIsTableRows)
    {
        UpdateParaContents_Impl(lDiff, MOVE_RIGHT);
    }
    else if(!bLeftIndentsCorrected &&
            GetActLeftColumn() == nIndex &&
            nDragSize != RULER_DRAGSIZE_1 &&
            pIndents)
    {
        UpdateParaContents_Impl(lDiff, MOVE_LEFT);
    }
    SetBorders(mpColumnItem->Count() - 1, pBorders);
}

void SvxRuler::DragObjectBorder()
{
    /* Dragging of object edges */
    if(RULER_DRAGSIZE_MOVE == GetDragSize())
    {
        const long lPosition = MakePositionSticky(GetCorrectedDragPos());

        const sal_uInt16 nIdx = GetDragAryPos();
        pObjectBorders[GetObjectBordersOff(nIdx)].nPos = lPosition;
        SetBorders(2, pObjectBorders + GetObjectBordersOff(0));
        DrawLine_Impl(lTabPos, 7, bHorz);

    }
}

void SvxRuler::ApplyMargins()
{
    /* Applying margins; changed by dragging. */
    const SfxPoolItem *pItem = 0;
    sal_uInt16 nId = SID_ATTR_LONG_LRSPACE;
    if(bHorz)
    {
        const long lOldNull = lLogicNullOffset;
        if(pRuler_Imp->lMaxLeftLogic!=-1&&nMaxLeft==GetMargin1()+Ruler::GetNullOffset())
            mpLRSpaceItem->SetLeft(lLogicNullOffset=pRuler_Imp->lMaxLeftLogic);
        else
            mpLRSpaceItem->SetLeft(PixelHAdjust(
                lLogicNullOffset =  ConvertHPosLogic(GetFrameLeft()) -
                lAppNullOffset, mpLRSpaceItem->GetLeft()));

        if(bAppSetNullOffset)
            lAppNullOffset += lLogicNullOffset - lOldNull;

        if(pRuler_Imp->lMaxRightLogic!=-1
           &&nMaxRight==GetMargin2()+Ruler::GetNullOffset())
            mpLRSpaceItem->SetRight(GetPageWidth()-pRuler_Imp->lMaxRightLogic);
        else
            mpLRSpaceItem->SetRight(
                PixelHAdjust(
                    std::max((long)0,mpPagePosItem->GetWidth() -
                        mpLRSpaceItem->GetLeft() -
                        (ConvertHPosLogic(GetMargin2()) -
                         lAppNullOffset)),mpLRSpaceItem->GetRight()));
        pItem = mpLRSpaceItem.get();
#ifdef DEBUGLIN
        Debug_Impl(pEditWin, *mpLRSpaceItem);
#endif // DEBUGLIN
    }
    else {
        const long lOldNull = lLogicNullOffset;
        mpULSpaceItem->SetUpper(
            PixelVAdjust(
                lLogicNullOffset =
                ConvertVPosLogic(GetFrameLeft()) -
                lAppNullOffset,mpULSpaceItem->GetUpper()));
        if(bAppSetNullOffset)
            lAppNullOffset += lLogicNullOffset - lOldNull;
        mpULSpaceItem->SetLower(
            PixelVAdjust(
                std::max((long)0, mpPagePosItem->GetHeight() -
                    mpULSpaceItem->GetUpper() -
                    (ConvertVPosLogic(GetMargin2()) -
                     lAppNullOffset)),mpULSpaceItem->GetLower()));
        pItem = mpULSpaceItem.get();
        nId = SID_ATTR_LONG_ULSPACE;
#ifdef DEBUGLIN
        Debug_Impl(pEditWin,*mpULSpaceItem.get());
#endif // DEBUGLIN
    }
    pBindings->GetDispatcher()->Execute( nId, SFX_CALLMODE_RECORD, pItem, 0L );
    if(mpTabStopItem.get())
        UpdateTabs();
}

void SvxRuler::ApplyIndents()
{
    /* Applying paragraph settings; changed by dragging. */
    long nNewTxtLeft;
    if(mpColumnItem.get() && !IsActFirstColumn(sal_True))
    {
        long nLeftCol = GetActLeftColumn(sal_True);
        nNewTxtLeft =
            PixelHAdjust(
                ConvertHPosLogic(
                    pIndents[INDENT_LEFT_MARGIN].nPos -
                    (pBorders[nLeftCol].nPos +
                    pBorders[nLeftCol].nWidth)) -
                lAppNullOffset,
                mpParaItem->GetTxtLeft());
    }
    else
    {
        nNewTxtLeft = PixelHAdjust(
                        ConvertHPosLogic(pIndents[INDENT_LEFT_MARGIN].nPos),
                        mpParaItem->GetTxtLeft());
    }

    sal_Bool bRTL = pRuler_Imp->pTextRTLItem && pRuler_Imp->pTextRTLItem->GetValue();

    long nNewFirstLineOffset;

    if(bRTL)
    {
        long nRightFrameMargin = GetRightFrameMargin();
        nNewFirstLineOffset = PixelHAdjust(
                                nRightFrameMargin -
                                    ConvertHPosLogic(pIndents[INDENT_FIRST_LINE].nPos ) -
                                    lAppNullOffset,
                                mpParaItem->GetTxtFirstLineOfst());
    }
    else
    {
        nNewFirstLineOffset = PixelHAdjust(
                                ConvertHPosLogic(pIndents[INDENT_FIRST_LINE].nPos -
                                    pIndents[INDENT_LEFT_MARGIN].nPos) -
                                    lAppNullOffset,
                                mpParaItem->GetTxtFirstLineOfst());
    }

    // If the new TxtLeft is smaller than the old FirstLineIndent, then the
    // difference is lost and the paragraph is in total indented too far,
    // so first set the FirstLineOffset, then the TxtLeft

    if(bRTL)
    {
        long nLeftFrameMargin = GetLeftFrameMargin();
        long nRightFrameMargin = GetRightFrameMargin();
        nNewTxtLeft = nRightFrameMargin - nNewTxtLeft - nLeftFrameMargin;
        nNewFirstLineOffset -= nNewTxtLeft;
        if(mpParaBorderItem.get())
        {
            nNewTxtLeft += mpParaBorderItem->GetLeft() + mpParaBorderItem->GetRight();
            nNewFirstLineOffset -= mpParaBorderItem->GetRight();
        }
    }
    mpParaItem->SetTxtFirstLineOfst(sal::static_int_cast< short >(nNewFirstLineOffset));
    mpParaItem->SetTxtLeft(nNewTxtLeft);

    if(mpColumnItem.get() && ((!bRTL && !IsActLastColumn( sal_True )) || (bRTL && !IsActFirstColumn())))
    {
        if(bRTL)
        {
            long nActBorder = pBorders[GetActLeftColumn( sal_True )].nPos;
            long nRightMargin = pIndents[INDENT_RIGHT_MARGIN].nPos;
            long nConvert = ConvertHPosLogic( nRightMargin - nActBorder );
            mpParaItem->SetRight( PixelHAdjust( nConvert - lAppNullOffset, mpParaItem->GetRight() ) );
        }
        else
        {
            mpParaItem->SetRight(
                PixelHAdjust(
                    ConvertHPosLogic(
                        pBorders[GetActRightColumn( sal_True )].nPos -
                            pIndents[INDENT_RIGHT_MARGIN].nPos) -
                            lAppNullOffset,
                        mpParaItem->GetRight()));
        }
    }
    else
    {
        if(bRTL)
        {
            mpParaItem->SetRight( PixelHAdjust(
                ConvertHPosLogic(GetMargin1() +
                             pIndents[INDENT_RIGHT_MARGIN].nPos) - GetLeftFrameMargin() +
                             (mpParaBorderItem.get() ? mpParaBorderItem->GetLeft() : 0) -
                lAppNullOffset, mpParaItem->GetRight()));
        }
        else
        {
            mpParaItem->SetRight( PixelHAdjust(
                ConvertHPosLogic(GetMargin2() -
                             pIndents[INDENT_RIGHT_MARGIN].nPos) -
                lAppNullOffset, mpParaItem->GetRight()));
        }
    }
    sal_uInt16 nParaId  = bHorz ? SID_ATTR_PARA_LRSPACE : SID_ATTR_PARA_LRSPACE_VERTICAL;
    pBindings->GetDispatcher()->Execute( nParaId, SFX_CALLMODE_RECORD, mpParaItem.get(), 0L );
    UpdateTabs();
}

void SvxRuler::ApplyTabs()
{
    /* Apply tab settings, changed by dragging. */
    sal_Bool bRTL = pRuler_Imp->pTextRTLItem && pRuler_Imp->pTextRTLItem->GetValue();
    const sal_uInt16 nCoreIdx = GetDragAryPos();
    if(IsDragDelete())
    {
        mpTabStopItem->Remove(nCoreIdx);
    }
    else if(DRAG_OBJECT_SIZE_LINEAR & nDragType ||
            DRAG_OBJECT_SIZE_PROPORTIONAL & nDragType)
    {
        SvxTabStopItem *pItem = new SvxTabStopItem(mpTabStopItem->Which());
        //remove default tab stops
        for ( sal_uInt16 i = 0; i < pItem->Count(); )
        {
            if ( SVX_TAB_ADJUST_DEFAULT == (*pItem)[i].GetAdjustment() )
            {
                pItem->Remove(i);
                continue;
            }
            ++i;
        }

        sal_uInt16 j;
        for(j = 0; j < nCoreIdx; ++j)
        {
            pItem->Insert((*mpTabStopItem.get())[j]);
        }
        for(; j < mpTabStopItem->Count(); ++j)
        {
            SvxTabStop aTabStop = (*mpTabStopItem.get())[j];
            aTabStop.GetTabPos() = PixelHAdjust(
                ConvertHPosLogic(pTabs[j+TAB_GAP].nPos -
                                 GetLeftIndent()) -
                lAppNullOffset,
                aTabStop.GetTabPos());
            pItem->Insert(aTabStop);
        }
        mpTabStopItem.reset(pItem);
    }
    else if( mpTabStopItem->Count() == 0 )
        return;
    else
    {
        SvxTabStop aTabStop = (*mpTabStopItem.get())[nCoreIdx];
        if(pRuler_Imp->lMaxRightLogic!=-1&&
           pTabs[nCoreIdx+TAB_GAP].nPos+Ruler::GetNullOffset()==nMaxRight)
            aTabStop.GetTabPos() = pRuler_Imp->lMaxRightLogic-lLogicNullOffset;
        else
        {
            if(bRTL)
            {
                //#i24363# tab stops relative to indent
                const long nTmpLeftIndent = pRuler_Imp->bIsTabsRelativeToIndent ?
                                            GetLeftIndent() :
                                            ConvertHPosPixel( GetRightFrameMargin() + lAppNullOffset );

                aTabStop.GetTabPos() = PixelHAdjust(
                    ConvertHPosLogic( nTmpLeftIndent - pTabs[nCoreIdx+TAB_GAP].nPos) - lAppNullOffset,
                                                                                        aTabStop.GetTabPos());
            }
            else
            {
                //#i24363# tab stops relative to indent
                const long nTmpLeftIndent = pRuler_Imp->bIsTabsRelativeToIndent ?
                                            GetLeftIndent() :
                                            0;

                aTabStop.GetTabPos() = PixelHAdjust(
                    ConvertHPosLogic( pTabs[nCoreIdx+TAB_GAP].nPos - nTmpLeftIndent ) - lAppNullOffset,
                                                                                         aTabStop.GetTabPos() );
            }
        }
        mpTabStopItem->Remove(nCoreIdx);
        mpTabStopItem->Insert(aTabStop);
    }
    sal_uInt16 nTabStopId = bHorz ? SID_ATTR_TABSTOP : SID_ATTR_TABSTOP_VERTICAL;
    pBindings->GetDispatcher()->Execute( nTabStopId, SFX_CALLMODE_RECORD, mpTabStopItem.get(), 0L );
    UpdateTabs();
}

void SvxRuler::ApplyBorders()
{
    /* Applying (table) column settings; changed by dragging. */
    if(mpColumnItem->IsTable())
    {
        long l = GetFrameLeft();
        if(l != pRuler_Imp->nColLeftPix)
            mpColumnItem->SetLeft( PixelHAdjust(
                ConvertHPosLogic(l) - lAppNullOffset, mpColumnItem->GetLeft()));
        l = GetMargin2();
        if(l != pRuler_Imp->nColRightPix)
        {
            long nWidthOrHeight = bHorz ? mpPagePosItem->GetWidth() : mpPagePosItem->GetHeight();
            mpColumnItem->SetRight( PixelHAdjust( nWidthOrHeight -
                    mpColumnItem->GetLeft() - ConvertHPosLogic(l) -
                    lAppNullOffset, mpColumnItem->GetRight() ) );
        }
    }
    for(sal_uInt16 i = 0; i < mpColumnItem->Count()-1; ++i)
    {
        long& nEnd = (*mpColumnItem.get())[i].nEnd;
        nEnd = PixelHAdjust(
            ConvertPosLogic(pBorders[i].nPos),
            (*mpColumnItem.get())[i].nEnd);
        long& nStart = (*mpColumnItem.get())[i+1].nStart;
        nStart = PixelHAdjust(
            ConvertSizeLogic(pBorders[i].nPos +
                             pBorders[i].nWidth) -
            lAppNullOffset,
            (*mpColumnItem.get())[i+1].nStart);
        // It may be that, due to the PixelHAdjust readjustment to old values,
        // the width becomes  < 0. This we readjust.
        if( nEnd > nStart )
            nStart = nEnd;
    }
#ifdef DEBUGLIN
        Debug_Impl(pEditWin,*mpColumnItem.get());
#endif // DEBUGLIN
    SfxBoolItem aFlag(SID_RULER_ACT_LINE_ONLY,
                      nDragType & DRAG_OBJECT_ACTLINE_ONLY? sal_True : sal_False);
    sal_uInt16 nColId = pRuler_Imp->bIsTableRows ? (bHorz ? SID_RULER_ROWS : SID_RULER_ROWS_VERTICAL) :
                            (bHorz ? SID_RULER_BORDERS : SID_RULER_BORDERS_VERTICAL);
    pBindings->GetDispatcher()->Execute( nColId, SFX_CALLMODE_RECORD, mpColumnItem.get(), &aFlag, 0L );
}

void SvxRuler::ApplyObject()
{
    /* Applying object settings, changed by dragging. */

    // to the page margin
    long nMargin = mpLRSpaceItem.get() ? mpLRSpaceItem->GetLeft(): 0;
    mpObjectItem->SetStartX(
                           PixelAdjust(
                              ConvertPosLogic(pObjectBorders[0].nPos)
                              + nMargin - lAppNullOffset,mpObjectItem->GetStartX()));
    mpObjectItem->SetEndX(
                         PixelAdjust(
                             ConvertPosLogic(pObjectBorders[1].nPos)
                         + nMargin -  lAppNullOffset,mpObjectItem->GetEndX()));
    nMargin = mpULSpaceItem.get() ? mpULSpaceItem->GetUpper() : 0;
    mpObjectItem->SetStartY(
                         PixelAdjust(
                             ConvertPosLogic(pObjectBorders[2].nPos)
                           + nMargin - lAppNullOffset,mpObjectItem->GetStartY()));
    mpObjectItem->SetEndY(
                     PixelAdjust(
                         ConvertPosLogic(pObjectBorders[3].nPos)
                         + nMargin - lAppNullOffset,mpObjectItem->GetEndY()));
    pBindings->GetDispatcher()->Execute( SID_RULER_OBJECT, SFX_CALLMODE_RECORD, mpObjectItem.get(), 0L );
}

void SvxRuler::PrepareProportional_Impl(RulerType eType)
{
    /*
       Preparation proportional dragging, and it is calculated based on the
       proportional share of the total width in parts per thousand.
    */
    pRuler_Imp->nTotalDist = GetMargin2();
    switch((int)eType)
    {
      case RULER_TYPE_MARGIN2:
      case RULER_TYPE_MARGIN1:
      case RULER_TYPE_BORDER:
        {
            DBG_ASSERT(mpColumnItem.get(), "no ColumnItem");

            pRuler_Imp->SetPercSize(mpColumnItem->Count());

            long lPos;
            long lWidth=0;
            sal_uInt16 nStart;
            sal_uInt16 nIdx=GetDragAryPos();
            long lActWidth=0;
            long lActBorderSum;
            long lOrigLPos;

            if(eType != RULER_TYPE_BORDER)
            {
                lOrigLPos = GetMargin1();
                nStart = 0;
                lActBorderSum = 0;
            }
            else
            {
                if(pRuler_Imp->bIsTableRows &&!bHorz)
                {
                    lOrigLPos = GetMargin1();
                    nStart = 0;
                }
                else
                {
                    lOrigLPos = pBorders[nIdx].nPos + pBorders[nIdx].nWidth;
                    nStart = 1;
                }
                lActBorderSum = pBorders[nIdx].nWidth;
            }

            //in horizontal mode the percentage value has to be
            //calculated on a "current change" position base
            //because the height of the table changes while dragging
            if(pRuler_Imp->bIsTableRows && RULER_TYPE_BORDER == eType)
            {
                sal_uInt16 nStartBorder;
                sal_uInt16 nEndBorder;
                if(bHorz)
                {
                    nStartBorder = nIdx + 1;
                    nEndBorder = mpColumnItem->Count() - 1;
                }
                else
                {
                    nStartBorder = 0;
                    nEndBorder = nIdx;
                }

                lWidth = pBorders[nIdx].nPos;
                if(bHorz)
                    lWidth = GetMargin2() - lWidth;
                pRuler_Imp->nTotalDist = lWidth;
                lPos = lOrigLPos = pBorders[nIdx].nPos;

                for(sal_uInt16 i = nStartBorder; i < nEndBorder; ++i)
                {
                    if(bHorz)
                    {
                        lActWidth += pBorders[i].nPos - lPos;
                        lPos = pBorders[i].nPos + pBorders[i].nWidth;
                    }
                    else
                        lActWidth = pBorders[i].nPos;
                    pRuler_Imp->pPercBuf[i] = (sal_uInt16)((lActWidth * 1000)
                                                    / pRuler_Imp->nTotalDist);
                    pRuler_Imp->pBlockBuf[i] = (sal_uInt16)lActBorderSum;
                    lActBorderSum += pBorders[i].nWidth;
                }
            }
            else
            {
                lPos = lOrigLPos;
                for(sal_uInt16 ii = nStart; ii < mpColumnItem->Count() - 1; ++ii)
                {
                    lWidth += pBorders[ii].nPos - lPos;
                    lPos = pBorders[ii].nPos + pBorders[ii].nWidth;
                }

                lWidth += GetMargin2() - lPos;
                pRuler_Imp->nTotalDist = lWidth;
                lPos = lOrigLPos;

                for(sal_uInt16 i = nStart; i < mpColumnItem->Count() - 1; ++i)
                {
                    lActWidth += pBorders[i].nPos - lPos;
                    lPos = pBorders[i].nPos + pBorders[i].nWidth;
                    pRuler_Imp->pPercBuf[i] = (sal_uInt16)((lActWidth * 1000)
                                                    / pRuler_Imp->nTotalDist);
                    pRuler_Imp->pBlockBuf[i] = (sal_uInt16)lActBorderSum;
                    lActBorderSum += pBorders[i].nWidth;
                }
            }
        }
        break;
        case RULER_TYPE_TAB:
        {
            const sal_uInt16 nIdx = GetDragAryPos()+TAB_GAP;
            pRuler_Imp->nTotalDist -= pTabs[nIdx].nPos;
            pRuler_Imp->SetPercSize(nTabCount);
            for(sal_uInt16 n=0;n<=nIdx;pRuler_Imp->pPercBuf[n++]=0) ;
            for(sal_uInt16 i = nIdx+1; i < nTabCount; ++i)
            {
                const long nDelta = pTabs[i].nPos - pTabs[nIdx].nPos;
                pRuler_Imp->pPercBuf[i] = (sal_uInt16)((nDelta * 1000) / pRuler_Imp->nTotalDist);
            }
            break;
        }
    }
}

void SvxRuler::EvalModifier()
{
    /*
    Eval Drag Modifier
    Shift: move linear
    Control: move proportional
    Shift+Control: Table: only current line
    Alt: dimension arrows (not implemented) //!!

    */

    sal_uInt16 nModifier = GetDragModifier();
    if(pRuler_Imp->bIsTableRows)
    {
        //rows can only be moved in one way, additionally current column is possible
        if(nModifier == KEY_SHIFT)
            nModifier = 0;
    }
    switch(nModifier)
    {
     case KEY_SHIFT:
        nDragType = DRAG_OBJECT_SIZE_LINEAR;
        break;
     case KEY_MOD1:  {
         const RulerType eType = GetDragType();
         nDragType = DRAG_OBJECT_SIZE_PROPORTIONAL;
         if( RULER_TYPE_TAB == eType ||
             ( ( RULER_TYPE_BORDER == eType || RULER_TYPE_MARGIN1 == eType ||
                 RULER_TYPE_MARGIN2 == eType ) &&
               mpColumnItem.get() ) )
             PrepareProportional_Impl(eType);
         break;
     }
     case KEY_MOD1 | KEY_SHIFT:
        if(GetDragType()!=RULER_TYPE_MARGIN1&&
           GetDragType()!=RULER_TYPE_MARGIN2)
            nDragType = DRAG_OBJECT_ACTLINE_ONLY;
        break;
        // ALT: Dimension arrows
    }
}

void SvxRuler::Click()
{
    /* Overloaded handler SV; sets Tab per dispatcher call */
    Ruler::Click();
    if( bActive )
    {
        pBindings->Update( SID_RULER_LR_MIN_MAX );
        pBindings->Update( SID_ATTR_LONG_ULSPACE );
        pBindings->Update( SID_ATTR_LONG_LRSPACE );
        pBindings->Update( SID_RULER_PAGE_POS );
        pBindings->Update( bHorz ? SID_ATTR_TABSTOP : SID_ATTR_TABSTOP_VERTICAL);
        pBindings->Update( bHorz ? SID_ATTR_PARA_LRSPACE : SID_ATTR_PARA_LRSPACE_VERTICAL);
        pBindings->Update( bHorz ? SID_RULER_BORDERS : SID_RULER_BORDERS_VERTICAL);
        pBindings->Update( bHorz ? SID_RULER_ROWS : SID_RULER_ROWS_VERTICAL);
        pBindings->Update( SID_RULER_OBJECT );
        pBindings->Update( SID_RULER_PROTECT );
        pBindings->Update( SID_ATTR_PARA_LRSPACE_VERTICAL );
    }
    sal_Bool bRTL = pRuler_Imp->pTextRTLItem && pRuler_Imp->pTextRTLItem->GetValue();
    if(mpTabStopItem.get() &&
       (nFlags & SVXRULER_SUPPORT_TABS) == SVXRULER_SUPPORT_TABS)
    {
        sal_Bool bContentProtected = pRuler_Imp->aProtectItem.IsCntntProtected();
        if( bContentProtected ) return;
        const long lPos = GetClickPos();
        if((bRTL && lPos < std::min(GetFirstLineIndent(), GetLeftIndent()) && lPos > GetRightIndent()) ||
            (!bRTL && lPos > std::min(GetFirstLineIndent(), GetLeftIndent()) && lPos < GetRightIndent()))
        {
            //convert position in left-to-right text
            long nTabPos;
    //#i24363# tab stops relative to indent
            if(bRTL)
                nTabPos = ( pRuler_Imp->bIsTabsRelativeToIndent ?
                            GetLeftIndent() :
                            ConvertHPosPixel( GetRightFrameMargin() + lAppNullOffset ) ) -
                          lPos;
            else
                nTabPos = lPos -
                          ( pRuler_Imp->bIsTabsRelativeToIndent ?
                            GetLeftIndent() :
                            0 );

            SvxTabStop aTabStop(ConvertHPosLogic(nTabPos),
                                ToAttrTab_Impl(nDefTabType));
            mpTabStopItem->Insert(aTabStop);
            UpdateTabs();
        }
    }
}

sal_Bool SvxRuler::CalcLimits ( long &nMax1,    // minimum value to be set
                                long &nMax2,    // minimum value to be set
                                sal_Bool ) const
{
    /*
       Default implementation of the virtual function; the application can be
       overloaded to implement customized limits. The values are based on the page.
    */
    nMax1 = LONG_MIN;
    nMax2 = LONG_MAX;
    return sal_False;
}

void SvxRuler::CalcMinMax()
{
    /*
       Calculates the limits for dragging; which are in pixels relative to the
       page edge
    */
    sal_Bool bRTL = pRuler_Imp->pTextRTLItem && pRuler_Imp->pTextRTLItem->GetValue();
    const long lNullPix = ConvertPosPixel(lLogicNullOffset);
    pRuler_Imp->lMaxLeftLogic=pRuler_Imp->lMaxRightLogic=-1;
    switch(GetDragType())
    {
        case RULER_TYPE_MARGIN1:
        {        // left edge of the surrounding Frame
            // DragPos - NOf between left - right
            pRuler_Imp->lMaxLeftLogic = GetLeftMin();
            nMaxLeft=ConvertSizePixel(pRuler_Imp->lMaxLeftLogic);

            if (!mpColumnItem.get() || mpColumnItem->Count() == 1 )
            {
                if(bRTL)
                {
                    nMaxRight = lNullPix - GetRightIndent() +
                        std::max(GetFirstLineIndent(), GetLeftIndent()) -
                        lMinFrame;
                }
                else
                {
                    nMaxRight = lNullPix + GetRightIndent() -
                        std::max(GetFirstLineIndent(), GetLeftIndent()) -
                        lMinFrame;
                }
            }
            else if(pRuler_Imp->bIsTableRows)
            {
                //top border is not moveable when table rows are displayed
                // protection of content means the margin is not moveable
                // - it's just a page break inside of a cell
                if(bHorz && !pRuler_Imp->aProtectItem.IsCntntProtected())
                {
                    nMaxLeft = pBorders[0].nMinPos + lNullPix;
                    if(nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
                        nMaxRight = GetRightIndent() + lNullPix -
                                (mpColumnItem->Count() - 1 ) * lMinFrame;
                    else
                        nMaxRight = pBorders[0].nPos - lMinFrame + lNullPix;
                }
                else
                    nMaxLeft = nMaxRight = lNullPix;
            }
            else
            {
                if (nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
                {
                    nMaxRight=lNullPix+CalcPropMaxRight();
                }
                else if (nDragType & DRAG_OBJECT_SIZE_LINEAR)
                {
                    nMaxRight = ConvertPosPixel(
                        GetPageWidth() - (
                            (mpColumnItem->IsTable() && mpLRSpaceItem.get())
                            ? mpLRSpaceItem->GetRight() : 0))
                            - GetMargin2() + GetMargin1();
                }
                else
                {
                    nMaxRight = lNullPix - lMinFrame;
                    if (mpColumnItem->IsFirstAct())
                    {
                        if(bRTL)
                        {
                            nMaxRight += std::min(
                                pBorders[0].nPos,
                                std::max(GetFirstLineIndent(), GetLeftIndent()) - GetRightIndent());
                        }
                        else
                        {
                            nMaxRight += std::min(
                                pBorders[0].nPos, GetRightIndent() -
                                std::max(GetFirstLineIndent(), GetLeftIndent()));
                        }
                    }
                    else if ( mpColumnItem->Count() > 1 )
                    {
                        nMaxRight += pBorders[0].nPos;
                    }
                    else
                    {
                        nMaxRight += GetRightIndent() - std::max(GetFirstLineIndent(), GetLeftIndent());
                    }
                    // Do not drag the left table edge over the edge of the page
                    if(mpLRSpaceItem.get() && mpColumnItem->IsTable())
                    {
                        long nTmp=ConvertSizePixel(mpLRSpaceItem->GetLeft());
                        if(nTmp>nMaxLeft)
                            nMaxLeft=nTmp;
                    }
                }
            }
            break;
        }
        case RULER_TYPE_MARGIN2:
        {        // right edge of the surrounding Frame
            pRuler_Imp->lMaxRightLogic =
                mpMinMaxItem.get() ?
                    GetPageWidth() - GetRightMax() :
                    GetPageWidth();
            nMaxRight = ConvertSizePixel(pRuler_Imp->lMaxRightLogic);


            if(!mpColumnItem.get())
            {
                if(bRTL)
                {
                    nMaxLeft =  GetMargin2() + GetRightIndent() -
                        std::max(GetFirstLineIndent(),GetLeftIndent())  - GetMargin1()+
                            lMinFrame + lNullPix;
                }
                else
                {
                    nMaxLeft =  GetMargin2() - GetRightIndent() +
                        std::max(GetFirstLineIndent(),GetLeftIndent())  - GetMargin1()+
                            lMinFrame + lNullPix;
                }
            }
            else if(pRuler_Imp->bIsTableRows)
            {
                // get the bottom move range from the last border position - only available for rows!
                // protection of content means the margin is not moveable - it's just a page break inside of a cell
                if(bHorz || pRuler_Imp->aProtectItem.IsCntntProtected())
                {
                    nMaxLeft = nMaxRight = pBorders[mpColumnItem->Count() - 1].nMaxPos + lNullPix;
                }
                else
                {
                    if(nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
                    {
                        nMaxLeft = (mpColumnItem->Count()) * lMinFrame + lNullPix;
                    }
                    else
                    {
                        if(mpColumnItem->Count() > 1)
                            nMaxLeft = pBorders[mpColumnItem->Count() - 2].nPos + lMinFrame + lNullPix;
                        else
                            nMaxLeft = lMinFrame + lNullPix;
                    }
                    if(mpColumnItem->Count() > 1)
                        nMaxRight = pBorders[mpColumnItem->Count() - 2].nMaxPos + lNullPix;
                    else
                        nMaxRight -= GetRightIndent() - lNullPix;
                }
            }
            else
            {
                nMaxLeft = lMinFrame + lNullPix;
                if(IsActLastColumn() || mpColumnItem->Count() < 2 ) //If last active column
                {
                    if(bRTL)
                    {
                        nMaxLeft = lMinFrame + lNullPix + GetMargin2() +
                            GetRightIndent() - std::max(GetFirstLineIndent(),
                                                   GetLeftIndent());
                    }
                    else
                    {
                        nMaxLeft = lMinFrame + lNullPix + GetMargin2() -
                            GetRightIndent() + std::max(GetFirstLineIndent(),
                                                   GetLeftIndent());
                    }
                }
                if( mpColumnItem->Count() >= 2 )
                {
                    long nNewMaxLeft =
                        lMinFrame + lNullPix +
                        pBorders[mpColumnItem->Count() - 2].nPos +
                        pBorders[mpColumnItem->Count() - 2].nWidth;
                    nMaxLeft = std::max(nMaxLeft, nNewMaxLeft);
                }

            }
            break;
        }
        case RULER_TYPE_BORDER:
        {                // Table, column (Modifier)
        const sal_uInt16 nIdx = GetDragAryPos();
        switch(GetDragSize())
        {
          case RULER_DRAGSIZE_1 :
            {
                nMaxRight = pBorders[nIdx].nPos +
                    pBorders[nIdx].nWidth + lNullPix;

                if(0 == nIdx)
                    nMaxLeft = lNullPix;
                else
                    nMaxLeft = pBorders[nIdx - 1].nPos + pBorders[nIdx - 1].nWidth + lNullPix;
                if(nIdx == mpColumnItem->GetActColumn())
                {
                    if(bRTL)
                    {
                        nMaxLeft += pBorders[nIdx].nPos +
                            GetRightIndent() - std::max(GetFirstLineIndent(),
                                                   GetLeftIndent());
                    }
                    else
                    {
                        nMaxLeft += pBorders[nIdx].nPos -
                            GetRightIndent() + std::max(GetFirstLineIndent(),
                                                   GetLeftIndent());
                    }
                    if(0 != nIdx)
                        nMaxLeft -= pBorders[nIdx-1].nPos +
                            pBorders[nIdx-1].nWidth;
                }
                nMaxLeft += lMinFrame;
                nMaxLeft += nDragOffset;
                break;
            }
          case RULER_DRAGSIZE_MOVE:
            {
                if(mpColumnItem.get())
                {
                    //nIdx contains the position of the currently moved item
                    //next visible separator on the left
                    sal_uInt16 nLeftCol=GetActLeftColumn(sal_False, nIdx);
                    //next visible separator on the right
                    sal_uInt16 nRightCol=GetActRightColumn(sal_False, nIdx);
                    //next separator on the left - regardless if visible or not
                    sal_uInt16 nActLeftCol=GetActLeftColumn();
                    //next separator on the right - regardless if visible or not
                    sal_uInt16 nActRightCol=GetActRightColumn();
                    if(mpColumnItem->IsTable())
                    {
                        if(nDragType & DRAG_OBJECT_ACTLINE_ONLY)
                        {
                            //the current row/column should be modified only
                            //then the next/previous visible border position
                            //marks the min/max positions
                            nMaxLeft = nLeftCol == USHRT_MAX ?
                                0 :
                                pBorders[nLeftCol].nPos;
                            //rows can always be increased without a limit
                            if(pRuler_Imp->bIsTableRows)
                                nMaxRight = pBorders[nIdx].nMaxPos;
                            else
                                nMaxRight = nRightCol == USHRT_MAX ?
                                    GetMargin2():
                                    pBorders[nRightCol].nPos;
                            nMaxLeft += lNullPix;
                            nMaxRight += lNullPix;
                        }
                        else
                        {
                            if(DRAG_OBJECT_SIZE_PROPORTIONAL & nDragType && !bHorz && pRuler_Imp->bIsTableRows)
                                nMaxLeft = (nIdx + 1) * lMinFrame + lNullPix;
                            else
                                nMaxLeft = pBorders[nIdx].nMinPos + lNullPix;
                            if(DRAG_OBJECT_SIZE_PROPORTIONAL & nDragType||
                            (DRAG_OBJECT_SIZE_LINEAR & nDragType) )
                            {
                                if(pRuler_Imp->bIsTableRows)
                                {
                                    if(bHorz)
                                        nMaxRight = GetRightIndent() + lNullPix -
                                                (mpColumnItem->Count() - nIdx - 1) * lMinFrame;
                                    else
                                        nMaxRight = pBorders[nIdx].nMaxPos + lNullPix;
                                }
                                else
                                    nMaxRight=lNullPix+CalcPropMaxRight(nIdx);
                            }
                            else
                                nMaxRight = pBorders[nIdx].nMaxPos + lNullPix;
                        }
                        nMaxLeft += lMinFrame;
                        nMaxRight -= lMinFrame;

                    }
                    else
                    {
                        if(nLeftCol==USHRT_MAX)
                            nMaxLeft=lNullPix;
                        else
                            nMaxLeft = pBorders[nLeftCol].nPos +
                                pBorders[nLeftCol].nWidth + lNullPix;

                        if(nActRightCol == nIdx)
                        {
                            if(bRTL)
                            {
                                nMaxLeft += pBorders[nIdx].nPos +
                                    GetRightIndent() - std::max(GetFirstLineIndent(),
                                                           GetLeftIndent());
                                if(nActLeftCol!=USHRT_MAX)
                                    nMaxLeft -= pBorders[nActLeftCol].nPos +
                                        pBorders[nActLeftCol].nWidth;
                            }
                            else
                            {
                                nMaxLeft += pBorders[nIdx].nPos -
                                    GetRightIndent() + std::max(GetFirstLineIndent(),
                                                           GetLeftIndent());
                                if(nActLeftCol!=USHRT_MAX)
                                    nMaxLeft -= pBorders[nActLeftCol].nPos +
                                        pBorders[nActLeftCol].nWidth;
                            }
                        }
                        nMaxLeft += lMinFrame;
                        nMaxLeft += nDragOffset;

                        // nMaxRight
                        // linear / proprotional move
                        if(DRAG_OBJECT_SIZE_PROPORTIONAL & nDragType||
                           (DRAG_OBJECT_SIZE_LINEAR & nDragType) )
                        {
                            nMaxRight=lNullPix+CalcPropMaxRight(nIdx);
                        }
                        else if(DRAG_OBJECT_SIZE_LINEAR & nDragType)
                        {
                            nMaxRight=lNullPix+GetMargin2()-GetMargin1()+
                                (nBorderCount-nIdx-1)*lMinFrame;
                        }
                        else
                        {
                            if(nRightCol==USHRT_MAX)
                            { // last column
                                nMaxRight = GetMargin2() + lNullPix;
                                if(IsActLastColumn())
                                {
                                    if(bRTL)
                                    {
                                        nMaxRight -=
                                            GetMargin2() + GetRightIndent() -
                                                std::max(GetFirstLineIndent(),
                                                    GetLeftIndent());
                                    }
                                    else
                                    {
                                        nMaxRight -=
                                            GetMargin2() - GetRightIndent() +
                                                std::max(GetFirstLineIndent(),
                                                    GetLeftIndent());
                                    }
                                    nMaxRight += pBorders[nIdx].nPos +
                                        pBorders[nIdx].nWidth;
                                }
                            }
                            else
                            {
                                nMaxRight = lNullPix + pBorders[nRightCol].nPos;
                                sal_uInt16 nNotHiddenRightCol =
                                    GetActRightColumn(sal_True, nIdx);

                                if( nActLeftCol == nIdx )
                                {
                                    long nBorder = nNotHiddenRightCol ==
                                        USHRT_MAX ?
                                        GetMargin2() :
                                        pBorders[nNotHiddenRightCol].nPos;
                                    if(bRTL)
                                    {
                                        nMaxRight -= nBorder + GetRightIndent() -
                                            std::max(GetFirstLineIndent(),
                                                GetLeftIndent());
                                    }
                                    else
                                    {
                                        nMaxRight -= nBorder - GetRightIndent() +
                                            std::max(GetFirstLineIndent(),
                                                GetLeftIndent());
                                    }
                                    nMaxRight += pBorders[nIdx].nPos +
                                        pBorders[nIdx].nWidth;
                                }
                            }
                            nMaxRight -= lMinFrame;
                            nMaxRight -= pBorders[nIdx].nWidth;
                        }
                    }
                }
                // ObjectItem
                else
                {
                    if(mpObjectItem->HasLimits())
                    {
                        if(CalcLimits(nMaxLeft, nMaxRight, nIdx & 1? sal_False : sal_True))
                        {
                            nMaxLeft = ConvertPosPixel(nMaxLeft);
                            nMaxRight = ConvertPosPixel(nMaxRight);
                        }
                    }
                    else
                    {
                        nMaxLeft = LONG_MIN;
                        nMaxRight = LONG_MAX;
                    }
                }
                break;
            }
          case RULER_DRAGSIZE_2:
            {
                nMaxLeft = lNullPix + pBorders[nIdx].nPos;
                if(nIdx == mpColumnItem->Count()-2) { // last column
                    nMaxRight = GetMargin2() + lNullPix;
                    if(mpColumnItem->IsLastAct()) {
                        nMaxRight -=
                            GetMargin2() - GetRightIndent() +
                                std::max(GetFirstLineIndent(),
                                    GetLeftIndent());
                        nMaxRight += pBorders[nIdx].nPos +
                            pBorders[nIdx].nWidth;
                    }
                }
                else {
                    nMaxRight = lNullPix + pBorders[nIdx+1].nPos;
                    if(mpColumnItem->GetActColumn()-1 == nIdx) {
                        nMaxRight -= pBorders[nIdx+1].nPos  - GetRightIndent() +
                            std::max(GetFirstLineIndent(),
                                GetLeftIndent());
                        nMaxRight += pBorders[nIdx].nPos +
                            pBorders[nIdx].nWidth;
                    }
            }
                nMaxRight -= lMinFrame;
                nMaxRight -= pBorders[nIdx].nWidth;
                break;
            }
        }
        nMaxRight += nDragOffset;
        break;
    }
      case RULER_TYPE_INDENT:
        {
        const sal_uInt16 nIdx = GetDragAryPos();
        switch(nIdx) {
        case INDENT_FIRST_LINE - INDENT_GAP:
        case INDENT_LEFT_MARGIN - INDENT_GAP:
            {
                if(bRTL)
                {
                    nMaxLeft = lNullPix + GetRightIndent();

                    if(mpColumnItem.get() && !mpColumnItem->IsFirstAct())
                        nMaxLeft += pBorders[mpColumnItem->GetActColumn()-1].nPos +
                            pBorders[mpColumnItem->GetActColumn()-1].nWidth;
                    nMaxRight = lNullPix + GetMargin2();

                    // Dragging along
                    if((INDENT_FIRST_LINE - INDENT_GAP) != nIdx &&
                       (nDragType & DRAG_OBJECT_LEFT_INDENT_ONLY) !=
                       DRAG_OBJECT_LEFT_INDENT_ONLY)
                    {
                        if(GetLeftIndent() > GetFirstLineIndent())
                            nMaxLeft += GetLeftIndent() - GetFirstLineIndent();
                        else
                            nMaxRight -= GetFirstLineIndent() - GetLeftIndent();
                    }
                }
                else
                {
                    nMaxLeft = lNullPix;

                    if(mpColumnItem.get() && !mpColumnItem->IsFirstAct())
                        nMaxLeft += pBorders[mpColumnItem->GetActColumn()-1].nPos +
                            pBorders[mpColumnItem->GetActColumn()-1].nWidth;
                    nMaxRight = lNullPix + GetRightIndent() - lMinFrame;

                    // Dragging along
                    if((INDENT_FIRST_LINE - INDENT_GAP) != nIdx &&
                       (nDragType & DRAG_OBJECT_LEFT_INDENT_ONLY) !=
                       DRAG_OBJECT_LEFT_INDENT_ONLY)
                    {
                        if(GetLeftIndent() > GetFirstLineIndent())
                            nMaxLeft += GetLeftIndent() - GetFirstLineIndent();
                        else
                            nMaxRight -= GetFirstLineIndent() - GetLeftIndent();
                    }
                }
            }
          break;
          case INDENT_RIGHT_MARGIN - INDENT_GAP:
            {
                if(bRTL)
                {
                    nMaxLeft = lNullPix;
                    nMaxRight = lNullPix + std::min(GetFirstLineIndent(), GetLeftIndent()) - lMinFrame;
                    if(mpColumnItem.get())
                    {
                        sal_uInt16 nRightCol=GetActRightColumn( sal_True );
                        if(!IsActLastColumn( sal_True ))
                            nMaxRight += pBorders[nRightCol].nPos;
                        else
                            nMaxRight += GetMargin2();
                    }
                    else
                    {
                        nMaxLeft += GetMargin1();
                    }
                    nMaxLeft += lMinFrame;
                }
                else
                {
                    nMaxLeft = lNullPix +
                        std::max(GetFirstLineIndent(), GetLeftIndent());
                    nMaxRight = lNullPix;
                    if(mpColumnItem.get())
                    {
                        sal_uInt16 nRightCol=GetActRightColumn( sal_True );
                        if(!IsActLastColumn( sal_True ))
                            nMaxRight += pBorders[nRightCol].nPos;
                        else
                            nMaxRight += GetMargin2();
                    }
                    else
                        nMaxRight += GetMargin2();
                    nMaxLeft += lMinFrame;
                }
            }
            break;
        }
        break;
    }
    case RULER_TYPE_TAB:                // Tabs (Modifier)
        /* left = NOf + Max(LAR, EZ)
           right = NOf + RAR */

        if (bRTL)
            nMaxLeft = lNullPix + GetRightIndent();
        else
            nMaxLeft = lNullPix + std::min(GetFirstLineIndent(), GetLeftIndent());

        pRuler_Imp->lMaxRightLogic = GetLogicRightIndent() + lLogicNullOffset;
        nMaxRight = ConvertSizePixel(pRuler_Imp->lMaxRightLogic);
        break;
    default: ; //prevent warning
    }
}

long SvxRuler::StartDrag()
{
    /*
       Beginning of a drag operation (SV-handler) evaluates modifier and
       calculated values

       [Cross-reference]

       <SvxRuler::EvalModifier()>
       <SvxRuler::CalcMinMax()>
       <SvxRuler::EndDrag()>
    */
    sal_Bool bContentProtected = pRuler_Imp->aProtectItem.IsCntntProtected();
    if(!bValid)
        return sal_False;

    pRuler_Imp->lLastLMargin = GetMargin1();
    pRuler_Imp->lLastRMargin = GetMargin2();
    long bOk = 1;
    if(GetStartDragHdl().IsSet())
        bOk = Ruler::StartDrag();
    if(bOk)
    {
        lInitialDragPos = GetDragPos();
        switch(GetDragType())
        {
            case RULER_TYPE_MARGIN1:        // left edge of the surrounding Frame
            case RULER_TYPE_MARGIN2:        // right edge of the surrounding Frame
                if((bHorz && mpLRSpaceItem.get()) || (!bHorz && mpULSpaceItem.get()))
                {
                    if(mpColumnItem.get())
                        EvalModifier();
                    else
                        nDragType = DRAG_OBJECT;
                }
                else
                {
                    bOk = sal_False;
                }
                break;
            case RULER_TYPE_BORDER: // Table, column (Modifier)
                if(mpColumnItem.get())
                {
                    nDragOffset = 0;
                    if (!mpColumnItem->IsTable())
                        nDragOffset = GetDragPos() - pBorders[GetDragAryPos()].nPos;
                    EvalModifier();
                }
                else
                    nDragOffset = 0;
                break;
            case RULER_TYPE_INDENT: // Paragraph indents (Modifier)
            {
                if( bContentProtected )
                    return sal_False;
                sal_uInt16 nIndent = INDENT_LEFT_MARGIN;
                if((nIndent) == GetDragAryPos() + INDENT_GAP) {  // Left paragraph indent
                    pIndents[0] = pIndents[INDENT_FIRST_LINE];
                    pIndents[0].nStyle |= RULER_STYLE_DONTKNOW;
                    EvalModifier();
                }
                else
                {
                    nDragType = DRAG_OBJECT;
                }
                pIndents[1] = pIndents[GetDragAryPos() + INDENT_GAP];
                pIndents[1].nStyle |= RULER_STYLE_DONTKNOW;
                break;
            }
            case RULER_TYPE_TAB: // Tabs (Modifier)
                if( bContentProtected )
                    return sal_False;
                EvalModifier();
                pTabs[0] = pTabs[GetDragAryPos() + 1];
                pTabs[0].nStyle |= RULER_STYLE_DONTKNOW;
                break;
            default:
                nDragType = NONE;
        }
    }
    else
    {
        nDragType = NONE;
    }

    if(bOk)
        CalcMinMax();
    return bOk;
}

void  SvxRuler::Drag()
{
    /* SV-Draghandler */
    if(IsDragCanceled())
    {
        Ruler::Drag();
        return;
    }
    switch(GetDragType()) {
        case RULER_TYPE_MARGIN1: // left edge of the surrounding Frame
            DragMargin1();
            pRuler_Imp->lLastLMargin = GetMargin1();
            break;
        case RULER_TYPE_MARGIN2: // right edge of the surrounding Frame
            DragMargin2();
            pRuler_Imp->lLastRMargin = GetMargin2();
            break;
        case RULER_TYPE_INDENT: // Paragraph indents
            DragIndents();
            break;
        case RULER_TYPE_BORDER: // Table, columns
            if(mpColumnItem.get())
                DragBorders();
            else if(mpObjectItem.get())
                DragObjectBorder();
            break;
        case RULER_TYPE_TAB: // Tabs
            DragTabs();
            break;
        default:
            break; //prevent warning
    }
    Ruler::Drag();
}

void SvxRuler::EndDrag()
{
    /*
       SV-handler; is called when ending the dragging. Triggers the updating of data
       on the application, by calling the respective Apply...() methods to send the
       data to the application.
    */
    const sal_Bool bUndo = IsDragCanceled();
    const long lPos = GetDragPos();
    DrawLine_Impl(lTabPos, 6, bHorz);
    lTabPos = -1;

    if(!bUndo)
    {
        switch(GetDragType())
        {
            case RULER_TYPE_MARGIN1: // upper left edge of the surrounding Frame
            case RULER_TYPE_MARGIN2: // lower right edge of the surrounding Frame
                {
                    if(!mpColumnItem.get() || !mpColumnItem->IsTable())
                        ApplyMargins();

                    if(mpColumnItem.get() &&
                       (mpColumnItem->IsTable() ||
                        (nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)))
                        ApplyBorders();

                }
                break;
            case RULER_TYPE_BORDER: // Table, columns
                if(lInitialDragPos != lPos ||
                    (pRuler_Imp->bIsTableRows && bHorz)) //special case - the null offset is changed here
                {
                    if(mpColumnItem.get())
                    {
                        ApplyBorders();
                        if(bHorz)
                            UpdateTabs();
                    }
                    else if(mpObjectItem.get())
                        ApplyObject();
                }
                break;
            case RULER_TYPE_INDENT: // Paragraph indents
                if(lInitialDragPos != lPos)
                    ApplyIndents();
                SetIndents(INDENT_COUNT, pIndents + INDENT_GAP);
                break;
            case RULER_TYPE_TAB: // Tabs
                {
                    ApplyTabs();
                    pTabs[GetDragAryPos()].nStyle &= ~RULER_STYLE_INVISIBLE;
                    SetTabs(nTabCount, pTabs+TAB_GAP);
                }
                break;
            default:
                break; //prevent warning
        }
    }
    nDragType = NONE;
    Ruler::EndDrag();
    if(bUndo)
    {
        for(sal_uInt16 i = 0; i < pRuler_Imp->nControlerItems; i++)
        {
            pCtrlItem[i]->ClearCache();
            pCtrlItem[i]->GetBindings().Invalidate(pCtrlItem[i]->GetId());
        }
    }
}

void SvxRuler::ExtraDown()
{
    /* Overloaded SV method, sets the new type for the Default tab. */

    // Switch Tab Type
    if(mpTabStopItem.get() &&
        (nFlags & SVXRULER_SUPPORT_TABS) == SVXRULER_SUPPORT_TABS)
    {
        ++nDefTabType;
        if(RULER_TAB_DEFAULT == nDefTabType)
            nDefTabType = RULER_TAB_LEFT;
        SetExtraType(RULER_EXTRA_TAB, nDefTabType);
    }
    Ruler::ExtraDown();
}

void SvxRuler::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    /*
       Report through the bindings that the status update is completed. The ruler
       updates its appearance and gets registered again in the bindings.
    */

    // start update
    if(bActive &&
       rHint.Type() == TYPE(SfxSimpleHint) &&
       ((SfxSimpleHint&) rHint ).GetId() == SFX_HINT_UPDATEDONE )
     {
        Update();
        EndListening(*pBindings);
        bValid = sal_True;
        bListening = sal_False;
    }
}


IMPL_LINK_INLINE_START( SvxRuler, MenuSelect, Menu *, pMenu )
{
    /* Handler of the context menus for switching the unit of measurement */
    SetUnit(FieldUnit(pMenu->GetCurItemId()));
    return 0;
}
IMPL_LINK_INLINE_END( SvxRuler, MenuSelect, Menu *, pMenu )

IMPL_LINK( SvxRuler, TabMenuSelect, Menu *, pMenu )
{
    /* Handler of the tab menu for setting the type */
    if(mpTabStopItem.get() && mpTabStopItem->Count() > pRuler_Imp->nIdx)
    {
        SvxTabStop aTabStop = (*mpTabStopItem.get())[pRuler_Imp->nIdx];
        aTabStop.GetAdjustment() = ToAttrTab_Impl(pMenu->GetCurItemId()-1);
        mpTabStopItem->Remove(pRuler_Imp->nIdx);
        mpTabStopItem->Insert(aTabStop);
        sal_uInt16 nTabStopId = bHorz ? SID_ATTR_TABSTOP : SID_ATTR_TABSTOP_VERTICAL;
        pBindings->GetDispatcher()->Execute( nTabStopId, SFX_CALLMODE_RECORD, mpTabStopItem.get(), 0L );
        UpdateTabs();
        pRuler_Imp->nIdx = 0;
    }
    return 0;
}

void SvxRuler::Command( const CommandEvent& rCommandEvent )
{
    /* Mouse context menu for switching the unit of measurement */
    if ( COMMAND_CONTEXTMENU == rCommandEvent.GetCommand() )
    {
        CancelDrag();
        sal_Bool bRTL = pRuler_Imp->pTextRTLItem && pRuler_Imp->pTextRTLItem->GetValue();
        if ( pTabs &&
             RULER_TYPE_TAB ==
             GetType( rCommandEvent.GetMousePosPixel(), &pRuler_Imp->nIdx ) &&
             pTabs[pRuler_Imp->nIdx + TAB_GAP].nStyle < RULER_TAB_DEFAULT )
        {
            PopupMenu aMenu;
            aMenu.SetSelectHdl(LINK(this, SvxRuler, TabMenuSelect));
            VirtualDevice aDev;
            const Size aSz(RULER_TAB_WIDTH+2, RULER_TAB_HEIGHT+2);
            aDev.SetOutputSize(aSz);
            aDev.SetBackground(Wallpaper(Color(COL_WHITE)));
            Color aFillColor(aDev.GetSettings().GetStyleSettings().GetShadowColor());
            const Point aPt(aSz.Width() / 2, aSz.Height() / 2);

            for ( sal_uInt16 i = RULER_TAB_LEFT; i < RULER_TAB_DEFAULT; ++i )
            {
                sal_uInt16 nStyle = bRTL ? i|RULER_TAB_RTL : i;
                nStyle |= (sal_uInt16)(bHorz ? WB_HORZ : WB_VERT);
                DrawTab(&aDev, aFillColor, aPt, nStyle);
                aMenu.InsertItem(i + 1,
                                 ResId(RID_SVXSTR_RULER_START + i, DIALOG_MGR()).toString(),
                                 Image(aDev.GetBitmap(Point(), aSz), Color(COL_WHITE)));
                aMenu.CheckItem(i+1, i == pTabs[pRuler_Imp->nIdx+TAB_GAP].nStyle);
                aDev.SetOutputSize(aSz); // delete device
            }
            aMenu.Execute( this, rCommandEvent.GetMousePosPixel() );
        }
        else
        {
            PopupMenu aMenu(ResId(RID_SVXMN_RULER, DIALOG_MGR()));
            aMenu.SetSelectHdl(LINK(this, SvxRuler, MenuSelect));
            FieldUnit eUnit = GetUnit();
            const sal_uInt16 nCount = aMenu.GetItemCount();

            sal_Bool bReduceMetric = 0 != (nFlags & SVXRULER_SUPPORT_REDUCED_METRIC);
            for ( sal_uInt16 i = nCount; i; --i )
            {
                const sal_uInt16 nId = aMenu.GetItemId(i - 1);
                aMenu.CheckItem(nId, nId == (sal_uInt16)eUnit);
                if(bReduceMetric &&
                        (nId == FUNIT_M ||
                         nId == FUNIT_KM ||
                         nId == FUNIT_FOOT ||
                         nId == FUNIT_MILE ||
                         nId == FUNIT_CHAR ||
                         nId == FUNIT_LINE ))
                {
                    if (( nId == FUNIT_CHAR ) && bHorz )
                       ;
                    else if (( nId == FUNIT_LINE ) && !bHorz )
                       ;
                    else
                       aMenu.RemoveItem(i - 1);
                }
            }
            aMenu.Execute( this, rCommandEvent.GetMousePosPixel() );
        }
    }
    else
    {
        Ruler::Command( rCommandEvent );
    }
}

sal_uInt16 SvxRuler::GetActRightColumn(
                        sal_Bool bForceDontConsiderHidden,
                        sal_uInt16 nAct ) const
{
    if( nAct == USHRT_MAX )
        nAct = mpColumnItem->GetActColumn();
    else nAct++; //To be able to pass on the ActDrag

    sal_Bool bConsiderHidden = !bForceDontConsiderHidden &&
        !( nDragType & DRAG_OBJECT_ACTLINE_ONLY );

    while( nAct < mpColumnItem->Count() - 1 )
    {
        if( (*mpColumnItem.get())[nAct].bVisible || bConsiderHidden )
            return nAct;
        else
            nAct++;
    }
    return USHRT_MAX;
}

sal_uInt16 SvxRuler::GetActLeftColumn(
                        sal_Bool bForceDontConsiderHidden,
                        sal_uInt16 nAct ) const
{
    if(nAct == USHRT_MAX)
        nAct = mpColumnItem->GetActColumn();

    sal_uInt16 nLOffs = 1;

    sal_Bool bConsiderHidden = !bForceDontConsiderHidden &&
        !( nDragType & DRAG_OBJECT_ACTLINE_ONLY );

    while(nAct >= nLOffs)
    {
        if( (*mpColumnItem.get())[nAct - nLOffs].bVisible || bConsiderHidden )
            return nAct-nLOffs;
        else
            nLOffs++;
    }
    return USHRT_MAX;
}

sal_Bool SvxRuler::IsActLastColumn(
                        sal_Bool bForceDontConsiderHidden,
                        sal_uInt16 nAct) const
{
    return GetActRightColumn(bForceDontConsiderHidden, nAct) == USHRT_MAX;
}

sal_Bool SvxRuler::IsActFirstColumn(
                        sal_Bool bForceDontConsiderHidden,
                        sal_uInt16 nAct) const
{
    return GetActLeftColumn(bForceDontConsiderHidden, nAct) == USHRT_MAX;
}

long SvxRuler::CalcPropMaxRight(sal_uInt16 nCol) const
{

    if(!(nDragType & DRAG_OBJECT_SIZE_LINEAR))
    {
        // Remove the minimum width for all affected columns
        // starting from the right edge
        long _nMaxRight = GetMargin2() - GetMargin1();

        long lFences = 0;
        long lMinSpace = USHRT_MAX;
        long lOldPos;
        long lColumns = 0;

        sal_uInt16 nStart;
        if(!mpColumnItem->IsTable())
        {
            if(nCol == USHRT_MAX)
            {
                lOldPos = GetMargin1();
                nStart = 0;
            }
            else
            {
                lOldPos = pBorders[nCol].nPos + pBorders[nCol].nWidth;
                nStart = nCol + 1;
                lFences = pBorders[nCol].nWidth;
            }

            for(sal_uInt16 i = nStart; i < nBorderCount - 1; ++i)
            {
                long lWidth = pBorders[i].nPos - lOldPos;
                lColumns += lWidth;
                if(lWidth < lMinSpace)
                    lMinSpace = lWidth;
                lOldPos = pBorders[i].nPos+pBorders[i].nWidth;
                lFences += pBorders[i].nWidth;
            }
            long lWidth = GetMargin2() - lOldPos;
            lColumns += lWidth;
            if(lWidth < lMinSpace)
                lMinSpace = lWidth;
        }
        else
        {
            sal_uInt16 nActCol;
            if(nCol == USHRT_MAX) //CalcMinMax for LeftMargin
            {
                lOldPos = GetMargin1();
            }
            else
            {
                lOldPos = pBorders[nCol].nPos;
            }
            lColumns = GetMargin2()-lOldPos;
            nActCol = nCol;
            lFences = 0;
            while(nActCol < nBorderCount || nActCol == USHRT_MAX)
            {
                sal_uInt16 nRight;
                if(nActCol == USHRT_MAX)
                {
                    nRight = 0;
                    while(!(*mpColumnItem.get())[nRight].bVisible)
                    {
                        nRight++;
                    }
                }
                else
                {
                    nRight = GetActRightColumn(sal_False, nActCol);
                }

                long lWidth;
                if(nRight != USHRT_MAX)
                {
                    lWidth = pBorders[nRight].nPos - lOldPos;
                    lOldPos = pBorders[nRight].nPos;
                }
                else
                {
                    lWidth=GetMargin2() - lOldPos;
                }
                nActCol = nRight;
                if(lWidth < lMinSpace)
                    lMinSpace = lWidth;
                if(nActCol == USHRT_MAX)
                    break;
            }
        }

        _nMaxRight -= (long)(lFences + lMinFrame / (float) lMinSpace * lColumns);
        return _nMaxRight;
    }
    else
    {
        if(mpColumnItem->IsTable())
        {
            sal_uInt16 nVisCols = 0;
            for(sal_uInt16 i = GetActRightColumn(sal_False, nCol); i<nBorderCount;)
            {
                if((*mpColumnItem.get())[i].bVisible)
                    nVisCols++;
                i = GetActRightColumn(sal_False, i);
            }
            return GetMargin2() - GetMargin1() - (nVisCols + 1) * lMinFrame;
        }
        else
        {
            long lWidth = 0;
            for(sal_uInt16 i = nCol; i < nBorderCount - 1; i++)
            {
                lWidth += lMinFrame + pBorders[i].nWidth;
            }
            return GetMargin2() - GetMargin1() - lWidth;
        }
    }
}

// Tab stops relative to indent (#i24363#)
void SvxRuler::SetTabsRelativeToIndent( sal_Bool bRel )
{
    pRuler_Imp->bIsTabsRelativeToIndent = bRel;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
