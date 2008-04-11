/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlgedlist.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _BASCTL_DLGEDLIST_HXX
#define _BASCTL_DLGEDLIST_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>

class DlgEdObj;

//============================================================================
// DlgEdPropListenerImpl
//============================================================================

typedef ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertyChangeListener > PropertyChangeListenerHelper;

class DlgEdPropListenerImpl: public PropertyChangeListenerHelper
{
private:
    DlgEdObj*       pDlgEdObj;

public:
    DlgEdPropListenerImpl();
    DlgEdPropListenerImpl(DlgEdObj* pObj);
    virtual ~DlgEdPropListenerImpl();

    // XEventListener
    virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

};

//============================================================================
// DlgEdEvtContListenerImpl
//============================================================================

typedef ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener > ContainerListenerHelper;

class DlgEdEvtContListenerImpl: public ContainerListenerHelper
{
private:
    DlgEdObj*       pDlgEdObj;

public:
    DlgEdEvtContListenerImpl();
    DlgEdEvtContListenerImpl(DlgEdObj* pObj);
    virtual ~DlgEdEvtContListenerImpl();

    // XEventListener
    virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
};

#endif // _BASCTL_DLGEDLIST_HXX
