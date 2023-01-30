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

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svx/msdffdef.hxx>

#include "customshapeproperties.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/XShape.hpp>

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
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */