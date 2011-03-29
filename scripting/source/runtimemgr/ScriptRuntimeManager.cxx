/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <vcl/msgbox.hxx>

#include "ScriptExecDialog.hrc"

#include <util/scriptingconstants.hxx>

#include <cppuhelper/implementationentry.hxx>
#include <tools/diagnose_ex.h>

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

static OUString s_implName(RTL_CONSTASCII_USTRINGPARAM(
 "drafts.com.sun.star.script.framework.runtime.ScriptRuntimeManager" ));
static OUString s_serviceName(RTL_CONSTASCII_USTRINGPARAM(
 "drafts.com.sun.star.script.framework.runtime.ScriptRuntimeManager" ));
static Sequence< OUString > s_serviceNames = Sequence< OUString >( &s_serviceName, 1 );

::rtl_StandardModuleCount s_moduleCount = MODULE_COUNT_INIT;

//*************************************************************************
// ScriptRuntimeManager Constructor
ScriptRuntimeManager::ScriptRuntimeManager(
    const Reference< XComponentContext > & xContext ) :
    m_xContext( xContext, UNO_SET_THROW )
{
    OSL_TRACE( "< ScriptRuntimeManager ctor called >\n" );
    m_xMgr.set( m_xContext->getServiceManager(), UNO_SET_THROW );
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

        OUStringBuffer* buf( 80 );
        buf.appendAscii("/singletons/drafts.com.sun.star.script.framework.runtime.theScriptRuntimeFor");
        buf.append(sinfo->getLanguage());

        xInterface.set( m_xContext->getValueByName( buf.makeStringAndClear() ), UNO_QUERY_THROW );
        xScriptInvocation.set( xInterface, UNO_QUERY_THROW );
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
        Reference< XInterface > xInterface(
            m_xMgr->createInstanceWithContext(
                OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "drafts.com.sun.star.script.framework.runtime.DefaultScriptNameResolver" )),
                m_xContext
            ),
            UNO_SET_THROW
        );
        xScriptNameResolver.set( xInterface, UNO_QUERY_THROW );
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
        Reference< storage::XScriptInfo > resolvedScript = resolve( scriptURI, resolvedCtx );
        ENSURE_OR_THROW( resolvedScript.is(), "ScriptRuntimeManager::invoke: No resolvedURI" );

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
        ENSURE_OR_THROW( xScriptInvocation.is(),
            "ScriptRuntimeManager::invoke: cannot get instance of language specific runtime." );

        // the scriptURI is currently passed to the language-dept runtime but
        // is not used (may be useful in the future?). All of the script info
        // is contained as a property(SCRIPT_INFO) within the resolvedCtx
        results = xScriptInvocation->invoke( scriptURI, resolvedCtx, aParams,
                                             aOutParamIndex, aOutParam );

        // need to dispose of filesystem storage
        OUString filesysString(RTL_CONSTASCII_USTRINGPARAM(
                                        "location=filesystem" ));
        if ( scriptURI.indexOf( filesysString ) != -1 )
        {
            Any a = m_xContext->getValueByName(
                    scriptingConstantsPool.SCRIPTSTORAGEMANAGER_SERVICE );
            Reference < lang::XEventListener > xEL_ScriptStorageManager( a, UNO_QUERY_THROW );
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
    ENSURE_OR_THROW( xScriptNameResolver.is(),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
