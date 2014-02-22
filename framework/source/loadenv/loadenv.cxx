/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <loadenv/loadenv.hxx>

#include <loadenv/targethelper.hxx>
#include <framework/framelistanalyzer.hxx>

#include <interaction/quietinteraction.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/resetableguard.hxx>
#include <properties.h>
#include <protocols.h>
#include <services.h>
#include <comphelper/interaction.hxx>
#include <framework/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/configuration.hxx>
#include "officecfg/Office/Common.hxx"

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
#include "rtl/bootstrap.hxx"
#include <vcl/svapp.hxx>

#include <config_orcus.h>

const char PROP_TYPES[] = "Types";
const char PROP_NAME[] = "Name";

namespace framework {

using namespace com::sun::star;


class LoadEnvListener : private ThreadHelpBase
                      , public ::cppu::WeakImplHelper2< css::frame::XLoadEventListener      ,
                                                        css::frame::XDispatchResultListener >
{
    private:

        bool m_bWaitingResult;
        LoadEnv* m_pLoadEnv;

    public:

        
        LoadEnvListener(LoadEnv* pLoadEnv)
            : m_bWaitingResult(true)
            , m_pLoadEnv(pLoadEnv)
        {
        }

        
        
        virtual void SAL_CALL loadFinished(const css::uno::Reference< css::frame::XFrameLoader >& xLoader)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL loadCancelled(const css::uno::Reference< css::frame::XFrameLoader >& xLoader)
            throw(css::uno::RuntimeException);

        
        
        virtual void SAL_CALL dispatchFinished(const css::frame::DispatchResultEvent& aEvent)
            throw(css::uno::RuntimeException);

        
        
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);
};


