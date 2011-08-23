/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "oox/ppt/slidetransition.hxx"

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>

#include "oox/helper/helper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/core/namespaces.hxx"
#include "pptfilterhelpers.hxx"
#include "properties.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

namespace oox { namespace ppt {


    SlideTransition::SlideTransition()
        : mnTransitionType( 0 )
        , mnTransitionSubType( 0 )
        , mbTransitionDirectionNormal( true )
        , mnAnimationSpeed( AnimationSpeed_FAST )
        , mnFadeColor( 0 )
        , mbMode( true )
        , mnAdvanceTime( -1 )
    {

    }


    SlideTransition::SlideTransition(const OUString & sFilterName)
        : mnTransitionType( 0 )
        , mnTransitionSubType( 0 )
        , mbTransitionDirectionNormal( true )
        , mnAnimationSpeed( AnimationSpeed_FAST )
        , mnFadeColor( 0 )
        , mbMode( true )
        , mnAdvanceTime( -1 )
    {
        const transition *p = transition::find( sFilterName );
        if( p )
        {
            mnTransitionType = p->mnType;
            mnTransitionSubType = p->mnSubType;
            mbTransitionDirectionNormal = p->mbDirection;
        }
    }


    void SlideTransition::setSlideProperties( PropertyMap & aProps )
    {
        try
        {
            aProps[ PROP_TransitionType ] <<= mnTransitionType;
            aProps[ PROP_TransitionSubtype ] <<= mnTransitionSubType;
            aProps[ PROP_TransitionDirection ] <<= mbTransitionDirectionNormal;
            aProps[ PROP_Speed ] <<= mnAnimationSpeed;
            aProps[ PROP_TransitionFadeColor ] <<= mnFadeColor;
        if( mnAdvanceTime != -1 ) {
        aProps[ PROP_Duration ] <<= mnAdvanceTime/1000;
        aProps[ PROP_Change ] <<= static_cast<sal_Int32>(1);
        }
        }
        catch( Exception& )
        {
            // should not happen
            OSL_ENSURE( false, "exception raised" );
        }
    }

    void SlideTransition::setTransitionFilterProperties( const Reference< XTransitionFilter > & xFilter )
    {
        try
        {
            xFilter->setTransition( mnTransitionType );
            xFilter->setSubtype( mnTransitionSubType );
            xFilter->setDirection( mbTransitionDirectionNormal );
            xFilter->setFadeColor( mnFadeColor );
            xFilter->setMode( mbMode );
        }
        catch( Exception& )
        {
            // should not happen
            OSL_ENSURE( false, "exception raised" );
        }
    }


    void SlideTransition::setOoxTransitionSpeed( sal_Int32 nToken)
    {
        switch( nToken  )
        {
            /* In case you want to use time values in second,
             * the speed values are located in the PPT97 importer
             * sd/source/filter/ppt/ppt97animations.cxx:664
             * (void Ppt97Animation::UpdateCacheData() const)
             */
        case XML_fast:
            mnAnimationSpeed = AnimationSpeed_FAST;
            break;
        case XML_med:
            mnAnimationSpeed = AnimationSpeed_MEDIUM;
            break;
        case XML_slow:
            mnAnimationSpeed = AnimationSpeed_SLOW;
            break;
        default:
            // should not happen. just ignore
            break;
        }
    }

    sal_Int16 SlideTransition::ooxToOdpEightDirections( ::sal_Int32 nOoxType )
    {
    sal_Int16 nOdpDirection;
        nOdpDirection = ooxToOdpBorderDirections( nOoxType );
        if( nOdpDirection == 0 )
        {
            nOdpDirection = ooxToOdpCornerDirections( nOoxType );
        }
        return nOdpDirection;
    }


    sal_Int16 SlideTransition::ooxToOdpBorderDirections( ::sal_Int32 nOoxType )
    {
    sal_Int16 nOdpDirection;
        switch( nOoxType )
        {
        case XML_d:
            nOdpDirection = TransitionSubType::FROMTOP;
            break;
        case XML_l:
            nOdpDirection = TransitionSubType::FROMLEFT;
            break;
        case XML_r:
            nOdpDirection = TransitionSubType::FROMRIGHT;
            break;
        case XML_u:
            nOdpDirection = TransitionSubType::FROMBOTTOM;
            break;
        default:
            nOdpDirection= 0;
            break;
        }
        return nOdpDirection;
    }

