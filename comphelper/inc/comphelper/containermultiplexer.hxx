/*************************************************************************
 *
 *  $RCSfile: containermultiplexer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 10:18:21 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_CONTAINERMULTIPLEXER_HXX_
#define _COMPHELPER_CONTAINERMULTIPLEXER_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    class OContainerListenerAdapter;
    //=====================================================================
    //= OContainerListener
    //=====================================================================
    /** a non-UNO container listener
        <p>Usefull if you have a non-refcountable class which should act as container listener.<br/>
        In this case, derive this class from OContainerListener, and create an adapter
        <type>OContainerListenerAdapter</type> which multiplexes the changes.</p>
    */
    class COMPHELPER_DLLPUBLIC OContainerListener
    {
        friend class OContainerListenerAdapter;
    protected:
        OContainerListenerAdapter*  m_pAdapter;
        ::osl::Mutex&               m_rMutex;

    public:
        OContainerListener(::osl::Mutex& _rMutex);
        virtual ~OContainerListener();

        virtual void _elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void _elementRemoved( const ::com::sun::star::container::ContainerEvent& _Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void _elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void _disposing(const ::com::sun::star::lang::EventObject& _rSource) throw( ::com::sun::star::uno::RuntimeException);

    protected:
        void setAdapter(OContainerListenerAdapter* _pAdapter);
    };

    //=====================================================================
    //= OContainerListenerAdapter
    //=====================================================================
    class COMPHELPER_DLLPUBLIC OContainerListenerAdapter
            :public cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener >
    {
        friend class OContainerListener;

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >
                                m_xContainer;
        OContainerListener*     m_pListener;
        sal_Int32               m_nLockCount;

        virtual ~OContainerListenerAdapter();

    public:
        OContainerListenerAdapter(OContainerListener* _pListener,
            const  ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _rxContainer);

        // XEventListener
        virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException);

        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

        // locking the multiplexer
        void        lock();
        void        unlock();
        sal_Int32   locked() const { return m_nLockCount; }

        /// dispose the object. No multiplexing anymore
        void        dispose();

        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >&
                    getContainer() const { return m_xContainer; }
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _COMPHELPER_CONTAINERMULTIPLEXER_HXX_

