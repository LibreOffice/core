/*************************************************************************
 *
 *  $RCSfile: MDatabaseMetaDataHelper.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 18:31:28 $
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
#ifndef CONNECTIVITY_MOZAB_MCONFIGACCESS_HXX
#include "MConfigAccess.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

#include <MNSInit.hxx>
#include <MNameMapper.hxx>

#ifndef _CONNECTIVITY_MAB_MOZABHELPER_HXX_
#include "MNSMozabProxy.hxx"
#endif
static ::osl::Mutex m_aMetaMutex;


#if OSL_DEBUG_LEVEL > 1
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */


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
    m_aTableTypes.clear();

    sal_Bool rv = MNS_Term();

    OSL_TRACE( "IN/OUT MDatabaseMetaDataHelper::~MDatabaseMetaDataHelper()\n" );
}

static nsresult enumSubs(nsISimpleEnumerator * subDirs,nsISupportsArray * array);
//enum all sub folders
static nsresult enumSubs(nsIAbDirectory * parentDir,nsISupportsArray * array)
{
    nsresult rv = NS_OK ;
    nsCOMPtr<nsISimpleEnumerator> subDirectories;
    rv = parentDir->GetChildNodes(getter_AddRefs(subDirectories));
    if (NS_FAILED(rv)) { return rv; }

    rv = enumSubs(subDirectories,array);
    return rv;
}

