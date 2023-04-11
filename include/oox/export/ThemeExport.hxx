/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <oox/dllapi.h>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/export/utils.hxx>

namespace model
{
class Theme;
class FontScheme;
class FormatScheme;
class EffectStyle;
class LineStyle;
class FillStyle;
class BlipFill;
class PatternFill;
class GradientFill;
class SolidFill;
struct ColorDefinition;
struct Transformation;
}

namespace oox
{
class OOX_DLLPUBLIC ThemeExport
{
private:
    oox::core::XmlFilterBase* mpFilterBase;
    oox::drawingml::DocumentType meDocumentType;
    sax_fastparser::FSHelperPtr mpFS;

public:
    ThemeExport(oox::core::XmlFilterBase* pFilterBase, oox::drawingml::DocumentType eDocumentType);

    void write(OUString const& rPath, model::Theme const& rTheme);

private:
    bool writeColorSet(model::Theme const& rTheme);
    bool writeFontScheme(model::FontScheme const& rFontScheme);
    bool writeFormatScheme(model::FormatScheme const& rFormatScheme);

    void writeEffectStyle(model::EffectStyle const& rEffectStyle);
    void writeLineStyle(model::LineStyle const& rLineStyle);
    void writeBackgroundFillStyle(model::FillStyle const& rFillStyle);
    void writeFillStyle(model::FillStyle const& rFillStyle);
    void writeBlipFill(model::BlipFill const& rBlipFill);
    void writeBlip(model::BlipFill const& rBlipFill);
    void writePatternFill(model::PatternFill const& rPatternFill);
    void writeGradientFill(model::GradientFill const& rGradientFill);
    void writeSolidFill(model::SolidFill const& rSolidFill);
    void writeColorDefinition(model::ColorDefinition const& rColorDefinition);
    void writeColorPlaceholder(model::ColorDefinition const& rColorDefinition);
    void writeColorSystem(model::ColorDefinition const& rColorDefinition);
    void writeColorScheme(model::ColorDefinition const& rColorDefinition);
    void writeColorHSL(model::ColorDefinition const& rColorDefinition);
    void writeColorCRGB(model::ColorDefinition const& rColorDefinition);
    void writeColorRGB(model::ColorDefinition const& rColorDefinition);
    void writeColorTransformations(std::vector<model::Transformation> const& rTransformations);
};

} // end namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
