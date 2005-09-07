/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wakeupevent.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:33:46 $
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
#include <canvas/verbosetrace.hxx>

#include <wakeupevent.hxx>


namespace presentation
{
    namespace internal
    {
        WakeupEvent::WakeupEvent(
            boost::shared_ptr<canvas::tools::ElapsedTime> const & pTimeBase,
            ActivitiesQueue& rActivityQueue ) :
            maTimer(pTimeBase),
            mnNextTime(0.0),
            mpActivity(),
            mrActivityQueue( rActivityQueue )
        {
        }

        void WakeupEvent::dispose()
        {
            mpActivity.reset();
        }

        bool WakeupEvent::fire()
        {
            if( !mpActivity.get() )
                return false;

            return mrActivityQueue.addActivity( mpActivity );
        }

        bool WakeupEvent::isCharged() const
        {
            // this event won't expire, we fire everytime we're
            // re-inserted into the event queue.
            return true;
        }

        double WakeupEvent::getActivationTime( double nCurrentTime ) const
        {
            const double nElapsedTime( maTimer.getElapsedTime() );

            return ::std::max( nCurrentTime,
                               nCurrentTime - nElapsedTime + mnNextTime );
        }

        void WakeupEvent::start()
        {
            // start timer
            maTimer.reset();
        }

        void WakeupEvent::setNextTimeout( double rNextTime )
        {
            mnNextTime = rNextTime;
        }

        void WakeupEvent::setActivity( const ActivitySharedPtr& rActivity )
        {
            mpActivity = rActivity;
        }
    }
}
