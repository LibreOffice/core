/*************************************************************************
 *
 *  $RCSfile: activitiesqueue.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2004-03-18 10:44:24 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CANVAS_VERBOSETRACE_HXX
#include <canvas/verbosetrace.hxx>
#endif

#include "activity.hxx"
#include "activitiesqueue.hxx"

using namespace ::drafts::com::sun::star;
using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        ActivitiesQueue::ActivitiesQueue( const ::cppcanvas::SpriteCanvasSharedPtr& rDisplayCanvas ) :
            mpSpriteCanvas( rDisplayCanvas ),
            maCurrentActivitiesWaiting(),
            maCurrentActivitiesReinsert(),
            mbCurrentRoundNeedsScreenUpdate( false )
        {
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

            // process list of activities
            if( !maCurrentActivitiesWaiting.empty() )
            {
                // process topmost activity
                ActivitySharedPtr pActivity( maCurrentActivitiesWaiting.front() );
                maCurrentActivitiesWaiting.pop_front();

                bool bReinsert( false );

                try
                {
                    // fire up activity
                    bReinsert = pActivity->perform();

                    OSL_ENSURE( bReinsert == pActivity->isActive(),
                                "::presentation::internal::ActivitiesQueue: Inconsistent Activity state" );
                }
                catch(...)
                {
                    // catch anything here, we don't want
                    // to leave this scope under _any_
                    // circumstance. Although, do _not_
                    // reinsert an activity that threw
                    // once.
                    OSL_TRACE( "::presentation::internal::ActivitiesQueue: Activity threw, removing from ring" );
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
                if( mbCurrentRoundNeedsScreenUpdate &&
                    mpSpriteCanvas.get() != NULL )
                {
                    // flush rendered content to screen, in a
                    // controlled, atomic update operation
                    mpSpriteCanvas->updateScreen();
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
        }

        bool ActivitiesQueue::isEmpty()
        {
            return maCurrentActivitiesWaiting.empty() && maCurrentActivitiesReinsert.empty();
        }

    }
}
