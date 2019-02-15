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


#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <slideshowexceptions.hxx>
#include <activity.hxx>
#include <activitiesqueue.hxx>

#include <algorithm>
#include <memory>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        ActivitiesQueue::ActivitiesQueue(
          const std::shared_ptr< ::canvas::tools::ElapsedTime >& pPresTimer ) :
            mpTimer( pPresTimer ),
            maCurrentActivitiesWaiting(),
            maCurrentActivitiesReinsert(),
            maDequeuedActivities()
        {
        }

        ActivitiesQueue::~ActivitiesQueue()
        {
            // dispose all queue entries
            try
            {
                for( const auto& pActivity : maCurrentActivitiesWaiting )
                    pActivity->dispose();
                for( const auto& pActivity : maCurrentActivitiesReinsert )
                    pActivity->dispose();
            }
            catch (const uno::Exception& e)
            {
                SAL_WARN("slideshow", e);
            }
        }

        bool ActivitiesQueue::addActivity( const ActivitySharedPtr& pActivity )
        {
            OSL_ENSURE( pActivity, "ActivitiesQueue::addActivity: activity ptr NULL" );

            if( !pActivity )
                return false;

            // add entry to waiting list
            maCurrentActivitiesWaiting.push_back( pActivity );

            return true;
        }

        void ActivitiesQueue::process()
        {
            SAL_INFO("slideshow.verbose", "ActivitiesQueue: outer loop heartbeat" );

            // accumulate time lag for all activities, and lag time
            // base if necessary:
            double fLag = 0.0;
            for ( const auto& rxActivity : maCurrentActivitiesWaiting )
                fLag = std::max<double>( fLag, rxActivity->calcTimeLag() );
            if (fLag > 0.0)
            {
                mpTimer->adjustTimer( -fLag );
            }

            // process list of activities
            while( !maCurrentActivitiesWaiting.empty() )
            {
                // process topmost activity
                ActivitySharedPtr pActivity( maCurrentActivitiesWaiting.front() );
                maCurrentActivitiesWaiting.pop_front();

                bool bReinsert( false );

                try
                {
                    // fire up activity
                    bReinsert = pActivity->perform();
                }
                catch( uno::RuntimeException& )
                {
                    throw;
                }
                catch( uno::Exception& )
                {
                    // catch anything here, we don't want
                    // to leave this scope under _any_
                    // circumstance. Although, do _not_
                    // reinsert an activity that threw
                    // once.

                    // NOTE: we explicitly don't catch(...) here,
                    // since this will also capture segmentation
                    // violations and the like. In such a case, we
                    // still better let our clients now...
                    SAL_WARN( "slideshow", exceptionToString(cppu::getCaughtException()) );
                }
                catch( SlideShowException& )
                {
                    // catch anything here, we don't want
                    // to leave this scope under _any_
                    // circumstance. Although, do _not_
                    // reinsert an activity that threw
                    // once.

                    // NOTE: we explicitly don't catch(...) here,
                    // since this will also capture segmentation
                    // violations and the like. In such a case, we
                    // still better let our clients now...
                    SAL_WARN("slideshow", "::presentation::internal::ActivitiesQueue: Activity threw a SlideShowException, removing from ring" );
                }

                if( bReinsert )
                    maCurrentActivitiesReinsert.push_back( pActivity );
                else
                    maDequeuedActivities.push_back( pActivity );

                SAL_INFO("slideshow.verbose", "ActivitiesQueue: inner loop heartbeat" );
            }

            if( !maCurrentActivitiesReinsert.empty() )
            {
                // reinsert all processed, but not finished
                // activities back to waiting queue. With swap(),
                // we kill two birds with one stone: we reuse the
                // list nodes, and we clear the
                // maCurrentActivitiesReinsert list
                maCurrentActivitiesWaiting.swap( maCurrentActivitiesReinsert );
            }
        }

        void ActivitiesQueue::processDequeued()
        {
            // notify all dequeued activities from last round
            for( const auto& pActivity : maDequeuedActivities )
                pActivity->dequeued();
            maDequeuedActivities.clear();
        }

        bool ActivitiesQueue::isEmpty() const
        {
            return maCurrentActivitiesWaiting.empty() && maCurrentActivitiesReinsert.empty();
        }

        void ActivitiesQueue::clear()
        {
            // dequeue all entries:
            for( const auto& pActivity : maCurrentActivitiesWaiting )
                pActivity->dequeued();
            ActivityQueue().swap( maCurrentActivitiesWaiting );

            for( const auto& pActivity : maCurrentActivitiesReinsert )
                pActivity->dequeued();
            ActivityQueue().swap( maCurrentActivitiesReinsert );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
