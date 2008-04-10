/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MQueryHelper.cxx,v $
 * $Revision: 1.17 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include <MQueryHelper.hxx>

#include "MTypeConverter.hxx"
#include <connectivity/dbexception.hxx>
#ifndef _CONNECTIVITY_MOZAB_BCONNECTION_HXX_
#include <MConnection.hxx>
#endif

#include "resource/mozab_res.hrc"
#include "MNSDeclares.hxx"

using namespace connectivity::mozab;


enum
{
        index_FirstName=0,
        index_LastName,
        index_DisplayName,
        index_NickName,
        index_PrimaryEmail,
        index_SecondEmail,
        index_PreferMailFormat,
        index_WorkPhone,
        index_HomePhone,
        index_FaxNumber,
        index_PagerNumber,
        index_CellularNumber,
        index_HomeAddress,
        index_HomeAddress2,
        index_HomeCity,
        index_HomeState,
        index_HomeZipCode,
        index_HomeCountry,
        index_WorkAddress,
        index_WorkAddress2,
        index_WorkCity,
        index_WorkState,
        index_WorkZipCode,
        index_WorkCountry,
        index_JobTitle,
        index_Department,
        index_Company,
        index_WebPage1,
        index_WebPage2,
        index_BirthYear,
        index_BirthMonth,
        index_BirthDay,
        index_Custom1,
        index_Custom2,
        index_Custom3,
        index_Custom4,
        index_Notes
};


NS_IMPL_THREADSAFE_ISUPPORTS1(connectivity::mozab::MQueryHelper,nsIAbDirectoryQueryResultListener)

//
// class MQueryHelperResultEntry
//


MQueryHelperResultEntry::MQueryHelperResultEntry()
{
    m_Card = NULL;
    m_RowStates = 0;
}

MQueryHelperResultEntry::~MQueryHelperResultEntry()
{


    OSL_TRACE("IN MQueryHelperResultEntry::~MQueryHelperResultEntry()\n");
    OSL_TRACE("OUT MQueryHelperResultEntry::~MQueryHelperResultEntry()\n");

}
void
MQueryHelperResultEntry::setCard(nsIAbCard *card)
{
    m_Card = card;
}
nsIAbCard *
MQueryHelperResultEntry::getCard()
{
    return m_Card;
}
void
MQueryHelperResultEntry::insert( const rtl::OUString &key, rtl::OUString &value )
{
    m_Fields.insert( fieldMap::value_type( key, value ) );
}

rtl::OUString
MQueryHelperResultEntry::getValue( const rtl::OUString &key ) const
{
    fieldMap::const_iterator  iter;

    iter = m_Fields.find( key );

    if ( iter == m_Fields.end() ) {
        return rtl::OUString();
    } else {
        return (*iter).second;
    }
}

rtl::OUString
MQueryHelperResultEntry::setValue( const rtl::OUString &key, const rtl::OUString & rValue)
{
    m_Fields.erase(key);
    m_Fields.insert( fieldMap::value_type( key, rValue ) );
    return rValue;
}
//
// class MQueryHelper
//
// MIME-types.
static char PreferMailFormatTypes[2][11] = {"text/plain",
                        "text/html"};
MQueryHelper::MQueryHelper()
    :m_nIndex( 0 )
    ,m_bHasMore( sal_True )
    ,m_bAtEnd( sal_False )
    ,m_bErrorCondition( sal_False )
    ,m_bQueryComplete( sal_False )
    ,mRefCnt( 0 ) // NSISUPPORTS - Initialize RefCnt to 0
{
    m_aResults.clear();
#if OSL_DEBUG_LEVEL > 0
    m_oThreadID = osl_getThreadIdentifier(NULL);
#endif
}

MQueryHelper::~MQueryHelper()
{
    OSL_TRACE("IN MQueryHelper::~MQueryHelper()\n");
    clear_results();
    OSL_TRACE("OUT MQueryHelper::~MQueryHelper()\n");
}

void
MQueryHelper::append(MQueryHelperResultEntry* resEnt )
{
    if ( resEnt != NULL ) {
        m_aResults.push_back( resEnt );
        m_bAtEnd   = sal_False;
    }
}

