/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <loadenv/loadenv.hxx>

#include <loadenv/targethelper.hxx>
#include <framework/framelistanalyzer.hxx>

#include <interaction/quietinteraction.hxx>
#include <properties.h>
#include <protocols.h>
#include <services.h>
#include <comphelper/interaction.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/propertysequence.hxx>
#include <framework/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/OfficeFrameLoader.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/FrameLoaderFactory.hpp>
#include <com/sun/star/frame/ContentHandlerFactory.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/syswin.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <unotools/ucbhelper.hxx>
#include <comphelper/configurationhelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <vcl/errcode.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/profilezone.hxx>
#include <classes/taskcreator.hxx>

const char PROP_TYPES[] = "Types";
const char PROP_NAME[] = "Name";

namespace framework {

using namespace com::sun::star;

class LoadEnvListener : public ::cppu::WeakImplHelper< css::frame::XLoadEventListener      ,
                                                        css::frame::XDispatchResultListener >
{
    private:
        osl::Mutex m_mutex;
        bool m_bWaitingResult;
        LoadEnv* m_pLoadEnv;

    public:

        explicit LoadEnvListener(LoadEnv* pLoadEnv)
            : m_bWaitingResult(true)
            , m_pLoadEnv(pLoadEnv)
        {
        }

        // frame.XLoadEventListener
        virtual void SAL_CALL loadFinished(const css::uno::Reference< css::frame::XFrameLoader >& xLoader) override;

        virtual void SAL_CALL loadCancelled(const css::uno::Reference< css::frame::XFrameLoader >& xLoader) override;

        // frame.XDispatchResultListener
        virtual void SAL_CALL dispatchFinished(const css::frame::DispatchResultEvent& aEvent) override;

        // lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;
};

LoadEnv::LoadEnv(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : m_xContext(xContext)
    , m_nSearchFlags(0)
    , m_eFeature(LoadEnvFeatures::NONE)
    , m_eContentType(E_UNSUPPORTED_CONTENT)
    , m_bCloseFrameOnError(false)
    , m_bReactivateControllerOnError(false)
    , m_bLoaded( false )
{
}

LoadEnv::~LoadEnv()
{
}

css::uno::Reference< css::lang::XComponent > LoadEnv::loadComponentFromURL(const css::uno::Reference< css::frame::XComponentLoader >&    xLoader,
                                                                           const css::uno::Reference< css::uno::XComponentContext >&     xContext  ,
                                                                           const OUString&                                        sURL   ,
                                                                           const OUString&                                        sTarget,
                                                                                 sal_Int32                                               nSearchFlags ,
                                                                           const css::uno::Sequence< css::beans::PropertyValue >&        lArgs  )
{
    css::uno::Reference< css::lang::XComponent > xComponent;
    comphelper::ProfileZone aZone("loadComponentFromURL");

    try
    {
        LoadEnv aEnv(xContext);

        LoadEnvFeatures loadEnvFeatures = LoadEnvFeatures::WorkWithUI;
        comphelper::NamedValueCollection aDescriptor( lArgs );
        // tdf#118238 Only disable UI interaction when loading as hidden
        if (aDescriptor.get("Hidden") == uno::Any(true) || Application::IsHeadlessModeEnabled())
            loadEnvFeatures = LoadEnvFeatures::NONE;

        aEnv.initializeLoading(sURL,
                               lArgs,
                               css::uno::Reference< css::frame::XFrame >(xLoader, css::uno::UNO_QUERY),
                               sTarget,
                               nSearchFlags,
                               loadEnvFeatures);
        aEnv.startLoading();
        aEnv.waitWhileLoading(); // wait for ever!

        xComponent = aEnv.getTargetComponent();
    }
    catch(const LoadEnvException& ex)
    {
        switch(ex.m_nID)
        {
            case LoadEnvException::ID_INVALID_MEDIADESCRIPTOR:
                throw css::lang::IllegalArgumentException(
                    "Optional list of arguments seem to be corrupted.", xLoader, 4);

            case LoadEnvException::ID_UNSUPPORTED_CONTENT:
                throw css::lang::IllegalArgumentException(
                    "Unsupported URL <" + sURL + ">: \"" + ex.m_sMessage + "\"",
                    xLoader, 1);

            default:
                SAL_WARN(
                    "fwk.loadenv",
                    "caught LoadEnvException " << +ex.m_nID << " \""
                        << ex.m_sMessage << "\""
                        << (ex.m_exOriginal.has<css::uno::Exception>()
                            ? (", " + ex.m_exOriginal.getValueTypeName() + " \""
                               + (ex.m_exOriginal.get<css::uno::Exception>().
                                  Message)
                               + "\"")
                            : OUString())
                        << " while loading <" << sURL << ">");
                xComponent.clear();
                break;
        }
    }

    return xComponent;
}

namespace {

utl::MediaDescriptor addModelArgs(const uno::Sequence<beans::PropertyValue>& rDescriptor)
{
    utl::MediaDescriptor rResult(rDescriptor);
    uno::Reference<frame::XModel> xModel(rResult.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_MODEL(), uno::Reference<frame::XModel>()));

    if (xModel.is())
    {
        utl::MediaDescriptor aModelArgs(xModel->getArgs());
        utl::MediaDescriptor::iterator pIt = aModelArgs.find( utl::MediaDescriptor::PROP_MACROEXECUTIONMODE());
        if (pIt != aModelArgs.end())
            rResult[utl::MediaDescriptor::PROP_MACROEXECUTIONMODE()] = pIt->second;
    }

    return rResult;
}

}

void LoadEnv::initializeLoading(const OUString& sURL, const uno::Sequence<beans::PropertyValue>& lMediaDescriptor,
        const uno::Reference<frame::XFrame>& xBaseFrame, const OUString& sTarget,
        sal_Int32 nSearchFlags, LoadEnvFeatures eFeature)
{
    osl::MutexGuard g(m_mutex);

    // Handle still running processes!
    if (m_xAsynchronousJob.is())
        throw LoadEnvException(LoadEnvException::ID_STILL_RUNNING);

    // take over all new parameters.
    m_xTargetFrame.clear();
    m_xBaseFrame = xBaseFrame;
    m_lMediaDescriptor = addModelArgs(lMediaDescriptor);
    m_sTarget = sTarget;
    m_nSearchFlags = nSearchFlags;
    m_eFeature = eFeature;
    m_eContentType = E_UNSUPPORTED_CONTENT;
    m_bCloseFrameOnError = false;
    m_bReactivateControllerOnError = false;
    m_bLoaded = false;

    // try to find out, if its really a content, which can be loaded or must be "handled"
    // We use a default value for this in-parameter. Then we have to start a complex check method
    // internally. But if this check was already done outside it can be suppressed to perform
    // the load request. We take over the result then!
    m_eContentType = LoadEnv::classifyContent(sURL, lMediaDescriptor);
    if (m_eContentType == E_UNSUPPORTED_CONTENT)
        throw LoadEnvException(LoadEnvException::ID_UNSUPPORTED_CONTENT, "from LoadEnv::initializeLoading");

    // make URL part of the MediaDescriptor
    // It doesn't matter if it is already an item of it.
    // It must be the same value... so we can overwrite it :-)
    m_lMediaDescriptor[utl::MediaDescriptor::PROP_URL()] <<= sURL;

    // parse it - because some following code require that
    m_aURL.Complete = sURL;
    uno::Reference<util::XURLTransformer> xParser(util::URLTransformer::create(m_xContext));
    xParser->parseStrict(m_aURL);

    // BTW: Split URL and JumpMark ...
    // Because such mark is an explicit value of the media descriptor!
    if (!m_aURL.Mark.isEmpty())
        m_lMediaDescriptor[utl::MediaDescriptor::PROP_JUMPMARK()] <<= m_aURL.Mark;

    // By the way: remove the old and deprecated value "FileName" from the descriptor!
    utl::MediaDescriptor::iterator pIt = m_lMediaDescriptor.find(utl::MediaDescriptor::PROP_FILENAME());
    if (pIt != m_lMediaDescriptor.end())
        m_lMediaDescriptor.erase(pIt);

    // patch the MediaDescriptor, so it fulfil the outside requirements
    // Means especially items like e.g. UI InteractionHandler, Status Indicator,
    // MacroExecutionMode, etc.

    /*TODO progress is bound to a frame ... How can we set it here? */

    // UI mode
    const bool bUIMode =
        (m_eFeature & LoadEnvFeatures::WorkWithUI) &&
        !m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_HIDDEN(), false) &&
        !m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_PREVIEW(), false);

    initializeUIDefaults(m_xContext, m_lMediaDescriptor, bUIMode, &m_pQuietInteraction);
}

