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

#include <string.h>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <tools/poly.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/settings.hxx>

#include <svtools/ruler.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>
#include <svtools/colorcfg.hxx>

#include <memory>
#include <vector>

using namespace std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;

#define RULER_OFF           3
#define RULER_RESIZE_OFF    4
#define RULER_MIN_SIZE      3

#define RULER_VAR_SIZE      8

#define RULER_UPDATE_LINES  0x01

#define RULER_CLIP          150

#define RULER_UNIT_MM       0
#define RULER_UNIT_CM       1
#define RULER_UNIT_M        2
#define RULER_UNIT_KM       3
#define RULER_UNIT_INCH     4
#define RULER_UNIT_FOOT     5
#define RULER_UNIT_MILE     6
#define RULER_UNIT_POINT    7
#define RULER_UNIT_PICA     8
#define RULER_UNIT_CHAR     9
#define RULER_UNIT_LINE    10
#define RULER_UNIT_COUNT   11

class ImplRulerData
{
    friend class Ruler;

private:
    vector<RulerLine>    pLines;
    vector<RulerBorder>  pBorders;
    vector<RulerIndent>  pIndents;
    vector<RulerTab>     pTabs;

    long       nNullVirOff;
    long       nRulVirOff;
    long       nRulWidth;
    long       nPageOff;
    long       nPageWidth;
    long       nNullOff;
    long       nMargin1;
    long       nMargin2;
    long       nLeftFrameMargin;
    long       nRightFrameMargin;
    sal_uInt16 nMargin1Style;
    sal_uInt16 nMargin2Style;
    bool       bAutoPageWidth;
    bool       bTextRTL;

public:
    ImplRulerData();
    ~ImplRulerData();

    ImplRulerData& operator=( const ImplRulerData& rData );
};

ImplRulerData::ImplRulerData() :
    nNullVirOff       (0),
    nRulVirOff        (0),
    nRulWidth         (0),
    nPageOff          (0),
    nPageWidth        (0),
    nNullOff          (0),
    nMargin1          (0),
    nMargin2          (0),
    nLeftFrameMargin  (0),
    nRightFrameMargin (0),
    nMargin1Style     (0),
    nMargin2Style     (0),
    bAutoPageWidth    (true), // Page width == EditWin width
    bTextRTL          (false)
{
}

ImplRulerData::~ImplRulerData()
{}

ImplRulerData& ImplRulerData::operator=( const ImplRulerData& rData )
{
    if( this == &rData )
        return *this;

    nNullVirOff       = rData.nNullVirOff;
    nRulVirOff        = rData.nRulVirOff;
    nRulWidth         = rData.nRulWidth;
    nPageOff          = rData.nPageOff;
    nPageWidth        = rData.nPageWidth;
    nNullOff          = rData.nNullOff;
    nMargin1          = rData.nMargin1;
    nMargin2          = rData.nMargin2;
    nLeftFrameMargin  = rData.nLeftFrameMargin;
    nRightFrameMargin = rData.nRightFrameMargin;
    nMargin1Style     = rData.nMargin1Style;
    nMargin2Style     = rData.nMargin2Style;
    bAutoPageWidth    = rData.bAutoPageWidth;
    bTextRTL          = rData.bTextRTL;

    if ( !rData.pLines.empty() )
    {
        pLines.resize(rData.pLines.size());
        std::copy(rData.pLines.begin(), rData.pLines.end(), pLines.begin());
    }
    else
    {
        pLines.clear();
    }

    if ( !rData.pBorders.empty() )
    {
        pBorders.resize(rData.pBorders.size());
        std::copy(rData.pBorders.begin(), rData.pBorders.end(), pBorders.begin());
    }
    else
    {
        pBorders.clear();
    }

    if ( !rData.pIndents.empty() )
    {
        pIndents.resize(rData.pIndents.size());
        std::copy(rData.pIndents.begin(), rData.pIndents.end(), pIndents.begin());
    }
    else
    {
        pIndents.clear();
    }

    if ( !rData.pTabs.empty() )
    {
        pTabs.resize(rData.pTabs.size());
        std::copy(rData.pTabs.begin(), rData.pTabs.end(), pTabs.begin());
    }
    else
    {
        pTabs.clear();
    }

    return *this;
}

static const RulerUnitData aImplRulerUnitTab[RULER_UNIT_COUNT] =
{
{ MAP_100TH_MM,        100,    25.0,    25.0,     50.0,    100.0,  " mm"    }, // MM
{ MAP_100TH_MM,       1000,   100.0,   500.0,   1000.0,   1000.0,  " cm"    }, // CM
{ MAP_MM,             1000,    10.0,   250.0,    500.0,   1000.0,  " m"     }, // M
{ MAP_CM,           100000, 12500.0, 25000.0,  50000.0, 100000.0,  " km"    }, // KM
{ MAP_1000TH_INCH,    1000,    62.5,   125.0,    500.0,   1000.0,  "\""     }, // INCH
{ MAP_100TH_INCH,     1200,   120.0,   120.0,    600.0,   1200.0,  "'"      }, // FOOT
{ MAP_10TH_INCH,    633600, 63360.0, 63360.0, 316800.0, 633600.0,  " miles" }, // MILE
{ MAP_POINT,             1,    12.0,    12.0,     12.0,     36.0,  " pt"    }, // POINT
{ MAP_100TH_MM,        423,   423.0,   423.0,    423.0,    846.0,  " pi"    }, // PICA
{ MAP_100TH_MM,        371,   371.0,   371.0,    371.0,    743.0,  " ch"    }, // CHAR
{ MAP_100TH_MM,        551,   551.0,   551.0,    551.0,   1102.0,  " li"    }  // LINE
};

static RulerTabData ruler_tab =
{
    0, // DPIScaleFactor to be set
    7, // ruler_tab_width
    6, // ruler_tab_height
    2, // ruler_tab_height2
    2, // ruler_tab_width2
    8, // ruler_tab_cwidth
    4, // ruler_tab_cwidth2
    4, // ruler_tab_cwidth3
    2, // ruler_tab_cwidth4
    4, // ruler_tab_dheight
    1, // ruler_tab_dheight2
    5, // ruler_tab_dwidth
    3, // ruler_tab_dwidth2
    3, // ruler_tab_dwidth3
    1, // ruler_tab_dwidth4
    5  // ruler_tab_textoff
};

void Ruler::ImplInit( WinBits nWinBits )
{
    // Default WinBits setzen
    if ( !(nWinBits & WB_VERT) )
    {
        nWinBits |= WB_HORZ;

        // --- RTL --- no UI mirroring for horizontal rulers, because
        // the document is also not mirrored
        EnableRTL( false );
    }

    // Initialize variables
    mnWinStyle      = nWinBits;             // Window-Style
    mnBorderOff     = 0;                    // Border-Offset
    mnWinOff        = 0;                    // EditWinOffset
    mnWinWidth      = 0;                    // EditWinWidth
    mnWidth         = 0;                    // Window width
    mnHeight        = 0;                    // Window height
    mnVirOff        = 0;                    // Offset of VirtualDevice from top-left corner
    mnVirWidth      = 0;                    // width or height from VirtualDevice
    mnVirHeight     = 0;                    // height of width from VirtualDevice
    mnDragPos       = 0;                    // Drag-Position (Null point)
    mnUpdateEvtId   = nullptr;                    // Update event was not sent yet
    mnDragAryPos    = 0;                    // Drag-Array-Index
    mnDragSize      = 0;                    // Did size change at dragging
    mnDragModifier  = 0;                    // Modifier key at dragging
    mnExtraStyle    = 0;                    // Style of Extra field
    mnExtraClicks   = 0;                    // No. of clicks for Extra field
    mnExtraModifier = 0;                    // Modifier key at click in extra field
    mnCharWidth     = 371;
    mnLineHeight    = 551;
    mbCalc          = true;                 // Should recalculate page width
    mbFormat        = true;                 // Should redraw
    mbDrag          = false;                // Currently at dragging
    mbDragDelete    = false;                // Has mouse left the dragging area
    mbDragCanceled  = false;                // Dragging cancelled?
    mbAutoWinWidth  = true;                 // EditWinWidth == RulerWidth
    mbActive        = true;                 // Is ruler active
    mnUpdateFlags   = 0;                    // What needs to be updated
    mpData          = mpSaveData;           // Pointer to normal data
    meExtraType     = RULER_EXTRA_DONTKNOW; // What is in extra field
    meDragType      = RULER_TYPE_DONTKNOW;  // Which element is dragged

    // Initialize Units
    mnUnitIndex     = RULER_UNIT_CM;
    meUnit          = FUNIT_CM;
    maZoom          = Fraction( 1, 1 );
    meSourceUnit    = MAP_100TH_MM;

    // Recalculate border widths
    if ( nWinBits & WB_BORDER )
        mnBorderWidth = 1;
    else
        mnBorderWidth = 0;

    // Settings
    ImplInitSettings( true, true, true );

    // Setup the default size
    Rectangle aRect;
    GetTextBoundRect( aRect, "0123456789" );
    long nDefHeight = aRect.GetHeight() + RULER_OFF * 2 + ruler_tab.textoff * 2 + mnBorderWidth;

    Size aDefSize;
    if ( nWinBits & WB_HORZ )
        aDefSize.Height() = nDefHeight;
    else
        aDefSize.Width() = nDefHeight;
    SetOutputSizePixel( aDefSize );
    SetType(WINDOW_RULER);
    pAccContext = nullptr;
}

Ruler::Ruler( vcl::Window* pParent, WinBits nWinStyle ) :
    Window( pParent, nWinStyle & WB_3DLOOK ),
    maVirDev( VclPtr<VirtualDevice>::Create(*this) ),
    maMapMode( MAP_100TH_MM ),
    mpSaveData(new ImplRulerData),
    mpData(nullptr),
    mpDragData(new ImplRulerData)
{
    // Check to see if the ruler constructor has
    // already been called before otherwise
    // we end up with over-scaled elements
    if (ruler_tab.DPIScaleFactor == 0)
    {
        ruler_tab.DPIScaleFactor = GetDPIScaleFactor();
        ruler_tab.width    *= ruler_tab.DPIScaleFactor;
        ruler_tab.height   *= ruler_tab.DPIScaleFactor;
        ruler_tab.height2  *= ruler_tab.DPIScaleFactor;
        ruler_tab.width2   *= ruler_tab.DPIScaleFactor;
        ruler_tab.cwidth   *= ruler_tab.DPIScaleFactor;
        ruler_tab.cwidth2  *= ruler_tab.DPIScaleFactor;
        ruler_tab.cwidth3  *= ruler_tab.DPIScaleFactor;
        ruler_tab.cwidth4  *= ruler_tab.DPIScaleFactor;
        ruler_tab.dheight  *= ruler_tab.DPIScaleFactor;
        ruler_tab.dheight2 *= ruler_tab.DPIScaleFactor;
        ruler_tab.dwidth   *= ruler_tab.DPIScaleFactor;
        ruler_tab.dwidth2  *= ruler_tab.DPIScaleFactor;
        ruler_tab.dwidth3  *= ruler_tab.DPIScaleFactor;
        ruler_tab.dwidth4  *= ruler_tab.DPIScaleFactor;
        ruler_tab.textoff  *= ruler_tab.DPIScaleFactor;
    }


    ImplInit( nWinStyle );
}

Ruler::~Ruler()
{
    disposeOnce();
}

void Ruler::dispose()
{
    if ( mnUpdateEvtId )
        Application::RemoveUserEvent( mnUpdateEvtId );
    delete mpSaveData;
    mpSaveData = nullptr;
    delete mpDragData;
    mpDragData = nullptr;
    if( pAccContext )
    {
        pAccContext->release();
        pAccContext = nullptr;
    }
    Window::dispose();
}

