/*************************************************************************
 *
 *  $RCSfile: ftpcontentprovider.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:26:47 $
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
             m_ftpLoaderThread(0),
             m_pProxyDecider(0)
{
}

//=========================================================================
// virtual
FTPContentProvider::~FTPContentProvider()
{
    delete m_ftpLoaderThread;
    delete m_pProxyDecider;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_3(FTPContentProvider,
                  XTypeProvider,
                  XServiceInfo,
                  XContentProvider)

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3(FTPContentProvider,
                     XTypeProvider,
                     XServiceInfo,
                     XContentProvider)

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
        if(!m_ftpLoaderThread || !m_pProxyDecider)
        {
            try {
                init();
            } catch( ... ) {
                throw RuntimeException();
            }

            if(!m_ftpLoaderThread || !m_pProxyDecider)
                throw RuntimeException();
        }
    }

    try {
        FTPURL aURL(xCanonicId->getContentIdentifier(),
                    this);

        if(!m_pProxyDecider->shouldUseProxy(
            rtl::OUString::createFromAscii("ftp"),
            aURL.host(),
            aURL.port().toInt32()))
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




void FTPContentProvider::init() {
    m_ftpLoaderThread = new FTPLoaderThread();
    m_pProxyDecider = new ucbhelper::InternetProxyDecider(m_xSMgr);
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



Reference<XContentProvider>
FTPContentProvider::getHttpProvider()
    throw(RuntimeException)
{
    // used for access to ftp-proxy
    ucb::ContentBroker *pBroker = ucb::ContentBroker::get();

    if(pBroker) {
        Reference<XContentProviderManager > xManager(
            pBroker->getContentProviderManagerInterface());

        if(xManager.is())
            return
                xManager->queryContentProvider(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("http:")));
        else
            throw RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "bad ucb::ContentBroker")),
                *this);
    } else
        return 0;

}