void LoadEnv::initializeUIDefaults( const css::uno::Reference< css::uno::XComponentContext >& i_rxContext,
                                    utl::MediaDescriptor& io_lMediaDescriptor, const bool i_bUIMode,
                                    rtl::Reference<QuietInteraction>* o_ppQuietInteraction )
{
    css::uno::Reference< css::task::XInteractionHandler > xInteractionHandler;
    sal_Int16                                             nMacroMode;
    sal_Int16                                             nUpdateMode;

    if ( i_bUIMode )
    {
        nMacroMode  = css::document::MacroExecMode::USE_CONFIG;
        nUpdateMode = css::document::UpdateDocMode::ACCORDING_TO_CONFIG;
        try
        {
            xInteractionHandler.set( css::task::InteractionHandler::createWithParent( i_rxContext, nullptr ), css::uno::UNO_QUERY_THROW );
        }
        catch(const css::uno::RuntimeException&) {throw;}
        catch(const css::uno::Exception&       ) {      }
    }
    // hidden mode
    else
    {
        nMacroMode  = css::document::MacroExecMode::NEVER_EXECUTE;
        nUpdateMode = css::document::UpdateDocMode::NO_UPDATE;
        rtl::Reference<QuietInteraction> pQuietInteraction = new QuietInteraction();
        xInteractionHandler = pQuietInteraction.get();
        if ( o_ppQuietInteraction != nullptr )
        {
            *o_ppQuietInteraction = pQuietInteraction;
        }
    }

    if ( xInteractionHandler.is() )
    {
        if( io_lMediaDescriptor.find(utl::MediaDescriptor::PROP_INTERACTIONHANDLER()) == io_lMediaDescriptor.end() )
        {
            io_lMediaDescriptor[utl::MediaDescriptor::PROP_INTERACTIONHANDLER()] <<= xInteractionHandler;
        }
        if( io_lMediaDescriptor.find(utl::MediaDescriptor::PROP_AUTHENTICATIONHANDLER()) == io_lMediaDescriptor.end() )
        {
            io_lMediaDescriptor[utl::MediaDescriptor::PROP_AUTHENTICATIONHANDLER()] <<= xInteractionHandler;
        }
    }

    if (io_lMediaDescriptor.find(utl::MediaDescriptor::PROP_MACROEXECUTIONMODE()) == io_lMediaDescriptor.end())
        io_lMediaDescriptor[utl::MediaDescriptor::PROP_MACROEXECUTIONMODE()] <<= nMacroMode;

    if (io_lMediaDescriptor.find(utl::MediaDescriptor::PROP_UPDATEDOCMODE()) == io_lMediaDescriptor.end())
        io_lMediaDescriptor[utl::MediaDescriptor::PROP_UPDATEDOCMODE()] <<= nUpdateMode;
}

void LoadEnv::startLoading()
{
    // SAFE ->
    osl::ClearableMutexGuard aReadLock(m_mutex);

    // Handle still running processes!
    if (m_xAsynchronousJob.is())
        throw LoadEnvException(LoadEnvException::ID_STILL_RUNNING);

    // content can not be loaded or handled
    // check "classifyContent()" failed before ...
    if (m_eContentType == E_UNSUPPORTED_CONTENT)
        throw LoadEnvException(LoadEnvException::ID_UNSUPPORTED_CONTENT, "from LoadEnv::startLoading");

    // <- SAFE
    aReadLock.clear();

    // detect its type/filter etc.
    // These information will be available by the
    // used descriptor member afterwards and is needed
    // for all following operations!
    // Note: An exception will be thrown, in case operation was not successfully ...
    if (m_eContentType != E_CAN_BE_SET)/* Attention: special feature to set existing component on a frame must ignore type detection! */
        impl_detectTypeAndFilter();

    // start loading the content ...
    // Attention: Don't check m_eContentType deeper then UNSUPPORTED/SUPPORTED!
    // Because it was made in the easiest way ... may a flat detection was made only.
    // And such simple detection can fail some times .-)
    // Use another strategy here. Try it and let it run into the case "loading not possible".
    bool bStarted = false;
    if (
        (m_eFeature & LoadEnvFeatures::AllowContentHandler) &&
        (m_eContentType                        != E_CAN_BE_SET          )   /* Attention: special feature to set existing component on a frame must ignore type detection! */
       )
    {
        bStarted = impl_handleContent();
    }

    if (!bStarted)
        bStarted = impl_loadContent();

    // not started => general error
    // We can't say - what was the reason for.
    if (!bStarted)
        throw LoadEnvException(
            LoadEnvException::ID_GENERAL_ERROR, "not started");
}

/*-----------------------------------------------
    TODO
        First draft does not implement timeout using [ms].
        Current implementation counts yield calls only ...
-----------------------------------------------*/
bool LoadEnv::waitWhileLoading(sal_uInt32 nTimeout)
{
    // Because it's not a good idea to block the main thread
    // (and we can't be sure that we are currently not used inside the
    // main thread!), we can't use conditions here really. We must yield
    // in an intelligent manner :-)

    sal_Int32 nTime = nTimeout;
    while(true)
    {
        // SAFE -> ------------------------------
        osl::ClearableMutexGuard aReadLock1(m_mutex);
        if (!m_xAsynchronousJob.is())
            break;
        aReadLock1.clear();
        // <- SAFE ------------------------------

        Application::Yield();

        // forever!
        if (nTimeout==0)
            continue;

        // timed out?
        --nTime;
        if (nTime<1)
            break;
    }

    osl::MutexGuard g(m_mutex);
    return !m_xAsynchronousJob.is();
}

css::uno::Reference< css::lang::XComponent > LoadEnv::getTargetComponent() const
{
    osl::MutexGuard g(m_mutex);

    if (!m_xTargetFrame.is())
        return css::uno::Reference< css::lang::XComponent >();

    css::uno::Reference< css::frame::XController > xController = m_xTargetFrame->getController();
    if (!xController.is())
        return css::uno::Reference< css::lang::XComponent >(m_xTargetFrame->getComponentWindow(), css::uno::UNO_QUERY);

    css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
    if (!xModel.is())
        return css::uno::Reference< css::lang::XComponent >(xController, css::uno::UNO_QUERY);

    return css::uno::Reference< css::lang::XComponent >(xModel, css::uno::UNO_QUERY);
}

void SAL_CALL LoadEnvListener::loadFinished(const css::uno::Reference< css::frame::XFrameLoader >&)
{
    osl::MutexGuard g(m_mutex);
    if (m_bWaitingResult)
        m_pLoadEnv->impl_setResult(true);
    m_bWaitingResult = false;
}

void SAL_CALL LoadEnvListener::loadCancelled(const css::uno::Reference< css::frame::XFrameLoader >&)
{
    osl::MutexGuard g(m_mutex);
    if (m_bWaitingResult)
        m_pLoadEnv->impl_setResult(false);
    m_bWaitingResult = false;
}

void SAL_CALL LoadEnvListener::dispatchFinished(const css::frame::DispatchResultEvent& aEvent)
{
    osl::MutexGuard g(m_mutex);

    if (!m_bWaitingResult)
        return;

    switch(aEvent.State)
    {
        case css::frame::DispatchResultState::FAILURE :
            m_pLoadEnv->impl_setResult(false);
            break;

        case css::frame::DispatchResultState::SUCCESS :
            m_pLoadEnv->impl_setResult(false);
            break;

        case css::frame::DispatchResultState::DONTKNOW :
            m_pLoadEnv->impl_setResult(false);
            break;
    }
    m_bWaitingResult = false;
}

void SAL_CALL LoadEnvListener::disposing(const css::lang::EventObject&)
{
    osl::MutexGuard g(m_mutex);
    if (m_bWaitingResult)
        m_pLoadEnv->impl_setResult(false);
    m_bWaitingResult = false;
}

