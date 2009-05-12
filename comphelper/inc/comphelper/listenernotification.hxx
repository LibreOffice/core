/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: listenernotification.hxx,v $
 * $Revision: 1.11 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX
#define COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX

#include <cppuhelper/interfacecontainer.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/lang/XEventListener.hpp>
/** === end UNO includes === **/
#include "comphelper/comphelperdllapi.h"

#include <memory>

//........................................................................
namespace comphelper
{
//........................................................................

    //====================================================================
    //= OListenerContainer
    //====================================================================
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

        See <type>OListenerContainerBase</type> for an implementation which even saves
        you some more work, by doing the casts for you.

        @see http://www.openoffice.org/servlets/ReadMsg?list=interface-announce&msgId=494345
        @see OListenerContainerBase
    */
    class COMPHELPER_DLLPUBLIC OListenerContainer
    {
    private:
        ::cppu::OInterfaceContainerHelper    m_aListeners;

    public:
        /** sends a XEventObject::disposing notification to all listeners, and clears the
            listener container

            You'll usually call this from within your own dispose/disposing method
        */
        void    disposing( const ::com::sun::star::lang::EventObject& _rEventSource );

        /** clears the container without calling <member scope="com::sun::star::lang">XEventListener::disposing</member>
            at the listeners
        */
        void    clear();

        /** determines whether the listener container is currently empty
        */
        inline bool
                empty() const SAL_THROW(());

        /** determines the number of elements in the container
        */
        inline size_t
                size() const SAL_THROW(());

        /** creates an iterator for looping through all registered listeners
        */
        inline ::std::auto_ptr< ::cppu::OInterfaceIteratorHelper >
                createIterator();

    protected:
                OListenerContainer( ::osl::Mutex& _rMutex );

        virtual ~OListenerContainer();

