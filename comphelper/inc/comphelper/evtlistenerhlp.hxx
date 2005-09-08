/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: evtlistenerhlp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:30:41 $
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
#ifndef COMPHELPER_EVENTLISTENERHELPER_HXX
#define COMPHELPER_EVENTLISTENERHELPER_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

//........................................................................
namespace comphelper
{
//........................................................................

    //==========================================================================
    //= OCommandsListener
    // is helper class to avoid a cycle in refcount between the XEventListener
    // and the member XEventBroadcaster
    //==========================================================================
    class COMPHELPER_DLLPUBLIC OEventListenerHelper : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
    {
        ::com::sun::star::uno::WeakReference< ::com::sun::star::lang::XEventListener> m_xListener;
    public:
        OEventListenerHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>& _rxListener);
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
    };
//........................................................................
}   // namespace comphelper
//........................................................................
#endif // COMPHELPER_EVENTLISTENERHELPER_HXX