void Ruler::ImplVDrawLine(vcl::RenderContext& rRenderContext, long nX1, long nY1, long nX2, long nY2)
{
    if ( nX1 < -RULER_CLIP )
    {
        nX1 = -RULER_CLIP;
        if ( nX2 < -RULER_CLIP )
            return;
    }
    long nClip = mnVirWidth + RULER_CLIP;
    if ( nX2 > nClip )
    {
        nX2 = nClip;
        if ( nX1 > nClip )
            return;
    }

    if ( mnWinStyle & WB_HORZ )
        rRenderContext.DrawLine( Point( nX1, nY1 ), Point( nX2, nY2 ) );
    else
        rRenderContext.DrawLine( Point( nY1, nX1 ), Point( nY2, nX2 ) );
}

void Ruler::ImplVDrawRect(vcl::RenderContext& rRenderContext, long nX1, long nY1, long nX2, long nY2)
{
    if ( nX1 < -RULER_CLIP )
    {
        nX1 = -RULER_CLIP;
        if ( nX2 < -RULER_CLIP )
            return;
    }
    long nClip = mnVirWidth + RULER_CLIP;
    if ( nX2 > nClip )
    {
        nX2 = nClip;
        if ( nX1 > nClip )
            return;
    }

    if ( mnWinStyle & WB_HORZ )
        rRenderContext.DrawRect(Rectangle(nX1, nY1, nX2, nY2));
    else
        rRenderContext.DrawRect(Rectangle(nY1, nX1, nY2, nX2));
}

void Ruler::ImplVDrawText(vcl::RenderContext& rRenderContext, long nX, long nY, const OUString& rText, long nMin, long nMax)
{
    Rectangle aRect;
    rRenderContext.GetTextBoundRect(aRect, rText);

    long nShiftX = ( aRect.GetWidth() / 2 ) + aRect.Left();
    long nShiftY = ( aRect.GetHeight() / 2 ) + aRect.Top();

    if ( (nX > -RULER_CLIP) && (nX < mnVirWidth + RULER_CLIP) && ( nX < nMax - nShiftX ) && ( nX > nMin + nShiftX ) )
    {
        if ( mnWinStyle & WB_HORZ )
            rRenderContext.DrawText(Point(nX - nShiftX, nY - nShiftY), rText);
        else
            rRenderContext.DrawText(Point(nY - nShiftX, nX - nShiftY), rText);
    }
}

void Ruler::ImplInvertLines(vcl::RenderContext& rRenderContext)
{
    // Position lines
    if (!mpData->pLines.empty() && mbActive && !mbDrag && !mbFormat && !(mnUpdateFlags & RULER_UPDATE_LINES) )
    {
        long nNullWinOff = mpData->nNullVirOff + mnVirOff;
        long nRulX1      = mpData->nRulVirOff  + mnVirOff;
        long nRulX2      = nRulX1 + mpData->nRulWidth;
        long nY          = (RULER_OFF * 2) + mnVirHeight - 1;

        // Calculate rectangle
        Rectangle aRect;
        if (mnWinStyle & WB_HORZ)
            aRect.Bottom() = nY;
        else
            aRect.Right() = nY;

        // Draw lines
        for (size_t i = 0; i < mpData->pLines.size(); i++)
        {
            const long n = mpData->pLines[i].nPos + nNullWinOff;
            if ((n >= nRulX1) && (n < nRulX2))
            {
                if (mnWinStyle & WB_HORZ )
                {
                    aRect.Left()   = n;
                    aRect.Right()  = n;
                }
                else
                {
                    aRect.Top()    = n;
                    aRect.Bottom() = n;
                }
                Rectangle aTempRect = aRect;

                if (mnWinStyle & WB_HORZ)
                    aTempRect.Bottom() = RULER_OFF - 1;
                else
                    aTempRect.Right() = RULER_OFF - 1;

                rRenderContext.Erase(aTempRect);

                if (mnWinStyle & WB_HORZ)
                {
                    aTempRect.Bottom() = aRect.Bottom();
                    aTempRect.Top()    = aTempRect.Bottom() - RULER_OFF + 1;
                }
                else
                {
                    aTempRect.Right()  = aRect.Right();
                    aTempRect.Left()   = aTempRect.Right() - RULER_OFF + 1;
                }
                rRenderContext.Erase(aTempRect);
                Invert(aRect);
            }
        }
        mnUpdateFlags = 0;
    }
}

void Ruler::ImplDrawTicks(vcl::RenderContext& rRenderContext, long nMin, long nMax, long nStart, long nTop, long nBottom)
{
    double nCenter = nTop + ((nBottom - nTop) / 2);

    long nTickLength3 = (nBottom - nTop) * 0.5;
    long nTickLength2 = nTickLength3 * 0.66;
    long nTickLength1 = nTickLength2 * 0.66;

    long nScale = ruler_tab.DPIScaleFactor;
    long DPIOffset = nScale - 1;

    double nTick4 = aImplRulerUnitTab[mnUnitIndex].nTick4;
    double nTick2 = 0;
    double nTickCount = aImplRulerUnitTab[mnUnitIndex].nTick1 / nScale;
    double nTickUnit = 0;
    long nTickWidth;
    bool bNoTicks = false;

    Size aPixSize = rRenderContext.LogicToPixel(Size(nTick4, nTick4), maMapMode);

    if (mnUnitIndex == RULER_UNIT_CHAR)
    {
        if (mnCharWidth == 0)
            mnCharWidth = 371;
        nTick4 = mnCharWidth * 2;
        nTick2 = mnCharWidth;
        nTickCount = mnCharWidth;
        nTickUnit = mnCharWidth;
    }
    else if (mnUnitIndex == RULER_UNIT_LINE)
    {
        if (mnLineHeight == 0)
            mnLineHeight = 551;
        nTick4 = mnLineHeight * 2;
        nTick2 = mnLineHeight;
        nTickUnit = mnLineHeight;
        nTickCount = mnLineHeight;
    }

    if (mnWinStyle & WB_HORZ)
    {
        nTickWidth = aPixSize.Width();
    }
    else
    {
        vcl::Font aFont = rRenderContext.GetFont();
        if (mnWinStyle & WB_RIGHT_ALIGNED)
            aFont.SetOrientation(2700);
        else
            aFont.SetOrientation(900);
        rRenderContext.SetFont(aFont);
        nTickWidth = aPixSize.Height();
    }

    long nMaxWidth = rRenderContext.PixelToLogic(Size(mpData->nPageWidth, 0), maMapMode).Width();
    if (nMaxWidth < 0)
        nMaxWidth = -nMaxWidth;

    if ((mnUnitIndex == RULER_UNIT_CHAR) || (mnUnitIndex == RULER_UNIT_LINE))
        nMaxWidth /= nTickUnit;
    else
        nMaxWidth /= aImplRulerUnitTab[mnUnitIndex].nTickUnit;

    OUString aNumString = OUString::number(nMaxWidth);
    long nTxtWidth = rRenderContext.GetTextWidth( aNumString );
    const long nTextOff = 4;

    // Determine the number divider for ruler drawn numbers - means which numbers
    // should be shown on the ruler and which should be skipped because the ruler
    // is not big enough to draw them
    if (nTickWidth < nTxtWidth + nTextOff)
    {
        // Calculate the scale of the ruler
        long nMulti = 1;
        long nOrgTick4 = nTick4;

        while (nTickWidth < nTxtWidth + nTextOff)
        {
            long nOldMulti = nMulti;
            if (nTickWidth == 0)
                nMulti *= 10;
            else if (nMulti < 10)
                nMulti++;
            else if (nMulti < 100)
                nMulti += 10;
            else if (nMulti < 1000)
                nMulti += 100;
            else
                nMulti += 1000;

            // Overflow - in this case don't draw ticks and exit
            if (nMulti < nOldMulti)
            {
                bNoTicks = true;
                break;
            }

            nTick4 = nOrgTick4 * nMulti;
            aPixSize = rRenderContext.LogicToPixel(Size(nTick4, nTick4), maMapMode);
            if (mnWinStyle & WB_HORZ)
                nTickWidth = aPixSize.Width();
            else
                nTickWidth = aPixSize.Height();
        }
        nTickCount = nTick4;
    }
    else
    {
        rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetShadowColor());
    }

    if (!bNoTicks)
    {
        long n = 0;
        double nTick = 0.0;
        double nTick3 = 0;

        if ((mnUnitIndex != RULER_UNIT_CHAR) && (mnUnitIndex != RULER_UNIT_LINE))
        {
            nTick2 = aImplRulerUnitTab[mnUnitIndex].nTick2;
            nTick3 = aImplRulerUnitTab[mnUnitIndex].nTick3;
        }

        Size nTickGapSize;

        nTickGapSize = rRenderContext.LogicToPixel(Size(nTickCount, nTickCount), maMapMode);
        long nTickGap1 = mnWinStyle & WB_HORZ ? nTickGapSize.Width() : nTickGapSize.Height();
        nTickGapSize = rRenderContext.LogicToPixel(Size(nTick2, nTick2), maMapMode);
        long nTickGap2 = mnWinStyle & WB_HORZ ? nTickGapSize.Width() : nTickGapSize.Height();
        nTickGapSize = rRenderContext.LogicToPixel(Size(nTick3, nTick3), maMapMode);
        long nTickGap3 = mnWinStyle & WB_HORZ ? nTickGapSize.Width() : nTickGapSize.Height();

        while (((nStart - n) >= nMin) || ((nStart + n) <= nMax))
        {
            // Null point
            if (nTick == 0.0)
            {
                if (nStart > nMin)
                {
                    // 0 is only painted when Margin1 is not equal to zero
                    if ((mpData->nMargin1Style & RULER_STYLE_INVISIBLE) || (mpData->nMargin1 != 0))
                    {
                        aNumString = "0";
                        ImplVDrawText(rRenderContext, nStart, nCenter, aNumString);
                    }
                }
            }
            else
            {
                aPixSize = rRenderContext.LogicToPixel(Size(nTick, nTick), maMapMode);

                if (mnWinStyle & WB_HORZ)
                    n = aPixSize.Width();
                else
                    n = aPixSize.Height();

                // Tick4 - Output (Text)
                double aStep = (nTick / nTick4);
                double aRest = std::abs(aStep - std::floor(aStep));
                double nAcceptanceDelta = 0.0001;

                if (aRest < nAcceptanceDelta)
                {
                    if ((mnUnitIndex == RULER_UNIT_CHAR) || (mnUnitIndex == RULER_UNIT_LINE))
                        aNumString = OUString::number(nTick / nTickUnit);
                    else
                        aNumString = OUString::number(nTick / aImplRulerUnitTab[mnUnitIndex].nTickUnit);

                    long nHorizontalLocation = nStart + n;
                    ImplVDrawText(rRenderContext, nHorizontalLocation, nCenter, aNumString, nMin, nMax);

                    if (nMin < nHorizontalLocation && nHorizontalLocation < nMax)
                    {
                        ImplVDrawRect(rRenderContext, nHorizontalLocation, nBottom, nHorizontalLocation + DPIOffset, nBottom - 1 * nScale);
                        ImplVDrawRect(rRenderContext, nHorizontalLocation, nTop,    nHorizontalLocation + DPIOffset, nTop + 1 * nScale);
                    }

                    nHorizontalLocation = nStart - n;
                    ImplVDrawText(rRenderContext, nHorizontalLocation, nCenter, aNumString, nMin, nMax);

                    if (nMin < nHorizontalLocation && nHorizontalLocation < nMax)
                    {
                        ImplVDrawRect(rRenderContext, nHorizontalLocation, nBottom,
                                                      nHorizontalLocation + DPIOffset, nBottom - 1 * nScale);
                        ImplVDrawRect(rRenderContext, nHorizontalLocation, nTop,
                                                      nHorizontalLocation + DPIOffset, nTop + 1 * nScale);
                    }
                }
                // Tick/Tick2 - Output (Strokes)
                else
                {
                    long nTickLength = nTickLength1;

                    aStep = (nTick / nTick2);
                    aRest = std::abs(aStep - std::floor(aStep));
                    if (aRest < nAcceptanceDelta)
                        nTickLength = nTickLength2;

                    aStep = (nTick / nTick3);
                    aRest = std::abs(aStep - std::floor(aStep));
                    if (aRest < nAcceptanceDelta )
                        nTickLength = nTickLength3;

                    if ((nTickLength == nTickLength1 && nTickGap1 > 6) ||
                        (nTickLength == nTickLength2 && nTickGap2 > 6) ||
                        (nTickLength == nTickLength3 && nTickGap3 > 6))
                    {
                        long nT1 = nCenter - (nTickLength / 2.0);
                        long nT2 = nT1 + nTickLength - 1;
                        long nT;

                        nT = nStart + n;

                        if (nT < nMax)
                            ImplVDrawRect(rRenderContext, nT, nT1, nT + DPIOffset, nT2);
                        nT = nStart - n;
                        if (nT > nMin)
                            ImplVDrawRect(rRenderContext, nT, nT1, nT + DPIOffset, nT2);
                    }
                }
            }
            nTick += nTickCount;
        }
    }
}

