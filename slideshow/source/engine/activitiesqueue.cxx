/*************************************************************************
 *
 *  $RCSfile: activitiesqueue.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 18:45:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// must be first
#include <canvas/debug.hxx>

#ifndef _CANVAS_VERBOSETRACE_HXX
#include <canvas/verbosetrace.hxx>
#endif

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif
#ifndef BOOST_MEM_FN_HPP_INCLUDED
#include <boost/mem_fn.hpp>
#endif

#include <algorithm>

#include <slideshowexceptions.hxx>
#include <activity.hxx>
#include <activitiesqueue.hxx>


using namespace ::drafts::com::sun::star;
using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        ActivitiesQueue::ActivitiesQueue( const UnoViewContainer& rViews ) :
            mpLayerManager(),
            maCurrentActivitiesWaiting(),
            maCurrentActivitiesReinsert(),
            mrViews( rViews ),
            mbCurrentRoundNeedsScreenUpdate( false )
        {
        }

        ActivitiesQueue::~ActivitiesQueue()
        {
            // dispose all queues
            ::std::for_each( maCurrentActivitiesWaiting.begin(),
                             maCurrentActivitiesWaiting.end(),
                             ::boost::mem_fn(&Disposable::dispose) );
            ::std::for_each( maCurrentActivitiesReinsert.begin(),
                             maCurrentActivitiesReinsert.end(),
                             ::boost::mem_fn(&Disposable::dispose) );
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

            bool bPerformScreenUpdate( false );

            // was:
            // if( !maCurrentActivitiesWaiting.empty() )
            // for low-prio activities

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

                VERBOSE_TRACE( "ActivitiesQueue: inner loop heartbeat" );
            }

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
            if( bPerformScreenUpdate ||
                (mpLayerManager.get() &&
                 mpLayerManager->isUpdatePending() ) )
            {
                // call update() on the registered
                // LayerManager. This will only update the
                // backbuffer, not flush anything to screen
                if( mpLayerManager.get() )
                    mpLayerManager->update();

                // call updateScreen() on all registered views (which
                // will copy the backbuffers to the front). Do NOT use
                // LayerManager::updateScreen(), we might need screen
                // updates independent from a valid LayerManager.
                ::std::for_each( mrViews.begin(),
                                 mrViews.end(),
                                 ::boost::mem_fn( &View::updateScreen ) );

                VERBOSE_TRACE( "ActivitiesQueue: update done" );
            }
        }

        bool ActivitiesQueue::isEmpty()
        {
            return maCurrentActivitiesWaiting.empty() && maCurrentActivitiesReinsert.empty();
        }

        void ActivitiesQueue::setLayerManager( const LayerManagerSharedPtr& rMgr )
        {
            mpLayerManager = rMgr;
        }

        void ActivitiesQueue::clear()
        {
            ActivityQueue aTmp0;
            maCurrentActivitiesWaiting.swap( aTmp0 );

            ActivityQueue aTmp1;
            maCurrentActivitiesReinsert.swap( aTmp1 );

            mbCurrentRoundNeedsScreenUpdate = false;
        }
    }
}
