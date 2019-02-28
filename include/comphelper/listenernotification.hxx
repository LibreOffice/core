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

#ifndef INCLUDED_COMPHELPER_LISTENERNOTIFICATION_HXX
#define INCLUDED_COMPHELPER_LISTENERNOTIFICATION_HXX

#include <comphelper/interfacecontainer2.hxx>

#include <com/sun/star/lang/EventObject.hpp>
#include <comphelper/comphelperdllapi.h>

#include <memory>

namespace com { namespace sun { namespace star { namespace lang { class XEventListener; } } } }

namespace comphelper
{


    //= OListenerContainer

    /** abstract base class which manages a listener container, including
        THB's listener notification pattern which cares for removing listeners
        which throw an DisposedException upon notification

        Using this class is pretty easy:
        <ul>
            <li>Derive from it, and overwrite implNotify.</li>
            <li>Use <member>impl_addListener</member> and <member>impl_removeListener</member> in your
                XFoo::addFooListener and XFoo::removeFooListener methods.</li>
            <li>call <member>impl_notify</member> whenever the event you want to notify happened</li>
            <li>call <member>disposing</member> upon the disposal of your broadcaster.</li>
        </ul>

        See OListenerContainerBase for an implementation which even saves
        you some more work, by doing the casts for you.

        @see http://www.openoffice.org/servlets/ReadMsg?list=interface-announce&msgId=494345
        @see OListenerContainerBase
    */
    class COMPHELPER_DLLPUBLIC OListenerContainer
    {
    private:
        ::comphelper::OInterfaceContainerHelper2    m_aListeners;

    public:
        /** sends a XEventObject::disposing notification to all listeners, and clears the
            listener container

            You'll usually call this from within your own dispose/disposing method
        */
        void    disposing( const css::lang::EventObject& _rEventSource );

        /** clears the container without calling <member scope="css::lang">XEventListener::disposing</member>
            at the listeners
        */
        void    clear();

        /** determines whether the listener container is currently empty
        */
        inline bool
                empty() const;

        /** creates an iterator for looping through all registered listeners
        */
        ::std::unique_ptr< ::comphelper::OInterfaceIteratorHelper2 > createIterator()
        {
            ::std::unique_ptr< ::comphelper::OInterfaceIteratorHelper2 > pIterator( new ::comphelper::OInterfaceIteratorHelper2( m_aListeners ) );
            return pIterator;
        }

    protected:
                OListenerContainer( ::osl::Mutex& _rMutex );

        virtual ~OListenerContainer();

        void    impl_addListener( const css::uno::Reference< css::lang::XEventListener >& _rxListener );
        void    impl_removeListener( const css::uno::Reference< css::lang::XEventListener >& _rxListener );

        /** notifies all listeners of the given event, using THB's notification pattern

            internally, this method will call <member>implNotify</member> for every listener

            @return
                <TRUE/> if all listeners have been notified, <FALSE/> else. The latter can happen
                if <member>implNotify</member> cancels the notification loop.

            @see implNotify
        */
        bool    impl_notify( const css::lang::EventObject& _rEvent );

    protected:
        /** call a single listener

            @pure

            @throws css::uno::Exception
                if the listener throws an exception during notification. Please don't catch
                any listener exceptions in your implementation of this method, simply let them
                pass to the caller.

            @param _rxListener
                specifies the listener to call. Is guaranteed to not be <NULL/>
            @param _rEvent
                the event to broadcast. This is the same as passed to <member>notify</member>, so if
                your base class knows the type passed into <member>notify</member>, it can safely assume
                that <arg>_rEvent</arg> is also of this type.

            @return
                <TRUE/> if the remaining listeners should be called, <FALSE/> if the notification
                loop should be cancelled

            @see notify
        */
        virtual bool    implNotify(
                            const css::uno::Reference< css::lang::XEventListener >& _rxListener,
                            const css::lang::EventObject& _rEvent
                        ) = 0;
    };


