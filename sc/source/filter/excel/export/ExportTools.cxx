/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <export/ExportTools.hxx>
#include <oox/export/ColorExportUtils.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>
#include <sax/fastattribs.hxx>
#include <xestream.hxx>

namespace oox::xls
{
void writeComplexColor(sax_fastparser::FSHelperPtr& pFS, sal_Int32 nElement,
                       model::ComplexColor const& rComplexColor, Color const& rColor)
{
    if (rComplexColor.isValidThemeType())
    {
        sal_Int32 nTheme
            = oox::convertThemeColorTypeToExcelThemeNumber(rComplexColor.getThemeColorType());
        double fTintShade = oox::convertColorTransformsToTintOrShade(rComplexColor);
        pFS->singleElement(nElement, XML_theme, OString::number(nTheme), XML_tint,
                           sax_fastparser::UseIf(OString::number(fTintShade), fTintShade != 0.0));
    }
    else if (rColor != COL_TRANSPARENT)
    {
        pFS->singleElement(nElement, XML_rgb, XclXmlUtils::ToOString(rColor));
    }
}

void writeComplexColor(sax_fastparser::FSHelperPtr& pFS, sal_Int32 nElement,
                       model::ComplexColor const& rComplexColor)
{
    if (rComplexColor.isValidThemeType() || rComplexColor.getType() == model::ColorType::RGB)
    {
        writeComplexColor(pFS, nElement, rComplexColor, rComplexColor.getFinalColor());
    }
}

void writeDateGroupItem(const sax_fastparser::FSHelperPtr& pStream, sal_Int32 nElement,
                        std::string_view aDateString)
{
    rtl::Reference<sax_fastparser::FastAttributeList> pAttrList
        = sax_fastparser::FastSerializerHelper::createAttrList();

    // Tokenize on '-' to extract year, month, day components.
    // The date string may be partial: "YYYY", "YYYY-MM", or "YYYY-MM-DD".
    static constexpr sal_Int32 aDateTokens[] = { XML_year, XML_month, XML_day };
    const char* pGrouping = "year";

    size_t nStart = 0;
    for (size_t i = 0; i < 3 && nStart < aDateString.size(); ++i)
    {
        size_t nSep = aDateString.find('-', nStart);
        std::string_view aPart;
        if (nSep != std::string_view::npos)
            aPart = aDateString.substr(nStart, nSep - nStart);
        else
            aPart = aDateString.substr(nStart);

        if (!aPart.empty())
            pAttrList->add(aDateTokens[i], aPart);

        if (i == 1)
            pGrouping = "month";
        else if (i == 2)
            pGrouping = "day";

        if (nSep == std::string_view::npos)
            break;
        nStart = nSep + 1;
    }

    pAttrList->add(XML_dateTimeGrouping, pGrouping);
    pStream->singleElement(nElement, pAttrList);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
