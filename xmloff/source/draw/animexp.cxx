/*************************************************************************
 *
 *  $RCSfile: animexp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-16 14:35:56 $
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

#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif

#include <list>

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#include "xmlkywd.hxx"

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_SHAPEEXPORT_HXX_
#include "shapeexport.hxx"
#endif

#ifndef _XMLOFF_ANIM_HXX
#include "anim.hxx"
#endif

using namespace ::rtl;
using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::presentation;


SvXMLEnumMapEntry aXML_AnimationEffect_EnumMap[] =
{
    { sXML_none,        EK_none },
    { sXML_fade,        EK_fade },
    { sXML_move,        EK_move },
    { sXML_stripes,     EK_stripes },
    { sXML_open,        EK_open },
    { sXML_close,       EK_close },
    { sXML_dissolve,    EK_dissolve },
    { sXML_wavyline,    EK_wavyline },
    { sXML_random,      EK_random },
    { sXML_lines,       EK_lines },
    { sXML_laser,       EK_laser },
    { sXML_appear,      EK_appear },
    { sXML_hide,        EK_hide },
    { sXML_move_short,  EK_move_short },
    { sXML_checkerboard,EK_checkerboard },
    { sXML_rotate,      EK_rotate },
    { sXML_stretch,     EK_stretch },
    { NULL, 0 }
};

SvXMLEnumMapEntry aXML_AnimationDirection_EnumMap[] =
{
    { sXML_none,            ED_none },
    { sXML_from_left,       ED_from_left },
    { sXML_from_top,        ED_from_top },
    { sXML_from_right,      ED_from_right },
    { sXML_from_bottom,     ED_from_bottom },
    { sXML_from_center,     ED_from_center },
    { sXML_from_upper_left, ED_from_upperleft },
    { sXML_from_upper_right,ED_from_upperright },
    { sXML_from_lower_left, ED_from_lowerleft },
    { sXML_from_lower_right,ED_from_lowerright },
    { sXML_to_left,         ED_to_left },
    { sXML_to_top,          ED_to_top },
    { sXML_to_right,        ED_to_right },
    { sXML_to_bottom,       ED_to_bottom },
    { sXML_to_upper_left,   ED_to_upperleft },
    { sXML_to_upper_right,  ED_to_upperright },
    { sXML_to_lower_right,  ED_to_lowerright },
    { sXML_to_lower_left,   ED_to_lowerleft },
    { sXML_path,            ED_path },
    { sXML_spiral_inward_left, ED_spiral_inward_left },
    { sXML_spiral_inward_right,ED_spiral_inward_right },
    { sXML_spiral_outward_left, ED_spiral_outward_left },
    { sXML_spiral_outward_right, ED_spiral_outward_right },
    { sXML_vertical,        ED_vertical },
    { sXML_horizontal,      ED_horizontal },
    { sXML_to_center,       ED_to_center },
    { sXML_clockwise,       ED_clockwise },
    { sXML_counter_clockwise,ED_cclockwise },
    { NULL, 0 }
};

const struct Effect
{
    XMLEffect meKind;
    XMLEffectDirection meDirection;
    sal_Int16 mnStartScale;
    sal_Bool mbIn;
}
    AnimationEffectMap[] =
{
    { EK_none, ED_none,             -1, sal_True }, // AnimationEffect_NONE
    { EK_fade, ED_from_left,        -1, sal_True }, // AnimationEffect_FADE_FROM_LEFT
    { EK_fade, ED_from_top,         -1, sal_True }, // AnimationEffect_FADE_FROM_TOP
    { EK_fade, ED_from_right,       -1, sal_True }, // AnimationEffect_FADE_FROM_RIGHT
    { EK_fade, ED_from_bottom,      -1, sal_True }, // AnimationEffect_FADE_FROM_BOTTOM
    { EK_fade, ED_to_center,        -1, sal_True }, // AnimationEffect_FADE_TO_CENTER
    { EK_fade, ED_from_center,      -1, sal_True }, // AnimationEffect_FADE_FROM_CENTER
    { EK_move, ED_from_left,        -1, sal_True }, // AnimationEffect_MOVE_FROM_LEFT
    { EK_move, ED_from_top,         -1, sal_True }, // AnimationEffect_MOVE_FROM_TOP
    { EK_move, ED_from_right,       -1, sal_True }, // AnimationEffect_MOVE_FROM_RIGHT
    { EK_move, ED_from_bottom,      -1, sal_True }, // AnimationEffect_MOVE_FROM_BOTTOM
    { EK_stripes, ED_vertical,      -1, sal_True }, // AnimationEffect_VERTICAL_STRIPES
    { EK_stripes, ED_horizontal,    -1, sal_True }, // AnimationEffect_HORIZONTAL_STRIPES
    { EK_fade, ED_clockwise,        -1, sal_True }, // AnimationEffect_CLOCKWISE
    { EK_fade, ED_cclockwise,       -1, sal_True }, // AnimationEffect_COUNTERCLOCKWISE
    { EK_fade, ED_from_upperleft,   -1, sal_True }, // AnimationEffect_FADE_FROM_UPPERLEFT
    { EK_fade, ED_from_upperright,  -1, sal_True }, // AnimationEffect_FADE_FROM_UPPERRIGHT
    { EK_fade, ED_from_lowerleft,   -1, sal_True }, // AnimationEffect_FADE_FROM_LOWERLEFT
    { EK_fade, ED_from_lowerright,  -1, sal_True }, // AnimationEffect_FADE_FROM_LOWERRIGHT
    { EK_close,ED_vertical,         -1, sal_True }, // AnimationEffect_CLOSE_VERTICAL
    { EK_close,ED_horizontal,       -1, sal_True }, // AnimationEffect_CLOSE_HORIZONTAL
    { EK_open, ED_vertical,         -1, sal_True }, // AnimationEffect_OPEN_VERTICAL
    { EK_open, ED_horizontal,       -1, sal_True }, // AnimationEffect_OPEN_HORIZONTAL
    { EK_move, ED_path,             -1, sal_True }, // AnimationEffect_PATH
    { EK_move, ED_to_left,          -1, sal_False },// AnimationEffect_MOVE_TO_LEFT
    { EK_move, ED_to_top,           -1, sal_False },// AnimationEffect_MOVE_TO_TOP
    { EK_move, ED_to_right,         -1, sal_False },// AnimationEffect_MOVE_TO_RIGHT
    { EK_move, ED_to_bottom,        -1, sal_False },// AnimationEffect_MOVE_TO_BOTTOM
    { EK_fade, ED_spiral_inward_left,   -1, sal_True }, // AnimationEffect_SPIRALIN_LEFT
    { EK_fade, ED_spiral_inward_right,  -1, sal_True }, // AnimationEffect_SPIRALIN_RIGHT
    { EK_fade, ED_spiral_outward_left,  -1, sal_True }, // AnimationEffect_SPIRALOUT_LEFT
    { EK_fade, ED_spiral_outward_right, -1, sal_True }, // AnimationEffect_SPIRALOUT_RIGHT
    { EK_dissolve, ED_none,         -1, sal_True }, // AnimationEffect_DISSOLVE
    { EK_wavyline, ED_from_left,    -1, sal_True }, // AnimationEffect_WAVYLINE_FROM_LEFT
    { EK_wavyline, ED_from_top,     -1, sal_True }, // AnimationEffect_WAVYLINE_FROM_TOP
    { EK_wavyline, ED_from_right,   -1, sal_True }, // AnimationEffect_WAVYLINE_FROM_RIGHT
    { EK_wavyline, ED_from_bottom,  -1, sal_True }, // AnimationEffect_WAVYLINE_FROM_BOTTOM
    { EK_random, ED_none,           -1, sal_True }, // AnimationEffect_RANDOM
    { EK_lines, ED_vertical,        -1, sal_True }, // AnimationEffect_VERTICAL_LINES
    { EK_lines, ED_horizontal,      -1, sal_True }, // AnimationEffect_HORIZONTAL_LINES
    { EK_laser, ED_from_left,       -1, sal_True }, // AnimationEffect_LASER_FROM_LEFT
    { EK_laser, ED_from_top,        -1, sal_True }, // AnimationEffect_LASER_FROM_TOP
    { EK_laser, ED_from_right,      -1, sal_True }, // AnimationEffect_LASER_FROM_RIGHT
    { EK_laser, ED_from_bottom,     -1, sal_True }, // AnimationEffect_LASER_FROM_BOTTOM
    { EK_laser, ED_from_upperleft,  -1, sal_True }, // AnimationEffect_LASER_FROM_UPPERLEFT
    { EK_laser, ED_from_upperright, -1, sal_True }, // AnimationEffect_LASER_FROM_UPPERRIGHT
    { EK_laser, ED_from_lowerleft,  -1, sal_True }, // AnimationEffect_LASER_FROM_LOWERLEFT
    { EK_laser, ED_from_lowerright, -1, sal_True }, // AnimationEffect_LASER_FROM_LOWERRIGHT
    { EK_appear,ED_none,            -1, sal_True }, // AnimationEffect_APPEAR
    { EK_hide,  ED_none,            -1, sal_False },// AnimationEffect_HIDE
    { EK_move,  ED_from_upperleft,  -1, sal_True }, // AnimationEffect_MOVE_FROM_UPPERLEFT
    { EK_move,  ED_from_upperright, -1, sal_True }, // AnimationEffect_MOVE_FROM_UPPERRIGHT
    { EK_move,  ED_from_lowerright, -1, sal_True }, // AnimationEffect_MOVE_FROM_LOWERRIGHT
    { EK_move,  ED_from_lowerleft,  -1, sal_True }, // AnimationEffect_MOVE_FROM_LOWERLEFT
    { EK_move,  ED_to_upperleft,    -1, sal_False },// AnimationEffect_MOVE_TO_UPPERLEFT
    { EK_move,  ED_to_upperright,   -1, sal_False },// AnimationEffect_MOVE_TO_UPPERRIGHT
    { EK_move,  ED_to_lowerright,   -1, sal_False },// AnimationEffect_MOVE_TO_LOWERRIGHT
    { EK_move,  ED_to_lowerleft,    -1, sal_False },// AnimationEffect_MOVE_TO_LOWERLEFT
    { EK_move_short, ED_from_left,  -1, sal_True }, // AnimationEffect_MOVE_SHORT_FROM_LEFT
    { EK_move_short, ED_from_upperleft, -1, sal_True }, // AnimationEffect_MOVE_SHORT_FROM_UPPERLEFT
    { EK_move_short, ED_from_top,       -1, sal_True }, // AnimationEffect_MOVE_SHORT_FROM_TOP
    { EK_move_short, ED_from_upperright,-1, sal_True }, // AnimationEffect_MOVE_SHORT_FROM_UPPERRIGHT
    { EK_move_short, ED_from_right,     -1, sal_True }, // AnimationEffect_MOVE_SHORT_FROM_RIGHT
    { EK_move_short, ED_from_lowerright,-1, sal_True }, // AnimationEffect_MOVE_SHORT_FROM_LOWERRIGHT
    { EK_move_short, ED_from_bottom,    -1, sal_True }, // AnimationEffect_MOVE_SHORT_FROM_BOTTOM
    { EK_move_short, ED_from_lowerleft, -1, sal_True }, // AnimationEffect_MOVE_SHORT_FROM_LOWERLEFT
    { EK_move_short, ED_to_left,        -1, sal_False },// AnimationEffect_MOVE_SHORT_TO_LEFT
    { EK_move_short, ED_to_upperleft,   -1, sal_False },// AnimationEffect_MOVE_SHORT_TO_UPPERLEFT
    { EK_move_short, ED_to_top,         -1, sal_False },// AnimationEffect_MOVE_SHORT_TO_TOP
    { EK_move_short, ED_to_upperright,  -1, sal_False },// AnimationEffect_MOVE_SHORT_TO_UPPERRIGHT
    { EK_move_short, ED_to_right,       -1, sal_False },// AnimationEffect_MOVE_SHORT_TO_RIGHT
    { EK_move_short, ED_to_lowerright,  -1, sal_False },// AnimationEffect_MOVE_SHORT_TO_LOWERRIGHT
    { EK_move_short, ED_to_bottom,      -1, sal_False },// AnimationEffect_MOVE_SHORT_TO_BOTTOM
    { EK_move_short, ED_to_lowerleft,   -1, sal_False },// AnimationEffect_MOVE_SHORT_TO_LOWERLEFT
    { EK_checkerboard, ED_vertical,     -1, sal_True }, // AnimationEffect_VERTICAL_CHECKERBOARD
    { EK_checkerboard, ED_horizontal,   -1, sal_True }, // AnimationEffect_HORIZONTAL_CHECKERBOARD
    { EK_rotate, ED_horizontal,     -1, sal_True }, // AnimationEffect_HORIZONTAL_ROTATE
    { EK_rotate, ED_vertical,       -1, sal_True }, // AnimationEffect_VERTICAL_ROTATE
    { EK_stretch,ED_horizontal,     -1, sal_True }, // AnimationEffect_HORIZONTAL_STRETCH
    { EK_stretch,ED_vertical,       -1, sal_True }, // AnimationEffect_VERTICAL_STRETCH
    { EK_stretch,ED_from_left,      -1, sal_True }, // AnimationEffect_STRETCH_FROM_LEFT
    { EK_stretch,ED_from_upperleft, -1, sal_True }, // AnimationEffect_STRETCH_FROM_UPPERLEFT
    { EK_stretch,ED_from_top,       -1, sal_True }, // AnimationEffect_STRETCH_FROM_TOP
    { EK_stretch,ED_from_upperright,-1, sal_True }, // AnimationEffect_STRETCH_FROM_UPPERRIGHT
    { EK_stretch,ED_from_right,     -1, sal_True }, // AnimationEffect_STRETCH_FROM_RIGHT
    { EK_stretch,ED_from_lowerright,-1, sal_True }, // AnimationEffect_STRETCH_FROM_LOWERRIGHT
    { EK_stretch,ED_from_bottom,    -1, sal_True }, // AnimationEffect_STRETCH_FROM_BOTTOM
    { EK_stretch,ED_from_lowerleft, -1, sal_True }, // AnimationEffect_STRETCH_FROM_LOWERLEFT
    { EK_move,  ED_none,             0, sal_True }, // AnimationEffect_ZOOM_IN
    { EK_move,  ED_none,            50, sal_True }, // AnimationEffect_ZOOM_IN_SMALL
    { EK_move,  ED_spiral_inward_left,   0, sal_True }, // AnimationEffect_ZOOM_IN_SPIRAL
    { EK_move,  ED_none,           400, sal_True }, // AnimationEffect_ZOOM_OUT
    { EK_move,  ED_none,           200, sal_True }, // AnimationEffect_ZOOM_OUT_SMALL
    { EK_move,  ED_spiral_inward_left, 400, sal_True }, // AnimationEffect_ZOOM_OUT_SPIRAL
    { EK_move,  ED_from_left,        0, sal_True }, // AnimationEffect_ZOOM_IN_FROM_LEFT
    { EK_move,  ED_from_upperleft,   0, sal_True }, // AnimationEffect_ZOOM_IN_FROM_UPPERLEFT
    { EK_move,  ED_from_top,         0, sal_True }, // AnimationEffect_ZOOM_IN_FROM_TOP
    { EK_move,  ED_from_upperright,  0, sal_True }, // AnimationEffect_ZOOM_IN_FROM_UPPERRIGHT
    { EK_move,  ED_from_right,       0, sal_True }, // AnimationEffect_ZOOM_IN_FROM_RIGHT
    { EK_move,  ED_from_lowerright,  0, sal_True }, // AnimationEffect_ZOOM_IN_FROM_LOWERRIGHT
    { EK_move,  ED_from_bottom,      0, sal_True }, // AnimationEffect_ZOOM_IN_FROM_BOTTOM
    { EK_move,  ED_from_lowerleft,   0, sal_True }, // AnimationEffect_ZOOM_IN_FROM_LOWERLEFT
    { EK_move,  ED_from_center,      0, sal_True }, // AnimationEffect_ZOOM_IN_FROM_CENTER
    { EK_move,  ED_from_left,      400, sal_True }, // AnimationEffect_ZOOM_OUT_FROM_LEFT
    { EK_move,  ED_from_upperleft, 400, sal_True }, // AnimationEffect_ZOOM_OUT_FROM_UPPERLEFT
    { EK_move,  ED_from_top,       400, sal_True }, // AnimationEffect_ZOOM_OUT_FROM_TOP
    { EK_move,  ED_from_upperright,400, sal_True }, // AnimationEffect_ZOOM_OUT_FROM_UPPERRIGHT
    { EK_move,  ED_from_right,     400, sal_True }, // AnimationEffect_ZOOM_OUT_FROM_RIGHT
    { EK_move,  ED_from_lowerright,400, sal_True }, // AnimationEffect_ZOOM_OUT_FROM_LOWERRIGHT
    { EK_move,  ED_from_bottom,    400, sal_True }, // AnimationEffect_ZOOM_OUT_FROM_BOTTOM
    { EK_move,  ED_from_lowerleft, 400, sal_True }, // AnimationEffect_ZOOM_OUT_FROM_LOWERLEFT
    { EK_move,  ED_from_center,    400, sal_True }  // AnimationEffect_ZOOM_OUT_FROM_CENTER
};

void SdXMLImplSetEffect( AnimationEffect eEffect, XMLEffect& eKind, XMLEffectDirection& eDirection, sal_Int16& nStartScale, sal_Bool& bIn )
{
    if( eEffect < AnimationEffect_NONE || eEffect > AnimationEffect_ZOOM_OUT_FROM_CENTER )
    {
        DBG_ERROR( "unknown animation effect!" );
        eEffect = AnimationEffect_NONE;
    }

    const Effect& rEffect = AnimationEffectMap[eEffect];
    eKind = rEffect.meKind;
    eDirection = rEffect.meDirection;
    nStartScale = rEffect.mnStartScale;
    bIn = rEffect.mbIn;
}

SvXMLEnumMapEntry aXML_AnimationSpeed_EnumMap[] =
{
    { sXML_slow,    AnimationSpeed_SLOW },
    { sXML_medium,  AnimationSpeed_MEDIUM },
    { sXML_fast,    AnimationSpeed_FAST },
    { NULL, 0 }
};

enum XMLActionKind
{
    XMLE_SHOW,
    XMLE_HIDE,
    XMLE_DIM
};

struct XMLEffectHint
{
    XMLActionKind   meKind;
    sal_Bool        mbTextEffect;
    sal_Int32       mnShapeId;

    XMLEffect       meEffect;
    XMLEffectDirection  meDirection;
    sal_Int16       mnStartScale;

    AnimationSpeed  meSpeed;
    Color           maDimColor;
    OUString        maSoundURL;
    sal_Bool        mbPlayFull;
    sal_Int32       mnPresId;
    sal_Int32       mnPathShapeId;

    int operator<(const XMLEffectHint& rComp) const { return mnPresId < rComp.mnPresId; }

    XMLEffectHint()
    :   meKind( XMLE_SHOW ), mbTextEffect( sal_False ), mnShapeId( -1 ),
        meEffect( EK_none ), meDirection( ED_none ), mnStartScale( -1 ),
        meSpeed( AnimationSpeed_SLOW ), maDimColor(0), mbPlayFull( sal_False ),
        mnPresId( 0 ), mnPathShapeId( -1 )
        {}
};

class AnimExpImpl
{
public:
    list<XMLEffectHint> maEffects;
    UniReference< XMLShapeExport > mxShapeExp;

    OUString msDimColor;
    OUString msDimHide;
    OUString msDimPrev;
    OUString msEffect;
    OUString msPlayFull;
    OUString msPresOrder;
    OUString msSound;
    OUString msSoundOn;
    OUString msSpeed;
    OUString msTextEffect;
    OUString msAnimPath;

    AnimExpImpl()
    :   msDimColor( RTL_CONSTASCII_USTRINGPARAM( "DimColor" ) ),
        msDimHide( RTL_CONSTASCII_USTRINGPARAM( "DimHide" ) ),
        msDimPrev( RTL_CONSTASCII_USTRINGPARAM( "DimPrevious" ) ),
        msEffect( RTL_CONSTASCII_USTRINGPARAM( "Effect" ) ),
        msPlayFull( RTL_CONSTASCII_USTRINGPARAM( "PlayFull" ) ),
        msPresOrder( RTL_CONSTASCII_USTRINGPARAM( "PresentationOrder" ) ),
        msSound( RTL_CONSTASCII_USTRINGPARAM( "Sound" ) ),
        msSoundOn( RTL_CONSTASCII_USTRINGPARAM( "SoundOn" ) ),
        msSpeed( RTL_CONSTASCII_USTRINGPARAM( "Speed" ) ),
        msTextEffect( RTL_CONSTASCII_USTRINGPARAM( "TextEffect" ) ),
        msAnimPath( RTL_CONSTASCII_USTRINGPARAM( "AnimationPath" ) )
    {}
};

XMLAnimationsExporter::XMLAnimationsExporter( XMLShapeExport* pShapeExp )
{
    mpImpl = new AnimExpImpl;
    mpImpl->mxShapeExp = pShapeExp;
}

XMLAnimationsExporter::~XMLAnimationsExporter()
{
    delete mpImpl;
    mpImpl = NULL;
}

void XMLAnimationsExporter::collect( Reference< XShape > xShape )
{
    try
    {
        // check for presentation shape service
        {
            Reference< XServiceInfo > xServiceInfo( xShape, UNO_QUERY );
            if( !xServiceInfo.is() || !xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.shape" ) ) ) )
                return;
        }

        Reference< XPropertySet > xProps( xShape, UNO_QUERY );
        if( xProps.is() )
        {
            const OUString aEmptyStr;

            Reference< XPropertySetInfo > xInfo( xProps->getPropertySetInfo() );
            AnimationEffect eEffect;
            XMLEffectHint aEffect;

            if( any2bool( xProps->getPropertyValue( mpImpl->msSoundOn ) ) )
            {
                xProps->getPropertyValue( mpImpl->msSound ) >>= aEffect.maSoundURL;
                xProps->getPropertyValue( mpImpl->msPlayFull ) >>= aEffect.mbPlayFull;
            }

            xProps->getPropertyValue( mpImpl->msPresOrder ) >>= aEffect.mnPresId;
            xProps->getPropertyValue( mpImpl->msSpeed ) >>= aEffect.meSpeed;
            xProps->getPropertyValue( mpImpl->msEffect ) >>= eEffect;
            if( eEffect != AnimationEffect_NONE )
            {
                sal_Bool bIn = sal_True;
                SdXMLImplSetEffect( eEffect, aEffect.meEffect, aEffect.meDirection, aEffect.mnStartScale, bIn );

                aEffect.meKind = bIn ? XMLE_SHOW : XMLE_HIDE;

                mpImpl->mxShapeExp->createShapeId( xShape );
                aEffect.mnShapeId = mpImpl->mxShapeExp->getShapeId( xShape );

                if( eEffect == AnimationEffect_PATH )
                {
                    Reference< XShape > xPath;
                    xProps->getPropertyValue( mpImpl->msAnimPath ) >>= xPath;
                    if( xPath.is() )
                    {
                        mpImpl->mxShapeExp->createShapeId( xPath );
                        aEffect.mnPathShapeId = mpImpl->mxShapeExp->getShapeId( xPath );
                    }
                }
                mpImpl->maEffects.push_back( aEffect );

                aEffect.mnPathShapeId = -1;
                aEffect.maSoundURL = aEmptyStr;
            }

            xProps->getPropertyValue( mpImpl->msTextEffect ) >>= eEffect;
            if( eEffect != AnimationEffect_NONE )
            {
                sal_Bool bIn = sal_True;
                SdXMLImplSetEffect( eEffect, aEffect.meEffect, aEffect.meDirection, aEffect.mnStartScale, bIn );
                aEffect.meKind = bIn ? XMLE_SHOW : XMLE_HIDE;
                aEffect.mbTextEffect = sal_True;

                if( aEffect.mnShapeId == -1 )
                {
                    mpImpl->mxShapeExp->createShapeId( xShape );
                    aEffect.mnShapeId = mpImpl->mxShapeExp->getShapeId( xShape );
                }

                mpImpl->maEffects.push_back( aEffect );
                aEffect.mbTextEffect = sal_False;
                aEffect.maSoundURL = aEmptyStr;
            }

            sal_Bool bDimPrev;
            sal_Bool bDimHide;
            xProps->getPropertyValue( mpImpl->msDimPrev ) >>= bDimPrev;
            xProps->getPropertyValue( mpImpl->msDimHide ) >>= bDimHide;
            if( bDimPrev || bDimHide )
            {
                aEffect.meKind = bDimPrev ? XMLE_DIM : XMLE_HIDE;
                aEffect.meEffect = EK_none;
                aEffect.meDirection = ED_none;
                aEffect.meSpeed = AnimationSpeed_MEDIUM;
                if( bDimPrev )
                {
                    sal_Int32 nColor;
                    xProps->getPropertyValue( mpImpl->msDimColor ) >>= nColor;
                    aEffect.maDimColor.SetColor( nColor );
                }

                if( aEffect.mnShapeId == -1 )
                {
                    mpImpl->mxShapeExp->createShapeId( xShape );
                    aEffect.mnShapeId = mpImpl->mxShapeExp->getShapeId( xShape );
                }

                mpImpl->maEffects.push_back( aEffect );
                aEffect.maSoundURL = aEmptyStr;
            }
        }
    }
    catch( Exception e )
    {
        DBG_ERROR("exception catched while collection animation information!");
    }
}

void XMLAnimationsExporter::exportAnimations( SvXMLExport& rExport )
{
    mpImpl->maEffects.sort();

    list<XMLEffectHint>::iterator aIter = mpImpl->maEffects.begin();
    const list<XMLEffectHint>::iterator aEnd = mpImpl->maEffects.end();

    OUStringBuffer sTmp;

    if( aIter != aEnd )
    {
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_PRESENTATION, sXML_animations, sal_True, sal_True );

        do
        {
            XMLEffectHint& rEffect = *aIter;

            DBG_ASSERT( rEffect.mnShapeId != -1, "shape id creation failed for animation effect?" );

            rExport.AddAttribute( XML_NAMESPACE_DRAW, sXML_shape_id, OUString::valueOf( rEffect.mnShapeId ) );

            if( rEffect.meKind == XMLE_DIM )
            {
                // export a dim action;

                SvXMLUnitConverter::convertColor( sTmp, rEffect.maDimColor );
                rExport.AddAttribute( XML_NAMESPACE_DRAW, sXML_color, sTmp.makeStringAndClear() );

                SvXMLElementExport aElem( rExport, XML_NAMESPACE_PRESENTATION, sXML_dim, sal_True, sal_True );
            }
            else
            {

                if( rEffect.meEffect != EK_none )
                {
                    SvXMLUnitConverter::convertEnum( sTmp, rEffect.meEffect, aXML_AnimationEffect_EnumMap );
                    rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, sXML_effect, sTmp.makeStringAndClear() );
                }

                if( rEffect.meDirection != ED_none )
                {
                    SvXMLUnitConverter::convertEnum( sTmp, rEffect.meDirection, aXML_AnimationDirection_EnumMap );
                    rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, sXML_direction, sTmp.makeStringAndClear() );
                }

                if( rEffect.mnStartScale != -1 )
                {
                    SvXMLUnitConverter::convertPercent( sTmp, rEffect.mnStartScale );
                    rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, sXML_start_scale, sTmp.makeStringAndClear() );
                }

                if( rEffect.meSpeed != AnimationSpeed_MEDIUM )
                {
                    SvXMLUnitConverter::convertEnum( sTmp, rEffect.meSpeed, aXML_AnimationSpeed_EnumMap );
                    rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, sXML_speed, sTmp.makeStringAndClear() );
                }

                if( rEffect.mnPathShapeId != -1 )
                {
                    rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, sXML_path_id, OUString::valueOf( rEffect.mnPathShapeId ) );
                }

                char *pLocalName;
                if( rEffect.meKind == XMLE_SHOW )
                {
                    if( rEffect.mbTextEffect )
                        pLocalName = sXML_show_text;
                    else
                        pLocalName = sXML_show_shape;
                }
                else
                {
                    if( rEffect.mbTextEffect )
                        pLocalName = sXML_hide_text;
                    else
                        pLocalName = sXML_hide_shape;
                }

                SvXMLElementExport aElem( rExport, XML_NAMESPACE_PRESENTATION, pLocalName, sal_True, sal_True );
                if( rEffect.maSoundURL.getLength() != 0 )
                {
                    rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_href, rEffect.maSoundURL );
                    rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_type, sXML_simple );
                    rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_show, sXML_new );
                    rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_actuate, sXML_onRequest );
                    if( rEffect.mbPlayFull )
                        rExport.AddAttributeASCII( XML_NAMESPACE_PRESENTATION, sXML_play_full, sXML_true );

                    SvXMLElementExport aElem( rExport, XML_NAMESPACE_PRESENTATION, sXML_sound, sal_True, sal_True );
                }
            }

            aIter++;
        }
        while( aIter != aEnd );
    }

    mpImpl->maEffects.clear();
}
