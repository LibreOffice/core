/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventmultiplexer.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:11:26 $
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

#ifndef _SLIDESHOW_EVENTMULTIPLEXER_HXX
#define _SLIDESHOW_EVENTMULTIPLEXER_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSELISTENER_HPP_
#include <com/sun/star/awt/XMouseListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSEMOTIONLISTENER_HPP_
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

#include <eventhandler.hxx>
#include <mouseeventhandler.hxx>
#include <animationeventhandler.hxx>
#include <pauseeventhandler.hxx>

#include <layermanager.hxx>
#include <animationnode.hxx>
#include <eventqueue.hxx>
#include <unoview.hxx>

#include <boost/weak_ptr.hpp>
#include <vector>


namespace com { namespace sun { namespace star { namespace awt
{
    struct MouseEvent;
} } } }

/* Definition of EventMultiplexer class */

namespace presentation
{
    namespace internal
    {
        /** This class multiplexes user-activated and
            slide-show-global events.

            This class listens at the XSlideShowView and fires events
            registered for certain user actions. Furthermore, global
            slide show state changes (such as start or end of a slide)
            are handled as well. Note that registered events which
            have a non-zero timeout (i.e. events that return non-zero
            from getActivationTime()) will not be fired immediately
            after the user action occured, but only after the given
            timeout. Which is actually a feature.
         */
        class EventMultiplexer
        {
        public:
            /** Create an event multiplexer

                @param rEventQueue
                Reference to the main event queue. Since we hold this
                object by plain reference, it must live longer than we
                do. On the other hand, that queue must not fire events
                after this object is destroyed, since we might
                schedule events there which itself contain plain
                references to this object. Basically, EventQueue and
                EventMultiplexer should have the same lifetime, and since
                this is not possible, both must be destructed in a
                phased mode: first clear both of any remaining events,
                then destruct them.
             */
            EventMultiplexer( EventQueue& );
            ~EventMultiplexer();


            // Management methods
            // =========================================================

            /** Add a view the show is displayed upon.

                This method adds another view, which the show is
                displayed on. On every added view, the EventMultiplexer
                registers mouse and motion event listeners.
             */
            bool addView( const UnoViewSharedPtr& rView );

            /** Remove a view the show was displayed upon.

                This method removes a view. Registered mouse and
                motion event listeners are revoked.
             */
            bool removeView( const UnoViewSharedPtr& rView );

            /** Clear all registered handlers.
             */
            void clear();

            // TODO(Q3): This is a wart. Remove from this class!
            // Probably, there should exist a separate ViewContainer,
            // which would expose such a method.
            /**
             */
            void setMouseCursor( sal_Int16 );

            // TODO(Q3): This is a wart. Remove from this class!
            // Probably, there should exist a separate ViewContainer,
            // which would expose such a method.
            /** Set volatile mouse cursor.

                A volatile mouse cursor only stays in effect, as long
                as the mouse has not moved. As soon as the mouse
                moves, the mouse cursor is set back to the value set
                by setMouseCursor().
             */
            void setVolatileMouseCursor( sal_Int16 );

            /** Set a LayerManager for update() calls.

                A LayerManager set via this method will receive an
                update() call whenever updateScreen() is called on the
                EventMultiplexer. This is handy to avoid multiple
                redraws. Note that every setLayerManager() call will
                overwrite any previously set.
             */
            void setLayerManager( const LayerManagerSharedPtr& rMgr );

            /** Update screen content

                This method updates the screen content, by first
                updating all layers (if setLayerManager() was called
                with a valid layer manager previously), and then
                calling updateScreen() on all registered views.

                @param bForceUpdate
                Force updateScreen() call, even if layer manager has
                no updates pending. Set this parameter to true, if you
                changed screen content or sprites and bypassed the
                layer manager.
             */
            void updateScreenContent( bool bForceUpdate );


            // Automatic mode methods
            // =========================================================

            /** Change automatic mode.

                @param bIsAuto
                When true, events will be fired automatically, not
                only triggered by UI events. When false, auto events
                will quit.
             */
            void setAutomaticMode( bool bIsAuto );

            /** Get automatic mode setting.
             */
            bool getAutomaticMode() const;

            /** Set the timeout for automatic mode.

                @param nTimeout
                Timeout, between end of effect until start of next
                effect.
             */
            void setAutomaticTimeout( double nTimeout );

            /** Get automatic mode timeout value.
             */
            double getAutomaticTimeout() const;



            // Handler registration methods
            // =========================================================