#include <prmem.h> //need for PR_FREEIF
static nsresult insertPABDescription()
{
    nsresult rv = NS_OK ;
    // mozilla loads the descriptions of pab and history from a file named addressbook.properties
    // we never deliver it with OOo,so mozab does not auot load personal addressbook and Collected addresses
    // If we want to deliver addressbook.properties with OOo,we have to deal with hole mozilla chrome mechanism.
    // So I insert the descriptiones as user prefes,they will cover the default value.

    //"Personal Address Book" "Collected Addresses" are loaded from officecfg
    nsCOMPtr<nsIPref> prefs = do_GetService(NS_PREF_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCAutoString pabPrefName(NS_LITERAL_CSTRING("ldap_2.servers.pab.description"));

    sal_Bool bSetted=sal_False;
    char *value = nsnull;
    if (0 == prefs->CopyCharPref(pabPrefName.get(), &value))
    {
        if (!(value == nsnull || value[0] == 0))
        {
            bSetted = sal_True;
        }
    }
    PR_FREEIF (value);
    value=nsnull;
    if (bSetted == sal_True)
    {
        const char* pPabDescription = static_cast< const char* >( getPabDescription( ) );
        rv = prefs->SetCharPref (pabPrefName.get(), pPabDescription);
        NS_ENSURE_SUCCESS(rv, rv);
    }

    nsCAutoString hisPrefName(NS_LITERAL_CSTRING("ldap_2.servers.history.description"));

    bSetted=sal_False;
    if (0 == prefs->CopyCharPref(hisPrefName.get(), &value))
    {
        if (!(value == nsnull || value[0] == 0))
        {
            bSetted = sal_True;
        }
    }
    PR_FREEIF (value);
    value=nsnull;
    if (bSetted == sal_True)
    {
        const char* pHisDescription = static_cast< const char* >( getHisDescription( ) );

        rv = prefs->SetCharPref (hisPrefName.get(), pHisDescription);
        NS_ENSURE_SUCCESS(rv, rv);
    }
    return rv;
}
// -------------------------------------------------------------------------
// Case where we get a parent uri, and need to list its children.
static nsresult getSubsFromParent(const rtl::OString& aParent, nsIEnumerator **aSubs)
{
    if (aSubs == nsnull) { return NS_ERROR_NULL_POINTER ; }
    *aSubs = nsnull ;
    nsresult retCode = NS_OK ;
    nsCOMPtr<nsISupportsArray> array ;

    NS_NewISupportsArray(getter_AddRefs(array)) ;

    retCode = insertPABDescription();
    NS_ENSURE_SUCCESS(retCode, retCode) ;

    nsCOMPtr<nsIRDFService> rdfService(do_GetService(kRDFServiceCID, &retCode)) ;
    NS_ENSURE_SUCCESS(retCode, retCode) ;
    nsCOMPtr<nsIRDFResource> rdfResource ;

    OSL_TRACE("uri: %s\n", aParent.getStr()) ;
    retCode = rdfService->GetResource(nsDependentCString(aParent.getStr(),aParent.getLength()), getter_AddRefs(rdfResource)) ;
    NS_ENSURE_SUCCESS(retCode, retCode) ;
    nsCOMPtr<nsIAbDirectory> directory = do_QueryInterface(rdfResource, &retCode) ;
    nsCOMPtr<nsISimpleEnumerator> tempEnum ;

    NS_ENSURE_SUCCESS(retCode, retCode) ;
    retCode = directory->GetChildNodes(getter_AddRefs(tempEnum)) ;
    NS_ENSURE_SUCCESS(retCode, retCode) ;

    PRBool hasMore = PR_TRUE ;
    while (NS_SUCCEEDED(tempEnum->HasMoreElements(&hasMore)) && hasMore) {
        nsCOMPtr<nsIAbDirectory> element ;
        retCode = tempEnum->GetNext(getter_AddRefs(element)) ;
        if (NS_SUCCEEDED(retCode))
        {
            array->AppendElement(element) ;
            enumSubs(element,array);
        }
    }

    array->Enumerate(aSubs) ;
    return retCode ;
}

static nsresult enumSubs(nsISimpleEnumerator * subDirs,nsISupportsArray * array)
{
    PRBool hasMore = PR_TRUE ;
    nsresult rv = NS_OK ;
    while (NS_SUCCEEDED(subDirs->HasMoreElements(&hasMore)) && hasMore) {
        nsCOMPtr<nsISupports> newDirSupports ;

        rv = subDirs->GetNext(getter_AddRefs(newDirSupports)) ;
                NS_ENSURE_SUCCESS(rv, rv);
        nsCOMPtr<nsIAbDirectory> childDir = do_QueryInterface(newDirSupports, &rv) ;
                NS_ENSURE_SUCCESS(rv, rv);
        array->AppendElement(childDir);

        rv = enumSubs(childDir,array);
    }
    return rv;
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
    nsCOMPtr <nsIAbDirectoryProperties> properties;
    properties = do_CreateInstance(NS_ABDIRECTORYPROPERTIES_CONTRACTID, &retCode);
    NS_ENSURE_SUCCESS(retCode, retCode) ;

    retCode = properties->SetURI(aFactory.getStr());
    NS_ENSURE_SUCCESS(retCode,retCode);

    nsCOMPtr<nsISimpleEnumerator> subDirs ;
    retCode = dirFactory->CreateDirectory(properties, getter_AddRefs(subDirs));
    NS_ENSURE_SUCCESS(retCode, retCode) ;
    nsCOMPtr<nsISupportsArray> array ;

    NS_NewISupportsArray(getter_AddRefs(array)) ;
    retCode = enumSubs(subDirs,array);
    NS_ENSURE_SUCCESS(retCode, retCode) ;
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

    retCode = rdfService->GetResource(nsDependentCString(aUri.getStr(),aUri.getLength()), getter_AddRefs(rdfResource)) ;
    NS_ENSURE_SUCCESS(retCode, retCode) ;
    retCode = rdfService->UnregisterResource(rdfResource) ;
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

void MDatabaseMetaDataHelper::setAbSpecificError( OConnection* _pCon, sal_Bool bGivenURI )
{
    if ( ! bGivenURI ) {
        m_aErrorString = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("No Mozilla Addressbook Directories Exist"));
    }
    else {
        if (_pCon->usesFactory()) {
            if ( _pCon->isOutlookExpress() ) {
                m_aErrorString = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("No Outlook Express Addressbook Exists"));
            }
            else {
                m_aErrorString = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("No Outlook (MAPI) Addressbook Exists"));
            }
        }
        else {
            if (_pCon->isLDAP()) {
                m_aErrorString = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Unable to connect to LDAP Server"));
            }
            else {
                m_aErrorString = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("No Mozilla Addressbook Directories Exist"));
            }
        }
    }
}