void
MQueryHelper::clear_results()
{
    resultsArray::iterator iter = m_aResults.begin();
    while ( iter != m_aResults.end() ) {
        delete (*iter);
        ++iter;
    }
    m_aResults.clear();
}

void
MQueryHelper::rewind()
{
    m_nIndex = 0;
}

void
MQueryHelper::reset()
{
    m_nIndex = 0;
    m_bHasMore = sal_True;
    m_bQueryComplete = sal_False;
    m_bAtEnd = sal_False;
    m_bErrorCondition = sal_False;
    clear_results();
}

void
MQueryHelper::clearResultOrComplete()
{
    // Don't use a Mutex, it should be called by a method already holding it.
    OSL_TRACE("In/Out : clearResultOrComplete()");
    m_aCondition.reset();
}

void
MQueryHelper::notifyResultOrComplete()
{
    OSL_TRACE("In/Out : notifyResultOrComplete()");
    m_aCondition.set();
}

sal_Bool
MQueryHelper::waitForResultOrComplete(  )
{
    TimeValue               timeValue = { 1, 0 };  // 20 Seconds 0 NanoSecond timeout
    sal_Int32               times=0;
    osl::Condition::Result  rv = ::osl::Condition::result_ok;

    OSL_TRACE("In : waitForResultOrComplete()");
    // Can't hold mutex or condition would never get set...
    while( (m_aCondition.check() == sal_False || rv  == ::osl::Condition::result_error) && times < 20) {
        rv = m_aCondition.wait( &timeValue );
        times ++;
    }
    if (times >= 20 &&  rv == ::osl::Condition::result_timeout ) {
        OSL_TRACE("waitForResultOrComplete() : Timeout!");
        setError( STR_TIMEOUT_WAITING );
        return sal_False;
    }

    if ( isError() ) {
        OSL_TRACE("waitForResultOrComplete() : Error returned!");
        setError( STR_ERR_EXECUTING_QUERY );
        return sal_False;
    }
    resetError();
    OSL_TRACE("  Out : waitForResultOrComplete()");
    return sal_True;
}


MQueryHelperResultEntry*
MQueryHelper::next( )
{
    MQueryHelperResultEntry* result;
    sal_Int32                     index;

    m_aMutex.acquire();
    index = m_nIndex;
    m_aMutex.release();

    result = getByIndex( m_nIndex + 1) ; // Add 1 as Row is numbered from 1 to N

    if ( result ) {
        m_aMutex.acquire();
        m_nIndex++;
        m_aMutex.release();
    }

    return( result );
}

MQueryHelperResultEntry*
MQueryHelper::getByIndex( sal_uInt32 nRow )
{
    // Row numbers are from 1 to N, need to ensure this, and then
    // substract 1
    if ( nRow < 1 ) {
        return( NULL );
    }

    do {
        // Obtain the Mutex - don't use a guard as we want to be able to release
        // and acquire again...
        m_aMutex.acquire();
        if ( nRow > m_aResults.size() )
        {
            if ( m_bQueryComplete )
            {
                m_bAtEnd = sal_True;
                m_aMutex.release();
                return( NULL );
            }
            else
            {
                clearResultOrComplete();
                m_aMutex.release();
                if ( !waitForResultOrComplete( ) )
                    return( NULL );
            }
        }
        else
        {
            m_aMutex.release();
            return( m_aResults[ nRow -1 ] );
        }
    } while ( sal_True );
}

sal_Bool
MQueryHelper::hasMore() const
{
    return m_bHasMore;
}

sal_Bool
MQueryHelper::atEnd() const
{
    return m_bAtEnd;
}

sal_Bool
MQueryHelper::isError() const
{
    return m_bErrorCondition;
}

sal_Bool
MQueryHelper::queryComplete() const
{
    return m_bQueryComplete;
}

sal_Bool
MQueryHelper::waitForQueryComplete(  )
{
    m_aMutex.acquire();

    OSL_TRACE("In : waitForQueryComplete()");
    if ( ! m_bQueryComplete ) {
        do
        {
            m_aMutex.release();
            clearResultOrComplete();
            if ( !waitForResultOrComplete( ) )
                return( sal_False );
            m_aMutex.acquire();
        }
        while ( !m_bQueryComplete );
    }

    m_aMutex.release();
    OSL_TRACE("Out : waitForQueryComplete()");
    return( sal_True );
}