            /** Register an event handler that will be called when the
                user requests the next effect.

                For every nextEffect event, only one of the handlers
                registered here is called. The handlers are considered
                with decreasing priority, i.e. the handler with the
                currently highest priority will be called.

                @param rHandler
                Handler to call when the next slide starts

                @param nPriority
                Priority with which the handlers are called. The
                higher the priority, the earlier this handler will be
                tried.
             */
            void addNextEffectHandler( const EventHandlerSharedPtr& rHandler,
                                       double                       nPriority );
            void removeNextEffectHandler( const EventHandlerSharedPtr& rHandler );

            /** Register an event handler that will be called when the
                slide is just shown.

                Note that <em>all</em> registered handlers will be called
                when the slide start occurs. This is in contrast to
                the mouse events below.

                @param rHandler
                Handler to call when the next slide starts
             */
            void addSlideStartHandler( const EventHandlerSharedPtr& rHandler );
            void removeSlideStartHandler( const EventHandlerSharedPtr& rHandler );

            /** Register an event handler that will be called when the
                slide is about to vanish.

                Note that <em>all</em> registered handlers will be
                called when the slide end occurs. This is in contrast
                to the mouse events below.

                @param rHandler
                Handler to call when the current slide ends
             */
            void addSlideEndHandler( const EventHandlerSharedPtr& rHandler );
            void removeSlideEndHandler( const EventHandlerSharedPtr& rHandler );

            /** Register an event handler that will be called when an
                XAnimationNode starts its active duration.

                Note that <em>all</em> registered handlers will be called
                when the animation start occurs. This is in contrast to
                the mouse events below.

                @param rHandler
                Handler to call when the animation start
             */
            void addAnimationStartHandler( const AnimationEventHandlerSharedPtr&    rHandler );
            void removeAnimationStartHandler( const AnimationEventHandlerSharedPtr& rHandler );

            /** Register an event handler that will be called when an
                XAnimationNode ends its active duration.

                Note that <em>all</em> registered handlers will be called
                when the animation end occurs. This is in contrast to
                the mouse events below.

                @param rHandler
                Handler to call when the animation ends
             */
            void addAnimationEndHandler( const AnimationEventHandlerSharedPtr&      rHandler );
            void removeAnimationEndHandler( const AnimationEventHandlerSharedPtr&   rHandler );

            /** Register an event handler that will be called when the
                main animation sequence of a slide ends its active
                duration.

                Note that <em>all</em> registered handlers will be
                called when the animation end occurs. This is in
                contrast to the mouse events below.

                @param rHandler
                Handler to call when the animation ends
             */
            void addSlideAnimationsEndHandler( const EventHandlerSharedPtr& rHandler );
            void removeSlideAnimationsEndHandler( const EventHandlerSharedPtr&  rHandler );

            /** Register an event handler that will be called when an
                XAudio node's sound stops playing.

                Note that <em>all</em> registered handlers will be
                called when the audio stops. This is in contrast to
                the mouse events below.

                @param rHandler
                Handler to call when the audio stops
             */
            void addAudioStoppedHandler( const AnimationEventHandlerSharedPtr&      rHandler );
            void removeAudioStoppedHandler( const AnimationEventHandlerSharedPtr&   rHandler );

            /** Register an event handler that will be called when an
                XCommand node's with the command STOPAUDIO is activated.

                Note that <em>all</em> registered handlers will be
                called when the audio stops. This is in contrast to
                the mouse events below.

                @param rHandler
                Handler to call when command is activated
             */
            void addCommandStopAudioHandler( const AnimationEventHandlerSharedPtr&      rHandler );
            void removeCommandStopAudioHandler( const AnimationEventHandlerSharedPtr&   rHandler );

            /** Register a handler that is called when the show enters
                or exits pause mode.
             */
            void addPauseHandler( const PauseEventHandlerSharedPtr& rHandler );
            void removePauseHandler( const PauseEventHandlerSharedPtr& rHandler );

            /** Register a mouse handler that is called on mouse click

                For every mouse click, only one of the handlers
                registered here is called. The handlers are considered
                with decreasing priority, i.e. the handler with the
                currently highest priority will be called.

                Since the handlers can reject down and up events
                individually, handlers should expect to be called with
                non-matching down and up-press counts. If your handler
                cannot cope with that, it must have the highest
                priority of all added handlers.
             */
            void addClickHandler( const MouseEventHandlerSharedPtr& rHandler,
                                  double                            nPriority );
            void removeClickHandler( const MouseEventHandlerSharedPtr&  rHandler );

