/*************************************************************************
*
*  $RCSfile: ScriptStorageManager.cxx,v $
*
*  $Revision: 1.29 $
*
*  last change: $Author: npower $ $Date: 2003-10-15 08:31:40 $
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
*  Contributor(s): _______________________________________
*
*
************************************************************************/

#include <stdio.h>

#include <cppuhelper/implementationentry.hxx>
#include <sal/config.h>
#include <rtl/uri.hxx>

#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptInfoAccess.hpp>

#include "ScriptStorageManager.hxx"
#include <util/util.hxx>
#include <util/scriptingconstants.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

namespace scripting_impl
{

static OUString s_implName =
    ::rtl::OUString::createFromAscii(
        "drafts.com.sun.star.script.framework.storage.ScriptStorageManager" );
static OUString s_serviceName =
    ::rtl::OUString::createFromAscii(
        "drafts.com.sun.star.script.framework.storage.ScriptStorageManager" );
static Sequence< OUString > s_serviceNames = Sequence< OUString >( &s_serviceName, 1 );

//extern ::rtl_StandardModuleCount s_moduleCount = MODULE_COUNT_INIT;
//extern ::rtl_StandardModuleCount s_moduleCount;


//*************************************************************************
// ScriptStorageManager Constructor
ScriptStorageManager::ScriptStorageManager( const Reference<
        XComponentContext > & xContext ) SAL_THROW ( ( RuntimeException ) )
        : m_xContext( xContext ), m_count( 0 ), m_securityMgr( xContext )
{
    OSL_TRACE( "< ScriptStorageManager ctor called >\n" );
    //s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );

    validateXRef( m_xContext,
                  "ScriptStorageManager::ScriptStorageManager : cannot get component context" );

    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr,
                  "ScriptStorageManager::ScriptStorageManager : cannot get service manager" );

    try
    {
        // obtain the macro expander singleton to use in determining the
        // location of the application script storage
        Any aAny = m_xContext->getValueByName( OUString::createFromAscii(
                                                   "/singletons/com.sun.star.util.theMacroExpander" ) );
        Reference< util::XMacroExpander > xME;
        if ( sal_False == ( aAny >>= xME ) )
        {
            throw RuntimeException(
                OUSTR( "ScriptStorageManager::ScriptStorageManager: can't get XMacroExpander" ),
                Reference< XInterface >() );
        }
        validateXRef( xME, "ScriptStorageManager constructor: can't get MacroExpander" );

        OUString base = OUString::createFromAscii(
                            SAL_CONFIGFILE( "${$SYSBINDIR/bootstrap" ) );

        setupAppStorage( xME,
                         base.concat( OUString::createFromAscii( "::BaseInstallation}/share" ) ),
                         OUSTR( "SHARE" ) );
        setupAppStorage( xME,
                         base.concat( OUString::createFromAscii( "::UserInstallation}/user" ) ),
                         OUSTR( "USER" ) );

    }
    catch ( Exception & e )
    {
        throw RuntimeException( OUSTR( "ScriptStorageManager::ScriptStorageManager: " ).concat( e.Message ), Reference< XInterface >() );
    }
    OSL_ASSERT( m_count == 2 );
}

//*************************************************************************
// ScriptStorageManager setupAppStorage
void
ScriptStorageManager::setupAppStorage(
    const Reference< util::XMacroExpander > & xME,
    const OUString & storageStr,
    const OUString & appStr)
SAL_THROW ( ( RuntimeException ) )
{
    try
    {
        Reference< XInterface > xInterface =
            m_xMgr->createInstanceWithContext(
                OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ), m_xContext );
        validateXRef( xInterface,
                      "ScriptStorageManager constructor: can't get SimpleFileAccess XInterface" );
        Reference< ucb::XSimpleFileAccess > xSFA( xInterface, UNO_QUERY_THROW );

        setupAnyStorage( xSFA, xME->expandMacros( storageStr ), appStr );
    }
    catch ( Exception & e )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorageManager::setupAppStorage: " ).concat( e.Message ),
            Reference< XInterface >() );
    }
}

//*************************************************************************
// ScriptStorageManager setupAnyStorage
sal_Int32
ScriptStorageManager::setupAnyStorage(
    const Reference< ucb::XSimpleFileAccess > & xSFA,
    const OUString & storageStr,
    const OUString & origStringURI )
