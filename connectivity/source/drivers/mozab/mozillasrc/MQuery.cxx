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


#include <MQueryHelper.hxx>
#include <MNameMapper.hxx>
#include <MConnection.hxx>
#include <connectivity/dbexception.hxx>
#include "MQuery.hxx"
#include "MLdapAttributeMap.hxx"
#include "MTypeConverter.hxx"
#include "MNSMozabProxy.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#include <osl/thread.hxx>

#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */

static NS_DEFINE_CID(kRDFServiceCID, NS_RDFSERVICE_CID);
static NS_DEFINE_CID(kAbDirectoryQueryArgumentsCID, NS_ABDIRECTORYQUERYARGUMENTS_CID);
static NS_DEFINE_CID(kBooleanConditionStringCID, NS_BOOLEANCONDITIONSTRING_CID);
static NS_DEFINE_CID(kBooleanExpressionCID, NS_BOOLEANEXPRESSION_CID);
static NS_DEFINE_CID(kAbDirectoryQueryProxyCID, NS_ABDIRECTORYQUERYPROXY_CID);
static NS_DEFINE_CID(kAbLDAPAttributeMap, NS_IABLDAPATTRIBUTEMAP_IID);

using namespace connectivity::mozab;
using namespace connectivity;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace connectivity;

// -------------------------------------------------------------------------
// Used to store an nsIAbDirectoryQuery member without the need to use Mozilla
// types in the MQuery.hxx file.
//
namespace connectivity {
    namespace mozab {
        struct MQueryDirectory {
            nsCOMPtr<nsIAbDirectory> directory;
            nsCOMPtr<nsIAbDirectoryQuery> directoryQuery;
            PRInt32                       contextId;
            MQueryDirectory() : contextId(-1) {}
        };
    }
}

