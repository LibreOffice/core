/*************************************************************************
 *
 *  $RCSfile: loadenv.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:08:28 $
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

//_______________________________________________
// includes of own project

#ifndef __FRAMEWORK_LOADENV_LOADENV_HXX_
#include <loadenv/loadenv.hxx>
#endif

#ifndef __FRAMEWORK_LOADENV_TARGETHELPER_HXX_
#include <loadenv/targethelper.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_FRAMELISTANALYZER_HXX_
#include <classes/framelistanalyzer.hxx>
#endif

#ifndef __FRAMEWORK_CONSTANT_FILTER_HXX_
#include <constant/filter.hxx>
#endif

#ifndef __FRAMEWORK_CONSTANT_FRAMELOADER_HXX_
#include <constant/frameloader.hxx>
#endif

#ifndef __FRAMEWORK_CONSTANT_CONTENTHANDLER_HXX_
#include <constant/contenthandler.hxx>
#endif

#ifndef __FRAMEWORK_CONSTANT_CONTAINERQUERY_HXX_
#include <constant/containerquery.hxx>
#endif

#ifndef __FRAMEWORK_INTERACTION_STILLINTERACTION_HXX_
#include <interaction/stillinteraction.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_PROTOCOLS_H_
#include <protocols.h>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_______________________________________________
// includes of uno interface

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADER_HPP_
#include <com/sun/star/frame/XFrameLoader.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSYNCHRONOUSFRAMELOADER_HPP_
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XACTIONLOCKABLE_HPP_
#include <com/sun/star/document/XActionLockable.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERQUERY_HPP_
#include <com/sun/star/container/XContainerQuery.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif

//_______________________________________________
// includes of an other project

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_SYSWIN_HXX
#include <vcl/syswin.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//_______________________________________________
// namespace

namespace framework{

// may there exist already a define .-(
#ifndef css
namespace css = ::com::sun::star;
#endif

//_______________________________________________
// declarations

class LoadEnvListener : private ThreadHelpBase
                      , public ::cppu::WeakImplHelper2< css::frame::XLoadEventListener      ,
                                                        css::frame::XDispatchResultListener >
{
    private:

        void**   m_ppCheck ;
        LoadEnv* m_pLoadEnv;

    public:

        //_______________________________________
        LoadEnvListener(void*    pCheck  ,
                        LoadEnv* pLoadEnv)
        {
            m_ppCheck  = &pCheck ;
            m_pLoadEnv = pLoadEnv;
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

/*-----------------------------------------------
    14.10.2003 13:43
-----------------------------------------------*/
LoadEnv::LoadEnv(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    throw(LoadEnvException, css::uno::RuntimeException)
    : ThreadHelpBase(     )
    , m_xSMGR       (xSMGR)
    , m_pCheck      (this )
{
}

/*-----------------------------------------------
    14.10.2003 13:43
-----------------------------------------------*/
LoadEnv::~LoadEnv()
{
    m_pCheck = 0;
}

/*-----------------------------------------------
    10.09.2003 14:05
-----------------------------------------------*/
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
                            ::rtl::OUString::createFromAscii("Optional list of arguments seem to be corrupted."),
                            xLoader,
                            4);
                    break;

            case LoadEnvException::ID_UNSUPPORTED_CONTENT:
                    throw css::lang::IllegalArgumentException(
                            ::rtl::OUString::createFromAscii("URL seems to be an unsupported one."),
                            xLoader,
                            1);
                    break;

            default: xComponent.clear();
                    break;
        }
    }

    return xComponent;
}

