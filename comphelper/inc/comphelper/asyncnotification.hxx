/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef COMPHELPER_ASYNCNOTIFICATION_HXX
#define COMPHELPER_ASYNCNOTIFICATION_HXX

#include <osl/thread.hxx>
#include <rtl/ref.hxx>
#include <comphelper/comphelperdllapi.h>
#include <rtl/alloc.h>

#include <memory>

//........................................................................
namespace comphelper
{
//........................................................................

    //====================================================================
    //= AnyEvent
    //====================================================================
    /** the very basic instance to hold a description of an event
    */
    class COMPHELPER_DLLPUBLIC AnyEvent : ::rtl::IReference
    {
    private:
        oslInterlockedCount m_refCount;

    public:
        AnyEvent();

        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    protected:
        virtual ~AnyEvent();

    private:
        AnyEvent( AnyEvent& ); // not defined
        void operator=( AnyEvent& ); // not defined
    };

    //====================================================================
    //= typedefs
    //====================================================================
    typedef ::rtl::Reference< AnyEvent >    AnyEventRef;

    //====================================================================
    //= IEventProcessor
    //====================================================================
    /** an event processor

        @see AsyncEventNotifier
    */
    class SAL_NO_VTABLE IEventProcessor
    {
    public:
        /** process a single event
        */
        virtual void processEvent( const AnyEvent& _rEvent ) = 0;

        virtual void SAL_CALL acquire() = 0;
        virtual void SAL_CALL release() = 0;
    };

    //====================================================================
    //= AsyncEventNotifier
    //====================================================================
    typedef ::osl::Thread  AsyncEventNotifier_TBASE;
    struct EventNotifierImpl;

    /** a helper class for notifying events asynchronously

        If you need to notify certain events to external components, you usually should
        not do this while you have mutexes locked, to prevent multi-threading issues.

        However, you do not always have complete control over all mutex guards on the stack.
        If, in such a case, the listener notification is one-way, you can decide to do it
        asynchronously.

        The ->AsyncEventNotifier helps you to process such events asynchronously. Every
        event is tied to an ->IEventProcessor which is responsible for processing it.

        The AsyncEventNotifier is implemented as a thread itself, which sleeps as long as there are no
        events in the queue. As soon as you add an event, the thread is woken up, processes the event,
        and sleeps again.
    */
    class COMPHELPER_DLLPUBLIC AsyncEventNotifier   :protected AsyncEventNotifier_TBASE
                                                    ,public ::rtl::IReference
    {
        friend struct EventNotifierImpl;

    private:
        ::std::auto_ptr< EventNotifierImpl >        m_pImpl;

    protected:
        // Thread
        virtual void SAL_CALL run();
        virtual void SAL_CALL onTerminated();

    public:
        /** constructs a notifier thread
        */
        AsyncEventNotifier();

        // IReference implementations
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

        using AsyncEventNotifier_TBASE::create;
        using AsyncEventNotifier_TBASE::join;
        using AsyncEventNotifier_TBASE::getIdentifier;

        using AsyncEventNotifier_TBASE::operator new;
        using AsyncEventNotifier_TBASE::operator delete;

        /** terminates the thread

            Note that this is a cooporative termination - if you call this from a thread different
            from the notification thread itself, then it will block until the notification thread
            finished processing the current event. If you call it from the notification thread
            itself, it will return immediately, and the thread will be terminated as soon as
            the current notification is finished.
        */
        virtual void SAL_CALL terminate();

        /** adds an event to the queue, together with the instance which is responsible for
            processing it

            @param _rEvent
                the event to add to the queue
            @param _xProcessor
                the processor for the event.<br/>
                Beware of life time issues here. If your event processor dies or becomes otherwise
                nonfunctional, you are responsible for removing all respective events from the queue.
                You can do this by calling ->removeEventsForProcessor
        */
        void addEvent( const AnyEventRef& _rEvent, const ::rtl::Reference< IEventProcessor >& _xProcessor );

        /** removes all events for the given event processor from the queue
        */
        void removeEventsForProcessor( const ::rtl::Reference< IEventProcessor >& _xProcessor );

    protected:
        virtual ~AsyncEventNotifier();
    };

    //====================================================================
    //= EventHolder
    //====================================================================
    /** AnyEvent derivee holding an foreign event instance
    */
    template < typename EVENT_OBJECT >
    class EventHolder : public AnyEvent
    {
    public:
        typedef EVENT_OBJECT    EventObjectType;

    private:
        EventObjectType m_aEvent;

    public:
        inline EventHolder( const EventObjectType& _rEvent )
            :m_aEvent( _rEvent )
        {
        }

        inline const EventObjectType& getEventObject() const { return m_aEvent; }
    };

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_ASYNCNOTIFICATION_HXX