    sal_Int16 SlideTransition::ooxToOdpSideDirections( ::sal_Int32 nOoxType )
    {
    sal_Int16 nOdpDirection;
        switch( nOoxType )
        {
        case XML_d:
        case XML_u:
            nOdpDirection = TransitionSubType::TOPTOBOTTOM;
            break;
        case XML_l:
        case XML_r:
            nOdpDirection = TransitionSubType::LEFTTORIGHT;
            break;
        default:
            nOdpDirection= 0;
            break;
        }
        return nOdpDirection;
    }

    sal_Bool SlideTransition::ooxToOdpSideDirectionsDirectionNormal( ::sal_Int32 nOoxType )
    {
    sal_Bool nOdpDirection = true;
        switch( nOoxType )
        {
        case XML_u:
        case XML_l:
            nOdpDirection = false;
            break;
        }
        return nOdpDirection;
    }

    sal_Int16 SlideTransition::ooxToOdpCornerDirections( ::sal_Int32 nOoxType )
    {
    sal_Int16 nOdpDirection;
        switch( nOoxType )
        {
        case XML_lu:
            nOdpDirection = TransitionSubType::FROMBOTTOMRIGHT;
            break;
        case XML_ru:
            nOdpDirection = TransitionSubType::FROMBOTTOMLEFT;
            break;
        case XML_ld:
            nOdpDirection = TransitionSubType::FROMTOPRIGHT;
            break;
        case XML_rd:
            nOdpDirection = TransitionSubType::FROMTOPLEFT;
            break;
        default:
            nOdpDirection = 0;
            break;
        }
        return nOdpDirection;
    }


    sal_Int16 SlideTransition::ooxToOdpDirection( ::sal_Int32 nOoxType )
    {
    sal_Int16 nOdpDir;
        switch( nOoxType )
        {
        case XML_vert:
            nOdpDir = TransitionSubType::VERTICAL;
            break;
        case XML_horz:
            nOdpDir = TransitionSubType::HORIZONTAL;
            break;
        default:
            nOdpDir = 0;
            break;
        }
        return nOdpDir;
    }