SAL_THROW ( ( RuntimeException ) )
{
    // Required for scope of fnc to protect all access read and writes to m_count
    ::osl::Guard< ::osl::Mutex > aGuard( m_mutex );
    try
    {

        // create a ScriptingStorage using the SimpleFileAccess, the storageID
        // (from the count), and the URL to the application's shared area
        Sequence < Any > aArgs( 3 );
        aArgs[ 0 ] <<= xSFA;
        aArgs[ 1 ] <<= m_count;
        aArgs[ 2 ] <<= storageStr;

        OSL_TRACE( "creating storage for: %s\n",
                   ::rtl::OUStringToOString( storageStr,
                                             RTL_TEXTENCODING_ASCII_US ).pData->buffer );

        Reference< XInterface > xInterface =
            m_xMgr->createInstanceWithArgumentsAndContext(
                OUString::createFromAscii(
                    "drafts.com.sun.star.script.framework.storage.ScriptStorage" ),
                aArgs, m_xContext );

        validateXRef( xInterface, "ScriptStorageManager:: setupAnyStorage: Can't create ScriptStorage for share" );

        // and place it in the hash_map. Increment the counter
        m_ScriptStorageMap[ m_count++ ] = xInterface;
        sal_Int32 sid =  m_count - 1;

        // create hash of original uri i.e. file:///home/users/docs/mydoc.sxw
        // and storage id (sid) this will allow us to retrieve
        // the sid based on the url of the document.
        m_StorageIdOrigURIHash [ origStringURI ] = sid;
        OSL_TRACE( "\tcreated with ID=%d\n", m_count - 1 );

    }
    catch ( Exception & e )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorageManager::setupAnyStorage: " ).concat(
                e.Message ), Reference< XInterface >() );
    }

    return m_count -1;
}

//*************************************************************************
// ScriptStorageManager Destructor
ScriptStorageManager::~ScriptStorageManager()
SAL_THROW ( () )
{
    OSL_TRACE( "< ScriptStorageManager dtor called >\n" );
//    s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
}

//*************************************************************************
// This method assumes that the XSimpleFileAccess knows it's on root URL
// and can be used with relative URLs
sal_Int32 SAL_CALL
ScriptStorageManager::createScriptStorage(
    const Reference< ucb::XSimpleFileAccess >& xSFA )
throw ( RuntimeException )
{
    OSL_TRACE( "** ==> ScriptStorageManager in createScriptingStorage\n" );
    validateXRef( xSFA,
                  "ScriptStorageManager::createScriptStorage: XSimpleFileAccess is not valid" );

    return setupAnyStorage( xSFA, ::rtl::OUString::createFromAscii( "" ),
                            ::rtl::OUString::createFromAscii( "" ) );
}

//*************************************************************************
sal_Int32 SAL_CALL
ScriptStorageManager::createScriptStorageWithURI(
    const Reference< ucb::XSimpleFileAccess >& xSFA, const OUString & cStringURI )
