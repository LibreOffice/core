/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: listenercontainers.hxx,v $
 * $Revision: 1.5 $
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

#ifndef FORMS_SOURCE_INC_LISTENERCONTAINERS_HXX
#define FORMS_SOURCE_INC_LISTENERCONTAINERS_HXX

/** === begin UNO includes === **/
#include <com/sun/star/form/XResetListener.hpp>
/** === end UNO includes === **/
#include <comphelper/listenernotification.hxx>
#include <cppuhelper/weak.hxx>

//........................................................................
namespace frm
{
//........................................................................

    //=====================================================================
    //= EventListeners
    //=====================================================================
    template < class LISTENER >
    class EventListeners : public ::comphelper::OListenerContainerBase< LISTENER, ::com::sun::star::lang::EventObject >
    {
    public:
        typedef LISTENER                            ListenerClass;
        typedef ::com::sun::star::lang::EventObject EventClass;
        typedef ::comphelper::OListenerContainerBase< ListenerClass, EventClass >
                                                    EventListeners_Base;

    private:
        ::cppu::OWeakObject&    m_rInstigator;

    protected:
        ::cppu::OWeakObject&    getInstigator() { return m_rInstigator; }

    protected:
        inline EventListeners( ::cppu::OWeakObject& _rInstigator, ::osl::Mutex& _rMutex )
            :EventListeners_Base( _rMutex )
            ,m_rInstigator( _rInstigator )
        {
        }

        // still waiting to be overwritten
        virtual bool    implTypedNotify(
                            const ::com::sun::star::uno::Reference< ListenerClass >& _rxListener,
                            const EventClass& _rEvent
                        )   SAL_THROW( ( ::com::sun::star::uno::Exception ) ) = 0;

    public:
        inline bool notify()
        {
            ::com::sun::star::lang::EventObject aEvent( m_rInstigator );
            return EventListeners_Base::notify( aEvent );
        }

        inline void disposing()
        {
            ::com::sun::star::lang::EventObject aEvent( m_rInstigator );
            EventListeners_Base::disposing( aEvent );
        }
    protected:
        using EventListeners_Base::notify;
        using EventListeners_Base::disposing;
    };

    //=====================================================================
    //= ResetListeners
    //=====================================================================
    typedef EventListeners  <   ::com::sun::star::form::XResetListener
                            >   ResetListeners_Base;
    class ResetListeners : public ResetListeners_Base
    {
    private:
        enum NotificationType
        {
            eApproval,
            eFinal
        };
        NotificationType        m_eCurrentNotificationType;

    public:
        inline ResetListeners( ::cppu::OWeakObject& _rInstigator, ::osl::Mutex& _rMutex )
            :ResetListeners_Base( _rInstigator, _rMutex )
            ,m_eCurrentNotificationType( eApproval )
        {
        }

        /** see whether all our listeners approve the reset
        */
        sal_Bool approveReset()
        {
            m_eCurrentNotificationType = eApproval;
            return notify();
        }

        /** tell all our listeners that the reset happened
        */
        void resetted()
        {
            m_eCurrentNotificationType = eFinal;
            notify();
        }

    protected:
        virtual bool    implTypedNotify(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& _rxListener,
                            const ::com::sun::star::lang::EventObject& _rEvent
                        )   SAL_THROW( ( ::com::sun::star::uno::Exception ) );
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_INC_LISTENERCONTAINERS_HXX

