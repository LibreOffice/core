/*************************************************************************
 *
 *  $RCSfile: MQuery.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:39:04 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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
 *  Contributor(s): Willem van Dorp, Darren Kenny
 *
 *
 ************************************************************************/

#include <MQueryHelper.hxx>
#include <MNameMapper.hxx>
#include <MConnection.hxx>

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_MAB_QUERY_HXX_
#include "MQuery.hxx"
#endif
#ifndef _CONNECTIVITY_MAB_CONVERSIONS_HXX_
#include "MTypeConverter.hxx"
#endif

#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */

static NS_DEFINE_CID(kRDFServiceCID, NS_RDFSERVICE_CID);
static NS_DEFINE_CID(kAbDirectoryQueryArgumentsCID, NS_ABDIRECTORYQUERYARGUMENTS_CID);
static NS_DEFINE_CID(kBooleanConditionStringCID, NS_BOOLEANCONDITIONSTRING_CID);
static NS_DEFINE_CID(kBooleanExpressionCID, NS_BOOLEANEXPRESSION_CID);
static NS_DEFINE_CID(kAbDirectoryQueryProxyCID, NS_ABDIRECTORYQUERYPROXY_CID);

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
            nsCOMPtr<nsIAbDirectoryQuery> directory;
            PRInt32                       contextId;

            MQueryDirectory() : contextId(-1) {}
        };
    }
}

