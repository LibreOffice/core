/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_OOX_DRAWINGML_TEXTBODYPROPERTIES_HXX
#define INCLUDED_OOX_DRAWINGML_TEXTBODYPROPERTIES_HXX

#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <oox/helper/helper.hxx>
#include <oox/helper/propertymap.hxx>
#include <optional>
#include <array>

class Size;

namespace oox::drawingml {

struct TextBodyProperties
{
    PropertyMap                                     maPropertyMap;
    // TextPreRotateAngle. Used in diagram (SmartArt) import.
    std::optional< sal_Int32 >                      moTextPreRotation;
    // TextRotateAngle. ODF draw:text-rotate-angle, OOXML 'rot' attribute in <bodyPr> element
    std::optional< sal_Int32 >                      moTextAreaRotation;
    bool                                            mbAnchorCtr;
    std::optional< sal_Int32 >                      moVert;
    bool                                            moUpright = false;
    std::array<std::optional<sal_Int32>, 4> moInsets;
    std::optional< sal_Int32 >                    moTextOffUpper;
    std::optional< sal_Int32 >                    moTextOffLeft;
    std::optional< sal_Int32 >                    moTextOffLower;
    std::optional< sal_Int32 >                    moTextOffRight;
    css::drawing::TextVerticalAdjust                meVA;
    OUString                                        msPrst;
    /// Normal autofit: font scale (default: 100%).
    sal_Int32 mnFontScale = 100000;
    sal_Int32 mnSpacingScale = 100000;
    OUString msHorzOverflow;
    std::optional< sal_Int32 > moVertOverflow{};

    std::array<std::optional<sal_Int32>, 4> maTextDistanceValues;

    explicit TextBodyProperties();

    void pushTextDistances(Size const& rShapeSize);
    void readjustTextDistances(css::uno::Reference<css::drawing::XShape> const& xShape);
    void pushVertSimulation();

};


} // namespace oox::drawingml

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
