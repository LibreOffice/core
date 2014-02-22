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
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <activitybase.hxx>


namespace slideshow
{
    namespace internal
    {
        
        

        ActivityBase::ActivityBase( const ActivityParameters& rParms ) :
            mpEndEvent( rParms.mrEndEvent ),
            mrEventQueue( rParms.mrEventQueue ),
            mpShape(),
            mpAttributeLayer(),
            maRepeats( rParms.mrRepeats ),
            mnAccelerationFraction( rParms.mnAccelerationFraction ),
            mnDecelerationFraction( rParms.mnDecelerationFraction ),
            mbAutoReverse( rParms.mbAutoReverse ),
            mbFirstPerformCall( true ),
            mbIsActive( true ) {}

        void ActivityBase::dispose()
        {
            
            mbIsActive = false;

            
            if( mpEndEvent )
                mpEndEvent->dispose();

            
            mpEndEvent.reset();
            mpShape.reset();
            mpAttributeLayer.reset();
        }

        double ActivityBase::calcTimeLag() const
        {
            
            if (isActive() && mbFirstPerformCall)
            {
                mbFirstPerformCall = false;

                
                
                const_cast<ActivityBase *>(this)->startAnimation();
            }
            return 0.0;
        }

        bool ActivityBase::perform()
        {
            
            if( !isActive() )
                return false; 

            OSL_ASSERT( ! mbFirstPerformCall );

            return true;
        }

        bool ActivityBase::isActive() const
        {
            return mbIsActive;
        }

        void ActivityBase::setTargets( const AnimatableShapeSharedPtr&      rShape,
                                       const ShapeAttributeLayerSharedPtr&  rAttrLayer )
        {
            ENSURE_OR_THROW( rShape,
                              "ActivityBase::setTargets(): Invalid shape" );
            ENSURE_OR_THROW( rAttrLayer,
                              "ActivityBase::setTargets(): Invalid attribute layer" );

            mpShape = rShape;
            mpAttributeLayer = rAttrLayer;
        }

        void ActivityBase::endActivity()
        {
            
            mbIsActive = false;

            
            if( mpEndEvent )
                mrEventQueue.addEvent( mpEndEvent );

            
            mpEndEvent.reset();
        }

        void ActivityBase::dequeued()
        {
            




            if( !isActive() )
                endAnimation();
        }

        void ActivityBase::end()
        {
            if (!isActive() || isDisposed())
                return;
            
            if (mbFirstPerformCall) {
                mbFirstPerformCall = false;
                
                this->startAnimation();
            }

            performEnd(); 
            endAnimation();
            endActivity();
        }

        double ActivityBase::calcAcceleratedTime( double nT ) const
        {
            
            

            
            nT = ::basegfx::clamp( nT, 0.0, 1.0 );

            
            
            
            if( (mnAccelerationFraction > 0.0 ||
                 mnDecelerationFraction > 0.0) &&
                mnAccelerationFraction + mnDecelerationFraction <= 1.0 )
            {
                /*
                
                //
                
                
                
                
                
                //
                
                
                
                
                
                //
                
                
                
                
                //
                
                //
                
                //
                
                //
                
                
                
                //
                
                //
                
                
                
                
                
                
                
                //
                
                const double nC( 1.0 - 0.5*mnAccelerationFraction - 0.5*mnDecelerationFraction );

                
                double nTPrime(0.0);

                if( nT < mnAccelerationFraction )
                {
                    nTPrime += 0.5*nT*nT/mnAccelerationFraction; 
                }
                else
                {
                    nTPrime += 0.5*mnAccelerationFraction; 

                    if( nT <= 1.0-mnDecelerationFraction )
                    {
                        nTPrime += nT-mnAccelerationFraction; 
                    }
                    else
                    {
                        nTPrime += 1.0 - mnAccelerationFraction - mnDecelerationFraction; 

                        const double nTRelative( nT - 1.0 + mnDecelerationFraction );

                        nTPrime += nTRelative - 0.5*nTRelative*nTRelative / mnDecelerationFraction;
                    }
                }

                
                nT = nTPrime / nC;
            }

            return nT;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
