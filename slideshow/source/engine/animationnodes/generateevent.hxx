/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: generateevent.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:44:36 $
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

#ifndef _SLIDESHOW_GENERATEEVENT_HXX
#define _SLIDESHOW_GENERATEEVENT_HXX

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODE_HPP_
#include <com/sun/star/animations/XAnimationNode.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_TIMING_HPP_
#include <com/sun/star/animations/Timing.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_EVENTTRIGGER_HPP_
#include <com/sun/star/animations/EventTrigger.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_EVENT_HPP_
#include <com/sun/star/animations/Event.hpp>
#endif

#include <shape.hxx>
#include <slideshowcontext.hxx>
#include <delayevent.hxx>


namespace presentation
{
    namespace internal
    {
        /** Create an event for the given description, call the
            given functor.

            @param rEventDescription
            Directly from the API

            @param rFunctor
            Functor to call when event fires.

            @param rContext
            Context struct, to provide event queue

            @param nAdditionalDelay
            Additional delay, gets added on top of timeout.
        */
        template< typename Functor > void generateEvent( const ::com::sun::star::uno::Any&      rEventDescription,
                                                         const Functor&                         rFunctor,
                                                         const SlideShowContext&                rContext,
                                                         double                                 nAdditionalDelay )
        {
            if( !rEventDescription.hasValue() )
                return;

            ::com::sun::star::animations::Timing                            eTiming;
            ::com::sun::star::animations::Event                             aEvent;
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >   aSequence;
            double                                                          nDelay;

            if( (rEventDescription >>= eTiming) )
            {
                switch( eTiming )
                {
                    case ::com::sun::star::animations::Timing_INDEFINITE:
                        return; // don't schedule no event

                    case ::com::sun::star::animations::Timing_MEDIA:
                        OSL_ENSURE( false,
                                    "generateEvent(): MEDIA timing not yet implemented" );
                        return;

                    default:
                        ENSURE_AND_THROW( false,
                                          "generateEvent(): unexpected case" );
                }
            }
            else if( (rEventDescription >>= aEvent) )
            {
                double nDelay(0.0);

                // try to extract additional event delay
                if( aEvent.Offset.hasValue() &&
                    !(aEvent.Offset >>= nDelay) )
                {
                    OSL_ENSURE( false,
                                "generateEvent(): offset values apart from <double> not "
                                "recognized in animations::Event" );
                }

                // common vars used inside switch
                ::com::sun::star::uno::Reference<
                      ::com::sun::star::animations::XAnimationNode > xNode;
                ::com::sun::star::uno::Reference<
                      ::com::sun::star::drawing::XShape >            xShape;
                ShapeSharedPtr                                       pShape;

                // TODO(F1): Respect aEvent.Repeat value

                switch( aEvent.Trigger )
                {
                    default:
                        ENSURE_AND_THROW( false,
                                          "generateEvent(): unexpected event trigger" );

                    case ::com::sun::star::animations::EventTrigger::NONE:
                        // no event at all
                        break;

                    case ::com::sun::star::animations::EventTrigger::ON_BEGIN:
                        OSL_ENSURE( false,
                                    "generateEvent(): event trigger ON_BEGIN not yet implemented" );
                        break;

                    case ::com::sun::star::animations::EventTrigger::ON_END:
                        OSL_ENSURE( false,
                                    "generateEvent(): event trigger ON_END not yet implemented" );
                        break;

                    case ::com::sun::star::animations::EventTrigger::BEGIN_EVENT:
                        // try to extract XAnimationNode event source
                        if( (aEvent.Source >>= xNode) )
                        {
                            rContext.mrUserEventQueue.registerAnimationStartEvent(
                                makeDelay( rFunctor,
                                           nDelay + nAdditionalDelay ),
                                xNode );
                        }
                        else
                        {
                            OSL_ENSURE(false,
                                       "generateEvent(): could not extract source XAnimationNode "
                                       "for BEGIN_EVENT");
                        }
                        break;

                    case ::com::sun::star::animations::EventTrigger::END_EVENT:
                        // try to extract XAnimationNode event source
                        if( (aEvent.Source >>= xNode) )
                        {
                            rContext.mrUserEventQueue.registerAnimationEndEvent(
                                makeDelay( rFunctor,
                                           nDelay + nAdditionalDelay ),
                                xNode );
                        }
                        else
                        {
                            OSL_ENSURE(false,
                                       "generateEvent(): could not extract source XAnimationNode "
                                       "for END_EVENT");
                        }
                        break;

                    case ::com::sun::star::animations::EventTrigger::ON_CLICK:
                        // try to extract XShape event source
                        if( (aEvent.Source >>= xShape) &&
                            (pShape=rContext.mpLayerManager->lookupShape(xShape)).get() )
                        {
                            rContext.mrUserEventQueue.registerShapeClickEvent(
                                makeDelay( rFunctor,
                                           nDelay + nAdditionalDelay ),
                                pShape );
                        }
                        else
                        {
                            OSL_ENSURE(false,
                                       "generateEvent(): could not extract source XAnimationNode "
                                       "for ON_CLICK");
                        }
                        break;

                    case ::com::sun::star::animations::EventTrigger::ON_DBL_CLICK:
                        // try to extract XShape event source
                        if( (aEvent.Source >>= xShape) &&
                            (pShape=rContext.mpLayerManager->lookupShape(xShape)).get() )
                        {
                            rContext.mrUserEventQueue.registerShapeDoubleClickEvent(
                                makeDelay( rFunctor,
                                           nDelay + nAdditionalDelay ),
                                pShape );
                        }
                        else
                        {
                            OSL_ENSURE(false,
                                       "generateEvent(): could not extract source XAnimationNode "
                                       "for ON_DBL_CLICK");
                        }
                        break;

                    case ::com::sun::star::animations::EventTrigger::ON_MOUSE_ENTER:
                        // try to extract XShape event source
                        if( (aEvent.Source >>= xShape) &&
                            (pShape=rContext.mpLayerManager->lookupShape(xShape)).get() )
                        {
                            rContext.mrUserEventQueue.registerMouseEnterEvent(
                                makeDelay( rFunctor,
                                           nDelay + nAdditionalDelay ),
                                pShape );
                        }
                        else
                        {
                            OSL_ENSURE(false,
                                       "generateEvent(): could not extract source XAnimationNode "
                                       "for ON_MOUSE_ENTER");
                        }
                        break;

                    case ::com::sun::star::animations::EventTrigger::ON_MOUSE_LEAVE:
                        // try to extract XShape event source
                        if( (aEvent.Source >>= xShape) &&
                            (pShape=rContext.mpLayerManager->lookupShape(xShape)).get() )
                        {
                            rContext.mrUserEventQueue.registerMouseLeaveEvent(
                                makeDelay( rFunctor,
                                           nDelay + nAdditionalDelay ),
                                pShape );
                        }
                        else
                        {
                            OSL_ENSURE(false,
                                       "generateEvent(): could not extract source XAnimationNode "
                                       "for ON_MOUSE_LEAVE");
                        }
                        break;

                    case ::com::sun::star::animations::EventTrigger::ON_PREV:
                        OSL_ENSURE( false,
                                    "generateEvent(): event trigger ON_PREV not yet implemented, mapped to ON_NEXT" );
                        // FALLTHROUGH intended
                    case ::com::sun::star::animations::EventTrigger::ON_NEXT:
                        rContext.mrUserEventQueue.registerNextEffectEvent(
                            makeDelay( rFunctor,
                                       nDelay + nAdditionalDelay ) );
                        break;

                    case ::com::sun::star::animations::EventTrigger::ON_STOP_AUDIO:
                        // try to extract XAnimationNode event source
                        if( (aEvent.Source >>= xNode) )
                        {
                            rContext.mrUserEventQueue.registerAudioStoppedEvent(
                                makeDelay( rFunctor,
                                           nDelay + nAdditionalDelay ),
                                xNode );
                        }
                        else
                        {
                            OSL_ENSURE(false,
                                       "generateEvent(): could not extract source XAnimationNode "
                                       "for ON_STOP_AUDIO");
                        }
                        break;

                    case ::com::sun::star::animations::EventTrigger::REPEAT:
                        OSL_ENSURE( false,
                                    "generateEvent(): event trigger REPEAT not yet implemented" );
                        break;
                }

                return;
            }
            else if( (rEventDescription >>= aSequence) )
            {
                OSL_ENSURE( false,
                            "generateEvent(): sequence of timing primitives not yet implemented" );
                return;
            }
            else if( (rEventDescription >>= nDelay) )
            {
                // schedule delay event
                rContext.mrEventQueue.addEvent( makeDelay( rFunctor,
                                                           nDelay + nAdditionalDelay ) );
            }
        }
    }
}

#endif /* _SLIDESHOW_GENERATEEVENT_HXX */
