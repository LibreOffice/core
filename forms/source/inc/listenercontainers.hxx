/*************************************************************************
 *
 *  $RCSfile: listenercontainers.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-10-22 11:40:31 $
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
        virtual bool    implNotify(
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
        virtual bool    implNotify(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& _rxListener,
                            const ::com::sun::star::lang::EventObject& _rEvent
                        )   SAL_THROW( ( ::com::sun::star::uno::Exception ) );
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_INC_LISTENERCONTAINERS_HXX