void LoadEnv::impl_setResult(bool bResult)
{
    osl::MutexGuard g(m_mutex);

    m_bLoaded = bResult;

    impl_reactForLoadingState();

    // clearing of this reference will unblock waitWhileLoading()!
    // So we must be sure, that loading process was really finished.
    // => do it as last operation of this method ...
    m_xAsynchronousJob.clear();
}

/*-----------------------------------------------
    TODO: Is it a good idea to change Sequence<>
          parameter to stl-adapter?
-----------------------------------------------*/
LoadEnv::EContentType LoadEnv::classifyContent(const OUString&                                 sURL            ,
                                               const css::uno::Sequence< css::beans::PropertyValue >& lMediaDescriptor)
{

    // (i) Filter some special well known URL protocols,
    //     which can not be handled or loaded in general.
    //     Of course an empty URL must be ignored here too.
    //     Note: These URL schemata are fix and well known ...
    //     But there can be some additional ones, which was not
    //     defined at implementation time of this class :-(
    //     So we have to make sure, that the following code
    //     can detect such protocol schemata too :-)

    if(
        (sURL.isEmpty()                                          ) ||
        (ProtocolCheck::isProtocol(sURL,EProtocol::Uno    )) ||
        (ProtocolCheck::isProtocol(sURL,EProtocol::Slot   )) ||
        (ProtocolCheck::isProtocol(sURL,EProtocol::Macro  )) ||
        (ProtocolCheck::isProtocol(sURL,EProtocol::Service)) ||
        (ProtocolCheck::isProtocol(sURL,EProtocol::MailTo )) ||
        (ProtocolCheck::isProtocol(sURL,EProtocol::News   ))
      )
    {
        return E_UNSUPPORTED_CONTENT;
    }

    // (ii) Some special URLs indicates a given input stream,
    //      a full featured document model directly or
    //      specify a request for opening an empty document.
    //      Such contents are loadable in general.
    //      But we have to check, if the media descriptor contains
    //      all needed resources. If they are missing - the following
    //      load request will fail.

    /* Attention: The following code can't work on such special URLs!
                  It should not break the office .. but it make no sense
                  to start expensive object creations and complex search
                  algorithm if its clear, that such URLs must be handled
                  in a special way .-)
    */

    // creation of new documents
    if (ProtocolCheck::isProtocol(sURL,EProtocol::PrivateFactory))
        return E_CAN_BE_LOADED;

    // using of an existing input stream
    utl::MediaDescriptor                 stlMediaDescriptor(lMediaDescriptor);
    utl::MediaDescriptor::const_iterator pIt;
    if (ProtocolCheck::isProtocol(sURL,EProtocol::PrivateStream))
    {
        pIt = stlMediaDescriptor.find(utl::MediaDescriptor::PROP_INPUTSTREAM());
        css::uno::Reference< css::io::XInputStream > xStream;
        if (pIt != stlMediaDescriptor.end())
            pIt->second >>= xStream;
        if (xStream.is())
            return E_CAN_BE_LOADED;
        SAL_INFO("fwk.loadenv", "LoadEnv::classifyContent(): loading from stream with right URL but invalid stream detected");
        return E_UNSUPPORTED_CONTENT;
    }

    // using of a full featured document
    if (ProtocolCheck::isProtocol(sURL,EProtocol::PrivateObject))
    {
        pIt = stlMediaDescriptor.find(utl::MediaDescriptor::PROP_MODEL());
        css::uno::Reference< css::frame::XModel > xModel;
        if (pIt != stlMediaDescriptor.end())
            pIt->second >>= xModel;
        if (xModel.is())
            return E_CAN_BE_SET;
        SAL_INFO("fwk.loadenv", "LoadEnv::classifyContent(): loading with object with right URL but invalid object detected");
        return E_UNSUPPORTED_CONTENT;
    }

    // following operations can work on an internal type name only :-(
    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    css::uno::Reference< css::document::XTypeDetection > xDetect(
         xContext->getServiceManager()->createInstanceWithContext(
             "com.sun.star.document.TypeDetection", xContext),
         css::uno::UNO_QUERY_THROW);

    OUString sType = xDetect->queryTypeByURL(sURL);

    css::uno::Sequence< css::beans::NamedValue >           lQuery(1);
    css::uno::Reference< css::frame::XLoaderFactory >      xLoaderFactory;
    css::uno::Reference< css::container::XEnumeration >    xSet;
    css::uno::Sequence< OUString >                  lTypesReg(1);

    // (iii) If a FrameLoader service (or at least
    //      a Filter) can be found, which supports
    //      this URL - it must be a loadable content.
    //      Because both items are registered for types
    //      it's enough to check for frame loaders only.
    //      Mos of our filters are handled by our global
    //      default loader. But there exist some specialized
    //      loader, which does not work on top of filters!
    //      So it's not enough to search on the filter configuration.
    //      Further it's not enough to search for types!
    //      Because there exist some types, which are referenced by
    //      other objects... but not by filters nor frame loaders!

    OUString sPROP_TYPES(PROP_TYPES);

    lTypesReg[0]      = sType;
    lQuery[0].Name    = sPROP_TYPES;
    lQuery[0].Value <<= lTypesReg;

    xLoaderFactory = css::frame::FrameLoaderFactory::create(xContext);
    xSet       = xLoaderFactory->createSubSetEnumerationByProperties(lQuery);
    // at least one registered frame loader is enough!
    if (xSet->hasMoreElements())
        return E_CAN_BE_LOADED;

    // (iv) Some URL protocols are supported by special services.
    //      E.g. ContentHandler.
    //      Such contents can be handled ... but not loaded.

    lTypesReg[0]      = sType;
    lQuery[0].Name    = sPROP_TYPES;
    lQuery[0].Value <<= lTypesReg;

    xLoaderFactory = css::frame::ContentHandlerFactory::create(xContext);
    xSet       = xLoaderFactory->createSubSetEnumerationByProperties(lQuery);
    // at least one registered content handler is enough!
    if (xSet->hasMoreElements())
        return E_CAN_BE_HANDLED;

    // (v) Last but not least the UCB is used inside office to
    //     load contents. He has a special configuration to know
    //     which URL schemata can be used inside office.
    css::uno::Reference< css::ucb::XUniversalContentBroker > xUCB(css::ucb::UniversalContentBroker::create(xContext));
    if (xUCB->queryContentProvider(sURL).is())
        return E_CAN_BE_LOADED;

    // (TODO) At this point, we have no idea .-)
    //        But it seems to be better, to break all
    //        further requests for this URL. Otherwise
    //        we can run into some trouble.
    return E_UNSUPPORTED_CONTENT;
}

namespace {

bool queryOrcusTypeAndFilter(const uno::Sequence<beans::PropertyValue>& rDescriptor, OUString& rType, OUString& rFilter)
{
    OUString aURL;
    sal_Int32 nSize = rDescriptor.getLength();
    for (sal_Int32 i = 0; i < nSize; ++i)
    {
        const beans::PropertyValue& rProp = rDescriptor[i];
        if (rProp.Name == "URL")
        {
            rProp.Value >>= aURL;
            break;
        }
    }

    if (aURL.isEmpty() || aURL.copy(0,8).equalsIgnoreAsciiCase("private:"))
        return false;

    // TODO : Type must be set to be generic_Text (or any other type that
    // exists) in order to find a usable loader. Exploit it as a temporary
    // hack.

    // depending on the experimental mode
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    if (!xContext.is() || !officecfg::Office::Common::Misc::ExperimentalMode::get(xContext))
    {
        return false;
    }

    OUString aUseOrcus;
    rtl::Bootstrap::get("LIBO_USE_ORCUS", aUseOrcus);
    bool bUseOrcus = (aUseOrcus == "YES");

    if (!bUseOrcus)
        return false;

    if (aURL.endsWith(".xlsx"))
    {
        rType = "generic_Text";
        rFilter = "xlsx";
        return true;
    }
    else if (aURL.endsWith(".ods"))
    {
        rType = "generic_Text";
        rFilter = "ods";
        return true;
    }
    else if (aURL.endsWith(".csv"))
    {
        rType = "generic_Text";
        rFilter = "csv";
        return true;
    }

    return false;
}

}

