/*************************************************************************
 *
 *  $RCSfile: ScriptRuntimeManager.cxx,v $
 *
 *  $Revision: 1.19 $
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

#ifndef _VCL_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#include "ScriptExecDialog.hrc"

#include <util/scriptingconstants.hxx>

#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>

#include "ScriptNameResolverImpl.hxx"
#include "ScriptRuntimeManager.hxx"
#include <util/util.hxx>
#include <util/scriptingconstants.hxx>

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

namespace scripting_runtimemgr
{

static OUString s_implName = ::rtl::OUString::createFromAscii(
 "drafts.com.sun.star.script.framework.runtime.ScriptRuntimeManager" );
static OUString s_serviceName = ::rtl::OUString::createFromAscii(
 "drafts.com.sun.star.script.framework.runtime.ScriptRuntimeManager" );
static Sequence< OUString > s_serviceNames = Sequence< OUString >( &s_serviceName, 1 );

::rtl_StandardModuleCount s_moduleCount = MODULE_COUNT_INIT;

//*************************************************************************
// ScriptRuntimeManager Constructor
ScriptRuntimeManager::ScriptRuntimeManager(
    const Reference< XComponentContext > & xContext ) :
    m_xContext( xContext )
{
    OSL_TRACE( "< ScriptRuntimeManager ctor called >\n" );
    validateXRef( m_xContext,
        "ScriptRuntimeManager::ScriptRuntimeManager: invalid context" );
    m_xMgr = m_xContext->getServiceManager();
    validateXRef( m_xMgr,
        "ScriptRuntimeManager::ScriptRuntimeManager: cannot get ServiceManager" );
    s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );
    // test
    //scripting_securitymgr::ScriptSecurityManager ssm(xContext);
}

//*************************************************************************
// ScriptRuntimeManager Destructor
ScriptRuntimeManager::~ScriptRuntimeManager()
{
    OSL_TRACE( "< ScriptRuntimeManager dtor called >\n" );
    s_moduleCount.modCnt.release( &s_moduleCount.modCnt );
}

//*************************************************************************
// Get the proper XScriptInvocation
Reference< runtime::XScriptInvocation > SAL_CALL ScriptRuntimeManager::getScriptRuntime(
const Reference< XInterface >& scriptInfo )
throw( RuntimeException )
{
    OSL_TRACE( "** ==> ScriptRuntimeManager in getScriptRuntime\n" );

    Reference< runtime::XScriptInvocation > xScriptInvocation;

    try
    {
        Reference< XInterface > xInterface;

        Reference< storage::XScriptInfo > sinfo =
            Reference< storage::XScriptInfo >( scriptInfo, UNO_QUERY_THROW );

        OUStringBuffer *buf = new OUStringBuffer(80);
        buf->appendAscii("/singletons/drafts.com.sun.star.script.framework.runtime.theScriptRuntimeFor");
        buf->append(sinfo->getLanguage());

        Any a = m_xContext->getValueByName(buf->makeStringAndClear());

        if ( sal_False == ( a >>= xInterface ) )
        {
            throw RuntimeException(
                sinfo->getLanguage().concat( OUSTR( " runtime support is not installed for this language" ) ),
                Reference< XInterface >() );
        }
        validateXRef( xInterface,
            "ScriptRuntimeManager::GetScriptRuntime: cannot get appropriate ScriptRuntime Service"
        );
        xScriptInvocation = Reference< runtime::XScriptInvocation >( xInterface, UNO_QUERY_THROW );
    }
    catch ( Exception & e )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::GetScriptRuntime: " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }

    return xScriptInvocation;
}

//*************************************************************************
// Get the proper XScriptNameResolver
Reference< runtime::XScriptNameResolver > SAL_CALL
ScriptRuntimeManager::getScriptNameResolver()
throw( RuntimeException )
{
    OSL_TRACE( "** ==> ScriptRuntimeManager in getScriptNameResolver\n" );
    Reference< runtime::XScriptNameResolver > xScriptNameResolver;

    try
    {
        Reference< XInterface > xInterface = m_xMgr->createInstanceWithContext(
            OUString::createFromAscii(
                "drafts.com.sun.star.script.framework.runtime.DefaultScriptNameResolver" ),
                m_xContext );
        validateXRef( xInterface,
            "ScriptRuntimeManager::GetScriptRuntime: cannot get instance of DefaultScriptNameResolver" );
        xScriptNameResolver = Reference< runtime::XScriptNameResolver >( xInterface, UNO_QUERY_THROW );
    }
    catch ( Exception & e )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::GetScriptNameResolver: " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
    return xScriptNameResolver;
}

//*************************************************************************
// XScriptInvocation implementation
Any SAL_CALL ScriptRuntimeManager::invoke(
    const ::rtl::OUString & scriptURI,
    const Any& invocationCtx, const Sequence< Any >& aParams,
    Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam )
    throw ( lang::IllegalArgumentException, script::CannotConvertException,
            reflection::InvocationTargetException, RuntimeException )
{
    OSL_TRACE( "** ==> ScriptRuntimeManager in runtimemgr invoke\n" );

    Any results;
    scripting_constants::ScriptingConstantsPool& scriptingConstantsPool =
                scripting_constants::ScriptingConstantsPool::instance();

    // Initialise resolved context with invocation context,
    // the resolved context (resolvedCtx will be modified by the
    // resolve method to contain the storage where the script code is
    // stored
    Any resolvedCtx = invocationCtx;

    try
    {
        Reference< storage::XScriptInfo > resolvedScript = resolve( scriptURI,
            resolvedCtx );
        validateXRef( resolvedScript, "ScriptRuntimeManager::invoke: No resolvedURI" );

        Reference< beans::XPropertySet > xPropSetResolvedCtx;
        if ( sal_False == ( resolvedCtx >>= xPropSetResolvedCtx ) )
        {
            throw RuntimeException( OUSTR(
                "ScriptRuntimeManager::invoke : unable to get XPropSetScriptingContext from param" ),
                Reference< XInterface > () );
        }

        Any any = xPropSetResolvedCtx->getPropertyValue(
            scriptingConstantsPool.RESOLVED_STORAGE_ID );
        sal_Int32 resolvedSid;
        if ( sal_False == ( any >>= resolvedSid ) )
        {
            throw RuntimeException( OUSTR(
                "ScriptRuntimeManager::invoke : unable to get resolved storage id from xPropSetResolvedCtx" ),
                Reference< XInterface > () );
        }

        OSL_TRACE("Storage sid is: %d\n", resolvedSid);

        // modifying the XPropertySet on the resolved Context to contain the
        // full script info
        Any aResolvedScript;
        aResolvedScript <<= resolvedScript;

        xPropSetResolvedCtx->setPropertyValue( scriptingConstantsPool.SCRIPT_INFO,
                aResolvedScript );

        Reference< runtime::XScriptInvocation > xScriptInvocation =
            getScriptRuntime( resolvedScript );
        validateXRef( xScriptInvocation,
            "ScriptRuntimeManager::invoke: cannot get instance of language specific runtime." );

        // the scriptURI is currently passed to the language-dept runtime but
        // is not used (may be useful in the future?). All of the script info
        // is contained as a property(SCRIPT_INFO) within the resolvedCtx
        results = xScriptInvocation->invoke( scriptURI, resolvedCtx, aParams,
                                             aOutParamIndex, aOutParam );

        // need to dispose of filesystem storage
        OUString filesysString = OUString::createFromAscii(
                                        "location=filesystem" );
        if ( scriptURI.indexOf( filesysString ) != -1 )
        {
            Any a = m_xContext->getValueByName(
                    scriptingConstantsPool.SCRIPTSTORAGEMANAGER_SERVICE );
            Reference < lang::XEventListener > xEL_ScriptStorageManager;
            if ( sal_False == ( a >>= xEL_ScriptStorageManager ) )
            {
                throw RuntimeException( OUSTR( "ScriptRuntimeManager::invoke: can't get ScriptStorageManager XEventListener interface when trying to dispose of filesystem storage" ),
                        Reference< XInterface > () );
            }
            validateXRef( xEL_ScriptStorageManager, "Cannot get XEventListener from ScriptStorageManager" );
            lang::EventObject event(resolvedScript);
            xEL_ScriptStorageManager->disposing( event );
        }
    }
    catch ( lang::IllegalArgumentException & iae )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::invoke IllegalArgumentException: " );
        throw lang::IllegalArgumentException( temp.concat( iae.Message ),
                                              Reference< XInterface > (),
                                              iae.ArgumentPosition );
    }
    catch ( script::CannotConvertException & cce )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::invoke CannotConvertException: " );
        throw script::CannotConvertException( temp.concat( cce.Message ),
                                              Reference< XInterface > (),
                                              cce.DestinationTypeClass, cce.Reason,
                                              cce.ArgumentIndex );
    }
    catch ( reflection::InvocationTargetException & ite )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::invoke InvocationTargetException: " );
        throw reflection::InvocationTargetException( temp.concat( ite.Message ),
                Reference< XInterface > (), ite.TargetException );
    }
    catch ( beans::UnknownPropertyException & e )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::invoke UnknownPropertyException: " );
        throw RuntimeException( temp.concat( e.Message ),
                                Reference< XInterface > () );
    }
    catch ( lang::WrappedTargetException  & e )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::invoke WrappedTargetException : " );
        throw RuntimeException( temp.concat( e.Message ),
                                Reference< XInterface > () );
    }
    catch ( RuntimeException & re )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::invoke RuntimeException: " );
        throw RuntimeException( temp.concat( re.Message ),
                                Reference< XInterface > () );
    }
    catch ( Exception & e )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::invoke Exception: " );
        throw RuntimeException( temp.concat( e.Message ),
                                Reference< XInterface > () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        throw RuntimeException( OUSTR( "ScriptRuntimeManager::invoke UnknownException: " ),
                                Reference< XInterface > () );
    }
#endif
    OSL_TRACE( "** ==> ScriptRuntimeManager returned from invoke: %s\n", ::rtl::OUStringToOString( results.getValueTypeName(),  RTL_TEXTENCODING_ASCII_US ).pData->buffer );
    return results;
}

//*************************************************************************
// XScriptNameResolver implementation
Reference< storage::XScriptInfo > SAL_CALL
ScriptRuntimeManager::resolve( const ::rtl::OUString& scriptURI,
    Any& invocationCtx )
throw( lang::IllegalArgumentException, script::CannotConvertException, RuntimeException )
{
    OSL_TRACE( "** ==> ScriptRuntimeManager in resolve\n" );
    Reference< storage::XScriptInfo > resolvedURI;

    Reference< runtime::XScriptNameResolver > xScriptNameResolver = getScriptNameResolver();
    validateXRef( xScriptNameResolver,
        "ScriptRuntimeManager::resolve: No ScriptNameResolver" );

    try
    {
        resolvedURI = xScriptNameResolver->resolve( scriptURI, invocationCtx );
    }
    catch ( lang::IllegalArgumentException & iae )
    {
        OUString temp =
            OUSTR( "ScriptRuntimeManager::resolve IllegalArgumentException: " );
        throw lang::IllegalArgumentException( temp.concat( iae.Message ),
                                              Reference< XInterface > (),
                                              iae.ArgumentPosition );
    }
    catch ( script::CannotConvertException & cce )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::resolve CannotConvertException: " );
        throw script::CannotConvertException( temp.concat( cce.Message ),
                                              Reference< XInterface > (),
                                              cce.DestinationTypeClass, cce.Reason,
                                              cce.ArgumentIndex );
    }
    catch ( RuntimeException & re )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::resolve RuntimeException: " );
        throw RuntimeException( temp.concat( re.Message ),
                                Reference< XInterface > () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        throw RuntimeException(
            OUSTR( "ScriptRuntimeManager::resolve UnknownException: " ),
            Reference< XInterface > () );
    }
#endif

    return resolvedURI;
}

//*************************************************************************
OUString SAL_CALL ScriptRuntimeManager::getImplementationName( )
throw( RuntimeException )
{
    return s_implName;
}

//*************************************************************************
sal_Bool SAL_CALL ScriptRuntimeManager::supportsService( const OUString& serviceName )
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
Sequence<OUString> SAL_CALL ScriptRuntimeManager::getSupportedServiceNames( )
throw( RuntimeException )
{
    return s_serviceNames;
}

//*************************************************************************
static Reference< XInterface > SAL_CALL srm_create(
    const Reference< XComponentContext > & xCompC )
{
    return ( cppu::OWeakObject * ) new ScriptRuntimeManager( xCompC );
}

//*************************************************************************
static Sequence<OUString> srm_getSupportedServiceNames( )
SAL_THROW( () )
{
    return s_serviceNames;
}

//*************************************************************************
static OUString srm_getImplementationName( )
SAL_THROW( () )
{
    return s_implName;
}

//*************************************************************************
Reference< XInterface > SAL_CALL scriptnri_create(
    Reference< XComponentContext > const & xComponentContext )
SAL_THROW( ( Exception ) );

//*************************************************************************
Sequence< OUString > scriptnri_getSupportedServiceNames() SAL_THROW( () );

//*************************************************************************
OUString scriptnri_getImplementationName() SAL_THROW( () );

//******************** ScriptStorageMangaer defines ***********************
Reference< XInterface > SAL_CALL ssm_create(
    Reference< XComponentContext > const & xComponentContext )
SAL_THROW( ( Exception ) );
//*************************************************************************
Sequence< OUString > ssm_getSupportedServiceNames() SAL_THROW( () );
//*************************************************************************
OUString ssm_getImplementationName() SAL_THROW( () );
//*************************************************************************

//************ Script Provider defines ************************************
Reference< XInterface > SAL_CALL sp_create( const Reference< XComponentContext > & xCompC );
//******************** ScriptProvider defines ***************************
Sequence< OUString > sp_getSupportedServiceNames( ) SAL_THROW( () );
//*************************************************************************
OUString sp_getImplementationName( ) SAL_THROW( () );
//*************************************************************************

//************ ScriptStorage defines **************************************
Reference< XInterface > SAL_CALL ss_create( const Reference< XComponentContext > & xCompC );
//******************** ScriptProvider defines ***************************
Sequence< OUString > ss_getSupportedServiceNames( ) SAL_THROW( () );
//*************************************************************************
OUString ss_getImplementationName( ) SAL_THROW( () );
//*************************************************************************


static struct cppu::ImplementationEntry s_entries [] =
    {
        {
            srm_create, srm_getImplementationName,
            srm_getSupportedServiceNames, cppu::createSingleComponentFactory,
            &s_moduleCount.modCnt, 0
        },
        {
            scriptnri_create, scriptnri_getImplementationName,
            scriptnri_getSupportedServiceNames, cppu::createSingleComponentFactory,
            &s_moduleCount.modCnt, 0
        },
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
} // Namespace

//#######################################################################################
//#### EXPORTED #########################################################################
//#######################################################################################

/**
 * Gives the environment this component belongs to.
 */
