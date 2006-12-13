/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slideshowcontext.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:03:32 $
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

#ifndef _SLIDESHOW_SLIDESHOWCONTEXT_HXX
#define _SLIDESHOW_SLIDESHOWCONTEXT_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "layermanager.hxx"
#include "eventqueue.hxx"
#include "activitiesqueue.hxx"
#include "usereventqueue.hxx"
#include "eventmultiplexer.hxx"
#include "unoviewcontainer.hxx"


namespace slideshow
{
    namespace internal
    {
        /** Common arguments for slideshow objects.

            This struct combines a number of object references
            ubiquituously needed throughout the slideshow.
         */
        struct SlideShowContext
        {
            /** Common context for node creation

                @param rLayerManager
                Layer manager, which holds all shapes

                @param rEventQueue
                Event queue, where time-based events are to be
                scheduled. A node must not schedule events there
                before it's not resolved.

                @param rActivitiesQueue
                Activities queue, where repeating activities are
                to be scheduled.

                @param rUserEventQueue
                User event queue
            */
            SlideShowContext( const LayerManagerSharedPtr&                      rLayerManager,
                              EventQueue&                                       rEventQueue,
                              EventMultiplexer&                                 rEventMultiplexer,
                              ActivitiesQueue&                                  rActivitiesQueue,
                              UserEventQueue&                                   rUserEventQueue,
                              const UnoViewContainer&                           rViewContainer,
                              const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::uno::XComponentContext>&  rComponentContext ) :
                mpLayerManager( rLayerManager ),
                mrEventQueue( rEventQueue ),
                mrEventMultiplexer( rEventMultiplexer ),
                mrActivitiesQueue( rActivitiesQueue ),
                mrUserEventQueue( rUserEventQueue ),
                mrViewContainer( rViewContainer ),
                mxComponentContext( rComponentContext )
            {
            }

            void dispose()
            {
                mxComponentContext.clear();
                mpLayerManager.reset();
            }

            LayerManagerSharedPtr                           mpLayerManager;
            EventQueue&                                     mrEventQueue;
            EventMultiplexer&                               mrEventMultiplexer;
            ActivitiesQueue&                                mrActivitiesQueue;
            UserEventQueue&                                 mrUserEventQueue;
            const UnoViewContainer&                         mrViewContainer;
            ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext>   mxComponentContext;
        };
    }
}

#endif /* _SLIDESHOW_SLIDESHOWCONTEXT_HXX */
