/* -*- Mode: C++; eval:(c-set-style "bsd"); tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <loadenv/loadenv.hxx>

#include <loadenv/targethelper.hxx>
#include <framework/framelistanalyzer.hxx>

#include <constant/containerquery.hxx>
#include <interaction/quietinteraction.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/resetableguard.hxx>
#include <properties.h>
#include <protocols.h>
#include <services.h>
#include <comphelper/interaction.hxx>
#include <framework/interaction.hxx>

#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>

#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/syswin.hxx>

#include <toolkit/unohlp.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbhelper.hxx>
#include <comphelper/configurationhelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

const char PROP_TYPES[] = "Types";
const char PROP_NAME[] = "Name";

namespace framework {

// may there exist already a define .-(
#ifndef css
namespace css = ::com::sun::star;
#endif
using namespace com::sun::star;


class LoadEnvListener : private ThreadHelpBase
                      , public ::cppu::WeakImplHelper2< css::frame::XLoadEventListener      ,
                                                        css::frame::XDispatchResultListener >
{
    private:

        bool m_bWaitingResult;
        LoadEnv* m_pLoadEnv;

    public:

        //_______________________________________
        LoadEnvListener(LoadEnv* pLoadEnv)
            : m_bWaitingResult(true)
            , m_pLoadEnv(pLoadEnv)
        {
        }

        //_______________________________________
        // frame.XLoadEventListener
        virtual void SAL_CALL loadFinished(const css::uno::Reference< css::frame::XFrameLoader >& xLoader)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL loadCancelled(const css::uno::Reference< css::frame::XFrameLoader >& xLoader)
            throw(css::uno::RuntimeException);

        //_______________________________________
        // frame.XDispatchResultListener
        virtual void SAL_CALL dispatchFinished(const css::frame::DispatchResultEvent& aEvent)
            throw(css::uno::RuntimeException);

        //_______________________________________
        // lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);
};


LoadEnv::LoadEnv(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    throw(LoadEnvException, css::uno::RuntimeException)
    : ThreadHelpBase(     )
    , m_xSMGR       (xSMGR)
    , m_pQuietInteraction( 0 )
{
}


LoadEnv::~LoadEnv()
{
}


css::uno::Reference< css::lang::XComponent > LoadEnv::loadComponentFromURL(const css::uno::Reference< css::frame::XComponentLoader >&    xLoader,
                                                                           const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR  ,
                                                                           const ::rtl::OUString&                                        sURL   ,
                                                                           const ::rtl::OUString&                                        sTarget,
                                                                                 sal_Int32                                               nFlags ,
                                                                           const css::uno::Sequence< css::beans::PropertyValue >&        lArgs  )
    throw(css::lang::IllegalArgumentException,
          css::io::IOException               ,
          css::uno::RuntimeException         )
{
    css::uno::Reference< css::lang::XComponent > xComponent;

    try
    {
        LoadEnv aEnv(xSMGR);

        aEnv.initializeLoading(sURL,
                               lArgs,
                               css::uno::Reference< css::frame::XFrame >(xLoader, css::uno::UNO_QUERY),
                               sTarget,
                               nFlags,
                               LoadEnv::E_NO_FEATURE);
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
                    ::rtl::OUString("Optional list of arguments seem to be corrupted."),
                    xLoader,
                    4);

            case LoadEnvException::ID_UNSUPPORTED_CONTENT:
            {
                rtl::OUStringBuffer aMsg;
                aMsg.appendAscii(RTL_CONSTASCII_STRINGPARAM("Unsupported URL <")).
                    append(sURL).append('>');

                if (!ex.m_sMessage.isEmpty())
                {
                    aMsg.appendAscii(RTL_CONSTASCII_STRINGPARAM(": \"")).
                        append(rtl::OStringToOUString(
                             ex.m_sMessage, RTL_TEXTENCODING_UTF8)).
                        appendAscii(RTL_CONSTASCII_STRINGPARAM("\""));
                }

                throw css::lang::IllegalArgumentException(aMsg.makeStringAndClear(),
                    xLoader, 1);
            }

            default:
                xComponent.clear();
                break;
        }
    }

    return xComponent;
}

//-----------------------------------------------
::comphelper::MediaDescriptor impl_mergeMediaDescriptorWithMightExistingModelArgs(const css::uno::Sequence< css::beans::PropertyValue >& lOutsideDescriptor)
{
    ::comphelper::MediaDescriptor lDescriptor(lOutsideDescriptor);
    css::uno::Reference< css::frame::XModel > xModel     = lDescriptor.getUnpackedValueOrDefault(
                                                            ::comphelper::MediaDescriptor::PROP_MODEL (),
                                                            css::uno::Reference< css::frame::XModel > ());
    if (xModel.is ())
    {
        ::comphelper::MediaDescriptor lModelDescriptor(xModel->getArgs());
        ::comphelper::MediaDescriptor::iterator pIt = lModelDescriptor.find( ::comphelper::MediaDescriptor::PROP_MACROEXECUTIONMODE() );
        if ( pIt != lModelDescriptor.end() )
            lDescriptor[::comphelper::MediaDescriptor::PROP_MACROEXECUTIONMODE()] = pIt->second;
    }

    return lDescriptor;
}


void LoadEnv::initializeLoading(const ::rtl::OUString&                                           sURL            ,
                                const css::uno::Sequence< css::beans::PropertyValue >&           lMediaDescriptor,
                                const css::uno::Reference< css::frame::XFrame >&                 xBaseFrame      ,
                                const ::rtl::OUString&                                           sTarget         ,
                                      sal_Int32                                                  nSearchFlags    ,
                                      EFeature                                                   eFeature        , // => use default ...
                                      EContentType                                               eContentType    ) // => use default ...
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // Handle still running processes!
    if (m_xAsynchronousJob.is())
        throw LoadEnvException(LoadEnvException::ID_STILL_RUNNING);

    // take over all new parameters.
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

    // try to find out, if its really a content, which can be loaded or must be "handled"
    // We use a default value for this in-parameter. Then we have to start a complex check method
    // internally. But if this check was already done outside it can be supressed to perform
    // the load request. We take over the result then!
    if (m_eContentType == E_UNSUPPORTED_CONTENT)
    {
        m_eContentType = LoadEnv::classifyContent(sURL, lMediaDescriptor);
        if (m_eContentType == E_UNSUPPORTED_CONTENT)
            throw LoadEnvException(LoadEnvException::ID_UNSUPPORTED_CONTENT);
    }

    // make URL part of the MediaDescriptor
    // It doesn't mater, if it is already an item of it.
    // It must be the same value ... so we can overwrite it :-)
    m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_URL()] <<= sURL;

    // parse it - because some following code require that
    m_aURL.Complete = sURL;
    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(::comphelper::getComponentContext(m_xSMGR)));
    xParser->parseStrict(m_aURL);

    // BTW: Split URL and JumpMark ...
    // Because such mark is an explicit value of the media descriptor!
    if (!m_aURL.Mark.isEmpty())
        m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_JUMPMARK()] <<= m_aURL.Mark;

    // By the way: remove the old and deprecated value "FileName" from the descriptor!
    ::comphelper::MediaDescriptor::iterator pIt = m_lMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_FILENAME());
    if (pIt != m_lMediaDescriptor.end())
        m_lMediaDescriptor.erase(pIt);

    // patch the MediaDescriptor, so it fullfill the outside requirements
    // Means especially items like e.g. UI InteractionHandler, Status Indicator,
    // MacroExecutionMode etcpp.

    /*TODO progress is bound to a frame ... How can we set it here? */

    // UI mode
    const bool bUIMode =
        ( ( m_eFeature & E_WORK_WITH_UI )                                                                          == E_WORK_WITH_UI ) &&
        ( m_lMediaDescriptor.getUnpackedValueOrDefault( ::comphelper::MediaDescriptor::PROP_HIDDEN() , sal_False ) == sal_False      ) &&
        ( m_lMediaDescriptor.getUnpackedValueOrDefault( ::comphelper::MediaDescriptor::PROP_PREVIEW(), sal_False ) == sal_False      );

    initializeUIDefaults(
        m_xSMGR,
        m_lMediaDescriptor,
        bUIMode,
        &m_pQuietInteraction
    );

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}