LoadEnv::LoadEnv(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    throw(LoadEnvException, css::uno::RuntimeException)
    : ThreadHelpBase(     )
    , m_xContext    (xContext)
    , m_pQuietInteraction( 0 )
{
}


LoadEnv::~LoadEnv()
{
}


css::uno::Reference< css::lang::XComponent > LoadEnv::loadComponentFromURL(const css::uno::Reference< css::frame::XComponentLoader >&    xLoader,
                                                                           const css::uno::Reference< css::uno::XComponentContext >&     xContext  ,
                                                                           const OUString&                                        sURL   ,
                                                                           const OUString&                                        sTarget,
                                                                                 sal_Int32                                               nFlags ,
                                                                           const css::uno::Sequence< css::beans::PropertyValue >&        lArgs  )
    throw(css::lang::IllegalArgumentException,
          css::io::IOException               ,
          css::uno::RuntimeException         )
{
    css::uno::Reference< css::lang::XComponent > xComponent;

    try
    {
        LoadEnv aEnv(xContext);

        aEnv.initializeLoading(sURL,
                               lArgs,
                               css::uno::Reference< css::frame::XFrame >(xLoader, css::uno::UNO_QUERY),
                               sTarget,
                               nFlags,
                               LoadEnv::E_NO_FEATURE);
        aEnv.startLoading();
        aEnv.waitWhileLoading(); 

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
                    ("Unsupported URL <" + sURL + ">" + ": \"" + ex.m_sMessage
                     + "\""),
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


utl::MediaDescriptor impl_mergeMediaDescriptorWithMightExistingModelArgs(const css::uno::Sequence< css::beans::PropertyValue >& lOutsideDescriptor)
{
    utl::MediaDescriptor lDescriptor(lOutsideDescriptor);
    css::uno::Reference< css::frame::XModel > xModel     = lDescriptor.getUnpackedValueOrDefault(
                                                            utl::MediaDescriptor::PROP_MODEL (),
                                                            css::uno::Reference< css::frame::XModel > ());
    if (xModel.is ())
    {
        utl::MediaDescriptor lModelDescriptor(xModel->getArgs());
        utl::MediaDescriptor::iterator pIt = lModelDescriptor.find( utl::MediaDescriptor::PROP_MACROEXECUTIONMODE() );
        if ( pIt != lModelDescriptor.end() )
            lDescriptor[utl::MediaDescriptor::PROP_MACROEXECUTIONMODE()] = pIt->second;
    }

    return lDescriptor;
}


void LoadEnv::initializeLoading(const OUString&                                           sURL            ,
                                const css::uno::Sequence< css::beans::PropertyValue >&           lMediaDescriptor,
                                const css::uno::Reference< css::frame::XFrame >&                 xBaseFrame      ,
                                const OUString&                                           sTarget         ,
                                      sal_Int32                                                  nSearchFlags    ,
                                      EFeature                                                   eFeature        , 
                                      EContentType                                               eContentType    ) 
{
    
    WriteGuard aWriteLock(m_aLock);

    
    if (m_xAsynchronousJob.is())
        throw LoadEnvException(LoadEnvException::ID_STILL_RUNNING);

    
    m_xTargetFrame.clear();
    m_xBaseFrame                    = xBaseFrame        ;
    m_lMediaDescriptor              = impl_mergeMediaDescriptorWithMightExistingModelArgs(lMediaDescriptor);
    m_sTarget                       = sTarget           ;
    m_nSearchFlags                  = nSearchFlags      ;
    m_eFeature                      = eFeature          ;
    m_eContentType                  = eContentType      ;
    m_bCloseFrameOnError            = sal_False         ;
    m_bReactivateControllerOnError  = sal_False         ;
    m_bLoaded                       = sal_False         ;

    
    
    
    
    if (m_eContentType == E_UNSUPPORTED_CONTENT)
    {
        m_eContentType = LoadEnv::classifyContent(sURL, lMediaDescriptor);
        if (m_eContentType == E_UNSUPPORTED_CONTENT)
            throw LoadEnvException(LoadEnvException::ID_UNSUPPORTED_CONTENT, "from LoadEnv::initializeLoading");
    }

    
    
    
    m_lMediaDescriptor[utl::MediaDescriptor::PROP_URL()] <<= sURL;

    
    m_aURL.Complete = sURL;
    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(m_xContext));
    xParser->parseStrict(m_aURL);

    
    
    if (!m_aURL.Mark.isEmpty())
        m_lMediaDescriptor[utl::MediaDescriptor::PROP_JUMPMARK()] <<= m_aURL.Mark;

    
    utl::MediaDescriptor::iterator pIt = m_lMediaDescriptor.find(utl::MediaDescriptor::PROP_FILENAME());
    if (pIt != m_lMediaDescriptor.end())
        m_lMediaDescriptor.erase(pIt);

    
    
    

    /*TODO progress is bound to a frame ... How can we set it here? */

    
    const bool bUIMode =
        ( ( m_eFeature & E_WORK_WITH_UI )                                                                          == E_WORK_WITH_UI ) &&
        ( m_lMediaDescriptor.getUnpackedValueOrDefault( utl::MediaDescriptor::PROP_HIDDEN() , sal_False ) == sal_False      ) &&
        ( m_lMediaDescriptor.getUnpackedValueOrDefault( utl::MediaDescriptor::PROP_PREVIEW(), sal_False ) == sal_False      );

    initializeUIDefaults(
        m_xContext,
        m_lMediaDescriptor,
        bUIMode,
        &m_pQuietInteraction
    );

    aWriteLock.unlock();
    
}


void LoadEnv::initializeUIDefaults( const css::uno::Reference< css::uno::XComponentContext >& i_rxContext,
                                    utl::MediaDescriptor& io_lMediaDescriptor, const bool i_bUIMode,
                                    QuietInteraction** o_ppQuietInteraction )
{
    css::uno::Reference< css::task::XInteractionHandler > xInteractionHandler;
    sal_Int16                                             nMacroMode         ;
    sal_Int16                                             nUpdateMode        ;

    if ( i_bUIMode )
    {
        nMacroMode  = css::document::MacroExecMode::USE_CONFIG;
        nUpdateMode = css::document::UpdateDocMode::ACCORDING_TO_CONFIG;
        try
        {
            xInteractionHandler.set( css::task::InteractionHandler::createWithParent( i_rxContext, 0 ), css::uno::UNO_QUERY_THROW );
        }
        catch(const css::uno::RuntimeException&) {throw;}
        catch(const css::uno::Exception&       ) {      }
    }
    
    else
    {
        nMacroMode  = css::document::MacroExecMode::NEVER_EXECUTE;
        nUpdateMode = css::document::UpdateDocMode::NO_UPDATE;
        QuietInteraction* pQuietInteraction = new QuietInteraction();
        xInteractionHandler = css::uno::Reference< css::task::XInteractionHandler >(static_cast< css::task::XInteractionHandler* >(pQuietInteraction), css::uno::UNO_QUERY);
        if ( o_ppQuietInteraction != NULL )
        {
            *o_ppQuietInteraction = pQuietInteraction;
            (*o_ppQuietInteraction)->acquire();
        }
    }

    if (
        (xInteractionHandler.is()                                                                                       ) &&
        (io_lMediaDescriptor.find(utl::MediaDescriptor::PROP_INTERACTIONHANDLER()) == io_lMediaDescriptor.end())
       )
    {
        io_lMediaDescriptor[utl::MediaDescriptor::PROP_INTERACTIONHANDLER()] <<= xInteractionHandler;
    }

    if (io_lMediaDescriptor.find(utl::MediaDescriptor::PROP_MACROEXECUTIONMODE()) == io_lMediaDescriptor.end())
        io_lMediaDescriptor[utl::MediaDescriptor::PROP_MACROEXECUTIONMODE()] <<= nMacroMode;

    if (io_lMediaDescriptor.find(utl::MediaDescriptor::PROP_UPDATEDOCMODE()) == io_lMediaDescriptor.end())
        io_lMediaDescriptor[utl::MediaDescriptor::PROP_UPDATEDOCMODE()] <<= nUpdateMode;
}


void LoadEnv::startLoading()
{
    
    ReadGuard aReadLock(m_aLock);

    
    if (m_xAsynchronousJob.is())
        throw LoadEnvException(LoadEnvException::ID_STILL_RUNNING);

    
    
    if (m_eContentType == E_UNSUPPORTED_CONTENT)
        throw LoadEnvException(LoadEnvException::ID_UNSUPPORTED_CONTENT, "from LoadEnv::startLoading");

    
    aReadLock.unlock();

    
    
    
    
    
    if (m_eContentType != E_CAN_BE_SET)/* Attention: special feature to set existing component on a frame must ignore type detection! */
        impl_detectTypeAndFilter();

    
    
    
    
    
    sal_Bool bStarted = sal_False;
    if (
        ((m_eFeature & E_ALLOW_CONTENTHANDLER) == E_ALLOW_CONTENTHANDLER) &&
        (m_eContentType                        != E_CAN_BE_SET          )   /* Attention: special feature to set existing component on a frame must ignore type detection! */
       )
    {
        bStarted = impl_handleContent();
    }

    if (!bStarted)
        bStarted = impl_loadContent();

    
    
    if (!bStarted)
        throw LoadEnvException(
            LoadEnvException::ID_GENERAL_ERROR, "not started");
}

/*-----------------------------------------------
    TODO
        First draft does not implement timeout using [ms].
        Current implementation counts yield calls only ...
-----------------------------------------------*/
sal_Bool LoadEnv::waitWhileLoading(sal_uInt32 nTimeout)
{
    
    
    
    

    sal_Int32 nTime = nTimeout;
    while(true)
    {
        
        ReadGuard aReadLock1(m_aLock);
        if (!m_xAsynchronousJob.is())
            break;
        aReadLock1.unlock();
        

        Application::Yield();

        
        if (nTimeout==0)
            continue;

        
        --nTime;
        if (nTime<1)
            break;
    }

    
    ReadGuard aReadLock2(m_aLock);
    return !m_xAsynchronousJob.is();
    
}

css::uno::Reference< css::lang::XComponent > LoadEnv::getTargetComponent() const
{
    
    ReadGuard aReadLock(m_aLock);

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
    throw(css::uno::RuntimeException)
{
    
    WriteGuard aWriteLock(m_aLock);

    if (m_bWaitingResult)
        m_pLoadEnv->impl_setResult(sal_True);
    m_bWaitingResult = false;

    aWriteLock.unlock();
    
}


void SAL_CALL LoadEnvListener::loadCancelled(const css::uno::Reference< css::frame::XFrameLoader >&)
    throw(css::uno::RuntimeException)
{
    
    WriteGuard aWriteLock(m_aLock);

    if (m_bWaitingResult)
        m_pLoadEnv->impl_setResult(sal_False);
    m_bWaitingResult = false;

    aWriteLock.unlock();
    
}


void SAL_CALL LoadEnvListener::dispatchFinished(const css::frame::DispatchResultEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    
    WriteGuard aWriteLock(m_aLock);

    if (!m_bWaitingResult)
        return;

    switch(aEvent.State)
    {
        case css::frame::DispatchResultState::FAILURE :
            m_pLoadEnv->impl_setResult(sal_False);
            break;

        case css::frame::DispatchResultState::SUCCESS :
            m_pLoadEnv->impl_setResult(sal_False);
            break;

        case css::frame::DispatchResultState::DONTKNOW :
            m_pLoadEnv->impl_setResult(sal_False);
            break;
    }
    m_bWaitingResult = false;

    aWriteLock.unlock();
    
}


void SAL_CALL LoadEnvListener::disposing(const css::lang::EventObject&)
    throw(css::uno::RuntimeException)
{
    
    WriteGuard aWriteLock(m_aLock);

    if (m_bWaitingResult)
        m_pLoadEnv->impl_setResult(sal_False);
    m_bWaitingResult = false;

    aWriteLock.unlock();
    
}


void LoadEnv::impl_setResult(sal_Bool bResult)
{
    
    WriteGuard aWriteLock(m_aLock);

    m_bLoaded = bResult;

    impl_reactForLoadingState();

    
    
    
    m_xAsynchronousJob.clear();

    aWriteLock.unlock();
    
}

/*-----------------------------------------------
    TODO: Is it a good idea to change Sequence<>
          parameter to stl-adapter?
-----------------------------------------------*/
LoadEnv::EContentType LoadEnv::classifyContent(const OUString&                                 sURL            ,
                                               const css::uno::Sequence< css::beans::PropertyValue >& lMediaDescriptor)
{
    
    
    
    
    
    
    
    
    

    if(
        (sURL.isEmpty()                                          ) ||
        (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_UNO    )) ||
        (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_SLOT   )) ||
        (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_MACRO  )) ||
        (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_SERVICE)) ||
        (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_MAILTO )) ||
        (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_NEWS   ))
      )
    {
        return E_UNSUPPORTED_CONTENT;
    }

    
    
    
    
    
    
    
    

    /* Attention: The following code can't work on such special URLs!
                  It should not break the office .. but it make no sense
                  to start expensive object creations and complex search
                  algorithm if its clear, that such URLs must be handled
                  in a special way .-)
    */

    
    if (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_PRIVATE_FACTORY))
        return E_CAN_BE_LOADED;

    
    utl::MediaDescriptor                 stlMediaDescriptor(lMediaDescriptor);
    utl::MediaDescriptor::const_iterator pIt;
    if (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_PRIVATE_STREAM))
    {
        pIt = stlMediaDescriptor.find(utl::MediaDescriptor::PROP_INPUTSTREAM());
        css::uno::Reference< css::io::XInputStream > xStream;
        if (pIt != stlMediaDescriptor.end())
            pIt->second >>= xStream;
        if (xStream.is())
            return E_CAN_BE_LOADED;
        SAL_INFO("fwk", "LoadEnv::classifyContent(): loading from stream with right URL but invalid stream detected");
        return E_UNSUPPORTED_CONTENT;
    }

    
    if (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_PRIVATE_OBJECT))
    {
        pIt = stlMediaDescriptor.find(utl::MediaDescriptor::PROP_MODEL());
        css::uno::Reference< css::frame::XModel > xModel;
        if (pIt != stlMediaDescriptor.end())
            pIt->second >>= xModel;
        if (xModel.is())
            return E_CAN_BE_SET;
        SAL_INFO("fwk", "LoadEnv::classifyContent(): loading with object with right URL but invalid object detected");
        return E_UNSUPPORTED_CONTENT;
    }

    
    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    css::uno::Reference< css::document::XTypeDetection > xDetect(
         xContext->getServiceManager()->createInstanceWithContext(
             "com.sun.star.document.TypeDetection", xContext),
         css::uno::UNO_QUERY_THROW);

    OUString sType = xDetect->queryTypeByURL(sURL);

    css::uno::Sequence< css::beans::NamedValue >           lQuery(1)   ;
    css::uno::Reference< css::frame::XLoaderFactory >      xLoaderFactory;
    css::uno::Reference< css::container::XEnumeration >    xSet        ;
    css::uno::Sequence< OUString >                  lTypesReg(1);


    
    
    
    
    
    
    
    
    
    
    
    
    

    OUString sPROP_TYPES(PROP_TYPES);

    lTypesReg[0]      = sType;
    lQuery[0].Name    = sPROP_TYPES;
    lQuery[0].Value <<= lTypesReg;

    xLoaderFactory = css::frame::FrameLoaderFactory::create(xContext);
    xSet       = xLoaderFactory->createSubSetEnumerationByProperties(lQuery);
    
    if (xSet->hasMoreElements())
        return E_CAN_BE_LOADED;

    
    
    
    

    lTypesReg[0]      = sType;
    lQuery[0].Name    = sPROP_TYPES;
    lQuery[0].Value <<= lTypesReg;

    xLoaderFactory = css::frame::ContentHandlerFactory::create(xContext);
    xSet       = xLoaderFactory->createSubSetEnumerationByProperties(lQuery);
    
    if (xSet->hasMoreElements())
        return E_CAN_BE_HANDLED;

    
    
    
    
    css::uno::Reference< css::ucb::XUniversalContentBroker > xUCB(css::ucb::UniversalContentBroker::create(xContext));
    if (xUCB->queryContentProvider(sURL).is())
        return E_CAN_BE_LOADED;

    
    
    
    
    
    return E_UNSUPPORTED_CONTENT;
}