/*-----------------------------------------------
    20.08.2003 09:49
-----------------------------------------------*/
void LoadEnv::initializeLoading(const ::rtl::OUString&                                           sURL            ,
                                const css::uno::Sequence< css::beans::PropertyValue >&           lMediaDescriptor,
                                const css::uno::Reference< css::frame::XFrame >&                 xBaseFrame      ,
                                const ::rtl::OUString&                                           sTarget         ,
                                      sal_Int32                                                  nSearchFlags    ,
                                      EFeature                                                   eFeature        , // => use default ...
                                      EContentType                                               eContentType    ) // => use default ...
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // Handle still running processes!
    if (m_xAsynchronousJob.is())
        throw LoadEnvException(LoadEnvException::ID_STILL_RUNNING);

    // take over all new parameters.
    m_xTargetFrame.clear();
    m_xBaseFrame                    = xBaseFrame        ;
    m_lMediaDescriptor              << lMediaDescriptor ;
    m_sTarget                       = sTarget           ;
    m_nSearchFlags                  = nSearchFlags      ;
    m_eFeature                      = eFeature          ;
    m_eContentType                  = eContentType      ;
    m_bCloseFrameOnError            = sal_False         ;
    m_bReactivateControllerOnError  = sal_False         ;
    m_bLoaded                       = sal_False         ;

    // try to find out, if its realy a content, which can be loaded or must be "handled"
    // We use a default value for this in-parameter. Then we have to start a complex check method
    // internaly. But if this check was already done outside it can be supressed to perform
    // the load request. We take over the result then!
    if (m_eContentType == E_UNSUPPORTED_CONTENT)
    {
        m_eContentType = LoadEnv::classifyContent(sURL, lMediaDescriptor);
        if (m_eContentType == E_UNSUPPORTED_CONTENT)
            throw LoadEnvException(LoadEnvException::ID_UNSUPPORTED_CONTENT);
    }

    // make URL part of the MediaDescriptor
    // It doesnt mater, if its already an item of it.
    // It must be the same value ... so we can overwrite it :-)
    m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_URL()] <<= sURL;

    // parse it - because some following code require that
    m_aURL.Complete = sURL;
    css::uno::Reference< css::util::XURLTransformer > xParser(m_xSMGR->createInstance(SERVICENAME_URLTRANSFORMER), css::uno::UNO_QUERY);
    xParser->parseStrict(m_aURL);

    // BTW: Split URL and JumpMark ...
    // Because such mark is an explicit value of the media descriptor!
    if (m_aURL.Mark.getLength())
        m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_JUMPMARK()] <<= m_aURL.Mark;

    // By the way: remove the old and deprecated value "FileName" from the descriptor!
    ::comphelper::MediaDescriptor::iterator pIt = m_lMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_FILENAME());
    if (pIt != m_lMediaDescriptor.end())
        m_lMediaDescriptor.erase(pIt);

    // patch the MediaDescriptor, so it fullfill the outside requirements
    // Means especialy items like e.g. UI InteractionHandler, Status Indicator,
    // MacroExecutionMode etcpp.

    /*TODO progress is bound to a frame ... How can we set it here? */

    css::uno::Reference< css::task::XInteractionHandler > xInteractionHandler;
    sal_Int16                                             nMacroMode         ;
    sal_Int16                                             nUpdateMode        ;

    // UI mode
    if (
        ((m_eFeature & E_WORK_WITH_UI)                                                                          == E_WORK_WITH_UI) &&
        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_HIDDEN() , sal_False) == sal_False     ) &&
        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_PREVIEW(), sal_False) == sal_False     )
       )
    {
        nMacroMode  = css::document::MacroExecMode::USE_CONFIG;
        nUpdateMode = css::document::UpdateDocMode::ACCORDING_TO_CONFIG;
        try
        {
            xInteractionHandler = css::uno::Reference< css::task::XInteractionHandler >(m_xSMGR->createInstance(IMPLEMENTATIONNAME_UIINTERACTIONHANDLER), css::uno::UNO_QUERY);
        }
        catch(const css::uno::RuntimeException&) {throw;}
        catch(const css::uno::Exception&       ) {      }
    }
    // hidden mode
    else
    {
        nMacroMode  = css::document::MacroExecMode::NEVER_EXECUTE;
        nUpdateMode = css::document::UpdateDocMode::NO_UPDATE;
        StillInteraction* pInteraction = new StillInteraction();
        xInteractionHandler = css::uno::Reference< css::task::XInteractionHandler >(static_cast< css::task::XInteractionHandler* >(pInteraction), css::uno::UNO_QUERY);
    }

    if (
        (xInteractionHandler.is()                                                                                            ) &&
        (m_lMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_INTERACTIONHANDLER()) == m_lMediaDescriptor.end())
       )
    {
        m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_INTERACTIONHANDLER()] <<= xInteractionHandler;
    }

    if (m_lMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_MACROEXECUTIONMODE()) == m_lMediaDescriptor.end())
        m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_MACROEXECUTIONMODE()] <<= nMacroMode;

    if (m_lMediaDescriptor.find(::comphelper::MediaDescriptor::PROP_UPDATEDOCMODE()) == m_lMediaDescriptor.end())
        m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_UPDATEDOCMODE()] <<= nUpdateMode;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