void LoadEnv::initializeUIDefaults( const css::uno::Reference< css::lang::XMultiServiceFactory >& i_rSMGR,
                                    ::comphelper::MediaDescriptor& io_lMediaDescriptor, const bool i_bUIMode,
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
            xInteractionHandler = css::uno::Reference< css::task::XInteractionHandler >(i_rSMGR->createInstance(IMPLEMENTATIONNAME_UIINTERACTIONHANDLER), css::uno::UNO_QUERY);
        }
        catch(const css::uno::RuntimeException&) {throw;}
        catch(const css::uno::Exception&       ) {      }
    }
    // hidden mode
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
        (io_lMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_INTERACTIONHANDLER()) == io_lMediaDescriptor.end())
       )
    {
        io_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_INTERACTIONHANDLER()] <<= xInteractionHandler;
    }

    if (io_lMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_MACROEXECUTIONMODE()) == io_lMediaDescriptor.end())
        io_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_MACROEXECUTIONMODE()] <<= nMacroMode;

    if (io_lMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_UPDATEDOCMODE()) == io_lMediaDescriptor.end())
        io_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_UPDATEDOCMODE()] <<= nUpdateMode;
}


void LoadEnv::startLoading()
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    // Handle still running processes!
    if (m_xAsynchronousJob.is())
        throw LoadEnvException(LoadEnvException::ID_STILL_RUNNING);

    // content can not be loaded or handled
    // check "classifyContent()" failed before ...
    if (m_eContentType == E_UNSUPPORTED_CONTENT)
        throw LoadEnvException(LoadEnvException::ID_UNSUPPORTED_CONTENT);

    // <- SAFE
    aReadLock.unlock();

    // detect its type/filter etcpp.
    // These information will be available by the
    // used descriptor member afterwards and is needed
    // for all following operations!
    // Note: An exception will be thrown, in case operation was not successfully ...
    if (m_eContentType != E_CAN_BE_SET)/* Attention: special feature to set existing component on a frame must ignore type detection! */
        impl_detectTypeAndFilter();

    // start loading the content ...
    // Attention: Dont check m_eContentType deeper then UNSUPPORTED/SUPPORTED!
    // Because it was made in th easiest way ... may a flat detection was made only.
    // And such simple detection can fail some times .-)
    // Use another strategy here. Try it and let it run into the case "loading not possible".
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

    // not started => general error
    // We can't say - what was the reason for.
    if (!bStarted)
        throw LoadEnvException(LoadEnvException::ID_GENERAL_ERROR);
}

/*-----------------------------------------------
    TODO
        First draft does not implement timeout using [ms].
        Current implementation counts yield calls only ...
-----------------------------------------------*/
sal_Bool LoadEnv::waitWhileLoading(sal_uInt32 nTimeout)
{
    // Because its not a good idea to block the main thread
    // (and we can't be sure that we are currently not used inside the
    // main thread!), we can't use conditions here really. We must yield
    // in an intellegent manner :-)

    sal_Int32 nTime = nTimeout;
    while(true)
    {
        // SAFE -> ------------------------------
        ReadGuard aReadLock1(m_aLock);
        if (!m_xAsynchronousJob.is())
            break;
        aReadLock1.unlock();
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

    // SAFE -> ----------------------------------
    ReadGuard aReadLock2(m_aLock);
    return !m_xAsynchronousJob.is();
    // <- SAFE ----------------------------------
}

css::uno::Reference< css::lang::XComponent > LoadEnv::getTargetComponent() const
{
    // SAFE ->
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
    // <- SAFE
}


void SAL_CALL LoadEnvListener::loadFinished(const css::uno::Reference< css::frame::XFrameLoader >&)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (m_bWaitingResult)
        m_pLoadEnv->impl_setResult(sal_True);
    m_bWaitingResult = false;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}


void SAL_CALL LoadEnvListener::loadCancelled(const css::uno::Reference< css::frame::XFrameLoader >&)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (m_bWaitingResult)
        m_pLoadEnv->impl_setResult(sal_False);
    m_bWaitingResult = false;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}


void SAL_CALL LoadEnvListener::dispatchFinished(const css::frame::DispatchResultEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
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
    // <- SAFE ----------------------------------
}


void SAL_CALL LoadEnvListener::disposing(const css::lang::EventObject&)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (m_bWaitingResult)
        m_pLoadEnv->impl_setResult(sal_False);
    m_bWaitingResult = false;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}


