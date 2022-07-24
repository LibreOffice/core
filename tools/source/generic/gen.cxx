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

#include <sal/config.h>
#include <rtl/string.hxx>

#include <algorithm>
#include <tuple>
#include <o3tl/hash_combine.hxx>
#include <o3tl/safeint.hxx>
#include <tools/gen.hxx>

OString Pair::toString() const
{
    // Note that this is not just used for debugging output but the
    // format is parsed by external code (passed in callbacks to
    // LibreOfficeKit clients). So don't change.
    return OString::number(A()) + ", " + OString::number(B());
}

size_t Pair::GetHashValue() const
{
    size_t hash = 0;
    o3tl::hash_combine( hash, mnA );
    o3tl::hash_combine( hash, mnB );
    return hash;
}

void tools::Rectangle::SaturatingSetSize(const Size& rSize)
{
    if (rSize.Width() < 0)
        mnRight = o3tl::saturating_add(mnLeft, (rSize.Width() + 1));
    else if ( rSize.Width() > 0 )
        mnRight = o3tl::saturating_add(mnLeft, (rSize.Width() - 1));
    else
        SetWidthEmpty();

    if ( rSize.Height() < 0 )
        mnBottom = o3tl::saturating_add(mnTop, (rSize.Height() + 1));
    else if ( rSize.Height() > 0 )
        mnBottom = o3tl::saturating_add(mnTop, (rSize.Height() - 1));
    else
        SetHeightEmpty();
}

void tools::Rectangle::SaturatingSetPosX(tools::Long x)
{
    if (!IsWidthEmpty())
        mnRight = o3tl::saturating_add(mnRight, x - mnLeft);
    mnLeft = x;
}

void tools::Rectangle::SaturatingSetPosY(tools::Long y)
{
    if (!IsHeightEmpty())
        mnBottom = o3tl::saturating_add(mnBottom, y - mnTop);
    mnTop = y;
}

tools::Rectangle& tools::Rectangle::Union( const tools::Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
        return *this;

    if ( IsEmpty() )
        *this = rRect;
    else
    {
        std::tie(mnLeft, mnRight) = std::minmax({ mnLeft, rRect.mnLeft, mnRight, rRect.mnRight });
        std::tie(mnTop, mnBottom) = std::minmax({ mnTop, rRect.mnTop, mnBottom, rRect.mnBottom });
    }

    return *this;
}

tools::Rectangle& tools::Rectangle::Intersection( const tools::Rectangle& rRect )
{
    if ( IsEmpty() )
        return *this;
    if ( rRect.IsEmpty() )
    {
        *this = tools::Rectangle();
        return *this;
    }

    // Justify rectangle
    tools::Rectangle aTmpRect( rRect );
    Justify();
    aTmpRect.Justify();

    // Perform intersection
    mnLeft  = std::max( mnLeft, aTmpRect.mnLeft );
    mnRight = std::min( mnRight, aTmpRect.mnRight );
    mnTop   = std::max( mnTop, aTmpRect.mnTop );
    mnBottom= std::min( mnBottom, aTmpRect.mnBottom );

    // Determine if intersection is empty
    if ( mnRight < mnLeft || mnBottom < mnTop )
        *this = tools::Rectangle();

    return *this;
}

void tools::Rectangle::Justify()
{
    if ((mnRight < mnLeft) && (!IsWidthEmpty()))
    {
        std::swap(mnLeft, mnRight);
    }

    if ((mnBottom < mnTop) && (!IsHeightEmpty()))
    {
        std::swap(mnBottom, mnTop);
    }
}

bool tools::Rectangle::Contains( const Point& rPoint ) const
{
    if ( IsEmpty() )
        return false;

    if ( mnLeft <= mnRight )
    {
        if ( (rPoint.X() < mnLeft) || (rPoint.X() > mnRight) )
            return false;
    }
    else
    {
        if ( (rPoint.X() > mnLeft) || (rPoint.X() < mnRight) )
            return false;
    }
    if ( mnTop <= mnBottom )
    {
        if ( (rPoint.Y() < mnTop) || (rPoint.Y() > mnBottom) )
            return false;
    }
    else
    {
        if ( (rPoint.Y() > mnTop) || (rPoint.Y() < mnBottom) )
            return false;
    }
    return true;
}

bool tools::Rectangle::Contains( const tools::Rectangle& rRect ) const
{
    return Contains( rRect.TopLeft() ) && Contains( rRect.BottomRight() );
}

bool tools::Rectangle::Overlaps( const tools::Rectangle& rRect ) const
{
    // If there's no intersection, they don't overlap
    return !GetIntersection( rRect ).IsEmpty();
}

OString tools::Rectangle::toString() const
{
    // Note that this is not just used for debugging output but the
    // format is parsed by external code (passed in callbacks to
    // LibreOfficeKit clients). So don't change.
    return OString::number(Left()) + ", "
            + OString::number(Top()) + ", "
            + OString::number(getOpenWidth()) + ", "
            + OString::number(getOpenHeight());
}

void tools::Rectangle::expand(tools::Long nExpandBy)
{
    AdjustLeft(-nExpandBy);
    AdjustTop(-nExpandBy);
    AdjustRight(nExpandBy);
    AdjustBottom(nExpandBy);
}

void tools::Rectangle::shrink(tools::Long nShrinkBy)
{
    mnLeft   += nShrinkBy;
    mnTop    += nShrinkBy;
    if (!IsWidthEmpty())
        mnRight -= nShrinkBy;
    if (!IsHeightEmpty())
        mnBottom -= nShrinkBy;
}

tools::Long tools::Rectangle::AdjustRight(tools::Long nHorzMoveDelta)
{
    if (IsWidthEmpty())
        mnRight = mnLeft + nHorzMoveDelta - 1;
    else
        mnRight += nHorzMoveDelta;
    return mnRight;
}

tools::Long tools::Rectangle::AdjustBottom( tools::Long nVertMoveDelta )
{
    if (IsHeightEmpty())
        mnBottom = mnTop + nVertMoveDelta - 1;
    else
        mnBottom += nVertMoveDelta;
    return mnBottom;
}

static_assert( std::is_trivially_copyable< Pair >::value );
static_assert( std::is_trivially_copyable< Point >::value );
static_assert( std::is_trivially_copyable< Size >::value );
static_assert( std::is_trivially_copyable< Range >::value );
static_assert( std::is_trivially_copyable< Selection >::value );
static_assert( std::is_trivially_copyable< tools::Rectangle >::value );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