sal_Bool
MQueryHelper::waitForRow( sal_Int32 rowNum )
{
    m_aMutex.acquire();
    do
    {
        m_aMutex.release();
        clearResultOrComplete();
        if ( !waitForResultOrComplete() )
            return( sal_False );
        m_aMutex.acquire();
    }
    while ( !m_bQueryComplete && m_aResults.size() < (size_t)rowNum );

    m_aMutex.release();
    return( sal_True );
}

// -------------------------------------------------------------------------

sal_Int32
MQueryHelper::getResultCount() const
{
    OSL_TRACE( "IN MQueryHelper::getResultCount()" );
    if ( !m_bQueryComplete )
    {
        OSL_TRACE( "\tOUT MQueryHelper::getResultCount() = -1\n");
        return -1;
    }
    else
    {
        OSL_TRACE( "\tOUT MQueryHelper::getResultCount() = %d\n", m_aResults.size() );
        return m_aResults.size();
    }
}

// -------------------------------------------------------------------------

sal_uInt32
MQueryHelper::getRealCount() const
{
    OSL_TRACE( "IN/OUT MQueryHelper::getRealCount() = %d\n", m_aResults.size() );
    return m_aResults.size();
}

// -------------------------------------------------------------------------
NS_IMETHODIMP MQueryHelper::OnQueryItem(nsIAbDirectoryQueryResult *result)
{
    ::osl::MutexGuard aGuard( m_aMutex );
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "IN MQueryHelper::OnQueryItem() Caller thread: %4d \n",m_oThreadID );
#endif
    nsresult rv;
    PRInt32 resultType;

    if ( result == NULL ) {
        OSL_TRACE("\tresult ptr is NULL\n");
        return NS_OK;
    }

    // Get return status of executeQuery() call.
    rv = result -> GetType(&resultType);
    NS_ENSURE_SUCCESS(rv, rv);

    // Check for errors of the executeQuery() call.
    switch ( resultType ) {
    case nsIAbDirectoryQueryResult::queryResultError:
        OSL_TRACE("\tresultType == nsIAbDirectoryQueryResult::queryResultError\n");
        m_bQueryComplete = sal_True;
        m_bErrorCondition = sal_True;
        notifyResultOrComplete();
        return NS_OK;
    case nsIAbDirectoryQueryResult::queryResultStopped:
        OSL_TRACE("\tresultType == nsIAbDirectoryQueryResult::queryResultStopped\n");
        m_bQueryComplete = sal_True;
        notifyResultOrComplete();
        return NS_OK;
    case nsIAbDirectoryQueryResult::queryResultComplete:
        OSL_TRACE("\tresultType == nsIAbDirectoryQueryResult::queryResultComplete\n");
        m_bQueryComplete = sal_True;
        notifyResultOrComplete();
        return NS_OK;
    case nsIAbDirectoryQueryResult::queryResultMatch:
        OSL_TRACE("IN MQueryHelper::OnQueryItem --> queryResultMatch\n");
        // Don't return, continues onto rest of method.
        break;
    default:
        OSL_TRACE("\t******** Unexpected : resultType\n");
        m_bQueryComplete = sal_True;
        return NS_OK;
    }

    // Initialise an array that holds the resultset of the query.
    nsCOMPtr<nsISupportsArray> properties;
    rv = result -> GetResult(getter_AddRefs (properties));
    NS_ENSURE_SUCCESS(rv, rv);


    nsCOMPtr<nsISupports> item;
    rv = properties -> GetElementAt(0, getter_AddRefs(item));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIAbDirectoryQueryPropertyValue> property(do_QueryInterface(item, &rv));
    NS_ENSURE_SUCCESS(rv, rv);

    char *name;
    rv = property -> GetName(&name);
    NS_ENSURE_SUCCESS(rv, rv);
    if ( !strcmp(name,"card:nsIAbCard") )
    {
        nsCOMPtr<nsISupports> cardSupports;
        property->GetValueISupports (getter_AddRefs (cardSupports));
        nsCOMPtr<nsIAbCard> card(do_QueryInterface(cardSupports, &rv));
        NS_ENSURE_SUCCESS(rv, rv);

            getCardValues(card);
    }
    nsMemory::Free(name);

    OSL_TRACE( "\tOUT MQueryHelper::OnQueryItem()\n" );

    notifyResultOrComplete();

    return(NS_OK);
}

