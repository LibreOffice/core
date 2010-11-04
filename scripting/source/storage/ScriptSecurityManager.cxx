/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_scripting.hxx"

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
#include <tools/diagnose_ex.h>

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
    : m_xContext( xContext, UNO_SET_THROW )
{
    OSL_TRACE( "< ScriptSecurityManager ctor called >\n" );

    // get the service manager from the context
    Reference< lang::XMultiComponentFactory > xMgr( m_xContext->getServiceManager(), UNO_SET_THROW );

    // create an instance of the ConfigurationProvider
    m_xConfigProvFactory.set( xMgr->createInstanceWithContext( s_configProv, m_xContext ), UNO_QUERY_THROW );
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
        Reference< lang::XMultiComponentFactory > xMgr( m_xContext->getServiceManager(), UNO_SET_THROW );
        Reference< awt::XDialog > xDialog(
            xMgr->createInstanceWithArgumentsAndContext( s_securityDialog, aArgs, m_xContext ),
            UNO_QUERY_THROW );
        result = xDialog->execute();
        Reference< lang::XComponent > xComponent( xDialog, UNO_QUERY_THROW );
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
    try
    {
        beans::PropertyValue configPath;
        configPath.Name = ::rtl::OUString::createFromAscii( "nodepath" );
        configPath.Value <<= ::rtl::OUString::createFromAscii( "org.openoffice.Office.Common/Security/Scripting" );
        Sequence < Any > aargs( 1 );
        aargs[ 0 ] <<= configPath;
        ENSURE_OR_THROW( m_xConfigProvFactory.is(),
            "ScriptSecurityManager::readConfiguration: ConfigProviderFactory no longer valid!" );
        // get the XPropertySet interface from the ConfigurationAccess service
        Reference < beans::XPropertySet > xPropSet( m_xConfigProvFactory->createInstanceWithArguments( s_configAccess, aargs ), UNO_QUERY_THROW );

        m_confirmationRequired = sal_True;
        OSL_VERIFY( xPropSet->getPropertyValue( OUSTR( "Confirmation" ) ) >>= m_confirmationRequired );
        if ( m_confirmationRequired == sal_True )
        {
            OSL_TRACE( "ScriptSecurityManager:readConfiguration: confirmation is true" );
        }
        else
        {
            OSL_TRACE( "ScriptSecurityManager:readConfiguration: confirmation is false" );
        }

        m_warning = true;
        OSL_VERIFY( xPropSet->getPropertyValue( OUSTR( "Warning" ) ) >>= m_warning );

        if ( m_warning == sal_True )
        {
            OSL_TRACE( "ScriptSecurityManager:readConfiguration: warning is true" );
        }
        else
        {
            OSL_TRACE( "ScriptSecurityManager:readConfiguration: warning is false" );
        }

        m_runMacroSetting = sal_True;
        OSL_VERIFY( xPropSet->getPropertyValue( OUSTR( "OfficeBasic" ) ) >>= m_runMacroSetting );
        OSL_TRACE( "ScriptSecurityManager:readConfiguration: OfficeBasic = %d", m_runMacroSetting );

        m_secureURL = ::rtl::OUString();
        OSL_VERIFY( xPropSet->getPropertyValue( OUSTR( "SecureURL" ) ) >>= m_secureURL );
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
    Reference< lang::XMultiComponentFactory > xMgr( m_xContext->getServiceManager(), UNO_SET_THROW );
    Reference< XInterface > xInterface = );
    Reference< util::XStringSubstitution > xStringSubstitution(
        xMgr->createInstanceWithContext(
            ::rtl::OUString::createFromAscii( "com.sun.star.util.PathSubstitution" ), m_xContext
        ),
        UNO_QUERY_THROW
    );
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
    Reference < container::XNameReplace > xNameReplace(
        m_xConfigProvFactory->createInstanceWithArguments( s_configUpdate, aargs ), UNO_QUERY_THROW );
    Reference < util::XChangesBatch > xChangesBatch( xNameReplace, UNO_QUERY_THROW );

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
