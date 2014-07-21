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

#ifndef INCLUDED_TOOLS_MAPUNIT_HXX
#define INCLUDED_TOOLS_MAPUNIT_HXX

#include "gen.hxx"

enum MapUnit { MAP_100TH_MM, MAP_10TH_MM, MAP_MM, MAP_CM,
               MAP_1000TH_INCH, MAP_100TH_INCH, MAP_10TH_INCH, MAP_INCH,
               MAP_POINT, MAP_TWIP, MAP_PIXEL, MAP_SYSFONT, MAP_APPFONT,
               MAP_RELATIVE, MAP_LASTENUMDUMMY };

inline sal_Int64 convertTwipToMm100(sal_Int64 n)
{
    if (n >= 0)
        return (n*127+36)/72;
    else
        return (n*127-36)/72;
}

inline Point convertTwipToMm100(const Point& rPoint)
{
    return Point(convertTwipToMm100(rPoint.getX()),
                 convertTwipToMm100(rPoint.getY()));
}

inline Size convertTwipToMm100(const Size& rSize)
{
    return Size(convertTwipToMm100(rSize.getWidth()),
                convertTwipToMm100(rSize.getHeight()));
}

inline sal_Int64 convertMm100ToTwip(sal_Int64 n)
{
    if (n >= 0)
        return (n*72+63)/127;
    else
        return (n*72-63)/127;
}

inline Point convertMm100ToTwip(const Point& rPoint)
{
    return Point(convertMm100ToTwip(rPoint.getX()),
                 convertMm100ToTwip(rPoint.getY()));
}

inline Size convertMm100ToTwip(const Size& rSize)
{
    return Size(convertMm100ToTwip(rSize.getWidth()),
                convertMm100ToTwip(rSize.getHeight()));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