nsresult getTableStringsProxied(const sal_Char* sAbURI, sal_Int32 *nDirectoryType,MNameMapper *nmap,
                        ::std::vector< ::rtl::OUString >*   _rStrings,
                        ::std::vector< ::rtl::OUString >*   _rTypes,
                        rtl::OUString * sError)
{
    if (!sAbURI || !nmap || !_rStrings || !_rTypes || !sError)
    {
        return NS_ERROR_NULL_POINTER;
    }
    OSL_TRACE("Using URI %s to getTables()\n",sAbURI );

    // Get the mozilla addressbooks from the base directory.
    nsresult rv = NS_OK;
    nsCOMPtr<nsIEnumerator> subDirectories;

    switch(*nDirectoryType)
    {
    case SDBCAddress::Mozilla:
        rv = getSubsFromParent(sAbURI, getter_AddRefs(subDirectories)) ;
        break;
    case SDBCAddress::Outlook:
    case SDBCAddress::OutlookExp:
        rv = getSubsFromFactory(sAbURI, getter_AddRefs(subDirectories)) ;
        break;
    case SDBCAddress::LDAP:
        rv = getSubsFromURI(sAbURI, getter_AddRefs(subDirectories)) ;
        break;
    default:
        rv = getSubsFromParent(sAbURI, getter_AddRefs(subDirectories));
    }
    NS_ENSURE_SUCCESS(rv, rv);

    // At this point we have a list containing the nsIAbDirectory we need to map as tables
    rv = subDirectories -> First();
    NS_ENSURE_SUCCESS(rv, rv);

    PRUnichar              *name = nsnull;
    PRBool                  bIsMailList = PR_FALSE;

    ::rtl::OUString aTableName;

    nmap->reset();
    do {
        nsCOMPtr<nsIAbDirectory> subDirectory;

        nsCOMPtr<nsISupports> item;
        rv = subDirectories -> CurrentItem(getter_AddRefs(item));
        if ( NS_FAILED( rv ) ) {
            *sError = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Problem Getting Addressbook Entry"));
            return NS_ERROR_FAILURE;
        }

        subDirectory = do_QueryInterface(item, &rv);

        subDirectory -> GetIsMailList(&bIsMailList);
        // For now we're not interested in mailing lists.
        rv = subDirectory -> GetDirName(&name);
        if ( NS_FAILED( rv ) ) {
            *sError = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Problem Getting Addressbook Directory Name"));
            return NS_ERROR_FAILURE;
        }
        MTypeConverter::prUnicharToOUString(name, aTableName);
        OSL_TRACE("TableName = >%s<\n", OUtoCStr( aTableName ) );

        ODatabaseMetaDataResultSet::ORow aRow(3);

        // Insert table into map
        if ( aTableName.getLength() == 0 )
            aTableName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("AddressBook"));

        OSL_TRACE("TableName = >%s<\n", OUtoCStr( aTableName ) );

        rv = nmap->add( aTableName, subDirectory);
        if (!NS_FAILED(rv)) //failed means we have added this directory
        {
            //map mailing lists as views
            _rStrings->push_back( aTableName ); // Table name
            if (!bIsMailList) {
                ::rtl::OUString aTableType(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TABLE")));
                _rTypes->push_back( aTableType ); // Table type
            }
            else
            {
                ::rtl::OUString aTableType(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("VIEW")));
                _rTypes->push_back( aTableType ); // Table type
            }
        }
        rv = subDirectories -> Next();
    } while ( rv == NS_OK);

    return( NS_OK );
}
sal_Bool MDatabaseMetaDataHelper::getTableStrings( OConnection*                        _pCon,
                                                   ::std::vector< ::rtl::OUString >&   _rStrings,
                                                   ::std::vector< ::rtl::OUString >&   _rTypes)
{
    sal_Bool                                    bGivenURI;
    rtl::OUString                               sAbURI;
    OString                                     sAbURIString;

    OSL_TRACE( "IN MDatabaseMetaDataHelper::getTableStrings( 0x%08X, %s)\n", _pCon, _pCon->getForceLoadTables()?"True":"False" );

    ::osl::MutexGuard aGuard( m_aMetaMutex );
    // Only do the query if we have to - performance degrades otherwise
    if ( ! _pCon->getForceLoadTables() && m_aTableNames.size() > 0 ) {
        _rStrings = m_aTableNames;
        _rTypes   = m_aTableTypes;
        return( sal_True );
    }

    m_aTableNames.clear();
    m_aTableTypes.clear();

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
    sal_Int32 nDirectoryType=0;
    if (!bGivenURI) {
        sAbURIString = s_pADDRESSBOOKROOTDIR;
        nDirectoryType = SDBCAddress::Mozilla;
    }
    else {
        if (_pCon->usesFactory()) {
            nDirectoryType = SDBCAddress::Outlook;
        }
        else {
            if (_pCon->isLDAP()) {
                nDirectoryType = SDBCAddress::LDAP;
            }
            else {
                sAbURIString = s_pADDRESSBOOKROOTDIR;
                nDirectoryType = SDBCAddress::Mozilla;
            }
        }
    }
    if (nDirectoryType == SDBCAddress::Mozilla && !m_bProfileExists)
    {
        setAbSpecificError( _pCon, bGivenURI );
        return sal_False;
    }

    MNameMapper *nmap = _pCon->getNameMapper();
    nmap->reset();

    //rv = getTableStringsProxied(sAbURIString.getStr(),&nDirectoryType,nmap,&m_aTableNames,&m_aTableTypes,&m_aErrorString);

    MNSMozabProxy xMProxy;
    RunArgs args;
    args.funcIndex = ProxiedFunc::FUNC_GET_TABLE_STRINGS;
    args.argCount = 6;
    args.arg1 = (void*)sAbURIString.getStr();
    args.arg2 = (void*)&nDirectoryType;
    args.arg3 = (void*)nmap;
    args.arg4 = (void*)&m_aTableNames;
    args.arg5 = (void*)&m_aTableTypes;
    args.arg6 = (void*)&m_aErrorString;
    rv = xMProxy.StartProxy(&args);

    if (NS_FAILED(rv))
    {
        setAbSpecificError( _pCon, bGivenURI );
        return sal_False;
    }
    OSL_TRACE( "\tOUT MDatabaseMetaDataHelper::getTableStrings()\n" );
    _rStrings = m_aTableNames;
    _rTypes   = m_aTableTypes;

    _pCon->setForceLoadTables(sal_False);
    return( sal_True );
}