// -------------------------------------------------------------------------
MQuery::MQuery()
{
    OSL_TRACE( "IN MQuery::MQuery()\n" );

   construct();

    OSL_TRACE( "\tOUT MQuery::MQuery()\n" );
}
// -------------------------------------------------------------------------
MQuery::MQuery(const ::std::map< ::rtl::OUString, ::rtl::OUString>  & ca)
{
    OSL_TRACE( "IN MQuery::MQuery( ca )\n" );

    construct();

    m_aColumnAliasMap = ca;

    OSL_TRACE( "\tOUT MQuery::MQuery( ca )\n" );
}
// -------------------------------------------------------------------------
MQuery::~MQuery()
{
    OSL_TRACE("IN MQuery::~MQuery()\n");

    // MQueryHelper is reference counted, so we need to decrement the
    // count here.
    //
    if ( m_aQueryDirectory->contextId != -1 && m_aQueryDirectory->directory !=
    NULL )
        m_aQueryDirectory->directory->StopQuery(m_aQueryDirectory->contextId);

    if ( m_aQueryDirectory )
        delete m_aQueryDirectory;

    NS_IF_RELEASE( m_aQueryHelper);

    OSL_TRACE("\tOUT MQuery::~MQuery()\n");
}
// -----------------------------------------------------------------------------
void MQuery::construct()
{
     // Set default values. (For now just as a reminder).
    m_aErrorOccurred  = sal_False;
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
void MQuery::setAttributes(::std::vector< ::rtl::OUString> &attrs)
{
    OSL_TRACE("IN MQuery::setAttributes()\n");
    ::osl::MutexGuard aGuard( m_aMutex );

    m_aAttributes.clear();
    m_aAttributes.reserve(attrs.size());
    ::std::vector< ::rtl::OUString>::iterator aIterAttr = attrs.begin();
    ::std::map< ::rtl::OUString, ::rtl::OUString>::iterator aIterMap;
    for(aIterAttr; aIterAttr != attrs.end();++aIterAttr)
    {
        aIterMap = m_aColumnAliasMap.find(*aIterAttr);
        if (aIterMap == m_aColumnAliasMap.end()) {
            // Not found.
            m_aAttributes.push_back(*aIterAttr);
        } else {
            m_aAttributes.push_back(aIterMap->second);
        }
    }

    OSL_TRACE("\tOUT MQuery::setAttributes()\n");
}
// -------------------------------------------------------------------------
const ::std::vector< ::rtl::OUString> &MQuery::getAttributes() const
{
    OSL_TRACE("IN MQuery::getAttributes()\n");

    OSL_TRACE("\tOUT MQuery::getAttributes()\n");

    return(m_aAttributes);
}
// -------------------------------------------------------------------------
void MQuery::setAddressbook(::rtl::OUString &ab)
{
    OSL_TRACE("IN MQuery::setAddressbook()\n");
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aAddressbook = ab;

    OSL_TRACE("\tOUT MQuery::setAddressbook()\n");
}
// -------------------------------------------------------------------------
::rtl::OUString MQuery::getAddressbook() const
{
    OSL_TRACE("IN MQuery::getAddressbook()\n");

    OSL_TRACE("\tOUT MQuery::getAddressbook()\n");

    return(m_aAddressbook);
}
// -------------------------------------------------------------------------
void MQuery::setMaxNrOfReturns(const sal_Int32 mnr)
{
    OSL_TRACE( "IN MQuery::setMaxNrOfReturns()\n" );
    ::osl::MutexGuard aGuard(m_aMutex);

    m_nMaxNrOfReturns = mnr;
    OSL_TRACE("\tOUT MQuery::setMaxNrOfReturns()\n" );
}
// -------------------------------------------------------------------------
sal_Int32 MQuery::getMaxNrOfReturns() const
{
    OSL_TRACE("IN MQuery::getMaxNrOfReturns()\n");

    OSL_TRACE("\tOUT MQuery::getMaxNrOfReturns()\n");

    return(m_nMaxNrOfReturns);
}
// -------------------------------------------------------------------------
void MQuery::setQuerySubDirs(sal_Bool &qsd)
{
    OSL_TRACE("IN MQuery::setQuerySubDirs()\n");
    ::osl::MutexGuard aGuard(m_aMutex);

    m_bQuerySubDirs = qsd;
    OSL_TRACE("\tOUT MQuery::setQuerySubDirs()\n");
}
// -------------------------------------------------------------------------
sal_Bool MQuery::getQuerySubDirs() const
{
    OSL_TRACE("IN MQuery::getQuerySubDirs()\n");

    OSL_TRACE("\tOUT MQuery::getQuerySubDirs()\n");

    return(m_bQuerySubDirs);
}
// -------------------------------------------------------------------------
void MQuery::setExpression( MQueryExpression &_expr )
{
    OSL_TRACE("IN MQuery::setExpression()\n");
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aExpr = _expr;

    OSL_TRACE("\tOUT MQuery::setExpression()\n");
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
            rtl::OUString attrName;
            ::std::map< ::rtl::OUString, ::rtl::OUString>::const_iterator aIterMap;
            aIterMap = _aQuery->getColumnAliasMap().find(evStr->getName());
            if (aIterMap == _aQuery->getColumnAliasMap().end()) {
                // Not found.
                attrName = evStr->getName();
            } else {
                attrName = aIterMap->second;
            }
            string aMiName = MTypeConverter::ouStringToStlString(attrName);
            boolString->SetName(strdup(aMiName.c_str()));
            OSL_TRACE("Name = %s ;", aMiName.c_str() );
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
                OSL_TRACE("Value = %s \n", OUtoCStr( evStr->getValue() ) );
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
            OSL_ASSERT("Unknown Expression Type!");
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

// -------------------------------------------------------------------------
sal_Int32 MQuery::executeQuery(OConnection* _pCon)
{
    OSL_TRACE("IN MQuery::executeQuery()\n");
    ::osl::MutexGuard aGuard(m_aMutex);


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

    SDBCAddress::sdbc_address_type eSDBCAddressType = _pCon->getSDBCAddressType();

    // Since Outlook Express and Outlook in OCL mode support a very limited query capability,
    // we use the following bool to judge whether we need bypass any use of a DirectoryQuery
    // interface and instead force the use of the QueryProxy.
    sal_Bool forceQueryProxyUse = (eSDBCAddressType == SDBCAddress::Outlook) || (eSDBCAddressType == SDBCAddress::OutlookExp);

    // Initialize directory in cases of LDAP and Mozilla
    if (!forceQueryProxyUse) m_aQueryDirectory->directory = do_QueryInterface(directory, &rv);

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

        m_aQueryDirectory->directory = do_QueryInterface (directoryQueryProxy, &rv);
        NS_ENSURE_SUCCESS(rv, rv);
        OSL_TRACE("Using the directoryQueryProxy\n");
    }
#if OSL_DEBUG_LEVEL > 0
    else
        OSL_TRACE("Not using a Query Proxy, Query i/f supported by directory\n");
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

    // Execute the query.
    OSL_TRACE( "****** calling DoQuery\n");

    m_aErrorOccurred = sal_False;

    m_aQueryHelper->reset();

    rv = m_aQueryDirectory->directory->DoQuery(arguments, m_aQueryHelper, m_nMaxNrOfReturns, -1, &m_aQueryDirectory->contextId);


    if (NS_FAILED(rv))  {
        m_aQueryDirectory->contextId = -1;
        OSL_TRACE( "****** DoQuery failed\n");
        OSL_TRACE("\tOUT MQuery::executeQuery()\n");
        m_aQueryHelper->notifyQueryError() ;
        return(-1);
    }
#if OSL_DEBUG_LEVEL > 0
    else {
        OSL_TRACE( "****** DoQuery succeeded \n");
    }
#endif

    OSL_TRACE("\tOUT MQuery::executeQuery()\n");

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
    return( m_aErrorOccurred || m_aQueryHelper->queryComplete() );
}

sal_Bool
MQuery::waitForQueryComplete( void )
{
    if( m_aQueryHelper->waitForQueryComplete( m_aErrorString ) )
        return sal_True;

    m_aErrorOccurred = sal_True;
    return( sal_False );
}

// -------------------------------------------------------------------------

sal_Bool
MQuery::checkRowAvailable( sal_Int32 nDBRow )
{
    while( !queryComplete() && m_aQueryHelper->getRealCount() <= (sal_uInt32)nDBRow )
        if ( !m_aQueryHelper->waitForRow( nDBRow, m_aErrorString ) ) {
            m_aErrorOccurred = sal_True;
            return( sal_False );
        }

    return( getRowCount() > nDBRow );
}

// -------------------------------------------------------------------------
sal_Bool
MQuery::getRowValue( ORowSetValue& rValue, sal_Int32 nDBRow,const rtl::OUString& aDBColumnName, sal_Int32 nType ) const
{
    MQueryHelperResultEntry*   xResEntry = m_aQueryHelper->getByIndex( nDBRow, m_aErrorString );

    OSL_ENSURE( xResEntry != NULL, "xResEntry == NULL");
    if (xResEntry == NULL )
    {
        m_aErrorOccurred = sal_True;
        rValue.setNull();
        return sal_False;
    }
    switch ( nType )
    {
        case DataType::VARCHAR:
            {
                ::std::map< ::rtl::OUString, ::rtl::OUString>::const_iterator aIterMap = m_aColumnAliasMap.find(aDBColumnName);
                if (aIterMap != m_aColumnAliasMap.end())
                    rValue = xResEntry->getValue(aIterMap->second);
                else
                    rValue = xResEntry->getValue( aDBColumnName );
            }
            break;
        default:
            rValue.setNull();
            break;
    }

    return sal_True;
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
