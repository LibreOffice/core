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

#include <tools/GenericTypeSerializer.hxx>
#include <sal/config.h>
#include <sal/log.hxx>
#include <vector>

namespace tools
{
constexpr sal_uInt16 COL_NAME_USER = 0x8000;

constexpr sal_Int32 RECT_EMPTY_VALUE_RIGHT_BOTTOM = -32767;

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

void GenericTypeSerializer::readPoint(Point& rPoint)
{
    sal_Int32 nX(0);
    sal_Int32 nY(0);

    mrStream.ReadInt32(nX);
    mrStream.ReadInt32(nY);

    rPoint.setX(nX);
    rPoint.setY(nY);
}

void GenericTypeSerializer::writePoint(const Point& rPoint)
{
    mrStream.WriteInt32(rPoint.getX());
    mrStream.WriteInt32(rPoint.getY());
}

void GenericTypeSerializer::readSize(Size& rSize)
{
    sal_Int32 nWidth(0);
    sal_Int32 nHeight(0);

    mrStream.ReadInt32(nWidth);
    mrStream.ReadInt32(nHeight);

    rSize.setWidth(nWidth);
    rSize.setHeight(nHeight);
}

void GenericTypeSerializer::writeSize(const Size& rSize)
{
    mrStream.WriteInt32(rSize.getWidth());
    mrStream.WriteInt32(rSize.getHeight());
}

void GenericTypeSerializer::readRectangle(Rectangle& rRectangle)
{
    sal_Int32 nLeft(0);
    sal_Int32 nTop(0);
    sal_Int32 nRight(0);
    sal_Int32 nBottom(0);

    mrStream.ReadInt32(nLeft);
    mrStream.ReadInt32(nTop);
    mrStream.ReadInt32(nRight);
    mrStream.ReadInt32(nBottom);

    if (nRight == RECT_EMPTY_VALUE_RIGHT_BOTTOM || nBottom == RECT_EMPTY_VALUE_RIGHT_BOTTOM)
    {
        rRectangle.SetEmpty();
    }
    else
    {
        rRectangle.SetLeft(nLeft);
        rRectangle.SetTop(nTop);
        rRectangle.SetRight(nRight);
        rRectangle.SetBottom(nBottom);
    }
}

void GenericTypeSerializer::writeRectangle(const Rectangle& rRectangle)
{
    if (rRectangle.IsEmpty())
    {
        mrStream.WriteInt32(0);
        mrStream.WriteInt32(0);
        mrStream.WriteInt32(RECT_EMPTY_VALUE_RIGHT_BOTTOM);
        mrStream.WriteInt32(RECT_EMPTY_VALUE_RIGHT_BOTTOM);
    }
    else
    {
        mrStream.WriteInt32(rRectangle.Left());
        mrStream.WriteInt32(rRectangle.Top());
        mrStream.WriteInt32(rRectangle.Right());
        mrStream.WriteInt32(rRectangle.Bottom());
    }
}

void GenericTypeSerializer::readFraction(Fraction& rFraction)
{
    sal_Int32 nNumerator(0);
    sal_Int32 nDenominator(0);

    mrStream.ReadInt32(nNumerator);
    mrStream.ReadInt32(nDenominator);

    rFraction = Fraction(nNumerator, nDenominator);
}

void GenericTypeSerializer::writeFraction(Fraction const& rFraction)
{
    if (!rFraction.IsValid())
    {
        SAL_WARN("tools.fraction", "'writeFraction()' write an invalid fraction");
        mrStream.WriteInt32(0);
        mrStream.WriteInt32(0);
    }
    else
    {
        mrStream.WriteInt32(rFraction.GetNumerator());
        mrStream.WriteInt32(rFraction.GetDenominator());
    }
}

} // end namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
