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

#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>


#include "rect.hxx"
#include "types.hxx"
#include "utility.hxx"
#include "smmod.hxx"




// '\0' terminated Array with symbol, which should be treat as letters in
// StarMath Font, (to get a normal (non-clipped) SmRect in contrast to the
// other operators and symbols).
static sal_Unicode const aMathAlpha[] =
{
    MS_ALEPH,               MS_IM,                  MS_RE,
    MS_WP,                  sal_Unicode(0xE070),    MS_EMPTYSET,
    sal_Unicode(0x2113),    sal_Unicode(0xE0D6),    sal_Unicode(0x2107),
    sal_Unicode(0x2127),    sal_Unicode(0x210A),    MS_HBAR,
    MS_LAMBDABAR,           MS_SETN,                MS_SETZ,
    MS_SETQ,                MS_SETR,                MS_SETC,
    sal_Unicode(0x2373),    sal_Unicode(0xE0A5),    sal_Unicode(0x2112),
    sal_Unicode(0x2130),    sal_Unicode(0x2131),
    sal_Unicode('\0')
};

bool SmIsMathAlpha(const OUString &rText)
    // true iff symbol (from StarMath Font) should be treated as letter
{
    if (rText.isEmpty())
        return false;

    OSL_ENSURE(rText.getLength() == 1, "Sm : string must be exactly one character long");
    sal_Unicode cChar = rText[0];

    // is it a greek symbol?
    if (sal_Unicode(0xE0AC) <= cChar  &&  cChar <= sal_Unicode(0xE0D4))
        return true;
    else
    {
        // appears it in 'aMathAlpha'?
        const sal_Unicode *pChar = aMathAlpha;
        while (*pChar  &&  *pChar != cChar)
            pChar++;
        return *pChar != '\0';
    }
}


// SmRect members


SmRect::SmRect()
    // constructs empty rectangle at (0, 0) with width and height 0.
{
    OSL_ENSURE(aTopLeft == Point(0, 0), "Sm: ooops...");
    OSL_ENSURE(aSize == Size(0, 0), "Sm: ooops...");

    bHasBaseline = bHasAlignInfo = false;
    nBaseline = nAlignT = nAlignM = nAlignB =
    nGlyphTop = nGlyphBottom =
    nItalicLeftSpace = nItalicRightSpace =
    nLoAttrFence = nHiAttrFence = 0;
    nBorderWidth = 0;
}


SmRect::SmRect(const SmRect &rRect)
:   aTopLeft(rRect.aTopLeft),
    aSize(rRect.aSize)
{
    bHasBaseline  = rRect.bHasBaseline;
    nBaseline     = rRect.nBaseline;
    nAlignT       = rRect.nAlignT;
    nAlignM       = rRect.nAlignM;
    nAlignB       = rRect.nAlignB;
    nGlyphTop     = rRect.nGlyphTop;
    nGlyphBottom  = rRect.nGlyphBottom;
    nHiAttrFence  = rRect.nHiAttrFence;
    nLoAttrFence  = rRect.nLoAttrFence;
    bHasAlignInfo = rRect.bHasAlignInfo;
    nItalicLeftSpace  = rRect.nItalicLeftSpace;
    nItalicRightSpace = rRect.nItalicRightSpace;
    nBorderWidth  = rRect.nBorderWidth;
}


void SmRect::CopyAlignInfo(const SmRect &rRect)
{
    nBaseline     = rRect.nBaseline;
    bHasBaseline  = rRect.bHasBaseline;
    nAlignT       = rRect.nAlignT;
    nAlignM       = rRect.nAlignM;
    nAlignB       = rRect.nAlignB;
    bHasAlignInfo = rRect.bHasAlignInfo;
    nLoAttrFence  = rRect.nLoAttrFence;
    nHiAttrFence  = rRect.nHiAttrFence;
}


