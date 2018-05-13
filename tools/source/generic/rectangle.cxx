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

#include <tools/Rectangle.hxx>
#include <tools/stream.hxx>

#include <o3tl/safeint.hxx>

namespace tools
{
void Rectangle::SetSize(const Size& rSize)
{
    if (rSize.Width() < 0)
        mnRight = mnLeft + rSize.Width() + 1;
    else if (rSize.Width() > 0)
        mnRight = mnLeft + rSize.Width() - 1;
    else
        mnRight = RECT_EMPTY;

    if (rSize.Height() < 0)
        mnBottom = mnTop + rSize.Height() + 1;
    else if (rSize.Height() > 0)
        mnBottom = mnTop + rSize.Height() - 1;
    else
        mnBottom = RECT_EMPTY;
}

void Rectangle::SaturatingSetSize(const Size& rSize)
{
    if (rSize.Width() < 0)
        mnRight = o3tl::saturating_add(mnLeft, (rSize.Width() + 1));
    else if (rSize.Width() > 0)
        mnRight = o3tl::saturating_add(mnLeft, (rSize.Width() - 1));
    else
        mnRight = RECT_EMPTY;

    if (rSize.Height() < 0)
        mnBottom = o3tl::saturating_add(mnTop, (rSize.Height() + 1));
    else if (rSize.Height() > 0)
        mnBottom = o3tl::saturating_add(mnTop, (rSize.Height() - 1));
    else
        mnBottom = RECT_EMPTY;
}

void Rectangle::SaturatingSetX(long x)
{
    mnRight = o3tl::saturating_add(mnRight, x - mnLeft);
    mnLeft = x;
}

void Rectangle::SaturatingSetY(long y)
{
    mnBottom = o3tl::saturating_add(mnBottom, y - mnTop);
    mnTop = y;
}

Rectangle& Rectangle::Union(const Rectangle& rRect)
{
    if (rRect.IsEmpty())
        return *this;

    if (IsEmpty())
    {
        *this = rRect;
    }
    else
    {
        mnLeft = std::min(std::min(mnLeft, rRect.mnLeft), std::min(mnRight, rRect.mnRight));
        mnRight = std::max(std::max(mnLeft, rRect.mnLeft), std::max(mnRight, rRect.mnRight));
        mnTop = std::min(std::min(mnTop, rRect.mnTop), std::min(mnBottom, rRect.mnBottom));
        mnBottom = std::max(std::max(mnTop, rRect.mnTop), std::max(mnBottom, rRect.mnBottom));
    }

    return *this;
}

Rectangle& Rectangle::Intersection(const Rectangle& rRect)
{
    if (IsEmpty())
        return *this;

    if (rRect.IsEmpty())
    {
        *this = Rectangle();
        return *this;
    }

    // Justify rectangle
    Rectangle aTmpRect(rRect);
    Justify();
    aTmpRect.Justify();

    // Perform intersection
    mnLeft = std::max(mnLeft, aTmpRect.mnLeft);
    mnRight = std::min(mnRight, aTmpRect.mnRight);
    mnTop = std::max(mnTop, aTmpRect.mnTop);
    mnBottom = std::min(mnBottom, aTmpRect.mnBottom);

    // Determine if intersection is empty
    if (mnRight < mnLeft || mnBottom < mnTop)
        *this = Rectangle();

    return *this;
}

void Rectangle::Justify()
{
    long nHelp;

    if ((mnRight < mnLeft) && (mnRight != RECT_EMPTY))
    {
        nHelp = mnLeft;
        mnLeft = mnRight;
        mnRight = nHelp;
    }

    if ((mnBottom < mnTop) && (mnBottom != RECT_EMPTY))
    {
        nHelp = mnBottom;
        mnBottom = mnTop;
        mnTop = nHelp;
    }
}

bool Rectangle::IsInside(const Point& rPoint) const
{
    if (IsEmpty())
        return false;

    if (mnLeft <= mnRight)
    {
        if ((rPoint.X() < mnLeft) || (rPoint.X() > mnRight))
            return false;
    }
    else
    {
        if ((rPoint.X() > mnLeft) || (rPoint.X() < mnRight))
            return false;
    }

    if (mnTop <= mnBottom)
    {
        if ((rPoint.Y() < mnTop) || (rPoint.Y() > mnBottom))
            return false;
    }
    else
    {
        if ((rPoint.Y() > mnTop) || (rPoint.Y() < mnBottom))
            return false;
    }

    return true;
}

bool Rectangle::IsInside(const Rectangle& rRect) const
{
    if (IsInside(rRect.TopLeft()) && IsInside(rRect.BottomRight()))
        return true;
    else
        return false;
}

bool Rectangle::IsOver(const Rectangle& rRect) const
{
    // If there's no intersection, they don't overlap
    return !GetIntersection(rRect).IsEmpty();
}

SvStream& ReadRectangle(SvStream& rIStream, Rectangle& rRect)
{
    sal_Int32 nTmpL(0), nTmpT(0), nTmpR(0), nTmpB(0);

    rIStream.ReadInt32(nTmpL).ReadInt32(nTmpT).ReadInt32(nTmpR).ReadInt32(nTmpB);

    rRect.mnLeft = nTmpL;
    rRect.mnTop = nTmpT;
    rRect.mnRight = nTmpR;
    rRect.mnBottom = nTmpB;

    return rIStream;
}

SvStream& WriteRectangle(SvStream& rOStream, const Rectangle& rRect)
{
    rOStream.WriteInt32(rRect.mnLeft)
        .WriteInt32(rRect.mnTop)
        .WriteInt32(rRect.mnRight)
        .WriteInt32(rRect.mnBottom);

    return rOStream;
}

OString Rectangle::toString() const
{
    std::stringstream ss;
    // Note that this is not just used for debugging output but the
    // format is parsed by external code (passed in callbacks to
    // LibreOfficeKit clients). So don't change.
    ss << getX() << ", " << getY() << ", " << getWidth() << ", " << getHeight();
    return ss.str().c_str();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