void LoadEnv::impl_detectTypeAndFilter()
{
    static const sal_Int32 FILTERFLAG_TEMPLATEPATH  = 16;

    // SAFE ->
    osl::ClearableMutexGuard aReadLock(m_mutex);

    // Attention: Because our stl media descriptor is a copy of an uno sequence
    // we can't use as an in/out parameter here. Copy it before and don't forget to
    // update structure afterwards again!
    css::uno::Sequence< css::beans::PropertyValue >        lDescriptor = m_lMediaDescriptor.getAsConstPropertyValueList();
    css::uno::Reference< css::uno::XComponentContext >     xContext = m_xContext;

    aReadLock.clear();
    // <- SAFE

    OUString sType, sFilter;

    if (queryOrcusTypeAndFilter(lDescriptor, sType, sFilter) && !sType.isEmpty() && !sFilter.isEmpty())
    {
        // Orcus type detected.  Skip the normal type detection process.
        m_lMediaDescriptor << lDescriptor;
        m_lMediaDescriptor[utl::MediaDescriptor::PROP_TYPENAME()] <<= sType;
        m_lMediaDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
        m_lMediaDescriptor[utl::MediaDescriptor::PROP_FILTERPROVIDER()] <<= OUString("orcus");
        m_lMediaDescriptor[utl::MediaDescriptor::PROP_DOCUMENTSERVICE()] <<= OUString("com.sun.star.sheet.SpreadsheetDocument");
        return;
    }

    css::uno::Reference< css::document::XTypeDetection > xDetect(
        xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.document.TypeDetection", xContext),
        css::uno::UNO_QUERY_THROW);
    sType = xDetect->queryTypeByDescriptor(lDescriptor, true); /*TODO should deep detection be able for enable/disable it from outside? */

    // no valid content -> loading not possible
    if (sType.isEmpty())
        throw LoadEnvException(
            LoadEnvException::ID_UNSUPPORTED_CONTENT, "type detection failed");

    // SAFE ->
    osl::ResettableMutexGuard aWriteLock(m_mutex);

    // detection was successfully => update the descriptor member of this class
    m_lMediaDescriptor << lDescriptor;
    m_lMediaDescriptor[utl::MediaDescriptor::PROP_TYPENAME()] <<= sType;
    // Is there an already detected (may be preselected) filter?
    // see below ...
    sFilter = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_FILTERNAME(), OUString());

    aWriteLock.clear();
    // <- SAFE

    // We do have potentially correct type, but the detection process was aborted.
    if (m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_ABORTED(), false))
        throw LoadEnvException(
            LoadEnvException::ID_UNSUPPORTED_CONTENT, "type detection aborted");

    // But the type isn't enough. For loading sometimes we need more information.
    // E.g. for our "_default" feature, where we recycle any frame which contains
    // and "Untitled" document, we must know if the new document is based on a template!
    // But this information is available as a filter property only.
    // => We must try(!) to detect the right filter for this load request.
    // On the other side ... if no filter is available .. ignore it.
    // Then the type information must be enough.
    if (sFilter.isEmpty())
    {
        // no -> try to find a preferred filter for the detected type.
        // Don't forget to update the media descriptor.
        css::uno::Reference< css::container::XNameAccess > xTypeCont(xDetect, css::uno::UNO_QUERY_THROW);
        try
        {
            ::comphelper::SequenceAsHashMap lTypeProps(xTypeCont->getByName(sType));
            sFilter = lTypeProps.getUnpackedValueOrDefault("PreferredFilter", OUString());
            if (!sFilter.isEmpty())
            {
                // SAFE ->
                aWriteLock.reset();
                m_lMediaDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
                aWriteLock.clear();
                // <- SAFE
            }
        }
        catch(const css::container::NoSuchElementException&)
            {}
    }

    // check if the filter (if one exists) points to a template format filter.
    // Then we have to add the property "AsTemplate".
    // We need this information to decide afterwards if we can use a "recycle frame"
    // for target "_default" or has to create a new one every time.
    // On the other side we have to suppress that, if this property already exists
    // and should trigger a special handling. Then the outside call of this method here,
    // has to know, what he is doing .-)

    bool bIsOwnTemplate = false;
    if (!sFilter.isEmpty())
    {
        css::uno::Reference< css::container::XNameAccess > xFilterCont(xContext->getServiceManager()->createInstanceWithContext(SERVICENAME_FILTERFACTORY, xContext), css::uno::UNO_QUERY_THROW);
        try
        {
            ::comphelper::SequenceAsHashMap lFilterProps(xFilterCont->getByName(sFilter));
            sal_Int32 nFlags         = lFilterProps.getUnpackedValueOrDefault("Flags", sal_Int32(0));
            bIsOwnTemplate = ((nFlags & FILTERFLAG_TEMPLATEPATH) == FILTERFLAG_TEMPLATEPATH);
        }
        catch(const css::container::NoSuchElementException&)
            {}
    }
    if (bIsOwnTemplate)
    {
        // SAFE ->
        aWriteLock.reset();
        // Don't overwrite external decisions! See comments before ...
        utl::MediaDescriptor::const_iterator pAsTemplateItem = m_lMediaDescriptor.find(utl::MediaDescriptor::PROP_ASTEMPLATE());
        if (pAsTemplateItem == m_lMediaDescriptor.end())
            m_lMediaDescriptor[utl::MediaDescriptor::PROP_ASTEMPLATE()] <<= true;
        aWriteLock.clear();
        // <- SAFE
    }
}

bool LoadEnv::impl_handleContent()
{
    // SAFE -> -----------------------------------
    osl::ClearableMutexGuard aReadLock(m_mutex);

    // the type must exist inside the descriptor ... otherwise this class is implemented wrong :-)
    OUString sType = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_TYPENAME(), OUString());
    if (sType.isEmpty())
        throw LoadEnvException(LoadEnvException::ID_INVALID_MEDIADESCRIPTOR);

    // convert media descriptor and URL to right format for later interface call!
    css::uno::Sequence< css::beans::PropertyValue > lDescriptor;
    m_lMediaDescriptor >> lDescriptor;
    css::util::URL aURL = m_aURL;

    // get necessary container to query for a handler object
    css::uno::Reference< css::frame::XLoaderFactory > xLoaderFactory = css::frame::ContentHandlerFactory::create(m_xContext);

    aReadLock.clear();
    // <- SAFE -----------------------------------

    // query
    css::uno::Sequence< OUString > lTypeReg { sType };

    css::uno::Sequence< css::beans::NamedValue > lQuery { { PROP_TYPES, css::uno::makeAny(lTypeReg) } };

    css::uno::Reference< css::container::XEnumeration > xSet = xLoaderFactory->createSubSetEnumerationByProperties(lQuery);
    while(xSet->hasMoreElements())
    {
        ::comphelper::SequenceAsHashMap lProps   (xSet->nextElement());
        OUString                 sHandler = lProps.getUnpackedValueOrDefault(PROP_NAME, OUString());

        css::uno::Reference< css::frame::XNotifyingDispatch > xHandler;
        try
        {
            xHandler.set(xLoaderFactory->createInstance(sHandler), css::uno::UNO_QUERY);
            if (!xHandler.is())
                continue;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }

        // SAFE -> -----------------------------------
        osl::ClearableMutexGuard aWriteLock(m_mutex);
        m_xAsynchronousJob = xHandler;
        LoadEnvListener* pListener = new LoadEnvListener(this);
        aWriteLock.clear();
        // <- SAFE -----------------------------------

        css::uno::Reference< css::frame::XDispatchResultListener > xListener(static_cast< css::frame::XDispatchResultListener* >(pListener), css::uno::UNO_QUERY);
        xHandler->dispatchWithNotification(aURL, lDescriptor, xListener);

        return true;
    }

    return false;
}

