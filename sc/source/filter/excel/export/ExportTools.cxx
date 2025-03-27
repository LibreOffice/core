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
#include <xestream.hxx>

namespace oox::xls
{
void writeComplexColor(const sax_fastparser::FSHelperPtr& pFS, sal_Int32 nElement,
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

void writeComplexColor(const sax_fastparser::FSHelperPtr& pFS, sal_Int32 nElement,
                       model::ComplexColor const& rComplexColor)
{
    if (rComplexColor.isValidThemeType() || rComplexColor.getType() == model::ColorType::RGB)
    {
        writeComplexColor(pFS, nElement, rComplexColor, rComplexColor.getFinalColor());
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
