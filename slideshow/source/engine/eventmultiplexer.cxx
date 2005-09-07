/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventmultiplexer.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:25:28 $
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
#include <eventmultiplexer.hxx>

#ifndef _COM_SUN_STAR_AWT_SYSTEMPOINTER_HPP_
#include <com/sun/star/awt/SystemPointer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MOUSEBUTTON_HPP_
#include <com/sun/star/awt/MouseButton.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_XSLIDESHOWVIEW_HPP_
#include <com/sun/star/presentation/XSlideShowView.hpp>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif

#include <delayevent.hxx>

#include <functional>
#include <algorithm>


using namespace ::com::sun::star;

/* Implementation of EventMultiplexer class */

namespace presentation
{
    namespace internal
    {
        bool EventMultiplexer::Listener::isMouseListenerRegistered() const
        {
            return !(maMouseClickHandlers.empty() && maMouseDoubleClickHandlers.empty());
        }

        template< typename Container, typename Handler >
            void EventMultiplexer::Listener::addHandler( Container&                             rContainer,
                                                         const ::boost::shared_ptr< Handler >&  rHandler )
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            ENSURE_AND_THROW( rHandler.get(),
                              "EventMultiplexer::Listener::addHandler(): Invalid handler" );

            rContainer.push_back( rHandler );
        }

        template< typename Container, typename Handler >
            void EventMultiplexer::Listener::removeHandler( Container&                              rContainer,
                                                            const ::boost::shared_ptr< Handler >&   rHandler )
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            ENSURE_AND_THROW( rHandler.get(),
                              "EventMultiplexer::Listener::removeHandler(): Invalid handler" );