void MQueryHelper::notifyQueryError()
{
    m_bQueryComplete = sal_True ;
    notifyResultOrComplete() ;
}

static const ::rtl::OUString& getAttribute(PRUint32 index)
{
    static const ::rtl::OUString sAttributeNames[] =
    {
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FirstName")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LastName")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DisplayName")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NickName")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PrimaryEmail")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SecondEmail")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PreferMailFormat")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkPhone")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomePhone")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FaxNumber")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("PagerNumber")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CellularNumber")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeAddress")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeAddress2")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeCity")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeState")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeZipCode")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("HomeCountry")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkAddress")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkAddress2")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkCity")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkState")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkZipCode")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WorkCountry")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("JobTitle")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Department")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Company")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WebPage1")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("WebPage2")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BirthYear")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BirthMonth")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BirthDay")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Custom1")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Custom2")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Custom3")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Custom4")),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Notes"))
    };
    return sAttributeNames[index];
}
const char * getAddrURI(const nsIAbDirectory*  directory)
{
    nsresult retCode;
    nsCOMPtr<nsIRDFResource> rdfResource = do_QueryInterface((nsISupports *)directory, &retCode) ;
    if (NS_FAILED(retCode)) { return NULL; }
    const char * uri;
    retCode=rdfResource->GetValueConst(&uri);
    if (NS_FAILED(retCode)) { return NULL; }
    return uri;
 }

#define ENSURE_GETUPDATECARD(x) \
    if (NS_FAILED(retCode)) \
    {   \
        OSL_TRACE( x ); \
        return card;    \
    }
static NS_DEFINE_CID(kRDFServiceCID, NS_RDFSERVICE_CID);

//Some address book does not support query uri on card
//In this case, we can't resync the cards,  we just return the old cards
nsIAbCard * getUpdatedCard( nsIAbCard*  card)
{
    OSL_ENSURE(card != NULL, "getUpdatedCard for NULL");
    nsresult retCode;
    nsCOMPtr<nsIRDFResource> rdfResource = do_QueryInterface((nsISupports *)card, &retCode) ;
    ENSURE_GETUPDATECARD( "IN getUpdatedCard: Card does not support nsIRDFResource\n" );

    const char * uri;
    retCode=rdfResource->GetValueConst(&uri);
    ENSURE_GETUPDATECARD( "IN getUpdatedCard: Card does not has a uri\n" );

    nsCOMPtr<nsIRDFService> rdfService (do_GetService(kRDFServiceCID, &retCode)) ;
    ENSURE_GETUPDATECARD( "IN getUpdatedCard: Card does not has a uri\n" );

    nsCOMPtr<nsIRDFResource> rdfCard;

    retCode = rdfService->GetResource(nsDependentCString(uri), getter_AddRefs(rdfCard)) ;
    ENSURE_GETUPDATECARD( "IN getUpdatedCard: Can not get the updated card\n" );

    nsCOMPtr<nsIAbCard> aNewCard=do_QueryInterface((nsISupports *)rdfCard, &retCode);
    ENSURE_GETUPDATECARD( "IN getUpdatedCard: Error in get new card\n" );

    return aNewCard;
 }

#define ENSURE_MOZAB_PROFILE_NOT_LOOKED(directory)  \
    if (getDirectoryType(directory) == SDBCAddress::Mozilla && isProfileLocked(NULL))   \
    {   \
        setError( STR_MOZILLA_IS_RUNNIG_NO_CHANGES ); \
        return sal_False;   \
    }