void LoadEnv::impl_setResult(sal_Bool bResult)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    m_bLoaded = bResult;

    impl_reactForLoadingState();

    // clearing of this reference will unblock waitWhileLoading()!
    // So we must be sure, that loading process was really finished.
    // => do it as last operation of this method ...
    m_xAsynchronousJob.clear();

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

/*-----------------------------------------------
    TODO: Is it a good idea to change Sequence<>
          parameter to stl-adapter?
-----------------------------------------------*/
LoadEnv::EContentType LoadEnv::classifyContent(const ::rtl::OUString&                                 sURL            ,
                                               const css::uno::Sequence< css::beans::PropertyValue >& lMediaDescriptor)
{
    //-------------------------------------------
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

    //-------------------------------------------
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
    if (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_PRIVATE_FACTORY))
        return E_CAN_BE_LOADED;

    // using of an existing input stream
    ::comphelper::MediaDescriptor                 stlMediaDescriptor(lMediaDescriptor);
    ::comphelper::MediaDescriptor::const_iterator pIt;
    if (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_PRIVATE_STREAM))
    {
        pIt = stlMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_INPUTSTREAM());
        css::uno::Reference< css::io::XInputStream > xStream;
        if (pIt != stlMediaDescriptor.end())
            pIt->second >>= xStream;
        if (xStream.is())
            return E_CAN_BE_LOADED;
        LOG_WARNING("LoadEnv::classifyContent()", "loading from stream with right URL but invalid stream detected")
        return E_UNSUPPORTED_CONTENT;
    }

    // using of a full featured document
    if (ProtocolCheck::isProtocol(sURL,ProtocolCheck::E_PRIVATE_OBJECT))
    {
        pIt = stlMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_MODEL());
        css::uno::Reference< css::frame::XModel > xModel;
        if (pIt != stlMediaDescriptor.end())
            pIt->second >>= xModel;
        if (xModel.is())
            return E_CAN_BE_SET;
        LOG_WARNING("LoadEnv::classifyContent()", "loading with object with right URL but invalid object detected")
        return E_UNSUPPORTED_CONTENT;
    }

    // following operatons can work on an internal type name only :-(
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    css::uno::Reference< css::document::XTypeDetection > xDetect(xSMGR->createInstance(SERVICENAME_TYPEDETECTION), css::uno::UNO_QUERY);

    ::rtl::OUString sType = xDetect->queryTypeByURL(sURL);

    css::uno::Sequence< css::beans::NamedValue >           lQuery(1)   ;
    css::uno::Reference< css::container::XContainerQuery > xContainer  ;
    css::uno::Reference< css::container::XEnumeration >    xSet        ;
    css::uno::Sequence< ::rtl::OUString >                  lTypesReg(1);


    //-------------------------------------------
    // (iii) If a FrameLoader service (or at least
    //      a Filter) can be found, which supports
    //      this URL - it must be a loadable content.
    //      Because both items are registered for types
    //      its enough to check for frame loaders only.
    //      Mos of our filters are handled by our global
    //      default loader. But there exist some specialized
    //      loader, which does not work on top of filters!
    //      So its not enough to search on the filter configuration.
    //      Further its not enough to search for types!
    //      Because there exist some types, which are referenced by
    //      other objects ... but not by filters nor frame loaders!

    rtl::OUString sPROP_TYPES(PROP_TYPES);

    lTypesReg[0]      = sType;
    lQuery[0].Name    = sPROP_TYPES;
    lQuery[0].Value <<= lTypesReg;

    xContainer = css::uno::Reference< css::container::XContainerQuery >(xSMGR->createInstance(SERVICENAME_FRAMELOADERFACTORY), css::uno::UNO_QUERY);
    xSet       = xContainer->createSubSetEnumerationByProperties(lQuery);
    // at least one registered frame loader is enough!
    if (xSet->hasMoreElements())
        return E_CAN_BE_LOADED;

    //-------------------------------------------
    // (iv) Some URL protocols are supported by special services.
    //      E.g. ContentHandler.
    //      Such contents can be handled ... but not loaded.

    lTypesReg[0]      = sType;
    lQuery[0].Name    = sPROP_TYPES;
    lQuery[0].Value <<= lTypesReg;

    xContainer = css::uno::Reference< css::container::XContainerQuery >(xSMGR->createInstance(SERVICENAME_CONTENTHANDLERFACTORY), css::uno::UNO_QUERY);
    xSet       = xContainer->createSubSetEnumerationByProperties(lQuery);
    // at least one registered content handler is enough!
    if (xSet->hasMoreElements())
        return E_CAN_BE_HANDLED;

    //-------------------------------------------
    // (v) Last but not least the UCB is used inside office to
    //     load contents. He has a special configuration to know
    //     which URL schemata can be used inside office.
    css::uno::Reference< css::ucb::XUniversalContentBroker > xUCB(css::ucb::UniversalContentBroker::create(comphelper::getComponentContext(xSMGR)));
    if (xUCB->queryContentProvider(sURL).is())
        return E_CAN_BE_LOADED;

    //-------------------------------------------
    // (TODO) At this point, we have no idea .-)
    //        But it seems to be better, to break all
    //        further requests for this URL. Otherwhise
    //        we can run into some trouble.
    LOG_WARNING("LoadEnv::classifyContent()", "really an unsupported content?")
    return E_UNSUPPORTED_CONTENT;
}

namespace {

#if 1
bool queryOrcusTypeAndFilter(const uno::Sequence<beans::PropertyValue>&, OUString&, OUString&)
{
    return false;
}
#else
// TODO: We will reinstate this function later, so don't remove this!
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

    if (aURL.endsWith(".csv"))
    {
        // Use .csv as the first test file type.
        rType = "generic_Text";
        rFilter = "orcus-test-filter";
        return true;
    }

    return false;
}
#endif

}