    inline bool OListenerContainer::empty() const
    {
        return ( m_aListeners.getLength() == 0 );
    }


    //= OSimpleListenerContainer

    /** helper class for simple notification of the form LISTENER::METHOD( EVENT )

        This class is not threadsafe!

        @param LISTENER
            the listener class to call, e.g. css::lang::XEventListener
        @param EVENT
            the event type to notify, e.g. css::lang::EventObject
    */
    template< class LISTENER, class EVENT >
    class OSimpleListenerContainer : protected OListenerContainer
    {
    public:
        typedef LISTENER    ListenerClass;
        typedef EVENT       EventClass;
        typedef void ( SAL_CALL LISTENER::*NotificationMethod )( const EventClass& );

    private:
        NotificationMethod  m_pNotificationMethod;

    public:
        OSimpleListenerContainer( ::osl::Mutex& _rMutex )
            :OListenerContainer( _rMutex )
            ,m_pNotificationMethod( nullptr )
        {
        }

        void addListener( const css::uno::Reference< ListenerClass >& _rxListener )
        {
            OListenerContainer::impl_addListener( _rxListener.get() );
        }

        void removeListener( const css::uno::Reference< ListenerClass >& _rxListener )
        {
            OListenerContainer::impl_removeListener( _rxListener.get() );
        }

        // publish some otherwise hidden base functionality
        using OListenerContainer::disposing;
        using OListenerContainer::clear;
        using OListenerContainer::empty;
        using OListenerContainer::createIterator;

        /// typed notification
        inline void    notify( const EventClass& _rEvent, NotificationMethod _pNotify );

    protected:
        virtual bool    implNotify(
                            const css::uno::Reference< css::lang::XEventListener >& _rxListener,
                            const css::lang::EventObject& _rEvent
                        ) override
        {
            const EventClass& rTypedEvent( static_cast< const EventClass& >( _rEvent ) );
            ListenerClass* pTypedListener( static_cast< ListenerClass* >( _rxListener.get() ) );
            (pTypedListener->*m_pNotificationMethod)( rTypedEvent );
            return true;
        }
    };


    template< class LISTENER, class EVENT >
    inline void OSimpleListenerContainer< LISTENER, EVENT >::notify( const EventClass& _rEvent, NotificationMethod _pNotify )
    {
        m_pNotificationMethod = _pNotify;
        OListenerContainer::impl_notify( _rEvent );
        m_pNotificationMethod = nullptr;
    }

    //= OListenerContainerBase

    /** is a specialization of OListenerContainer which saves you some additional type casts,
        by making the required listener and event types template arguments.
    */
    template< class ListenerClass, class EventClass >
    class OListenerContainerBase : public OListenerContainer
    {
    public:
        OListenerContainerBase( ::osl::Mutex& _rMutex ) : OListenerContainer( _rMutex )
        {
        }

        void addTypedListener( const css::uno::Reference< ListenerClass >& _rxListener )
        {
            OListenerContainer::impl_addListener( _rxListener.get() );
        }

        void removeTypedListener( const css::uno::Reference< ListenerClass >& _rxListener )
        {
            OListenerContainer::impl_removeListener( _rxListener.get() );
        }

        bool notify( const EventClass& _rEvent )
        {
            return OListenerContainer::impl_notify( _rEvent );
        }

        using OListenerContainer::impl_notify;

    protected:
        virtual bool    implNotify(
                            const css::uno::Reference< css::lang::XEventListener >& _rxListener,
                            const css::lang::EventObject& _rEvent
                        ) override
        {
            return implTypedNotify(
                css::uno::Reference< ListenerClass >( static_cast< ListenerClass* >( _rxListener.get() ) ),
                static_cast< const EventClass& >( _rEvent )
            );
        }

        virtual bool    implTypedNotify(
                            const css::uno::Reference< ListenerClass >& _rxListener,
                            const EventClass& _rEvent
                        ) = 0;
    };

} // namespace comphelper


#endif // INCLUDED_COMPHELPER_LISTENERNOTIFICATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
