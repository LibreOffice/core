/*************************************************************************
 *
 *  $RCSfile: multistratumbackend.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 14:35:48 $
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

#ifndef CONFIGMGR_BACKEND_MULTISTRATUMBACKEND_HXX_
#include "multistratumbackend.hxx"
#endif // CONFIGMGR_BACKEND_SINGLEBACKENDADAPTER_HXX_

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif // CONFIGMGR_API_FACTORY_HXX_

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif // CONFIGMGR_SERVICEINFOHELPER_HXX_

#ifndef _CONFIGMGR_BOOTSTRAP_HXX
#include "bootstrap.hxx"
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XMULTILAYERSTRATUM_HPP_
#include <com/sun/star/configuration/backend/XMultiLayerStratum.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSINGLELAYERSTRATUM_HPP_
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef CONFIGMGR_BACKEND_BACKENDSTRATALISTENER_HXX
#include "backendstratalistener.hxx"
#endif CONFIGMGR_BACKEND_BACKENDSTRATALISTENER_HXX

#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETRUNTIMEEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#include <stdio.h>

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
typedef std::pair<rtl::OUString, rtl::OUString> ServiceInfo;
typedef std::vector<ServiceInfo> ServiceInfoList;
void parseStrataInfo(const rtl::OUString aServiceList,
                     ServiceInfoList& aServiceInfoList,
                     const uno::Reference<uno::XInterface>& pContext)
{
    sal_Int32 nNextToken =0;
    sal_Int32 nLength = aServiceList.getLength();

    do
    {
        OUString aServiceName =aServiceList.getToken(0, ':',nNextToken);
        if((nNextToken ==-1)||(aServiceName.getLength()==0))
        {
             throw backenduno::BackendSetupException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Malformed Strata Service specified")),
                        pContext, uno::Any()) ;
        }
        OUString aServiceData = aServiceList.getToken(0, ';',nNextToken);

        ServiceInfo aServiceInfo(aServiceName,aServiceData);
        aServiceInfoList.push_back(aServiceInfo);
    }
    while (nNextToken >= 0 && nNextToken < nLength ) ;
}
//------------------------------------------------------------------------------
MultiStratumBackend::MultiStratumBackend(
        const uno::Reference<uno::XComponentContext>& xContext)
        : BackendBase(mMutex), mFactory(xContext->getServiceManager(),uno::UNO_QUERY_THROW)
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
            OUString sDefaultEntity;
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
    catch(uno::Exception& aException)
    {
        if (mSchemaSupplier.is())
        {
            mSchemaSupplier.clear();
        }
        if(mBackendStrata.empty())
        {
            mBackendStrata.clear() ;
        }
        throw;
    }

}
//------------------------------------------------------------------------------
typedef uno::Reference<backenduno::XSchemaSupplier> xSchemaSupplier;
void  MultiStratumBackend::initializeSchemaSupplier(const uno::Reference<uno::XComponentContext>& aContext)
{

    OUString aServiceName;

    aContext->getValueByName(kSchemaServiceParam) >>= aServiceName;
    uno::Sequence< uno::Any > aInitArgs( 1 );
    aInitArgs[0] <<= aContext;
    mSchemaSupplier = xSchemaSupplier::query(mFactory->createInstanceWithArguments(aServiceName,aInitArgs)) ;
    if (!mSchemaSupplier.is())
    {
         throw backenduno::BackendSetupException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "MultiStratumBackend: Could not create Schema Supplier service")),
                        *this, uno::Any()) ;

    }
}
//------------------------------------------------------------------------------
void MultiStratumBackend::initializeBackendStrata(const uno::Reference<uno::XComponentContext>& aContext)
{

    OUString sStrata;
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
    ServiceInfoList aServiceInfoList;
    parseStrataInfo(sStrata,aServiceInfoList,*this);

    for (ServiceInfoList::const_iterator it = aServiceInfoList.begin(); it != aServiceInfoList.end(); ++it)
    {
        uno::Sequence< uno::Any > aInitArgs( 1 );
        OUString sServiceName = it->first;
        const OUString& sServiceData = it->second;
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
                static const sal_Char sErrContext[] = "MultiStratumBackend: Could not create Backend Strata Service: ";
                OUString const sContext(RTL_CONSTASCII_USTRINGPARAM(sErrContext));
                exception.Message = sContext.concat(exception.Message);
                throw;
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
    return OUString();
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
            return OUString();
        }
    }
    return OUString();
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL
    MultiStratumBackend::supportsEntity( const rtl::OUString& aEntity )
    throw (backenduno::BackendAccessException, uno::RuntimeException)
{

    osl::MutexGuard aGuard(mMutex);
    if (checkOkState())
    {
        for (BackendStrata::const_iterator it =  mBackendStrata.begin(); it !=  mBackendStrata.end(); ++it)
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
        for (BackendStrata::const_iterator it =  mBackendStrata.begin(); it !=  mBackendStrata.end(); ++it)
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
    return NULL;
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
    return NULL;
}
//------------------------------------------------------------------------------
sal_Int32 MultiStratumBackend::findSupportingStratum(const rtl::OUString& aEntity)
{
    sal_Int32 nNumLayers = mBackendStrata.size();
    for (BackendStrata::reverse_iterator it = mBackendStrata.rbegin(); it != mBackendStrata.rend(); ++it)
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
    return 0;
}
//------------------------------------------------------------------------------
typedef std::vector<uno::Reference<backenduno::XLayer> > BackendLayers;
uno::Sequence<uno::Reference<backenduno::XLayer> >
    MultiStratumBackend::searchSupportingStrata(sal_Int32 nNumLayers,
                                                rtl::OUString aEntity,
                                                const rtl::OUString& aComponent)
{
    uno::Sequence<uno::Reference<backenduno::XLayer> > aLayers;
    BackendLayers aBackendLayers;
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
                    uno::Reference<backenduno::XLayer> xLayer = xSingleLayerStratum->getLayer( aComponent, OUString());
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
        OUString aUsedEntity;

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
        for (BackendStrata::const_iterator it =  mBackendStrata.begin(); it !=  mBackendStrata.end(); ++it)
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

static const AsciiServiceName kServiceNames [] =
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
    instantiateMultiStratumBackend(const CreationContext& xContext)
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
void SAL_CALL MultiStratumBackend::removeChangesListener( const uno::Reference<backenduno::XBackendChangesListener>& xListner,
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
void MultiStratumBackend::disposing( lang::EventObject const & rSource )
    throw (::com::sun::star::uno::RuntimeException)
{}
//------------------------------------------------------------------------------
void MultiStratumBackend::notifyListeners(const backenduno::ComponentChangeEvent& aEvent)const
{
    //fire off notification to all registered listeners for specific Component
    ListenerList::const_iterator aIter;
    OUString aComponentName = aEvent.Component;
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

