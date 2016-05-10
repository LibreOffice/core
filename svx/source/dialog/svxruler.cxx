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

#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
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
class RulerDebugWindow : public vcl::Window
{
    ListBox aBox;
public:
        explicit RulerDebugWindow(vcl::Window* pParent) :
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
        vcl::Window* pParent = Application::GetFocusWindow();
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
    bool bIsTableRows : 1;  // mxColumnItem contains table rows instead of columns
    //#i24363# tab stops relative to indent
    bool bIsTabsRelativeToIndent : 1; // Tab stops relative to paragraph indent?

    SvxRuler_Impl() :
        pPercBuf(nullptr), pBlockBuf(nullptr), nPercSize(0), nTotalDist(0),
        lOldWinPos(0), lMaxLeftLogic(0), lMaxRightLogic(0),
        lLastLMargin(0), lLastRMargin(0), aProtectItem(SID_RULER_PROTECT),
        pTextRTLItem(nullptr), nControlerItems(0), nIdx(0),
        nColLeftPix(0), nColRightPix(0),
        bIsTableRows(false),
        bIsTabsRelativeToIndent(true)
    {
    }

    ~SvxRuler_Impl()
    {
        nPercSize = 0; nTotalDist = 0;
        delete[] pPercBuf; delete[] pBlockBuf; pPercBuf = nullptr;
        delete pTextRTLItem;
    }
    void SetPercSize(sal_uInt16 nSize);

};

static RulerTabData ruler_tab_svx =
{
    0, // DPIScaleFactor to be set
    7, // ruler_tab_width
    6, // ruler_tab_height
    0, // ruler_tab_height2
    0, // ruler_tab_width2
    0, // ruler_tab_cwidth
    0, // ruler_tab_cwidth2
    0, // ruler_tab_cwidth3
    0, // ruler_tab_cwidth4
    0, // ruler_tab_dheight
    0, // ruler_tab_dheight2
    0, // ruler_tab_dwidth
    0, // ruler_tab_dwidth2
    0, // ruler_tab_dwidth3
    0, // ruler_tab_dwidth4
    0  // ruler_tab_textoff
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
// expects as parameter the initial value of the page and page width
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
            vcl::Window* pParent,        // StarView Parent
            vcl::Window* pWin,           // Output window: is used for conversion
                                         // logical units <-> pixels
            SvxRulerSupportFlags flags,  // Display flags, see ruler.hxx
            SfxBindings &rBindings,      // associated Bindings
            WinBits nWinStyle) :         // StarView WinBits
    Ruler(pParent, nWinStyle),
    pCtrlItems(CTRL_ITEM_COUNT),
    pEditWin(pWin),
    mxRulerImpl(new SvxRuler_Impl),
    bAppSetNullOffset(false),  // Is the 0-offset of the ruler set by the application?
    lLogicNullOffset(0),
    lAppNullOffset(LONG_MAX),
    lMinFrame(5),
    lInitialDragPos(0),
    nFlags(flags),
    nDragType(SvxRulerDragFlags::NONE),
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
    bValid(false),
    bListening(false),
    bActive(true),
    mbCoarseSnapping(false),
    mbSnapping(true)

