/*************************************************************************
 *
 *  $RCSfile: MasterScriptProvider.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-19 08:28:10 $
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

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>

#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>

#include <drafts/com/sun/star/script/browse/BrowseNodeTypes.hpp>

#include <util/scriptingconstants.hxx>
#include <util/util.hxx>
#include <util/MiscUtils.hxx>


#include "ActiveMSPList.hxx"
#include "MasterScriptProvider.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script;
using namespace ::sf_misc;

namespace func_provider
{

::rtl::OUString s_implName = ::rtl::OUString::createFromAscii(
    "drafts.com.sun.star.script.provider.MasterScriptProvider" );
const ::rtl::OUString s_serviceNameList[] = {
    ::rtl::OUString::createFromAscii(
        "drafts.com.sun.star.script.provider.MasterScriptProvider" ),
    ::rtl::OUString::createFromAscii(
        "drafts.com.sun.star.script.provider.ScriptProvider" ) };

Sequence< ::rtl::OUString > s_serviceNames = Sequence <
        ::rtl::OUString > ( s_serviceNameList, 2 );

//*************************************************************************
//  Definitions for MasterScriptProviderFactory global methods.
//*************************************************************************

::rtl::OUString SAL_CALL mspf_getImplementationName() ;
Reference< XInterface > SAL_CALL mspf_create( Reference< XComponentContext > const & xComponentContext );
Sequence< ::rtl::OUString > SAL_CALL mspf_getSupportedServiceNames();

//::rtl_StandardModuleCount s_moduleCount = MODULE_COUNT_INIT;

/* should be available in some central location. */
//*************************************************************************
// XScriptProvider implementation
//
//*************************************************************************
MasterScriptProvider::MasterScriptProvider( const Reference< XComponentContext > & xContext ) throw ( RuntimeException ):
        m_xContext( xContext ), m_bIsValid( false ), m_bInitialised( false ),
        m_pPCache( 0 )
{
    OSL_TRACE( "< MasterScriptProvider ctor called >\n" );

    validateXRef( m_xContext, "MasterScriptProvider::MasterScriptProvider: No context available\n" );
    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr,
                  "MasterScriptProvider::MasterScriptProvider: No service manager available\n" );
    m_XScriptingContext = new InvocationCtxProperties( m_xContext );


    m_bIsValid = true;
}

//*************************************************************************
MasterScriptProvider::~MasterScriptProvider()
{
    OSL_TRACE( "< MasterScriptProvider dtor called >\n" );

    //s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
    if ( m_pPCache )
    {
        delete m_pPCache;
    }
    m_pPCache = 0;
}