void Ruler::ImplDrawBorders(vcl::RenderContext& rRenderContext, long nMin, long nMax, long nVirTop, long nVirBottom)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    long    n;
    long    n1;
    long    n2;
    long    nTemp1;
    long    nTemp2;
    sal_uInt32  i;

    for (i = 0; i < mpData->pBorders.size(); i++)
    {
        if (mpData->pBorders[i].nStyle & RULER_STYLE_INVISIBLE)
            continue;

        n1 = mpData->pBorders[i].nPos + mpData->nNullVirOff;
        n2 = n1 + mpData->pBorders[i].nWidth;

        if (((n1 >= nMin) && (n1 <= nMax)) || ((n2 >= nMin) && (n2 <= nMax)))
        {
            if ((n2 - n1) > 3)
            {
                rRenderContext.SetLineColor();
                rRenderContext.SetFillColor(rStyleSettings.GetFaceColor());
                ImplVDrawRect(rRenderContext, n1, nVirTop, n2, nVirBottom);

                rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
                ImplVDrawLine(rRenderContext, n1 + 1, nVirTop, n1 + 1, nVirBottom);
                ImplVDrawLine(rRenderContext, n1,     nVirTop, n2,     nVirTop);

                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                ImplVDrawLine(rRenderContext, n1,     nVirTop,    n1,     nVirBottom);
                ImplVDrawLine(rRenderContext, n1,     nVirBottom, n2,     nVirBottom);
                ImplVDrawLine(rRenderContext, n2 - 1, nVirTop,    n2 - 1, nVirBottom);

                rRenderContext.SetLineColor(rStyleSettings.GetDarkShadowColor());
                ImplVDrawLine(rRenderContext, n2, nVirTop, n2, nVirBottom);

                if (mpData->pBorders[i].nStyle & RULER_BORDER_VARIABLE)
                {
                    if (n2 - n1 > RULER_VAR_SIZE + 4)
                    {
                        nTemp1 = n1 + (((n2 - n1 + 1) - RULER_VAR_SIZE) / 2);
                        nTemp2 = nVirTop + (((nVirBottom - nVirTop + 1) - RULER_VAR_SIZE) / 2);
                        long nTemp3 = nTemp1 + RULER_VAR_SIZE - 1;
                        long nTemp4 = nTemp2 + RULER_VAR_SIZE - 1;
                        long nTempY = nTemp2;

                        rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
                        while (nTempY <= nTemp4)
                        {
                            ImplVDrawLine(rRenderContext, nTemp1, nTempY, nTemp3, nTempY);
                            nTempY += 2;
                        }

                        nTempY = nTemp2 + 1;
                        rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                        while (nTempY <= nTemp4)
                        {
                            ImplVDrawLine(rRenderContext, nTemp1, nTempY, nTemp3, nTempY);
                            nTempY += 2;
                        }
                    }
                }

                if (mpData->pBorders[i].nStyle & RULER_BORDER_SIZEABLE)
                {
                    if (n2 - n1 > RULER_VAR_SIZE + 10)
                    {
                        rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                        ImplVDrawLine(rRenderContext, n1 + 4, nVirTop + 3, n1 + 4, nVirBottom - 3);
                        ImplVDrawLine(rRenderContext, n2 - 5, nVirTop + 3, n2 - 5, nVirBottom - 3);
                        rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
                        ImplVDrawLine(rRenderContext, n1 + 5, nVirTop + 3, n1 + 5, nVirBottom - 3);
                        ImplVDrawLine(rRenderContext, n2 - 4, nVirTop + 3, n2 - 4, nVirBottom - 3);
                    }
                }
            }
            else
            {
                n = n1 + ((n2 - n1) / 2);
                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());

                if (mpData->pBorders[i].nStyle & RULER_BORDER_SNAP)
                    ImplVDrawLine(rRenderContext, n, nVirTop, n, nVirBottom);
                else if (mpData->pBorders[i].nStyle & RULER_BORDER_MARGIN)
                    ImplVDrawLine(rRenderContext, n, nVirTop, n, nVirBottom);
                else
                {
                    ImplVDrawLine(rRenderContext, n - 1, nVirTop, n - 1, nVirBottom);
                    ImplVDrawLine(rRenderContext, n + 1, nVirTop, n + 1, nVirBottom);
                    rRenderContext.SetLineColor();
                    rRenderContext.SetFillColor(rStyleSettings.GetWindowColor());
                    ImplVDrawRect(rRenderContext, n, nVirTop, n, nVirBottom);
                }
            }
        }
    }
}

void Ruler::ImplDrawIndent(vcl::RenderContext& rRenderContext, const tools::Polygon& rPoly, sal_uInt16 nStyle, bool bIsHit)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    if (nStyle & RULER_STYLE_INVISIBLE)
        return;

    rRenderContext.SetLineColor(rStyleSettings.GetDarkShadowColor());
    rRenderContext.SetFillColor(bIsHit ? rStyleSettings.GetDarkShadowColor() : rStyleSettings.GetWorkspaceColor());
    rRenderContext.DrawPolygon(rPoly);
}

void Ruler::ImplDrawIndents(vcl::RenderContext& rRenderContext, long nMin, long nMax, long nVirTop, long nVirBottom)
{
    sal_uInt32  j;
    long n;
    long nIndentHeight = (mnVirHeight / 2) - 1;
    long nIndentWidth2 = nIndentHeight-3;

    tools::Polygon aPoly(5);

    for (j = 0; j < mpData->pIndents.size(); j++)
    {
        if (mpData->pIndents[j].nStyle & RULER_STYLE_INVISIBLE)
            continue;

        sal_uInt16  nStyle = mpData->pIndents[j].nStyle;
        sal_uInt16  nIndentStyle = nStyle & RULER_INDENT_STYLE;

        n = mpData->pIndents[j].nPos+mpData->nNullVirOff;

        if ((n >= nMin) && (n <= nMax))
        {
            if (nIndentStyle == RULER_INDENT_BORDER)
            {
                const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
                rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                ImplVDrawLine(rRenderContext, n, nVirTop + 1, n, nVirBottom - 1);
            }
            else if (nIndentStyle == RULER_INDENT_BOTTOM)
            {
                aPoly.SetPoint(Point(n + 0, nVirBottom - nIndentHeight), 0);
                aPoly.SetPoint(Point(n - nIndentWidth2, nVirBottom - 3), 1);
                aPoly.SetPoint(Point(n - nIndentWidth2, nVirBottom),     2);
                aPoly.SetPoint(Point(n + nIndentWidth2, nVirBottom),     3);
                aPoly.SetPoint(Point(n + nIndentWidth2, nVirBottom - 3), 4);
            }
            else
            {
                aPoly.SetPoint(Point(n + 0, nVirTop + nIndentHeight), 0);
                aPoly.SetPoint(Point(n - nIndentWidth2, nVirTop + 3), 1);
                aPoly.SetPoint(Point(n - nIndentWidth2, nVirTop),     2);
                aPoly.SetPoint(Point(n + nIndentWidth2, nVirTop),     3);
                aPoly.SetPoint(Point(n + nIndentWidth2, nVirTop + 3), 4);
            }

            if (0 == (mnWinStyle & WB_HORZ))
            {
                Point aTmp;
                for (sal_uInt16 i = 0; i < 5; i++)
                {
                    aTmp = aPoly[i];
                    Point aSet(nVirBottom - aTmp.Y(), aTmp.X());
                    aPoly[i] = aSet;
                }
            }
            if (RULER_INDENT_BORDER != nIndentStyle)
            {
                bool bIsHit = false;
                if(mxCurrentHitTest.get() != nullptr && mxCurrentHitTest->eType == RULER_TYPE_INDENT)
                {
                    bIsHit = mxCurrentHitTest->nAryPos == j;
                }
                else if(mbDrag && meDragType == RULER_TYPE_INDENT)
                {
                    bIsHit = mnDragAryPos == j;
                }
                ImplDrawIndent(rRenderContext, aPoly, nStyle, bIsHit);
            }
        }
    }
}

static void ImplCenterTabPos(Point& rPos, sal_uInt16 nTabStyle)
{
    bool bRTL  = 0 != (nTabStyle & RULER_TAB_RTL);
    nTabStyle &= RULER_TAB_STYLE;
    rPos.Y() += ruler_tab.height/2;

    if ( (!bRTL && nTabStyle == RULER_TAB_LEFT) ||
         ( bRTL && nTabStyle == RULER_TAB_RIGHT) )
    {
        rPos.X() -= ruler_tab.width / 2;
    }
    else if ( (!bRTL && nTabStyle == RULER_TAB_RIGHT) ||
              ( bRTL && nTabStyle == RULER_TAB_LEFT) )
    {
        rPos.X() += ruler_tab.width / 2;
    }
}

static void lcl_RotateRect_Impl(Rectangle& rRect, const long nReference, bool bRightAligned)
{
    if (!rRect.IsEmpty())
    {
        Rectangle aTmp(rRect);
        rRect.Top()    = aTmp.Left();
        rRect.Bottom() = aTmp.Right();
        rRect.Left()   = aTmp.Top();
        rRect.Right()  = aTmp.Bottom();

        if (bRightAligned)
        {
            long nRef = 2 * nReference;
            rRect.Left() = nRef - rRect.Left();
            rRect.Right() = nRef - rRect.Right();
        }
    }
}

