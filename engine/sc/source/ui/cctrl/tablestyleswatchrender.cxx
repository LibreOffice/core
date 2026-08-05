/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tablestyleswatchrender.hxx>

#include <editeng/brushitem.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <vcl/BinaryDataContainer.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/graph.hxx>
#include <vcl/vectorgraphicdata.hxx>

#include <patattr.hxx>
#include <scitems.hxx>
#include <tablestyle.hxx>
#include <tablestyleswatch.hxx>

#include <map>
#include <optional>

namespace
{
std::optional<Color>
elementFill(const std::map<ScTableStyleElement, const ScPatternAttr*>& rPatterns,
            ScTableStyleElement eElement)
{
    auto it = rPatterns.find(eElement);
    if (it == rPatterns.end())
        return std::nullopt;
    const SvxBrushItem* pBrush = it->second->GetItemSet().GetItemIfSet(ATTR_BACKGROUND);
    if (!pBrush)
        return std::nullopt;
    return pBrush->GetColor();
}

Bitmap rasterizeSvg(const OUString& rSvg, const Size& rPixelSize)
{
    const OString aUtf8 = rSvg.toUtf8();
    SvMemoryStream aStream;
    aStream.WriteBytes(aUtf8.getStr(), aUtf8.getLength());
    aStream.Seek(0);
    BinaryDataContainer aContainer(aStream, aUtf8.getLength());
    auto pData = std::make_shared<VectorGraphicData>(aContainer, VectorGraphicDataType::Svg);
    Graphic aGraphic(pData);
    return aGraphic.GetBitmap(GraphicConversionParameters(rPixelSize, false, true, false));
}
}

Bitmap ScRenderTableStyleSwatchBitmap(const ScTableStyle& rStyle, const Size& rPixelSize)
{
    const std::map<ScTableStyleElement, const ScPatternAttr*> aPatterns = rStyle.GetSetPatterns();

    const Color aBody = elementFill(aPatterns, ScTableStyleElement::WholeTable).value_or(COL_WHITE);
    const Color aHeader = elementFill(aPatterns, ScTableStyleElement::HeaderRow).value_or(aBody);
    const Color aFirst
        = elementFill(aPatterns, ScTableStyleElement::FirstRowStripe).value_or(aBody);
    const Color aSecond
        = elementFill(aPatterns, ScTableStyleElement::SecondRowStripe).value_or(aBody);

    return rasterizeSvg(
        ScApplyTableStyleSwatch(ScComputeTableStyleSwatch(rStyle.GetName(), rStyle.IsOOXMLDefault(),
                                                          aHeader, aBody, aFirst, aSecond)),
        rPixelSize);
}

Bitmap ScRenderNoneSwatchBitmap(const Size& rPixelSize)
{
    // The "no style" entry uses the custom template with plain white colors.
    return rasterizeSvg(ScApplyTableStyleSwatch(ScComputeTableStyleSwatch(
                            u"None"_ustr, false, COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE)),
                        rPixelSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
