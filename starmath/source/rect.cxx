/*************************************************************************
 *
 *  $RCSfile: rect.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2000-10-12 08:25:53 $
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

#pragma hdrstop

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif


#include "rect.hxx"
#include "types.hxx"
#include "xchar.hxx"
#include "utility.hxx"
#include "smmod.hxx"


////////////////////////////////////////////////////////////////////////////////


// '\0' terminiertes Array mit Zeichen, die im StarMath Font als Buchstaben
// betrachtet werden sollen, (um im Gegensatz zu den anderen Operatoren
// und Symbolen ein "normales"(ungecliptes) SmRect zu erhalten).
static xub_Unicode __READONLY_DATA aMathAlpha[] =
{
    MS_ALEPH,       MS_IM,          MS_RE,          MS_WP,
    xub_Unicode('\x70'), MS_EMPTYSET,    xub_Unicode('\xF0'),   xub_Unicode('\xF1'),
    xub_Unicode('\xF2'),    xub_Unicode('\xF3'),    xub_Unicode('\xF4'), MS_HBAR,
    MS_LAMBDABAR,   MS_SETN,        MS_SETZ,        MS_SETQ,
    MS_SETR,        MS_SETC,        xub_Unicode('\xB4'),        xub_Unicode('\xB5'),
    xub_Unicode('\xB8'),    xub_Unicode('\xB9'),    xub_Unicode('\xBA'),
    xub_Unicode('\0')
};

BOOL SmIsMathAlpha(const XubString &rText)
    // ergibt genau dann TRUE, wenn das Zeichen (aus dem StarMath Font) wie ein
    // Buchstabe behandelt werden soll.
{
    if (rText.Len() == 0)
        return FALSE;

    DBG_ASSERT(rText.Len() == 1, "Sm : String enthält nicht genau ein Zeichen");
    xub_Unicode cChar = rText.GetChar(0);

    // ist es ein griechisches Zeichen ?
    if (xub_Unicode('\xC6') <= cChar  &&  cChar <= xub_Unicode('\xEE'))
        return TRUE;
    else
    {
        // kommt es in 'aMathAlpha' vor ?
        const xub_Unicode *pChar = aMathAlpha;
        while (*pChar  &&  *pChar != cChar)
            pChar++;
        return *pChar != xub_Unicode('\0');
    }
}


////////////////////////////////////////
//
// SmRect members
//


SmRect::SmRect()
    // constructs empty rectangle at (0, 0) with width and height 0.
{
    DBG_ASSERT(aTopLeft == Point(0, 0), "Sm: ooops...");
    DBG_ASSERT(aSize == Size(0, 0), "Sm: ooops...");

    bHasBaseline = bHasAlignInfo = FALSE;
    nBaseline = nAlignT = nAlignM = nAlignB =
    nGlyphTop = nGlyphBottom =
    nItalicLeftSpace = nItalicRightSpace =
    nLoAttrFence = nHiAttrFence = 0;
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
                       const XubString &rText, long nBorderWidth)
{
#ifndef PRODUCT
    if (rDev.GetOutDevType() != OUTDEV_PRINTER)
        DBG_WARNING("Sm :  Referenz-Device ist kein Drucker");
#endif

    DBG_ASSERT(aTopLeft == Point(0, 0), "Sm: Ooops...");

    aSize = Size(rDev.GetTextWidth(rText), rDev.GetTextHeight());

    const FontMetric  aFM (rDev.GetFontMetric());
    BOOL              bIsMath  = aFM.GetName().EqualsIgnoreCaseAscii("StarMath");
    BOOL              bAllowSmaller = bIsMath && !SmIsMathAlpha(rText);
    const long        nFontHeight = rDev.GetFont().GetSize().Height();

    bHasAlignInfo = TRUE;
    bHasBaseline  = TRUE;
    nBaseline     = aFM.GetAscent();
    nAlignT       = nBaseline - nFontHeight * 750L / 1000L;
    nAlignM       = nBaseline - nFontHeight * 121L / 422L;
        // that's where the horizontal bars of '+', '-', ... are
        // (1/3 of ascent over baseline)
        // (121 = 1/3 of 12pt ascent, 422 = 12pt fontheight)
    nAlignB       = nBaseline;

    // workaround for printer fonts with very small (possible 0 or even
    // negative(!)) leading
    if (aFM.GetLeading() < 5  &&  rDev.GetOutDevType() == OUTDEV_PRINTER)
    {
        OutputDevice    *pWindow = Application::GetDefaultDevice();

        pWindow->Push(PUSH_MAPMODE | PUSH_FONT);

        pWindow->SetMapMode(rDev.GetMapMode());
        pWindow->SetFont(rDev.GetFontMetric());

        long  nDelta = pWindow->GetFontMetric().GetLeading();
        if (nDelta == 0)
        {   // dieser Wert entspricht etwa einem Leading von 80 bei einer
            // Fonthöhe von 422 (12pt)
            nDelta = nFontHeight * 8L / 43;
        }
        SetTop(GetTop() - nDelta);

        pWindow->Pop();
    }

    // get GlyphBoundRect
    Rectangle  aGlyphRect;
    BOOL       bSuccess = SmGetGlyphBoundRect(rDev, rText, aGlyphRect);
    DBG_ASSERT(bSuccess, "Sm : Ooops... (fehlt evtl. der Font?)");

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
        // für Symbole und Operatoren aus dem StarMath Font passen wir den
        // oberen und unteren Rand dem Zeichen an.
        SetTop(nGlyphTop);
        SetBottom(nGlyphBottom);
    }

    if (nHiAttrFence < GetTop())
        nHiAttrFence = GetTop();

    if (nLoAttrFence > GetBottom())
        nLoAttrFence = GetBottom();

    DBG_ASSERT(rText.Len() == 0  ||  !IsEmpty(),
               "Sm: leeres Rechteck erzeugt");
}


void SmRect::Init(const OutputDevice &rDev, const SmFormat *pFormat,
                  const XubString &rText, long nBorderWidth)
    // get rectangle fitting for drawing 'rText' on OutputDevice 'rDev'
{
    SmRectCache *pRectCache = SM_MOD1()->GetRectCache();
    DBG_ASSERT(pRectCache, "Sm : NULL pointer");

    // build key for rectangle (to look up in cache for)
    const SmRectCache::Key  aKey (rText, rDev.GetFont());

    const SmRect *pResult = pRectCache->Search(aKey);
    if (pResult)
        *this = *pResult;
    else
    {   // build rectangle and put it in cache
        BuildRect(rDev, pFormat, rText, nBorderWidth);
        pResult = pRectCache->Add(aKey, *this);
    }
    DBG_ASSERT(pResult, "Sm : NULL pointer");
}


SmRect::SmRect(const OutputDevice &rDev, const SmFormat *pFormat,
               const XubString &rText, long nBorderWidth)
{
    Init(rDev, pFormat, rText, nBorderWidth);
}


SmRect::SmRect(const OutputDevice &rDev, const SmFormat *pFormat,
               const SmPolygon &rPoly, long nBorderWidth)
{
    Init(rDev, pFormat, rPoly.GetChar(), nBorderWidth);

    // den Offset in der Zeichenzelle passend waehlen
    Point aPolyOffset (rPoly.GetOrigPos());
    aPolyOffset.X() *= rPoly.GetScaleX();
    aPolyOffset.Y() *= rPoly.GetScaleY();

    // und es an diese Position schieben
    Rectangle aPolyRect ( rPoly.GetBoundRect(rDev) );
    Point  aDelta (aPolyOffset - aPolyRect.TopLeft());
    aPolyRect.Move( aDelta.X(), aDelta.Y() );

    aTopLeft.X() = aPolyRect.Left() - nBorderWidth;
    aTopLeft.Y() = aPolyRect.Top()  - nBorderWidth;

    aSize = aPolyRect.GetSize();
    aSize.Width()  += 2 * nBorderWidth;
    aSize.Height() += 2 * nBorderWidth;

    nItalicLeftSpace = nItalicRightSpace = 0;
}


SmRect::SmRect(long nWidth, long nHeight)
    // this constructor should never be used for anything textlike because
    // it will not provide useful values for baseline, AlignT and AlignB!
    // It's purpose is to get a 'SmRect' for the horizontal line in fractions
    // as used in 'SmBinVerNode'.
:   aSize(nWidth, nHeight)
{
    DBG_ASSERT(aTopLeft == Point(0, 0), "Sm: ooops...");

    bHasBaseline  = FALSE;
    bHasAlignInfo = TRUE;
    nBaseline     = 0;
    nAlignT       = GetTop();
    nAlignB       = GetBottom();
    nAlignM       = (nAlignT + nAlignB) / 2;        // this is the default
    nItalicLeftSpace = nItalicRightSpace = 0;
    nGlyphTop    = nHiAttrFence  = GetTop();
    nGlyphBottom = nLoAttrFence  = GetBottom();
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
            DBG_ASSERT(FALSE, "Sm: unbekannter Fall");
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
                DBG_ASSERT(FALSE, "Sm: unbekannter Fall");
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
                DBG_ASSERT(FALSE, "Sm: unbekannter Fall");
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
    long  nL = Min(GetItalicLeft(),  rRect.GetItalicLeft()),
          nR = Max(GetItalicRight(), rRect.GetItalicRight());

    Union(rRect);

    SetItalicSpaces(GetLeft() - nL, nR - GetRight());

    if (!HasAlignInfo())
        CopyAlignInfo(rRect);
    else if (rRect.HasAlignInfo())
    {   nAlignT = Min(GetAlignT(), rRect.GetAlignT());
        nAlignB = Max(GetAlignB(), rRect.GetAlignB());
        nHiAttrFence = Min(GetHiAttrFence(), rRect.GetHiAttrFence());
        nLoAttrFence = Max(GetLoAttrFence(), rRect.GetLoAttrFence());
        DBG_ASSERT(HasAlignInfo(), "Sm: ooops...");

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
                DBG_ASSERT(FALSE, "Sm: unbekannter Fall");
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
    DBG_ASSERT(HasAlignInfo(), "Sm: keine Align Info");

    ExtendBy(rRect, eCopyMode);
    nAlignM = nNewAlignM;

    return *this;
}


SmRect & SmRect::ExtendBy(const SmRect &rRect, RectCopyMBL eCopyMode,
                          BOOL bKeepVerAlignParams)
    // as 'ExtendBy' but keeps original values for AlignT, -M and -B and
    // baseline.
    // (this is used in 'SmSupSubNode' where the sub-/supscripts shouldn't
    // be allowed to modify these values.)
{
    long  nOldAlignT   = GetAlignT(),
          nOldAlignM   = GetAlignM(),
          nOldAlignB   = GetAlignB(),
          nOldBaseline = nBaseline;     //! depends not on 'HasBaseline'
    BOOL  bOldHasAlignInfo = HasAlignInfo();

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


SmRect & SmRect::ExtendBy(const Point &rPoint)
    // extend current rectangle to include 'rPoint'.
    // The effect should be similar to
    //      "ExtendBy(rRect, RCP_THIS, (BOOL) TRUE)"
    // where 'rRect' is a SmRect of size and width 1 with no italic spaces
    // (as by "SmRect (1, 1)") and position at 'rPoint'.
{
    // get some values used for italic spaces adaption
    // ! (need to be done before changing current SmRect) !
    long  nL = Min(GetItalicLeft(),  rPoint.X()),
          nR = Max(GetItalicRight(), rPoint.X());

    // this is the adaption of rectangle union
    if (rPoint.X() < GetLeft())
        SetLeft(rPoint.X());
    if (rPoint.X() > GetRight())
        SetRight(rPoint.X());
    if (rPoint.Y() < GetTop())
        SetTop(rPoint.Y());
    if (rPoint.Y() > GetBottom())
        SetBottom(rPoint.Y());

    SetItalicSpaces(GetLeft() - nL, nR - GetRight());

    return *this;
}


long SmRect::OrientedDist(const Point &rPoint) const
    // return oriented distance of rPoint to the current rectangle,
    // especially the return value is <= 0 iff the point is inside the
    // rectangle.
    // For simplicity the maximum-norm is used.
{
    BOOL  bIsInside = IsInsideItalicRect(rPoint);

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

    return bIsInside ? - Min(nAbsX, nAbsY) : Max (nAbsX, nAbsY);
}


BOOL SmRect::IsInsideRect(const Point &rPoint) const
{
    return     rPoint.Y() >= GetTop()
           &&  rPoint.Y() <= GetBottom()
           &&  rPoint.X() >= GetLeft()
           &&  rPoint.X() <= GetRight();
}


BOOL SmRect::IsInsideItalicRect(const Point &rPoint) const
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


// forward declaration
void SmDrawFrame(OutputDevice &rDev, const Rectangle &rRec,
                 const Color aCol = COL_BLACK);

void SmRect::Draw(OutputDevice &rDev, const Point &rPosition, int nFlags) const
{
    if (IsEmpty())
        return;

    rDev.Push(PUSH_LINECOLOR);

    if (nFlags & SM_RECT_LINES)
    {   long   nLeftSpace  = 0,
               nRightSpace = 0;

        if (nFlags & SM_RECT_ITALIC)
        {   nLeftSpace  = GetItalicLeftSpace();
            nRightSpace = GetItalicRightSpace();
        }

        long  nLeft  = GetLeft()  - nLeftSpace,
              nRight = GetRight() + nRightSpace;

        Point aOffset (rPosition - GetTopLeft());

        rDev.SetLineColor(COL_LIGHTBLUE);
        rDev.DrawLine(Point(nLeft,  GetAlignB()) += aOffset,
                      Point(nRight, GetAlignB()) += aOffset);
        rDev.DrawLine(Point(nLeft,  GetAlignT()) += aOffset,
                      Point(nRight, GetAlignT()) += aOffset);
        if (HasBaseline())
            rDev.DrawLine(Point(nLeft,  GetBaseline()) += aOffset,
                          Point(nRight, GetBaseline()) += aOffset);

        rDev.SetLineColor(COL_GRAY);
        rDev.DrawLine(Point(nLeft,  GetHiAttrFence()) += aOffset,
                      Point(nRight, GetHiAttrFence()) += aOffset);
    }

    if (nFlags & SM_RECT_MID)
    {   Point   aCenter = rPosition
                          + (Point(GetItalicCenterX(), GetAlignM()) -= GetTopLeft()),
                aLenX     (GetWidth() / 5, 0),
                aLenY     (0, GetHeight() / 16);

        rDev.SetLineColor(COL_LIGHTGREEN);
        rDev.DrawLine(aCenter - aLenX, aCenter + aLenX);
        rDev.DrawLine(aCenter - aLenY, aCenter + aLenY);
    }

    if (nFlags & SM_RECT_ITALIC)
        SmDrawFrame(rDev, Rectangle(rPosition - Point(GetItalicLeftSpace(), 0),
                GetItalicSize()));

    if (nFlags & SM_RECT_CORE)
        SmDrawFrame(rDev, Rectangle(rPosition, GetSize()), COL_LIGHTRED);

    rDev.Pop();
}



////////////////////////////////////////
// misc functions
//


void SmDrawFrame(OutputDevice &rDev, const Rectangle &rRec,
                 const Color aCol)
{
    rDev.Push(PUSH_LINECOLOR);

    rDev.SetLineColor(aCol);

    rDev.DrawLine(rRec.TopLeft(),     rRec.BottomLeft());
    rDev.DrawLine(rRec.BottomLeft(),  rRec.BottomRight());
    rDev.DrawLine(rRec.BottomRight(), rRec.TopRight());
    rDev.DrawLine(rRec.TopRight(),    rRec.TopLeft());

    rDev.Pop();
}


BOOL SmGetGlyphBoundRect(const OutputDevice &rDev,
                         const XubString &rText, Rectangle &rRect)
    // basically the same as 'GetGlyphBoundRect' (in class 'OutputDevice')
    // but with a string as argument.
{
    // handle special case first
    xub_StrLen nLen = rText.Len();
    if (nLen == 0)
    {   rRect.SetEmpty();
        return TRUE;
    }

    // get a device where 'OutputDevice::GetGlyphBoundRect' will be successful
    OutputDevice *pGlyphDev;
    if (rDev.GetOutDevType() != OUTDEV_PRINTER)
        pGlyphDev = (OutputDevice *) &rDev;
    else
    {
        // since we format for the printer (where GetGlyphBoundRect will fail)
        // we need a virtual device here.
        pGlyphDev = SM_MOD1()->GetRectCache()->GetVirDev();
    }

    const FontMetric  aDevFM (rDev.GetFontMetric());

    pGlyphDev->Push(PUSH_FONT);
    pGlyphDev->SetFont(rDev.GetFont());
    //! Da in der FontMetric die Weite immer != 0 ist (was fuer wide-Attribute
    //! und skalierbare Klammern auch so benötigt wird) kann dies zu einer
    //! Verzerrung der Proportionen im 'pGlyphDev' gegnüber dem 'rDev' kommen!

    const BOOL  bOptimize = FALSE;
    BOOL        bSuccess  = TRUE;
    Point       aPoint;
    Rectangle   aResult (aPoint, Size(rDev.GetTextWidth(rText), rDev.GetTextHeight())),
                aTmp;
    long        nDelta;

    // setzen des linken Randes (dabei Leerzeichen erhalten!)
    xub_Unicode  cChar = rText.GetChar(0);
    if (cChar != xub_Unicode(' '))
    {
        bSuccess &= pGlyphDev->GetGlyphBoundRect(cChar, aTmp, bOptimize);
        if (!aTmp.IsEmpty())
        {
            // linken Rand am 'rDev' ermitteln
            // (wir nehmen den linken Rand bezüglich 'pGlyphDev' und skalieren
            // ihn passen für 'rDev')
            long nLeftSpace = aTmp.Left() * rDev.GetTextWidth(cChar)
                                        / pGlyphDev->GetTextWidth(cChar);
            aResult.Left() += nLeftSpace;
        }
    }

    // setzen des rechten Randes (dabei Leerzeichen erhalten!)
    cChar = rText.GetChar(nLen - 1);
    if (cChar != xub_Unicode(' '))
    {
        bSuccess &= pGlyphDev->GetGlyphBoundRect(cChar, aTmp, bOptimize);
        if (!aTmp.IsEmpty())
        {
            // rechten Rand am 'rDev' ermitteln (analog wie beim linken Rand)
            long nGlyphWidth = pGlyphDev->GetTextWidth(cChar),
                 nRightSpace = (nGlyphWidth - 1 - aTmp.Right())
                                    * rDev.GetTextWidth(cChar)
                                    / nGlyphWidth;
            aResult.Right() -= nRightSpace;
        }
    }

    // oberen und unteren Rand bestimmen.
    // Im Augenblick gehen wird davon aus, daß die Texthöhen an den beiden
    // Devices im wesentlichen gleich sind und skalieren diese Ränder daher
    // nicht um.
    long  nTop    = aResult.Bottom() + 1,
          nBottom = aResult.Top() - 1;
    for (USHORT i = 0;  i < nLen;  i++)
    {
        cChar = rText.GetChar(i);
        if (cChar != xub_Unicode(' '))
        {
            //! Anmerkung: Leerzeichen *können* leere Rechtecke ergeben, aber
            //! der Returnwert sollte auch dann TRUE sein.
            bSuccess &= pGlyphDev->GetGlyphBoundRect(cChar, aTmp, bOptimize);

            if (!aTmp.IsEmpty()  &&  aTmp.Top() < nTop)
                nTop = aTmp.Top();
            if (!aTmp.IsEmpty()  &&  aTmp.Bottom() > nBottom)
                nBottom = aTmp.Bottom();
        }
    }
    aResult.Top()    = nTop;
    aResult.Bottom() = nBottom;

    // move rectangle to match possibly different baselines
    // (because of different devices)
    nDelta = aDevFM.GetAscent() - pGlyphDev->GetFontMetric().GetAscent();
    aResult.Move(0, nDelta);

    rRect = aResult;
    pGlyphDev->Pop();
    return bSuccess;
}


