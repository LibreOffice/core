/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef OOX_DRAWINGML_LINEPROPERTIES_HXX
#define OOX_DRAWINGML_LINEPROPERTIES_HXX

#include "oox/drawingml/fillproperties.hxx"

namespace oox {
namespace drawingml {

// ============================================================================

struct LineArrowProperties
{
    OptValue< sal_Int32 > moArrowType;
    OptValue< sal_Int32 > moArrowWidth;
    OptValue< sal_Int32 > moArrowLength;

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const LineArrowProperties& rSourceProps );
};

// ============================================================================

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
                            sal_Int32 nPhClr = API_RGB_TRANSPARENT ) const;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

