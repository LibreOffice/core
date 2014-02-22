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

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include "slideshowexceptions.hxx"
#include "activity.hxx"
#include "activitiesqueue.hxx"

#include <boost/mem_fn.hpp>
#include <boost/shared_ptr.hpp>
#include <algorithm>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        ActivitiesQueue::ActivitiesQueue(
          const ::boost::shared_ptr< ::canvas::tools::ElapsedTime >& pPresTimer ) :
            mpTimer( pPresTimer ),
            maCurrentActivitiesWaiting(),
            maCurrentActivitiesReinsert(),
            maDequeuedActivities()
        {
        }

        ActivitiesQueue::~ActivitiesQueue()
        {
            
            try
            {
                std::for_each( maCurrentActivitiesWaiting.begin(),
                               maCurrentActivitiesWaiting.end(),
                               boost::mem_fn( &Disposable::dispose ) );
                std::for_each( maCurrentActivitiesReinsert.begin(),
                               maCurrentActivitiesReinsert.end(),
                               boost::mem_fn( &Disposable::dispose ) );
            }
            catch (uno::Exception &)
            {
                OSL_FAIL( OUStringToOString(
                                comphelper::anyToString(
                                    cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }

        bool ActivitiesQueue::addActivity( const ActivitySharedPtr& pActivity )
        {
            OSL_ENSURE( pActivity, "ActivitiesQueue::addActivity: activity ptr NULL" );

            if( !pActivity )
                return false;

            
            maCurrentActivitiesWaiting.push_back( pActivity );

            return true;
        }

        void ActivitiesQueue::process()
        {
            VERBOSE_TRACE( "ActivitiesQueue: outer loop heartbeat" );

            
            
            ActivityQueue::const_iterator iPos(
                maCurrentActivitiesWaiting.begin() );
            const ActivityQueue::const_iterator iEnd(
                maCurrentActivitiesWaiting.end() );
            double fLag = 0.0;
            for ( ; iPos != iEnd; ++iPos )
                fLag = std::max<double>( fLag, (*iPos)->calcTimeLag() );
            if (fLag > 0.0)
            {
                mpTimer->adjustTimer( -fLag );
            }

            
            while( !maCurrentActivitiesWaiting.empty() )
            {
                
                ActivitySharedPtr pActivity( maCurrentActivitiesWaiting.front() );
                maCurrentActivitiesWaiting.pop_front();

                bool bReinsert( false );

                try
                {
                    
                    bReinsert = pActivity->perform();
                }
                catch( uno::RuntimeException& )
                {
                    throw;
                }
                catch( uno::Exception& )
                {
                    
                    
                    
                    
                    

                    
                    
                    
                    
                    OSL_FAIL( OUStringToOString(
                                    comphelper::anyToString( cppu::getCaughtException() ),
                                    RTL_TEXTENCODING_UTF8 ).getStr() );
                }
                catch( SlideShowException& )
                {
                    
                    
                    
                    
                    

                    
                    
                    
                    
                    OSL_TRACE( "::presentation::internal::ActivitiesQueue: Activity threw a SlideShowException, removing from ring" );
                }

                if( bReinsert )
                    maCurrentActivitiesReinsert.push_back( pActivity );
                else
                    maDequeuedActivities.push_back( pActivity );

                VERBOSE_TRACE( "ActivitiesQueue: inner loop heartbeat" );
            }

            if( !maCurrentActivitiesReinsert.empty() )
            {
                
                
                
                
                
                maCurrentActivitiesWaiting.swap( maCurrentActivitiesReinsert );
            }
        }

        void ActivitiesQueue::processDequeued()
        {
            
            ::std::for_each( maDequeuedActivities.begin(),
                             maDequeuedActivities.end(),
                             ::boost::mem_fn( &Activity::dequeued ) );
            maDequeuedActivities.clear();
        }

        bool ActivitiesQueue::isEmpty() const
        {
            return maCurrentActivitiesWaiting.empty() && maCurrentActivitiesReinsert.empty();
        }

        void ActivitiesQueue::clear()
        {
            
            std::for_each( maCurrentActivitiesWaiting.begin(),
                           maCurrentActivitiesWaiting.end(),
                           boost::mem_fn( &Activity::dequeued ) );
            ActivityQueue().swap( maCurrentActivitiesWaiting );

            std::for_each( maCurrentActivitiesReinsert.begin(),
                           maCurrentActivitiesReinsert.end(),
                           boost::mem_fn( &Activity::dequeued ) );
            ActivityQueue().swap( maCurrentActivitiesReinsert );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
