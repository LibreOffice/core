/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MasterScriptProviderFactory.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:29:42 $
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
#include "precompiled_scripting.hxx"
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase1.hxx>

#include <util/util.hxx>

#include "MasterScriptProviderFactory.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;

namespace func_provider
{

MasterScriptProviderFactory::MasterScriptProviderFactory(
    Reference< XComponentContext > const & xComponentContext )
    : m_xComponentContext( xComponentContext )
{
}

MasterScriptProviderFactory::~MasterScriptProviderFactory()
{
}


//############################################################################
// Implementation of XScriptProviderFactory
//############################################################################


Reference< provider::XScriptProvider > SAL_CALL
MasterScriptProviderFactory::createScriptProvider( const Any& context ) throw ( lang::IllegalArgumentException, RuntimeException)
{
    Reference< provider::XScriptProvider > xMsp( getActiveMSPList() ->getMSPFromAnyContext( context ), UNO_QUERY_THROW );
    return xMsp;
}

//############################################################################
// Helper methods
//############################################################################

const rtl::Reference< ActiveMSPList > &
MasterScriptProviderFactory::getActiveMSPList() const
{
    if ( !m_MSPList.is() )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if ( !m_MSPList.is() )
           m_MSPList = new ActiveMSPList( m_xComponentContext );
    }
    return m_MSPList;
}

//############################################################################
// Namespace global methods for setting up MasterScriptProviderFactory service
//############################################################################

Sequence< ::rtl::OUString > SAL_CALL
mspf_getSupportedServiceNames( )
    SAL_THROW( () )
{
    ::rtl::OUString str_name = ::rtl::OUString::createFromAscii(
        "com.sun.star.script.provider.MasterScriptProviderFactory");

    return Sequence< ::rtl::OUString >( &str_name, 1 );
}

::rtl::OUString SAL_CALL
mspf_getImplementationName( )
    SAL_THROW( () )
{
    return ::rtl::OUString::createFromAscii(
        "com.sun.star.script.provider.MasterScriptProviderFactory");
}

Reference< XInterface > SAL_CALL
mspf_create( Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    return static_cast< ::cppu::OWeakObject * >(
        new MasterScriptProviderFactory( xComponentContext ) );
}

//############################################################################
// Implementation of XServiceInfo
//############################################################################

::rtl::OUString SAL_CALL
MasterScriptProviderFactory::getImplementationName()
    throw (RuntimeException)
{
    return mspf_getImplementationName();
}

Sequence< ::rtl::OUString > SAL_CALL
MasterScriptProviderFactory::getSupportedServiceNames()
    throw (RuntimeException)
{
    return mspf_getSupportedServiceNames();
}

sal_Bool MasterScriptProviderFactory::supportsService(
    ::rtl::OUString const & serviceName )
    throw (RuntimeException)
{
//     check();

    Sequence< ::rtl::OUString > supported_services(
        getSupportedServiceNames() );

    ::rtl::OUString const * ar = supported_services.getConstArray();

    for ( sal_Int32 pos = supported_services.getLength(); pos--; )
    {
        if (ar[ pos ].equals( serviceName ))
            return true;
    }
    return false;
}

} // namespace browsenodefactory