/*-----------------------------------------------
    15.08.2003 08:16
-----------------------------------------------*/
void LoadEnv::startLoading()
    throw(LoadEnvException, css::uno::RuntimeException)
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
    // We cant say - what was the reason for.
    if (!bStarted)
        throw LoadEnvException(LoadEnvException::ID_GENERAL_ERROR);
}

/*-----------------------------------------------
    15.08.2003 09:50
    TODO
        First draft does not implement timeout using [ms].
        Current implementation counts yield calls only ...
-----------------------------------------------*/
sal_Bool LoadEnv::waitWhileLoading(sal_uInt32 nTimeout)
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // Because its not a good idea to block the main thread
    // (and we cant be shure that we are currently not used inside the
    // main thread!), we cant use conditions here realy. We must yield
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

/*-----------------------------------------------
    20.08.2003 10:00
-----------------------------------------------*/
void LoadEnv::cancelLoading()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // PARTIAL(!) SAFE -> ------------------------------
    ReadGuard aReadLock(m_aLock);

    // Still running? Might waitWhileLoading()
    // runned into the timeout!
    if (m_xAsynchronousJob.is())
    {
        // try to cancel it ... if its an asynchronous frame loader
        css::uno::Reference< css::frame::XFrameLoader > xAsyncLoader(m_xAsynchronousJob, css::uno::UNO_QUERY);
        if (xAsyncLoader.is())
        {
            aReadLock.unlock();
            // <- BREAK SAFE ------------------------------
            xAsyncLoader->cancel();
            // <- RESTART SAFE ----------------------------
            aReadLock.lock();
            /* Attention:
                After returning from any cancel/dispose call, neither the frame nor weself
                may be called back. Because only we can cancel this job, we already know
                the result! => Thats why its not usefull nor neccessary to wait for any
                asynchronous listener notification.
            */
            m_bLoaded = sal_False;
            m_xAsynchronousJob.clear();
        }
        // or may be its a content handler? Such handler cant be cancelled in its running
        // operation :-( And we cant deregister us there again :-(
        // => The only chance is an exception :-)
        else
            throw LoadEnvException(LoadEnvException::ID_STILL_RUNNING);
    }

    impl_reactForLoadingState();

    aReadLock.unlock();
    // <- PARTIAL(!) SAFE ------------------------------
}

/*-----------------------------------------------
    14.08.2003 13:33
-----------------------------------------------*/
css::uno::Reference< css::frame::XFrame > LoadEnv::getTarget() const
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    return m_xTargetFrame;
    // <- SAFE
}

/*-----------------------------------------------
    14.08.2003 13:35
-----------------------------------------------*/
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

/*-----------------------------------------------
    15.08.2003 11:15
-----------------------------------------------*/
void SAL_CALL LoadEnvListener::loadFinished(const css::uno::Reference< css::frame::XFrameLoader >& xLoader)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (m_ppCheck && *m_ppCheck)
        m_pLoadEnv->impl_setResult(sal_True);
    m_ppCheck = NULL;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

/*-----------------------------------------------
    14.10.2003 12:23
-----------------------------------------------*/
void SAL_CALL LoadEnvListener::loadCancelled(const css::uno::Reference< css::frame::XFrameLoader >& xLoader)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (m_ppCheck && *m_ppCheck)
        m_pLoadEnv->impl_setResult(sal_False);
    m_ppCheck = NULL;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

