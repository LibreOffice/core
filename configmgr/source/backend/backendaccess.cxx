/*************************************************************************
 *
 *  $RCSfile: backendaccess.cxx,v $
 *
 *  $Revision: 1.19 $
 *  last change: $Author: kz $ $Date: 2004-08-31 15:23:10 $
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

#ifndef CONFIGMGR_BACKEND_BACKENDACCESS_HXX_
#include "backendaccess.hxx"
#endif // CONFIGMGR_BACKEND_BACKENDACCESS_HXX_

#ifndef CONFIGMGR_LOGGER_HXX
#include "logger.hxx"
#endif

#ifndef CONFIGMGR_MATCHLOCALE_HXX
#include "matchlocale.hxx"
#endif // CONFIGMGR_MATCHLOCALE_HXX

#ifndef CONFIGMGR_BACKEND_LAYERMERGE_HXX
#include "layermerge.hxx"
#endif // CONFIGMGR_BACKEND_LAYERMERGE_HXX

#ifndef CONFIGMGR_BACKEND_SCHEMABUILDER_HXX
#include "schemabuilder.hxx"
#endif // CONFIGMGR_BACKEND_SCHEMABUILDER_HXX

#ifndef CONFIGMGR_BACKEND_UPDATEDISPATCHER_HXX
#include "updatedispatch.hxx"
#endif // CONFIGMGR_BACKEND_UPDATEDISPATCHER_HXX

#ifndef CONFIGMGR_BACKEND_BACKENDNOTIFIER_HXX
#include "backendnotifier.hxx"
#endif

#ifndef CONFIGMGR_BACKEND_EMPTYLAYER_HXX
#include "emptylayer.hxx"
#endif

#ifndef _CONFIGMGR_FILEHELPER_HXX_
#include "filehelper.hxx"
#endif

#ifndef CONFIGMGR_SIMPLEINTERACTIONREQUEST_HXX
#include "simpleinteractionrequest.hxx"
#endif

#ifndef CONFIGMGR_CONFIGINTERACTIONHANDLER_HXX
#include "configinteractionhandler.hxx"
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMASUPPLIER_HPP_
#include <com/sun/star/configuration/backend/XSchemaSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XCOMPOSITELAYER_HPP_
#include <com/sun/star/configuration/backend/XCompositeLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XCOMPOSITELAYER_HPP_
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XUPDATABLELAYER_HPP_
#include <com/sun/star/configuration/backend/XUpdatableLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XUPDATABLELAYER_HPP_
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDENTITIES_HPP_
#include <com/sun/star/configuration/backend/XBackendEntities.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_MERGERECOVERYREQUEST_HPP_
#include <com/sun/star/configuration/backend/MergeRecoveryRequest.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_MERGERECOVERYREQUEST_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_MALFORMEDDATAEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/MalformedDataException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif // _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#include <cppuhelper/exc_hlp.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif //INCLUDED_VECTOR

#define OU2A(rtlOUString)   (::rtl::OUStringToOString((rtlOUString), RTL_TEXTENCODING_ASCII_US).getStr())
#define RTL_LOGFILE_OU2A(rtlOUString)   OU2A(rtlOUString)

#define OUSTR(txt)  OUString( RTL_CONSTASCII_USTRINGPARAM(txt) )

namespace configmgr { namespace backend {
//------------------------------------------------------------------------------
    namespace task = com::sun::star::task;
    typedef uno::Reference< backenduno::XUpdatableLayer > LayerDataRemover;

inline
uno::Reference<lang::XMultiServiceFactory> BackendAccess::getServiceFactory() const
{
    return uno::Reference<lang::XMultiServiceFactory>(mContext->getServiceManager(),uno::UNO_QUERY_THROW);
}
//------------------------------------------------------------------------------

BackendAccess::BackendAccess(
        const uno::Reference<backenduno::XBackend>& xBackend,
        const uno::Reference<uno::XComponentContext>& xContext)
    : mContext(xContext)
    , mBackend(xBackend)
    , mBinaryCache(xContext)
 {
    OSL_ENSURE(mContext.is(), "BackendAccess: Context is missing");
    if (!mContext.is())
        throw lang::NullPointerException(OUString::createFromAscii("BackendAccess: NULL Context passed"), NULL);
    if (!xBackend.is())
        throw lang::NullPointerException(OUSTR("Configuration: Trying to create backend access without backend"),NULL);
    if (!uno::Reference<backenduno::XSchemaSupplier>::query(xBackend).is())
        throw lang::NullPointerException(OUSTR("Configuration: No backend for schemas available"),NULL);


     mNotifier = new BackendChangeNotifier(xBackend);
     //Stored as uno::reference to facilitate sharing with MultiStratumBackend
     mXNotifier = mNotifier;

     //Create Binary Cache
    uno::Reference<backenduno::XBackendEntities> xBackendEntities = uno::Reference<backenduno::XBackendEntities>( mBackend, uno::UNO_QUERY) ;
    OSL_ENSURE(xBackendEntities.is(),"Backend does not provide entity information");

    if ( xBackendEntities.is() ) mBinaryCache.setOwnerEntity(xBackendEntities->getOwnerEntity());
}
//------------------------------------------------------------------------------

BackendAccess::~BackendAccess() {}
//------------------------------------------------------------------------------

static rtl::OUString findBestLocale(
        const uno::Sequence<rtl::OUString>& aLocales,
        const rtl::OUString& aWanted) {
    rtl::OUString fallback ;

    for (sal_Int32 i = 0 ; i < aLocales.getLength() ; ++ i) {
        if (aLocales [i].equals(aWanted)) { return aWanted ; }
        if (fallback.getLength() == 0) {
            sal_Int32 compLength = aWanted.getLength() ;

            if (aLocales [i].getLength() < compLength) {
                compLength = aLocales [i].getLength() ;
            }
            if (aLocales [i].compareTo(aWanted, compLength) == 0) {
                fallback = aLocales [i] ;
            }
        }
    }
    return fallback ;
}
//------------------------------------------------------------------------------

void BackendAccess::merge(
        MergedComponentData& aData,
        const uno::Reference<backenduno::XLayer> * pLayers,
        sal_Int32 aNumLayers,
        RequestOptions const & aOptions,
        ITemplateDataProvider *aTemplateProvider,
        sal_Int32 * pLayersMerged)
    CFG_UNO_THROW_ALL()
{
    LayerMergeHandler * pMerger = new LayerMergeHandler(mContext, aData, aTemplateProvider );
    uno::Reference<backenduno::XLayerHandler> xLayerMerger(pMerger);

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::merge()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "merging %d layers", int(aNumLayers) );

    OUString const aLocale = aOptions.getLocale();
    bool const needLocalizedData = aLocale.getLength() && !localehelper::isDefaultLanguage(aLocale);

    if (pLayersMerged) *pLayersMerged = 0;

    for (sal_Int32 i = 0 ; i < aNumLayers ; ++ i)
    {
        pMerger->prepareLayer() ;
        pLayers [i]->readData(xLayerMerger) ;

        if (needLocalizedData)
        {
            uno::Reference<backenduno::XCompositeLayer> compositeLayer(
                 pLayers [i], uno::UNO_QUERY) ;

            if (compositeLayer.is())
            {
                uno::Sequence<rtl::OUString> aSubLayerIds = compositeLayer->listSubLayerIds();
                if(localehelper::isAnyLanguage(aLocale))
                {
                    //Loop thru layers
                    for (sal_Int32 j = 0; j < aSubLayerIds.getLength(); ++j)
                    {
                        if(pMerger->prepareSublayer(aSubLayerIds[j]))
                        {
                            compositeLayer->readSubLayerData(xLayerMerger,aSubLayerIds[j]) ;
                        }
                    }
                }
                else
                {
                    rtl::OUString bestLocale = findBestLocale(aSubLayerIds, aLocale) ;

                    if (pMerger->prepareSublayer(bestLocale) )
                    {
                        compositeLayer->readSubLayerData(xLayerMerger, bestLocale) ;
                    }
                }
            }
        }
        if (pLayersMerged) ++*pLayersMerged;
    }
}
//------------------------------------------------------------------------------

bool BackendAccess::readDefaultData( MergedComponentData & aComponentData,
                                        OUString const & aComponent,
                                        RequestOptions const & aOptions,
                                        bool bIncludeTemplates,
                                        const uno::Reference<backenduno::XLayer> * pLayers,
                                        sal_Int32 nNumLayers,
                                        ITemplateDataProvider *aTemplateProvider,
                                        sal_Int32 * pLayersMerged)
    CFG_UNO_THROW_ALL()
{
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog1, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::readDefaultData()");

    Logger logger(mContext);
    const Logger::Level detail = LogLevel::FINER;
    bool const bLogDetail = logger.isLogging(detail);

    if (bLogDetail)
        logger.log( detail, OUString::createFromAscii("Reading data for component ") + aComponent,
                    "readDefaultData()","configmgr::Backend");

    if (bLogDetail) logger.log(detail, "... attempt to read from binary cache", "readDefaultData()","configmgr::Backend");
    bool bCacheHit = mBinaryCache.readComponentData(aComponentData, getServiceFactory(),
                                                    aComponent, aOptions.getEntity(), aOptions.getLocale(),
                                                    pLayers, nNumLayers, bIncludeTemplates);

    if (!bCacheHit)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog2, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::readDefaultData() - not in cache");

        if (bLogDetail) logger.log(detail, "... cache miss - need full merge", "readDefaultData()","configmgr::Backend");
        if (bLogDetail) logger.log(detail, "... reading schema", "readDefaultData()","configmgr::Backend");
        {
            SchemaBuilder *schemaBuilder = new SchemaBuilder( mContext, aComponent, aComponentData, aTemplateProvider );
            uno::Reference<backenduno::XSchemaHandler> schemaHandler = schemaBuilder ;

            uno::Reference<backenduno::XSchema> schema = this->getSchema(aComponent);
            schema->readSchema(schemaHandler) ;
        }

        if (bLogDetail) logger.log(detail, "... merging layers", "readDefaultData()","configmgr::Backend");
        this->merge(aComponentData, pLayers, nNumLayers, aOptions, aTemplateProvider, pLayersMerged );
        promoteToDefault(aComponentData);

        if (mBinaryCache.isCacheEnabled(aOptions.getEntity()))
        {
            if (bLogDetail) logger.log(detail, "... creating binary cache", "readDefaultData()","configmgr::Backend");
            bool bWriteSuccess = mBinaryCache.writeComponentData( aComponentData, getServiceFactory(),
                                                                  aComponent, aOptions.getEntity(), aOptions.getLocale(),
                                                                  pLayers, nNumLayers );

            if (!bWriteSuccess)
            {
                logger.info("Binary cache write failed - disabling binary cache","readDefaultData()","configmgr::Backend");
                mBinaryCache.disableCache();
            }
        }
        else if (bLogDetail)
            logger.log(detail, "... cache hit", "readDefaultData()","configmgr::Backend");
    }
    else if (pLayersMerged)
        *pLayersMerged = nNumLayers;

    return aComponentData.hasSchema();
}
//------------------------------------------------------------------------------

static OUString getLayerURL(const uno::Reference<backenduno::XLayer> & aLayer)
{
    try
    {
        namespace beans = com::sun::star::beans;
        uno::Reference< beans::XPropertySet > xLayerProps( aLayer, uno::UNO_QUERY );
        if (xLayerProps.is())
        {
            uno::Any aPropVal = xLayerProps->getPropertyValue( OUSTR("URL") );
            OUString aResult;
            if (aPropVal >>= aResult)
                return aResult;
        }
        OSL_TRACE("Warning - Cannot get location of layer data\n");
    }
    catch (uno::Exception & e)
    {
        OSL_TRACE("Warning - Configuration: Retrieving error (interaction) handler failed: [%s]\n", OU2A(e.Message));
    }
    // TODO: use fallback, e.g. ServiceName
    return OUString();
}
//------------------------------------------------------------------------------
static inline
OUString getLayerIdentifier(const uno::Reference<backenduno::XLayer> & aLayer)
{
    return getLayerURL(aLayer);
}

//------------------------------------------------------------------------------
static void removeLayerData(uno::Reference< backenduno::XLayer > const & xLayer)
{
    OSL_ASSERT(xLayer.is());

    LayerDataRemover xLayerRemover(xLayer,uno::UNO_QUERY);
    if (xLayerRemover.is())
    try
    {
        xLayerRemover->replaceWith(createEmptyLayer());
    }
    catch (uno::Exception & e)
    {
        OSL_TRACE("Warning - Configuration: Could not clear Layer data. Error: [%s]\n", OU2A(e.Message));
    }
    else
    {
        if (! FileHelper::tryToRemoveFile(getLayerURL(xLayer),true))
            OSL_TRACE("Warning - Configuration: Could not remove broken user Layer data: [-Not Updatable-]\n");
    }
}
//------------------------------------------------------------------------------

static void discardLayer(uno::Sequence<uno::Reference<backenduno::XLayer> >& layers, sal_Int32 nLayer)
{
    OSL_ASSERT(0 <= nLayer && nLayer < layers.getLength());
    sal_Int32 nNewSize = layers.getLength() - 1;

    for (sal_Int32 i = nLayer; i<nNewSize; ++i)
        layers[i] = layers[i+1];

    layers.realloc(nNewSize);
}
//------------------------------------------------------------------------------

bool BackendAccess::approveRecovery(const uno::Any & aMergeException,
                                    const uno::Reference<backenduno::XLayer> & aBrokenLayer,
                                    bool bUserLayerData)
    CFG_UNO_THROW_ALL()
{
    using namespace apihelper;
    typedef SimpleInteractionRequest::Continuation Choice;
    Choice const k_supported_choices = CONTINUATION_APPROVE | CONTINUATION_DISAPPROVE;

    Choice chosen = CONTINUATION_UNKNOWN;

    ConfigurationInteractionHandler aHandler;
    if (aHandler.is())
    try
    {
        const char * const message = "Recover from configuration merge failure";
        const OUString aLayerId = getLayerIdentifier(aBrokenLayer);
        const bool bRemoveLayerData = bUserLayerData &&
                                        (LayerDataRemover::query(aBrokenLayer).is() ||
                                         FileHelper::fileExists(getLayerURL(aBrokenLayer)) );

        rtl::Reference< SimpleInteractionRequest > xRequest(
            new SimpleInteractionRequest(
                uno::makeAny(
                    backenduno::MergeRecoveryRequest(OUString::createFromAscii(message), aBrokenLayer,
                                                        aMergeException, aLayerId, bRemoveLayerData) ),
                k_supported_choices
            ) );
        aHandler.handle(xRequest.get());
        chosen = xRequest->getResponse();
    }
    catch (uno::Exception & e)
    {
        OSL_TRACE("Warning - Configuration: Interaction handler failed: [%s]\n", OU2A(e.Message));
    }

    switch (chosen)
    {
    case CONTINUATION_APPROVE:      return true;
    case CONTINUATION_DISAPPROVE:   return false;
    case CONTINUATION_UNKNOWN:      break;

    default: OSL_ENSURE(false,"Unsolicited continuation chosen"); break;
    }
    // no choice available - default: approve, if user data
    return bUserLayerData;
}
//------------------------------------------------------------------------------

ComponentResult BackendAccess::getNodeData(const ComponentRequest& aRequest,
                                           ITemplateDataProvider *_aTemplateProvider,
                                           INodeDataListener *aListener)
    CFG_UNO_THROW_ALL()
{
    rtl::OUString const component = aRequest.getComponentName().toString() ;
    ITemplateDataProvider * const aTemplateProvider = _aTemplateProvider ? _aTemplateProvider : this;

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::getNodeData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "request path: %s", RTL_LOGFILE_OU2A(component) );

    uno::Sequence<uno::Reference<backenduno::XLayer> > layers =
        this->getLayers(component, aRequest.getOptions()) ;

    sal_Int32 const k_NumUserLayers = 1;

    sal_Int32 nNumDefaultLayers = layers.getLength() - k_NumUserLayers;
    sal_Int32 nCurrentLayer;
    bool bDefaultRecoveryApproved = false;

    Logger logger(mContext);
    do try // loop to allow recovery from merge failures
    {
        MergedComponentData aComponentData;
        nCurrentLayer = -1;

        if (!this->readDefaultData(aComponentData, component, aRequest.getOptions(), true,
                                    layers.getConstArray(),nNumDefaultLayers,
                                    aTemplateProvider, &nCurrentLayer))
        {
            rtl::OUStringBuffer sMessage;
            sMessage.appendAscii("Configuration: No data for request. Component \"");
            sMessage.append(component);
            sMessage.appendAscii("\" contains no data. ");

            throw com::sun::star::container::NoSuchElementException(sMessage.makeStringAndClear(),mBackend);
        }
        OSL_ASSERT(nCurrentLayer == nNumDefaultLayers);

        sal_Int32 const nNumUserLayers = layers.getLength() - nNumDefaultLayers;
        if (nNumUserLayers > 0)
        {
            //Merge User layer
            logger.finer("... merging user layer", "getNodeData()","configmgr::Backend");
            merge(aComponentData,
                    layers.getConstArray()+nNumDefaultLayers, nNumUserLayers,
                    aRequest.getOptions(), aTemplateProvider );

            // mark this one as done
            ++nCurrentLayer;
        }

        logger.finer("Reading component data finished successfully", "getNodeData()","configmgr::Backend");

        ComponentInstance retCode(aComponentData.extractSchemaTree(),
                                aComponentData.extractTemplatesTree(),
                                aRequest.getComponentName()) ;

        //Register listener with notifier
        if(aListener)
        {
            mNotifier->addListener(aListener, aRequest);
        }
        return ComponentResult(retCode) ;
    }
    catch (com::sun::star::container::NoSuchElementException &) { throw; }
    catch (com::sun::star::uno::RuntimeException &) { throw; }
    catch (uno::Exception & )
    {
        // can only recover if layer merging broke
        if (nCurrentLayer < 0 || layers.getLength() <= nCurrentLayer)
            throw;

        uno::Reference< backenduno::XLayer > xBrokenLayer = layers[nCurrentLayer];

        bool bUserLayerBroken = (nCurrentLayer == nNumDefaultLayers);

        if (!bDefaultRecoveryApproved || bUserLayerBroken)
        {
            logger.info("Parsing data layer failed. Requesting approval for automated recovery", "getNodeData()","configmgr::Backend");
            uno::Any theError = cppu::getCaughtException();
            if (!approveRecovery(theError,xBrokenLayer,bUserLayerBroken))
                cppu::throwException( theError );
        }

        // now do the recovery
        discardLayer(layers,nCurrentLayer);
        if (bUserLayerBroken)
        {
            logger.info("Recovering from broken user data layer - discarding the data", "getNodeData()","configmgr::Backend");
            removeLayerData(xBrokenLayer);
        }
        else
        {
            logger.info("Recovering from broken default layer - skipping", "getNodeData()","configmgr::Backend");
            bDefaultRecoveryApproved = true;
            --nNumDefaultLayers;
        }
    }
    while (true);
}
//------------------------------------------------------------------------------

void BackendAccess::updateNodeData(const UpdateRequest& aUpdate)
    CFG_UNO_THROW_ALL()
{
    rtl::OUString entity = aUpdate.getOptions().getEntity() ;
    rtl::OUString component =
                    aUpdate.getUpdateRoot().getModuleName().toString() ;
    uno::Reference<backenduno::XUpdateHandler> handler ;

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::updateNodeData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "updating component: %s", RTL_LOGFILE_OU2A(component) );

    if (entity.getLength() == 0) {
        handler = mBackend->getOwnUpdateHandler(component) ;
    }
    else { handler = mBackend->getUpdateHandler(component, entity) ; }
    UpdateDispatcher dispatcher(handler, aUpdate.getOptions().getLocale()) ;

    dispatcher.dispatchUpdate(aUpdate.getUpdateRoot().location(),
                              *aUpdate.getUpdateData()) ;
}
//------------------------------------------------------------------------------

NodeResult BackendAccess::getDefaultData(const NodeRequest& aRequest)
    CFG_UNO_THROW_ALL()
{
    rtl::OUString const component = aRequest.getPath().getModuleName().toString() ;

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::getDefaultData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "request path: %s", RTL_LOGFILE_OU2A(aRequest.getPath().toString()) );

    uno::Sequence<uno::Reference<backenduno::XLayer> > const layers =
        this->getLayers(component, aRequest.getOptions()) ;

    sal_Int32 const nNumUserLayers = 1;
    sal_Int32 const nNumDefaultLayers = layers.getLength() - nNumUserLayers;

    MergedComponentData aComponentData;

    if (!this->readDefaultData(aComponentData, component, aRequest.getOptions(), false,
                                layers.getConstArray(),nNumDefaultLayers,
                                this))
    {
        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("Configuration: No data for request. Component \"");
        sMessage.append(component);
        sMessage.appendAscii("\" contains no default data. ");

        OUString const sMsg = sMessage.makeStringAndClear();
        Logger(mContext).finer(sMsg,"getDefaultData()","configmgr::BackendAccess");
        throw com::sun::star::container::NoSuchElementException(sMsg,mBackend);
    }

    std::auto_ptr<ISubtree> aResultTree = aComponentData.extractSchemaTree();

    AbsolutePath aPath = aRequest.getPath();
    if( aPath.begin() != aPath.end())
    {
        for(AbsolutePath::Iterator it=aPath.begin()+1,endIt=aPath.end();it!=endIt; ++it)
        {
            std::auto_ptr<INode> aChild=aResultTree->removeChild(it->getName().toString());
            if(aChild.get()== NULL)
            {
                rtl::OUStringBuffer sMessage;
                sMessage.appendAscii("Configuration: No data for request. Element \"");
                sMessage.append(aPath.toString());
                sMessage.appendAscii("\" does not exist in the default data. ");

                OUString const sMsg = sMessage.makeStringAndClear();
                Logger(mContext).finest(sMsg,"getDefaultData()","configmgr::BackendAccess");
                throw com::sun::star::container::NoSuchElementException(sMsg,mBackend);
            }

            ISubtree *pChildAsSubtree = aChild->asISubtree();
            if(pChildAsSubtree == NULL)
            {
                OUString sMsg = OUString::createFromAscii("BackendAccess::getDefaultData - Node Expected, Found Property: ").concat(it->getName().toString());
                Logger(mContext).finer(sMsg,"getDefaultData()","configmgr::BackendAccess");
                throw MalformedDataException(sMsg, mBackend, uno::Any());
            }
            aResultTree.reset(pChildAsSubtree);
            aChild.release();
        }
    }

    NodeInstance retCode(aResultTree, aRequest.getPath()) ;
    return NodeResult(retCode) ;
}
//------------------------------------------------------------------------------

TemplateResult BackendAccess::getTemplateData(const TemplateRequest& aRequest)
    CFG_UNO_THROW_ALL()
{
    rtl::OUString component = aRequest.getComponentName().toString();

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::getTemplateData()");
    RTL_LOGFILE_CONTEXT_TRACE2(aLog, "requested template: %s/%s",
                                    RTL_LOGFILE_OU2A(aRequest.getComponentName().toString()) ,
                                    aRequest.isComponentRequest() ? "*" : RTL_LOGFILE_OU2A(aRequest.getComponentName().toString()) );

    MergedComponentData aComponentData;

    {
        SchemaBuilder *schemaBuilder = new SchemaBuilder( mContext, component, aComponentData ) ;
        uno::Reference<backenduno::XSchemaHandler> handler = schemaBuilder ;

        uno::Reference<backenduno::XSchema> schema = this->getSchema(component);

        schema->readTemplates(handler) ;
    }

    TemplateInstance::Data aResultData;
    if (aRequest.isComponentRequest())
    {
        aResultData.reset( aComponentData.extractTemplatesTree().release() );
    }
    else
    {
        backenduno::TemplateIdentifier templateId ;
        templateId.Name = aRequest.getTemplateName().toString() ;
        templateId.Component = aRequest.getComponentName().toString() ;

        aResultData = aComponentData.extractTemplateNode(templateId.Name);
    }

    TemplateInstance retCode(aResultData,aRequest.getTemplateName(), aRequest.getComponentName()) ;
    return TemplateResult(retCode) ;
}
//------------------------------------------------------------------------------

uno::Reference< backenduno::XSchema > BackendAccess::getSchema(const OUString& aComponent)
{
    uno::Reference< backenduno::XSchemaSupplier > xSchemaBackend(mBackend, uno::UNO_QUERY);
    OSL_ASSERT(xSchemaBackend.is());

    uno::Reference< backenduno::XSchema > xSchema = xSchemaBackend->getComponentSchema(aComponent) ;
    if (!xSchema.is())
    {
        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("Configuration: No data for request. Component \"");
        sMessage.append(aComponent);
        sMessage.appendAscii("\" is unknown. [No schema available]");

        OUString const sMsg = sMessage.makeStringAndClear();
        Logger(mContext).warning(sMsg,"getSchema()","configmgr::BackendAccess");
        throw com::sun::star::container::NoSuchElementException(sMsg,xSchemaBackend);
    }

    return xSchema;
}
//------------------------------------------------------------------------------

uno::Sequence< uno::Reference<backenduno::XLayer> > BackendAccess::getLayers(const OUString& aComponent,const RequestOptions& aOptions)
{
    rtl::OUString aEntity = aOptions.getEntity() ;

    if (aEntity.getLength() == 0)
    {
        // Use own entity instead
        return mBackend->listOwnLayers(aComponent) ;
    }
    else
    {
        return mBackend->listLayers(aComponent, aEntity) ;
    }
}
//------------------------------------------------------------------------------
void BackendAccess::removeRequestListener(INodeDataListener *aListener,
                                          const ComponentRequest& aRequest)
   CFG_NOTHROW()
{

    OSL_PRECOND(aListener, "ERROR: trying to remove a NULL listener");
    mNotifier->removeListener(aListener, aRequest);
}
//------------------------------------------------------------------------------
} } // configmgr.backend
