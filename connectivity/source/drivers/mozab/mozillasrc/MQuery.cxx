/*************************************************************************
 *
 *  $RCSfile: MQuery.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-10-15 12:59:14 $
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

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_MAB_QUERY_HXX_
#include "MQuery.hxx"
#endif
#ifndef _CONNECTIVITY_MAB_CONVERSIONS_HXX_
#include "MTypeConverter.hxx"
#endif

#ifdef DEBUG
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* DEBUG */
# define OUtoCStr( x ) ("dummy")
#endif /* DEBUG */

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

    return;
}
// -------------------------------------------------------------------------
::rtl::OUString MQuery::getAddressbook() const
{
    OSL_TRACE("IN MQuery::getAddressbook()\n");

    OSL_TRACE("\tOUT MQuery::getAddressbook()\n");

    return(m_aAddressbook);
}
// -------------------------------------------------------------------------
void MQuery::setMatchItems(::std::vector< ::rtl::OUString> &mi)
{
    OSL_TRACE("IN MQuery::setMatchItems()\n");
    ::osl::MutexGuard aGuard(m_aMutex);

    ::std::vector< ::rtl::OUString>::iterator aIter = mi.begin();
    ::std::map< ::rtl::OUString, ::rtl::OUString>::const_iterator aIterMap;

    m_aMatchItems.clear();
    m_aMatchItems.reserve(mi.size());
    for(aIter; aIter != mi.end();++aIter)
    {
        aIterMap = m_aColumnAliasMap.find(*aIter);
        if (aIterMap == m_aColumnAliasMap.end()) {
            // Not found.
            m_aMatchItems.push_back(*aIter);
        }
        else {
            m_aMatchItems.push_back(aIterMap->second);
        }
    }
    OSL_TRACE("\tOUT MQuery::setMatchItems()\n");

    return;
}
// -------------------------------------------------------------------------
const ::std::vector< ::rtl::OUString> &MQuery::getMatchItems() const
{
    OSL_TRACE("IN MQuery::getMatchItems()\n");

    OSL_TRACE("\tOUT MQuery::getMatchItems()\n");

    return(m_aMatchItems);
}
// -------------------------------------------------------------------------
void MQuery::setMatchValues(::std::vector< ::rtl::OUString>& mv)
{
    OSL_TRACE("IN MQuery::setMatchValues()\n");
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aMatchValues.clear();
    m_aMatchValues = mv;
    OSL_TRACE("\tOUT MQuery::setMatchValues()\n");

    return;
}
// -------------------------------------------------------------------------
const ::std::vector< ::rtl::OUString>& MQuery::getMatchValues( void ) const
{
    OSL_TRACE("IN MQuery::getMatchValues()\n");

    OSL_TRACE("\tOUT MQuery::getMatchValue()\n");

    return(m_aMatchValues);
}
// -------------------------------------------------------------------------
void MQuery::setMaxNrOfReturns(const sal_Int32 mnr)
{
    OSL_TRACE( "IN MQuery::setMaxNrOfReturns()\n" );
    ::osl::MutexGuard aGuard(m_aMutex);

    m_nMaxNrOfReturns = mnr;
    OSL_TRACE("\tOUT MQuery::setMaxNrOfReturns()\n" );

    return;
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

    return;
}
// -------------------------------------------------------------------------
sal_Bool MQuery::getQuerySubDirs() const
{
    OSL_TRACE("IN MQuery::getQuerySubDirs()\n");

    OSL_TRACE("\tOUT MQuery::getQuerySubDirs()\n");

    return(m_bQuerySubDirs);
}
// -------------------------------------------------------------------------
void MQuery::setSqlOppr(::std::vector< MQuery::eSqlOppr > &so)
{
    OSL_TRACE("IN MQuery::setSqlOppr()\n");
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aSqlOppr.clear();
    m_aSqlOppr = so;

    OSL_TRACE("\tOUT MQuery::setSqlOppr()\n");

    return;
}
// -------------------------------------------------------------------------
const ::std::vector< MQuery::eSqlOppr > &MQuery::getSqlOppr() const
{
    OSL_TRACE("IN MQuery::getSqlOppr()\n");

    OSL_TRACE("\tOUT MQuery::getSqlOppr()\n");

    return(m_aSqlOppr);
}
// -------------------------------------------------------------------------
sal_Int32 MQuery::executeQuery(sal_Bool _bIsOutlookExpress)
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
    MNameMapper *nmap = MNameMapper::getInstance();

    if ( nmap->getDir( m_aAddressbook, getter_AddRefs( directory ) ) == sal_False )
        return( -1 );

    // Create a nsIAbDirectoryQuery object which eventually will execute
    // the query by calling DoQuery().
    nsCOMPtr< nsIAbDirectoryQueryProxy > directoryQueryProxy = do_CreateInstance( kAbDirectoryQueryProxyCID, &rv);

    m_aQueryDirectory->directory = do_QueryInterface(directory, &rv);

    if ( NS_FAILED(rv) || _bIsOutlookExpress)
    {
        // Need to turn this off for anything using the Query Proxy since it
        // treats Mailing Lists as directories!
        m_bQuerySubDirs = sal_False;

        rv = directoryQueryProxy->Initiate (directory);
        NS_ENSURE_SUCCESS(rv, rv);

        m_aQueryDirectory->directory = do_QueryInterface (directoryQueryProxy, &rv);
        NS_ENSURE_SUCCESS(rv, rv);
        OSL_TRACE("Using the directoryQueryProxy\n");
    }