namespace {

#if ENABLE_ORCUS

bool queryOrcusTypeAndFilter(const uno::Sequence<beans::PropertyValue>& rDescriptor, OUString& rType, OUString& rFilter)
{
    
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    if (!xContext.is() || !officecfg::Office::Common::Misc::ExperimentalMode::get(xContext))
    {
        return false;
    }

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

    OUString aUseOrcus;
    rtl::Bootstrap::get("LIBO_USE_ORCUS", aUseOrcus);
    bool bUseOrcus = (aUseOrcus == "YES");

    
    
    

    if (aURL.endsWith(".gnumeric"))
    {
        rType = "generic_Text";
        rFilter = "gnumeric";
        return true;
    }

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

#else

bool queryOrcusTypeAndFilter(const uno::Sequence<beans::PropertyValue>&, OUString&, OUString&)
{
    return false;
}

#endif

}

void LoadEnv::impl_detectTypeAndFilter()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    static OUString TYPEPROP_PREFERREDFILTER("PreferredFilter");
    static OUString FILTERPROP_FLAGS        ("Flags");
    static sal_Int32       FILTERFLAG_TEMPLATEPATH  = 16;

    
    ReadGuard aReadLock(m_aLock);

    
    
    
    css::uno::Sequence< css::beans::PropertyValue >        lDescriptor = m_lMediaDescriptor.getAsConstPropertyValueList();
    css::uno::Reference< css::uno::XComponentContext >     xContext = m_xContext;

