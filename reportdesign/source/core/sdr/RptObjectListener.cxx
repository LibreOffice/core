/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RptObjectListener.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 09:55:14 $
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

#ifndef _REPORT_RPTUILIST_HXX
#include "RptObjectListener.hxx"
#endif

#ifndef _REPORT_RPTUIOBJ_HXX
#include "RptObject.hxx"
#endif
#ifndef _REPORT_RPTUIDEF_HXX
#include "RptDef.hxx"
#endif

namespace rptui
{
//============================================================================
// OObjectListener
//============================================================================

//----------------------------------------------------------------------------
DBG_NAME(rpt_OObjectListener)
OObjectListener::OObjectListener(OObjectBase* _pObject)
          :m_pObject(_pObject)
{
    DBG_CTOR(rpt_OObjectListener,NULL);
}

//----------------------------------------------------------------------------

OObjectListener::~OObjectListener()
{
    DBG_DTOR(rpt_OObjectListener,NULL);
}

// XEventListener
//----------------------------------------------------------------------------

void SAL_CALL OObjectListener::disposing( const  ::com::sun::star::lang::EventObject& ) throw( ::com::sun::star::uno::RuntimeException)
{
    /*
    // disconnect the listener
    if (m_pObject)
    {
        (m_pObject->m_xPropertyChangeListener).clear();
    }
    */
}

// XPropertyChangeListener
//----------------------------------------------------------------------------

void SAL_CALL OObjectListener::propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException)
{
    m_pObject->_propertyChange( evt );
}

//----------------------------------------------------------------------------

//============================================================================
// DlgEdHint
//============================================================================

TYPEINIT1( DlgEdHint, SfxHint );

//----------------------------------------------------------------------------

DlgEdHint::DlgEdHint( DlgEdHintKind eHint )
    :eHintKind( eHint )
{
}

//----------------------------------------------------------------------------

DlgEdHint::DlgEdHint( DlgEdHintKind eHint, OUnoObject* pObj )
    :eHintKind( eHint )
    ,pDlgEdObj( pObj )
{
}

//----------------------------------------------------------------------------

DlgEdHint::~DlgEdHint()
{
}
//----------------------------------------------------------------------------
}
