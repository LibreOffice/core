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
#include <oox/helper/helper.hxx>
#include <oox/helper/propertymap.hxx>
#include <optional>

namespace oox::drawingml {


struct TextBodyProperties
{
    PropertyMap                                     maPropertyMap;
    OptValue< sal_Int32 >                           moRotation;
    bool                                            mbAnchorCtr;
    OptValue< sal_Int32 >                           moVert;
    bool                                            moUpright = false;
    std::optional< sal_Int32 >                    moInsets[4];
    std::optional< sal_Int32 >                    moTextOffUpper;
    std::optional< sal_Int32 >                    moTextOffLeft;
    std::optional< sal_Int32 >                    moTextOffLower;
    std::optional< sal_Int32 >                    moTextOffRight;
    css::drawing::TextVerticalAdjust                meVA;
    OUString                                        msPrst;
    /// Number of requested columns.
    sal_Int32 mnNumCol = 1;
    /// Normal autofit: font scale (default: 100%).
    sal_Int32 mnFontScale = 100000;
    OUString msHorzOverflow;
    OUString msVertOverflow;

    explicit            TextBodyProperties();

    void                pushRotationAdjustments();
    void                pushVertSimulation();
};


} // namespace oox::drawingml

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
