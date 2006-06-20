 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NDriver.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:24:41 $
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

#ifndef _CONNECTIVITY_EVOAB_DRIVER_HXX_
#include "NDriver.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_CONNECTION_HXX_
#include "NConnection.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
//#ifndef _CONNECTIVITY_EVOAB_CONFIGACCESS_HXX_
//#include "LConfigAccess.hxx"
//#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_SECURITY_HXX_
#include "osl/security.hxx"
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef CONNECTIVITY_EVOAB_DEBUG_HELPER_HXX
#include "NDebug.hxx"
#endif
#include <signal.h>
using namespace osl;
using namespace connectivity::evoab;
//using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;

// --------------------------------------------------------------------------------
OEvoabDriver::OEvoabDriver(const Reference< XMultiServiceFactory >& _rxFactory) :
        ODriver_BASE( m_aMutex ), m_xFactory( _rxFactory )
{
}
// -----------------------------------------------------------------------------
OEvoabDriver::~OEvoabDriver()
{
}
// -----------------------------------------------------------------------------
void OEvoabDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // when driver will be destroied so all our connections have to be destroied as well
    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is()) {
            try {
            xComp->dispose();
            }
            catch (com::sun::star::lang::DisposedException e) {
             xComp=NULL;
            }
        }
    }
    m_xConnections.clear();
    connectivity::OWeakRefArray().swap(m_xConnections); // this really clears

    ODriver_BASE::disposing();
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OEvoabDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii(EVOAB_DRIVER_IMPL_NAME);
    // this name is referenced in the configuration and in the evoab.xml
    // Please take care when changing it.
}

//------------------------------------------------------------------
Sequence< ::rtl::OUString > OEvoabDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");
    return aSNS;
}
//------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}
//------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OEvoabDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL connectivity::evoab::OEvoabDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new OEvoabDriver(_rxFactory));
}
// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OEvoabDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if ( ! acceptsURL(url) )
        return NULL;

    OEvoabConnection* pCon = new OEvoabConnection(this);
    pCon->construct(url,info);
        Reference< XConnection > xCon = pCon;
        m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDriver::acceptsURL( const ::rtl::OUString& url )
    throw(SQLException, RuntimeException)
{
    return acceptsURL_Stat(url);
}

// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL OEvoabDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
    if ( ! acceptsURL(url) )
        ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid URL!")) ,*this);
    // if you have somthing special to say return it here :-)
    return Sequence< DriverPropertyInfo >();
}

// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDriver::getMajorVersion(  ) throw(RuntimeException)
{
    return 1;
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabDriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// --------------------------------------------------------------------------------
sal_Bool OEvoabDriver::acceptsURL_Stat( const ::rtl::OUString& url )
{
    return (url.equalsAscii("sdbc:address:evolution:local") || url.equalsAscii("sdbc:address:evolution:groupwise")||url.equalsAscii("sdbc:address:evolution:ldap"))&& EApiInit();
}
// -----------------------------------------------------------------------------
