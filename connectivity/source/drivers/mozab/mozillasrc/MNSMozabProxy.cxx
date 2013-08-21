/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "MNSMozabProxy.hxx"

#include "resource/mozab_res.hrc"
#include "MDatabaseMetaDataHelper.hxx"
#include "MQuery.hxx"
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <osl/thread.hxx>

#include "pre_include_mozilla.h"
#include <nsIProxyObjectManager.h>
// More Mozilla includes for LDAP Connection Test
#include "prprf.h"
#include "nsILDAPURL.h"
#include "nsILDAPMessage.h"
#include "nsILDAPMessageListener.h"
#include "nsILDAPErrors.h"
#include "nsILDAPConnection.h"
#include "nsILDAPOperation.h"
#include "post_include_mozilla.h"
#include <MQueryHelper.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/processfactory.hxx>
#include "com/sun/star/mozilla/XProxyRunner.hpp"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::mozilla;


#define TYPEASSERT(value,type) if (value != type) return !NS_OK;

using namespace connectivity::mozab;

/* Implementation file */

static ::osl::Mutex m_aThreadMutex;
extern nsresult NewAddressBook(const OUString * aName);


MNSMozabProxy::MNSMozabProxy()
{
  m_Args = NULL;
#if OSL_DEBUG_LEVEL > 0
  m_oThreadID = osl::Thread::getCurrentIdentifier();
#endif
  acquire();
}

MNSMozabProxy::~MNSMozabProxy()
{
}

sal_Int32 MNSMozabProxy::StartProxy(RunArgs * args,::com::sun::star::mozilla::MozillaProductType aProduct,const OUString &aProfile)
{
    OSL_TRACE( "IN : MNSMozabProxy::StartProxy()" );
    ::osl::MutexGuard aGuard(m_aThreadMutex);
    m_Product = aProduct;
    m_Profile = aProfile;
    m_Args = args;
    if (!xRunner.is())
    {
        Reference<XMultiServiceFactory> xFactory = ::comphelper::getProcessServiceFactory();
        OSL_ENSURE( xFactory.is(), "can't get service factory" );
        ::com::sun::star::uno::Reference<XInterface> xInstance = xFactory->createInstance("com.sun.star.mozilla.MozillaBootstrap");
        OSL_ENSURE( xInstance.is(), "failed to create instance" );
        xRunner = ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XProxyRunner >(xInstance,UNO_QUERY);
    }
    const ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XCodeProxy > aCode(this);
    return xRunner->Run(aCode);
}

extern nsresult getTableStringsProxied(const sal_Char* sAbURI, sal_Int32 *nDirectoryType,MNameMapper *nmap,
                        ::std::vector< OUString >*   _rStrings,
                        ::std::vector< OUString >*   _rTypes,
                        sal_Int32* pErrorId );

::com::sun::star::mozilla::MozillaProductType SAL_CALL MNSMozabProxy::getProductType(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_Product;
}
OUString SAL_CALL MNSMozabProxy::getProfileName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_Profile;
}

sal_Int32 SAL_CALL MNSMozabProxy::run(  ) throw (::com::sun::star::uno::RuntimeException)
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "IN : MNSMozabProxy::Run() Caller thread :%4d" , m_oThreadID );
#else
    OSL_TRACE( "IN : MNSMozabProxy::Run()" );
#endif
    nsresult rv = NS_ERROR_INVALID_ARG;
    if (m_Args == NULL)
        return NS_ERROR_INVALID_ARG;
    switch(m_Args->funcIndex)
    {
    case ProxiedFunc::FUNC_TESTLDAP_INIT_LDAP:
    case ProxiedFunc::FUNC_TESTLDAP_IS_LDAP_CONNECTED:
    case ProxiedFunc::FUNC_TESTLDAP_RELEASE_RESOURCE:
        rv = testLDAPConnection();
        break;
    case ProxiedFunc::FUNC_GET_TABLE_STRINGS:
        rv = getTableStringsProxied((const sal_Char*)m_Args->arg1,
                                (sal_Int32 *)m_Args->arg2,
                                (MNameMapper *)m_Args->arg3,
                                (::std::vector< OUString >*)m_Args->arg4,
                                (::std::vector< OUString >*)m_Args->arg5,
                                (sal_Int32 *)m_Args->arg6);
        break;
    case ProxiedFunc::FUNC_EXECUTE_QUERY:
        if (m_Args->arg1 && m_Args->arg2)
        {
            rv = ((MQuery*)m_Args->arg1)->executeQueryProxied((OConnection*)m_Args->arg2);
        }
        break;
    case ProxiedFunc::FUNC_QUERYHELPER_CREATE_NEW_CARD:
    case ProxiedFunc::FUNC_QUERYHELPER_DELETE_CARD:
    case ProxiedFunc::FUNC_QUERYHELPER_COMMIT_CARD:
    case ProxiedFunc::FUNC_QUERYHELPER_RESYNC_CARD:
        if (m_Args->arg1)
        {
            rv = QueryHelperStub();
        }
        break;
    case ProxiedFunc::FUNC_NEW_ADDRESS_BOOK:
        if (m_Args->arg1)
        {
            rv = NewAddressBook((const OUString*)m_Args->arg1 );
        }
        break;
    default:
           return NS_ERROR_INVALID_ARG;
    }
    return rv;
}

