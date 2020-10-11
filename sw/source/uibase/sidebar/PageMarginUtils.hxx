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
#include <cmath>
#define SWPAGE_NO_MARGIN       0
#define SWPAGE_NARROW_VALUE    720   // twips: 0.5 (1/2) in or 1.27 cm
#define SWPAGE_MODERATE_LR     1080  // twips: 0.75 (3/4) in or 1.91 cm
#define SWPAGE_NORMAL_VALUE    1136  // twips: 0.79 in or 2.00 cm
#define SWPAGE_WIDE_VALUE1     1440  // twips: 1.00 in or 2.54 cm
#define SWPAGE_WIDE_VALUE2     2880  // twips: 2.00 in or 5.08 cm
#define SWPAGE_WIDE_VALUE3     1800  // twips: 1.25 (5/4) in or 3.18 cm
#define SWPAGE_UNIT_THRESHOLD  5

namespace sw::sidebar{

bool IsNone( const tools::Long nPageLeftMargin, const tools::Long nPageRightMargin,
               const tools::Long nPageTopMargin, const tools::Long nPageBottomMargin, bool bMirrored)
{
    return( std::abs(nPageLeftMargin - SWPAGE_NO_MARGIN) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SWPAGE_NO_MARGIN ) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SWPAGE_NO_MARGIN) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SWPAGE_NO_MARGIN) <= SWPAGE_UNIT_THRESHOLD &&
        !bMirrored );
}

void SetNone( tools::Long& nPageLeftMargin, tools::Long& nPageRightMargin,
                tools::Long& nPageTopMargin, tools::Long& nPageBottomMargin, bool& bMirrored)
{
    nPageLeftMargin = SWPAGE_NO_MARGIN;
    nPageRightMargin = SWPAGE_NO_MARGIN;
    nPageTopMargin = SWPAGE_NO_MARGIN;
    nPageBottomMargin = SWPAGE_NO_MARGIN;
    bMirrored = false;
}

bool IsNarrow( const tools::Long nPageLeftMargin, const tools::Long nPageRightMargin,
               const tools::Long nPageTopMargin, const tools::Long nPageBottomMargin, bool bMirrored)
{
    return( std::abs(nPageLeftMargin - SWPAGE_NARROW_VALUE) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SWPAGE_NARROW_VALUE) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SWPAGE_NARROW_VALUE) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SWPAGE_NARROW_VALUE) <= SWPAGE_UNIT_THRESHOLD &&
        !bMirrored );
}

void SetNarrow( tools::Long& nPageLeftMargin, tools::Long& nPageRightMargin,
                tools::Long& nPageTopMargin, tools::Long& nPageBottomMargin, bool& bMirrored)
{
    nPageLeftMargin = SWPAGE_NARROW_VALUE;
    nPageRightMargin = SWPAGE_NARROW_VALUE;
    nPageTopMargin = SWPAGE_NARROW_VALUE;
    nPageBottomMargin = SWPAGE_NARROW_VALUE;
    bMirrored = false;
}

bool IsModerate( const tools::Long nPageLeftMargin, const tools::Long nPageRightMargin,
               const tools::Long nPageTopMargin, const tools::Long nPageBottomMargin, bool bMirrored)
{
    return( std::abs(nPageLeftMargin - SWPAGE_MODERATE_LR) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SWPAGE_MODERATE_LR) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        !bMirrored );
}

void SetModerate( tools::Long& nPageLeftMargin, tools::Long& nPageRightMargin,
                tools::Long& nPageTopMargin, tools::Long& nPageBottomMargin, bool& bMirrored)
{
    nPageLeftMargin = SWPAGE_MODERATE_LR;
    nPageRightMargin = SWPAGE_MODERATE_LR;
    nPageTopMargin = SWPAGE_WIDE_VALUE1;
    nPageBottomMargin = SWPAGE_WIDE_VALUE1;
    bMirrored = false;
}

bool IsNormal079( const tools::Long nPageLeftMargin, const tools::Long nPageRightMargin,
               const tools::Long nPageTopMargin, const tools::Long nPageBottomMargin, bool bMirrored)
{
    return( std::abs(nPageLeftMargin - SWPAGE_NORMAL_VALUE) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SWPAGE_NORMAL_VALUE) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SWPAGE_NORMAL_VALUE) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SWPAGE_NORMAL_VALUE) <= SWPAGE_UNIT_THRESHOLD &&
        !bMirrored );
}

