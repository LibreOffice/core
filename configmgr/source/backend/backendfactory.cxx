/*************************************************************************
 *
 *  $RCSfile: backendfactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cyrillem $ $Date: 2002-06-13 16:45:55 $
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

#ifndef CONFIGMGR_BOOTSTRAPCONTEXT_HXX_
#include "bootstrapcontext.hxx"
#endif
#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_BACKENDACCESS_HXX_
#include "backendaccess.hxx"
#endif
#ifndef CONFIGMGR_BACKENDWRAP_HXX
#include "backendwrap.hxx"
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
const sal_Char k_DefaultBackendService[] = "com.sun.star.configuration.backend.Backend";
// -------------------------------------------------------------------------
typedef uno::Sequence< uno::Any > UnoInitArgs;

static
UnoInitArgs createInitArgs(ConnectionSettings const & _aSettings)
{
    uno::Reference< uno::XCurrentContext > xBootstrapArgs = new BootstrapContext(_aSettings.getUnoSettings());
    uno::Sequence< uno::Any > aResult( 1 );
    aResult[0] <<= xBootstrapArgs;
    return aResult;
}
// -------------------------------------------------------------------------

typedef BackendFactory::CreationContext CreationContext;

static
inline
uno::Reference< uno::XInterface > createService(CreationContext const & _xCtx, UnoInitArgs const & _aInitArgs, OUString const & _aSvc)
{
    OSL_ASSERT(_xCtx.is());
    return _xCtx->createInstanceWithArguments( _aSvc, _aInitArgs);
}
// -------------------------------------------------------------------------

typedef uno::Reference< backenduno::XSingleBackend >    UnoSingleBackend;
typedef uno::Reference< backenduno::XBackend >          UnoBackend;

static
UnoBackend wrapSingleBackend(ConnectionSettings const & _aSettings, CreationContext const & _xCtx, UnoInitArgs const & _aInitArgs, UnoSingleBackend const & _xWrappedBackend)
{
    OUString aWrapperSvc = _aSettings.hasUnoBackendWrapper() ?
                                _aSettings.getUnoBackendWrapper() :
                                OUString::createFromAscii(k_DefaultBackendWrapper);

    OSL_ENSURE (aWrapperSvc.getLength(), "ERROR: No wrapper service for wrapped configuration");
    OSL_ENSURE (_xWrappedBackend.is(), "ERROR: No backend to wrap for wrapped configuration");

    sal_Int32 const nBaseArgsCount = _aInitArgs.getLength();
    UnoInitArgs aExtendedArgs( _aInitArgs );
    aExtendedArgs.realloc( nBaseArgsCount + 1 );
    aExtendedArgs[nBaseArgsCount] <<= _xWrappedBackend;

    return UnoBackend::query( createService(_xCtx,aExtendedArgs,aWrapperSvc) );
}
// -------------------------------------------------------------------------

static
UnoBackend createOfflineBackend(ConnectionSettings const & _aSettings, CreationContext const & _xCtx, UnoInitArgs const & _aInitArgs)
{
    UnoBackend xResult;
    if (_aSettings.hasOfflineSetting() && _aSettings.getOfflineSetting())
    {
        OSL_ENSURE(_aSettings.hasUnoBackendWrapper(), "ERROR: No offline service for offline configuration");
        if (_aSettings.hasUnoBackendWrapper())
        {
            xResult.set( createService(_xCtx,_aInitArgs,_aSettings.getUnoBackendWrapper()), uno::UNO_QUERY );

            OSL_ENSURE(xResult.is(), "ERROR: Cannot create offline service");
        }
    }
    return xResult;
}
// -------------------------------------------------------------------------

static
UnoBackend createOnlineBackend(ConnectionSettings const & _aSettings, CreationContext const & _xCtx, UnoInitArgs const & _aInitArgs)
{
    OSL_ENSURE( _aSettings.isUnoBackend(), "ERROR - BackendFactory: For legacy backends use createSessionBackend()");

    UnoBackend xResult;

    if (_aSettings.hasUnoBackendService())
    {
        uno::Reference< uno::XInterface > xRealBackend = createService(_xCtx,_aInitArgs,_aSettings.getUnoBackendService());
        xResult.set(xRealBackend, uno::UNO_QUERY);

        if (!xResult.is())
        {
            UnoSingleBackend xSingleRealBackend( xRealBackend, uno::UNO_QUERY);
            if (xSingleRealBackend.is())
                xResult = wrapSingleBackend(_aSettings,_xCtx,_aInitArgs,xSingleRealBackend);

            else
                OSL_ENSURE( !xRealBackend.is(), "Configuration Backend implements no known interface" );
        }
    }
    else
    {
        OUString aBackendFallback( RTL_CONSTASCII_USTRINGPARAM(k_DefaultBackendService) );
        xResult.set( createService(_xCtx,_aInitArgs,aBackendFallback), uno::UNO_QUERY );
    }
    return xResult;
}
// -------------------------------------------------------------------------

static
UnoBackend createUnoBackend(ConnectionSettings const & _aSettings, CreationContext const & _xCtx)
{
    UnoInitArgs aArguments = createInitArgs(_aSettings);

    UnoBackend xResult = createOfflineBackend(_aSettings,_xCtx,aArguments);

    // not offline or no offline implementation available
    if (!xResult.is())
        xResult = createOnlineBackend(_aSettings,_xCtx,aArguments);

    return xResult;
}
// -------------------------------------------------------------------------

rtl::Reference<IMergedDataProvider>
    BackendFactory::createBackend(ConnectionSettings const & _aSettings, CreationContext const & _xCtx)
{
    OSL_ENSURE( _aSettings.isUnoBackend(), "ERROR - BackendFactory: For legacy backends use createSessionBackend()");

    rtl::Reference< IMergedDataProvider > xBackend;

    UnoBackend xBackendService = createUnoBackend(_aSettings, _xCtx);

    if (xBackendService.is())
        xBackend = new BackendAccess(xBackendService, _xCtx);

    return xBackend;
}
// -------------------------------------------------------------------------

rtl::Reference<IMergedDataProvider>
    BackendFactory::createSessionBackend(IConfigSession * _pSession,
                                            TypeConverterRef const & _xTCV)
{
    rtl::Reference< IMergedDataProvider > xBackend;

    if (_pSession)
        xBackend = wrapSession(*_pSession,_xTCV);

    return xBackend;
}
// -------------------------------------------------------------------------

BackendFactory & BackendFactory::instance()
{
    static BackendFactory aStaticFactory;
    return aStaticFactory;
}

//-----------------------------------------------------------------------------
    } // namespace
//-----------------------------------------------------------------------------
} // namespace
