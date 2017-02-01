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

#ifndef INCLUDED_STARMATH_INC_RECT_HXX
#define INCLUDED_STARMATH_INC_RECT_HXX

#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <tools/gen.hxx>
#include <vcl/outdev.hxx>
#include <vcl/metric.hxx>

#include "format.hxx"


inline long SmFromTo(long nFrom, long nTo, double fRelDist)
{
    return nFrom + static_cast<long>(fRelDist * (nTo - nFrom));
}


// SmRect
// ... (to be done)
// This Implementation assumes that the x-axis points to the right and the
// y-axis to the bottom.
// Note: however, italic spaces can be negative!


// possible positions and alignments for the 'AlignTo' function
enum class RectPos
{
    Left, // align the current object to the left of the argument
    Right,
    Top,
    Bottom,
    Attribute
};

enum class RectHorAlign
{
    Left,
    Center,
    Right
};

enum class RectVerAlign
{
    Top,
    Mid,
    Bottom,
    Baseline,
    CenterY,
    AttributeHi,
    AttributeMid,
    AttributeLo
};

// different methods of copying baselines and mid's in 'ExtendBy' function
enum class RectCopyMBL
{
    This,   // keep baseline of current object even if it has none
    Arg,    // as above but for the argument
    None,   // result will have no baseline
    Xor     // if current object has a baseline keep it else copy
            //   the arguments baseline (even if it has none)
};


class SmRect
{
    Point   aTopLeft;
    Size    aSize;
    long    nBaseline,
            nAlignT,
            nAlignM,
            nAlignB,
            nGlyphTop,
            nGlyphBottom,
            nItalicLeftSpace,
            nItalicRightSpace,
            nLoAttrFence,
            nHiAttrFence;
    sal_uInt16  nBorderWidth;
    bool    bHasBaseline,
            bHasAlignInfo;

    inline  void CopyMBL(const SmRect& rRect);
            void CopyAlignInfo(const SmRect& rRect);

            void Union(const SmRect &rRect);

public:
            SmRect();
            SmRect(const OutputDevice &rDev, const SmFormat *pFormat,
                   const OUString &rText, sal_uInt16 nBorderWidth);
            SmRect(long nWidth, long nHeight);


            sal_uInt16  GetBorderWidth() const  { return nBorderWidth; }

            void SetItalicSpaces(long nLeftSpace, long nRightSpace);

            void SetWidth(sal_uLong nWidth)     { aSize.Width()  = nWidth; }

            void SetLeft(long nLeft);
            void SetRight(long nRight);
            void SetBottom(long nBottom);
            void SetTop(long nTop);

            const Point & GetTopLeft() const { return aTopLeft; }

            long GetTop()     const { return GetTopLeft().Y(); }
            long GetLeft()    const { return GetTopLeft().X(); }
            long GetBottom()  const { return GetTop() + GetHeight() - 1; }
            long GetRight()   const { return GetLeft() + GetWidth() - 1; }
            long GetCenterY() const { return (GetTop() + GetBottom()) / 2; }
            long GetWidth()   const { return GetSize().Width(); }
            long GetHeight()  const { return GetSize().Height(); }

            long GetItalicLeftSpace()  const { return nItalicLeftSpace; }
            long GetItalicRightSpace() const { return nItalicRightSpace; }

            long GetHiAttrFence() const     { return nHiAttrFence; }
            long GetLoAttrFence() const     { return nLoAttrFence; }

            long GetItalicLeft() const      { return GetLeft() - GetItalicLeftSpace(); }
            long GetItalicCenterX() const   { return (GetItalicLeft() + GetItalicRight()) / 2; }
            long GetItalicRight() const     { return GetRight() + GetItalicRightSpace(); }
            long GetItalicWidth() const     { return GetWidth() + GetItalicLeftSpace() + GetItalicRightSpace(); }

            bool HasBaseline() const        { return bHasBaseline; }
    inline  long GetBaseline() const;
            long GetBaselineOffset() const  { return GetBaseline() - GetTop(); }

            long GetAlignT() const  { return nAlignT; }
            long GetAlignM() const  { return nAlignM; }
            long GetAlignB() const  { return nAlignB; }

            const Size & GetSize() const    { return aSize; }

            const Size  GetItalicSize() const
            {   return Size(GetItalicWidth(), GetHeight()); }

            void Move  (const Point &rPosition);
            void MoveTo(const Point &rPosition) { Move(rPosition - GetTopLeft()); }

            bool IsEmpty() const
            {
                return GetWidth() == 0  ||  GetHeight() == 0;
            }

            bool HasAlignInfo() const { return bHasAlignInfo; }

            const Point AlignTo(const SmRect &rRect, RectPos ePos,
                                RectHorAlign eHor, RectVerAlign eVer) const;

            SmRect & ExtendBy(const SmRect &rRect, RectCopyMBL eCopyMode);
            void     ExtendBy(const SmRect &rRect, RectCopyMBL eCopyMode,
                              long nNewAlignM);
            SmRect & ExtendBy(const SmRect &rRect, RectCopyMBL eCopyMode,
                      bool bKeepVerAlignParams);

            long    OrientedDist(const Point &rPoint) const;
            bool    IsInsideRect(const Point &rPoint) const;
            bool    IsInsideItalicRect(const Point &rPoint) const;

    inline  Rectangle   AsRectangle() const;
            SmRect      AsGlyphRect() const;
};


inline void SmRect::SetItalicSpaces(long nLeftSpace, long nRightSpace)
    // set extra spacing to the left and right for (italic)
    // letters/text
{
    nItalicLeftSpace  = nLeftSpace;
    nItalicRightSpace = nRightSpace;
}


inline void SmRect::CopyMBL(const SmRect &rRect)
    // copy AlignM baseline and value of 'rRect'
{
    nBaseline    = rRect.nBaseline;
    bHasBaseline = rRect.bHasBaseline;
    nAlignM      = rRect.nAlignM;
}


inline long SmRect::GetBaseline() const
{
    SAL_WARN_IF( !HasBaseline(), "starmath", "Baseline does not exist" );
    return nBaseline;
}


inline Rectangle SmRect::AsRectangle() const
{
    return Rectangle(Point(GetItalicLeft(), GetTop()), GetItalicSize());
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
