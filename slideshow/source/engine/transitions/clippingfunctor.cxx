/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include "clippingfunctor.hxx"
#include "transitiontools.hxx"

#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

namespace slideshow
{
    namespace internal
    {
        ClippingFunctor::ClippingFunctor(const ParametricPolyPolygonSharedPtr&   rPolygon,
                                         const TransitionInfo&                   rTransitionInfo,
                                         bool                                    bDirectionForward,
                                         bool                                    bModeIn ) :
            mpParametricPoly( rPolygon ),
            maStaticTransformation(),
            mbForwardParameterSweep( true ),
            mbSubtractPolygon( false ),
            mbScaleIsotrophically( rTransitionInfo.mbScaleIsotrophically ),
            mbFlip(false)
        {
            ENSURE_OR_THROW( rPolygon,
                              "ClippingFunctor::ClippingFunctor(): Invalid parametric polygon" );

            
            
            
            
            
            

            
            
            
            
            
            

            
            

            
            
            
            
            
            if (rTransitionInfo.mnRotationAngle != 0.0 ||
                rTransitionInfo.mnScaleX != 1.0 ||
                rTransitionInfo.mnScaleY != 1.0)
            {
                maStaticTransformation.translate( -0.5, -0.5 );
                
                if (rTransitionInfo.mnRotationAngle != 0.0)
                {
                    maStaticTransformation.rotate(
                        basegfx::deg2rad(rTransitionInfo.mnRotationAngle) );
                }
                if (rTransitionInfo.mnScaleX != 1.0 ||
                    rTransitionInfo.mnScaleY != 1.0)
                {
                    maStaticTransformation.scale(
                        rTransitionInfo.mnScaleX,
                        rTransitionInfo.mnScaleY );
                }
                maStaticTransformation.translate( 0.5, 0.5 );
            }

            if( !bDirectionForward )
            {
                
                
                
                switch( rTransitionInfo.meReverseMethod )
                {
                    default:
                        ENSURE_OR_THROW(
                            false,
                            "TransitionFactory::TransitionFactory(): Unexpected reverse method" );
                        break;

                    case TransitionInfo::REVERSEMETHOD_IGNORE:
                        break;

                    case TransitionInfo::REVERSEMETHOD_INVERT_SWEEP:
                        mbForwardParameterSweep = !mbForwardParameterSweep;
                        break;

                    case TransitionInfo::REVERSEMETHOD_SUBTRACT_POLYGON:
                        mbSubtractPolygon = !mbSubtractPolygon;
                        break;

                    case TransitionInfo::REVERSEMETHOD_SUBTRACT_AND_INVERT:
                        mbForwardParameterSweep = !mbForwardParameterSweep;
                        mbSubtractPolygon = !mbSubtractPolygon;
                        break;

                    case TransitionInfo::REVERSEMETHOD_ROTATE_180:
                        maStaticTransformation = basegfx::tools::createRotateAroundPoint(0.5, 0.5, M_PI)
                            * maStaticTransformation;
                        break;

                    case TransitionInfo::REVERSEMETHOD_FLIP_X:
                        maStaticTransformation = basegfx::tools::createScaleTranslateB2DHomMatrix(-1.0, 1.0, 1.0, 0.0)
                            * maStaticTransformation;
                        mbFlip = true;
                        break;

                    case TransitionInfo::REVERSEMETHOD_FLIP_Y:
                        maStaticTransformation = basegfx::tools::createScaleTranslateB2DHomMatrix(1.0, -1.0, 0.0, 1.0)
                            * maStaticTransformation;
                        mbFlip = true;
                        break;
                }
            }

            if( !bModeIn )
            {
                
                
                if( rTransitionInfo.mbOutInvertsSweep )
                    mbForwardParameterSweep = !mbForwardParameterSweep;
                else
                    mbSubtractPolygon = !mbSubtractPolygon;
            }
        }

        ::basegfx::B2DPolyPolygon ClippingFunctor::operator()( double                       nValue,
                                                               const ::basegfx::B2DSize&    rTargetSize )
        {
            
            
            ::basegfx::B2DHomMatrix aMatrix( maStaticTransformation );

            
            ::basegfx::B2DPolyPolygon aClipPoly = (*mpParametricPoly)(
                mbForwardParameterSweep ? nValue : 1.0 - nValue );

            
            if (aClipPoly.count() == 0)
                aClipPoly.append( basegfx::B2DPolygon() );

            if (mbFlip)
                aClipPoly.flip();

            
            
            
            

            if( mbSubtractPolygon )
            {
                
                

                
                

                
                
                const basegfx::B2DRange aBackgroundRange(-1, -1, 2, 2);
                const basegfx::B2DRange aClipPolyRange(aClipPoly.getB2DRange());

                if(aBackgroundRange.isInside(aClipPolyRange))
                {
                    
                    aClipPoly = ::basegfx::tools::correctOrientations(aClipPoly);
                    aClipPoly.flip();
                    aClipPoly.insert(0, basegfx::tools::createPolygonFromRect(aBackgroundRange));
                }
                else
                {
                    
                    
                    const basegfx::B2DPolyPolygon aBackgroundPolyPoly(basegfx::tools::createPolygonFromRect(aBackgroundRange));
                    aClipPoly = basegfx::tools::solvePolygonOperationDiff(aBackgroundPolyPoly, aClipPoly);
                }
            }

            
            if( mbScaleIsotrophically )
            {
                const double nScale( ::std::max( rTargetSize.getX(),
                                                 rTargetSize.getY() ) );
                aMatrix.scale( nScale, nScale );
                aMatrix.translate( -(nScale-rTargetSize.getX())/2.0,
                                   -(nScale-rTargetSize.getY())/2.0 );
            }
            else
            {
                aMatrix.scale( rTargetSize.getX(),
                               rTargetSize.getY() );
            }

            
            aClipPoly.transform( aMatrix );

            return aClipPoly;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
