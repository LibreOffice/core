/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: multistratumbackend.cxx,v $
 * $Revision: 1.13 $
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

#include "multistratumbackend.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif // CONFIGMGR_API_FACTORY_HXX_
#include "serviceinfohelper.hxx"
#include "backendstratalistener.hxx"

#ifndef _CONFIGMGR_BOOTSTRAP_HXX
#include "bootstrap.hxx"
#endif
#include "simpleinteractionrequest.hxx"
#include "configinteractionhandler.hxx"
#include <com/sun/star/configuration/backend/XMultiLayerStratum.hpp>
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#include <com/sun/star/configuration/backend/StratumCreationException.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <rtl/ustrbuf.hxx>

#include <cppuhelper/exc_hlp.hxx>

#include <stdio.h>

//==============================================================================
#define OU2A(rtlOUString)   (::rtl::OUStringToOString((rtlOUString), RTL_TEXTENCODING_ASCII_US).getStr())
//==============================================================================

namespace {

namespace css = com::sun::star;

}

namespace configmgr { namespace backend {

//==============================================================================
static const rtl::OUString kSchemaServiceParam(
    RTL_CONSTASCII_USTRINGPARAM( CONTEXT_ITEM_PREFIX_ "SchemaSupplier"));

static const rtl::OUString kStrataServiceParam(
    RTL_CONSTASCII_USTRINGPARAM( CONTEXT_ITEM_PREFIX_ "Strata"));

static const rtl::OUString kEntity(
    RTL_CONSTASCII_USTRINGPARAM(CONTEXT_ITEM_PREFIX_"EntityLayer")) ;

static const rtl::OUString kAdminModeFlag(
    RTL_CONSTASCII_USTRINGPARAM(CONTEXT_ITEM_ADMINFLAG)) ;

static const rtl::OUString kThisOwnerEntity(
    RTL_CONSTASCII_USTRINGPARAM("<ThisOwnerEntity>")) ;

//------------------------------------------------------------------------------
bool checkOptionalArg(rtl::OUString& aArg)
    {
        if (aArg.getLength() && aArg[0] == sal_Unicode('?'))
        {
            aArg = aArg.copy(1);
            return true;
        }
        else
        {
            return false;
        }
    }
//------------------------------------------------------------------------------
void parseStrataInfo(const rtl::OUString aServiceList,
                     std::vector< std::pair<rtl::OUString, rtl::OUString> >& aServiceInfoList,
                     const uno::Reference<uno::XInterface>& pContext)
{
    sal_Int32 nNextToken =0;
    sal_Int32 nLength = aServiceList.getLength();

    do
    {
        rtl::OUString aServiceName =aServiceList.getToken(0, ':',nNextToken);
        if((nNextToken ==-1)||(aServiceName.getLength()==0))
        {
             throw backenduno::BackendSetupException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Malformed Strata Service specified")),
                        pContext, uno::Any()) ;
        }
        rtl::OUString aServiceData = aServiceList.getToken(0, ';',nNextToken);

        std::pair<rtl::OUString, rtl::OUString> aServiceInfo(aServiceName,aServiceData);
        aServiceInfoList.push_back(aServiceInfo);
    }
    while (nNextToken >= 0 && nNextToken < nLength ) ;
}
//------------------------------------------------------------------------------
MultiStratumBackend::MultiStratumBackend(
        const uno::Reference<uno::XComponentContext>& xContext)
        : cppu::WeakComponentImplHelper7< backenduno::XBackend, backenduno::XBackendEntities, backenduno::XVersionedSchemaSupplier, backenduno::XBackendChangesNotifier, backenduno::XBackendChangesListener, lang::XInitialization, lang::XServiceInfo >(mMutex), mFactory(xContext->getServiceManager(),uno::UNO_QUERY_THROW)
          ,mListenerList()
{

}
//------------------------------------------------------------------------------

MultiStratumBackend::~MultiStratumBackend()
{
}
//------------------------------------------------------------------------------
bool MultiStratumBackend::checkOkState()
{
    if (!mSchemaSupplier.is())
    {
        if(rBHelper.bDisposed)
        {
            throw lang::DisposedException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "MultiStratumBackend: Backends already disposed")),*this);
        }
        else
        {
            throw uno::RuntimeException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "MultiStratumBackend: Object was never Initialised")),*this);
        }
    }
    return true;
}
//------------------------------------------------------------------------------