#ifdef DEBUG
    else
        OSL_TRACE("Not using a Query Proxy, Query i/f supported by directory\n");
#endif /* DEBUG */

    // Array that holds all matchItems, to be passed to DoQuery().
    nsCOMPtr<nsISupportsArray> matchItems;
    NS_NewISupportsArray(getter_AddRefs(matchItems));

    // Add every individual boolString to matchItems array.
    ::std::vector< ::rtl::OUString>::iterator aIterMi = m_aMatchItems.begin();
    ::std::vector< eSqlOppr >::iterator aIterOp = m_aSqlOppr.begin();
    ::std::vector< ::rtl::OUString>::iterator aIterVal = m_aMatchValues.begin();
    nsString matchValue;
    // Initialise the matchItems container.
    for(aIterMi; aIterMi != m_aMatchItems.end();++aIterMi, ++aIterOp, ++aIterVal)
    {
        nsCOMPtr<nsIAbBooleanConditionString> boolString = do_CreateInstance (kBooleanConditionStringCID, &rv);
        NS_ENSURE_SUCCESS( rv, rv );
        // Set the 'name' property of the boolString.
        string aMiName = MTypeConverter::ouStringToStlString(*aIterMi);
        boolString->SetName(strdup(aMiName.c_str()));
        OSL_TRACE("Name = %s ;", aMiName.c_str() );
        // Set the 'matchType' property of the boolString. Check for equal length.
        if (aIterOp == m_aSqlOppr.end() && aIterMi != m_aMatchItems.end()) {
            m_aSqlOppr.push_back(matchIs); // Add matchIs for non-set value.
        }
        switch(*aIterOp) {
            case matchExists:
                OSL_TRACE("matchExists; ");
                boolString->SetCondition(nsIAbBooleanConditionTypes::Exists);
                break;
            case matchDoesNotExist:
                OSL_TRACE("matchDoesNotExist; ");
                boolString->SetCondition(nsIAbBooleanConditionTypes::DoesNotExist);
                break;
            case matchContains:
                OSL_TRACE("matchContains; ");
                boolString->SetCondition(nsIAbBooleanConditionTypes::Contains);
                break;
            case matchDoesNotContain:
                OSL_TRACE("matchDoesNotContain; ");
                boolString->SetCondition(nsIAbBooleanConditionTypes::DoesNotContain);
                break;
            case matchIs:
                OSL_TRACE("matchIs; ");
                boolString->SetCondition(nsIAbBooleanConditionTypes::Is);
                break;
            case matchIsNot:
                OSL_TRACE("matchIsNot; ");
                boolString->SetCondition(nsIAbBooleanConditionTypes::IsNot);
                break;
            case matchBeginsWith:
                OSL_TRACE("matchBeginsWith; ");
                boolString->SetCondition(nsIAbBooleanConditionTypes::BeginsWith);
                break;
            case matchEndsWith:
                OSL_TRACE("matchEndsWith; ");
                boolString->SetCondition(nsIAbBooleanConditionTypes::EndsWith);
                break;
            case matchSoundsLike:
                OSL_TRACE("matchSoundsLike; ");
                boolString->SetCondition(nsIAbBooleanConditionTypes::SoundsLike);
                break;
            case matchRegExp:
                OSL_TRACE("matchRegExp; ");
                boolString->SetCondition(nsIAbBooleanConditionTypes::RegExp);
                break;
            default:
                OSL_TRACE("(default) matchIs; ");
                boolString->SetCondition(nsIAbBooleanConditionTypes::Is);
                break;
        }
        // Set the 'matchValue' property of the boolString. Value returned in unicode.
        if ( (*aIterVal) )
        {
            OSL_TRACE("Value = %s \n", OUtoCStr( (*aIterVal) ) );
            MTypeConverter::ouStringToNsString( (*aIterVal), matchValue);
            boolString->SetValue(matchValue.ToNewUnicode ());
        }
        // Add the individual boolString to the container of matchItems.
        matchItems->AppendElement(boolString);
    }
    nsCOMPtr< nsIAbBooleanExpression > queryExpression = do_CreateInstance( kBooleanExpressionCID , &rv);
    NS_ENSURE_SUCCESS( rv, rv );
    queryExpression->SetExpressions(matchItems);
    queryExpression->SetOperation(nsIAbBooleanOperationTypes::OR);

    // Add every atribute we're interested in to the return properties array.
    ::std::vector< ::rtl::OUString>::iterator aIterAttr = m_aAttributes.begin();
    char    **returnProperties = new char* [ m_aAttributes.size() + 2 ];

    PRInt32   count=0;
    returnProperties[count] = strdup( "card:nsIAbCard");
    for(aIterAttr, count=1; aIterAttr != m_aAttributes.end();++aIterAttr,++count)
    {
        ::std::string aAttrName = MTypeConverter::ouStringToStlString(*aIterAttr);
        returnProperties[count] = strdup( aAttrName.c_str() );
        OSL_TRACE("returnProperties[%d] = %s\n", count, returnProperties[count] );
    }
    returnProperties[count] = NULL;

    nsCOMPtr< nsIAbDirectoryQueryArguments > arguments = do_CreateInstance( kAbDirectoryQueryArgumentsCID, &rv);

    NS_ENSURE_SUCCESS( rv, rv );
    rv = arguments->SetExpression(queryExpression);
    NS_ENSURE_SUCCESS( rv, rv );

    rv = arguments->SetReturnProperties(count, (const char **)returnProperties);
    NS_ENSURE_SUCCESS( rv, rv );

    rv = arguments->SetQuerySubDirectories(m_bQuerySubDirs);
    NS_ENSURE_SUCCESS( rv, rv );

    // Execute the query.
    OSL_TRACE( "****** calling DoQuery\n");

    m_aQueryHelper->reset();

    rv = m_aQueryDirectory->directory->DoQuery(arguments, m_aQueryHelper, m_nMaxNrOfReturns, -1, &m_aQueryDirectory->contextId);

    if (NS_FAILED(rv))  {
        m_aQueryDirectory->contextId = -1;
        OSL_TRACE( "****** DoQuery failed\n");
        OSL_TRACE("\tOUT MQuery::executeQuery()\n");
        m_aQueryHelper->notifyQueryError() ;
        return(-1);
    } else {
        OSL_TRACE( "****** DoQuery succeeded \n");
    }

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
    return( m_aQueryHelper->queryComplete() );
}