void LoadEnv::impl_detectTypeAndFilter()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    static ::rtl::OUString TYPEPROP_PREFERREDFILTER("PreferredFilter");
    static ::rtl::OUString FILTERPROP_FLAGS        ("Flags");
    static sal_Int32       FILTERFLAG_TEMPLATEPATH  = 16;

    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    // Attention: Because our stl media descriptor is a copy of an uno sequence
    // we can't use as an in/out parameter here. Copy it before and dont forget to
    // update structure afterwards again!
    css::uno::Sequence< css::beans::PropertyValue >        lDescriptor = m_lMediaDescriptor.getAsConstPropertyValueList();
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR       = m_xSMGR;

    aReadLock.unlock();
    // <- SAFE

    rtl::OUString sType, sFilter;

    if (queryOrcusTypeAndFilter(lDescriptor, sType, sFilter) && !sType.isEmpty() && !sFilter.isEmpty())
    {
        // Orcus type detected.  Skip the normal type detection process.
        m_lMediaDescriptor << lDescriptor;
        m_lMediaDescriptor[comphelper::MediaDescriptor::PROP_TYPENAME()] <<= sType;
        m_lMediaDescriptor[comphelper::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
        m_lMediaDescriptor[comphelper::MediaDescriptor::PROP_FILTERPROVIDER()] <<= OUString("orcus");
        return;
    }

    css::uno::Reference< css::document::XTypeDetection > xDetect(xSMGR->createInstance(SERVICENAME_TYPEDETECTION), css::uno::UNO_QUERY);
    if (xDetect.is())
        sType = xDetect->queryTypeByDescriptor(lDescriptor, sal_True); /*TODO should deep detection be able for enable/disable it from outside? */

    // no valid content -> loading not possible
    if (sType.isEmpty())
        throw LoadEnvException(LoadEnvException::ID_UNSUPPORTED_CONTENT);

    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    // detection was successfully => update the descriptor member of this class
    m_lMediaDescriptor << lDescriptor;
    m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_TYPENAME()] <<= sType;
    // Is there an already detected (may be preselected) filter?
    // see below ...
    sFilter = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_FILTERNAME(), ::rtl::OUString());

    aWriteLock.unlock();
    // <- SAFE

    // But the type isnt enough. For loading sometimes we need more informations.
    // E.g. for our "_default" feature, where we recylce any frame which contains
    // and "Untitled" document, we must know if the new document is based on a template!
    // But this information is available as a filter property only.
    // => We must try(!) to detect the right filter for this load request.
    // On the other side ... if no filter is available .. ignore it.
    // Then the type information must be enough.
    if (sFilter.isEmpty())
    {
        // no -> try to find a preferred filter for the detected type.
        // Dont forget to updatet he media descriptor.
        css::uno::Reference< css::container::XNameAccess > xTypeCont(xDetect, css::uno::UNO_QUERY_THROW);
        try
        {
            ::comphelper::SequenceAsHashMap lTypeProps(xTypeCont->getByName(sType));
            sFilter = lTypeProps.getUnpackedValueOrDefault(TYPEPROP_PREFERREDFILTER, ::rtl::OUString());
            if (!sFilter.isEmpty())
            {
                // SAFE ->
                aWriteLock.lock();
                m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
                aWriteLock.unlock();
                // <- SAFE
            }
        }
        catch(const css::container::NoSuchElementException&)
            {}
    }

    // check if the filter (if one exists) points to a template format filter.
    // Then we have to add the property "AsTemplate".
    // We need this information to decide afterwards if we can use a "recycle frame"
    // for target "_default" or has to create a new one everytimes.
    // On the other side we have to supress that, if this property already exists
    // and should trigger a special handling. Then the outside calli of this method here,
    // has to know, what he is doing .-)

    sal_Bool bIsOwnTemplate = sal_False;
    if (!sFilter.isEmpty())
    {
        css::uno::Reference< css::container::XNameAccess > xFilterCont(xSMGR->createInstance(SERVICENAME_FILTERFACTORY), css::uno::UNO_QUERY_THROW);
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
        // SAFE ->
        aWriteLock.lock();
        // Dont overwrite external decisions! See comments before ...
        ::comphelper::MediaDescriptor::const_iterator pAsTemplateItem = m_lMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_ASTEMPLATE());
        if (pAsTemplateItem == m_lMediaDescriptor.end())
            m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_ASTEMPLATE()] <<= sal_True;
        aWriteLock.unlock();
        // <- SAFE
    }
}


sal_Bool LoadEnv::impl_handleContent()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // SAFE -> -----------------------------------
    ReadGuard aReadLock(m_aLock);

    // the type must exist inside the descriptor ... otherwhise this class is implemented wrong :-)
    ::rtl::OUString sType = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_TYPENAME(), ::rtl::OUString());
    if (sType.isEmpty())
        throw LoadEnvException(LoadEnvException::ID_INVALID_MEDIADESCRIPTOR);

    // convert media descriptor and URL to right format for later interface call!
    css::uno::Sequence< css::beans::PropertyValue > lDescriptor;
    m_lMediaDescriptor >> lDescriptor;
    css::util::URL aURL = m_aURL;

    // get neccessary container to query for a handler object
    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory(m_xSMGR->createInstance(SERVICENAME_CONTENTHANDLERFACTORY), css::uno::UNO_QUERY);
    css::uno::Reference< css::container::XContainerQuery > xQuery  (xFactory                                                  , css::uno::UNO_QUERY);

    aReadLock.unlock();
    // <- SAFE -----------------------------------

    // query
    css::uno::Sequence< ::rtl::OUString > lTypeReg(1);
    lTypeReg[0] = sType;

    css::uno::Sequence< css::beans::NamedValue > lQuery(1);
    lQuery[0].Name    = rtl::OUString(PROP_TYPES);
    lQuery[0].Value <<= lTypeReg;

    ::rtl::OUString sPROP_NAME(PROP_NAME);

    css::uno::Reference< css::container::XEnumeration > xSet = xQuery->createSubSetEnumerationByProperties(lQuery);
    while(xSet->hasMoreElements())
    {
        ::comphelper::SequenceAsHashMap lProps   (xSet->nextElement());
        ::rtl::OUString                 sHandler = lProps.getUnpackedValueOrDefault(sPROP_NAME, ::rtl::OUString());

        css::uno::Reference< css::frame::XNotifyingDispatch > xHandler;
        try
        {
            xHandler = css::uno::Reference< css::frame::XNotifyingDispatch >(xFactory->createInstance(sHandler), css::uno::UNO_QUERY);
            if (!xHandler.is())
                continue;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }

        // SAFE -> -----------------------------------
        WriteGuard aWriteLock(m_aLock);
        m_xAsynchronousJob = xHandler;
        LoadEnvListener* pListener = new LoadEnvListener(this);
        aWriteLock.unlock();
        // <- SAFE -----------------------------------

        css::uno::Reference< css::frame::XDispatchResultListener > xListener(static_cast< css::frame::XDispatchResultListener* >(pListener), css::uno::UNO_QUERY);
        xHandler->dispatchWithNotification(aURL, lDescriptor, xListener);

        return sal_True;
    }

    return sal_False;
}