sal_Int32 MQueryHelper::commitCard(const sal_Int32 rowIndex,nsIAbDirectory * directory)
{
    ENSURE_MOZAB_PROFILE_NOT_LOOKED(directory);

    MQueryHelperResultEntry *resEntry = getByIndex(rowIndex);
    if (!resEntry)
    {
        return sal_False;
    }
    nsIAbCard *card=resEntry->getCard();
    if (!card)
    {
        return sal_False;
    }
    nsresult rv;
    PRBool hasCard;
    rv = directory->HasCard(card,&hasCard);
    if (setCardValues(rowIndex) != sal_True)
        return sal_False;

    if (!NS_FAILED(rv) && hasCard)
    {
                rv = card->EditCardToDatabase(getAddrURI(directory));
    }
    else
    {
        nsIAbCard *addedCard=NULL;
        rv = directory->AddCard(card,&addedCard);
        if (!NS_FAILED(rv))
            resEntry->setCard(addedCard);
    }
    //We return NS_ERROR_FILE_ACCESS_DENIED in the case the mozillaAB has been changed out side of our process
    if (rv == NS_ERROR_FILE_ACCESS_DENIED )
        setError( STR_FOREIGN_PROCESS_CHANGED_AB );

    return !(NS_FAILED(rv));
}

sal_Int32 MQueryHelper::deleteCard(const sal_Int32 rowIndex,nsIAbDirectory * directory)
{
    ENSURE_MOZAB_PROFILE_NOT_LOOKED(directory);
    MQueryHelperResultEntry *resEntry = getByIndex(rowIndex);
    if (!resEntry)
    {
        return sal_False;
    }
    nsIAbCard *card=resEntry->getCard();
    if (!card)
    {
        return sal_False;
    }
    nsresult rv;
    PRBool hasCard;
    if (resEntry->getRowStates() == RowStates_Inserted)
    {
        rv = 0;
        hasCard = sal_True;
    }
    else
        rv = directory->HasCard(card,&hasCard);

    if (!NS_FAILED(rv) && hasCard)
    {
        nsCOMPtr <nsISupportsArray> cardsToDelete;
        rv = NS_NewISupportsArray(getter_AddRefs(cardsToDelete));
        if (NS_SUCCEEDED(rv))
        {
            nsCOMPtr<nsISupports> supports = do_QueryInterface(card, &rv);
            if (NS_SUCCEEDED(rv))
            {
                rv = cardsToDelete->AppendElement(supports);
                if (NS_SUCCEEDED(rv))
                    rv = directory->DeleteCards(cardsToDelete);
            }
        }
    }

    if (NS_SUCCEEDED(rv))
        resEntry->setRowStates(RowStates_Deleted);
    //We return NS_ERROR_FILE_ACCESS_DENIED in the case the mozillaAB has been changed out side of our process
    if (rv == NS_ERROR_FILE_ACCESS_DENIED )
        setError( STR_FOREIGN_PROCESS_CHANGED_AB );
    return !(NS_FAILED(rv));
}