sal_Bool MDatabaseMetaDataHelper::getTables( OConnection* _pCon,
                                             const ::rtl::OUString& tableNamePattern,
                                             const Sequence< ::rtl::OUString >& types,
                                             ODatabaseMetaDataResultSet::ORows& _rRows)
{

    static ODatabaseMetaDataResultSet::ORows    aRows;

    OSL_TRACE( "IN MDatabaseMetaDataHelper::getTables()\n" );
    ::osl::MutexGuard aGuard( m_aMetaMutex );

    ODatabaseMetaDataResultSet::ORows().swap(aRows); // this makes real clear where memory is freed as well
    aRows.clear();

    ::std::vector< ::rtl::OUString > tables;
    ::std::vector< ::rtl::OUString > tabletypes;
    ::rtl::OUString matchAny = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%"));;

    if ( !getTableStrings( _pCon, tables,tabletypes ) )
        return sal_False;

    const ::rtl::OUString* pTArray = types.getConstArray();

    for ( sal_Int32 i = 0; i < tables.size(); i++ ) {
        ODatabaseMetaDataResultSet::ORow aRow(3);

        ::rtl::OUString aTableName  = tables[i];
        ::rtl::OUString aTableType      = tabletypes[i];
        OSL_TRACE("TableName = >%s<\n", OUtoCStr( aTableName ) );


        // return tables to caller
        if (match( tableNamePattern, aTableName, '\0' ) &&
                      ( 0 == types.getLength() ||   //types is null or types include aTableType or types include "%"
                       0 != ::comphelper::findValue( types, aTableType, sal_True ).getLength() ||
                       0 != ::comphelper::findValue( types, matchAny, sal_True ).getLength())) {
            if ( aTableName.getLength() == 0 ) {
                aTableName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("AddressBook"));
            }

            OSL_TRACE( "TableName = %s ; TableType = %s\n", OUtoCStr(aTableName), OUtoCStr(aTableType) );

            aRow.push_back( new ORowSetValueDecorator( aTableName ) ); // Table name
            aRow.push_back( new ORowSetValueDecorator( aTableType ) );     // Table type
            aRow.push_back( ODatabaseMetaDataResultSet::getEmptyValue() );                 // Remarks
            aRows.push_back(aRow);
        }
    }

    OSL_TRACE( "\tOUT MDatabaseMetaDataHelper::getTables()\n" );
    _rRows = aRows;
    return(sal_True);
}