static void ImplDrawRulerTab(vcl::RenderContext& rRenderContext, const Point& rPos,
                              sal_uInt16 nStyle, WinBits nWinBits)
{
    if (nStyle & RULER_STYLE_INVISIBLE)
        return;

    sal_uInt16 nTabStyle = nStyle & RULER_TAB_STYLE;
    bool bRTL = 0 != (nStyle & RULER_TAB_RTL);

    // Scale by the screen DPI scaling factor
    // However when doing this some of the rectangles
    // drawn become asymmetric due to the +1 offsets
    sal_uInt16 DPIOffset = rRenderContext.GetDPIScaleFactor() - 1;

    Rectangle aRect1;
    Rectangle aRect2;
    Rectangle aRect3;

    aRect3.SetEmpty();

    if (nTabStyle == RULER_TAB_DEFAULT)
    {
        aRect1.Left()   = rPos.X() - ruler_tab.dwidth2 + 1;
        aRect1.Top()    = rPos.Y() - ruler_tab.dheight2 + 1;
        aRect1.Right()  = rPos.X() - ruler_tab.dwidth2 + ruler_tab.dwidth + DPIOffset;
        aRect1.Bottom() = rPos.Y();

        aRect2.Left()   = rPos.X() - ruler_tab.dwidth2 + ruler_tab.dwidth3;
        aRect2.Top()    = rPos.Y() - ruler_tab.dheight + 1;
        aRect2.Right()  = rPos.X() - ruler_tab.dwidth2 + ruler_tab.dwidth3 + ruler_tab.dwidth4 - 1;
        aRect2.Bottom() = rPos.Y();

    }
    else if ((!bRTL && nTabStyle == RULER_TAB_LEFT) || (bRTL && nTabStyle == RULER_TAB_RIGHT))
    {
        aRect1.Left()   = rPos.X();
        aRect1.Top()    = rPos.Y() - ruler_tab.height2 + 1;
        aRect1.Right()  = rPos.X() + ruler_tab.width - 1;
        aRect1.Bottom() = rPos.Y();

        aRect2.Left()   = rPos.X();
        aRect2.Top()    = rPos.Y() - ruler_tab.height + 1;
        aRect2.Right()  = rPos.X() + ruler_tab.width2 - 1;
        aRect2.Bottom() = rPos.Y();
    }
    else if ((!bRTL && nTabStyle == RULER_TAB_RIGHT) || (bRTL && nTabStyle == RULER_TAB_LEFT))
    {
        aRect1.Left()   = rPos.X() - ruler_tab.width + 1;
        aRect1.Top()    = rPos.Y() - ruler_tab.height2 + 1;
        aRect1.Right()  = rPos.X();
        aRect1.Bottom() = rPos.Y();

        aRect2.Left()   = rPos.X() - ruler_tab.width2 + 1;
        aRect2.Top()    = rPos.Y() - ruler_tab.height + 1;
        aRect2.Right()  = rPos.X();
        aRect2.Bottom() = rPos.Y();
    }
    else
    {
        aRect1.Left()   = rPos.X() - ruler_tab.cwidth2 + 1;
        aRect1.Top()    = rPos.Y() - ruler_tab.height2 + 1;
        aRect1.Right()  = rPos.X() - ruler_tab.cwidth2 + ruler_tab.cwidth + DPIOffset;
        aRect1.Bottom() = rPos.Y();

        aRect2.Left()   = rPos.X() - ruler_tab.cwidth2 + ruler_tab.cwidth3;
        aRect2.Top()    = rPos.Y() - ruler_tab.height + 1;
        aRect2.Right()  = rPos.X() - ruler_tab.cwidth2 + ruler_tab.cwidth3 + ruler_tab.cwidth4 - 1;
        aRect2.Bottom() = rPos.Y();

        if (nTabStyle == RULER_TAB_DECIMAL)
        {
            aRect3.Left()   = rPos.X() - ruler_tab.cwidth2 + ruler_tab.cwidth - 1;
            aRect3.Top()    = rPos.Y() - ruler_tab.height + 1 + 1 - DPIOffset;
            aRect3.Right()  = rPos.X() - ruler_tab.cwidth2 + ruler_tab.cwidth + DPIOffset;
            aRect3.Bottom() = rPos.Y() - ruler_tab.height + 1 + 2;
        }
    }
    if (0 == (nWinBits & WB_HORZ))
    {
        bool bRightAligned = 0 != (nWinBits & WB_RIGHT_ALIGNED);
        lcl_RotateRect_Impl(aRect1, rPos.Y(), bRightAligned);
        lcl_RotateRect_Impl(aRect2, rPos.Y(), bRightAligned);
        lcl_RotateRect_Impl(aRect3, rPos.Y(), bRightAligned);
    }
    rRenderContext.DrawRect(aRect1);
    rRenderContext.DrawRect(aRect2);

    if (!aRect3.IsEmpty())
        rRenderContext.DrawRect(aRect3);
}

void Ruler::ImplDrawTab(vcl::RenderContext& rRenderContext, const Point& rPos, sal_uInt16 nStyle)
{
    if (nStyle & RULER_STYLE_INVISIBLE)
        return;

    rRenderContext.SetLineColor();

    if (nStyle & RULER_STYLE_DONTKNOW)
        rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetFaceColor());
    else
        rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetDarkShadowColor());

    if (mpData->bTextRTL)
        nStyle |= RULER_TAB_RTL;

    ImplDrawRulerTab(rRenderContext, rPos, nStyle, GetStyle());
}

void Ruler::ImplDrawTabs(vcl::RenderContext& rRenderContext, long nMin, long nMax, long nVirTop, long nVirBottom)
{
    for (size_t i = 0; i < mpData->pTabs.size(); i++)
    {
        if (mpData->pTabs[i].nStyle & RULER_STYLE_INVISIBLE)
            continue;

        long aPosition;
        aPosition = mpData->pTabs[i].nPos;
        aPosition += +mpData->nNullVirOff;
        long nTopBottom = (GetStyle() & WB_RIGHT_ALIGNED) ? nVirTop : nVirBottom;
        if (nMin <= aPosition && aPosition <= nMax)
            ImplDrawTab(rRenderContext, Point( aPosition, nTopBottom ), mpData->pTabs[i].nStyle);
    }
}

static int adjustSize(int nOrig)
{
    if (nOrig <= 0)
        return 0;

    // make sure we return an odd number, that looks better in the ruler
    return ( (3*nOrig) / 8) * 2 + 1;
}

void Ruler::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    vcl::Font aFont = rStyleSettings.GetToolFont();
    // make the font a bit smaller than default
    Size aSize(adjustSize(aFont.GetFontSize().Width()), adjustSize(aFont.GetFontSize().Height()));
    aFont.SetFontSize(aSize);

    ApplyControlFont(rRenderContext, aFont);

    ApplyControlForeground(*this, rStyleSettings.GetDarkShadowColor());
    SetTextFillColor();

    Color aColor;
    svtools::ColorConfig aColorConfig;
    aColor = Color(aColorConfig.GetColorValue(svtools::APPBACKGROUND).nColor);
    ApplyControlBackground(rRenderContext, aColor);
}

void Ruler::ImplInitSettings(bool bFont, bool bForeground, bool bBackground)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if (bFont)
    {
        vcl::Font aFont = rStyleSettings.GetToolFont();
        // make the font a bit smaller than default
        Size aSize(adjustSize(aFont.GetFontSize().Width()), adjustSize(aFont.GetFontSize().Height()));
        aFont.SetFontSize(aSize);

        ApplyControlFont(*this, aFont);
    }

    if (bForeground || bFont)
    {
        ApplyControlForeground(*this, rStyleSettings.GetDarkShadowColor());
        SetTextFillColor();
    }

    if (bBackground)
    {
        Color aColor;
        svtools::ColorConfig aColorConfig;
        aColor = Color(aColorConfig.GetColorValue(svtools::APPBACKGROUND).nColor);
        ApplyControlBackground(*this, aColor);
    }

    maVirDev->SetSettings( GetSettings() );
    maVirDev->SetBackground( GetBackground() );
    vcl::Font aFont = GetFont();

    if (mnWinStyle & WB_VERT)
        aFont.SetOrientation(900);

    maVirDev->SetFont(aFont);
    maVirDev->SetTextColor(GetTextColor());
    maVirDev->SetTextFillColor(GetTextFillColor());
}

void Ruler::ImplCalc()
{
    // calculate offset
    mpData->nRulVirOff = mnWinOff + mpData->nPageOff;
    if ( mpData->nRulVirOff > mnVirOff )
        mpData->nRulVirOff -= mnVirOff;
    else
        mpData->nRulVirOff = 0;
    long nRulWinOff = mpData->nRulVirOff+mnVirOff;

    // calculate non-visual part of the page
    long nNotVisPageWidth;
    if ( mpData->nPageOff < 0 )
    {
        nNotVisPageWidth = -(mpData->nPageOff);
        if ( nRulWinOff < mnWinOff )
            nNotVisPageWidth -= mnWinOff-nRulWinOff;
    }
    else
        nNotVisPageWidth = 0;

    // calculate width
    if ( mnWinStyle & WB_HORZ )
    {
        if ( mbAutoWinWidth )
            mnWinWidth = mnWidth - mnVirOff;
        if ( mpData->bAutoPageWidth )
            mpData->nPageWidth = mnWinWidth;
        mpData->nRulWidth = std::min( mnWinWidth, mpData->nPageWidth-nNotVisPageWidth );
        if ( nRulWinOff+mpData->nRulWidth > mnWidth )
            mpData->nRulWidth = mnWidth-nRulWinOff;
    }
    else
    {
        if ( mbAutoWinWidth )
            mnWinWidth = mnHeight - mnVirOff;
        if ( mpData->bAutoPageWidth )
            mpData->nPageWidth = mnWinWidth;
        mpData->nRulWidth = std::min( mnWinWidth, mpData->nPageWidth-nNotVisPageWidth );
        if ( nRulWinOff+mpData->nRulWidth > mnHeight )
            mpData->nRulWidth = mnHeight-nRulWinOff;
    }

    mbCalc = false;
}