    aReadLock.unlock();
    

    OUString sType, sFilter;

    if (queryOrcusTypeAndFilter(lDescriptor, sType, sFilter) && !sType.isEmpty() && !sFilter.isEmpty())
    {
        
        m_lMediaDescriptor << lDescriptor;
        m_lMediaDescriptor[utl::MediaDescriptor::PROP_TYPENAME()] <<= sType;
        m_lMediaDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
        m_lMediaDescriptor[utl::MediaDescriptor::PROP_FILTERPROVIDER()] <<= OUString("orcus");
        return;
    }

    css::uno::Reference< css::document::XTypeDetection > xDetect(
        xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.document.TypeDetection", xContext),
        css::uno::UNO_QUERY_THROW);
    sType = xDetect->queryTypeByDescriptor(lDescriptor, sal_True); /*TODO should deep detection be able for enable/disable it from outside? */

    
    if (sType.isEmpty())
        throw LoadEnvException(
            LoadEnvException::ID_UNSUPPORTED_CONTENT, "type detection failed");

    
    WriteGuard aWriteLock(m_aLock);

    
    m_lMediaDescriptor << lDescriptor;
    m_lMediaDescriptor[utl::MediaDescriptor::PROP_TYPENAME()] <<= sType;
    
    
    sFilter = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_FILTERNAME(), OUString());

    aWriteLock.unlock();
    

    
    
    
    
    
    
    
    if (sFilter.isEmpty())
    {
        
        
        css::uno::Reference< css::container::XNameAccess > xTypeCont(xDetect, css::uno::UNO_QUERY_THROW);
        try
        {
            ::comphelper::SequenceAsHashMap lTypeProps(xTypeCont->getByName(sType));
            sFilter = lTypeProps.getUnpackedValueOrDefault(TYPEPROP_PREFERREDFILTER, OUString());
            if (!sFilter.isEmpty())
            {
                
                aWriteLock.lock();
                m_lMediaDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
                aWriteLock.unlock();
                
            }
        }
        catch(const css::container::NoSuchElementException&)
            {}
    }

    
    
    
    
    
    
    

    sal_Bool bIsOwnTemplate = sal_False;
    if (!sFilter.isEmpty())
    {
        css::uno::Reference< css::container::XNameAccess > xFilterCont(xContext->getServiceManager()->createInstanceWithContext(SERVICENAME_FILTERFACTORY, xContext), css::uno::UNO_QUERY_THROW);
        try
        {
            ::comphelper::SequenceAsHashMap lFilterProps(xFilterCont->getByName(sFilter));
            sal_Int32 nFlags         = lFilterProps.getUnpackedValueOrDefault(FILTERPROP_FLAGS, (sal_Int32)0);
                      bIsOwnTemplate = ((nFlags & FILTERFLAG_TEMPLATEPATH) == FILTERFLAG_TEMPLATEPATH);
        }
        catch(const css::container::NoSuchElementException&)
            {}
    }
    if (bIsOwnTemplate)
    {
        
        aWriteLock.lock();
        
        utl::MediaDescriptor::const_iterator pAsTemplateItem = m_lMediaDescriptor.find(utl::MediaDescriptor::PROP_ASTEMPLATE());
        if (pAsTemplateItem == m_lMediaDescriptor.end())
            m_lMediaDescriptor[utl::MediaDescriptor::PROP_ASTEMPLATE()] <<= sal_True;
        aWriteLock.unlock();
        
    }
}


sal_Bool LoadEnv::impl_handleContent()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    
    ReadGuard aReadLock(m_aLock);

    
    OUString sType = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_TYPENAME(), OUString());
    if (sType.isEmpty())
        throw LoadEnvException(LoadEnvException::ID_INVALID_MEDIADESCRIPTOR);

    
    css::uno::Sequence< css::beans::PropertyValue > lDescriptor;
    m_lMediaDescriptor >> lDescriptor;
    css::util::URL aURL = m_aURL;

    
    css::uno::Reference< css::frame::XLoaderFactory > xLoaderFactory = css::frame::ContentHandlerFactory::create(m_xContext);

    aReadLock.unlock();
    

    
    css::uno::Sequence< OUString > lTypeReg(1);
    lTypeReg[0] = sType;

    css::uno::Sequence< css::beans::NamedValue > lQuery(1);
    lQuery[0].Name    = OUString(PROP_TYPES);
    lQuery[0].Value <<= lTypeReg;

    OUString sPROP_NAME(PROP_NAME);

    css::uno::Reference< css::container::XEnumeration > xSet = xLoaderFactory->createSubSetEnumerationByProperties(lQuery);
    while(xSet->hasMoreElements())
    {
        ::comphelper::SequenceAsHashMap lProps   (xSet->nextElement());
        OUString                 sHandler = lProps.getUnpackedValueOrDefault(sPROP_NAME, OUString());

        css::uno::Reference< css::frame::XNotifyingDispatch > xHandler;
        try
        {
            xHandler = css::uno::Reference< css::frame::XNotifyingDispatch >(xLoaderFactory->createInstance(sHandler), css::uno::UNO_QUERY);
            if (!xHandler.is())
                continue;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }

        
        WriteGuard aWriteLock(m_aLock);
        m_xAsynchronousJob = xHandler;
        LoadEnvListener* pListener = new LoadEnvListener(this);
        aWriteLock.unlock();
        

        css::uno::Reference< css::frame::XDispatchResultListener > xListener(static_cast< css::frame::XDispatchResultListener* >(pListener), css::uno::UNO_QUERY);
        xHandler->dispatchWithNotification(aURL, lDescriptor, xListener);

        return sal_True;
    }

    return sal_False;
}