//-----------------------------------------------
sal_Bool LoadEnv::impl_furtherDocsAllowed()
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE

    sal_Bool bAllowed = sal_True;

    try
    {
        css::uno::Any aVal = ::comphelper::ConfigurationHelper::readDirectKey(
                                xSMGR,
                                ::rtl::OUString("org.openoffice.Office.Common/"),
                                ::rtl::OUString("Misc"),
                                ::rtl::OUString("MaxOpenDocuments"),
                                ::comphelper::ConfigurationHelper::E_READONLY);

        // NIL means: count of allowed documents = infinite !
        //     => return sal_True
        if ( ! aVal.hasValue())
            bAllowed = sal_True;
        else
        {
            sal_Int32 nMaxOpenDocuments = 0;
            aVal >>= nMaxOpenDocuments;

            css::uno::Reference< css::frame::XFramesSupplier > xDesktop(
                xSMGR->createInstance(SERVICENAME_DESKTOP),
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
        { bAllowed = sal_True; } // !! internal errors are no reason to disturb the office from opening documents .-)

    if ( ! bAllowed )
    {
        // SAFE ->
        aReadLock.lock();
        css::uno::Reference< css::task::XInteractionHandler > xInteraction = m_lMediaDescriptor.getUnpackedValueOrDefault(
                                                                                ::comphelper::MediaDescriptor::PROP_INTERACTIONHANDLER(),
                                                                                css::uno::Reference< css::task::XInteractionHandler >());
        aReadLock.unlock();
        // <- SAFE

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

//-----------------------------------------------
sal_Bool LoadEnv::impl_loadContent()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // SAFE -> -----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // search or create right target frame
    ::rtl::OUString sTarget = m_sTarget;
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

    // If we couldn't find a valid frame or the frame has no container window
    // we have to throw an exception.
    if (
        ( ! m_xTargetFrame.is()                       ) ||
        ( ! m_xTargetFrame->getContainerWindow().is() )
       )
        throw LoadEnvException(LoadEnvException::ID_NO_TARGET_FOUND);

    css::uno::Reference< css::frame::XFrame > xTargetFrame = m_xTargetFrame;

    // Now we have a valid frame ... and type detection was already done.
    // We should apply the module dependend window position and size to the
    // frame window.
    impl_applyPersistentWindowState(xTargetFrame->getContainerWindow());

    // Don't forget to lock task for following load process. Otherwise it could die
    // during this operation runs by terminating the office or closing this task via api.
    // If we set this lock "close()" will return false and closing will be broken.
    // Attention: Don't forget to reset this lock again after finishing operation.
    // Otherwise task AND office couldn't die!!!
    // This includes gracefully handling of Exceptions (Runtime!) too ...
    // Thats why we use a specialized guard, which will reset the lock
    // if it will be run out of scope.

    // Note further: ignore if this internal guard already contains a resource.
    // Might impl_searchRecylcTarget() set it before. But incase this impl-method wasnt used
    // and the target frame was new created ... this lock here must be set!
    css::uno::Reference< css::document::XActionLockable > xTargetLock(xTargetFrame, css::uno::UNO_QUERY);
    m_aTargetLock.setResource(xTargetLock);

    // Add status indicator to descriptor. Loader can show an progresses then.
    // But don't do it, if loading should be hidden or preview is used ...!
    // So we prevent our code against wrong using. Why?
    // It could be, that using of this progress could make trouble. e.g. He make window visible ...
    // but shouldn't do that. But if no indicator is available ... nobody has a chance to do that!
    sal_Bool                                           bHidden    = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_HIDDEN()         , sal_False                                           );
    sal_Bool                                           bMinimized = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_MINIMIZED()      , sal_False                                           );
    sal_Bool                                           bPreview   = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_PREVIEW()        , sal_False                                           );
    css::uno::Reference< css::task::XStatusIndicator > xProgress  = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_STATUSINDICATOR(), css::uno::Reference< css::task::XStatusIndicator >());

    if (!bHidden && !bMinimized && !bPreview && !xProgress.is())
    {
        // Note: its an optional interface!
        css::uno::Reference< css::task::XStatusIndicatorFactory > xProgressFactory(xTargetFrame, css::uno::UNO_QUERY);
        if (xProgressFactory.is())
        {
            xProgress = xProgressFactory->createStatusIndicator();
            if (xProgress.is())
                m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_STATUSINDICATOR()] <<= xProgress;
        }
    }

    // convert media descriptor and URL to right format for later interface call!
    css::uno::Sequence< css::beans::PropertyValue > lDescriptor;
    m_lMediaDescriptor >> lDescriptor;
    ::rtl::OUString sURL = m_aURL.Complete;

    // try to locate any interested frame loader
    css::uno::Reference< css::uno::XInterface >                xLoader     = impl_searchLoader();
    css::uno::Reference< css::frame::XFrameLoader >            xAsyncLoader(xLoader, css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XSynchronousFrameLoader > xSyncLoader (xLoader, css::uno::UNO_QUERY);

    if (xAsyncLoader.is())
    {
        // SAFE -> -----------------------------------
        aWriteLock.lock();
        m_xAsynchronousJob = xAsyncLoader;
        LoadEnvListener* pListener = new LoadEnvListener(this);
        aWriteLock.unlock();
        // <- SAFE -----------------------------------

        css::uno::Reference< css::frame::XLoadEventListener > xListener(static_cast< css::frame::XLoadEventListener* >(pListener), css::uno::UNO_QUERY);
        xAsyncLoader->load(xTargetFrame, sURL, lDescriptor, xListener);

        return sal_True;
    }
    else
    if (xSyncLoader.is())
    {
        sal_Bool bResult = xSyncLoader->load(lDescriptor, xTargetFrame);
        // react for the result here, so the outside waiting
        // code can ask for it later.
        impl_setResult(bResult);
        // But the return value indicates a valid started(!) operation.
        // And thats true everxtimes, we reach this line :-)
        return sal_True;
    }

    aWriteLock.unlock();
    // <- SAFE

    return sal_False;
}


