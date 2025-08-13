/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <oox/drawingml/color.hxx>
#include <docmodel/color/ComplexColor.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/helper/propertymap.hxx>
#include <memory>
#include <variant>

namespace model
{
struct StyleColor
{
    enum class StyleColorEnum
    {
        AUTO
    };

    // The schema uses 'union' here. Implement with a std::variant for safety.
    using StyleColorVal = std::variant<sal_uInt32, enum StyleColorEnum, OUString>;
    typedef std::vector<StyleColorVal> StyleColorVec;

    StyleColorVec maStyleClr;
    sal_Int32 mnIdx;
    oox::drawingml::Color maColor;
    model::ComplexColor maComplexColor;
    // There's also an a:EG_ColorTransform member and a 'mods' member for
    // StyleColor. Ignore those for now. TODO
};

struct StyleEntry
{
    std::shared_ptr<StyleColor> mxLnClr;
    double mfLineWidthScale = 1.0;
    std::shared_ptr<StyleColor> mxFillClr;
    std::shared_ptr<StyleColor> mxEffectClr;
    std::shared_ptr<StyleColor> mxFontClr;
    std::shared_ptr<oox::drawingml::Shape> mxShapePr;
    // The following is derived from a TextCharacterProperties
    std::shared_ptr<oox::PropertyMap> mrTextCharacterPr;
    // The following is derived from a TextBodyProperties
    std::shared_ptr<oox::PropertyMap> mxTextBodyPr;

    StyleEntry(std::shared_ptr<StyleColor> aLnClr, double fLineScale,
               std::shared_ptr<StyleColor> aFillClr, std::shared_ptr<StyleColor> aEffectClr,
               std::shared_ptr<StyleColor> aFontClr, std::shared_ptr<oox::drawingml::Shape> aShape,
               std::shared_ptr<oox::PropertyMap> aCharProps,
               std::shared_ptr<oox::PropertyMap> aBodyProps)
        : mxLnClr(aLnClr)
        , mfLineWidthScale(fLineScale)
        , mxFillClr(aFillClr)
        , mxEffectClr(aEffectClr)
        , mxFontClr(aFontClr)
        , mxShapePr(aShape)
        , mrTextCharacterPr(aCharProps)
        , mxTextBodyPr(aBodyProps)
    {
    }
};

struct DOCMODEL_DLLPUBLIC StyleSet
{
    enum class StyleEntryType
    {
        AXISTITLE,
        CATEGORYAXIS,
        CHARTAREA,
        DATALABEL,
        DATALABELCALLOUT,
        DATAPOINT,
        DATAPOINT3D,
        DATAPOINTLINE,
        DATAPOINTMARKER,
        DATAPOINTMARKERLAYOUT,
        DATAPOINTWIREFRAME,
        DATATABLE,
        DOWNBAR,
        DROPLINE,
        ERRORBAR,
        FLOOR,
        GRIDLINEMAJOR,
        GRIDLINEMINOR,
        HILOLINE,
        LEADERLINE,
        LEGEND,
        PLOTAREA,
        PLOTAREA3D,
        SERIESAXIS,
        SERIESLINE,
        TITLE,
        TRENDLINE,
        TRENDLINELABEL,
        UPBAR,
        VALUEAXIS,
        WALL
    };

    std::map<enum StyleEntryType, StyleEntry> maEntryMap;

    sal_Int32 mnId;

    void addEntry(enum StyleEntryType eType, const StyleEntry& aEntry);
};

} // namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
