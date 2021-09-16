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
#ifndef INCLUDED_SW_INC_SWREGION_HXX
#define INCLUDED_SW_INC_SWREGION_HXX

#include <vector>

#include "swrect.hxx"

typedef std::vector<SwRect> SwRects;

// A collection of rectangles within a given area (origin).
// One way to use is to initially set rStartRect, which will set the origin
// and the rectangles to rStartRect, and then use operator-= to punch holes.
// Other way to use is to use empty constructor, call ChangeOrigin() and
// then use operator+= to add rectangles, possibly followed by final
// LimitToOrigin() and Compress(). The second way should be faster than
// the first way followed by Invert().
class SwRegionRects : public SwRects
{
    SwRect m_aOrigin; // Origin area, limits the total area e.g. for Invert()

    inline void InsertRect( const SwRect &rRect, const sal_uInt16 nPos, bool &rDel);

public:
    // Sets rStartRect as the first element and also the origin area.
    explicit SwRegionRects( const SwRect& rStartRect, sal_uInt16 nInit = 20 );
    // Empty constructor, does not set elements or origin area. You may
    // most likely want to call ChangeOrigin() afterwards.
    explicit SwRegionRects( sal_uInt16 nInit = 20 );

    // For punching from aOrigin.
    void operator-=( const SwRect& rRect );
    void operator+=( const SwRect& rRect );

    // From holes to areas, from areas to holes, within the origin area.
    void Invert();

    // Ensures all rectangles are within the origin area.
    void LimitToOrigin();

    // Combine adjacent rectangles.
    void Compress();

    const SwRect &GetOrigin() const { return m_aOrigin; }
    void ChangeOrigin( const SwRect &rRect ) { m_aOrigin = rRect; }
};

#endif // INCLUDED_SW_INC_SWREGION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
