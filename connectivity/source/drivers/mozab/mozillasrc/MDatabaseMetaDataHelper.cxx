/*************************************************************************
 *
 *  $RCSfile: MDatabaseMetaDataHelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mmaher $ $Date: 2001-10-31 17:24:23 $
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
 *  Contributor(s): Willem van Dorp, Darren Kenny
 *
 *
 ************************************************************************/

// Mozilla includes.
#include <MNSInclude.hxx>

#ifndef _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_
#include "MDatabaseMetaDataHelper.hxx"
#endif
#ifndef _CONNECTIVITY_MAB_CONVERSIONS_HXX_
#include "MTypeConverter.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

#include <MNSInit.hxx>
#include <MNameMapper.hxx>

#ifdef DEBUG
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* DEBUG */
# define OUtoCStr( x ) ("dummy")
#endif /* DEBUG */


static NS_DEFINE_CID(kRDFServiceCID, NS_RDFSERVICE_CID);
static NS_DEFINE_CID(kAbDirFactoryServiceCID, NS_ABDIRFACTORYSERVICE_CID);
static NS_DEFINE_CID(kAddrBookSessionCID, NS_ADDRBOOKSESSION_CID);

using namespace connectivity::mozab;
using namespace connectivity;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;

namespace connectivity
{
    namespace mozab
    {
        static const char * const s_pADDRESSBOOKROOTDIR = "moz-abdirectory://";
    }
}

// -------------------------------------------------------------------------
MDatabaseMetaDataHelper::MDatabaseMetaDataHelper()
    : m_bProfileExists(sal_False)
{
    OSL_TRACE( "IN MDatabaseMetaDataHelper::MDatabaseMetaDataHelper()\n" );
    sal_Bool rv = MNS_Init(m_bProfileExists);

    OSL_TRACE( "\tMAB_NS_Init returned %s\n", rv?"True":"False" );
    OSL_TRACE( "\tOUT MDatabaseMetaDataHelper::MDatabaseMetaDataHelper()\n" );

}
// -------------------------------------------------------------------------
MDatabaseMetaDataHelper::~MDatabaseMetaDataHelper()
{
    m_aTableNames.clear();

    sal_Bool rv = MNS_Term();

    OSL_TRACE( "IN/OUT MDatabaseMetaDataHelper::~MDatabaseMetaDataHelper()\n" );
}
// -------------------------------------------------------------------------
// Case where we get a parent uri, and need to list its children.
// We know we are dealing with the top level address book, so we add the personal
// and collected addresses address books by hand.
static nsresult getSubsFromParent(const rtl::OString& aParent, nsIEnumerator **aSubs, sal_Bool aProfileExists)
{
    if (aSubs == nsnull) { return NS_ERROR_NULL_POINTER ; }
    *aSubs = nsnull ;
    nsresult retCode = NS_OK ;
    nsCOMPtr<nsISupportsArray> array ;

    NS_NewISupportsArray(getter_AddRefs(array)) ;
    if (aProfileExists) {
        nsCOMPtr<nsIRDFService> rdfService(do_GetService(kRDFServiceCID, &retCode)) ;
        NS_ENSURE_SUCCESS(retCode, retCode) ;
        nsCOMPtr<nsIRDFResource> rdfResource ;

        OSL_TRACE("uri: %s\n", aParent.getStr()) ;
        retCode = rdfService->GetResource(aParent.getStr(), getter_AddRefs(rdfResource)) ;
        NS_ENSURE_SUCCESS(retCode, retCode) ;
        nsCOMPtr<nsIAbDirectory> directory = do_QueryInterface(rdfResource, &retCode) ;
        nsCOMPtr<nsIEnumerator> tempEnum ;

        NS_ENSURE_SUCCESS(retCode, retCode) ;
        retCode = directory->GetChildNodes(getter_AddRefs(tempEnum)) ;
        NS_ENSURE_SUCCESS(retCode, retCode) ;
        nsCOMPtr<nsIRDFResource> personalAddressBook ;
        nsCOMPtr<nsIRDFResource> collectedAddresses ;

        retCode = rdfService->GetResource("moz-abmdbdirectory://abook.mab", getter_AddRefs(personalAddressBook)) ;
        NS_ENSURE_SUCCESS(retCode, retCode) ;
        nsCOMPtr<nsIAbDirectory> dirPAB = do_QueryInterface(personalAddressBook, &retCode) ;

        NS_ENSURE_SUCCESS(retCode, retCode) ;
        nsAutoString nameAB ;

        nameAB.AssignWithConversion("Personal Address Book") ;
        dirPAB->SetDirName(nameAB.get()) ;
        array->AppendElement(personalAddressBook) ;
        retCode = rdfService->GetResource("moz-abmdbdirectory://history.mab", getter_AddRefs(collectedAddresses)) ;
        NS_ENSURE_SUCCESS(retCode, retCode) ;
        nsCOMPtr<nsIAbDirectory> dirCA = do_QueryInterface(collectedAddresses, &retCode) ;

        NS_ENSURE_SUCCESS(retCode, retCode) ;
        nameAB.AssignWithConversion("Collected Addresses") ;
        dirCA->SetDirName(nameAB.get()) ;
        array->AppendElement(collectedAddresses) ;
        if (NS_SUCCEEDED(tempEnum->First())) {
            do {
                nsCOMPtr<nsISupports> element ;

                retCode = tempEnum->CurrentItem(getter_AddRefs(element)) ;
                if (NS_SUCCEEDED(retCode)) { array->AppendElement(element) ; }
            } while (NS_SUCCEEDED(tempEnum->Next())) ;
        }
    }
    array->Enumerate(aSubs) ;
    return retCode ;
}

