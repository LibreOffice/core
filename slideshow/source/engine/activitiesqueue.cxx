/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: activitiesqueue.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:22:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// must be first
#include <canvas/debug.hxx>
#ifndef _CANVAS_VERBOSETRACE_HXX
#include <canvas/verbosetrace.hxx>
#endif
#include <comphelper/scopeguard.hxx>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <slideshowexceptions.hxx>
#include <activity.hxx>
#include <activitiesqueue.hxx>

#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <algorithm>


using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        ActivitiesQueue::ActivitiesQueue( const ::boost::shared_ptr< ::canvas::tools::ElapsedTime >&    pPresTimer,
                                          EventMultiplexer&                                             rEventMultiplexer ) :
            mpTimer( pPresTimer ),
            maCurrentActivitiesWaiting(),
            maCurrentActivitiesReinsert(),
            mrEventMultiplexer( rEventMultiplexer ),
            mbCurrentRoundNeedsScreenUpdate( false )
        {
        }

        ActivitiesQueue::~ActivitiesQueue()
        {
            // dispose all queue entries
            try {
                std::for_each( maCurrentActivitiesWaiting.begin(),
                               maCurrentActivitiesWaiting.end(),
                               boost::mem_fn( &Disposable::dispose ) );
                std::for_each( maCurrentActivitiesReinsert.begin(),
                               maCurrentActivitiesReinsert.end(),
                               boost::mem_fn( &Disposable::dispose ) );
            }
            catch (uno::Exception &) {
                OSL_ENSURE( false, rtl::OUStringToOString(
                                comphelper::anyToString(
                                    cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }

        bool ActivitiesQueue::addActivity( const ActivitySharedPtr& pActivity )
        {
            OSL_ENSURE( pActivity.get() != NULL, "ActivitiesQueue::addActivity: activity ptr NULL" );

            if( pActivity.get() == NULL )
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
            if (fLag > 0.0) {
                mpTimer->adjustTimer( -fLag );
            }

            // This list collects all activities which did not request
            // a reinsertion. After the screen update has been
            // performed, those are notified via dequeued(). This
            // facilitates cleanup actions taking place _after_ the
            // current frame has been displayed.
            ActivityQueue aDequeuedActivities;

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
                    OSL_TRACE( "::presentation::internal::ActivitiesQueue: Activity threw a uno::Exception, removing from ring" );
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

                // always query need for screen updates. Note that
                // ending activities (i.e. those that return
                // bReinsert=false) might also need one last screen
                // update
                if( pActivity->needsScreenUpdate() )
                    mbCurrentRoundNeedsScreenUpdate = true;

                if( bReinsert )
                    maCurrentActivitiesReinsert.push_back( pActivity );
                else
                    aDequeuedActivities.push_back( pActivity );

                VERBOSE_TRACE( "ActivitiesQueue: inner loop heartbeat" );
            }

            // when true, the code below has determined that a screen
            // update is necessary.
            bool bPerformScreenUpdate( false );

            // waiting activities exhausted? Then update screen, and
            // reinsert
            if( maCurrentActivitiesWaiting.empty() )
            {
                if( mbCurrentRoundNeedsScreenUpdate )
                {
                    bPerformScreenUpdate = true;
                }

                // always clear update flag. There's no need to update
                // yesterday's display, even if the canvas sometimes
                // become valid
                mbCurrentRoundNeedsScreenUpdate = false;

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

            // perform screen update (not only if one of the
            // activities requested that, but also if the layer
            // manager signals that it needs one. This frees us from
            // introducing dummy activities, just to trigger screen
            // updates. OTOH, this makes it necessary to always call
            // BOTH event queue and activities queue, such that no
            // pending update is unduly delayed)
            mrEventMultiplexer.updateScreenContent( bPerformScreenUpdate );

            // notify all dequeued activities, but only _after_ the
            // screen update.
            ::std::for_each( aDequeuedActivities.begin(),
                             aDequeuedActivities.end(),
                             ::boost::mem_fn( &Activity::dequeued ) );
        }

        bool ActivitiesQueue::isEmpty()
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

            mbCurrentRoundNeedsScreenUpdate = false;
        }
    }
}
