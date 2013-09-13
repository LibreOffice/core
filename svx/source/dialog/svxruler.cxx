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

#include <svx/svdtrans.hxx>

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
        static void     AddDebugText(const sal_Char* pDescription, const OUString& rText );
};
static RulerDebugWindow* pDebugWindow = 0;

RulerDebugWindow::~RulerDebugWindow()
{
    pDebugWindow = 0;
}
void     RulerDebugWindow::AddDebugText(const sal_Char* pDescription, const OUString& rText )
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
    pCtrlItem(new SvxRulerItem* [CTRL_ITEM_COUNT]),
    pEditWin(pWin),
    mpRulerImpl(new SvxRuler_Impl),
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
    mpBorders(1), // due to one column tables
    pBindings(&rBindings),
    nDragOffset(0),
    nMaxLeft(0),
    nMaxRight(0),
    bValid(sal_False),
    bListening(sal_False),
    bActive(sal_True),
    mbCoarseSnapping(false),
    mbSnapping(true)

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

        mpIndents.resize(5 + INDENT_GAP);

        for(sal_uInt32 nIn = 0; nIn < mpIndents.size(); nIn++)
        {
            mpIndents[nIn].nPos = 0;
            mpIndents[nIn].nStyle = RULER_STYLE_DONTKNOW;
        }

        mpIndents[0].nStyle = RULER_STYLE_DONTKNOW;
        mpIndents[1].nStyle = RULER_STYLE_DONTKNOW;
        mpIndents[INDENT_FIRST_LINE].nStyle = RULER_INDENT_TOP;
        mpIndents[INDENT_LEFT_MARGIN].nStyle = RULER_INDENT_BOTTOM;
        mpIndents[INDENT_RIGHT_MARGIN].nStyle = RULER_INDENT_BOTTOM;
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
        mpObjectBorders.resize(OBJECT_BORDER_COUNT);
        for(sal_uInt16 nBorder = 0; nBorder < OBJECT_BORDER_COUNT; ++nBorder)
        {
            mpObjectBorders[nBorder].nPos   = 0;
            mpObjectBorders[nBorder].nWidth = 0;
            mpObjectBorders[nBorder].nStyle = RULER_BORDER_MOVEABLE;
        }
    }

    pCtrlItem[i++] = new SvxRulerItem(SID_RULER_PROTECT, *this, rBindings );
    pCtrlItem[i++] = new SvxRulerItem(SID_RULER_BORDER_DISTANCE, *this, rBindings);
    mpRulerImpl->nControlerItems=i;

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

    pBindings->LeaveRegistrations();
}