/*-----------------------------------------------
    14.10.2003 12:23
-----------------------------------------------*/
void SAL_CALL LoadEnvListener::dispatchFinished(const css::frame::DispatchResultEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (!m_ppCheck || !*m_ppCheck)
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
    m_ppCheck = NULL;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

/*-----------------------------------------------
    14.10.2003 12:24
-----------------------------------------------*/
void SAL_CALL LoadEnvListener::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (m_ppCheck && *m_ppCheck)
        m_pLoadEnv->impl_setResult(sal_False);
    m_ppCheck = NULL;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

/*-----------------------------------------------
    14.10.2003 12:20
-----------------------------------------------*/
void LoadEnv::impl_setResult(sal_Bool bResult)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    m_bLoaded = bResult;

    impl_reactForLoadingState();

    // clearing of this reference will unblock waitWhileLoading()!
    // So we must be shure, that loading process was realy finished.
    // => do it as last operation of this method ...
    m_xAsynchronousJob.clear();

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

/*-----------------------------------------------
    06.02.2004 14:03
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
    //     So we have to make shure, that the following code
    //     can detect such protocol schemata too :-)

    if(
        (!sURL.getLength()                                       ) ||
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

    /* Attention: The following code cant work on such special URLs!
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
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::utl::getProcessServiceFactory();
    css::uno::Reference< css::document::XTypeDetection > xDetect(xSMGR->createInstance(SERVICENAME_TYPEDETECTION), css::uno::UNO_QUERY);

    ::rtl::OUString sType = xDetect->queryTypeByURL(sURL);

    css::uno::Sequence< css::beans::NamedValue >           lQuery(1)   ;
    css::uno::Reference< css::container::XContainerQuery > xContainer  ;
    css::uno::Reference< css::container::XEnumeration >    xSet        ;
    css::uno::Sequence< ::rtl::OUString >                  lTypesReg(1);

    /*
    //-------------------------------------------
    lQuery[0].Name    = ::framework::constant::Filter::PROP_TYPE;
    lQuery[0].Value <<= sType;

    xContainer = css::uno::Reference< css::container::XContainerQuery >(xSMGR->createInstance(SERVICENAME_FILTERFACTORY), css::uno::UNO_QUERY);
    xSet       = xContainer->createSubSetEnumerationByProperties(lQuery);
    // at least one registered frame loader is enough!
    if (xSet->hasMoreElements())
        return E_CAN_BE_LOADED;
    */

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

    lTypesReg[0]      = sType;
    lQuery[0].Name    = ::framework::constant::FrameLoader::PROP_TYPES;
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
    lQuery[0].Name    = ::framework::constant::ContentHandler::PROP_TYPES;
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
    css::uno::Reference< css::ucb::XContentProviderManager > xUCB(xSMGR->createInstance(SERVICENAME_UCBCONTENTBROKER), css::uno::UNO_QUERY);
    if (xUCB->queryContentProvider(sURL).is())
        return E_CAN_BE_LOADED;

    //-------------------------------------------
    // (TODO) At this point, we have no idea .-)
    //        But it seems to be better, to break all
    //        further requests for this URL. Otherwhise
    //        we can run into some trouble.
    LOG_WARNING("LoadEnv::classifyContent()", "realy an unsupported content?")
    return E_UNSUPPORTED_CONTENT;
}

/*-----------------------------------------------
    03.11.2003 09:31
-----------------------------------------------*/
void LoadEnv::impl_detectTypeAndFilter()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    // start combined flat/deep detection
    // It can agree with the current preselection or return any
    // other type, which match to the specified content!

    // Attention: Because our stl media descriptor is a copy of an uno sequence
    // we cant use as an in/out parameter here. Copy it before and dont forget to
    // actualize structure afterwards again!
    css::uno::Reference< css::document::XTypeDetection > xDetect(m_xSMGR->createInstance(SERVICENAME_TYPEDETECTION), css::uno::UNO_QUERY);

    css::uno::Sequence< css::beans::PropertyValue > lDescriptor;
    m_lMediaDescriptor >> lDescriptor;
    ::rtl::OUString sDetectedType = xDetect->queryTypeByDescriptor(lDescriptor, sal_True); /*TODO should deep detection be able for enable/disable it from outside? */
    m_lMediaDescriptor << lDescriptor;

    // a) content couldn't be detected successfully => return immediatly
    //    MediaDescriptor should be already up-to-date here!
    if (!sDetectedType.getLength())
        throw LoadEnvException(LoadEnvException::ID_UNSUPPORTED_CONTENT);

    // b) if detection was successfully => update the descriptor
    m_lMediaDescriptor[::comphelper::MediaDescriptor::PROP_TYPENAME()] <<= sDetectedType;

    aWriteLock.unlock();
    // <- SAFE
}

