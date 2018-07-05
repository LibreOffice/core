/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string.h>
#include <tools/gen.hxx>
#include <Qt5System.hxx>

Qt5System::Qt5System() {}
Qt5System::~Qt5System() {}

unsigned int Qt5System::GetDisplayScreenCount() { return 1; }

tools::Rectangle Qt5System::GetDisplayScreenPosSizePixel(unsigned int nScreen)
{
    tools::Rectangle aRect;
    if (nScreen == 0)
        aRect = tools::Rectangle(Point(0, 0), Size(1024, 768));
    return aRect;
}

int Qt5System::ShowNativeDialog(const OUString&, const OUString&, const std::vector<OUString>&)
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