sal_Bool MQueryHelper::setCardValues(const sal_Int32 rowIndex)
{
    MQueryHelperResultEntry *resEntry = getByIndex(rowIndex);
    if (!resEntry)
    {
        setError( STR_CANT_FIND_ROW );
        return sal_False;
    }
    nsIAbCard *card=resEntry->getCard();
    if (!card)
    {
        setError( STR_CANT_FIND_CARD_FOR_ROW );
        return sal_False;
    }

    ::rtl::OUString sValue;

    getCardAttributeAndValue(getAttribute(index_FirstName),sValue,resEntry);
    card->SetFirstName(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_LastName),sValue,resEntry);
    card->SetLastName(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_DisplayName),sValue,resEntry);
    card->SetDisplayName(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_NickName),sValue,resEntry);
    card->SetNickName(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_PrimaryEmail),sValue,resEntry);
    card->SetPrimaryEmail(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_SecondEmail),sValue,resEntry);
    card->SetSecondEmail(sValue.getStr());

    unsigned int format;
    ::rtl::OUString prefMailValue;
    getCardAttributeAndValue(getAttribute(index_WorkPhone),prefMailValue,resEntry);
    for(format=2;format >0;format--)
    {
         if (! prefMailValue.compareTo(
              ::rtl::OUString::createFromAscii(PreferMailFormatTypes[format-1]) ) )
            break;
    }
    card->SetPreferMailFormat(format);

    getCardAttributeAndValue(getAttribute(index_WorkPhone),sValue,resEntry);
    card->SetWorkPhone(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_HomePhone),sValue,resEntry);
    card->SetHomePhone(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_FaxNumber),sValue,resEntry);
    card->SetFaxNumber(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_PagerNumber),sValue,resEntry);
    card->SetPagerNumber(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_CellularNumber),sValue,resEntry);
    card->SetCellularNumber(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_HomeAddress),sValue,resEntry);
    card->SetHomeAddress(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_HomeAddress2),sValue,resEntry);
    card->SetHomeAddress2(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_HomeCity),sValue,resEntry);
    card->SetHomeCity(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_HomeState),sValue,resEntry);
    card->SetHomeState(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_HomeZipCode),sValue,resEntry);
    card->SetHomeZipCode(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_HomeCountry),sValue,resEntry);
    card->SetHomeCountry(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_WorkAddress),sValue,resEntry);
    card->SetWorkAddress(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_WorkAddress2),sValue,resEntry);
    card->SetWorkAddress2(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_WorkCity),sValue,resEntry);
    card->SetWorkCity(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_WorkState),sValue,resEntry);
    card->SetWorkState(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_WorkZipCode),sValue,resEntry);
    card->SetWorkZipCode(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_WorkCountry),sValue,resEntry);
    card->SetWorkCountry(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_JobTitle),sValue,resEntry);
    card->SetJobTitle(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_Department),sValue,resEntry);
    card->SetDepartment(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_Company),sValue,resEntry);
    card->SetCompany(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_WebPage1),sValue,resEntry);
    card->SetWebPage1(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_WebPage2),sValue,resEntry);
    card->SetWebPage2(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_BirthYear),sValue,resEntry);
    card->SetBirthYear(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_BirthMonth),sValue,resEntry);
    card->SetBirthMonth(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_BirthDay),sValue,resEntry);
    card->SetBirthDay(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_Custom1),sValue,resEntry);
    card->SetCustom1(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_Custom2),sValue,resEntry);
    card->SetCustom2(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_Custom3),sValue,resEntry);
    card->SetCustom3(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_Custom4),sValue,resEntry);
    card->SetCustom4(sValue.getStr());

    getCardAttributeAndValue(getAttribute(index_Notes),sValue,resEntry);
    card->SetNotes(sValue.getStr());

    return sal_True;
}

