/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_ANIM_HXX
#define _XMLOFF_ANIM_HXX

#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONEFFECT_HPP_
#include <com/sun/star/presentation/AnimationEffect.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_ 
#include <com/sun/star/drawing/XShape.hpp>
#endif

#ifndef _UNIVERSALL_REFERENCE_HXX
#include <uniref.hxx>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif
namespace binfilter {

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

}//end of namespace binfilter
#endif	//  _XMLOFF_ANIM_HXX

