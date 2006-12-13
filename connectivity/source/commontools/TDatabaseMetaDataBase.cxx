/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TDatabaseMetaDataBase.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:13:09 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#ifndef _CONNECTIVITY_ODATABASEMETADATABASE_HXX_
#include "TDatabaseMetaDataBase.hxx"
#endif
#ifndef COMPHELPER_EVENTLISTENERHELPER_HXX
#include <comphelper/evtlistenerhlp.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace comphelper;
using namespace connectivity;


ODatabaseMetaDataBase::ODatabaseMetaDataBase(const Reference< XConnection >& _rxConnection)
    : m_xConnection(_rxConnection)
{
    osl_incrementInterlockedCount( &m_refCount );
    {
        m_xListenerHelper = new OEventListenerHelper(this);
        Reference<XComponent> xCom(m_xConnection,UNO_QUERY);
        if(xCom.is())
            xCom->addEventListener(m_xListenerHelper);
    }
    osl_decrementInterlockedCount( &m_refCount );
}
// -------------------------------------------------------------------------
ODatabaseMetaDataBase::~ODatabaseMetaDataBase()
{
}

// -----------------------------------------------------------------------------
Sequence< PropertyValue > SAL_CALL ODatabaseMetaDataBase::getConnectionInfo(  ) throw (RuntimeException)
{
    return m_aConnectionInfo;
}

// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataBase::disposing( const EventObject& /*Source*/ ) throw(RuntimeException)
{
    // cut off all references to the connection
    m_xConnection       = NULL;
    m_xListenerHelper   = NULL;
}

