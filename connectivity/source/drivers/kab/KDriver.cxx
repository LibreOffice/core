/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: KDriver.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-12-19 16:49:48 $
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

#include "KDriver.hxx"
#ifndef _CONNECTIVITY_KAB_CONNECTION_HXX_
#include "KConnection.hxx"
#endif

#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace connectivity::kab;

KabDriver::KabDriver(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : KDriver_BASE(m_aMutex),
      m_xMSFactory(_rxFactory),
      m_pKApplication(NULL)
{
    // we create a KDE application only if it is not already done
    if (KApplication::kApplication() == NULL)
    {
        // version 0.1
        char *kabargs[1] = {"libkab1"};
        KCmdLineArgs::init(1, kabargs, "KAddressBook", *kabargs, "Address Book driver", "0.1");

        m_pKApplication = new KApplication(false, false);
    }

    // set language
    rtl_Locale *pProcessLocale;
    osl_getProcessLocale(&pProcessLocale);
    // sal_Unicode and QChar are (currently) both 16 bits characters
    QString aLanguage(
        (const QChar *) pProcessLocale->Language->buffer,
        (int) pProcessLocale->Language->length);
    KGlobal::locale()->setLanguage(aLanguage);
}
// --------------------------------------------------------------------------------
void KabDriver::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // when driver will be destroied so all our connections have to be destroied as well
    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    if (m_pKApplication != NULL)
    {
        delete m_pKApplication;
        m_pKApplication = NULL;
    }

    KDriver_BASE::disposing();
}
// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString KabDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.sdbc.kab.Driver");
        // this name is referenced in the configuration and in the kab.xml
        // Please be careful when changing it.
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > KabDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");

    return aSNS;
}
//------------------------------------------------------------------
::rtl::OUString SAL_CALL KabDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}
//------------------------------------------------------------------
sal_Bool SAL_CALL KabDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();

    while (pSupported != pEnd && !pSupported->equals(_rServiceName))
        ++pSupported;
    return pSupported != pEnd;
}
//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL KabDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL KabDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    // create a new connection with the given properties and append it to our vector
    KabConnection* pCon = new KabConnection(this);
    Reference< XConnection > xCon = pCon;   // important here because otherwise the connection could be deleted inside (refcount goes -> 0)
    pCon->construct(url,info);              // late constructor call which can throw exception and allows a correct dtor call when so
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL KabDriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    // here we have to look whether we support this URL format
    return (!url.compareTo(::rtl::OUString::createFromAscii("sdbc:address:kab:"), 16));
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL KabDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    // if you have something special to say, return it here :-)
    return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDriver::getMajorVersion(  ) throw(RuntimeException)
{
    // version 0.1
    return 0;
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL KabDriver::getMinorVersion(  ) throw(RuntimeException)
{
    // version 0.1
    return 1;
}
// --------------------------------------------------------------------------------

//.........................................................................
namespace connectivity
{
    namespace kab
    {
//.........................................................................

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL KabDriver_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory
        ) throw( ::com::sun::star::uno::Exception )
{
    return *(new KabDriver(_rxFactory));
}

void checkDisposed(sal_Bool _bThrow) throw ( DisposedException )
{
    if (_bThrow)
        throw DisposedException();
}

//.........................................................................
    }
}
//.........................................................................