// Case where we get a factory uri and need to have it build the directories.
static const char *kPropertyName = "uri" ;

static nsresult getSubsFromFactory(const rtl::OString& aFactory, nsIEnumerator **aSubs)
{
    if (aSubs == nsnull) { return NS_ERROR_NULL_POINTER ; }
    *aSubs = nsnull ;
    nsresult retCode = NS_OK ;

    nsCOMPtr<nsIAbDirFactoryService> dirFactoryService = do_GetService(NS_ABDIRFACTORYSERVICE_CONTRACTID, &retCode);
    NS_ENSURE_SUCCESS(retCode, retCode) ;
    nsCOMPtr<nsIAbDirFactory> dirFactory ;

    retCode = dirFactoryService->GetDirFactory(aFactory.getStr(), getter_AddRefs(dirFactory)) ;
    NS_ENSURE_SUCCESS (retCode, retCode) ;
    OUString value = OStringToOUString(aFactory, RTL_TEXTENCODING_ASCII_US) ;
    const PRUnichar *valueChars = value.getStr() ;
    nsCOMPtr<nsISimpleEnumerator> subDirs ;

    retCode = dirFactory->CreateDirectory(1, &kPropertyName, &valueChars, getter_AddRefs(subDirs)) ;
    NS_ENSURE_SUCCESS(retCode, retCode) ;
    PRBool hasMore = PR_TRUE ;
    nsCOMPtr<nsISupportsArray> array ;

    NS_NewISupportsArray(getter_AddRefs(array)) ;
    while (NS_SUCCEEDED(subDirs->HasMoreElements(&hasMore)) && hasMore) {
        nsCOMPtr<nsISupports> newDirSupports ;

        retCode = subDirs->GetNext(getter_AddRefs(newDirSupports)) ;
        if (NS_FAILED(retCode)) { continue ; }
        nsCOMPtr<nsIAbDirectory> childDir = do_QueryInterface(newDirSupports, &retCode) ;
        if (NS_FAILED(retCode)) { continue ; }
        array->AppendElement(childDir);
    }
    array->Enumerate(aSubs) ;
    return retCode ;
}