void Ruler::ImplFormat(vcl::RenderContext& rRenderContext)
{
    // if already formatted, don't do it again
    if (!mbFormat)
        return;

    // don't do anything if the window still has no size
    if (!mnVirWidth)
        return;

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    long    nP1;            // pixel position of Page1
    long    nP2;            // pixel position of Page2
    long    nM1;            // pixel position of Margin1
    long    nM2;            // pixel position of Margin2
    long    nVirTop;        // top/left corner
    long    nVirBottom;     // bottom/right corner
    long    nVirLeft;       // left/top corner
    long    nVirRight;      // right/bottom corner
    long    nNullVirOff;    // for faster calculation

    // calculate values
    if (mbCalc)
        ImplCalc();

    mpData->nNullVirOff = mnWinOff + mpData->nPageOff + mpData->nNullOff - mnVirOff;

    nNullVirOff = mpData->nNullVirOff;
    nVirLeft    = mpData->nRulVirOff;
    nVirRight   = nVirLeft + mpData->nRulWidth - 1;
    nVirTop     = 0;
    nVirBottom  = mnVirHeight - 1;

    if (!IsReallyVisible())
        return;

    Size aVirDevSize;

    // initialize VirtualDevice
    if (mnWinStyle & WB_HORZ)
    {
        aVirDevSize.Width() = mnVirWidth;
        aVirDevSize.Height() = mnVirHeight;
    }
    else
    {
        aVirDevSize.Height() = mnVirWidth;
        aVirDevSize.Width() = mnVirHeight;
    }
    if (aVirDevSize != maVirDev->GetOutputSizePixel())
        maVirDev->SetOutputSizePixel(aVirDevSize);
    else
        maVirDev->Erase();

    // calculate margins
    if (!(mpData->nMargin1Style & RULER_STYLE_INVISIBLE))
    {
        nM1 = mpData->nMargin1 + nNullVirOff;
        if (mpData->bAutoPageWidth)
        {
            nP1 = nVirLeft;
            if (nM1 < nVirLeft)
                nP1--;
        }
        else
            nP1 = nNullVirOff - mpData->nNullOff;
    }
    else
    {
        nM1 = nVirLeft-1;
        nP1 = nM1;
    }
    if (!(mpData->nMargin2Style & RULER_STYLE_INVISIBLE))
    {
        nM2 = mpData->nMargin2 + nNullVirOff;
        if (mpData->bAutoPageWidth)
        {
            nP2 = nVirRight;
            if (nM2 > nVirRight)
                nP2++;
        }
        else
            nP2 = nNullVirOff - mpData->nNullOff + mpData->nPageWidth;
        if (nM2 > nP2)
            nM2 = nP2;
    }
    else
    {
        nM2 = nVirRight+1;
        nP2 = nM2;
    }

    // top/bottom border
    maVirDev->SetLineColor(rStyleSettings.GetShadowColor());
    ImplVDrawLine(*maVirDev.get(), nVirLeft, nVirTop + 1, nM1,     nVirTop + 1); //top left line
    ImplVDrawLine(*maVirDev.get(), nM2,      nVirTop + 1, nP2 - 1, nVirTop + 1); //top right line

    nVirTop++;
    nVirBottom--;

    // draw margin1, margin2 and in-between
    maVirDev->SetLineColor();
    maVirDev->SetFillColor(rStyleSettings.GetDialogColor());
    if (nM1 > nVirLeft)
        ImplVDrawRect(*maVirDev.get(), nP1, nVirTop + 1, nM1, nVirBottom); //left gray rectangle
    if (nM2 < nP2)
        ImplVDrawRect(*maVirDev.get(), nM2, nVirTop + 1, nP2, nVirBottom); //right gray rectangle
    if (nM2 - nM1 > 0)
    {
        maVirDev->SetFillColor(rStyleSettings.GetWindowColor());
        ImplVDrawRect(*maVirDev.get(), nM1 + 1, nVirTop, nM2 - 1, nVirBottom); //center rectangle
    }
    maVirDev->SetLineColor(rStyleSettings.GetShadowColor());
    if (nM1 > nVirLeft)
    {
        ImplVDrawLine(*maVirDev.get(), nM1, nVirTop + 1, nM1, nVirBottom); //right line of the left rectangle
        ImplVDrawLine(*maVirDev.get(), nP1, nVirBottom,  nM1, nVirBottom); //bottom line of the left rectangle
        if (nP1 >= nVirLeft)
        {
            ImplVDrawLine(*maVirDev.get(), nP1, nVirTop + 1, nP1,     nVirBottom); //left line of the left rectangle
            ImplVDrawLine(*maVirDev.get(), nP1, nVirBottom,  nP1 + 1, nVirBottom); //?
        }
    }
    if (nM2 < nP2)
    {
        ImplVDrawLine(*maVirDev.get(), nM2, nVirBottom,  nP2 - 1, nVirBottom); //bottom line of the right rectangle
        ImplVDrawLine(*maVirDev.get(), nM2, nVirTop + 1, nM2,     nVirBottom); //left line of the right rectangle
        if (nP2 <= nVirRight + 1)
            ImplVDrawLine(*maVirDev.get(), nP2 - 1, nVirTop + 1, nP2 - 1, nVirBottom); //right line of the right rectangle
    }

    long nMin = nVirLeft;
    long nMax = nP2;
    long nStart = 0;

    if (mpData->bTextRTL)
        nStart = mpData->nRightFrameMargin + nNullVirOff;
    else
        nStart = mpData->nLeftFrameMargin + nNullVirOff;

    if (nP1 > nVirLeft)
        nMin++;

    if (nP2 < nVirRight)
        nMax--;

    // Draw captions
    ImplDrawTicks(*maVirDev.get(), nMin, nMax, nStart, nVirTop, nVirBottom);

    // Draw borders
    if (!mpData->pBorders.empty())
        ImplDrawBorders(*maVirDev.get(), nVirLeft, nP2, nVirTop, nVirBottom);

    // Draw indents
    if (!mpData->pIndents.empty())
        ImplDrawIndents(*maVirDev.get(), nVirLeft, nP2, nVirTop - 1, nVirBottom + 1);

    // Tabs
    if (!mpData->pTabs.empty())
        ImplDrawTabs(*maVirDev.get(), nVirLeft, nP2, nVirTop-1, nVirBottom + 1);

    mbFormat = false;
}

void Ruler::ImplInitExtraField( bool bUpdate )
{
    Size aWinSize = GetOutputSizePixel();

    // extra field evaluate
    if ( mnWinStyle & WB_EXTRAFIELD )
    {
        maExtraRect.Left()   = RULER_OFF;
        maExtraRect.Top()    = RULER_OFF;
        maExtraRect.Right()  = RULER_OFF + mnVirHeight - 1;
        maExtraRect.Bottom() = RULER_OFF + mnVirHeight - 1;
        if(mpData->bTextRTL)
        {
            if(mnWinStyle & WB_HORZ)
                maExtraRect.Move(aWinSize.Width() - maExtraRect.GetWidth() - maExtraRect.Left(), 0);
            else
                maExtraRect.Move(0, aWinSize.Height() - maExtraRect.GetHeight() - maExtraRect.Top());
            mnVirOff = 0;
        }
        else
            mnVirOff = maExtraRect.Right()+1;

    }
    else
    {
        maExtraRect.SetEmpty();
        mnVirOff = 0;
    }

    // mnVirWidth depends on mnVirOff
    if ( (mnVirWidth > RULER_MIN_SIZE) ||
     ((aWinSize.Width() > RULER_MIN_SIZE) && (aWinSize.Height() > RULER_MIN_SIZE)) )
    {
        if ( mnWinStyle & WB_HORZ )
            mnVirWidth = aWinSize.Width()-mnVirOff;
        else
            mnVirWidth = aWinSize.Height()-mnVirOff;

        if ( mnVirWidth < RULER_MIN_SIZE )
            mnVirWidth = 0;
    }

    if ( bUpdate )
    {
        mbCalc      = true;
        mbFormat    = true;
        Invalidate();
    }
}

void Ruler::ImplDraw(vcl::RenderContext& rRenderContext)
{
    if (mbFormat)
    {
        ImplFormat(rRenderContext);
    }

    if (IsReallyVisible())
    {
        // output the ruler to the virtual device
        Point aOffPos;
        Size aVirDevSize = maVirDev->GetOutputSizePixel();

        if (mnWinStyle & WB_HORZ)
        {
            aOffPos.X() = mnVirOff;
            if (mpData->bTextRTL)
                aVirDevSize.Width() -= maExtraRect.GetWidth();

            aOffPos.Y() = RULER_OFF;
        }
        else
        {
            aOffPos.X() = RULER_OFF;
            aOffPos.Y() = mnVirOff;
        }
        rRenderContext.DrawOutDev(aOffPos, aVirDevSize, Point(), aVirDevSize, *maVirDev.get());

        // redraw positionlines
        ImplInvertLines(rRenderContext);
    }
}

void Ruler::ImplDrawExtra(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    Rectangle aRect = maExtraRect;
    bool bEraseRect = false;

    aRect.Left()   += 2;
    aRect.Top()    += 2;
    aRect.Right()  -= 2;
    aRect.Bottom() -= 2;

    if (mnExtraStyle & RULER_STYLE_HIGHLIGHT)
    {
        rRenderContext.SetFillColor(rStyleSettings.GetCheckedColor());
        bEraseRect = true;
    }

    if (bEraseRect)
    {
        rRenderContext.SetLineColor();
        rRenderContext.DrawRect(aRect);
    }

    // output content
    if (meExtraType == RULER_EXTRA_NULLOFFSET)
    {
        rRenderContext.SetLineColor(rStyleSettings.GetButtonTextColor());
        rRenderContext.DrawLine(Point(aRect.Left() + 1, aRect.Top() + 4),
                                Point(aRect.Right() - 1, aRect.Top() + 4));
        rRenderContext.DrawLine(Point(aRect.Left() + 4, aRect.Top() + 1),
                                Point(aRect.Left() + 4, aRect.Bottom() - 1));
    }
    else if (meExtraType == RULER_EXTRA_TAB)
    {
        sal_uInt16 nTabStyle = mnExtraStyle & RULER_TAB_STYLE;
        if (mpData->bTextRTL)
            nTabStyle |= RULER_TAB_RTL;
        Point aCenter = aRect.Center();
        Point aDraw(aCenter);
        ImplCenterTabPos(aDraw, nTabStyle);
        WinBits nWinBits = GetStyle();
        if (0 == (nWinBits & WB_HORZ))
        {
            if ((nWinBits & WB_RIGHT_ALIGNED) != 0)
                aDraw.Y() = 2 * aCenter.Y() - aDraw.Y();

            if (mpData->bTextRTL)
            {
                long nTemp = aDraw.X();
                aDraw.X() = aDraw.Y();
                aDraw.Y() = nTemp;
            }
        }
        ImplDrawTab(rRenderContext, aDraw, nTabStyle);
    }
}

void Ruler::ImplUpdate( bool bMustCalc )
{
    // clear lines in this place so they aren't considered at recalculation
    if (!mbFormat)
        Invalidate(InvalidateFlags::NoErase);

    // set flags
    if (bMustCalc)
        mbCalc = true;
    mbFormat = true;

    // abort if we are dragging as drag-handler will update the ruler after drag is finished
    if (mbDrag)
        return;

    // otherwise trigger update
    if (IsReallyVisible() && IsUpdateMode())
    {
        Invalidate(InvalidateFlags::NoErase);
    }
}

