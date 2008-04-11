/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlgedlist.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"
#include "dlgedlist.hxx"
#include "dlgedobj.hxx"

//============================================================================
// DlgEdPropListenerImpl
//============================================================================

//----------------------------------------------------------------------------

DlgEdPropListenerImpl::DlgEdPropListenerImpl()
{
}

//----------------------------------------------------------------------------

DlgEdPropListenerImpl::DlgEdPropListenerImpl(DlgEdObj* pObj)
          :pDlgEdObj(pObj)
{
}

//----------------------------------------------------------------------------

DlgEdPropListenerImpl::~DlgEdPropListenerImpl()
{
}

// XEventListener
//----------------------------------------------------------------------------

void SAL_CALL DlgEdPropListenerImpl::disposing( const ::com::sun::star::lang::EventObject& ) throw( ::com::sun::star::uno::RuntimeException)
{
    /*
    // disconnect the listener
    if (pDlgEdObj)
    {
        (pDlgEdObj->m_xPropertyChangeListener).clear();
    }
    */
}

// XPropertyChangeListener
//----------------------------------------------------------------------------

void SAL_CALL DlgEdPropListenerImpl::propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException)
{
    pDlgEdObj->_propertyChange( evt );
}

//----------------------------------------------------------------------------

//============================================================================
// DlgEdEvtContListenerImpl
//============================================================================

//----------------------------------------------------------------------------

DlgEdEvtContListenerImpl::DlgEdEvtContListenerImpl()
{
}

//----------------------------------------------------------------------------

DlgEdEvtContListenerImpl::DlgEdEvtContListenerImpl(DlgEdObj* pObj)
          :pDlgEdObj(pObj)
{
}

//----------------------------------------------------------------------------

DlgEdEvtContListenerImpl::~DlgEdEvtContListenerImpl()
{
}

// XEventListener
//----------------------------------------------------------------------------

void SAL_CALL DlgEdEvtContListenerImpl::disposing( const  ::com::sun::star::lang::EventObject& ) throw( ::com::sun::star::uno::RuntimeException)
{
    /*
    // disconnect the listener
    if (pDlgEdObj)
    {
        (pDlgEdObj->m_xContainerListener).clear();
    }
    */
}

// XContainerListener
//----------------------------------------------------------------------------

void SAL_CALL DlgEdEvtContListenerImpl::elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException)
{
    pDlgEdObj->_elementInserted( Event );
}

//----------------------------------------------------------------------------

void SAL_CALL DlgEdEvtContListenerImpl::elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException)
{
    pDlgEdObj->_elementReplaced( Event );
}

//----------------------------------------------------------------------------

void SAL_CALL DlgEdEvtContListenerImpl::elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException)
{
    pDlgEdObj->_elementRemoved( Event );
}

//----------------------------------------------------------------------------