sal_Bool LoadEnv::impl_furtherDocsAllowed()
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    

    sal_Bool bAllowed = sal_True;

    try
    {
        css::uno::Any aVal = ::comphelper::ConfigurationHelper::readDirectKey(
                                xContext,
                                OUString("org.openoffice.Office.Common/"),
                                OUString("Misc"),
                                OUString("MaxOpenDocuments"),
                                ::comphelper::ConfigurationHelper::E_READONLY);

        
        
        if ( ! aVal.hasValue())
            bAllowed = sal_True;
        else
        {
            sal_Int32 nMaxOpenDocuments = 0;
            aVal >>= nMaxOpenDocuments;

            css::uno::Reference< css::frame::XFramesSupplier > xDesktop(
                css::frame::Desktop::create(xContext),
                css::uno::UNO_QUERY_THROW);

            FrameListAnalyzer aAnalyzer(xDesktop,
                                        css::uno::Reference< css::frame::XFrame >(),
                                        FrameListAnalyzer::E_HELP |
                                        FrameListAnalyzer::E_BACKINGCOMPONENT |
                                        FrameListAnalyzer::E_HIDDEN);

            sal_Int32 nOpenDocuments = aAnalyzer.m_lOtherVisibleFrames.getLength();
                      bAllowed       = (nOpenDocuments < nMaxOpenDocuments);
        }
    }
    catch(const css::uno::Exception&)
        { bAllowed = sal_True; } 

    if ( ! bAllowed )
    {
        
        aReadLock.lock();
        css::uno::Reference< css::task::XInteractionHandler > xInteraction = m_lMediaDescriptor.getUnpackedValueOrDefault(
                                                                                utl::MediaDescriptor::PROP_INTERACTIONHANDLER(),
                                                                                css::uno::Reference< css::task::XInteractionHandler >());
        aReadLock.unlock();
        

        if (xInteraction.is())
        {
            css::uno::Any                                                                    aInteraction;
            css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations(2);

            comphelper::OInteractionAbort*   pAbort   = new comphelper::OInteractionAbort();
            comphelper::OInteractionApprove* pApprove = new comphelper::OInteractionApprove();

            lContinuations[0] = css::uno::Reference< css::task::XInteractionContinuation >(
                                    static_cast< css::task::XInteractionContinuation* >(pAbort),
                                    css::uno::UNO_QUERY_THROW);
            lContinuations[1] = css::uno::Reference< css::task::XInteractionContinuation >(
                                    static_cast< css::task::XInteractionContinuation* >(pApprove),
                                    css::uno::UNO_QUERY_THROW);

            css::task::ErrorCodeRequest aErrorCode;
            aErrorCode.ErrCode = ERRCODE_SFX_NOMOREDOCUMENTSALLOWED;
            aInteraction <<= aErrorCode;
            xInteraction->handle( InteractionRequest::CreateRequest(aInteraction, lContinuations) );
        }
    }

    return bAllowed;
}