bool LoadEnv::impl_furtherDocsAllowed()
{
    // SAFE ->
    osl::ResettableMutexGuard aReadLock(m_mutex);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.clear();
    // <- SAFE

    bool bAllowed = true;

    try
    {
        css::uno::Any aVal = ::comphelper::ConfigurationHelper::readDirectKey(
                                xContext,
                                "org.openoffice.Office.Common/",
                                "Misc",
                                "MaxOpenDocuments",
                                ::comphelper::EConfigurationModes::ReadOnly);

        // NIL means: count of allowed documents = infinite !
        //     => return sal_True
        if ( ! aVal.hasValue())
            bAllowed = true;
        else
        {
            sal_Int32 nMaxOpenDocuments = 0;
            aVal >>= nMaxOpenDocuments;

            css::uno::Reference< css::frame::XFramesSupplier > xDesktop(
                css::frame::Desktop::create(xContext),
                css::uno::UNO_QUERY_THROW);

            FrameListAnalyzer aAnalyzer(xDesktop,
                                        css::uno::Reference< css::frame::XFrame >(),
                                        FrameAnalyzerFlags::Help |
                                        FrameAnalyzerFlags::BackingComponent |
                                        FrameAnalyzerFlags::Hidden);

            sal_Int32 nOpenDocuments = aAnalyzer.m_lOtherVisibleFrames.size();
            bAllowed       = (nOpenDocuments < nMaxOpenDocuments);
        }
    }
    catch(const css::uno::Exception&)
        { bAllowed = true; } // !! internal errors are no reason to disturb the office from opening documents .-)

    if ( ! bAllowed )
    {
        // SAFE ->
        aReadLock.reset();
        css::uno::Reference< css::task::XInteractionHandler > xInteraction = m_lMediaDescriptor.getUnpackedValueOrDefault(
                                                                                utl::MediaDescriptor::PROP_INTERACTIONHANDLER(),
                                                                                css::uno::Reference< css::task::XInteractionHandler >());
        aReadLock.clear();
        // <- SAFE

        if (xInteraction.is())
        {
            css::uno::Any                                                                    aInteraction;
            css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations(2);

            comphelper::OInteractionAbort*   pAbort   = new comphelper::OInteractionAbort();
            comphelper::OInteractionApprove* pApprove = new comphelper::OInteractionApprove();

            lContinuations[0].set( static_cast< css::task::XInteractionContinuation* >(pAbort),
                                   css::uno::UNO_QUERY_THROW);
            lContinuations[1].set( static_cast< css::task::XInteractionContinuation* >(pApprove),
                                   css::uno::UNO_QUERY_THROW);

            css::task::ErrorCodeRequest aErrorCode;
            aErrorCode.ErrCode = sal_uInt32(ERRCODE_SFX_NOMOREDOCUMENTSALLOWED);
            aInteraction <<= aErrorCode;
            xInteraction->handle( InteractionRequest::CreateRequest(aInteraction, lContinuations) );
        }
    }

    return bAllowed;
}

bool LoadEnv::impl_filterHasInteractiveDialog() const
{
    //show the frame now so it can be the parent for any message dialogs shown during import

    //unless (tdf#114648) an Interactive case such as the new database wizard
    if (m_aURL.Arguments == "Interactive")
       return true;

    // unless (tdf#116277) its the labels/business cards slave frame
    if (m_aURL.Arguments.indexOf("slot=") != -1)
        return true;

    OUString sFilter = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_FILTERNAME(), OUString());
    if (sFilter.isEmpty())
        return false;

    // unless (tdf#115683) the filter has a UIComponent
    OUString sUIComponent;
    css::uno::Reference<css::container::XNameAccess> xFilterCont(m_xContext->getServiceManager()->createInstanceWithContext(SERVICENAME_FILTERFACTORY, m_xContext),
                                                                 css::uno::UNO_QUERY_THROW);
    try
    {
        ::comphelper::SequenceAsHashMap lFilterProps(xFilterCont->getByName(sFilter));
        sUIComponent = lFilterProps.getUnpackedValueOrDefault("UIComponent", OUString());
    }
    catch(const css::container::NoSuchElementException&)
    {
    }

    return !sUIComponent.isEmpty();
}

bool LoadEnv::impl_loadContent()
{
    // SAFE -> -----------------------------------
    osl::ClearableMutexGuard aWriteLock(m_mutex);

    // search or create right target frame
    OUString sTarget = m_sTarget;
    if (TargetHelper::matchSpecialTarget(sTarget, TargetHelper::ESpecialTarget::Default))
    {
        m_xTargetFrame = impl_searchAlreadyLoaded();
        if (m_xTargetFrame.is())
        {
            impl_setResult(true);
            return true;
        }
        m_xTargetFrame = impl_searchRecycleTarget();
    }

    if (! m_xTargetFrame.is())
    {
        if (
            (TargetHelper::matchSpecialTarget(sTarget, TargetHelper::ESpecialTarget::Blank  )) ||
            (TargetHelper::matchSpecialTarget(sTarget, TargetHelper::ESpecialTarget::Default))
           )
        {
            if (! impl_furtherDocsAllowed())
                return false;
            TaskCreator aCreator(m_xContext);
            m_xTargetFrame = aCreator.createTask(SPECIALTARGET_BLANK, m_lMediaDescriptor);
            m_bCloseFrameOnError = m_xTargetFrame.is();
        }
        else
        {
            sal_Int32 nSearchFlags = m_nSearchFlags & ~css::frame::FrameSearchFlag::CREATE;
            m_xTargetFrame   = m_xBaseFrame->findFrame(sTarget, nSearchFlags);
            if (! m_xTargetFrame.is())
            {
                if (! impl_furtherDocsAllowed())
                    return false;
                m_xTargetFrame       = m_xBaseFrame->findFrame(SPECIALTARGET_BLANK, 0);
                m_bCloseFrameOnError = m_xTargetFrame.is();
            }
        }
    }

    // If we couldn't find a valid frame or the frame has no container window
    // we have to throw an exception.
    if (
        ( ! m_xTargetFrame.is()                       ) ||
        ( ! m_xTargetFrame->getContainerWindow().is() )
       )
        throw LoadEnvException(LoadEnvException::ID_NO_TARGET_FOUND);

    css::uno::Reference< css::frame::XFrame > xTargetFrame = m_xTargetFrame;

    // Now we have a valid frame ... and type detection was already done.
    // We should apply the module dependent window position and size to the
    // frame window.
    impl_applyPersistentWindowState(xTargetFrame->getContainerWindow());

    // Don't forget to lock task for following load process. Otherwise it could die
    // during this operation runs by terminating the office or closing this task via api.
    // If we set this lock "close()" will return false and closing will be broken.
    // Attention: Don't forget to reset this lock again after finishing operation.
    // Otherwise task AND office couldn't die!!!
    // This includes gracefully handling of Exceptions (Runtime!) too ...
    // That's why we use a specialized guard, which will reset the lock
    // if it will be run out of scope.

    // Note further: ignore if this internal guard already contains a resource.
    // Might impl_searchRecylcTarget() set it before. But in case this impl-method wasn't used
    // and the target frame was new created ... this lock here must be set!
    css::uno::Reference< css::document::XActionLockable > xTargetLock(xTargetFrame, css::uno::UNO_QUERY);
    m_aTargetLock.setResource(xTargetLock);

    // Add status indicator to descriptor. Loader can show an progresses then.
    // But don't do it, if loading should be hidden or preview is used ...!
    // So we prevent our code against wrong using. Why?
    // It could be, that using of this progress could make trouble. e.g. He make window visible ...
    // but shouldn't do that. But if no indicator is available ... nobody has a chance to do that!
    bool bHidden    = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_HIDDEN(), false);
    bool bMinimized = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_MINIMIZED(), false);
    bool bPreview   = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_PREVIEW(), false);
    css::uno::Reference< css::task::XStatusIndicator > xProgress  = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_STATUSINDICATOR(), css::uno::Reference< css::task::XStatusIndicator >());

    if (!bHidden && !bMinimized && !bPreview)
    {
        if (!xProgress.is())
        {
            // Note: it's an optional interface!
            css::uno::Reference< css::task::XStatusIndicatorFactory > xProgressFactory(xTargetFrame, css::uno::UNO_QUERY);
            if (xProgressFactory.is())
            {
                xProgress = xProgressFactory->createStatusIndicator();
                if (xProgress.is())
                    m_lMediaDescriptor[utl::MediaDescriptor::PROP_STATUSINDICATOR()] <<= xProgress;
            }
        }

        // Now that we have a target window into which we can load, reinit the interaction handler to have this
        // window as its parent for modal dialogs and ensure the window is visible
        css::uno::Reference< css::task::XInteractionHandler > xInteraction = m_lMediaDescriptor.getUnpackedValueOrDefault(
                                                                                utl::MediaDescriptor::PROP_INTERACTIONHANDLER(),
                                                                                css::uno::Reference< css::task::XInteractionHandler >());
        css::uno::Reference<css::lang::XInitialization> xHandler(xInteraction, css::uno::UNO_QUERY);
        if (xHandler.is())
        {
            css::uno::Reference<css::awt::XWindow> xWindow = xTargetFrame->getContainerWindow();
            uno::Sequence<uno::Any> aArguments(comphelper::InitAnyPropertySequence(
            {
                {"Parent", uno::Any(xWindow)}
            }));
            xHandler->initialize(aArguments);
            //show the frame as early as possible to make it the parent of any message dialogs
            if (!impl_filterHasInteractiveDialog())
                impl_makeFrameWindowVisible(xWindow, false);
        }
    }

    // convert media descriptor and URL to right format for later interface call!
    css::uno::Sequence< css::beans::PropertyValue > lDescriptor;
    m_lMediaDescriptor >> lDescriptor;
    OUString sURL = m_aURL.Complete;

    // try to locate any interested frame loader
    css::uno::Reference< css::uno::XInterface >                xLoader     = impl_searchLoader();
    css::uno::Reference< css::frame::XFrameLoader >            xAsyncLoader(xLoader, css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XSynchronousFrameLoader > xSyncLoader (xLoader, css::uno::UNO_QUERY);

    if (xAsyncLoader.is())
    {
        m_xAsynchronousJob = xAsyncLoader;
        LoadEnvListener* pListener = new LoadEnvListener(this);
        aWriteLock.clear();
        // <- SAFE -----------------------------------

        css::uno::Reference< css::frame::XLoadEventListener > xListener(static_cast< css::frame::XLoadEventListener* >(pListener), css::uno::UNO_QUERY);
        xAsyncLoader->load(xTargetFrame, sURL, lDescriptor, xListener);

        return true;
    }
    else if (xSyncLoader.is())
    {
        bool bResult = xSyncLoader->load(lDescriptor, xTargetFrame);
        // react for the result here, so the outside waiting
        // code can ask for it later.
        impl_setResult(bResult);
        // But the return value indicates a valid started(!) operation.
        // And that's true every time we reach this line :-)
        return true;
    }

    aWriteLock.clear();
    // <- SAFE

    return false;
}

