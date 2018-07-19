/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_COMPHELPER_ASYNCNOTIFICATION_HXX
#define INCLUDED_COMPHELPER_ASYNCNOTIFICATION_HXX

#include <sal/config.h>

#include <comphelper/comphelperdllapi.h>
#include <rtl/ref.hxx>
#include <sal/types.h>
#include <salhelper/thread.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <memory>

namespace comphelper
{
    //= AnyEvent

    /** the very basic instance to hold a description of an event
    */
    class COMPHELPER_DLLPUBLIC AnyEvent : public salhelper::SimpleReferenceObject
    {
    public:
        AnyEvent();

    protected:
        virtual ~AnyEvent() override;

    private:
        AnyEvent( AnyEvent& ) = delete;
        void operator=( AnyEvent& ) = delete;
    };


    //= typedefs

    typedef ::rtl::Reference< AnyEvent >    AnyEventRef;


    //= IEventProcessor

    /** an event processor

        @see AsyncEventNotifier
    */
    class SAL_NO_VTABLE IEventProcessor
    {
    public:
        /** process a single event
        */
        virtual void processEvent( const AnyEvent& _rEvent ) = 0;

        virtual void SAL_CALL acquire() throw () = 0;
        virtual void SAL_CALL release() throw () = 0;

    protected:
        ~IEventProcessor() {}
    };


    //= AsyncEventNotifier

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
    class COMPHELPER_DLLPUBLIC AsyncEventNotifierBase
    {
        friend struct EventNotifierImpl;

    protected:
        std::unique_ptr<EventNotifierImpl>        m_xImpl;

        SAL_DLLPRIVATE virtual ~AsyncEventNotifierBase();

        // Thread
        SAL_DLLPRIVATE virtual void execute();

    public:
        AsyncEventNotifierBase();

        /** terminates the thread

            Note that this is a cooperative termination - if you call this from a thread different
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
    };

    /** This class is usable with rtl::Reference.
        As always, the thread must be joined somewhere.
     */
    class COMPHELPER_DLLPUBLIC AsyncEventNotifier
        : public AsyncEventNotifierBase
        , public salhelper::Thread
    {

    private:
        SAL_DLLPRIVATE virtual ~AsyncEventNotifier() override;

        SAL_DLLPRIVATE virtual void execute() override;

    public:
        /** constructs a notifier thread

            @param name the thread name, see ::osl_setThreadName; must not be
            null
        */
        AsyncEventNotifier(char const* name);

        virtual void SAL_CALL terminate() override;
    };

    /** This is a hack (when proper joining is not possible), use of which
        should be avoided by good design.
     */
    class COMPHELPER_DLLPUBLIC AsyncEventNotifierAutoJoin
        : public AsyncEventNotifierBase
        , private osl::Thread
    {

    private:
        SAL_DLLPRIVATE AsyncEventNotifierAutoJoin(char const* name);

        SAL_DLLPRIVATE virtual void SAL_CALL run() override;
        SAL_DLLPRIVATE virtual void SAL_CALL onTerminated() override;

    public:
        // only public so shared_ptr finds it
        SAL_DLLPRIVATE virtual ~AsyncEventNotifierAutoJoin() override;

        static std::shared_ptr<AsyncEventNotifierAutoJoin>
            newAsyncEventNotifierAutoJoin(char const* name);

        virtual void SAL_CALL terminate() override;

        using osl::Thread::join;
        using osl::Thread::operator new;
        using osl::Thread::operator delete; // clang really wants this?

        static void launch(std::shared_ptr<AsyncEventNotifierAutoJoin> const&);
    };


    //= EventHolder

    /** AnyEvent derivee holding an foreign event instance
    */
    template < typename EVENT_OBJECT >
    class SAL_DLLPUBLIC_RTTI EventHolder : public AnyEvent
    {
    public:
        typedef EVENT_OBJECT    EventObjectType;

    private:
        EventObjectType const m_aEvent;

    public:
        EventHolder( const EventObjectType& _rEvent )
            :m_aEvent( _rEvent )
        {
        }

        const EventObjectType& getEventObject() const { return m_aEvent; }
    };

    COMPHELPER_DLLPUBLIC void JoinAsyncEventNotifiers();

} // namespace comphelper


#endif // INCLUDED_COMPHELPER_ASYNCNOTIFICATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
