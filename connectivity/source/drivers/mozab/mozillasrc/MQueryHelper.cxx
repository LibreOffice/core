/*************************************************************************
 *
 *  $RCSfile: MQueryHelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dkenny $ $Date: 2002-01-09 10:56:33 $
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
MQueryHelperResultEntry::insert( rtl::OUString &key, rtl::OUString &value )
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

    PRUint32 count;
    rv = properties -> Count(&count);
    NS_ENSURE_SUCCESS(rv, rv);

    // Initialise the resultset (xResultSet).
    OSL_TRACE("\tGot %d properties\n", count );
    if ( count > 0 ) {
        PRUint32 i;

        // Need to determine if it's a MailingList, so first find the
        // nsIAbCard property any check the boolean isMailList
        for ( i = 0; i < count; ++i) {
            nsCOMPtr<nsISupports> item;
            rv = properties -> GetElementAt(i, getter_AddRefs(item));
            NS_ENSURE_SUCCESS(rv, rv);

            nsCOMPtr<nsIAbDirectoryQueryPropertyValue> property(do_QueryInterface(item,
&rv));

            char *name;
            rv = property -> GetName(&name);
            if ( !strcmp(name,"card:nsIAbCard") ) {
                nsCOMPtr<nsISupports> cardSupports;
                property->GetValueISupports (getter_AddRefs (cardSupports));
                nsCOMPtr<nsIAbCard> card(do_QueryInterface(cardSupports, &rv));

                PRBool bIsMailList = PR_FALSE;
                card->GetIsMailList( &bIsMailList );
                if ( bIsMailList ) {
                    // No just skip, ie. return without inserting.
                    nsMemory::Free(name);
                    return(NS_OK);
                }
                else {
                    break;
                }
            }
            nsMemory::Free(name);
        }
        MQueryHelperResultEntry *resEntry = new MQueryHelperResultEntry();
        for (i = 0; i < count; ++i) {
            nsCOMPtr<nsISupports> item;
            rv = properties -> GetElementAt(i, getter_AddRefs(item));
            NS_ENSURE_SUCCESS(rv, rv);

            nsCOMPtr<nsIAbDirectoryQueryPropertyValue> property(do_QueryInterface(item, &rv));

            // TODO (1line)
            char *name;
            rv = property -> GetName(&name);
            ::rtl::OUString sValue;
            ::rtl::OUString sName = ::rtl::OUString::createFromAscii( name );
            OSL_TRACE("IN OMozabQueryHelper::OnQueryItem --> name = %s\n", name);
            if ( !strcmp(name,"card:nsIAbCard") ) {
                // Do Nothing, property is useless outside here
            }
            else {

            unsigned short *value;
            rv = property -> GetValue(&value);
            // Default value for PerferMailFormat is mime type text/plain.
        // Mapping for PreferMailFormat values to Mime-types.
            if (strcmp(name, "PreferMailFormat") == 0) {
                nsCAutoString val_c;
                val_c.AssignWithConversion(value);
                if (strcmp((char *) val_c.get(), mozPreferMailFormatTypes[0]) == 0) {
                        // unknown, insert plain/text mime type
                        sValue = ::rtl::OUString::createFromAscii(PreferMailFormatTypes[0]);
                } else if (strcmp((char *) val_c.get(), mozPreferMailFormatTypes[1]) == 0) {
                        // plaintext
                        sValue = ::rtl::OUString::createFromAscii(PreferMailFormatTypes[0]);
                } else if (strcmp((char *) val_c.get(), mozPreferMailFormatTypes[2]) == 0) {
                        // html
                        sValue = ::rtl::OUString::createFromAscii(PreferMailFormatTypes[1]);
                } else {
                        // undefined value passed back from mozilla, insert text/plain
                        sValue = ::rtl::OUString::createFromAscii(PreferMailFormatTypes[0]);
                }
            } else {
        // All other fields but PreferMailFormat.
                nsAutoString v (value);
                MTypeConverter::nsStringToOUString( v, sValue );
            }
            nsMemory::Free(value);
        }
            nsMemory::Free(name);
            resEntry->insert( sName, sValue );
        }
        append( resEntry );
    }

    OSL_TRACE( "\tOUT MQueryHelper::OnQueryItem()\n" );

    notifyResultOrComplete();

    return(NS_OK);
}

void MQueryHelper::notifyQueryError()
{
    m_bQueryComplete = sal_True ;
    notifyResultOrComplete() ;
}

// -------------------------------------------------------------------------