void SAL_CALL MultiStratumBackend::initialize(
        const uno::Sequence<uno::Any>& aParameters)
    throw (uno::RuntimeException, uno::Exception,
           css::configuration::InvalidBootstrapFileException,
           backenduno::BackendSetupException) {

    if (aParameters.getLength() == 0) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "No parameters provided to MultiStratumBackend")),
                *this, 0) ;
    }

    uno::Reference<uno::XComponentContext> context ;

    for (sal_Int32 i = 0 ; i < aParameters.getLength() ; ++ i) {
        if (aParameters [i] >>= context) { break ; }
    }


    try
    {
        //Initialize Backends
        initializeSchemaSupplier (context);
        initializeBackendStrata(context);

        sal_Bool bAdminMode = false;
        context->getValueByName(kAdminModeFlag) >>= bAdminMode;

        if (bAdminMode)
        {
            // find given entity
            rtl::OUString sDefaultEntity;
            if ( (context->getValueByName(kEntity) >>= sDefaultEntity) && sDefaultEntity.getLength() )
            {
                for (sal_uInt32 i = 0; i < mBackendStrata.size(); i++)
                {
                    uno::Reference< backenduno::XBackendEntities > xEntities( mBackendStrata[i], uno::UNO_QUERY );
                    if (xEntities.is()&& xEntities->supportsEntity(sDefaultEntity))
                    {
                        mBackendStrata.resize(i+1);
                        mOwnerEntity = sDefaultEntity;
                        break;
                    }
                }
            }
            else
            {
                mBackendStrata.resize(1);
            }
         }

         if(mOwnerEntity.getLength()==0)
         {
             uno::Reference< backenduno::XBackendEntities > xEntities(
                 mBackendStrata[mBackendStrata.size()-1], uno::UNO_QUERY );
             if (xEntities.is())
             {
                 mOwnerEntity = xEntities->getOwnerEntity();
             }
             else
             {
                 mOwnerEntity = kThisOwnerEntity;
             }
         }
         mStrataListener = new BackendStrataListener(*this);

    }
    catch(uno::Exception& )
    {
        mSchemaSupplier.clear();
        mBackendStrata.clear() ;

        throw;
    }

}
//------------------------------------------------------------------------------
void  MultiStratumBackend::initializeSchemaSupplier(const uno::Reference<uno::XComponentContext>& aContext)
{

    rtl::OUString aServiceName;

    aContext->getValueByName(kSchemaServiceParam) >>= aServiceName;
    uno::Sequence< uno::Any > aInitArgs( 1 );
    aInitArgs[0] <<= aContext;
    mSchemaSupplier = uno::Reference<backenduno::XSchemaSupplier>::query(mFactory->createInstanceWithArguments(aServiceName,aInitArgs)) ;
    if (!mSchemaSupplier.is())
    {
         throw backenduno::BackendSetupException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "MultiStratumBackend: Could not create Schema Supplier service")),
                        *this, uno::Any()) ;

    }
}
//------------------------------------------------------------------------------
static
bool approveRecovery(const backenduno::StratumCreationException & aError)
{
    sal_uInt32 const k_supported_choices = apihelper::CONTINUATION_APPROVE ; //| apihelper::CONTINUATION_DISAPPROVE;

    sal_uInt32 chosen = apihelper::CONTINUATION_UNKNOWN;

    apihelper::ConfigurationInteractionHandler handler;
    try {
        uno::Reference< css::task::XInteractionHandler > h(handler.get());
        if (h.is()) {
            rtl::Reference< apihelper::SimpleInteractionRequest > req(
                new apihelper::SimpleInteractionRequest(
                    uno::makeAny(aError), k_supported_choices));
            h->handle(req.get());
            chosen = req->getResponse();
        }
    } catch (uno::Exception & e) {
        OSL_TRACE("Warning - Configuration: Interaction handler failed: [%s]\n", OU2A(e.Message));
    }

    switch (chosen)
    {
    case apihelper::CONTINUATION_APPROVE:      return true;
    case apihelper::CONTINUATION_DISAPPROVE:   return false;
    case apihelper::CONTINUATION_UNKNOWN:      break;

    default: OSL_ENSURE(false,"Unsolicited continuation chosen"); break;
    }
    // no choice available - default: disapprove
    return false;
}
//------------------------------------------------------------------------------
void MultiStratumBackend::initializeBackendStrata(const uno::Reference<uno::XComponentContext>& aContext)
{

    rtl::OUString sStrata;
    //Get Strata
    aContext->getValueByName(kStrataServiceParam) >>= sStrata;
    if(sStrata.getLength()==0)
    {
        throw backenduno::BackendSetupException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "MultiStratumBackend: No Strata Services Specified")),
                        *this, uno::Any()) ;

    }

    //need to parse the Strata to extract service names and data location
    std::vector< std::pair<rtl::OUString, rtl::OUString> > aServiceInfoList;
    parseStrataInfo(sStrata,aServiceInfoList,*this);

    for (std::vector< std::pair<rtl::OUString, rtl::OUString> >::const_iterator it = aServiceInfoList.begin(); it != aServiceInfoList.end(); ++it)
    {
        uno::Sequence< uno::Any > aInitArgs( 1 );
        rtl::OUString sServiceName = it->first;
        const rtl::OUString& sServiceData = it->second;
        aInitArgs[0] <<= sServiceData;
        uno::Reference <uno::XInterface> xBackend;
        bool bOptional = checkOptionalArg(sServiceName);

        try
        {
            xBackend= mFactory->createInstanceWithArguments(sServiceName,aInitArgs);
        }
        catch (uno::Exception& exception)
        {
            if(!bOptional)
            {
                static const sal_Char sErrContext[] = "MultiStratumBackend: Could not create Backend Stratum Service: ";
                rtl::OUString const sContext(RTL_CONSTASCII_USTRINGPARAM(sErrContext));
                rtl::OUString const sMessage = sContext.concat(exception.Message);

                backenduno::StratumCreationException error(sMessage,*this,
                                                            ::cppu::getCaughtException(),
                                                            sServiceName,sServiceData);
                if (!approveRecovery(error))
                    throw error;
            }

        }
        if (xBackend.is())
        {
            mBackendStrata.push_back(xBackend) ;
        }
    }
}
//---------------------------------------------------------------------------------------------
// XBackendEntities
rtl::OUString SAL_CALL
    MultiStratumBackend::getOwnerEntity(  )
        throw (uno::RuntimeException)
{

    if (checkOkState())
    {
        return mOwnerEntity;
    }
    return rtl::OUString();
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
    MultiStratumBackend::getAdminEntity()
        throw (uno::RuntimeException)
{
    osl::MutexGuard aGuard(mMutex);
    if (checkOkState())
    {
        uno::Reference< backenduno::XBackendEntities > xEntities( mBackendStrata[0], uno::UNO_QUERY );
        if(xEntities.is())
        {
            return xEntities->getAdminEntity();
        }
        else
        {
            return rtl::OUString();
        }
    }
    return rtl::OUString();
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL
    MultiStratumBackend::supportsEntity( const rtl::OUString& aEntity )
    throw (backenduno::BackendAccessException, uno::RuntimeException)
{

    osl::MutexGuard aGuard(mMutex);
    if (checkOkState())
    {
        for (std::vector< uno::Reference <uno::XInterface> >::const_iterator it =  mBackendStrata.begin(); it !=  mBackendStrata.end(); ++it)
        {
            uno::Reference< backenduno::XBackendEntities > xEntities( *it, uno::UNO_QUERY );
            if (xEntities.is())
            {
                if( xEntities->supportsEntity(aEntity))
                    return true;
            }
        }
        return false;
    }
    return false;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL
    MultiStratumBackend::isEqualEntity( const rtl::OUString& aEntity, const rtl::OUString& aOtherEntity )
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    osl::MutexGuard aGuard(mMutex);
    if (aEntity.getLength() == 0)
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM(
                "LocalSingleBackend - Invalid empty entity."));

        throw lang::IllegalArgumentException(sMsg, *this, 1);
    }
    if (aOtherEntity.getLength() == 0)
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM(
                "LocalSingleBackend - Invalid empty entity."));

        throw lang::IllegalArgumentException(sMsg, *this, 2);
    }


    if (aEntity.equals(aOtherEntity))
    {
        return true;
    }

    if (checkOkState())
    {
        for (std::vector< uno::Reference <uno::XInterface> >::const_iterator it =  mBackendStrata.begin(); it !=  mBackendStrata.end(); ++it)
        {
            uno::Reference< backenduno::XBackendEntities > xEntities( *it, uno::UNO_QUERY );
            if (xEntities.is())
            {
                if( xEntities->supportsEntity(aEntity))
                    return  xEntities->isEqualEntity(aEntity,aOtherEntity);
            }
        }
        return false;
    }
    return false;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
    MultiStratumBackend::getSchemaVersion(const rtl::OUString& aComponent)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    osl::MutexGuard aGuard(mMutex);

    if (checkOkState())
    {
        uno::Reference<backenduno::XVersionedSchemaSupplier> xVersionSupplier(mSchemaSupplier,uno::UNO_QUERY);
        if (xVersionSupplier.is())
            return xVersionSupplier->getSchemaVersion(aComponent) ;
    }
    return rtl::OUString();
}
//------------------------------------------------------------------------------