sal_Bool LoadEnv::impl_loadContent()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    
    WriteGuard aWriteLock(m_aLock);

    
    OUString sTarget = m_sTarget;
    if (TargetHelper::matchSpecialTarget(sTarget, TargetHelper::E_DEFAULT))
    {
        m_xTargetFrame = impl_searchAlreadyLoaded();
        if (m_xTargetFrame.is())
        {
            impl_setResult(sal_True);
            return sal_True;
        }
        m_xTargetFrame = impl_searchRecycleTarget();
    }

    if (! m_xTargetFrame.is())
    {
        if (
            (TargetHelper::matchSpecialTarget(sTarget, TargetHelper::E_BLANK  )) ||
            (TargetHelper::matchSpecialTarget(sTarget, TargetHelper::E_DEFAULT))
           )
        {
            if (! impl_furtherDocsAllowed())
                return sal_False;
            m_xTargetFrame       = m_xBaseFrame->findFrame(SPECIALTARGET_BLANK, 0);
            m_bCloseFrameOnError = m_xTargetFrame.is();
        }
        else
        {
            sal_Int32 nFlags = m_nSearchFlags & ~css::frame::FrameSearchFlag::CREATE;
            m_xTargetFrame   = m_xBaseFrame->findFrame(sTarget, nFlags);
            if (! m_xTargetFrame.is())
            {
                if (! impl_furtherDocsAllowed())
                    return sal_False;
                m_xTargetFrame       = m_xBaseFrame->findFrame(SPECIALTARGET_BLANK, 0);
                m_bCloseFrameOnError = m_xTargetFrame.is();
            }
        }
    }

    
    
    if (
        ( ! m_xTargetFrame.is()                       ) ||
        ( ! m_xTargetFrame->getContainerWindow().is() )
       )
        throw LoadEnvException(LoadEnvException::ID_NO_TARGET_FOUND);

    css::uno::Reference< css::frame::XFrame > xTargetFrame = m_xTargetFrame;

    
    
    
    impl_applyPersistentWindowState(xTargetFrame->getContainerWindow());

    
    
    
    
    
    
    
    

    
    
    
    css::uno::Reference< css::document::XActionLockable > xTargetLock(xTargetFrame, css::uno::UNO_QUERY);
    m_aTargetLock.setResource(xTargetLock);

    
    
    
    
    
    sal_Bool                                           bHidden    = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_HIDDEN()         , sal_False                                           );
    sal_Bool                                           bMinimized = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_MINIMIZED()      , sal_False                                           );
    sal_Bool                                           bPreview   = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_PREVIEW()        , sal_False                                           );
    css::uno::Reference< css::task::XStatusIndicator > xProgress  = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_STATUSINDICATOR(), css::uno::Reference< css::task::XStatusIndicator >());

    if (!bHidden && !bMinimized && !bPreview && !xProgress.is())
    {
        
        css::uno::Reference< css::task::XStatusIndicatorFactory > xProgressFactory(xTargetFrame, css::uno::UNO_QUERY);
        if (xProgressFactory.is())
        {
            xProgress = xProgressFactory->createStatusIndicator();
            if (xProgress.is())
                m_lMediaDescriptor[utl::MediaDescriptor::PROP_STATUSINDICATOR()] <<= xProgress;
        }
    }

    
    css::uno::Sequence< css::beans::PropertyValue > lDescriptor;
    m_lMediaDescriptor >> lDescriptor;
    OUString sURL = m_aURL.Complete;

    
    css::uno::Reference< css::uno::XInterface >                xLoader     = impl_searchLoader();
    css::uno::Reference< css::frame::XFrameLoader >            xAsyncLoader(xLoader, css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XSynchronousFrameLoader > xSyncLoader (xLoader, css::uno::UNO_QUERY);

    if (xAsyncLoader.is())
    {
        
        aWriteLock.lock();
        m_xAsynchronousJob = xAsyncLoader;
        LoadEnvListener* pListener = new LoadEnvListener(this);
        aWriteLock.unlock();
        

        css::uno::Reference< css::frame::XLoadEventListener > xListener(static_cast< css::frame::XLoadEventListener* >(pListener), css::uno::UNO_QUERY);
        xAsyncLoader->load(xTargetFrame, sURL, lDescriptor, xListener);

        return sal_True;
    }
    else if (xSyncLoader.is())
    {
        sal_Bool bResult = xSyncLoader->load(lDescriptor, xTargetFrame);
        
        
        impl_setResult(bResult);
        
        
        return sal_True;
    }

    aWriteLock.unlock();
    

    return sal_False;
}


css::uno::Reference< css::uno::XInterface > LoadEnv::impl_searchLoader()
{
    
    ReadGuard aReadLock(m_aLock);

    
    
    
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

    
    
    
    OUString sType = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_TYPENAME(), OUString());
    if (sType.isEmpty())
        throw LoadEnvException(LoadEnvException::ID_INVALID_MEDIADESCRIPTOR);

    
    css::uno::Reference< css::frame::XLoaderFactory > xLoaderFactory = css::frame::FrameLoaderFactory::create(m_xContext);

    aReadLock.unlock();
    

    css::uno::Sequence< OUString > lTypesReg(1);
    lTypesReg[0] = sType;

    css::uno::Sequence< css::beans::NamedValue > lQuery(1);
    lQuery[0].Name    = OUString(PROP_TYPES);
    lQuery[0].Value <<= lTypesReg;

    OUString sPROP_NAME(PROP_NAME);

    css::uno::Reference< css::container::XEnumeration > xSet = xLoaderFactory->createSubSetEnumerationByProperties(lQuery);
    while(xSet->hasMoreElements())
    {
        
        
        ::comphelper::SequenceAsHashMap             lLoaderProps(xSet->nextElement());
        OUString                             sLoader     = lLoaderProps.getUnpackedValueOrDefault(sPROP_NAME, OUString());
        css::uno::Reference< css::uno::XInterface > xLoader     ;
        try
        {
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

    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    

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
    throw(LoadEnvException, css::uno::RuntimeException)
{
    
    ReadGuard aReadLock(m_aLock);

    
    
    if (
        ( ! TargetHelper::matchSpecialTarget(m_sTarget, TargetHelper::E_DEFAULT)                                               ) ||
        (m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_ASTEMPLATE() , sal_False) == sal_True) ||

        (m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_OPENNEWVIEW(), sal_False) == sal_True)
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    
    
    
    if (
        (ProtocolCheck::isProtocol(m_aURL.Complete, ProtocolCheck::E_PRIVATE_STREAM )) ||
        (ProtocolCheck::isProtocol(m_aURL.Complete, ProtocolCheck::E_PRIVATE_OBJECT ))
        /*TODO should be private:factory here tested too? */
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    
    
    css::uno::Reference< css::frame::XDesktop2 >  xSupplier = css::frame::Desktop::create( m_xContext );
    css::uno::Reference< css::container::XIndexAccess > xTaskList(xSupplier->getFrames()                      , css::uno::UNO_QUERY);

    if (!xTaskList.is())
        return css::uno::Reference< css::frame::XFrame >(); 

    
    
    
    sal_Int16 nNewVersion = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_VERSION(), (sal_Int16)(-1));

    
    
    
    css::uno::Reference< css::frame::XFrame > xHiddenTask;
    css::uno::Reference< css::frame::XFrame > xTask;

    sal_Int32 count = xTaskList->getCount();
    for (sal_Int32 i=0; i<count; ++i)
    {
        try
        {
            
            
            
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

            
            
            const OUString sURL = xModel->getURL();
            if (!::utl::UCBContentHelper::EqualURLs( m_aURL.Main, sURL ))
            {
                xTask.clear ();
                continue;
            }

            
            
            
            
            utl::MediaDescriptor lOldDocDescriptor(xModel->getArgs());

            if (lOldDocDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_VERSION(), (sal_Int32)(-1)) != nNewVersion)
            {
                xTask.clear ();
                continue;
            }

            
            
            
            ::sal_Bool bIsHidden = lOldDocDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_HIDDEN(), sal_False);
            if (
                (   bIsHidden       ) &&
                ( ! xHiddenTask.is())
               )
            {
                xHiddenTask = xTask;
                xTask.clear ();
                continue;
            }

            
            
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
        
        
        
        if (!m_aURL.Mark.isEmpty())
            impl_jumpToMark(xResult, m_aURL);

        
        impl_makeFrameWindowVisible(xResult->getContainerWindow(), sal_True);
    }

    aReadLock.unlock();
    

    return xResult;
}


