/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: singlebackendadapter.cxx,v $
 * $Revision: 1.14 $
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
#include "precompiled_configmgr.hxx"
#include "singlebackendadapter.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif // CONFIGMGR_API_FACTORY_HXX_
#include "serviceinfohelper.hxx"
#include <com/sun/star/configuration/backend/XMultiLayerStratum.hpp>
#include <com/sun/star/configuration/CannotLoadConfigurationException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
namespace configmgr { namespace backend {

//==============================================================================

SingleBackendAdapter::SingleBackendAdapter(
        const uno::Reference<uno::XComponentContext>& xContext)
        : cppu::WeakComponentImplHelper5< backenduno::XBackend, backenduno::XBackendEntities, backenduno::XSchemaSupplier, lang::XInitialization, lang::XServiceInfo >(mMutex), mFactory(xContext->getServiceManager(),uno::UNO_QUERY) {
}
//------------------------------------------------------------------------------

SingleBackendAdapter::~SingleBackendAdapter(void) {}
//------------------------------------------------------------------------------

void SAL_CALL SingleBackendAdapter::initialize(
        const uno::Sequence<uno::Any>& aParameters)
    throw (uno::RuntimeException, uno::Exception) {

    uno::Any const * const pParams = aParameters.getConstArray();
    sal_Int32 nCount = aParameters.getLength();

    for (sal_Int32 ix = 0; ix < nCount; ++ix)
    {
        if (pParams[ix] >>= mBackend) break;
    }

    if (!mBackend.is())
    {
        throw com::sun::star::configuration::CannotLoadConfigurationException(
            rtl::OUString::createFromAscii("Online SingleBackend Adapter: Cannot operate without real (Single)Backend"),
            *this);
    }
}
//------------------------------------------------------------------------------
static inline rtl::OUString getSingleLayerDummyEntity()
{ return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*")); }
//------------------------------------------------------------------------------
bool SingleBackendAdapter::checkOkState()
{
    if (!mBackend.is())
    {
        if(rBHelper.bDisposed)
        {
            throw lang::DisposedException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "SingleBackendAdapter: Backend already disposed")),*this);
        }
        else
        {
            throw backenduno::BackendAccessException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "SingleBackendAdapter: Object was never Initialised")),*this,uno::Any() );
        }
    }
    return true;
}

//------------------------------------------------------------------------------

// XBackendEntities
rtl::OUString SAL_CALL
    SingleBackendAdapter::getOwnerEntity(  )
        throw (uno::RuntimeException)
{
    if (mBackend.is())
    {
        uno::Reference< backenduno::XBackendEntities > xEntities( mBackend, uno::UNO_REF_QUERY_THROW );

        return xEntities->getOwnerEntity();
    }
    else
    {
        throw uno::RuntimeException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "SingleBackendAdapter: Object was never Initialised")),*this);
    }
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
    SingleBackendAdapter::getAdminEntity(  )
        throw (uno::RuntimeException)
{
    if (mBackend.is())
    {
        uno::Reference< backenduno::XBackendEntities > xEntities( mBackend, uno::UNO_REF_QUERY_THROW );
        return xEntities->getAdminEntity();
    }
    else
    {
        throw uno::RuntimeException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "SingleBackendAdapter: Object was never Initialised")),*this);
    }
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL
    SingleBackendAdapter::supportsEntity( const rtl::OUString& aEntity )
    throw (backenduno::BackendAccessException, uno::RuntimeException)
{
    if (checkOkState())
    {
        uno::Reference< backenduno::XBackendEntities > xEntities( mBackend, uno::UNO_REF_QUERY_THROW );
        return xEntities->supportsEntity(aEntity);
    }
    return false;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL
    SingleBackendAdapter::isEqualEntity( const rtl::OUString& aEntity, const rtl::OUString& aOtherEntity )
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    if (checkOkState())
    {
        uno::Reference< backenduno::XBackendEntities > xEntities( mBackend, uno::UNO_REF_QUERY_THROW );
        return xEntities->isEqualEntity(aEntity,aOtherEntity);
    }
    return false;
}
//------------------------------------------------------------------------------

uno::Reference<backenduno::XSchema> SAL_CALL
    SingleBackendAdapter::getComponentSchema(const rtl::OUString& aComponent)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    if (checkOkState())
    {
        return mBackend->getComponentSchema(aComponent) ;
    }
    return false;
}
//------------------------------------------------------------------------------

uno::Sequence<uno::Reference<backenduno::XLayer> > SAL_CALL
    SingleBackendAdapter::listOwnLayers(const rtl::OUString& aComponent)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    if (checkOkState())
    {
        return listLayers(aComponent, this->getOwnerEntity()) ;
    }
    return false;
}
//------------------------------------------------------------------------------

