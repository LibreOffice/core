#ifndef _CONNECTIVITY_MAB_MOZABHELPER_HXX_
#include "MNSMozabProxy.hxx"
#endif

#ifndef _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_
#include "MDatabaseMetaDataHelper.hxx"
#endif
#ifndef _CONNECTIVITY_MAB_QUERY_HXX_
#include "MQuery.hxx"
#endif

#include <nsIProxyObjectManager.h>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
// More Mozilla includes for LDAP Connection Test
#include "prprf.h"
#include "nsILDAPURL.h"
#include "nsILDAPMessage.h"
#include "nsILDAPMessageListener.h"
#include "nsILDAPErrors.h"
#include "nsILDAPConnection.h"
#include "nsILDAPOperation.h"

#ifndef _CONNECTIVITY_MAB_QUERY_HXX_
#include "MQuery.hxx"
#endif
#ifndef _CONNECTIVITY_MAB_QUERYHELPER_HXX_
#include <MQueryHelper.hxx>
#endif


#define TYPEASSERT(value,type) if (value != type) return !NS_OK;

using namespace connectivity::mozab;

/* Implementation file */
NS_IMPL_ISUPPORTS1(MNSMozabProxy, nsIRunnable)

static ::osl::Mutex m_aThreadMutex;
extern nsresult NewAddressBook(const ::rtl::OUString * aName);

nsIRunnable * MNSMozabProxy::ProxiedObject()
{
    if (!_ProxiedObject)
    {

        nsresult rv = NS_GetProxyForObject(NS_UI_THREAD_EVENTQ,
                NS_GET_IID(nsIRunnable),
                this,
                PROXY_SYNC,
                (void**)&_ProxiedObject);

        _ProxiedObject->AddRef();
    }
    return _ProxiedObject;
}

MNSMozabProxy::MNSMozabProxy()
{
  NS_INIT_ISUPPORTS();
  _ProxiedObject=NULL;
  m_Args = NULL;
#if OSL_DEBUG_LEVEL > 0
  m_oThreadID = osl_getThreadIdentifier(NULL);
#endif
  AddRef();
}

MNSMozabProxy::~MNSMozabProxy()
{
}
sal_Int32 MNSMozabProxy::StartProxy(RunArgs * args)
{
    OSL_TRACE( "IN : MNSMozabProxy::StartProxy() \n" );
    ::osl::MutexGuard aGuard(m_aThreadMutex);
    m_Args = args;
    return ProxiedObject()->Run();
}

extern nsresult getTableStringsProxied(const sal_Char* sAbURI, sal_Int32 *nDirectoryType,MNameMapper *nmap,
                        ::std::vector< ::rtl::OUString >*   _rStrings,
                        ::std::vector< ::rtl::OUString >*   _rTypes,
                        rtl::OUString * sError);

NS_IMETHODIMP MNSMozabProxy::Run()
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "IN : MNSMozabProxy::Run() Caller thread :%4d \n" , m_oThreadID );
#else
    OSL_TRACE( "IN : MNSMozabProxy::Run() \n" );
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
                                (::std::vector< ::rtl::OUString >*)m_Args->arg4,
                                (::std::vector< ::rtl::OUString >*)m_Args->arg5,
                                (rtl::OUString *)m_Args->arg6);
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
            rv = NewAddressBook((const ::rtl::OUString*)m_Args->arg1 );
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
            rv = mHelper->deleteCard(*((sal_Int32*)m_Args->arg2),(nsIAbDirectory*)m_Args->arg3);
        }
        break;
    case ProxiedFunc::FUNC_QUERYHELPER_COMMIT_CARD:
        if (m_Args->arg2 && m_Args->arg3 )  //m_Args->arg2 used to get the cord number
        {
            rv = mHelper->commitCard(*((sal_Int32*)m_Args->arg2),(nsIAbDirectory*)m_Args->arg3);
        }
        break;
    case ProxiedFunc::FUNC_QUERYHELPER_RESYNC_CARD:
        if (m_Args->arg2)  //m_Args->arg2 used to get the cord number
        {
            rv = mHelper->resyncRow(*((sal_Int32*)m_Args->arg2));
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
        class MLDAPMessageListener : public nsILDAPMessageListener {
            NS_DECL_ISUPPORTS
            NS_DECL_NSILDAPMESSAGELISTENER

            MLDAPMessageListener();
            ~MLDAPMessageListener();

            sal_Bool    connected();
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

sal_Bool MLDAPMessageListener::connected()
{

    return m_aCondition.check();
}

void MLDAPMessageListener::setConnectionStatus( sal_Bool _good )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_IsComplete = sal_True;
    m_GoodConnection = _good;

    m_aCondition.set();
}

NS_IMETHODIMP MLDAPMessageListener::OnLDAPInit(nsILDAPConnection *aConn, nsresult aStatus )
{

    // Make sure that the Init() worked properly
    if ( NS_FAILED(aStatus ) ) {
        setConnectionStatus( sal_False );
    }
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
        if (errCode != nsILDAPErrors::SUCCESS) {
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
            if ( ((MLDAPMessageListener*)m_Args->arg5)->connected())
            {
                rv = 0;
            }
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
MNSMozabProxy::InitLDAP(sal_Char* sUri, sal_Unicode* sBindDN, sal_Unicode* sPasswd,sal_Bool * nUseSSL)
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
    nsBind.AssignWithConversion(sBindDN);

    // Now lets initialize the LDAP connection properly.
    rv = ldapConnection->Init(host.get(), port, useSSL, nsBind,
                              messageListener,NULL,nsILDAPConnection::VERSION3);
    // Initiate the LDAP operation
    nsCOMPtr<nsILDAPOperation> ldapOperation =
    do_CreateInstance(NS_LDAPOPERATION_CONTRACTID, &rv);

    rv = ldapOperation->Init(ldapConnection, messageListener, nsnull);
    if (NS_FAILED(rv))
    return NS_ERROR_UNEXPECTED; // this should never happen

    nsCAutoString nsPassword;
    nsPassword.AssignWithConversion(sPasswd);

    rv = ldapOperation->SimpleBind(nsPassword);
    if (NS_SUCCEEDED(rv))
        m_Args->arg5 = messageListener;
    return rv;
}