uno::Reference<backenduno::XSchema> SAL_CALL
    MultiStratumBackend::getComponentSchema(const rtl::OUString& aComponent)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    osl::MutexGuard aGuard(mMutex);

    if (checkOkState())
    {
        return mSchemaSupplier->getComponentSchema(aComponent) ;
    }
    return NULL;
}
//------------------------------------------------------------------------------

uno::Sequence<uno::Reference<backenduno::XLayer> > SAL_CALL
    MultiStratumBackend::listOwnLayers(const rtl::OUString& aComponent)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    if (checkOkState())
    {
        return listLayers(aComponent, mOwnerEntity ) ;
    }
    return uno::Sequence<uno::Reference<backenduno::XLayer> >() ;
}
//------------------------------------------------------------------------------

uno::Reference<backenduno::XUpdateHandler> SAL_CALL
    MultiStratumBackend::getOwnUpdateHandler(const rtl::OUString& aComponent)
        throw (backenduno::BackendAccessException,
                lang::NoSupportException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    if (checkOkState())
    {
        return getUpdateHandler(aComponent, mOwnerEntity) ;
    }
    return NULL;
}
//------------------------------------------------------------------------------

uno::Sequence<uno::Reference<backenduno::XLayer> > SAL_CALL
    MultiStratumBackend::listLayers(const rtl::OUString& aComponent,
                                    const rtl::OUString& aEntity)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{

    osl::MutexGuard aGuard(mMutex);
    if (checkOkState())
    {
        sal_Int32 nNumSupportedLayers = mBackendStrata.size();
        if(aEntity != mOwnerEntity)
        {
            nNumSupportedLayers = findSupportingStratum(aEntity);
        }
        return searchSupportingStrata(nNumSupportedLayers,aEntity,aComponent);
    }
    return uno::Sequence<uno::Reference<backenduno::XLayer> >();
}
//------------------------------------------------------------------------------
sal_Int32 MultiStratumBackend::findSupportingStratum(const rtl::OUString& aEntity)
{
    sal_Int32 nNumLayers = mBackendStrata.size();
    for (std::vector< uno::Reference <uno::XInterface> >::reverse_iterator it = mBackendStrata.rbegin(); it != mBackendStrata.rend(); ++it)
    {
        uno::Reference< backenduno::XBackendEntities > xEntities( *it, uno::UNO_QUERY );
        if (xEntities.is())
        {
            if( xEntities->supportsEntity(aEntity))
            {
                return nNumLayers;
            }
        }
        nNumLayers--;
    }
    rtl::OUStringBuffer sMsg;
    sMsg.appendAscii("\n MultiStratumBackend: No Backend supports Entity: \"");
    sMsg.append(aEntity);
    throw lang::IllegalArgumentException(sMsg.makeStringAndClear(),
                                         *this, 0) ;
}
//------------------------------------------------------------------------------
uno::Sequence<uno::Reference<backenduno::XLayer> >
    MultiStratumBackend::searchSupportingStrata(sal_Int32 nNumLayers,
                                                rtl::OUString aEntity,
                                                const rtl::OUString& aComponent)
{
    uno::Sequence<uno::Reference<backenduno::XLayer> > aLayers;
    std::vector<uno::Reference<backenduno::XLayer> > aBackendLayers;
    for (sal_Int32 i = 0 ; i < nNumLayers ; ++ i)
    {
        uno::Sequence<uno::Reference<backenduno::XLayer> > aMultiLayers;
        uno::Reference< backenduno::XBackendEntities > xEntities(mBackendStrata[i], uno::UNO_QUERY );

        uno::Reference<backenduno::XBackend> xBackend(mBackendStrata[i], uno::UNO_QUERY) ;
        if (xBackend.is())
        {
            if (xEntities.is())
            {
                if( !xEntities->supportsEntity(aEntity))
                {
                    aEntity = xEntities->getOwnerEntity();
                }
                aMultiLayers = xBackend->listLayers(aComponent, aEntity) ;
            }
            else
            {
                aMultiLayers = xBackend->listOwnLayers(aComponent);
            }
        }
        else
        {
            uno::Reference<backenduno::XMultiLayerStratum> xMultiLayerStratum(
                    mBackendStrata[i], uno::UNO_QUERY) ;

            if (xMultiLayerStratum.is())
            {
                if (xEntities.is())
                {
                    if( !xEntities->supportsEntity(aEntity))
                    {
                        aEntity = xEntities->getOwnerEntity();
                    }
                    aMultiLayers = xMultiLayerStratum->getLayers(
                        xMultiLayerStratum->listLayerIds(aComponent, aEntity),
                        rtl::OUString()) ;

                }
                else
                {
                     throw backenduno::BackendSetupException(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "MultiStratumBackend: XMultiLayerStratum must support XBackendEntities")),
                        *this, uno::Any()) ;
                }

            }
            else
            {
                uno::Reference<backenduno::XSingleLayerStratum> xSingleLayerStratum(
                    mBackendStrata[i], uno::UNO_REF_QUERY_THROW) ;
                if (xSingleLayerStratum.is())
                {
                    uno::Reference<backenduno::XLayer> xLayer = xSingleLayerStratum->getLayer( aComponent, rtl::OUString());
                    //Could be an empty layer
                    if (xLayer.is())
                    {
                        aBackendLayers.push_back(xLayer);
                    }
                }
            }
        }
        // There might be non-existent layers in the list if there's no
        // actual data associated to a given layer id. Hence we have to
        // compress the list.
        for (sal_Int32 j = 0 ; j < aMultiLayers.getLength() ; ++j)
        {
            if (aMultiLayers [j].is())
            {
                aBackendLayers.push_back( aMultiLayers[j]);
            }
        }
    }
    aLayers.realloc(aBackendLayers.size());
    for (sal_uInt32 k =0; k < aBackendLayers.size();k++)
    {
        aLayers[k] = aBackendLayers[k];
    }
    return aLayers;
}
//------------------------------------------------------------------------------
uno::Reference<backenduno::XUpdateHandler> SAL_CALL
    MultiStratumBackend::getUpdateHandler(const rtl::OUString& aComponent,
                                          const rtl::OUString& aEntity)
        throw (backenduno::BackendAccessException,
                lang::NoSupportException,
                lang::IllegalArgumentException,
                uno::RuntimeException)
{
    osl::MutexGuard aGuard(mMutex);

    if (checkOkState())
    {

        sal_Int32 nNumSupportedLayers = mBackendStrata.size();
        rtl::OUString aUsedEntity;

        if(aEntity != mOwnerEntity)
        {
            nNumSupportedLayers = findSupportingStratum(aEntity);
            aUsedEntity = aEntity;
        }
        else
        {
            uno::Reference< backenduno::XBackendEntities > xEntities( mBackendStrata[mBackendStrata.size()-1], uno::UNO_QUERY );
            if  (xEntities.is())
            {
                aUsedEntity = xEntities->getOwnerEntity();
            }
        }
        sal_Int32 nStrataIndex = nNumSupportedLayers -1;

        uno::Reference<backenduno::XBackend> xBackend(
           mBackendStrata[nStrataIndex], uno::UNO_QUERY) ;
           uno::Reference< uno::XInterface > xHandler;
        if(xBackend.is())
        {
            if (aUsedEntity.getLength()==0)
            {
                xHandler = xBackend->getOwnUpdateHandler(aComponent) ;
                return uno::Reference<backenduno::XUpdateHandler>(xHandler,uno::UNO_REF_QUERY_THROW);
            }
            else
            {
                xHandler = xBackend->getUpdateHandler(aComponent, aUsedEntity) ;
                return uno::Reference<backenduno::XUpdateHandler>(xHandler,uno::UNO_REF_QUERY_THROW);
            }
        }
        else
        {

            uno::Sequence<uno::Any> arguments(1) ;

            uno::Reference< backenduno::XMultiLayerStratum > xMultiLayerStratum
                (mBackendStrata[nStrataIndex], uno::UNO_QUERY );
            if(xMultiLayerStratum.is())
            {
                arguments [0] <<= xMultiLayerStratum->getUpdatableLayer(
                    xMultiLayerStratum->getUpdateLayerId(aComponent,aUsedEntity));
            }
            else
            {
                uno::Reference< backenduno::XSingleLayerStratum > xSingleLayerStratum(
                    mBackendStrata[nStrataIndex], uno::UNO_REF_QUERY_THROW );

                arguments [0] <<= xSingleLayerStratum->getUpdatableLayer(aComponent);

            }

            if(!xHandler.is())
            {
                try
                {
                    const rtl::OUString kUpdateMerger(RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.configuration.backend.LayerUpdateMerger")) ;


                    if(!mFactory.is())
                    {
                        throw lang::DisposedException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "MultiStratumBackend: Service Factory already dispoed")),*this);
                    }

                    xHandler = mFactory->createInstanceWithArguments(kUpdateMerger, arguments);
                }
                catch (uno::RuntimeException & )
                {throw;}
                catch (uno::Exception & e)
                {
                    const rtl::OUString sMessage(RTL_CONSTASCII_USTRINGPARAM(
                            "Configuration MultiStratumBackend: Cannot create UpdateMerger - error message: ")) ;
                    throw uno::RuntimeException(sMessage.concat(e.Message),*this);
                }
            }
            return uno::Reference<backenduno::XUpdateHandler>(xHandler, uno::UNO_REF_QUERY_THROW) ;
        }
    }
    return NULL;
}
// ---------------------------------------------------------------------------
// ComponentHelper
void SAL_CALL MultiStratumBackend::disposing()
{
    osl::MutexGuard aGuard(mMutex);
    if (mFactory.is())
    {
        mFactory.clear();
    }
    if (mSchemaSupplier.is())
    {
        uno::Reference< lang::XComponent> xComp( mSchemaSupplier, uno::UNO_QUERY);
        if (xComp.is())
        {
            xComp->dispose();
        }
        if (mSchemaSupplier.is())
        {
            mSchemaSupplier.clear();
        }
    }
    if (!mBackendStrata.empty())
    {
        for (std::vector< uno::Reference <uno::XInterface> >::const_iterator it =  mBackendStrata.begin(); it !=  mBackendStrata.end(); ++it)
        {
            uno::Reference<  lang::XComponent> xComp( *it, uno::UNO_QUERY );
            if (xComp.is())
            {
                try
                {
                    xComp->dispose();
                }
                catch(uno::Exception &){}
            }
        }
        mBackendStrata.clear();

    }

 }