void SmRect::BuildRect(const OutputDevice &rDev, const SmFormat *pFormat,
                       const OUString &rText, sal_uInt16 nBorder)
{
    OSL_ENSURE(aTopLeft == Point(0, 0), "Sm: Ooops...");

    aSize = Size(rDev.GetTextWidth(rText), rDev.GetTextHeight());

    const FontMetric  aFM (rDev.GetFontMetric());
    bool              bIsMath  = aFM.GetFamilyName().equalsIgnoreAsciiCase( FONTNAME_MATH );
    bool              bAllowSmaller = bIsMath && !SmIsMathAlpha(rText);
    const long        nFontHeight = rDev.GetFont().GetSize().Height();

    nBorderWidth  = nBorder;
    bHasAlignInfo = true;
    bHasBaseline  = true;
    nBaseline     = aFM.GetAscent();
    nAlignT       = nBaseline - nFontHeight * 750L / 1000L;
    nAlignM       = nBaseline - nFontHeight * 121L / 422L;
        // that's where the horizontal bars of '+', '-', ... are
        // (1/3 of ascent over baseline)
        // (121 = 1/3 of 12pt ascent, 422 = 12pt fontheight)
    nAlignB       = nBaseline;

    // workaround for printer fonts with very small (possible 0 or even
    // negative(!)) leading
    if (aFM.GetIntLeading() < 5  &&  rDev.GetOutDevType() == OUTDEV_PRINTER)
    {
        OutputDevice    *pWindow = Application::GetDefaultDevice();

        pWindow->Push(PushFlags::MAPMODE | PushFlags::FONT);

        pWindow->SetMapMode(rDev.GetMapMode());
        pWindow->SetFont(rDev.GetFontMetric());

        long  nDelta = pWindow->GetFontMetric().GetIntLeading();
        if (nDelta == 0)
        {   // this value approx. fits a Leading of 80 at a
            // Fontheight of 422 (12pt)
            nDelta = nFontHeight * 8L / 43;
        }
        SetTop(GetTop() - nDelta);

        pWindow->Pop();
    }

    // get GlyphBoundRect
    Rectangle  aGlyphRect;
    bool bSuccess = SmGetGlyphBoundRect(rDev, rText, aGlyphRect);
    if (!bSuccess)
        SAL_WARN("starmath", "Ooops... (Font missing?)");

    nItalicLeftSpace  = GetLeft() - aGlyphRect.Left() + nBorderWidth;
    nItalicRightSpace = aGlyphRect.Right() - GetRight() + nBorderWidth;
    if (nItalicLeftSpace  < 0  &&  !bAllowSmaller)
        nItalicLeftSpace  = 0;
    if (nItalicRightSpace < 0  &&  !bAllowSmaller)
        nItalicRightSpace = 0;

    long  nDist = 0;
    if (pFormat)
        nDist = (rDev.GetFont().GetSize().Height()
                * pFormat->GetDistance(DIS_ORNAMENTSIZE)) / 100L;

    nHiAttrFence = aGlyphRect.TopLeft().Y() - 1 - nBorderWidth - nDist;
    nLoAttrFence = SmFromTo(GetAlignB(), GetBottom(), 0.0);

    nGlyphTop    = aGlyphRect.Top() - nBorderWidth;
    nGlyphBottom = aGlyphRect.Bottom() + nBorderWidth;

    if (bAllowSmaller)
    {
        // for symbols and operators from the StarMath Font
        // we adjust upper and lower margin of the symbol
        SetTop(nGlyphTop);
        SetBottom(nGlyphBottom);
    }

    if (nHiAttrFence < GetTop())
        nHiAttrFence = GetTop();

    if (nLoAttrFence > GetBottom())
        nLoAttrFence = GetBottom();

    OSL_ENSURE(rText.isEmpty() || !IsEmpty(),
               "Sm: empty rectangle created");
}


void SmRect::Init(const OutputDevice &rDev, const SmFormat *pFormat,
                  const OUString &rText, sal_uInt16 nEBorderWidth)
    // get rectangle fitting for drawing 'rText' on OutputDevice 'rDev'
{
    BuildRect(rDev, pFormat, rText, nEBorderWidth);
}