throw ( RuntimeException )
{
    OSL_TRACE( "** ==> ScriptStorageManager in createScriptingStorageWithURI\n" );
    validateXRef( xSFA, "ScriptStorageManager::createScriptStorage: XSimpleFileAccess is not valid" );

    // related to issue 11866
    // warning dialog gets launched when adding binding to script in doc
    // workaround issue: no functionProvider created on doc open
    // if NODIALOG tag, strip from stringURI, set boolean=true
    bool displayDialog = true;
    ::rtl::OUString dialogTag = ::rtl::OUString::createFromAscii( "NoDialog::" );
    ::rtl::OUString stringURI = cStringURI;
    if( stringURI.indexOf( dialogTag ) == 0 )
    {
        OSL_TRACE( "ScriptStorageManager::createScriptStorage: will not display security dialogs" );
        stringURI = stringURI.copy( dialogTag.getLength() );
        displayDialog = false;
    }
    sal_Int32 returnedID = getScriptStorageID(stringURI);


    // convert file:///... url to vnd... syntax
    ::rtl::OUString canonicalURI(
        ::rtl::OUString::createFromAscii( "vnd.sun.star.pkg://" ) );
    canonicalURI = canonicalURI.concat( ::rtl::Uri::encode( stringURI,
                                        rtl_UriCharClassUricNoSlash, rtl_UriEncodeCheckEscapes,
                                        RTL_TEXTENCODING_ASCII_US ) );

    if (returnedID == -1)
    {
        OSL_TRACE("Creating new storage for %s",
            ::rtl::OUStringToOString( stringURI,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        returnedID = setupAnyStorage( xSFA, canonicalURI, stringURI );
    }
    else
    {
       OSL_TRACE("Using existing storage for %s",
           ::rtl::OUStringToOString( stringURI,
               RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    }

// np - removed previous scripting framework security handling
// now handled by modification to existing calls in sfx for basic
//
/*    if( displayDialog )
    {
        try
        {
           OSL_TRACE("Adding to security mgr for %s",
               ::rtl::OUStringToOString( stringURI,
                   RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            m_securityMgr.addScriptStorage( stringURI, returnedID );
        }
        catch ( RuntimeException & rte )
        {
            throw RuntimeException(
                OUSTR( "ScriptStorageManager::createScriptStorageWithURI: " ).concat(
                    rte.Message ), Reference< XInterface >() );
        }
    }
    else
    {
       OSL_TRACE("No need to security mgr for %s",
           ::rtl::OUStringToOString( stringURI,
               RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    }*/
    return returnedID;
}

//*************************************************************************
Reference < XInterface > SAL_CALL
ScriptStorageManager::getScriptStorage( sal_Int32 scriptStorageID )
throw( RuntimeException )
{
    OSL_TRACE( "** ==> ScriptStorageManager in getStorageInstance\n" );
    OSL_TRACE( "** ==> request for id=%d",scriptStorageID );

    ScriptStorage_map::const_iterator itr =
        m_ScriptStorageMap.find( scriptStorageID );

    if ( itr == m_ScriptStorageMap.end() )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorageManager::getScriptStorage: invalid storage ID" ),
            Reference< XInterface >() );
    }
    validateXRef( itr->second,
                  "ScriptStorageManager::getScriptStorage: Cannot get ScriptStorage from ScriptStorageHash" );
    return itr->second;
}

//*******************************************************************
sal_Int32 SAL_CALL
ScriptStorageManager::getScriptStorageID( const ::rtl::OUString& origURI )
        throw (::com::sun::star::uno::RuntimeException)
{
    StorageId_hash::const_iterator it = m_StorageIdOrigURIHash.find( origURI );

    if ( it == m_StorageIdOrigURIHash.end() )
    {
        OUString message = OUSTR( "ScriptStorageManager::getScriptStorageID(): Cannot find storage for " );
        if ( origURI.getLength() == 0 )
        {
            message = message.concat( OUSTR("Empty URI") );
        }
        else
        {
            message = message.concat( origURI );
        }
        OSL_TRACE( ::rtl::OUStringToOString( message,
                                            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        return -1;
    }

    return it->second;
}

//*******************************************************************
void
ScriptStorageManager::removeScriptDocURIHashEntry( const OUString & origURI )
{
    StorageId_hash::iterator it = m_StorageIdOrigURIHash.find( origURI );
    if ( it == m_StorageIdOrigURIHash.end() )
    {
        OSL_TRACE( "ScriptStorageManager::removeScriptDocURIHashEntry: no entry to remove" );
        return;
    }

    // remove entry for this doc url from orig uri map.
    m_StorageIdOrigURIHash.erase( it );
}

//*******************************************************************
void SAL_CALL
ScriptStorageManager::refreshScriptStorage( const OUString & stringURI )
throw( RuntimeException )
{
    OSL_TRACE( "** => ScriptStorageManager in refreshScriptStorage\n" );
    OSL_TRACE( "** => refreshing URI: %s\n",
               ::rtl::OUStringToOString(
                   stringURI, RTL_TEXTENCODING_ASCII_US ).pData->buffer);

    sal_Int32 storageID = getScriptStorageID( stringURI );

    if ( storageID == -1 )
    {
        OSL_TRACE( "** id was -1, no storage");
        // Refreshing noexistent storage - just return
        return;
    }

    try
    {
        Reference < storage::XScriptStorageRefresh > xSSR(
            getScriptStorage( storageID ), UNO_QUERY );

        xSSR->refresh();
    }
    catch ( RuntimeException & e )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorageManager::refreshScriptStorage: " ).concat(
                e.Message ), Reference< XInterface >() );
    }
    catch ( Exception & e )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorageManager::refreshScriptStorage: " ).concat(
                e.Message ), Reference< XInterface >() );
    }
}

//*************************************************************************
void SAL_CALL
ScriptStorageManager::checkPermission( const OUString &
scriptStorageURI, const OUString & permissionRequest )
throw ( RuntimeException, lang::IllegalArgumentException, css::security::AccessControlException )
{
    try
    {
        m_securityMgr.checkPermission( scriptStorageURI, permissionRequest );
    }
    catch ( css::security::AccessControlException & e )
    {
        throw css::security::AccessControlException(
            OUSTR( "ScriptStorageManager::checkPermission: AccessControlException: " ).concat(
                e.Message ), Reference< XInterface >(), e.LackingPermission );
    }
    catch ( lang::IllegalArgumentException & e )
    {
        throw lang::IllegalArgumentException(
            OUSTR( "ScriptStorageManager::checkPermission: IllegalArgumentException: " ).concat(
                e.Message ), Reference< XInterface >(), e.ArgumentPosition );
    }
    catch ( RuntimeException & e )
    {
        throw RuntimeException(
            OUSTR( "ScriptStorageManager::checkPermission: RuntimeException: " ).concat(
                e.Message ), Reference< XInterface >() );
    }
}

