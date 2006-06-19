/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listenercontainers.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 12:57:53 $
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

#ifndef FORMS_SOURCE_INC_LISTENERCONTAINERS_HXX
#define FORMS_SOURCE_INC_LISTENERCONTAINERS_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FORM_XRESETLISTENER_HPP_
#include <com/sun/star/form/XResetListener.hpp>
#endif
/** === end UNO includes === **/

#ifndef COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX
#include <comphelper/listenernotification.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

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

