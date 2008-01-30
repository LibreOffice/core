/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TConnection.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 07:47:03 $
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
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#include <comphelper/types.hxx>

using namespace connectivity;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace::osl;

//------------------------------------------------------------------------------
void OMetaConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_xMetaData = WeakReference< XDatabaseMetaData>();
    for (OWeakRefArray::iterator i = m_aStatements.begin(); m_aStatements.end() != i; ++i)
    {
        try
        {
            Reference< XInterface > xStatement( i->get() );
            ::comphelper::disposeComponent( xStatement );
        }
        catch (DisposedException)
        {
        }
    }
    m_aStatements.clear();
}
//XUnoTunnel
sal_Int64 SAL_CALL OMetaConnection::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw (::com::sun::star::uno::RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        ? reinterpret_cast< sal_Int64 >( this )
        : (sal_Int64)0;
}
// -----------------------------------------------------------------------------
Sequence< sal_Int8 > OMetaConnection::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}
// -----------------------------------------------------------------------------
::dbtools::OPropertyMap& OMetaConnection::getPropMap()
{
    static ::dbtools::OPropertyMap s_aPropertyNameMap;
    return s_aPropertyNameMap;
}
// -----------------------------------------------------------------------------

