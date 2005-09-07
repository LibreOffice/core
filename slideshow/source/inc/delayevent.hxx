/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: delayevent.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:08:40 $
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

#ifndef _SLIDESHOW_DELAYEVENT_HXX
#define _SLIDESHOW_DELAYEVENT_HXX

#include <event.hxx>

namespace presentation
{
    namespace internal
    {
        /** Event, which delays the functor call the given amount of time

            @tpl Functor
            Functor to call after given timeout

            @attention
            This class might generate circular dependencies, since it
            has no means to release shared_ptrs maybe contained in the
            Functor
        */
        template< typename Functor > class Delay : public Event
        {
        public:
            Delay( const Functor&   rFunctor,
                   double           nTimeout    ) :
                maFunctor( rFunctor ),
                mnTimeout( nTimeout ),
                mbWasFired( false )
            {
            }

            virtual bool fire()
            {
                if( isCharged() )
                {
                    mbWasFired = true;

                    maFunctor();
                }

                return true;
            }

            virtual bool isCharged() const
            {
                return !mbWasFired;
            }

            virtual double getActivationTime( double nCurrentTime ) const
            {
                return nCurrentTime + mnTimeout;
            }

            virtual void dispose()
            {
            }

        private:
            Functor maFunctor;
            double  mnTimeout;
            bool    mbWasFired;
        };

        /** Generate delay event

            @param rFunctor
            Functor to call when the event fires.

            @param nTimeout
            Timeout in seconds, to wait until functor is called.

            @return generated delay event
         */
        template< typename Functor > EventSharedPtr makeDelay( const Functor&   rFunctor,
                                                               double           nTimeout    )
        {
            return EventSharedPtr( new Delay< Functor >(rFunctor, nTimeout) );
        }

        /** Generate immediate event

            @param rFunctor
            Functor to call when the event fires.

            @return generated immediate event.
         */
        template< typename Functor > EventSharedPtr makeEvent( const Functor&   rFunctor )
        {
            return EventSharedPtr( new Delay< Functor >(rFunctor, 0.0) );
        }
    }
}

#endif /* _SLIDESHOW_DELAYEVENT_HXX */