// -------------------------------------------------------------------------
MQuery::MQuery( const OColumnAlias& _ca )
    :m_rColumnAlias( _ca )
{
    OSL_TRACE( "IN MQuery::MQuery( ca )" );

    construct();

#if OSL_DEBUG_LEVEL > 0
    m_oThreadID = osl::Thread::getCurrentIdentifier();
#endif

    OSL_TRACE( "\tOUT MQuery::MQuery( ca )" );
}
// -------------------------------------------------------------------------
MQuery::~MQuery()
{
    OSL_TRACE("IN MQuery::~MQuery()");

    // MQueryHelper is reference counted, so we need to decrement the
    // count here.
    //
    if ( m_aQueryDirectory->contextId != -1 && m_aQueryDirectory->directoryQuery !=
    NULL )
        m_aQueryDirectory->directoryQuery->StopQuery(m_aQueryDirectory->contextId);

    if ( m_aQueryDirectory )
        delete m_aQueryDirectory;

    NS_IF_RELEASE( m_aQueryHelper);

    OSL_TRACE("\tOUT MQuery::~MQuery()");
}
// -----------------------------------------------------------------------------
void MQuery::construct()
{
     // Set default values. (For now just as a reminder).
    m_aError.reset();
    m_bQuerySubDirs   = sal_True;       // LDAP Queryies require this to be set!
    m_nMaxNrOfReturns = -1; // Unlimited number of returns.

    m_aQueryDirectory = new MQueryDirectory();
    // MQueryHelper is reference counted, so we need to add to the
    // count here to prevent accidental deletion else where...
    //
    m_aQueryHelper = new MQueryHelper();
    NS_IF_ADDREF( m_aQueryHelper);
}
// -------------------------------------------------------------------------
void MQuery::setAddressbook(OUString &ab)
{
    OSL_TRACE("IN MQuery::setAddressbook()");
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aAddressbook = ab;

    OSL_TRACE("\tOUT MQuery::setAddressbook()");
}
// -------------------------------------------------------------------------
void MQuery::setMaxNrOfReturns(const sal_Int32 mnr)
{
    OSL_TRACE( "IN MQuery::setMaxNrOfReturns()" );
    ::osl::MutexGuard aGuard(m_aMutex);

    m_nMaxNrOfReturns = mnr;
    OSL_TRACE("\tOUT MQuery::setMaxNrOfReturns()" );
}
// -------------------------------------------------------------------------
void MQuery::setExpression( MQueryExpression &_expr )
{
    OSL_TRACE("IN MQuery::setExpression()");
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aExpr = _expr;

    OSL_TRACE("\tOUT MQuery::setExpression()");
}
// -------------------------------------------------------------------------
static sal_Int32 generateExpression( MQuery* _aQuery, MQueryExpression*  _aExpr,
                                     nsIAbBooleanExpression* queryExpression )
{
    nsresult rv;        // Store return values.
    // Array that holds all matchItems, to be passed to DoQuery().
    nsCOMPtr<nsISupportsArray> matchItems;
    NS_NewISupportsArray(getter_AddRefs(matchItems));

    // Add every individual boolString to matchItems array.
    nsString matchValue;
    // Initialise the matchItems container.
    MQueryExpression::ExprVector::iterator    evIter;
    for( evIter = _aExpr->getExpressions().begin();
         evIter != _aExpr->getExpressions().end();
         ++evIter )
    {
        if ( (*evIter)->isStringExpr() ) {
            nsCOMPtr<nsIAbBooleanConditionString> boolString = do_CreateInstance (kBooleanConditionStringCID, &rv);
            NS_ENSURE_SUCCESS( rv, rv );

            MQueryExpressionString* evStr = static_cast<MQueryExpressionString*> (*evIter);

            // Set the 'name' property of the boolString.
            // Check if it's an alias first...
            OString attrName = _aQuery->getColumnAlias().getProgrammaticNameOrFallbackToUTF8Alias( evStr->getName() );
            boolString->SetName( strdup( attrName.getStr() ) );
            OSL_TRACE("Name = %s ;", attrName.getStr() );
            // Set the 'matchType' property of the boolString. Check for equal length.
            sal_Bool requiresValue = sal_True;
            switch(evStr->getCond()) {
                case MQueryOp::Exists:
                    OSL_TRACE("MQueryOp::Exists; ");
                    boolString->SetCondition(nsIAbBooleanConditionTypes::Exists);
                    requiresValue = sal_False;
                    break;
                case MQueryOp::DoesNotExist:
                    OSL_TRACE("MQueryOp::DoesNotExist; ");
                    boolString->SetCondition(nsIAbBooleanConditionTypes::DoesNotExist);
                    requiresValue = sal_False;
                    break;
                case MQueryOp::Contains:
                    OSL_TRACE("MQueryOp::Contains; ");
                    boolString->SetCondition(nsIAbBooleanConditionTypes::Contains);
                    break;
                case MQueryOp::DoesNotContain:
                    OSL_TRACE("MQueryOp::DoesNotContain; ");
                    boolString->SetCondition(nsIAbBooleanConditionTypes::DoesNotContain);
                    break;
                case MQueryOp::Is:
                    OSL_TRACE("MQueryOp::Is; ");
                    boolString->SetCondition(nsIAbBooleanConditionTypes::Is);
                    break;
                case MQueryOp::IsNot:
                    OSL_TRACE("MQueryOp::IsNot; ");
                    boolString->SetCondition(nsIAbBooleanConditionTypes::IsNot);
                    break;
                case MQueryOp::BeginsWith:
                    OSL_TRACE("MQueryOp::BeginsWith; ");
                    boolString->SetCondition(nsIAbBooleanConditionTypes::BeginsWith);
                    break;
                case MQueryOp::EndsWith:
                    OSL_TRACE("MQueryOp::EndsWith; ");
                    boolString->SetCondition(nsIAbBooleanConditionTypes::EndsWith);
                    break;
                case MQueryOp::SoundsLike:
                    OSL_TRACE("MQueryOp::SoundsLike; ");
                    boolString->SetCondition(nsIAbBooleanConditionTypes::SoundsLike);
                    break;
                case MQueryOp::RegExp:
                    OSL_TRACE("MQueryOp::RegExp; ");
                    boolString->SetCondition(nsIAbBooleanConditionTypes::RegExp);
                    break;
                default:
                    OSL_TRACE("(default) MQueryOp::Is; ");
                    boolString->SetCondition(nsIAbBooleanConditionTypes::Is);
                    break;
            }
            // Set the 'matchValue' property of the boolString. Value returned in unicode.
            if ( requiresValue )
            {
                OSL_TRACE("Value = %s", OUtoCStr( evStr->getValue() ) );
                MTypeConverter::ouStringToNsString( evStr->getValue(), matchValue);
                boolString->SetValue(matchValue.get ());
            }
            // Add the individual boolString to the container of matchItems.
            matchItems->AppendElement(boolString);
        }
        else if ( (*evIter)->isExpr() ) {
            nsCOMPtr< nsIAbBooleanExpression > subQueryExpr = do_CreateInstance( kBooleanExpressionCID , &rv);
            NS_ENSURE_SUCCESS( rv, rv );
            rv = generateExpression( _aQuery, static_cast< MQueryExpression* >(*evIter),
                                     subQueryExpr );
            NS_ENSURE_SUCCESS( rv, rv );
            matchItems->AppendElement(subQueryExpr);
        }
        else {
            // Should never see this...
            OSL_FAIL("Unknown Expression Type!");
            return( NS_ERROR_UNEXPECTED );
        }
    }

    queryExpression->SetExpressions(matchItems);
    if ( _aExpr->getExpressionCondition() == MQueryExpression::AND )
        queryExpression->SetOperation(nsIAbBooleanOperationTypes::AND);
    else
        queryExpression->SetOperation(nsIAbBooleanOperationTypes::OR);

    return( NS_OK );
}
sal_uInt32 MQuery::InsertLoginInfo(OConnection* _pCon)
{
    nsresult rv;        // Store return values.

    OUString nameAB    = _pCon->getHost().replace('.','_');
    OUString bindDN    = _pCon->getBindDN();
    OUString password  = _pCon->getPassword();
    sal_Bool      useSSL    = _pCon->getUseSSL();

    nsCOMPtr<nsIPref> prefs = do_GetService(NS_PREF_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    // create the ldap maxHits entry for the preferences file.
    // Note: maxHits is applicable to LDAP only in mozilla.
    nsCAutoString prefName(NS_LITERAL_CSTRING("ldap_2.servers."));
    const char *pAddressBook = MTypeConverter::ouStringToCCharStringAscii(nameAB.getStr());
    prefName.Append(pAddressBook);

    if (!bindDN.isEmpty())
    {
        nsCAutoString bindPrefName=prefName;
        bindPrefName.Append(NS_LITERAL_CSTRING(".auth.dn"));
        rv = prefs->SetCharPref (bindPrefName.get(),
            MTypeConverter::ouStringToCCharStringAscii( bindDN.getStr() ) );
        NS_ENSURE_SUCCESS(rv, rv);

        nsCAutoString pwdPrefName=prefName;
        pwdPrefName.Append(NS_LITERAL_CSTRING(".auth.pwd"));
        rv = prefs->SetCharPref (pwdPrefName.get(),
            MTypeConverter::ouStringToCCharStringAscii( password.getStr() ) );
        NS_ENSURE_SUCCESS(rv, rv);
    }
    if (useSSL)
    {
        nsCAutoString sslPrefName=prefName;
        sslPrefName.Append(NS_LITERAL_CSTRING(".UseSSL"));
        rv = prefs->SetBoolPref (sslPrefName.get(),useSSL);
        NS_ENSURE_SUCCESS(rv, rv);
    }
    return rv;
}

//determine whether current profile is locked,any error will lead to return true
sal_Bool isProfileLocked(OConnection* _pCon)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XMozillaBootstrap > xMozillaBootstrap;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    OSL_ENSURE( xFactory.is(), "can't get service factory" );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInstance = xFactory->createInstance("com.sun.star.mozilla.MozillaBootstrap");
    OSL_ENSURE( xInstance.is(), "failed to create instance" );
    xMozillaBootstrap = ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XMozillaBootstrap >(xInstance,::com::sun::star::uno::UNO_QUERY);
    if (_pCon)
        return xMozillaBootstrap->isProfileLocked(_pCon->getProduct(),_pCon->getMozProfile());
    else
        return xMozillaBootstrap->isCurrentProfileLocked();
}