bool Ruler::ImplHitTest( const Point& rPos, RulerSelection* pHitTest,
                         bool bRequireStyle, sal_uInt16 nRequiredStyle ) const
{
    sal_Int32   i;
    sal_uInt16  nStyle;
    long        nHitBottom;
    long        nX;
    long        nY;
    long        n1;

    if ( !mbActive )
        return false;

    // determine positions
    bool bIsHori = 0 != (mnWinStyle & WB_HORZ);
    if ( bIsHori )
    {
        nX = rPos.X();
        nY = rPos.Y();
    }
    else
    {
        nX = rPos.Y();
        nY = rPos.X();
    }
    nHitBottom = mnVirHeight + (RULER_OFF * 2);

    // #i32608#
    pHitTest->nAryPos = 0;
    pHitTest->mnDragSize = 0;
    pHitTest->bSize = false;
    pHitTest->bSizeBar = false;

    // so that leftover tabs and indents are taken into account
    long nXExtraOff;
    if ( !mpData->pTabs.empty() || !mpData->pIndents.empty() )
        nXExtraOff = (mnVirHeight / 2) - 4;
    else
        nXExtraOff = 0;

    // test if outside
    nX -= mnVirOff;
    long nXTemp = nX;
    if ( (nX < mpData->nRulVirOff - nXExtraOff) ||
         (nX > mpData->nRulVirOff + mpData->nRulWidth + nXExtraOff) ||
         (nY < 0) ||
         (nY > nHitBottom) )
    {
        pHitTest->nPos = 0;
        pHitTest->eType = RULER_TYPE_OUTSIDE;
        return false;
    }

    nX -= mpData->nNullVirOff;
    pHitTest->nPos  = nX;
    pHitTest->eType = RULER_TYPE_DONTKNOW;

    // first test the tabs
    Rectangle aRect;
    if ( !mpData->pTabs.empty() )
    {
        aRect.Bottom()  = nHitBottom;
        aRect.Top()     = aRect.Bottom() - ruler_tab.height - RULER_OFF;

        for ( i = mpData->pTabs.size() - 1; i >= 0; i-- )
        {
            nStyle = mpData->pTabs[i].nStyle;
            if ( !(nStyle & RULER_STYLE_INVISIBLE) )
            {
                nStyle &= RULER_TAB_STYLE;

                // default tabs are only shown (no action)
                if ( nStyle != RULER_TAB_DEFAULT )
                {
                    n1 = mpData->pTabs[i].nPos;

                    if ( nStyle == RULER_TAB_LEFT )
                    {
                        aRect.Left()  = n1;
                        aRect.Right() = n1 + ruler_tab.width - 1;
                    }
                    else if ( nStyle == RULER_TAB_RIGHT )
                    {
                        aRect.Right() = n1;
                        aRect.Left()  = n1 - ruler_tab.width - 1;
                    }
                    else
                    {
                        aRect.Left()  = n1 - ruler_tab.cwidth2 + 1;
                        aRect.Right() = n1 - ruler_tab.cwidth2 + ruler_tab.cwidth;
                    }

                    if ( aRect.IsInside( Point( nX, nY ) ) )
                    {
                        pHitTest->eType   = RULER_TYPE_TAB;
                        pHitTest->nAryPos = i;
                        return true;
                    }
                }
            }
        }
    }

    // Indents
    if ( !mpData->pIndents.empty() )
    {
        long nIndentHeight = (mnVirHeight / 2) - 1;
        long nIndentWidth2 = nIndentHeight - 3;

        for ( i = mpData->pIndents.size(); i; i-- )
        {
            nStyle = mpData->pIndents[i-1].nStyle;
            if ( (! bRequireStyle || nStyle == nRequiredStyle) &&
                 !(nStyle & RULER_STYLE_INVISIBLE) )
            {
                nStyle &= RULER_INDENT_STYLE;
                n1 = mpData->pIndents[i-1].nPos;

                if ( (nStyle == RULER_INDENT_BOTTOM) != !bIsHori )
                {
                    aRect.Left()    = n1-nIndentWidth2;
                    aRect.Right()   = n1+nIndentWidth2;
                    aRect.Top()     = nHitBottom-nIndentHeight-RULER_OFF+1;
                    aRect.Bottom()  = nHitBottom;
                }
                else
                {
                    aRect.Left()    = n1-nIndentWidth2;
                    aRect.Right()   = n1+nIndentWidth2;
                    aRect.Top()     = 0;
                    aRect.Bottom()  = nIndentHeight+RULER_OFF-1;
                }

                if ( aRect.IsInside( Point( nX, nY ) ) )
                {
                    pHitTest->eType     = RULER_TYPE_INDENT;
                    pHitTest->nAryPos   = i-1;
                    return true;
                }
            }
        }
    }

    // everything left and right is outside and don't take this into account
    if ( (nXTemp < mpData->nRulVirOff) || (nXTemp > mpData->nRulVirOff+mpData->nRulWidth) )
    {
        pHitTest->nPos = 0;
        pHitTest->eType = RULER_TYPE_OUTSIDE;
        return false;
    }

    // test the borders
    int nBorderTolerance = 1;
    if(pHitTest->bExpandTest)
    {
        nBorderTolerance++;
    }

    for ( i = mpData->pBorders.size(); i; i-- )
    {
        n1 = mpData->pBorders[i-1].nPos;
        long n2 = n1 + mpData->pBorders[i-1].nWidth;

        // borders have at least 3 pixel padding
        if ( !mpData->pBorders[i-1].nWidth )
        {
             n1 -= nBorderTolerance;
             n2 += nBorderTolerance;

        }

        if ( (nX >= n1) && (nX <= n2) )
        {
            nStyle = mpData->pBorders[i-1].nStyle;
            if ( !(nStyle & RULER_STYLE_INVISIBLE) )
            {
                pHitTest->eType     = RULER_TYPE_BORDER;
                pHitTest->nAryPos   = i-1;

                if ( !(nStyle & RULER_BORDER_SIZEABLE) )
                {
                    if ( nStyle & RULER_BORDER_MOVEABLE )
                    {
                        pHitTest->bSizeBar = true;
                        pHitTest->mnDragSize = RULER_DRAGSIZE_MOVE;
                    }
                }
                else
                {
                    long nMOff = RULER_MOUSE_BORDERWIDTH;
                    while ( nMOff*2 >= (n2-n1-RULER_MOUSE_BORDERMOVE) )
                    {
                        if ( nMOff < 2 )
                        {
                            nMOff = 0;
                            break;
                        }
                        else
                            nMOff--;
                    }

                    if ( nX <= n1+nMOff )
                    {
                        pHitTest->bSize = true;
                        pHitTest->mnDragSize = RULER_DRAGSIZE_1;
                    }
                    else if ( nX >= n2-nMOff )
                    {
                        pHitTest->bSize = true;
                        pHitTest->mnDragSize = RULER_DRAGSIZE_2;
                    }
                    else
                    {
                        if ( nStyle & RULER_BORDER_MOVEABLE )
                        {
                            pHitTest->bSizeBar = true;
                            pHitTest->mnDragSize = RULER_DRAGSIZE_MOVE;
                        }
                    }
                }

                return true;
            }
        }
    }

    // Margins
    int nMarginTolerance = pHitTest->bExpandTest ? nBorderTolerance : RULER_MOUSE_MARGINWIDTH;

    if ( (mpData->nMargin1Style & (RULER_MARGIN_SIZEABLE | RULER_STYLE_INVISIBLE)) == RULER_MARGIN_SIZEABLE )
    {
        n1 = mpData->nMargin1;
        if ( (nX >= n1 - nMarginTolerance) && (nX <= n1 + nMarginTolerance) )
        {
            pHitTest->eType = RULER_TYPE_MARGIN1;
            pHitTest->bSize = true;
            return true;
        }
    }
    if ( (mpData->nMargin2Style & (RULER_MARGIN_SIZEABLE | RULER_STYLE_INVISIBLE)) == RULER_MARGIN_SIZEABLE )
    {
        n1 = mpData->nMargin2;
        if ( (nX >= n1 - nMarginTolerance) && (nX <= n1 + nMarginTolerance) )
        {
            pHitTest->eType = RULER_TYPE_MARGIN2;
            pHitTest->bSize = true;
            return true;
        }
    }

    // test tabs again
    if ( !mpData->pTabs.empty() )
    {
        aRect.Top()     = RULER_OFF;
        aRect.Bottom()  = nHitBottom;

        for ( i = mpData->pTabs.size() - 1; i >= 0; i-- )
        {
            nStyle = mpData->pTabs[i].nStyle;
            if ( !(nStyle & RULER_STYLE_INVISIBLE) )
            {
                nStyle &= RULER_TAB_STYLE;

                // default tabs are only shown (no action)
                if ( nStyle != RULER_TAB_DEFAULT )
                {
                    n1 = mpData->pTabs[i].nPos;

                    if ( nStyle == RULER_TAB_LEFT )
                    {
                        aRect.Left()  = n1;
                        aRect.Right() = n1 + ruler_tab.width - 1;
                    }
                    else if ( nStyle == RULER_TAB_RIGHT )
                    {
                        aRect.Right() = n1;
                        aRect.Left()  = n1 - ruler_tab.width - 1;
                    }
                    else
                    {
                        aRect.Left()  = n1 - ruler_tab.cwidth2 + 1;
                        aRect.Right() = n1 - ruler_tab.cwidth2 + ruler_tab.cwidth;
                    }

                    aRect.Left()--;
                    aRect.Right()++;

                    if ( aRect.IsInside( Point( nX, nY ) ) )
                    {
                        pHitTest->eType   = RULER_TYPE_TAB;
                        pHitTest->nAryPos = i;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool Ruler::ImplDocHitTest( const Point& rPos, RulerType eDragType,
                                RulerSelection* pHitTest ) const
{
    Point aPos = rPos;
    bool bRequiredStyle = false;
    sal_uInt16 nRequiredStyle = 0;

    if (eDragType == RULER_TYPE_INDENT)
    {
        bRequiredStyle = true;
        nRequiredStyle = RULER_INDENT_BOTTOM;
    }

    if ( mnWinStyle & WB_HORZ )
        aPos.X() += mnWinOff;
    else
        aPos.Y() += mnWinOff;

    if ( (eDragType == RULER_TYPE_INDENT) || (eDragType == RULER_TYPE_DONTKNOW) )
    {
        if ( mnWinStyle & WB_HORZ )
            aPos.Y() = RULER_OFF + 1;
        else
            aPos.X() = RULER_OFF + 1;

        if ( ImplHitTest( aPos, pHitTest, bRequiredStyle, nRequiredStyle ) )
        {
            if ( (pHitTest->eType == eDragType) || (eDragType == RULER_TYPE_DONTKNOW) )
                return true;
        }
    }

    if ( (eDragType == RULER_TYPE_INDENT) ||
         (eDragType == RULER_TYPE_TAB) ||
         (eDragType == RULER_TYPE_DONTKNOW) )
    {
        if ( mnWinStyle & WB_HORZ )
            aPos.Y() = mnHeight - RULER_OFF - 1;
        else
            aPos.X() = mnWidth - RULER_OFF - 1;

        if ( ImplHitTest( aPos, pHitTest, bRequiredStyle, nRequiredStyle ) )
        {
            if ( (pHitTest->eType == eDragType) || (eDragType == RULER_TYPE_DONTKNOW) )
                return true;
        }
    }

    if ( (eDragType == RULER_TYPE_MARGIN1) || (eDragType == RULER_TYPE_MARGIN2) ||
         (eDragType == RULER_TYPE_BORDER) || (eDragType == RULER_TYPE_DONTKNOW) )
    {
        if ( mnWinStyle & WB_HORZ )
            aPos.Y() = RULER_OFF + (mnVirHeight / 2);
        else
            aPos.X() = RULER_OFF + (mnVirHeight / 2);

        if ( ImplHitTest( aPos, pHitTest ) )
        {
            if ( (pHitTest->eType == eDragType) || (eDragType == RULER_TYPE_DONTKNOW) )
                return true;
        }
    }

    pHitTest->eType = RULER_TYPE_DONTKNOW;

    return false;
}

bool Ruler::ImplStartDrag( RulerSelection* pHitTest, sal_uInt16 nModifier )
{
    // don't trigger drag if a border that was clicked can not be changed
    if ( (pHitTest->eType == RULER_TYPE_BORDER) &&
         !pHitTest->bSize && !pHitTest->bSizeBar )
        return false;

    // Set drag data
    meDragType      = pHitTest->eType;
    mnDragPos       = pHitTest->nPos;
    mnDragAryPos    = pHitTest->nAryPos;
    mnDragSize      = pHitTest->mnDragSize;
    mnDragModifier  = nModifier;
    *mpDragData     = *mpSaveData;
    mpData          = mpDragData;

    // call handler
    if (StartDrag())
    {
        // if the handler allows dragging, initialize dragging
        mbDrag = true;
        mnStartDragPos = mnDragPos;
        StartTracking();
        Invalidate(InvalidateFlags::NoErase);
        return true;
    }
    else
    {
        // otherwise reset the data
        meDragType      = RULER_TYPE_DONTKNOW;
        mnDragPos       = 0;
        mnDragAryPos    = 0;
        mnDragSize      = 0;
        mnDragModifier  = 0;
        mpData          = mpSaveData;
    }

    return false;
}

void Ruler::ImplDrag( const Point& rPos )
{
    long  nX;
    long  nY;
    long  nOutHeight;

    if ( mnWinStyle & WB_HORZ )
    {
        nX          = rPos.X();
        nY          = rPos.Y();
        nOutHeight  = mnHeight;
    }
    else
    {
        nX          = rPos.Y();
        nY          = rPos.X();
        nOutHeight  = mnWidth;
    }

    // calculate and fit X
    nX -= mnVirOff;
    if ( nX < mpData->nRulVirOff )
    {
        nX = mpData->nRulVirOff;
    }
    else if ( nX > mpData->nRulVirOff+mpData->nRulWidth )
    {
        nX = mpData->nRulVirOff+mpData->nRulWidth;
    }
    nX -= mpData->nNullVirOff;

    // if upper or left from ruler, then consider old values
    mbDragDelete = false;
    if ( nY < 0 )
    {
        if ( !mbDragCanceled )
        {
            // reset the data
            mbDragCanceled = true;
            ImplRulerData aTempData;
            aTempData = *mpDragData;
            *mpDragData = *mpSaveData;
            mbCalc = true;
            mbFormat = true;

            // call handler
            mnDragPos = mnStartDragPos;
            Drag();

            // and redraw
            Invalidate(InvalidateFlags::NoErase);

            // reset the data as before cancel
            *mpDragData = aTempData;
        }
    }
    else
    {
        mbDragCanceled = false;

        // +2, so the tabs are not cleared too quickly
        if ( nY > nOutHeight + 2 )
            mbDragDelete = true;

        mnDragPos = nX;

        // call handler
        Drag();

        // redraw
        if (mbFormat)
            Invalidate(InvalidateFlags::NoErase);
    }
}

void Ruler::ImplEndDrag()
{
    // get values
    if ( mbDragCanceled )
        *mpDragData = *mpSaveData;
    else
        *mpSaveData = *mpDragData;

    mpData = mpSaveData;
    mbDrag = false;

    // call handler
    EndDrag();

    // reset drag values
    meDragType      = RULER_TYPE_DONTKNOW;
    mnDragPos       = 0;
    mnDragAryPos    = 0;
    mnDragSize      = 0;
    mbDragCanceled  = false;
    mbDragDelete    = false;
    mnDragModifier  = 0;
    mnStartDragPos  = 0;

    // redraw
    Invalidate(InvalidateFlags::NoErase);
}

void Ruler::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && !IsTracking() )
    {
        Point   aMousePos = rMEvt.GetPosPixel();
        sal_uInt16  nMouseClicks = rMEvt.GetClicks();
        sal_uInt16  nMouseModifier = rMEvt.GetModifier();

        // update ruler
        if ( mbFormat )
        {
            Invalidate(InvalidateFlags::NoErase);
        }

        if ( maExtraRect.IsInside( aMousePos ) )
        {
            mnExtraClicks = nMouseClicks;
            mnExtraModifier = nMouseModifier;
            ExtraDown();
            mnExtraClicks = 0;
            mnExtraModifier = 0;
        }
        else
        {
            std::unique_ptr<RulerSelection> pHitTest(new RulerSelection);
            bool bHitTestResult = ImplHitTest(aMousePos, pHitTest.get());

            if ( nMouseClicks == 1 )
            {
                if ( bHitTestResult )
                {
                    ImplStartDrag( pHitTest.get(), nMouseModifier );
                }
                else
                {
                    // calculate position inside of ruler area
                    if ( pHitTest->eType == RULER_TYPE_DONTKNOW )
                    {
                        mnDragPos = pHitTest->nPos;
                        Click();
                        mnDragPos = 0;

                        // call HitTest again as a click, for example, could set a new tab
                        if ( ImplHitTest(aMousePos, pHitTest.get()) )
                            ImplStartDrag(pHitTest.get(), nMouseModifier);
                    }
                }
            }
            else
            {
                if (bHitTestResult)
                {
                    mnDragPos    = pHitTest->nPos;
                    mnDragAryPos = pHitTest->nAryPos;
                }
                meDragType = pHitTest->eType;

                DoubleClick();

                meDragType      = RULER_TYPE_DONTKNOW;
                mnDragPos       = 0;
                mnDragAryPos    = 0;
            }
        }
    }
}

void Ruler::MouseMove( const MouseEvent& rMEvt )
{
    PointerStyle ePtrStyle = PointerStyle::Arrow;

    mxPreviousHitTest.swap(mxCurrentHitTest);

    mxCurrentHitTest.reset(new RulerSelection);

    maHoverSelection.eType = RULER_TYPE_DONTKNOW;

    if (ImplHitTest( rMEvt.GetPosPixel(), mxCurrentHitTest.get() ))
    {
        maHoverSelection = *mxCurrentHitTest.get();

        if (mxCurrentHitTest->bSize)
        {
            if (mnWinStyle & WB_HORZ)
            {
                if (mxCurrentHitTest->mnDragSize == RULER_DRAGSIZE_1)
                    ePtrStyle = PointerStyle::TabSelectW;
                else if (mxCurrentHitTest->mnDragSize == RULER_DRAGSIZE_2)
                    ePtrStyle = PointerStyle::TabSelectE;
                else
                    ePtrStyle = PointerStyle::ESize;
            }
            else
            {
                if (mxCurrentHitTest->mnDragSize == RULER_DRAGSIZE_1)
                    ePtrStyle = PointerStyle::WindowNSize;
                else if (mxCurrentHitTest->mnDragSize == RULER_DRAGSIZE_2)
                    ePtrStyle = PointerStyle::WindowSSize;
                else
                    ePtrStyle = PointerStyle::SSize;
            }
        }
        else if (mxCurrentHitTest->bSizeBar)
        {
            if (mnWinStyle & WB_HORZ)
                ePtrStyle = PointerStyle::HSizeBar;
            else
                ePtrStyle = PointerStyle::VSizeBar;
        }
    }

    if (mxPreviousHitTest.get() != nullptr && mxPreviousHitTest->eType != mxCurrentHitTest->eType)
    {
        mbFormat = true;
    }

    SetPointer( Pointer(ePtrStyle) );

    if (mbFormat)
    {
        Invalidate(InvalidateFlags::NoErase);
    }
}

void Ruler::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        // reset the old state at cancel
        if ( rTEvt.IsTrackingCanceled() )
        {
            mbDragCanceled = true;
            mbFormat       = true;
        }

        ImplEndDrag();
    }
    else
        ImplDrag( rTEvt.GetMouseEvent().GetPosPixel() );
}

void Ruler::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    ImplDraw(rRenderContext);

    // consider extra field
    if (mnWinStyle & WB_EXTRAFIELD)
        ImplDrawExtra(rRenderContext);
}

void Ruler::Resize()
{
    Size aWinSize = GetOutputSizePixel();

    long nNewHeight;
    if ( mnWinStyle & WB_HORZ )
    {
        if ( aWinSize.Height() != mnHeight )
            nNewHeight = aWinSize.Height();
        else
            nNewHeight = 0;
    }
    else
    {
        if ( aWinSize.Width() != mnWidth )
            nNewHeight = aWinSize.Width();
        else
            nNewHeight = 0;
    }

    mbFormat = true;

    // clear lines
    bool bVisible = IsReallyVisible();
    if ( bVisible && !mpData->pLines.empty() )
    {
        mnUpdateFlags |= RULER_UPDATE_LINES;
        Invalidate(InvalidateFlags::NoErase);
    }

    // recalculate some values if the height/width changes
    // extra field should always be updated
    ImplInitExtraField( mpData->bTextRTL );
    if ( nNewHeight )
    {
        mbCalc = true;
        mnVirHeight = nNewHeight - mnBorderWidth - ( RULER_OFF * 2 );
    }
    else
    {
        if ( mpData->bAutoPageWidth )
            ImplUpdate( true );
        else if ( mbAutoWinWidth )
            mbCalc = true;
    }

    // clear part of the border
    if ( bVisible )
    {
        if ( nNewHeight )
            Invalidate(InvalidateFlags::NoErase);
        else if ( mpData->bAutoPageWidth )
        {
            // only at AutoPageWidth muss we redraw
            Rectangle aRect;

            if ( mnWinStyle & WB_HORZ )
            {
                if ( mnWidth < aWinSize.Width() )
                    aRect.Left() = mnWidth - RULER_RESIZE_OFF;
                else
                    aRect.Left() = aWinSize.Width() - RULER_RESIZE_OFF;
                aRect.Right()   = aRect.Left() + RULER_RESIZE_OFF;
                aRect.Top()     = RULER_OFF;
                aRect.Bottom()  = RULER_OFF + mnVirHeight;
            }
            else
            {
                if ( mnHeight < aWinSize.Height() )
                    aRect.Top() = mnHeight-RULER_RESIZE_OFF;
                else
                    aRect.Top() = aWinSize.Height()-RULER_RESIZE_OFF;
                aRect.Bottom() = aRect.Top() + RULER_RESIZE_OFF;
                aRect.Left()    = RULER_OFF;
                aRect.Right()   = RULER_OFF + mnVirHeight;
            }

            Invalidate(aRect, InvalidateFlags::NoErase);
        }
    }

    mnWidth  = aWinSize.Width();
    mnHeight = aWinSize.Height();
}

void Ruler::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == StateChangedType::InitShow )
        Invalidate();
    else if ( nType == StateChangedType::UpdateMode )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( (nType == StateChangedType::Zoom) ||
              (nType == StateChangedType::ControlFont) )
    {
        ImplInitSettings( true, false, false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ImplInitSettings( false, true, false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings( false, false, true );
        Invalidate();
    }
}

void Ruler::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        mbFormat = true;
        ImplInitSettings( true, true, true );
        Invalidate();
    }
}