// Case where the uri itself is the directory we're looking for.
static nsresult getSubsFromURI(const rtl::OString& aUri, nsIEnumerator **aSubs)
{
    if (aSubs == nsnull) { return NS_ERROR_NULL_POINTER ; }
    *aSubs = nsnull ;
    nsresult retCode = NS_OK ;

    nsCOMPtr<nsIRDFService> rdfService(do_GetService(kRDFServiceCID, &retCode)) ;
    NS_ENSURE_SUCCESS(retCode, retCode) ;
    nsCOMPtr<nsIRDFResource> rdfResource ;

    retCode = rdfService->GetResource(aUri.getStr(), getter_AddRefs(rdfResource)) ;
    NS_ENSURE_SUCCESS(retCode, retCode) ;
    nsCOMPtr<nsIAbDirectory> directory = do_QueryInterface(rdfResource, &retCode) ;

    NS_ENSURE_SUCCESS(retCode, retCode) ;
    nsAutoString nameAB ;

    nameAB.AssignWithConversion("LDAP Directory") ;
    directory->SetDirName(nameAB.get()) ;
    nsCOMPtr<nsISupportsArray> array ;

    NS_NewISupportsArray(getter_AddRefs(array)) ;
    array->AppendElement(directory) ;
    array->Enumerate(aSubs) ;
    return retCode ;
}

static void throwABSpecificException( OConnection* _pCon, sal_Bool bGivenURI ) throw (SQLException)
{
    if ( ! bGivenURI ) {
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii("No Mozilla Addressbook Directories Exist"),NULL);
    }
    else {
        if (_pCon->usesFactory()) {
            if ( _pCon->isOutlookExpress() ) {
                ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii("No Outlook Express Addressbook Exists"),NULL);
            }
            else {
                ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii("No Outlook (MAPI) Addressbook Exists"),NULL);
            }
        }
        else {
            if (_pCon->isLDAP()) {
                ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii("Unable to connect to LDAP Server"),NULL);
            }
            else {
                ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii("No Mozilla Addressbook Directories Exist"),NULL);
            }
        }
    }
}

::std::vector< ::rtl::OUString >& MDatabaseMetaDataHelper::getTableStrings(
                                                            OConnection*           _pCon,
                                                            sal_Bool forceLoad )
                                                            throw ( SQLException )
{
    sal_Bool                                    bGivenURI;
    sal_Bool                                    bIsRootDir;
    rtl::OUString                               sAbURI;
    OString                                     sAbURIString;

    OSL_TRACE( "IN MDatabaseMetaDataHelper::getTableStrings( 0x%08X, %s)\n", _pCon, forceLoad?"True":"False" );

    // Only do the query if we have to - performance degrades otherwise
    if ( ! forceLoad && m_aTableNames.size() > 0 ) {
        return( m_aTableNames );
    }

    m_aTableNames.clear();

    sAbURI = _pCon->getMozURI();

    OSL_TRACE("AbURI = %s\n", OUtoCStr( sAbURI ) );
    if ( sAbURI.getLength() == 0 )
        bGivenURI = sal_False;
    else {
        sAbURIString = ::rtl::OUStringToOString( sAbURI,
                                                 RTL_TEXTENCODING_ASCII_US);
        bGivenURI = sal_True;
    }

    if ( bGivenURI )
        OSL_TRACE("Using URI %s to getTables()\n", sAbURIString.getStr() );
    else
        OSL_TRACE("Using URI of ROOTDIR to getTables()\n");

    // Get the mozilla addressbooks from the base directory.
    nsresult rv = NS_OK;
    nsCOMPtr<nsIEnumerator> subDirectories;

    if (!bGivenURI) {
        rv = getSubsFromParent(s_pADDRESSBOOKROOTDIR, getter_AddRefs(subDirectories), m_bProfileExists) ;
        if ( NS_FAILED( rv ) ) {
            throwABSpecificException( _pCon, bGivenURI );
        }
    }
    else {
        if (_pCon->usesFactory()) {
            rv = getSubsFromFactory(sAbURIString, getter_AddRefs(subDirectories)) ;
            if ( NS_FAILED( rv ) ) {
                throwABSpecificException( _pCon, bGivenURI );
            }
        }
        else {
            if (_pCon->isLDAP()) {
                rv = getSubsFromURI(sAbURIString, getter_AddRefs(subDirectories)) ;
                if ( NS_FAILED( rv ) ) {
                    throwABSpecificException( _pCon, bGivenURI );
                }
            }
            else {
                rv = getSubsFromParent(sAbURIString, getter_AddRefs(subDirectories), m_bProfileExists);
                if ( NS_FAILED( rv ) ) {
                    throwABSpecificException( _pCon, bGivenURI );
                }
            }
        }
    }
    // Catch all (just in case), should be caught before this
    if ( NS_FAILED( rv ) ) {
        OSL_ASSERT( "Shouldn't have reached this line with a failure");
        throwABSpecificException( _pCon, bGivenURI );
    }
    // At this point we have a list containing the nsIAbDirectory we need to map as tables
    rv = subDirectories -> First();
    if ( NS_FAILED( rv ) ) {
        throwABSpecificException( _pCon, bGivenURI );
    }
    else {
        bIsRootDir = sal_True;
    }

    PRUnichar              *name = nsnull;
    PRBool                  bIsMailList = PR_FALSE;

    ::rtl::OUString aTableName;

    MNameMapper *nmap = _pCon->getNameMapper();

    do {
        nsCOMPtr<nsIAbDirectory> subDirectory;

        if ( bIsRootDir ) {
            nsCOMPtr<nsISupports> item;
            rv = subDirectories -> CurrentItem(getter_AddRefs(item));
            if ( NS_FAILED( rv ) ) {
                ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Problem Getting Addressbook Entry"),NULL);
            }

            subDirectory = do_QueryInterface(item, &rv);
        }
        subDirectory -> GetIsMailList(&bIsMailList);
        // For now we're not interested in mailing lists.
        rv = subDirectory -> GetDirName(&name);
        if ( NS_FAILED( rv ) ) {
            ::dbtools::throwGenericSQLException(::rtl::OUString::createFromAscii("Problem Getting Addressbook Directory Name"),NULL);
        }
        MTypeConverter::prUnicharToOUString(name, aTableName);
        OSL_TRACE("TableName = >%s<\n", OUtoCStr( aTableName ) );
        if (!bIsMailList) {
            ODatabaseMetaDataResultSet::ORow aRow(3);

            // Insert table into map
            if ( aTableName.getLength() == 0 ) {
                aTableName = rtl::OUString::createFromAscii("AddressBook");
            }

            OSL_TRACE("TableName = >%s<\n", OUtoCStr( aTableName ) );

            nmap->add( aTableName, subDirectory );

            m_aTableNames.push_back( aTableName ); // Table name
        }

        if ( bIsRootDir ) {
            rv = subDirectories -> Next();
        }
    } while ( bIsRootDir && rv == NS_OK);
    //  delete pTc;

    OSL_TRACE( "\tOUT MDatabaseMetaDataHelper::getTableStrings()\n" );
    return( m_aTableNames );
}

