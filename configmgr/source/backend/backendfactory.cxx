/*************************************************************************
 *
 *  $RCSfile: backendfactory.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:46 $
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

#include "backendfactory.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#ifndef CONFIGMGR_BOOTSTRAPCONTEXT_HXX_
#include "bootstrapcontext.hxx"
#endif
#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_BACKENDACCESS_HXX_
#include "backendaccess.hxx"
#endif
#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif
#ifndef CONFIGMGR_WRAPEXCEPTION_HXX
#include "wrapexception.hxx"
#endif


#ifndef _COM_SUN_STAR_CONFIGURATION_CANNOTLOADCONFIGURATIONEXCEPTION_HPP_
#include <com/sun/star/configuration/CannotLoadConfigurationException.hpp>
#endif

#include <drafts/com/sun/star/configuration/backend/XBackend.hpp>
#include <drafts/com/sun/star/configuration/backend/XSingleBackend.hpp>

namespace configmgr
{
// -------------------------------------------------------------------------
    namespace backend
    {
// -------------------------------------------------------------------------
        namespace uno = ::com::sun::star::uno;
        namespace lang = ::com::sun::star::lang;
        namespace backenduno = drafts::com::sun::star::configuration::backend;
// -------------------------------------------------------------------------
const sal_Char k_DefaultBackendWrapper[] = "com.sun.star.comp.configuration.backend.SingleBackendAdapter";
const sal_Char k_DefaultBackendService[] = "com.sun.star.comp.configuration.backend.LocalSingleBackend";

// -------------------------------------------------------------------------
const sal_Char k_DefaultBackendServiceAndImplName[]         = K_DefaultBackendServiceAndImplName ;
const sal_Char k_DefaultSingleBackendServiceAndImplName[]   = K_DefaultSingleBackendServiceAndImplName ;

// -------------------------------------------------------------------------
const sal_Char k_GenericBackendServiceAndImplName[]         = "com.sun.star.configuration.backend.Backend" ;
const sal_Char k_GenericSingleBackendServiceAndImplName[]   = "com.sun.star.configuration.backend.SingleBackend" ;

// -------------------------------------------------------------------------
static AsciiServiceName const k_BackendServiceNames [] =
{
    k_DefaultBackendServiceAndImplName,
    k_GenericBackendServiceAndImplName,
    0
};
static AsciiServiceName const k_SingleBackendServiceNames [] =
{
    k_DefaultSingleBackendServiceAndImplName,
    k_GenericSingleBackendServiceAndImplName,
    0
};
// -------------------------------------------------------------------------
static const ServiceRegistrationInfo k_DefaultBackendServiceInfo =
{
    k_DefaultBackendServiceAndImplName,
    k_BackendServiceNames
};
static const ServiceRegistrationInfo k_DefaultSingleBackendServiceInfo =
{
    k_DefaultSingleBackendServiceAndImplName,
    k_SingleBackendServiceNames
};
// -------------------------------------------------------------------------
static const ServiceRegistrationInfo k_GenericBackendServiceInfo =
{
    k_GenericBackendServiceAndImplName,
    k_BackendServiceNames + 1
};
static const ServiceRegistrationInfo k_GenericSingleBackendServiceInfo =
{
    k_GenericSingleBackendServiceAndImplName,
    k_SingleBackendServiceNames + 1
};
// -------------------------------------------------------------------------
static const SingletonRegistrationInfo k_DefaultBackendSingletonInfo =
{
    K_DefaultBackendSingletonName,
    k_DefaultBackendServiceAndImplName,
    k_DefaultBackendServiceAndImplName,
    & k_GenericBackendServiceInfo
};
static const SingletonRegistrationInfo k_DefaultSingleBackendSingletonInfo =
{
    K_DefaultSingleBackendSingletonName,
    k_DefaultSingleBackendServiceAndImplName,
    k_DefaultSingleBackendServiceAndImplName,
    & k_GenericSingleBackendServiceInfo
};
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
const SingletonRegistrationInfo * getDefaultBackendSingletonInfo()
{
    return & k_DefaultBackendSingletonInfo;
}
// -------------------------------------------------------------------------
const SingletonRegistrationInfo * getDefaultSingleBackendSingletonInfo()
{
    return & k_DefaultSingleBackendSingletonInfo;
}
// -------------------------------------------------------------------------

const ServiceRegistrationInfo   * getDefaultBackendServiceInfo()
{
    return & k_DefaultBackendServiceInfo;
}
// -------------------------------------------------------------------------
const ServiceRegistrationInfo   * getDefaultSingleBackendServiceInfo()
{
    return & k_DefaultSingleBackendServiceInfo;
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

uno::Reference<uno::XInterface> SAL_CALL
    getDefaultBackendSingleton( CreationContext const& xContext )
{
    OSL_ENSURE( xContext.is(), "ERROR: NULL context has no singletons" );

    UnoContextTunnel aTunnel;
    aTunnel.passthru( xContext );

    uno::Reference<uno::XInterface> xResult;

    if (xContext.is())
    try
    {
        xContext->getValueByName(SINGLETON(K_DefaultBackendSingletonName))
            >>= xResult;
    }
    catch (uno::Exception & )
    {
        // to do: really use the tunneled failure when that is set properly
        if ( aTunnel.recoverFailure(true).hasValue() )
        {
            // have a failure, but can't recover it
            // -> try to regenerate
            instantiateDefaultBackend(xContext);

            OSL_ENSURE(false, "Cannot recreate configuration backend instantiation failure - using generic error");
        }
        // cannot recover any failure
        throw;
    }

    return xResult;
}
// -------------------------------------------------------------------------

uno::Reference<uno::XInterface> SAL_CALL
    getDefaultSingleBackendSingleton( CreationContext const& xContext )
{
    OSL_ENSURE( xContext.is(), "ERROR: NULL context has no singletons" );

    UnoContextTunnel aTunnel;
    aTunnel.passthru( xContext );

    uno::Reference<uno::XInterface> xResult;

    if (xContext.is())
    try
    {
        xContext->getValueByName(SINGLETON(K_DefaultSingleBackendSingletonName))
            >>= xResult;
    }
    catch (uno::Exception & )
    {
        // to do: really use the tunneled failure when that is set properly
        if ( aTunnel.recoverFailure(true).hasValue() )
        {
            // have a failure, but can't recover it
            // -> try to regenerate
            instantiateDefaultSingleBackend(xContext);

            OSL_ENSURE(false, "Cannot recreate configuration backend instantiation failure - using generic error");
        }
        // cannot recover any failure
        throw;
    }

    return xResult;
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

typedef BackendFactory::CreationContext CreationContext;
typedef uno::Sequence< uno::Any >       UnoInitArgs;
// -------------------------------------------------------------------------

static
UnoInitArgs createInitArgs(ContextReader const & _aContext)
{
    OSL_ASSERT(_aContext.hasBootstrapContext());
    uno::Sequence< uno::Any > aResult( 1 );
    aResult[0] <<= _aContext.getBootstrapContext();
    return aResult;
}
// -------------------------------------------------------------------------

static
inline
uno::Reference< uno::XInterface > createService(ContextReader const & _aCtx, UnoInitArgs const & _aInitArgs, OUString const & _aSvc)
{
    uno::Reference< lang::XMultiComponentFactory > xFactory = _aCtx.getServiceManager();
    OSL_ENSURE(xFactory.is(),"ERROR: ComponentContext has no service manager\n");
    if (!xFactory.is()) throw uno::RuntimeException( OUString::createFromAscii("ERROR: ComponentContext has no service manager\n"), NULL );
    return xFactory->createInstanceWithArgumentsAndContext( _aSvc, _aInitArgs, _aCtx.getBaseContext());
}
// -------------------------------------------------------------------------

typedef uno::Reference< backenduno::XSingleBackend >    UnoSingleBackend;
typedef uno::Reference< backenduno::XBackend >          UnoBackend;

static
UnoBackend wrapSingleBackend(ContextReader const & _aSettings, UnoInitArgs const & _aInitArgs, UnoSingleBackend const & _xWrappedBackend)
{
    OSL_ASSERT(_aSettings.hasUnoBackendWrapper() || _aSettings.hasBootstrapContext());

    OUString aWrapperSvc = _aSettings.hasUnoBackendWrapper() ?
                                _aSettings.getUnoBackendWrapper() :
                                OUString::createFromAscii(k_DefaultBackendWrapper);

    OSL_ENSURE (aWrapperSvc.getLength(), "ERROR: No wrapper service for wrapped configuration");
    OSL_ENSURE (_xWrappedBackend.is(), "ERROR: No backend to wrap for wrapped configuration");

    sal_Int32 const nBaseArgsCount = _aInitArgs.getLength();
    UnoInitArgs aExtendedArgs( _aInitArgs );
    aExtendedArgs.realloc( nBaseArgsCount + 1 );
    aExtendedArgs[nBaseArgsCount] <<= _xWrappedBackend;

    return UnoBackend::query( createService(_aSettings,aExtendedArgs,aWrapperSvc) );
}
// -------------------------------------------------------------------------

static
UnoBackend createOfflineBackend(ContextReader const & _aSettings, UnoInitArgs const & _aInitArgs)
{
    OSL_ASSERT(_aSettings.hasUnoBackendWrapper() || _aSettings.hasBootstrapContext());

    UnoBackend xResult;
    if ( _aSettings.hasUnoBackendWrapper() )
    {
        OUString const aWrapperSvc = _aSettings.getUnoBackendWrapper();

        xResult = UnoBackend::query( createService(_aSettings,_aInitArgs,aWrapperSvc) );
    }

    return xResult;
}
// -------------------------------------------------------------------------

static
uno::Reference< uno::XInterface > createRealBackend(ContextReader const & _aSettings, UnoInitArgs const & _aInitArgs)
{
    OSL_ASSERT(_aSettings.hasUnoBackendService() || _aSettings.hasBootstrapContext());

    OUString const aBackendServiceName = _aSettings.hasUnoBackendService() ?
                                        _aSettings.getUnoBackendService() :
                                        OUString::createFromAscii(k_DefaultBackendService);

    uno::Reference< uno::XInterface > xResult =
        createService(_aSettings,_aInitArgs,aBackendServiceName);

    return xResult;
}
// -------------------------------------------------------------------------

static
UnoBackend createOnlineBackend(ContextReader const & _aSettings, UnoInitArgs const & _aInitArgs)
{
    OSL_ENSURE( _aSettings.isUnoBackend(), "ERROR - BackendFactory: For legacy backends use createSessionBackend()");

    UnoBackend xResult;

    uno::Reference< uno::XInterface > xRealBackend = createRealBackend(_aSettings,_aInitArgs);

    if (_aSettings.hasUnoBackendWrapper())
    {
        // try wrapping a single backend
        UnoSingleBackend xSingleRealBackend( xRealBackend, uno::UNO_QUERY);
        if (xSingleRealBackend.is())
            xResult = wrapSingleBackend(_aSettings,_aInitArgs,xSingleRealBackend);

        // if we don't have one, try using it as unwrapped backend
        else
            xResult.set(xRealBackend, uno::UNO_QUERY);
    }
    else
    {
         // look for a direct implementation of XBackend
        xResult.set(xRealBackend, uno::UNO_QUERY);
        if (!xResult.is())
        {
            // try the default wrapper if we only have a single backend
            UnoSingleBackend xSingleRealBackend( xRealBackend, uno::UNO_QUERY);
            if (xSingleRealBackend.is())
                xResult = wrapSingleBackend(_aSettings,_aInitArgs,xSingleRealBackend);

            else
                OSL_ENSURE( !xRealBackend.is(), "Configuration Backend implements no known backend interface" );
        }
    }

    return xResult;
}
// -------------------------------------------------------------------------

static UnoBackend createUnoBackend(CreationContext const& _xCtx)
{
    ContextReader aSettings(_xCtx);
    OSL_ENSURE( aSettings.isUnoBackend(), "ERROR - BackendFactory: Legacy backends are not supported any more");

    UnoInitArgs aArguments = createInitArgs(aSettings);

    sal_Bool bOffline = aSettings.hasOfflineSetting() ? aSettings.getOfflineSetting() : !aSettings.hasUnoBackendService();

    UnoBackend xResult;

    if (!bOffline)
        xResult = createOnlineBackend (aSettings,aArguments);

    if (!xResult.is())
        xResult = createOfflineBackend(aSettings,aArguments);

    return xResult;
}
// -------------------------------------------------------------------------
// to do: tunnel and raise fully typed exception information (and use it in the get..Singleton wrappers)
    #define TUNNEL_ALL_EXCEPTIONS()     \
        WRAP_CONFIGBACKEND_CREATION_EXCEPTIONS1( UnoContextTunnel::tunnelFailure, true )

// -------------------------------------------------------------------------

uno::Reference<uno::XInterface> SAL_CALL instantiateDefaultBackend( CreationContext const& xTargetContext )
{
    CreationContext xContext = UnoContextTunnel::recoverContext(xTargetContext);

    try
    {
        return createUnoBackend(xContext);
    }
    TUNNEL_ALL_EXCEPTIONS()
}
// -------------------------------------------------------------------------
uno::Reference<uno::XInterface> SAL_CALL instantiateDefaultSingleBackend( CreationContext const& xTargetContext )
{
    CreationContext xContext = UnoContextTunnel::recoverContext(xTargetContext);

    ContextReader aSettings( xContext );

    UnoInitArgs aArguments = createInitArgs(aSettings);

    try
    {
        return createRealBackend(aSettings,aArguments);
    }
    TUNNEL_ALL_EXCEPTIONS()
}

// -------------------------------------------------------------------------

UnoBackend BackendFactory::getUnoBackend()
{
    return UnoBackend::query( getDefaultBackendSingleton(m_xCtx) );
}
// -------------------------------------------------------------------------

rtl::Reference<IMergedDataProvider> BackendFactory::createBackend()
{
    rtl::Reference< IMergedDataProvider > xBackend;

    UnoBackend xBackendService = this->getUnoBackend();

    if (xBackendService.is())
        xBackend = new BackendAccess(xBackendService, m_xCtx);

    return xBackend;
}
// -------------------------------------------------------------------------

BackendFactory BackendFactory::instance(CreationContext const & _xCtx)
{
    return BackendFactory(_xCtx);
}

//-----------------------------------------------------------------------------
    } // namespace
//-----------------------------------------------------------------------------
} // namespace