            const typename Container::iterator aEnd( rContainer.end() );
            rContainer.erase( ::std::remove(rContainer.begin(),
                                            aEnd,
                                            rHandler),
                              aEnd );
        }

        template< typename Function >
            void EventMultiplexer::Listener::forEachView( Function pViewMethod )
        {
            if( pViewMethod )
            {
                // (un)register mouse listener on all views
                for( UnoViewVector::const_iterator aIter=maViews.begin(), aEnd=maViews.end();
                     aIter!=aEnd;
                     ++aIter )
                {
                    ((*aIter)->getUnoView().get()->*pViewMethod)( this );
                }
            }
        }

        template< typename RegisterFunction >
            void EventMultiplexer::Listener::addMouseHandler( ImplMouseHandlers&            rHandlerContainer,
                                                              const MouseEventHandlerSharedPtr& rHandler,
                                                              double                        nPriority,
                                                              RegisterFunction              pRegisterListener )
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            ENSURE_AND_THROW( rHandler.get(),
                              "EventMultiplexer::Listener::addMouseHandler(): Invalid handler" );

            // register mouse listener on all views
            forEachView( pRegisterListener );

            const bool bEmptyContainer( rHandlerContainer.empty() );

            // insert into queue (not yet sorted)
            rHandlerContainer.push_back(
                EventMultiplexer::Listener::MouseEventHandlerEntry( rHandler,
                                                               nPriority ) );

            if( !bEmptyContainer )
            {
                // precond: rHandlerContainer.size() is now at least 2,
                // which is ensured by the rHandlerContainer.empty()
                // check above.

                // element was inserted, but now we have to keep the
                // entries sorted
                ::std::inplace_merge( rHandlerContainer.begin(),
                                      rHandlerContainer.end()-1,
                                      rHandlerContainer.end() );
            }
            // no need to call inplace_merge otherwise, added
            // entry is the only one on the heap.
        }

        void EventMultiplexer::Listener::tick()
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if( !mbIsAutoMode )
                return; // this event is just a left-over, ignore

            notifyNextEffect();

            if( !maNextEffectHandlers.empty() )
            {
                // still handlers left, schedule next timeout
                // event. Will also set mbIsTickEventOn back to true
                scheduleTick();
            }
        }

        void EventMultiplexer::Listener::scheduleTick()
        {
            // TODO(Q3): make robust (no boost::ref()) when
            //           get_pointer(uno::Ref...) is available
            EventSharedPtr pEvent(
                makeDelay(
                    boost::bind( &EventMultiplexer::Listener::tick, this ),
                    mnTimeout ) );

            // store weak reference to generated event, to notice when
            // the event queue gets cleansed (we then have to
            // regenerate the tick event!)
            mpTickEvent = pEvent;

            // enabled auto mode: simply schedule a timeout event,
            // which will eventually call our tick() method
            mrEventQueue.addEvent( pEvent );
        }

        void EventMultiplexer::Listener::handleTicks()
        {
            if( !mbIsAutoMode )
                return; // nothing to do, don't need no ticks

            EventSharedPtr pTickEvent( mpTickEvent.lock() );
            if( pTickEvent )
                return; // nothing to do, there's already a tick
                        // pending

            // schedule initial tick (which reschedules itself
            // after that, all by itself)
            scheduleTick();
        }

        void EventMultiplexer::Listener::implSetMouseCursor( sal_Int16 nCursor ) const
        {
            // change all views to the requested cursor ID
            ::std::for_each( maViews.begin(),
                             maViews.end(),
                             ::boost::bind( &View::setMouseCursor,
                                            _1,
                                            nCursor ) );
        }


        // public methods
        // ===============================================

        EventMultiplexer::EventMultiplexer( EventQueue& rEventQueue ) :
            mpListener( new Listener( rEventQueue ) )
        {
            ENSURE_AND_THROW( mpListener.is(),
                              "EventMultiplexer::EventMultiplexer(): Could not create Listener" );
        }

        EventMultiplexer::~EventMultiplexer()
        {
            // dispose listener, making it release
            // all references, and deregistering from the
            // broadcaster
            mpListener->dispose();
        }

        bool EventMultiplexer::addView( const UnoViewSharedPtr& rView )
        {
            return mpListener->addView( rView );
        }

        bool EventMultiplexer::removeView( const UnoViewSharedPtr& rView )
        {
            return mpListener->removeView( rView );
        }

        void EventMultiplexer::clear()
        {
            return mpListener->clear();
        }

        void EventMultiplexer::setMouseCursor( sal_Int16 nCursor )
        {
            mpListener->setMouseCursor( nCursor );
        }

        void EventMultiplexer::setVolatileMouseCursor( sal_Int16 nCursor )
        {
            mpListener->setVolatileMouseCursor( nCursor );
        }

        void EventMultiplexer::setLayerManager( const LayerManagerSharedPtr& rMgr )
        {
            mpListener->setLayerManager( rMgr );
        }

        void EventMultiplexer::updateScreenContent( bool bForceUpdate )
        {
            mpListener->updateScreenContent( bForceUpdate );
        }

        void EventMultiplexer::setAutomaticMode( bool bIsAuto )
        {
            mpListener->setAutomaticMode( bIsAuto );
        }

        bool EventMultiplexer::getAutomaticMode() const
        {
            return mpListener->getAutomaticMode();
        }

        void EventMultiplexer::setAutomaticTimeout( double nTimeout )
        {
            mpListener->setAutomaticTimeout( nTimeout );
        }

        double EventMultiplexer::getAutomaticTimeout() const
        {
            return mpListener->getAutomaticTimeout();
        }


        void EventMultiplexer::addNextEffectHandler( const EventHandlerSharedPtr&   rHandler,
                                                     double                         nPriority )
        {
            mpListener->addNextEffectHandler( rHandler,
                                              nPriority );
        }

        void EventMultiplexer::removeNextEffectHandler( const EventHandlerSharedPtr& rHandler )
        {
            mpListener->removeNextEffectHandler( rHandler );
        }

        void EventMultiplexer::addSlideStartHandler( const EventHandlerSharedPtr& rHandler )
        {
            mpListener->addSlideStartHandler( rHandler );
        }

        void EventMultiplexer::removeSlideStartHandler( const EventHandlerSharedPtr& rHandler )
        {
            mpListener->removeSlideStartHandler( rHandler );
        }

        void EventMultiplexer::addSlideEndHandler( const EventHandlerSharedPtr& rHandler )
        {
            mpListener->addSlideEndHandler( rHandler );
        }

        void EventMultiplexer::removeSlideEndHandler( const EventHandlerSharedPtr& rHandler )
        {
            mpListener->removeSlideEndHandler( rHandler );
        }

        void EventMultiplexer::addAnimationStartHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            mpListener->addAnimationStartHandler( rHandler );
        }

        void EventMultiplexer::removeAnimationStartHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            mpListener->removeAnimationStartHandler( rHandler );
        }

        void EventMultiplexer::addAnimationEndHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            mpListener->addAnimationEndHandler( rHandler );
        }

        void EventMultiplexer::removeAnimationEndHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            mpListener->removeAnimationEndHandler( rHandler );
        }

        void EventMultiplexer::addSlideAnimationsEndHandler( const EventHandlerSharedPtr& rHandler )
        {
            mpListener->addSlideAnimationsEndHandler( rHandler );
        }

        void EventMultiplexer::removeSlideAnimationsEndHandler( const EventHandlerSharedPtr& rHandler )
        {
            mpListener->removeSlideAnimationsEndHandler( rHandler );
        }

        void EventMultiplexer::addAudioStoppedHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            mpListener->addAudioStoppedHandler( rHandler );
        }

        void EventMultiplexer::removeAudioStoppedHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            mpListener->removeAudioStoppedHandler( rHandler );
        }

        void EventMultiplexer::addCommandStopAudioHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            mpListener->addCommandStopAudioHandler( rHandler );
        }

        void EventMultiplexer::removeCommandStopAudioHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            mpListener->removeCommandStopAudioHandler( rHandler );
        }

        void EventMultiplexer::addPauseHandler( const PauseEventHandlerSharedPtr& rHandler )
        {
            return mpListener->addPauseHandler( rHandler );
        }

        void EventMultiplexer::removePauseHandler( const PauseEventHandlerSharedPtr& rHandler )
        {
            return mpListener->removePauseHandler( rHandler );
        }

        void EventMultiplexer::addClickHandler( const MouseEventHandlerSharedPtr&   rHandler,
                                                double                          nPriority )
        {
            return mpListener->addClickHandler( rHandler,
                                                nPriority );
        }

        void EventMultiplexer::removeClickHandler( const MouseEventHandlerSharedPtr&    rHandler )
        {
            return mpListener->removeClickHandler( rHandler );
        }

        void EventMultiplexer::addDoubleClickHandler( const MouseEventHandlerSharedPtr& rHandler,
                                                      double                        nPriority )
        {
            return mpListener->addDoubleClickHandler( rHandler,
                                                      nPriority );
        }

        void EventMultiplexer::removeDoubleClickHandler( const MouseEventHandlerSharedPtr&  rHandler )
        {
            return mpListener->removeDoubleClickHandler( rHandler );
        }

        void EventMultiplexer::addMouseMoveHandler( const MouseEventHandlerSharedPtr&   rHandler,
                                                    double                          nPriority )
        {
            return mpListener->addMouseMoveHandler( rHandler,
                                                    nPriority );
        }

        void EventMultiplexer::removeMouseMoveHandler( const MouseEventHandlerSharedPtr&    rHandler )
        {
            return mpListener->removeMouseMoveHandler( rHandler );
        }


        bool EventMultiplexer::notifyNextEffect()
        {
            return mpListener->notifyNextEffect();
        }

        bool EventMultiplexer::notifySlideStartEvent()
        {
            return mpListener->notifySlideStartEvent();
        }

        bool EventMultiplexer::notifySlideEndEvent()
        {
            return mpListener->notifySlideEndEvent();
        }

        bool EventMultiplexer::notifyAnimationStart( const AnimationNodeSharedPtr& rNode )
        {
            return mpListener->notifyAnimationStart( rNode );
        }

        bool EventMultiplexer::notifyAnimationEnd( const AnimationNodeSharedPtr& rNode )
        {
            return mpListener->notifyAnimationEnd( rNode );
        }

        bool EventMultiplexer::notifySlideAnimationsEnd()
        {
            return mpListener->notifySlideAnimationsEnd();
        }

        bool EventMultiplexer::notifyAudioStopped( const AnimationNodeSharedPtr& rNode )
        {
            return mpListener->notifyAudioStopped( rNode );
        }

        bool EventMultiplexer::notifyPauseMode( bool bPauseShow )
        {
            return mpListener->notifyPauseMode( bPauseShow );
        }
        void EventMultiplexer::notifyCommandStopAudio( const AnimationNodeSharedPtr& rNode )
        {
            mpListener->notifyCommandStopAudio( rNode );
        }

        EventMultiplexer::Listener::Listener( EventQueue& rEventQueue ) :
            Listener_UnoBase( m_aMutex ),
            mrEventQueue( rEventQueue ),
            maViews(),
            maNextEffectHandlers(),
            maSlideStartHandlers(),
            maSlideEndHandlers(),
            maAnimationStartHandlers(),
            maAnimationEndHandlers(),
            maSlideAnimationsEndHandlers(),
            maAudioStoppedHandlers(),
            maCommandStopAudioHandlers(),
            maPauseHandlers(),
            maMouseClickHandlers(),
            maMouseDoubleClickHandlers(),
            maMouseMoveHandlers(),
            mnTimeout( 0.0 ),
            mpTickEvent(),
            mnMouseCursor( awt::SystemPointer::ARROW ),
            mnVolatileMouseCursor( -1 ),
            mnLastVolatileMouseCursor( mnMouseCursor ),
            mbIsAutoMode( false )
        {
        }

        bool EventMultiplexer::Listener::addView( const UnoViewSharedPtr& rView )
        {
            ENSURE_AND_THROW( rView.get(),
                              "EventMultiplexer::Listener::addView(): Invalid view" );

            ::osl::MutexGuard aGuard( m_aMutex );

            // check whether same view is already added
            const UnoViewVector::iterator aEnd( maViews.end() );

            // already added?
            if( ::std::find( maViews.begin(),
                             aEnd,
                             rView ) != aEnd )
            {
                // yes, nothing to do
                return false;
            }

            maViews.push_back( rView );

            return true;
        }

        bool EventMultiplexer::Listener::removeView( const UnoViewSharedPtr& rView )
        {
            ENSURE_AND_THROW( rView.get(),
                              "EventMultiplexer::Listener::removeView(): Invalid view" );

            ::osl::MutexGuard aGuard( m_aMutex );

            // check whether same view is already added
            const UnoViewVector::iterator aEnd( maViews.end() );
            UnoViewVector::iterator aIter;

            // view added?
            if( (aIter=::std::find( maViews.begin(),
                                    aEnd,
                                    rView )) == aEnd )
            {
                // nope, nothing to do
                return false;
            }

            // revoke event listeners
            const uno::Reference< ::com::sun::star::presentation::XSlideShowView >&
                rUnoView( rView->getUnoView() );

            if( isMouseListenerRegistered() )
                rUnoView->removeMouseListener( this );

            if( !maMouseMoveHandlers.empty() )
                rUnoView->removeMouseMotionListener( this );

            // actually erase from container
            maViews.erase( aIter );

            return true;
        }

        void EventMultiplexer::Listener::clear()
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // deregister from all views.
            if( isMouseListenerRegistered() )
            {
                for( UnoViewVector::iterator aIter=maViews.begin(), aEnd=maViews.end();
                     aIter!=aEnd;
                     ++aIter )
                {
                    (*aIter)->getUnoView()->removeMouseListener( this );
                }
            }

            if( !maMouseMoveHandlers.empty() )
            {
                for( UnoViewVector::iterator aIter=maViews.begin(), aEnd=maViews.end();
                     aIter!=aEnd;
                     ++aIter )
                {
                    (*aIter)->getUnoView()->removeMouseMotionListener( this );
                }
            }

            // release all references
            maViews.clear();

            maNextEffectHandlers.clear();
            maSlideStartHandlers.clear();
            maSlideEndHandlers.clear();
            maAnimationStartHandlers.clear();
            maAnimationEndHandlers.clear();
            maSlideAnimationsEndHandlers.clear();
            maAudioStoppedHandlers.clear();
            maCommandStopAudioHandlers.clear();
            maPauseHandlers.clear();
            maMouseClickHandlers.clear();
            maMouseDoubleClickHandlers.clear();
            maMouseMoveHandlers.clear();
            mpLayerManager.reset();
        }

        void EventMultiplexer::Listener::setMouseCursor( sal_Int16 nCursor )
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            mnMouseCursor = nCursor;

            implSetMouseCursor( mnMouseCursor );
        }

        void EventMultiplexer::Listener::setVolatileMouseCursor( sal_Int16 nCursor )
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // only change, if volatile cursor is not already
            // set. This effectively prioritizes handlers with the
            // highest priority (otherwise, the handler with the
            // lowest prio would determine the mouse cursor -
            // unwanted).
            if( mnVolatileMouseCursor == -1 )
                mnVolatileMouseCursor = nCursor;
        }

        void EventMultiplexer::Listener::setLayerManager( const LayerManagerSharedPtr& rMgr )
        {
            mpLayerManager = rMgr;
        }

        void EventMultiplexer::Listener::updateScreenContent( bool bForceUpdate )
        {
            // perform screen update (either if we're forced to do it,
            // or if the layer manager signals that it needs one).
            if( bForceUpdate ||
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
                // updates independent from a valid LayerManager!
                ::std::for_each( maViews.begin(),
                                 maViews.end(),
                                 ::boost::mem_fn( &View::updateScreen ) );
            }
        }

        void EventMultiplexer::Listener::setAutomaticMode( bool bIsAuto )
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if( bIsAuto == mbIsAutoMode )
                return; // no change, nothing to do

            mbIsAutoMode = bIsAuto;

            handleTicks();
        }

        bool EventMultiplexer::Listener::getAutomaticMode() const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            return mbIsAutoMode;
        }

        void EventMultiplexer::Listener::setAutomaticTimeout( double nTimeout )
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            mnTimeout = nTimeout;
        }

        double EventMultiplexer::Listener::getAutomaticTimeout() const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            return mnTimeout;
        }

        void EventMultiplexer::Listener::addNextEffectHandler( const EventHandlerSharedPtr& rHandler,
                                                               double                       nPriority )
        {
            addHandler( maNextEffectHandlers,
                        rHandler );

            // Enable tick events, if not done already
            handleTicks();
        }

        void EventMultiplexer::Listener::removeNextEffectHandler( const EventHandlerSharedPtr& rHandler )
        {
            removeHandler( maNextEffectHandlers,
                           rHandler );
        }

        void EventMultiplexer::Listener::addSlideStartHandler( const EventHandlerSharedPtr& rHandler )
        {
            addHandler( maSlideStartHandlers,
                        rHandler );
        }

        void EventMultiplexer::Listener::removeSlideStartHandler( const EventHandlerSharedPtr& rHandler )
        {
            removeHandler( maSlideStartHandlers,
                           rHandler );
        }

        void EventMultiplexer::Listener::addSlideEndHandler( const EventHandlerSharedPtr& rHandler )
        {
            addHandler( maSlideEndHandlers,
                        rHandler );
        }

        void EventMultiplexer::Listener::removeSlideEndHandler( const EventHandlerSharedPtr& rHandler )
        {
            removeHandler( maSlideEndHandlers,
                           rHandler );
        }

        void EventMultiplexer::Listener::addAnimationStartHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            addHandler( maAnimationStartHandlers,
                        rHandler );
        }

        void EventMultiplexer::Listener::removeAnimationStartHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            removeHandler( maAnimationStartHandlers,
                           rHandler );
        }

        void EventMultiplexer::Listener::addAnimationEndHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            addHandler( maAnimationEndHandlers,
                        rHandler );
        }

        void EventMultiplexer::Listener::removeAnimationEndHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            removeHandler( maAnimationEndHandlers,
                           rHandler );
        }

        void EventMultiplexer::Listener::addSlideAnimationsEndHandler( const EventHandlerSharedPtr& rHandler )
        {
            addHandler( maSlideAnimationsEndHandlers,
                        rHandler );
        }

        void EventMultiplexer::Listener::removeSlideAnimationsEndHandler( const EventHandlerSharedPtr& rHandler )
        {
            removeHandler( maSlideAnimationsEndHandlers,
                           rHandler );
        }

        void EventMultiplexer::Listener::addAudioStoppedHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            addHandler( maAudioStoppedHandlers,
                        rHandler );
        }

        void EventMultiplexer::Listener::removeAudioStoppedHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            removeHandler( maAudioStoppedHandlers,
                           rHandler );
        }

        void EventMultiplexer::Listener::addCommandStopAudioHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            addHandler( maCommandStopAudioHandlers,
                        rHandler );
        }

        void EventMultiplexer::Listener::removeCommandStopAudioHandler( const AnimationEventHandlerSharedPtr& rHandler )
        {
            removeHandler( maCommandStopAudioHandlers,
                           rHandler );
        }

        void EventMultiplexer::Listener::addPauseHandler( const PauseEventHandlerSharedPtr& rHandler )
        {
            addHandler( maPauseHandlers,
                        rHandler );
        }

        void EventMultiplexer::Listener::removePauseHandler( const PauseEventHandlerSharedPtr&  rHandler )
        {
            removeHandler( maPauseHandlers,
                           rHandler );
        }

        void EventMultiplexer::Listener::addClickHandler( const MouseEventHandlerSharedPtr& rHandler,
                                                          double                        nPriority )
        {
            addMouseHandler( maMouseClickHandlers,
                             rHandler,
                             nPriority,
                             isMouseListenerRegistered() ? NULL : &::com::sun::star::presentation::XSlideShowView::addMouseListener );
        }

        void EventMultiplexer::Listener::removeClickHandler( const MouseEventHandlerSharedPtr&  rHandler )
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            removeHandler( maMouseClickHandlers,
                           rHandler );

            if( !isMouseListenerRegistered() )
                forEachView( &::com::sun::star::presentation::XSlideShowView::removeMouseListener );
        }

        void EventMultiplexer::Listener::addDoubleClickHandler( const MouseEventHandlerSharedPtr&   rHandler,
                                                                double                          nPriority )
        {
            addMouseHandler( maMouseDoubleClickHandlers,
                             rHandler,
                             nPriority,
                             isMouseListenerRegistered() ? NULL : &::com::sun::star::presentation::XSlideShowView::addMouseListener );
        }

        void EventMultiplexer::Listener::removeDoubleClickHandler( const MouseEventHandlerSharedPtr&    rHandler )
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            removeHandler( maMouseDoubleClickHandlers,
                           rHandler );

            if( !isMouseListenerRegistered() )
                forEachView( &::com::sun::star::presentation::XSlideShowView::removeMouseListener );
        }

        void EventMultiplexer::Listener::addMouseMoveHandler( const MouseEventHandlerSharedPtr& rHandler,
                                                              double                        nPriority )
        {
            addMouseHandler( maMouseMoveHandlers,
                             rHandler,
                             nPriority,
                             maMouseMoveHandlers.empty() ? &::com::sun::star::presentation::XSlideShowView::addMouseMotionListener : NULL );
        }

        void EventMultiplexer::Listener::removeMouseMoveHandler( const MouseEventHandlerSharedPtr&  rHandler )
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            removeHandler( maMouseMoveHandlers,
                           rHandler );

            if( maMouseMoveHandlers.empty() )
                forEachView( &::com::sun::star::presentation::XSlideShowView::removeMouseMotionListener );
        }


        bool EventMultiplexer::Listener::notifyNextEffect()
        {
            return notifyHandlers( maNextEffectHandlers );
        }

        bool EventMultiplexer::Listener::notifySlideStartEvent()
        {
            return notifyHandlers( maSlideStartHandlers );
        }

        bool EventMultiplexer::Listener::notifySlideEndEvent()
        {
            return notifyHandlers( maSlideEndHandlers );
        }

        bool EventMultiplexer::Listener::notifyAnimationStart( const AnimationNodeSharedPtr& rNode )
        {
            return notifyHandlers( maAnimationStartHandlers,
                                   rNode );
        }

        bool EventMultiplexer::Listener::notifyAnimationEnd( const AnimationNodeSharedPtr& rNode )
        {
            return notifyHandlers( maAnimationEndHandlers,
                                   rNode );
        }

        bool EventMultiplexer::Listener::notifySlideAnimationsEnd()
        {
            return notifyHandlers( maSlideAnimationsEndHandlers );
        }

        bool EventMultiplexer::Listener::notifyAudioStopped( const AnimationNodeSharedPtr& rNode )
        {
            return notifyHandlers( maAudioStoppedHandlers,
                                   rNode );
        }

        void EventMultiplexer::Listener::notifyCommandStopAudio( const AnimationNodeSharedPtr& rNode )
        {
            notifyHandlers( maCommandStopAudioHandlers,
                                   rNode );
        }

        bool EventMultiplexer::Listener::notifyPauseMode( bool bPauseShow )
        {
            ::osl::ClearableMutexGuard aGuard( m_aMutex );

            // generate a local copy of all handlers, since we have to
            // release the object mutex before firing.
            ImplPauseHandlers aLocalHandlers( maPauseHandlers );

            aGuard.clear();

            if( ::std::count_if( aLocalHandlers.begin(),
                                 aLocalHandlers.end(),
                                 ::boost::bind( &PauseEventHandler::handlePause,
                                                _1,
                                                bPauseShow ) ) > 0 )
                return true; // at least one handler returned true
            else
                return false; // not a single handler returned true
        }

        void SAL_CALL EventMultiplexer::Listener::dispose() throw (uno::RuntimeException)
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // release all references
            clear();

            // call parent
            WeakComponentImplHelperBase::dispose();
        }

        // XMouseListener implementation
        void SAL_CALL EventMultiplexer::Listener::disposing( const lang::EventObject& rSource ) throw (uno::RuntimeException)
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // remove given event source
            for( UnoViewVector::iterator aIter=maViews.begin(), aEnd=maViews.end();
                 aIter!=aEnd;
                 ++aIter )
            {
                if( (*aIter)->getUnoView() == rSource.Source )
                {
                    // one instance found, remove
                    aIter = maViews.erase( aIter );
                    aEnd = maViews.end(); // erase might invalidate all iterators!
                }
            }
        }

        bool EventMultiplexer::Listener::notifyHandlers( const EventMultiplexer::Listener::ImplEventHandlers& rHandlerContainer )
        {
            ::osl::ClearableMutexGuard aGuard( m_aMutex );

            // generate a local copy of all handlers, since we have to
            // release the object mutex before firing.
            ImplEventHandlers aLocalHandlers( rHandlerContainer );

            aGuard.clear();

            if( ::std::count_if( aLocalHandlers.begin(),
                                 aLocalHandlers.end(),
                                 ::boost::mem_fn(&EventHandler::handleEvent) ) > 0 )
                return true; // at least one handler returned true
            else
                return false; // not a single handler returned true
        }

        bool EventMultiplexer::Listener::notifyHandlers( const EventMultiplexer::Listener::ImplAnimationHandlers&   rHandlerContainer,
                                                         const AnimationNodeSharedPtr&                              rNode )
        {
            ::osl::ClearableMutexGuard aGuard( m_aMutex );

            // generate a local copy of all handlers, since we have to
            // release the object mutex before firing.
            ImplAnimationHandlers aLocalHandlers( rHandlerContainer );

            aGuard.clear();

            if( ::std::count_if( aLocalHandlers.begin(),
                                 aLocalHandlers.end(),
                                 ::boost::bind(&AnimationEventHandler::handleAnimationEvent,
                                               _1,
                                               ::boost::cref(rNode) ) ) > 0 )
                return true; // at least one handler returned true
            else
                return false; // not a single handler returned true
        }

        bool EventMultiplexer::Listener::notifyHandlers( const EventMultiplexer::Listener::ImplMouseHandlers&   rQueue,
                                                         bool (MouseEventHandler::*pHandlerMethod)( const awt::MouseEvent& ),
                                                         const awt::MouseEvent&                                 e )
        {
            uno::Reference< ::com::sun::star::presentation::XSlideShowView >
                xView( e.Source,
                       uno::UNO_QUERY );

            ENSURE_AND_RETURN( xView.is(),
                               "EventMultiplexer::Listener::notifyHandlers(): event source is not an XSlideShowView" );

            // find corresponding view (to mouse position into user coordinate space)
            UnoViewVector::iterator         aIter;
            const UnoViewVector::iterator   aEnd( maViews.end() );
            if( (aIter=::std::find_if( maViews.begin(),
                                       aEnd,
                                       ::boost::bind(
                                           ::std::equal_to< uno::Reference< ::com::sun::star::presentation::XSlideShowView > >(),
                                           ::boost::cref( xView ),
                                           ::boost::bind(
                                               &UnoView::getUnoView,
                                               _1 ) ) ) ) == aEnd )
            {
                ENSURE_AND_RETURN( false,
                                   "EventMultiplexer::Listener::notifyHandlers(): event source not found under registered views" );
            }

            // convert mouse position to user coordinate space
            ::basegfx::B2DPoint     aPosition( e.X, e.Y );
            ::basegfx::B2DHomMatrix aMatrix( (*aIter)->getTransformation() );
            if( !aMatrix.invert() )
                ENSURE_AND_THROW( false,
                                  "EventMultiplexer::Listener::notifyHandlers(): view matrix singular" );
            aPosition *= aMatrix;

            awt::MouseEvent aEvent( e );
            aEvent.X = ::basegfx::fround( aPosition.getX() );
            aEvent.Y = ::basegfx::fround( aPosition.getY() );

            // fire event on handlers, try in order of precedence. If
            // one high-priority handler rejects the event
            // (i.e. returns false), try next handler.
            const EventMultiplexer::Listener::ImplMouseHandlers::const_iterator aEndOfHandlers( rQueue.end() );
            if( ::std::find_if( rQueue.begin(),
                                aEndOfHandlers,
                                ::boost::bind( pHandlerMethod,
                                               ::boost::bind(
                                                   &EventMultiplexer::Listener::MouseEventHandlerEntry::getHandler,
                                                   _1 ),
                                               aEvent ) ) == aEndOfHandlers )
            {
                return false; // no handler in this queue finally processed the event
            }
            else
            {
                return true; // a handler in this queue processed the event
            }
        }

        void EventMultiplexer::Listener::implMousePressed( const ::com::sun::star::awt::MouseEvent& e )
        {
            ::osl::ClearableMutexGuard aGuard( m_aMutex );

            // ignore everything, except left button clicks
            if( e.Buttons != awt::MouseButton::LEFT )
                return;

            // generate a local copy of all handlers, since we have to
            // release the object mutex before firing.
            ImplMouseHandlers aLocalDoubleClickHandlers( maMouseDoubleClickHandlers );
            ImplMouseHandlers aLocalClickHandlers( maMouseClickHandlers );

            aGuard.clear();

            // fire double-click events for every second click
            sal_Int32 nCurrClickCount = e.ClickCount;
            while( nCurrClickCount > 1 &&
                   notifyHandlers( aLocalDoubleClickHandlers,
                                   &MouseEventHandler::handleMousePressed,
                                   e ) )
            {
                nCurrClickCount -= 2;
            }

            // fire single-click events for all remaining clicks
            while( nCurrClickCount > 0 &&
                   notifyHandlers( aLocalClickHandlers,
                                   &MouseEventHandler::handleMousePressed,
                                   e ) )
            {
                --nCurrClickCount;
            }
        }

        void EventMultiplexer::Listener::implMouseReleased( const ::com::sun::star::awt::MouseEvent& e )
        {
            ::osl::ClearableMutexGuard aGuard( m_aMutex );

            // ignore everything, except left button clicks
            if( e.Buttons != awt::MouseButton::LEFT )
                return;

            // generate a local copy of all handlers, since we have to
            // release the object mutex before firing.
            ImplMouseHandlers aLocalDoubleClickHandlers( maMouseDoubleClickHandlers );
            ImplMouseHandlers aLocalClickHandlers( maMouseClickHandlers );

            aGuard.clear();

            // fire double-click events for every second click
            sal_Int32 nCurrClickCount = e.ClickCount;
            while( nCurrClickCount > 1 &&
                   notifyHandlers( aLocalDoubleClickHandlers,
                                   &MouseEventHandler::handleMouseReleased,
                                   e ) )
            {
                nCurrClickCount -= 2;
            }

            // fire single-click events for all remaining clicks
            while( nCurrClickCount > 0 &&
                   notifyHandlers( aLocalClickHandlers,
                                   &MouseEventHandler::handleMouseReleased,
                                   e ) )
            {
                --nCurrClickCount;
            }
        }

        void EventMultiplexer::Listener::implMouseDragged( const ::com::sun::star::awt::MouseEvent& e )
        {
            ::osl::ResettableMutexGuard aGuard( m_aMutex );

            // generate a local copy of all handlers, since we have to
            // release the object mutex before firing.
            ImplMouseHandlers aLocalHandlers( maMouseMoveHandlers );

            aGuard.clear();

            notifyHandlers( aLocalHandlers,
                            &MouseEventHandler::handleMouseDragged,
                            e );

            // re-acquire mutex
            aGuard.reset();

            if( mnVolatileMouseCursor != -1 )
            {
                // handlers requested a volatile mouse cursor - set
                // one
                mnLastVolatileMouseCursor = mnVolatileMouseCursor;
                implSetMouseCursor( mnVolatileMouseCursor );

                mnVolatileMouseCursor = -1;
            }
            else if( mnMouseCursor != mnLastVolatileMouseCursor )
            {
                // handlers did not request a volatile mouse cursor -
                // clear to default, if necessary
                implSetMouseCursor( mnMouseCursor );

                mnLastVolatileMouseCursor = mnMouseCursor;
            }
        }

        void EventMultiplexer::Listener::implMouseMoved( const ::com::sun::star::awt::MouseEvent& e )
        {
            ::osl::ResettableMutexGuard aGuard( m_aMutex );

            // generate a local copy of all handlers, since we have to
            // release the object mutex before firing.
            ImplMouseHandlers aLocalHandlers( maMouseMoveHandlers );

            aGuard.clear();

            notifyHandlers( aLocalHandlers,
                            &MouseEventHandler::handleMouseMoved,
                            e );

            // re-acquire mutex
            aGuard.reset();

            if( mnVolatileMouseCursor != -1 )
            {
                // handlers requested a volatile mouse cursor - set
                // one
                mnLastVolatileMouseCursor = mnVolatileMouseCursor;
                implSetMouseCursor( mnVolatileMouseCursor );

                mnVolatileMouseCursor = -1;
            }
            else if( mnMouseCursor != mnLastVolatileMouseCursor )
            {
                // handlers did not request a volatile mouse cursor -
                // clear to default, if necessary
                implSetMouseCursor( mnMouseCursor );

                mnLastVolatileMouseCursor = mnMouseCursor;
            }
        }

        void SAL_CALL EventMultiplexer::Listener::mousePressed( const awt::MouseEvent& e ) throw (uno::RuntimeException)
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // notify mouse press. Don't call handlers directly, this
            // might not be the main thread!
            mrEventQueue.addEvent(
                makeEvent( ::boost::bind( &EventMultiplexer::Listener::implMousePressed,
                                          this,
                                          e ) ) );
        }

        void SAL_CALL EventMultiplexer::Listener::mouseReleased( const awt::MouseEvent& e ) throw (uno::RuntimeException)
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // notify mouse release. Don't call handlers directly,
            // this might not be the main thread!
            mrEventQueue.addEvent(
                makeEvent( ::boost::bind( &EventMultiplexer::Listener::implMouseReleased,
                                          this,
                                          e ) ) );
        }

        void SAL_CALL EventMultiplexer::Listener::mouseEntered( const awt::MouseEvent& e ) throw (uno::RuntimeException)
        {
            // not used here
        }

        void SAL_CALL EventMultiplexer::Listener::mouseExited( const awt::MouseEvent& e ) throw (uno::RuntimeException)
        {
            // not used here
        }

        // XMouseMotionListener implementation
        void SAL_CALL EventMultiplexer::Listener::mouseDragged( const awt::MouseEvent& e ) throw (uno::RuntimeException)
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // notify mouse drag. Don't call handlers directly, this
            // might not be the main thread!
            mrEventQueue.addEvent(
                makeEvent( ::boost::bind( &EventMultiplexer::Listener::implMouseDragged,
                                          this,
                                          e ) ) );
        }

        void SAL_CALL EventMultiplexer::Listener::mouseMoved( const awt::MouseEvent& e ) throw (uno::RuntimeException)
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // notify mouse move. Don't call handlers directly, this
            // might not be the main thread!
            mrEventQueue.addEvent(
                makeEvent( ::boost::bind( &EventMultiplexer::Listener::implMouseMoved,
                                          this,
                                          e ) ) );
        }
    }
}