bool Ruler::StartDrag()
{
    return false;
}

void Ruler::Drag()
{
}

void Ruler::EndDrag()
{
}

void Ruler::Click()
{
}

void Ruler::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

void Ruler::ExtraDown()
{
}

void Ruler::Activate()
{
    mbActive = true;

    // update positionlies - draw is delayed
    mnUpdateFlags |= RULER_UPDATE_LINES;
    Invalidate(InvalidateFlags::NoErase);
}

void Ruler::Deactivate()
{
    // clear positionlines
    Invalidate(InvalidateFlags::NoErase);

    mbActive = false;
}

bool Ruler::StartDocDrag( const MouseEvent& rMEvt, RulerType eDragType )
{
    if ( !mbDrag )
    {
        Point          aMousePos = rMEvt.GetPosPixel();
        sal_uInt16     nMouseClicks = rMEvt.GetClicks();
        sal_uInt16     nMouseModifier = rMEvt.GetModifier();
        RulerSelection aHitTest;

        if(eDragType != RULER_TYPE_DONTKNOW)
            aHitTest.bExpandTest = true;

        // update ruler
        if ( mbFormat )
        {
            if (!IsReallyVisible())
            {
                // set mpData for ImplDocHitTest()
                ImplFormat(*this);
            }

            Invalidate(InvalidateFlags::NoErase);
        }

        if ( nMouseClicks == 1 )
        {
            if ( ImplDocHitTest( aMousePos, eDragType, &aHitTest ) )
            {
                Pointer aPtr;

                if ( aHitTest.bSize )
                {
                    if ( mnWinStyle & WB_HORZ )
                        aPtr = Pointer( PointerStyle::ESize );
                    else
                        aPtr = Pointer( PointerStyle::SSize );
                }
                else if ( aHitTest.bSizeBar )
                {
                    if ( mnWinStyle & WB_HORZ )
                        aPtr = Pointer( PointerStyle::HSizeBar );
                    else
                        aPtr = Pointer( PointerStyle::VSizeBar );
                }
                SetPointer( aPtr );
                return ImplStartDrag( &aHitTest, nMouseModifier );
            }
        }
        else if ( nMouseClicks == 2 )
        {
            if ( ImplDocHitTest( aMousePos, eDragType, &aHitTest ) )
            {
                mnDragPos    = aHitTest.nPos;
                mnDragAryPos = aHitTest.nAryPos;
            }

            DoubleClick();

            mnDragPos       = 0;
            mnDragAryPos    = 0;

            return true;
        }
    }

    return false;
}

void Ruler::CancelDrag()
{
    if ( mbDrag )
    {
        ImplDrag( Point( -1, -1 ) );
        ImplEndDrag();
    }
}

RulerType Ruler::GetType( const Point& rPos, sal_uInt16* pAryPos )
{
    RulerSelection aHitTest;

    // update ruler
    if ( IsReallyVisible() && mbFormat )
    {
        Invalidate(InvalidateFlags::NoErase);
    }

    (void)ImplHitTest(rPos, &aHitTest);

    // return values
    if ( pAryPos )
        *pAryPos = aHitTest.nAryPos;
    return aHitTest.eType;
}

void Ruler::SetWinPos( long nNewOff, long nNewWidth )
{
    // should widths be automatically calculated
    if ( !nNewWidth )
        mbAutoWinWidth = true;
    else
        mbAutoWinWidth = false;

    mnWinOff = nNewOff;
    mnWinWidth = nNewWidth;
    ImplUpdate( true );
}

void Ruler::SetPagePos( long nNewOff, long nNewWidth )
{
    // should we do anything?
    if ( (mpData->nPageOff == nNewOff) && (mpData->nPageWidth == nNewWidth) )
        return;

    // should widths be automatically calculated
    if ( !nNewWidth )
        mpData->bAutoPageWidth = true;
    else
        mpData->bAutoPageWidth = false;

    mpData->nPageOff     = nNewOff;
    mpData->nPageWidth   = nNewWidth;
    ImplUpdate( true );
}