void SetNormal079( tools::Long& nPageLeftMargin, tools::Long& nPageRightMargin,
                tools::Long& nPageTopMargin, tools::Long& nPageBottomMargin, bool& bMirrored)
{
    nPageLeftMargin = SWPAGE_NORMAL_VALUE;
    nPageRightMargin = SWPAGE_NORMAL_VALUE;
    nPageTopMargin = SWPAGE_NORMAL_VALUE;
    nPageBottomMargin = SWPAGE_NORMAL_VALUE;
    bMirrored = false;
}

bool IsNormal100( const tools::Long nPageLeftMargin, const tools::Long nPageRightMargin,
               const tools::Long nPageTopMargin, const tools::Long nPageBottomMargin, bool bMirrored)
{
    return( std::abs(nPageLeftMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        !bMirrored );
}

void SetNormal100( tools::Long& nPageLeftMargin, tools::Long& nPageRightMargin,
                tools::Long& nPageTopMargin, tools::Long& nPageBottomMargin, bool& bMirrored)
{
    nPageLeftMargin = SWPAGE_WIDE_VALUE1;
    nPageRightMargin = SWPAGE_WIDE_VALUE1;
    nPageTopMargin = SWPAGE_WIDE_VALUE1;
    nPageBottomMargin = SWPAGE_WIDE_VALUE1;
    bMirrored = false;
}

bool IsNormal125( const tools::Long nPageLeftMargin, const tools::Long nPageRightMargin,
               const tools::Long nPageTopMargin, const tools::Long nPageBottomMargin, bool bMirrored)
{
    return( std::abs(nPageLeftMargin - SWPAGE_WIDE_VALUE3) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SWPAGE_WIDE_VALUE3) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        !bMirrored );
}

void SetNormal125( tools::Long& nPageLeftMargin, tools::Long& nPageRightMargin,
                tools::Long& nPageTopMargin, tools::Long& nPageBottomMargin, bool& bMirrored)
{
    nPageLeftMargin = SWPAGE_WIDE_VALUE3;
    nPageRightMargin = SWPAGE_WIDE_VALUE3;
    nPageTopMargin = SWPAGE_WIDE_VALUE1;
    nPageBottomMargin = SWPAGE_WIDE_VALUE1;
    bMirrored = false;
}

bool IsWide( const tools::Long nPageLeftMargin, const tools::Long nPageRightMargin,
               const tools::Long nPageTopMargin, const tools::Long nPageBottomMargin, bool bMirrored)
{
    return( std::abs(nPageLeftMargin - SWPAGE_WIDE_VALUE2) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SWPAGE_WIDE_VALUE2) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        !bMirrored );
}

void SetWide( tools::Long& nPageLeftMargin, tools::Long& nPageRightMargin,
                tools::Long& nPageTopMargin, tools::Long& nPageBottomMargin, bool& bMirrored)
{
    nPageLeftMargin = SWPAGE_WIDE_VALUE2;
    nPageRightMargin = SWPAGE_WIDE_VALUE2;
    nPageTopMargin = SWPAGE_WIDE_VALUE1;
    nPageBottomMargin = SWPAGE_WIDE_VALUE1;
    bMirrored = false;
}

bool IsMirrored( const tools::Long nPageLeftMargin, const tools::Long nPageRightMargin,
               const tools::Long nPageTopMargin, const tools::Long nPageBottomMargin, bool bMirrored)
{
    return( std::abs(nPageLeftMargin - SWPAGE_WIDE_VALUE3) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SWPAGE_WIDE_VALUE1) <= SWPAGE_UNIT_THRESHOLD &&
        bMirrored );
}

void SetMirrored( tools::Long& nPageLeftMargin, tools::Long& nPageRightMargin,
                tools::Long& nPageTopMargin, tools::Long& nPageBottomMargin, bool& bMirrored)
{
    nPageLeftMargin = SWPAGE_WIDE_VALUE3;
    nPageRightMargin = SWPAGE_WIDE_VALUE1;
    nPageTopMargin = SWPAGE_WIDE_VALUE1;
    nPageBottomMargin = SWPAGE_WIDE_VALUE1;
    bMirrored = true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
