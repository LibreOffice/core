/*************************************************************************
 *
 *  $RCSfile: ftpcontentprovider.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: abi $ $Date: 2002-10-23 08:00:05 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <ucbhelper/contentbroker.hxx>
#include <osl/socket.hxx>
#include "ftpcontentprovider.hxx"
#include "ftpcontent.hxx"
#include "ftploaderthread.hxx"


using namespace ftp;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;



//=========================================================================
//=========================================================================
//
// ContentProvider Implementation.
//
//=========================================================================
//=========================================================================

FTPContentProvider::FTPContentProvider(const Reference< XMultiServiceFactory >& rSMgr)
    : ::ucb::ContentProviderImplHelper(rSMgr),
             m_ftpLoaderThread(NULL)
{
}

//=========================================================================
// virtual
FTPContentProvider::~FTPContentProvider()
{
    delete m_ftpLoaderThread;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_6(FTPContentProvider,
                  XTypeProvider,
                  XServiceInfo,
                  XContentProvider,
                  XComponent,
                  XEventListener,
                  XContainerListener);

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_6(FTPContentProvider,
                     XTypeProvider,
                     XServiceInfo,
                     XContentProvider,
                     XComponent,
                     XEventListener,
                     XContainerListener);

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1(FTPContentProvider,
                    rtl::OUString::createFromAscii("com.sun.star.comp.FTPContentProvider"),
                    rtl::OUString::createFromAscii(MYUCP_CONTENT_PROVIDER_SERVICE_NAME));

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL(FTPContentProvider);


//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

// virtual
Reference<XContent> SAL_CALL
FTPContentProvider::queryContent(
    const Reference< XContentIdentifier >& xCanonicId
)
    throw(
        IllegalIdentifierException,
        RuntimeException
    )
{
    // Check, if a content with given id already exists...
    Reference<XContent> xContent
        = queryExistingContent(xCanonicId).getBodyPtr();
    if(xContent.is())
        return xContent;

    // A new content has to be returned:
    {
        // Initialize
        osl::MutexGuard aGuard( m_aMutex );
        if(!m_ftpLoaderThread)
        {
            init();
            if(!m_ftpLoaderThread)
                throw RuntimeException();
        }
    }

    try {
        FTPURL aURL(xCanonicId->getContentIdentifier(),
                    this);
        if(!ShouldUseFtpProxy(aURL))
            xContent = new FTPContent(m_xSMgr,this,xCanonicId,aURL);
        else {
            Reference<XContentProvider>
                xProvider(getHttpProvider());
            if(xProvider.is())
                return xProvider->queryContent(xCanonicId);
            else
                throw RuntimeException();
        }
    } catch(const malformed_exception&) {
        throw IllegalIdentifierException();
    }

    // may throw IllegalIdentifierException
    return xContent;
}



// from inetoptions


enum ProxyType { NONE, AUTOMATIC, MANUAL };


bool FTPContentProvider::ShouldUseFtpProxy(const FTPURL& aURL) const
{
    // Check URL.
    if(m_eType == NONE)
        return false;

    if(! m_aFtpProxy.getLength())
        return false;

    if(m_aNoProxyList.getLength())
    {
        // Setup Endpoint.
        rtl::OUString host(aURL.host());
        if(!host.getLength())
            return false;

        osl::SocketAddr aAddr(host,21);  // port does not matter here
        host = aAddr.getHostname().toAsciiLowerCase();
        sal_Int32 port = aURL.port().toInt32();

        // Match NoProxyList.
        sal_Int32 nIndex = 0;
        do {
            rtl::OUString aDomain =
                m_aNoProxyList.getToken(0,';',nIndex).toAsciiLowerCase();
            rtl::OUString aPort;

            sal_Int32 i(aDomain.indexOf(':',0));
            if(i != -1) {
                aPort = aDomain.copy(1+i);
                aDomain = aDomain.copy(0,i).trim();
            }

            if(!aDomain.getLength())
                continue;

            if(host == aDomain)
                if(aPort.getLength() == 0 || port == aPort.toInt32())
                    return false;
                else
                    return true;

            i = host.lastIndexOf(aDomain);
            if(i!=-1 &&
               // i == 0 not possible here anymore!
               // really last part of string of the string?:
               host.getLength() == i+aDomain.getLength() &&
               // ensure not to match "xy.z" on "x.z" or ".x.z"
               (aDomain.getStr()[0] == '.' || host.getStr()[i-1] == '.') &&
               // does the port match?
               (aPort.getLength() == 0 || port == aPort.toInt32()))
                return false;
        }
        while ( nIndex != -1 );
    }

    return true;
}


void SAL_CALL
FTPContentProvider::elementReplaced(const ContainerEvent& Event)
    throw(RuntimeException)
{
    rtl::OUString accessor;
    Event.Accessor >>= accessor;
    if(accessor.compareToAscii("ooInetFTPProxyName") == 0) {
        rtl::OUString replacedElement,element;
        if((Event.ReplacedElement >>= replacedElement) &&
           (Event.Element >>= element) )
        {
            osl::MutexGuard aGuard(m_aMutex);
            m_aFtpProxy = element;
        }
    } else if(accessor.compareToAscii("ooInetNoProxy") == 0) {
        rtl::OUString replacedElement,element;
        if((Event.ReplacedElement >>= replacedElement) &&
           (Event.Element >>= element))
        {
            osl::MutexGuard aGuard(m_aMutex);
            m_aNoProxyList = element;
        }
    }
    else if(accessor.compareToAscii("ooInetProxyType") == 0) {
        sal_Int32 replacedElement,element;
        if((Event.ReplacedElement >>= replacedElement) &&
           (Event.Element >>= element))
        {
            osl::MutexGuard aGuard(m_aMutex);
            m_eType = element;
        }
    }
}


void FTPContentProvider::init() {
    m_ftpLoaderThread = new FTPLoaderThread();

    Reference< XMultiServiceFactory >  sProvider( getConfiguration());
    Reference< XHierarchicalNameAccess > xHierAccess(
        getHierAccess(sProvider,"org.openoffice.Inet"));

    m_aFtpProxy = getKey(xHierAccess,
                         "Settings/ooInetFTPProxyName");
    m_aNoProxyList = getKey(xHierAccess,
                            "Settings/ooInetNoProxy");
    m_eType = getIntKey(xHierAccess,
                        "Settings/ooInetProxyType");

    try
    {
        // add as configuration change listener for the proxy settings
        Reference<XNameAccess> xAccess(xHierAccess,UNO_QUERY);
        Any aAny =
            xAccess->getByName(rtl::OUString::createFromAscii("Settings"));
        aAny >>= m_xContainer;
        if(m_xContainer.is())
            m_xContainer->addContainerListener(this);
    }
    catch(const com::sun::star::uno::Exception& )
    {
    }
}



CURL* FTPContentProvider::handle() {
    // Cannot be zero if called from here;
    return m_ftpLoaderThread->handle();
}


bool FTPContentProvider::forHost(
    const rtl::OUString& host,
    const rtl::OUString& port,
    const rtl::OUString& username,
    rtl::OUString& password,
    rtl::OUString& account)
{
    osl::MutexGuard aGuard(m_aMutex);
    for(unsigned int i = 0; i < m_ServerInfo.size(); ++i)
        if(host == m_ServerInfo[i].host &&
           port == m_ServerInfo[i].port &&
           username == m_ServerInfo[i].username )
        {
            password = m_ServerInfo[i].password;
            account = m_ServerInfo[i].account;
            return true;
        }

    return false;
}


bool  FTPContentProvider::setHost(
    const rtl::OUString& host,
    const rtl::OUString& port,
    const rtl::OUString& username,
    const rtl::OUString& password,
    const rtl::OUString& account)
{
    ServerInfo inf;
    inf.host = host;
    inf.port = port;
    inf.username = username;
    inf.password = password;
    inf.account = account;

    bool present(false);
    osl::MutexGuard aGuard(m_aMutex);
    for(unsigned int i = 0; i < m_ServerInfo.size(); ++i)
        if(host == m_ServerInfo[i].host &&
           port == m_ServerInfo[i].port &&
           username == m_ServerInfo[i].username)
        {
            present = true;
            m_ServerInfo[i].password = password;
            m_ServerInfo[i].account = account;
        }

    if(!present)
        m_ServerInfo.push_back(inf);

    return !present;
}


Reference<XMultiServiceFactory>
FTPContentProvider::getConfiguration() const
{
    Reference< XMultiServiceFactory > sProvider;
    if(m_xSMgr.is())
    {
        Any aAny;
        aAny <<= rtl::OUString::createFromAscii( "plugin" );
        PropertyValue aProp(
            rtl::OUString::createFromAscii( "servertype" ),
            -1,
            aAny,
            PropertyState_DIRECT_VALUE );

        Sequence<Any> seq(1);
        seq[0] <<= aProp;

        try
        {
            rtl::OUString sProviderService =
                rtl::OUString::createFromAscii(
                    "com.sun.star.configuration.ConfigurationProvider" );
            sProvider =
                Reference<XMultiServiceFactory>(
                    m_xSMgr->createInstanceWithArguments(
                        sProviderService,seq ),
                    UNO_QUERY );
        }
        catch( const com::sun::star::uno::Exception& )
        {
            OSL_ENSURE( sProvider.is(),"cant instantiate configuration" );
        }
    }

    return sProvider;
}


Reference<XHierarchicalNameAccess>
FTPContentProvider::getHierAccess(
    const Reference<XMultiServiceFactory >& sProvider,
    const char* file ) const
{
    Reference<XHierarchicalNameAccess>
        xHierAccess;

    if( sProvider.is() )
    {
        Sequence<Any> seq(1);
        rtl::OUString sReaderService =
            rtl::OUString::createFromAscii(
                "com.sun.star.configuration.ConfigurationAccess" );

        seq[0] <<= rtl::OUString::createFromAscii(file);

        try
        {
            xHierAccess =
                Reference<
                XHierarchicalNameAccess >(
                    sProvider->createInstanceWithArguments(
                        sReaderService,seq ),
                    UNO_QUERY );
        }
        catch( const Exception& )
        {
        }
    }

    return xHierAccess;
}


rtl::OUString
FTPContentProvider::getKey(
    const Reference<XHierarchicalNameAccess>& xHierAccess,
    const char* key ) const
{
    rtl::OUString val;
    if( xHierAccess.is() )
    {
        Any aAny;
        try
        {
            aAny =
                xHierAccess->getByHierarchicalName(
                    rtl::OUString::createFromAscii(key));
        }
        catch( const NoSuchElementException& )
        {
        }
        aAny >>= val;
    }
    return val;
}


sal_Int32
FTPContentProvider::getIntKey(
    const Reference<XHierarchicalNameAccess>& xHierAccess,
    const char* key ) const
{
    sal_Int32 val = 0;
    if( xHierAccess.is() )
    {
        Any aAny;
        try
        {
            aAny =
                xHierAccess->getByHierarchicalName(
                    rtl::OUString::createFromAscii(key));
        }
        catch( const NoSuchElementException& )
        {
        }
        aAny >>= val;
    }
    return val;
}



Reference<XContentProvider>
FTPContentProvider::getHttpProvider()
    throw(RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    if (!m_xManager.is())
    {
        if (!m_xManager.is())
        {
            ucb::ContentBroker * pBroker = ucb::ContentBroker::get();
            if (pBroker)
            {
                m_xManager = pBroker->getContentProviderManagerInterface();
                if (!m_xManager.is())
                    throw RuntimeException(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "bad ucb::ContentBroker")),
                        *this);
            }
            if (!m_xManager.is())
                return 0;
        }
    }

    return m_xManager->
        queryContentProvider(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "http:")));
    //TODO! input ok?
}