//------------------------------------------------------------------------------

static const sal_Char * const kBackendService = "com.sun.star.configuration.backend.Backend" ;

static const sal_Char * const kImplementation =
                "com.sun.star.comp.configuration.backend.MultiStratumBackend" ;

static sal_Char const * const kServiceNames [] =
{
    kBackendService,
    0
} ;
static const ServiceImplementationInfo kServiceInfo =
{
    kImplementation,
    kServiceNames,
    0
} ;

const ServiceRegistrationInfo *getMultiStratumBackendServiceInfo()
{
    return getRegistrationInfo(&kServiceInfo) ;
}

uno::Reference<uno::XInterface> SAL_CALL
    instantiateMultiStratumBackend(const uno::Reference< uno::XComponentContext >& xContext)
{
    return *new MultiStratumBackend(xContext) ;
}
//------------------------------------------------------------------------------

static const rtl::OUString kImplementationName(
                                RTL_CONSTASCII_USTRINGPARAM(kImplementation)) ;
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
    MultiStratumBackend::getImplementationName(void)
        throw (uno::RuntimeException)
{
    return ServiceInfoHelper(&kServiceInfo).getImplementationName() ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL
    MultiStratumBackend::supportsService(const rtl::OUString& aServiceName)
        throw (uno::RuntimeException)
{
    return  ServiceInfoHelper(&kServiceInfo).supportsService(aServiceName) ;
}
//------------------------------------------------------------------------------
uno::Sequence<rtl::OUString> SAL_CALL
    MultiStratumBackend::getSupportedServiceNames(void)
        throw (uno::RuntimeException)
{
    return ServiceInfoHelper(&kServiceInfo).getSupportedServiceNames() ;
}
//------------------------------------------------------------------------------
void SAL_CALL MultiStratumBackend::addChangesListener( const uno::Reference<backenduno::XBackendChangesListener>& xListener,
                                                       const rtl::OUString& aComponent)
    throw (::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(mMutex);
    if(checkOkState())
    {

        mListenerList.insert( ListenerList::value_type(aComponent, xListener));
        //Now register with lower layers
        for (sal_uInt32 i = 0 ; i < mBackendStrata.size()  ; ++ i)
        {
            uno::Reference<backenduno::XBackendChangesNotifier> xBackend(mBackendStrata[i], uno::UNO_QUERY) ;
            if (xBackend.is())
            {
                xBackend->addChangesListener(mStrataListener, aComponent);
            }
        }
    }
}
//------------------------------------------------------------------------------
void SAL_CALL MultiStratumBackend::removeChangesListener( const uno::Reference<backenduno::XBackendChangesListener>& /*xListner*/,
                                                          const rtl::OUString& aComponent)
    throw (::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(mMutex);
    if(checkOkState())
    {
        sal_Int32 nCount =  mListenerList.count(aComponent);
        if (nCount == 0)
        {
            OSL_TRACE("MultiStratumBackend: removeListener: no listener registered for component %s"
                      , aComponent.getStr());
        }
        else
        {
            ListenerList::iterator aIter;
            aIter = mListenerList.find(aComponent);
            mListenerList.erase(aIter);
            if (nCount == 1)
            {
                //Deregister Listener from strata backend if they support notification
                for (sal_uInt32 i = 0 ; i < mBackendStrata.size(); ++ i)
                {
                    uno::Reference<backenduno::XBackendChangesNotifier> xBackend(mBackendStrata[i], uno::UNO_QUERY) ;
                    if (xBackend.is())
                    {
                        xBackend->removeChangesListener(mStrataListener, aComponent);
                    }
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void MultiStratumBackend::componentDataChanged(const backenduno::ComponentChangeEvent& aEvent)
    throw (::com::sun::star::uno::RuntimeException)
{
     try
     {
         notifyListeners( aEvent);

     }
     catch (uno::RuntimeException& ) { throw; }
     catch (uno::Exception& e)
     {
            throw lang::WrappedTargetRuntimeException(e.Message, *this, uno::makeAny(e));
     }
}
//------------------------------------------------------------------------------
void MultiStratumBackend::disposing( lang::EventObject const & /*rSource*/ )
    throw (::com::sun::star::uno::RuntimeException)
{}
//------------------------------------------------------------------------------
void MultiStratumBackend::notifyListeners(const backenduno::ComponentChangeEvent& aEvent)const
{
    //fire off notification to all registered listeners for specific Component
    ListenerList::const_iterator aIter;
    rtl::OUString aComponentName = aEvent.Component;
    if (mListenerList.empty())
    {
        OSL_TRACE("MultiStratumBackend: notifyListeners: no listeners registered for component %s",
                   aComponentName.getStr());
    }
    else
    {
        aIter = mListenerList.begin();
        do{
            if (aIter->first == aComponentName)
            {
                aIter->second->componentDataChanged(aEvent);
            }
            aIter++;
        }while (aIter != mListenerList.end());
    }
 }
//------------------------------------------------------------------------------
} } // configmgr.backend

