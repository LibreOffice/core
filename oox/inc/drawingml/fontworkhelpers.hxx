/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
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
    const css::uno::Reference<css::drawing::XShape>& xShape,
    const oox::drawingml::CustomShapePropertiesPtr& pCustomShapePropertiesPtr,
    const OUString& sMSPresetType, const bool bFromWordArt);

/** Returns the markup for the v:shapetype element for export of a Fontwork shape to VML.
    If eShapeType is not a Fontwork shape type or the special type is not yet implemented,
    it returns an empty string.*/
OString GetVMLFontworkShapetypeMarkup(const MSO_SPT eShapeType);

/** Collects the properties "CharColor", "CharLumMod", "CharLumOff", "CharColorTheme",
    "CharComplexColor" and "CharTransparence" from the first non-empty run in rXText and puts
    them into rCharPropVec.*/
void collectCharColorProps(const css::uno::Reference<css::text::XText>& rXText,
                           std::vector<css::beans::PropertyValue>& rCharPropVec);

/** Applies all properties in rTextPropVec excluding "CharInteropGrabBag" to all non-empty runs in
    rXText.*/
void applyPropsToRuns(const std::vector<css::beans::PropertyValue>& rTextPropVec,
                      const css::uno::Reference<css::text::XText>& rXText);

/** Generates the properties "CharColor", "CharLumMod", "CharLumOff", "CharColorTheme",
    "CharComplexColor" and "CharTransparence" from the shape properties "FillColor",
    "FillColorLumMod, "FillColorLumOff", "FillColorTheme", "FillComplexColor" and
    "FillTransparence" and puts them into rCharPropVec.*/
void createCharFillPropsFromShape(const css::uno::Reference<css::beans::XPropertySet>& rXPropSet,
                                  std::vector<css::beans::PropertyValue>& rCharPropVec);

/** Creates the properties "CharTextFillTextEffect", "CharTextOutlineTextEffect", "CharThemeColor",
    "CharThemeColorShade" or "CharThemeColorTint", and "CharThemeOriginalColor" from the FillFoo and
    LineBar properties of the shape and puts them into rUpdatePropVec.*/
void createCharInteropGrabBagUpdatesFromShapeProps(
    const css::uno::Reference<css::beans::XPropertySet>& rXPropSet,
    std::vector<css::beans::PropertyValue>& rUpdatePropVec);

/** Puts all properties in rUpdatePropVec into the "CharInteropGrabBag" of all non-empty runs in rXText.
    Existing properties are overwritten.*/
void applyUpdatesToCharInteropGrabBag(const std::vector<css::beans::PropertyValue>& rUpdatePropVec,
                                      const css::uno::Reference<css::text::XText>& rXText);

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
                            const css::uno::Reference<css::beans::XPropertySet>& xPropertySet,
                            model::ComplexColor& rComplexColor);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
