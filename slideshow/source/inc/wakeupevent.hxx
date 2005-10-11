/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wakeupevent.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2005-10-11 08:55:49 $
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
#if ! defined(INCLUDED_SLIDESHOW_WAKEUPEVENT_HXX)
#define INCLUDED_SLIDESHOW_WAKEUPEVENT_HXX

#include "event.hxx"
#include "activitiesqueue.hxx"
#include "canvas/elapsedtime.hxx"
#include "boost/bind.hpp"
#include "boost/noncopyable.hpp"

namespace presentation {
namespace internal {

/** Little helper class, used to set Activities active again
    after some sleep period.

    Clients can use this class to schedule wakeup events at
    the EventQueue, to avoid busy-waiting for the next
    discrete time instant.
*/
class WakeupEvent : public Event,
                    private ::boost::noncopyable
{
public:
    WakeupEvent(
        ::boost::shared_ptr< ::canvas::tools::ElapsedTime > const& pTimeBase,
        ActivitiesQueue & rActivityQueue );

    virtual void dispose();
    virtual bool fire();
    virtual bool isCharged() const;
    virtual double getActivationTime( double nCurrentTime ) const;

    /// Start the internal timer
    void start();

    /** Set the next timeout this object should generate.

        @param nextTime
        Absolute time, measured from the last start() call,
        when this event should wakeup the Activity again. If
        your time is relative, simply call start() just before
        every setNextTimeout() call.
    */
    void setNextTimeout( double nextTime );

    /** Set activity to wakeup.

        The activity given here will be reinserted into the
        ActivitiesQueue, once the timeout is reached.
    */
    void setActivity( const ActivitySharedPtr& rActivity );

private:
    ::canvas::tools::ElapsedTime    maTimer;
    double                          mnNextTime;
    ActivitySharedPtr               mpActivity;
    ActivitiesQueue&                mrActivityQueue;
};

typedef ::boost::shared_ptr< WakeupEvent > WakeupEventSharedPtr;

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_WAKEUPEVENT_HXX */
