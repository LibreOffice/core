/*************************************************************************
 *
 *  $RCSfile: dlgedlist.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tbe $ $Date: 2001-03-12 11:31:30 $
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