css::uno::Reference< css::uno::XInterface > LoadEnv::impl_searchLoader()
{
    // SAFE -> -----------------------------------
    ReadGuard aReadLock(m_aLock);

    // special mode to set an existing component on this frame
    // In such case the laoder is fix. It must be the SFX based implementation,
    // which can create a view on top of such xModel components :-)
    if (m_eContentType == E_CAN_BE_SET)
    {
        try
        {
            return m_xSMGR->createInstance(IMPLEMENTATIONNAME_GENERICFRAMELOADER);
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            {}
        throw LoadEnvException(LoadEnvException::ID_INVALID_ENVIRONMENT);
    }

    // Otherwhise ...
    // We need this type information to locate an registered frame loader
    // Without such information we can't work!
    ::rtl::OUString sType = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_TYPENAME(), ::rtl::OUString());
    if (sType.isEmpty())
        throw LoadEnvException(LoadEnvException::ID_INVALID_MEDIADESCRIPTOR);

    // try to locate any interested frame loader
    css::uno::Reference< css::lang::XMultiServiceFactory > xLoaderFactory(m_xSMGR->createInstance(SERVICENAME_FRAMELOADERFACTORY), css::uno::UNO_QUERY);
    css::uno::Reference< css::container::XContainerQuery > xQuery        (xLoaderFactory                                         , css::uno::UNO_QUERY);

    aReadLock.unlock();
    // <- SAFE -----------------------------------

    css::uno::Sequence< ::rtl::OUString > lTypesReg(1);
    lTypesReg[0] = sType;

    css::uno::Sequence< css::beans::NamedValue > lQuery(1);
    lQuery[0].Name    = rtl::OUString(PROP_TYPES);
    lQuery[0].Value <<= lTypesReg;

    ::rtl::OUString sPROP_NAME(PROP_NAME);

    css::uno::Reference< css::container::XEnumeration > xSet = xQuery->createSubSetEnumerationByProperties(lQuery);
    while(xSet->hasMoreElements())
    {
        // try everyone ...
        // Ignore any loader, which makes trouble :-)
        ::comphelper::SequenceAsHashMap             lLoaderProps(xSet->nextElement());
        ::rtl::OUString                             sLoader     = lLoaderProps.getUnpackedValueOrDefault(sPROP_NAME, ::rtl::OUString());
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

    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE

    css::util::URL aCmd;
    aCmd.Complete = ::rtl::OUString(".uno:JumpToMark");

    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(::comphelper::getComponentContext(m_xSMGR)));
    xParser->parseStrict(aCmd);

    css::uno::Reference< css::frame::XDispatch > xDispatcher = xProvider->queryDispatch(aCmd, SPECIALTARGET_SELF, 0);
    if (! xDispatcher.is())
        return;

    ::comphelper::SequenceAsHashMap lArgs;
    lArgs[::rtl::OUString("Bookmark")] <<= aURL.Mark;
    xDispatcher->dispatch(aCmd, lArgs.getAsConstPropertyValueList());
}


css::uno::Reference< css::frame::XFrame > LoadEnv::impl_searchAlreadyLoaded()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    // such search is allowed for special requests only ...
    // or better its not allowed for some requests in general :-)
    if (
        ( ! TargetHelper::matchSpecialTarget(m_sTarget, TargetHelper::E_DEFAULT)                                               ) ||
        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_ASTEMPLATE() , sal_False) == sal_True) ||
//      (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_HIDDEN()     , sal_False) == sal_True) ||
        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_OPENNEWVIEW(), sal_False) == sal_True)
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    // check URL
    // May its not useful to start expensive document search, if it
    // can fail only .. because we load from a stream or model directly!
    if (
        (ProtocolCheck::isProtocol(m_aURL.Complete, ProtocolCheck::E_PRIVATE_STREAM )) ||
        (ProtocolCheck::isProtocol(m_aURL.Complete, ProtocolCheck::E_PRIVATE_OBJECT ))
        /*TODO should be private:factory here tested too? */
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    // otherwhise - iterate through the tasks of the desktop container
    // to find out, which of them might contains the requested document
    css::uno::Reference< css::frame::XFramesSupplier >  xSupplier(m_xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
    css::uno::Reference< css::container::XIndexAccess > xTaskList(xSupplier->getFrames()                      , css::uno::UNO_QUERY);

    if (!xTaskList.is())
        return css::uno::Reference< css::frame::XFrame >(); // task list can be empty!

    // Note: To detect if a document was alrady loaded before
    // we check URLs here only. But might the existing and the required
    // document has different versions! Then its URLs are the same ...
    sal_Int16 nNewVersion = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_VERSION(), (sal_Int16)(-1));

    // will be used to save the first hidden frame referring the searched model
    // Normally we are interested on visible frames ... but if there is no such visible
    // frame we referr to any hidden frame also (but as fallback only).
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
            const ::rtl::OUString sURL = xModel->getURL();
            if (!::utl::UCBContentHelper::EqualURLs( m_aURL.Main, sURL ))
            {
                xTask.clear ();
                continue;
            }

            // get the original load arguments from the current document
            // and decide if its really the same then the one will be.
            // It must be visible and must use the same file revision ...
            // or must not have any file revision set (-1 == -1!)
            ::comphelper::MediaDescriptor lOldDocDescriptor(xModel->getArgs());

            if (lOldDocDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_VERSION(), (sal_Int32)(-1)) != nNewVersion)
            {
                xTask.clear ();
                continue;
            }

            // Hidden frames are special.
            // They will be used as "last chance" if there is no visible frame pointing to the same model.
            // Safe the result but continue with current loop might be looking for other visible frames.
            ::sal_Bool bIsHidden = lOldDocDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_HIDDEN(), sal_False);
            if (
                (   bIsHidden       ) &&
                ( ! xHiddenTask.is())
               )
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
    else
    if (xHiddenTask.is())
        xResult = xHiddenTask;

    if (xResult.is())
    {
        // Now we are sure, that this task includes the searched document.
        // It's time to activate it. As special feature we try to jump internally
        // if an optional jumpmark is given too.
        if (!m_aURL.Mark.isEmpty())
            impl_jumpToMark(xResult, m_aURL);

        // bring it to front and make sure it's visible...
        impl_makeFrameWindowVisible(xResult->getContainerWindow(), sal_True);
    }

    aReadLock.unlock();
    // <- SAFE

    return xResult;
}