long SvxRuler::MakePositionSticky(long aPosition, long aPointOfReference, bool aSnapToFrameMargin) const
{
    long aPointOfReferencePixel = ConvertHPosPixel(aPointOfReference);
    long aLeftFramePosition     = ConvertHPosPixel(GetLeftFrameMargin());
    long aRightFramePosition    = ConvertHPosPixel(GetRightFrameMargin());

    double aTick = GetCurrentRulerUnit().nTick1;

    if (mbCoarseSnapping)
        aTick = GetCurrentRulerUnit().nTick2;

    long aTickPixel = pEditWin->LogicToPixel(Size(aTick, 0), GetCurrentMapMode()).Width();

    double aHalfTick = aTick / 2.0;
    double aHalfTickPixel = aTickPixel / 2.0;

    if (aSnapToFrameMargin)
    {
        if (aPosition > aLeftFramePosition - aHalfTickPixel && aPosition < aLeftFramePosition + aHalfTickPixel)
            return aLeftFramePosition;

        if (aPosition > aRightFramePosition - aHalfTickPixel && aPosition < aRightFramePosition + aHalfTickPixel)
            return aRightFramePosition;
    }

    if (!mbSnapping)
        return aPosition;

    // Move "coordinate system" to frame position so ticks are calculated correctly
    long aTranslatedPosition = aPosition - aPointOfReferencePixel;
    // Convert position to current selected map mode
    long aPositionLogic = pEditWin->PixelToLogic(Size(aTranslatedPosition, 0), GetCurrentMapMode()).Width();
    // Normalize -- snap to nearest tick
    aPositionLogic = std::round((aPositionLogic + aHalfTick) / aTick) * aTick;
    // Convert back to pixels
    aPosition = pEditWin->LogicToPixel(Size(aPositionLogic, 0), GetCurrentMapMode()).Width();
    // Move "coordinate system" back to original position
    return aPosition + aPointOfReferencePixel;
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
        ( mpRulerImpl->aProtectItem.IsSizeProtected() ||
          mpRulerImpl->aProtectItem.IsPosProtected() ) ?
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
            SetMargin1(0, nMarginStyle);
            lAppNullOffset = 0;
        }
        else
        {
            SetMargin1(ConvertHPosPixel(lAppNullOffset), nMarginStyle);
        }

        long lRight = 0;

        // evaluate the table right edge of the table
        if(mpColumnItem.get() && mpColumnItem->IsTable())
            lRight = mpColumnItem->GetRight();
        else
            lRight = mpLRSpaceItem->GetRight();

        long aWidth = mpPagePosItem->GetWidth() - lRight - lLogicNullOffset + lAppNullOffset;
        long aWidthPixel = ConvertHPosPixel(aWidth);

        SetMargin2(aWidthPixel, nMarginStyle);
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
            SetMargin1(0, nMarginStyle);
        }
        else
        {
            SetMargin1(ConvertVPosPixel(lAppNullOffset), nMarginStyle);
        }

        long lLower = mpColumnItem.get() ? mpColumnItem->GetRight() : mpULSpaceItem->GetLower();
        long nMargin2 = mpPagePosItem->GetHeight() - lLower - lLogicNullOffset + lAppNullOffset;
        long nMargin2Pixel = ConvertVPosPixel(nMargin2);

        SetMargin2(nMargin2Pixel, nMarginStyle);
    }
    else
    {
        // turns off the view
        SetMargin1();
        SetMargin2();
    }

    if(mpColumnItem.get())
    {
        mpRulerImpl->nColLeftPix = (sal_uInt16) ConvertSizePixel(mpColumnItem->GetLeft());
        mpRulerImpl->nColRightPix = (sal_uInt16) ConvertSizePixel(mpColumnItem->GetRight());
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

    RulerSelection aSelection = GetHoverSelection();

    if (aSelection.eType == RULER_TYPE_DONTKNOW)
    {
        SetQuickHelpText("");
        return;
    }

    RulerUnitData aUnitData = GetCurrentRulerUnit();
    double aRoundingFactor = aUnitData.nTickUnit / aUnitData.nTick1;
    sal_Int32 aNoDecimalPlaces = 1 + std::ceil(std::log10(aRoundingFactor));
    OUString sUnit = OUString::createFromAscii(aUnitData.aUnitStr);

    switch (aSelection.eType)
    {
        case RULER_TYPE_INDENT:
        {
            long nIndex = aSelection.nAryPos + INDENT_GAP;

            long nIndentValue = 0.0;
            if (nIndex == INDENT_LEFT_MARGIN)
                nIndentValue = mpParaItem->GetTxtLeft();
            else if (nIndex == INDENT_FIRST_LINE)
                nIndentValue = mpParaItem->GetTxtFirstLineOfst();
            else if (nIndex == INDENT_RIGHT_MARGIN)
                nIndentValue = mpParaItem->GetRight();

            double fValue = pEditWin->LogicToLogic(Size(nIndentValue, 0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
            fValue = rtl::math::round(fValue / aUnitData.nTickUnit, aNoDecimalPlaces);

            SetQuickHelpText(OUString::number(fValue) + " " + sUnit);
            break;
        }
        case RULER_TYPE_BORDER:
        {
            if (mpColumnItem.get() == NULL)
                break;

            SvxColumnItem& aColumnItem = *mpColumnItem.get();

            if (aSelection.nAryPos + 1 >= aColumnItem.Count())
                break;

            double fStart = pEditWin->LogicToLogic(Size(aColumnItem[aSelection.nAryPos].nEnd,       0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
            fStart = rtl::math::round(fStart / aUnitData.nTickUnit, aNoDecimalPlaces);
            double fEnd   = pEditWin->LogicToLogic(Size(aColumnItem[aSelection.nAryPos + 1].nStart, 0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
            fEnd = rtl::math::round(fEnd / aUnitData.nTickUnit, aNoDecimalPlaces);

            SetQuickHelpText(
                OUString::number(fStart) + " " + sUnit + " - " +
                OUString::number(fEnd)   + " " + sUnit );
            break;
        }
        case RULER_TYPE_MARGIN1:
        {
            long nLeft = 0.0;
            if (mpLRSpaceItem.get())
                nLeft = mpLRSpaceItem->GetLeft();
            else if (mpULSpaceItem.get())
                nLeft = mpULSpaceItem->GetUpper();
            else
                break;

            double fValue = pEditWin->LogicToLogic(Size(nLeft, 0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
            fValue = rtl::math::round(fValue / aUnitData.nTickUnit, aNoDecimalPlaces);
            SetQuickHelpText(OUString::number(fValue) + " " + sUnit);

            break;
        }
        case RULER_TYPE_MARGIN2:
        {
            long nRight = 0.0;
            if (mpLRSpaceItem.get())
                nRight = mpLRSpaceItem->GetRight();
            else if (mpULSpaceItem.get())
                nRight = mpULSpaceItem->GetLower();
            else
                break;

            double fValue = pEditWin->LogicToLogic(Size(nRight, 0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
            fValue = rtl::math::round(fValue / aUnitData.nTickUnit, aNoDecimalPlaces);
            SetQuickHelpText(OUString::number(fValue) + " " + sUnit);

            break;
        }
        default:
        {
            SetQuickHelpText("");
            break;
        }
    }
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
        mpRulerImpl->aProtectItem = *pItem;
}

void SvxRuler::UpdateTextRTL(const SfxBoolItem* pItem)
{
    if(bActive && bHorz)
    {
        delete mpRulerImpl->pTextRTLItem;
        mpRulerImpl->pTextRTLItem = 0;
        if(pItem)
            mpRulerImpl->pTextRTLItem = new SfxBoolItem(*pItem);
        SetTextRTL(mpRulerImpl->pTextRTLItem && mpRulerImpl->pTextRTLItem->GetValue());
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
            mpRulerImpl->bIsTableRows = (pItem->Which() == SID_RULER_ROWS || pItem->Which() == SID_RULER_ROWS_VERTICAL);
            if(!bHorz && !mpRulerImpl->bIsTableRows)
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
            mpRulerImpl->bIsTableRows = sal_False;
        }
        StartListening_Impl();
    }
}


void SvxRuler::UpdateColumns()
{
    /* Update column view */
    if(mpColumnItem.get() && mpColumnItem->Count() > 1)
    {
        mpBorders.resize(mpColumnItem->Count());

        sal_uInt16 nStyleFlags = RULER_BORDER_VARIABLE;

        sal_Bool bProtectColumns =
                    mpRulerImpl->aProtectItem.IsSizeProtected() ||
                    mpRulerImpl->aProtectItem.IsPosProtected();

        if( !bProtectColumns )
            nStyleFlags |= RULER_BORDER_MOVEABLE;

        if( mpColumnItem->IsTable() )
            nStyleFlags |= RULER_BORDER_TABLE;
        else if ( !bProtectColumns )
            nStyleFlags |= RULER_BORDER_SIZEABLE;

        sal_uInt16 nBorders = mpColumnItem->Count();

        if(!mpRulerImpl->bIsTableRows)
            --nBorders;

        for(sal_uInt16 i = 0; i < nBorders; ++i)
        {
            mpBorders[i].nStyle = nStyleFlags;
            if(!mpColumnItem->At(i).bVisible)
                mpBorders[i].nStyle |= RULER_STYLE_INVISIBLE;

            mpBorders[i].nPos = ConvertPosPixel(mpColumnItem->At(i).nEnd + lAppNullOffset);

            if(mpColumnItem->Count() == i + 1)
            {
                //with table rows the end of the table is contained in the
                //column item but it has no width!
                mpBorders[i].nWidth = 0;
            }
            else
            {
                mpBorders[i].nWidth = ConvertSizePixel(mpColumnItem->At(i + 1).nStart - mpColumnItem->At(i).nEnd);
            }
            mpBorders[i].nMinPos = ConvertPosPixel(mpColumnItem->At(i).nEndMin + lAppNullOffset);
            mpBorders[i].nMaxPos = ConvertPosPixel(mpColumnItem->At(i).nEndMax + lAppNullOffset);
        }
        SetBorders(mpColumnItem->Count() - 1, &mpBorders[0]);
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
        DBG_ASSERT(!mpObjectBorders.empty(), "no Buffer");
        // !! to the page margin
        long nMargin = mpLRSpaceItem.get() ? mpLRSpaceItem->GetLeft() : 0;
        mpObjectBorders[0].nPos =
            ConvertPosPixel(mpObjectItem->GetStartX() -
                            nMargin + lAppNullOffset);
        mpObjectBorders[1].nPos =
            ConvertPosPixel(mpObjectItem->GetEndX() - nMargin + lAppNullOffset);
        nMargin = mpULSpaceItem.get() ? mpULSpaceItem->GetUpper() : 0;
        mpObjectBorders[2].nPos =
            ConvertPosPixel(mpObjectItem->GetStartY() -
                            nMargin + lAppNullOffset);
        mpObjectBorders[3].nPos =
            ConvertPosPixel(mpObjectItem->GetEndY() - nMargin + lAppNullOffset);

        const sal_uInt16 nOffset = GetObjectBordersOff(0);
        SetBorders(2, &mpObjectBorders[0] + nOffset);
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
        mpIndents[0] = Buffer for old intent
        mpIndents[1] = Buffer for old intent
        mpIndents[INDENT_FIRST_LINE]   = first line indent
        mpIndents[INDENT_LEFT_MARGIN]  = left margin
        mpIndents[INDENT_RIGHT_MARGIN] = right margin
    */

    // Dependence on PagePosItem
    if(mpParaItem.get() && mpPagePosItem.get() && !mpObjectItem.get())
    {
        sal_Bool bRTLText = mpRulerImpl->pTextRTLItem && mpRulerImpl->pTextRTLItem->GetValue();
        // First-line indent is negative to the left paragraph margin
        long nLeftFrameMargin = GetLeftFrameMargin();
        long nRightFrameMargin = GetRightFrameMargin();
        SetLeftFrameMargin(ConvertHPosPixel(nLeftFrameMargin));
        SetRightFrameMargin(ConvertHPosPixel(nRightFrameMargin));

        long leftMargin;
        long leftFirstLine;
        long rightMargin;

        if(bRTLText)
        {
            leftMargin    = nRightFrameMargin - mpParaItem->GetTxtLeft() + lAppNullOffset;
            leftFirstLine = leftMargin - mpParaItem->GetTxtFirstLineOfst();
            rightMargin   = nLeftFrameMargin + mpParaItem->GetRight() + lAppNullOffset;
        }
        else
        {
            leftMargin    = nLeftFrameMargin + mpParaItem->GetTxtLeft() + lAppNullOffset;
            leftFirstLine = leftMargin + mpParaItem->GetTxtFirstLineOfst();
            rightMargin   = nRightFrameMargin - mpParaItem->GetRight() + lAppNullOffset;
        }

        mpIndents[INDENT_LEFT_MARGIN].nPos  = ConvertHPosPixel(leftMargin);
        mpIndents[INDENT_FIRST_LINE].nPos   = ConvertHPosPixel(leftFirstLine);
        mpIndents[INDENT_RIGHT_MARGIN].nPos = ConvertHPosPixel(rightMargin);

        if( mpParaItem->IsAutoFirst() )
            mpIndents[INDENT_FIRST_LINE].nStyle |= RULER_STYLE_INVISIBLE;
        else
            mpIndents[INDENT_FIRST_LINE].nStyle &= ~RULER_STYLE_INVISIBLE;

        SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
    }
    else
    {
        if(!mpIndents.empty())
        {
            mpIndents[INDENT_FIRST_LINE].nPos = 0;
            mpIndents[INDENT_LEFT_MARGIN].nPos = 0;
            mpIndents[INDENT_RIGHT_MARGIN].nPos = 0;
        }
        SetIndents(); // turn off
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
    if(lPos != mpRulerImpl->lOldWinPos)
    {
        mpRulerImpl->lOldWinPos=lPos;
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

    if( mpPagePosItem.get() &&
        mpParaItem.get()    &&
        mpTabStopItem.get() &&
        !mpObjectItem.get() )
    {
        // buffer for DefaultTabStop
        // Distance last Tab <-> Right paragraph margin / DefaultTabDist
        sal_Bool bRTL = mpRulerImpl->pTextRTLItem && mpRulerImpl->pTextRTLItem->GetValue();

        long nLeftFrameMargin = GetLeftFrameMargin();
        long nRightFrameMargin = GetRightFrameMargin();

        //#i24363# tab stops relative to indent
        const long nParaItemTxtLeft = mpParaItem->GetTxtLeft();

        const long lParaIndent = nLeftFrameMargin + nParaItemTxtLeft;

        const long lLastTab = mpTabStopItem->Count()
                                ? ConvertHPosPixel(mpTabStopItem->At(mpTabStopItem->Count() - 1).GetTabPos())
                                : 0;
        const long lPosPixel = ConvertHPosPixel(lParaIndent) + lLastTab;
        const long lRightIndent = ConvertHPosPixel(nRightFrameMargin - mpParaItem->GetRight());

        long nDefTabDist = ConvertHPosPixel(lDefTabDist);

        if( !nDefTabDist )
            nDefTabDist = 1;

        const sal_uInt16 nDefTabBuf = lPosPixel > lRightIndent || lLastTab > lRightIndent
                    ? 0
                    : (sal_uInt16)( (lRightIndent - lPosPixel) / nDefTabDist );

        if(mpTabStopItem->Count() + TAB_GAP + nDefTabBuf > nTabBufSize)
        {
            // 10 (GAP) in stock
            nTabBufSize = mpTabStopItem->Count() + TAB_GAP + nDefTabBuf + GAP;
            mpTabs.resize(nTabBufSize);
        }

        nTabCount = 0;
        sal_uInt16 j;

        //#i24363# tab stops relative to indent
        const long lRightPixMargin = ConvertSizePixel(nRightFrameMargin - nParaItemTxtLeft );
        const long lParaIndentPix = ConvertSizePixel(lParaIndent);

        for(j = 0; j < mpTabStopItem->Count(); ++j)
        {
            const SvxTabStop* pTab = &mpTabStopItem->At(j);
            if (mpRulerImpl->bIsTabsRelativeToIndent)
            {
                long nTabPosition = ConvertHPosPixel(lParaIndent + pTab->GetTabPos() + lAppNullOffset);
                mpTabs[nTabCount + TAB_GAP].nPos = nTabPosition;
            }
            else
            {
                long nTabPosition = ConvertHPosPixel(0 + pTab->GetTabPos() + lAppNullOffset);
                mpTabs[nTabCount + TAB_GAP].nPos = nTabPosition;
            }

            if(bRTL)
            {
                mpTabs[nTabCount + TAB_GAP].nPos = lParaIndentPix + lRightPixMargin - mpTabs[nTabCount + TAB_GAP].nPos;
            }
            mpTabs[nTabCount + TAB_GAP].nStyle = ToSvTab_Impl(pTab->GetAdjustment());
            ++nTabCount;
        }

        if(!mpTabStopItem->Count())
            mpTabs[0].nPos = bRTL ? lRightPixMargin : lParaIndentPix;

        // fill the rest with default Tabs
        if(bRTL)
        {
            sal_Int32 aFirst = mpTabs[nTabCount].nPos;
            for(j = 0; j < nDefTabBuf; ++j)
            {
                mpTabs[nTabCount + TAB_GAP].nPos =
                    aFirst - ConvertHPosPixel(j * lDefTabDist);

                if(j == 0 )
                {
                    mpTabs[nTabCount + TAB_GAP].nPos -=
                        ((mpTabs[nTabCount + TAB_GAP].nPos - lRightPixMargin)
                         % nDefTabDist );
                }

                if(mpTabs[nTabCount + TAB_GAP].nPos <= lParaIndentPix)
                    break;
                mpTabs[nTabCount + TAB_GAP].nStyle = RULER_TAB_DEFAULT;
                ++nTabCount;
            }
        }
        else
        {
            sal_Int32 aFirst = 0;
            for(j = 0; j < nDefTabBuf; ++j)
            {
                if( j == 0 )
                {
                    //set the first default tab stop
                    if(mpRulerImpl->bIsTabsRelativeToIndent)
                    {
                        mpTabs[nTabCount + TAB_GAP].nPos = (mpTabs[nTabCount].nPos + nDefTabDist);

                        mpTabs[nTabCount + TAB_GAP].nPos -=
                            (mpTabs[nTabCount + TAB_GAP].nPos - lParaIndentPix) % nDefTabDist;
                        aFirst = mpTabs[nTabCount + TAB_GAP].nPos;
                    }
                    else
                    {
                        if( mpTabs[nTabCount].nPos < 0 )
                            aFirst = ( mpTabs[nTabCount].nPos / nDefTabDist ) * nDefTabDist;
                        else
                            aFirst = ( mpTabs[nTabCount].nPos / nDefTabDist + 1 ) * nDefTabDist;
                        mpTabs[nTabCount + TAB_GAP].nPos = aFirst;
                    }
                }
                else
                {
                    //simply add the default distance to the last position

                    mpTabs[nTabCount + TAB_GAP].nPos = aFirst + ConvertHPosPixel(j * lDefTabDist);
                }

                if(mpTabs[nTabCount + TAB_GAP].nPos >= lRightIndent)
                    break;
                mpTabs[nTabCount + TAB_GAP].nStyle = RULER_TAB_DEFAULT;
                ++nTabCount;
            }
        }
        SetTabs(nTabCount, &mpTabs[0] + TAB_GAP);
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
                mpRulerImpl->aProtectItem.IsSizeProtected() ||
                mpRulerImpl->aProtectItem.IsPosProtected();
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
    return mpParaItem.get() ? mpIndents[INDENT_FIRST_LINE].nPos : GetMargin1();
}

long SvxRuler::GetLeftIndent() const
{
    /* Get Left paragraph margin in Pixels */
    return mpParaItem.get() ? mpIndents[INDENT_LEFT_MARGIN].nPos : GetMargin1();
}

long SvxRuler::GetRightIndent() const
{
    /* Get Right paragraph margin in Pixels */
    return mpParaItem.get() ? mpIndents[INDENT_RIGHT_MARGIN].nPos : GetMargin2();
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
        mpColumnItem->IsConsistent())
    {
        nLeft = mpColumnItem->GetActiveColumnDescription().nStart;
    }

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
    if (mpColumnItem.get())
    {
        if (!IsActLastColumn(true))
        {
            return mpColumnItem->At(GetActRightColumn(true)).nEnd;
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
     sal_Bool bHoriRows = bHorz && mpRulerImpl->bIsTableRows;
    if((bLeft || (bHoriRows)) && lDragPos < nMaxLeft)
        lDragPos = nMaxLeft;
    else if((bRight||bHoriRows) && lDragPos > nMaxRight)
        lDragPos = nMaxRight;
    return lDragPos - lNullPix;
}

void ModifyTabs_Impl( sal_uInt16 nCount, // Number of Tabs
                      RulerTab* pTabs,   // Tab buffer
                      long lDiff)        // difference to be added
{
    /* Helper function, move all the tabs by a fixed value */
    if( pTabs )
    {
        for(sal_uInt16 i = 0; i < nCount; ++i)
        {
            pTabs[i].nPos += lDiff;
        }
    }
}

void SvxRuler::DragMargin1()
{
    /* Dragging the left edge of frame */
    long aDragPosition = GetCorrectedDragPos( !TAB_FLAG || !NEG_FLAG, sal_True );

    aDragPosition = MakePositionSticky(aDragPosition, GetRightFrameMargin(), false);

    // Check if position changed
    if (aDragPosition == 0)
        return;

    DrawLine_Impl(lTabPos, ( TAB_FLAG && NEG_FLAG ) ? 3 : 7, bHorz);
    if (mpColumnItem.get() && (nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL))
        DragBorders();
    AdjustMargin1(aDragPosition);
}

void SvxRuler::AdjustMargin1(long lInputDiff)
{
    const long nOld = bAppSetNullOffset? GetMargin1(): GetNullOffset();
    const long lDragPos = lInputDiff;

    sal_Bool bProtectColumns =
        mpRulerImpl->aProtectItem.IsSizeProtected() ||
        mpRulerImpl->aProtectItem.IsPosProtected();

    const sal_uInt16 nMarginStyle =
        bProtectColumns ? 0 : RULER_MARGIN_SIZEABLE;

    if(!bAppSetNullOffset)
    {
        long lDiff = lDragPos;
        SetNullOffset(nOld + lDiff);
        if (!mpColumnItem.get() || !(nDragType & DRAG_OBJECT_SIZE_LINEAR))
        {
            SetMargin2( GetMargin2() - lDiff, nMarginStyle );

            if (!mpColumnItem.get() && !mpObjectItem.get() && mpParaItem.get())
            {
                // Right indent of the old position
                mpIndents[INDENT_RIGHT_MARGIN].nPos -= lDiff;
                SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
            }
            if(mpObjectItem.get())
            {
                mpObjectBorders[GetObjectBordersOff(0)].nPos -= lDiff;
                mpObjectBorders[GetObjectBordersOff(1)].nPos -= lDiff;
                SetBorders(2, &mpObjectBorders[0] + GetObjectBordersOff(0));
            }
            if(mpColumnItem.get())
            {
                for(sal_uInt16 i = 0; i < mpColumnItem->Count()-1; ++i)
                    mpBorders[i].nPos -= lDiff;
                SetBorders(mpColumnItem->Count()-1, &mpBorders[0]);
                if(mpColumnItem->IsFirstAct())
                {
                    // Right indent of the old position
                    if(mpParaItem.get())
                    {
                        mpIndents[INDENT_RIGHT_MARGIN].nPos -= lDiff;
                        SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
                    }
                }
                else
                {
                    if(mpParaItem.get())
                    {
                        mpIndents[INDENT_FIRST_LINE].nPos -= lDiff;
                        mpIndents[INDENT_LEFT_MARGIN].nPos -= lDiff;
                        mpIndents[INDENT_RIGHT_MARGIN].nPos -= lDiff;
                        SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
                    }
                }
                if(mpTabStopItem.get() && (nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
                   &&!IsActFirstColumn())
                {
                    ModifyTabs_Impl(nTabCount + TAB_GAP, &mpTabs[0], -lDiff);
                    SetTabs(nTabCount, &mpTabs[0] + TAB_GAP);
                }
            }
        }
    }
    else
    {
        long lDiff = lDragPos - nOld;
        SetMargin1(nOld + lDiff, nMarginStyle);

        if (!mpColumnItem.get() || !(nDragType & (DRAG_OBJECT_SIZE_LINEAR | DRAG_OBJECT_SIZE_PROPORTIONAL)))
        {
            if (!mpColumnItem.get() && !mpObjectItem.get() && mpParaItem.get())
            {
                // Left indent of the old position
                mpIndents[INDENT_FIRST_LINE].nPos += lDiff;
                mpIndents[INDENT_LEFT_MARGIN].nPos += lDiff;
                SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
            }

            if (mpColumnItem.get())
            {
                for(sal_uInt16 i = 0; i < mpColumnItem->Count() - 1; ++i)
                    mpBorders[i].nPos += lDiff;
                SetBorders(mpColumnItem->Count() - 1, &mpBorders[0]);
                if (mpColumnItem->IsFirstAct())
                {
                    // Left indent of the old position
                    if(mpParaItem.get())
                    {
                        mpIndents[INDENT_FIRST_LINE].nPos += lDiff;
                        mpIndents[INDENT_LEFT_MARGIN].nPos += lDiff;
                        SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
                    }
                }
                else
                {
                    if(mpParaItem.get())
                    {
                        mpIndents[INDENT_FIRST_LINE].nPos += lDiff;
                        mpIndents[INDENT_LEFT_MARGIN].nPos += lDiff;
                        mpIndents[INDENT_RIGHT_MARGIN].nPos += lDiff;
                        SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
                    }
                }
            }
            if(mpTabStopItem.get())
            {
                ModifyTabs_Impl(nTabCount + TAB_GAP, &mpTabs[0], lDiff);
                SetTabs(nTabCount, &mpTabs[0] + TAB_GAP);
            }
        }
    }
}

void SvxRuler::DragMargin2()
{
    /* Dragging the right edge of frame */
    long aDragPosition = GetCorrectedDragPos( sal_True, !TAB_FLAG || !NEG_FLAG);
    aDragPosition = MakePositionSticky(aDragPosition, GetLeftFrameMargin(), false);
    long lDiff = aDragPosition - GetMargin2();

    // Check if position changed
    if (lDiff == 0)
        return;

    if( mpRulerImpl->bIsTableRows &&
        !bHorz &&
        mpColumnItem.get() &&
        (nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL))
    {
        DragBorders();
    }

    sal_Bool bProtectColumns =
        mpRulerImpl->aProtectItem.IsSizeProtected() ||
        mpRulerImpl->aProtectItem.IsPosProtected();

    const sal_uInt16 nMarginStyle = bProtectColumns ? 0 : RULER_MARGIN_SIZEABLE;

    SetMargin2( aDragPosition, nMarginStyle );

    // Right indent of the old position
    if((!mpColumnItem.get() || IsActLastColumn()) && mpParaItem.get())
    {
        mpIndents[INDENT_FIRST_LINE].nPos += lDiff;
        SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
    }

    DrawLine_Impl(lTabPos, ( TAB_FLAG && NEG_FLAG ) ? 5 : 7, bHorz);
}

void SvxRuler::DragIndents()
{
    /* Dragging the paragraph indents */
    long aDragPosition = NEG_FLAG ? GetDragPos() : GetCorrectedDragPos();
    const sal_uInt16 nIndex = GetDragAryPos() + INDENT_GAP;

    bool bRTL = mpRulerImpl->pTextRTLItem && mpRulerImpl->pTextRTLItem->GetValue();

    if(nIndex == INDENT_RIGHT_MARGIN)
        aDragPosition = MakePositionSticky(aDragPosition, bRTL ? GetLeftFrameMargin() : GetRightFrameMargin());
    else
        aDragPosition = MakePositionSticky(aDragPosition, bRTL ? GetRightFrameMargin() : GetLeftFrameMargin());

    const long lDiff = mpIndents[nIndex].nPos - aDragPosition;

    // Check if position changed
    if (lDiff == 0)
        return;

    if((nIndex == INDENT_FIRST_LINE || nIndex == INDENT_LEFT_MARGIN )  &&
        (nDragType & DRAG_OBJECT_LEFT_INDENT_ONLY) != DRAG_OBJECT_LEFT_INDENT_ONLY)
    {
        mpIndents[INDENT_FIRST_LINE].nPos -= lDiff;
    }

    mpIndents[nIndex].nPos = aDragPosition;

    SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
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
            nDrapPosition = MakePositionSticky(nDrapPosition, GetLeftFrameMargin());
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
            nDrapPosition = MakePositionSticky(nDrapPosition, GetLeftFrameMargin());
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
    aDragPosition = MakePositionSticky(aDragPosition, GetLeftFrameMargin());

    sal_uInt16 nIdx = GetDragAryPos() + TAB_GAP;
    long nDiff = aDragPosition - mpTabs[nIdx].nPos;
    if (nDiff == 0)
        return;

    DrawLine_Impl(lTabPos, 7, bHorz);

    if(nDragType & DRAG_OBJECT_SIZE_LINEAR)
    {

        for(sal_uInt16 i = nIdx; i < nTabCount; ++i)
        {
            mpTabs[i].nPos += nDiff;
            // limit on maximum
            if(mpTabs[i].nPos > GetMargin2())
                mpTabs[nIdx].nStyle |= RULER_STYLE_INVISIBLE;
            else
                mpTabs[nIdx].nStyle &= ~RULER_STYLE_INVISIBLE;
        }
    }
    else if(nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
    {
        mpRulerImpl->nTotalDist -= nDiff;
        mpTabs[nIdx].nPos = aDragPosition;
        for(sal_uInt16 i = nIdx+1; i < nTabCount; ++i)
        {
            if(mpTabs[i].nStyle & RULER_TAB_DEFAULT)
                // can be canceled at the DefaultTabs
                break;
            long nDelta = mpRulerImpl->nTotalDist * mpRulerImpl->pPercBuf[i];
            nDelta /= 1000;
            mpTabs[i].nPos = mpTabs[nIdx].nPos + nDelta;
            if(mpTabs[i].nPos + GetNullOffset() > nMaxRight)
                mpTabs[i].nStyle |= RULER_STYLE_INVISIBLE;
            else
                mpTabs[i].nStyle &= ~RULER_STYLE_INVISIBLE;
        }
    }
    else
    {
        mpTabs[nIdx].nPos = aDragPosition;
    }

    if(IsDragDelete())
        mpTabs[nIdx].nStyle |= RULER_STYLE_INVISIBLE;
    else
        mpTabs[nIdx].nStyle &= ~RULER_STYLE_INVISIBLE;
    SetTabs(nTabCount, &mpTabs[0] + TAB_GAP);
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
            for(sal_uInt16 i=0;i<mpRulerImpl->nControlerItems;i++)
                pCtrlItem[i]->ReBind();
        else
            for(sal_uInt16 j=0;j<mpRulerImpl->nControlerItems;j++)
                pCtrlItem[j]->UnBind();
        pBindings->LeaveRegistrations();
    }
    bActive = bOn;
}

void SvxRuler::UpdateParaContents_Impl(
                            long lDifference,
                            UpdateType eType)  // Art (all, left or right)
{
    /* Helper function; carry Tabs and Paragraph Margins */
    switch(eType)
    {
        case MOVE_RIGHT:
            mpIndents[INDENT_RIGHT_MARGIN].nPos += lDifference;
            break;
        case MOVE_ALL:
            mpIndents[INDENT_RIGHT_MARGIN].nPos += lDifference;
            // no break
        case MOVE_LEFT:
        {
            mpIndents[INDENT_FIRST_LINE].nPos += lDifference;
            mpIndents[INDENT_LEFT_MARGIN].nPos += lDifference;
            if (!mpTabs.empty())
            {
                for(sal_uInt16 i = 0; i < nTabCount+TAB_GAP; ++i)
                {
                    mpTabs[i].nPos += lDifference;
                }
                SetTabs(nTabCount, &mpTabs[0] + TAB_GAP);
            }
            break;
        }
    }
    SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
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
    long lPos = MakePositionSticky(GetCorrectedDragPos(), GetLeftFrameMargin());

    switch(nDragSize)
    {
        case RULER_DRAGSIZE_MOVE:
        {
ADD_DEBUG_TEXT("lLastLMargin: ", OUString::number(mpRulerImpl->lLastLMargin))
            if(GetDragType() == RULER_TYPE_BORDER)
                lDiff = lPos - nDragOffset - mpBorders[nIndex].nPos;
            else
                lDiff = GetDragType() == RULER_TYPE_MARGIN1 ? lPos - mpRulerImpl->lLastLMargin : lPos - mpRulerImpl->lLastRMargin;

            if(nDragType & DRAG_OBJECT_SIZE_LINEAR)
            {
                long nRight = GetMargin2() - lMinFrame; // Right limiters
                for(int i = mpBorders.size() - 2; i >= nIndex; --i)
                {
                    long l = mpBorders[i].nPos;
                    mpBorders[i].nPos += lDiff;
                    mpBorders[i].nPos = std::min(mpBorders[i].nPos, nRight - mpBorders[i].nWidth);
                    nRight = mpBorders[i].nPos - lMinFrame;
                    // RR update the column
                    if(i == GetActRightColumn())
                    {
                        UpdateParaContents_Impl(mpBorders[i].nPos - l, MOVE_RIGHT);
                        bRightIndentsCorrected = sal_True;
                    }
                    // LAR, EZE update the column
                    else if(i == GetActLeftColumn())
                    {
                        UpdateParaContents_Impl(mpBorders[i].nPos - l, MOVE_LEFT);
                        bLeftIndentsCorrected = sal_True;
                    }
                }
            }
            else if(nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
            {
                int nLimit;
                long lLeft;
                int nStartLimit = mpBorders.size() - 2;
                switch(GetDragType())
                {
                default: ;//prevent warning
                    OSL_FAIL("svx::SvxRuler::DragBorders(), unknown drag type!" );
                case RULER_TYPE_BORDER:
                    if(mpRulerImpl->bIsTableRows)
                    {
                        mpBorders[nIndex].nPos += lDiff;
                        if(bHorz)
                        {
                            lLeft = mpBorders[nIndex].nPos;
                            mpRulerImpl->nTotalDist -= lDiff;
                            nLimit = nIndex + 1;
                        }
                        else
                        {
                            lLeft = 0;
                            nStartLimit = nIndex - 1;
                            mpRulerImpl->nTotalDist += lDiff;
                            nLimit = 0;
                        }
                    }
                    else
                    {
                        nLimit = nIndex + 1;
                        mpBorders[nIndex].nPos += lDiff;
                        lLeft = mpBorders[nIndex].nPos;
                        mpRulerImpl->nTotalDist -= lDiff;
                    }
                break;
                case RULER_TYPE_MARGIN1:
                    nLimit = 0;
                    lLeft = mpRulerImpl->lLastLMargin + lDiff;
                    mpRulerImpl->nTotalDist -= lDiff;
                break;
                case RULER_TYPE_MARGIN2:
                    nLimit = 0;
                    lLeft= 0;
                    nStartLimit = mpBorders.size() - 2;
                    mpRulerImpl->nTotalDist += lDiff;
                break;
                }

                for(int i  = nStartLimit; i >= nLimit; --i)
                {

                    long l = mpBorders[i].nPos;
                    mpBorders[i].nPos =
                        lLeft +
                        (mpRulerImpl->nTotalDist * mpRulerImpl->pPercBuf[i]) / 1000 +
                        mpRulerImpl->pBlockBuf[i];

                    // RR update the column
                    if(!mpRulerImpl->bIsTableRows)
                    {
                        if(i == GetActRightColumn())
                        {
                            UpdateParaContents_Impl(mpBorders[i].nPos - l, MOVE_RIGHT);
                            bRightIndentsCorrected = sal_True;
                        }
                        // LAR, EZE update the column
                        else if(i == GetActLeftColumn())
                        {
                            UpdateParaContents_Impl(mpBorders[i].nPos - l, MOVE_LEFT);
                            bLeftIndentsCorrected = sal_True;
                        }
                    }
                }
                if(mpRulerImpl->bIsTableRows)
                {
                    //in vertical tables the left borders have to be moved
                    if(bHorz)
                    {
                        for(int i  = 0; i < nIndex; ++i)
                            mpBorders[i].nPos += lDiff;
                        AdjustMargin1(lDiff);
                    }
                    else
                    {
                        //otherwise the right borders are moved
                        for(int i  = mpColumnItem->Count() - 1; i > nIndex; --i)
                            mpBorders[i].nPos += lDiff;
                        SetMargin2( GetMargin2() + lDiff, 0 );
                    }
                }
            }
            else if(mpRulerImpl->bIsTableRows)
            {
                //moving rows: if a row is resized all following rows
                //have to be moved by the same amount.
                //This includes the left border when the table is not limited
                //to a lower frame border.
                int nLimit;
                if(GetDragType()==RULER_TYPE_BORDER)
                {
                    nLimit = nIndex + 1;
                    mpBorders[nIndex].nPos += lDiff;
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
                        mpBorders[i].nPos += lDiff;
                    }
                    AdjustMargin1(lDiff);
                }
                else
                {
                    //otherwise the right borders are moved
                    for(int i  = mpBorders.size() - 2; i >= nLimit; --i)
                    {
                        mpBorders[i].nPos += lDiff;
                    }
                    SetMargin2( GetMargin2() + lDiff, 0 );
                }
            }
            else
                mpBorders[nIndex].nPos += lDiff;
            break;
        }
      case RULER_DRAGSIZE_1:
        {
            lDiff = lPos - mpBorders[nIndex].nPos;
            mpBorders[nIndex].nWidth += mpBorders[nIndex].nPos - lPos;
            mpBorders[nIndex].nPos = lPos;
            break;
        }
      case RULER_DRAGSIZE_2:
        {
            const long nOld = mpBorders[nIndex].nWidth;
            mpBorders[nIndex].nWidth = lPos - mpBorders[nIndex].nPos;
            lDiff = mpBorders[nIndex].nWidth - nOld;
            break;
        }
    }
    if(!bRightIndentsCorrected &&
       GetActRightColumn() == nIndex &&
       nDragSize != RULER_DRAGSIZE_2 &&
       !mpIndents.empty() &&
       !mpRulerImpl->bIsTableRows)
    {
        UpdateParaContents_Impl(lDiff, MOVE_RIGHT);
    }
    else if(!bLeftIndentsCorrected &&
            GetActLeftColumn() == nIndex &&
            nDragSize != RULER_DRAGSIZE_1 &&
            !mpIndents.empty())
    {
        UpdateParaContents_Impl(lDiff, MOVE_LEFT);
    }
    SetBorders(mpColumnItem->Count() - 1, &mpBorders[0]);
}

void SvxRuler::DragObjectBorder()
{
    /* Dragging of object edges */
    if(RULER_DRAGSIZE_MOVE == GetDragSize())
    {
        const long lPosition = MakePositionSticky(GetCorrectedDragPos(), GetLeftFrameMargin());

        const sal_uInt16 nIdx = GetDragAryPos();
        mpObjectBorders[GetObjectBordersOff(nIdx)].nPos = lPosition;
        SetBorders(2, &mpObjectBorders[0] + GetObjectBordersOff(0));
        DrawLine_Impl(lTabPos, 7, bHorz);

    }
}

void SvxRuler::ApplyMargins()
{
    /* Applying margins; changed by dragging. */
    const SfxPoolItem* pItem = NULL;
    sal_uInt16 nId = SID_ATTR_LONG_LRSPACE;

    if(bHorz)
    {
        const long lOldNull = lLogicNullOffset;
        if(mpRulerImpl->lMaxLeftLogic != -1 && nMaxLeft == GetMargin1() + Ruler::GetNullOffset())
        {
            lLogicNullOffset = mpRulerImpl->lMaxLeftLogic;
            mpLRSpaceItem->SetLeft(lLogicNullOffset);
        }
        else
        {
            lLogicNullOffset = ConvertHPosLogic(GetFrameLeft()) - lAppNullOffset;
            mpLRSpaceItem->SetLeft(PixelHAdjust(lLogicNullOffset, mpLRSpaceItem->GetLeft()));
        }

        if(bAppSetNullOffset)
            lAppNullOffset += lLogicNullOffset - lOldNull;

        long nRight;
        if(mpRulerImpl->lMaxRightLogic != -1
           && nMaxRight == GetMargin2() + Ruler::GetNullOffset())
        {
            nRight = GetPageWidth() - mpRulerImpl->lMaxRightLogic;
        }
        else
        {
            nRight = std::max((long)0,
                            mpPagePosItem->GetWidth() - mpLRSpaceItem->GetLeft() -
                                (ConvertHPosLogic(GetMargin2()) - lAppNullOffset));

            nRight = PixelHAdjust( nRight, mpLRSpaceItem->GetRight());
        }
        mpLRSpaceItem->SetRight(nRight);

        pItem = mpLRSpaceItem.get();

#ifdef DEBUGLIN
        Debug_Impl(pEditWin, *mpLRSpaceItem);
#endif // DEBUGLIN

    }
    else
    {
        const long lOldNull = lLogicNullOffset;
        mpULSpaceItem->SetUpper(
            PixelVAdjust(
                lLogicNullOffset =
                ConvertVPosLogic(GetFrameLeft()) -
                lAppNullOffset, mpULSpaceItem->GetUpper()));
        if(bAppSetNullOffset)
            lAppNullOffset += lLogicNullOffset - lOldNull;
        mpULSpaceItem->SetLower(
            PixelVAdjust(
                std::max((long)0, mpPagePosItem->GetHeight() -
                    mpULSpaceItem->GetUpper() -
                    (ConvertVPosLogic(GetMargin2()) -
                     lAppNullOffset)), mpULSpaceItem->GetLower()));
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

long SvxRuler::RoundToCurrentMapMode(long lValue) const
{
    RulerUnitData aUnitData = GetCurrentRulerUnit();
    double aRoundingFactor = aUnitData.nTickUnit / aUnitData.nTick1;

    long lNewValue = pEditWin->LogicToLogic(Size(lValue, 0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
    lNewValue = (std::round(lNewValue / (double) aUnitData.nTickUnit * aRoundingFactor) / aRoundingFactor) * aUnitData.nTickUnit;
    return pEditWin->LogicToLogic(Size(lNewValue, 0), GetCurrentMapMode(), pEditWin->GetMapMode()).Width();
}

void SvxRuler::ApplyIndents()
{
    /* Applying paragraph settings; changed by dragging. */

    long nLeftFrameMargin  = GetLeftFrameMargin();

    bool bRTL = mpRulerImpl->pTextRTLItem && mpRulerImpl->pTextRTLItem->GetValue();

    long nNewTxtLeft;
    long nNewFirstLineOffset;
    long nNewRight;

    long nFirstLine    = ConvertPosLogic(mpIndents[INDENT_FIRST_LINE].nPos);
    long nLeftMargin   = ConvertPosLogic(mpIndents[INDENT_LEFT_MARGIN].nPos);
    long nRightMargin  = ConvertPosLogic(mpIndents[INDENT_RIGHT_MARGIN].nPos);

    if(mpColumnItem.get() && ((bRTL && !IsActLastColumn(sal_True)) || (!bRTL && !IsActFirstColumn(sal_True))))
    {
        if(bRTL)
        {
            long nRightColumn  = GetActRightColumn(sal_True);
            long nRightBorder  = ConvertPosLogic(mpBorders[nRightColumn].nPos);
            nNewTxtLeft = nRightBorder - nLeftMargin - lAppNullOffset;
        }
        else
        {
            long nLeftColumn = GetActLeftColumn(sal_True);
            long nLeftBorder = ConvertPosLogic(mpBorders[nLeftColumn].nPos + mpBorders[nLeftColumn].nWidth);
            nNewTxtLeft = nLeftMargin - nLeftBorder - lAppNullOffset;
        }
    }
    else
    {
        if(bRTL)
        {
            long nRightBorder = ConvertPosLogic(GetMargin2());
            nNewTxtLeft = nRightBorder - nLeftMargin - lAppNullOffset;
        }
        else
        {
            long nLeftBorder = ConvertPosLogic(GetMargin1());
            nNewTxtLeft = nLeftBorder + nLeftMargin - nLeftFrameMargin - lAppNullOffset;
        }
    }

    if(bRTL)
        nNewFirstLineOffset = nLeftMargin - nFirstLine - lAppNullOffset;
    else
        nNewFirstLineOffset = nFirstLine - nLeftMargin - lAppNullOffset;

    if(mpColumnItem.get() && ((!bRTL && !IsActLastColumn(sal_True)) || (bRTL && !IsActFirstColumn(sal_True))))
    {
        if(bRTL)
        {
            long nLeftColumn = GetActLeftColumn(sal_True);
            long nLeftBorder = ConvertPosLogic(mpBorders[nLeftColumn].nPos + mpBorders[nLeftColumn].nWidth);
            nNewRight = nRightMargin - nLeftBorder - lAppNullOffset;
        }
        else
        {
            long nRightColumn  = GetActRightColumn(sal_True);
            long nRightBorder  = ConvertPosLogic(mpBorders[nRightColumn].nPos);
            nNewRight = nRightBorder - nRightMargin - lAppNullOffset;
        }
    }
    else
    {
        if(bRTL)
        {
            long nLeftBorder = ConvertPosLogic(GetMargin1());
            nNewRight = nLeftBorder + nRightMargin - nLeftFrameMargin - lAppNullOffset;
        }
        else
        {
            long nRightBorder = ConvertPosLogic(GetMargin2());
            nNewRight = nRightBorder - nRightMargin - lAppNullOffset;
        }
    }

    if (mbSnapping)
    {
        nNewTxtLeft         = RoundToCurrentMapMode(nNewTxtLeft);
        nNewFirstLineOffset = RoundToCurrentMapMode(nNewFirstLineOffset);
        nNewRight           = RoundToCurrentMapMode(nNewRight);
    }

    mpParaItem->SetTxtFirstLineOfst(sal::static_int_cast<short>(nNewFirstLineOffset));
    mpParaItem->SetTxtLeft(nNewTxtLeft);
    mpParaItem->SetRight(nNewRight);

    sal_uInt16 nParagraphId  = bHorz ? SID_ATTR_PARA_LRSPACE : SID_ATTR_PARA_LRSPACE_VERTICAL;
    pBindings->GetDispatcher()->Execute( nParagraphId, SFX_CALLMODE_RECORD, mpParaItem.get(), 0L );
    UpdateTabs();
}

void SvxRuler::ApplyTabs()
{
    /* Apply tab settings, changed by dragging. */
    sal_Bool bRTL = mpRulerImpl->pTextRTLItem && mpRulerImpl->pTextRTLItem->GetValue();
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
            pItem->Insert(mpTabStopItem->At(j));
        }
        for(; j < mpTabStopItem->Count(); ++j)
        {
            SvxTabStop aTabStop = mpTabStopItem->At(j);
            aTabStop.GetTabPos() = PixelHAdjust(
                ConvertHPosLogic(
                    mpTabs[j + TAB_GAP].nPos - GetLeftIndent()) - lAppNullOffset,
                aTabStop.GetTabPos());
            pItem->Insert(aTabStop);
        }
        mpTabStopItem.reset(pItem);
    }
    else if( mpTabStopItem->Count() == 0 )
        return;
    else
    {
        SvxTabStop aTabStop = mpTabStopItem->At(nCoreIdx);
        if( mpRulerImpl->lMaxRightLogic != -1 &&
            mpTabs[nCoreIdx + TAB_GAP].nPos + Ruler::GetNullOffset() == nMaxRight )
        {
            aTabStop.GetTabPos() = mpRulerImpl->lMaxRightLogic - lLogicNullOffset;
        }
        else
        {
            if(bRTL)
            {
                //#i24363# tab stops relative to indent
                const long nTmpLeftIndent = mpRulerImpl->bIsTabsRelativeToIndent ?
                                            GetLeftIndent() :
                                            ConvertHPosPixel( GetRightFrameMargin() + lAppNullOffset );

                long nNewPosition = ConvertHPosLogic(nTmpLeftIndent - mpTabs[nCoreIdx + TAB_GAP].nPos);
                aTabStop.GetTabPos() = PixelHAdjust(nNewPosition - lAppNullOffset, aTabStop.GetTabPos());
            }
            else
            {
                //#i24363# tab stops relative to indent
                const long nTmpLeftIndent = mpRulerImpl->bIsTabsRelativeToIndent ?
                                            GetLeftIndent() : 0;

                long nNewPosition = ConvertHPosLogic(mpTabs[nCoreIdx + TAB_GAP].nPos - nTmpLeftIndent);
                aTabStop.GetTabPos() = PixelHAdjust(nNewPosition - lAppNullOffset, aTabStop.GetTabPos());
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
        long lValue = GetFrameLeft();
        if(lValue != mpRulerImpl->nColLeftPix)
        {
            long nLeft = PixelHAdjust(
                            ConvertHPosLogic(lValue) -
                                lAppNullOffset,
                            mpColumnItem->GetLeft());
            mpColumnItem->SetLeft(nLeft);
        }

        lValue = GetMargin2();

        if(lValue != mpRulerImpl->nColRightPix)
        {
            long nWidthOrHeight = bHorz ? mpPagePosItem->GetWidth() : mpPagePosItem->GetHeight();
            long nRight = PixelHAdjust(
                            nWidthOrHeight -
                                mpColumnItem->GetLeft() -
                                ConvertHPosLogic(lValue) -
                                lAppNullOffset,
                            mpColumnItem->GetRight() );
            mpColumnItem->SetRight(nRight);
        }
    }

    for(sal_uInt16 i = 0; i < mpColumnItem->Count() - 1; ++i)
    {
        long& nEnd = mpColumnItem->At(i).nEnd;
        nEnd = PixelHAdjust(
                ConvertPosLogic(mpBorders[i].nPos),
                mpColumnItem->At(i).nEnd);
        long& nStart = mpColumnItem->At(i + 1).nStart;
        nStart = PixelHAdjust(
                    ConvertSizeLogic(mpBorders[i].nPos +
                        mpBorders[i].nWidth) -
                        lAppNullOffset,
                    mpColumnItem->At(i + 1).nStart);
        // It may be that, due to the PixelHAdjust readjustment to old values,
        // the width becomes  < 0. This we readjust.
        if( nEnd > nStart )
            nStart = nEnd;
    }

#ifdef DEBUGLIN
        Debug_Impl(pEditWin,*mpColumnItem.get());
#endif // DEBUGLIN

    SfxBoolItem aFlag(SID_RULER_ACT_LINE_ONLY,
                      nDragType & DRAG_OBJECT_ACTLINE_ONLY ? sal_True : sal_False);

    sal_uInt16 nColId = mpRulerImpl->bIsTableRows ? (bHorz ? SID_RULER_ROWS : SID_RULER_ROWS_VERTICAL) :
                            (bHorz ? SID_RULER_BORDERS : SID_RULER_BORDERS_VERTICAL);

    pBindings->GetDispatcher()->Execute( nColId, SFX_CALLMODE_RECORD, mpColumnItem.get(), &aFlag, 0L );
}

void SvxRuler::ApplyObject()
{
    /* Applying object settings, changed by dragging. */

    // to the page margin
    long nMargin = mpLRSpaceItem.get() ? mpLRSpaceItem->GetLeft() : 0;
    long nStartX = PixelAdjust(
                    ConvertPosLogic(mpObjectBorders[0].nPos) +
                        nMargin -
                        lAppNullOffset,
                    mpObjectItem->GetStartX());
    mpObjectItem->SetStartX(nStartX);

    long nEndX = PixelAdjust(
                    ConvertPosLogic(mpObjectBorders[1].nPos) +
                        nMargin -
                        lAppNullOffset,
                    mpObjectItem->GetEndX());
    mpObjectItem->SetEndX(nEndX);

    nMargin = mpULSpaceItem.get() ? mpULSpaceItem->GetUpper() : 0;
    long nStartY = PixelAdjust(
                    ConvertPosLogic(mpObjectBorders[2].nPos) +
                        nMargin -
                        lAppNullOffset,
                    mpObjectItem->GetStartY());
    mpObjectItem->SetStartY(nStartY);

    long nEndY = PixelAdjust(
                    ConvertPosLogic(mpObjectBorders[3].nPos) +
                        nMargin -
                        lAppNullOffset,
                    mpObjectItem->GetEndY());
    mpObjectItem->SetEndY(nEndY);

    pBindings->GetDispatcher()->Execute(SID_RULER_OBJECT, SFX_CALLMODE_RECORD, mpObjectItem.get(), 0L);
}

void SvxRuler::PrepareProportional_Impl(RulerType eType)
{
    /*
       Preparation proportional dragging, and it is calculated based on the
       proportional share of the total width in parts per thousand.
    */
    mpRulerImpl->nTotalDist = GetMargin2();
    switch((int)eType)
    {
      case RULER_TYPE_MARGIN2:
      case RULER_TYPE_MARGIN1:
      case RULER_TYPE_BORDER:
        {
            DBG_ASSERT(mpColumnItem.get(), "no ColumnItem");

            mpRulerImpl->SetPercSize(mpColumnItem->Count());

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
                if(mpRulerImpl->bIsTableRows &&!bHorz)
                {
                    lOrigLPos = GetMargin1();
                    nStart = 0;
                }
                else
                {
                    lOrigLPos = mpBorders[nIdx].nPos + mpBorders[nIdx].nWidth;
                    nStart = 1;
                }
                lActBorderSum = mpBorders[nIdx].nWidth;
            }

            //in horizontal mode the percentage value has to be
            //calculated on a "current change" position base
            //because the height of the table changes while dragging
            if(mpRulerImpl->bIsTableRows && RULER_TYPE_BORDER == eType)
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

                lWidth = mpBorders[nIdx].nPos;
                if(bHorz)
                    lWidth = GetMargin2() - lWidth;
                mpRulerImpl->nTotalDist = lWidth;
                lPos = lOrigLPos = mpBorders[nIdx].nPos;

                for(sal_uInt16 i = nStartBorder; i < nEndBorder; ++i)
                {
                    if(bHorz)
                    {
                        lActWidth += mpBorders[i].nPos - lPos;
                        lPos = mpBorders[i].nPos + mpBorders[i].nWidth;
                    }
                    else
                        lActWidth = mpBorders[i].nPos;
                    mpRulerImpl->pPercBuf[i] = (sal_uInt16)((lActWidth * 1000)
                                                    / mpRulerImpl->nTotalDist);
                    mpRulerImpl->pBlockBuf[i] = (sal_uInt16)lActBorderSum;
                    lActBorderSum += mpBorders[i].nWidth;
                }
            }
            else
            {
                lPos = lOrigLPos;
                for(sal_uInt16 ii = nStart; ii < mpColumnItem->Count() - 1; ++ii)
                {
                    lWidth += mpBorders[ii].nPos - lPos;
                    lPos = mpBorders[ii].nPos + mpBorders[ii].nWidth;
                }

                lWidth += GetMargin2() - lPos;
                mpRulerImpl->nTotalDist = lWidth;
                lPos = lOrigLPos;

                for(sal_uInt16 i = nStart; i < mpColumnItem->Count() - 1; ++i)
                {
                    lActWidth += mpBorders[i].nPos - lPos;
                    lPos = mpBorders[i].nPos + mpBorders[i].nWidth;
                    mpRulerImpl->pPercBuf[i] = (sal_uInt16)((lActWidth * 1000)
                                                    / mpRulerImpl->nTotalDist);
                    mpRulerImpl->pBlockBuf[i] = (sal_uInt16)lActBorderSum;
                    lActBorderSum += mpBorders[i].nWidth;
                }
            }
        }
        break;
        case RULER_TYPE_TAB:
        {
            const sal_uInt16 nIdx = GetDragAryPos()+TAB_GAP;
            mpRulerImpl->nTotalDist -= mpTabs[nIdx].nPos;
            mpRulerImpl->SetPercSize(nTabCount);
            for(sal_uInt16 n=0;n<=nIdx;mpRulerImpl->pPercBuf[n++]=0) ;
            for(sal_uInt16 i = nIdx+1; i < nTabCount; ++i)
            {
                const long nDelta = mpTabs[i].nPos - mpTabs[nIdx].nPos;
                mpRulerImpl->pPercBuf[i] = (sal_uInt16)((nDelta * 1000) / mpRulerImpl->nTotalDist);
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
    Shift + Control: Table: only current line
    Alt: disable snapping
    Alt + Shift: coarse snapping
    */

    sal_uInt16 nModifier = GetDragModifier();
    if(mpRulerImpl->bIsTableRows)
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
        case KEY_MOD2 | KEY_SHIFT:
            mbCoarseSnapping = true;
        break;
        case KEY_MOD2:
            mbSnapping = false;
        break;
        case KEY_MOD1:
        {
            const RulerType eType = GetDragType();
            nDragType = DRAG_OBJECT_SIZE_PROPORTIONAL;
            if( RULER_TYPE_TAB == eType ||
                ( ( RULER_TYPE_BORDER == eType  ||
                    RULER_TYPE_MARGIN1 == eType ||
                    RULER_TYPE_MARGIN2 == eType ) &&
                mpColumnItem.get() ) )
            {
                PrepareProportional_Impl(eType);
            }
        }
        break;
        case KEY_MOD1 | KEY_SHIFT:
            if( GetDragType() != RULER_TYPE_MARGIN1 &&
                GetDragType() != RULER_TYPE_MARGIN2 )
            {
                nDragType = DRAG_OBJECT_ACTLINE_ONLY;
            }
        break;
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
    sal_Bool bRTL = mpRulerImpl->pTextRTLItem && mpRulerImpl->pTextRTLItem->GetValue();
    if(mpTabStopItem.get() &&
       (nFlags & SVXRULER_SUPPORT_TABS) == SVXRULER_SUPPORT_TABS)
    {
        sal_Bool bContentProtected = mpRulerImpl->aProtectItem.IsCntntProtected();
        if( bContentProtected ) return;
        const long lPos = GetClickPos();
        if((bRTL && lPos < std::min(GetFirstLineIndent(), GetLeftIndent()) && lPos > GetRightIndent()) ||
            (!bRTL && lPos > std::min(GetFirstLineIndent(), GetLeftIndent()) && lPos < GetRightIndent()))
        {
            //convert position in left-to-right text
            long nTabPos;
    //#i24363# tab stops relative to indent
            if(bRTL)
                nTabPos = ( mpRulerImpl->bIsTabsRelativeToIndent ?
                            GetLeftIndent() :
                            ConvertHPosPixel( GetRightFrameMargin() + lAppNullOffset ) ) -
                          lPos;
            else
                nTabPos = lPos -
                          ( mpRulerImpl->bIsTabsRelativeToIndent ?
                            GetLeftIndent() :
                            0 );

            SvxTabStop aTabStop(ConvertHPosLogic(nTabPos),
                                ToAttrTab_Impl(nDefTabType));
            mpTabStopItem->Insert(aTabStop);
            UpdateTabs();
        }
    }
}

sal_Bool SvxRuler::CalcLimits ( long& nMax1,    // minimum value to be set
                                long& nMax2,    // minimum value to be set
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
    sal_Bool bRTL = mpRulerImpl->pTextRTLItem && mpRulerImpl->pTextRTLItem->GetValue();
    const long lNullPix = ConvertPosPixel(lLogicNullOffset);
    mpRulerImpl->lMaxLeftLogic=mpRulerImpl->lMaxRightLogic=-1;
    switch(GetDragType())
    {
        case RULER_TYPE_MARGIN1:
        {        // left edge of the surrounding Frame
            // DragPos - NOf between left - right
            mpRulerImpl->lMaxLeftLogic = GetLeftMin();
            nMaxLeft=ConvertSizePixel(mpRulerImpl->lMaxLeftLogic);

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
            else if(mpRulerImpl->bIsTableRows)
            {
                //top border is not moveable when table rows are displayed
                // protection of content means the margin is not moveable
                // - it's just a page break inside of a cell
                if(bHorz && !mpRulerImpl->aProtectItem.IsCntntProtected())
                {
                    nMaxLeft = mpBorders[0].nMinPos + lNullPix;
                    if(nDragType & DRAG_OBJECT_SIZE_PROPORTIONAL)
                        nMaxRight = GetRightIndent() + lNullPix -
                                (mpColumnItem->Count() - 1 ) * lMinFrame;
                    else
                        nMaxRight = mpBorders[0].nPos - lMinFrame + lNullPix;
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
                                mpBorders[0].nPos,
                                std::max(GetFirstLineIndent(), GetLeftIndent()) - GetRightIndent());
                        }
                        else
                        {
                            nMaxRight += std::min(
                                mpBorders[0].nPos, GetRightIndent() -
                                std::max(GetFirstLineIndent(), GetLeftIndent()));
                        }
                    }
                    else if ( mpColumnItem->Count() > 1 )
                    {
                        nMaxRight += mpBorders[0].nPos;
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
            mpRulerImpl->lMaxRightLogic =
                mpMinMaxItem.get() ?
                    GetPageWidth() - GetRightMax() :
                    GetPageWidth();
            nMaxRight = ConvertSizePixel(mpRulerImpl->lMaxRightLogic);


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
            else if(mpRulerImpl->bIsTableRows)
            {
                // get the bottom move range from the last border position - only available for rows!
                // protection of content means the margin is not moveable - it's just a page break inside of a cell
                if(bHorz || mpRulerImpl->aProtectItem.IsCntntProtected())
                {
                    nMaxLeft = nMaxRight = mpBorders[mpColumnItem->Count() - 1].nMaxPos + lNullPix;
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
                            nMaxLeft = mpBorders[mpColumnItem->Count() - 2].nPos + lMinFrame + lNullPix;
                        else
                            nMaxLeft = lMinFrame + lNullPix;
                    }
                    if(mpColumnItem->Count() > 1)
                        nMaxRight = mpBorders[mpColumnItem->Count() - 2].nMaxPos + lNullPix;
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
                        mpBorders[mpColumnItem->Count() - 2].nPos +
                        mpBorders[mpColumnItem->Count() - 2].nWidth;
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
                nMaxRight = mpBorders[nIdx].nPos +
                    mpBorders[nIdx].nWidth + lNullPix;

                if(0 == nIdx)
                    nMaxLeft = lNullPix;
                else
                    nMaxLeft = mpBorders[nIdx - 1].nPos + mpBorders[nIdx - 1].nWidth + lNullPix;
                if(nIdx == mpColumnItem->GetActColumn())
                {
                    if(bRTL)
                    {
                        nMaxLeft += mpBorders[nIdx].nPos +
                            GetRightIndent() - std::max(GetFirstLineIndent(),
                                                   GetLeftIndent());
                    }
                    else
                    {
                        nMaxLeft += mpBorders[nIdx].nPos -
                            GetRightIndent() + std::max(GetFirstLineIndent(),
                                                   GetLeftIndent());
                    }
                    if(0 != nIdx)
                        nMaxLeft -= mpBorders[nIdx-1].nPos +
                            mpBorders[nIdx-1].nWidth;
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
                                mpBorders[nLeftCol].nPos;
                            //rows can always be increased without a limit
                            if(mpRulerImpl->bIsTableRows)
                                nMaxRight = mpBorders[nIdx].nMaxPos;
                            else
                                nMaxRight = nRightCol == USHRT_MAX ?
                                    GetMargin2():
                                    mpBorders[nRightCol].nPos;
                            nMaxLeft += lNullPix;
                            nMaxRight += lNullPix;
                        }
                        else
                        {
                            if(DRAG_OBJECT_SIZE_PROPORTIONAL & nDragType && !bHorz && mpRulerImpl->bIsTableRows)
                                nMaxLeft = (nIdx + 1) * lMinFrame + lNullPix;
                            else
                                nMaxLeft = mpBorders[nIdx].nMinPos + lNullPix;
                            if(DRAG_OBJECT_SIZE_PROPORTIONAL & nDragType||
                            (DRAG_OBJECT_SIZE_LINEAR & nDragType) )
                            {
                                if(mpRulerImpl->bIsTableRows)
                                {
                                    if(bHorz)
                                        nMaxRight = GetRightIndent() + lNullPix -
                                                (mpColumnItem->Count() - nIdx - 1) * lMinFrame;
                                    else
                                        nMaxRight = mpBorders[nIdx].nMaxPos + lNullPix;
                                }
                                else
                                    nMaxRight=lNullPix+CalcPropMaxRight(nIdx);
                            }
                            else
                                nMaxRight = mpBorders[nIdx].nMaxPos + lNullPix;
                        }
                        nMaxLeft += lMinFrame;
                        nMaxRight -= lMinFrame;

                    }
                    else
                    {
                        if(nLeftCol==USHRT_MAX)
                            nMaxLeft=lNullPix;
                        else
                            nMaxLeft = mpBorders[nLeftCol].nPos +
                                mpBorders[nLeftCol].nWidth + lNullPix;

                        if(nActRightCol == nIdx)
                        {
                            if(bRTL)
                            {
                                nMaxLeft += mpBorders[nIdx].nPos +
                                    GetRightIndent() - std::max(GetFirstLineIndent(),
                                                           GetLeftIndent());
                                if(nActLeftCol!=USHRT_MAX)
                                    nMaxLeft -= mpBorders[nActLeftCol].nPos +
                                        mpBorders[nActLeftCol].nWidth;
                            }
                            else
                            {
                                nMaxLeft += mpBorders[nIdx].nPos -
                                    GetRightIndent() + std::max(GetFirstLineIndent(),
                                                           GetLeftIndent());
                                if(nActLeftCol!=USHRT_MAX)
                                    nMaxLeft -= mpBorders[nActLeftCol].nPos +
                                        mpBorders[nActLeftCol].nWidth;
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
                            nMaxRight = lNullPix + GetMargin2() - GetMargin1() +
                                (mpBorders.size() - nIdx - 1) * lMinFrame;
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
                                    nMaxRight += mpBorders[nIdx].nPos +
                                        mpBorders[nIdx].nWidth;
                                }
                            }
                            else
                            {
                                nMaxRight = lNullPix + mpBorders[nRightCol].nPos;
                                sal_uInt16 nNotHiddenRightCol =
                                    GetActRightColumn(sal_True, nIdx);

                                if( nActLeftCol == nIdx )
                                {
                                    long nBorder = nNotHiddenRightCol ==
                                        USHRT_MAX ?
                                        GetMargin2() :
                                        mpBorders[nNotHiddenRightCol].nPos;
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
                                    nMaxRight += mpBorders[nIdx].nPos +
                                        mpBorders[nIdx].nWidth;
                                }
                            }
                            nMaxRight -= lMinFrame;
                            nMaxRight -= mpBorders[nIdx].nWidth;
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
                nMaxLeft = lNullPix + mpBorders[nIdx].nPos;
                if(nIdx == mpColumnItem->Count()-2) { // last column
                    nMaxRight = GetMargin2() + lNullPix;
                    if(mpColumnItem->IsLastAct()) {
                        nMaxRight -=
                            GetMargin2() - GetRightIndent() +
                                std::max(GetFirstLineIndent(),
                                    GetLeftIndent());
                        nMaxRight += mpBorders[nIdx].nPos +
                            mpBorders[nIdx].nWidth;
                    }
                }
                else {
                    nMaxRight = lNullPix + mpBorders[nIdx+1].nPos;
                    if(mpColumnItem->GetActColumn()-1 == nIdx) {
                        nMaxRight -= mpBorders[nIdx+1].nPos  - GetRightIndent() +
                            std::max(GetFirstLineIndent(),
                                GetLeftIndent());
                        nMaxRight += mpBorders[nIdx].nPos +
                            mpBorders[nIdx].nWidth;
                    }
            }
                nMaxRight -= lMinFrame;
                nMaxRight -= mpBorders[nIdx].nWidth;
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
                        nMaxLeft += mpBorders[mpColumnItem->GetActColumn()-1].nPos +
                            mpBorders[mpColumnItem->GetActColumn()-1].nWidth;
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
                        nMaxLeft += mpBorders[mpColumnItem->GetActColumn()-1].nPos +
                            mpBorders[mpColumnItem->GetActColumn()-1].nWidth;
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
                            nMaxRight += mpBorders[nRightCol].nPos;
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
                            nMaxRight += mpBorders[nRightCol].nPos;
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

        mpRulerImpl->lMaxRightLogic = GetLogicRightIndent() + lLogicNullOffset;
        nMaxRight = ConvertSizePixel(mpRulerImpl->lMaxRightLogic);
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
    sal_Bool bContentProtected = mpRulerImpl->aProtectItem.IsCntntProtected();

    if(!bValid)
        return sal_False;

    mpRulerImpl->lLastLMargin = GetMargin1();
    mpRulerImpl->lLastRMargin = GetMargin2();

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
                    if(!mpColumnItem.get())
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
                        nDragOffset = GetDragPos() - mpBorders[GetDragAryPos()].nPos;
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
                    mpIndents[0] = mpIndents[INDENT_FIRST_LINE];
                    mpIndents[0].nStyle |= RULER_STYLE_DONTKNOW;
                    EvalModifier();
                }
                else
                {
                    nDragType = DRAG_OBJECT;
                }
                mpIndents[1] = mpIndents[GetDragAryPos() + INDENT_GAP];
                mpIndents[1].nStyle |= RULER_STYLE_DONTKNOW;
                break;
            }
            case RULER_TYPE_TAB: // Tabs (Modifier)
                if( bContentProtected )
                    return sal_False;
                EvalModifier();
                mpTabs[0] = mpTabs[GetDragAryPos() + 1];
                mpTabs[0].nStyle |= RULER_STYLE_DONTKNOW;
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
            mpRulerImpl->lLastLMargin = GetMargin1();
            break;
        case RULER_TYPE_MARGIN2: // right edge of the surrounding Frame
            DragMargin2();
            mpRulerImpl->lLastRMargin = GetMargin2();
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
                    (mpRulerImpl->bIsTableRows && bHorz)) //special case - the null offset is changed here
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
                SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
                break;
            case RULER_TYPE_TAB: // Tabs
                {
                    ApplyTabs();
                    mpTabs[GetDragAryPos()].nStyle &= ~RULER_STYLE_INVISIBLE;
                    SetTabs(nTabCount, &mpTabs[0] + TAB_GAP);
                }
                break;
            default:
                break; //prevent warning
        }
    }
    nDragType = NONE;

    mbCoarseSnapping = false;
    mbSnapping = true;

    Ruler::EndDrag();
    if(bUndo)
    {
        for(sal_uInt16 i = 0; i < mpRulerImpl->nControlerItems; i++)
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
    if(mpTabStopItem.get() && mpTabStopItem->Count() > mpRulerImpl->nIdx)
    {
        SvxTabStop aTabStop = mpTabStopItem->At(mpRulerImpl->nIdx);
        aTabStop.GetAdjustment() = ToAttrTab_Impl(pMenu->GetCurItemId() - 1);
        mpTabStopItem->Remove(mpRulerImpl->nIdx);
        mpTabStopItem->Insert(aTabStop);
        sal_uInt16 nTabStopId = bHorz ? SID_ATTR_TABSTOP : SID_ATTR_TABSTOP_VERTICAL;
        pBindings->GetDispatcher()->Execute( nTabStopId, SFX_CALLMODE_RECORD, mpTabStopItem.get(), 0L );
        UpdateTabs();
        mpRulerImpl->nIdx = 0;
    }
    return 0;
}

void SvxRuler::Command( const CommandEvent& rCommandEvent )
{
    /* Mouse context menu for switching the unit of measurement */
    if ( COMMAND_CONTEXTMENU == rCommandEvent.GetCommand() )
    {
        CancelDrag();
        sal_Bool bRTL = mpRulerImpl->pTextRTLItem && mpRulerImpl->pTextRTLItem->GetValue();
        if ( !mpTabs.empty() &&
             RULER_TYPE_TAB ==
             GetType( rCommandEvent.GetMousePosPixel(), &mpRulerImpl->nIdx ) &&
             mpTabs[mpRulerImpl->nIdx + TAB_GAP].nStyle < RULER_TAB_DEFAULT )
        {
            PopupMenu aMenu;
            aMenu.SetSelectHdl(LINK(this, SvxRuler, TabMenuSelect));
            VirtualDevice aDev;
            const Size aSz(RULER_TAB_WIDTH + 2, RULER_TAB_HEIGHT + 2);
            aDev.SetOutputSize(aSz);
            aDev.SetBackground(Wallpaper(Color(COL_WHITE)));
            Color aFillColor(aDev.GetSettings().GetStyleSettings().GetShadowColor());
            const Point aPt(aSz.Width() / 2, aSz.Height() / 2);

            for ( sal_uInt16 i = RULER_TAB_LEFT; i < RULER_TAB_DEFAULT; ++i )
            {
                sal_uInt16 nStyle = bRTL ? i|RULER_TAB_RTL : i;
                nStyle |= static_cast<sal_uInt16>(bHorz ? WB_HORZ : WB_VERT);
                DrawTab(&aDev, aFillColor, aPt, nStyle);
                aMenu.InsertItem(i + 1,
                                 ResId(RID_SVXSTR_RULER_START + i, DIALOG_MGR()).toString(),
                                 Image(aDev.GetBitmap(Point(), aSz), Color(COL_WHITE)));
                aMenu.CheckItem(i + 1, i == mpTabs[mpRulerImpl->nIdx + TAB_GAP].nStyle);
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
                if( bReduceMetric )
                {
                    if ( nId == FUNIT_M    ||
                         nId == FUNIT_KM   ||
                         nId == FUNIT_FOOT ||
                         nId == FUNIT_MILE )
                    {
                        aMenu.RemoveItem(i - 1);
                    }
                    else if (( nId == FUNIT_CHAR ) && !bHorz )
                    {
                        aMenu.RemoveItem(i - 1);
                    }
                    else if (( nId == FUNIT_LINE ) && bHorz )
                    {
                        aMenu.RemoveItem(i - 1);
                    }
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
    else
        nAct++; //To be able to pass on the ActDrag

    sal_Bool bConsiderHidden = !bForceDontConsiderHidden &&
                               !(nDragType & DRAG_OBJECT_ACTLINE_ONLY);

    while( nAct < mpColumnItem->Count() - 1 )
    {
        if (mpColumnItem->At(nAct).bVisible || bConsiderHidden)
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

    sal_uInt16 nLeftOffset = 1;

    sal_Bool bConsiderHidden = !bForceDontConsiderHidden &&
                               !(nDragType & DRAG_OBJECT_ACTLINE_ONLY);

    while(nAct >= nLeftOffset)
    {
        if (mpColumnItem->At(nAct - nLeftOffset).bVisible || bConsiderHidden)
            return nAct - nLeftOffset;
        else
            nLeftOffset++;
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
                lOldPos = mpBorders[nCol].nPos + mpBorders[nCol].nWidth;
                nStart = nCol + 1;
                lFences = mpBorders[nCol].nWidth;
            }

            for(sal_uInt16 i = nStart; i < mpBorders.size() - 1; ++i)
            {
                long lWidth = mpBorders[i].nPos - lOldPos;
                lColumns += lWidth;
                if(lWidth < lMinSpace)
                    lMinSpace = lWidth;
                lOldPos = mpBorders[i].nPos + mpBorders[i].nWidth;
                lFences += mpBorders[i].nWidth;
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
                lOldPos = mpBorders[nCol].nPos;
            }
            lColumns = GetMargin2()-lOldPos;
            nActCol = nCol;
            lFences = 0;
            while(nActCol < mpBorders.size() || nActCol == USHRT_MAX)
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
                    lWidth = mpBorders[nRight].nPos - lOldPos;
                    lOldPos = mpBorders[nRight].nPos;
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
            for(sal_uInt16 i = GetActRightColumn(sal_False, nCol); i < mpBorders.size();)
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
            for(sal_uInt16 i = nCol; i < mpBorders.size() - 1; i++)
            {
                lWidth += lMinFrame + mpBorders[i].nWidth;
            }
            return GetMargin2() - GetMargin1() - lWidth;
        }
    }
}

// Tab stops relative to indent (#i24363#)
void SvxRuler::SetTabsRelativeToIndent( sal_Bool bRel )
{
    mpRulerImpl->bIsTabsRelativeToIndent = bRel;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