    void SlideTransition::setOoxTransitionType( ::sal_Int32 OoxType, ::sal_Int32 param1, ::sal_Int32 param2 )
    {
        switch( OoxType )
        {
        case NMSP_PPT|XML_blinds:
            mnTransitionType = TransitionType::BLINDSWIPE;
            mnTransitionSubType = ooxToOdpDirection( param1 );
            break;
        case NMSP_PPT|XML_checker:
            mnTransitionType = TransitionType::CHECKERBOARDWIPE;
            switch ( param1 )
            {
            case XML_vert:
                mnTransitionSubType = TransitionSubType::DOWN;
                break;
            case XML_horz:
                mnTransitionSubType = TransitionSubType::ACROSS;
                break;
            default:
                break;
            }
            break;
        case NMSP_PPT|XML_comb:
            mnTransitionType = TransitionType::PUSHWIPE;
            switch( param1 )
            {
            case XML_vert:
                mnTransitionSubType = TransitionSubType::COMBVERTICAL;
                break;
            case XML_horz:
                mnTransitionSubType = TransitionSubType::COMBHORIZONTAL;
                break;
            default:
                break;
            }
            break;
        case NMSP_PPT|XML_cover:
            mnTransitionType = TransitionType::SLIDEWIPE;
            mnTransitionSubType = ooxToOdpEightDirections( param1 );
            break;
        case NMSP_PPT|XML_pull: // uncover
            mnTransitionType = TransitionType::SLIDEWIPE;
            mnTransitionSubType = ooxToOdpEightDirections( param1 );
            mbTransitionDirectionNormal = false;
            break;
        case NMSP_PPT|XML_cut:
            // The binfilter seems to ignore this transition.
            // Fade to black instead if thrBlk is true.
            if( param1 )
            {
                mnTransitionType = TransitionType::FADE;
                mnTransitionSubType = TransitionSubType::FADEOVERCOLOR;
            }
            OSL_TRACE( "OOX: cut transition fallback." );
            break;
        case NMSP_PPT|XML_fade:
            mnTransitionType = TransitionType::FADE;
            if( param1 )
            {
                mnTransitionSubType = TransitionSubType::FADEOVERCOLOR;
            }
            else
            {
                mnTransitionSubType = TransitionSubType::CROSSFADE;
            }
            break;
        case NMSP_PPT|XML_push:
            mnTransitionType = TransitionType::PUSHWIPE;
            mnTransitionSubType = ooxToOdpBorderDirections( param1 );
            break;
        case NMSP_PPT|XML_wipe:
            mnTransitionType = TransitionType::BARWIPE;
            mnTransitionSubType = ooxToOdpSideDirections( param1 );
            mbTransitionDirectionNormal = ooxToOdpSideDirectionsDirectionNormal( param1 );
            break;
        case NMSP_PPT|XML_split:
            mnTransitionType = TransitionType::BARNDOORWIPE;
            mnTransitionSubType = ooxToOdpDirection( param1 );
            if( param2 == XML_in )
            {
                // reverse
                mbTransitionDirectionNormal = false;
            }
            break;
        case NMSP_PPT|XML_wheel:
            mnTransitionType = TransitionType::PINWHEELWIPE;
            switch( param1 )
            {
            case 1:
                mnTransitionSubType = TransitionSubType::ONEBLADE;
                break;
            case 2:
                mnTransitionSubType = TransitionSubType::TWOBLADEVERTICAL;
                break;
            case 3:
                mnTransitionSubType = TransitionSubType::THREEBLADE;
                break;
            case 4:
                mnTransitionSubType = TransitionSubType::FOURBLADE;
                break;
            case 8:
                mnTransitionSubType = TransitionSubType::EIGHTBLADE;
                break;
            default:
                OSL_TRACE( "OOX: strange number of blades for thw wheel-wipe %d", param1 );
                if( param1 > 8 )
                {
                    mnTransitionSubType = TransitionSubType::EIGHTBLADE;
                }
                else if( param1 > 4 )
                {
                    mnTransitionSubType = TransitionSubType::FOURBLADE;
                }
                else if( param1 == 0)
                {
                    mnTransitionSubType = TransitionSubType::ONEBLADE;
                }
                break;
            }
            break;
        case NMSP_PPT|XML_randomBar:
            mnTransitionType = TransitionType::RANDOMBARWIPE;
            mnTransitionSubType = ooxToOdpDirection( param1 );
            break;
        case NMSP_PPT|XML_circle:
            mnTransitionType = TransitionType::ELLIPSEWIPE;
            mnTransitionSubType = TransitionSubType::CIRCLE;
            break;
        case NMSP_PPT|XML_diamond:
            mnTransitionType = TransitionType::IRISWIPE;
            mnTransitionSubType = TransitionSubType::DIAMOND;
            break;
        case NMSP_PPT|XML_dissolve:
            mnTransitionType = TransitionType::DISSOLVE;
            mnTransitionSubType = TransitionSubType::DEFAULT;
            break;
        case NMSP_PPT|XML_newsflash:
            // this is what the PPT binary filter does.... not sure I agree.
            mnTransitionType = TransitionType::FOURBOXWIPE;
            mnTransitionSubType = TransitionSubType::CORNERSOUT;
            break;
        case NMSP_PPT|XML_plus:
            mnTransitionType = TransitionType::FOURBOXWIPE;
            mnTransitionSubType = TransitionSubType::CORNERSOUT;
            break;
        case NMSP_PPT|XML_random:
            mnTransitionType = TransitionType::RANDOM;
            mnTransitionSubType = TransitionSubType::DEFAULT;
            break;
        case NMSP_PPT|XML_wedge:
            mnTransitionType = TransitionType::FANWIPE;
            mnTransitionSubType = TransitionSubType::CENTERTOP;
            break;
        case NMSP_PPT|XML_zoom:
            mnTransitionType = TransitionType::ZOOM;
            mnTransitionSubType = TransitionSubType::DEFAULT;
            break;
        default:
            mnTransitionType = 0;
            break;
        }
    }


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