sal_Bool LoadEnv::impl_isFrameAlreadyUsedForLoading(const css::uno::Reference< css::frame::XFrame >& xFrame) const
{
    css::uno::Reference< css::document::XActionLockable > xLock(xFrame, css::uno::UNO_QUERY);

    // ? no lock interface ?
    // Might its an external written frame implementation :-(
    // Allowing using of it ... but it can fail if its not synchronized with our processes !
    if (!xLock.is())
        return sal_False;

    // Otherwise we have to look for any other existing lock.
    return xLock->isActionLocked();
}


css::uno::Reference< css::frame::XFrame > LoadEnv::impl_searchRecycleTarget()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // SAFE -> ..................................
    ReadGuard aReadLock(m_aLock);

    // The special backing mode frame will be recycled by definition!
    // It doesn't matter if somehwere whish to create a new view
    // or open a new untitled document ...
    // The only exception form that - hidden frames!
    if (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_HIDDEN(), sal_False) == sal_True)
        return css::uno::Reference< css::frame::XFrame >();

    css::uno::Reference< css::frame::XFramesSupplier > xSupplier(m_xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
    FrameListAnalyzer aTasksAnalyzer(xSupplier, css::uno::Reference< css::frame::XFrame >(), FrameListAnalyzer::E_BACKINGCOMPONENT);
    if (aTasksAnalyzer.m_xBackingComponent.is())
    {
        if (!impl_isFrameAlreadyUsedForLoading(aTasksAnalyzer.m_xBackingComponent))
        {
            // bring it to front ...
            impl_makeFrameWindowVisible(aTasksAnalyzer.m_xBackingComponent->getContainerWindow(), sal_True);
            return aTasksAnalyzer.m_xBackingComponent;
        }
    }

    // These states indicates a wish for creation of a new view in general.
    if (
        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_ASTEMPLATE() , sal_False) == sal_True) ||
        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_OPENNEWVIEW(), sal_False) == sal_True)
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    // On the other side some special URLs will open a new frame everytimes (expecting
    // they can use the backing-mode frame!)
    if (
        (ProtocolCheck::isProtocol(m_aURL.Complete, ProtocolCheck::E_PRIVATE_FACTORY )) ||
        (ProtocolCheck::isProtocol(m_aURL.Complete, ProtocolCheck::E_PRIVATE_STREAM  )) ||
        (ProtocolCheck::isProtocol(m_aURL.Complete, ProtocolCheck::E_PRIVATE_OBJECT  ))
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

    // not a real error - may its a view only
    css::uno::Reference< css::frame::XController > xController = xTask->getController();
    if (!xController.is())
        return css::uno::Reference< css::frame::XFrame >();

    // not a real error - may its a db component instead of a full feartured office document
    css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
    if (!xModel.is())
        return css::uno::Reference< css::frame::XFrame >();

    // get some more informations ...

    // A valid set URL means: there is already a location for this document.
    // => it was saved there or opened from there. Such Documents can not be used here.
    // We search for empty document ... created by a private:factory/ URL!
    if (xModel->getURL().getLength()>0)
        return css::uno::Reference< css::frame::XFrame >();

    // The old document must be unmodified ...
    css::uno::Reference< css::util::XModifiable > xModified(xModel, css::uno::UNO_QUERY);
    if (xModified->isModified())
        return css::uno::Reference< css::frame::XFrame >();

    Window* pWindow = VCLUnoHelper::GetWindow(xTask->getContainerWindow());
    if (pWindow && pWindow->IsInModalMode())
        return css::uno::Reference< css::frame::XFrame >();

    // find out the application type of this document
    // We can recycle only documents, which uses the same application
    // then the new one.
    SvtModuleOptions::EFactory eOldApp = SvtModuleOptions::ClassifyFactoryByModel(xModel);
    SvtModuleOptions::EFactory eNewApp = SvtModuleOptions::ClassifyFactoryByURL  (m_aURL.Complete, m_lMediaDescriptor.getAsConstPropertyValueList());

    aReadLock.unlock();
    // <- SAFE ..................................

    if (eOldApp != eNewApp)
        return css::uno::Reference< css::frame::XFrame >();

    // OK this task seams to be useable for recycling
    // But we should mark it as such - means set an action lock.
    // Otherwhise it would be used more then ones or will be destroyed
    // by a close() or terminate() request.
    // But if such lock already exist ... it means this task is used for
    // any other operation already. Don't use it then.
    if (impl_isFrameAlreadyUsedForLoading(xTask))
        return css::uno::Reference< css::frame::XFrame >();

    // OK - there is a valid target frame.
    // But may be it contains already a document.
    // Then we have to ask it, if it allows recylcing of this frame .-)
    sal_Bool bReactivateOldControllerOnError = sal_False;
    css::uno::Reference< css::frame::XController > xOldDoc = xTask->getController();
    if (xOldDoc.is())
    {
        bReactivateOldControllerOnError = xOldDoc->suspend(sal_True);
        if (! bReactivateOldControllerOnError)
            return css::uno::Reference< css::frame::XFrame >();
    }

    // SAFE -> ..................................
    WriteGuard aWriteLock(m_aLock);

    css::uno::Reference< css::document::XActionLockable > xLock(xTask, css::uno::UNO_QUERY);
    if (!m_aTargetLock.setResource(xLock))
        return css::uno::Reference< css::frame::XFrame >();

    m_bReactivateControllerOnError = bReactivateOldControllerOnError;
    aWriteLock.unlock();
    // <- SAFE ..................................

    // bring it to front ...
    impl_makeFrameWindowVisible(xTask->getContainerWindow(), sal_True);

    return xTask;
}