void Ruler::SetBorderPos( long nOff )
{
    if ( mnWinStyle & WB_BORDER )
    {
        if ( mnBorderOff != nOff )
        {
            mnBorderOff = nOff;

            if ( IsReallyVisible() && IsUpdateMode() )
                Invalidate(InvalidateFlags::NoErase);
        }
    }
}

void Ruler::SetUnit( FieldUnit eNewUnit )
{
    if ( meUnit != eNewUnit )
    {
        meUnit = eNewUnit;
        switch ( meUnit )
        {
            case FUNIT_MM:
                mnUnitIndex = RULER_UNIT_MM;
                break;
            case FUNIT_CM:
                mnUnitIndex = RULER_UNIT_CM;
                break;
            case FUNIT_M:
                mnUnitIndex = RULER_UNIT_M;
                break;
            case FUNIT_KM:
                mnUnitIndex = RULER_UNIT_KM;
                break;
            case FUNIT_INCH:
                mnUnitIndex = RULER_UNIT_INCH;
                break;
            case FUNIT_FOOT:
                mnUnitIndex = RULER_UNIT_FOOT;
                break;
            case FUNIT_MILE:
                mnUnitIndex = RULER_UNIT_MILE;
                break;
            case FUNIT_POINT:
                mnUnitIndex = RULER_UNIT_POINT;
                break;
            case FUNIT_PICA:
                mnUnitIndex = RULER_UNIT_PICA;
                break;
            case FUNIT_CHAR:
                mnUnitIndex = RULER_UNIT_CHAR;
                break;
            case FUNIT_LINE:
                mnUnitIndex = RULER_UNIT_LINE;
                break;
            default:
                SAL_WARN( "svtools.control", "Ruler::SetUnit() - Wrong Unit" );
                break;
        }

        maMapMode.SetMapUnit( aImplRulerUnitTab[mnUnitIndex].eMapUnit );
        ImplUpdate();
    }
}

void Ruler::SetZoom( const Fraction& rNewZoom )
{
    DBG_ASSERT( rNewZoom.GetNumerator(), "Ruler::SetZoom() with scale 0 is not allowed" );

    if ( maZoom != rNewZoom )
    {
        maZoom = rNewZoom;
        maMapMode.SetScaleX( maZoom );
        maMapMode.SetScaleY( maZoom );
        ImplUpdate();
    }
}

void Ruler::SetExtraType( RulerExtra eNewExtraType, sal_uInt16 nStyle )
{
    if ( mnWinStyle & WB_EXTRAFIELD )
    {
        meExtraType  = eNewExtraType;
        mnExtraStyle = nStyle;
        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();
    }
}

void Ruler::SetNullOffset( long nPos )
{
    if ( mpData->nNullOff != nPos )
    {
        mpData->nNullOff = nPos;
        ImplUpdate();
    }
}

void Ruler::SetLeftFrameMargin( long nPos )
{
    if ( (mpData->nLeftFrameMargin != nPos) )
    {
        mpData->nLeftFrameMargin  = nPos;
        ImplUpdate();
    }
}

void Ruler::SetRightFrameMargin( long nPos )
{
    if ( (mpData->nRightFrameMargin != nPos) )
    {
        mpData->nRightFrameMargin  = nPos;
        ImplUpdate();
    }
}

void Ruler::SetMargin1( long nPos, sal_uInt16 nMarginStyle )
{
    if ( (mpData->nMargin1 != nPos) || (mpData->nMargin1Style != nMarginStyle) )
    {
        mpData->nMargin1      = nPos;
        mpData->nMargin1Style = nMarginStyle;
        ImplUpdate();
    }
}

void Ruler::SetMargin2( long nPos, sal_uInt16 nMarginStyle )
{
    DBG_ASSERT( (nPos >= mpData->nMargin1) ||
                (mpData->nMargin1Style & RULER_STYLE_INVISIBLE) ||
                (mpData->nMargin2Style & RULER_STYLE_INVISIBLE),
                "Ruler::SetMargin2() - Margin2 < Margin1" );

    if ( (mpData->nMargin2 != nPos) || (mpData->nMargin2Style != nMarginStyle) )
    {
        mpData->nMargin2      = nPos;
        mpData->nMargin2Style = nMarginStyle;
        ImplUpdate();
    }
}

void Ruler::SetLines( sal_uInt32 aLineArraySize, const RulerLine* pLineArray )
{
    // To determine if what has changed
    if ( mpData->pLines.size() == aLineArraySize )
    {
        sal_uInt32           i = aLineArraySize;
        vector<RulerLine>::const_iterator aItr1 = mpData->pLines.begin();
        const RulerLine* pAry2 = pLineArray;
        while ( i )
        {
            if ( (aItr1->nPos   != pAry2->nPos)   ||
                 (aItr1->nStyle != pAry2->nStyle) )
                break;
            ++aItr1;
            ++pAry2;
            i--;
        }
        if ( !i )
            return;
    }

    // New values and new share issue
    bool bMustUpdate;
    if ( IsReallyVisible() && IsUpdateMode() )
        bMustUpdate = true;
    else
        bMustUpdate = false;

    // Delete old lines
    if ( bMustUpdate )
        Invalidate(InvalidateFlags::NoErase);

    // New data set
    if ( !aLineArraySize || !pLineArray )
    {
        if ( mpData->pLines.empty() )
            return;
        mpData->pLines.clear();
    }
    else
    {
        if ( mpData->pLines.size() != aLineArraySize )
        {
            mpData->pLines.resize(aLineArraySize);
        }

        std::copy( pLineArray,
                   pLineArray + aLineArraySize,
                   mpData->pLines.begin() );

        if ( bMustUpdate )
            Invalidate(InvalidateFlags::NoErase);
    }
}

void Ruler::SetBorders( sal_uInt32 aBorderArraySize, const RulerBorder* pBorderArray )
{
    if ( !aBorderArraySize || !pBorderArray )
    {
        if ( mpData->pBorders.empty() )
            return;
        mpData->pBorders.clear();
    }
    else
    {
        if ( mpData->pBorders.size() != aBorderArraySize )
        {
            mpData->pBorders.resize(aBorderArraySize);
        }
        else
        {
            sal_uInt32             i = aBorderArraySize;
            const RulerBorder* pAry1 = &mpData->pBorders[0];
            const RulerBorder* pAry2 = pBorderArray;
            while ( i )
            {
                if ( (pAry1->nPos   != pAry2->nPos)   ||
                     (pAry1->nWidth != pAry2->nWidth) ||
                     (pAry1->nStyle != pAry2->nStyle) )
                    break;
                pAry1++;
                pAry2++;
                i--;
            }
            if ( !i )
                return;
        }
        std::copy( pBorderArray,
                   pBorderArray + aBorderArraySize,
                   mpData->pBorders.begin() );
    }

    ImplUpdate();
}

void Ruler::SetIndents( sal_uInt32 aIndentArraySize, const RulerIndent* pIndentArray )
{

    if ( !aIndentArraySize || !pIndentArray )
    {
        if ( mpData->pIndents.empty() )
            return;
        mpData->pIndents.clear();
    }
    else
    {
        if ( mpData->pIndents.size() != aIndentArraySize )
        {
            mpData->pIndents.resize(aIndentArraySize);
        }
        else
        {
            sal_uInt32             i = aIndentArraySize;
            const RulerIndent* pAry1 = &mpData->pIndents[0];
            const RulerIndent* pAry2 = pIndentArray;
            while ( i )
            {
                if ( (pAry1->nPos   != pAry2->nPos) ||
                     (pAry1->nStyle != pAry2->nStyle) )
                    break;
                pAry1++;
                pAry2++;
                i--;
            }
            if ( !i )
                return;
        }

        std::copy( pIndentArray,
                   pIndentArray + aIndentArraySize,
                   mpData->pIndents.begin() );
    }

    ImplUpdate();
}

void Ruler::SetTabs( sal_uInt32 aTabArraySize, const RulerTab* pTabArray )
{
    if ( aTabArraySize == 0 || pTabArray == nullptr )
    {
        if ( mpData->pTabs.empty() )
            return;
        mpData->pTabs.clear();
    }
    else
    {
        if ( mpData->pTabs.size() != aTabArraySize )
        {
            mpData->pTabs.resize(aTabArraySize);
        }
        else
        {
            sal_uInt32 i = aTabArraySize;
            vector<RulerTab>::iterator aTabIterator = mpData->pTabs.begin();
            const RulerTab* pInputArray = pTabArray;
            while ( i )
            {
                RulerTab& aCurrent = *aTabIterator;
                if ( aCurrent.nPos   != pInputArray->nPos ||
                     aCurrent.nStyle != pInputArray->nStyle )
                {
                    break;
                }
                ++aTabIterator;
                pInputArray++;
                i--;
            }
            if ( !i )
                return;
        }
        std::copy(pTabArray, pTabArray + aTabArraySize, mpData->pTabs.begin());
    }

    ImplUpdate();
}

void Ruler::SetStyle( WinBits nStyle )
{
    if ( mnWinStyle != nStyle )
    {
        mnWinStyle = nStyle;
        ImplInitExtraField( true );
    }
}

void Ruler::DrawTab(vcl::RenderContext& rRenderContext, const Color &rFillColor, const Point& rPos, sal_uInt16 nStyle)
{
    Point aPos(rPos);
    sal_uInt16 nTabStyle = nStyle & (RULER_TAB_STYLE | RULER_TAB_RTL);

    rRenderContext.Push(PushFlags::LINECOLOR | PushFlags::FILLCOLOR);
    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(rFillColor);
    ImplCenterTabPos(aPos, nTabStyle);
    ImplDrawRulerTab(rRenderContext, aPos, nTabStyle, nStyle);
    rRenderContext.Pop();
}

void Ruler::SetTextRTL(bool bRTL)
{
    if(mpData->bTextRTL != (bool) bRTL)
    {
        mpData->bTextRTL = bRTL;
        if ( IsReallyVisible() && IsUpdateMode() )
            ImplInitExtraField( true );
    }

}

long Ruler::GetPageOffset() const
{
    return mpData->nPageOff;
}

long Ruler::GetNullOffset() const
{
    return mpData->nNullOff;
}

long Ruler::GetMargin1() const
{
    return mpData->nMargin1;
}

long Ruler::GetMargin2() const
{
    return mpData->nMargin2;
}


bool Ruler::GetTextRTL()
{
    return mpData->bTextRTL;
}

const RulerUnitData& Ruler::GetCurrentRulerUnit() const
{
    return aImplRulerUnitTab[mnUnitIndex];
}

void Ruler::DrawTicks()
{
    mbFormat = true;
    Invalidate(InvalidateFlags::NoErase);
}

uno::Reference< XAccessible > Ruler::CreateAccessible()
{
    vcl::Window* pParent = GetAccessibleParentWindow();
    OSL_ENSURE( pParent, "-SvxRuler::CreateAccessible(): No Parent!" );
    uno::Reference< XAccessible >   xAccParent  = pParent->GetAccessible();
    if( xAccParent.is() )
    {
        // MT: Fixed compiler issue because the address from a temporary object was used.
        // BUT: Should it really be a Pointer, instead of const&???
        OUString aStr;
        if ( mnWinStyle & WB_HORZ )
        {
            aStr = SvtResId(STR_SVT_ACC_RULER_HORZ_NAME);
        }
        else
        {
            aStr = SvtResId(STR_SVT_ACC_RULER_VERT_NAME);
        }
        pAccContext = new SvtRulerAccessible( xAccParent, *this, aStr );
        pAccContext->acquire();
        this->SetAccessible(pAccContext);
        return pAccContext;
    }
    else
        return uno::Reference< XAccessible >();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
