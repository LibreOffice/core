/*************************************************************************
 *
 *  $RCSfile: ScriptSecurityManager.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: npower $ $Date: 2003-08-19 09:49:26 $
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

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/security/AccessControlException.hpp>
#include <com/sun/star/security/RuntimePermission.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptStorageManager.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptInfoAccess.hpp>
#include "ScriptSecurityManager.hxx"
#include <util/util.hxx>
#include <util/scriptingconstants.hxx>


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

// is this in the utils?
const char* const SCRIPTSTORAGEMANAGER_SERVICE =
    "/singletons/drafts.com.sun.star.script.framework.storage.theScriptStorageManager";

namespace scripting_securitymgr
{

static OUString s_configProv = ::rtl::OUString::createFromAscii(
    "com.sun.star.configuration.ConfigurationProvider");

static OUString s_configAccess = ::rtl::OUString::createFromAscii(
    "com.sun.star.configuration.ConfigurationAccess");

static OUString s_configUpdate = ::rtl::OUString::createFromAscii(
    "com.sun.star.configuration.ConfigurationUpdateAccess");

static OUString s_securityDialog = ::rtl::OUString::createFromAscii(
    "com.sun.star.script.framework.security.SecurityDialog");

static const int PERMISSION_NEVER = 0;
static const int PERMISSION_PATHLIST = 1;
static const int PERMISSION_ALWAYS = 2;

static const int ALLOW_RUN = 1;
static const int ADD_TO_PATH = 2;

//*************************************************************************
// ScriptSecurityManager Constructor
ScriptSecurityManager::ScriptSecurityManager(
    const Reference< XComponentContext > & xContext ) throw ( RuntimeException )
    : m_xContext( xContext)
{
    OSL_TRACE( "< ScriptSecurityManager ctor called >\n" );
    validateXRef( m_xContext,
        "ScriptSecurityManager::ScriptSecurityManager: invalid context" );

    // get the service manager from the context
    Reference< lang::XMultiComponentFactory > xMgr = m_xContext->getServiceManager();
    validateXRef( xMgr,
        "ScriptSecurityManager::ScriptSecurityManager: cannot get ServiceManager" );

    // create an instance of the ConfigurationProvider
    Reference< XInterface > xInterface = xMgr->createInstanceWithContext(
        s_configProv, m_xContext );
    validateXRef( xInterface,
        "ScriptSecurityManager::ScriptSecurityManager: cannot get ConfigurationProvider" );
    // create an instance of the ConfigurationAccess for accessing the
    // scripting security settings
    m_xConfigProvFactory = Reference < lang::XMultiServiceFactory > ( xInterface, UNO_QUERY );
    validateXRef( m_xConfigProvFactory,
        "ScriptSecurityManager::ScriptSecurityManager: cannot get XMultiServiceFactory interface from ConfigurationProvider" );

}

void ScriptSecurityManager::addScriptStorage( rtl::OUString scriptStorageURL,
    sal_Int32 storageID)
throw ( RuntimeException )
{
    Permission_Hash::const_iterator ph_it = m_permissionSettings.find( scriptStorageURL );
    if ( ph_it != m_permissionSettings.end() )
    {
        OSL_TRACE( "ScriptSecurityManager::addScriptStorage: already called for %s",
            ::rtl::OUStringToOString( scriptStorageURL,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer);
        return;
    }
    StoragePerm newPerm;
    newPerm.scriptStorageURL=scriptStorageURL;
    newPerm.storageID=storageID;

    // we err on the side of caution!!
    newPerm.execPermission=sal_False;

    //need to check if storage has any scripts
    try
    {
        /* need to replace this with something better, now logical names are
         * gone

        Reference< XInterface > xInterface;
        Any a = m_xContext->getValueByName(
                OUString::createFromAscii( SCRIPTSTORAGEMANAGER_SERVICE ) );
        if ( sal_False == ( a >>= xInterface ) )
        {
            throw RuntimeException(
                OUSTR( "ScriptSecurityManager::addScriptStorage: could not obtain ScriptStorageManager singleton" ),
            Reference< XInterface >() );
        }
        validateXRef( xInterface,
            "ScriptSecurityManager::addScriptStorage: cannot get Storage service" );
        Reference< storage::XScriptStorageManager > xScriptStorageManager(
            xInterface, UNO_QUERY_THROW );
        Reference< XInterface > xScriptStorage =
            xScriptStorageManager->getScriptStorage( storageID );
        validateXRef( xScriptStorage,
          "ScriptNameResolverImpl::getStorageInstance: cannot get Script Storage service" );
        Reference< storage::XScriptInfoAccess > xScriptInfoAccess =
            Reference< storage::XScriptInfoAccess > ( xScriptStorage,
            UNO_QUERY_THROW );
        Sequence< ::rtl::OUString > logicalNames = xScriptInfoAccess->getScriptLogicalNames();
        if( !logicalNames.getLength() ) // we have no logical names
        {
            return;
        } */

        // we have some scripts so read config & decide on that basis
        // Setup flags: m_runMacroSetting, m_warning, m_confirmationRequired,
        readConfiguration();
    }
    catch ( RuntimeException & rte )
    {
        OSL_TRACE( "ScriptSecurityManager::addScriptStorage: caught RuntimeException: %s",
            ::rtl::OUStringToOString( rte.Message,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer);
        throw RuntimeException(
            OUSTR( "ScriptSecurityManager::addScriptStorage: caught RuntimeException" ).concat( rte.Message ),
        Reference< XInterface >() );
    }

    switch( m_runMacroSetting )
    {
        case PERMISSION_NEVER:         // never
        {
            OSL_TRACE("never run");
            break;
        }
        case PERMISSION_PATHLIST:         // according to path list
        {
            OSL_TRACE("according to path");
            // check path
            rtl::OUString path = scriptStorageURL.copy( 0, scriptStorageURL.lastIndexOf( '/' ) );
            OSL_TRACE( "no of elts in path list = %d",
                (int)m_secureURL.getLength() );
            bool match = isSecureURL( path );
            if( match &&  ( m_warning == sal_True ) )
            {
                OSL_TRACE("path match & warning dialog");
                int result = (int)executeStandardDialog();
                OSL_TRACE("result = %d", (int)result);
                if ( (result&ALLOW_RUN) == ALLOW_RUN )
                {
                    newPerm.execPermission=sal_True;
                }
                break;
            }
            else if ( match )
            {
                OSL_TRACE("path match & no warning dialog");
                newPerm.execPermission=sal_True;
                break;
            }
            else if( m_confirmationRequired == sal_True )
            {
                OSL_TRACE("no path match & confirmation dialog");
                int result = (int)executePathDialog( path );
                OSL_TRACE("result = %d", (int)result);
                if ( (result&ALLOW_RUN) == ALLOW_RUN )
                {
                    newPerm.execPermission=sal_True;
                }
                if ( (result&ADD_TO_PATH) == ADD_TO_PATH )
                {
                    /* if checkbox clicked then need to add path to registry*/
                    addToSecurePaths(path);
                }
            }
            break;
        }
        case PERMISSION_ALWAYS:         // always
            if( m_warning == sal_True )
            {
                OSL_TRACE("always & warning dialog");
                short result = executeStandardDialog();
                if ( (result&ALLOW_RUN) == ALLOW_RUN )
                {
                    newPerm.execPermission=sal_True;
                }
            }
            else
            {
                OSL_TRACE("always & no warning dialog");
                newPerm.execPermission=sal_True;
            }
            break;
        default:
                //
                throw RuntimeException(
                    OUSTR( "ScriptSecurityManager::addScriptStorage got invalid OfficeBasic setting"),
                    Reference< XInterface > ());
    }

    if ( newPerm.execPermission == sal_True )
    {
        OSL_TRACE("setting exec permission to true for %s",
            ::rtl::OUStringToOString( scriptStorageURL,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    }
    else
    {
        OSL_TRACE("setting exec permission to false for %s",
            ::rtl::OUStringToOString( scriptStorageURL,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    }

    m_permissionSettings[ scriptStorageURL ] = newPerm;
}

bool ScriptSecurityManager::isSecureURL( const OUString & path )
{
    bool match = false;
    OSL_TRACE( "no of elts in path list = %d",
        (int)m_secureURL.getLength() );
    OSL_TRACE("document path: %s",
        ::rtl::OUStringToOString( path,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer);
    int length = m_secureURL.getLength();
    for( int j = 0; j < length ; j++ )
    {
        OSL_TRACE("path list element: %s",
            ::rtl::OUStringToOString( m_secureURL[j],
                RTL_TEXTENCODING_ASCII_US ).pData->buffer);
#ifdef WIN32
        OSL_TRACE("case insensitive comparison");
        if( path.equalsIgnoreAsciiCase( m_secureURL[j] ) )
#else
        OSL_TRACE("case sensitive comparison");
        if( path.equals( m_secureURL[j] ) )
#endif
        {
            match = true;
            break;
        }
    }
    return match;
}

short ScriptSecurityManager::executeStandardDialog()
throw ( RuntimeException )
{
    OUString dummyString;
    return executeDialog( dummyString );
}

short ScriptSecurityManager::executePathDialog( const OUString & path )
throw ( RuntimeException )
{
    return executeDialog( path );
}

short ScriptSecurityManager::executeDialog( const OUString & path )
throw ( RuntimeException )
{
    Sequence < Any > aArgs;
    if( path.getLength() != 0 )
    {
        OSL_TRACE("reallocing");
        aArgs.realloc(1);
        aArgs[ 0 ] <<= path;
    }
    short result;
    try
    {
        Reference< lang::XMultiComponentFactory > xMgr = m_xContext->getServiceManager();
        validateXRef( xMgr,
            "ScriptSecurityManager::executeDialog: cannot get ServiceManager" );
        Reference< XInterface > xInterface =
            xMgr->createInstanceWithArgumentsAndContext( s_securityDialog,
                aArgs, m_xContext );
        validateXRef( xInterface, "ScriptSecurityManager::executeDialog: Can't create SecurityDialog" );
        Reference< awt::XDialog > xDialog( xInterface, UNO_QUERY_THROW );
        result = xDialog->execute();
        Reference< lang::XComponent > xComponent( xInterface, UNO_QUERY_THROW );
        validateXRef( xInterface, "ScriptSecurityManager::executeDialog: Can't get XComponent to dispose dialog" );
        xComponent->dispose();
    }
    catch ( RuntimeException & rte )
    {
        throw RuntimeException(
            OUSTR( "ScriptSecurityManager::executeDialog: caught RuntimeException: ").concat( rte.Message ),
            Reference< XInterface > ());
    }
    catch ( Exception & e )
    {
        throw RuntimeException(
            OUSTR( "ScriptSecurityManager::executeDialog: caught Exception: ").concat( e.Message ),
            Reference< XInterface > ());
    }
    return result;
}

/**
 * checks to see whether the requested ScriptPermission is allowed.
 * This was modelled after the Java AccessController, but at this time
 * we can't see a good reason not to return a bool, rather than throw
 * an exception if the request is not granted (as is the case in Java).
 */
void ScriptSecurityManager::checkPermission( const OUString & scriptStorageURL,
    const OUString & permissionRequest )
    throw ( RuntimeException, lang::IllegalArgumentException, security::AccessControlException )
{
    if( permissionRequest.equals( OUString::createFromAscii( "execute" ) ) )
    {
        OSL_TRACE(
            "ScriptSecurityManager::checkPermission: execute permission request for %s",
            ::rtl::OUStringToOString( scriptStorageURL,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer);
        Permission_Hash::const_iterator ph_it = m_permissionSettings.find( scriptStorageURL );
        Permission_Hash::const_iterator ph_itend =
            m_permissionSettings.end();
        if ( ph_it != ph_itend )
        {
            if ( ph_it->second.execPermission )
            {
                return;
            }
            else
            {
                OSL_TRACE( "permission refused" );
                Any aPermission;
                security::RuntimePermission permission;
                permission.Name = OUString::createFromAscii( "execute" ).concat( scriptStorageURL );
                aPermission <<= permission;
                throw security::AccessControlException(
                    OUString::createFromAscii( "ScriptSecurityManager::checkPermission: no execute permission for URL" ).concat( scriptStorageURL ),
                    Reference< XInterface > (), aPermission );
            }
        }
        // we should never get here!!
        throw lang::IllegalArgumentException( OUString::createFromAscii( "ScriptSecurityManager::checkPermission: storageURL not found" ), Reference< XInterface > (), 0 );
    }
    // inappropriate permission request
    throw lang::IllegalArgumentException( OUString::createFromAscii( "ScriptSecurityManager::checkPermission: storageURL not found" ), Reference< XInterface > (), 1 );
}

void ScriptSecurityManager::removePermissionSettings ( ::rtl::OUString & scriptStorageURL )
{
    Permission_Hash::const_iterator ph_it =
        m_permissionSettings.find( scriptStorageURL );

    if ( ph_it == m_permissionSettings.end() )
    {
        OSL_TRACE( "Entry for storage url %s doesn't exist in map",
            ::rtl::OUStringToOString( scriptStorageURL,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer);
        return;
    }

    // erase the entry from the hash
    m_permissionSettings.erase( scriptStorageURL );

}

void ScriptSecurityManager::readConfiguration()
    throw ( RuntimeException)
{
    Reference< XInterface > xInterface;
    try
    {
    beans::PropertyValue configPath;
    configPath.Name = ::rtl::OUString::createFromAscii( "nodepath" );
    configPath.Value <<= ::rtl::OUString::createFromAscii( "org.openoffice.Office.Common/Security/Scripting" );
    Sequence < Any > aargs( 1 );
    aargs[ 0 ] <<= configPath;
    validateXRef( m_xConfigProvFactory,
        "ScriptSecurityManager::readConfiguration: ConfigProviderFactory no longer valid!" );
    xInterface = m_xConfigProvFactory->createInstanceWithArguments( s_configAccess,
            aargs );
    validateXRef( xInterface,
        "ScriptSecurityManager::readConfiguration: cannot get ConfigurationAccess" );
    // get the XPropertySet interface from the ConfigurationAccess service
    Reference < beans::XPropertySet > xPropSet( xInterface, UNO_QUERY );
    Any value;

        value=xPropSet->getPropertyValue( OUSTR( "Confirmation" ) );
        if ( sal_False == ( value >>= m_confirmationRequired ) )
        {
            throw RuntimeException(
                OUSTR( "ScriptSecurityManager:readConfiguration: can't get Confirmation setting" ),
                Reference< XInterface > () );
        }
        if ( m_confirmationRequired == sal_True )
        {
            OSL_TRACE( "ScriptSecurityManager:readConfiguration: confirmation is true" );
        }
        else
        {
            OSL_TRACE( "ScriptSecurityManager:readConfiguration: confirmation is false" );
        }
        value=xPropSet->getPropertyValue( OUSTR( "Warning" ) );
        if ( sal_False == ( value >>= m_warning ) )
        {
            throw RuntimeException(
                OUSTR( "ScriptSecurityManager:readConfiguration: can't get Warning setting" ),
                Reference< XInterface > () );
        }
        if ( m_warning == sal_True )
        {
            OSL_TRACE( "ScriptSecurityManager:readConfiguration: warning is true" );
        }
        else
        {
            OSL_TRACE( "ScriptSecurityManager:readConfiguration: warning is false" );
        }
        value=xPropSet->getPropertyValue( OUSTR( "OfficeBasic" ) );
        if ( sal_False == ( value >>= m_runMacroSetting ) )
        {
            throw RuntimeException(
                OUSTR( "ScriptSecurityManager:readConfiguration: can't get OfficeBasic setting" ),
                Reference< XInterface > () );
        }
        OSL_TRACE( "ScriptSecurityManager:readConfiguration: OfficeBasic = %d", m_runMacroSetting );
        value=xPropSet->getPropertyValue( OUSTR( "SecureURL" ) );
        if ( sal_False == ( value >>= m_secureURL ) )
        {
            throw RuntimeException(
                OUSTR( "ScriptSecurityManager:readConfiguration: can't get SecureURL setting" ),
                Reference< XInterface > () );
        }
    }
    catch ( beans::UnknownPropertyException & upe )
    {
        throw RuntimeException(
            OUSTR( "ScriptSecurityManager:readConfiguration: Attempt to read unknown property: " ).concat( upe.Message ),
            Reference< XInterface > () );
    }
    catch ( lang::WrappedTargetException & wte )
    {
        throw RuntimeException(
            OUSTR( "ScriptSecurityManager:readConfiguration: wrapped target exception? :" ).concat( wte.Message ),
            Reference< XInterface > () );
    }
    catch ( Exception & e )
    {
        OSL_TRACE( "Unknown exception in readconf: %s",
            ::rtl::OUStringToOString(e.Message ,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer  );
        throw RuntimeException(
            OUSTR( "ScriptSecurityManager:readConfiguration: exception? :" ).concat( e.Message ),
            Reference< XInterface > () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        OSL_TRACE( "Completely Unknown exception in readconf!!!!!!");
        throw RuntimeException(
            OUSTR( "ScriptSecurityManager:readConfiguration: exception? :" ),
            Reference< XInterface > () );
    }
#endif

    int length = m_secureURL.getLength();

    // PathSubstitution needed to interpret variables found in config
    Reference< lang::XMultiComponentFactory > xMgr = m_xContext->getServiceManager();
    validateXRef( xMgr,
        "ScriptSecurityManager::readConfiguration: cannot get XMultiComponentFactory" );
    xInterface = xMgr->createInstanceWithContext(
        ::rtl::OUString::createFromAscii(
        "com.sun.star.util.PathSubstitution"), m_xContext);
    validateXRef( xInterface,
        "ScriptSecurityManager::readConfiguration: cannot get ConfigurationProvider" );
    Reference< util::XStringSubstitution > xStringSubstitution(
        xInterface, UNO_QUERY);
    validateXRef( xStringSubstitution,
        "ScriptSecurityManager::readConfiguration: cannot get ConfigurationProvider" );
    for( int i = 0; i < length; i++ )
    {
        OSL_TRACE( "ScriptSecurityManager:readConfiguration path = %s",
            ::rtl::OUStringToOString(m_secureURL[i] ,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer  );

        OSL_TRACE( "ScriptSecurityManager: subpath = %s",
            ::rtl::OUStringToOString(
            xStringSubstitution->substituteVariables( m_secureURL[i], true ),
            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        m_secureURL[i] = xStringSubstitution->substituteVariables( m_secureURL[i], true );
    }
#ifdef _DEBUG
    int length2 = m_secureURL.getLength();
    for( int j = 0; j < length2 ; j++ )
    {
        OSL_TRACE( "ScriptSecurityManager: path = %s",
            ::rtl::OUStringToOString(m_secureURL[j] ,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer  );
    }
#endif
}

void ScriptSecurityManager::addToSecurePaths( const OUString & path )
throw ( RuntimeException )
{
    OSL_TRACE( "--->ScriptSecurityManager::addToSecurePaths" );
    beans::PropertyValue configPath;
    configPath.Name = ::rtl::OUString::createFromAscii( "nodepath" );
    configPath.Value <<= ::rtl::OUString::createFromAscii( "org.openoffice.Office.Common/Security/Scripting" );
    Sequence < Any > aargs( 1 );
    aargs[ 0 ] <<= configPath;
    Reference< XInterface > xInterface = m_xConfigProvFactory->createInstanceWithArguments( s_configUpdate,
            aargs );
    validateXRef( xInterface,
        "ScriptSecurityManager::addToSecurePaths: ScriptSecurityManager: cannot get ConfigurationUpdateAccess" );
    Reference < container::XNameReplace > xNameReplace( xInterface, UNO_QUERY );
    validateXRef( xNameReplace,
        "ScriptSecurityManager::addToSecurePaths: ScriptSecurityManager: cannot get XNameReplace" );
    Reference < util::XChangesBatch > xChangesBatch( xInterface, UNO_QUERY );
    validateXRef( xChangesBatch,
        "ScriptSecurityManager::addToSecurePaths: cannot get XChangesBatch" );

    OSL_TRACE( "--->ScriptSecurityManager::addToSecurePaths: after if stuff" );
    Reference < beans::XPropertySet > xPropSet( xInterface, UNO_QUERY );
    css::uno::Sequence< rtl::OUString > newSecureURL;
    Any value;
    OUString pathListPropName = OUSTR ( "SecureURL" );
    value=xPropSet->getPropertyValue( pathListPropName );
    if ( sal_False == ( value >>= newSecureURL ) )
    {
        throw RuntimeException(
            OUSTR( "ScriptSecurityManager::addToSecurePaths: can't get SecureURL setting" ),
            Reference< XInterface > () );
    }
    try
    {
        sal_Int32 length = newSecureURL.getLength();
        newSecureURL.realloc( length + 1 );
        newSecureURL[ length ] = path;
        Any aNewSecureURL;
        aNewSecureURL <<= newSecureURL;
        xNameReplace->replaceByName( pathListPropName, aNewSecureURL );
        xChangesBatch->commitChanges();
        m_secureURL = newSecureURL;
    }
    catch ( Exception & e )
    {
        OSL_TRACE( "Error updating secure paths: " );
        throw RuntimeException(
            OUSTR( "ScriptSecurityManager::addToSecurePaths: error updating SecureURL setting" ).concat( e.Message ),
            Reference< XInterface > () );
    }
}

//*************************************************************************
// ScriptSecurityManager Destructor
ScriptSecurityManager::~ScriptSecurityManager()
{
    OSL_TRACE( "< ScriptSecurityManager dtor called >\n" );
}

} // Namespace
