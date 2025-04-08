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

#ifndef INCLUDED_OOX_DRAWINGML_SHAPE3DPROPERTIES_HXX
#define INCLUDED_OOX_DRAWINGML_SHAPE3DPROPERTIES_HXX

#include <oox/drawingml/color.hxx>
#include <oox/helper/helper.hxx>

namespace oox::drawingml {


struct RotationProperties
{
    std::optional< sal_Int32 > mnLatitude;
    std::optional< sal_Int32 > mnLongitude;
    std::optional< sal_Int32 > mnRevolution;
};

struct BevelProperties
{
    std::optional< sal_Int32 > mnPreset;
    std::optional< sal_Int32 > mnWidth;
    std::optional< sal_Int32 > mnHeight;
};

struct Generic3DProperties
{
    std::optional< sal_Int32 > mnPreset;
    std::optional< float > mfFieldOfVision;
    std::optional< float > mfZoom;
    std::optional< sal_Int32 > mnLightRigDirection;
    std::optional< sal_Int32 > mnLightRigType;
    RotationProperties maCameraRotation;
    RotationProperties maLightRigRotation;

    std::optional< sal_Int32 > mnExtrusionH;
    std::optional< sal_Int32 > mnContourW;
    std::optional< sal_Int32 > mnShapeZ;
    std::optional< sal_Int32 > mnMaterial;
    Color maExtrusionColor;
    Color maContourColor;

    std::optional< BevelProperties > maTopBevelProperties;
    std::optional< BevelProperties > maBottomBevelProperties;

    static OUString     getCameraPrstName( sal_Int32 nElement );
    static OUString     getLightRigName( sal_Int32 nElement );
    static OUString     getLightRigDirName( sal_Int32 nElement );
    static OUString     getBevelPresetTypeString( sal_Int32 nType );
    static OUString     getPresetMaterialTypeString( sal_Int32 nType );

    css::uno::Sequence< css::beans::PropertyValue > getCameraAttributes();
    css::uno::Sequence< css::beans::PropertyValue > getLightRigAttributes();
    css::uno::Sequence< css::beans::PropertyValue > getShape3DAttributes(
            const GraphicHelper& rGraphicHelper, ::Color rPhClr );
    static css::uno::Sequence< css::beans::PropertyValue > getBevelAttributes( BevelProperties rProps );
    static css::uno::Sequence< css::beans::PropertyValue > getColorAttributes(
            const Color& rColor, const GraphicHelper& rGraphicHelper, ::Color rPhClr );
};

struct Shape3DProperties : Generic3DProperties
{};

struct Text3DProperties : Generic3DProperties
{};

} // namespace oox::drawingml

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