SmRect::SmRect(const OutputDevice &rDev, const SmFormat *pFormat,
               const OUString &rText, long nEBorderWidth)
{
    OSL_ENSURE( nEBorderWidth >= 0, "BorderWidth is negative" );
    if (nEBorderWidth < 0)
        nEBorderWidth = 0;
    Init(rDev, pFormat, rText, (sal_uInt16) nEBorderWidth);
}


SmRect::SmRect(long nWidth, long nHeight)
    // this constructor should never be used for anything textlike because
    // it will not provide useful values for baseline, AlignT and AlignB!
    // It's purpose is to get a 'SmRect' for the horizontal line in fractions
    // as used in 'SmBinVerNode'.
:   aSize(nWidth, nHeight)
{
    OSL_ENSURE(aTopLeft == Point(0, 0), "Sm: ooops...");

    bHasBaseline  = false;
    bHasAlignInfo = true;
    nBaseline     = 0;
    nAlignT       = GetTop();
    nAlignB       = GetBottom();
    nAlignM       = (nAlignT + nAlignB) / 2;        // this is the default
    nItalicLeftSpace = nItalicRightSpace = 0;
    nGlyphTop    = nHiAttrFence  = GetTop();
    nGlyphBottom = nLoAttrFence  = GetBottom();
    nBorderWidth  = 0;
}


void SmRect::SetLeft(long nLeft)
{
    if (nLeft <= GetRight())
    {   aSize.Width() = GetRight() - nLeft + 1;
        aTopLeft.X()  = nLeft;
    }
}


void SmRect::SetRight(long nRight)
{
    if (nRight >= GetLeft())
        aSize.Width() = nRight - GetLeft() + 1;
}


void SmRect::SetBottom(long nBottom)
{
    if (nBottom >= GetTop())
        aSize.Height() = nBottom - GetTop() + 1;
}


void SmRect::SetTop(long nTop)
{
    if (nTop <= GetBottom())
    {   aSize.Height()   = GetBottom() - nTop + 1;
        aTopLeft.Y() = nTop;
    }
}


void SmRect::Move(const Point &rPosition)
    // move rectangle by position 'rPosition'.
{
    aTopLeft  += rPosition;

    long  nDelta = rPosition.Y();
    nBaseline += nDelta;
    nAlignT   += nDelta;
    nAlignM   += nDelta;
    nAlignB   += nDelta;
    nGlyphTop    += nDelta;
    nGlyphBottom += nDelta;
    nHiAttrFence += nDelta;
    nLoAttrFence += nDelta;
}