void LoadEnv::impl_reactForLoadingState()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    /*TODO reset action locks */

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    if (m_bLoaded)
    {
        // Bring the new loaded document to front (if allowed!).
        // Note: We show new created frames here only.
        // We dont hide already visible frames here ...
        css::uno::Reference< css::awt::XWindow > xWindow      = m_xTargetFrame->getContainerWindow();
        sal_Bool                                 bHidden      = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_HIDDEN(), sal_False);
        sal_Bool                                 bMinimized = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_MINIMIZED(), sal_False);

        if (bMinimized)
        {
            SolarMutexGuard aSolarGuard;
            Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
            // check for system window is neccessary to guarantee correct pointer cast!
            if (pWindow && pWindow->IsSystemWindow())
                ((WorkWindow*)pWindow)->Minimize();
        }
        else
        if (!bHidden)
        {
            // show frame ... if it's not still visible ...
            // But do nothing if it's already visible!
            impl_makeFrameWindowVisible(xWindow, sal_False);
        }

        // Note: Only if an existing property "FrameName" is given by this media descriptor,
        // it should be used. Otherwhise we should do nothing. May be the outside code has already
        // set a frame name on the target!
        ::comphelper::MediaDescriptor::const_iterator pFrameName = m_lMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_FRAMENAME());
        if (pFrameName != m_lMediaDescriptor.end())
        {
            ::rtl::OUString sFrameName;
            pFrameName->second >>= sFrameName;
            // Check the name again. e.g. "_default" isnt allowed.
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
            sal_Bool bReactivated = xOldDoc->suspend(sal_False);
            if (!bReactivated)
                throw LoadEnvException(LoadEnvException::ID_COULD_NOT_REACTIVATE_CONTROLLER);
            m_bReactivateControllerOnError = sal_False;
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

    // This max force an implicit closing of our target frame ...
    // e.g. in case close(sal_True) was called before and the frame
    // kill itself if our external use-lock is released here!
    // Thats why we releas this lock AFTER ALL OPERATIONS on this frame
    // are finished. The frame itslef must handle then
    // this situation gracefully.
    m_aTargetLock.freeResource();

    // Last but not least :-)
    // We have to clear the current media descriptor.
    // Otherwhise it hold a might existing stream open!
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
            throw LoadEnvException( LoadEnvException::ID_GENERAL_ERROR, aRequest );
    }

    // <- SAFE ----------------------------------
}


void LoadEnv::impl_makeFrameWindowVisible(const css::uno::Reference< css::awt::XWindow >& xWindow      ,
                                                sal_Bool bForceToFront)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR( m_xSMGR.get(), css::uno::UNO_QUERY );
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    SolarMutexGuard aSolarGuard;
    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    if ( pWindow )
    {
        bool const preview( m_lMediaDescriptor.getUnpackedValueOrDefault(
                ::comphelper::MediaDescriptor::PROP_PREVIEW(), sal_False) );

        bool bForceFrontAndFocus(false);
        if ( !preview )
        {
            css::uno::Any const a =
                ::comphelper::ConfigurationHelper::readDirectKey(
                  xSMGR,
                  ::rtl::OUString("org.openoffice.Office.Common/View"),
                  ::rtl::OUString("NewDocumentHandling"),
                  ::rtl::OUString("ForceFocusAndToFront"),
                  ::comphelper::ConfigurationHelper::E_READONLY);
            a >>= bForceFrontAndFocus;
        }

        if( pWindow->IsVisible() && (bForceFrontAndFocus || bForceToFront) )
            pWindow->ToTop();
        else
            pWindow->Show(sal_True, (bForceFrontAndFocus || bForceToFront) ? SHOW_FOREGROUNDTASK : 0 );
    }
}


void LoadEnv::impl_applyPersistentWindowState(const css::uno::Reference< css::awt::XWindow >& xWindow)
{
    static ::rtl::OUString PACKAGE_SETUP_MODULES("/org.openoffice.Setup/Office/Factories");

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

    Window*  pWindow       = VCLUnoHelper::GetWindow(xWindow);
    if (!pWindow)
        return;

    sal_Bool bSystemWindow = pWindow->IsSystemWindow();
    sal_Bool bWorkWindow   = (pWindow->GetType() == WINDOW_WORKWINDOW);

    if (!bSystemWindow && !bWorkWindow)
        return;

    // dont overwrite this special state!
    WorkWindow* pWorkWindow = (WorkWindow*)pWindow;
    if (pWorkWindow->IsMinimized())
        return;

    aSolarGuard1.clear();
    // <- SOLAR SAFE

    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    // no filter -> no module -> no persistent window state
    ::rtl::OUString sFilter = m_lMediaDescriptor.getUnpackedValueOrDefault(
                                    ::comphelper::MediaDescriptor::PROP_FILTERNAME(),
                                    ::rtl::OUString());
    if (sFilter.isEmpty())
        return;

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;

    aReadLock.unlock();
    // <- SAFE

    try
    {
        // retrieve the module name from the filter configuration
        css::uno::Reference< css::container::XNameAccess > xFilterCfg(
            xSMGR->createInstance(SERVICENAME_FILTERFACTORY),
            css::uno::UNO_QUERY_THROW);
        ::comphelper::SequenceAsHashMap lProps (xFilterCfg->getByName(sFilter));
        ::rtl::OUString                 sModule = lProps.getUnpackedValueOrDefault(FILTER_PROPNAME_DOCUMENTSERVICE, ::rtl::OUString());

        // get access to the configuration of this office module
        css::uno::Reference< css::container::XNameAccess > xModuleCfg(::comphelper::ConfigurationHelper::openConfig(
                                                                        xSMGR,
                                                                        PACKAGE_SETUP_MODULES,
                                                                        ::comphelper::ConfigurationHelper::E_READONLY),
                                                                      css::uno::UNO_QUERY_THROW);

        // read window state from the configuration
        // and apply it on the window.
        // Do nothing, if no configuration entry exists!
        ::rtl::OUString sWindowState ;
        ::comphelper::ConfigurationHelper::readRelativeKey(xModuleCfg, sModule, OFFICEFACTORY_PROPNAME_WINDOWATTRIBUTES) >>= sWindowState;
        if (!sWindowState.isEmpty())
        {
            // SOLAR SAFE ->
            SolarMutexGuard aSolarGuard;

            // We have to retrieve the window pointer again. Because nobody can guarantee
            // that the XWindow was not disposed inbetween .-)
            // But if we get a valid pointer we can be sure, that it's the system window pointer
            // we already checked and used before. Because nobody recylce the same uno reference for
            // a new internal c++ implementation ... hopefully .-))
            Window* pWindowCheck  = VCLUnoHelper::GetWindow(xWindow);
            if (! pWindowCheck)
                return;

            SystemWindow* pSystemWindow = (SystemWindow*)pWindowCheck;
            pSystemWindow->SetWindowState(rtl::OUStringToOString(sWindowState,RTL_TEXTENCODING_UTF8));
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
