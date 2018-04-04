/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/types.h>
#include <map>

struct SAL_WARN_UNUSED Point
{
};
struct SAL_WARN_UNUSED Rectangle
{
    Rectangle();
    Rectangle(int width, int height);
    Rectangle& Union(const Rectangle&) { return *this; }
    Rectangle& Intersection(const Rectangle&);
    Rectangle& operator+=(const Point& rPt);
};

void func1()
{
    Rectangle aTmp1; // expected-error {{unused variable 'aTmp1' [loplugin:unusedvariablemore]}}
    aTmp1.Union(Rectangle(10, 10));
}

void func2()
{
    Rectangle aViewArea(
        10, 10); // expected-error@-1 {{unused variable 'aViewArea' [loplugin:unusedvariablemore]}}
    aViewArea += Point();
    aViewArea.Intersection(Rectangle(0, 0));
}

//---------------------------------------------------------------------
// Negative tests
//---------------------------------------------------------------------

Rectangle func3(const Rectangle& rRect)
{
    Rectangle aTmpRect(Rectangle(10, 10));
    return aTmpRect.Union(rRect);
}

void func4()
{
    std::map<int, int> aMimeTypeMap;
    aMimeTypeMap[1] = 0;
    int aExportMimeType(aMimeTypeMap[0]);
    (void)aExportMimeType;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
