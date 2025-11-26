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
#include <comphelper/string.hxx>
#include <rtl/ustring.hxx>
#include <memory>
#include <variant>

namespace model
{
struct FontOrStyleRef
{
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

    // Convert an input string to ColorVal
    void setColorValStr(const std::optional<OUString>& str)
    {
        // The attribute here can be an integer, a string, or "auto" (which
        // of course is also a string, but is considered special). So we
        // need to try to convert the input string to an integer, and handle
        // it as an int if we can.
        if (str)
        {
            StyleColorVal v;

            const sal_Unicode* pRawStr = str->getStr();

            if (comphelper::string::isdigitAsciiString(pRawStr))
            {
                sal_uInt32 nIntVal = str->toUInt32();
                v = nIntVal;
            }
            else
            {
                // Not an integer, so see if it's the fixed enum
                if (*str == "auto")
                {
                    v = StyleColorEnum::AUTO;
                }
                else
                {
                    v = *str;
                }
            }
            maStyleClr = std::make_unique<StyleColorVal>(v);
        }
    }

    // A child element of cs:CT_FontReference or cs:CT_StyleReference is
    // a:EG_ColorChoice. The latter is handled by ColorValueContext.
    model::ComplexColor maComplexColor; // needed for ColorValueContext
    // There's also an a:EG_ColorTransform member and a 'mods' member for
    // FontOrStyleRef. Ignore those for now. TODO
};

struct StyleRef : public FontOrStyleRef
{
    sal_Int32 mnIdx = 0; // StyleMatrixColumnIndex
};

struct FontRef : public FontOrStyleRef
{
    enum class FontCollectionIndex
    {
        MAJOR,
        MINOR,
        NONE
    };

    FontCollectionIndex meIdx = FontCollectionIndex::MINOR; // required

    // Get the string value of the font collection index
    const char* getFontCollectionStr() const
    {
        switch (meIdx)
        {
            case FontCollectionIndex::MINOR:
                return "minor";
            case FontCollectionIndex::MAJOR:
                return "major";
            case FontCollectionIndex::NONE:
                return "none";
            default:
                assert(false);
        }
        return nullptr;
    }

    // Set the font collection index from a string
    void setFontCollectionIndex(std::u16string_view sIdx)
    {
        if (sIdx == std::u16string_view(u"major"))
        {
            meIdx = FontCollectionIndex::MAJOR;
        }
        else if (sIdx == std::u16string_view(u"minor"))
        {
            meIdx = FontCollectionIndex::MINOR;
        }
        else if (sIdx == std::u16string_view(u"none"))
        {
            meIdx = FontCollectionIndex::NONE;
        }
    }
};

struct StyleEntry
{
    std::shared_ptr<StyleRef> mxLnClr;
    double mfLineWidthScale = 1.0;
    std::shared_ptr<StyleRef> mxFillClr;
    std::shared_ptr<StyleRef> mxEffectClr;
    std::shared_ptr<FontRef> mxFontClr;
    std::shared_ptr<oox::drawingml::Shape> mxShapePr;
    // The following is derived from a TextCharacterProperties
    std::shared_ptr<oox::PropertyMap> mrTextCharacterPr;
    // The following is derived from a TextBodyProperties
    std::shared_ptr<oox::PropertyMap> mxTextBodyPr;

    StyleEntry(std::shared_ptr<StyleRef> aLnClr, double fLineScale,
               std::shared_ptr<StyleRef> aFillClr, std::shared_ptr<StyleRef> aEffectClr,
               std::shared_ptr<FontRef> aFontClr, std::shared_ptr<oox::drawingml::Shape> aShape,
               std::shared_ptr<oox::PropertyMap> aCharProps,
               std::shared_ptr<oox::PropertyMap> aBodyProps)
        : mxLnClr(std::move(aLnClr))
        , mfLineWidthScale(fLineScale)
        , mxFillClr(std::move(aFillClr))
        , mxEffectClr(std::move(aEffectClr))
        , mxFontClr(std::move(aFontClr))
        , mxShapePr(std::move(aShape))
        , mrTextCharacterPr(std::move(aCharProps))
        , mxTextBodyPr(std::move(aBodyProps))
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

    sal_Int32 mnId = -1;

    void addEntry(enum StyleEntryType eType, const StyleEntry& aEntry);
};

} // namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