sal_Bool LoadEnv::impl_isFrameAlreadyUsedForLoading(const css::uno::Reference< css::frame::XFrame >& xFrame) const
{
    css::uno::Reference< css::document::XActionLockable > xLock(xFrame, css::uno::UNO_QUERY);

    
    
    
    if (!xLock.is())
        return sal_False;

    
    return xLock->isActionLocked();
}


css::uno::Reference< css::frame::XFrame > LoadEnv::impl_searchRecycleTarget()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    
    ReadGuard aReadLock(m_aLock);

    
    
    
    
    if (m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_HIDDEN(), sal_False) == sal_True)
        return css::uno::Reference< css::frame::XFrame >();

    css::uno::Reference< css::frame::XFramesSupplier > xSupplier( css::frame::Desktop::create( m_xContext ), css::uno::UNO_QUERY);
    FrameListAnalyzer aTasksAnalyzer(xSupplier, css::uno::Reference< css::frame::XFrame >(), FrameListAnalyzer::E_BACKINGCOMPONENT);
    if (aTasksAnalyzer.m_xBackingComponent.is())
    {
        if (!impl_isFrameAlreadyUsedForLoading(aTasksAnalyzer.m_xBackingComponent))
        {
            
            impl_makeFrameWindowVisible(aTasksAnalyzer.m_xBackingComponent->getContainerWindow(), sal_True);
            return aTasksAnalyzer.m_xBackingComponent;
        }
    }

    
    if (
        (m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_ASTEMPLATE() , sal_False) == sal_True) ||
        (m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_OPENNEWVIEW(), sal_False) == sal_True)
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    
    
    if (
        (ProtocolCheck::isProtocol(m_aURL.Complete, ProtocolCheck::E_PRIVATE_FACTORY )) ||
        (ProtocolCheck::isProtocol(m_aURL.Complete, ProtocolCheck::E_PRIVATE_STREAM  )) ||
        (ProtocolCheck::isProtocol(m_aURL.Complete, ProtocolCheck::E_PRIVATE_OBJECT  ))
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    
    
    
    css::uno::Reference< css::frame::XFrame > xTask = xSupplier->getActiveFrame();

    
    if (!xTask.is())
        return css::uno::Reference< css::frame::XFrame >();

    
    css::uno::Reference< css::frame::XController > xController = xTask->getController();
    if (!xController.is())
        return css::uno::Reference< css::frame::XFrame >();

    
    css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
    if (!xModel.is())
        return css::uno::Reference< css::frame::XFrame >();

    

    
    
    
    if (xModel->getURL().getLength()>0)
        return css::uno::Reference< css::frame::XFrame >();

    
    css::uno::Reference< css::util::XModifiable > xModified(xModel, css::uno::UNO_QUERY);
    if (xModified->isModified())
        return css::uno::Reference< css::frame::XFrame >();

    Window* pWindow = VCLUnoHelper::GetWindow(xTask->getContainerWindow());
    if (pWindow && pWindow->IsInModalMode())
        return css::uno::Reference< css::frame::XFrame >();

    
    
    
    SvtModuleOptions::EFactory eOldApp = SvtModuleOptions::ClassifyFactoryByModel(xModel);
    SvtModuleOptions::EFactory eNewApp = SvtModuleOptions::ClassifyFactoryByURL  (m_aURL.Complete, m_lMediaDescriptor.getAsConstPropertyValueList());

    aReadLock.unlock();
    

    if (eOldApp != eNewApp)
        return css::uno::Reference< css::frame::XFrame >();

    
    
    
    
    
    
    if (impl_isFrameAlreadyUsedForLoading(xTask))
        return css::uno::Reference< css::frame::XFrame >();

    
    
    
    sal_Bool bReactivateOldControllerOnError = sal_False;
    css::uno::Reference< css::frame::XController > xOldDoc = xTask->getController();
    if (xOldDoc.is())
    {
        bReactivateOldControllerOnError = xOldDoc->suspend(sal_True);
        if (! bReactivateOldControllerOnError)
            return css::uno::Reference< css::frame::XFrame >();
    }

    
    WriteGuard aWriteLock(m_aLock);

    css::uno::Reference< css::document::XActionLockable > xLock(xTask, css::uno::UNO_QUERY);
    if (!m_aTargetLock.setResource(xLock))
        return css::uno::Reference< css::frame::XFrame >();

    m_bReactivateControllerOnError = bReactivateOldControllerOnError;
    aWriteLock.unlock();
    

    
    impl_makeFrameWindowVisible(xTask->getContainerWindow(), sal_True);

    return xTask;
}


