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


#include "MQueryHelper.hxx"
#include "MTypeConverter.hxx"
#include "MConnection.hxx"
#include "MNSDeclares.hxx"
#include "MLdapAttributeMap.hxx"

#include <connectivity/dbexception.hxx>
#include <osl/thread.hxx>

#include "resource/mozab_res.hrc"

using namespace connectivity::mozab;


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
    OSL_TRACE("IN MQueryHelperResultEntry::~MQueryHelperResultEntry()");
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
void MQueryHelperResultEntry::insert( const OString &key, OUString &value )
{
    m_Fields[ key ] = value;
}

OUString MQueryHelperResultEntry::getValue( const OString &key ) const
{
    FieldMap::const_iterator iter = m_Fields.find( key );
    if ( iter == m_Fields.end() )
    {
        return OUString();
    }
    else
    {
        return iter->second;
    }
}

void MQueryHelperResultEntry::setValue( const OString &key, const OUString & rValue)
{
    m_Fields[ key ] = rValue;
}
//
// class MQueryHelper
//
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
    m_oThreadID = osl::Thread::getCurrentIdentifier();
#endif
}

MQueryHelper::~MQueryHelper()
{
    OSL_TRACE("IN MQueryHelper::~MQueryHelper()");
    clear_results();
    OSL_TRACE("OUT MQueryHelper::~MQueryHelper()");
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
        m_aError.setResId( STR_TIMEOUT_WAITING );
        return sal_False;
    }

    if ( isError() ) {
        OSL_TRACE("waitForResultOrComplete() : Error returned!");
        m_aError.setResId( STR_ERR_EXECUTING_QUERY );
        return sal_False;
    }
    m_aError.reset();
    OSL_TRACE("  Out : waitForResultOrComplete()");
    return sal_True;
}


MQueryHelperResultEntry*
MQueryHelper::next( )
{
    MQueryHelperResultEntry* result;
    sal_uInt32 index;

    m_aMutex.acquire();
    index = m_nIndex;
    m_aMutex.release();

    result = getByIndex( index + 1) ; // Add 1 as Row is numbered from 1 to N

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
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( nRow > m_aResults.size() )
        {
            if ( m_bQueryComplete )
            {
                m_bAtEnd = sal_True;
                return( NULL );
            }
            else
            {
                clearResultOrComplete();
                aGuard.clear();
                if ( !waitForResultOrComplete( ) )
                    return( NULL );
            }
        }
        else
        {
            return( m_aResults[ nRow -1 ] );
        }
    } while ( true );
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
        OSL_TRACE( "\tOUT MQueryHelper::getResultCount() = -1");
        return -1;
    }
    else
    {
        OSL_TRACE( "\tOUT MQueryHelper::getResultCount() = %d", m_aResults.size() );
        return static_cast<sal_Int32>(m_aResults.size());
    }
}

// -------------------------------------------------------------------------

sal_uInt32
MQueryHelper::getRealCount() const
{
    OSL_TRACE( "IN/OUT MQueryHelper::getRealCount() = %d", m_aResults.size() );
    return static_cast<sal_Int32>(m_aResults.size());
}