ODatabaseMetaDataResultSet::ORows& MDatabaseMetaDataHelper::getTables(
                                                            OConnection*           _pCon,
                                                            const ::rtl::OUString& tableNamePattern)
                                                            throw ( SQLException )
{
    static ODatabaseMetaDataResultSet::ORows    aRows;

    OSL_TRACE( "IN MDatabaseMetaDataHelper::getTables()\n" );

    ODatabaseMetaDataResultSet::ORows().swap(aRows); // this makes real clear where memory is freed as well
    aRows.clear();

    // will not be detected (for now).
    ::rtl::OUString aTable(::rtl::OUString::createFromAscii("TABLE"));
    ::std::vector< ::rtl::OUString >& tables = getTableStrings( _pCon );

    for ( sal_Int32 i = 0; i < tables.size(); i++ ) {
        ODatabaseMetaDataResultSet::ORow aRow(3);

        ::rtl::OUString aTableName  = tables[i];

        OSL_TRACE("TableName = >%s<\n", OUtoCStr( aTableName ) );

        // Insert table into map
        if (match( tableNamePattern, aTableName, '\0' ) ) {
            if ( aTableName.getLength() == 0 ) {
                aTableName = rtl::OUString::createFromAscii("AddressBook");
            }

            OSL_TRACE( "TableName = %s ; TableType = %s\n", OUtoCStr(aTableName), OUtoCStr(aTable) );

            aRow.push_back( new ORowSetValueDecorator( aTableName ) ); // Table name
            aRow.push_back( new ORowSetValueDecorator( aTable ) );     // Table type
            aRow.push_back( ODatabaseMetaDataResultSet::getEmptyValue() );                 // Remarks
            aRows.push_back(aRow);
        }
    }

    OSL_TRACE( "\tOUT MDatabaseMetaDataHelper::getTables()\n" );
    return( aRows );
}
