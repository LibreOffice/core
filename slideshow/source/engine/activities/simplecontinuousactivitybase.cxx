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
#include <canvas/verbosetrace.hxx>

#include <simplecontinuousactivitybase.hxx>


namespace slideshow
{
    namespace internal
    {
        SimpleContinuousActivityBase::SimpleContinuousActivityBase(
            const ActivityParameters& rParms ) :
            ActivityBase( rParms ),
            maTimer( rParms.mrActivitiesQueue.getTimer() ),
            mnMinSimpleDuration( rParms.mnMinDuration ),
            mnMinNumberOfFrames( rParms.mnMinNumberOfFrames ),
            mnCurrPerformCalls( 0 )
        {
        }

        void SimpleContinuousActivityBase::startAnimation()
        {
            
            
            maTimer.reset();
        }

        double SimpleContinuousActivityBase::calcTimeLag() const
        {
            ActivityBase::calcTimeLag();
            if (! isActive())
                return 0.0;

            
            const double nCurrElapsedTime( maTimer.getElapsedTime() );

            
            VERBOSE_TRACE( "SimpleContinuousActivityBase::calcTimeLag(): "
                           "next step is based on time: %f", nCurrElapsedTime );

            
            
            
            
            
            

            
            const double nFractionElapsedTime(
                nCurrElapsedTime / mnMinSimpleDuration );

            
            const double nFractionRequiredCalls(
                double(mnCurrPerformCalls) / mnMinNumberOfFrames );

            
            //
            
            
            
            
            //
            
            
            
            
            
            
            
            if( nFractionElapsedTime < nFractionRequiredCalls )
            {
                VERBOSE_TRACE( "SimpleContinuousActivityBase::calcTimeLag(): "
                               "t=%f is based on time", nFractionElapsedTime );
                return 0.0;
            }
            else
            {
                VERBOSE_TRACE( "SimpleContinuousActivityBase::perform(): "
                               "t=%f is based on number of calls",
                               nFractionRequiredCalls );

                
                return ((nFractionElapsedTime - nFractionRequiredCalls)
                        * mnMinSimpleDuration);
            }
        }

        bool SimpleContinuousActivityBase::perform()
        {
            
            if( !ActivityBase::perform() )
                return false; 


            
            

            const double nCurrElapsedTime( maTimer.getElapsedTime() );
            double nT( nCurrElapsedTime / mnMinSimpleDuration );


            
            

            
            
            bool bActivityEnding( false );

            if( isRepeatCountValid() )
            {
                
                

                
                
                const double nRepeatCount( getRepeatCount() );
                const double nEffectiveRepeat( isAutoReverse() ?
                                               2.0*nRepeatCount :
                                               nRepeatCount );

                
                if( nEffectiveRepeat <= nT )
                {
                    
                    
                    
                    bActivityEnding = true;

                    
                    nT = nEffectiveRepeat;
                }
            }


            
            

            double nRepeats;
            double nRelativeSimpleTime;

            
            if( isAutoReverse() )
            {
                
                
                const double nFractionalActiveDuration( modf(nT, &nRepeats) );

                
                
                if( ((int)nRepeats) % 2 )
                {
                    
                    nRelativeSimpleTime = 1.0 - nFractionalActiveDuration;
                }
                else
                {
                    
                    nRelativeSimpleTime = nFractionalActiveDuration;
                }

                
                
                
                nRepeats /= 2;
            }
            else
            {
                
                

                
                
                
                
                nRelativeSimpleTime = modf(nT, &nRepeats);

                
                if( isRepeatCountValid() &&
                    nRepeats >= getRepeatCount() )
                {
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    

                    
                    
                    
                    //
                    
                    
                    
                    
                    nRelativeSimpleTime = 1.0;
                    nRepeats -= 1.0;
                }
            }

            
            

            simplePerform( nRelativeSimpleTime,
                           
                           static_cast<sal_uInt32>( nRepeats ) );


            
            
            
            
            
            if( bActivityEnding )
                endActivity();

            
            ++mnCurrPerformCalls;

            return isActive();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