// -------------------------------------------------------------------------
sal_Int32 getDirectoryType(const nsIAbDirectory*  directory)
{
    nsresult retCode;
    nsCOMPtr<nsIRDFResource> rdfResource = do_QueryInterface((nsISupports *)directory, &retCode) ;
    if (NS_FAILED(retCode)) { return SDBCAddress::Unknown; }
    const char * uri;
    retCode=rdfResource->GetValueConst(&uri);
    if (NS_FAILED(retCode)) { return SDBCAddress::Unknown; }
    const sal_Char *sUriPrefix = ::connectivity::mozab::getSchemeURI( ::connectivity::mozab::SCHEME_LDAP );
    if (strncmp(uri,sUriPrefix,strlen(sUriPrefix)) == 0)
    {
        return SDBCAddress::LDAP;
    }
    sUriPrefix = ::connectivity::mozab::getSchemeURI( ::connectivity::mozab::SCHEME_MOZILLA );
    if (strncmp(uri,sUriPrefix,strlen(sUriPrefix)) == 0)
    {
        return SDBCAddress::Mozilla;
    }
    sUriPrefix = ::connectivity::mozab::getSchemeURI( ::connectivity::mozab::SCHEME_MOZILLA_MDB );
    if (strncmp(uri,sUriPrefix,strlen(sUriPrefix)) == 0)
    {
        return SDBCAddress::Mozilla;
    }
    sUriPrefix = ::connectivity::mozab::getSchemeURI( ::connectivity::mozab::SCHEME_OUTLOOK_EXPRESS );
    if (strncmp(uri,sUriPrefix,strlen(sUriPrefix)) == 0)
    {
        return SDBCAddress::OutlookExp;
    }
    sUriPrefix = ::connectivity::mozab::getSchemeURI( ::connectivity::mozab::SCHEME_OUTLOOK_MAPI );
    if (strncmp(uri,sUriPrefix,strlen(sUriPrefix)) == 0)
    {
        return SDBCAddress::Outlook;
    }
    return SDBCAddress::Unknown;

}
// -------------------------------------------------------------------------
sal_Bool isForceQueryProxyUsed(const nsIAbDirectory*  directory)
{
    sal_Int32 nType = getDirectoryType(directory);
    if (nType == SDBCAddress::Outlook || nType == SDBCAddress::OutlookExp)
        return sal_True;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 MQuery::commitRow(const sal_Int32 rowIndex)
{
    if (!m_aQueryHelper || !m_aQueryDirectory || !m_aQueryDirectory->directoryQuery)
        return sal_False;
        MNSMozabProxy xMProxy;
    RunArgs args;
    args.funcIndex = ProxiedFunc::FUNC_QUERYHELPER_COMMIT_CARD;
    args.argCount = 3;
    args.arg1 = (void*)m_aQueryHelper;
    args.arg2 = (void*)&rowIndex;
    args.arg3 = (void*)m_aQueryDirectory->directory;
    nsresult rv = xMProxy.StartProxy(&args,m_Product,m_Profile);
    m_aError = m_aQueryHelper->getError();
    return rv;
}

// -------------------------------------------------------------------------
sal_Int32 MQuery::deleteRow(const sal_Int32 rowIndex)
{
    if (!m_aQueryHelper || !m_aQueryDirectory || !m_aQueryDirectory->directoryQuery)
        return sal_False;
    MNSMozabProxy xMProxy;
    RunArgs args;
    args.funcIndex = ProxiedFunc::FUNC_QUERYHELPER_DELETE_CARD;
    args.argCount = 3;
    args.arg1 = (void*)m_aQueryHelper;
    args.arg2 = (void*)&rowIndex;
    args.arg3 = (void*)m_aQueryDirectory->directory;
    nsresult rv = xMProxy.StartProxy(&args,m_Product,m_Profile);
    m_aError = m_aQueryHelper->getError();
    return rv;

}

// -------------------------------------------------------------------------
sal_Int32 MQuery::executeQuery(OConnection* _pCon)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OSL_TRACE("IN MQuery::executeQuery()");
    m_Product = _pCon->getProduct();
    m_Profile = _pCon->getMozProfile();

    nsresult rv;
    MNSMozabProxy xMProxy;
    RunArgs args;
    args.funcIndex = ProxiedFunc::FUNC_EXECUTE_QUERY;
    args.argCount = 2;
    args.arg1 = (void*)this;
    args.arg2 = (void*)_pCon;
    rv = xMProxy.StartProxy(&args,m_Product,m_Profile);
    return rv;
}
// -------------------------------------------------------------------------
sal_Int32 MQuery::executeQueryProxied(OConnection* _pCon)
{
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE("IN MQuery::executeQueryProxied() Caller thread: %4d", m_oThreadID);
#endif

    nsresult rv;        // Store return values.
    //  MTypeConverter aTypeConverter;
    // Create a nsIAbDirectory object to initialise the nsIAbDirectoryQuery object.
    nsCOMPtr<nsIRDFService> rdfService(do_GetService(kRDFServiceCID, &rv)) ;
    if (NS_FAILED(rv))
        return(-1);

    nsCOMPtr<nsIAbDirectory> directory;
    MNameMapper *nmap = _pCon->getNameMapper();


    if ( nmap->getDir( m_aAddressbook, getter_AddRefs( directory ) ) == sal_False )
        return( -1 );


    //insert ldap bind info to mozilla profile(in memery,none saved),so we can use it in mozilla part codes
    if (_pCon->isLDAP())
    {
          rv = InsertLoginInfo(_pCon);
        NS_ENSURE_SUCCESS(rv, rv);

    }
    // Since Outlook Express and Outlook in OCL mode support a very limited query capability,
    // we use the following bool to judge whether we need bypass any use of a DirectoryQuery
    // interface and instead force the use of the QueryProxy.
    sal_Bool forceQueryProxyUse = isForceQueryProxyUsed(directory);

     m_aQueryDirectory->directory = directory;
    // Initialize directory in cases of LDAP and Mozilla
    if (!forceQueryProxyUse) m_aQueryDirectory->directoryQuery = do_QueryInterface(directory, &rv);

    if ( NS_FAILED(rv) || forceQueryProxyUse)
    {
        // Create a nsIAbDirectoryQuery object which eventually will execute
        // the query by calling DoQuery().
        nsCOMPtr< nsIAbDirectoryQueryProxy > directoryQueryProxy = do_CreateInstance( kAbDirectoryQueryProxyCID, &rv);

        // Need to turn this off for anything using the Query Proxy since it
        // treats Mailing Lists as directories!

        m_bQuerySubDirs = sal_False;

        rv = directoryQueryProxy->Initiate (directory);
        NS_ENSURE_SUCCESS(rv, rv);

        m_aQueryDirectory->directoryQuery = do_QueryInterface (directoryQueryProxy, &rv);
        NS_ENSURE_SUCCESS(rv, rv);
        OSL_TRACE("Using the directoryQueryProxy");
    }
#if OSL_DEBUG_LEVEL > 0
    else
        OSL_TRACE("Not using a Query Proxy, Query i/f supported by directory");
#endif /* OSL_DEBUG_LEVEL */

    /*
    // The problem here is that an LDAP Address Book may exist as
    // a Mozilla Address Book. So we need to limit the number of
    // records returned by the Server:
    // 1. Determine if this is an LDAP Address Book
    // [LDAP overrides the default operations(write|read|search) of all types with search only].
    // 2. Determine if the limit is already set by us.
    // 3. Use the mozilla preferences to see if this value is set.
    // 4. Use value or else default to 100.
    */
    PRBool isWriteable;
    rv = directory->GetOperations (&isWriteable);
    NS_ENSURE_SUCCESS(rv, rv);
    if (!(isWriteable & nsIAbDirectory::opWrite))
    {
        if(m_nMaxNrOfReturns == -1)
        {
            // Determine if the limit maxHits has been set in the mozilla preferences
            // if set, then use the value otherwise default to 100
            nsCOMPtr<nsIPref> prefs = do_GetService(NS_PREF_CONTRACTID, &rv);
            NS_ENSURE_SUCCESS(rv, rv);

            // create the ldap maxHits entry for the preferences file.
            // Note: maxHits is applicable to LDAP only in mozilla.
            nsCAutoString prefName(NS_LITERAL_CSTRING("ldap_2.servers."));
            const char *pAddressBook = MTypeConverter::ouStringToCCharStringAscii(m_aAddressbook);
            prefName.Append(pAddressBook);
            prefName.Append(NS_LITERAL_CSTRING(".maxHits"));

           PRInt32 maxHits;
           rv = prefs->GetIntPref(prefName.get(), &maxHits);
           if (NS_FAILED(rv))
               m_nMaxNrOfReturns = 100;
           else
               m_nMaxNrOfReturns = maxHits;
        }
    }


    nsCOMPtr< nsIAbBooleanExpression > queryExpression = do_CreateInstance( kBooleanExpressionCID , &rv);
    NS_ENSURE_SUCCESS( rv, rv );
    rv = generateExpression( this, &m_aExpr, queryExpression );
    NS_ENSURE_SUCCESS( rv, rv );

    // Use the nsIAbCard to return the card properties.
    const char    *returnProperties[] = {"card:nsIAbCard"};
    PRInt32   count=1;

    nsCOMPtr< nsIAbDirectoryQueryArguments > arguments = do_CreateInstance( kAbDirectoryQueryArgumentsCID, &rv);
    NS_ENSURE_SUCCESS( rv, rv );

    rv = arguments->SetExpression(queryExpression);
    NS_ENSURE_SUCCESS( rv, rv );

    rv = arguments->SetReturnProperties(count, returnProperties);
    NS_ENSURE_SUCCESS( rv, rv );

    rv = arguments->SetQuerySubDirectories(m_bQuerySubDirs);
    NS_ENSURE_SUCCESS( rv, rv );

    nsCOMPtr< nsIAbLDAPAttributeMap > attributeMap( new MLdapAttributeMap );
    rv = arguments->SetTypeSpecificArg( attributeMap );
    NS_ENSURE_SUCCESS( rv, rv );

    // Execute the query.
    OSL_TRACE( "****** calling DoQuery");

    m_aError.reset();

    m_aQueryHelper->reset();

    rv = m_aQueryDirectory->directoryQuery->DoQuery(arguments, m_aQueryHelper, m_nMaxNrOfReturns, -1, &m_aQueryDirectory->contextId);


    if (NS_FAILED(rv))  {
        m_aQueryDirectory->contextId = -1;
        OSL_TRACE( "****** DoQuery failed");
        OSL_TRACE("\tOUT MQuery::executeQueryProxied()\n");
        m_aQueryHelper->notifyQueryError() ;
        return(-1);
    }
#if OSL_DEBUG_LEVEL > 0
    else {
        OSL_TRACE( "****** DoQuery succeeded");
    }
#endif

    OSL_TRACE("\tOUT MQuery::executeQueryProxied()");

    return(0);
}