//*************************************************************************
void SAL_CALL MasterScriptProvider::initialize( const Sequence < Any >& args )
throw ( Exception, RuntimeException )
{
    OSL_TRACE( "< MasterScriptProvider::initialize() method called >\n" );
    if ( m_bInitialised )
    {
        OSL_TRACE( "MasterScriptProvider Already initialised" );
        return ;
    }

    m_bIsValid = false;


    sal_Int32 len = args.getLength();
    if ( len > 1  )
    {
        throw RuntimeException(
            OUSTR( "MasterScriptProvider::initialize: invalid number of arguments" ),
            Reference< XInterface >() );
    }

    Sequence< Any > invokeArgs( 1 );

    if ( args.getLength() != 0 )
    {
        // check if first parameter is a string
        // if it is, this implies that this is a MSP created
        // with a user or share ctx ( used for browse functionality )

        //
        if ( args[ 0 ].getValueType() ==  ::getCppuType((const ::rtl::OUString* ) NULL ) )
        {
            args[ 0 ] >>= m_sCtxString;
            OSL_TRACE("Creating MSP for user or share, dir is %s",
            ::rtl::OUStringToOString( m_sCtxString , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            invokeArgs[ 0 ] = args[ 0 ];
        }
        else if ( args[ 0 ].getValueType() ==  ::getCppuType((const Reference< frame::XModel >* ) NULL ) )

        {
            try
            {
                m_xModel.set( args[ 0 ], UNO_QUERY_THROW );

                Any propValXModel = makeAny( m_xModel );

                ::rtl::OUString url ( m_xModel->getURL() );

                // Initial val, indicates no document script storage
                scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
                scripting_constants::ScriptingConstantsPool::instance();

                Any propValUrl = makeAny( url );
                OSL_TRACE( "!!** XModel URL inserted into any is %s \n",
                       ::rtl::OUStringToOString( url , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                // set up invocation context.
                m_XScriptingContext->setPropertyValue( scriptingConstantsPool.DOC_REF,
                                                   propValXModel );
                m_XScriptingContext->setPropertyValue( scriptingConstantsPool.DOC_URI,
                                                   propValUrl );
                invokeArgs[ 0 ] <<= m_XScriptingContext;
            }

            catch ( beans::UnknownPropertyException & e )
            {
                ::rtl::OUString temp = OUSTR(
                                       "MasterScriptProvider::initialize: caught UnknownPropertyException: " );
                throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
            }
            catch ( beans::PropertyVetoException & e )
            {
                ::rtl::OUString temp = OUSTR(
                                       "MasterScriptProvider::initialize: caught PropertyVetoException: " );
                throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
            }
            catch ( lang::IllegalArgumentException & e )
            {
                ::rtl::OUString temp = OUSTR(
                                       "MasterScriptProvider::initialize: caught IllegalArgumentException: " );
                throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
            }
            catch ( lang::WrappedTargetException & e )
            {
                ::rtl::OUString temp = OUSTR(
                                       "MasterScriptProvider::initialize: caught WrappedTargetException: " );
                throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
            }
            catch ( RuntimeException & e )
            {
                ::rtl::OUString temp = OUSTR( "MasterScriptProvider::initialize: " );
                throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
            }
        }

    }
    else // no args
    {
        // use either scriping context or maybe zero args?
        invokeArgs = Sequence< Any >( 0 ); // no arguments
    }
    m_sAargs = invokeArgs;
    OSL_TRACE( "Initialised XMasterScriptProvider" );
    m_bInitialised = true;
    m_bIsValid = true;
}


//*************************************************************************
Reference< provider::XScript >
MasterScriptProvider::getScript( const ::rtl::OUString& scriptURI )
throw ( lang::IllegalArgumentException, RuntimeException )
{
    if ( !isValid() )
    {
        throw RuntimeException(
            OUSTR( "MasterScriptProvider::getScript(), service object not initialised properly." ),
            Reference< XInterface >() );
    }

    OSL_TRACE( "**  ==> MasterScriptProvider in getScript\n" );
    OSL_TRACE( "Script URI is %s",
        ::rtl::OUStringToOString( scriptURI,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer  );

    // need to get the language from the string

    Reference< uri::XUriReferenceFactory > xFac (
         m_xMgr->createInstanceWithContext( rtl::OUString::createFromAscii(
            "com.sun.star.uri.UriReferenceFactory"), m_xContext ) , UNO_QUERY );
    if ( !xFac.is() )
    {
        throw RuntimeException(
            OUSTR( "MasterScriptProvider::getScript(), could not instatiate UriReferenceFactory." ),
            Reference< XInterface >() );
    }

    Reference<  uri::XUriReference > uriRef(
        xFac->parse( scriptURI ), UNO_QUERY );

    Reference < uri::XVndSunStarScriptUrl > sfUri( uriRef, UNO_QUERY );

    if ( !uriRef.is() || !sfUri.is() )
    {
        ::rtl::OUString errorMsg = OUSTR( "Incorrect format for Script URI: " );
        errorMsg.concat( scriptURI );
        throw lang::IllegalArgumentException(
                OUSTR( "invalid URI: " ).concat( errorMsg ),
                Reference < XInterface > (), 1 );
    }

    ::rtl::OUString langKey = ::rtl::OUString::createFromAscii( "language" );

    if ( sfUri->hasParameter( langKey ) == sal_False ||
         ( sfUri->getName().getLength() == 0  ) )
    {
        ::rtl::OUString errorMsg = OUSTR( "Incorrect format for Script URI: " );
        errorMsg.concat( scriptURI );
        throw lang::IllegalArgumentException(
                OUSTR( "invalid URI: " ).concat( errorMsg ),
                Reference < XInterface > (), 1 );
    }
    ::rtl::OUString language = sfUri->getParameter( langKey );

    Reference< provider::XScript > xScript;
    Reference< provider::XScriptProvider > xScriptProvider;
    try
    {
        ::rtl::OUStringBuffer buf( 80 );
        buf.appendAscii( "drafts.com.sun.star.script.provider.ScriptProviderFor");
        buf.append( language );
        ::rtl::OUString serviceName = buf.makeStringAndClear();
        if ( providerCache() )
        {
            xScriptProvider = providerCache()->getProvider( serviceName );
            validateXRef( xScriptProvider, "MasterScriptProvider::getScript() failed to obtain provider" );
            xScript=xScriptProvider->getScript( scriptURI );
        }
        else
        {
            ::rtl::OUString temp = OUSTR( "MasterScriptProvider::getScript: can't access ProviderCache " );
            throw RuntimeException( temp,
                    Reference< XInterface >() );
        }
    }

    catch ( RuntimeException & e )
    {
        ::rtl::OUString temp = OUSTR( "MasterScriptProvider::getScript: can't get XScript for " );
        temp.concat(scriptURI);
        temp.concat( OUSTR( " :" ) );
        throw RuntimeException( temp.concat( e.Message ),
                Reference< XInterface >() );
    }
    catch ( Exception & e )
    {
        ::rtl::OUString temp = OUSTR( "MasterScriptProvider::getScript: catch exception: can't get XScript for " );
        temp.concat(scriptURI);
        temp.concat( OUSTR( " :" ) );
        throw RuntimeException( temp.concat( e.Message ),
                Reference< XInterface >() );
    }

    return xScript;
}
//*************************************************************************
bool
MasterScriptProvider::isValid()
{
    return m_bIsValid;
}

//*************************************************************************
ProviderCache*
MasterScriptProvider::providerCache()
{
    if ( !m_pPCache )
    {
        ::osl::MutexGuard aGuard( m_mutex );
        if ( !m_pPCache )
        {
            m_pPCache = new ProviderCache( m_xContext, m_sAargs );
        }
    }
    return m_pPCache;
}


//*************************************************************************
::rtl::OUString SAL_CALL
MasterScriptProvider::getName()
        throw ( css::uno::RuntimeException )
{
    if ( m_xModel.is() )
    {
        m_sNodeName = MiscUtils::xModelToDocTitle( m_xModel );
    }
    else
    {
        m_sNodeName = parseLocationName( m_sCtxString );
    }
    return m_sNodeName;
}

//*************************************************************************
Sequence< Reference< browse::XBrowseNode > > SAL_CALL
MasterScriptProvider::getChildNodes()
        throw ( css::uno::RuntimeException )
{
    OSL_TRACE("MasterScriptProvider:getChildNodes() ctx = %s",
        ::rtl::OUStringToOString( m_sNodeName,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    Sequence< Reference< provider::XScriptProvider > > providers = getAllProviders();

    sal_Int32 size = providers.getLength();
    Sequence<  Reference< browse::XBrowseNode > > children( size );
    sal_Int32 provIndex = 0;
    for ( ; provIndex < providers.getLength(); provIndex++ )
    {
        children[ provIndex ] = Reference< browse::XBrowseNode >( providers[ provIndex ], UNO_QUERY );
    }
    return children;
}

//*************************************************************************
sal_Bool SAL_CALL
MasterScriptProvider::hasChildNodes()
        throw ( css::uno::RuntimeException )
{
    return sal_True;
}

//*************************************************************************
sal_Int16 SAL_CALL
MasterScriptProvider::getType()
        throw ( css::uno::RuntimeException )
{
    return browse::BrowseNodeTypes::CONTAINER;
}

//*************************************************************************

::rtl::OUString
MasterScriptProvider::parseLocationName( const ::rtl::OUString& location )
{
    // strip out the last leaf of location name
    // e.g. file://dir1/dir2/Blah.sxw - > Blah.sxw
    ::rtl::OUString temp = location;
    sal_Int32 lastSlashIndex = temp.lastIndexOf( ::rtl::OUString::createFromAscii( "/" ) );

    if ( lastSlashIndex > -1 )
    {
        if ( ( lastSlashIndex + 1 ) <  temp.getLength()  )
        {
            temp = temp.copy( lastSlashIndex + 1 );
        }
    }
    else
    {
        OSL_TRACE("Something wrong with name, perhaps we should throw an exception");
    }
    return temp;
}

//*************************************************************************
Sequence< Reference< provider::XScriptProvider > > SAL_CALL
MasterScriptProvider::getAllProviders() throw ( css::uno::RuntimeException )
{
    OSL_TRACE("Entering MasterScriptProvider Get ALL Providers() " );
    if ( providerCache() )
    {
        OSL_TRACE("Leaving MasterScriptProvider Get ALL Providers() " );
        return providerCache()->getAllProviders();
    }
    else
    {
        OSL_TRACE("MasterScriptProvider::getAllProviders() something wrong, no language providers");
        ::rtl::OUString errorMsg;
        errorMsg = ::rtl::OUString::createFromAscii("MasterScriptProvider::getAllProviders, unknown error, cache not initialised");
        OSL_TRACE("Leaving MasterScriptProvider Get ALL Providers() THROWING " );
        return providerCache()->getAllProviders();
        throw RuntimeException( errorMsg.concat( errorMsg ),
                        Reference< XInterface >() );

    }
}
//*************************************************************************
::rtl::OUString SAL_CALL MasterScriptProvider::getImplementationName( )
throw( RuntimeException )
{
    return s_implName;
}

//*************************************************************************
sal_Bool SAL_CALL MasterScriptProvider::supportsService( const ::rtl::OUString& serviceName )
throw( RuntimeException )
{
    ::rtl::OUString const * pNames = s_serviceNames.getConstArray();
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
Sequence< ::rtl::OUString > SAL_CALL MasterScriptProvider::getSupportedServiceNames( )
throw( RuntimeException )
{
        return s_serviceNames;
    }

    } // namespace func_provider
namespace browsenodefactory
{
::rtl::OUString SAL_CALL bnf_getImplementationName() ;
Reference< XInterface > SAL_CALL bnf_create( Reference< XComponentContext > const & xComponentContext );
Sequence< ::rtl::OUString > SAL_CALL bnf_getSupportedServiceNames();
}

    namespace scripting_runtimemgr
    {
    //*************************************************************************
    Reference< XInterface > SAL_CALL sp_create(
        const Reference< XComponentContext > & xCompC )
    {
        return ( cppu::OWeakObject * ) new ::func_provider::MasterScriptProvider( xCompC );
    }

    //*************************************************************************
    Sequence< ::rtl::OUString > sp_getSupportedServiceNames( )
    SAL_THROW( () )
    {
        return ::func_provider::s_serviceNames;
}


//*************************************************************************
::rtl::OUString sp_getImplementationName( )
SAL_THROW( () )
{
    return ::func_provider::s_implName;
}

static struct cppu::ImplementationEntry s_entries [] =
    {
        {
            sp_create, sp_getImplementationName,
            sp_getSupportedServiceNames, cppu::createSingleComponentFactory,
            0, 0
        },
        {
            func_provider::mspf_create, func_provider::mspf_getImplementationName,
            func_provider::mspf_getSupportedServiceNames, cppu::createSingleComponentFactory,
            0, 0
        },
        {
            browsenodefactory::bnf_create, browsenodefactory::bnf_getImplementationName,
            browsenodefactory::bnf_getSupportedServiceNames, cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}

//############################################################################
//#### EXPORTED ##############################################################
//############################################################################

/**
 * Gives the environment this component belongs to.
 */
extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(
            const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    /**
     * This function creates an implementation section in the registry and another subkey
     *
     * for each supported service.
     * @param pServiceManager   the service manager
     * @param pRegistryKey      the registry key
     */
    sal_Bool SAL_CALL component_writeInfo(
            lang::XMultiServiceFactory * pServiceManager,
            registry::XRegistryKey * pRegistryKey )
    {
        if (::cppu::component_writeInfoHelper( pServiceManager, pRegistryKey,
            ::scripting_runtimemgr::s_entries ))
        {
            try
            {
                // ScriptStorage Mangaer singleton
                registry::XRegistryKey * pKey =
                    reinterpret_cast< registry::XRegistryKey * >(pRegistryKey);

                Reference< registry::XRegistryKey >xKey = pKey->createKey(
                    OUSTR("drafts.com.sun.star.script.provider.MasterScriptProviderFactory/UNO/SINGLETONS/drafts.com.sun.star.script.provider.theMasterScriptProviderFactory"));
                xKey->setStringValue( OUSTR("drafts.com.sun.star.script.provider.MasterScriptProviderFactory") );
                // BrowseNodeFactory Mangager singleton
                xKey = pKey->createKey(
                    OUSTR("drafts.com.sun.star.script.browse.BrowseNodeFactory/UNO/SINGLETONS/drafts.com.sun.star.script.browse.theBrowseNodeFactory"));
                xKey->setStringValue( OUSTR("drafts.com.sun.star.script.browse.BrowseNodeFactory") );
                return sal_True;
            }
            catch (Exception & exc)
            {
            }
        }
        return sal_False;
    }

    /**
     * This function is called to get service factories for an implementation.
     *
     * @param pImplName       name of implementation
     * @param pServiceManager a service manager, need for component creation
     * @param pRegistryKey    the registry key for this component, need for persistent
     *                        data
     * @return a component factory
     */
    void * SAL_CALL component_getFactory( const sal_Char * pImplName,
        lang::XMultiServiceFactory * pServiceManager,
        registry::XRegistryKey * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper( pImplName, pServiceManager,
            pRegistryKey, ::scripting_runtimemgr::s_entries );
    }
}