// -------------------------------------------------------------------------
NS_IMETHODIMP MQueryHelper::OnQueryItem(nsIAbDirectoryQueryResult *result)
{
    ::osl::MutexGuard aGuard( m_aMutex );
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "IN MQueryHelper::OnQueryItem() Caller thread: %4d",m_oThreadID );
#endif
    nsresult rv;
    PRInt32 resultType;

    if ( result == NULL ) {
        OSL_TRACE("\tresult ptr is NULL");
        return NS_OK;
    }

    // Get return status of executeQuery() call.
    rv = result -> GetType(&resultType);
    NS_ENSURE_SUCCESS(rv, rv);

    // Check for errors of the executeQuery() call.
    switch ( resultType ) {
    case nsIAbDirectoryQueryResult::queryResultError:
        OSL_TRACE("\tresultType == nsIAbDirectoryQueryResult::queryResultError");
        m_bQueryComplete = sal_True;
        m_bErrorCondition = sal_True;
        notifyResultOrComplete();
        return NS_OK;
    case nsIAbDirectoryQueryResult::queryResultStopped:
        OSL_TRACE("\tresultType == nsIAbDirectoryQueryResult::queryResultStopped");
        m_bQueryComplete = sal_True;
        notifyResultOrComplete();
        return NS_OK;
    case nsIAbDirectoryQueryResult::queryResultComplete:
        OSL_TRACE("\tresultType == nsIAbDirectoryQueryResult::queryResultComplete");
        m_bQueryComplete = sal_True;
        notifyResultOrComplete();
        return NS_OK;
    case nsIAbDirectoryQueryResult::queryResultMatch:
        OSL_TRACE("IN MQueryHelper::OnQueryItem --> queryResultMatch");
        // Don't return, continues onto rest of method.
        break;
    default:
        OSL_TRACE("\t******** Unexpected : resultType");
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

    OSL_TRACE( "\tOUT MQueryHelper::OnQueryItem()" );

    notifyResultOrComplete();

    return(NS_OK);
}

// -----------------------------------------------------------------------------
void MQueryHelper::notifyQueryError()
{
    m_bQueryComplete = sal_True ;
    notifyResultOrComplete() ;
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
        m_aError.setResId( STR_MOZILLA_IS_RUNNIG_NO_CHANGES ); \
        return sal_False;   \
    }

sal_Int32 MQueryHelper::commitCard(const sal_uInt32 rowIndex,nsIAbDirectory * directory)
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
        m_aError.setResId( STR_FOREIGN_PROCESS_CHANGED_AB );

    return !(NS_FAILED(rv));
}

sal_Int32 MQueryHelper::deleteCard(const sal_uInt32 rowIndex,nsIAbDirectory * directory)
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
        m_aError.setResId( STR_FOREIGN_PROCESS_CHANGED_AB );
    return !(NS_FAILED(rv));
}

sal_Bool MQueryHelper::setCardValues(const sal_uInt32 rowIndex)
{
    MQueryHelperResultEntry *resEntry = getByIndex(rowIndex);
    if (!resEntry)
    {
        m_aError.setResId( STR_CANT_FIND_ROW );
        return sal_False;
    }
    nsIAbCard *card=resEntry->getCard();
    if (!card)
    {
        m_aError.setResId( STR_CANT_FIND_CARD_FOR_ROW );
        return sal_False;
    }

    MLdapAttributeMap::fillCardFromResult( *card, *resEntry );
    return sal_True;
}

void MQueryHelper::getCardValues(nsIAbCard *card,sal_uInt32 rowIndex)
{
    MQueryHelperResultEntry *resEntry;
    if (rowIndex>0)
    {
        resEntry = getByIndex(rowIndex);
    }
    else
        resEntry = new MQueryHelperResultEntry();

    MLdapAttributeMap::fillResultFromCard( *resEntry, *card );
    resEntry->setCard(card);
    if (!rowIndex)
        append( resEntry );
}
sal_Bool MQueryHelper::resyncRow(sal_uInt32 rowIndex)
{

    MQueryHelperResultEntry *resEntry = getByIndex(rowIndex);
    if (!resEntry)
    {
        m_aError.setResId( STR_CANT_FIND_ROW );
        return sal_False;
    }
    nsIAbCard *card=resEntry->getCard();
    card = getUpdatedCard(card);
    getCardValues(card,rowIndex);
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32 MQueryHelper::createNewCard()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    nsresult rv;
    nsCOMPtr <nsIAbCard> card = do_CreateInstance(NS_ABCARDPROPERTY_CONTRACTID, &rv);
    //set default values
      getCardValues(card);
    return static_cast<sal_Int32>(m_aResults.size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
