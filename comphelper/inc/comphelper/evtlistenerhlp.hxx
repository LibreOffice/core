/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: evtlistenerhlp.hxx,v $
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
#ifndef COMPHELPER_EVENTLISTENERHELPER_HXX
#define COMPHELPER_EVENTLISTENERHELPER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <osl/diagnose.h>
#include <cppuhelper/weakref.hxx>
#include "comphelper/comphelperdllapi.h"

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