            /** Register a mouse handler that is called on a double
                mouse click

                For every mouse double click, only one of the handlers
                registered here is called. The handlers are considered
                with decreasing priority, i.e. the handler with the
                currently highest priority will be called.

                Since the handlers can reject down and up events
                individually, handlers should expect to be called with
                non-matching down and up-press counts. If your handler
                cannot cope with that, it must have the highest
                priority of all added handlers.
             */
            void addDoubleClickHandler( const MouseEventHandlerSharedPtr&   rHandler,
                                        double                              nPriority );
            void removeDoubleClickHandler( const MouseEventHandlerSharedPtr&    rHandler );

            /** Register a mouse handler that is called for mouse moves.

                For every mouse move, only one of the handlers
                registered here is called. The handlers are considered
                with decreasing priority, i.e. the handler with the
                currently highest priority will be called.
             */
            void addMouseMoveHandler( const MouseEventHandlerSharedPtr& rHandler,
                                      double                            nPriority );
            void removeMouseMoveHandler( const MouseEventHandlerSharedPtr&  rHandler );


            // External event notifications
            // =========================================================

            /** Notify that the user requested the next effect.

                This requests the slideshow to display the next
                effect, or move to the next slide, if none are left.

                @return true, if this event was processed by
                anybody. If false is returned, no handler processed
                this event (and probably, nothing will happen at all)
             */
            bool notifyNextEffect();

            /** Notify that a new slide has started

                This method is to be used from the Presentation object
                to signal that a new slide is starting now. This will
                invoke all registered slide start handlers.

                @return true, if this event was processed by
                anybody. If false is returned, no handler processed
                this event (and probably, nothing will happen at all)
             */
            bool notifySlideStartEvent();

            /** Notify that a slide has ended

                This method is to be used from the Presentation object
                to signal that a slide is ending now. This will invoke
                all registered slide end handlers.

                @return true, if this event was processed by
                anybody. If false is returned, no handler processed
                this event (and probably, nothing will happen at all)
             */
            bool notifySlideEndEvent();

            /** Notify that the given node enters its active duration.

                This method is to be used from the AnimationNode
                objects to signal that the active duration
                begins. This will invoke all registered animation
                start handlers.

                @param rNode
                Node which enters active duration.

                @return true, if this event was processed by
                anybody. If false is returned, no handler processed
                this event (and probably, nothing will happen at all)
             */
            bool notifyAnimationStart( const AnimationNodeSharedPtr& rNode );

            /** Notify that the given node leaves its active duration.

                This method is to be used from the AnimationNode
                objects to signal that the active duration
                ends now. This will invoke all registered animation
                end handlers.

                @param rNode
                Node which leaves active duration.

                @return true, if this event was processed by
                anybody. If false is returned, no handler processed
                this event (and probably, nothing will happen at all)
             */
            bool notifyAnimationEnd( const AnimationNodeSharedPtr& rNode );

            /** Notify that the slide animations sequence leaves its
                active duration.

                @return true, if this event was processed by
                anybody. If false is returned, no handler processed
                this event (and probably, nothing will happen at all)
             */
            bool notifySlideAnimationsEnd();

            /** Notify that for the given node, audio output has stopped.

                This method is to be used from the AnimationNode
                objects to signal that audio playback has just
                stopped.  This will invoke all registered audio
                stopped andlers.

                @param rNode
                Node for which audio has stopped.

                @return true, if this event was processed by
                anybody. If false is returned, no handler processed
                this event (and probably, nothing will happen at all)
             */
            bool notifyAudioStopped( const AnimationNodeSharedPtr& rNode );

            /** Notify that the show has entered or exited pause mode

                This method is to be used from the Presentation object
                to signal that a slide is entering (bPauseShow=true)
                or exiting (bPauseShow=false) pause mode. This will
                invoke all registered slide end handlers.

                @return true, if this event was processed by
                anybody. If false is returned, no handler processed
                this event (and probably, nothing will happen at all)
             */
            bool notifyPauseMode( bool bPauseShow );

            /** Notify that all audio has to be stoped.

                This method is used by XCommand nodes and all sound
                playing nodes should listen for this command and
                stop theire sounds when its fired. */
            void notifyCommandStopAudio( const AnimationNodeSharedPtr& rNode );

        public:
            typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::awt::XMouseListener,
                                                       ::com::sun::star::awt::XMouseMotionListener >  Listener_UnoBase;

