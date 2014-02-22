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

#include <discreteactivitybase.hxx>


namespace slideshow
{
    namespace internal
    {
        DiscreteActivityBase::DiscreteActivityBase( const ActivityParameters& rParms ) :
            ActivityBase( rParms ),
            mpWakeupEvent( rParms.mpWakeupEvent ),
            maDiscreteTimes( rParms.maDiscreteTimes ),
            mnSimpleDuration( rParms.mnMinDuration ),
            mnCurrPerformCalls( 0 )
        {
            ENSURE_OR_THROW( mpWakeupEvent,
                              "DiscreteActivityBase::DiscreteActivityBase(): Invalid wakeup event" );

            ENSURE_OR_THROW( !maDiscreteTimes.empty(),
                              "DiscreteActivityBase::DiscreteActivityBase(): time vector is empty, why do you create me?" );

#ifdef DBG_UTIL
            
            
            
            for( ::std::size_t i=1, len=maDiscreteTimes.size(); i<len; ++i )
            {
                if( maDiscreteTimes[i] < 0.0 ||
                    maDiscreteTimes[i] > 1.0 ||
                    maDiscreteTimes[i-1] < 0.0 ||
                    maDiscreteTimes[i-1] > 1.0 )
                {
                    ENSURE_OR_THROW( false, "DiscreteActivityBase::DiscreteActivityBase(): time values not within [0,1] range!" );
                }

                if( maDiscreteTimes[i-1] > maDiscreteTimes[i] )
                    ENSURE_OR_THROW( false, "DiscreteActivityBase::DiscreteActivityBase(): time vector is not sorted in ascending order!" );
            }

            
#endif
        }

        void DiscreteActivityBase::startAnimation()
        {
            
            mpWakeupEvent->start();
        }

        sal_uInt32 DiscreteActivityBase::calcFrameIndex( sal_uInt32     nCurrCalls,
                                                         ::std::size_t  nVectorSize ) const
        {
            if( isAutoReverse() )
            {
                
                
                sal_uInt32 nFrameIndex( nCurrCalls % (2*nVectorSize) );

                
                
                if( nFrameIndex >= nVectorSize )
                    nFrameIndex = 2*nVectorSize - nFrameIndex; 

                return nFrameIndex;
            }
            else
            {
                return nCurrCalls % nVectorSize ;
            }
        }

        sal_uInt32 DiscreteActivityBase::calcRepeatCount( sal_uInt32    nCurrCalls,
                                                          ::std::size_t nVectorSize ) const
        {
            if( isAutoReverse() )
                return nCurrCalls / (2*nVectorSize); 
            else
                return nCurrCalls / nVectorSize;
        }

        bool DiscreteActivityBase::perform()
        {
            
            if( !ActivityBase::perform() )
                return false; 

            const ::std::size_t nVectorSize( maDiscreteTimes.size() );

            
            

            

            
            
            perform( calcFrameIndex( mnCurrPerformCalls, nVectorSize ),
                     calcRepeatCount( mnCurrPerformCalls, nVectorSize ) );

            
            ++mnCurrPerformCalls;

            
            double nCurrRepeat( double(mnCurrPerformCalls) / nVectorSize );

            
            
            
            if( isAutoReverse() )
                nCurrRepeat /= 2.0;

            
            
            
            if( !isRepeatCountValid() ||
                nCurrRepeat < getRepeatCount() )
            {
                
                

                
                
                
                //
                
                
                //
                
                //
                
                //
                
                
                //
                mpWakeupEvent->setNextTimeout(
                    mnSimpleDuration*(
                        calcRepeatCount(
                            mnCurrPerformCalls,
                            nVectorSize ) +
                        calcAcceleratedTime(
                            maDiscreteTimes[
                                calcFrameIndex(
                                    mnCurrPerformCalls,
                                    nVectorSize ) ] ) ) );

                getEventQueue().addEvent( mpWakeupEvent );
            }
            else
            {
                
                
                mpWakeupEvent.reset();

                
                endActivity();
            }

            return false; 
        }

        void DiscreteActivityBase::dispose()
        {
            
            if( mpWakeupEvent )
                mpWakeupEvent->dispose();

            
            mpWakeupEvent.reset();

            ActivityBase::dispose();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