/*-----------------------------------------------
    15.08.2003 09:38
-----------------------------------------------*/
sal_Bool LoadEnv::impl_handleContent()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // SAFE -> -----------------------------------
    ReadGuard aReadLock(m_aLock);

    // the type must exist inside the descriptor ... otherwhise this class is implemented wrong :-)
    ::rtl::OUString sType = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_TYPENAME(), ::rtl::OUString());
    if (!sType.getLength())
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
    lQuery[0].Name    = ::framework::constant::ContentHandler::PROP_TYPES;
    lQuery[0].Value <<= lTypeReg;

    css::uno::Reference< css::container::XEnumeration > xSet = xQuery->createSubSetEnumerationByProperties(lQuery);
    while(xSet->hasMoreElements())
    {
        ::comphelper::SequenceAsHashMap lProps   (xSet->nextElement());
        ::rtl::OUString                 sHandler = lProps.getUnpackedValueOrDefault(::framework::constant::ContentHandler::PROP_NAME, ::rtl::OUString());

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
        m_pCheck           = this;
        LoadEnvListener* pListener = new LoadEnvListener(m_pCheck, this);
        aWriteLock.unlock();
        // <- SAFE -----------------------------------

        css::uno::Reference< css::frame::XDispatchResultListener > xListener(static_cast< css::frame::XDispatchResultListener* >(pListener), css::uno::UNO_QUERY);
        xHandler->dispatchWithNotification(aURL, lDescriptor, xListener);

        return sal_True;
    }

    return sal_False;
}

/*-----------------------------------------------
    15.08.2003 09:35
-----------------------------------------------*/
sal_Bool LoadEnv::impl_loadContent()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // SAFE -> -----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // search or create right target frame
    if (TargetHelper::matchSpecialTarget(m_sTarget, TargetHelper::E_DEFAULT))
    {
        m_xTargetFrame = impl_searchAlreadyLoaded();
        // document is already loaded ... => break loading with SUCCESS=TRUE!
        if (m_xTargetFrame.is())
        {
            impl_setResult(sal_True);
            return sal_True;
        }
        m_xTargetFrame = impl_searchRecycleTarget();
        if (!m_xTargetFrame.is())
        {
            m_xTargetFrame = m_xBaseFrame->findFrame(SPECIALTARGET_BLANK, 0);
            // it is a new created target frame ... we have to close it
            // in case loading failed. Because its an hidden frame now!
            m_bCloseFrameOnError = m_xTargetFrame.is();
        }
    }
    else
        m_xTargetFrame = m_xBaseFrame->findFrame(m_sTarget, m_nSearchFlags);

    if (!m_xTargetFrame.is())
        throw LoadEnvException(LoadEnvException::ID_NO_TARGET_FOUND);
    css::uno::Reference< css::frame::XFrame > xTargetFrame = m_xTargetFrame;

    // OK - there is a valid target frame.
    // But may be it contains already a valid document.
    css::uno::Reference< css::frame::XController > xOldDoc = xTargetFrame->getController();
    if (xOldDoc.is())
    {
        m_bReactivateControllerOnError = xOldDoc->suspend(sal_True);
        if (!m_bReactivateControllerOnError)
            throw LoadEnvException(LoadEnvException::ID_COULD_NOT_SUSPEND_CONTROLLER);
    }

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
        WriteGuard aWriteLock(m_aLock);
        m_xAsynchronousJob = xAsyncLoader;
        m_pCheck           = this;
        LoadEnvListener* pListener = new LoadEnvListener(m_pCheck, this);
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

