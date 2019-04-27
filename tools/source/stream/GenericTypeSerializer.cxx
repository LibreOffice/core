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

#include <tools/GenericTypeSerializer.hxx>
#include <vector>

namespace tools
{
constexpr sal_uInt16 COL_NAME_USER = 0x8000;

void GenericTypeSerializer::readColor(Color& rColor)
{
    sal_uInt16 nColorNameID(0);

    mrStream.ReadUInt16(nColorNameID);

    if (nColorNameID & COL_NAME_USER)
    {
        sal_uInt16 nRed;
        sal_uInt16 nGreen;
        sal_uInt16 nBlue;

        mrStream.ReadUInt16(nRed);
        mrStream.ReadUInt16(nGreen);
        mrStream.ReadUInt16(nBlue);

        rColor = Color(nRed >> 8, nGreen >> 8, nBlue >> 8);
    }
    else
    {
        static const std::vector<Color> staticColorArray = {
            COL_BLACK, // COL_BLACK
            COL_BLUE, // COL_BLUE
            COL_GREEN, // COL_GREEN
            COL_CYAN, // COL_CYAN
            COL_RED, // COL_RED
            COL_MAGENTA, // COL_MAGENTA
            COL_BROWN, // COL_BROWN
            COL_GRAY, // COL_GRAY
            COL_LIGHTGRAY, // COL_LIGHTGRAY
            COL_LIGHTBLUE, // COL_LIGHTBLUE
            COL_LIGHTGREEN, // COL_LIGHTGREEN
            COL_LIGHTCYAN, // COL_LIGHTCYAN
            COL_LIGHTRED, // COL_LIGHTRED
            COL_LIGHTMAGENTA, // COL_LIGHTMAGENTA
            COL_YELLOW, // COL_YELLOW
            COL_WHITE, // COL_WHITE
            COL_WHITE, // COL_MENUBAR
            COL_BLACK, // COL_MENUBARTEXT
            COL_WHITE, // COL_POPUPMENU
            COL_BLACK, // COL_POPUPMENUTEXT
            COL_BLACK, // COL_WINDOWTEXT
            COL_WHITE, // COL_WINDOWWORKSPACE
            COL_BLACK, // COL_HIGHLIGHT
            COL_WHITE, // COL_HIGHLIGHTTEXT
            COL_BLACK, // COL_3DTEXT
            COL_LIGHTGRAY, // COL_3DFACE
            COL_WHITE, // COL_3DLIGHT
            COL_GRAY, // COL_3DSHADOW
            COL_LIGHTGRAY, // COL_SCROLLBAR
            COL_WHITE, // COL_FIELD
            COL_BLACK // COL_FIELDTEXT
        };

        if (nColorNameID < staticColorArray.size())
            rColor = staticColorArray[nColorNameID];
        else
            rColor = COL_BLACK;
    }
}

void GenericTypeSerializer::writeColor(const Color& rColor)
{
    mrStream.WriteUInt16(COL_NAME_USER);

    sal_uInt16 nR = rColor.GetRed();
    sal_uInt16 nG = rColor.GetGreen();
    sal_uInt16 nB = rColor.GetBlue();

    mrStream.WriteUInt16((nR << 8) + nR);
    mrStream.WriteUInt16((nG << 8) + nG);
    mrStream.WriteUInt16((nB << 8) + nB);
}

} // end namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
