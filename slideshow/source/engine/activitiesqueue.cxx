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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include "slideshowexceptions.hxx"
#include "activity.hxx"
#include "activitiesqueue.hxx"

#include <boost/bind.hpp>
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
            // dispose all queue entries
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
                OSL_FAIL( rtl::OUStringToOString(
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

            // add entry to waiting list
            maCurrentActivitiesWaiting.push_back( pActivity );

            return true;
        }

        void ActivitiesQueue::process()
        {
            VERBOSE_TRACE( "ActivitiesQueue: outer loop heartbeat" );

            // accumulate time lag for all activities, and lag time
            // base if necessary:
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

                    // NOTE: we explicitely don't catch(...) here,
                    // since this will also capture segmentation
                    // violations and the like. In such a case, we
                    // still better let our clients now...
                    OSL_FAIL( rtl::OUStringToOString(
                                    comphelper::anyToString( cppu::getCaughtException() ),
                                    RTL_TEXTENCODING_UTF8 ).getStr() );
                }
                catch( SlideShowException& )
                {
                    // catch anything here, we don't want
                    // to leave this scope under _any_
                    // circumstance. Although, do _not_
                    // reinsert an activity that threw
                    // once.

                    // NOTE: we explicitely don't catch(...) here,
                    // since this will also capture segmentation
                    // violations and the like. In such a case, we
                    // still better let our clients now...
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
            // dequeue all entries:
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