// -------------------------------------------------------------------------
//
//
//  If the query executed is being done asynchronously then this may return
//  -1 as the count, ie. it's undetermined.
//
sal_Int32
MQuery::getRowCount()
{
    return( m_aQueryHelper->getResultCount() );
}

// -------------------------------------------------------------------------
//
//
// As opposed to getRowCount() this returns the actual number of rows fetched
// so far (if is an async query)
//
sal_uInt32
MQuery::getRealRowCount()
{
    return( m_aQueryHelper->getRealCount() );
}

//
//  If the query executed is being done asynchronously then this may be
//  false
//
sal_Bool
MQuery::queryComplete( void )
{
    return( hadError() || m_aQueryHelper->queryComplete() );
}

sal_Bool
MQuery::waitForQueryComplete( void )
{
    if( m_aQueryHelper->waitForQueryComplete( ) )
        return sal_True;
    m_aError = m_aQueryHelper->getError();
    return( sal_False );
}

// -------------------------------------------------------------------------

sal_Bool
MQuery::checkRowAvailable( sal_Int32 nDBRow )
{
    while( !queryComplete() && m_aQueryHelper->getRealCount() <= (sal_uInt32)nDBRow )
        if ( !m_aQueryHelper->waitForRow( nDBRow ) ) {
            m_aError = m_aQueryHelper->getError();
            return( sal_False );
        }

    return( getRowCount() > nDBRow );
}
// -------------------------------------------------------------------------
sal_Bool
MQuery::setRowValue( ORowSetValue& rValue, sal_Int32 nDBRow,const OUString& aDBColumnName, sal_Int32 nType ) const
{
    MQueryHelperResultEntry*   xResEntry = m_aQueryHelper->getByIndex( nDBRow );

    OSL_ENSURE( xResEntry != NULL, "xResEntry == NULL");
    if (xResEntry == NULL )
    {
        const_cast< MQuery* >( this )->m_aError = m_aQueryHelper->getError();
        return sal_False;
    }
    switch ( nType )
    {
        case DataType::VARCHAR:
            xResEntry->setValue( m_rColumnAlias.getProgrammaticNameOrFallbackToUTF8Alias( aDBColumnName ), rValue.getString() );
            break;
        default:
            OSL_FAIL( "invalid data type!" );
            break;
    }

    return sal_True;
}

