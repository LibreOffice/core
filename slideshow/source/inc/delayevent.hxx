/*************************************************************************
 *
 *  $RCSfile: delayevent.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:14:29 $
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
                if( !wasFired() )
                {
                    mbWasFired = true;

                    maFunctor();
                }

                return true;
            }

            virtual bool wasFired() const
            {
                return mbWasFired;
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

        template< typename Functor > EventSharedPtr makeDelay( const Functor&   rFunctor,
                                                               double           nTimeout    )
        {
            return EventSharedPtr( new Delay< Functor >(rFunctor, nTimeout) );
        }

        template< typename Functor > EventSharedPtr makeEvent( const Functor&   rFunctor )
        {
            return EventSharedPtr( new Delay< Functor >(rFunctor, 0.0) );
        }
    }
}

#endif /* _SLIDESHOW_DELAYEVENT_HXX */
