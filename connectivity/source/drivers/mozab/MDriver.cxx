/*************************************************************************
 *
 *  $RCSfile: MDriver.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:06:24 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef CONNECTIVITY_SDRIVER_HXX
#include "MDriver.hxx"
#endif
#ifndef CONNECTIVITY_SCONNECTION_HXX
#include "MConnection.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace connectivity::mozab;

namespace connectivity
{
    namespace mozab
    {
        //------------------------------------------------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL MozabDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
        {
            return *(new MozabDriver( _rxFactory ));
        }
    }
}
// --------------------------------------------------------------------------------
MozabDriver::MozabDriver(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : ODriver_BASE(m_aMutex), m_xMSFactory( _rxFactory )
    ,s_hModule(NULL)
    ,s_pCreationFunc(NULL)
{
}
// -----------------------------------------------------------------------------
MozabDriver::~MozabDriver()
{
}
// --------------------------------------------------------------------------------
void MozabDriver::disposing()
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
    connectivity::OWeakRefArray().swap(m_xConnections); // this really clears

    ODriver_BASE::disposing();
    if(s_hModule)
    {
        s_pCreationFunc = NULL;
        osl_unloadModule(s_hModule);
        s_hModule = NULL;
    }
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString MozabDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii(MOZAB_DRIVER_IMPL_NAME);
        // this name is referenced in the configuration and in the mozab.xml
        // Please take care when changing it.
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > MozabDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.sdbc.Driver
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.Driver"));
    return aSNS;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL MozabDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------
sal_Bool SAL_CALL MozabDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL MozabDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL MozabDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( ! acceptsURL(url) )
        return NULL;
    // create a new connection with the given properties and append it to our vector
    registerClient();
    Reference< XConnection > xCon;
    if (s_pCreationFunc)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        OConnection* pCon = reinterpret_cast<OConnection*>((*s_pCreationFunc)(this));
        xCon = pCon;    // important here because otherwise the connection could be deleted inside (refcount goes -> 0)
        pCon->construct(url,info);              // late constructor call which can throw exception and allows a correct dtor call when so
        m_xConnections.push_back(WeakReferenceHelper(*pCon));
    }
    else
    {
        ::rtl::OUString sMsg = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Could not load the library "));
        sMsg += ::rtl::OUString::createFromAscii(SAL_MODULENAME( "mozabdrv2" ));
        ::dbtools::throwGenericSQLException(sMsg,*this);
    }

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL MozabDriver::acceptsURL( const ::rtl::OUString& url )
        throw(SQLException, RuntimeException)
{
    // here we have to look if we support this url format
    return acceptsURL_Stat(url) != Unknown;
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL MozabDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;
        if ( acceptsURL_Stat(url) == LDAP )
        {
            aDriverInfo.push_back(DriverPropertyInfo(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BaseDN"))
                    ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Base DN."))
                    ,sal_False
                    ,::rtl::OUString()
                    ,Sequence< ::rtl::OUString >())
                    );
            aDriverInfo.push_back(DriverPropertyInfo(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MaxRowCount"))
                    ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Records (max.)"))
                    ,sal_False
                    ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("100"))
                    ,Sequence< ::rtl::OUString >())
                    );
        }
        return Sequence< DriverPropertyInfo >(aDriverInfo.begin(),aDriverInfo.size());
    }
    ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid URL!")) ,*this);
    // if you have somthing special to say return it here :-)
    return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL MozabDriver::getMajorVersion(  ) throw(RuntimeException)
{
    return 1; // depends on you
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL MozabDriver::getMinorVersion(  ) throw(RuntimeException)
{
    return 0; // depends on you
}
// --------------------------------------------------------------------------------
EDriverType MozabDriver::acceptsURL_Stat( const ::rtl::OUString& url )
{
    // Skip 'sdbc:mozab: part of URL
    //
    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aAddrbookURI(url.copy(nLen+1));
    // Get Scheme
    nLen = aAddrbookURI.indexOf(':');
    ::rtl::OUString aAddrbookScheme;
    if ( nLen == -1 )
    {
        // There isn't any subschema: - but could be just subschema
        if ( aAddrbookURI.getLength() > 0 )
            aAddrbookScheme= aAddrbookURI;
        else if(url == ::rtl::OUString::createFromAscii("sdbc:address:") )
            return Unknown; // TODO check
        else
            return Unknown;
    }
    else
        aAddrbookScheme = aAddrbookURI.copy(0, nLen);

    if ( aAddrbookScheme.compareToAscii( MozabDriver::getSDBC_SCHEME_MOZILLA() ) == 0 )
        return Mozilla;
    if ( aAddrbookScheme.compareToAscii( MozabDriver::getSDBC_SCHEME_LDAP() ) == 0 )
        return LDAP;

#if defined(WNT) || defined(WIN)
    if ( aAddrbookScheme.compareToAscii( MozabDriver::getSDBC_SCHEME_OUTLOOK_MAPI() ) == 0 )
        return Outlook;
    if ( aAddrbookScheme.compareToAscii( MozabDriver::getSDBC_SCHEME_OUTLOOK_EXPRESS() ) == 0 )
        return OutlookExpress;

#endif

    return Unknown;
}
// -----------------------------------------------------------------------------
const sal_Char* MozabDriver::getSDBC_SCHEME_MOZILLA()
{
    static sal_Char*    SDBC_SCHEME_MOZILLA         = MOZAB_MOZILLA_SCHEMA;
    return SDBC_SCHEME_MOZILLA;
}
// -----------------------------------------------------------------------------
const sal_Char* MozabDriver::getSDBC_SCHEME_LDAP()
{
    static sal_Char*    SDBC_SCHEME_LDAP            = MOZAB_LDAP_SCHEMA;
    return SDBC_SCHEME_LDAP;
}
// -----------------------------------------------------------------------------
const sal_Char* MozabDriver::getSDBC_SCHEME_OUTLOOK_MAPI()
{
    static sal_Char*    SDBC_SCHEME_OUTLOOK_MAPI    = MOZAB_OUTLOOK_SCHEMA;
    return SDBC_SCHEME_OUTLOOK_MAPI;
}
// -----------------------------------------------------------------------------
const sal_Char* MozabDriver::getSDBC_SCHEME_OUTLOOK_EXPRESS()
{
    static sal_Char*    SDBC_SCHEME_OUTLOOK_EXPRESS = MOZAB_OUTLOOKEXP_SCHEMA;
    return SDBC_SCHEME_OUTLOOK_EXPRESS;
}// -----------------------------------------------------------------------------
void MozabDriver::registerClient()
{
    if (!s_hModule)
    {
        OSL_ENSURE(NULL == s_pCreationFunc, "MozabDriver::registerClient: inconsistence: already have a factory function!");

        const ::rtl::OUString sModuleName = ::rtl::OUString::createFromAscii(SAL_MODULENAME( "mozabdrv2" ));

        // load the dbtools library
        s_hModule = osl_loadModule(sModuleName.pData, 0);
        OSL_ENSURE(NULL != s_hModule, "MozabDriver::registerClient: could not load the dbtools library!");
        if (NULL != s_hModule)
        {
            // first, we need to announce our service factory to the lib
            // see the documentation of setMozabServiceFactory for more details
            const ::rtl::OUString sSetFactoryFuncName( RTL_CONSTASCII_USTRINGPARAM( "setMozabServiceFactory" ) );
            //  reinterpret_cast< OSetMozabServiceFactory >>> removed GNU C
            OSetMozabServiceFactory pSetFactoryFunc =
                ( OSetMozabServiceFactory )( osl_getSymbol( s_hModule, sSetFactoryFuncName.pData ) );

            OSL_ENSURE( pSetFactoryFunc, "MozabDriver::registerClient: missing an entry point!" );
            if ( pSetFactoryFunc && m_xMSFactory.is() )
            {
                // for purpose of transfer safety, the interface needs to be acuired once
                // (will be release by the callee)
                m_xMSFactory->acquire();
                ( *pSetFactoryFunc )( m_xMSFactory.get() );
            }

            // get the symbol for the method creating the factory
            const ::rtl::OUString sFactoryCreationFunc = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("OMozabConnection_CreateInstance"));
            // reinterpret_cast<OMozabConnection_CreateInstanceFunction> removed GNU C
            s_pCreationFunc = (OMozabConnection_CreateInstanceFunction)(osl_getSymbol(s_hModule, sFactoryCreationFunc.pData));

            if (NULL == s_pCreationFunc)
            {   // did not find the symbol
                OSL_ENSURE(sal_False, "MozabDriver::registerClient: could not find the symbol for creating the factory!");
                osl_unloadModule(s_hModule);
                s_hModule = NULL;
            }
        }
    }
}
// -----------------------------------------------------------------------------