css::uno::Reference< css::uno::XInterface > LoadEnv::impl_searchLoader()
{
    // SAFE -> -----------------------------------
    osl::ClearableMutexGuard aReadLock(m_mutex);

    // special mode to set an existing component on this frame
    // In such case the loader is fix. It must be the SFX based implementation,
    // which can create a view on top of such xModel components :-)
    if (m_eContentType == E_CAN_BE_SET)
    {
        try
        {
            return css::frame::OfficeFrameLoader::create(m_xContext);
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            {}
        throw LoadEnvException(LoadEnvException::ID_INVALID_ENVIRONMENT);
    }

    // Otherwise ...
    // We need this type information to locate an registered frame loader
    // Without such information we can't work!
    OUString sType = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_TYPENAME(), OUString());
    if (sType.isEmpty())
        throw LoadEnvException(LoadEnvException::ID_INVALID_MEDIADESCRIPTOR);

    // try to locate any interested frame loader
    css::uno::Reference< css::frame::XLoaderFactory > xLoaderFactory = css::frame::FrameLoaderFactory::create(m_xContext);

    aReadLock.clear();
    // <- SAFE -----------------------------------

    css::uno::Sequence< OUString > lTypesReg { sType };

    css::uno::Sequence< css::beans::NamedValue > lQuery { { PROP_TYPES, css::uno::makeAny(lTypesReg) } };

    css::uno::Reference< css::container::XEnumeration > xSet = xLoaderFactory->createSubSetEnumerationByProperties(lQuery);
    while(xSet->hasMoreElements())
    {
        try
        {
            // try everyone ...
            // Ignore any loader, which makes trouble :-)
            ::comphelper::SequenceAsHashMap             lLoaderProps(xSet->nextElement());
            OUString                             sLoader     = lLoaderProps.getUnpackedValueOrDefault(PROP_NAME, OUString());
            css::uno::Reference< css::uno::XInterface > xLoader;

            xLoader = xLoaderFactory->createInstance(sLoader);
            if (xLoader.is())
                return xLoader;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }
    }

    return css::uno::Reference< css::uno::XInterface >();
}

void LoadEnv::impl_jumpToMark(const css::uno::Reference< css::frame::XFrame >& xFrame,
                              const css::util::URL&                            aURL  )
{
    if (aURL.Mark.isEmpty())
        return;

    css::uno::Reference< css::frame::XDispatchProvider > xProvider(xFrame, css::uno::UNO_QUERY);
    if (! xProvider.is())
        return;

    // SAFE ->
    osl::ClearableMutexGuard aReadLock(m_mutex);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.clear();
    // <- SAFE

    css::util::URL aCmd;
    aCmd.Complete = ".uno:JumpToMark";

    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(xContext));
    xParser->parseStrict(aCmd);

    css::uno::Reference< css::frame::XDispatch > xDispatcher = xProvider->queryDispatch(aCmd, SPECIALTARGET_SELF, 0);
    if (! xDispatcher.is())
        return;

    ::comphelper::SequenceAsHashMap lArgs;
    lArgs[OUString("Bookmark")] <<= aURL.Mark;
    xDispatcher->dispatch(aCmd, lArgs.getAsConstPropertyValueList());
}

css::uno::Reference< css::frame::XFrame > LoadEnv::impl_searchAlreadyLoaded()
{
    osl::MutexGuard g(m_mutex);

    // such search is allowed for special requests only ...
    // or better it's not allowed for some requests in general :-)
    if (
        ( ! TargetHelper::matchSpecialTarget(m_sTarget, TargetHelper::ESpecialTarget::Default)                                               ) ||
        m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_ASTEMPLATE() , false) ||
//      (m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_HIDDEN()     , false) == sal_True) ||
        m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_OPENNEWVIEW(), false)
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    // check URL
    // May it's not useful to start expensive document search, if it
    // can fail only .. because we load from a stream or model directly!
    if (
        (ProtocolCheck::isProtocol(m_aURL.Complete, EProtocol::PrivateStream )) ||
        (ProtocolCheck::isProtocol(m_aURL.Complete, EProtocol::PrivateObject ))
        /*TODO should be private:factory here tested too? */
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    // otherwise - iterate through the tasks of the desktop container
    // to find out, which of them might contains the requested document
    css::uno::Reference< css::frame::XDesktop2 >  xSupplier = css::frame::Desktop::create( m_xContext );
    css::uno::Reference< css::container::XIndexAccess > xTaskList(xSupplier->getFrames()                      , css::uno::UNO_QUERY);

    if (!xTaskList.is())
        return css::uno::Reference< css::frame::XFrame >(); // task list can be empty!

    // Note: To detect if a document was already loaded before
    // we check URLs here only. But might the existing and the required
    // document has different versions! Then its URLs are the same ...
    sal_Int16 nNewVersion = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_VERSION(), sal_Int16(-1));

    // will be used to save the first hidden frame referring the searched model
    // Normally we are interested on visible frames ... but if there is no such visible
    // frame we refer to any hidden frame also (but as fallback only).
    css::uno::Reference< css::frame::XFrame > xHiddenTask;
    css::uno::Reference< css::frame::XFrame > xTask;

    sal_Int32 count = xTaskList->getCount();
    for (sal_Int32 i=0; i<count; ++i)
    {
        try
        {
            // locate model of task
            // Note: Without a model there is no chance to decide if
            // this task contains the searched document or not!
            xTaskList->getByIndex(i) >>= xTask;
            if (!xTask.is())
                continue;

            css::uno::Reference< css::frame::XController > xController = xTask->getController();
            if (!xController.is())
            {
                xTask.clear ();
                continue;
            }

            css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
            if (!xModel.is())
            {
                xTask.clear ();
                continue;
            }

            // don't check the complete URL here.
            // use its main part - ignore optional jumpmarks!
            const OUString sURL = xModel->getURL();
            if (!::utl::UCBContentHelper::EqualURLs( m_aURL.Main, sURL ))
            {
                xTask.clear ();
                continue;
            }

            // get the original load arguments from the current document
            // and decide if its really the same then the one will be.
            // It must be visible and must use the same file revision ...
            // or must not have any file revision set (-1 == -1!)
            utl::MediaDescriptor lOldDocDescriptor(xModel->getArgs());

            if (lOldDocDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_VERSION(), sal_Int32(-1)) != nNewVersion)
            {
                xTask.clear ();
                continue;
            }

            // Hidden frames are special.
            // They will be used as "last chance" if there is no visible frame pointing to the same model.
            // Safe the result but continue with current loop might be looking for other visible frames.
            bool bIsHidden = lOldDocDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_HIDDEN(), false);
            if ( bIsHidden && ! xHiddenTask.is() )
            {
                xHiddenTask = xTask;
                xTask.clear ();
                continue;
            }

            // We found a visible task pointing to the right model ...
            // Break search.
            break;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }
    }

    css::uno::Reference< css::frame::XFrame > xResult;
    if (xTask.is())
        xResult = xTask;
    else if (xHiddenTask.is())
        xResult = xHiddenTask;

    if (xResult.is())
    {
        // Now we are sure, that this task includes the searched document.
        // It's time to activate it. As special feature we try to jump internally
        // if an optional jumpmark is given too.
        if (!m_aURL.Mark.isEmpty())
            impl_jumpToMark(xResult, m_aURL);

        // bring it to front and make sure it's visible...
        impl_makeFrameWindowVisible(xResult->getContainerWindow(), true);
    }

    return xResult;
}