const Point SmRect::AlignTo(const SmRect &rRect, RectPos ePos,
                            RectHorAlign eHor, RectVerAlign eVer) const
{   Point  aPos (GetTopLeft());
        // will become the topleft point of the new rectangle position

    // set horizontal or vertical new rectangle position depending on
    // 'ePos' is one of 'RP_LEFT', 'RP_RIGHT' or 'RP_TOP', 'RP_BOTTOM'
    switch (ePos)
    {   case RP_LEFT :
            aPos.X() = rRect.GetItalicLeft() - GetItalicRightSpace()
                       - GetWidth();
            break;
        case RP_RIGHT :
            aPos.X() = rRect.GetItalicRight() + 1 + GetItalicLeftSpace();
            break;
        case RP_TOP :
            aPos.Y() = rRect.GetTop() - GetHeight();
            break;
        case RP_BOTTOM :
            aPos.Y() = rRect.GetBottom() + 1;
            break;
        case RP_ATTRIBUT :
            aPos.X() = rRect.GetItalicCenterX() - GetItalicWidth() / 2
                       + GetItalicLeftSpace();
            break;
        default :
            SAL_WARN("starmath", "unknown case");
    }

    // check if horizontal position is already set
    if (ePos == RP_LEFT  ||  ePos == RP_RIGHT  ||  ePos == RP_ATTRIBUT)
        // correct error in current vertical position
        switch (eVer)
        {   case RVA_TOP :
                aPos.Y() += rRect.GetAlignT() - GetAlignT();
                break;
            case RVA_MID :
                aPos.Y() += rRect.GetAlignM() - GetAlignM();
                break;
            case RVA_BASELINE :
                // align baselines if possible else align mid's
                if (HasBaseline() && rRect.HasBaseline())
                    aPos.Y() += rRect.GetBaseline() - GetBaseline();
                else
                    aPos.Y() += rRect.GetAlignM() - GetAlignM();
                break;
            case RVA_BOTTOM :
                aPos.Y() += rRect.GetAlignB() - GetAlignB();
                break;
            case RVA_CENTERY :
                aPos.Y() += rRect.GetCenterY() - GetCenterY();
                break;
            case RVA_ATTRIBUT_HI:
                aPos.Y() += rRect.GetHiAttrFence() - GetBottom();
                break;
            case RVA_ATTRIBUT_MID :
                aPos.Y() += SmFromTo(rRect.GetAlignB(), rRect.GetAlignT(), 0.4)
                            - GetCenterY();
                break;
            case RVA_ATTRIBUT_LO :
                aPos.Y() += rRect.GetLoAttrFence() - GetTop();
                break;
        default :
                SAL_WARN("starmath", "unknown case");
        }

    // check if vertical position is already set
    if (ePos == RP_TOP  ||  ePos == RP_BOTTOM)
        // correct error in current horizontal position
        switch (eHor)
        {   case RHA_LEFT :
                aPos.X() += rRect.GetItalicLeft() - GetItalicLeft();
                break;
            case RHA_CENTER :
                aPos.X() += rRect.GetItalicCenterX() - GetItalicCenterX();
                break;
            case RHA_RIGHT :
                aPos.X() += rRect.GetItalicRight() - GetItalicRight();
                break;
            default :
                SAL_WARN("starmath", "unknown case");
        }

    return aPos;
}


SmRect & SmRect::Union(const SmRect &rRect)
    // rectangle union of current one with 'rRect'. The result is to be the
    // smallest rectangles that covers the space of both rectangles.
    // (empty rectangles cover no space)
    //! Italic correction is NOT taken into account here!
{
    if (rRect.IsEmpty())
        return *this;

    long  nL  = rRect.GetLeft(),
          nR  = rRect.GetRight(),
          nT  = rRect.GetTop(),
          nB  = rRect.GetBottom(),
          nGT = rRect.nGlyphTop,
          nGB = rRect.nGlyphBottom;
    if (!IsEmpty())
    {   long  nTmp;

        if ((nTmp = GetLeft()) < nL)
            nL = nTmp;
        if ((nTmp = GetRight()) > nR)
            nR = nTmp;
        if ((nTmp = GetTop()) < nT)
            nT = nTmp;
        if ((nTmp = GetBottom()) > nB)
            nB = nTmp;
        if ((nTmp = nGlyphTop) < nGT)
            nGT = nTmp;
        if ((nTmp = nGlyphBottom) > nGB)
            nGB = nTmp;
    }

    SetLeft(nL);
    SetRight(nR);
    SetTop(nT);
    SetBottom(nB);
    nGlyphTop    = nGT;
    nGlyphBottom = nGB;

    return *this;
}


