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
struct FontOrStyleRef
{
    sal_Int32 mnIdx; // required

    enum class StyleColorEnum
    {
        AUTO
    };

    // The schema uses 'union' here. Implement with a std::variant for safety.
    using StyleColorVal = std::variant<sal_uInt32, enum StyleColorEnum, OUString>;
    std::unique_ptr<StyleColorVal> maStyleClr; // optional

    // Get a string version of the ColorVal, for output/streaming
    OUString getColorValStr() const
    {
        OUString sV;

        if (maStyleClr)
        {
            switch (maStyleClr->index())
            {
                case 0: // sal_uInt32, a raw color value
                    sV = OUString::number(std::get<0>(*maStyleClr));
                    break;
                case 1: // enum; only value "auto"
                    sV = "auto";
                    break;
                case 2: // arbitrary string
                    sV = std::get<2>(*maStyleClr);
                    break;
            }
        }
        return sV;
    }

    // A child element of cs:CT_FontReference or cs:CT_StyleReference is
    // a:EG_ColorChoice. The latter is handled by ColorValueContext.
    oox::drawingml::Color maColor; // needed for ColorValueContext
    model::ComplexColor maComplexColor; // needed for ColorValueContext
    // There's also an a:EG_ColorTransform member and a 'mods' member for
    // FontOrStyleRef. Ignore those for now. TODO
};

struct StyleEntry
{
    std::shared_ptr<FontOrStyleRef> mxLnClr;
    double mfLineWidthScale = 1.0;
    std::shared_ptr<FontOrStyleRef> mxFillClr;
    std::shared_ptr<FontOrStyleRef> mxEffectClr;
    std::shared_ptr<FontOrStyleRef> mxFontClr;
    std::shared_ptr<oox::drawingml::Shape> mxShapePr;
    // The following is derived from a TextCharacterProperties
    std::shared_ptr<oox::PropertyMap> mrTextCharacterPr;
    // The following is derived from a TextBodyProperties
    std::shared_ptr<oox::PropertyMap> mxTextBodyPr;

    StyleEntry(std::shared_ptr<FontOrStyleRef> aLnClr, double fLineScale,
               std::shared_ptr<FontOrStyleRef> aFillClr, std::shared_ptr<FontOrStyleRef> aEffectClr,
               std::shared_ptr<FontOrStyleRef> aFontClr,
               std::shared_ptr<oox::drawingml::Shape> aShape,
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
        BEGIN = 0, // for iteration
        AXISTITLE = BEGIN,
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
        WALL,
        END // for iteration
    };

    std::map<enum StyleEntryType, StyleEntry> maEntryMap;

    sal_Int32 mnId;

    void addEntry(enum StyleEntryType eType, const StyleEntry& aEntry);
};

} // namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