void
MQuery::waitForQueryComplete( void )
  throw( ::com::sun::star::sdbc::SQLException )
{
    m_aQueryHelper->waitForQueryComplete();
}

// -------------------------------------------------------------------------

sal_Bool
MQuery::checkRowAvailable( sal_Int32 nDBRow )
  throw( ::com::sun::star::sdbc::SQLException )
{
    while( !queryComplete() && m_aQueryHelper->getRealCount() <= nDBRow )
        m_aQueryHelper->waitForRow( nDBRow );

    return( getRowCount() > nDBRow );
}

// -------------------------------------------------------------------------
void
MQuery::getRowValue( ORowSetValue& rValue, sal_Int32 nDBRow, rtl::OUString& aDBColumnName, sal_Int32 nType )
  throw( ::com::sun::star::sdbc::SQLException )
{
    rtl::OUString   sValue;

    OSL_TRACE( "IN MQuery::getRowValue()\n");

    MQueryHelperResultEntry*   xResEntry = m_aQueryHelper->getByIndex( nDBRow );

    OSL_ENSURE( xResEntry != NULL, "xResEntry == NULL");
    if (xResEntry == NULL )
    {
        rValue.setNull();
        return;
    }
    ::std::map< ::rtl::OUString, ::rtl::OUString>::const_iterator aIterMap;
    switch ( nType ) {
        case DataType::VARCHAR:
            aIterMap = m_aColumnAliasMap.find(aDBColumnName);
            if (aIterMap != m_aColumnAliasMap.end()) {
                sValue = xResEntry->getValue(aIterMap->second);
            } else {
                sValue = xResEntry->getValue( aDBColumnName );
            }
            rValue = sValue;
            break;
        default:
            OSL_TRACE("Unknown DataType : %d\n", nType );
            rValue.setNull();
            break;
    }

    OSL_TRACE( "\tOUT MQuery::getRowValue()\n");
}
