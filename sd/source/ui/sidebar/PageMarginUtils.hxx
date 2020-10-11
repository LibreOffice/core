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
#define SDPAGE_NO_MARGIN       0
#define SDPAGE_NARROW_VALUE    635   // 1/1000 cm: 0.25 (1/4) in or 0.64 cm
#define SDPAGE_MODERATE_LR     955   // 1/1000 cm: 0.38 (3/8) in or 0.96 cm
#define SDPAGE_NORMAL_VALUE    1000  // 1/1000 cm: 0.39 in or 1.00 cm
#define SDPAGE_WIDE_VALUE1     1270  // 1/1000 cm: 0.5 (1/2) in or 1.27 cm
#define SDPAGE_WIDE_VALUE2     2540  // 1/1000 cm: 1 in or 2.54 cm
#define SDPAGE_WIDE_VALUE3     1590  // 1/1000 cm: 0.63 (5/8) in or 1.59 cm
#define SDPAGE_UNIT_THRESHOLD  5

namespace sd::sidebar{

bool IsNone( const long nPageLeftMargin, const long nPageRightMargin,
               const long nPageTopMargin, const long nPageBottomMargin )
{
    return( std::abs(nPageLeftMargin - SDPAGE_NO_MARGIN) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SDPAGE_NO_MARGIN ) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SDPAGE_NO_MARGIN) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SDPAGE_NO_MARGIN) <= SDPAGE_UNIT_THRESHOLD );
}

void SetNone( long& nPageLeftMargin, long& nPageRightMargin,
                long& nPageTopMargin, long& nPageBottomMargin )
{
    nPageLeftMargin = SDPAGE_NO_MARGIN;
    nPageRightMargin = SDPAGE_NO_MARGIN;
    nPageTopMargin = SDPAGE_NO_MARGIN;
    nPageBottomMargin = SDPAGE_NO_MARGIN;
}

bool IsNarrow( const long nPageLeftMargin, const long nPageRightMargin,
               const long nPageTopMargin, const long nPageBottomMargin )
{
    return( std::abs(nPageLeftMargin - SDPAGE_NARROW_VALUE) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SDPAGE_NARROW_VALUE) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SDPAGE_NARROW_VALUE) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SDPAGE_NARROW_VALUE) <= SDPAGE_UNIT_THRESHOLD );
}

void SetNarrow( long& nPageLeftMargin, long& nPageRightMargin,
                long& nPageTopMargin, long& nPageBottomMargin )
{
    nPageLeftMargin = SDPAGE_NARROW_VALUE;
    nPageRightMargin = SDPAGE_NARROW_VALUE;
    nPageTopMargin = SDPAGE_NARROW_VALUE;
    nPageBottomMargin = SDPAGE_NARROW_VALUE;
}

bool IsModerate( const long nPageLeftMargin, const long nPageRightMargin,
               const long nPageTopMargin, const long nPageBottomMargin )
{
    return( std::abs(nPageLeftMargin - SDPAGE_MODERATE_LR) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SDPAGE_MODERATE_LR) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SDPAGE_WIDE_VALUE1) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SDPAGE_WIDE_VALUE1) <= SDPAGE_UNIT_THRESHOLD );
}

void SetModerate( long& nPageLeftMargin, long& nPageRightMargin,
                long& nPageTopMargin, long& nPageBottomMargin )
{
    nPageLeftMargin = SDPAGE_MODERATE_LR;
    nPageRightMargin = SDPAGE_MODERATE_LR;
    nPageTopMargin = SDPAGE_WIDE_VALUE1;
    nPageBottomMargin = SDPAGE_WIDE_VALUE1;
}

bool IsNormal039( const long nPageLeftMargin, const long nPageRightMargin,
               const long nPageTopMargin, const long nPageBottomMargin )
{
    return( std::abs(nPageLeftMargin - SDPAGE_NORMAL_VALUE) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SDPAGE_NORMAL_VALUE) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SDPAGE_NORMAL_VALUE) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SDPAGE_NORMAL_VALUE) <= SDPAGE_UNIT_THRESHOLD );
}

void SetNormal039( long& nPageLeftMargin, long& nPageRightMargin,
                long& nPageTopMargin, long& nPageBottomMargin )
{
    nPageLeftMargin = SDPAGE_NORMAL_VALUE;
    nPageRightMargin = SDPAGE_NORMAL_VALUE;
    nPageTopMargin = SDPAGE_NORMAL_VALUE;
    nPageBottomMargin = SDPAGE_NORMAL_VALUE;
}

bool IsNormal050( const long nPageLeftMargin, const long nPageRightMargin,
               const long nPageTopMargin, const long nPageBottomMargin )
{
    return( std::abs(nPageLeftMargin - SDPAGE_WIDE_VALUE1) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SDPAGE_WIDE_VALUE1) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SDPAGE_WIDE_VALUE1) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SDPAGE_WIDE_VALUE1) <= SDPAGE_UNIT_THRESHOLD );
}

void SetNormal050( long& nPageLeftMargin, long& nPageRightMargin,
                long& nPageTopMargin, long& nPageBottomMargin )
{
    nPageLeftMargin = SDPAGE_WIDE_VALUE1;
    nPageRightMargin = SDPAGE_WIDE_VALUE1;
    nPageTopMargin = SDPAGE_WIDE_VALUE1;
    nPageBottomMargin = SDPAGE_WIDE_VALUE1;
}

bool IsNormal063( const long nPageLeftMargin, const long nPageRightMargin,
               const long nPageTopMargin, const long nPageBottomMargin )
{
    return( std::abs(nPageLeftMargin - SDPAGE_WIDE_VALUE3) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SDPAGE_WIDE_VALUE3) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SDPAGE_WIDE_VALUE1) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SDPAGE_WIDE_VALUE1) <= SDPAGE_UNIT_THRESHOLD );
}

void SetNormal063( long& nPageLeftMargin, long& nPageRightMargin,
                long& nPageTopMargin, long& nPageBottomMargin )
{
    nPageLeftMargin = SDPAGE_WIDE_VALUE3;
    nPageRightMargin = SDPAGE_WIDE_VALUE3;
    nPageTopMargin = SDPAGE_WIDE_VALUE1;
    nPageBottomMargin = SDPAGE_WIDE_VALUE1;
}

bool IsWide( const long nPageLeftMargin, const long nPageRightMargin,
               const long nPageTopMargin, const long nPageBottomMargin )
{
    return( std::abs(nPageLeftMargin - SDPAGE_WIDE_VALUE2) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageRightMargin - SDPAGE_WIDE_VALUE2) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageTopMargin - SDPAGE_WIDE_VALUE1) <= SDPAGE_UNIT_THRESHOLD &&
        std::abs(nPageBottomMargin - SDPAGE_WIDE_VALUE1) <= SDPAGE_UNIT_THRESHOLD );
}

void SetWide( long& nPageLeftMargin, long& nPageRightMargin,
                long& nPageTopMargin, long& nPageBottomMargin )
{
    nPageLeftMargin = SDPAGE_WIDE_VALUE2;
    nPageRightMargin = SDPAGE_WIDE_VALUE2;
    nPageTopMargin = SDPAGE_WIDE_VALUE1;
    nPageBottomMargin = SDPAGE_WIDE_VALUE1;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