//*************************************************************************
OUString SAL_CALL
ScriptStorageManager::getImplementationName( )
throw( RuntimeException )
{
    return s_implName;
}

//*************************************************************************
sal_Bool SAL_CALL
ScriptStorageManager::supportsService( const OUString& serviceName )
throw( RuntimeException )
{
    OUString const * pNames = s_serviceNames.getConstArray();
    for ( sal_Int32 nPos = s_serviceNames.getLength(); nPos--; )
    {
        if ( serviceName.equals( pNames[ nPos ] ) )
        {
            return sal_True;
        }
    }
    return sal_False;
}

//*************************************************************************
Sequence< OUString > SAL_CALL
ScriptStorageManager::getSupportedServiceNames( )
throw( RuntimeException )
{
    return s_serviceNames;
}

//*************************************************************************
void SAL_CALL
ScriptStorageManager::disposing( const ::com::sun::star::lang::EventObject& Source )
throw ( ::com::sun::star::uno::RuntimeException )
{
    OSL_TRACE( "ScriptStorageManager::disposing started" );
    OSL_TRACE( "event object type=%s",
                      ::rtl::OUStringToOString( getCppuType( &Source ).getTypeName(),
                                                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    OUString storageURI;
    bool removeSecurityPermission = true;
    try
    {
        Reference< XInterface > xInterface = Source.Source;
        // no UNO_QUERY_THROW since we want a 2nd change to query if it's
        // not a document being disposed
        Reference< frame::XModel > xModel = Reference< frame::XModel > ( xInterface, UNO_QUERY );
        if( xModel.is() )
        {
            storageURI = xModel->getURL();
        }
        else
        {
            // UNO_QURY_THROW here since it's supposed to be either a doc
            // or a XScriptInfo (in the case of a filesys script)
            Reference< storage::XScriptInfo > xScriptInfo = Reference< storage::XScriptInfo > ( xInterface, UNO_QUERY_THROW );
            storageURI = xScriptInfo->getParcelURI().concat( xScriptInfo->getFunctionName() );
            // to save the user seeing the security dialogs every time they
            // run the script we hang on to the permission for a given script
            // for the lifetime of the Office
            removeSecurityPermission = false;
            // possibly need to encode it??
        }
        if ( storageURI.getLength() > 0 )
        {
            OSL_TRACE( "URI disposing is ... %s",
                      ::rtl::OUStringToOString( storageURI,
                                                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
    }
    catch ( RuntimeException& e )
    {
        OUString message =
            OUSTR(
                "ScriptStorageManager::disposing: can't get script context, reason = " );
        message = message.concat( e.Message );
        OSL_TRACE( ::rtl::OUStringToOString( message,
                                            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        return;
    }


    // grab storage id.
    sal_Int32 scriptStorageID = getScriptStorageID( storageURI );

    // no need to do anything if there's no doc storage
    if( scriptStorageID == -1 )
    {
        return;
    }

    OSL_TRACE( "disposing storageID = %d", scriptStorageID );

    // attempt to get the storage from the hash to ensure that we have a
    // valid storageID
    ScriptStorage_map::const_iterator itr =
        m_ScriptStorageMap.find( scriptStorageID );

    if ( itr == m_ScriptStorageMap.end() )
    {
        OSL_TRACE( "Entry for storage id %d doesn't exist in map", scriptStorageID );
        return;
    }

    // erase the entry from the hash
    m_ScriptStorageMap.erase( scriptStorageID );
    removeScriptDocURIHashEntry( storageURI );
    if ( removeSecurityPermission )
    {
        m_securityMgr.removePermissionSettings ( storageURI );
    }
}
} // Namespace

namespace scripting_runtimemgr
{
//*************************************************************************
Reference< XInterface > SAL_CALL
ssm_create(
    const Reference< XComponentContext > & xCompC )
{
    return ( cppu::OWeakObject * ) new ::scripting_impl::ScriptStorageManager( xCompC );
}

//*************************************************************************
Sequence< OUString >
ssm_getSupportedServiceNames( )
SAL_THROW( () )
{
    return ::scripting_impl::s_serviceNames;
}

//*************************************************************************
OUString
ssm_getImplementationName( )
SAL_THROW( () )
{
    return ::scripting_impl::s_implName;
}
}