{
    /* Constructor; Initialize data buffer; controller items are created */

    rBindings.EnterRegistrations();

    // Create Supported Items
    sal_uInt16 i = 0;

    // Page edges
    pCtrlItems[i++].reset(new SvxRulerItem(SID_RULER_LR_MIN_MAX, *this, rBindings));
    if((nWinStyle & WB_VSCROLL) == WB_VSCROLL)
    {
        bHorz = false;
        pCtrlItems[i++].reset(new SvxRulerItem(SID_ATTR_LONG_ULSPACE, *this, rBindings));
    }
    else
    {
        bHorz = true;
        pCtrlItems[i++].reset(new SvxRulerItem(SID_ATTR_LONG_LRSPACE, *this, rBindings));
    }

    // Page Position
    pCtrlItems[i++].reset(new SvxRulerItem(SID_RULER_PAGE_POS, *this, rBindings));

    if(nFlags & SvxRulerSupportFlags::TABS)
    {
        sal_uInt16 nTabStopId = bHorz ? SID_ATTR_TABSTOP : SID_ATTR_TABSTOP_VERTICAL;
        pCtrlItems[i++].reset(new SvxRulerItem(nTabStopId, *this, rBindings));
        SetExtraType(RULER_EXTRA_TAB, nDefTabType);
    }

    if(nFlags & (SvxRulerSupportFlags::PARAGRAPH_MARGINS |SvxRulerSupportFlags::PARAGRAPH_MARGINS_VERTICAL))
    {
        if(bHorz)
            pCtrlItems[i++].reset(new SvxRulerItem(SID_ATTR_PARA_LRSPACE, *this, rBindings));
        else
            pCtrlItems[i++].reset(new SvxRulerItem(SID_ATTR_PARA_LRSPACE_VERTICAL, *this, rBindings));

        mpIndents.resize(5 + INDENT_GAP);

        for(size_t nIn = 0; nIn < mpIndents.size(); nIn++)
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

    if( (nFlags & SvxRulerSupportFlags::BORDERS) ==  SvxRulerSupportFlags::BORDERS )
    {
        pCtrlItems[i++].reset(new SvxRulerItem(bHorz ? SID_RULER_BORDERS : SID_RULER_BORDERS_VERTICAL, *this, rBindings));
        pCtrlItems[i++].reset(new SvxRulerItem(bHorz ? SID_RULER_ROWS : SID_RULER_ROWS_VERTICAL, *this, rBindings));
    }

    pCtrlItems[i++].reset(new SvxRulerItem(SID_RULER_TEXT_RIGHT_TO_LEFT, *this, rBindings));

    if( (nFlags & SvxRulerSupportFlags::OBJECT) == SvxRulerSupportFlags::OBJECT )
    {
        pCtrlItems[i++].reset(new SvxRulerItem(SID_RULER_OBJECT, *this, rBindings));
        mpObjectBorders.resize(OBJECT_BORDER_COUNT);
        for(sal_uInt16 nBorder = 0; nBorder < OBJECT_BORDER_COUNT; ++nBorder)
        {
            mpObjectBorders[nBorder].nPos   = 0;
            mpObjectBorders[nBorder].nWidth = 0;
            mpObjectBorders[nBorder].nStyle = RULER_BORDER_MOVEABLE;
        }
    }

    pCtrlItems[i++].reset(new SvxRulerItem(SID_RULER_PROTECT, *this, rBindings));
    pCtrlItems[i++].reset(new SvxRulerItem(SID_RULER_BORDER_DISTANCE, *this, rBindings));
    mxRulerImpl->nControlerItems=i;

    if( (nFlags & SvxRulerSupportFlags::SET_NULLOFFSET) == SvxRulerSupportFlags::SET_NULLOFFSET )
        SetExtraType(RULER_EXTRA_NULLOFFSET);

    rBindings.LeaveRegistrations();

    ruler_tab_svx.DPIScaleFactor = pParent->GetDPIScaleFactor();
    ruler_tab_svx.height *= ruler_tab_svx.DPIScaleFactor;
    ruler_tab_svx.width  *= ruler_tab_svx.DPIScaleFactor;

}

SvxRuler::~SvxRuler()
{
    disposeOnce();
}

void SvxRuler::dispose()
{
    /* Destructor ruler; release internal buffer */
    REMOVE_DEBUG_WINDOW
    if(bListening)
        EndListening(*pBindings);

    pBindings->EnterRegistrations();

    pCtrlItems.clear();

    pBindings->LeaveRegistrations();

    pEditWin.clear();
    Ruler::dispose();
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
    aPositionLogic = rtl::math::round((aPositionLogic + aHalfTick) / aTick) * aTick;
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
        ( mxRulerImpl->aProtectItem.IsSizeProtected() ||
          mxRulerImpl->aProtectItem.IsPosProtected() ) ?
        0 : RULER_MARGIN_SIZEABLE;

    if(mxLRSpaceItem.get() && mxPagePosItem.get())
    {
        // if no initialization by default app behavior
        const long nOld = lLogicNullOffset;
        lLogicNullOffset = mxColumnItem.get() ? mxColumnItem->GetLeft(): mxLRSpaceItem->GetLeft();

        if(bAppSetNullOffset)
        {
            lAppNullOffset += lLogicNullOffset - nOld;
        }

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
        if(mxColumnItem.get() && mxColumnItem->IsTable())
            lRight = mxColumnItem->GetRight();
        else
            lRight = mxLRSpaceItem->GetRight();

        long aWidth = mxPagePosItem->GetWidth() - lRight - lLogicNullOffset + lAppNullOffset;
        long aWidthPixel = ConvertHPosPixel(aWidth);

        SetMargin2(aWidthPixel, nMarginStyle);
    }
    else if(mxULSpaceItem.get() && mxPagePosItem.get())
    {
        // relative the upper edge of the surrounding frame
        const long nOld = lLogicNullOffset;
        lLogicNullOffset = mxColumnItem.get() ? mxColumnItem->GetLeft() : mxULSpaceItem->GetUpper();

        if(bAppSetNullOffset)
        {
            lAppNullOffset += lLogicNullOffset - nOld;
        }

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

        long lLower = mxColumnItem.get() ? mxColumnItem->GetRight() : mxULSpaceItem->GetLower();
        long nMargin2 = mxPagePosItem->GetHeight() - lLower - lLogicNullOffset + lAppNullOffset;
        long nMargin2Pixel = ConvertVPosPixel(nMargin2);

        SetMargin2(nMargin2Pixel, nMarginStyle);
    }
    else
    {
        // turns off the view
        SetMargin1();
        SetMargin2();
    }

    if(mxColumnItem.get())
    {
        mxRulerImpl->nColLeftPix = (sal_uInt16) ConvertSizePixel(mxColumnItem->GetLeft());
        mxRulerImpl->nColRightPix = (sal_uInt16) ConvertSizePixel(mxColumnItem->GetRight());
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
            if (!mxParaItem.get())
                break;

            long nIndex = aSelection.nAryPos + INDENT_GAP;

            long nIndentValue = 0.0;
            if (nIndex == INDENT_LEFT_MARGIN)
                nIndentValue = mxParaItem->GetTextLeft();
            else if (nIndex == INDENT_FIRST_LINE)
                nIndentValue = mxParaItem->GetTextFirstLineOfst();
            else if (nIndex == INDENT_RIGHT_MARGIN)
                nIndentValue = mxParaItem->GetRight();

            double fValue = OutputDevice::LogicToLogic(Size(nIndentValue, 0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
            fValue = rtl::math::round(fValue / aUnitData.nTickUnit, aNoDecimalPlaces);

            SetQuickHelpText(OUString::number(fValue) + " " + sUnit);
            break;
        }
        case RULER_TYPE_BORDER:
        {
            if (mxColumnItem.get() == nullptr)
                break;

            SvxColumnItem& aColumnItem = *mxColumnItem.get();

            if (aSelection.nAryPos + 1 >= aColumnItem.Count())
                break;

            double fStart = OutputDevice::LogicToLogic(Size(aColumnItem[aSelection.nAryPos].nEnd,       0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
            fStart = rtl::math::round(fStart / aUnitData.nTickUnit, aNoDecimalPlaces);
            double fEnd   = OutputDevice::LogicToLogic(Size(aColumnItem[aSelection.nAryPos + 1].nStart, 0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
            fEnd = rtl::math::round(fEnd / aUnitData.nTickUnit, aNoDecimalPlaces);

            SetQuickHelpText(
                OUString::number(fStart) + " " + sUnit + " - " +
                OUString::number(fEnd)   + " " + sUnit );
            break;
        }
        case RULER_TYPE_MARGIN1:
        {
            long nLeft = 0.0;
            if (mxLRSpaceItem.get())
                nLeft = mxLRSpaceItem->GetLeft();
            else if (mxULSpaceItem.get())
                nLeft = mxULSpaceItem->GetUpper();
            else
                break;

            double fValue = OutputDevice::LogicToLogic(Size(nLeft, 0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
            fValue = rtl::math::round(fValue / aUnitData.nTickUnit, aNoDecimalPlaces);
            SetQuickHelpText(OUString::number(fValue) + " " + sUnit);

            break;
        }
        case RULER_TYPE_MARGIN2:
        {
            long nRight = 0.0;
            if (mxLRSpaceItem.get())
                nRight = mxLRSpaceItem->GetRight();
            else if (mxULSpaceItem.get())
                nRight = mxULSpaceItem->GetLower();
            else
                break;

            double fValue = OutputDevice::LogicToLogic(Size(nRight, 0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
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
        bValid = false;
        StartListening(*pBindings);
        bListening = true;
    }
}

void SvxRuler::UpdateFrame(const SvxLongLRSpaceItem *pItem) // new value LRSpace
{
    /* Store new value LRSpace; delete old ones if possible */
    if(bActive)
    {
        if(pItem)
            mxLRSpaceItem.reset(new SvxLongLRSpaceItem(*pItem));
        else
            mxLRSpaceItem.reset();
        StartListening_Impl();
    }
}

void SvxRuler::UpdateFrameMinMax(const SfxRectangleItem *pItem) // value for MinMax
{
    /* Set new value for MinMax; delete old ones if possible */
    if(bActive)
    {
        if(pItem)
            mxMinMaxItem.reset(new SfxRectangleItem(*pItem));
        else
            mxMinMaxItem.reset();
    }
}


void SvxRuler::UpdateFrame(const SvxLongULSpaceItem *pItem) // new value
{
    /* Update Right/bottom margin */
    if(bActive && !bHorz)
    {
        if(pItem)
            mxULSpaceItem.reset(new SvxLongULSpaceItem(*pItem));
        else
            mxULSpaceItem.reset();
        StartListening_Impl();
    }
}

void SvxRuler::Update( const SvxProtectItem* pItem )
{
    if( pItem )
        mxRulerImpl->aProtectItem = *pItem;
}

void SvxRuler::UpdateTextRTL(const SfxBoolItem* pItem)
{
    if(bActive && bHorz)
    {
        delete mxRulerImpl->pTextRTLItem;
        mxRulerImpl->pTextRTLItem = nullptr;
        if(pItem)
            mxRulerImpl->pTextRTLItem = new SfxBoolItem(*pItem);
        SetTextRTL(mxRulerImpl->pTextRTLItem && mxRulerImpl->pTextRTLItem->GetValue());
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
            mxColumnItem.reset(new SvxColumnItem(*pItem));
            mxRulerImpl->bIsTableRows = (pItem->Which() == SID_RULER_ROWS || pItem->Which() == SID_RULER_ROWS_VERTICAL);
            if(!bHorz && !mxRulerImpl->bIsTableRows)
                mxColumnItem->SetWhich(SID_RULER_BORDERS_VERTICAL);
        }
        else if(mxColumnItem.get() && mxColumnItem->Which() == nSID)
        //there are two groups of column items table/frame columns and table rows
        //both can occur in vertical or horizontal mode
        //the horizontal ruler handles the SID_RULER_BORDERS and SID_RULER_ROWS_VERTICAL
        //and the vertical handles SID_RULER_BORDERS_VERTICAL and SID_RULER_ROWS
        //if mxColumnItem is already set with one of the ids then a NULL pItem argument
        //must not delete it
        {
            mxColumnItem.reset();
            mxRulerImpl->bIsTableRows = false;
        }
        StartListening_Impl();
    }
}


void SvxRuler::UpdateColumns()
{
    /* Update column view */
    if(mxColumnItem.get() && mxColumnItem->Count() > 1)
    {
        mpBorders.resize(mxColumnItem->Count());

        sal_uInt16 nStyleFlags = RULER_BORDER_VARIABLE;

        bool bProtectColumns =
                    mxRulerImpl->aProtectItem.IsSizeProtected() ||
                    mxRulerImpl->aProtectItem.IsPosProtected();

        if( !bProtectColumns )
            nStyleFlags |= RULER_BORDER_MOVEABLE;

        if( mxColumnItem->IsTable() )
            nStyleFlags |= RULER_BORDER_TABLE;
        else if ( !bProtectColumns )
            nStyleFlags |= RULER_BORDER_SIZEABLE;

        sal_uInt16 nBorders = mxColumnItem->Count();

        if(!mxRulerImpl->bIsTableRows)
            --nBorders;

        for(sal_uInt16 i = 0; i < nBorders; ++i)
        {
            mpBorders[i].nStyle = nStyleFlags;
            if(!mxColumnItem->At(i).bVisible)
                mpBorders[i].nStyle |= RULER_STYLE_INVISIBLE;

            mpBorders[i].nPos = ConvertPosPixel(mxColumnItem->At(i).nEnd + lAppNullOffset);

            if(mxColumnItem->Count() == i + 1)
            {
                //with table rows the end of the table is contained in the
                //column item but it has no width!
                mpBorders[i].nWidth = 0;
            }
            else
            {
                mpBorders[i].nWidth = ConvertSizePixel(mxColumnItem->At(i + 1).nStart - mxColumnItem->At(i).nEnd);
            }
            mpBorders[i].nMinPos = ConvertPosPixel(mxColumnItem->At(i).nEndMin + lAppNullOffset);
            mpBorders[i].nMaxPos = ConvertPosPixel(mxColumnItem->At(i).nEndMax + lAppNullOffset);
        }
        SetBorders(mxColumnItem->Count() - 1, &mpBorders[0]);
    }
    else
    {
        SetBorders();
    }
}

void SvxRuler::UpdateObject()
{
    /* Update view of object representation */
    if(mxObjectItem.get())
    {
        DBG_ASSERT(!mpObjectBorders.empty(), "no Buffer");
        // !! to the page margin
        long nMargin = mxLRSpaceItem.get() ? mxLRSpaceItem->GetLeft() : 0;
        mpObjectBorders[0].nPos =
            ConvertPosPixel(mxObjectItem->GetStartX() -
                            nMargin + lAppNullOffset);
        mpObjectBorders[1].nPos =
            ConvertPosPixel(mxObjectItem->GetEndX() - nMargin + lAppNullOffset);
        nMargin = mxULSpaceItem.get() ? mxULSpaceItem->GetUpper() : 0;
        mpObjectBorders[2].nPos =
            ConvertPosPixel(mxObjectItem->GetStartY() -
                            nMargin + lAppNullOffset);
        mpObjectBorders[3].nPos =
            ConvertPosPixel(mxObjectItem->GetEndY() - nMargin + lAppNullOffset);

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
    if(mxParaItem.get() && mxPagePosItem.get() && !mxObjectItem.get())
    {
        bool bRTLText = mxRulerImpl->pTextRTLItem && mxRulerImpl->pTextRTLItem->GetValue();
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
            leftMargin    = nRightFrameMargin - mxParaItem->GetTextLeft() + lAppNullOffset;
            leftFirstLine = leftMargin - mxParaItem->GetTextFirstLineOfst();
            rightMargin   = nLeftFrameMargin + mxParaItem->GetRight() + lAppNullOffset;
        }
        else
        {
            leftMargin    = nLeftFrameMargin + mxParaItem->GetTextLeft() + lAppNullOffset;
            leftFirstLine = leftMargin + mxParaItem->GetTextFirstLineOfst();
            rightMargin   = nRightFrameMargin - mxParaItem->GetRight() + lAppNullOffset;
        }

        mpIndents[INDENT_LEFT_MARGIN].nPos  = ConvertHPosPixel(leftMargin);
        mpIndents[INDENT_FIRST_LINE].nPos   = ConvertHPosPixel(leftFirstLine);
        mpIndents[INDENT_RIGHT_MARGIN].nPos = ConvertHPosPixel(rightMargin);

        if( mxParaItem->IsAutoFirst() )
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
            mxParaItem.reset(new SvxLRSpaceItem(*pItem));
        else
            mxParaItem.reset();
        StartListening_Impl();
    }
}

void SvxRuler::UpdateParaBorder(const SvxLRSpaceItem * pItem )
{
    /* Border distance */
    if(bActive)
    {
        if(pItem)
            mxParaBorderItem.reset(new SvxLRSpaceItem(*pItem));
        else
            mxParaBorderItem.reset();
        StartListening_Impl();
    }
}

void SvxRuler::UpdatePage()
{
    /* Update view of position and width of page */
    if(mxPagePosItem.get())
    {
        // all objects are automatically adjusted
        if(bHorz)
        {
            SetPagePos(
                pEditWin->LogicToPixel(mxPagePosItem->GetPos()).X(),
                pEditWin->LogicToPixel(Size(mxPagePosItem->GetWidth(), 0)).
                Width());
        }
        else
        {
            SetPagePos(
                pEditWin->LogicToPixel(mxPagePosItem->GetPos()).Y(),
                pEditWin->LogicToPixel(Size(0, mxPagePosItem->GetHeight())).
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
    if( AllSettings::GetLayoutRTL() && bHorz )
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
    if(lPos != mxRulerImpl->lOldWinPos)
    {
        mxRulerImpl->lOldWinPos=lPos;
        SetWinPos(lPos);
    }
}

void SvxRuler::Update(const SvxPagePosSizeItem *pItem) // new value of page attributes
{
    /* Store new value of page attributes */
    if(bActive)
    {
        if(pItem)
            mxPagePosItem.reset(new SvxPagePosSizeItem(*pItem));
        else
            mxPagePosItem.reset();
        StartListening_Impl();
    }
}

void SvxRuler::SetDefTabDist(long inDefTabDist)  // New distance for DefaultTabs in App-Metrics
{
    if (lAppNullOffset == LONG_MAX)
        UpdateFrame(); // hack: try to get lAppNullOffset initialized
    /* New distance is set for DefaultTabs */
    lDefTabDist = inDefTabDist;
    UpdateTabs();
}

sal_uInt16 ToSvTab_Impl(SvxTabAdjust eAdj)
{
    /* Internal conversion routine between SV-Tab.-Enum and Svx */
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

    if( mxPagePosItem.get() &&
        mxParaItem.get()    &&
        mxTabStopItem.get() &&
        !mxObjectItem.get() )
    {
        // buffer for DefaultTabStop
        // Distance last Tab <-> Right paragraph margin / DefaultTabDist
        bool bRTL = mxRulerImpl->pTextRTLItem && mxRulerImpl->pTextRTLItem->GetValue();

        long nLeftFrameMargin = GetLeftFrameMargin();
        long nRightFrameMargin = GetRightFrameMargin();

        //#i24363# tab stops relative to indent
        const long nParaItemTxtLeft = mxParaItem->GetTextLeft();

        const long lParaIndent = nLeftFrameMargin + nParaItemTxtLeft;

        const long lLastTab = mxTabStopItem->Count()
                                ? ConvertHPosPixel(mxTabStopItem->At(mxTabStopItem->Count() - 1).GetTabPos())
                                : 0;
        const long lPosPixel = ConvertHPosPixel(lParaIndent) + lLastTab;
        const long lRightIndent = ConvertHPosPixel(nRightFrameMargin - mxParaItem->GetRight());

        long nDefTabDist = ConvertHPosPixel(lDefTabDist);

        if( !nDefTabDist )
            nDefTabDist = 1;

        const sal_uInt16 nDefTabBuf = lPosPixel > lRightIndent || lLastTab > lRightIndent
                    ? 0
                    : (sal_uInt16)( (lRightIndent - lPosPixel) / nDefTabDist );

        if(mxTabStopItem->Count() + TAB_GAP + nDefTabBuf > nTabBufSize)
        {
            // 10 (GAP) in stock
            nTabBufSize = mxTabStopItem->Count() + TAB_GAP + nDefTabBuf + GAP;
            mpTabs.resize(nTabBufSize);
        }

        nTabCount = 0;
        sal_uInt16 j;

        //#i24363# tab stops relative to indent
        const long lRightPixMargin = ConvertSizePixel(nRightFrameMargin - nParaItemTxtLeft );
        const long lParaIndentPix = ConvertSizePixel(lParaIndent);

        for(j = 0; j < mxTabStopItem->Count(); ++j)
        {
            const SvxTabStop* pTab = &mxTabStopItem->At(j);
            if (mxRulerImpl->bIsTabsRelativeToIndent)
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

        if(!mxTabStopItem->Count())
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
                    if(mxRulerImpl->bIsTabsRelativeToIndent)
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
            mxTabStopItem.reset(new SvxTabStopItem(*pItem));
            if(!bHorz)
                mxTabStopItem->SetWhich(SID_ATTR_TABSTOP_VERTICAL);
        }
        else
        {
            mxTabStopItem.reset();
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
            mxObjectItem.reset(new SvxObjectItem(*pItem));
        else
            mxObjectItem.reset();
        StartListening_Impl();
    }
}

void SvxRuler::SetNullOffsetLogic(long lVal) // Setting of the logic NullOffsets
{
    lAppNullOffset = lLogicNullOffset - lVal;
    bAppSetNullOffset = true;
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
    if(nFlags & SvxRulerSupportFlags::OBJECT)
        UpdateObject();
    else
        UpdateColumns();

    if(nFlags & (SvxRulerSupportFlags::PARAGRAPH_MARGINS | SvxRulerSupportFlags::PARAGRAPH_MARGINS_VERTICAL))
      UpdatePara();

    if(nFlags & SvxRulerSupportFlags::TABS)
      UpdateTabs();
}

long SvxRuler::GetPageWidth() const
{
    if (!mxPagePosItem.get())
        return 0;
    return bHorz ? mxPagePosItem->GetWidth() : mxPagePosItem->GetHeight();
}

inline long SvxRuler::GetFrameLeft() const
{
    /* Get Left margin in Pixels */
    return  bAppSetNullOffset ?
            GetMargin1() + ConvertSizePixel(lLogicNullOffset) :
            Ruler::GetNullOffset();
}

long SvxRuler::GetFirstLineIndent() const
{
    /* Get First-line indent in pixels */
    return mxParaItem.get() ? mpIndents[INDENT_FIRST_LINE].nPos : GetMargin1();
}

long SvxRuler::GetLeftIndent() const
{
    /* Get Left paragraph margin in Pixels */
    return mxParaItem.get() ? mpIndents[INDENT_LEFT_MARGIN].nPos : GetMargin1();
}

long SvxRuler::GetRightIndent() const
{
    /* Get Right paragraph margin in Pixels */
    return mxParaItem.get() ? mpIndents[INDENT_RIGHT_MARGIN].nPos : GetMargin2();
}

long SvxRuler::GetLogicRightIndent() const
{
    /* Get Right paragraph margin in Logic */
    return mxParaItem.get() ? GetRightFrameMargin() - mxParaItem->GetRight() : GetRightFrameMargin();
}

// Left margin in App values, is either the margin (= 0)  or the left edge of
// the column that is set in the column attribute as current column.
long SvxRuler::GetLeftFrameMargin() const
{
    // #126721# for some unknown reason the current column is set to 0xffff
    DBG_ASSERT(!mxColumnItem.get() || mxColumnItem->GetActColumn() < mxColumnItem->Count(),
                    "issue #126721# - invalid current column!");
    long nLeft = 0;
    if (mxColumnItem.get() &&
        mxColumnItem->Count() &&
        mxColumnItem->IsConsistent())
    {
        nLeft = mxColumnItem->GetActiveColumnDescription().nStart;
    }

    return nLeft;
}

inline long SvxRuler::GetLeftMin() const
{
    DBG_ASSERT(mxMinMaxItem.get(), "no MinMax value set");
    if (mxMinMaxItem.get())
    {
        if (bHorz)
            return mxMinMaxItem->GetValue().Left();
        else
            return mxMinMaxItem->GetValue().Top();
    }
    return 0;
}

inline long SvxRuler::GetRightMax() const
{
    DBG_ASSERT(mxMinMaxItem.get(), "no MinMax value set");
    if (mxMinMaxItem.get())
    {
        if (bHorz)
            return mxMinMaxItem->GetValue().Right();
        else
            return mxMinMaxItem->GetValue().Bottom();
    }
    return 0;
}


long SvxRuler::GetRightFrameMargin() const
{
    /* Get right frame margin (in logical units) */
    if (mxColumnItem.get())
    {
        if (!IsActLastColumn(true))
        {
            return mxColumnItem->At(GetActRightColumn(true)).nEnd;
        }
    }

    long lResult = lLogicNullOffset;

    // If possible deduct right table entry
    if(mxColumnItem.get() && mxColumnItem->IsTable())
        lResult += mxColumnItem->GetRight();
    else if(bHorz && mxLRSpaceItem.get())
        lResult += mxLRSpaceItem->GetRight();
    else if(!bHorz && mxULSpaceItem.get())
        lResult += mxULSpaceItem->GetLower();

    if(bHorz)
        lResult = mxPagePosItem->GetWidth() - lResult;
    else
        lResult = mxPagePosItem->GetHeight() - lResult;

    return lResult;
}

#define NEG_FLAG ( (nFlags & SvxRulerSupportFlags::NEGATIVE_MARGINS) == \
                   SvxRulerSupportFlags::NEGATIVE_MARGINS )
#define TAB_FLAG ( mxColumnItem.get() && mxColumnItem->IsTable() )

long SvxRuler::GetCorrectedDragPos( bool bLeft, bool bRight )
{
    /*
        Corrects the position within the calculated limits. The limit values are in
        pixels relative to the page edge.
    */

    const long lNullPix = Ruler::GetNullOffset();
    long lDragPos = GetDragPos() + lNullPix;
ADD_DEBUG_TEXT("lDragPos: ", OUString::number(lDragPos))
     bool bHoriRows = bHorz && mxRulerImpl->bIsTableRows;
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
    long aDragPosition = GetCorrectedDragPos( !TAB_FLAG || !NEG_FLAG );

    aDragPosition = MakePositionSticky(aDragPosition, GetRightFrameMargin(), false);

    // Check if position changed
    if (aDragPosition == 0)
        return;

    DrawLine_Impl(lTabPos, ( TAB_FLAG && NEG_FLAG ) ? 3 : 7, bHorz);
    if (mxColumnItem.get() && (nDragType & SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL))
        DragBorders();
    AdjustMargin1(aDragPosition);
}

void SvxRuler::AdjustMargin1(long lInputDiff)
{
    const long nOld = bAppSetNullOffset? GetMargin1(): GetNullOffset();
    const long lDragPos = lInputDiff;

    bool bProtectColumns =
        mxRulerImpl->aProtectItem.IsSizeProtected() ||
        mxRulerImpl->aProtectItem.IsPosProtected();

    const sal_uInt16 nMarginStyle =
        bProtectColumns ? 0 : RULER_MARGIN_SIZEABLE;

    if(!bAppSetNullOffset)
    {
        long lDiff = lDragPos;
        SetNullOffset(nOld + lDiff);
        if (!mxColumnItem.get() || !(nDragType & SvxRulerDragFlags::OBJECT_SIZE_LINEAR))
        {
            SetMargin2( GetMargin2() - lDiff, nMarginStyle );

            if (!mxColumnItem.get() && !mxObjectItem.get() && mxParaItem.get())
            {
                // Right indent of the old position
                mpIndents[INDENT_RIGHT_MARGIN].nPos -= lDiff;
                SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
            }
            if(mxObjectItem.get())
            {
                mpObjectBorders[GetObjectBordersOff(0)].nPos -= lDiff;
                mpObjectBorders[GetObjectBordersOff(1)].nPos -= lDiff;
                SetBorders(2, &mpObjectBorders[0] + GetObjectBordersOff(0));
            }
            if(mxColumnItem.get())
            {
                for(sal_uInt16 i = 0; i < mxColumnItem->Count()-1; ++i)
                    mpBorders[i].nPos -= lDiff;
                SetBorders(mxColumnItem->Count()-1, &mpBorders[0]);
                if(mxColumnItem->IsFirstAct())
                {
                    // Right indent of the old position
                    if(mxParaItem.get())
                    {
                        mpIndents[INDENT_RIGHT_MARGIN].nPos -= lDiff;
                        SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
                    }
                }
                else
                {
                    if(mxParaItem.get())
                    {
                        mpIndents[INDENT_FIRST_LINE].nPos -= lDiff;
                        mpIndents[INDENT_LEFT_MARGIN].nPos -= lDiff;
                        mpIndents[INDENT_RIGHT_MARGIN].nPos -= lDiff;
                        SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
                    }
                }
                if(mxTabStopItem.get() && (nDragType & SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL)
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

        if (!mxColumnItem.get() || !(nDragType & (SvxRulerDragFlags::OBJECT_SIZE_LINEAR | SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL)))
        {
            if (!mxColumnItem.get() && !mxObjectItem.get() && mxParaItem.get())
            {
                // Left indent of the old position
                mpIndents[INDENT_FIRST_LINE].nPos += lDiff;
                mpIndents[INDENT_LEFT_MARGIN].nPos += lDiff;
                SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
            }

            if (mxColumnItem.get())
            {
                for(sal_uInt16 i = 0; i < mxColumnItem->Count() - 1; ++i)
                    mpBorders[i].nPos += lDiff;
                SetBorders(mxColumnItem->Count() - 1, &mpBorders[0]);
                if (mxColumnItem->IsFirstAct())
                {
                    // Left indent of the old position
                    if(mxParaItem.get())
                    {
                        mpIndents[INDENT_FIRST_LINE].nPos += lDiff;
                        mpIndents[INDENT_LEFT_MARGIN].nPos += lDiff;
                        SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
                    }
                }
                else
                {
                    if(mxParaItem.get())
                    {
                        mpIndents[INDENT_FIRST_LINE].nPos += lDiff;
                        mpIndents[INDENT_LEFT_MARGIN].nPos += lDiff;
                        mpIndents[INDENT_RIGHT_MARGIN].nPos += lDiff;
                        SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
                    }
                }
            }
            if(mxTabStopItem.get())
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
    long aDragPosition = GetCorrectedDragPos( true, !TAB_FLAG || !NEG_FLAG);
    aDragPosition = MakePositionSticky(aDragPosition, GetLeftFrameMargin(), false);
    long lDiff = aDragPosition - GetMargin2();

    // Check if position changed
    if (lDiff == 0)
        return;

    if( mxRulerImpl->bIsTableRows &&
        !bHorz &&
        mxColumnItem.get() &&
        (nDragType & SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL))
    {
        DragBorders();
    }

    bool bProtectColumns =
        mxRulerImpl->aProtectItem.IsSizeProtected() ||
        mxRulerImpl->aProtectItem.IsPosProtected();

    const sal_uInt16 nMarginStyle = bProtectColumns ? 0 : RULER_MARGIN_SIZEABLE;

    SetMargin2( aDragPosition, nMarginStyle );

    // Right indent of the old position
    if((!mxColumnItem.get() || IsActLastColumn()) && mxParaItem.get())
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

    bool bRTL = mxRulerImpl->pTextRTLItem && mxRulerImpl->pTextRTLItem->GetValue();

    if(nIndex == INDENT_RIGHT_MARGIN)
        aDragPosition = MakePositionSticky(aDragPosition, bRTL ? GetLeftFrameMargin() : GetRightFrameMargin());
    else
        aDragPosition = MakePositionSticky(aDragPosition, bRTL ? GetRightFrameMargin() : GetLeftFrameMargin());

    const long lDiff = mpIndents[nIndex].nPos - aDragPosition;

    // Check if position changed
    if (lDiff == 0)
        return;

    if((nIndex == INDENT_FIRST_LINE || nIndex == INDENT_LEFT_MARGIN )  &&
        !(nDragType & SvxRulerDragFlags::OBJECT_LEFT_INDENT_ONLY))
    {
        mpIndents[INDENT_FIRST_LINE].nPos -= lDiff;
    }

    mpIndents[nIndex].nPos = aDragPosition;

    SetIndents(INDENT_COUNT, &mpIndents[0] + INDENT_GAP);
    DrawLine_Impl(lTabPos, 1, bHorz);
}

void SvxRuler::DrawLine_Impl(long& lTabPosition, int nNew, bool bHorizontal)
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
            if(mxPagePosItem.get())
                lTabPosition += mxPagePosItem->GetPos().X();
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
            if(mxPagePosItem.get())
                lTabPosition += mxPagePosItem->GetPos().Y();
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
    long aDragPosition = GetCorrectedDragPos(true, false);
    aDragPosition = MakePositionSticky(aDragPosition, GetLeftFrameMargin());

    sal_uInt16 nIdx = GetDragAryPos() + TAB_GAP;
    long nDiff = aDragPosition - mpTabs[nIdx].nPos;
    if (nDiff == 0)
        return;

    DrawLine_Impl(lTabPos, 7, bHorz);

    if(nDragType & SvxRulerDragFlags::OBJECT_SIZE_LINEAR)
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
    else if(nDragType & SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL)
    {
        mxRulerImpl->nTotalDist -= nDiff;
        mpTabs[nIdx].nPos = aDragPosition;
        for(sal_uInt16 i = nIdx+1; i < nTabCount; ++i)
        {
            if(mpTabs[i].nStyle & RULER_TAB_DEFAULT)
                // can be canceled at the DefaultTabs
                break;
            long nDelta = mxRulerImpl->nTotalDist * mxRulerImpl->pPercBuf[i];
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

void SvxRuler::SetActive(bool bOn)
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
            for(sal_uInt16 i=0;i<mxRulerImpl->nControlerItems;i++)
                pCtrlItems[i]->ReBind();
        else
            for(sal_uInt16 j=0;j<mxRulerImpl->nControlerItems;j++)
                pCtrlItems[j]->UnBind();
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
            SAL_FALLTHROUGH;
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
    bool bLeftIndentsCorrected  = false;
    bool bRightIndentsCorrected = false;
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
ADD_DEBUG_TEXT("lLastLMargin: ", OUString::number(mxRulerImpl->lLastLMargin))
            if(GetDragType() == RULER_TYPE_BORDER)
                lDiff = lPos - nDragOffset - mpBorders[nIndex].nPos;
            else
                lDiff = GetDragType() == RULER_TYPE_MARGIN1 ? lPos - mxRulerImpl->lLastLMargin : lPos - mxRulerImpl->lLastRMargin;

            if(nDragType & SvxRulerDragFlags::OBJECT_SIZE_LINEAR)
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
                        bRightIndentsCorrected = true;
                    }
                    // LAR, EZE update the column
                    else if(i == GetActLeftColumn())
                    {
                        UpdateParaContents_Impl(mpBorders[i].nPos - l, MOVE_LEFT);
                        bLeftIndentsCorrected = true;
                    }
                }
            }
            else if(nDragType & SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL)
            {
                int nLimit;
                long lLeft;
                int nStartLimit = mpBorders.size() - 2;
                switch(GetDragType())
                {
                default: ;//prevent warning
                    OSL_FAIL("svx::SvxRuler::DragBorders(), unknown drag type!" );
                    SAL_FALLTHROUGH;
                case RULER_TYPE_BORDER:
                    if(mxRulerImpl->bIsTableRows)
                    {
                        mpBorders[nIndex].nPos += lDiff;
                        if(bHorz)
                        {
                            lLeft = mpBorders[nIndex].nPos;
                            mxRulerImpl->nTotalDist -= lDiff;
                            nLimit = nIndex + 1;
                        }
                        else
                        {
                            lLeft = 0;
                            nStartLimit = nIndex - 1;
                            mxRulerImpl->nTotalDist += lDiff;
                            nLimit = 0;
                        }
                    }
                    else
                    {
                        nLimit = nIndex + 1;
                        mpBorders[nIndex].nPos += lDiff;
                        lLeft = mpBorders[nIndex].nPos;
                        mxRulerImpl->nTotalDist -= lDiff;
                    }
                break;
                case RULER_TYPE_MARGIN1:
                    nLimit = 0;
                    lLeft = mxRulerImpl->lLastLMargin + lDiff;
                    mxRulerImpl->nTotalDist -= lDiff;
                break;
                case RULER_TYPE_MARGIN2:
                    nLimit = 0;
                    lLeft= 0;
                    nStartLimit = mpBorders.size() - 2;
                    mxRulerImpl->nTotalDist += lDiff;
                break;
                }

                for(int i  = nStartLimit; i >= nLimit; --i)
                {

                    long l = mpBorders[i].nPos;
                    mpBorders[i].nPos =
                        lLeft +
                        (mxRulerImpl->nTotalDist * mxRulerImpl->pPercBuf[i]) / 1000 +
                        mxRulerImpl->pBlockBuf[i];

                    // RR update the column
                    if(!mxRulerImpl->bIsTableRows)
                    {
                        if(i == GetActRightColumn())
                        {
                            UpdateParaContents_Impl(mpBorders[i].nPos - l, MOVE_RIGHT);
                            bRightIndentsCorrected = true;
                        }
                        // LAR, EZE update the column
                        else if(i == GetActLeftColumn())
                        {
                            UpdateParaContents_Impl(mpBorders[i].nPos - l, MOVE_LEFT);
                            bLeftIndentsCorrected = true;
                        }
                    }
                }
                if(mxRulerImpl->bIsTableRows)
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
                        for(int i  = mxColumnItem->Count() - 1; i > nIndex; --i)
                            mpBorders[i].nPos += lDiff;
                        SetMargin2( GetMargin2() + lDiff, 0 );
                    }
                }
            }
            else if(mxRulerImpl->bIsTableRows)
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
       !mxRulerImpl->bIsTableRows)
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
    SetBorders(mxColumnItem->Count() - 1, &mpBorders[0]);
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
    const SfxPoolItem* pItem = nullptr;
    sal_uInt16 nId = SID_ATTR_LONG_LRSPACE;

    if(bHorz)
    {
        const long lOldNull = lLogicNullOffset;
        if(mxRulerImpl->lMaxLeftLogic != -1 && nMaxLeft == GetMargin1() + Ruler::GetNullOffset())
        {
            lLogicNullOffset = mxRulerImpl->lMaxLeftLogic;
            mxLRSpaceItem->SetLeft(lLogicNullOffset);
        }
        else
        {
            lLogicNullOffset = ConvertHPosLogic(GetFrameLeft()) - lAppNullOffset;
            mxLRSpaceItem->SetLeft(PixelHAdjust(lLogicNullOffset, mxLRSpaceItem->GetLeft()));
        }

        if(bAppSetNullOffset)
        {
            lAppNullOffset += lLogicNullOffset - lOldNull;
        }

        long nRight;
        if(mxRulerImpl->lMaxRightLogic != -1
           && nMaxRight == GetMargin2() + Ruler::GetNullOffset())
        {
            nRight = GetPageWidth() - mxRulerImpl->lMaxRightLogic;
        }
        else
        {
            nRight = std::max((long)0,
                            mxPagePosItem->GetWidth() - mxLRSpaceItem->GetLeft() -
                                (ConvertHPosLogic(GetMargin2()) - lAppNullOffset));

            nRight = PixelHAdjust( nRight, mxLRSpaceItem->GetRight());
        }
        mxLRSpaceItem->SetRight(nRight);

        pItem = mxLRSpaceItem.get();

#ifdef DEBUGLIN
        Debug_Impl(pEditWin, *mxLRSpaceItem);
#endif // DEBUGLIN

    }
    else
    {
        const long lOldNull = lLogicNullOffset;
        mxULSpaceItem->SetUpper(
            PixelVAdjust(
                lLogicNullOffset =
                ConvertVPosLogic(GetFrameLeft()) -
                lAppNullOffset, mxULSpaceItem->GetUpper()));
        if(bAppSetNullOffset)
        {
            lAppNullOffset += lLogicNullOffset - lOldNull;
        }
        mxULSpaceItem->SetLower(
            PixelVAdjust(
                std::max((long)0, mxPagePosItem->GetHeight() -
                    mxULSpaceItem->GetUpper() -
                    (ConvertVPosLogic(GetMargin2()) -
                     lAppNullOffset)), mxULSpaceItem->GetLower()));
        pItem = mxULSpaceItem.get();
        nId = SID_ATTR_LONG_ULSPACE;

#ifdef DEBUGLIN
        Debug_Impl(pEditWin,*mxULSpaceItem.get());
#endif // DEBUGLIN

    }
    pBindings->GetDispatcher()->Execute( nId, SfxCallMode::RECORD, pItem, 0L );
    if(mxTabStopItem.get())
        UpdateTabs();
}

long SvxRuler::RoundToCurrentMapMode(long lValue) const
{
    RulerUnitData aUnitData = GetCurrentRulerUnit();
    double aRoundingFactor = aUnitData.nTickUnit / aUnitData.nTick1;

    long lNewValue = OutputDevice::LogicToLogic(Size(lValue, 0), pEditWin->GetMapMode(), GetCurrentMapMode()).Width();
    lNewValue = (rtl::math::round(lNewValue / (double) aUnitData.nTickUnit * aRoundingFactor) / aRoundingFactor) * aUnitData.nTickUnit;
    return OutputDevice::LogicToLogic(Size(lNewValue, 0), GetCurrentMapMode(), pEditWin->GetMapMode()).Width();
}

void SvxRuler::ApplyIndents()
{
    /* Applying paragraph settings; changed by dragging. */

    long nLeftFrameMargin  = GetLeftFrameMargin();

    bool bRTL = mxRulerImpl->pTextRTLItem && mxRulerImpl->pTextRTLItem->GetValue();

    long nNewTxtLeft;
    long nNewFirstLineOffset;
    long nNewRight;

    long nFirstLine    = ConvertPosLogic(mpIndents[INDENT_FIRST_LINE].nPos);
    long nLeftMargin   = ConvertPosLogic(mpIndents[INDENT_LEFT_MARGIN].nPos);
    long nRightMargin  = ConvertPosLogic(mpIndents[INDENT_RIGHT_MARGIN].nPos);

    if(mxColumnItem.get() && ((bRTL && !IsActLastColumn(true)) || (!bRTL && !IsActFirstColumn(true))))
    {
        if(bRTL)
        {
            long nRightColumn  = GetActRightColumn(true);
            long nRightBorder  = ConvertPosLogic(mpBorders[nRightColumn].nPos);
            nNewTxtLeft = nRightBorder - nLeftMargin - lAppNullOffset;
        }
        else
        {
            long nLeftColumn = GetActLeftColumn(true);
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

    if(mxColumnItem.get() && ((!bRTL && !IsActLastColumn(true)) || (bRTL && !IsActFirstColumn(true))))
    {
        if(bRTL)
        {
            long nLeftColumn = GetActLeftColumn(true);
            long nLeftBorder = ConvertPosLogic(mpBorders[nLeftColumn].nPos + mpBorders[nLeftColumn].nWidth);
            nNewRight = nRightMargin - nLeftBorder - lAppNullOffset;
        }
        else
        {
            long nRightColumn  = GetActRightColumn(true);
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

    mxParaItem->SetTextFirstLineOfst(sal::static_int_cast<short>(nNewFirstLineOffset));
    mxParaItem->SetTextLeft(nNewTxtLeft);
    mxParaItem->SetRight(nNewRight);

    sal_uInt16 nParagraphId  = bHorz ? SID_ATTR_PARA_LRSPACE : SID_ATTR_PARA_LRSPACE_VERTICAL;
    pBindings->GetDispatcher()->Execute( nParagraphId, SfxCallMode::RECORD, mxParaItem.get(), 0L );
    UpdateTabs();
}

void SvxRuler::ApplyTabs()
{
    /* Apply tab settings, changed by dragging. */
    bool bRTL = mxRulerImpl->pTextRTLItem && mxRulerImpl->pTextRTLItem->GetValue();
    const sal_uInt16 nCoreIdx = GetDragAryPos();
    if(IsDragDelete())
    {
        mxTabStopItem->Remove(nCoreIdx);
    }
    else if(SvxRulerDragFlags::OBJECT_SIZE_LINEAR & nDragType ||
            SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL & nDragType)
    {
        SvxTabStopItem *pItem = new SvxTabStopItem(mxTabStopItem->Which());
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
            pItem->Insert(mxTabStopItem->At(j));
        }
        for(; j < mxTabStopItem->Count(); ++j)
        {
            SvxTabStop aTabStop = mxTabStopItem->At(j);
            aTabStop.GetTabPos() = PixelHAdjust(
                ConvertHPosLogic(
                    mpTabs[j + TAB_GAP].nPos - GetLeftIndent()) - lAppNullOffset,
                aTabStop.GetTabPos());
            pItem->Insert(aTabStop);
        }
        mxTabStopItem.reset(pItem);
    }
    else if( mxTabStopItem->Count() == 0 )
        return;
    else
    {
        SvxTabStop aTabStop = mxTabStopItem->At(nCoreIdx);
        if( mxRulerImpl->lMaxRightLogic != -1 &&
            mpTabs[nCoreIdx + TAB_GAP].nPos + Ruler::GetNullOffset() == nMaxRight )
        {
            aTabStop.GetTabPos() = mxRulerImpl->lMaxRightLogic - lLogicNullOffset;
        }
        else
        {
            if(bRTL)
            {
                //#i24363# tab stops relative to indent
                const long nTmpLeftIndent = mxRulerImpl->bIsTabsRelativeToIndent ?
                                            GetLeftIndent() :
                                            ConvertHPosPixel( GetRightFrameMargin() + lAppNullOffset );

                long nNewPosition = ConvertHPosLogic(nTmpLeftIndent - mpTabs[nCoreIdx + TAB_GAP].nPos);
                aTabStop.GetTabPos() = PixelHAdjust(nNewPosition - lAppNullOffset, aTabStop.GetTabPos());
            }
            else
            {
                //#i24363# tab stops relative to indent
                const long nTmpLeftIndent = mxRulerImpl->bIsTabsRelativeToIndent ?
                                            GetLeftIndent() : 0;

                long nNewPosition = ConvertHPosLogic(mpTabs[nCoreIdx + TAB_GAP].nPos - nTmpLeftIndent);
                aTabStop.GetTabPos() = PixelHAdjust(nNewPosition - lAppNullOffset, aTabStop.GetTabPos());
            }
        }
        mxTabStopItem->Remove(nCoreIdx);
        mxTabStopItem->Insert(aTabStop);
    }
    sal_uInt16 nTabStopId = bHorz ? SID_ATTR_TABSTOP : SID_ATTR_TABSTOP_VERTICAL;
    pBindings->GetDispatcher()->Execute( nTabStopId, SfxCallMode::RECORD, mxTabStopItem.get(), 0L );
    UpdateTabs();
}

void SvxRuler::ApplyBorders()
{
    /* Applying (table) column settings; changed by dragging. */
    if(mxColumnItem->IsTable())
    {
        long lValue = GetFrameLeft();
        if(lValue != mxRulerImpl->nColLeftPix)
        {
            long nLeft = PixelHAdjust(
                            ConvertHPosLogic(lValue) -
                                lAppNullOffset,
                            mxColumnItem->GetLeft());
            mxColumnItem->SetLeft(nLeft);
        }

        lValue = GetMargin2();

        if(lValue != mxRulerImpl->nColRightPix)
        {
            long nWidthOrHeight = bHorz ? mxPagePosItem->GetWidth() : mxPagePosItem->GetHeight();
            long nRight = PixelHAdjust(
                            nWidthOrHeight -
                                mxColumnItem->GetLeft() -
                                ConvertHPosLogic(lValue) -
                                lAppNullOffset,
                            mxColumnItem->GetRight() );
            mxColumnItem->SetRight(nRight);
        }
    }

    for(sal_uInt16 i = 0; i < mxColumnItem->Count() - 1; ++i)
    {
        long& nEnd = mxColumnItem->At(i).nEnd;
        nEnd = PixelHAdjust(
                ConvertPosLogic(mpBorders[i].nPos),
                mxColumnItem->At(i).nEnd);
        long& nStart = mxColumnItem->At(i + 1).nStart;
        nStart = PixelHAdjust(
                    ConvertSizeLogic(mpBorders[i].nPos +
                        mpBorders[i].nWidth) -
                        lAppNullOffset,
                    mxColumnItem->At(i + 1).nStart);
        // It may be that, due to the PixelHAdjust readjustment to old values,
        // the width becomes  < 0. This we readjust.
        if( nEnd > nStart )
            nStart = nEnd;
    }

#ifdef DEBUGLIN
        Debug_Impl(pEditWin,*mxColumnItem.get());
#endif // DEBUGLIN

    SfxBoolItem aFlag(SID_RULER_ACT_LINE_ONLY,
                      bool(nDragType & SvxRulerDragFlags::OBJECT_ACTLINE_ONLY));

    sal_uInt16 nColId = mxRulerImpl->bIsTableRows ? (bHorz ? SID_RULER_ROWS : SID_RULER_ROWS_VERTICAL) :
                            (bHorz ? SID_RULER_BORDERS : SID_RULER_BORDERS_VERTICAL);

    pBindings->GetDispatcher()->Execute( nColId, SfxCallMode::RECORD, mxColumnItem.get(), &aFlag, 0L );
}

void SvxRuler::ApplyObject()
{
    /* Applying object settings, changed by dragging. */

    // to the page margin
    long nMargin = mxLRSpaceItem.get() ? mxLRSpaceItem->GetLeft() : 0;
    long nStartX = PixelAdjust(
                    ConvertPosLogic(mpObjectBorders[0].nPos) +
                        nMargin -
                        lAppNullOffset,
                    mxObjectItem->GetStartX());
    mxObjectItem->SetStartX(nStartX);

    long nEndX = PixelAdjust(
                    ConvertPosLogic(mpObjectBorders[1].nPos) +
                        nMargin -
                        lAppNullOffset,
                    mxObjectItem->GetEndX());
    mxObjectItem->SetEndX(nEndX);

    nMargin = mxULSpaceItem.get() ? mxULSpaceItem->GetUpper() : 0;
    long nStartY = PixelAdjust(
                    ConvertPosLogic(mpObjectBorders[2].nPos) +
                        nMargin -
                        lAppNullOffset,
                    mxObjectItem->GetStartY());
    mxObjectItem->SetStartY(nStartY);

    long nEndY = PixelAdjust(
                    ConvertPosLogic(mpObjectBorders[3].nPos) +
                        nMargin -
                        lAppNullOffset,
                    mxObjectItem->GetEndY());
    mxObjectItem->SetEndY(nEndY);

    pBindings->GetDispatcher()->Execute(SID_RULER_OBJECT, SfxCallMode::RECORD, mxObjectItem.get(), 0L);
}

void SvxRuler::PrepareProportional_Impl(RulerType eType)
{
    /*
       Preparation proportional dragging, and it is calculated based on the
       proportional share of the total width in parts per thousand.
    */
    mxRulerImpl->nTotalDist = GetMargin2();
    switch((int)eType)
    {
      case RULER_TYPE_MARGIN2:
      case RULER_TYPE_MARGIN1:
      case RULER_TYPE_BORDER:
        {
            DBG_ASSERT(mxColumnItem.get(), "no ColumnItem");

            mxRulerImpl->SetPercSize(mxColumnItem->Count());

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
                if(mxRulerImpl->bIsTableRows &&!bHorz)
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
            if(mxRulerImpl->bIsTableRows && RULER_TYPE_BORDER == eType)
            {
                sal_uInt16 nStartBorder;
                sal_uInt16 nEndBorder;
                if(bHorz)
                {
                    nStartBorder = nIdx + 1;
                    nEndBorder = mxColumnItem->Count() - 1;
                }
                else
                {
                    nStartBorder = 0;
                    nEndBorder = nIdx;
                }

                lWidth = mpBorders[nIdx].nPos;
                if(bHorz)
                    lWidth = GetMargin2() - lWidth;
                mxRulerImpl->nTotalDist = lWidth;
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
                    mxRulerImpl->pPercBuf[i] = (sal_uInt16)((lActWidth * 1000)
                                                    / mxRulerImpl->nTotalDist);
                    mxRulerImpl->pBlockBuf[i] = (sal_uInt16)lActBorderSum;
                    lActBorderSum += mpBorders[i].nWidth;
                }
            }
            else
            {
                lPos = lOrigLPos;
                for(sal_uInt16 ii = nStart; ii < mxColumnItem->Count() - 1; ++ii)
                {
                    lWidth += mpBorders[ii].nPos - lPos;
                    lPos = mpBorders[ii].nPos + mpBorders[ii].nWidth;
                }

                lWidth += GetMargin2() - lPos;
                mxRulerImpl->nTotalDist = lWidth;
                lPos = lOrigLPos;

                for(sal_uInt16 i = nStart; i < mxColumnItem->Count() - 1; ++i)
                {
                    lActWidth += mpBorders[i].nPos - lPos;
                    lPos = mpBorders[i].nPos + mpBorders[i].nWidth;
                    mxRulerImpl->pPercBuf[i] = (sal_uInt16)((lActWidth * 1000)
                                                    / mxRulerImpl->nTotalDist);
                    mxRulerImpl->pBlockBuf[i] = (sal_uInt16)lActBorderSum;
                    lActBorderSum += mpBorders[i].nWidth;
                }
            }
        }
        break;
        case RULER_TYPE_TAB:
        {
            const sal_uInt16 nIdx = GetDragAryPos()+TAB_GAP;
            mxRulerImpl->nTotalDist -= mpTabs[nIdx].nPos;
            mxRulerImpl->SetPercSize(nTabCount);
            for(sal_uInt16 n=0;n<=nIdx;mxRulerImpl->pPercBuf[n++]=0) ;
            for(sal_uInt16 i = nIdx+1; i < nTabCount; ++i)
            {
                const long nDelta = mpTabs[i].nPos - mpTabs[nIdx].nPos;
                mxRulerImpl->pPercBuf[i] = (sal_uInt16)((nDelta * 1000) / mxRulerImpl->nTotalDist);
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
    if(mxRulerImpl->bIsTableRows)
    {
        //rows can only be moved in one way, additionally current column is possible
        if(nModifier == KEY_SHIFT)
            nModifier = 0;
    }

    switch(nModifier)
    {
        case KEY_SHIFT:
            nDragType = SvxRulerDragFlags::OBJECT_SIZE_LINEAR;
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
            nDragType = SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL;
            if( RULER_TYPE_TAB == eType ||
                ( ( RULER_TYPE_BORDER == eType  ||
                    RULER_TYPE_MARGIN1 == eType ||
                    RULER_TYPE_MARGIN2 == eType ) &&
                mxColumnItem.get() ) )
            {
                PrepareProportional_Impl(eType);
            }
        }
        break;
        case KEY_MOD1 | KEY_SHIFT:
            if( GetDragType() != RULER_TYPE_MARGIN1 &&
                GetDragType() != RULER_TYPE_MARGIN2 )
            {
                nDragType = SvxRulerDragFlags::OBJECT_ACTLINE_ONLY;
            }
        break;
    }
}

void SvxRuler::Click()
{
    /* Override handler SV; sets Tab per dispatcher call */
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
    bool bRTL = mxRulerImpl->pTextRTLItem && mxRulerImpl->pTextRTLItem->GetValue();
    if(mxTabStopItem.get() &&
       (nFlags & SvxRulerSupportFlags::TABS) == SvxRulerSupportFlags::TABS)
    {
        bool bContentProtected = mxRulerImpl->aProtectItem.IsContentProtected();
        if( bContentProtected ) return;
        const long lPos = GetClickPos();
        if((bRTL && lPos < std::min(GetFirstLineIndent(), GetLeftIndent()) && lPos > GetRightIndent()) ||
            (!bRTL && lPos > std::min(GetFirstLineIndent(), GetLeftIndent()) && lPos < GetRightIndent()))
        {
            //convert position in left-to-right text
            long nTabPos;
    //#i24363# tab stops relative to indent
            if(bRTL)
                nTabPos = ( mxRulerImpl->bIsTabsRelativeToIndent ?
                            GetLeftIndent() :
                            ConvertHPosPixel( GetRightFrameMargin() + lAppNullOffset ) ) -
                          lPos;
            else
                nTabPos = lPos -
                          ( mxRulerImpl->bIsTabsRelativeToIndent ?
                            GetLeftIndent() :
                            0 );

            SvxTabStop aTabStop(ConvertHPosLogic(nTabPos),
                                ToAttrTab_Impl(nDefTabType));
            mxTabStopItem->Insert(aTabStop);
            UpdateTabs();
        }
    }
}

bool SvxRuler::CalcLimits ( long& nMax1,    // minimum value to be set
                            long& nMax2,    // minimum value to be set
                            bool )
{
    /*
       Default implementation of the virtual function; the application can be
       overridden to implement customized limits. The values are based on the page.
    */
    nMax1 = LONG_MIN;
    nMax2 = LONG_MAX;
    return false;
}

void SvxRuler::CalcMinMax()
{
    /*
       Calculates the limits for dragging; which are in pixels relative to the
       page edge
    */
    bool bRTL = mxRulerImpl->pTextRTLItem && mxRulerImpl->pTextRTLItem->GetValue();
    const long lNullPix = ConvertPosPixel(lLogicNullOffset);
    mxRulerImpl->lMaxLeftLogic=mxRulerImpl->lMaxRightLogic=-1;
    switch(GetDragType())
    {
        case RULER_TYPE_MARGIN1:
        {        // left edge of the surrounding Frame
            // DragPos - NOf between left - right
            mxRulerImpl->lMaxLeftLogic = GetLeftMin();
            nMaxLeft=ConvertSizePixel(mxRulerImpl->lMaxLeftLogic);

            if (!mxColumnItem.get() || mxColumnItem->Count() == 1 )
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
            else if(mxRulerImpl->bIsTableRows)
            {
                //top border is not moveable when table rows are displayed
                // protection of content means the margin is not moveable
                // - it's just a page break inside of a cell
                if(bHorz && !mxRulerImpl->aProtectItem.IsContentProtected())
                {
                    nMaxLeft = mpBorders[0].nMinPos + lNullPix;
                    if(nDragType & SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL)
                        nMaxRight = GetRightIndent() + lNullPix -
                                (mxColumnItem->Count() - 1 ) * lMinFrame;
                    else
                        nMaxRight = mpBorders[0].nPos - lMinFrame + lNullPix;
                }
                else
                    nMaxLeft = nMaxRight = lNullPix;
            }
            else
            {
                if (nDragType & SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL)
                {
                    nMaxRight=lNullPix+CalcPropMaxRight();
                }
                else if (nDragType & SvxRulerDragFlags::OBJECT_SIZE_LINEAR)
                {
                    nMaxRight = ConvertPosPixel(
                        GetPageWidth() - (
                            (mxColumnItem->IsTable() && mxLRSpaceItem.get())
                            ? mxLRSpaceItem->GetRight() : 0))
                            - GetMargin2() + GetMargin1();
                }
                else
                {
                    nMaxRight = lNullPix - lMinFrame;
                    if (mxColumnItem->IsFirstAct())
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
                    else if ( mxColumnItem->Count() > 1 )
                    {
                        nMaxRight += mpBorders[0].nPos;
                    }
                    else
                    {
                        nMaxRight += GetRightIndent() - std::max(GetFirstLineIndent(), GetLeftIndent());
                    }
                    // Do not drag the left table edge over the edge of the page
                    if(mxLRSpaceItem.get() && mxColumnItem->IsTable())
                    {
                        long nTmp=ConvertSizePixel(mxLRSpaceItem->GetLeft());
                        if(nTmp>nMaxLeft)
                            nMaxLeft=nTmp;
                    }
                }
            }
            break;
        }
        case RULER_TYPE_MARGIN2:
        {        // right edge of the surrounding Frame
            mxRulerImpl->lMaxRightLogic =
                mxMinMaxItem.get() ?
                    GetPageWidth() - GetRightMax() :
                    GetPageWidth();
            nMaxRight = ConvertSizePixel(mxRulerImpl->lMaxRightLogic);


            if(!mxColumnItem.get())
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
            else if(mxRulerImpl->bIsTableRows)
            {
                // get the bottom move range from the last border position - only available for rows!
                // protection of content means the margin is not moveable - it's just a page break inside of a cell
                if(bHorz || mxRulerImpl->aProtectItem.IsContentProtected())
                {
                    nMaxLeft = nMaxRight = mpBorders[mxColumnItem->Count() - 1].nMaxPos + lNullPix;
                }
                else
                {
                    if(nDragType & SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL)
                    {
                        nMaxLeft = (mxColumnItem->Count()) * lMinFrame + lNullPix;
                    }
                    else
                    {
                        if(mxColumnItem->Count() > 1)
                            nMaxLeft = mpBorders[mxColumnItem->Count() - 2].nPos + lMinFrame + lNullPix;
                        else
                            nMaxLeft = lMinFrame + lNullPix;
                    }
                    if(mxColumnItem->Count() > 1)
                        nMaxRight = mpBorders[mxColumnItem->Count() - 2].nMaxPos + lNullPix;
                    else
                        nMaxRight -= GetRightIndent() - lNullPix;
                }
            }
            else
            {
                nMaxLeft = lMinFrame + lNullPix;
                if(IsActLastColumn() || mxColumnItem->Count() < 2 ) //If last active column
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
                if( mxColumnItem->Count() >= 2 )
                {
                    long nNewMaxLeft =
                        lMinFrame + lNullPix +
                        mpBorders[mxColumnItem->Count() - 2].nPos +
                        mpBorders[mxColumnItem->Count() - 2].nWidth;
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
                if(nIdx == mxColumnItem->GetActColumn())
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
                if(mxColumnItem.get())
                {
                    //nIdx contains the position of the currently moved item
                    //next visible separator on the left
                    sal_uInt16 nLeftCol=GetActLeftColumn(false, nIdx);
                    //next visible separator on the right
                    sal_uInt16 nRightCol=GetActRightColumn(false, nIdx);
                    //next separator on the left - regardless if visible or not
                    sal_uInt16 nActLeftCol=GetActLeftColumn();
                    //next separator on the right - regardless if visible or not
                    sal_uInt16 nActRightCol=GetActRightColumn();
                    if(mxColumnItem->IsTable())
                    {
                        if(nDragType & SvxRulerDragFlags::OBJECT_ACTLINE_ONLY)
                        {
                            //the current row/column should be modified only
                            //then the next/previous visible border position
                            //marks the min/max positions
                            nMaxLeft = nLeftCol == USHRT_MAX ?
                                0 :
                                mpBorders[nLeftCol].nPos;
                            //rows can always be increased without a limit
                            if(mxRulerImpl->bIsTableRows)
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
                            if(SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL & nDragType && !bHorz && mxRulerImpl->bIsTableRows)
                                nMaxLeft = (nIdx + 1) * lMinFrame + lNullPix;
                            else
                                nMaxLeft = mpBorders[nIdx].nMinPos + lNullPix;
                            if((SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL & nDragType) ||
                               (SvxRulerDragFlags::OBJECT_SIZE_LINEAR & nDragType) )
                            {
                                if(mxRulerImpl->bIsTableRows)
                                {
                                    if(bHorz)
                                        nMaxRight = GetRightIndent() + lNullPix -
                                                (mxColumnItem->Count() - nIdx - 1) * lMinFrame;
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
                        if((SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL & nDragType) ||
                           (SvxRulerDragFlags::OBJECT_SIZE_LINEAR & nDragType) )
                        {
                            nMaxRight=lNullPix+CalcPropMaxRight(nIdx);
                        }
                        else if(SvxRulerDragFlags::OBJECT_SIZE_LINEAR & nDragType)
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
                                    GetActRightColumn(true, nIdx);

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
                    if(mxObjectItem->HasLimits())
                    {
                        if(CalcLimits(nMaxLeft, nMaxRight, (nIdx & 1) == 0))
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
                if(nIdx == mxColumnItem->Count()-2) { // last column
                    nMaxRight = GetMargin2() + lNullPix;
                    if(mxColumnItem->IsLastAct()) {
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
                    if(mxColumnItem->GetActColumn()-1 == nIdx) {
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

                    if(mxColumnItem.get() && !mxColumnItem->IsFirstAct())
                        nMaxLeft += mpBorders[mxColumnItem->GetActColumn()-1].nPos +
                            mpBorders[mxColumnItem->GetActColumn()-1].nWidth;
                    nMaxRight = lNullPix + GetMargin2();

                    // Dragging along
                    if((INDENT_FIRST_LINE - INDENT_GAP) != nIdx &&
                       !(nDragType & SvxRulerDragFlags::OBJECT_LEFT_INDENT_ONLY))
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

                    if(mxColumnItem.get() && !mxColumnItem->IsFirstAct())
                        nMaxLeft += mpBorders[mxColumnItem->GetActColumn()-1].nPos +
                            mpBorders[mxColumnItem->GetActColumn()-1].nWidth;
                    nMaxRight = lNullPix + GetRightIndent() - lMinFrame;

                    // Dragging along
                    if((INDENT_FIRST_LINE - INDENT_GAP) != nIdx &&
                       !(nDragType & SvxRulerDragFlags::OBJECT_LEFT_INDENT_ONLY))
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
                    if(mxColumnItem.get())
                    {
                        sal_uInt16 nRightCol=GetActRightColumn( true );
                        if(!IsActLastColumn( true ))
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
                    if(mxColumnItem.get())
                    {
                        sal_uInt16 nRightCol=GetActRightColumn( true );
                        if(!IsActLastColumn( true ))
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

        mxRulerImpl->lMaxRightLogic = GetLogicRightIndent() + lLogicNullOffset;
        nMaxRight = ConvertSizePixel(mxRulerImpl->lMaxRightLogic);
        break;
    default: ; //prevent warning
    }
}

bool SvxRuler::StartDrag()
{
    /*
       Beginning of a drag operation (SV-handler) evaluates modifier and
       calculated values

       [Cross-reference]

       <SvxRuler::EvalModifier()>
       <SvxRuler::CalcMinMax()>
       <SvxRuler::EndDrag()>
    */
    bool bContentProtected = mxRulerImpl->aProtectItem.IsContentProtected();

    if(!bValid)
        return false;

    mxRulerImpl->lLastLMargin = GetMargin1();
    mxRulerImpl->lLastRMargin = GetMargin2();

    bool bOk = true;

    lInitialDragPos = GetDragPos();
    switch(GetDragType())
    {
        case RULER_TYPE_MARGIN1:        // left edge of the surrounding Frame
        case RULER_TYPE_MARGIN2:        // right edge of the surrounding Frame
            if((bHorz && mxLRSpaceItem.get()) || (!bHorz && mxULSpaceItem.get()))
            {
                if(!mxColumnItem.get())
                    EvalModifier();
                else
                    nDragType = SvxRulerDragFlags::OBJECT;
            }
            else
            {
                bOk = false;
            }
            break;
        case RULER_TYPE_BORDER: // Table, column (Modifier)
            if(mxColumnItem.get())
            {
                nDragOffset = 0;
                if (!mxColumnItem->IsTable())
                    nDragOffset = GetDragPos() - mpBorders[GetDragAryPos()].nPos;
                EvalModifier();
            }
            else
                nDragOffset = 0;
            break;
        case RULER_TYPE_INDENT: // Paragraph indents (Modifier)
        {
            if( bContentProtected )
                return false;
            sal_uInt16 nIndent = INDENT_LEFT_MARGIN;
            if((nIndent) == GetDragAryPos() + INDENT_GAP) {  // Left paragraph indent
                mpIndents[0] = mpIndents[INDENT_FIRST_LINE];
                mpIndents[0].nStyle |= RULER_STYLE_DONTKNOW;
                EvalModifier();
            }
            else
            {
                nDragType = SvxRulerDragFlags::OBJECT;
            }
            mpIndents[1] = mpIndents[GetDragAryPos() + INDENT_GAP];
            mpIndents[1].nStyle |= RULER_STYLE_DONTKNOW;
            break;
        }
        case RULER_TYPE_TAB: // Tabs (Modifier)
            if( bContentProtected )
                return false;
            EvalModifier();
            mpTabs[0] = mpTabs[GetDragAryPos() + 1];
            mpTabs[0].nStyle |= RULER_STYLE_DONTKNOW;
            break;
        default:
            nDragType = SvxRulerDragFlags::NONE;
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
            mxRulerImpl->lLastLMargin = GetMargin1();
            break;
        case RULER_TYPE_MARGIN2: // right edge of the surrounding Frame
            DragMargin2();
            mxRulerImpl->lLastRMargin = GetMargin2();
            break;
        case RULER_TYPE_INDENT: // Paragraph indents
            DragIndents();
            break;
        case RULER_TYPE_BORDER: // Table, columns
            if(mxColumnItem.get())
                DragBorders();
            else if(mxObjectItem.get())
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
    const bool bUndo = IsDragCanceled();
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
                    if(!mxColumnItem.get() || !mxColumnItem->IsTable())
                        ApplyMargins();

                    if(mxColumnItem.get() &&
                       (mxColumnItem->IsTable() ||
                        (nDragType & SvxRulerDragFlags::OBJECT_SIZE_PROPORTIONAL)))
                        ApplyBorders();

                }
                break;
            case RULER_TYPE_BORDER: // Table, columns
                if(lInitialDragPos != lPos ||
                    (mxRulerImpl->bIsTableRows && bHorz)) //special case - the null offset is changed here
                {
                    if(mxColumnItem.get())
                    {
                        ApplyBorders();
                        if(bHorz)
                            UpdateTabs();
                    }
                    else if(mxObjectItem.get())
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
    nDragType = SvxRulerDragFlags::NONE;

    mbCoarseSnapping = false;
    mbSnapping = true;

    Ruler::EndDrag();
    if(bUndo)
    {
        for(sal_uInt16 i = 0; i < mxRulerImpl->nControlerItems; i++)
        {
            pCtrlItems[i]->ClearCache();
            pCtrlItems[i]->GetBindings().Invalidate(pCtrlItems[i]->GetId());
        }
    }
}

void SvxRuler::ExtraDown()
{
    /* Override SV method, sets the new type for the Default tab. */

    // Switch Tab Type
    if(mxTabStopItem.get() &&
        (nFlags & SvxRulerSupportFlags::TABS) == SvxRulerSupportFlags::TABS)
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
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>( &rHint );
    if(bActive &&
       pSimpleHint &&
       pSimpleHint->GetId() == SFX_HINT_UPDATEDONE )
     {
        Update();
        EndListening(*pBindings);
        bValid = true;
        bListening = false;
    }
}


IMPL_LINK_TYPED( SvxRuler, MenuSelect, Menu *, pMenu, bool )
{
    /* Handler of the context menus for switching the unit of measurement */
    SetUnit(FieldUnit(pMenu->GetCurItemId()));
    return false;
}

IMPL_LINK_TYPED( SvxRuler, TabMenuSelect, Menu *, pMenu, bool )
{
    /* Handler of the tab menu for setting the type */
    if(mxTabStopItem.get() && mxTabStopItem->Count() > mxRulerImpl->nIdx)
    {
        SvxTabStop aTabStop = mxTabStopItem->At(mxRulerImpl->nIdx);
        aTabStop.GetAdjustment() = ToAttrTab_Impl(pMenu->GetCurItemId() - 1);
        mxTabStopItem->Remove(mxRulerImpl->nIdx);
        mxTabStopItem->Insert(aTabStop);
        sal_uInt16 nTabStopId = bHorz ? SID_ATTR_TABSTOP : SID_ATTR_TABSTOP_VERTICAL;
        pBindings->GetDispatcher()->Execute( nTabStopId, SfxCallMode::RECORD, mxTabStopItem.get(), 0L );
        UpdateTabs();
        mxRulerImpl->nIdx = 0;
    }
    return false;
}

void SvxRuler::Command( const CommandEvent& rCommandEvent )
{
    /* Mouse context menu for switching the unit of measurement */
    if ( CommandEventId::ContextMenu == rCommandEvent.GetCommand() )
    {
        CancelDrag();
        bool bRTL = mxRulerImpl->pTextRTLItem && mxRulerImpl->pTextRTLItem->GetValue();
        if ( !mpTabs.empty() &&
             RULER_TYPE_TAB ==
             GetType( rCommandEvent.GetMousePosPixel(), &mxRulerImpl->nIdx ) &&
             mpTabs[mxRulerImpl->nIdx + TAB_GAP].nStyle < RULER_TAB_DEFAULT )
        {
            PopupMenu aMenu;
            aMenu.SetSelectHdl(LINK(this, SvxRuler, TabMenuSelect));
            ScopedVclPtrInstance< VirtualDevice > pDev;
            const Size aSz(ruler_tab_svx.width + 2, ruler_tab_svx.height + 2);
            pDev->SetOutputSize(aSz);
            pDev->SetBackground(Wallpaper(Color(COL_WHITE)));
            Color aFillColor(pDev->GetSettings().GetStyleSettings().GetShadowColor());
            const Point aPt(aSz.Width() / 2, aSz.Height() / 2);

            for ( sal_uInt16 i = RULER_TAB_LEFT; i < RULER_TAB_DEFAULT; ++i )
            {
                sal_uInt16 nStyle = bRTL ? i|RULER_TAB_RTL : i;
                nStyle |= static_cast<sal_uInt16>(bHorz ? WB_HORZ : WB_VERT);
                DrawTab(*pDev, aFillColor, aPt, nStyle);
                aMenu.InsertItem(i + 1,
                                 ResId(RID_SVXSTR_RULER_START + i, DIALOG_MGR()).toString(),
                                 Image(pDev->GetBitmap(Point(), aSz), Color(COL_WHITE)));
                aMenu.CheckItem(i + 1, i == mpTabs[mxRulerImpl->nIdx + TAB_GAP].nStyle);
                pDev->SetOutputSize(aSz); // delete device
            }
            aMenu.Execute( this, rCommandEvent.GetMousePosPixel() );
        }
        else
        {
            PopupMenu aMenu(ResId(RID_SVXMN_RULER, DIALOG_MGR()));
            aMenu.SetSelectHdl(LINK(this, SvxRuler, MenuSelect));
            FieldUnit eUnit = GetUnit();
            const sal_uInt16 nCount = aMenu.GetItemCount();

            bool bReduceMetric = bool(nFlags & SvxRulerSupportFlags::REDUCED_METRIC);
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
                        bool bForceDontConsiderHidden,
                        sal_uInt16 nAct ) const
{
    if( nAct == USHRT_MAX )
        nAct = mxColumnItem->GetActColumn();
    else
        nAct++; //To be able to pass on the ActDrag

    bool bConsiderHidden = !bForceDontConsiderHidden &&
                               !(nDragType & SvxRulerDragFlags::OBJECT_ACTLINE_ONLY);

    while( nAct < mxColumnItem->Count() - 1 )
    {
        if (mxColumnItem->At(nAct).bVisible || bConsiderHidden)
            return nAct;
        else
            nAct++;
    }
    return USHRT_MAX;
}

sal_uInt16 SvxRuler::GetActLeftColumn(
                        bool bForceDontConsiderHidden,
                        sal_uInt16 nAct ) const
{
    if(nAct == USHRT_MAX)
        nAct = mxColumnItem->GetActColumn();

    sal_uInt16 nLeftOffset = 1;

    bool bConsiderHidden = !bForceDontConsiderHidden &&
                               !(nDragType & SvxRulerDragFlags::OBJECT_ACTLINE_ONLY);

    while(nAct >= nLeftOffset)
    {
        if (mxColumnItem->At(nAct - nLeftOffset).bVisible || bConsiderHidden)
            return nAct - nLeftOffset;
        else
            nLeftOffset++;
    }
    return USHRT_MAX;
}

bool SvxRuler::IsActLastColumn(
                        bool bForceDontConsiderHidden,
                        sal_uInt16 nAct) const
{
    return GetActRightColumn(bForceDontConsiderHidden, nAct) == USHRT_MAX;
}

bool SvxRuler::IsActFirstColumn(
                        bool bForceDontConsiderHidden,
                        sal_uInt16 nAct) const
{
    return GetActLeftColumn(bForceDontConsiderHidden, nAct) == USHRT_MAX;
}

long SvxRuler::CalcPropMaxRight(sal_uInt16 nCol) const
{

    if(!(nDragType & SvxRulerDragFlags::OBJECT_SIZE_LINEAR))
    {
        // Remove the minimum width for all affected columns
        // starting from the right edge
        long _nMaxRight = GetMargin2() - GetMargin1();

        long lFences = 0;
        long lMinSpace = USHRT_MAX;
        long lOldPos;
        long lColumns = 0;

        sal_uInt16 nStart;
        if(!mxColumnItem->IsTable())
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
                    while(!(*mxColumnItem.get())[nRight].bVisible)
                    {
                        nRight++;
                    }
                }
                else
                {
                    nRight = GetActRightColumn(false, nActCol);
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
        if(mxColumnItem->IsTable())
        {
            sal_uInt16 nVisCols = 0;
            for(size_t i = GetActRightColumn(false, nCol); i < mpBorders.size();)
            {
                if((*mxColumnItem.get())[i].bVisible)
                    nVisCols++;
                i = GetActRightColumn(false, i);
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
void SvxRuler::SetTabsRelativeToIndent( bool bRel )
{
    mxRulerImpl->bIsTabsRelativeToIndent = bRel;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
