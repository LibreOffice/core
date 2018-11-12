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

#include <oox/ppt/slidetransition.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>

#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <oox/helper/helper.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <oox/ppt/pptfilterhelpers.hxx>

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
        , mfTransitionDurationInSeconds( -1.0 )
        , mbMode( true )
        , mnAdvanceTime( -1 )
    {

    }

    SlideTransition::SlideTransition(const OUString & sFilterName)
        : mnTransitionType( 0 )
        , mnTransitionSubType( 0 )
        , mbTransitionDirectionNormal( true )
        , mnAnimationSpeed( AnimationSpeed_FAST )
        , mfTransitionDurationInSeconds( -1.0 )
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
            aProps.setProperty( PROP_TransitionType, mnTransitionType);
            aProps.setProperty( PROP_TransitionSubtype, mnTransitionSubType);
            aProps.setProperty( PROP_TransitionDirection, mbTransitionDirectionNormal);
            aProps.setProperty( PROP_Speed, mnAnimationSpeed);
            if( mfTransitionDurationInSeconds >= 0.0 )
                aProps.setProperty( PROP_TransitionDuration, mfTransitionDurationInSeconds);
            aProps.setProperty( PROP_TransitionFadeColor, sal_Int32(0));
            if( mnAdvanceTime != -1 ) {
                aProps.setProperty( PROP_Duration, mnAdvanceTime/1000);
                aProps.setProperty( PROP_Change, static_cast<sal_Int32>(1));
            }
        }
        catch( Exception& )
        {
            // should not happen
            OSL_FAIL( "exception raised" );
        }
    }

    void SlideTransition::setTransitionFilterProperties( const Reference< XTransitionFilter > & xFilter )
    {
        try
        {
            xFilter->setTransition( mnTransitionType );
            xFilter->setSubtype( mnTransitionSubType );
            xFilter->setDirection( mbTransitionDirectionNormal );
            xFilter->setFadeColor( 0 );
            xFilter->setMode( mbMode );
        }
        catch( Exception& )
        {
            // should not happen
            OSL_FAIL( "exception raised" );
        }
    }

    void SlideTransition::setOoxTransitionSpeed( sal_Int32 nToken)
    {
        switch( nToken  )
        {
            /* the speed values are located in the PPT97 importer
             * sd/source/filter/ppt/pptin.cxx:1783
             * (void ImplSdPPTImport::ImportPageEffect)
             */
        case XML_fast:
            mnAnimationSpeed = AnimationSpeed_FAST;
            mfTransitionDurationInSeconds = 0.5;
            break;
        case XML_med:
            mnAnimationSpeed = AnimationSpeed_MEDIUM;
            mfTransitionDurationInSeconds = 0.75;
            break;
        case XML_slow:
            mnAnimationSpeed = AnimationSpeed_SLOW;
            mfTransitionDurationInSeconds = 1.0;
            break;
        default:
            // should not happen. just ignore
            break;
        }
    }

    void SlideTransition::setOoxTransitionSpeed( double fDurationInSeconds )
    {
        // for compatibility
        mnAnimationSpeed = ( fDurationInSeconds <= 0.5 ) ? AnimationSpeed_FAST
                                : ( fDurationInSeconds >= 1.0 ) ? AnimationSpeed_SLOW : AnimationSpeed_MEDIUM;
        mfTransitionDurationInSeconds = fDurationInSeconds;
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

    bool SlideTransition::ooxToOdpSideDirectionsDirectionNormal( ::sal_Int32 nOoxType )
    {
        bool bOdpDirection = true;
        switch( nOoxType )
        {
        case XML_u:
        case XML_l:
            bOdpDirection = false;
            break;
        }
        return bOdpDirection;
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
        case PPT_TOKEN( blinds ):
            mnTransitionType = TransitionType::BLINDSWIPE;
            mnTransitionSubType = ooxToOdpDirection( param1 );
            break;
        case PPT_TOKEN( checker ):
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
        case PPT_TOKEN( comb ):
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
        case PPT_TOKEN( cover ):
            mnTransitionType = TransitionType::SLIDEWIPE;
            mnTransitionSubType = ooxToOdpEightDirections( param1 );
            break;
        case PPT_TOKEN( pull ): // uncover
            mnTransitionType = TransitionType::SLIDEWIPE;
            mnTransitionSubType = ooxToOdpEightDirections( param1 );
            mbTransitionDirectionNormal = false;
            break;
        case PPT_TOKEN( cut ):
            if( param1 )
            {
                mnTransitionType = TransitionType::BARWIPE;
                mnTransitionSubType = TransitionSubType::FADEOVERCOLOR;
            }
            SAL_WARN("oox.ppt", "OOX: cut transition fallback." );
            break;
        case PPT_TOKEN( fade ):
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
        case PPT_TOKEN( push ):
            mnTransitionType = TransitionType::PUSHWIPE;
            mnTransitionSubType = ooxToOdpBorderDirections( param1 );
            break;
        case PPT_TOKEN( wipe ):
            mnTransitionType = TransitionType::BARWIPE;
            mnTransitionSubType = ooxToOdpSideDirections( param1 );
            mbTransitionDirectionNormal = ooxToOdpSideDirectionsDirectionNormal( param1 );
            break;
        case PPT_TOKEN( split ):
            mnTransitionType = TransitionType::BARNDOORWIPE;
            mnTransitionSubType = ooxToOdpDirection( param1 );
            if( param2 == XML_in )
            {
                // reverse
                mbTransitionDirectionNormal = false;
            }
            break;
        case PPT_TOKEN( wheel ):
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
                SAL_INFO(
                    "oox.ppt",
                    "strange number of blades for thw wheel-wipe " << param1);
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
        case PPT_TOKEN( randomBar ):
            mnTransitionType = TransitionType::RANDOMBARWIPE;
            mnTransitionSubType = ooxToOdpDirection( param1 );
            break;
        case PPT_TOKEN( circle ):
            mnTransitionType = TransitionType::ELLIPSEWIPE;
            mnTransitionSubType = TransitionSubType::CIRCLE;
            break;
        case PPT_TOKEN( diamond ):
            mnTransitionType = TransitionType::IRISWIPE;
            mnTransitionSubType = TransitionSubType::DIAMOND;
            break;
        case PPT_TOKEN( dissolve ):
            mnTransitionType = TransitionType::DISSOLVE;
            mnTransitionSubType = TransitionSubType::DEFAULT;
            break;
        case PPT_TOKEN( newsflash ):
            mnTransitionType = TransitionType::ZOOM;
            mnTransitionSubType = TransitionSubType::ROTATEIN;
            break;
        case PPT_TOKEN( plus ):
            mnTransitionType = TransitionType::FOURBOXWIPE;
            mnTransitionSubType = TransitionSubType::CORNERSOUT;
            break;
        case PPT_TOKEN( random ):
            mnTransitionType = TransitionType::RANDOM;
            mnTransitionSubType = TransitionSubType::DEFAULT;
            break;
        case PPT_TOKEN( wedge ):
            mnTransitionType = TransitionType::FANWIPE;
            mnTransitionSubType = TransitionSubType::CENTERTOP;
            break;
        case PPT_TOKEN( zoom ):
            mnTransitionType = TransitionType::ZOOM;
            mnTransitionSubType = TransitionSubType::DEFAULT;
            break;
        case P14_TOKEN(prism):
            mnTransitionType = TransitionType::MISCSHAPEWIPE;
            if (param1)
                mnTransitionSubType = TransitionSubType::CORNERSIN;
            else
                mnTransitionSubType = TransitionSubType::CORNERSOUT;
            break;
        case P14_TOKEN(vortex):
            mnTransitionType = TransitionType::MISCSHAPEWIPE;
            mnTransitionSubType = TransitionSubType::VERTICAL;
            break;
        case P14_TOKEN(ripple):
            mnTransitionType = TransitionType::MISCSHAPEWIPE;
            mnTransitionSubType = TransitionSubType::HORIZONTAL;
            break;
        case P14_TOKEN(glitter):
            mnTransitionType = TransitionType::MISCSHAPEWIPE;
            mnTransitionSubType = TransitionSubType::DIAMOND;
            break;
        case P14_TOKEN(honeycomb):
            mnTransitionType = TransitionType::MISCSHAPEWIPE;
            mnTransitionSubType = TransitionSubType::HEART;
            break;
        default:
            mnTransitionType = 0;
            break;
        }
    }

    void SlideTransition::setPresetTransition(OUString const & sPresetTransition)
    {
        if (sPresetTransition == "fallOver")
        {
            mnTransitionType = TransitionType::MISCSHAPEWIPE;
            mnTransitionSubType = TransitionSubType::LEFTTORIGHT;
        }
        else
        {
            mnTransitionType = 0;
        }
    }

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
