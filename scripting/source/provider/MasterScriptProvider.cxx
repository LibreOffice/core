/*************************************************************************
 *
 *  $RCSfile: MasterScriptProvider.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: npower $ $Date: 2003-08-19 09:46:09 $
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

#include <util/scriptingconstants.hxx>
#include <util/util.hxx>
#include "MasterScriptProvider.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

namespace func_provider
{

::rtl::OUString s_implName = ::rtl::OUString::createFromAscii(
    "drafts.com.sun.star.script.framework.provider.MasterScriptProvider" );
::rtl::OUString s_serviceName = ::rtl::OUString::createFromAscii(
    "drafts.com.sun.star.script.framework.provider.MasterScriptProvider" );

Sequence< ::rtl::OUString > s_serviceNames = Sequence <
        ::rtl::OUString > ( &s_serviceName, 1 );

//::rtl_StandardModuleCount s_moduleCount = MODULE_COUNT_INIT;

/* should be available in some central location. */
//*************************************************************************
// XScriptProvider implementation
//
//*************************************************************************
MasterScriptProvider::MasterScriptProvider( const Reference< XComponentContext > & xContext ) :
        m_xContext( xContext ), m_bInitialised( false )
{
    OSL_TRACE( "< MasterScriptProvider ctor called >\n" );

    validateXRef( m_xContext, "MasterScriptProvider::MasterScriptProvider: No context available\n" );
    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr,
                  "MasterScriptProvider::MasterScriptProvider: No service manager available\n" );
    m_XScriptingContext = new ScriptingContext( m_xContext );

    // attemp to raise or access ScriptStorageMgr singleton
    try
    {
        scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
                        scripting_constants::ScriptingConstantsPool::instance();
        Any a = m_xContext->getValueByName(
        scriptingConstantsPool.SCRIPTSTORAGEMANAGER_SERVICE );
        Reference < XInterface > xInterface;
        if ( sal_False == ( a >>= xInterface ) )
        {
            throw RuntimeException(
                OUSTR( "MasterScriptProvider::MasterScriptProvider: could not obtain Stora geManager singleton" ),
                Reference< XInterface >() );
        }
        validateXRef( xInterface,
                          "MasterScriptProvider::initialise: cannot get StorageManager" );
        m_xScriptStorageMgr = Reference< storage::XScriptStorageManager > ( xInterface, UNO_QUERY_THROW );

    }
    catch ( Exception & e )
    {
        ::rtl::OUString temp = OUSTR(
            "MasterScriptProvider::MasterScriptProvider: could not raise instance of ScriptStorageManager : " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
}

//*************************************************************************
MasterScriptProvider::~MasterScriptProvider()
{
    OSL_TRACE( "< MasterScriptProvider dtor called >\n" );

    //s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
}

//*************************************************************************
void SAL_CALL MasterScriptProvider::initialize( Sequence < Any > const & args )
throw ( Exception, RuntimeException )
{
    OSL_TRACE( "< MasterScriptProvider::initialize() method called >\n" );
    if ( m_bInitialised )
    {
        OSL_TRACE( "MasterScriptProvider Already initialised" );
        return ;
    }

    // related to issue 11866
    // warning dialog gets launched when adding binding to script in doc
    // workaround issue: no functionProvider created on doc open
    sal_Int32 len = args.getLength();
    if ( ( len == 0 ) || ( len >=3 ) )
    {
        throw RuntimeException(
            OUSTR( "MasterScriptProvider::initialize: invalid number of arguments" ),
            Reference< XInterface >() );
    }
    sal_Bool displayDialog = sal_True;
    if ( len == 2 )
    {
        if ( sal_False == ( args[ 1 ] >>= displayDialog ) )
        {
            throw RuntimeException(
                OUSTR( "MasterScriptProvider::initialize: invalid argument, arg 2 must be a boolean" ),
                Reference< XInterface >() );
        }
    }


    try
    {
        Reference< XInterface > xInterface;
        if ( sal_False == ( args[ 0 ] >>= xInterface ) )
        {
            throw RuntimeException(
                OUSTR( "MasterScriptProvider::initialize: could not extract xModel from arguments" ),
                Reference< XInterface >() );
        }
        validateXRef( xInterface,
                      "MasterScriptProvider::initialize: invalid xModel xinterface" );

        m_xModel = Reference< frame::XModel > ( xInterface, UNO_QUERY_THROW );

        Any propValXModel;
        propValXModel <<= m_xModel;

        ::rtl::OUString url ( m_xModel->getURL() );

        if ( displayDialog == sal_False )
        {
            url = ::rtl::OUString::createFromAscii( "NoDialog::" ).concat( url );
        }

        // Initial val, indicates no document script storage
        scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
            scripting_constants::ScriptingConstantsPool::instance();
        sal_Int32 documentScriptStorageID =
            scriptingConstantsPool.DOC_STORAGE_ID_NOT_SET;

        OSL_TRACE( "about to call addStorageAsListener");
        addStorageAsListener();

        if ( url.getLength() )
        {
            // If document url is valid (i.e. pointing to a document) we want to get
            // the StorageManager to create a storage component, this will be done
            // by the document in the next rev.
            Reference< XInterface > xInterface = m_xMgr->createInstanceWithContext(
                ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ),
                                                     m_xContext );
            validateXRef( xInterface,
                "MasterScriptProvider::initialise: cannot get SimpleFileAccess Service\n" );
            Reference < ucb::XSimpleFileAccess > xSimpleFileAccess = Reference <
                    ucb::XSimpleFileAccess > ( xInterface, UNO_QUERY_THROW );


            OSL_TRACE( ">>>> About to create storage for %s",
                ::rtl::OUStringToOString( url,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            // ask storage manager to create storage
            try
            {
                documentScriptStorageID =
                    m_xScriptStorageMgr->createScriptStorageWithURI(
                        xSimpleFileAccess, url );
                OSL_TRACE( ">>>> Created storage %d - for %s ",
                    documentScriptStorageID, ::rtl::OUStringToOString(
                        url, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            }
            catch ( RuntimeException & e )
            {
                /* No need to rethrow exception because if no storage available
                   we continue on...*/
                OSL_TRACE( ">>>> Failed to create document storage for %s: %s",
                           ::rtl::OUStringToOString( url,
                                                     RTL_TEXTENCODING_ASCII_US ).pData->buffer,
                           ::rtl::OUStringToOString( e.Message,
                                                     RTL_TEXTENCODING_ASCII_US ).pData->buffer );
            }

        }

        Any propValUrl;
        propValUrl <<= url;

        Any propValSid;
        propValSid <<= documentScriptStorageID ;

        OSL_TRACE( "!!** XModel URL inserted into any is %s \n",
                   ::rtl::OUStringToOString( url , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        // set up invocation context.
        m_XScriptingContext->setPropertyValue( scriptingConstantsPool.DOC_REF,
                                               propValXModel );
        m_XScriptingContext->setPropertyValue( scriptingConstantsPool.DOC_URI,
                                               propValUrl );
        m_XScriptingContext->setPropertyValue( scriptingConstantsPool.DOC_STORAGE_ID,
                                               propValSid );
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
    OSL_TRACE( "Initialised XMasterScriptProvider" );
    m_bInitialised = true;
}


//*************************************************************************
Reference< provider::XScript >
MasterScriptProvider::getScript( const ::rtl::OUString& scriptURI )
throw ( RuntimeException )
{
    if ( !m_bInitialised )
    {
        throw RuntimeException(
            OUSTR( "MasterScriptProvider::getScript: XScriptProvider not initialised." ),
            Reference< XInterface >() );
    }

    OSL_TRACE( "**  ==> MasterScriptProvider in getScript\n" );
    OSL_TRACE( "Script URI is %s",
        ::rtl::OUStringToOString( scriptURI,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer  );
    // need to get the language from the string
    ::rtl::OUString language;
    ::rtl::OUString attr;
    sal_Int32 len = scriptURI.indexOf( '?' );
    // if we have a match, then start the search after the ?
    if( len > 0 )
    {
        len++;
    }
    do
    {
        attr = scriptURI.getToken( 0, '&', len );
        OSL_TRACE( "chunk is %s, len is %d",
            ::rtl::OUStringToOString( attr,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer, len  );
        if( attr.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( "language" ) )
            == sal_True )
        {
            sal_Int32 len2 = attr.indexOf('=');
            language = attr.copy( len2 + 1 );
            OSL_TRACE( "Language name is %s",
                ::rtl::OUStringToOString( language,
                    RTL_TEXTENCODING_ASCII_US ).pData->buffer  );
            break;
        }
    }
    while ( len >= 0 );

    Reference< provider::XScriptProvider > xScriptProvider;
    try
    {
        // need to attempt to get the runtime service (not singleton) for the lang
        ::rtl::OUStringBuffer *buf = new ::rtl::OUStringBuffer( 80 );
        buf->appendAscii( "drafts.com.sun.star.script.framework.provider.ScriptProviderFor");
        buf->append( language );
        ::rtl::OUString serviceName = buf->makeStringAndClear();
        OSL_TRACE( "Service name is %s",
            ::rtl::OUStringToOString( serviceName,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer  );

        // need to pass the scripting context
        Sequence < Any > aArgs( 1 );
        aArgs[ 0 ] <<= m_XScriptingContext;
        Reference< XInterface > xInterface =
            m_xMgr->createInstanceWithArgumentsAndContext(
                serviceName, aArgs, m_xContext );

        // need to get the XScriptInvocation interface from the service
        validateXRef( xInterface,
            "MasterScriptProvider::getScript: cannot get appropriate ScriptRuntime Service");
        xScriptProvider = Reference< provider::XScriptProvider > ( xInterface,
            UNO_QUERY_THROW );
        validateXRef( xScriptProvider,
            "Service doesn't support XScriptProvider interface" );
    }
    catch ( RuntimeException & e )
    {
        ::rtl::OUString temp = OUSTR( "MasterScriptProvider::getScript: no runtime implementation for " );
        temp.concat(language);
        temp.concat( OUSTR( " :" ) );
        throw RuntimeException( temp.concat( e.Message ),
                Reference< XInterface >() );
    }

    Reference< provider::XScript > xScript;
    try
    {
        //xScript = xScriptProvider->getScript( scriptURI );
        xScript=xScriptProvider->getScript( scriptURI );
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
void
MasterScriptProvider::addStorageAsListener()
{
    try
    {
        OSL_TRACE( "IN addStorageAsListener()" );
        // try get XEventListener from m_xScriptStorageMgr
        Reference< lang::XEventListener > xEventListener =
            Reference< lang::XEventListener >( m_xScriptStorageMgr, UNO_QUERY_THROW );
        validateXRef( xEventListener, "MasterScriptProvider::initialse: storage manager not XEventListener\n" );
        // try get XComponent from m_xModel, if we can, then
        // register as listener so we get event when doc is disposed
        Reference< lang::XComponent > xComponent =
            Reference< lang::XComponent >( m_xModel, UNO_QUERY_THROW );
        validateXRef( xComponent, "MasterScriptProvider::initialse: model not XComponent\n" );
        xComponent->addEventListener( xEventListener );
        OSL_TRACE( " addStorageAsListener(), successful" );

    }
    catch ( Exception & e )
    {
        OSL_TRACE( "Failed to add listener for storage, reason %s",
            ::rtl::OUStringToOString( e.Message,
                RTL_TEXTENCODING_ASCII_US ).pData->buffer  );
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
//******************** ScriptStorageMangaer defines ***********************
Reference< XInterface > SAL_CALL ssm_create(
    Reference< XComponentContext > const & xComponentContext )
SAL_THROW( ( Exception ) );
//*************************************************************************
Sequence< ::rtl::OUString > ssm_getSupportedServiceNames() SAL_THROW( () );
//*************************************************************************
::rtl::OUString ssm_getImplementationName() SAL_THROW( () );
//*************************************************************************

//************ ScriptStorage defines **************************************
Reference< XInterface > SAL_CALL ss_create( const Reference< XComponentContext > & xCompC );
//******************** ScriptProvider defines ***************************
Sequence< ::rtl::OUString > ss_getSupportedServiceNames( ) SAL_THROW( () );
//*************************************************************************
::rtl::OUString ss_getImplementationName( ) SAL_THROW( () );
//*************************************************************************

static struct cppu::ImplementationEntry s_entries [] =
    {
        {
            ssm_create, ssm_getImplementationName,
            ssm_getSupportedServiceNames, cppu::createSingleComponentFactory,
            0, 0
        },
        {
            ss_create, ss_getImplementationName,
            ss_getSupportedServiceNames, cppu::createSingleComponentFactory,
            0, 0
        },
        {
            sp_create, sp_getImplementationName,
            sp_getSupportedServiceNames, cppu::createSingleComponentFactory,
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
                    OUSTR("drafts.com.sun.star.script.framework.storage.ScriptStorageManager/UNO/SINGLETONS/drafts.com.sun.star.script.framework.storage.theScriptStorageManager"));
                 xKey->setStringValue( OUSTR("drafts.com.sun.star.script.framework.storage.ScriptStorageManager") );
                // Singleton entries are not handled by the setup process
                // below is the only alternative at the momement which
                // is to programmatically do this.

                /* No longer singletons?
                 // "Java" Runtime singleton entry
                xKey = pKey->createKey(
                    OUSTR("com.sun.star.scripting.runtime.java.ScriptRuntimeForJava$_ScriptRuntimeForJava/UNO/SINGLETONS/drafts.com.sun.star.script.framework.runtime.theScriptRuntimeForJava"));
                 xKey->setStringValue( OUSTR("drafts.com.sun.star.script.framework.runtime.ScriptRuntimeForJava") );

                // "JavaScript" Runtime singleton entry
                xKey = pKey->createKey(
                    OUSTR("com.sun.star.scripting.runtime.javascript.ScriptRuntimeForJavaScript$_ScriptRuntimeForJavaScript/UNO/SINGLETONS/drafts.com.sun.star.script.framework.runtime.theScriptRuntimeForJavaScript"));
                 xKey->setStringValue( OUSTR("drafts.com.sun.star.script.framework.runtime.ScriptRuntimeForJavaScript") );

                // "BeanShell" Runtime singleton entry
                xKey = pKey->createKey(
                    OUSTR("com.sun.star.scripting.runtime.beanshell.ScriptRuntimeForBeanShell$_ScriptRuntimeForBeanShell/UNO/SINGLETONS/drafts.com.sun.star.script.framework.runtime.theScriptRuntimeForBeanShell"));
                 xKey->setStringValue( OUSTR("drafts.com.sun.star.script.framework.runtime.ScriptRuntimeForBeanShell") );
                */

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
