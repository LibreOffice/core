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



#ifndef _XMLOFF_ANIM_HXX
#define _XMLOFF_ANIM_HXX

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <xmloff/uniref.hxx>
#include <xmloff/xmlement.hxx>

enum XMLEffect
{
    EK_none,
    EK_fade,
    EK_move,
    EK_stripes,
    EK_open,
    EK_close,
    EK_dissolve,
    EK_wavyline,
    EK_random,
    EK_lines,
    EK_laser,
    EK_appear,
    EK_hide,
    EK_move_short,
    EK_checkerboard,
    EK_rotate,
    EK_stretch
};

extern SvXMLEnumMapEntry aXML_AnimationEffect_EnumMap[];

enum XMLEffectDirection
{
    ED_none,
    ED_from_left,
    ED_from_top,
    ED_from_right,
    ED_from_bottom,
    ED_from_center,
    ED_from_upperleft,
    ED_from_upperright,
    ED_from_lowerleft,
    ED_from_lowerright,

    ED_to_left,
    ED_to_top,
    ED_to_right,
    ED_to_bottom,
    ED_to_upperleft,
    ED_to_upperright,
    ED_to_lowerright,
    ED_to_lowerleft,

    ED_path,
    ED_spiral_inward_left,
    ED_spiral_inward_right,
    ED_spiral_outward_left,
    ED_spiral_outward_right,

    ED_vertical,
    ED_horizontal,

    ED_to_center,

    ED_clockwise,
    ED_cclockwise
};

extern SvXMLEnumMapEntry aXML_AnimationDirection_EnumMap[];

extern SvXMLEnumMapEntry aXML_AnimationSpeed_EnumMap[];

void SdXMLImplSetEffect( ::com::sun::star::presentation::AnimationEffect eEffect, XMLEffect& eKind, XMLEffectDirection& eDirection, sal_Int16& nStartScale, sal_Bool& bIn );
::com::sun::star::presentation::AnimationEffect ImplSdXMLgetEffect( XMLEffect eKind, XMLEffectDirection eDirection, sal_Int16 nStartScale, sal_Bool bIn );

#endif  //  _XMLOFF_ANIM_HXX

