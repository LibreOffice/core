/*************************************************************************
 *
 *  $RCSfile: ScriptRuntimeManager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jmrice $ $Date: 2002-09-27 12:16:26 $
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

#include "ScriptNameResolverImpl.hxx"
#include "ScriptRuntimeManager.hxx"
#include <util/util.hxx>

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

namespace scripting_runtimemgr
{

static OUString s_implName = ::rtl::OUString::createFromAscii(
 "drafts.com.sun.star.script.framework.ScriptRuntimeManager" );
static OUString s_serviceName = ::rtl::OUString::createFromAscii(
 "drafts.com.sun.star.script.framework.ScriptRuntimeManager" );
static Sequence< OUString > s_serviceNames = Sequence< OUString >( &s_serviceName, 1 );

::rtl_StandardModuleCount s_moduleCount = MODULE_COUNT_INIT;

//*************************************************************************
// ScriptRuntimeManager Constructor
ScriptRuntimeManager::ScriptRuntimeManager(
    const Reference< XComponentContext > & xContext ) :
    m_xContext( xContext )
{
    OSL_TRACE( "< ScriptRuntimeManager ctor called >\n" );
    s_moduleCount.modCnt.acquire( &s_moduleCount.modCnt );
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
Reference< XScriptInvocation > SAL_CALL ScriptRuntimeManager::getScriptRuntime(
const Reference< scripturi::XScriptURI >& scriptURI )
throw( RuntimeException )
{
    OSL_TRACE( "** ==> ScriptRuntimeManager in getScriptRuntime\n" );

    Reference< XScriptInvocation > xScriptInvocation;

    validateXRef( m_xContext,
        "ScriptRuntimeManager::GetScriptRuntime: No context available" );
    Reference< lang::XMultiComponentFactory > xMgr = m_xContext->getServiceManager();
    validateXRef( xMgr,
        "ScriptRuntimeManager::GetScriptRuntime: No service manager available" );

    try
    {
        Reference< XInterface > xx;
        Any a = m_xContext->getValueByName( OUString::createFromAscii(
            "/singletons/drafts.com.sun.star.script.framework.theScriptRuntimeForJava" ) );
        a >>= xx;
        validateXRef( xx,
            "ScriptRuntimeManager::GetScriptRuntime: cannot get ScriptRuntimeForJava Service"
        );
        xScriptInvocation = Reference< XScriptInvocation >( xx, UNO_QUERY_THROW );
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
Reference< XScriptNameResolver > SAL_CALL
ScriptRuntimeManager::getScriptNameResolver()
throw( RuntimeException )
{
    OSL_TRACE( "** ==> ScriptRuntimeManager in getScriptNameResolver\n" );
    Reference< XScriptNameResolver > xScriptNameResolver;

    validateXRef( m_xContext,
        "ScriptRuntimeManager::GetScriptNameResolver: No context available" );
    Reference< lang::XMultiComponentFactory > xMgr = m_xContext->getServiceManager();
    validateXRef( xMgr,
        "ScriptRuntimeManager::GetScriptRuntime: No Service Manager available" );

    try
    {
        Reference< XInterface > xx = xMgr->createInstanceWithContext(
            OUString::createFromAscii(
                "drafts.com.sun.star.script.framework.DefaultScriptNameResolver" ),
                m_xContext );
        validateXRef( xx,
            "ScriptRuntimeManager::GetScriptRuntime: cannot get instance of DefaultScriptNameResolver" );
        xScriptNameResolver = Reference< XScriptNameResolver >( xx, UNO_QUERY_THROW );
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
    const Reference< scripturi::XScriptURI > & scriptURI,
    const Any& invocationCtx, const Sequence< Any >& aParams,
    Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam )
    throw ( lang::IllegalArgumentException, script::CannotConvertException,
            reflection::InvocationTargetException, RuntimeException )
{
    OSL_TRACE( "** ==> ScriptRuntimeManager in runtimemgr invoke\n" );
    Any results;
    // Initialise resolved context with invocation context,
    // the resolved context (resolvedCtx will be modified by the
    // resolve method to contain the storage where the script code is
    // stored
    Any resolvedCtx = invocationCtx;
    try
    {
        Reference< scripturi::XScriptURI > resolvedURI = resolve( scriptURI,
            resolvedCtx );
        validateXRef( resolvedURI, "ScriptRuntimeManager::invoke: No resolvedURI" );

        Reference< XScriptInvocation > xScriptInvocation =
            getScriptRuntime( resolvedURI );
        validateXRef( xScriptInvocation,
            "ScriptRuntimeManager::invoke: cannot get instance of language specific runtime." );


        results = xScriptInvocation->invoke( resolvedURI, resolvedCtx, aParams,
                                             aOutParamIndex, aOutParam );
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
    catch ( RuntimeException & re )
    {
        OUString temp = OUSTR( "ScriptRuntimeManager::invoke RuntimeException: " );
        throw RuntimeException( temp.concat( re.Message ),
                                Reference< XInterface > () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        throw RuntimeException( OUSTR( "ScriptRuntimeManager::invoke UnknownException: " ),
                                Reference< XInterface > () );
    }
#endif
    return results;
}

//*************************************************************************
// XScriptNameResolver implementation
Reference< scripturi::XScriptURI > SAL_CALL
ScriptRuntimeManager::resolve( const Reference< scripturi::XScriptURI >& scriptURI,
    Any& invocationCtx )
throw( lang::IllegalArgumentException, script::CannotConvertException, RuntimeException )
{
    OSL_TRACE( "** ==> ScriptRuntimeManager in resolve\n" );
    Reference< scripturi::XScriptURI > resolvedURI;

    Reference< XScriptNameResolver > xScriptNameResolver = getScriptNameResolver();
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
        return ::cppu::component_writeInfoHelper( pServiceManager, pRegistryKey,
            ::scripting_runtimemgr::s_entries );
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