// -------------------------------------------------------------------------
sal_Bool
MQuery::getRowValue( ORowSetValue& rValue, sal_Int32 nDBRow,const OUString& aDBColumnName, sal_Int32 nType ) const
{
    MQueryHelperResultEntry*   xResEntry = m_aQueryHelper->getByIndex( nDBRow );

    OSL_ENSURE( xResEntry != NULL, "xResEntry == NULL");
    if (xResEntry == NULL )
    {
        const_cast< MQuery* >( this )->m_aError = m_aQueryHelper->getError();
        rValue.setNull();
        return sal_False;
    }
    switch ( nType )
    {
        case DataType::VARCHAR:
            rValue = xResEntry->getValue( m_rColumnAlias.getProgrammaticNameOrFallbackToUTF8Alias( aDBColumnName ) );
            break;

        default:
            rValue.setNull();
            break;
    }

    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32
MQuery::getRowStates(sal_Int32 nDBRow)
{
    MQueryHelperResultEntry*   xResEntry = m_aQueryHelper->getByIndex( nDBRow );

    OSL_ENSURE( xResEntry != NULL, "xResEntry == NULL");
    if (xResEntry == NULL )
    {
        m_aError = m_aQueryHelper->getError();
        return RowStates_Error;
    }
    return xResEntry->getRowStates();
}
sal_Bool
MQuery::setRowStates(sal_Int32 nDBRow,sal_Int32 aState)
{
    MQueryHelperResultEntry*   xResEntry = m_aQueryHelper->getByIndex( nDBRow );

    OSL_ENSURE( xResEntry != NULL, "xResEntry == NULL");
    if (xResEntry == NULL )
    {
        m_aError = m_aQueryHelper->getError();
        return sal_False;
    }
    return xResEntry->setRowStates(aState);
}

sal_Bool
MQuery::resyncRow(sal_Int32 nDBRow)
{
    MNSMozabProxy xMProxy;
    RunArgs args;
    args.funcIndex = ProxiedFunc::FUNC_QUERYHELPER_RESYNC_CARD;
    args.argCount = 2;
    args.arg1 = (void*)m_aQueryHelper;
    args.arg2 = (void*)&nDBRow;
    nsresult rv = xMProxy.StartProxy(&args,m_Product,m_Profile);
    m_aError = m_aQueryHelper->getError();
    return NS_SUCCEEDED( rv ) ? sal_True : sal_False;
}

sal_Int32
MQuery::createNewCard()
{
    sal_Int32 nNumber = 0;
    MNSMozabProxy xMProxy;
    RunArgs args;
    args.funcIndex = ProxiedFunc::FUNC_QUERYHELPER_CREATE_NEW_CARD;
    args.argCount = 2;
    args.arg1 = (void*)m_aQueryHelper;
    args.arg2 = (void*)&nNumber;
    nsresult rv = xMProxy.StartProxy(&args,m_Product,m_Profile);

    m_aError = m_aQueryHelper->getError();
    NS_ENSURE_SUCCESS(rv,0);
    return nNumber;
}
// -------------------------------------------------------------------------

MNameMapper*
MQuery::CreateNameMapper()
{
    return( new MNameMapper() );
}

// -------------------------------------------------------------------------
void
MQuery::FreeNameMapper( MNameMapper* _ptr )
{
    delete _ptr;
}
// -------------------------------------------------------------------------
sal_Bool MQuery::isWritable(OConnection* _pCon)
{
    if ( !m_aQueryDirectory )
        return sal_False;

    nsresult rv;        // Store return values.
    nsCOMPtr<nsIAbDirectory> directory = do_QueryInterface(m_aQueryDirectory->directory, &rv);;
    if (NS_FAILED(rv))
        return sal_False;
    if (getDirectoryType(directory) == SDBCAddress::Mozilla && isProfileLocked(_pCon))
        return sal_False;

    PRBool isWriteable;
    rv = directory->GetOperations (&isWriteable);
    if (NS_FAILED(rv))
        return sal_False;
    sal_Bool bWritable = ( isWriteable & nsIAbDirectory::opWrite ) == nsIAbDirectory::opWrite;
    return  bWritable;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
