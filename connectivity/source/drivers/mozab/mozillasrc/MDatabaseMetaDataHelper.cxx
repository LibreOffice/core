/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MDatabaseMetaDataHelper.cxx,v $
 * $Revision: 1.16 $
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

// Mozilla includes.
#include <MNSInclude.hxx>

#include "resource/mozab_res.hrc"
#include "MDatabaseMetaDataHelper.hxx"
#ifndef _CONNECTIVITY_MAB_CONVERSIONS_HXX_
#include "MTypeConverter.hxx"
#endif
#ifndef CONNECTIVITY_MOZAB_MCONFIGACCESS_HXX
#include "MConfigAccess.hxx"
#endif
#include <connectivity/dbexception.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/sequence.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>

#include <MNSInit.hxx>
#include <MNameMapper.hxx>
#include "MNSMozabProxy.hxx"
#include <MNSDeclares.hxx>
static ::osl::Mutex m_aMetaMutex;

#include <osl/diagnose.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <unotools/processfactory.hxx>
#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>


#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */


static NS_DEFINE_CID(kRDFServiceCID, NS_RDFSERVICE_CID);
static NS_DEFINE_CID(kAbDirFactoryServiceCID, NS_ABDIRFACTORYSERVICE_CID);
static NS_DEFINE_CID(kAddrBookSessionCID, NS_ADDRBOOKSESSION_CID);

using namespace connectivity::mozab;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace com::sun::star::mozilla;

namespace connectivity
{
    namespace mozab
    {
        static const char * const s_pADDRESSBOOKROOTDIR = "moz-abdirectory://";
    }
}

extern sal_Bool MNS_Init(sal_Bool& aProfileExists);