        void    impl_addListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _rxListener );
        void    impl_removeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _rxListener );

        /** notifies all listeners of the given event, using THB's notification pattern

            internally, this method will call <member>implNotify</member> for every listener

            @return
                <TRUE/> if all listeners have been notified, <FALSE/> else. The latter can happen
                if <member>implNotify</member> cancelles the notification loop.

            @see implNotify
        */
        bool    impl_notify( const ::com::sun::star::lang::EventObject& _rEvent ) SAL_THROW(( ::com::sun::star::uno::Exception ));

    protected:
        /** call a single listener

            @pure

            @throws ::com::sun::star::uno::Exception
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
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _rxListener,
                            const ::com::sun::star::lang::EventObject& _rEvent
                        )   SAL_THROW( ( ::com::sun::star::uno::Exception ) ) = 0;
    };

    //====================================================================
    inline bool OListenerContainer::empty() const SAL_THROW(())
    {
        return ( m_aListeners.getLength() == 0 );
    }

    inline size_t OListenerContainer::size() const SAL_THROW(())
    {
        return m_aListeners.getLength();
    }

    inline ::std::auto_ptr< ::cppu::OInterfaceIteratorHelper > OListenerContainer::createIterator()
    {
        ::std::auto_ptr< ::cppu::OInterfaceIteratorHelper > pIterator( new ::cppu::OInterfaceIteratorHelper( m_aListeners ) );
        return pIterator;
    }

    //====================================================================
    //= OSimpleListenerContainer
    //====================================================================
    /** helper class for simple notification of the form LISTENER::METHOD( EVENT )

        This class is not threadsafe!

        @param LISTENER
            the listener class to call, e.g. <type scope="com::sun::star::lang">XEventListener</type>
        @param EVENT
            the event type to notify, e.g. <type scope="com::sun::star::lang">EventObject</type>
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
            ,m_pNotificationMethod( NULL )
        {
        }

        inline void addListener( const ::com::sun::star::uno::Reference< ListenerClass >& _rxListener )
        {
            OListenerContainer::impl_addListener( _rxListener.get() );
        }

        inline void removeListener( const ::com::sun::star::uno::Reference< ListenerClass >& _rxListener )
        {
            OListenerContainer::impl_removeListener( _rxListener.get() );
        }

        // publish some otherwise hidden base functionality
        using OListenerContainer::disposing;
        using OListenerContainer::clear;
        using OListenerContainer::empty;
        using OListenerContainer::size;
        using OListenerContainer::createIterator;

        /// typed notification
        inline bool    notify( const EventClass& _rEvent, NotificationMethod _pNotify ) SAL_THROW(( ::com::sun::star::uno::Exception ));

    protected:
        inline virtual bool    implNotify(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _rxListener,
                            const ::com::sun::star::lang::EventObject& _rEvent
                        )   SAL_THROW( ( ::com::sun::star::uno::Exception ) );
    };

    //--------------------------------------------------------------------
    template< class LISTENER, class EVENT >
    inline bool OSimpleListenerContainer< LISTENER, EVENT >::notify( const EventClass& _rEvent, NotificationMethod _pNotify ) SAL_THROW(( ::com::sun::star::uno::Exception ))
    {
        m_pNotificationMethod = _pNotify;
        bool bRet = OListenerContainer::impl_notify( _rEvent );
        m_pNotificationMethod = NULL;
        return bRet;
    }

    //--------------------------------------------------------------------
    template< class LISTENER, class EVENT >
    inline bool OSimpleListenerContainer< LISTENER, EVENT >::implNotify(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _rxListener,
            const ::com::sun::star::lang::EventObject& _rEvent )   SAL_THROW( ( ::com::sun::star::uno::Exception ) )
    {
        const EventClass& rTypedEvent( static_cast< const EventClass& >( _rEvent ) );
        ListenerClass* pTypedListener( static_cast< ListenerClass* >( _rxListener.get() ) );
        (pTypedListener->*m_pNotificationMethod)( rTypedEvent );
        return true;
    }

    //====================================================================
    //= OListenerContainerBase
    //====================================================================
    /** is a specialization of OListenerContainer which saves you some additional type casts,
        by making the required listener and event types template arguments.
    */
    template< class LISTENER, class EVENT >
    class OListenerContainerBase : public OListenerContainer
    {
    public:
        typedef LISTENER    ListenerClass;
        typedef EVENT       EventClass;

    public:
        inline OListenerContainerBase( ::osl::Mutex& _rMutex ) : OListenerContainer( _rMutex )
        {
        }

        inline void addTypedListener( const ::com::sun::star::uno::Reference< ListenerClass >& _rxListener )
        {
            OListenerContainer::impl_addListener( _rxListener.get() );
        }

        inline void removeTypedListener( const ::com::sun::star::uno::Reference< ListenerClass >& _rxListener )
        {
            OListenerContainer::impl_removeListener( _rxListener.get() );
        }

        inline bool notify( const EventClass& _rEvent )
        {
            return OListenerContainer::impl_notify( _rEvent );
        }

        using OListenerContainer::impl_notify;

    protected:
        inline virtual bool    implNotify(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _rxListener,
                            const ::com::sun::star::lang::EventObject& _rEvent
                        )   SAL_THROW( ( ::com::sun::star::uno::Exception ) );

        virtual bool    implTypedNotify(
                            const ::com::sun::star::uno::Reference< ListenerClass >& _rxListener,
                            const EventClass& _rEvent
                        )   SAL_THROW( ( ::com::sun::star::uno::Exception ) ) = 0;
    };

    template< class LISTENER, class EVENT >
    inline bool OListenerContainerBase< LISTENER, EVENT >::implNotify(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _rxListener,
            const ::com::sun::star::lang::EventObject& _rEvent )   SAL_THROW( ( ::com::sun::star::uno::Exception ) )
    {
        return implTypedNotify(
                    ::com::sun::star::uno::Reference< ListenerClass >( static_cast< ListenerClass* >( _rxListener.get() ) ),
                    static_cast< const EventClass& >( _rEvent )
        );
    }

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX

