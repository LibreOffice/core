/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: backendaccess.cxx,v $
 * $Revision: 1.27 $
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
#include "backendaccess.hxx"
#include "logger.hxx"
#include "matchlocale.hxx"
#include "layermerge.hxx"
#include "schemabuilder.hxx"

#ifndef CONFIGMGR_BACKEND_UPDATEDISPATCHER_HXX
#include "updatedispatch.hxx"
#endif // CONFIGMGR_BACKEND_UPDATEDISPATCHER_HXX
#include "backendnotifier.hxx"
#include "emptylayer.hxx"
#include "filehelper.hxx"
#include "simpleinteractionrequest.hxx"
#include "configinteractionhandler.hxx"
#include <com/sun/star/configuration/backend/XVersionedSchemaSupplier.hpp>
#include <com/sun/star/configuration/backend/XCompositeLayer.hpp>
#include <com/sun/star/configuration/backend/XUpdatableLayer.hpp>
#include <com/sun/star/configuration/backend/XBackendEntities.hpp>
#include <com/sun/star/configuration/backend/MergeRecoveryRequest.hpp>
#include <com/sun/star/configuration/backend/MalformedDataException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "com/sun/star/task/XInteractionHandler.hpp"
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <rtl/logfile.hxx>

#include <cppuhelper/exc_hlp.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif //INCLUDED_VECTOR

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif //INCLUDED_ALGORITHM

#define OU2A(rtlOUString)   (::rtl::OUStringToOString((rtlOUString), RTL_TEXTENCODING_ASCII_US).getStr())
#define RTL_LOGFILE_OU2A(rtlOUString)   OU2A(rtlOUString)

#define OUSTR(txt)  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(txt) )

namespace configmgr { namespace backend {
//------------------------------------------------------------------------------
    namespace task = com::sun::star::task;

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
        throw lang::NullPointerException(rtl::OUString::createFromAscii("BackendAccess: NULL Context passed"), NULL);
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
namespace
{
//------------------------------------------------------------------------------

    static inline
    bool findLocale(std::vector< com::sun::star::lang::Locale > const & seq, com::sun::star::lang::Locale const & loc)
    {
        std::vector< com::sun::star::lang::Locale >::const_iterator first = seq.begin();
        std::vector< com::sun::star::lang::Locale >::const_iterator last  = seq.end();
        for ( ; first != last; ++first)
            if (localehelper::equalLocale(*first, loc))
                return true;
        return false;
    }
//------------------------------------------------------------------------------

    static inline
    void addLocale( com::sun::star::lang::Locale const & aLocale, std::vector< com::sun::star::lang::Locale > & inoutLocales)
    {
        if (!findLocale(inoutLocales,aLocale))
            inoutLocales.push_back(aLocale);
    }
//------------------------------------------------------------------------------

    static rtl::OUString toString(uno::Sequence< rtl::OUString > const & seq, sal_Unicode separator = ',')
    {
        rtl::OUStringBuffer buf;

        if (sal_Int32 const nCount = seq.getLength())
        {
            buf.append(seq[0]);
            for (sal_Int32 ix=1; ix < nCount; ++ix)
                buf.append(separator).append(seq[ix]);
        }
        else
            buf.appendAscii("<none>");

        return buf.makeStringAndClear();
    }
//------------------------------------------------------------------------------

    static
    uno::Sequence< rtl::OUString > intersect(uno::Sequence< rtl::OUString > const & seq1, uno::Sequence< rtl::OUString > const & seq2)
    {
        sal_Int32 const len1 = seq1.getLength();
        uno::Sequence< rtl::OUString > aResult(len1);

        rtl::OUString const * const beg2 = seq2.getConstArray();
        rtl::OUString const * const end2 = beg2 + seq2.getLength();

        sal_Int32 ix = 0;
        for (sal_Int32 i1 = 0; i1 < len1; ++i1)
        {
            if (std::find(beg2,end2,seq1[i1]) != end2)
                aResult[ix++] = seq1[i1];
        }
        aResult.realloc(ix);
        return aResult;
    }
//------------------------------------------------------------------------------
} // anonymous namespace
//------------------------------------------------------------------------------

// helper used by the binary cache
uno::Sequence< rtl::OUString >
    getAvailableLocales(const uno::Reference<backenduno::XLayer> * pLayers, sal_Int32 nNumLayers)
{
    uno::Sequence< rtl::OUString > aResult;

    for (sal_Int32 i = 0 ; i < nNumLayers ; ++ i)
    {
        uno::Reference<backenduno::XCompositeLayer> compositeLayer(
             pLayers [i], uno::UNO_QUERY) ;

        if (compositeLayer.is())
        {
            uno::Sequence<rtl::OUString> aLocales = compositeLayer->listSubLayerIds();

            if (aResult.getLength() == 0)
            {
                aResult = aLocales;
            }
            else
            {
                OSL_TRACE("Warning: multiple composite layers found. Detection of available locales is inaccurate.");
                // be defensive: only locales present in all composite layers are 'available'
                aResult= intersect(aResult,aLocales);
            }
        }
    }
    return aResult;
}
//------------------------------------------------------------------------------
static rtl::OUString getLayerURL(const uno::Reference<backenduno::XLayer> & aLayer);

void BackendAccess::merge(
        MergedComponentData& aData,
        const uno::Reference<backenduno::XLayer> * pLayers,
        sal_Int32 aNumLayers,
        com::sun::star::lang::Locale const & aRequestedLocale,
        std::vector< com::sun::star::lang::Locale > & inoutMergedLocales,
        ITemplateDataProvider *aTemplateProvider,
        sal_Int32 * pLayersMerged)
    SAL_THROW((com::sun::star::uno::Exception))
{
    LayerMergeHandler * pMerger = new LayerMergeHandler(mContext, aData, aTemplateProvider );
    uno::Reference<backenduno::XLayerHandler> xLayerMerger(pMerger);

    Logger logger(mContext);

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::merge()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "merging %d layers", int(aNumLayers) );

    rtl::OUString const & aLanguage = aRequestedLocale.Language;
    bool const needAllLanguages = localehelper::isAnyLanguage(aLanguage);

    if (aLanguage.getLength() && !needAllLanguages)
    {
        if (!localehelper::isDefaultLanguage(aLanguage))
            addLocale(aRequestedLocale,inoutMergedLocales);
    }
    bool const needLocalizedData = needAllLanguages || !inoutMergedLocales.empty();

    if (logger.isLogging(LogLevel::FINEST))
    {
        if (!needLocalizedData)
            logger.finest("Starting merge with NO locales", "merge()","configmgr::Backend");

        else if (needAllLanguages)
            logger.finest("Starting merge for ALL locales", "merge()","configmgr::Backend");

        else
            logger.finest(OUSTR("Starting merge for locale(s): ") +
                                toString(localehelper::makeIsoSequence(inoutMergedLocales)),
                                "merge()","configmgr::Backend");
    }

    if (pLayersMerged) *pLayersMerged = 0;

    for (sal_Int32 i = 0 ; i < aNumLayers ; ++ i)
    {
        if (logger.isLogging(LogLevel::FINEST))
            logger.finest(OUSTR("+ Merging layer: ") + getLayerURL(pLayers[i]),
                                "merge()","configmgr::Backend");

        pMerger->prepareLayer() ;
        pLayers [i]->readData(xLayerMerger) ;

        if (needLocalizedData)
        {
            uno::Reference<backenduno::XCompositeLayer> compositeLayer(
                 pLayers [i], uno::UNO_QUERY) ;

            if (compositeLayer.is())
            {
                uno::Sequence<rtl::OUString> aSubLayerIds = compositeLayer->listSubLayerIds();
                logger.finest(OUSTR("++ Found locales: ") + toString(aSubLayerIds),
                                    "merge()","configmgr::Backend");

                for (sal_Int32 j = 0; j < aSubLayerIds.getLength(); ++j)
                {
                    rtl::OUString const & aLocaleIso = aSubLayerIds[j];
                    com::sun::star::lang::Locale aLocale = localehelper::makeLocale(aLocaleIso);

                    // requesting de-CH, we accept de-CH and de, but not de-DE
                    const localehelper::MatchQuality kMatchAccept = localehelper::MATCH_LANGUAGE_PLAIN;

                    if (needAllLanguages || localehelper::isMatch(aLocale,inoutMergedLocales,kMatchAccept))
                    {
                        if(pMerger->prepareSublayer(aLocaleIso))
                        {
                            logger.finest(OUSTR("++ Merging sublayer for locale: ") + aLocaleIso,
                                                "merge()","configmgr::Backend");
                            compositeLayer->readSubLayerData(xLayerMerger,aLocaleIso) ;
                            addLocale(aLocale,inoutMergedLocales);
                        }
                        // else dropLocale(aLocale,inoutMergedLocales); ?
                    }
                }
            }
        }
        if (pLayersMerged) ++*pLayersMerged;
    }
}
//------------------------------------------------------------------------------

bool BackendAccess::readDefaultData( MergedComponentData & aComponentData,
                                        rtl::OUString const & aComponent,
                                        RequestOptions const & aOptions,
                                        bool bIncludeTemplates,
                                        const uno::Reference<backenduno::XLayer> * pLayers,
                                        sal_Int32 nNumLayers,
                                        ITemplateDataProvider *aTemplateProvider,
                                        sal_Int32 * pLayersMerged)
    SAL_THROW((com::sun::star::uno::Exception))
{
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog1, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::readDefaultData()");

    Logger logger(mContext);
    const sal_Int32 detail = LogLevel::FINER;
    bool const bLogDetail = logger.isLogging(detail);

    rtl::OUString const aSchemaVersion = this->getSchemaVersion(aComponent);

    if (logger.isLogging(LogLevel::FINE))
    {
        rtl::OUStringBuffer aMsg;
        aMsg.appendAscii("Reading data for component \"").append(aComponent).appendAscii("\"");
        aMsg.appendAscii(" [version=").append(aSchemaVersion).appendAscii("]");

        logger.fine( aMsg.makeStringAndClear(), "readDefaultData()","configmgr::Backend");
    }

    com::sun::star::lang::Locale const aRequestedLocale = localehelper::makeLocale(aOptions.getLocale());
    std::vector< com::sun::star::lang::Locale > aKnownLocales;

    if (bLogDetail) logger.log(detail, "... attempt to read from binary cache", "readDefaultData()","configmgr::Backend");
    bool bCacheHit = mBinaryCache.readComponentData(aComponentData, getServiceFactory(),
                                                    aComponent, aSchemaVersion,
                                                    aOptions.getEntity(),
                                                    aRequestedLocale, aKnownLocales,
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
        this->merge(aComponentData, pLayers, nNumLayers, aRequestedLocale, aKnownLocales, aTemplateProvider, pLayersMerged );
        promoteToDefault(aComponentData);

        if (mBinaryCache.isCacheEnabled(aOptions.getEntity()))
        {
            if (bLogDetail) logger.log(detail, "... creating binary cache", "readDefaultData()","configmgr::Backend");
            bool bWriteSuccess = mBinaryCache.writeComponentData( aComponentData, getServiceFactory(),
                                                                  aComponent, aSchemaVersion,
                                                                  aOptions.getEntity(), aKnownLocales,
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

static rtl::OUString getLayerURL(const uno::Reference<backenduno::XLayer> & aLayer)
{
    try
    {
        namespace beans = com::sun::star::beans;
        uno::Reference< beans::XPropertySet > xLayerProps( aLayer, uno::UNO_QUERY );
        if (xLayerProps.is())
        {
            uno::Any aPropVal = xLayerProps->getPropertyValue( OUSTR("URL") );
            rtl::OUString aResult;
            if (aPropVal >>= aResult)
                return aResult;
        }
        OSL_TRACE("Warning - Cannot get location of layer data\n");
    }
    catch (uno::Exception & e)
    {
        OSL_TRACE("Warning - Configuration: Retrieving layer URL failed: [%s]\n", OU2A(e.Message));
    }
    // TODO: use better fallback, e.g. ServiceName
    const char * const aFallback = aLayer.is() ? "<Unknown Layer Type>" : "<NULL Layer>";
    return rtl::OUString::createFromAscii(aFallback);
}
//------------------------------------------------------------------------------
static inline
rtl::OUString getLayerIdentifier(const uno::Reference<backenduno::XLayer> & aLayer)
{
    return getLayerURL(aLayer);
}

//------------------------------------------------------------------------------
static void removeLayerData(uno::Reference< backenduno::XLayer > const & xLayer)
{
    OSL_ASSERT(xLayer.is());

    uno::Reference< backenduno::XUpdatableLayer > xLayerRemover(xLayer,uno::UNO_QUERY);
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

namespace {

class RecursiveHandler:
    public cppu::WeakImplHelper1< task::XInteractionHandler >
{
public:
    explicit RecursiveHandler(
        uno::Reference< task::XInteractionHandler > const & outer):
        m_outer(outer) {}

    virtual void SAL_CALL handle(
        uno::Reference< task::XInteractionRequest > const & request)
        throw (uno::RuntimeException)
    {
        backenduno::MergeRecoveryRequest req;
        if (request->getRequest() >>= req) {
            uno::Sequence< uno::Reference< task::XInteractionContinuation > >
                cs(request->getContinuations());
            for (sal_Int32 i = 0; i < cs.getLength(); ++i) {
                uno::Reference< task::XInteractionDisapprove > dis(
                    cs[i], uno::UNO_QUERY);
                if (dis.is()) {
                    dis->select();
                    break;
                }
            }
        } else if (m_outer.is()) {
            m_outer->handle(request);
        }
    }

private:
    RecursiveHandler(RecursiveHandler &); // not defined
    void operator =(RecursiveHandler &); // not defined

    virtual ~RecursiveHandler() {}

    uno::Reference< task::XInteractionHandler > m_outer;
};

}

bool BackendAccess::approveRecovery(const uno::Any & aMergeException,
                                    const uno::Reference<backenduno::XLayer> & aBrokenLayer,
                                    bool bUserLayerData)
    SAL_THROW((com::sun::star::uno::Exception))
{
    sal_uInt32 const k_supported_choices = apihelper::CONTINUATION_APPROVE | apihelper::CONTINUATION_DISAPPROVE;

    sal_uInt32 chosen = apihelper::CONTINUATION_UNKNOWN;

    apihelper::ConfigurationInteractionHandler handler;
    try {
        uno::Reference< task::XInteractionHandler > h(handler.get());
        if (h.is()) {
            handler.setRecursive(new RecursiveHandler(h));
            rtl::Reference< apihelper::SimpleInteractionRequest > req(
                new apihelper::SimpleInteractionRequest(
                    uno::makeAny(
                        backenduno::MergeRecoveryRequest(
                            rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "Recover from configuration merge"
                                    " failure")),
                            aBrokenLayer, aMergeException,
                            getLayerIdentifier(aBrokenLayer),
                            (bUserLayerData
                             && (uno::Reference< backenduno::XUpdatableLayer >::query(aBrokenLayer).is()
                                 || FileHelper::fileExists(
                                     getLayerURL(aBrokenLayer)))))),
                    k_supported_choices));
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
    // no choice available - default: approve, if user data
    return bUserLayerData;
}
//------------------------------------------------------------------------------

ResultHolder< ComponentInstance > BackendAccess::getNodeData(const ComponentRequest& aRequest,
                                           ITemplateDataProvider *_aTemplateProvider,
                                           INodeDataListener *aListener)
    SAL_THROW((com::sun::star::uno::Exception))
{
    rtl::OUString const component = aRequest.getComponentName();
    ITemplateDataProvider * const aTemplateProvider = _aTemplateProvider ? _aTemplateProvider : this;

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::getNodeData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "request path: %s", RTL_LOGFILE_OU2A(component) );

    uno::Sequence<uno::Reference<backenduno::XLayer> > layers =
        this->getLayers(component, aRequest.getOptions()) ;

    sal_Int32 const k_NumUserLayers = 1;

    sal_Int32 nNumDefaultLayers = layers.getLength() - k_NumUserLayers;
    sal_Int32 nCurrentLayer( -1 );
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
            //Merge User layer (with all locales)
            logger.finer("... merging user layer", "getNodeData()","configmgr::Backend");

            std::vector< com::sun::star::lang::Locale > aLocales;
            merge(aComponentData,
                    layers.getConstArray()+nNumDefaultLayers, nNumUserLayers,
                    localehelper::getAnyLocale(), aLocales, aTemplateProvider );

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
        return ResultHolder< ComponentInstance >(retCode) ;
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
    SAL_THROW((com::sun::star::uno::Exception))
{
    rtl::OUString entity = aUpdate.getOptions().getEntity() ;
    rtl::OUString component =
                    aUpdate.getUpdateRoot().getModuleName();
    uno::Reference<backenduno::XUpdateHandler> handler ;

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::updateNodeData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "updating component: %s", RTL_LOGFILE_OU2A(component) );

    if (entity.getLength() == 0) {
        handler = mBackend->getOwnUpdateHandler(component) ;
    }
    else { handler = mBackend->getUpdateHandler(component, entity) ; }
    UpdateDispatcher dispatcher(handler, aUpdate.getOptions().getLocale()) ;

    dispatcher.dispatchUpdate(aUpdate.getUpdateRoot(),
                              *aUpdate.getUpdateData()) ;
}
//------------------------------------------------------------------------------

ResultHolder< NodeInstance > BackendAccess::getDefaultData(const NodeRequest& aRequest)
    SAL_THROW((com::sun::star::uno::Exception))
{
    rtl::OUString const component = aRequest.getPath().getModuleName();

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

        rtl::OUString const sMsg = sMessage.makeStringAndClear();
        Logger(mContext).finer(sMsg,"getDefaultData()","configmgr::BackendAccess");
        throw com::sun::star::container::NoSuchElementException(sMsg,mBackend);
    }

    std::auto_ptr<ISubtree> aResultTree = aComponentData.extractSchemaTree();

    configuration::AbsolutePath aPath = aRequest.getPath();
    if( aPath.begin() != aPath.end())
    {
        for(std::vector<configuration::Path::Component>::const_reverse_iterator it=aPath.begin()+1,endIt=aPath.end();it!=endIt; ++it)
        {
            std::auto_ptr<INode> aChild=aResultTree->removeChild(it->getName());
            if(aChild.get()== NULL)
            {
                rtl::OUStringBuffer sMessage;
                sMessage.appendAscii("Configuration: No data for request. Element \"");
                sMessage.append(aPath.toString());
                sMessage.appendAscii("\" does not exist in the default data. ");

                rtl::OUString const sMsg = sMessage.makeStringAndClear();
                Logger(mContext).finest(sMsg,"getDefaultData()","configmgr::BackendAccess");
                throw com::sun::star::container::NoSuchElementException(sMsg,mBackend);
            }

            ISubtree *pChildAsSubtree = aChild->asISubtree();
            if(pChildAsSubtree == NULL)
            {
                rtl::OUString sMsg = rtl::OUString::createFromAscii("BackendAccess::getDefaultData - Node Expected, Found Property: ").concat(it->getName());
                Logger(mContext).finer(sMsg,"getDefaultData()","configmgr::BackendAccess");
                throw backenduno::MalformedDataException(sMsg, mBackend, uno::Any());
            }
            aResultTree.reset(pChildAsSubtree);
            aChild.release();
        }
    }

    NodeInstance retCode(aResultTree, aRequest.getPath()) ;
    return ResultHolder< NodeInstance >(retCode) ;
}
//------------------------------------------------------------------------------

ResultHolder< TemplateInstance > BackendAccess::getTemplateData(const TemplateRequest& aRequest)
    SAL_THROW((com::sun::star::uno::Exception))
{
    rtl::OUString component = aRequest.getComponentName();

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
        templateId.Name = aRequest.getTemplateName();
        templateId.Component = aRequest.getComponentName();

        aResultData = aComponentData.extractTemplateNode(templateId.Name);
    }

    TemplateInstance retCode(aResultData,aRequest.getTemplateName(), aRequest.getComponentName()) ;
    return ResultHolder< TemplateInstance >(retCode) ;
}
//------------------------------------------------------------------------------

uno::Reference< backenduno::XSchema > BackendAccess::getSchema(const rtl::OUString& aComponent)
{
    uno::Reference< backenduno::XSchemaSupplier > xSchemaBackend(mBackend, uno::UNO_QUERY_THROW);
    OSL_ASSERT(xSchemaBackend.is());

    uno::Reference< backenduno::XSchema > xSchema = xSchemaBackend->getComponentSchema(aComponent) ;
    if (!xSchema.is())
    {
        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("Configuration: No data for request. Component \"");
        sMessage.append(aComponent);
        sMessage.appendAscii("\" is unknown. [No schema available]");

        rtl::OUString const sMsg = sMessage.makeStringAndClear();
        Logger(mContext).warning(sMsg,"getSchema()","configmgr::BackendAccess");
        throw com::sun::star::container::NoSuchElementException(sMsg,xSchemaBackend);
    }

    return xSchema;
}
//------------------------------------------------------------------------------

rtl::OUString BackendAccess::getSchemaVersion(const rtl::OUString& aComponent)
{
    uno::Reference< backenduno::XVersionedSchemaSupplier > xSchemaBackend(mBackend, uno::UNO_QUERY);
    if (xSchemaBackend.is())
        return xSchemaBackend->getSchemaVersion(aComponent);
    else
        return rtl::OUString();
}
//------------------------------------------------------------------------------

uno::Sequence< uno::Reference<backenduno::XLayer> > BackendAccess::getLayers(const rtl::OUString& aComponent,const RequestOptions& aOptions)
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
   SAL_THROW(())
{

    OSL_PRECOND(aListener, "ERROR: trying to remove a NULL listener");
    mNotifier->removeListener(aListener, aRequest);
}
//------------------------------------------------------------------------------
} } // configmgr.backend
