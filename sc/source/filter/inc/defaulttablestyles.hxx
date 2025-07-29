/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tablestyle.hxx>
#include "stylesbuffer.hxx"

// structs used by the script generated code from the original OOXML spec

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
    int nBgColorId;
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

struct TableStyle
{
    const char* pName;
    size_t nElements;
    int pTableStyleElementIds[9];
};

// the actual import class

class DefaultOOXMLTableStyles : public oox::xls::WorkbookHelper
{
private:
    std::vector<oox::xls::XlsColor> maColors;
    oox::xls::FillVector maFills;
    oox::xls::BorderVector maBorders;
    oox::xls::FontVector maFonts;

    oox::xls::DxfVector maDxfs;

    void importColors();
    void importFills();
    void importBorderElement(oox::xls::BorderRef xBorder, sal_Int32 nBorderElement,
                             sal_Int32 nBorderElementId);
    void importBorders();
    void importFonts();
    void importDxfs();
    void importTableStyle(const TableStyle& rTableStyleInfo);

public:
    DefaultOOXMLTableStyles(const oox::xls::WorkbookHelper& rHelper);
    void importTableStyles();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
