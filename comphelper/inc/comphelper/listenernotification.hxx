/*************************************************************************
 *
 *  $RCSfile: listenernotification.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-10-22 12:46:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX
#define COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
/** === end UNO includes === **/

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
            <li>Use <member>addListener</member> and <member>removeListener</member> in your
                XFoo::addFooListener and XFoo::removeFooListener methods.</li>
            <li>call <member>notify</member> whenever the event you want to notify happened</li>
            <li>call <member>disposing</member> upon the disposal of your broadcaster.</li>
        </ul>

        See <type>OListenerContainerBase</type> for an implementation which even saves
        you some more work, by doing the casts for you.

        @see http://www.openoffice.org/servlets/ReadMsg?list=interface-announce&msgId=494345
        @see OListenerContainerBase
    */
    class OListenerContainer
    {
    private:
        ::cppu::OInterfaceContainerHelper    m_aListeners;

    public:
        /** sends a XEventObject::disposing notification to all listeners, and clears the
            listener container

            You'll usually call this from within your own dispose/disposing method
        */
        void    disposing( const ::com::sun::star::lang::EventObject& _rEventSource );

        /** determines whether the listener container is currently empty
        */
        inline bool
                empty() const SAL_THROW(());

        /** determines the number of elements in the container
        */
        inline size_t
                size() const SAL_THROW(());

    protected:
                OListenerContainer( ::osl::Mutex& _rMutex );

        void    addListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _rxListener );
        void    removeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _rxListener );

        /** notifies all listeners of the given event, using THB's notification pattern

            internally, this method will call <member>implNotify</member> for every listener

            @return
                <TRUE/> if all listeners have been notified, <FALSE/> else. The latter can happen
                if <member>implNotify</member> cancelles the notification loop.

            @see implNotify
        */
        bool    notify( const ::com::sun::star::lang::EventObject& _rEvent ) SAL_THROW(( ::com::sun::star::uno::Exception ));

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

        inline void addListener( const ::com::sun::star::uno::Reference< ListenerClass >& _rxListener )
        {
            OListenerContainer::addListener( _rxListener.get() );
        }

        inline void removeListener( const ::com::sun::star::uno::Reference< ListenerClass >& _rxListener )
        {
            OListenerContainer::removeListener( _rxListener.get() );
        }

        inline bool notify( const EventClass& _rEvent )
        {
            return OListenerContainer::notify( _rEvent );
        }

    protected:
        virtual bool    implNotify(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _rxListener,
                            const ::com::sun::star::lang::EventObject& _rEvent
                        )   SAL_THROW( ( ::com::sun::star::uno::Exception ) );

        virtual bool    implNotify(
                            const ::com::sun::star::uno::Reference< ListenerClass >& _rxListener,
                            const EventClass& _rEvent
                        )   SAL_THROW( ( ::com::sun::star::uno::Exception ) ) = 0;
    };

    template< class LISTENER, class EVENT >
    bool OListenerContainerBase< LISTENER, EVENT >::implNotify(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _rxListener,
            const ::com::sun::star::lang::EventObject& _rEvent )   SAL_THROW( ( ::com::sun::star::uno::Exception ) )
    {
        return implNotify(
                    ::com::sun::star::uno::Reference< ListenerClass >( static_cast< ListenerClass* >( _rxListener.get() ) ),
                    static_cast< const EventClass& >( _rEvent )
        );
    }

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX

