/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>


#include "rect.hxx"
#include "types.hxx"
#include "utility.hxx"
#include "smmod.hxx"








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
    
{
    if (rText.isEmpty())
        return false;

    OSL_ENSURE(rText.getLength() == 1, "Sm : string must be exactly one character long");
    sal_Unicode cChar = rText[0];

    
    if (sal_Unicode(0xE0AC) <= cChar  &&  cChar <= sal_Unicode(0xE0D4))
        return true;
    else
    {
        
        const sal_Unicode *pChar = aMathAlpha;
        while (*pChar  &&  *pChar != cChar)
            pChar++;
        return *pChar != '\0';
    }
}



//

//


SmRect::SmRect()
    
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
    bool              bIsMath  = aFM.GetName().equalsIgnoreAsciiCase( FONTNAME_MATH );
    bool              bAllowSmaller = bIsMath && !SmIsMathAlpha(rText);
    const long        nFontHeight = rDev.GetFont().GetSize().Height();

    nBorderWidth  = nBorder;
    bHasAlignInfo = true;
    bHasBaseline  = true;
    nBaseline     = aFM.GetAscent();
    nAlignT       = nBaseline - nFontHeight * 750L / 1000L;
    nAlignM       = nBaseline - nFontHeight * 121L / 422L;
        
        
        
    nAlignB       = nBaseline;

    
    
    if (aFM.GetIntLeading() < 5  &&  rDev.GetOutDevType() == OUTDEV_PRINTER)
    {
        OutputDevice    *pWindow = Application::GetDefaultDevice();

        pWindow->Push(PUSH_MAPMODE | PUSH_FONT);

        pWindow->SetMapMode(rDev.GetMapMode());
        pWindow->SetFont(rDev.GetFontMetric());

        long  nDelta = pWindow->GetFontMetric().GetIntLeading();
        if (nDelta == 0)
        {   
            
            nDelta = nFontHeight * 8L / 43;
        }
        SetTop(GetTop() - nDelta);

        pWindow->Pop();
    }

    
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
    
    
    
    
:   aSize(nWidth, nHeight)
{
    OSL_ENSURE(aTopLeft == Point(0, 0), "Sm: ooops...");

    bHasBaseline  = false;
    bHasAlignInfo = true;
    nBaseline     = 0;
    nAlignT       = GetTop();
    nAlignB       = GetBottom();
    nAlignM       = (nAlignT + nAlignB) / 2;        
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

    
    if (ePos == RP_LEFT  ||  ePos == RP_RIGHT  ||  ePos == RP_ATTRIBUT)
        
        switch (eVer)
        {   case RVA_TOP :
                aPos.Y() += rRect.GetAlignT() - GetAlignT();
                break;
            case RVA_MID :
                aPos.Y() += rRect.GetAlignM() - GetAlignM();
                break;
            case RVA_BASELINE :
                
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

    
    if (ePos == RP_TOP  ||  ePos == RP_BOTTOM)
        
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
    
    
    
    
    
{
    
    
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
    
    
    
    
{
    OSL_ENSURE(HasAlignInfo(), "Sm: no align info");

    ExtendBy(rRect, eCopyMode);
    nAlignM = nNewAlignM;

    return *this;
}


SmRect & SmRect::ExtendBy(const SmRect &rRect, RectCopyMBL eCopyMode,
                          bool bKeepVerAlignParams)
    
    
    
    
{
    long  nOldAlignT   = GetAlignT(),
          nOldAlignM   = GetAlignM(),
          nOldAlignB   = GetAlignB(),
          nOldBaseline = nBaseline;     
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
    
    
    
    
{
    bool  bIsInside = IsInsideItalicRect(rPoint);

    
    Point  aRef;
    if (bIsInside)
    {   Point  aIC (GetItalicCenterX(), GetCenterY());

        aRef.X() = rPoint.X() >= aIC.X() ? GetItalicRight() : GetItalicLeft();
        aRef.Y() = rPoint.Y() >= aIC.Y() ? GetBottom() : GetTop();
    }
    else
    {
        
        if (rPoint.X() > GetItalicRight())
            aRef.X() = GetItalicRight();
        else if (rPoint.X() < GetItalicLeft())
            aRef.X() = GetItalicLeft();
        else
            aRef.X() = rPoint.X();
        
        if (rPoint.Y() > GetBottom())
            aRef.Y() = GetBottom();
        else if (rPoint.Y() < GetTop())
            aRef.Y() = GetTop();
        else
            aRef.Y() = rPoint.Y();
    }

    
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

bool SmGetGlyphBoundRect(const OutputDevice &rDev,
                         const OUString &rText, Rectangle &rRect)
    
    
{
    
    if (rText.isEmpty())
    {
        rRect.SetEmpty();
        return true;
    }

    
    OutputDevice *pGlyphDev;
    if (rDev.GetOutDevType() != OUTDEV_PRINTER)
        pGlyphDev = (OutputDevice *) &rDev;
    else
    {
        
        
        pGlyphDev = &SM_MOD()->GetDefaultVirtualDev();
    }

    const FontMetric  aDevFM (rDev.GetFontMetric());

    pGlyphDev->Push(PUSH_FONT | PUSH_MAPMODE);
    Font aFnt(rDev.GetFont());
    aFnt.SetAlign(ALIGN_TOP);

    
    
    
    Size aFntSize = aFnt.GetSize();

    
    long nScaleFactor = 1;
    while( aFntSize.Height() > 2000 * nScaleFactor )
        nScaleFactor *= 2;

    aFnt.SetSize( Size( aFntSize.Width() / nScaleFactor, aFntSize.Height() / nScaleFactor ) );
    pGlyphDev->SetFont(aFnt);

    long nTextWidth = rDev.GetTextWidth(rText);
    Point aPoint;
    Rectangle   aResult (aPoint, Size(nTextWidth, rDev.GetTextHeight())),
                aTmp;

    bool bSuccess = pGlyphDev->GetTextBoundRect(aTmp, rText, 0, 0);
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

    
    
    long nDelta = aDevFM.GetAscent() - pGlyphDev->GetFontMetric().GetAscent() * nScaleFactor;
    aResult.Move(0, nDelta);

    pGlyphDev->Pop();

    rRect = aResult;
    return bSuccess;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