nsresult MNSMozabProxy::QueryHelperStub()
{
    nsresult rv = NS_ERROR_INVALID_ARG;
    MQueryHelper * mHelper=(MQueryHelper*) m_Args->arg1;
    switch(m_Args->funcIndex)
    {
    case ProxiedFunc::FUNC_QUERYHELPER_CREATE_NEW_CARD:
        if (m_Args->arg2 )  //m_Args->arg2 used to return cord number
        {
            *((sal_Int32*)m_Args->arg2) = mHelper->createNewCard();
            rv = NS_OK;
        }
        break;
    case ProxiedFunc::FUNC_QUERYHELPER_DELETE_CARD:
        if (m_Args->arg2 && m_Args->arg3 )  //m_Args->arg2 used to get the cord number
        {
            rv = mHelper->deleteCard(*((sal_uInt32*)m_Args->arg2),(nsIAbDirectory*)m_Args->arg3);
        }
        break;
    case ProxiedFunc::FUNC_QUERYHELPER_COMMIT_CARD:
        if (m_Args->arg2 && m_Args->arg3 )  //m_Args->arg2 used to get the cord number
        {
            rv = mHelper->commitCard(*((sal_uInt32*)m_Args->arg2),(nsIAbDirectory*)m_Args->arg3);
        }
        break;
    case ProxiedFunc::FUNC_QUERYHELPER_RESYNC_CARD:
        if (m_Args->arg2)  //m_Args->arg2 used to get the cord number
        {
            rv = mHelper->resyncRow(*((sal_uInt32*)m_Args->arg2));
        }
        break;
    default:
        break;
    }
    return rv;
}
//-------------------------------------------------------------------

#define NS_LDAPCONNECTION_CONTRACTID     "@mozilla.org/network/ldap-connection;1"
#define NS_LDAPOPERATION_CONTRACTID      "@mozilla.org/network/ldap-operation;1"
#define NS_LDAPMESSAGE_CONTRACTID      "@mozilla.org/network/ldap-message;1"
#define NS_LDAPURL_CONTRACTID       "@mozilla.org/network/ldap-url;1"

namespace connectivity {
    namespace mozab {
        class MLDAPMessageListener : public nsILDAPMessageListener
        {
            NS_DECL_ISUPPORTS
            NS_DECL_NSILDAPMESSAGELISTENER

            MLDAPMessageListener();
            virtual ~MLDAPMessageListener();

            sal_Bool    initialized() const;
            sal_Bool    goodConnection() const { return initialized() && m_GoodConnection; }

        protected:

            ::osl::Mutex        m_aMutex;
            ::osl::Condition    m_aCondition;

            sal_Bool    m_IsComplete;
            sal_Bool    m_GoodConnection;

            void        setConnectionStatus( sal_Bool _good );
        };
    }
}

NS_IMPL_THREADSAFE_ISUPPORTS1(MLDAPMessageListener, nsILDAPMessageListener)

MLDAPMessageListener::MLDAPMessageListener()
    : mRefCnt( 0 )
    , m_IsComplete( sal_False )
    , m_GoodConnection( sal_False )
{
    m_aCondition.reset();
}

MLDAPMessageListener::~MLDAPMessageListener()
{
}

sal_Bool MLDAPMessageListener::initialized() const
{
    return const_cast< MLDAPMessageListener* >( this )->m_aCondition.check();
}

void MLDAPMessageListener::setConnectionStatus( sal_Bool _good )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_IsComplete = sal_True;
    m_GoodConnection = _good;

    m_aCondition.set();
}

NS_IMETHODIMP MLDAPMessageListener::OnLDAPInit(nsILDAPConnection* /*aConn*/, nsresult aStatus )
{
    setConnectionStatus( NS_SUCCEEDED( aStatus ) ? sal_True : sal_False );
    return aStatus;
}

