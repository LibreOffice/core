/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <docmodel/theme/ColorSet.hxx>
#include <sstream>
#include <utility>
#include <libxml/xmlwriter.h>
#include <sal/log.hxx>

namespace model
{
ColorSet::ColorSet(OUString const& rName)
    : maName(rName)
{
}

void ColorSet::add(model::ThemeColorType eType, Color aColorData)
{
    if (eType == model::ThemeColorType::Unknown)
        return;
    maColors[sal_Int16(eType)] = aColorData;
}

Color ColorSet::getColor(model::ThemeColorType eType) const
{
    if (eType == model::ThemeColorType::Unknown)
    {
        SAL_WARN("svx", "ColorSet::getColor with ThemeColorType::Unknown");
        return COL_AUTO;
    }
    return maColors[size_t(eType)];
}

Color ColorSet::resolveColor(model::ComplexColor const& rComplexColor) const
{
    auto eThemeType = rComplexColor.getThemeColorType();
    if (eThemeType == model::ThemeColorType::Unknown)
    {
        SAL_WARN("svx", "ColorSet::resolveColor with ThemeColorType::Unknown");
        return COL_AUTO;
    }
    Color aColor = getColor(eThemeType);
    return rComplexColor.applyTransformations(aColor);
}

void ColorSet::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ColorSet"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("maName"),
                                      BAD_CAST(maName.toUtf8().getStr()));

    for (const auto& rColor : maColors)
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("Color"));
        std::stringstream ss;
        ss << rColor;
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(ss.str().c_str()));
        (void)xmlTextWriterEndElement(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