            /** Listener class

                Since EventMultiplexer is held by value from clients,
                had to factor out the UNO interface listener
                functionality (since UNO is ref-counted only)

                All methods are forwarded to from EventMultiplexer. See
                docs there.
             */
            class Listener : public ::comphelper::OBaseMutex,
                             public Listener_UnoBase
            {
            public:
                Listener( EventQueue& );

                // parent method implementations
                bool addView( const UnoViewSharedPtr& rView );
                bool removeView( const UnoViewSharedPtr& rView );
                void clear();

                void setMouseCursor( sal_Int16 );
                void setVolatileMouseCursor( sal_Int16 );

                void setLayerManager( const LayerManagerSharedPtr& rMgr );
                void updateScreenContent( bool bForceUpdate );

                void setAutomaticMode( bool bIsAuto );
                bool getAutomaticMode() const;
                void setAutomaticTimeout( double nTimeout );
                double getAutomaticTimeout() const;

                void addNextEffectHandler       ( const EventHandlerSharedPtr&  rHandler,
                                                  double                        nPriority );
                void removeNextEffectHandler    ( const EventHandlerSharedPtr&  rHandler );
                void addSlideStartHandler       ( const EventHandlerSharedPtr&  rEvent );
                void removeSlideStartHandler    ( const EventHandlerSharedPtr&  rEvent );
                void addSlideEndHandler         ( const EventHandlerSharedPtr&  rEvent );
                void removeSlideEndHandler      ( const EventHandlerSharedPtr&  rEvent );
                void addAnimationStartHandler   ( const AnimationEventHandlerSharedPtr& rHandler );
                void removeAnimationStartHandler( const AnimationEventHandlerSharedPtr& rHandler );
                void addAnimationEndHandler     ( const AnimationEventHandlerSharedPtr& rHandler );
                void removeAnimationEndHandler  ( const AnimationEventHandlerSharedPtr& rHandler );
                void addSlideAnimationsEndHandler   ( const EventHandlerSharedPtr&      rHandler );
                void removeSlideAnimationsEndHandler( const EventHandlerSharedPtr&      rHandler );
                void addAudioStoppedHandler     ( const AnimationEventHandlerSharedPtr& rHandler );
                void removeAudioStoppedHandler  ( const AnimationEventHandlerSharedPtr& rHandler );
                void addCommandStopAudioHandler     ( const AnimationEventHandlerSharedPtr& rHandler );
                void removeCommandStopAudioHandler  ( const AnimationEventHandlerSharedPtr& rHandler );
                void addPauseHandler            ( const PauseEventHandlerSharedPtr& rHandler );
                void removePauseHandler         ( const PauseEventHandlerSharedPtr& rHandler );
                void addClickHandler            ( const MouseEventHandlerSharedPtr& rHandler,
                                                  double                            nPriority );
                void removeClickHandler         ( const MouseEventHandlerSharedPtr& rHandler );
                void addDoubleClickHandler      ( const MouseEventHandlerSharedPtr& rHandler,
                                                  double                            nPriority );
                void removeDoubleClickHandler   ( const MouseEventHandlerSharedPtr& rHandler );
                void addMouseMoveHandler        ( const MouseEventHandlerSharedPtr& rHandler,
                                                  double                            nPriority );
                void removeMouseMoveHandler     ( const MouseEventHandlerSharedPtr& rHandler );

                bool notifyNextEffect           ();
                bool notifySlideStartEvent      ();
                bool notifySlideEndEvent        ();
                bool notifyAnimationStart       ( const AnimationNodeSharedPtr& rNode );
                bool notifyAnimationEnd         ( const AnimationNodeSharedPtr& rNode );
                bool notifySlideAnimationsEnd   ();
                bool notifyAudioStopped         ( const AnimationNodeSharedPtr& rNode );
                bool notifyPauseMode            ( bool bPauseShow );
                void notifyCommandStopAudio     ( const AnimationNodeSharedPtr& rNode );

                // internal method implementations
                virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

                // XMouseListener implementation
                virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
                virtual void SAL_CALL mousePressed( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
                virtual void SAL_CALL mouseReleased( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
                virtual void SAL_CALL mouseEntered( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
                virtual void SAL_CALL mouseExited( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);

                // XMouseMotionListener implementation
                virtual void SAL_CALL mouseDragged( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
                virtual void SAL_CALL mouseMoved( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);

            private:
                // actual handler implementations (the UNO interface
                // overrides above are only stubs)
                void implMousePressed( const ::com::sun::star::awt::MouseEvent& e );
                void implMouseReleased( const ::com::sun::star::awt::MouseEvent& e );
                void implMouseDragged( const ::com::sun::star::awt::MouseEvent& e );
                void implMouseMoved( const ::com::sun::star::awt::MouseEvent& e );

                bool isMouseListenerRegistered() const;

                struct MouseEventHandlerEntry
                {
                    MouseEventHandlerEntry( const MouseEventHandlerSharedPtr&   rHandler,
                                            double                              nPriority ) :
                        mpHandler( rHandler ),
                        mnPriority( nPriority )
                    {
                    }

                    MouseEventHandlerSharedPtr  mpHandler;
                    double                      mnPriority;

                    /** To facilitate bind access.

                        Note: though this should normally be a return
                        by value, current gcc chokes on that in the
                        nested bind case (see EventMultiplexer.cxx,
                        last statement in the processHandlers method
                     */
                    const MouseEventHandlerSharedPtr& getHandler() const
                    {
                        return mpHandler;
                    }

                    /// To sort according to priority
                    bool operator<( const MouseEventHandlerEntry& rRHS ) const
                    {
                        // reversed order - high prioritized entries
                        // should be at the beginning of the queue
                        return mnPriority > rRHS.mnPriority;
                    }

                    /// To permit ::std::remove in removeHandler template
                    bool operator==( const MouseEventHandlerSharedPtr& rRHS ) const
                    {
                        return mpHandler == rRHS;
                    }
                };

                typedef ::std::vector< EventHandlerSharedPtr >          ImplEventHandlers;
                typedef ::std::vector< AnimationEventHandlerSharedPtr > ImplAnimationHandlers;
                typedef ::std::vector< PauseEventHandlerSharedPtr >     ImplPauseHandlers;
                typedef ::std::vector< MouseEventHandlerEntry >         ImplMouseHandlers;

                template< typename Container, typename Handler >
                    void addHandler( Container&                             rContainer,
                                     const ::boost::shared_ptr< Handler >&  rHandler );

                template< typename Container, typename Handler >
                    void removeHandler( Container&                              rContainer,
                                        const ::boost::shared_ptr< Handler >&   rHandler );

                template< typename Function > void forEachView( Function        pViewMethod );

                template< typename RegisterFunction >
                    void addMouseHandler( ImplMouseHandlers&                rHandlerContainer,
                                          const MouseEventHandlerSharedPtr& rHandler,
                                          double                            nPriority,
                                          RegisterFunction                  pRegisterListener );

                bool notifyHandlers( const EventMultiplexer::Listener::ImplEventHandlers& rHandlerContainer );

                bool notifyHandlers( const EventMultiplexer::Listener::ImplAnimationHandlers&   rHandlerContainer,
                                     const AnimationNodeSharedPtr&                              rNode );

                bool notifyHandlers( const EventMultiplexer::Listener::ImplMouseHandlers&   rQueue,
                                     bool (MouseEventHandler::*pHandlerMethod)( const ::com::sun::star::awt::MouseEvent& ),
                                     const ::com::sun::star::awt::MouseEvent&               e );

                /// Called for automatic nextEffect
                void tick();

                /// Schedules a tick event
                void scheduleTick();

                /// Schedules tick events, if mbIsAutoMode is true
                void handleTicks();

                void implSetMouseCursor( sal_Int16 ) const;

                EventQueue&                 mrEventQueue;
                UnoViewVector               maViews;

                ImplEventHandlers           maNextEffectHandlers;
                ImplEventHandlers           maSlideStartHandlers;
                ImplEventHandlers           maSlideEndHandlers;
                ImplAnimationHandlers       maAnimationStartHandlers;
                ImplAnimationHandlers       maAnimationEndHandlers;
                ImplEventHandlers           maSlideAnimationsEndHandlers;
                ImplAnimationHandlers       maAudioStoppedHandlers;
                ImplAnimationHandlers       maCommandStopAudioHandlers;
                ImplPauseHandlers           maPauseHandlers;
                ImplMouseHandlers           maMouseClickHandlers;
                ImplMouseHandlers           maMouseDoubleClickHandlers;
                ImplMouseHandlers           maMouseMoveHandlers;

                LayerManagerSharedPtr       mpLayerManager; // for screen updates

                double                      mnTimeout;

                /** Holds ptr to optional tick event weakly

                    When event queue is cleansed, the next
                    setAutomaticMode(true) call is then able to
                    regenerate the event.
                 */
                ::boost::weak_ptr< Event >  mpTickEvent;

                sal_Int16                   mnMouseCursor;
                sal_Int16                   mnVolatileMouseCursor;
                sal_Int16                   mnLastVolatileMouseCursor;

                bool                        mbIsAutoMode;
            };


        private:
            // default: disabled copy/assignment
            EventMultiplexer(const EventMultiplexer&);
            EventMultiplexer& operator=( const EventMultiplexer& );

            typedef ::rtl::Reference< Listener >    ListenerRef;

            // Ref-counted Listener
            ListenerRef         mpListener;
        };
    }
}

#endif /* _SLIDESHOW_EVENTMULTIPLEXER_HXX */