NS_IMETHODIMP MLDAPMessageListener::OnLDAPMessage( nsILDAPMessage* aMessage )
{
    nsresult rv;

    PRInt32 messageType;
    rv = aMessage->GetType(&messageType);
    NS_ENSURE_SUCCESS(rv, rv);
    PRInt32 errCode;
    switch (messageType)
    {
    case nsILDAPMessage::RES_BIND:
        rv = aMessage->GetErrorCode(&errCode);
        // if the login failed
        if (errCode != (PRInt32)nsILDAPErrors::SUCCESS) {
            setConnectionStatus( sal_False );
        }
        else
            setConnectionStatus( sal_True );
        break;
    case nsILDAPMessage::RES_SEARCH_RESULT:
        setConnectionStatus( sal_True );
        break;
    default:
        break;
    }

    return NS_OK;
}

//-------------------------------------------------------------------

nsresult
MNSMozabProxy::testLDAPConnection( )
{
    nsresult rv=NS_ERROR_INVALID_ARG;
    switch(m_Args->funcIndex)
    {
    case ProxiedFunc::FUNC_TESTLDAP_INIT_LDAP:
        if (m_Args->arg1 && m_Args->arg4 )
        {
            rv = InitLDAP((sal_Char*)m_Args->arg1,(sal_Unicode*)m_Args->arg2,(sal_Unicode*)m_Args->arg3,(sal_Bool*)m_Args->arg4);
        }
        break;
    case ProxiedFunc::FUNC_TESTLDAP_IS_LDAP_CONNECTED:
        if (m_Args->arg5)
        {
            const MLDAPMessageListener* pListener( static_cast< const MLDAPMessageListener* >( m_Args->arg5 ) );
            if ( pListener->initialized() )
                rv = pListener->goodConnection() ? 0 : (nsresult)PR_NOT_CONNECTED_ERROR;
            else
                rv = (nsresult)PR_CONNECT_TIMEOUT_ERROR;
        }
        break;
    case ProxiedFunc::FUNC_TESTLDAP_RELEASE_RESOURCE:
        if (m_Args->arg5)
        {
            ((MLDAPMessageListener*)m_Args->arg5)->Release();
            delete (MLDAPMessageListener*)m_Args->arg5;
            m_Args->arg5 = NULL;
            rv = 0;
        }
        break;
    default:
        return NS_ERROR_INVALID_ARG;
    }
    return rv;
}
nsresult
MNSMozabProxy::InitLDAP(sal_Char* sUri, sal_Unicode* sBindDN, sal_Unicode* pPasswd,sal_Bool * nUseSSL)
{
    sal_Bool      useSSL    = *nUseSSL;
    nsresult       rv;

    nsCOMPtr<nsILDAPURL> url;
    url = do_CreateInstance(NS_LDAPURL_CONTRACTID, &rv);
    if ( NS_FAILED(rv) )
        return NS_ERROR_INVALID_ARG;

    rv = url->SetSpec( nsDependentCString(sUri) );
    NS_ENSURE_SUCCESS(rv, rv);

    nsCAutoString host;
    rv = url->GetAsciiHost(host);
    NS_ENSURE_SUCCESS(rv, rv);

    PRInt32 port;
    rv = url->GetPort(&port);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCString dn;
    rv = url->GetDn(dn);
    NS_ENSURE_SUCCESS(rv, rv);



    // Get the ldap connection
    nsCOMPtr<nsILDAPConnection> ldapConnection;
    ldapConnection = do_CreateInstance(NS_LDAPCONNECTION_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    MLDAPMessageListener* messageListener =
        new MLDAPMessageListener ( );
    if (messageListener == NULL)
        return NS_ERROR_INVALID_ARG;

    messageListener->AddRef();

    nsCAutoString nsBind;
    // PRUnichar != sal_Unicode in mingw
    nsBind.AssignWithConversion(reinterpret_cast_mingw_only<const PRUnichar *>(sBindDN));

    // Now lets initialize the LDAP connection properly.
    rv = ldapConnection->Init(host.get(), port, useSSL, nsBind,
                              messageListener,NULL,nsILDAPConnection::VERSION3);
    // Initiate the LDAP operation
    nsCOMPtr<nsILDAPOperation> ldapOperation =
    do_CreateInstance(NS_LDAPOPERATION_CONTRACTID, &rv);

    rv = ldapOperation->Init(ldapConnection, messageListener, nsnull);
    if (NS_FAILED(rv))
    return NS_ERROR_UNEXPECTED; // this should never happen

    if ( pPasswd && *pPasswd )
    {
        nsCAutoString nsPassword;
        // PRUnichar != sal_Unicode in mingw
        nsPassword.AssignWithConversion(reinterpret_cast_mingw_only<const PRUnichar *>(pPasswd));
        rv = ldapOperation->SimpleBind(nsPassword);
    }

    if (NS_SUCCEEDED(rv))
        m_Args->arg5 = messageListener;
    return rv;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
