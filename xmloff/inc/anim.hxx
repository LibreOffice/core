/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: anim.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:02:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