SmRect & SmRect::ExtendBy(const SmRect &rRect, RectCopyMBL eCopyMode)
    // let current rectangle be the union of itself and 'rRect'
    // (the smallest rectangle surrounding both). Also adapt values for
    // 'AlignT', 'AlignM', 'AlignB', baseline and italic-spaces.
    // The baseline is set according to 'eCopyMode'.
    // If one of the rectangles has no relevant info the other one is copied.
{
    // get some values used for (italic) spaces adaption
    // ! (need to be done before changing current SmRect) !
    long  nL = std::min(GetItalicLeft(),  rRect.GetItalicLeft()),
          nR = std::max(GetItalicRight(), rRect.GetItalicRight());

    Union(rRect);

    SetItalicSpaces(GetLeft() - nL, nR - GetRight());

    if (!HasAlignInfo())
        CopyAlignInfo(rRect);
    else if (rRect.HasAlignInfo())
    {   nAlignT = std::min(GetAlignT(), rRect.GetAlignT());
        nAlignB = std::max(GetAlignB(), rRect.GetAlignB());
        nHiAttrFence = std::min(GetHiAttrFence(), rRect.GetHiAttrFence());
        nLoAttrFence = std::max(GetLoAttrFence(), rRect.GetLoAttrFence());
        OSL_ENSURE(HasAlignInfo(), "Sm: ooops...");

        switch (eCopyMode)
        {   case RCP_THIS:
                // already done
                break;
            case RCP_ARG:
                CopyMBL(rRect);
                break;
            case RCP_NONE:
                ClearBaseline();
                nAlignM = (nAlignT + nAlignB) / 2;
                break;
            case RCP_XOR:
                if (!HasBaseline())
                    CopyMBL(rRect);
                break;
            default :
                SAL_WARN("starmath", "unknown case");
        }
    }

    return *this;
}


SmRect & SmRect::ExtendBy(const SmRect &rRect, RectCopyMBL eCopyMode,
                          long nNewAlignM)
    // as 'ExtendBy' but sets AlignM value to 'nNewAlignM'.
    // (this version will be used in 'SmBinVerNode' to provide means to
    // align eg "{a over b} over c" correctly where AlignM should not
    // be (AlignT + AlignB) / 2)
{
    OSL_ENSURE(HasAlignInfo(), "Sm: no align info");

    ExtendBy(rRect, eCopyMode);
    nAlignM = nNewAlignM;

    return *this;
}


SmRect & SmRect::ExtendBy(const SmRect &rRect, RectCopyMBL eCopyMode,
                          bool bKeepVerAlignParams)
    // as 'ExtendBy' but keeps original values for AlignT, -M and -B and
    // baseline.
    // (this is used in 'SmSupSubNode' where the sub-/supscripts shouldn't
    // be allowed to modify these values.)
{
    long  nOldAlignT   = GetAlignT(),
          nOldAlignM   = GetAlignM(),
          nOldAlignB   = GetAlignB(),
          nOldBaseline = nBaseline;     //! depends not on 'HasBaseline'
    bool  bOldHasAlignInfo = HasAlignInfo();

    ExtendBy(rRect, eCopyMode);

    if (bKeepVerAlignParams)
    {   nAlignT   = nOldAlignT;
        nAlignM   = nOldAlignM;
        nAlignB   = nOldAlignB;
        nBaseline = nOldBaseline;
        bHasAlignInfo = bOldHasAlignInfo;
    }

    return *this;
}


long SmRect::OrientedDist(const Point &rPoint) const
    // return oriented distance of rPoint to the current rectangle,
    // especially the return value is <= 0 iff the point is inside the
    // rectangle.
    // For simplicity the maximum-norm is used.
{
    bool  bIsInside = IsInsideItalicRect(rPoint);

    // build reference point to define the distance
    Point  aRef;
    if (bIsInside)
    {   Point  aIC (GetItalicCenterX(), GetCenterY());

        aRef.X() = rPoint.X() >= aIC.X() ? GetItalicRight() : GetItalicLeft();
        aRef.Y() = rPoint.Y() >= aIC.Y() ? GetBottom() : GetTop();
    }
    else
    {
        // x-coordinate
        if (rPoint.X() > GetItalicRight())
            aRef.X() = GetItalicRight();
        else if (rPoint.X() < GetItalicLeft())
            aRef.X() = GetItalicLeft();
        else
            aRef.X() = rPoint.X();
        // y-coordinate
        if (rPoint.Y() > GetBottom())
            aRef.Y() = GetBottom();
        else if (rPoint.Y() < GetTop())
            aRef.Y() = GetTop();
        else
            aRef.Y() = rPoint.Y();
    }

    // build distance vector
    Point  aDist (aRef - rPoint);

    long nAbsX = labs(aDist.X()),
         nAbsY = labs(aDist.Y());

    return bIsInside ? - std::min(nAbsX, nAbsY) : std::max (nAbsX, nAbsY);
}