extern "C"
{
    void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName,
        uno_Environment ** ppEnv )
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
    sal_Bool SAL_CALL component_writeInfo( lang::XMultiServiceFactory * pServiceManager,
        registry::XRegistryKey * pRegistryKey )
    {
        if (::cppu::component_writeInfoHelper( pServiceManager, pRegistryKey,
            ::scripting_runtimemgr::s_entries ))
        {
            try
            {
                // register RuntimeManager singleton

                registry::XRegistryKey * pKey =
                    reinterpret_cast< registry::XRegistryKey * >(pRegistryKey);

                Reference< registry::XRegistryKey > xKey(
                    pKey->createKey(

                    OUSTR("drafts.com.sun.star.script.framework.runtime.ScriptRuntimeManager/UNO/SINGLETONS/drafts.com.sun.star.script.framework.runtime.theScriptRuntimeManager")));
                    xKey->setStringValue( OUSTR("drafts.com.sun.star.script.framework.runtime.ScriptRuntimeManager") );

                // ScriptStorage Mangaer singleton

                xKey = pKey->createKey(
                    OUSTR("drafts.com.sun.star.script.framework.storage.ScriptStorageManager/UNO/SINGLETONS/drafts.com.sun.star.script.framework.storage.theScriptStorageManager"));
                 xKey->setStringValue( OUSTR("drafts.com.sun.star.script.framework.storage.ScriptStorageManager") );
                // Singleton entries are not handled by the setup process
                // below is the only alternative at the momement which
                // is to programmatically do this.

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
