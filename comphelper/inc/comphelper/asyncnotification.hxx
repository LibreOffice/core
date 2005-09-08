/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: asyncnotification.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:27:30 $
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

#ifndef COMPHELPER_ASYNCNOTIFICATION_HXX
#define COMPHELPER_ASYNCNOTIFICATION_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XADAPTER_HPP_
#include <com/sun/star/uno/XAdapter.hpp>
#endif
/** === end UNO includes === **/

#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

#include <deque>
#include <memory>

//........................................................................
namespace comphelper
{
//........................................................................

    //====================================================================
    //= EventDescription
    //====================================================================
    /** the very basic instance to hold a description of an event

        In this simple version, an event is described by an numeric value only, whose
        semantics is completely up to the user of this class.

        Normally, you won't use this description directly, but instead derive own
        classes which can hold more information.
    */
    class COMPHELPER_DLLPUBLIC EventDescription : ::rtl::IReference
    {
    public:
        typedef sal_uInt16  EventType;

    private:
        oslInterlockedCount m_refCount;
        EventType           m_nType;

    public:
        EventDescription( EventType nType );

        inline EventType   getEventType() const { return m_nType; }

        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    protected:
        EventDescription( const EventDescription& _rSource );
        EventDescription& operator=( const EventDescription& _rSource );

        virtual ~EventDescription();

    private:
        EventDescription( );        // never implemented
    };

    //====================================================================
    //= EventObjectHolder
    //====================================================================
    /** EventDescription derivee holding an UNO EventObject derivee
    */
    template <typename EVENT_OBJECT >
    class EventObjectHolder : public EventDescription
    {
    public:
        typedef EVENT_OBJECT    EventObjectType;

    private:
        EventObjectType m_aEvent;

    public:
        inline EventObjectHolder( const EventObjectType& _rUnoEvent, EventType _nType )
            :EventDescription( _nType )
            ,m_aEvent( _rUnoEvent )
        {
        }

        inline const EventObjectType& getEventObject() const { return m_aEvent; }
    };

    //====================================================================
    //= typedefs
    //====================================================================
    typedef ::rtl::Reference< EventDescription >    EventDescriptionRef;

    //====================================================================
    //= IEventProcessor
    //====================================================================
    /** an event processor

        @see AsyncEventNotifier
    */
    class IEventProcessor
    {
    public:
        /** process a single event
        */
        virtual void processEvent( const EventDescription& _rEvent ) = 0;

        /** retrieve the component associated with the event processor

            If this method returns a non-<NULL/> object, this can be used to signal
            the ending of the life time of the processor.
            Every instance which hols a pointer to the processor instance is then required
            to register as <type scope="com::sun::star::lang">XEventListener</type> at the
            component, and to <em>not</em> access the event processor anymore after the
            component has notified its disposal.
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
            getComponent() = 0;
    };

    //====================================================================
    //= AsyncEventNotifier
    //====================================================================
    typedef ::vos::OThread                                                      AsyncEventNotifier_TBASE;
    typedef ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >   AsyncEventNotifier_BASE;
    struct EventNotifierImpl;

    /** a helper class for notifying events asynchronously

        If you need to notify certain events to external components, you usually should
        not do this while you have mutexes locked, to prevent multi-threading issues.

        However, you do not always have complete control over all mutex guards on the stack.
        If, in such a case, the listener notification is one-way, you can decide to do it
        asynchronously.

        For this, you need to
        <ul>
            <li>implement the <type>IEventProcessor</type> interface</li>
            <li>instantiate an <type>AsyncEventNotifier</type> instance, and <member>acquire</member>
                and <member>create</member> it</li>
            <li>derive a class from <type>EventDescription</type>, which is able to hold
                all the necessary information about your event</li>
            <li>pass an instance of this derived <type>EventDescription</type> to <member>processEvent</member>
                of your <type>AsyncEventNotifier</type></li>
        </ul>

        Your event processor's <member>IEventProcessor::processEvent</member> method will then be called
        with the given event, in a separate thread where definately no mutexes are locked.

        The AsyncEventNotifier is implemented as a thread itself, which sleeps as long as there are no
        events in the queue. As soon as you add an event, the thread is woken up, processes the event,
        and sleeps again.
    */
    class COMPHELPER_DLLPUBLIC AsyncEventNotifier
                :public AsyncEventNotifier_TBASE
                ,public AsyncEventNotifier_BASE
    {
        friend struct EventNotifierImpl;

    private:
        typedef ::std::deque< EventDescriptionRef > Events;

    private:
        ::std::auto_ptr< EventNotifierImpl >        m_pImpl;

    protected:
        // OThread
        virtual void SAL_CALL run();
        virtual void SAL_CALL kill();
        virtual void SAL_CALL onTerminated();

    public:
        /** constructs a notifier thread

        @param _pProcessor
            the instance processing the events
        */
        AsyncEventNotifier( IEventProcessor* _pProcessor );

        /** adds an event to the queue
        */
        void addEvent( const EventDescriptionRef& _rEvent );

        // XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        /* resolve ambiguity : both OWeakObject and OObject have these memory operators */
        void* SAL_CALL operator new( size_t size ) throw() { return OThread::operator new( size ); }
        void SAL_CALL operator delete( void * p ) throw() { OThread::operator delete( p ); }

    protected:
        virtual ~AsyncEventNotifier();

    private:
        void    implStarted( );
        void    implTerminated( );
    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_ASYNCNOTIFICATION_HXX
