/*************************************************************************
 *
 *  $RCSfile: ScriptNameResolverImpl.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: dfoster $ $Date: 2003-07-23 14:05:13 $
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

#include <vector>
#include <stdlib.h>

#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/security/AccessControlException.hpp>

#include <util/util.hxx>
#include <util/scriptingconstants.hxx>

#include <drafts/com/sun/star/script/framework/storage/XScriptStorageManager.hpp>
#include <drafts/com/sun/star/script/framework/security/XScriptSecurity.hpp>

#include "ScriptNameResolverImpl.hxx"
#include "ScriptRuntimeManager.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

namespace scripting_runtimemgr
{

const sal_Char* const LANGUAGE_TO_RESOLVE_ON[] = { "All" }; // should be configurable
OUString nrs_implName = OUString::createFromAscii(
    "drafts.com.sun.star.script.framework.runtime.DefaultScriptNameResolver" );
OUString nrs_serviceName = OUString::createFromAscii(
    "drafts.com.sun.star.script.framework.runtime.DefaultScriptNameResolver" );
Sequence< OUString > nrs_serviceNames = Sequence< OUString >( &nrs_serviceName, 1 );

const char* const SCRIPTSTORAGEMANAGER_SERVICE =
    "/singletons/drafts.com.sun.star.script.framework.storage.theScriptStorageManager";

extern ::rtl_StandardModuleCount s_moduleCount;

// define storages to search
static ::std::vector< sal_Int32 >* m_pSearchIDs = NULL;

//*************************************************************************
ScriptNameResolverImpl::ScriptNameResolverImpl(
    const Reference< XComponentContext > & xContext ) :
    m_xContext( xContext )
{
    OSL_TRACE( "< ScriptNameResolverImpl ctor called >\n" );
    validateXRef( m_xContext, "ScriptNameResolverImpl::ScriptNameResolverImpl: invalid context" );
    m_xMultiComFac = m_xContext->getServiceManager();

    validateXRef( m_xMultiComFac, "ScriptNameResolverImpl::ScriptNameResolverImpl: invalid XMultiComponentFactory " );

    if( !m_pSearchIDs )
    {
        osl::Guard< osl::Mutex > aGuard( m_mutex );
        if( !m_pSearchIDs )
        {
            scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
                scripting_constants::ScriptingConstantsPool::instance();
            m_pSearchIDs = new ::std::vector< sal_Int32 >();
            m_pSearchIDs->push_back( scriptingConstantsPool.DOC_STORAGE_ID_NOT_SET );
            m_pSearchIDs->push_back( scriptingConstantsPool.USER_STORAGE_ID );
            m_pSearchIDs->push_back( scriptingConstantsPool.SHARED_STORAGE_ID );
        }
    }

    s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );
}

//*************************************************************************
ScriptNameResolverImpl::~ScriptNameResolverImpl()
{
    OSL_TRACE( "< ScriptNameResolverImpl dtor called >\n" );
    s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
}

//*************************************************************************
Reference< storage::XScriptInfo > ScriptNameResolverImpl::resolve(
const ::rtl::OUString & scriptURI, Any& invocationCtx )
throw ( lang::IllegalArgumentException, script::CannotConvertException, RuntimeException )
{

    Reference< storage::XScriptInfo > resolvedName;
    Reference< beans::XPropertySet > xPropSetScriptingContext;
    scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
            scripting_constants::ScriptingConstantsPool::instance();

    OSL_TRACE( "ScriptNameResolverImpl::resolve: in resolve - start" );

    if ( sal_False == ( invocationCtx >>= xPropSetScriptingContext ) )
    {
        throw RuntimeException( OUSTR(
            "ScriptNameResolverImpl::resolve : unable to get XScriptingContext from param" ),
            Reference< XInterface > () );
    }

    Any any;
    OUString docUri;
    sal_Int32 filesysScriptStorageID = -1;
    Reference < storage::XScriptStorageManager > xScriptStorageMgr;
    sal_Int32 docSid;
    try
    {
        any = xPropSetScriptingContext->getPropertyValue(
            scriptingConstantsPool.DOC_URI );
        OSL_TRACE( "ScriptNameResolverImpl::resolve: in resolve - got anyUri" );
        if ( sal_False == ( any >>= docUri ) )
        {
            throw RuntimeException( OUSTR(
            "ScriptNameResolverImpl::resolve : unable to get doc Uri from xPropSetScriptingContext" ),
                Reference< XInterface > () );
        }
        any = xPropSetScriptingContext->getPropertyValue(
            scriptingConstantsPool.DOC_STORAGE_ID );
        if ( sal_False == ( any >>= docSid ) )
        {
            throw RuntimeException( OUSTR(
                "ScriptNameResolverImpl::resolve : unable to get doc storage id from xPropSetScriptingContext" ),
                Reference< XInterface > () );
        }
    }
    catch ( Exception & e )
    {
        OUString temp = OUSTR(
            "ScriptNameResolverImpl::resolve : problem with getPropertyValue" );
        throw RuntimeException( temp.concat( e.Message ),
                                Reference< XInterface > () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        throw RuntimeException( OUSTR(
            "ScriptNameResolverImpl::resolve Unknown Exception caught - RuntimeException rethrown" ),
            Reference< XInterface > () );
    }
#endif


    ::rtl::OString docUriO(
        ::rtl::OUStringToOString( docUri , RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE(
        "ScriptNameResolverImpl::resolve: *** >>> DOC URI: %s, doc sid is %d\n",
        docUriO.pData->buffer, docSid );


    OSL_TRACE( "ScriptNameResolverImpl::resolve Starting..." );
    OUString docString = OUString::createFromAscii( "location=document" );
    OUString userString = OUString::createFromAscii( "location=user" );
    OUString shareString = OUString::createFromAscii( "location=share" );
    OUString filesysString = OUString::createFromAscii( "location=filesystem" );

    // initialise vector with doc, user and share

    // m_pSearchIDs is initialised as follows,
    // m_pSearchIDs [ 0 ] empty
    // m_pSearchIDs [ 1 ] user storage id
    // m_pSearchIDs [ 2 ] share "      "

    ::std::vector< sal_Int32 > m_vSearchIDs = *m_pSearchIDs;
    m_vSearchIDs[ 0 ] = docSid;

    if ( scriptURI.indexOf( docString ) != -1 )
    {
        OSL_TRACE("Full resolution available, search document");
        // search in document
        m_vSearchIDs.resize( 1 );
    }
    else if ( scriptURI.indexOf( userString ) != -1 )
    {
        OSL_TRACE("Full resolution available, search user");
        // search in user
        m_vSearchIDs[ 0 ] = ( *m_pSearchIDs )[ 1 ];
        m_vSearchIDs.resize( 1 );
    }
    else if ( scriptURI.indexOf( shareString ) != -1 )
    {
        OSL_TRACE("Full resolution available, search share");
        // search in share
        m_vSearchIDs[ 0 ] = ( *m_pSearchIDs )[ 2 ];
        m_vSearchIDs.resize( 1 );
    }
    else if ( scriptURI.indexOf( filesysString ) != -1 )
    {
        OSL_TRACE("Full resolution available, create & search filesystem");
        OUString filesysURL;
        try
        {
            filesysURL = getFilesysURL( scriptURI );
        }
        catch ( lang::IllegalArgumentException & e )
        {
            OUString temp = OUSTR( "ScriptNameResolverImpl::resolve: " );
            throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
        }
        Reference< XInterface > xInterface = m_xMultiComFac->createInstanceWithContext(
            ::rtl::OUString::createFromAscii(
                    "com.sun.star.ucb.SimpleFileAccess" ), m_xContext );
        validateXRef( xInterface,
            "ScriptProvider::initialise: cannot get SimpleFileAccess Service\n" );
        Reference < ucb::XSimpleFileAccess > xSimpleFileAccess = Reference <
                    ucb::XSimpleFileAccess > ( xInterface, UNO_QUERY_THROW );

        // do we need to encode this? hope not.
        OSL_TRACE( ">>>> About to create storage for %s",
                ::rtl::OUStringToOString( filesysURL,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        // ask storage manager to create storage
        try
        {
            // need to get the ScriptStorageManager
            Any a = m_xContext->getValueByName(
                    scriptingConstantsPool.SCRIPTSTORAGEMANAGER_SERVICE );
            if ( sal_False == ( a >>= xScriptStorageMgr ) )
            {
                OUString temp = OUSTR( "ScriptNameResolverImpl::resolve: failed to get ScriptStorageManager" );
                throw RuntimeException( temp, Reference< XInterface >() );
                // need to throw
            }
            validateXRef( xScriptStorageMgr, "Cannot get ScriptStorageManager" );
            filesysScriptStorageID =
                    xScriptStorageMgr->createScriptStorageWithURI(
                        xSimpleFileAccess, filesysURL );
                OSL_TRACE( ">>>> Created storage %d - for %s ",
                    filesysScriptStorageID, ::rtl::OUStringToOString(
                        filesysURL, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
        catch ( RuntimeException & e )
        {
            OUString temp = OUSTR( "ScriptNameResolverImpl::resolve: " );
            throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
        }
        m_vSearchIDs[ 0 ] = filesysScriptStorageID;
        m_vSearchIDs.resize( 1 );
    }
    else
    {
        OSL_TRACE("Only partial uri available, search doc, user & share");
        // is this illegal or do we search in a default way
        // if we get to here a uri has been passed in that has:
        // a) not got a location specified
        // b) an illegal location

        // detect illegal location
        if (  scriptURI.indexOf( OUString::createFromAscii( "location=" ) ) != -1 )
        {
            OSL_TRACE(
                "ScriptNameResolver::resolve, throwing IllegalArgException" );
            throw lang::IllegalArgumentException(
                OUSTR( "invalid URI: " ).concat( scriptURI ),
                Reference < XInterface > (), 1 );

        }
        // leave vSearchIDs take care of the search...
    }

    ::std::vector< sal_Int32 >::const_iterator iter;
    ::std::vector< sal_Int32 >::const_iterator iterEnd = m_vSearchIDs.end();

    for ( iter = m_vSearchIDs.begin() ; iter != iterEnd; ++iter )
    {
        try
        {
            OSL_TRACE( "** about to resolve from storage using id %d from vector of size %d",
                *iter, m_vSearchIDs.size() );
            if ( ( resolvedName = resolveURIFromStorageID( *iter, docUri, scriptURI ) ).is() )
            {
                OSL_TRACE( "found match in uri from storage %d", *iter );
                xPropSetScriptingContext->setPropertyValue(
                scriptingConstantsPool.RESOLVED_STORAGE_ID, makeAny(*iter) );
                break;
            }

        }
        catch ( css::security::AccessControlException  & e )
        {
            // no execute permission
            OSL_TRACE( "ScriptNameResolverImpl::resolve : AccessControlException " );
            continue;
        }
        catch ( beans::UnknownPropertyException & e )
        {
            OUString temp = OUSTR(
                "ScriptNameResolverImpl::resolve : UnknownPropertyException" );
            throw RuntimeException( temp.concat( e.Message ),
                Reference< XInterface > () );
        }
        catch ( beans::PropertyVetoException  & e )
        {
            OUString temp = OUSTR(
                "ScriptNameResolverImpl::resolve : PropertyVetoException " );
            throw RuntimeException( temp.concat( e.Message ),
                Reference< XInterface > () );
        }
        catch ( lang::IllegalArgumentException  & e )
        {
            OUString temp = OUSTR(
                "ScriptNameResolverImpl::resolve : IllegalArgumentException " );
            throw lang::IllegalArgumentException( temp.concat( e.Message ),
                Reference< XInterface > (), e.ArgumentPosition );
        }
        catch ( lang::WrappedTargetException & e )
        {
        OUString temp = OUSTR(
                "ScriptNameResolverImpl::resolve : WrappedTargetException " );
            throw RuntimeException( temp.concat( e.Message ),
                Reference< XInterface > () );
        }
        catch ( Exception & e )
        {
            OSL_TRACE(
                "Exception thrown by storage %d, failed to match uri: %s",
                 *iter,
                 ::rtl::OUStringToOString( e.Message,
                 RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        OUString temp = OUSTR(
                "ScriptNameResolverImpl::resolve : unknown exception" );
            throw RuntimeException( temp.concat( e.Message ),
                Reference< XInterface > () );
        }
#ifdef _DEBUG
        catch ( ... )
        {
            OSL_TRACE(
                "unknown exception thrown by storage %d, failed to match uri",
                *iter );
        OUString temp = OUSTR(
                "ScriptNameResolverImpl::resolve Unknown exception caught - RuntimeException rethrown" );
            throw RuntimeException( temp,
                Reference< XInterface > () );
        }
#endif

    }
    if ( !resolvedName.is() )
    {
        if( filesysScriptStorageID >  2 )
        {
            // get the filesys storage and dispose of it
            Reference< XInterface > xScriptStorage =
                xScriptStorageMgr->getScriptStorage( filesysScriptStorageID );
            validateXRef( xScriptStorage,
                          "ScriptNameResolverImpl::getStorageInstance: cannot get Script Storage service" );
            Reference< storage::XScriptInfoAccess > xScriptInfoAccess = Reference<
                storage::XScriptInfoAccess > ( xScriptStorage, UNO_QUERY_THROW );
            validateXRef( xScriptInfoAccess,
            "ScriptNameResolverImpl::resolveURIFromStorageID: cannot get XScriptInfoAccess" );
            Sequence< Reference< storage::XScriptInfo > > results =
                xScriptInfoAccess->getAllImplementations( );
            Reference < lang::XEventListener > xEL_ScriptStorageMgr =
                Reference< lang::XEventListener >
                    ( xScriptStorageMgr ,UNO_QUERY_THROW );
            validateXRef( xEL_ScriptStorageMgr, "ScriptNameResolverImpl::resolve: can't get ScriptStorageManager XEventListener interface when trying to dispose of filesystem storage" );
            lang::EventObject event( results[ 0 ] );
            xEL_ScriptStorageMgr->disposing( event );
        }
        throw lang::IllegalArgumentException( OUSTR(
            "ScriptNameResolverImpl::resolve: no script found for uri=" ).concat( scriptURI ),
            Reference< XInterface > (), 0 );
    }
    return resolvedName;
}

//*************************************************************************
OUString SAL_CALL
ScriptNameResolverImpl::getImplementationName( )
throw( RuntimeException )
{
    return nrs_implName;
}

//*************************************************************************
sal_Bool SAL_CALL
ScriptNameResolverImpl::supportsService( const OUString& serviceName )
throw( RuntimeException )
{
    OUString const * pNames = nrs_serviceNames.getConstArray();
    for ( sal_Int32 nPos = nrs_serviceNames.getLength(); nPos--; )
    {
        if ( serviceName.equals( pNames[ nPos ] ) )
        {
            return sal_True;
        }
    }
    return sal_False;
}

//*************************************************************************

Reference< storage::XScriptInfo >
ScriptNameResolverImpl::resolveURIFromStorageID
( sal_Int32 sid, const ::rtl::OUString & docURI,
  const ::rtl::OUString& scriptURI )
SAL_THROW ( ( lang::IllegalArgumentException, css::security::AccessControlException, RuntimeException ) )
{
    Reference< storage::XScriptInfo > resolvedScriptInfo;
    scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
        scripting_constants::ScriptingConstantsPool::instance();
    if ( sid == scriptingConstantsPool.DOC_STORAGE_ID_NOT_SET )
    {
        OSL_TRACE( "@@@@ **** ScriptNameResolverImpl::resolve DOC_STORAGE_ID_NOT_SET" );
        return resolvedScriptInfo;
    }
    try
    {
        OUString permissionURI = docURI;
        OUString filesysString = OUString::createFromAscii( "location=filesystem" );
        if ( scriptURI.indexOf( filesysString ) != -1 )
        {
            // in the case of filesys scripts we're checking whether the
            // location of the script, rather than the location of the document,
            // has execute permission
            try
            {
                permissionURI = getFilesysURL( scriptURI );
            }
            catch ( lang::IllegalArgumentException & e )
            {
                OUString temp = OUSTR( "ScriptNameResolverImpl::resolveFromURI: " );
                throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
            }
        }
        Reference< storage::XScriptInfoAccess > storage = getStorageInstance( sid, permissionURI );
        validateXRef( storage,
        "ScriptNameResolverImpl::resolveURIFromStorageID: cannot get XScriptInfoAccess" );
        Sequence< Reference< storage::XScriptInfo > > results =
            storage->getImplementations( scriptURI );

        const sal_Int32 length = results.getLength();

        if ( !length )
        {
            return resolvedScriptInfo;
        }

        OSL_TRACE( "ScriptNameResolverImpl::resolve Got some results..." );
        // if we get results, just return first in list,
        // storage has already matched language, function name etc. if
        // that information was in the uri
        resolvedScriptInfo = results[ 0 ];
    }
    catch ( css::security::AccessControlException & ace )
    {
        OUString temp = OUSTR(
            "ScriptRuntimeManager::resolveURIFromStorageID AccessControlException: " );
        throw css::security::AccessControlException( temp.concat( ace.Message ),
                                              Reference< XInterface > (),
                                                ace.LackingPermission );
    }
    catch ( lang::IllegalArgumentException & iae )
    {
        OUString temp = OUSTR(
            "ScriptRuntimeManager::resolveURIFromStorageID IllegalArgumentException: " );
        throw lang::IllegalArgumentException( temp.concat( iae.Message ),
                                              Reference< XInterface > (),
                                              iae.ArgumentPosition );
    }
    catch ( RuntimeException & re )
    {
        OUString temp = OUSTR(
            "ScriptRuntimeManager::resolveURIFromStorageID RuntimeException: " );
        throw RuntimeException( temp.concat( re.Message ),
                                Reference< XInterface > () );
    }
    catch ( Exception & e )
    {
        OUString temp = OUSTR(
            "ScriptNameResolverImpl::resolveURIFromStorageID : Exception caught - RuntimeException rethrown" );
        throw RuntimeException( temp.concat( e.Message ),
                                Reference< XInterface > () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        throw RuntimeException( OUSTR(
            "ScriptNameResolverImpl::resolveURIFromStorageID Unknown exception caught - RuntimeException rethrown" ),
            Reference< XInterface > () );
    }
#endif
    return resolvedScriptInfo;
}
//*************************************************************************

Reference< storage::XScriptInfoAccess >

ScriptNameResolverImpl::getStorageInstance( sal_Int32 sid,
const ::rtl::OUString & permissionURI ) SAL_THROW ( ( RuntimeException, css::security::AccessControlException, lang::IllegalArgumentException ) )
{
    Reference< storage::XScriptInfoAccess > xScriptInfoAccess;
    try
    {
        Reference< XInterface > xInterface;

        Any a = m_xContext->getValueByName(
                    OUString::createFromAscii( SCRIPTSTORAGEMANAGER_SERVICE ) );
        if ( sal_False == ( a >>= xInterface ) )
        {
            throw RuntimeException(
                OUSTR( "ScriptNameResolverImpl::getStorageInstance: could not obtain ScriptStorageManager singleton" ),
                Reference< XInterface >() );
        }
        validateXRef( xInterface,
                      "ScriptNameResolverImpl::getStorageInstance: cannot get Storage service" );
        // check that we have permissions for this storage
        Reference< dcsssf::security::XScriptSecurity > xScriptSecurity( xInterface, UNO_QUERY_THROW );
        validateXRef( xScriptSecurity,
                      "ScriptNameResolverImpl::getStorageInstance:  cannot get Script Security service" );
        scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
                scripting_constants::ScriptingConstantsPool::instance();
        // if we dealing with a document storage (ie. not user or share
        // we need to check the permission
        if( ( sid != scriptingConstantsPool.USER_STORAGE_ID ) &&
            ( sid != scriptingConstantsPool.SHARED_STORAGE_ID ) )
        {
            xScriptSecurity->checkPermission( permissionURI,
                OUString::createFromAscii( "execute" ) );
            // if we get here, the checkPermission hasn't thrown an
            // AccessControlException, ie. permission has been granted
            OSL_TRACE( "ScriptNameResolverImpl::getStorageInstance: got execute permission for ID=%d", sid );
        }
        Reference< storage::XScriptStorageManager > xScriptStorageManager( xInterface, UNO_QUERY_THROW );
        validateXRef( xScriptStorageManager,
                      "ScriptNameResolverImpl::getStorageInstance:  cannot get Script Storage Manager service" );
        Reference< XInterface > xScriptStorage =
            xScriptStorageManager->getScriptStorage( sid );
        validateXRef( xScriptStorage,
                      "ScriptNameResolverImpl::getStorageInstance: cannot get Script Storage service" );
        xScriptInfoAccess = Reference<
            storage::XScriptInfoAccess > ( xScriptStorage, UNO_QUERY_THROW );
    }
    catch ( lang::IllegalArgumentException & e )
    {
        OUString temp = OUSTR( "ScriptNameResolverImpl::getStorageInstance: " );
        throw lang::IllegalArgumentException( temp.concat( e.Message ),
            Reference< XInterface >(), e.ArgumentPosition );
    }
    catch ( css::security::AccessControlException & e )
    {
        OUString temp = OUSTR( "ScriptNameResolverImpl::getStorageInstance: AccessControlException " );
        throw css::security::AccessControlException( temp.concat( e.Message ), Reference< XInterface >(), e.LackingPermission );
    }
    catch ( RuntimeException & re )
    {
        OUString temp = OUSTR( "ScriptNameResolverImpl::getStorageInstance: " );
        throw RuntimeException( temp.concat( re.Message ), Reference< XInterface >() );
    }
    catch ( Exception & e )
    {
        OUString temp = OUSTR( "ScriptNameResolverImpl::getStorageInstance: " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
    return xScriptInfoAccess;
}
//*************************************************************************
OUString
ScriptNameResolverImpl::getFilesysURL( const OUString & scriptURI )
throw( lang::IllegalArgumentException )
{
        OUString filePath;
        OUString fileName;
        OUString filesysString = OUString::createFromAscii( "location=filesystem" );
        sal_Int32 locationPos = scriptURI.indexOf( filesysString );
        // expect location=filesys:file:///foo/bar/myscript.bsh etc
        // except the file url at this point is encoded
        // so we should be ok searching for the '&'
        sal_Int32 filesysStrLen = filesysString.getLength() + 1;
        sal_Int32 endOfLocn = scriptURI.indexOf( '&', locationPos );
        if (endOfLocn == -1 )
        {
                filePath = scriptURI.copy( locationPos + filesysString.getLength() + 1 );
        }
        else
        {
                filePath = scriptURI.copy( locationPos + filesysStrLen,
                                endOfLocn - locationPos - filesysStrLen );
        }
        //file name shoul also be encoded so again ok to search for '&'
        OUString functionKey = OUString::createFromAscii( "function=" );
        sal_Int32 functionKeyLength = functionKey.getLength();
        sal_Int32 functionNamePos = scriptURI.indexOf( functionKey );
        if ( functionNamePos > 0 )
        {
            sal_Int32 endOfFn = scriptURI.indexOf( '&', functionNamePos );
            if ( endOfFn == -1 )
            {
                fileName = scriptURI.copy( functionNamePos + functionKeyLength );
            }
            else
            {
                fileName = scriptURI.copy( functionNamePos + functionKeyLength,
                                endOfFn - functionNamePos - functionKeyLength );
            }
        }
        else
        {
            // we need to throw
            OUString temp = OUSTR( "ScriptNameResolverImpl::getFilesysURL: error getting the filesysURL" );
            throw lang::IllegalArgumentException( temp, Reference< XInterface >(), 0 );
        }
        filePath+=fileName;
        OSL_TRACE( "ScriptNameResolverImpl::getFilesysURL: filesys URL = %s",
                 ::rtl::OUStringToOString( filePath,
                 RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        return filePath;
}
//*************************************************************************
Sequence<OUString> SAL_CALL
ScriptNameResolverImpl::getSupportedServiceNames( )
throw( RuntimeException )
{
    return nrs_serviceNames;
}

//*************************************************************************
Reference< XInterface > SAL_CALL scriptnri_create(
    Reference< XComponentContext > const & xComponentContext )
SAL_THROW( ( Exception ) )
{
    return ( cppu::OWeakObject * ) new ScriptNameResolverImpl( xComponentContext );
}

//*************************************************************************
Sequence< OUString > scriptnri_getSupportedServiceNames() SAL_THROW( () )
{
    return nrs_serviceNames;
}

//*************************************************************************
OUString scriptnri_getImplementationName() SAL_THROW( () )
{
    return nrs_implName;
}
} // namespace scripting_runtimemgr
