/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slideshowcontext.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:15:45 $
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

#ifndef INCLUDED_SLIDESHOW_SLIDESHOWCONTEXT_HXX
#define INCLUDED_SLIDESHOW_SLIDESHOWCONTEXT_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <boost/shared_ptr.hpp>

namespace com{ namespace sun{ namespace star{ namespace uno
{
    class XComponentContext;
}}}}


namespace slideshow
{
    namespace internal
    {
        class ShapeManager;
        class EventQueue;
        class ActivitiesQueue;
        class UserEventQueue;
        class EventMultiplexer;
        class ScreenUpdater;
        class UnoViewContainer;
        class CursorManager;
        class SubsettableShapeManager;

        /** Common arguments for slideshow objects.

            This struct combines a number of object references
            ubiquituously needed throughout the slideshow.
         */
        struct SlideShowContext
        {
            /** Common context for node creation

                @param rShapeManager
                ShapeManager, which handles all shapes

                @param rEventQueue
                Event queue, where time-based events are to be
                scheduled. A node must not schedule events there
                before it's not resolved.

                @param rEventMultiplexer
                Event multiplexer. Clients can register there for
                about any event that happens in the slideshow

                @param rScreenUpdater
                Screen updater. Gets notified of necessary screen
                updates.

                @param rActivitiesQueue
                Activities queue, where repeating activities are
                to be scheduled.

                @param rUserEventQueue
                User event queue

                @param rViewContainer
                Holds all views added to slideshow

                @param rComponentContext
                To create UNO services from
            */
            SlideShowContext( boost::shared_ptr<SubsettableShapeManager>&       rSubsettableShapeManager,
                              EventQueue&                                       rEventQueue,
                              EventMultiplexer&                                 rEventMultiplexer,
                              ScreenUpdater&                                    rScreenUpdater,
                              ActivitiesQueue&                                  rActivitiesQueue,
                              UserEventQueue&                                   rUserEventQueue,
                              CursorManager&                                    rCursorManager,
                              const UnoViewContainer&                           rViewContainer,
                              const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::uno::XComponentContext>&    rComponentContext );
            void dispose();

            boost::shared_ptr<SubsettableShapeManager>&     mpSubsettableShapeManager;
            EventQueue&                                     mrEventQueue;
            EventMultiplexer&                               mrEventMultiplexer;
            ScreenUpdater&                                  mrScreenUpdater;
            ActivitiesQueue&                                mrActivitiesQueue;
            UserEventQueue&                                 mrUserEventQueue;
            CursorManager&                                  mrCursorManager;
            const UnoViewContainer&                         mrViewContainer;
            ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext>   mxComponentContext;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_SLIDESHOWCONTEXT_HXX */