void LoadEnv::impl_reactForLoadingState()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    /*TODO reset action locks */

    
    ReadGuard aReadLock(m_aLock);

    if (m_bLoaded)
    {
        
        
        
        css::uno::Reference< css::awt::XWindow > xWindow      = m_xTargetFrame->getContainerWindow();
        sal_Bool                                 bHidden      = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_HIDDEN(), sal_False);
        sal_Bool                                 bMinimized = m_lMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_MINIMIZED(), sal_False);

        if (bMinimized)
        {
            SolarMutexGuard aSolarGuard;
            Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
            
            if (pWindow && pWindow->IsSystemWindow())
                ((WorkWindow*)pWindow)->Minimize();
        }
        else if (!bHidden)
        {
            
            
            impl_makeFrameWindowVisible(xWindow, sal_False);
        }

        
        
        
        utl::MediaDescriptor::const_iterator pFrameName = m_lMediaDescriptor.find(utl::MediaDescriptor::PROP_FRAMENAME());
        if (pFrameName != m_lMediaDescriptor.end())
        {
            OUString sFrameName;
            pFrameName->second >>= sFrameName;
            
            
            if (TargetHelper::isValidNameForFrame(sFrameName))
                m_xTargetFrame->setName(sFrameName);
        }
    }
    else if (m_bReactivateControllerOnError)
    {
        
        css::uno::Reference< css::frame::XController > xOldDoc = m_xTargetFrame->getController();
        
        
        
        m_xTargetFrame.clear();
        if (xOldDoc.is())
        {
            sal_Bool bReactivated = xOldDoc->suspend(sal_False);
            if (!bReactivated)
                throw LoadEnvException(LoadEnvException::ID_COULD_NOT_REACTIVATE_CONTROLLER);
            m_bReactivateControllerOnError = sal_False;
        }
    }
    else if (m_bCloseFrameOnError)
    {
        
        css::uno::Reference< css::util::XCloseable > xCloseable (m_xTargetFrame, css::uno::UNO_QUERY);
        css::uno::Reference< css::lang::XComponent > xDisposable(m_xTargetFrame, css::uno::UNO_QUERY);

        try
        {
            if (xCloseable.is())
                xCloseable->close(sal_True);
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

    
    
    
    
    
    
    m_aTargetLock.freeResource();

    
    
    
    m_lMediaDescriptor.clear();

    css::uno::Any aRequest;
    bool bThrow = false;
    if ( !m_bLoaded && m_pQuietInteraction && m_pQuietInteraction->wasUsed() )
    {
        aRequest = m_pQuietInteraction->getRequest();
        m_pQuietInteraction->release();
        m_pQuietInteraction = 0;
        bThrow = true;
    }

    aReadLock.unlock();

    if (bThrow)
    {
        if  ( aRequest.isExtractableTo( ::cppu::UnoType< css::uno::Exception >::get() ) )
            throw LoadEnvException(
                LoadEnvException::ID_GENERAL_ERROR, "interaction request",
                aRequest);
    }

    
}


void LoadEnv::impl_makeFrameWindowVisible(const css::uno::Reference< css::awt::XWindow >& xWindow      ,
                                                sal_Bool bForceToFront)
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    

    SolarMutexGuard aSolarGuard;
    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    if ( pWindow )
    {
        bool const preview( m_lMediaDescriptor.getUnpackedValueOrDefault(
                utl::MediaDescriptor::PROP_PREVIEW(), sal_False) );

        bool bForceFrontAndFocus(false);
        if ( !preview )
        {
            css::uno::Any const a =
                ::comphelper::ConfigurationHelper::readDirectKey(
                  xContext,
                  OUString("org.openoffice.Office.Common/View"),
                  OUString("NewDocumentHandling"),
                  OUString("ForceFocusAndToFront"),
                  ::comphelper::ConfigurationHelper::E_READONLY);
            a >>= bForceFrontAndFocus;
        }

        if( pWindow->IsVisible() && (bForceFrontAndFocus || bForceToFront) )
            pWindow->ToTop();
        else
            pWindow->Show(true, (bForceFrontAndFocus || bForceToFront) ? SHOW_FOREGROUNDTASK : 0 );
    }
}


void LoadEnv::impl_applyPersistentWindowState(const css::uno::Reference< css::awt::XWindow >& xWindow)
{
    static OUString PACKAGE_SETUP_MODULES("/org.openoffice.Setup/Office/Factories");

    
    if (!xWindow.is())
        return;

    
    
    css::uno::Reference< css::awt::XWindow2 > xVisibleCheck(xWindow, css::uno::UNO_QUERY);
    if (
        (xVisibleCheck.is()        ) &&
        (xVisibleCheck->isVisible())
       )
       return;

    
    SolarMutexClearableGuard aSolarGuard1;

    Window*  pWindow       = VCLUnoHelper::GetWindow(xWindow);
    if (!pWindow)
        return;

    sal_Bool bSystemWindow = pWindow->IsSystemWindow();
    sal_Bool bWorkWindow   = (pWindow->GetType() == WINDOW_WORKWINDOW);

    if (!bSystemWindow && !bWorkWindow)
        return;

    
    WorkWindow* pWorkWindow = (WorkWindow*)pWindow;
    if (pWorkWindow->IsMinimized())
        return;

    aSolarGuard1.clear();
    

    
    ReadGuard aReadLock(m_aLock);

    
    OUString sFilter = m_lMediaDescriptor.getUnpackedValueOrDefault(
                                    utl::MediaDescriptor::PROP_FILTERNAME(),
                                    OUString());
    if (sFilter.isEmpty())
        return;

    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;

    aReadLock.unlock();
    

    try
    {
        
        css::uno::Reference< css::container::XNameAccess > xFilterCfg(
            xContext->getServiceManager()->createInstanceWithContext(SERVICENAME_FILTERFACTORY, xContext),
            css::uno::UNO_QUERY_THROW);
        ::comphelper::SequenceAsHashMap lProps (xFilterCfg->getByName(sFilter));
        OUString                 sModule = lProps.getUnpackedValueOrDefault(FILTER_PROPNAME_DOCUMENTSERVICE, OUString());

        
        css::uno::Reference< css::container::XNameAccess > xModuleCfg(::comphelper::ConfigurationHelper::openConfig(
                                                                        xContext,
                                                                        PACKAGE_SETUP_MODULES,
                                                                        ::comphelper::ConfigurationHelper::E_READONLY),
                                                                      css::uno::UNO_QUERY_THROW);

        
        
        
        OUString sWindowState ;
        ::comphelper::ConfigurationHelper::readRelativeKey(xModuleCfg, sModule, OFFICEFACTORY_PROPNAME_WINDOWATTRIBUTES) >>= sWindowState;
        if (!sWindowState.isEmpty())
        {
            
            SolarMutexGuard aSolarGuard;

            
            
            
            
            
            Window* pWindowCheck  = VCLUnoHelper::GetWindow(xWindow);
            if (! pWindowCheck)
                return;

            SystemWindow* pSystemWindow = (SystemWindow*)pWindowCheck;
            pSystemWindow->SetWindowState(OUStringToOString(sWindowState,RTL_TEXTENCODING_UTF8));
            
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
