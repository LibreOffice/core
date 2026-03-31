/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "tablestyle.hxx"

// POD structs used by the script-generated constexpr data in defaulttablestyles.inc
// (generated from the ECMA-376 presetTableStyles.xml by generate_ooxml_tablestyle_info.py)

struct ThemeColor
{
    int nThemeId;
    float fTint;
};

struct Font
{
    bool bBold;
    int nThemeColorId;
};

struct Fill
{
    int nFgColorId;
};

enum class BorderElementStyle
{
    THIN,
    MEDIUM,
    THICK,
    DOUBLE
};

struct BorderElement
{
    BorderElementStyle eBorderStyle;
    int nColorId;
};

struct Border
{
    int nTopId;
    int nBottomId;
    int nLeftId;
    int nRightId;
    int nHorizontalId;
    int nVerticalId;
};

struct Dxf
{
    int nFillId;
    int nBorderId;
    int nFontId;
};

struct TableStyleElement
{
    ScTableStyleElement eElement;
    int nDxfId;
};

enum class TableStyleCategory
{
    Light,
    Medium,
    Dark
};

struct TableStyle
{
    const char* pName;
    TableStyleCategory eCategory;
    int nNumber;
    size_t nElements;
    int pTableStyleElementIds[9];
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
