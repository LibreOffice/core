/*************************************************************************
 *
 *  $RCSfile: ScriptNameResolverImpl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dfoster $ $Date: 2002-10-23 14:11:22 $
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

#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <util/util.hxx>
#include <util/scriptingconstants.hxx>

#include "ScriptNameResolverImpl.hxx"
#include "ScriptRuntimeManager.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

namespace scripting_runtimemgr
{

const sal_Char* const LANGUAGE_TO_RESOLVE_ON = "Java"; // should be configurable
OUString nrs_implName = OUString::createFromAscii(
    "drafts.com.sun.star.script.framework.DefaultScriptNameResolver" );
OUString nrs_serviceName = OUString::createFromAscii(
    "drafts.com.sun.star.script.framework.DefaultScriptNameResolver" );
Sequence< OUString > nrs_serviceNames = Sequence< OUString >( &nrs_serviceName, 1 );

extern ::rtl_StandardModuleCount s_moduleCount;

// define storages to search
static ::std::vector< sal_Int32 >* m_pSearchIDs = NULL;

//*************************************************************************
ScriptNameResolverImpl::ScriptNameResolverImpl(
    const Reference< XComponentContext > & xContext ) :
    m_xContext( xContext ), m_StorageFactory( xContext )
{
    OSL_TRACE( "< ScriptNameResolverImpl ctor called >\n" );
    if(!m_pSearchIDs)
    {
        osl::Guard< osl::Mutex > aGuard( m_mutex );
        if(!m_pSearchIDs)
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
Reference< XInterface > ScriptNameResolverImpl::resolve(
const ::rtl::OUString & scriptURI, Any& invocationCtx )
throw ( lang::IllegalArgumentException, script::CannotConvertException, RuntimeException )
{

    Reference< XInterface > resolvedName;
    Reference< beans::XPropertySet > xPropSetScriptingContext;
    OSL_TRACE( "ScriptNameResolverImpl::resolve: in resolve - start" );

    if ( sal_False == ( invocationCtx >>= xPropSetScriptingContext ) )
    {
        throw RuntimeException( OUSTR(
            "ScriptNameResolverImpl::resolve : unable to get XScriptingContext from param" ),
            Reference< XInterface > () );
    }

    Any any;
    OUString docUri;
    sal_Int32 docSid;
    try
    {
        scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
            scripting_constants::ScriptingConstantsPool::instance();
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


#ifdef _DEBUG
    ::rtl::OString docUriO(
        ::rtl::OUStringToOString( docUri , RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr,
        "ScriptNameResolverImpl::resolve: *** >>> DOC URI: %s, doc sid is %d\n",
        docUriO.pData->buffer, docSid );
#endif


    OSL_TRACE( "ScriptNameResolverImpl::resolve Starting..." );
    ::std::vector< sal_Int32 >& m_vSearchIDs = *m_pSearchIDs;
    m_vSearchIDs[ 0 ] = docSid;
    ::std::vector< sal_Int32 >::const_iterator iter;
    ::std::vector< sal_Int32 >::const_iterator iterEnd = m_vSearchIDs.end();

    for ( iter = m_vSearchIDs.begin() ; iter != iterEnd; ++iter )
    {
        try
        {
            OSL_TRACE( "** about to resolve from storage using id %d from vector of size %d",
                *iter, m_vSearchIDs.size() );
            if ( ( resolvedName = resolveURIFromStorageID( *iter, scriptURI ) ).is() )
            {
                OSL_TRACE( "found match in uri from storage %d", *iter );
                break;
            }

        }

        catch ( Exception & e )
        {
            OSL_TRACE( "Exception thrown by storage %d, failed to match uri: %s",
                *iter,
                ::rtl::OUStringToOString( e.Message,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
#ifdef _DEBUG
        catch ( ... )
        {
            OSL_TRACE( "unknown exception thrown by storage %d, failed to match uri",
                *iter );
        }
#endif

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

Reference< XInterface >
ScriptNameResolverImpl::resolveURIFromStorageID
( sal_Int32 sid, const ::rtl::OUString& scriptURI )
SAL_THROW ( ( lang::IllegalArgumentException, RuntimeException ) )
{
#ifdef FOOOOO //_DEBUG

    ::rtl::OString locationO( ::rtl::OUStringToOString(
        scriptURI.getLocation(), RTL_TEXTENCODING_ASCII_US ) );
    ::rtl::OString languageO( ::rtl::OUStringToOString(
        scriptURI.getLanguage(), RTL_TEXTENCODING_ASCII_US ) );
    ::rtl::OString functionName( ::rtl::OUStringToOString(
        scriptURI.getFunctionName(), RTL_TEXTENCODING_ASCII_US ) );
    ::rtl::OString logicalName( ::rtl::OUStringToOString(
        scriptURI.getLogicalName(), RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr,
        "trying to resolve URI, {location = %s}, {language = %s}, {funtionName = %s}, {logicalName = %s}\n",
        locationO.pData->buffer, languageO.pData->buffer,
        functionName.pData->buffer, logicalName.pData->buffer );
#endif

    Reference< XInterface > resolvedName;
    scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
        scripting_constants::ScriptingConstantsPool::instance();
    if ( sid == scriptingConstantsPool.DOC_STORAGE_ID_NOT_SET )
    {
        OSL_TRACE( "@@@@ **** ScriptNameResolverImpl::resolve DOC_STORAGE_ID_NOT_SET" );
        return resolvedName;
    }
    try
    {
        Reference< storage::XScriptInfoAccess > storage =
            m_StorageFactory.getStorageInstance( sid );
        validateXRef( storage,
        "ScriptNameResolverImpl::resolveURIFromStorageID: cannot get XScriptInfoAccess" );
        Sequence< Reference< storage::XScriptInfo > > results =
            storage->getImplementations( scriptURI );

        const sal_Int32 length = results.getLength();

        if ( !length )
        {
            return resolvedName;
        }

        OSL_TRACE( "ScriptNameResolverImpl::resolve Got some results..." );
        for ( sal_Int32 index = 0;index < length;index++ )
        {
            Reference< storage::XScriptInfo > uri = results[ index ];
#ifdef _DEBUG

            ::rtl::OString locationO( ::rtl::OUStringToOString( uri->getScriptLocation(),
                RTL_TEXTENCODING_ASCII_US ) );
            ::rtl::OString languageO( ::rtl::OUStringToOString( uri->getLanguage(),
                RTL_TEXTENCODING_ASCII_US ) );
            ::rtl::OString functionName( ::rtl::OUStringToOString( uri->getFunctionName(),
                RTL_TEXTENCODING_ASCII_US ) );
            ::rtl::OString logicalName( ::rtl::OUStringToOString( uri->getLogicalName(),
                RTL_TEXTENCODING_ASCII_US ) );
            fprintf( stderr, "[%d] URI, {location = %s}, {language = %s}, {funtionName = %s}, {logicalName = %s}\n",
                     index, locationO.pData->buffer, languageO.pData->buffer,
                     functionName.pData->buffer, logicalName.pData->buffer );
#endif

            // just choose first one that has language=LANGUAGE_TO_RESOLVE_ON
            ::rtl::OUString language( uri->getLanguage() );

            if ( ( language.compareToAscii( LANGUAGE_TO_RESOLVE_ON ) == 0 ) )
            {
                OSL_TRACE( "Found desired language\n" );
                resolvedName = uri;
                break;
            }
        }
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
    return resolvedName;
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