/*-----------------------------------------------
    06.02.2004 14:40
-----------------------------------------------*/
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
    // Without such information we cant work!
    ::rtl::OUString sType = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_TYPENAME(), ::rtl::OUString());
    if (!sType.getLength())
        throw LoadEnvException(LoadEnvException::ID_INVALID_MEDIADESCRIPTOR);

    // try to locate any interested frame loader
    css::uno::Reference< css::lang::XMultiServiceFactory > xLoaderFactory(m_xSMGR->createInstance(SERVICENAME_FRAMELOADERFACTORY), css::uno::UNO_QUERY);
    css::uno::Reference< css::container::XContainerQuery > xQuery        (xLoaderFactory                                         , css::uno::UNO_QUERY);

    aReadLock.unlock();
    // <- SAFE -----------------------------------

    css::uno::Sequence< ::rtl::OUString > lTypesReg(1);
    lTypesReg[0] = sType;

    css::uno::Sequence< css::beans::NamedValue > lQuery(1);
    lQuery[0].Name    = ::framework::constant::FrameLoader::PROP_TYPES;
    lQuery[0].Value <<= lTypesReg;

    css::uno::Reference< css::container::XEnumeration > xSet = xQuery->createSubSetEnumerationByProperties(lQuery);
    while(xSet->hasMoreElements())
    {
        // try everyone ...
        // Ignore any loader, which makes trouble :-)
        ::comphelper::SequenceAsHashMap             lLoaderProps(xSet->nextElement());
        ::rtl::OUString                             sLoader     = lLoaderProps.getUnpackedValueOrDefault(::framework::constant::FrameLoader::PROP_NAME, ::rtl::OUString());
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

/*-----------------------------------------------
    31.07.2003 09:02
-----------------------------------------------*/
css::uno::Reference< css::frame::XFrame > LoadEnv::impl_searchAlreadyLoaded()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    // such search is allowed for special requests only ...
    // or better its not allowed for some requests in general :-)
    if (
        (!TargetHelper::matchSpecialTarget(m_sTarget, TargetHelper::E_DEFAULT)                                                        ) ||
        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_ASTEMPLATE() , sal_False) == sal_True) ||
        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_HIDDEN()     , sal_False) == sal_True) ||
        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_OPENNEWVIEW(), sal_False) == sal_True)
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    // check URL
    // May its not usefull to start expensive document search, if it
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
    // we check URLs here only. But might the existing and the requred
    // document has different versions! Then its URLs are the same ...
    sal_Int16 nNewVersion = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_VERSION(), (sal_Int16)(-1));

    sal_Int32 count = xTaskList->getCount();
    for (sal_Int32 i=0; i<count; ++i)
    {
        try
        {
            // locate model of task
            // Note: Without a model there is no chance to decide if
            // this task contains the searched document or not!
            css::uno::Reference< css::frame::XFrame > xTask;
            xTaskList->getByIndex(i) >>= xTask;
            if (!xTask.is())
                continue;

            css::uno::Reference< css::frame::XController > xController = xTask->getController();
            if (!xController.is())
                continue;

            css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
            if (!xModel.is())
                continue;

            // don't check the complete URL here.
            // use its main part - ignore optional jumpmarks!
            if (!m_aURL.Main.equals(xModel->getURL()))
                continue;

            // get the original load arguments from the current document
            // and decide if its realy the same then the one will be.
            // It must be visible and must use the same file revision ...
            // or must not have any file revision set (-1 == -1!)
            ::comphelper::MediaDescriptor lOldDocDescriptor(xModel->getArgs());

            if (lOldDocDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_VERSION(), (sal_Int32)(-1)) != nNewVersion)
                continue;

            if (lOldDocDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_HIDDEN(), sal_False) == sal_True)
                continue;

            // Now we are shure, that this task includes the searched document.
            // It's time to activate it. As special feature we try to jump internaly
            // if an optional jumpmark is given too.
            if (m_aURL.Mark.getLength())
            {
                css::uno::Reference< css::frame::XDispatchProvider > xProvider(xTask, css::uno::UNO_QUERY);
                if (xProvider.is())
                {
                    css::uno::Reference< css::frame::XDispatch > xDispatcher = xProvider->queryDispatch(m_aURL, SPECIALTARGET_SELF, 0);
                    if (xDispatcher.is())
                        xDispatcher->dispatch(m_aURL, m_lMediaDescriptor.getAsConstPropertyValueList());
                }
            }

            // bring it to front ...
            css::uno::Reference< css::awt::XWindow >    xTaskWindow = xTask->getContainerWindow();
            css::uno::Reference< css::awt::XTopWindow > xTopWindow  (xTaskWindow, css::uno::UNO_QUERY);
            // Check it! May we are plugged into a browser/java canvas etcpp ...
            // Then such "bring to front" operation isnt supported yet nor neccessary .-)
            if (xTopWindow.is())
            {
                xTaskWindow->setVisible(sal_True);
                xTopWindow->toFront();
            }

            // It doesn't matter if we was able to bring it to front.
            // But we have found such task and can return it as our result.
            return xTask;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }
    }

    aReadLock.unlock();
    // <- SAFE

    return css::uno::Reference< css::frame::XFrame >();
}

