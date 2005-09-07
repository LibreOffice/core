/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedlist.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:14:31 $
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

#ifndef _BASCTL_DLGEDLIST_HXX
#include "dlgedlist.hxx"
#endif

#ifndef _BASCTL_DLGEDOBJ_HXX
#include "dlgedobj.hxx"
#endif

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

void SAL_CALL DlgEdPropListenerImpl::disposing( const  ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException)
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

void SAL_CALL DlgEdEvtContListenerImpl::disposing( const  ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException)
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
