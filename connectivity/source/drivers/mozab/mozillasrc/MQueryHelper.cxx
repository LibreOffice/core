/*************************************************************************
 *
 *  $RCSfile: MQueryHelper.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 16:02:06 $
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
 *  Contributor(s): Darren Kenny, Willem van Dorp
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_MAB_QUERYHELPER_HXX_
#include <MQueryHelper.hxx>
#endif
#include "MTypeConverter.hxx"
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

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
}

MQueryHelperResultEntry::~MQueryHelperResultEntry()
{
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

//
// class MQueryHelper
//
// MIME-types.
static char PreferMailFormatTypes[2][11] = {"text/plain",
                        "text/html"};
// This is what mozilla returns.
static char mozPreferMailFormatTypes[3][11] = {"unknown",
                                               "plaintext",
                                               "html"};


MQueryHelper::MQueryHelper()
    : mRefCnt( 0 ) // NSISUPPORTS - Initialize RefCnt to 0
    , m_nIndex( 0 )
    , m_bHasMore( sal_True )
    , m_bQueryComplete( sal_False )
    , m_bAtEnd( sal_False )
    , m_bErrorCondition( sal_False )
{
    m_aResults.clear();
}

MQueryHelper::~MQueryHelper()
{
    clear_results();
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
MQueryHelper::waitForResultOrComplete( ::rtl::OUString& _rError )
{
    TimeValue               timeValue = { 20, 0 };  // 20 Seconds 0 NanoSecond timeout
    osl::Condition::Result  rv = ::osl::Condition::result_ok;

    OSL_TRACE("In : waitForResultOrComplete()");
    // Can't hold mutex or condition would never get set...
    while( m_aCondition.check() == sal_False || rv  == ::osl::Condition::result_error ) {
        rv = m_aCondition.wait( &timeValue );
        if ( rv == ::osl::Condition::result_timeout ) {
            OSL_TRACE("waitForResultOrComplete() : Timeout!");
            _rError = ::rtl::OUString::createFromAscii("Timeout waiting for result.");
            return sal_False;
        }
    }

    if ( isError() ) {
        OSL_TRACE("waitForResultOrComplete() : Error returned!");
        _rError = ::rtl::OUString::createFromAscii("Error found when executing query");
        return sal_False;
    }
    _rError = ::rtl::OUString::createFromAscii("");
    OSL_TRACE("  Out : waitForResultOrComplete()");
    return sal_True;
}


MQueryHelperResultEntry*
MQueryHelper::next( ::rtl::OUString& _rError )
{
    MQueryHelperResultEntry* result;
    sal_Int32                     index;

    m_aMutex.acquire();
    index = m_nIndex;
    m_aMutex.release();

    result = getByIndex( m_nIndex + 1, _rError ) ; // Add 1 as Row is numbered from 1 to N

    if ( result ) {
        m_aMutex.acquire();
        m_nIndex++;
        m_aMutex.release();
    }

    return( result );
}

MQueryHelperResultEntry*
MQueryHelper::getByIndex( sal_Int32 nRow, ::rtl::OUString& _rError )
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
                if ( !waitForResultOrComplete( _rError ) )
                    return( NULL );
            }
        }
        else
        {
            m_aMutex.release();
            return( m_aResults[ nRow -1 ] );
        }
    } while ( sal_True );

    OSL_TRACE("!!!!! Shouldn't have reached this!!!!\n");

    return( NULL );

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
MQueryHelper::waitForQueryComplete( ::rtl::OUString& _rError )
{
    m_aMutex.acquire();

    OSL_TRACE("In : waitForQueryComplete()");
    if ( ! m_bQueryComplete ) {
        do
        {
            m_aMutex.release();
            clearResultOrComplete();
            if ( !waitForResultOrComplete( _rError ) )
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
MQueryHelper::waitForRow( sal_Int32 rowNum, ::rtl::OUString& _rError  )
{
    m_aMutex.acquire();
    do
    {
        m_aMutex.release();
        clearResultOrComplete();
        if ( !waitForResultOrComplete( _rError ) )
            return( sal_False );
        m_aMutex.acquire();
    }
    while ( !m_bQueryComplete && m_aResults.size() < rowNum );

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

    OSL_TRACE( "IN MQueryHelper::OnQueryItem()\n" );

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
        break;
    case nsIAbDirectoryQueryResult::queryResultStopped:
        OSL_TRACE("\tresultType == nsIAbDirectoryQueryResult::queryResultStopped\n");
        m_bQueryComplete = sal_True;
        notifyResultOrComplete();
        return NS_OK;
        break;
    case nsIAbDirectoryQueryResult::queryResultComplete:
        OSL_TRACE("\tresultType == nsIAbDirectoryQueryResult::queryResultComplete\n");
        m_bQueryComplete = sal_True;
        notifyResultOrComplete();
        return NS_OK;
        break;
    case nsIAbDirectoryQueryResult::queryResultMatch:
        OSL_TRACE("IN MQueryHelper::OnQueryItem --> queryResultMatch\n");
        // Don't return, continues onto rest of method.
        break;
    default:
        OSL_TRACE("\t******** Unexpected : resultType\n");
        m_bQueryComplete = sal_True;
        return NS_OK;
        break;
    }

    // Initialise an array that holds the resultset of the query.
    nsCOMPtr<nsISupportsArray> properties;
    rv = result -> GetResult(getter_AddRefs (properties));
    NS_ENSURE_SUCCESS(rv, rv);



    // Need to determine if it's a MailingList, so first find the
    // nsIAbCard property and check the boolean isMailList
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

        PRBool bIsMailList = PR_FALSE;
        card->GetIsMailList( &bIsMailList );
        if ( bIsMailList )
        {
            // No just skip, ie. return without inserting.
            nsMemory::Free(name);
            return(NS_OK);
        }
        else
        {
            getCardValues(card);
        }
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
        ::rtl::OUString::createFromAscii("FirstName"),
        ::rtl::OUString::createFromAscii("LastName"),
        ::rtl::OUString::createFromAscii("DisplayName"),
        ::rtl::OUString::createFromAscii("NickName"),
        ::rtl::OUString::createFromAscii("PrimaryEmail"),
        ::rtl::OUString::createFromAscii("SecondEmail"),
        ::rtl::OUString::createFromAscii("PreferMailFormat"),
        ::rtl::OUString::createFromAscii("WorkPhone"),
        ::rtl::OUString::createFromAscii("HomePhone"),
        ::rtl::OUString::createFromAscii("FaxNumber"),
        ::rtl::OUString::createFromAscii("PagerNumber"),
        ::rtl::OUString::createFromAscii("CellularNumber"),
        ::rtl::OUString::createFromAscii("HomeAddress"),
        ::rtl::OUString::createFromAscii("HomeAddress2"),
        ::rtl::OUString::createFromAscii("HomeCity"),
        ::rtl::OUString::createFromAscii("HomeState"),
        ::rtl::OUString::createFromAscii("HomeZipCode"),
        ::rtl::OUString::createFromAscii("HomeCountry"),
        ::rtl::OUString::createFromAscii("WorkAddress"),
        ::rtl::OUString::createFromAscii("WorkAddress2"),
        ::rtl::OUString::createFromAscii("WorkCity"),
        ::rtl::OUString::createFromAscii("WorkState"),
        ::rtl::OUString::createFromAscii("WorkZipCode"),
        ::rtl::OUString::createFromAscii("WorkCountry"),
        ::rtl::OUString::createFromAscii("JobTitle"),
        ::rtl::OUString::createFromAscii("Department"),
        ::rtl::OUString::createFromAscii("Company"),
        ::rtl::OUString::createFromAscii("WebPage1"),
        ::rtl::OUString::createFromAscii("WebPage2"),
        ::rtl::OUString::createFromAscii("BirthYear"),
        ::rtl::OUString::createFromAscii("BirthMonth"),
        ::rtl::OUString::createFromAscii("BirthDay"),
        ::rtl::OUString::createFromAscii("Custom1"),
        ::rtl::OUString::createFromAscii("Custom2"),
        ::rtl::OUString::createFromAscii("Custom3"),
        ::rtl::OUString::createFromAscii("Custom4"),
        ::rtl::OUString::createFromAscii("Notes")
    };
    return sAttributeNames[index];
}

void MQueryHelper::getCardValues(nsIAbCard *card)
{
    MQueryHelperResultEntry *resEntry = new MQueryHelperResultEntry();
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

    append( resEntry );
}

void MQueryHelper::addCardAttributeAndValue(const ::rtl::OUString& sName, nsXPIDLString Value, MQueryHelperResultEntry *resEntry)
{
    nsAutoString temp(Value) ;
    ::rtl::OUString attrValue;
    MTypeConverter::nsStringToOUString( temp, attrValue );
    resEntry->insert( sName, attrValue );
}

// -------------------------------------------------------------------------
