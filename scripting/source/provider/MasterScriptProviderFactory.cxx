/*************************************************************************
 *
 *  $RCSfile: MasterScriptProviderFactory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2004-12-23 11:50:28 $
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
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <util/util.hxx>

#include "MasterScriptProviderFactory.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;

namespace func_provider
{

MasterScriptProviderFactory::MasterScriptProviderFactory(
    Reference< XComponentContext > const & xComponentContext )
    : m_xComponentContext( xComponentContext ), m_MSPList(0)
{
    OSL_TRACE("MasterScriptProviderFactory is being created!");
}

MasterScriptProviderFactory::~MasterScriptProviderFactory()
{
    OSL_TRACE("MasterScriptProviderFactory is being destructed!");
    if (m_MSPList) delete m_MSPList;
}


//############################################################################
// Implementation of XScriptProviderFactory
//############################################################################


Reference< provider::XScriptProvider > SAL_CALL
MasterScriptProviderFactory::createScriptProvider( const Any& context ) throw ( lang::IllegalArgumentException, RuntimeException)
{
    OSL_TRACE("In MasterScriptProviderFactory::createScriptProvider()" );
    Reference< provider::XScriptProvider > xMsp( getActiveMSPList().createMSP( context ), UNO_QUERY_THROW );
    return xMsp;
}

//############################################################################
// Helper methods
//############################################################################

ActiveMSPList&
MasterScriptProviderFactory::getActiveMSPList()
{
    if ( !m_MSPList )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if ( !m_MSPList )
        {
           m_MSPList = new ActiveMSPList( m_xComponentContext );
           m_MSPListHolder = m_MSPList;
        }
    }
    return *m_MSPList;
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