bool LoadEnv::impl_isFrameAlreadyUsedForLoading(const css::uno::Reference< css::frame::XFrame >& xFrame) const
{
    css::uno::Reference< css::document::XActionLockable > xLock(xFrame, css::uno::UNO_QUERY);

    // ? no lock interface ?
    // Maybe it's an external written frame implementation :-(
    // Allowing using of it... but it can fail if it's not synchronized with our processes!
    if (!xLock.is())
        return false;

    // Otherwise we have to look for any other existing lock.
    return xLock->isActionLocked();
}

css::uno::Reference< css::frame::XFrame > LoadEnv::impl_searchRecycleTarget()
{
    // SAFE -> ..................................
    osl::ClearableMutexGuard aReadLock(m_mutex);

    // The special backing mode frame will be recycled by definition!
    // It doesn't matter if somewhere wants to create a new view
    // or open a new untitled document ...
    // The only exception form that - hidden frames!
    if (m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_HIDDEN(), false))
        return css::uno::Reference< css::frame::XFrame >();

    css::uno::Reference< css::frame::XFramesSupplier > xSupplier( css::frame::Desktop::create( m_xContext ), css::uno::UNO_QUERY);
    FrameListAnalyzer aTasksAnalyzer(xSupplier, css::uno::Reference< css::frame::XFrame >(), FrameAnalyzerFlags::BackingComponent);
    if (aTasksAnalyzer.m_xBackingComponent.is())
    {
        if (!impl_isFrameAlreadyUsedForLoading(aTasksAnalyzer.m_xBackingComponent))
        {
            // bring it to front ...
            impl_makeFrameWindowVisible(aTasksAnalyzer.m_xBackingComponent->getContainerWindow(), true);
            m_bReactivateControllerOnError = true;
            return aTasksAnalyzer.m_xBackingComponent;
        }
    }

    // These states indicates a wish for creation of a new view in general.
    if (
        m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_ASTEMPLATE() , false) ||
        m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_OPENNEWVIEW(), false)
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    // On the other side some special URLs will open a new frame every time (expecting
    // they can use the backing-mode frame!)
    if (
        (ProtocolCheck::isProtocol(m_aURL.Complete, EProtocol::PrivateFactory )) ||
        (ProtocolCheck::isProtocol(m_aURL.Complete, EProtocol::PrivateStream  )) ||
        (ProtocolCheck::isProtocol(m_aURL.Complete, EProtocol::PrivateObject  ))
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    // No backing frame! No special URL => recycle active task - if possible.
    // Means - if it does not already contains a modified document, or
    // use another office module.
    css::uno::Reference< css::frame::XFrame > xTask = xSupplier->getActiveFrame();

    // not a real error - but might a focus problem!
    if (!xTask.is())
        return css::uno::Reference< css::frame::XFrame >();

    // not a real error - may it's a view only
    css::uno::Reference< css::frame::XController > xController = xTask->getController();
    if (!xController.is())
        return css::uno::Reference< css::frame::XFrame >();

    // not a real error - may it's a db component instead of a full featured office document
    css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
    if (!xModel.is())
        return css::uno::Reference< css::frame::XFrame >();

    // get some more information ...

    // A valid set URL means: there is already a location for this document.
    // => it was saved there or opened from there. Such Documents can not be used here.
    // We search for empty document ... created by a private:factory/ URL!
    if (xModel->getURL().getLength()>0)
        return css::uno::Reference< css::frame::XFrame >();

    // The old document must be unmodified ...
    css::uno::Reference< css::util::XModifiable > xModified(xModel, css::uno::UNO_QUERY);
    if (xModified->isModified())
        return css::uno::Reference< css::frame::XFrame >();

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xTask->getContainerWindow());
    if (pWindow && pWindow->IsInModalMode())
        return css::uno::Reference< css::frame::XFrame >();

    // find out the application type of this document
    // We can recycle only documents, which uses the same application
    // then the new one.
    SvtModuleOptions::EFactory eOldApp = SvtModuleOptions::ClassifyFactoryByModel(xModel);
    SvtModuleOptions::EFactory eNewApp = SvtModuleOptions::ClassifyFactoryByURL  (m_aURL.Complete, m_lMediaDescriptor.getAsConstPropertyValueList());

    aReadLock.clear();
    // <- SAFE ..................................

    if (eOldApp != eNewApp)
        return css::uno::Reference< css::frame::XFrame >();

    // OK this task seems to be usable for recycling
    // But we should mark it as such - means set an action lock.
    // Otherwise it would be used more than ones or will be destroyed
    // by a close() or terminate() request.
    // But if such lock already exist ... it means this task is used for
    // any other operation already. Don't use it then.
    if (impl_isFrameAlreadyUsedForLoading(xTask))
        return css::uno::Reference< css::frame::XFrame >();

    // OK - there is a valid target frame.
    // But may be it contains already a document.
    // Then we have to ask it, if it allows recycling of this frame .-)
    bool bReactivateOldControllerOnError = false;
    css::uno::Reference< css::frame::XController > xOldDoc = xTask->getController();
    if (xOldDoc.is())
    {
        bReactivateOldControllerOnError = xOldDoc->suspend(true);
        if (! bReactivateOldControllerOnError)
            return css::uno::Reference< css::frame::XFrame >();
    }

    // SAFE -> ..................................
    osl::ClearableMutexGuard aWriteLock(m_mutex);

    css::uno::Reference< css::document::XActionLockable > xLock(xTask, css::uno::UNO_QUERY);
    if (!m_aTargetLock.setResource(xLock))
        return css::uno::Reference< css::frame::XFrame >();

    m_bReactivateControllerOnError = bReactivateOldControllerOnError;
    aWriteLock.clear();
    // <- SAFE ..................................

    // bring it to front ...
    impl_makeFrameWindowVisible(xTask->getContainerWindow(), true);

    return xTask;
}