uno::Reference<backenduno::XUpdateHandler> SAL_CALL
    SingleBackendAdapter::getOwnUpdateHandler(const rtl::OUString& aComponent)
        throw (backenduno::BackendAccessException,
                lang::NoSupportException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    if (checkOkState())
    {
        return getUpdateHandler(aComponent, this->getOwnerEntity()) ;
    }
    return false;
}
//------------------------------------------------------------------------------

uno::Sequence<uno::Reference<backenduno::XLayer> > SAL_CALL
    SingleBackendAdapter::listLayers(const rtl::OUString& aComponent,
                                          const rtl::OUString& aEntity)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    if (checkOkState())
    {
        uno::Reference< backenduno::XMultiLayerStratum > xBackend( mBackend, uno::UNO_REF_QUERY_THROW );

        uno::Sequence<uno::Reference<backenduno::XLayer> > retCode =
            xBackend->getLayers(xBackend->listLayerIds(aComponent, aEntity),
                            rtl::OUString()) ;

        // There might be non-existent layers in the list if there's no
        // actual data associated to a given layer id. Hence we have to
        // compress the list.
        sal_Int32 maxLayer = 0 ;

        for (sal_Int32 i = 0 ; i < retCode.getLength() ; ++ i)
        {
            if (retCode [i].is())
            {
                if (i != maxLayer) { retCode [maxLayer] = retCode [i] ; }
                ++ maxLayer ;
            }
        }
        retCode.realloc(maxLayer) ;
        return retCode ;
    }
    return false;
}
//------------------------------------------------------------------------------

uno::Reference<backenduno::XUpdateHandler> SAL_CALL
    SingleBackendAdapter::getUpdateHandler(const rtl::OUString& aComponent,
                                                const rtl::OUString& aEntity)
        throw (backenduno::BackendAccessException,
                lang::NoSupportException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    if (checkOkState())
    {
        uno::Reference< backenduno::XMultiLayerStratum > xBackend( mBackend, uno::UNO_REF_QUERY_THROW );

        uno::Sequence<uno::Any> arguments(1) ;

        arguments [0] <<= xBackend->getUpdatableLayer(
                                        xBackend->getUpdateLayerId(aComponent,
                                                                   aEntity)) ;
        uno::Reference< uno::XInterface > xHandler;
        try
        {
            const rtl::OUString kUpdateMerger(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.backend.LayerUpdateMerger")) ;

            xHandler = mFactory->createInstanceWithArguments(kUpdateMerger, arguments);
        }
        catch (uno::RuntimeException & )
        {throw;}
        catch (uno::Exception & e)
        {
            const rtl::OUString sMessage(RTL_CONSTASCII_USTRINGPARAM(
                "Configuration SingleBackendAdapter: Cannot create UpdateMerger - error message: ")) ;
            throw uno::RuntimeException(sMessage.concat(e.Message),*this);
        }

        return uno::Reference<backenduno::XUpdateHandler>(xHandler, uno::UNO_REF_QUERY_THROW) ;
    }
    return false;
}
//------------------------------------------------------------------------------

static const sal_Char * const kBackendService = "com.sun.star.configuration.backend.Backend" ;
static const sal_Char * const kAdapterService = "com.sun.star.configuration.backend.BackendAdapter" ;
static const sal_Char * const kOnlineService  = "com.sun.star.configuration.backend.OnlineBackend" ;

static const sal_Char * const kImplementation =
                "com.sun.star.comp.configuration.backend.SingleBackendAdapter" ;

static sal_Char const * const kServiceNames [] =
{
    kOnlineService,
    kAdapterService,
    0,
    kBackendService,
    0
} ;
static const ServiceImplementationInfo kServiceInfo =
{
    kImplementation,
    kServiceNames,
    kServiceNames + 3
} ;

const ServiceRegistrationInfo *getSingleBackendAdapterServiceInfo()
{
    return getRegistrationInfo(&kServiceInfo) ;
}

uno::Reference<uno::XInterface> SAL_CALL
    instantiateSingleBackendAdapter(const uno::Reference< uno::XComponentContext >& xContext)
{
    return *new SingleBackendAdapter(xContext) ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
    SingleBackendAdapter::getImplementationName(void)
        throw (uno::RuntimeException)
{
    return ServiceInfoHelper(&kServiceInfo).getImplementationName() ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL
    SingleBackendAdapter::supportsService(const rtl::OUString& aServiceName)
        throw (uno::RuntimeException)
{
    return  ServiceInfoHelper(&kServiceInfo).supportsService(aServiceName) ;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL
    SingleBackendAdapter::getSupportedServiceNames(void)
        throw (uno::RuntimeException)
{
    return ServiceInfoHelper(&kServiceInfo).getSupportedServiceNames() ;
}
//------------------------------------------------------------------------------

} } // configmgr.backend

