/*************************************************************************
 *
 *  $RCSfile: anim.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2000-12-19 14:03:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_ANIM_HXX
#define _XMLOFF_ANIM_HXX

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

#ifndef _UNIVERSALL_REFERENCE_HXX
#include <uniref.hxx>
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
    ED_cclockwise,
};

extern SvXMLEnumMapEntry aXML_AnimationDirection_EnumMap[];

extern SvXMLEnumMapEntry aXML_AnimationSpeed_EnumMap[];

#endif  //  _XMLOFF_ANIM_HXX