void MQueryHelper::getCardValues(nsIAbCard *card,sal_Int32 rowIndex)
{
    MQueryHelperResultEntry *resEntry;
    if (rowIndex>0)
    {
        resEntry = getByIndex(rowIndex);
    }
    else
        resEntry = new MQueryHelperResultEntry();

    nsXPIDLString sValue;

    card->GetFirstName(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_FirstName),sValue,resEntry);

    card->GetLastName(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_LastName),sValue,resEntry);

    card->GetDisplayName(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_DisplayName),sValue,resEntry);

    card->GetNickName(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_NickName),sValue,resEntry);

    card->GetPrimaryEmail(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_PrimaryEmail),sValue,resEntry);

    card->GetSecondEmail(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_SecondEmail),sValue,resEntry);

    unsigned int format = 0;
    card->GetPreferMailFormat(&format);
    ::rtl::OUString prefMailValue;
    switch(format)
    {
        case nsIAbPreferMailFormat::html:
            prefMailValue = ::rtl::OUString::createFromAscii(PreferMailFormatTypes[1]);
            break;
         case nsIAbPreferMailFormat::plaintext:
            prefMailValue = ::rtl::OUString::createFromAscii(PreferMailFormatTypes[0]);
            break;
         case nsIAbPreferMailFormat::unknown:
         default:
            prefMailValue = ::rtl::OUString::createFromAscii(PreferMailFormatTypes[0]);
            break;

    }
    resEntry->insert( getAttribute(index_PreferMailFormat), prefMailValue );

    card->GetWorkPhone(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_WorkPhone),sValue,resEntry);

    card->GetHomePhone(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_HomePhone),sValue,resEntry);

    card->GetFaxNumber(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_FaxNumber),sValue,resEntry);

    card->GetPagerNumber(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_PagerNumber),sValue,resEntry);

    card->GetCellularNumber(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_CellularNumber),sValue,resEntry);

    card->GetHomeAddress(getter_Copies(sValue));
    PRInt32 offset;
    nsXPIDLString space;
    space.Assign(NS_LITERAL_STRING(" "));
    while ((offset = sValue.FindChar('\r')) >= 0) sValue.Replace(offset, 1, space);
    while ((offset = sValue.FindChar('\n')) >= 0) sValue.Replace(offset, 1, space);
    addCardAttributeAndValue(getAttribute(index_HomeAddress),sValue,resEntry);

    card->GetHomeAddress2(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_HomeAddress2),sValue,resEntry);

    card->GetHomeCity(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_HomeCity),sValue,resEntry);

    card->GetHomeState(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_HomeState),sValue,resEntry);

    card->GetHomeZipCode(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_HomeZipCode),sValue,resEntry);

    card->GetHomeCountry(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_HomeCountry),sValue,resEntry);

    card->GetWorkAddress(getter_Copies(sValue));
    while ((offset = sValue.FindChar('\r')) >= 0) sValue.Replace(offset, 1, space);
    while ((offset = sValue.FindChar('\n')) >= 0) sValue.Replace(offset, 1, space);
    addCardAttributeAndValue(getAttribute(index_WorkAddress),sValue,resEntry);

    card->GetWorkAddress2(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_WorkAddress2),sValue,resEntry);

    card->GetWorkCity(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_WorkCity),sValue,resEntry);

    card->GetWorkState(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_WorkState),sValue,resEntry);

    card->GetWorkZipCode(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_WorkZipCode),sValue,resEntry);

    card->GetWorkCountry(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_WorkCountry),sValue,resEntry);

    card->GetJobTitle(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_JobTitle),sValue,resEntry);

    card->GetDepartment(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_Department),sValue,resEntry);

    card->GetCompany(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_Company),sValue,resEntry);

    card->GetWebPage1(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_WebPage1),sValue,resEntry);

    card->GetWebPage2(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_WebPage2),sValue,resEntry);

    card->GetBirthYear(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_BirthYear),sValue,resEntry);

    card->GetBirthMonth(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_BirthMonth),sValue,resEntry);

    card->GetBirthDay(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_BirthDay),sValue,resEntry);

    card->GetCustom1(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_Custom1),sValue,resEntry);

    card->GetCustom2(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_Custom2),sValue,resEntry);

    card->GetCustom3(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_Custom3),sValue,resEntry);

    card->GetCustom4(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_Custom4),sValue,resEntry);

    card->GetNotes(getter_Copies(sValue));
    addCardAttributeAndValue(getAttribute(index_Notes),sValue,resEntry);

    resEntry->setCard(card);
    if (!rowIndex)
        append( resEntry );
}
sal_Bool MQueryHelper::resyncRow(sal_Int32 rowIndex)
{

    MQueryHelperResultEntry *resEntry = getByIndex(rowIndex);
    if (!resEntry)
    {
        setError( STR_CANT_FIND_ROW );
        return sal_False;
    }
    nsIAbCard *card=resEntry->getCard();
    card = getUpdatedCard(card);
    getCardValues(card,rowIndex);
    return sal_True;
}
void MQueryHelper::addCardAttributeAndValue(const ::rtl::OUString& sName, nsXPIDLString Value, MQueryHelperResultEntry *resEntry)
{
    nsAutoString temp(Value) ;
    ::rtl::OUString attrValue;
    MTypeConverter::nsStringToOUString( temp, attrValue );
    resEntry->insert( sName, attrValue );
}

void MQueryHelper::getCardAttributeAndValue(const ::rtl::OUString& sName, ::rtl::OUString &ouValue, MQueryHelperResultEntry *resEntry)
{
    ouValue = resEntry->getValue( sName);
}

// -------------------------------------------------------------------------
sal_Int32           MQueryHelper::createNewCard()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    nsresult rv;
    nsCOMPtr <nsIAbCard> card = do_CreateInstance(NS_ABCARDPROPERTY_CONTRACTID, &rv);
    //set default values
      getCardValues(card);
    return   m_aResults.size();
}