bool SmRect::IsInsideRect(const Point &rPoint) const
{
    return     rPoint.Y() >= GetTop()
           &&  rPoint.Y() <= GetBottom()
           &&  rPoint.X() >= GetLeft()
           &&  rPoint.X() <= GetRight();
}


bool SmRect::IsInsideItalicRect(const Point &rPoint) const
{
    return     rPoint.Y() >= GetTop()
           &&  rPoint.Y() <= GetBottom()
           &&  rPoint.X() >= GetItalicLeft()
           &&  rPoint.X() <= GetItalicRight();
}

SmRect SmRect::AsGlyphRect() const
{
    SmRect aRect (*this);
    aRect.SetTop(nGlyphTop);
    aRect.SetBottom(nGlyphBottom);
    return aRect;
}

bool SmGetGlyphBoundRect(const vcl::RenderContext &rDev,
                         const OUString &rText, Rectangle &rRect)
    // basically the same as 'GetTextBoundRect' (in class 'OutputDevice')
    // but with a string as argument.
{
    // handle special case first
    if (rText.isEmpty())
    {
        rRect.SetEmpty();
        return true;
    }

    // get a device where 'OutputDevice::GetTextBoundRect' will be successful
    OutputDevice *pGlyphDev;
    if (rDev.GetOutDevType() != OUTDEV_PRINTER)
        pGlyphDev = const_cast<OutputDevice *>(&rDev);
    else
    {
        // since we format for the printer (where GetTextBoundRect will fail)
        // we need a virtual device here.
        pGlyphDev = &SM_MOD()->GetDefaultVirtualDev();
    }

    const FontMetric  aDevFM (rDev.GetFontMetric());

    pGlyphDev->Push(PushFlags::FONT | PushFlags::MAPMODE);
    vcl::Font aFnt(rDev.GetFont());
    aFnt.SetAlign(ALIGN_TOP);

    // use scale factor when calling GetTextBoundRect to counter
    // negative effects from antialiasing which may otherwise result
    // in significant incorrect bounding rectangles for some characters.
    Size aFntSize = aFnt.GetSize();

    // Workaround to avoid HUGE font sizes and resulting problems
    long nScaleFactor = 1;
    while( aFntSize.Height() > 2000 * nScaleFactor )
        nScaleFactor *= 2;

    aFnt.SetSize( Size( aFntSize.Width() / nScaleFactor, aFntSize.Height() / nScaleFactor ) );
    pGlyphDev->SetFont(aFnt);

    long nTextWidth = rDev.GetTextWidth(rText);
    Point aPoint;
    Rectangle   aResult (aPoint, Size(nTextWidth, rDev.GetTextHeight())),
                aTmp;

    bool bSuccess = pGlyphDev->GetTextBoundRect(aTmp, rText);
    OSL_ENSURE( bSuccess, "GetTextBoundRect failed" );


    if (!aTmp.IsEmpty())
    {
        aResult = Rectangle(aTmp.Left() * nScaleFactor, aTmp.Top() * nScaleFactor,
                            aTmp.Right() * nScaleFactor, aTmp.Bottom() * nScaleFactor);
        if (&rDev != pGlyphDev) /* only when rDev is a printer... */
        {
            long nGDTextWidth  = pGlyphDev->GetTextWidth(rText);
            if (nGDTextWidth != 0  &&
                nTextWidth != nGDTextWidth)
            {
                aResult.Right() *= nTextWidth;
                aResult.Right() /= nGDTextWidth * nScaleFactor;
            }
        }
    }

    // move rectangle to match possibly different baselines
    // (because of different devices)
    long nDelta = aDevFM.GetAscent() - pGlyphDev->GetFontMetric().GetAscent() * nScaleFactor;
    aResult.Move(0, nDelta);

    pGlyphDev->Pop();

    rRect = aResult;
    return bSuccess;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