void LoadEnv::impl_reactForLoadingState()
{
    /*TODO reset action locks */

    // SAFE -> ----------------------------------
    osl::ClearableMutexGuard aReadLock(m_mutex);

    if (m_bLoaded)
    {
        // Bring the new loaded document to front (if allowed!).
        // Note: We show new created frames here only.
        // We don't hide already visible frames here ...
        css::uno::Reference< css::awt::XWindow > xWindow      = m_xTargetFrame->getContainerWindow();
        bool                                 bHidden      = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_HIDDEN(), false);
        bool                                 bMinimized = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_MINIMIZED(), false);

        if (bMinimized)
        {
            SolarMutexGuard aSolarGuard;
            VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xWindow);
            // check for system window is necessary to guarantee correct pointer cast!
            if (pWindow && pWindow->IsSystemWindow())
                static_cast<WorkWindow*>(pWindow.get())->Minimize();
        }
        else if (!bHidden)
        {
            // show frame ... if it's not still visible ...
            // But do nothing if it's already visible!
            impl_makeFrameWindowVisible(xWindow, false);
        }

        // Note: Only if an existing property "FrameName" is given by this media descriptor,
        // it should be used. Otherwise we should do nothing. May be the outside code has already
        // set a frame name on the target!
        utl::MediaDescriptor::const_iterator pFrameName = m_lMediaDescriptor.find(utl::MediaDescriptor::PROP_FRAMENAME());
        if (pFrameName != m_lMediaDescriptor.end())
        {
            OUString sFrameName;
            pFrameName->second >>= sFrameName;
            // Check the name again. e.g. "_default" isn't allowed.
            // On the other side "_beamer" is a valid name :-)
            if (TargetHelper::isValidNameForFrame(sFrameName))
                m_xTargetFrame->setName(sFrameName);
        }
    }
    else if (m_bReactivateControllerOnError)
    {
        // Try to reactivate the old document (if any exists!)
        css::uno::Reference< css::frame::XController > xOldDoc = m_xTargetFrame->getController();
        // clear does not depend from reactivation state of a might existing old document!
        // We must make sure, that a might following getTargetComponent() call does not return
        // the old document!
        m_xTargetFrame.clear();
        if (xOldDoc.is())
        {
            bool bReactivated = xOldDoc->suspend(false);
            if (!bReactivated)
                throw LoadEnvException(LoadEnvException::ID_COULD_NOT_REACTIVATE_CONTROLLER);
            m_bReactivateControllerOnError = false;
        }
    }
    else if (m_bCloseFrameOnError)
    {
        // close empty frames
        css::uno::Reference< css::util::XCloseable > xCloseable (m_xTargetFrame, css::uno::UNO_QUERY);
        css::uno::Reference< css::lang::XComponent > xDisposable(m_xTargetFrame, css::uno::UNO_QUERY);

        try
        {
            if (xCloseable.is())
                xCloseable->close(true);
            else
            if (xDisposable.is())
                xDisposable->dispose();
        }
        catch(const css::util::CloseVetoException&)
        {}
        catch(const css::lang::DisposedException&)
        {}
        m_xTargetFrame.clear();
    }

    // This max force an implicit closing of our target frame ...
    // e.g. in case close(sal_True) was called before and the frame
    // kill itself if our external use-lock is released here!
    // That's why we release this lock AFTER ALL OPERATIONS on this frame
    // are finished. The frame itself must handle then
    // this situation gracefully.
    m_aTargetLock.freeResource();

    // Last but not least :-)
    // We have to clear the current media descriptor.
    // Otherwise it hold a might existing stream open!
    m_lMediaDescriptor.clear();

    css::uno::Any aRequest;
    bool bThrow = false;
    if ( !m_bLoaded && m_pQuietInteraction.is() && m_pQuietInteraction->wasUsed() )
    {
        aRequest = m_pQuietInteraction->getRequest();
        m_pQuietInteraction.clear();
        bThrow = true;
    }

    aReadLock.clear();

    if (bThrow)
    {
        if  ( aRequest.isExtractableTo( ::cppu::UnoType< css::uno::Exception >::get() ) )
            throw LoadEnvException(
                LoadEnvException::ID_GENERAL_ERROR, "interaction request",
                aRequest);
    }

    // <- SAFE ----------------------------------
}

void LoadEnv::impl_makeFrameWindowVisible(const css::uno::Reference< css::awt::XWindow >& xWindow      ,
                                                bool bForceToFront)
{
    // SAFE -> ----------------------------------
    osl::ClearableMutexGuard aReadLock(m_mutex);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.clear();
    // <- SAFE ----------------------------------

    SolarMutexGuard aSolarGuard;
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xWindow);
    if ( pWindow )
    {
        bool const preview( m_lMediaDescriptor.getUnpackedValueOrDefault(
                utl::MediaDescriptor::PROP_PREVIEW(), false) );

        bool bForceFrontAndFocus(false);
        if ( !preview )
        {
            css::uno::Any const a =
                ::comphelper::ConfigurationHelper::readDirectKey(
                  xContext,
                  "org.openoffice.Office.Common/View",
                  "NewDocumentHandling",
                  "ForceFocusAndToFront",
                  ::comphelper::EConfigurationModes::ReadOnly);
            a >>= bForceFrontAndFocus;
        }

        if( pWindow->IsVisible() && (bForceFrontAndFocus || bForceToFront) )
            pWindow->ToTop( ToTopFlags::RestoreWhenMin | ToTopFlags::ForegroundTask );
        else
            pWindow->Show(true, (bForceFrontAndFocus || bForceToFront) ? ShowFlags::ForegroundTask : ShowFlags::NONE );
    }
}

void LoadEnv::impl_applyPersistentWindowState(const css::uno::Reference< css::awt::XWindow >& xWindow)
{
    // no window -> action not possible
    if (!xWindow.is())
        return;

    // window already visible -> do nothing! If we use a "recycle frame" for loading ...
    // the current position and size must be used.
    css::uno::Reference< css::awt::XWindow2 > xVisibleCheck(xWindow, css::uno::UNO_QUERY);
    if (
        (xVisibleCheck.is()        ) &&
        (xVisibleCheck->isVisible())
       )
       return;

    // SOLAR SAFE ->
    SolarMutexClearableGuard aSolarGuard1;

    VclPtr<vcl::Window>  pWindow = VCLUnoHelper::GetWindow(xWindow);
    if (!pWindow)
        return;

    bool bSystemWindow = pWindow->IsSystemWindow();
    bool bWorkWindow   = (pWindow->GetType() == WindowType::WORKWINDOW);

    if (!bSystemWindow && !bWorkWindow)
        return;

    // don't overwrite this special state!
    WorkWindow* pWorkWindow = static_cast<WorkWindow*>(pWindow.get());
    if (pWorkWindow->IsMinimized())
        return;

    aSolarGuard1.clear();
    // <- SOLAR SAFE

    // SAFE ->
    osl::ClearableMutexGuard aReadLock(m_mutex);

    // no filter -> no module -> no persistent window state
    OUString sFilter = m_lMediaDescriptor.getUnpackedValueOrDefault(
                                    utl::MediaDescriptor::PROP_FILTERNAME(),
                                    OUString());
    if (sFilter.isEmpty())
        return;

    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;

    aReadLock.clear();
    // <- SAFE

    try
    {
        // retrieve the module name from the filter configuration
        css::uno::Reference< css::container::XNameAccess > xFilterCfg(
            xContext->getServiceManager()->createInstanceWithContext(SERVICENAME_FILTERFACTORY, xContext),
            css::uno::UNO_QUERY_THROW);
        ::comphelper::SequenceAsHashMap lProps (xFilterCfg->getByName(sFilter));
        OUString                 sModule = lProps.getUnpackedValueOrDefault(FILTER_PROPNAME_ASCII_DOCUMENTSERVICE, OUString());

        // get access to the configuration of this office module
        css::uno::Reference< css::container::XNameAccess > xModuleCfg(::comphelper::ConfigurationHelper::openConfig(
                                                                        xContext,
                                                                        "/org.openoffice.Setup/Office/Factories",
                                                                        ::comphelper::EConfigurationModes::ReadOnly),
                                                                      css::uno::UNO_QUERY_THROW);

        // read window state from the configuration
        // and apply it on the window.
        // Do nothing, if no configuration entry exists!
        OUString sWindowState;

        // Don't look for persistent window attributes when used through LibreOfficeKit
        if( !comphelper::LibreOfficeKit::isActive() )
            comphelper::ConfigurationHelper::readRelativeKey(xModuleCfg, sModule, OFFICEFACTORY_PROPNAME_ASCII_WINDOWATTRIBUTES) >>= sWindowState;

        if (!sWindowState.isEmpty())
        {
            // SOLAR SAFE ->
            SolarMutexGuard aSolarGuard;

            // We have to retrieve the window pointer again. Because nobody can guarantee
            // that the XWindow was not disposed in between .-)
            // But if we get a valid pointer we can be sure, that it's the system window pointer
            // we already checked and used before. Because nobody recycle the same uno reference for
            // a new internal c++ implementation ... hopefully .-))
            VclPtr<vcl::Window> pWindowCheck = VCLUnoHelper::GetWindow(xWindow);
            if (! pWindowCheck)
                return;

            SystemWindow* pSystemWindow = static_cast<SystemWindow*>(pWindowCheck.get());
            pSystemWindow->SetWindowState(OUStringToOString(sWindowState,RTL_TEXTENCODING_UTF8));
            // <- SOLAR SAFE
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