// -------------------------------------------------------------------------
MDatabaseMetaDataHelper::MDatabaseMetaDataHelper()
    :m_bProfileExists(sal_False)
{
    OSL_TRACE( "IN MDatabaseMetaDataHelper::MDatabaseMetaDataHelper()\n" );

    OSL_TRACE( "\tOUT MDatabaseMetaDataHelper::MDatabaseMetaDataHelper()\n" );

}
// -------------------------------------------------------------------------
MDatabaseMetaDataHelper::~MDatabaseMetaDataHelper()
{
    m_aTableNames.clear();
    m_aTableTypes.clear();

    if (m_bProfileExists)
    {
        Reference<XMozillaBootstrap> xMozillaBootstrap;
        Reference<XMultiServiceFactory> xFactory = ::comphelper::getProcessServiceFactory();
        OSL_ENSURE( xFactory.is(), "can't get service factory" );

        Reference<XInterface> xInstance = xFactory->createInstance(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.mozilla.MozillaBootstrap")) );
        OSL_ENSURE( xInstance.is(), "failed to create instance" );
        xMozillaBootstrap = Reference<XMozillaBootstrap>(xInstance,UNO_QUERY);
        m_bProfileExists = xMozillaBootstrap->shutdownProfile() > 0;
    }

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
//
// nsAbDirectoryDataSource
//
#define NS_RDF_CONTRACTID                           "@mozilla.org/rdf"
#define NS_RDF_DATASOURCE_CONTRACTID                NS_RDF_CONTRACTID "/datasource;1"
#define NS_RDF_DATASOURCE_CONTRACTID_PREFIX NS_RDF_DATASOURCE_CONTRACTID "?name="
#define NS_ABDIRECTORYDATASOURCE_CONTRACTID \
  NS_RDF_DATASOURCE_CONTRACTID_PREFIX "addressdirectory"
#define database_uri "@mozilla.org/rdf/datasource;1?name=addressdirectory"
#define NS_ABDIRECTORYDATASOURCE_CID            \
{ /* 0A79186D-F754-11d2-A2DA-001083003D0C */        \
    0xa79186d, 0xf754, 0x11d2,              \
    {0xa2, 0xda, 0x0, 0x10, 0x83, 0x0, 0x3d, 0xc}   \
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

    nsCOMPtr<nsIRDFDataSource> rdfDirectory ;

    rtl::OString dir("rdf:addressdirectory");
    retCode = rdfService->GetDataSource(dir.getStr(),getter_AddRefs(rdfDirectory)) ;



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
    if ( ! bGivenURI && m_ProductType ==::com::sun::star::mozilla::MozillaProductType_Mozilla)
    {
        setError( STR_NO_MOZIILA_ADDRESSBOOK );
    }
    else
    {
        if ( m_ProductType ==::com::sun::star::mozilla::MozillaProductType_Thunderbird)
        {
            setError( STR_NO_THUNDERBIRD_ADDRESSBOOK );
        }
        else
        {
            if (_pCon->usesFactory())
            {
                if ( _pCon->isOutlookExpress() )
                {
                    setError( STR_NO_OUTLOOKEXPRESS_ADDRESSBOOK );
                }
                else
                {
                    setError( STR_NO_OUTLOOK_ADDRESSBOOK );
                }
            }
            else
            {
                if (_pCon->isLDAP())
                {
                    setError( STR_COULDNOTCONNECT_TO_LDAP );
                }
                else
                {
                    setError( STR_NO_MOZIILA_ADDRESSBOOK );
                }
            }
        }
    }
}

nsresult getTableStringsProxied(const sal_Char* sAbURI, sal_Int32 *nDirectoryType,MNameMapper *nmap,
                        ::std::vector< ::rtl::OUString >*   _rStrings,
                        ::std::vector< ::rtl::OUString >*   _rTypes,
                        sal_Int32* pErrorId )
{
    if (!sAbURI || !nmap || !_rStrings || !_rTypes || !pErrorId)
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
    nsCOMPtr<nsIRDFService> rdfService(do_GetService(kRDFServiceCID, &rv)) ;
    NS_ENSURE_SUCCESS(rv, rv) ;

    nmap->reset();
    do {
        nsCOMPtr<nsIAbDirectory> subDirectory;

        nsCOMPtr<nsISupports> item;
        rv = subDirectories -> CurrentItem(getter_AddRefs(item));
        if ( NS_FAILED( rv ) )
        {
            *pErrorId = STR_COULD_NOT_RETRIEVE_AB_ENTRY;
            return NS_ERROR_FAILURE;
        }

        subDirectory = do_QueryInterface(item, &rv);

        subDirectory -> GetIsMailList(&bIsMailList);
        // For now we're not interested in mailing lists.
        rv = subDirectory -> GetDirName(&name);
        if ( NS_FAILED( rv ) ) {
            *pErrorId = STR_COULD_NOT_GET_DIRECTORY_NAME;
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
        nsCOMPtr<nsIRDFResource> rdfResource = do_QueryInterface(subDirectory, &rv) ;
        if (!NS_FAILED(rv))
            rdfService->UnregisterResource(rdfResource);

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
    ::rtl::OUString                             sAbURI;
    ::rtl::OString                                      sAbURIString;

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
    m_ProductType=::com::sun::star::mozilla::MozillaProductType_Mozilla;
    m_ProfileName = _pCon->getMozProfile();


    if (_pCon->isThunderbird())
    {
        if (!bGivenURI)
            sAbURIString = s_pADDRESSBOOKROOTDIR;
        nDirectoryType = SDBCAddress::ThunderBird;
        m_ProductType = ::com::sun::star::mozilla::MozillaProductType_Thunderbird;
    }
    else
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

    if (!m_bProfileExists)
    {
        Reference<XMozillaBootstrap> xMozillaBootstrap;
        Reference<XMultiServiceFactory> xFactory = ::comphelper::getProcessServiceFactory();
        OSL_ENSURE( xFactory.is(), "can't get service factory" );
         Reference<XInterface> xInstance = xFactory->createInstance(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.mozilla.MozillaBootstrap")) );
        OSL_ENSURE( xInstance.is(), "failed to create instance" );
        xMozillaBootstrap = Reference<XMozillaBootstrap>(xInstance,UNO_QUERY);
        m_bProfileExists = sal_False;
        //If there are no profiles for this product
        //Or the given profile name does not found
        //We will reaise a "No Addressbook Directories Exist" error
        if ( xMozillaBootstrap->getProfileCount(m_ProductType) == 0 ||
            ( m_ProfileName.getLength() && !(xMozillaBootstrap->getProfilePath(m_ProductType,m_ProfileName).getLength())))
            m_bProfileExists = sal_False;
        else
            if (xMozillaBootstrap->bootupProfile(m_ProductType,m_ProfileName) > 0)
                m_bProfileExists = sal_True;

    }
    if ( ( nDirectoryType == SDBCAddress::Mozilla
         || m_ProductType ==::com::sun::star::mozilla::MozillaProductType_Thunderbird)
        && !m_bProfileExists)
    {
        setAbSpecificError( _pCon, bGivenURI );
        return sal_False;
    }

    MNameMapper *nmap = _pCon->getNameMapper();
    nmap->reset();

    sal_Int32 nErrorResourceId( 0 );

    MNSMozabProxy xMProxy;
    RunArgs args;
    args.funcIndex = ProxiedFunc::FUNC_GET_TABLE_STRINGS;
    args.argCount = 6;
    args.arg1 = (void*)sAbURIString.getStr();
    args.arg2 = (void*)&nDirectoryType;
    args.arg3 = (void*)nmap;
    args.arg4 = (void*)&m_aTableNames;
    args.arg5 = (void*)&m_aTableTypes;
    args.arg6 = (void*)&nErrorResourceId;
    rv = xMProxy.StartProxy(&args,m_ProductType,m_ProfileName);
    setError( nErrorResourceId );

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

    for ( size_t i = 0; i < tables.size(); i++ ) {
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

    nsresult       rv(0);

    sAbURI = OUStringToOString( _pCon->getMozURI(), RTL_TEXTENCODING_ASCII_US );
    sAbBindDN   = _pCon->getBindDN();
    sAbPassword = _pCon->getPassword();


    sal_Int32 pos = sAbURI.indexOf( MOZ_SCHEMA );
    if ( pos != -1 ) {
        sAbURI = sAbURI.replaceAt (pos, strlen( MOZ_SCHEMA ), ::rtl::OString(LDAP_SCHEMA) );
    }

    pos = sAbURI.indexOf( QUERY_CHAR );
    if ( pos != -1 ) {
        sal_Int32 len =  sAbURI.getLength();
        sAbURI = sAbURI.replaceAt( pos, len - pos, ::rtl::OString("") );
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
    rv = xMProxy.StartProxy( &args, m_ProductType, ::rtl::OUString() );
    if ( NS_SUCCEEDED( rv ) )   //Init LDAP,pass OUString() to StarProxy to ignore profile switch
    {
        args.funcIndex = ProxiedFunc::FUNC_TESTLDAP_IS_LDAP_CONNECTED;
        TimeValue               timeValue = { 1, 0 };  // 1 * 30 Seconds timeout
        sal_Int32               times=0;
        while ( times++ < 30 )
        {
            rv = xMProxy.StartProxy( &args, m_ProductType, ::rtl::OUString() );
            if ( NS_SUCCEEDED( rv ) )
                // connected successfully
                break;

            if ( rv == (nsresult)PR_NOT_CONNECTED_ERROR )
                // connection failed
                break;

            // not yet decided - continue waiting
            osl_waitThread( &timeValue );
        }
    }
    setError( STR_COULD_NOT_CONNECT_LDAP );
    return NS_SUCCEEDED( rv );
}

sal_Bool MDatabaseMetaDataHelper::NewAddressBook(OConnection* _pCon,const ::rtl::OUString & aTableName)
{
    sal_Bool                                    bIsMozillaAB;

    bIsMozillaAB = !_pCon->usesFactory() && ! _pCon->isLDAP();

    if ( !bIsMozillaAB )
    {
        setError( STR_NO_TABLE_CREATION_SUPPORT );
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
    rv = xMProxy.StartProxy(&args,m_ProductType,m_ProfileName);

    _pCon->setForceLoadTables(sal_True); //force reload table next time
    if (rv == NS_ERROR_FILE_IS_LOCKED)
    {
        setError( STR_MOZILLA_IS_RUNNING );
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
    if (isProfileLocked(NULL))
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