/*-----------------------------------------------
    30.03.2004 09:12
-----------------------------------------------*/
css::uno::Reference< css::frame::XFrame > LoadEnv::impl_searchRecycleTarget()
    throw(LoadEnvException, css::uno::RuntimeException)
{
    // SAFE -> ..................................
    ReadGuard aReadLock(m_aLock);

    // such search is allowed for special requests only ...
    // or better its not allowed for some requests in general :-)
    if (
//        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_ASTEMPLATE() , sal_False) == sal_True) ||
        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_HIDDEN()     , sal_False) == sal_True) ||
        (m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_OPENNEWVIEW(), sal_False) == sal_True)
       )
    {
        return css::uno::Reference< css::frame::XFrame >();
    }

    // get access to the list of possible open tasks
    css::uno::Reference< css::frame::XFramesSupplier > xSupplier(m_xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);

    // The special backing mode frame will be recycled everytimes!
    FrameListAnalyzer aTasksAnalyzer(xSupplier, css::uno::Reference< css::frame::XFrame >(), FrameListAnalyzer::E_BACKINGCOMPONENT);
    if (aTasksAnalyzer.m_xBackingComponent.is())
        return aTasksAnalyzer.m_xBackingComponent;

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
    css::uno::Reference< css::document::XActionLockable > xLock(xTask, css::uno::UNO_QUERY);

    // ? no lock interface ?
    // Might its an external written frame implementation :-(
    // Use it ... but it can fail if its not synchronized with our processes.
    if (!xLock.is())
        return xTask;

    // Otherwhise we have to look for any other existing lock.
    if (xLock->isActionLocked())
        return css::uno::Reference< css::frame::XFrame >();

    // SAFE -> ..................................
    WriteGuard aWriteLock(m_aLock);
    if (!m_aTargetLock.setResource(xLock))
        return css::uno::Reference< css::frame::XFrame >();
    aWriteLock.unlock();
    // <- SAFE ..................................

    return xTask;
}

/*-----------------------------------------------
    15.08.2003 12:39
-----------------------------------------------*/
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
        css::uno::Reference< css::awt::XWindow > xWindow    = m_xTargetFrame->getContainerWindow();
        sal_Bool                                 bHidden    = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_HIDDEN()   , sal_False);
        sal_Bool                                 bMinimized = m_lMediaDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_MINIMIZED(), sal_False);

        if (bMinimized)
        {
            ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());
            Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
            // check for system window is neccessary to guarantee correct pointer cast!
            if (pWindow && pWindow->IsSystemWindow())
                ((WorkWindow*)pWindow)->Minimize();
        }
        else
        if (!bHidden)
        {
            xWindow->setVisible(sal_True);
            css::uno::Reference< css::awt::XTopWindow > xTopWindow(xWindow, css::uno::UNO_QUERY);
            if(xTopWindow.is())
                xTopWindow->toFront();
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
    else
    if (m_bReactivateControllerOnError)
    {
        // Try to reactivate the old document (if any exists!)
        css::uno::Reference< css::frame::XController > xOldDoc = m_xTargetFrame->getController();
        // clear does not depend from reactivation state of a might existing old document!
        // We must make shure, that a might following getTargetComponent() call does not return
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
    else
    if (m_bCloseFrameOnError)
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
    // e.g. in case close(TRUE) was called before and the frame
    // kill itself if our external use-lock is released here!
    // Thats why we releas this lock AFTER ALL OPERATIONS on this frame
    // are finished. The frame itslef must handle then
    // this situation gracefully.
    m_aTargetLock.freeResource();

    // Last but not least :-)
    // We have to clear the current media descriptor.
    // Otherwhise it hold a might existing stream open!
    m_lMediaDescriptor.clear();

    aReadLock.unlock();
    // <- SAFE ----------------------------------
}

} // namespace framework
