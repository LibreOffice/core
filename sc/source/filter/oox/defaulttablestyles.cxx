/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

#include "defaulttablestyles.inc"
#include <workbookhelper.hxx>

DefaultOOXMLTableStyles::DefaultOOXMLTableStyles(const oox::xls::WorkbookHelper& rHelper)
    : WorkbookHelper(rHelper)
{
    importColors();
    importFills();
    importBorders();
    importFonts();
    importDxfs();
}

void DefaultOOXMLTableStyles::importColors()
{
    size_t nColors = sizeof(aThemeColors) / sizeof(ThemeColor);
    for (size_t i = 0; i < nColors; ++i)
    {
        oox::xls::XlsColor aColor;
        aColor.setTheme(aThemeColors[i].nThemeId, aThemeColors[i].fTint);
        maColors.push_back(aColor);
    }
}

void DefaultOOXMLTableStyles::importFills()
{
    size_t nFills = sizeof(aFills) / sizeof(Fill);
    for (size_t i = 0; i < nFills; ++i)
    {
        auto xFill = std::make_shared<oox::xls::Fill>(*this, true);
        xFill->setFillColors(maColors[aFills[i].nFgColorId], maColors[aFills[i].nBgColorId]);
        maFills.push_back(xFill);
    }
}

void DefaultOOXMLTableStyles::importBorderElement(oox::xls::BorderRef xBorder,
                                                  sal_Int32 nBorderElement,
                                                  sal_Int32 nBorderElementId)
{
    const BorderElement& rBorderElement = aBorderElements[nBorderElementId];
    if (rBorderElement.nColorId >= static_cast<sal_Int32>(maColors.size()))
        return;

    sal_Int32 nBorderStyle;
    switch (rBorderElement.eBorderStyle)
    {
        case BorderElementStyle::THIN:
            nBorderStyle = oox::XML_thin;
            break;
        case BorderElementStyle::MEDIUM:
            nBorderStyle = oox::XML_medium;
            break;
        case BorderElementStyle::THICK:
            nBorderStyle = oox::XML_thick;
            break;
        case BorderElementStyle::DOUBLE:
            nBorderStyle = oox::XML_double;
            break;
    }

    oox::xls::XlsColor& rColor = maColors[rBorderElement.nColorId];
    xBorder->setBorderElement(nBorderElement, rColor, nBorderStyle);
}

void DefaultOOXMLTableStyles::importBorders()
{
    size_t nBorders = sizeof(aBorders) / sizeof(Border);
    for (size_t i = 0; i < nBorders; ++i)
    {
        auto xBorder = std::make_shared<oox::xls::Border>(*this, true);

        const Border& rBorderInfo = aBorders[i];
        if (rBorderInfo.nTopId >= 0)
            importBorderElement(xBorder, XLS_TOKEN(top), rBorderInfo.nTopId);
        if (rBorderInfo.nBottomId >= 0)
            importBorderElement(xBorder, XLS_TOKEN(bottom), rBorderInfo.nBottomId);
        if (rBorderInfo.nLeftId >= 0)
            importBorderElement(xBorder, XLS_TOKEN(left), rBorderInfo.nLeftId);
        if (rBorderInfo.nRightId >= 0)
            importBorderElement(xBorder, XLS_TOKEN(right), rBorderInfo.nRightId);

        maBorders.push_back(xBorder);
    }
}

void DefaultOOXMLTableStyles::importFonts() {}

void DefaultOOXMLTableStyles::importDxfs()
{
    size_t nDxfs = sizeof(aDxfs) / sizeof(Dxf);
    for (size_t i = 0; i < nDxfs; ++i)
    {
        auto xDxf = std::make_shared<oox::xls::Dxf>(*this);
        auto aDxfInfo = aDxfs[i];

        if (aDxfInfo.nFillId >= 0)
            xDxf->setFill(maFills[aDxfInfo.nFillId]);

        if (aDxfInfo.nBorderId >= 0)
            xDxf->setBorder(maBorders[aDxfInfo.nBorderId]);

        xDxf->finalizeImport();
        maDxfs.push_back(xDxf);
    }
}

void DefaultOOXMLTableStyles::importTableStyle(const TableStyle& rTableStyleInfo)
{
    oox::xls::TableStyle aTableStyle(*this, true);
    aTableStyle.setName(OUString::createFromAscii(rTableStyleInfo.pName));
    for (size_t i = 0; i < rTableStyleInfo.nElements; ++i)
    {
        const TableStyleElement& rTableStyleElement
            = aTableStyleElements[rTableStyleInfo.pTableStyleElementIds[i]];
        aTableStyle.setTableStyleElement(rTableStyleElement.eElement, rTableStyleElement.nDxfId);
    }

    aTableStyle.finalizeImport(maDxfs);
}

void DefaultOOXMLTableStyles::importTableStyles()
{
    size_t nTableStyles = sizeof(aTableStyles) / sizeof(TableStyle);
    for (size_t i = 0; i < nTableStyles; ++i)
    {
        importTableStyle(aTableStyles[i]);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