sal_Bool
MDatabaseMetaDataHelper::testLDAPConnection( OConnection* _pCon )
{
    const sal_Unicode QUERY_CHAR = '?';
    const sal_Char*   MOZ_SCHEMA = "moz-abldapdirectory://";
    const sal_Char*   LDAP_SCHEMA = "ldap://";

    rtl::OString   sAbURI;
    rtl::OUString   sAbBindDN;
    rtl::OUString   sAbPassword;
    sal_Bool      useSSL    = _pCon->getUseSSL();

    nsresult       rv;

    sAbURI = OUStringToOString( _pCon->getMozURI(), RTL_TEXTENCODING_ASCII_US );
    sAbBindDN   = _pCon->getBindDN();
    sAbPassword = _pCon->getPassword();


    sal_Int32 pos = sAbURI.indexOf( MOZ_SCHEMA );
    if ( pos != -1 ) {
        sAbURI = sAbURI.replaceAt (pos, strlen( MOZ_SCHEMA ), OString(LDAP_SCHEMA) );
    }

    pos = sAbURI.indexOf( QUERY_CHAR );
    if ( pos != -1 ) {
        sal_Int32 len =  sAbURI.getLength();
        sAbURI = sAbURI.replaceAt( pos, len - pos, OString("") );
    }
    const sal_Unicode* bindDN=nsnull;
    if (sAbBindDN.getLength() != 0)
    {
        bindDN = sAbBindDN.getStr();
    }

    RunArgs args;
    args.funcIndex = ProxiedFunc::FUNC_TESTLDAP_INIT_LDAP;
    args.argCount = 4;
    args.arg1 = (void*)sAbURI.getStr();
    args.arg2 = (void*)bindDN;
    args.arg3 = (void*)sAbPassword.getStr();
    args.arg4 = (void*)&useSSL;

    MNSMozabProxy xMProxy;
    if (xMProxy.StartProxy(&args))  //Init LDAP
    {
        args.funcIndex = ProxiedFunc::FUNC_TESTLDAP_IS_LDAP_CONNECTED;
        TimeValue               timeValue = { 1, 0 };  // 1 * 60 Seconds timeout
        sal_Int32               times=0;
        while( times < 60 )
        {
            rv = xMProxy.StartProxy(&args); //check whether ldap is connect
            if (!rv )
            {
                osl_waitThread(&timeValue);
                times++;
            }
            else
                break;
        }
        args.funcIndex = ProxiedFunc::FUNC_TESTLDAP_IS_LDAP_CONNECTED;
        rv = xMProxy.StartProxy(&args); //release resource
    }
    return rv;
}

sal_Bool MDatabaseMetaDataHelper::NewAddressBook(OConnection* _pCon,const ::rtl::OUString & aTableName)
{
    sal_Bool                                    bIsMozillaAB;

    bIsMozillaAB = !_pCon->usesFactory() && ! _pCon->isLDAP();

    if ( !bIsMozillaAB )
    {
        m_aErrorString = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("We do not support create table for this kind of AddressBook"));
        return sal_False;
    }
    else
        OSL_TRACE("Create table\n");

    // Get the mozilla addressbooks from the base directory.
    nsresult rv = NS_OK;

    MNSMozabProxy xMProxy;
    RunArgs args;
    args.funcIndex = ProxiedFunc::FUNC_NEW_ADDRESS_BOOK;
    args.argCount = 1;
    args.arg1 = (void*)&aTableName;
    rv = xMProxy.StartProxy(&args);

    _pCon->setForceLoadTables(sal_True); //force reload table next time
    if (rv == NS_ERROR_FILE_IS_LOCKED)
    {
        m_aErrorString = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("You can't create Mozilla AddressBook while Mozilla is running!"));
    }
    else if (NS_FAILED(rv))
    {
        setAbSpecificError( _pCon, !bIsMozillaAB );
    }
    OSL_TRACE( "OUT MDatabaseMetaDataHelper::NewAddressBook()\n" );
    return( NS_SUCCEEDED(rv) );
}
nsresult NewAddressBook(const ::rtl::OUString * aName)
{
    if (isProfileLocked())
        return NS_ERROR_FILE_IS_LOCKED;
    nsresult rv;
    nsCOMPtr<nsIAbDirectoryProperties> aProperties = do_CreateInstance(NS_ABDIRECTORYPROPERTIES_CONTRACTID, &rv);
    NS_ENSURE_ARG_POINTER(aProperties);
    const ::rtl::OUString& uName = *aName;
    nsString nsName;
    MTypeConverter::ouStringToNsString(uName,nsName);
    aProperties->SetDescription(nsName);

    nsCOMPtr<nsIRDFService> rdfService = do_GetService (kRDFServiceCID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIRDFResource> parentResource;
    rv = rdfService->GetResource(NS_LITERAL_CSTRING(kAllDirectoryRoot), getter_AddRefs(parentResource));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIAbDirectory> parentDir = do_QueryInterface(parentResource, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = parentDir->CreateNewDirectory(aProperties);
    return rv;
}
