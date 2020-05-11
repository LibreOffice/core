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

#ifndef INCLUDED_OOX_INC_DRAWINGML_LINEPROPERTIES_HXX
#define INCLUDED_OOX_INC_DRAWINGML_LINEPROPERTIES_HXX

#include <utility>
#include <vector>

#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <oox/helper/helper.hxx>
#include <sal/types.h>

#include <drawingml/fillproperties.hxx>

namespace oox { class GraphicHelper; }

namespace oox {
namespace drawingml {

class ShapePropertyMap;

struct LineArrowProperties
{
    OptValue< sal_Int32 > moArrowType;
    OptValue< sal_Int32 > moArrowWidth;
    OptValue< sal_Int32 > moArrowLength;

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const LineArrowProperties& rSourceProps );
};

struct LineProperties
{
    typedef ::std::pair< sal_Int32, sal_Int32 > DashStop;
    typedef ::std::vector< DashStop >           DashStopVector;

    LineArrowProperties maStartArrow;       /// Start line arrow style.
    LineArrowProperties maEndArrow;         /// End line arrow style.
    FillProperties      maLineFill;         /// Line fill (solid, gradient, ...).
    DashStopVector      maCustomDash;       /// User-defined line dash style.
    OptValue< sal_Int32 > moLineWidth;      /// Line width (EMUs).
    OptValue< sal_Int32 > moPresetDash;     /// Preset dash (OOXML token).
    OptValue< sal_Int32 > moLineCompound;   /// Line compound type (OOXML token).
    OptValue< sal_Int32 > moLineCap;        /// Line cap (OOXML token).
    OptValue< sal_Int32 > moLineJoint;      /// Line joint type (OOXML token).

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const LineProperties& rSourceProps );

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            ShapePropertyMap& rPropMap,
                            const GraphicHelper& rGraphicHelper,
                            ::Color nPhClr = API_RGB_TRANSPARENT ) const;

    /** Calculates the line style attribute from the internal state of the object */
    css::drawing::LineStyle  getLineStyle() const;
    /** Calculates the line cap attribute from the internal state of the object */
    css::drawing::LineCap  getLineCap() const;
    /** Calculates the line joint attribute from the internal state of the object */
    css::drawing::LineJoint  getLineJoint() const;
    /** Calculates the line width attribute from the internal state of the object */
    sal_Int32           getLineWidth() const;
    /** Calculates the precise line width attribute from the internal state of the object */
    double           getPreciseLineWidth() const;
};

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
