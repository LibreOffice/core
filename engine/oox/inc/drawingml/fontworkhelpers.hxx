/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <array>
#include <optional>
#include <span>
#include <docmodel/color/ComplexColor.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svx/msdffdef.hxx>

#include "customshapeproperties.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XText.hpp>

namespace FontworkHelpers
{
/** Removes the property specified by rName from the rPropVec vector of properties */
void resetPropertyValueInVec(std::vector<css::beans::PropertyValue>& rPropVec,
                             const OUString& rName);

/** Changes the EnhancedCustomShapeGeometry of xShape shape so, that it becomes a LO Fontwork
    shape corresponding to the OOXML shape specified by sMSPresetType.
    Precondition: xShape is a custom shape.*/
void putCustomShapeIntoTextPathMode(
    const cpo::uno::Reference<css::drawing::XShape>& xShape,
    const oox::drawingml::CustomShapePropertiesPtr& pCustomShapePropertiesPtr,
    const OUString& sMSPresetType, const bool bFromWordArt);

/** Returns the markup for the v:shapetype element for export of a Fontwork shape to VML.
    If eShapeType is not a Fontwork shape type or the special type is not yet implemented,
    it returns an empty string.*/
OString GetVMLFontworkShapetypeMarkup(const MSO_SPT eShapeType);

/** The colour properties of a run that the Fontwork export replaces by the shape's fill:
    "CharColor", "CharLumMod", "CharLumOff", "CharColorTheme", "CharComplexColor",
    "CharTransparence". */
inline constexpr std::array<std::u16string_view, 7> aFontworkCharColorNames
    = { u"CharColor",      u"CharLumMod",       u"CharLumOff",       u"CharColorTheme",
        u"CharComplexColor", u"CharTransparence", u"CharInteropGrabBag" };

/** Whether the colours of the runs of rXText follow the shape: the shape has a solid fill and the
    first non-empty run has that colour set directly, which is what the import of a WordArt leaves
    behind. The engine draws a Fontwork with the fill of the shape, so the colours of the runs are
    only meaningful when they are in step with it. */
bool runColorsFollowShape(const cpo::uno::Reference<css::text::XText>& rXText,
                          const cpo::uno::Reference<css::beans::XPropertySet>& rXPropSet);

/** The colour of a run that differs from the solid fill of the shape: a directly set CharColor
    that is not automatic and not the fill colour. Nothing when the run has no colour of its own.
    Whether such a colour counts is decided by runColorsFollowShape. */
std::optional<sal_Int32>
ownColorOfRun(const cpo::uno::Reference<css::beans::XPropertySet>& rXRun,
              const cpo::uno::Reference<css::beans::XPropertySet>& rXPropSet);

/** A non-empty run of a Fontwork text, and whether it has a colour of its own. The run keeps its
    range, so that the runs can be written to after they were changed and merged. */
struct FontworkRun
{
    cpo::uno::Reference<css::beans::XPropertySet> xRun;
    bool bOwnColor = false;
};

/** The non-empty runs of rXText, in order, with their own-colour flag; a run has a colour of its
    own only when the run colours follow the shape (see runColorsFollowShape). */
void collectFontworkRuns(const cpo::uno::Reference<css::text::XText>& rXText,
                         const cpo::uno::Reference<css::beans::XPropertySet>& rXPropSet,
                         std::vector<FontworkRun>& rRuns);

/** Applies the properties to the collected runs; a run with a colour of its own is left out when
    bSkipOwnColor is set. */
void applyPropsToFontworkRuns(const std::vector<css::beans::PropertyValue>& rTextPropVec,
                              const std::vector<FontworkRun>& rRuns, bool bSkipOwnColor);

/** Puts the updates into the "CharInteropGrabBag" of the collected runs. A run with a colour of
    its own gets the updates that are not about the text fill (the outline among them), and loses
    the text fill entries its grab bag holds, so that its colour is what Word shows. */
void applyUpdatesToFontworkRunGrabBags(const std::vector<css::beans::PropertyValue>& rUpdatePropVec,
                                       const std::vector<FontworkRun>& rRuns);

/** The character attributes a Fontwork is drawn with: "CharHeight", "CharWeight", "CharPosture",
    "CharFontName", "CharFontNameAsian", "CharFontNameComplex" and "CharScaleWidth", and
    "CharKerning", which the renderer does not draw and the import puts on the shape. */
inline constexpr std::array<std::u16string_view, 8> aFontworkCharAttrNames
    = { u"CharHeight",        u"CharWeight",          u"CharPosture", u"CharFontName",
        u"CharFontNameAsian", u"CharFontNameComplex", u"CharKerning", u"CharScaleWidth" };

/** A run of a text with the properties it had: the values set directly on it, and the names of
    the ones it inherited. The run keeps its range, so that the properties can be given back to
    it after the runs of the text were changed and merged. */
struct RunProps
{
    cpo::uno::Reference<css::beans::XPropertySet> xRun;
    std::vector<css::beans::PropertyValue> aDirect;
    std::vector<OUString> aInherited;
};

/** Collects the given properties of every non-empty run of rXText, in order. */
void collectRunProps(const cpo::uno::Reference<css::text::XText>& rXText,
                     std::span<const std::u16string_view> rNames,
                     std::vector<RunProps>& rRunProps);

/** Gives the collected runs their properties back: the direct values are set, the inherited
    ones set to their default. */
void applyRunProps(const std::vector<RunProps>& rRunProps);

/** Reads the character attributes of aFontworkCharAttrNames from the shape and puts them into
    rCharPropVec, so that they can be applied to the runs of its text. */
void createCharAttrPropsFromShape(const cpo::uno::Reference<css::beans::XPropertySet>& rXPropSet,
                                  std::vector<css::beans::PropertyValue>& rCharPropVec);

// The font height, in points, at which the longest line of the Fontwork's text in the shape's
// font is no wider than the shape: Word lays a warped text out at its font size and wraps it at
// the box, while the engine fits the text to the box. The shape's own height when the text fits.
float fittedCharHeight(const cpo::uno::Reference<css::beans::XPropertySet>& rXPropSet);

/** Generates the properties "CharColor", "CharLumMod", "CharLumOff", "CharColorTheme",
    "CharComplexColor" and "CharTransparence" from the shape properties "FillColor",
    "FillColorLumMod, "FillColorLumOff", "FillColorTheme", "FillComplexColor" and
    "FillTransparence" and puts them into rCharPropVec.*/
void createCharFillPropsFromShape(const cpo::uno::Reference<css::beans::XPropertySet>& rXPropSet,
                                  std::vector<css::beans::PropertyValue>& rCharPropVec);

/** Creates the properties "CharTextFillTextEffect", "CharTextOutlineTextEffect", "CharThemeColor",
    "CharThemeColorShade" or "CharThemeColorTint", and "CharThemeOriginalColor" from the FillFoo and
    LineBar properties of the shape and puts them into rUpdatePropVec.*/
void createCharInteropGrabBagUpdatesFromShapeProps(
    const cpo::uno::Reference<css::beans::XPropertySet>& rXPropSet,
    std::vector<css::beans::PropertyValue>& rUpdatePropVec);

// ToDo: This is essentially the same as contained in method DrawingML::WriteOutline. Change it
// there to use this method too, perhaps move this method to a common location.
/** Uses LineDash and LineCap to detect, whether the dashing comes from a prstDash in MS Office.
    If prstDash is detected, the method puts the corresponding string for markup into rsPrstDash
    and returns true.
    If no prstDash is detected, the method leaves rsPrstDash unchanged and returns false. */
bool createPrstDashFromLineDash(const css::drawing::LineDash& rLineDash,
                                const css::drawing::LineCap& rLineCap, OUString& rsPrstDash);

/** Returns true if a theme color with other type than model::ThemeColorType::Unknown was found.
    The theme color is then in rComplexColor.
    Returns false otherwise. rComplexColor is then unchanged or its type is
    model::ThemeColorType::Unknown */
bool getThemeColorFromShape(const OUString& rPropertyName,
                            const cpo::uno::Reference<css::beans::XPropertySet>& xPropertySet,
                            model::ComplexColor& rComplexColor);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
